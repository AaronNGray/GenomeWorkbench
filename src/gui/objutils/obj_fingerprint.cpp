/*  $Id: obj_fingerprint.cpp 43649 2019-08-13 18:12:56Z katargir $
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
 * Authors: Roman Katargin, Victor Joukov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbimtx.hpp>
#include <corelib/ncbi_bswap.hpp>
#include <gui/objutils/obj_fingerprint.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/utils/track_info.hpp>
#include <objmgr/util/sequence.hpp>
#include <serial/serialbase.hpp>

#include <stdio.h>

#define CKSUM_ADD_INTEGER(cksum, value) \
    CObjFingerprint::AddInteger(cksum, (unsigned char*)&value, sizeof(value))

#define CKSUM_INTEGER_FIELD(cksum, Type, field) \
if (obj.IsSet##field()) { \
    Type::T##field value = obj.Get##field(); \
    CObjFingerprint::AddInteger(cksum, (unsigned char*)&value, sizeof(value)); \
}

#define CKSUM_INTEGER_FIELD2(cksum, Type, field) \
{ \
    Type::T##field value = obj.Get##field(); \
    CObjFingerprint::AddInteger(cksum, (unsigned char*)&value, sizeof(value)); \
}

#include "fingp_seqid.cpp"
#include "fingp_seqloc.cpp"
#include "fingp_seqfeat.cpp"
#include "fingp_seqalign.cpp"
#include "fingp_seqgraph.cpp"

BEGIN_NCBI_SCOPE

DEFINE_STATIC_MUTEX(s_Mutex);

void CObjFingerprint::Register(TTypeInfo type_info, CObjFingerprint::TFingerPFunc func)
{
    CMutexGuard LOCK(s_Mutex);
    x_GetInstance().x_Register(type_info, func);
}

void CObjFingerprint::GetFingerprint(CChecksum& checksum, const CObject& obj,
                                     objects::CScope* scope)
{
    CMutexGuard LOCK(s_Mutex);
    x_GetInstance().Fingerprint(checksum, obj, scope);
}

CObjFingerprint& CObjFingerprint::x_GetInstance()
{
    static CObjFingerprint instance;
    return instance;
}

void CObjFingerprint::Fingerprint(CChecksum& checksum, const CObject& obj,
                                  objects::CScope* scope) const
{
    const CSerialObject* so = dynamic_cast<const CSerialObject*>(&obj);
    if (so == NULL)
        return;

    TMap::const_iterator it = m_Map.find(so->GetThisTypeInfo());
    if (it == m_Map.end())
        return;

    it->second(checksum, obj, scope, *this);
}

void CObjFingerprint::x_Register(TTypeInfo type_info, TFingerPFunc func)
{
    m_Map[type_info] = func;
}



void CObjFingerprint::AddInteger(CChecksum& checksum, unsigned char* pvalue, int size)
{
    Int4 value = 0;

    switch(size) {
    case 1:
        value = (Int4)*pvalue;
        break;
    case 2:
        value = (Int4)*(Int2*)pvalue;
        break;
    case 4:
        value = *(Int4*)pvalue;
        break;
    case 8:
        {{
            string str;
            str = NStr::UInt8ToString(*(Uint8*)pvalue, 0, 16);
            checksum.AddLine(str);
            return;
        }}
    }

#ifdef WORDS_BIGENDIAN
    CByteSwap::PutInt4((unsigned char*)&value, value);
#endif

    checksum.AddChars((const char*)&value, sizeof(value));
}

void CObjFingerprint::AddDouble(CChecksum& checksum, double value)
{
    string str;
    NStr::DoubleToString(str, value);
    checksum.AddLine(str);
}


// signatures
string CObjFingerprint::GetFeatSignature(const objects::CSeq_feat& feat,
    objects::CScope* scope, const string& data_source,
    const string& sAdditionalInfo)
{
    return GetFeatSignature(feat, feat.GetLocation(), scope, data_source, sAdditionalInfo);
}


// signatures
string CObjFingerprint::GetFeatSignature(const objects::CSeq_feat& feat,
    objects::CScope* scope, const objects::CSeq_annot_Handle& annot_handle,
    const string& sAdditionalInfo)
{
    return GetFeatSignature(feat, feat.GetLocation(), scope, annot_handle, sAdditionalInfo);
}


string CObjFingerprint::GetFeatSignature(const objects::CSeq_feat& feat,
                                         const objects::CSeq_loc& loc,
                                         objects::CScope* scope,
                                         const objects::CSeq_annot_Handle& annot_handle,
                                         const string& sAdditionalInfo)
{
    string annot_name = "Unnamed";
    if (annot_handle.IsNamed()) {
        annot_name = annot_handle.GetName();
    }
    return GetFeatSignature(feat, loc, scope, annot_name, sAdditionalInfo);
}


static inline void s_EscapeDash(string& id)
{
    // we need to escape '\' itself to cancel out
    // one JSON decoding so that '-' is still escaped
    NStr::ReplaceInPlace(id, "-", "\\\\-");
}


string CObjFingerprint::GetFeatSignature(const objects::CSeq_feat& feat,
                                         const objects::CSeq_loc& loc,
                                         objects::CScope* scope,
                                         const string& data_source,
                                         const string& sAdditionalInfo)
{
    //CSeq_id_Handle shdl = sequence::GetIdHandle(loc, scope);
    //shdl = sequence::GetId(shdl, *scope, sequence::eGetId_ForceGi);
    //int gi = shdl.GetGi();
    CSeq_id_Handle shdl = sequence::GetIdHandle(loc, scope);
    shdl = sequence::GetId(shdl, *scope, sequence::eGetId_Canonical);
    string id_str = shdl.GetSeqId()->AsFastaString();
    s_EscapeDash(id_str);

    TSeqRange range = loc.GetTotalRange();

    Uint2 type = (eFeature) << 8;
    type |= feat.GetData().GetSubtype() & 0xff;

    CChecksum cs;
    GetFingerprint(cs, feat, scope);
    Uint4 checksum = cs.GetChecksum();

    bool is_naa = CSeqUtils::IsNAA(data_source);
    bool is_rmt = CSeqUtils::isRmtAnnotName(data_source);
    char buf[128];
    if (data_source.empty()) {
        sprintf(buf, "-%08x-%08x-%04x-%08x",
                range.GetFrom(), range.GetTo(), type, checksum);
    } else if (is_naa || is_rmt) {
        sprintf(buf, "-%08x-%08x-%04x-%08x-",
                range.GetFrom(), range.GetTo(), type, checksum);
    } else {
        CChecksum ds_cs;
        ds_cs.AddLine(data_source);
        Uint4 ds_checksum = ds_cs.GetChecksum();
        sprintf(buf, "-%08x-%08x-%04x-%08x-%08x",
                range.GetFrom(), range.GetTo(), type, checksum, ds_checksum);
    }
    string sAdditionalInfoDigest;
    if(!sAdditionalInfo.empty()) {
        CChecksum AdditionalInfoCS;
        AdditionalInfoCS.AddLine(sAdditionalInfo);
        sAdditionalInfoDigest = AdditionalInfoCS.GetHexSum();

    }
    return id_str + buf + ((is_naa || is_rmt) ? data_source : "") + (sAdditionalInfoDigest.empty() ? "" : ("-" + sAdditionalInfoDigest));
}


string CObjFingerprint::GetGraphSignature(const objects::CSeq_graph& graph,
    objects::CScope* scope, const string& data_source)
{
    CSeq_id_Handle shdl = sequence::GetId(*graph.GetLoc().GetId(), *scope,
                                          sequence::eGetId_Canonical);
    string id_str = shdl.GetSeqId()->AsFastaString();
    s_EscapeDash(id_str);

    TSeqRange range = graph.GetLoc().GetTotalRange();

    Uint2 type = (eGraph) << 8;

    CChecksum cs;
    GetFingerprint(cs, graph, scope);
    Uint4 checksum = cs.GetChecksum();

    bool is_naa = CSeqUtils::IsNAA(data_source);
    char buf[128];
    if (data_source.empty()) {
        sprintf(buf, "-%08x-%08x-%04x-%08x",
                range.GetFrom(), range.GetTo(), type, checksum);
    } else if (is_naa) {
        sprintf(buf, "-%08x-%08x-%04x-%08x-",
                range.GetFrom(), range.GetTo(), type, checksum);
    } else {
        CChecksum ds_cs;
        ds_cs.AddLine(data_source);
        Uint4 ds_checksum = ds_cs.GetChecksum();
        sprintf(buf, "-%08x-%08x-%04x-%08x-%08x",
                range.GetFrom(), range.GetTo(), type, checksum, ds_checksum);
    }

    return id_str + buf + (is_naa ? data_source : "");
}


string CObjFingerprint::GetAlignSignature(const objects::CSeq_align& align,
                                          const objects::CSeq_loc&   on_loc,
                                          objects::CScope*           scope,
                                          const objects::CSeq_annot_Handle& annot_handle)
{
    string annot_name = "Unnamed";
    if (annot_handle.IsNamed()) {
        annot_name = annot_handle.GetName();
    }
    return GetAlignSignature(align, on_loc, scope, annot_name);
}


string CObjFingerprint::GetAlignSignature(const objects::CSeq_align& align,
    const objects::CSeq_loc& on_loc, objects::CScope* scope,
    const string& data_source)
{
    CSeq_id_Handle shdl = sequence::GetId(*on_loc.GetId(), *scope,
        sequence::eGetId_Canonical);

    string id_str = shdl.GetSeqId()->AsFastaString();
    s_EscapeDash(id_str);

    TSeqRange range = on_loc.GetTotalRange();

    Uint2 type = (eAlignment) << 8;

    CChecksum cs;
    GetFingerprint(cs, align, scope);
    Uint4 checksum = cs.GetChecksum();

    // we should include the data source/annot into the signature if it's either named or
    // AlignDb
    bool is_include_ds = CSeqUtils::IsNAA(data_source) || CSeqUtils::IsAlignDb(data_source);

    // named annotation or AlignDb#batch
    string AdjustedDS(data_source);
    char buf[128];
    if (data_source.empty()) {
        sprintf(buf, "-%08x-%08x-%04x-%08x",
                range.GetFrom(), range.GetTo(), type, checksum);
    } else if (is_include_ds) {
        sprintf(buf, "-%08x-%08x-%04x-%08x-",
                range.GetFrom(), range.GetTo(), type, checksum);
        if(CSeqUtils::IsAlignDb(data_source)) {
            AdjustedDS = "AlignDb";
            // get the batch from score named "batch_id"
            //!! an alternative way would be to parse data_source since it now contains all necessary info,
            //!! but in a proprietary way like: "AlignDb:batch_id in (114783) AND status = \'ONLINE\':subject"
            int batch(0);
            bool batch_found(align.GetNamedScore("batch_id", batch)); (void)batch_found;
            NCBI_ASSERT(batch_found, "AlignDb sourced alignment does not have a \"batch_id\" score!");

            // besides the batch_id, we need to get whether the query was obtained as "query" or "subject"
            // ("is_source_assembly_query" from SV-3197)
            // there is no direct way to get this from seq-align, but indirectly this can be seen by comparing the 
            // seq-id in the 0 row and the id_str
            string sIsQuery("N");
            try {
                if(align.GetSeq_id(0).AsFastaString() == id_str) {
                    sIsQuery = "Y";
                }
            } catch(...) {}
            
            AdjustedDS += string("#") + NStr::NumericToString(batch) + "#" + sIsQuery;
        }
    } else {
        CChecksum ds_cs;
        ds_cs.AddLine(data_source);
        Uint4 ds_checksum = ds_cs.GetChecksum();
        sprintf(buf, "-%08x-%08x-%04x-%08x-%08x",
                range.GetFrom(), range.GetTo(), type, checksum, ds_checksum);
    }
    

    return id_str + buf + (is_include_ds ? AdjustedDS : "");
}


string CObjFingerprint::GetSeqLocSignature(const objects::CSeq_loc& loc,
    objects::CScope* scope)
{
    CSeq_id_Handle shdl = sequence::GetId(*loc.GetId(), *scope,
        sequence::eGetId_Canonical);

    string id_str = shdl.GetSeqId()->AsFastaString();
    s_EscapeDash(id_str);

    TSeqRange range = loc.GetTotalRange();

    Uint2 type = (eLocation) << 8;

    CChecksum cs;
    GetFingerprint(cs, loc, scope);
    Uint4 checksum = cs.GetChecksum();

    char buf[128];
    sprintf(buf, "-%08x-%08x-%04x-%08x",
            range.GetFrom(), range.GetTo(), type, checksum);

    return id_str + buf;
}


string CObjFingerprint::GetHistogramSignature(const objects::CSeq_loc& loc,
                                              EObjectType obj_type,
                                              objects::CScope*   scope,
                                              const string& title,
                                              const string& data_source,
                                              int subtype)
{
    CSeq_id_Handle shdl = sequence::GetId(*loc.GetId(), *scope,
        sequence::eGetId_Canonical);

    string id_str = shdl.GetSeqId()->AsFastaString();
    TSeqRange range = loc.GetTotalRange();
    s_EscapeDash(id_str);

    Uint2 type = (obj_type) << 8;
    type |= subtype & 0xff;

    CChecksum cs;
    cs.AddLine(title);
    Uint4 checksum = cs.GetChecksum();

    bool is_naa = CSeqUtils::IsNAA(data_source);
    bool is_rmt = CSeqUtils::isRmtAnnotName(data_source);
    char buf[128];
    if (data_source.empty()) {
        sprintf(buf, "-%08x-%08x-%04x-%08x",
                range.GetFrom(), range.GetTo(), type, checksum);
    } else if (is_naa || is_rmt) {
        sprintf(buf, "-%08x-%08x-%04x-%08x-",
                range.GetFrom(), range.GetTo(), type, checksum);
    } else {
        CChecksum ds_cs;
        ds_cs.AddLine(data_source);
        Uint4 ds_checksum = ds_cs.GetChecksum();
        sprintf(buf, "-%08x-%08x-%04x-%08x-%08x",
                range.GetFrom(), range.GetTo(), type, checksum, ds_checksum);
    }

    return id_str + buf + ((is_naa || is_rmt) ? data_source : "");
}


string
CObjFingerprint::GetComponentSignature(const objects::CSeq_loc& comp_loc,
                                       const objects::CSeq_loc& mapped_loc,
                                       objects::CScope*         scope)
{
    CSeq_id_Handle shdl = sequence::GetId(*mapped_loc.GetId(), *scope,
        sequence::eGetId_Canonical);

    string id_str = shdl.GetSeqId()->AsFastaString();
    s_EscapeDash(id_str);

    TSeqRange range = mapped_loc.GetTotalRange();

    Uint2 type = (eComponent) << 8;

    CChecksum cs;
    GetFingerprint(cs, comp_loc, scope);
    Uint4 checksum = cs.GetChecksum();

    char buf[128];
    sprintf(buf, "-%08x-%08x-%04x-%08x",
            range.GetFrom(), range.GetTo(), type, checksum);

    return id_str + buf;
}


string CObjFingerprint::GetRsiteSignature(const CSeq_feat& feat, int pnt, CScope* scope)
{
    CSeq_id_Handle shdl = sequence::GetIdHandle(feat.GetLocation(), scope);
    shdl = sequence::GetId(shdl, *scope, sequence::eGetId_Canonical);
    string id_str = shdl.GetSeqId()->AsFastaString();
    s_EscapeDash(id_str);

    Uint2 type = (eRsite) << 8;

    CChecksum cs;
    GetFingerprint(cs, feat, scope);
    Uint4 checksum = cs.GetChecksum();

    char buf[128];
    sprintf(buf, "-%08x-%08x-%04x-%08x", pnt, pnt, type, checksum);
    return id_str + buf;
}


void CObjFingerprint::ParseSignature(const string&    sig,
                                     objects::CSeq_id_Handle&  sih,
                                     TSeqPos&         from,
                                     TSeqPos&         to,
                                     EObjectType&     type,
                                     int&             subtype,
                                     Uint4&           fingerprint,
                                     Uint4&           ds_fingerprint,
                                     string&          ds_name,
                                     objects::CScope* scope)
{
    if ( !NStr::MatchesMask(sig, "*-*-*-*-*") ) {
        NCBI_THROW(CException, eUnknown, "invalid object signature");
    }

    vector<string> toks;
    CTrackUtils::TokenizeWithEscape(sig, "-", toks, true);
    vector<string>::const_iterator iter = toks.begin();

    /// first: id can be in any forms (gi, accession, local, gnl ...)
    string id_str = *iter++;

    CRef<CSeq_id> id(new CSeq_id);
    id->Set(id_str);
    sih = sequence::GetId(*id, *scope);
    if ( !sih ) {
        NCBI_THROW(CException, eUnknown,
            "failed to retrieve sequence (ParseSignature) : " + id_str);
    }

    /// next: total range
    from = NStr::StringToUInt(*iter++, 0, 16);
    to   = NStr::StringToUInt(*iter++, 0, 16);

    /// next: type and subtype
    Uint4 tmp = NStr::StringToUInt(*iter++, 0, 16);
    type = static_cast<EObjectType>((tmp & 0xff00) >> 8);
    subtype = tmp & 0xff;

    /// next: fingerprint
    fingerprint = NStr::StringToUInt(*iter++, 0, 16);

    /// last: data source fingerprint (optional)
    if (iter != toks.end()) {
        const string& ds = *iter++;
        if (CSeqUtils::IsNAA(ds) || CSeqUtils::IsAlignDb(ds)) {
            ds_name = ds;
            ds_fingerprint = 0;
        } else {
            ds_fingerprint = NStr::StringToUInt(ds, 0, 16);
        }
    } else {
        ds_fingerprint = 0;
    }

}

string CObjFingerprint::ParseSignatureDS(const string&    sig)
{
    vector<string> toks;
    CTrackUtils::TokenizeWithEscape(sig, "-", toks, true);

    return toks.size() > 5 ? toks[5] : "";
}


END_NCBI_SCOPE

