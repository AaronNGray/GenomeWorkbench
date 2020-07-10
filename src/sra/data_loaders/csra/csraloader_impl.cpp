/*  $Id: csraloader_impl.cpp 593922 2019-09-26 15:39:24Z vasilche $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Author: Eugene Vasilchenko
 *
 * File Description: CSRA file data loader
 *
 * ===========================================================================
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>

#include <objects/general/general__.hpp>
#include <objects/seqloc/seqloc__.hpp>
#include <objects/seq/seq__.hpp>
#include <objects/seqres/seqres__.hpp>

#include <objmgr/impl/data_source.hpp>
#include <objmgr/impl/tse_loadlock.hpp>
#include <objmgr/impl/tse_chunk_info.hpp>
#include <objmgr/impl/tse_split_info.hpp>
#include <objmgr/data_loader_factory.hpp>
#include <corelib/plugin_manager_impl.hpp>
#include <corelib/plugin_manager_store.hpp>
#include <serial/objistr.hpp>
#include <serial/serial.hpp>

#include <sra/error_codes.hpp>
#include <sra/readers/ncbi_traces_path.hpp>
#include <sra/data_loaders/csra/csraloader.hpp>
#include <sra/data_loaders/csra/impl/csraloader_impl.hpp>

#include <util/sequtil/sequtil_manip.hpp>

#include <algorithm>
#include <cmath>

BEGIN_NCBI_SCOPE

#define NCBI_USE_ERRCODE_X   cSRALoader
NCBI_DEFINE_ERR_SUBCODE_X(16);

BEGIN_SCOPE(objects)

class CDataLoader;

// magic chunk ids
static const int kTSEId = 1;
static const int kMainChunkId = CTSE_Chunk_Info::kDelayedMain_ChunkId;

// algirithm options
static const bool kUseFullAlignCounts = true;
static const bool kEstimateAlignCounts = true;
static const bool kOmitEmptyPileup = true;

// splitter parameters for aligns and graphs
static const unsigned kAlignChunkSize = 1000;
static const unsigned kGraphChunkSize = 10000;
static const unsigned kAlignEmptyPages = 16;
static const unsigned kGraphEmptyPages = 4;
static const unsigned kChunkGraphMul = 8;
static const unsigned kChunkSeqDataMul = 8;

// splitter parameters for reads
static const unsigned kReadsPerBlob = 1;

// Seq-id parser parameters
static const unsigned kMaxReadId = 999;
static const Uint8 kMaxSpotId = NCBI_CONST_UINT8(999999999999999999);

#define SPOT_GROUP_SEPARATOR ": "
#define PILEUP_NAME_SUFFIX "pileup graphs"

NCBI_PARAM_DECL(int, CSRA_LOADER, DEBUG);
NCBI_PARAM_DEF_EX(int, CSRA_LOADER, DEBUG, 0,
                  eParam_NoThread, CSRA_LOADER_DEBUG);

static int GetDebugLevel(void)
{
    static CSafeStatic<NCBI_PARAM_TYPE(CSRA_LOADER, DEBUG)> s_Value;
    return s_Value->Get();
}


NCBI_PARAM_DECL(size_t, CSRA_LOADER, GC_SIZE);
NCBI_PARAM_DEF_EX(size_t, CSRA_LOADER, GC_SIZE, 100,
                  eParam_NoThread, CSRA_LOADER_GC_SIZE);

static size_t GetGCSize(void)
{
    static CSafeStatic<NCBI_PARAM_TYPE(CSRA_LOADER, GC_SIZE)> s_Value;
    return s_Value->Get();
}


/////////////////////////////////////////////////////////////////////////////
// CCSRABlobId
/////////////////////////////////////////////////////////////////////////////

CCSRABlobId::CCSRABlobId(const CTempString& str)
{
    FromString(str);
}


CCSRABlobId::CCSRABlobId(EBlobType blob_type,
                         const TRefLock& ref)
    : m_BlobType(blob_type),
      m_RefIdType(ref.first->m_File->GetRefIdType()),
      m_File(ref.first->m_File->GetCSRAName()),
      m_SeqId(ref.first->GetRefSeqId()),
      m_FirstSpotId(0),
      m_FileLock(ref.second)
{
    _ASSERT(blob_type < eBlobType_reads);
}


CCSRABlobId::CCSRABlobId(const TFileLock& file,
                         TVDBRowId first_spot_id)
    : m_BlobType(eBlobType_reads),
      m_RefIdType(file.first->GetRefIdType()),
      m_File(file.first->GetCSRAName()),
      m_FirstSpotId(first_spot_id),
      m_FileLock(file.second)
{
}


CCSRABlobId::~CCSRABlobId(void)
{
}


SIZE_TYPE CCSRABlobId::ParseReadId(CTempString str,
                                   TVDBRowId* spot_id_ptr,
                                   Uint4* read_id_ptr)
{
    const char* begin = str.data();
    const char* ptr = begin+str.size();
    const char* end = ptr;
    Uint8 number = 0;
    Uint8 number_digit = 1;
    bool parsing_read_id = true;
    for ( ; ptr != begin; ) {
        char c = *--ptr;
        if ( isdigit(c&0xff) ) {
            if ( parsing_read_id ) {
                if ( number_digit > kMaxReadId ) {
                    // too long read_id
                    return NPOS;
                }
            }
            else {
                if ( number_digit > kMaxSpotId ) {
                    // too long spot_id
                    return NPOS;
                }
            }
            number += (c-'0')*number_digit;
            number_digit *= 10;
        }
        else if ( c == '.' ) {
            // end of number
            if ( ptr == end ) {
                // no id part
                return NPOS;
            }
            if ( ptr[1] == '0' ) {
                // leading zero
                return NPOS;
            }
            if ( parsing_read_id ) {
                // got read_id
                if ( read_id_ptr ) {
                    *read_id_ptr = Uint4(number);
                }
                parsing_read_id = false;
                // prepare parsing spot_id number
                number = 0;
                number_digit = 1;
            }
            else {
                // got both spot_id and read_id
                if ( spot_id_ptr ) {
                    *spot_id_ptr = TVDBRowId(number);
                }
                break;
            }
            // start of next part
            end = ptr;
        }
        else {
            // bad format of short read id
            return NPOS;
        }
    }
    if ( ptr == begin ) {
        // no accession part
        return NPOS;
    }
    return ptr - begin;
}


CCSRABlobId::EGeneralIdType
CCSRABlobId::GetGeneralIdType(const CSeq_id_Handle& idh,
                              EGeneralIdType allow_type,
                              const string* srr)
{
    // id is of type gnl|SRA|<srr>/<label>
    // or gnl|SRA|<srr>.<spot>.<read>
    if ( idh.Which() != CSeq_id::e_General ) {
        return eNotGeneralIdType;
    }
    CConstRef<CSeq_id> id = idh.GetSeqId();
    const CDbtag& dbtag = id->GetGeneral();
    if ( !NStr::EqualNocase(dbtag.GetDb(), "SRA") ) {
        return eNotGeneralIdType;
    }
    if ( !dbtag.GetTag().IsStr() ) {
        return eNotGeneralIdType;
    }
    const string& str = dbtag.GetTag().GetStr();
    SIZE_TYPE srr_len = str.find('/');
    EGeneralIdType type;
    if ( srr_len != NPOS ) {
        if ( !(allow_type & eGeneralIdType_refseq) ) {
            return eNotGeneralIdType;
        }
        type = eGeneralIdType_refseq;
    }
    else {
        // check short read id
        if ( !(allow_type & eGeneralIdType_read) ) {
            return eNotGeneralIdType;
        }
        type = eGeneralIdType_read;
        srr_len = ParseReadId(str);
        if ( srr_len == NPOS ) {
            return eNotGeneralIdType;
        }
    }
    // check accession
    if ( srr ) {
        if ( srr->size() != srr_len ) {
            return eNotGeneralIdType;
        }
        if ( !NStr::StartsWith(str, *srr) ) {
            return eNotGeneralIdType;
        }
    }
    return type;
}


bool CCSRABlobId::GetGeneralSRAAccLabel(const CSeq_id_Handle& idh,
                                        string* srr_acc_ptr,
                                        string* label_ptr)
{
    // id is of type gnl|SRA|srr/label
    if ( !GetGeneralIdType(idh, eGeneralIdType_refseq) ) {
        return false;
    }
    CConstRef<CSeq_id> id = idh.GetSeqId();
    const string& str = id->GetGeneral().GetTag().GetStr();
    SIZE_TYPE srr_end = str.find('/');
    if ( srr_end == NPOS ) {
        return false;
    }
    if ( srr_acc_ptr ) {
        *srr_acc_ptr = str.substr(0, srr_end);
    }
    if ( label_ptr ) {
        *label_ptr = str.substr(srr_end+1);
    }
    return true;
}


bool CCSRABlobId::GetGeneralSRAAccReadId(const CSeq_id_Handle& idh,
                                         string* srr_acc_ptr,
                                         TVDBRowId* spot_id_ptr,
                                         Uint4* read_id_ptr)
{
    // id is of type gnl|SRA|srr/label
    if ( !GetGeneralIdType(idh, eGeneralIdType_read) ) {
        return false;
    }
    CConstRef<CSeq_id> id = idh.GetSeqId();
    CTempString str = id->GetGeneral().GetTag().GetStr();
    SIZE_TYPE srr_end = ParseReadId(str, spot_id_ptr, read_id_ptr);
    if ( srr_end == NPOS ) {
        return false;
    }
    if ( srr_acc_ptr ) {
        *srr_acc_ptr = str.substr(0, srr_end);
    }
    return true;
}


static const char kBlobPrefixAnnot[] = "annot|";
static const char kBlobPrefixRefSeq[] = "refseq|";
static const char kBlobPrefixReads[] = "reads|";
static const char kBlobPrefixReadsAlign[] = "align|";
static const char kRefIdPrefixGeneral[] = "gnl|";
static const char kRefIdPrefixId[] = "id|";
static const char kFileEnd[] = "|||";

string CCSRABlobId::ToString(void) const
{
    CNcbiOstrstream out;
    switch ( m_BlobType ) {
    case eBlobType_annot:
        out << kBlobPrefixAnnot;
        break;
    case eBlobType_refseq:
        out << kBlobPrefixRefSeq;
        break;
    case eBlobType_reads:
        out << kBlobPrefixReads;
        break;
    case eBlobType_reads_align:
        out << kBlobPrefixReadsAlign;
        break;
    default:
        return "";
    }
    out << (m_RefIdType == CCSraDb::eRefId_gnl_NAME?
            kRefIdPrefixGeneral: kRefIdPrefixId);
    out << m_File;
    out << kFileEnd;
    if ( m_BlobType < eBlobType_reads ) {
        out << m_SeqId;
    }
    else {
        out << m_FirstSpotId;
    }
    return CNcbiOstrstreamToString(out);
}


void CCSRABlobId::FromString(CTempString str0)
{
    CTempString str = str0;
    if ( NStr::StartsWith(kBlobPrefixAnnot, str) ) {
        str = str.substr(strlen(kBlobPrefixAnnot));
        m_BlobType = eBlobType_annot;
    }
    else if ( NStr::StartsWith(kBlobPrefixRefSeq, str) ) {
        str = str.substr(strlen(kBlobPrefixRefSeq));
        m_BlobType = eBlobType_refseq;
    }
    else if ( NStr::StartsWith(kBlobPrefixReads, str) ) {
        str = str.substr(strlen(kBlobPrefixReads));
        m_BlobType = eBlobType_reads;
    }
    else if ( NStr::StartsWith(kBlobPrefixReadsAlign, str) ) {
        str = str.substr(strlen(kBlobPrefixReadsAlign));
        m_BlobType = eBlobType_reads_align;
    }
    else {
        NCBI_THROW_FMT(CSraException, eOtherError,
                       "Bad CCSRABlobId: "<<str0);
    }
    if ( NStr::StartsWith(kRefIdPrefixGeneral, str) ) {
        str = str.substr(strlen(kRefIdPrefixGeneral));
        m_RefIdType = CCSraDb::eRefId_gnl_NAME;
    }
    else if ( NStr::StartsWith(kRefIdPrefixId, str) ) {
        str = str.substr(strlen(kRefIdPrefixId));
        m_RefIdType = CCSraDb::eRefId_SEQ_ID;
    }
    else {
        NCBI_THROW_FMT(CSraException, eOtherError,
                       "Bad CCSRABlobId: "<<str0);
    }
    SIZE_TYPE div = str.rfind(kFileEnd);
    if ( div == NPOS ) {
        NCBI_THROW_FMT(CSraException, eOtherError,
                       "Bad CCSRABlobId: "<<str0);
    }
    m_File = str.substr(0, div);
    str = str.substr(div+strlen(kFileEnd));
    if ( m_BlobType < eBlobType_reads ) {
        m_SeqId = CSeq_id_Handle::GetHandle(str);
        m_FirstSpotId = 0;
    }
    else {
        m_SeqId.Reset();
        m_FirstSpotId = NStr::StringToNumeric<TVDBRowId>(str);
    }
}


bool CCSRABlobId::operator<(const CBlobId& id) const
{
    const CCSRABlobId& csra2 = dynamic_cast<const CCSRABlobId&>(id);
    if ( m_File != csra2.m_File ) {
        return m_File < csra2.m_File;
    }
    if ( m_RefIdType != csra2.m_RefIdType ) {
        return m_RefIdType < csra2.m_RefIdType;
    }
    if ( m_BlobType != csra2.m_BlobType ) {
        return m_BlobType < csra2.m_BlobType;
    }
    if ( m_FirstSpotId != csra2.m_FirstSpotId ) {
        return m_FirstSpotId < csra2.m_FirstSpotId;
    }
    return m_SeqId < csra2.m_SeqId;
}


bool CCSRABlobId::operator==(const CBlobId& id) const
{
    const CCSRABlobId& csra2 = dynamic_cast<const CCSRABlobId&>(id);
    return m_BlobType == csra2.m_BlobType &&
        m_RefIdType == csra2.m_RefIdType && 
        m_FirstSpotId == csra2.m_FirstSpotId &&
        m_SeqId == csra2.m_SeqId &&
        m_File == csra2.m_File;
}


/////////////////////////////////////////////////////////////////////////////
// CCSRADataLoader_Impl
/////////////////////////////////////////////////////////////////////////////


CCSRADataLoader_Impl::CCSRADataLoader_Impl(
    const CCSRADataLoader::SLoaderParams& params)
    : m_SRRFiles(new TSRRFiles(GetGCSize())),
      m_IdMapper(params.m_IdMapper)
{
    m_DirPath = params.m_DirPath;
    m_AnnotName = params.m_AnnotName;
    m_MinMapQuality = params.GetEffectiveMinMapQuality();
    m_PileupGraphs = params.GetEffectivePileupGraphs();
    m_QualityGraphs = params.GetEffectiveQualityGraphs();
    m_SpotReadAlign = params.GetEffectiveSpotReadAlign();
    m_PathInId = params.m_PathInId;
    m_SpotGroups = params.GetEffectiveSpotGroups();
    
    if ( params.m_CSRAFiles.empty() ) {
        if ( !m_DirPath.empty() ) {
            m_DirPath.erase();
            AddCSRAFile(params.m_DirPath);
        }
    }
    ITERATE (vector<string>, it, params.m_CSRAFiles) {
        AddCSRAFile(*it);
    }
}


CCSRADataLoader_Impl::~CCSRADataLoader_Impl(void)
{
}


void CCSRADataLoader_Impl::AddCSRAFile(const string& csra)
{
    m_FixedFiles[csra] =
        new CCSRAFileInfo(*this, csra, CCSraDb::eRefId_SEQ_ID);
}


CCSRADataLoader_Impl::TFileLock CCSRADataLoader_Impl::GetSRRFile(const string& acc)
{
    if ( !m_FixedFiles.empty() ) {
        // no dynamic SRR accessions
        return TFileLock();
    }
    TSRRFiles::CLock lock = m_SRRFiles->get_lock(acc);
    if ( !*lock ) {
        CFastMutexGuard guard(lock.GetValueMutex());
        if ( !*lock ) {
            try {
                *lock = new CCSRAFileInfo(*this, acc, CCSraDb::eRefId_gnl_NAME);
            }
            catch ( CSraException& exc ) {
                if ( exc.GetErrCode() == exc.eNotFoundDb ||
                     exc.GetErrCode() == exc.eProtectedDb ) {
                    // no such SRA table
                    return TFileLock();
                }
                ERR_POST_X(4, "CCSRADataLoader::GetSRRFile("<<acc<<"): accession not found: "<<exc);
                return TFileLock();
            }
        }
    }
    return TFileLock(*lock, lock);
}


CCSRADataLoader_Impl::TRefLock
CCSRADataLoader_Impl::GetRefSeqInfo(const CSeq_id_Handle& idh)
{
    string acc;
    if ( CCSRABlobId::GetGeneralSRAAccLabel(idh, &acc) ) {
        TFileLock file = GetSRRFile(acc);
        if ( !file.first ) {
            return TRefLock();
        }
        return TRefLock(file.first->GetRefSeqInfo(idh), move(file.second));
    }
    TRefLock ret;
    NON_CONST_ITERATE ( TFixedFiles, it, m_FixedFiles ) {
        if ( CRef<CCSRARefSeqInfo> info = it->second->GetRefSeqInfo(idh) ) {
            if ( ret.first ) {
                // conflict
                ERR_POST_X(1, "CCSRADataLoader::GetRefSeqInfo: "
                           "Seq-id "<<idh<<" appears in two files: "
                           <<ret.first->m_File->GetCSRAName()<<" & "
                           <<info->m_File->GetCSRAName());
                continue;
            }
            ret.first = info;
        }
    }
    return ret;
}


CCSRADataLoader_Impl::TFileLock
CCSRADataLoader_Impl::GetReadsFileInfo(const CSeq_id_Handle& idh,
                                       TVDBRowId* spot_id_ptr,
                                       Uint4* read_id_ptr,
                                       CRef<CCSRARefSeqInfo>* ref_ptr,
                                       TSeqPos *ref_pos_ptr)
{
    string acc;
    TVDBRowId spot_id;
    Uint4 read_id;
    if ( ref_ptr ) {
        *ref_ptr = 0;
    }
    if ( !CCSRABlobId::GetGeneralSRAAccReadId(idh, &acc, &spot_id, &read_id) ) {
        return TFileLock();
    }
    TFileLock ret;
    NON_CONST_ITERATE ( TFixedFiles, it, m_FixedFiles ) {
        if ( it->second->m_CSRADb->GetSraIdPart() == acc ) {
            if ( ret.first ) {
                // duplicate id
                ERR_POST_X(2, "CCSRADataLoader::GetReadsFileInfo: "
                           "Seq-id "<<idh<<" appears in two files: "
                           <<it->second->GetCSRAName()<<" & "
                           <<ret.first->GetCSRAName());
                return TFileLock();
            }
            ret.first = it->second;
        }
    }
    if ( !ret.first ) {
        // load by SRR accession
        ret = GetSRRFile(acc);
        if ( !ret.first ) {
            return ret;
        }
    }
    // check if spot_id exists
    if ( !ret.first->IsValidReadId(spot_id, read_id, ref_ptr, ref_pos_ptr) ) {
        return TFileLock();
    }
    if ( spot_id_ptr ) {
        *spot_id_ptr = spot_id;
    }
    if ( read_id_ptr ) {
        *read_id_ptr = read_id;
    }
    return ret;
}


CCSRADataLoader_Impl::TFileLock
CCSRADataLoader_Impl::GetFileInfo(const CCSRABlobId& blob_id)
{
    if ( blob_id.m_RefIdType == CCSraDb::eRefId_SEQ_ID ) {
        TFixedFiles::iterator it = m_FixedFiles.find(blob_id.m_File);
        if ( it == m_FixedFiles.end() ) {
            return TFileLock();
        }
        return TFileLock(it->second, TSRRFiles::CLock());
    }
    TFileLock lock = GetSRRFile(blob_id.m_File);
    _ASSERT(lock.second == blob_id.m_FileLock);
    return lock;
}


CRef<CCSRABlobId> CCSRADataLoader_Impl::GetBlobId(const CSeq_id_Handle& idh)
{
    // return blob-id of blob with sequence
    // annots may be different
    {{
        TRefLock info = GetRefSeqInfo(idh);
        if ( info.first ) {
            return GetBlobId(info, CCSRABlobId::eBlobType_refseq);
        }
    }}
    {{
        TVDBRowId spot_id;
        TFileLock info = GetReadsFileInfo(idh, &spot_id);
        if ( info.first ) {
            return GetReadsBlobId(info, spot_id);
        }
    }}
    return null;
}


CTSE_LoadLock CCSRADataLoader_Impl::GetBlobById(CDataSource* data_source,
                                                const CCSRABlobId& blob_id)
{
    CDataLoader::TBlobId loader_blob_id(&blob_id);
    CTSE_LoadLock load_lock = data_source->GetTSE_LoadLock(loader_blob_id);
    if ( !load_lock.IsLoaded() ) {
        LoadBlob(blob_id, load_lock);
        load_lock.SetLoaded();
    }
    return load_lock;
}


void CCSRADataLoader_Impl::SetSpotReadAlign(bool value)
{
    m_SpotReadAlign = value;
}


CDataLoader::TTSE_LockSet
CCSRADataLoader_Impl::GetRecords(CDataSource* data_source,
                                 const CSeq_id_Handle& idh,
                                 CDataLoader::EChoice choice)
{
    CDataLoader::TTSE_LockSet locks;
    // return blob-id of blob with annotations and possibly with sequence

    bool need_seq = false, need_align = false, need_graph = false, need_orphan = false;
    switch ( choice ) {
    case CDataLoader::eAll:
        need_seq = need_align = need_graph = true;
        break;
    case CDataLoader::eBlob:
    case CDataLoader::eBioseq:
    case CDataLoader::eBioseqCore:
    case CDataLoader::eSequence:
        need_seq = true;
        break;
    case CDataLoader::eAnnot:
    case CDataLoader::eExtAnnot:
        need_align = need_graph = true;
        break;
    case CDataLoader::eGraph:
    case CDataLoader::eExtGraph:
        need_graph = true;
        break;
    case CDataLoader::eFeatures:
    case CDataLoader::eExtFeatures:
        break;
    case CDataLoader::eAlign:
    case CDataLoader::eExtAlign:
        need_align = true;
        break;
    case CDataLoader::eOrphanAnnot:
        need_orphan = true;
        break;
    default:
        break;
    }

    TRefLock ref = GetRefSeqInfo(idh);
    if ( ref.first  ) {
        // refseq: annots and possibly ref sequence
        if ( (!m_FixedFiles.empty() && idh.Which() == CSeq_id::e_Local) ||
             ref.first->m_File->m_RefIdType == CCSraDb::eRefId_gnl_NAME ) {
            // we have refseq+annot
            if ( need_align || need_graph ) {
                CRef<CCSRABlobId> annot_blob_id = GetBlobId(ref, CCSRABlobId::eBlobType_annot);
                locks.insert(GetBlobById(data_source, *annot_blob_id));
            }
            if ( need_seq ) {
                // include refseq blob
                CRef<CCSRABlobId> refseq_blob_id = GetBlobId(ref, CCSRABlobId::eBlobType_refseq);
                locks.insert(GetBlobById(data_source, *refseq_blob_id));
            }
        }
        else {
            // we have orphan annot only
            if ( need_orphan ) {
                CRef<CCSRABlobId> annot_blob_id = GetBlobId(ref, CCSRABlobId::eBlobType_annot);
                locks.insert(GetBlobById(data_source, *annot_blob_id));
            }
        }
        return locks;
    }

    // otherwise it might be request by short read id
    if ( need_orphan ) {
        // no orphan annots in this case
        return locks;
    }

    TVDBRowId spot_id;
    CRef<CCSRARefSeqInfo> ref_info;
    TSeqPos ref_pos;
    TFileLock file;
    if ( need_align && !GetSpotReadAlign() ) {
        // look for read alignments on reference sequence
        file = GetReadsFileInfo(idh, &spot_id, 0, &ref_info, &ref_pos);
    }
    else {
        // no reference sequence lookup
        file = GetReadsFileInfo(idh, &spot_id);
    }
    if ( file.first ) {
        // short read: we have sequence blob and alignment on refseq
        if ( need_seq || need_graph || (need_align && !ref_info) ) {
            if ( CRef<CCSRABlobId> blob_id = GetReadsBlobId(file, spot_id) ) {
                // we need short read blob
                _ASSERT(blob_id->m_BlobType == CCSRABlobId::eBlobType_reads);
                if ( need_align && !ref_info ) {
                    // aligns are needed too
                    // add separate align blob
                    CRef<CCSRABlobId> align_blob_id(new CCSRABlobId(*blob_id));
                    align_blob_id->m_BlobType = CCSRABlobId::eBlobType_reads_align;
                    locks.insert(GetBlobById(data_source, *align_blob_id));
                }
                locks.insert(GetBlobById(data_source, *blob_id));
            }
        }
        if ( need_align && ref_info ) {
            if ( CRef<CCSRABlobId> blob_id = GetBlobId(TRefLock(ref_info, move(file.second)), CCSRABlobId::eBlobType_annot) ) {
                CDataLoader::TTSE_Lock tse_lock = GetBlobById(data_source, *blob_id);
                tse_lock->x_LoadChunk(kMainChunkId);
                int chunk_id = ref_info->GetAnnotChunkId(ref_pos);
                if ( chunk_id >= 0 ) {
                    tse_lock->x_LoadChunk(chunk_id);
                }
                locks.insert(tse_lock);
            }
        }
    }
    return locks;
}


void CCSRADataLoader_Impl::LoadBlob(const CCSRABlobId& blob_id,
                                    CTSE_LoadLock& load_lock)
{
    TFileLock file_info = GetFileInfo(blob_id);
    switch ( blob_id.m_BlobType ) {
    case CCSRABlobId::eBlobType_annot:
        file_info.first->GetRefSeqInfo(blob_id)->LoadAnnotBlob(load_lock);
        break;
    case CCSRABlobId::eBlobType_refseq:
        file_info.first->GetRefSeqInfo(blob_id)->LoadRefSeqBlob(load_lock);
        break;
    default:
        file_info.first->LoadReadsBlob(blob_id, load_lock);
        break;
    }
}


void CCSRADataLoader_Impl::LoadChunk(const CCSRABlobId& blob_id,
                                    CTSE_Chunk_Info& chunk_info)
{
    _TRACE("Loading chunk "<<blob_id.ToString()<<"."<<chunk_info.GetChunkId());
    TFileLock file_info = GetFileInfo(blob_id);
    switch ( blob_id.m_BlobType ) {
    case CCSRABlobId::eBlobType_annot:
        file_info.first->GetRefSeqInfo(blob_id)->LoadAnnotChunk(chunk_info);
        break;
    case CCSRABlobId::eBlobType_refseq:
        file_info.first->GetRefSeqInfo(blob_id)->LoadRefSeqChunk(chunk_info);
        break;
    default:
        break;
    }
}


CCSRADataLoader::TAnnotNames
CCSRADataLoader_Impl::GetPossibleAnnotNames(void) const
{
    TAnnotNames names;
    ITERATE ( TFixedFiles, it, m_FixedFiles ) {
        it->second->GetPossibleAnnotNames(names);
    }
    sort(names.begin(), names.end());
    names.erase(unique(names.begin(), names.end()), names.end());
    return names;
}


CCSraRefSeqIterator
CCSRADataLoader_Impl::GetRefSeqIterator(const CSeq_id_Handle& idh)
{
    TRefLock info = GetRefSeqInfo(idh);
    if ( info.first  ) {
        return info.first->GetRefSeqIterator();
    }
    return CCSraRefSeqIterator();
}


CCSraShortReadIterator
CCSRADataLoader_Impl::GetShortReadIterator(const CSeq_id_Handle& idh)
{
    TVDBRowId spot_id;
    Uint4 read_id;
    TFileLock info = GetReadsFileInfo(idh, &spot_id, &read_id);
    if ( info.first ) {
        return CCSraShortReadIterator(info.first->m_CSRADb, spot_id, read_id);
    }
    return CCSraShortReadIterator();
}


void CCSRADataLoader_Impl::GetIds(const CSeq_id_Handle& idh, TIds& ids)
{
    if ( CCSraRefSeqIterator iter = GetRefSeqIterator(idh) ) {
        ITERATE ( CBioseq::TId, it, iter.GetRefSeq_ids() ) {
            ids.push_back(CSeq_id_Handle::GetHandle(**it));
        }
    }
    else if ( GetReadsFileInfo(idh).first ) {
        ids.push_back(idh);
    }
}


CDataSource::SAccVerFound
CCSRADataLoader_Impl::GetAccVer(const CSeq_id_Handle& idh)
{
    CDataSource::SAccVerFound ret;
    // the only possible acc.ver is for reference sequence
    if ( CCSraRefSeqIterator iter = GetRefSeqIterator(idh) ) {
        ret.sequence_found = true;
        ITERATE ( CBioseq::TId, it, iter.GetRefSeq_ids() ) {
            if ( (*it)->GetTextseq_Id() ) {
                ret.acc_ver = CSeq_id_Handle::GetHandle(**it);
                break;
            }
        }
    }
    else if ( GetReadsFileInfo(idh).first ) {
        ret.sequence_found = true;
    }
    return ret;
}


CDataSource::SGiFound
CCSRADataLoader_Impl::GetGi(const CSeq_id_Handle& idh)
{
    CDataSource::SGiFound ret;
    // the only possible gi is for reference sequence
    if ( CCSraRefSeqIterator iter = GetRefSeqIterator(idh) ) {
        ret.sequence_found = true;
        ITERATE ( CBioseq::TId, it, iter.GetRefSeq_ids() ) {
            if ( (*it)->IsGi() ) {
                ret.gi = (*it)->GetGi();
                break;
            }
        }
    }
    else if ( GetReadsFileInfo(idh).first ) {
        ret.sequence_found = true;
    }
    return ret;
}


string CCSRADataLoader_Impl::GetLabel(const CSeq_id_Handle& idh)
{
    if ( GetBlobId(idh) ) {
        return objects::GetLabel(idh); // default label from Seq-id
    }
    return string(); // sequence is unknown
}


int CCSRADataLoader_Impl::GetTaxId(const CSeq_id_Handle& idh)
{
    if ( GetBlobId(idh) ) {
        return 0; // taxid is not defined
    }
    return -1; // sequence is unknown
}


TSeqPos CCSRADataLoader_Impl::GetSequenceLength(const CSeq_id_Handle& idh)
{
    // the only possible acc.ver is for reference sequence
    if ( CCSraRefSeqIterator iter = GetRefSeqIterator(idh) ) {
        return iter.GetSeqLength();
    }
    if ( CCSraShortReadIterator iter = GetShortReadIterator(idh) ) {
        return iter.GetShortLen();
    }
    return kInvalidSeqPos;
}


CDataSource::STypeFound
CCSRADataLoader_Impl::GetSequenceType(const CSeq_id_Handle& idh)
{
    CDataSource::STypeFound ret;
    if ( GetBlobId(idh) ) {
        ret.sequence_found = true;
        ret.type = CSeq_inst::eMol_na;
    }
    return ret;
}


/////////////////////////////////////////////////////////////////////////////
// CCSRAFileInfo
/////////////////////////////////////////////////////////////////////////////


CCSRAFileInfo::CCSRAFileInfo(CCSRADataLoader_Impl& impl,
                             const string& csra,
                             CCSraDb::ERefIdType ref_id_type)
{
    x_Initialize(impl, csra, ref_id_type);
    for ( CCSraRefSeqIterator rit(m_CSRADb); rit; ++rit ) {
        CSeq_id_Handle seq_id = rit.GetRefSeq_id_Handle();
        string refseq_label = seq_id.AsString();
        AddRefSeq(refseq_label, seq_id);
    }
}


void CCSRAFileInfo::x_Initialize(CCSRADataLoader_Impl& impl,
                                 const string& csra,
                                 CCSraDb::ERefIdType ref_id_type)
{
    m_CSRAName = csra;
    m_RefIdType = ref_id_type;
    m_AnnotName = impl.m_AnnotName;
    if ( m_AnnotName.empty() ) {
        m_AnnotName = m_CSRAName;
    }
    m_MinMapQuality = impl.GetMinMapQuality();
    m_PileupGraphs = impl.GetPileupGraphs();
    m_QualityGraphs = impl.GetQualityGraphs();
    CCSraDb::EPathInIdType path_in_id_type;
    switch ( impl.GetPathInId() ) {
    case CCSRADataLoader::SLoaderParams::kPathInId_config:
        path_in_id_type = CCSraDb::ePathInId_config;
        break;
    case 0:
        path_in_id_type = CCSraDb::ePathInId_no;
        break;
    default:
        path_in_id_type = CCSraDb::ePathInId_yes;
    }
    m_CSRADb = CCSraDb(impl.m_Mgr,
                       CDirEntry::MakePath(impl.m_DirPath, csra),
                       CCSraDb::MakeSraIdPart(path_in_id_type,
                                              impl.m_DirPath, csra),
                       impl.m_IdMapper.get(),
                       ref_id_type);
    if ( GetDebugLevel() >= 1 ) {
        LOG_POST_X(8, Info <<
                   "CCSRADataLoader("<<csra<<")="<<m_CSRADb->GetSraIdPart());
    }
    int max_separate_spot_groups = impl.GetSpotGroups();
    if ( max_separate_spot_groups > 1 ) {
        m_CSRADb.GetSpotGroups(m_SeparateSpotGroups);
        if ( m_SeparateSpotGroups.size() > size_t(max_separate_spot_groups) ) {
            m_SeparateSpotGroups.clear();
        }
    }
}


string CCSRAFileInfo::GetAnnotName(const string& spot_group,
                                   ECSRAAnnotChunkIdType type) const
{
    string name = GetBaseAnnotName();
    if ( !m_SeparateSpotGroups.empty() ) {
        name += SPOT_GROUP_SEPARATOR;
        name += spot_group;
    }
    if ( type == eCSRAAnnotChunk_pileup_graph ) {
        if ( !name.empty() ) {
            name += ' ';
        }
        name += PILEUP_NAME_SUFFIX;
    }
    return name;
}


string CCSRAFileInfo::GetAlignAnnotName(void) const
{
    return GetAnnotName(kEmptyStr, eCSRAAnnotChunk_align);
}


string CCSRAFileInfo::GetAlignAnnotName(const string& spot_group) const
{
    return GetAnnotName(spot_group, eCSRAAnnotChunk_align);
}


string CCSRAFileInfo::GetPileupAnnotName(void) const
{
    return GetAnnotName(kEmptyStr, eCSRAAnnotChunk_pileup_graph);
}


string CCSRAFileInfo::GetPileupAnnotName(const string& spot_group) const
{
    return GetAnnotName(spot_group, eCSRAAnnotChunk_pileup_graph);
}


void CCSRAFileInfo::GetPossibleAnnotNames(TAnnotNames& names) const
{
    if ( GetSeparateSpotGroups().empty() ) {
        string align_annot_name = GetAlignAnnotName();
        if ( align_annot_name.empty() ) {
            names.push_back(CAnnotName());
        }
        else {
            names.push_back(CAnnotName(align_annot_name));
        }
        names.push_back(CAnnotName(GetPileupAnnotName()));
    }
    else {
        ITERATE ( vector<string>, it, GetSeparateSpotGroups() ) {
            names.push_back(CAnnotName(GetAlignAnnotName(*it)));
            names.push_back(CAnnotName(GetPileupAnnotName(*it)));
        }
    }
}


void CCSRAFileInfo::AddRefSeq(const string& refseq_label,
                              const CSeq_id_Handle& refseq_id)
{
    if ( GetDebugLevel() >= 1 ) {
        LOG_POST_X(9, Info << "CCSRADataLoader(" << m_CSRAName << "): "
                   "Found "<<refseq_label<<" -> "<<refseq_id);
    }
    m_RefSeqs[refseq_id] = new CCSRARefSeqInfo(this, refseq_id);
}


CRef<CCSRARefSeqInfo>
CCSRAFileInfo::GetRefSeqInfo(const CSeq_id_Handle& seq_id)
{
    TRefSeqs::const_iterator it = m_RefSeqs.find(seq_id);
    if ( it != m_RefSeqs.end() ) {
        return it->second;
    }
    string srr, label;
    if ( CCSRABlobId::GetGeneralSRAAccLabel(seq_id, &srr, &label) &&
         srr == GetCSRAName() ) {
        AddRefSeq(label, seq_id);
        it = m_RefSeqs.find(seq_id);
        if ( it != m_RefSeqs.end() ) {
            return it->second;
        }
    }
    return null;
}


bool CCSRAFileInfo::IsValidReadId(TVDBRowId spot_id, Uint4 read_id,
                                  CRef<CCSRARefSeqInfo>* ref_ptr,
                                  TSeqPos* ref_pos_ptr)
{
    CCSraShortReadIterator read_it(m_CSRADb, spot_id, read_id);
    if ( ref_ptr ) {
        *ref_ptr = 0;
    }
    if ( ref_pos_ptr ) {
        *ref_pos_ptr = kInvalidSeqPos;
    }
    if ( !read_it ) {
        return false;
    }
    if ( ref_ptr || ref_pos_ptr ) {
        CCSraRefSeqIterator ref_seq_it = read_it.GetRefSeqIter(ref_pos_ptr);
        if ( ref_seq_it ) {
            if ( ref_ptr ) {
                *ref_ptr = GetRefSeqInfo(ref_seq_it.GetRefSeq_id_Handle());
            }
        }
    }
    return true;
}


CRef<CCSRABlobId> CCSRADataLoader_Impl::GetReadsBlobId(const TFileLock& lock,
                                                       TVDBRowId spot_id)
{
    TVDBRowId first_spot_id = (spot_id-1)/kReadsPerBlob*kReadsPerBlob+1;
    return Ref(new CCSRABlobId(lock, first_spot_id));
}


void CCSRAFileInfo::LoadReadsBlob(const CCSRABlobId& blob_id,
                                  CTSE_LoadLock& load_lock)
{
    CRef<CSeq_entry> entry(new CSeq_entry);
    entry->SetSet().SetSeq_set();
    TVDBRowId first_spot_id = blob_id.m_FirstSpotId;
    TVDBRowId last_spot_id = first_spot_id + kReadsPerBlob - 1;
    if ( GetDebugLevel() >= 5 ) {
        LOG_POST_X(12, Info<<
                   "CCSRADataLoader:LoadReads("<<blob_id.ToString()<<", "<<
                   first_spot_id<<"-"<<last_spot_id);
    }
    if ( blob_id.m_BlobType == CCSRABlobId::eBlobType_reads ) {
        // add reads
        CCSraShortReadIterator::TBioseqFlags flags = CCSraShortReadIterator::fDefaultBioseqFlags;
        if ( m_QualityGraphs ) {
            flags |= CCSraShortReadIterator::fQualityGraph;
        }
        CCSraShortReadIterator it(*this, first_spot_id);
        it.SetLastSpotId(last_spot_id);
        for ( ; it; ++it ) {
            CRef<CSeq_entry> e(new CSeq_entry);
            e->SetSeq(*it.GetShortBioseq(flags));
            entry->SetSet().SetSeq_set().push_back(e);
        }
    }
    if ( blob_id.m_BlobType == CCSRABlobId::eBlobType_reads_align ) {
        // add primary alignments
        CRef<CSeq_annot> annot;
        CCSraShortReadIterator it(*this, first_spot_id);
        it.SetLastSpotId(last_spot_id);
        for ( ; it; ++it ) {
            if ( CCSraAlignIterator ait = it.GetAlignIter() ) {
                if ( !annot ) {
                    annot = new CSeq_annot;
                    annot->SetNameDesc(GetAlignAnnotName());
                    entry->SetSet().SetAnnot().push_back(annot);
                }
                annot->SetData().SetAlign().push_back(ait.GetMatchAlign());
            }
        }
    }

    load_lock->SetSeq_entry(*entry);
}


/////////////////////////////////////////////////////////////////////////////
// CCSRARefSeqInfo
/////////////////////////////////////////////////////////////////////////////


CCSRARefSeqInfo::CCSRARefSeqInfo(CCSRAFileInfo* csra_file,
                                 const CSeq_id_Handle& seq_id)
    : m_File(csra_file),
      m_RefSeqId(seq_id),
      m_MinMapQuality(csra_file->GetMinMapQuality())
{
}


CRef<CCSRABlobId> CCSRADataLoader_Impl::GetBlobId(const TRefLock& ref,
                                                  CCSRABlobId::EBlobType type)
{
    return Ref(new CCSRABlobId(type, ref));
}


namespace {
    struct SRefStat {
        SRefStat(void)
            : m_Count(0),
              m_RefPosFirst(0),
              m_RefPosLast(0)
            {
            }

        unsigned m_Count;
        TSeqPos m_RefPosFirst;
        TSeqPos m_RefPosLast;

        void Collect(CCSraDb& csra_db, const CSeq_id_Handle& ref_id,
                     TSeqPos ref_pos, unsigned count, int min_quality);

        unsigned GetStatCount(void) const {
            return m_Count;
        }
        double GetStatLen(void) const {
            return m_RefPosLast - m_RefPosFirst + .5;
        }
    };
    
    
    void SRefStat::Collect(CCSraDb& csra_db, const CSeq_id_Handle& ref_id,
                           TSeqPos ref_pos, unsigned count, int min_quality)
    {
        m_RefPosFirst = kInvalidSeqPos;
        m_RefPosLast = 0;
        size_t skipped = 0;
        CCSraAlignIterator ait(csra_db, ref_id, ref_pos);
        for ( ; ait; ++ait ) {
            if ( min_quality > 0 && ait.GetMapQuality() < min_quality ) {
                ++skipped;
                continue;
            }
            TSeqPos pos = ait.GetRefSeqPos();
            _ASSERT(pos >= ref_pos); // filtering is done by CCSraAlignIterator
            if ( pos < m_RefPosFirst ) {
                m_RefPosFirst = pos;
            }
            if ( pos > m_RefPosLast ) {
                m_RefPosLast = pos;
            }
            if ( ++m_Count == count ) {
                break;
            }
        }
        if ( GetDebugLevel() >= 2 ) {
            LOG_POST_X(4, Info << "CCSRADataLoader: "
                       "Stat @ "<<ref_pos<<": "<<m_Count<<" entries: "<<
                       m_RefPosFirst<<"-"<<m_RefPosLast<<
                       " skipped: "<<skipped);
        }
    }
};


void CCSRARefSeqInfo::LoadRanges(void)
{
    if ( !m_AlignChunks.empty() ) {
        return;
    }
    if ( !m_AlignChunks.empty() ) {
        return;
    }

    _TRACE("Loading "<<GetRefSeqId());

    m_CovAnnot = CCSraRefSeqIterator(*m_File, GetRefSeqId())
        .GetCoverageAnnot(m_File->GetAlignAnnotName());
    x_LoadRangesStat();

    _TRACE("Loaded ranges on "<<GetRefSeqId());
    _ASSERT(!m_AlignChunks.empty());
    if ( GetDebugLevel() >= 2 ) {
        CRange<TSeqPos> range;
        for ( size_t k = 0; k+1 < m_AlignChunks.size(); ++k ) {
            if ( !m_AlignChunks[k].align_count ) {
                continue;
            }
            range.SetFrom(m_AlignChunks[k].start_pos);
            range.SetToOpen(m_AlignChunks[k+1].start_pos);
            LOG_POST_X(6, Info << "CCSRADataLoader: "
                       "Align Chunk "<<k<<": "<<range<<
                       " with "<<m_AlignChunks[k].align_count<<" aligns");
        }
        for ( size_t k = 0; k+1 < m_GraphChunks.size(); ++k ) {
            if ( !m_GraphChunks[k].align_count ) {
                continue;
            }
            range.SetFrom(m_GraphChunks[k].start_pos);
            range.SetToOpen(m_GraphChunks[k+1].start_pos);
            LOG_POST_X(6, Info << "CCSRADataLoader: "
                       "Graph Chunk "<<k<<": "<<range<<
                       " with "<<m_GraphChunks[k].align_count<<" aligns");
        }
    }
}


CCSraRefSeqIterator
CCSRARefSeqInfo::GetRefSeqIterator(void) const
{
    return CCSraRefSeqIterator(*m_File, GetRefSeqId());
}


void CCSRARefSeqInfo::x_LoadRangesStat(void)
{
    if ( kUseFullAlignCounts ) {
        CStopWatch sw;
        if ( GetDebugLevel() >= 1 ) {
            sw.Start();
        }
        TSeqPos segment_len = m_File->GetDb().GetRowSize();
        const size_t kAlignLimitCount = kAlignChunkSize;
        const size_t kGraphLimitCount = kGraphChunkSize;
        const TSeqPos kAlignEmptyLength = kAlignEmptyPages*segment_len;
        const TSeqPos kGraphEmptyLength = kGraphEmptyPages*segment_len;
        const TSeqPos kGraphLimitLength = segment_len*kChunkGraphMul;
        CCSraRefSeqIterator iter(*m_File, GetRefSeqId());
        TSeqPos ref_length = iter.GetSeqLength();
        uint64_t total = 0;

        SChunkInfo a = { 0, 0 }, g = { 0, 0 };
        TSeqPos a_empty = 0, g_empty = 0;
        for ( TSeqPos p = 0; p < ref_length; p += segment_len ) {
            TSeqPos end = min(ref_length, p + segment_len);
            unsigned c = unsigned(iter.GetAlignCountAtPos(p));
            total += c;

            // align chunk
            // If this page itself is too big
            // or previous empty range is too big
            // add previous range as a separate chunk
            if ( (p > a.start_pos && a.align_count+c >= 2*kAlignLimitCount) ||
                 (p >= a.start_pos+kAlignEmptyLength && c && !a.align_count) ) {
                m_AlignChunks.push_back(a);
                a.start_pos = p;
                a.align_count = 0;
            }
            if ( c ) {
                a.align_count += c;
                a_empty = end;
            }
            else if ( a.align_count && end >= a_empty+kAlignEmptyLength ) {
                // too large empty region started
                m_AlignChunks.push_back(a);
                a.start_pos = a_empty;
                a.align_count = 0;
            }
            if ( a.align_count >= kAlignLimitCount ) {
                // collected data is big enough -> add it as a chunk
                m_AlignChunks.push_back(a);
                a.start_pos = end;
                a.align_count = 0;
            }

            // graph chunk
            // If this page itself is too big
            // or previous empty range is too big
            // add previous range as a separate chunk
            if ( (p > g.start_pos && g.align_count+c >= 2*kGraphLimitCount) ||
                 (p >= g.start_pos+kGraphEmptyLength && c && !g.align_count) ) {
                m_GraphChunks.push_back(g);
                g.start_pos = p;
                g.align_count = 0;
            }
            if ( c ) {
                g.align_count += c;
                g_empty = end;
            }
            else if ( g.align_count && end >= g_empty+kGraphEmptyLength ) {
                // too large empty region started
                m_GraphChunks.push_back(g);
                g.start_pos = g_empty;
                g.align_count = 0;
            }
            if ( g.align_count >= kGraphLimitCount ||
                 (g.align_count &&
                  (p + segment_len - g.start_pos) >= kGraphLimitLength) ) {
                // collected data is big enough -> add it as a chunk
                m_GraphChunks.push_back(g);
                g.start_pos = end;
                g.align_count = 0;
            }
        }

        // finalize annot chunks
        if ( a.align_count ) {
            _ASSERT(a.start_pos < ref_length);
            m_AlignChunks.push_back(a);
            a.start_pos = ref_length;
            a.align_count = 0;
        }
        _ASSERT(a.start_pos <= ref_length);
        _ASSERT(!a.align_count);
        m_AlignChunks.push_back(a);

        // finalize graph chunks
        if ( g.align_count ) {
            _ASSERT(g.start_pos < ref_length);
            m_GraphChunks.push_back(g);
            g.start_pos = ref_length;
            g.align_count = 0;
        }
        _ASSERT(g.start_pos <= ref_length);
        _ASSERT(!g.align_count);
        m_GraphChunks.push_back(g);

        if ( GetDebugLevel() >= 1 ) {
            size_t align_chunks = 0;
            ITERATE ( TChunks, it, m_AlignChunks ) {
                if ( it->align_count ) {
                    ++align_chunks;
                }
            }
            size_t graph_chunks = 0;
            ITERATE ( TChunks, it, m_GraphChunks ) {
                if ( it->align_count ) {
                    ++graph_chunks;
                }
            }
            LOG_POST_X(5, Info << "CCSRADataLoader:"
                       " align count: "<<total<<
                       " align chunks: "<<align_chunks<<
                       " graph chunks: "<<graph_chunks<<
                       " time: "<<sw.Elapsed());
        }
        return;
    }
    vector<TSeqPos> pp;
    if ( kEstimateAlignCounts ) {
        TSeqPos segment_len = m_File->GetDb().GetRowSize();
        CCSraRefSeqIterator iter(*m_File, GetRefSeqId());
        TSeqPos ref_length = iter.GetSeqLength();
        Uint8 est_count = iter.GetEstimatedNumberOfAlignments();
        if ( est_count <= 2*kAlignChunkSize ) {
            pp.push_back(0);
        }
        else {
            TSeqPos chunk_len = TSeqPos((double(ref_length)*kAlignChunkSize/est_count/segment_len+.5))*segment_len;
            chunk_len = max(chunk_len, segment_len);
            for ( TSeqPos pos = 0; pos < ref_length; pos += chunk_len ) {
                pp.push_back(pos);
            }
        }
        if ( GetDebugLevel() >= 1 ) {
            LOG_POST_X(5, Info << "CCSRADataLoader: "
                       " exp count: "<<est_count<<" chunks: "<<pp.size());
        }
        pp.push_back(ref_length);
    }
    else {
        const unsigned kNumStat = 10;
        const unsigned kStatCount = 1000;
        vector<SRefStat> stat(kNumStat);
        TSeqPos ref_length =
            CCSraRefSeqIterator(*m_File, GetRefSeqId()).GetSeqLength();
        TSeqPos ref_begin = 0, ref_end = ref_length;
        double stat_len = 0, stat_cnt = 0;
        const unsigned scan_first = 1;
        if ( scan_first ) {
            stat[0].Collect(*m_File, GetRefSeqId(), 0,
                            kStatCount, m_MinMapQuality);
            if ( stat[0].m_Count != kStatCount ) {
                // single chunk
                SChunkInfo c;
                if ( stat[0].m_Count > 0 ) {
                    c.start_pos = stat[0].m_RefPosFirst;
                    c.align_count = 1;
                    m_AlignChunks.push_back(c);
                    c.start_pos = stat[0].m_RefPosLast+1;
                }
                else {
                    c.start_pos = 0;
                }
                c.align_count = 0;
                m_AlignChunks.push_back(c);
                m_GraphChunks = m_AlignChunks;
                return;
            }
            ref_begin = stat[0].m_RefPosFirst;
            stat_len = stat[0].GetStatLen();
            stat_cnt = stat[0].GetStatCount();
        }
        for ( unsigned k = scan_first; k < kNumStat; ++k ) {
            TSeqPos ref_pos = ref_begin +
                TSeqPos(double(ref_end - ref_begin)*k/kNumStat);
            if ( k && ref_pos < stat[k-1].m_RefPosLast ) {
                ref_pos = stat[k-1].m_RefPosLast;
            }
            _TRACE("stat["<<k<<"] @ "<<ref_pos);
            stat[k].Collect(*m_File, GetRefSeqId(), ref_pos,
                            kStatCount, m_MinMapQuality);
            stat_len += stat[k].GetStatLen();
            stat_cnt += stat[k].GetStatCount();
        }
        double density = stat_cnt / stat_len;
        double exp_count = (ref_end-ref_begin)*density;
        unsigned chunks = unsigned(exp_count/kAlignChunkSize+1);
        chunks = min(chunks, unsigned(sqrt(exp_count)+1));
        if ( GetDebugLevel() >= 1 ) {
            LOG_POST_X(5, Info << "CCSRADataLoader: "
                       "Total range: "<<ref_begin<<"-"<<ref_end-1<<
                       " exp count: "<<exp_count<<" chunks: "<<chunks);
        }
        pp.resize(chunks+1);
        for ( unsigned k = 1; k < chunks; ++k ) {
            TSeqPos pos = ref_begin +
                TSeqPos(double(ref_end-ref_begin)*k/chunks);
            pp[k] = pos;
        }
        pp[chunks] = ref_end;
    }
    SChunkInfo c = { 0, 1 };
    ITERATE ( vector<TSeqPos>, it, pp ) {
        c.start_pos = *it;
        m_AlignChunks.push_back(c);
    }
    m_AlignChunks.back().align_count = 0;
    m_GraphChunks = m_AlignChunks;
}


int CCSRARefSeqInfo::GetAnnotChunkId(TSeqPos ref_pos) const
{
    TChunks::const_iterator it =
        upper_bound(m_AlignChunks.begin(), m_AlignChunks.end(),
                    ref_pos, SChunkInfo());
    if ( it == m_AlignChunks.begin() || it == m_AlignChunks.end() ) {
        return -1;
    }
    int k = int(it-m_AlignChunks.begin()-1);
    return k*eCSRAAnnotChunk_mul + eCSRAAnnotChunk_align;
}


void CCSRARefSeqInfo::LoadAnnotMainSplit(CTSE_LoadLock& load_lock)
{
    CRef<CSeq_entry> entry(new CSeq_entry);
    entry->SetSet().SetId().SetId(1);

    load_lock->SetSeq_entry(*entry);
    CTSE_Split_Info& split_info = load_lock->GetSplitInfo();

    bool has_pileup = m_File->GetPileupGraphs();
    bool separate_spot_groups = !m_File->GetSeparateSpotGroups().empty();
    string align_name, pileup_name;
    if ( !separate_spot_groups ) {
        align_name = m_File->GetAlignAnnotName();
        if ( has_pileup ) {
            pileup_name = m_File->GetPileupAnnotName();
        }
    }

    CRef<CTSE_Chunk_Info> chunk(new CTSE_Chunk_Info(kMainChunkId));
    CRange<TSeqPos> whole_range = CRange<TSeqPos>::GetWhole();

    if ( separate_spot_groups ) {
        ITERATE ( vector<string>, it, m_File->GetSeparateSpotGroups() ) {
            string align_name = m_File->GetAlignAnnotName(*it);
            chunk->x_AddAnnotType(align_name,
                                  SAnnotTypeSelector(CSeq_annot::C_Data::e_Graph),
                                  GetRefSeqId(),
                                  whole_range);
            chunk->x_AddAnnotType(align_name,
                                  SAnnotTypeSelector(CSeq_annot::C_Data::e_Align),
                                  GetRefSeqId(),
                                  whole_range);
            if ( has_pileup ) {
                string align_name = m_File->GetPileupAnnotName(*it);
                chunk->x_AddAnnotType(pileup_name,
                                      SAnnotTypeSelector(CSeq_annot::C_Data::e_Graph),
                                      GetRefSeqId(),
                                      whole_range);
            }
        }
    }
    else {
        chunk->x_AddAnnotType(align_name,
                              SAnnotTypeSelector(CSeq_annot::C_Data::e_Graph),
                              GetRefSeqId(),
                              whole_range);
        chunk->x_AddAnnotType(align_name,
                              SAnnotTypeSelector(CSeq_annot::C_Data::e_Align),
                              GetRefSeqId(),
                              whole_range);
        if ( has_pileup ) {
            chunk->x_AddAnnotType(pileup_name,
                                  SAnnotTypeSelector(CSeq_annot::C_Data::e_Graph),
                                  GetRefSeqId(),
                                  whole_range);
        }
    }
    split_info.AddChunk(*chunk);
}


static const Uint8 k_align_bytes = 300;
static const double k_read_byte_seconds = 7.5e-9; // 133 MB/s
static const double k_make_graph_seconds = 20e-9; // 50 MB/s
static const double k_make_align_seconds = 80e-9; // 12 MB/s
//static const double k_make_read_seconds = 80e-9; // 12 MB/s


void CCSRARefSeqInfo::LoadAnnotMainChunk(CTSE_Chunk_Info& chunk_info)
{
    if ( GetDebugLevel() >= 5 ) {
        LOG_POST_X(13, Info<<
                   "CCSRADataLoader:LoadAnnotMain("<<
                   chunk_info.GetBlobId().ToString()<<", "<<
                   chunk_info.GetChunkId());
    }
    LoadRanges(); // also loads m_CovAnnot
    CTSE_Split_Info& split_info =
        const_cast<CTSE_Split_Info&>(chunk_info.GetSplitInfo());
    string align_name;
    bool separate_spot_groups = !m_File->GetSeparateSpotGroups().empty();
    if ( !separate_spot_groups ) {
        align_name = m_File->GetAlignAnnotName();
    }
    CTSE_Chunk_Info::TPlace place(CSeq_id_Handle(), kTSEId);

    // whole coverage graph
    if ( separate_spot_groups ) {
        // duplucate coverage graph for all spot groups
        ITERATE ( vector<string>, it, m_File->GetSeparateSpotGroups() ) {
            string align_name = m_File->GetAlignAnnotName(*it);
            CRef<CSeq_annot> annot(new CSeq_annot);
            annot->SetData().SetGraph() = m_CovAnnot->GetData().GetGraph();
            CRef<CAnnotdesc> desc(new CAnnotdesc);
            desc->SetName(align_name);
            annot->SetDesc().Set().push_back(desc);
            chunk_info.x_LoadAnnot(place, *annot);
        }
    }
    else {
        chunk_info.x_LoadAnnot(place, *m_CovAnnot);
    }
    
    CCSraRefSeqIterator iter(*m_File, GetRefSeqId());
    CRange<TSeqPos> range;
    {{
        // aligns
        const TChunks& chunks = m_AlignChunks;
        // create chunk info for alignments
        for ( size_t k = 0; k+1 < chunks.size(); ++k ) {
            if ( !chunks[k].align_count ) {
                continue;
            }
            int id = int(k)*eCSRAAnnotChunk_mul + eCSRAAnnotChunk_align;
            CRef<CTSE_Chunk_Info> chunk(new CTSE_Chunk_Info(id));
            range.SetFrom(chunks[k].start_pos);
            range.SetToOpen(chunks[k+1].start_pos);
            range.SetToOpen(iter.GetAlnOverToOpen(range));
            if ( separate_spot_groups ) {
                ITERATE (vector<string>, it, m_File->GetSeparateSpotGroups()) {
                    chunk->x_AddAnnotType
                        (CAnnotName(m_File->GetAlignAnnotName(*it)),
                         SAnnotTypeSelector(CSeq_annot::C_Data::e_Align),
                         GetRefSeqId(),
                         range);
                }
            }
            else {
                chunk->x_AddAnnotType
                    (align_name,
                     SAnnotTypeSelector(CSeq_annot::C_Data::e_Align),
                     GetRefSeqId(),
                     range);
            }
            chunk->x_AddAnnotPlace(kTSEId);
            Uint8 bytes = chunks[k].align_count*k_align_bytes;
            double seconds = bytes*(k_read_byte_seconds+k_make_align_seconds);
            chunk->x_SetLoadBytes(bytes);
            chunk->x_SetLoadSeconds(seconds);
            split_info.AddChunk(*chunk);
        }
    }}
    if ( m_File->GetPileupGraphs() ) {
        string pileup_name = m_File->GetPileupAnnotName();
        
        // pileup
        const TChunks& chunks = m_GraphChunks;
        // create chunk info for alignments
        for ( size_t k = 0; k+1 < chunks.size(); ++k ) {
            int id = int(k)*eCSRAAnnotChunk_mul + eCSRAAnnotChunk_pileup_graph;
            CRef<CTSE_Chunk_Info> chunk(new CTSE_Chunk_Info(id));
            range.SetFrom(chunks[k].start_pos);
            range.SetToOpen(chunks[k+1].start_pos);
            if ( separate_spot_groups ) {
                ITERATE (vector<string>, it, m_File->GetSeparateSpotGroups()) {
                    chunk->x_AddAnnotType
                        (CAnnotName(m_File->GetPileupAnnotName(*it)),
                         SAnnotTypeSelector(CSeq_annot::C_Data::e_Graph),
                         GetRefSeqId(),
                         range);
                }
            }
            else {
                chunk->x_AddAnnotType
                    (pileup_name,
                     SAnnotTypeSelector(CSeq_annot::C_Data::e_Graph),
                     GetRefSeqId(),
                     range);
            }
            chunk->x_AddAnnotPlace(kTSEId);
            Uint8 bytes = max<Uint8>(1, chunks[k].align_count*k_align_bytes);
            double seconds = bytes*(k_read_byte_seconds+k_make_graph_seconds);
            chunk->x_SetLoadBytes(bytes);
            chunk->x_SetLoadSeconds(seconds);
            split_info.AddChunk(*chunk);
        }
    }

    chunk_info.SetLoaded();
}


void CCSRARefSeqInfo::LoadAnnotBlob(CTSE_LoadLock& load_lock)
{
    LoadAnnotMainSplit(load_lock);
}


void CCSRARefSeqInfo::LoadAnnotChunk(CTSE_Chunk_Info& chunk_info)
{
    if ( chunk_info.GetChunkId() == kMainChunkId ) {
        LoadAnnotMainChunk(chunk_info);
    }
    else {
        switch ( chunk_info.GetChunkId() % eCSRAAnnotChunk_mul ) {
        case eCSRAAnnotChunk_align:
            LoadAnnotAlignChunk(chunk_info);
            break;
        case eCSRAAnnotChunk_pileup_graph:
            LoadAnnotPileupChunk(chunk_info);
            break;
        }
    }
}


void CCSRARefSeqInfo::LoadRefSeqBlob(CTSE_LoadLock& load_lock)
{
    LoadRefSeqMainEntry(load_lock);
}


void CCSRARefSeqInfo::LoadRefSeqMainEntry(CTSE_LoadLock& load_lock)
{
    CRef<CSeq_entry> entry(new CSeq_entry);

    CCSraRefSeqIterator it(*m_File, GetRefSeqId());
    CRef<CBioseq> seq = it.GetRefBioseq(it.eOmitData);
    entry->SetSeq(*seq);
    TSeqPos ref_data_size = it.GetSeqLength();

    load_lock->SetSeq_entry(*entry);
    CTSE_Split_Info& split_info = load_lock->GetSplitInfo();

    // register ref seq data chunks
    TSeqPos chunk_size = m_File->GetDb().GetRowSize()*kChunkSeqDataMul;
    vector<CTSE_Chunk_Info::TLocation> loc_set(1);
    loc_set[0].first = GetRefSeqId();
    for ( TSeqPos i = 0; i*chunk_size < ref_data_size; ++i ) {
        int chunk_id = i;
        CRef<CTSE_Chunk_Info> chunk(new CTSE_Chunk_Info(chunk_id));
        loc_set[0].second.SetFrom(i*chunk_size);
        loc_set[0].second.SetToOpen(min(i*chunk_size+chunk_size,
                                        ref_data_size));
        chunk->x_AddSeq_data(loc_set);
        split_info.AddChunk(*chunk);
    }
}


void CCSRARefSeqInfo::LoadRefSeqChunk(CTSE_Chunk_Info& chunk_info)
{
    int range_id = chunk_info.GetChunkId();
    CTSE_Chunk_Info::TPlace place(GetRefSeqId(), 0);
    CRange<TSeqPos> range;
    TSeqPos chunk_data_size = m_File->GetDb().GetRowSize()*kChunkSeqDataMul;
    range.SetFrom(range_id*chunk_data_size);
    range.SetLength(chunk_data_size);
    if ( GetDebugLevel() >= 5 ) {
        LOG_POST_X(14, Info<<
                   "CCSRADataLoader:LoadRefSeqData("<<
                   chunk_info.GetBlobId().ToString()<<", "<<
                   chunk_info.GetChunkId());
    }
    _TRACE("Loading ref "<<GetRefSeqId()<<" @ "<<range);
    list< CRef<CSeq_literal> > data;
    CCSraRefSeqIterator(*m_File, GetRefSeqId()).GetRefLiterals(data, range);
    _ASSERT(!data.empty());
    chunk_info.x_LoadSequence(place, range.GetFrom(), data);
    chunk_info.x_AddUsedMemory(chunk_data_size/4); // 2na
    chunk_info.SetLoaded();
}


BEGIN_LOCAL_NAMESPACE;

enum EBaseStat {
    kStat_A = 0,
    kStat_C = 1,
    kStat_G = 2,
    kStat_T = 3,
    kStat_Insert = 4,
    kStat_Match = 5,
    kNumStat = 6
};

struct SBaseStat
{
    SBaseStat(void) {
        for ( int i = 0; i < kNumStat; ++i ) {
            cnts[i] = 0;
        }
    }

    unsigned total() const {
        unsigned ret = 0;
        for ( int i = 0; i < kNumStat; ++i ) {
            ret += cnts[i];
        }
        return ret;
    }

    void add_base(char b) {
        switch ( b ) {
        case 'A': cnts[kStat_A] += 1; break;
        case 'C': cnts[kStat_C] += 1; break;
        case 'G': cnts[kStat_G] += 1; break;
        case 'T': cnts[kStat_T] += 1; break;
        case '=': cnts[kStat_Match] += 1; break;
        }
    }
    void add_match() {
        cnts[kStat_Match] += 1;
    }
    void add_insert() {
        cnts[kStat_Insert] += 1;
    }

    unsigned cnts[kNumStat];
};

struct SBaseStats
{
    TSeqPos x_size;
    vector<SBaseStat> ss;

    TSeqPos size(void) const
        {
            return x_size;
        }
    void resize(TSeqPos len)
        {
            x_size = len;
        }
    bool x_empty(EBaseStat stat) const
        {
            return false;
        }
    unsigned x_get(EBaseStat stat, TSeqPos pos) const
        {
            return ss[pos].cnts[stat];
        }
    void x_add(EBaseStat stat, TSeqPos pos)
        {
            ss.resize(size());
            ss[pos].cnts[stat] += 1;
        }

    void add_stat(EBaseStat stat, TSeqPos pos)
        {
            x_add(stat, pos);
        }
    void add_stat(EBaseStat stat, TSeqPos pos, TSeqPos count)
        {
            TSeqPos end = pos + count;
            if ( pos > end ) { // alignment starts before current graph
                pos = 0;
            }
            end = min(size(), end);
            for ( TSeqPos i = pos; i < end; ++i ) {
                x_add(stat, i);
            }
        }
    void add_base(char b, TSeqPos pos)
        {
            if ( pos < size() ) {
                EBaseStat stat;
                switch ( b ) {
                case 'A': stat = kStat_A; break;
                case 'C': stat = kStat_C; break;
                case 'G': stat = kStat_G; break;
                case 'T': stat = kStat_T; break;
                case '=': stat = kStat_Match; break;
                default: return;
                }
                add_stat(stat, pos);
            }
        }
    void add_match(TSeqPos pos, TSeqPos count)
        {
            add_stat(kStat_Match, pos, count);
        }
    void add_insert(TSeqPos pos, TSeqPos count)
        {
            add_stat(kStat_Insert, pos, count);
        }

    pair<unsigned, unsigned> get_min_max(EBaseStat stat) const
        {
            pair<unsigned, unsigned> c_min_max;
            if ( !ss.empty() && !x_empty(stat) ) {
                c_min_max.first = c_min_max.second = x_get(stat, 0);
                for ( TSeqPos i = 1; i < size(); ++i ) {
                    unsigned c = x_get(stat, i);
                    c_min_max.first = min(c_min_max.first, c);
                    c_min_max.second = max(c_min_max.second, c);
                }
            }
            return c_min_max;
        }
    
    void get_bytes(EBaseStat stat, CByte_graph::TValues& vv)
        {
            vv.reserve(size());
            for ( TSeqPos i = 0; i < size(); ++i ) {
                vv.push_back(CByte_graph::TValues::value_type(x_get(stat, i)));
            }
        }
    void get_ints(EBaseStat stat, CInt_graph::TValues& vv)
        {
            vv.reserve(size());
            for ( TSeqPos i = 0; i < size(); ++i ) {
                vv.push_back(CInt_graph::TValues::value_type(x_get(stat, i)));
            }
        }
};


struct SChunkAnnots {
    SChunkAnnots(CCSRAFileInfo* file_info,
                 ECSRAAnnotChunkIdType type);

    CRef<CCSRAFileInfo> m_File;
    bool m_SeparateSpotGroups;
    ECSRAAnnotChunkIdType m_Type;
    typedef pair<CRef<CSeq_annot>, SBaseStats> TSlot;
    typedef map<string, TSlot> TAnnots;
    TAnnots m_Annots;
    TAnnots::iterator m_LastAnnot;

    TSlot& Select(const CCSraAlignIterator& ait);
    void Create(const string& name);
};


SChunkAnnots::SChunkAnnots(CCSRAFileInfo* file,
                           ECSRAAnnotChunkIdType type)
    : m_File(file),
      m_SeparateSpotGroups(!m_File->GetSeparateSpotGroups().empty()),
      m_Type(type),
      m_LastAnnot(m_Annots.end())
{
    if ( !m_SeparateSpotGroups ) {
        Create(kEmptyStr);
    }
    else if ( m_Type == eCSRAAnnotChunk_pileup_graph ) {
        ITERATE ( vector<string>, it, m_File->GetSeparateSpotGroups() ) {
            Create(*it);
        }
    }
}


SChunkAnnots::TSlot& SChunkAnnots::Select(const CCSraAlignIterator& ait)
{
    if ( m_SeparateSpotGroups ) {
        CTempString spot_group_name = ait.GetSpotGroup();
        if ( m_LastAnnot == m_Annots.end() ||
             spot_group_name != m_LastAnnot->first ) {
            Create(spot_group_name);
        }
    }
    return m_LastAnnot->second;
}


void SChunkAnnots::Create(const string& name)
{
    m_LastAnnot = m_Annots.insert(TAnnots::value_type(name, TSlot(null, SBaseStats()))).first;
    if ( !m_LastAnnot->second.first ) {
        const string& annot_name = m_File->GetAnnotName(name, m_Type);
        if ( m_Type == eCSRAAnnotChunk_align ) {
            m_LastAnnot->second.first =
                CCSraAlignIterator::MakeEmptyMatchAnnot(annot_name);
        }
        else {
            m_LastAnnot->second.first =
                CCSraAlignIterator::MakeSeq_annot(annot_name);
        }
    }
}

END_LOCAL_NAMESPACE;


void CCSRARefSeqInfo::LoadAnnotAlignChunk(CTSE_Chunk_Info& chunk_info)
{
    if ( GetDebugLevel() >= 5 ) {
        LOG_POST_X(15, Info<<
                   "CCSRADataLoader:LoadAlignChunk("<<
                   chunk_info.GetBlobId().ToString()<<", "<<
                   chunk_info.GetChunkId());
    }
    int range_id = chunk_info.GetChunkId() / eCSRAAnnotChunk_mul;
    TSeqPos pos = m_AlignChunks[range_id].start_pos;
    TSeqPos end = m_AlignChunks[range_id+1].start_pos;
    CTSE_Chunk_Info::TPlace place(CSeq_id_Handle(), kTSEId);
    int min_quality = m_MinMapQuality;
    _TRACE("Loading aligns "<<GetRefSeqId()<<" @ "<<pos<<"-"<<(end-1));
    size_t skipped = 0, count = 0;
    SChunkAnnots annots(m_File, eCSRAAnnotChunk_align);
   
    CCSraAlignIterator ait(*m_File, GetRefSeqId(), pos, end-pos,
                           CCSraAlignIterator::eSearchByStart);
    for( ; ait; ++ait ){
        _ASSERT(ait.GetRefSeqPos() >= pos && ait.GetRefSeqPos() < end);
        if ( min_quality > 0 && ait.GetMapQuality() < min_quality ) {
            ++skipped;
            continue;
        }
        ++count;

        annots.Select(ait).first->SetData().SetAlign().push_back(ait.GetMatchAlign());
    }
    if ( !annots.m_Annots.empty() ) {
        if ( GetDebugLevel() >= 9 ) {
            LOG_POST_X(8, Info <<
                       "CCSRADataLoader(" << m_File->GetCSRAName() << "): "
                       "Chunk "<<chunk_info.GetChunkId());
        }
        ITERATE ( SChunkAnnots::TAnnots, it, annots.m_Annots ) {
            chunk_info.x_LoadAnnot(place, *it->second.first);
        }
    }
    if ( GetDebugLevel() >= 2 ) {
        LOG_POST_X(7, Info<<"CCSRADataLoader: "
                   "Loaded "<<GetRefSeqId()<<" @ "<<
                   pos<<"-"<<(end-1)<<": "<<
                   count<<" skipped: "<<skipped);
    }
    // estimated memory usage
    chunk_info.x_AddUsedMemory(count*1000);
    chunk_info.SetLoaded();
}


void CCSRARefSeqInfo::LoadAnnotPileupChunk(CTSE_Chunk_Info& chunk_info)
{
    if ( GetDebugLevel() >= 5 ) {
        LOG_POST_X(16, Info<<
                   "CCSRADataLoader:LoadPileupChunk("<<
                   chunk_info.GetBlobId().ToString()<<", "<<
                   chunk_info.GetChunkId());
    }
    int chunk_id = chunk_info.GetChunkId();
    int range_id = chunk_id / eCSRAAnnotChunk_mul;
    TSeqPos pos = m_GraphChunks[range_id].start_pos;
    TSeqPos end = m_GraphChunks[range_id+1].start_pos;
    TSeqPos len = end - pos;
    CTSE_Chunk_Info::TPlace place(CSeq_id_Handle(), kTSEId);
    int min_quality = m_MinMapQuality;
    _TRACE("Loading pileup "<<GetRefSeqId()<<" @ "<<pos<<"-"<<(end-1));
    size_t count = 0, skipped = 0;

    SChunkAnnots annots(m_File, eCSRAAnnotChunk_pileup_graph);

    CCSraAlignIterator ait(*m_File, GetRefSeqId(), pos, len);
    for ( ; ait; ++ait ) {
        if ( min_quality > 0 && ait.GetMapQuality() < min_quality ) {
            ++skipped;
            continue;
        }
        ++count;
        SBaseStats& ss = annots.Select(ait).second;
        ss.resize(len);

        TSeqPos ref_pos = ait.GetRefSeqPos()-pos;
        TSeqPos read_pos = ait.GetShortPos();
        CTempString read = ait.GetMismatchRead();
        CTempString cigar = ait.GetCIGARLong();
        const char* ptr = cigar.data();
        const char* end = cigar.end();
        while ( ptr != end ) {
            char type = 0;
            int seglen = 0;
            for ( ; ptr != end; ) {
                char c = *ptr++;
                if ( c >= '0' && c <= '9' ) {
                    seglen = seglen*10+(c-'0');
                }
                else {
                    type = c;
                    break;
                }
            }
            if ( seglen == 0 ) {
                NCBI_THROW_FMT(CSraException, eOtherError,
                               "Bad CIGAR length: " << type <<
                               "0 in " << cigar);
            }
            if ( type == '=' ) {
                // match
                ss.add_match(ref_pos, seglen);
                ref_pos += seglen;
                read_pos += seglen;
            }
            else if ( type == 'M' || type == 'X' ) {
                // mismatch
                for ( int i = 0; i < seglen; ++i ) {
                    if ( ref_pos < len ) {
                        ss.add_base(read[read_pos], ref_pos);
                    }
                    ++ref_pos;
                    ++read_pos;
                }
            }
            else if ( type == 'I' || type == 'S' ) {
                if ( type == 'S' ) {
                    // soft clipping already accounted in seqpos
                    continue;
                }
                read_pos += seglen;
            }
            else if ( type == 'N' ) {
                // intron
                ref_pos += seglen;
            }
            else if ( type == 'D' ) {
                // delete
                ss.add_insert(ref_pos, seglen);
                ref_pos += seglen;
            }
            else if ( type != 'P' ) {
                NCBI_THROW_FMT(CSraException, eOtherError,
                               "Bad CIGAR char: " <<type<< " in " <<cigar);
            }
        }
    }

    if ( GetDebugLevel() >= 2 ) {
        LOG_POST_X(10, Info<<"CCSRADataLoader: "
                   "Loaded pileup "<<GetRefSeqId()<<" @ "<<
                   pos<<"-"<<(end-1)<<": "<<
                   count<<" skipped: "<<skipped);
    }

    size_t total_bytes = 0;
    NON_CONST_ITERATE ( SChunkAnnots::TAnnots, it, annots.m_Annots ) {
        SBaseStats& ss = it->second.second;
        if ( kOmitEmptyPileup && !ss.size() ) {
            continue;
        }
        for ( int k = 0; k < kNumStat; ++k ) {
            CRef<CSeq_graph> graph(new CSeq_graph);
            static const char* const titles[6] = {
                "Number of A bases",
                "Number of C bases",
                "Number of G bases",
                "Number of T bases",
                "Number of inserts",
                "Number of matches"
            };
            graph->SetTitle(titles[k]);
            CSeq_interval& loc = graph->SetLoc().SetInt();
            loc.SetId(*ait.GetRefSeq_id());
            loc.SetFrom(pos);
            loc.SetTo(pos+len-1);
            graph->SetNumval(len);

            pair<unsigned, unsigned> c_min_max = ss.get_min_max(EBaseStat(k));
            if ( c_min_max.second == 0 ) {
                CByte_graph& data = graph->SetGraph().SetByte();
                data.SetValues().resize(len);
                data.SetMin(0);
                data.SetMax(0);
                data.SetAxis(0);
                total_bytes += data.SetValues().size();
            }
            else if ( c_min_max.second < 256 ) {
                CByte_graph& data = graph->SetGraph().SetByte();
                CByte_graph::TValues& vv = data.SetValues();
                ss.get_bytes(EBaseStat(k), vv);
                data.SetMin(c_min_max.first);
                data.SetMax(c_min_max.second);
                data.SetAxis(0);
                total_bytes += vv.size();
            }
            else {
                CInt_graph& data = graph->SetGraph().SetInt();
                CInt_graph::TValues& vv = data.SetValues();
                ss.get_ints(EBaseStat(k), vv);
                data.SetMin(c_min_max.first);
                data.SetMax(c_min_max.second);
                data.SetAxis(0);
                total_bytes += vv.size()*sizeof(vv[0]);
            }
            it->second.first->SetData().SetGraph().push_back(graph);
        }
        if ( GetDebugLevel() >= 9 ) {
            LOG_POST_X(11, Info<<"CCSRADataLoader: "
                       "Loaded pileup "<<GetRefSeqId()<<" @ "<<
                       pos<<"-"<<(end-1)<<": "<<
                       MSerial_AsnText<<*it->second.first);
        }
        chunk_info.x_LoadAnnot(place, *it->second.first);
    }
    if ( GetDebugLevel() >= 2 ) {
        LOG_POST_X(11, Info<<"CCSRADataLoader: "
                   "Loaded pileup "<<GetRefSeqId()<<" @ "<<
                   pos<<"-"<<(end-1)<<": "<<total_bytes<<" bytes");
    }
    chunk_info.x_AddUsedMemory(total_bytes);
    chunk_info.SetLoaded();
}


END_SCOPE(objects)
END_NCBI_SCOPE
