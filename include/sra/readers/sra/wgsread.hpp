#ifndef SRA__READER__SRA__WGSREAD__HPP
#define SRA__READER__SRA__WGSREAD__HPP
/*  $Id: wgsread.hpp 593962 2019-09-26 19:42:58Z vasilche $
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
 * Authors:  Eugene Vasilchenko
 *
 * File Description:
 *   Access to WGS files
 *
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbimtx.hpp>
#include <util/range.hpp>
#include <util/rangemap.hpp>
#include <util/simple_buffer.hpp>
#include <serial/serialbase.hpp>
#include <sra/readers/sra/vdbread.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/seq/Seq_data.hpp>
#include <sra/readers/sra/snpread.hpp> // for CSafeFlags
#include <map>
#include <list>

//#include <ncbi/ncbi.h> // for NCBI_gb_state
typedef uint32_t NCBI_gb_state;
//#include <insdc/insdc.h> // for INSDC_coord_*, INSDC_quality_phred
typedef int32_t INSDC_coord_zero;
typedef int32_t INSDC_coord_one;
typedef uint32_t INSDC_coord_len;
typedef uint8_t INSDC_quality_phred;
//#include <ncbi/wgs-contig.h> // for NCBI_WGS_component_props, NCBI_WGS_gap_linkage
typedef int16_t NCBI_WGS_component_props;
typedef int32_t NCBI_WGS_gap_linkage;

// missing wgs-contig.h definitions
typedef uint8_t NCBI_WGS_seqtype;
enum
{
    NCBI_WGS_seqtype_contig                 = 0,
    NCBI_WGS_seqtype_scaffold               = 1,
    NCBI_WGS_seqtype_protein                = 2,
    NCBI_WGS_seqtype_mrna                   = 3
};

typedef uint8_t NCBI_WGS_feattype;

typedef uint8_t NCBI_WGS_loc_strand;
enum
{
    NCBI_WGS_loc_strand_unknown             = 0,
    NCBI_WGS_loc_strand_plus                = 1,
    NCBI_WGS_loc_strand_minus               = 2,
    NCBI_WGS_loc_strand_both                = 3
};


enum {
    NCBI_gb_state_eWGSGenBankLive = 0,
    NCBI_gb_state_eWGSGenBankSuppressed = 1,
    NCBI_gb_state_eWGSGenBankReplaced = 2,
    NCBI_gb_state_eWGSGenBankWithdrawn = 3,
    NCBI_gb_state_eWGSGenBankUnverified = 4,
    NCBI_gb_state_eWGSGenBankMissing = 5
};


BEGIN_NCBI_NAMESPACE;

class CObjectOStreamAsnBinary;

BEGIN_NAMESPACE(objects);

class CSeq_entry;
class CSeq_annot;
class CSeq_align;
class CSeq_graph;
class CSeq_feat;
class CBioseq;
class CSeq_literal;
class CUser_object;
class CUser_field;
class CID2S_Split_Info;
class CID2S_Chunk;

class CWGSSeqIterator;
class CWGSScaffoldIterator;
class CWGSGiIterator;
class CWGSProteinIterator;
class CWGSFeatureIterator;

struct SWGSCreateInfo;
struct SWGSFeatChunkInfo;

class NCBI_SRAREAD_EXPORT CAsnBinData : public CObject
{
public:
    explicit CAsnBinData(CSerialObject& obj);
    virtual ~CAsnBinData(void);

    const CSerialObject& GetMainObject(void) const {
        return *m_MainObject;
    }
    virtual void Serialize(CObjectOStreamAsnBinary& out) const;
    
private:
    CRef<CSerialObject> m_MainObject;
};


struct SWGSDb_Defs
{
    enum EFlags {
        fIds_gi       = 1<<0,
        fIds_acc      = 1<<1,
        fIds_gnl      = 1<<2,
        fMaskIds      = fIds_gi|fIds_acc|fIds_gnl,
        fDefaultIds   = fIds_gi|fIds_acc|fIds_gnl,

        fInst_ncbi4na = 0<<3,
        fInst_delta   = 1<<3,
        fMaskInst     = fInst_ncbi4na|fInst_delta,
        fDefaultInst  = fInst_delta,

        fSeqDescr     = 1<<4,
        fNucProtDescr = 1<<12,
        fMasterDescr  = 1<<5,
        fMaskDescr    = fSeqDescr|fNucProtDescr|fMasterDescr,
        fDefaultDescr = fSeqDescr|fNucProtDescr|fMasterDescr,

        fSeqAnnot     = 1<<6,
        fQualityGraph = 1<<7,
        fMaskAnnot    = fSeqAnnot|fQualityGraph,
        fDefaultAnnot = fSeqAnnot|fQualityGraph,

        fSplitQualityGraph  = 1<<8,
        fSplitSeqData       = 1<<9,
        fSplitProducts      = 1<<10,
        fSplitFeatures      = 1<<11,
        fSplitAll     = fSplitQualityGraph | fSplitSeqData | fSplitProducts | fSplitFeatures,
        fSplitMask    = fSplitQualityGraph | fSplitSeqData | fSplitProducts | fSplitFeatures,
        fDefaultSplit = fSplitAll,

        fDefaultFlags = fDefaultIds|fDefaultDescr|fDefaultAnnot|fDefaultInst|fDefaultSplit
    };
    DECLARE_SAFE_FLAGS_TYPE(EFlags, TFlags);

    enum EFeatLocIdType : Int1 {
        eFeatLocIdUninitialized = -1,
        eFeatLocIdGi = 0,
        eFeatLocIdAccVer,
        eFeatLocIdAccNoVer
    };
};
DECLARE_SAFE_FLAGS(SWGSDb_Defs::EFlags);


class NCBI_SRAREAD_EXPORT CWGSDb_Impl : public CObject, public SWGSDb_Defs
{
public:
    CWGSDb_Impl(CVDBMgr& mgr,
                CTempString path_or_acc,
                CTempString vol_path = CTempString());
    virtual ~CWGSDb_Impl(void);

    const string& GetIdPrefix(void) const {
        return m_IdPrefix;
    }
    const string& GetIdPrefixWithVersion(void) const {
        return m_IdPrefixWithVersion;
    }
    const string& GetWGSPath(void) const {
        return m_WGSPath;
    }

    // normalize accession to the form acceptable by SRA SDK
    // 0. if the argument looks like path - do not change it
    // 1. exclude contig/scaffold id
    // 2. add default version (1)
    static string NormalizePathOrAccession(CTempString path_or_acc,
                                           CTempString vol_path = CTempString());

    enum ERowType {
        eRowType_contig   = 0,
        eRowType_scaffold = 'S',
        eRowType_protein  = 'P'
    };
    typedef char TRowType;
    enum EAllowRowType {
        fAllowRowType_contig   = 1<<0,
        fAllowRowType_scaffold = 1<<1,
        fAllowRowType_protein  = 1<<2
    };
    DECLARE_SAFE_FLAGS_TYPE(EAllowRowType, TAllowRowType);
    // parse row id from accession
    // returns (row, accession_type) pair
    // row will be 0 if accession is in wrong format
    static
    pair<TVDBRowId, ERowType> ParseRowType(CTempString acc,
                                           TAllowRowType allow);
    // parse row id from accession
    // returns 0 if accession is in wrong format
    // if is_scaffold flag pointer is not null, then scaffold ids are also
    // accepted and the flag is set appropriately
    TVDBRowId ParseRow(CTempString acc, bool* is_scaffold) const;
    // parse contig row id from accession
    // returns 0 if accession is in wrong format
    static
    TVDBRowId ParseContigRow(CTempString acc) {
        return ParseRowType(acc, fAllowRowType_contig).first;
    }
    // parse scaffold row id from accession
    // returns 0 if accession is in wrong format
    static
    TVDBRowId ParseScaffoldRow(CTempString acc) {
        return ParseRowType(acc, fAllowRowType_scaffold).first;
    }
    // parse protein row id from accession
    // returns 0 if accession is in wrong format
    static
    TVDBRowId ParseProteinRow(CTempString acc) {
        return ParseRowType(acc, fAllowRowType_protein).first;
    }
    SIZE_TYPE GetIdRowDigits(void) const {
        return m_IdRowDigits;
    }

    bool IsTSA(void) const;

    enum EGnlIdFlags {
        fGnlId_NoWGSVersion = 1<<0,
        fGnlId_NoWGSId      = 1<<1,
        fGnlId_Default      = 0
    };
    DECLARE_SAFE_FLAGS_TYPE(EGnlIdFlags, TGnlIdFlags);
    
    CRef<CSeq_id> GetGeneralSeq_id(CTempString prefix,
                                   CTempString tag) const;
    CRef<CSeq_id> GetGeneralSeq_id(CTempString tag,
                                   TGnlIdFlags gnl_id_flags = fGnlId_Default) const;
    CRef<CSeq_id> GetPatentSeq_id(int id) const;

    CRef<CSeq_id> GetGeneralOrPatentSeq_id(CTempString str, TVDBRowId row,
                                           TGnlIdFlags gnl_id_flags = fGnlId_Default) const;
    CRef<CSeq_id> GetAccSeq_id(CTempString acc,
                               int version) const;
    CRef<CSeq_id> GetAccSeq_id(ERowType type,
                               TVDBRowId row_id,
                               int version) const;
    CRef<CSeq_id> GetMasterSeq_id(void) const;
    TGi GetMasterGi(void) const;
    CRef<CSeq_id> GetContigSeq_id(TVDBRowId row_id) const;
    CRef<CSeq_id> GetScaffoldSeq_id(TVDBRowId row_id) const;
    CRef<CSeq_id> GetProteinSeq_id(TVDBRowId row_id) const;

    CSeq_inst::TMol GetContigMolType(void) const;
    CSeq_inst::TMol GetScaffoldMolType(void) const;
    CSeq_inst::TMol GetProteinMolType(void) const;

    CRef<CSeq_entry> GetMasterSeq_entry(void) const;

    typedef list< CRef<CSeqdesc> > TMasterDescr;

    bool IsSetMasterDescr(void) const {
        return m_IsSetMasterDescr;
    }
    const TMasterDescr& GetMasterDescr(void) const {
        return m_MasterDescr;
    }

    typedef CSimpleBufferT<char> TMasterDescrBytes;
    // return size of the master Seq-entry data (ASN.1 binary)
    // the size is also available as buffer.size()
    size_t GetMasterDescrBytes(TMasterDescrBytes& buffer);
    // return entry or null if absent
    CRef<CSeq_entry> GetMasterDescrEntry(void);
    void AddMasterDescr(CSeq_descr& descr, const CBioseq* main_seq = 0) const;

    void ResetMasterDescr(void);
    void SetMasterDescr(const TMasterDescr& descr, int filter);
    bool LoadMasterDescr(int filter);

    void SetPatentId(CRef<CSeq_id> id);
    bool HasPatentId() const
        {
            return m_PatentId;
        }
    const CRef<CSeq_id>& GetPatentId(void) const
        {
            return m_PatentId;
        }

    // get GI range of nucleotide sequences
    pair<TGi, TGi> GetNucGiRange(void);
    // get GI range of proteine sequences
    pair<TGi, TGi> GetProtGiRange(void);
    // get row_id for a given GI or 0 if there is no GI
    // the second value in returned value is true if the sequence is protein
    pair<TVDBRowId, bool> GetGiRowId(TGi gi);
    // get nucleotide row_id (SEQUENCE) for a given GI or 0 if there is no GI
    TVDBRowId GetNucGiRowId(TGi gi);
    // get protein row_id (PROTEIN) for a given GI or 0 if there is no GI
    TVDBRowId GetProtGiRowId(TGi gi);

    // get contig row_id (SEQUENCE) for contig name or 0 if there is none
    TVDBRowId GetContigNameRowId(const string& name);
    // get scaffold row_id (SCAFFOLD) for scaffold name or 0 if there is none
    TVDBRowId GetScaffoldNameRowId(const string& name);
    // get protein row_id (PROTEIN) for protein name or 0 if there is none
    TVDBRowId GetProteinNameRowId(const string& name);
    // get protein row_id (PROTEIN) for product name or 0 if there is none
    TVDBRowId GetProductNameRowId(const string& name);
    // get protein row_id (PROTEIN) for GB accession or 0 if there is no acc
    TVDBRowId GetProtAccRowId(const string& acc);

    typedef COpenRange<TIntId> TGiRange;
    typedef vector<TGiRange> TGiRanges;
    // return sorted non-overlapping ranges of nucleotide GIs in the VDB
    TGiRanges GetNucGiRanges(void);
    // return sorted non-overlapping ranges of protein GIs in the VDB
    TGiRanges GetProtGiRanges(void);

    struct NCBI_SRAREAD_EXPORT SProtAccInfo
    {
        SProtAccInfo(void)
            : m_IdLength(0)
            {
            }
        SProtAccInfo(CTempString acc, Uint4& id);
        
        string GetAcc(Uint4 id) const;

        DECLARE_OPERATOR_BOOL(m_IdLength != 0);

        bool operator<(const SProtAccInfo& b) const {
            if ( m_IdLength != b.m_IdLength ) {
                return m_IdLength < b.m_IdLength;
            }
            return m_AccPrefix < b.m_AccPrefix;
        }

        bool operator==(const SProtAccInfo& b) const {
            return m_IdLength == b.m_IdLength &&
                m_AccPrefix == b.m_AccPrefix;
        }
        bool operator!=(const SProtAccInfo& b) const {
            return !(*this == b);
        }

        string m_AccPrefix;
        Uint4  m_IdLength;
    };

    typedef COpenRange<Uint4> TIdRange;
    typedef map<SProtAccInfo, TIdRange> TProtAccRanges;
    // return map of 3+5 accession ranges
    // Key of each element is accession prefix/length pair
    TProtAccRanges GetProtAccRanges(void);

    EFeatLocIdType GetFeatLocIdType();

    NCBI_gb_state GetProjectGBState() const {
        return m_ProjectGBState;
    }
    const string& GetReplacedBy() const {
        return m_ReplacedBy;
    }
    bool IsReplaced() const {
        return !m_ReplacedBy.empty();
    }

protected:
    friend class CWGSSeqIterator;
    friend class CWGSScaffoldIterator;
    friend class CWGSGiIterator;
    friend class CWGSProteinIterator;
    friend class CWGSFeatureIterator;

    // SSeq0TableCursor is helper accessor structure for SEQUENCE table, minimal columns
    struct SSeq0TableCursor;
    // SSeqTableCursor is helper accessor structure for SEQUENCE table, remaining columns
    struct SSeqTableCursor;
    // SScfTableCursor is helper accessor structure for SCAFFOLD table
    struct SScfTableCursor;
    // SProt0TableCursor is helper accessor structure for optional PROTEIN table, minimal columns
    struct SProt0TableCursor;
    // SProtTableCursor is helper accessor structure for optional PROTEIN table, remaining columns
    struct SProtTableCursor;
    // SFeatTableCursor is helper accessor structure for optional FEATURE table
    struct SFeatTableCursor;
    // SIdsTableCursor is helper accessor structure for SEQUENCE table
    struct SIdxTableCursor;

    const CVDBTable& SeqTable(void) {
        return m_SeqTable;
    }
    const CVDBTable& ScfTable(void) {
        if ( !m_ScfTableIsOpened ) {
            OpenScfTable();
        }
        return m_ScfTable;
    }
    const CVDBTable& ProtTable(void) {
        if ( !m_ProtTableIsOpened ) {
            OpenProtTable();
        }
        return m_ProtTable;
    }
    const CVDBTable& FeatTable(void) {
        if ( !m_FeatTableIsOpened ) {
            OpenFeatTable();
        }
        return m_FeatTable;
    }
    const CVDBTable& GiIdxTable(void) {
        if ( !m_GiIdxTableIsOpened ) {
            OpenGiIdxTable();
        }
        return m_GiIdxTable;
    }
    const CVDBTableIndex& ProtAccIndex(void) {
        if ( !m_ProtAccIndexIsOpened ) {
            OpenProtAccIndex();
        }
        return m_ProtAccIndex;
    }
    const CVDBTableIndex& ContigNameIndex(void) {
        if ( !m_ContigNameIndexIsOpened ) {
            OpenContigNameIndex();
        }
        return m_ContigNameIndex;
    }
    const CVDBTableIndex& ScaffoldNameIndex(void) {
        if ( !m_ScaffoldNameIndexIsOpened ) {
            OpenScaffoldNameIndex();
        }
        return m_ScaffoldNameIndex;
    }
    const CVDBTableIndex& ProteinNameIndex(void) {
        if ( !m_ProteinNameIndexIsOpened ) {
            OpenProteinNameIndex();
        }
        return m_ProteinNameIndex;
    }
    const CVDBTableIndex& ProductNameIndex(void) {
        if ( !m_ProductNameIndexIsOpened ) {
            OpenProductNameIndex();
        }
        return m_ProductNameIndex;
    }

    // get table accessor object for exclusive access
    CRef<SSeq0TableCursor> Seq0(TVDBRowId row = 0);
    CRef<SSeqTableCursor> Seq(TVDBRowId row = 0);
    CRef<SScfTableCursor> Scf(TVDBRowId row = 0);
    CRef<SProt0TableCursor> Prot0(TVDBRowId row = 0);
    CRef<SProtTableCursor> Prot(TVDBRowId row = 0);
    CRef<SFeatTableCursor> Feat(TVDBRowId row = 0);
    CRef<SIdxTableCursor> Idx(TVDBRowId row = 0);
    // return table accessor object for reuse
    void Put(CRef<SSeq0TableCursor>& curs, TVDBRowId row = 0);
    void Put(CRef<SSeqTableCursor>& curs, TVDBRowId row = 0);
    void Put(CRef<SScfTableCursor>& curs, TVDBRowId row = 0);
    void Put(CRef<SProt0TableCursor>& curs, TVDBRowId row = 0);
    void Put(CRef<SProtTableCursor>& curs, TVDBRowId row = 0);
    void Put(CRef<SFeatTableCursor>& curs, TVDBRowId row = 0);
    void Put(CRef<SIdxTableCursor>& curs, TVDBRowId row = 0);

    CRef<CSeq_id> GetGeneralOrPatentSeq_id(CTempString str,
                                           const SSeq0TableCursor& cur,
                                           TVDBRowId row) const;
    CRef<CSeq_id> GetGeneralOrPatentSeq_id(CTempString str,
                                           const SScfTableCursor& cur,
                                           TVDBRowId row) const;
    CRef<CSeq_id> GetGeneralOrPatentSeq_id(CTempString str,
                                           const SProt0TableCursor& cur,
                                           TVDBRowId row) const;

protected:
    // open tables
    void OpenTable(CVDBTable& table,
                   volatile bool& table_is_opened,
                   const char* table_name);
    void OpenIndex(const CVDBTable& table,
                   CVDBTableIndex& index,
                   volatile Int1& index_is_opened,
                   const char* index_name,
                   const char* backup_index_name = 0);

    void OpenScfTable(void);
    void OpenProtTable(void);
    void OpenFeatTable(void);
    void OpenGiIdxTable(void);
    void OpenProtAccIndex(void);
    void OpenContigNameIndex(void);
    void OpenScaffoldNameIndex(void);
    void OpenProteinNameIndex(void);
    void OpenProductNameIndex(void);

    TVDBRowId Lookup(const string& name,
                     const CVDBTableIndex& index,
                     bool upcase);

    void x_InitIdParams(void);
    void x_LoadMasterDescr(int filter);

    void x_SortGiRanges(TGiRanges& ranges);

private:
    CVDBMgr m_Mgr;
    string m_WGSPath;
    CVDB m_Db;
    CVDBTable m_SeqTable;
    string m_IdPrefixWithVersion;
    string m_IdPrefix;
    string m_IdPrefixDbWithVersion;
    string m_IdPrefixDb;
    int m_IdVersion;
    SIZE_TYPE m_IdRowDigits;

    CFastMutex m_TableMutex;
    volatile bool m_ScfTableIsOpened;
    volatile bool m_ProtTableIsOpened;
    volatile bool m_FeatTableIsOpened;
    volatile bool m_GiIdxTableIsOpened;
    volatile Int1 m_ProtAccIndexIsOpened;
    volatile Int1 m_ContigNameIndexIsOpened;
    volatile Int1 m_ScaffoldNameIndexIsOpened;
    volatile Int1 m_ProteinNameIndexIsOpened;
    volatile Int1 m_ProductNameIndexIsOpened;
    CVDBTable m_ScfTable;
    CVDBTable m_ProtTable;
    CVDBTable m_FeatTable;
    CVDBTable m_GiIdxTable;

    CVDBObjectCache<SSeq0TableCursor> m_Seq0;
    CVDBObjectCache<SSeqTableCursor> m_Seq;
    CVDBObjectCache<SScfTableCursor> m_Scf;
    CVDBObjectCache<SProt0TableCursor> m_Prot0;
    CVDBObjectCache<SProtTableCursor> m_Prot;
    CVDBObjectCache<SFeatTableCursor> m_Feat;
    CVDBObjectCache<SIdxTableCursor> m_GiIdx;
    CVDBTableIndex m_ProtAccIndex;
    CVDBTableIndex m_ContigNameIndex;
    CVDBTableIndex m_ScaffoldNameIndex;
    CVDBTableIndex m_ProteinNameIndex;
    CVDBTableIndex m_ProductNameIndex;

    CSeq_inst::TMol m_ContigMolType;
    bool m_IsSetMasterDescr;
    bool m_HasNoDefaultGnlId;
    EFeatLocIdType m_FeatLocIdType;
    CRef<CSeq_entry> m_MasterEntry;
    TMasterDescr m_MasterDescr;
    CRef<CSeq_id> m_PatentId;
    NCBI_gb_state m_ProjectGBState;
    CSeq_id::E_Choice m_SeqIdType;
    string m_ReplacedBy;
};
DECLARE_SAFE_FLAGS(CWGSDb_Impl::EGnlIdFlags);
DECLARE_SAFE_FLAGS(CWGSDb_Impl::EAllowRowType);


class CWGSDb : public CRef<CWGSDb_Impl>
{
public:
    CWGSDb(void)
        {
        }
    explicit CWGSDb(CWGSDb_Impl* impl)
        : CRef<CWGSDb_Impl>(impl)
        {
        }
    CWGSDb(CVDBMgr& mgr,
           CTempString path_or_acc,
           CTempString vol_path = CTempString())
        : CRef<CWGSDb_Impl>(new CWGSDb_Impl(mgr, path_or_acc, vol_path))
        {
        }

    const string& GetWGSPath(void) const {
        return GetObject().GetWGSPath();
    }

    // parse row id from accession
    // returns 0 if accession is in wrong format
    // if is_scaffold flag pointer is not null, then scaffold ids are also
    // accepted and the flag is set appropriately
    NCBI_DEPRECATED
    TVDBRowId ParseRow(CTempString acc, bool* is_scaffold = NULL) const {
        return GetObject().ParseRow(acc, is_scaffold);
    }
    // parse contig row id from accession
    // returns 0 if accession is in wrong format
    static
    TVDBRowId ParseContigRow(CTempString acc) {
        return CWGSDb_Impl::ParseContigRow(acc);
    }
    // parse scaffold row id from accession
    // returns 0 if accession is in wrong format
    static
    TVDBRowId ParseScaffoldRow(CTempString acc) {
        return CWGSDb_Impl::ParseScaffoldRow(acc);
    }
    // parse protein row id from accession
    // returns 0 if accession is in wrong format
    static
    TVDBRowId ParseProteinRow(CTempString acc) {
        return CWGSDb_Impl::ParseProteinRow(acc);
    }

    // get GI range of nucleotide sequences
    pair<TGi, TGi> GetNucGiRange(void) const {
        return GetNCObject().GetNucGiRange();
    }
    // get GI range of proteine sequences
    pair<TGi, TGi> GetProtGiRange(void) const {
        return GetNCObject().GetProtGiRange();
    }
    typedef CWGSDb_Impl::TGiRange TGiRange;
    typedef CWGSDb_Impl::TGiRanges TGiRanges;
    // return sorted non-overlapping ranges of nucleotide GIs in the VDB
    TGiRanges GetNucGiRanges(void) const {
        return GetNCObject().GetNucGiRanges();
    }
    // return sorted non-overlapping ranges of protein GIs in the VDB
    TGiRanges GetProtGiRanges(void) {
        return GetNCObject().GetProtGiRanges();
    }

    typedef CWGSDb_Impl::SProtAccInfo SProtAccInfo;
    typedef CWGSDb_Impl::TIdRange TIdRange;
    typedef CWGSDb_Impl::TProtAccRanges TProtAccRanges;
    // return map of 3+5 accession ranges
    // Key of each element is accession pattern, digital part zeroed.
    TProtAccRanges GetProtAccRanges(void) {
        return GetNCObject().GetProtAccRanges();
    }

    // get row_id for a given GI or 0 if there is no GI
    // the second value in returned value is true if the sequence is protein
    pair<TVDBRowId, bool> GetGiRowId(TGi gi) const {
        return GetNCObject().GetGiRowId(gi);
    }
    // get nucleotide row_id (SEQUENCE) for a given GI or 0 if there is no GI
    TVDBRowId GetNucGiRowId(TGi gi) const {
        return GetNCObject().GetNucGiRowId(gi);
    }
    // get protein row_id (PROTEIN) for a given GI or 0 if there is no GI
    TVDBRowId GetProtGiRowId(TGi gi) const {
        return GetNCObject().GetProtGiRowId(gi);
    }

    // get nucleotide row_id (SEQUENCE) for a given contig name or 0 if
    // name not found.
    TVDBRowId GetContigNameRowId(const string& name) const {
        return GetNCObject().GetContigNameRowId(name);
    }

    // get scaffold row_id (SCAFFOLD) for a given scaffold name or 0 if
    // name not found.
    TVDBRowId GetScaffoldNameRowId(const string& name) const {
        return GetNCObject().GetScaffoldNameRowId(name);
    }

    // get protein row_id (PROTEIN) for a protein name or 0 if
    // name not found.
    TVDBRowId GetProteinNameRowId(const string& name) const {
        return GetNCObject().GetProteinNameRowId(name);
    }

    // get protein row_id (PROTEIN) for a product name or 0 if
    // name not found.
    TVDBRowId GetProductNameRowId(const string& name) const {
        return GetNCObject().GetProductNameRowId(name);
    }

    // get protein row_id (PROTEIN) for GB accession or 0 if there is no acc
    TVDBRowId GetProtAccRowId(const string& acc) const {
        return GetNCObject().GetProtAccRowId(acc);
    }

    enum EDescrFilter {
        eDescrNoFilter,
        eDescrDefaultFilter
    };
    // load master descriptors from VDB metadata (if any)
    // doesn't try to load if master descriptors already set
    // returns true if descriptors are set at the end, or false if not
    bool LoadMasterDescr(EDescrFilter filter = eDescrDefaultFilter) const {
        return GetNCObject().LoadMasterDescr(filter);
    }
    // set master descriptors
    typedef list< CRef<CSeqdesc> > TMasterDescr;
    void SetMasterDescr(const TMasterDescr& descr,
                        EDescrFilter filter = eDescrDefaultFilter) const {
        GetNCObject().SetMasterDescr(descr, filter);
    }
    enum EDescrType {
        eDescr_skip,
        eDescr_default,
        eDescr_force
    };
    // return type of master descriptor propagation
    static EDescrType GetMasterDescrType(const CSeqdesc& desc);
};


class NCBI_SRAREAD_EXPORT CWGSSeqIterator : public SWGSDb_Defs
{
public:
    enum EWithdrawn {
        eExcludeWithdrawn,
        eIncludeWithdrawn
    };

    enum EClipType {
        eDefaultClip,  // as defined by config
        eNoClip,       // force no clipping
        eClipByQuality // force clipping
    };
    enum EIncludeFlags {
        fIncludeLive       = 1 << NCBI_gb_state_eWGSGenBankLive,
        fIncludeSuppressed = 1 << NCBI_gb_state_eWGSGenBankSuppressed,
        fIncludeReplaced   = 1 << NCBI_gb_state_eWGSGenBankReplaced,
        fIncludeWithdrawn  = 1 << NCBI_gb_state_eWGSGenBankWithdrawn,
        fIncludeUnverified = 1 << NCBI_gb_state_eWGSGenBankUnverified,
        fIncludeAll        = 0xff,
        fIncludeDefault    = fIncludeLive | fIncludeUnverified
    };
    DECLARE_SAFE_FLAGS_TYPE(EIncludeFlags, TIncludeFlags);
    
    CWGSSeqIterator(void);
    // TIncludeFlags versions
    explicit
    CWGSSeqIterator(const CWGSDb& wgs_db,
                    EIncludeFlags flags = fIncludeDefault,
                    EClipType clip_type = eDefaultClip);
    CWGSSeqIterator(const CWGSDb& wgs_db,
                    TVDBRowId row,
                    EIncludeFlags flags = fIncludeDefault,
                    EClipType clip_type = eDefaultClip);
    CWGSSeqIterator(const CWGSDb& wgs_db,
                    TVDBRowId first_row, TVDBRowId last_row,
                    EIncludeFlags flags = fIncludeDefault,
                    EClipType clip_type = eDefaultClip);
    CWGSSeqIterator(const CWGSDb& wgs_db,
                    CTempString acc,
                    EIncludeFlags flags = fIncludeDefault,
                    EClipType clip_type = eDefaultClip);
    CWGSSeqIterator(const CWGSDb& wgs_db,
                    TIncludeFlags flags,
                    EClipType clip_type = eDefaultClip);
    CWGSSeqIterator(const CWGSDb& wgs_db,
                    TVDBRowId row,
                    TIncludeFlags flags,
                    EClipType clip_type = eDefaultClip);
    CWGSSeqIterator(const CWGSDb& wgs_db,
                    TVDBRowId first_row, TVDBRowId last_row,
                    TIncludeFlags flags,
                    EClipType clip_type = eDefaultClip);
    CWGSSeqIterator(const CWGSDb& wgs_db,
                    CTempString acc,
                    TIncludeFlags flags,
                    EClipType clip_type = eDefaultClip);
    // EWithdrawn versions (deprecated)
    NCBI_DEPRECATED
    CWGSSeqIterator(const CWGSDb& wgs_db,
                    EWithdrawn withdrawn,
                    EClipType clip_type = eDefaultClip);
    NCBI_DEPRECATED
    CWGSSeqIterator(const CWGSDb& wgs_db,
                    TVDBRowId row,
                    EWithdrawn withdrawn,
                    EClipType clip_type = eDefaultClip);
    NCBI_DEPRECATED
    CWGSSeqIterator(const CWGSDb& wgs_db,
                    TVDBRowId first_row, TVDBRowId last_row,
                    EWithdrawn withdrawn,
                    EClipType clip_type = eDefaultClip);
    NCBI_DEPRECATED
    CWGSSeqIterator(const CWGSDb& wgs_db,
                    CTempString acc,
                    EWithdrawn withdrawn,
                    EClipType clip_type = eDefaultClip);
    ~CWGSSeqIterator(void);

    void Reset(void);
    CWGSSeqIterator(const CWGSSeqIterator& iter);
    CWGSSeqIterator& operator=(const CWGSSeqIterator& iter);

    CWGSSeqIterator& SelectRow(TVDBRowId row);

    DECLARE_OPERATOR_BOOL(m_CurrId < m_FirstBadId);

    CWGSSeqIterator& operator++(void)
        {
            x_CheckValid("CWGSSeqIterator::operator++");
            ++m_CurrId;
            x_Settle();
            return *this;
        }

    TVDBRowId GetCurrentRowId(void) const {
        return m_CurrId;
    }
    TVDBRowId GetFirstGoodRowId(void) const {
        return m_FirstGoodId;
    }
    TVDBRowId GetFirstBadRowId(void) const {
        return m_FirstBadId;
    }
    TVDBRowId GetLastRowId(void) const {
        return GetFirstBadRowId() - 1;
    }
    TVDBRowId GetRemainingCount(void) const {
        return GetFirstBadRowId() - GetCurrentRowId();
    }
    TVDBRowId GetSize(void) const {
        return GetFirstBadRowId() - GetFirstGoodRowId();
    }

    bool HasGi(void) const;
    CSeq_id::TGi GetGi(void) const;
    CTempString GetAccession(void) const;

    int GetLatestAccVersion(void) const;
    unsigned GetAccVersionCount(void) const;
    bool HasAccVersion(int version) const;

    int GetAccVersion(void) const {
        return GetLatestAccVersion() + m_AccVersion.m_Offset;
    }

    // default SVersion object means latest version
    // offset is negative, -1 means prevous version, -2 is secont previous...
    enum ELatest {
        eLatest
    };
    struct SVersionSelector {
        SVersionSelector(ELatest) : m_Offset(0) {}
        int m_Offset;
    };
    
    // if version == -1 select latest version
    void SelectAccVersion(int version);
 
    bool HasTitle(void) const;
    CTempString GetTitle(void) const;

    // return raw trim/clip values
    TSeqPos GetClipQualityLeft(void) const;
    TSeqPos GetClipQualityLength(void) const;
    TSeqPos GetClipQualityRight(void) const
        {
            // inclusive
            return GetClipQualityLeft() + GetClipQualityLength() - 1;
        }

    // Returns true if current read has clipping info that can or does 
    // reduce sequence length.
    bool HasClippingInfo(void) const;
    // Returns true if current read is actually clipped by quality.
    // It can be true only if clipping by quality is on.
    bool IsClippedByQuality(void) const {
        return m_ClipByQuality && HasClippingInfo();
    }
    // Returns true if current read has actual clipping info that is not
    // applied because clipping by quality is off.
    bool ShouldBeClippedByQuality(void) const {
        return !m_ClipByQuality && HasClippingInfo();
    }

    // return clip type
    bool GetClipByQualityFlag(EClipType clip_type = eDefaultClip) const
        {
            return (clip_type == eDefaultClip?
                    m_ClipByQuality:
                    clip_type == eClipByQuality);
        }

    // return raw unclipped sequence length
    TSeqPos GetRawSeqLength(void) const;
    // return clipping start position within raw unclipped sequence
    TSeqPos GetSeqOffset(EClipType clip_type = eDefaultClip) const;
    // return effective sequence length, depending on clip type
    TSeqPos GetSeqLength(EClipType clip_type = eDefaultClip) const;

    // return corresponding kind of Seq-id if exists
    // return null if there is no such Seq-id
    CRef<CSeq_id> GetAccSeq_id(void) const;
    CRef<CSeq_id> GetGiSeq_id(void) const;
    NCBI_DEPRECATED // use GetGeneralOrPatentSeq_id() instead
    CRef<CSeq_id> GetGeneralSeq_id(void) const;
    CRef<CSeq_id> GetGeneralOrPatentSeq_id(void) const;

    //CTempString GetGeneralId(void) const;
    CTempString GetContigName(void) const;

    bool HasTaxId(void) const;
    int GetTaxId(void) const;

    bool HasSeqHash(void) const;
    int GetSeqHash(void) const;

    TVDBRowIdRange GetLocFeatRowIdRange(void) const;

    typedef struct SWGSContigGapInfo {
        size_t gaps_count;
        const INSDC_coord_zero* gaps_start;
        const INSDC_coord_len* gaps_len;
        const NCBI_WGS_component_props* gaps_props;
        const NCBI_WGS_gap_linkage* gaps_linkage;
        SWGSContigGapInfo(void)
            : gaps_count(0),
              gaps_start(0),
              gaps_len(0),
              gaps_props(0),
              gaps_linkage(0)
            {
            }

        DECLARE_OPERATOR_BOOL(gaps_count > 0);
        void operator++(void) {
            _ASSERT(*this);
            --gaps_count;
            ++gaps_start;
            ++gaps_len;
            ++gaps_props;
            if ( gaps_linkage ) {
                ++gaps_linkage;
            }
        }
        TSeqPos GetLength(void) const { return *gaps_len; }
        TSeqPos GetFrom(void) const { return *gaps_start; }
        TSeqPos GetToOpen(void) const { return GetFrom()+GetLength(); }
        TSeqPos GetTo(void) const { return GetToOpen()-1; }

        // prepare iteration starting with pos
        void SetPos(TSeqPos pos);
        // check if pos is in current gap, ++ after use
        bool IsInGap(TSeqPos pos) const { return *this && pos >= GetFrom(); }
        // return intersecting length of current gap
        TSeqPos GetGapLength(TSeqPos pos, TSeqPos len) const {
            return min(len, GetToOpen()-pos);
        }
        // return intersecting length of data before current gap
        TSeqPos GetDataLength(TSeqPos pos, TSeqPos len) const {
            return !*this? len: min(len, GetFrom()-pos);
        }
    } TWGSContigGapInfo;

    bool HasGapInfo(void) const;
    void GetGapInfo(TWGSContigGapInfo& gap_info) const;

    CRef<CSeq_id> GetId(TFlags flags = fDefaultFlags) const;
    void GetIds(CBioseq::TId& ids, TFlags flags = fDefaultFlags) const;

    // Return descr binary byte sequence as is
    bool HasSeqDescrBytes(void) const;
    CTempString GetSeqDescrBytes(void) const;
    // Return descr binary byte sequence for nuc-prot set as is
    bool HasNucProtDescrBytes(void) const;
    CTempString GetNucProtDescrBytes(void) const;
    // return effective descr
    bool HasSeq_descr(TFlags flags = fDefaultFlags) const;
    // Parse the binary byte sequence and instantiate ASN.1 object
    CRef<CSeq_descr> GetSeq_descr(TFlags flags = fDefaultFlags) const;

    bool HasAnnotSet(void) const;
    // Return annot binary byte sequence as is
    CTempString GetAnnotBytes(void) const;
    typedef CBioseq::TAnnot TAnnotSet;
    void GetAnnotSet(TAnnotSet& annot_set, TFlags flags = fDefaultFlags) const;

    // check if the VDB structure allows quality graph in data
    bool CanHaveQualityGraph(void) const;
    // check if this sequence has quality graph
    bool HasQualityGraph(void) const;
    void GetQualityVec(vector<INSDC_quality_phred>& quality_vec) const;
    void GetQualityAnnot(TAnnotSet& annot_set,
                         TFlags flags = fDefaultFlags) const;
    string GetQualityAnnotName(void) const;

    NCBI_gb_state GetGBState(void) const;

    bool HasPublicComment(void) const;
    CTempString GetPublicComment(void) const;

    bool IsCircular(void) const;

    CRef<CSeq_inst> GetSeq_inst(TFlags flags = fDefaultFlags) const;
    CRef<CDelta_ext> GetDelta(TSeqPos pos, TSeqPos len) const;
    CRef<CDelta_ext> GetDelta(TSeqPos pos, TSeqPos len,
                              TWGSContigGapInfo gap_info,
                              vector< COpenRange<TSeqPos> >* split = 0) const;
    CRef<CSeq_data> Get2na(TSeqPos pos, TSeqPos len) const;
    CRef<CSeq_data> Get4na(TSeqPos pos, TSeqPos len) const;

    CRef<CBioseq> GetBioseq(TFlags flags = fDefaultFlags) const;
    // GetSeq_entry may create nuc-prot set if the sequence has products
    CRef<CSeq_entry> GetSeq_entry(TFlags flags = fDefaultFlags) const;
    CRef<CAsnBinData> GetSeq_entryData(TFlags flags = fDefaultFlags) const;
    // GetSplitInfo may create Seq-entry as a skeleton w/o actual splitting
    CRef<CID2S_Split_Info> GetSplitInfo(TFlags flags = fDefaultFlags) const;
    CRef<CAsnBinData> GetSplitInfoData(TFlags flags = fDefaultFlags) const;
    // Make chunk for the above split-info, flags must be the same
    typedef int TChunkId;
    CRef<CID2S_Chunk> GetChunk(TChunkId chunk_id,
                               TFlags flags = fDefaultFlags) const;
    CRef<CAsnBinData> GetChunkData(TChunkId chunk_id,
                                   TFlags flags = fDefaultFlags) const;

    enum EInstSegmentFlags {
        fInst_MakeData  = 1<<0, // generate Seq-data in data segments
        fInst_MakeGaps  = 1<<1, // generate gap segments
        fInst_Split     = 1<<2, // split data by chunk boundaries
        fInst_Minimal   = 1<<3  // minimize number of data segments
    };
    DECLARE_SAFE_FLAGS_TYPE(EInstSegmentFlags, TInstSegmentFlags);
    
protected:
    void x_Select(const CWGSDb& wgs_db,
                  TIncludeFlags include_flags,
                  EClipType clip_type);
    void x_Select(const CWGSDb& wgs_db,
                  TIncludeFlags include_flags,
                  EClipType clip_type,
                  TVDBRowId get_row);
    void x_Select(const CWGSDb& wgs_db,
                  TIncludeFlags include_flags,
                  EClipType clip_type,
                  TVDBRowId first_row,
                  TVDBRowId last_row);
    void x_Select(const CWGSDb& wgs_db,
                  TIncludeFlags include_flags,
                  EClipType clip_type,
                  CTempString acc);

    void x_Init(const CWGSDb& wgs_db,
                TIncludeFlags include_flags,
                EClipType clip_type,
                TVDBRowId get_row);

    CWGSDb_Impl& GetDb(void) const {
        return m_Db.GetNCObject();
    }
    
    void x_Settle(void);
    bool x_Excluded(void) const;

    void x_ReportInvalid(const char* method) const;
    void x_CheckValid(const char* method) const {
        if ( !*this ) {
            x_ReportInvalid(method);
        }
    }

    // if version == -1 return latest version
    SVersionSelector x_GetAccVersionSelector(int version) const;

    void x_CreateEntry(SWGSCreateInfo& info) const;
    void x_CreateBioseq(SWGSCreateInfo& info) const;
    bool x_InitSplit(SWGSCreateInfo& info) const;
    void x_CreateSplit(SWGSCreateInfo& info) const;
    void x_CreateChunk(SWGSCreateInfo& info,
                       TChunkId chunk_id) const;

    void x_CreateDataChunk(SWGSCreateInfo& info,
                           unsigned index) const;
    void x_CreateQualityChunk(SWGSCreateInfo& info,
                              unsigned index) const;
    void x_CreateProductsChunk(SWGSCreateInfo& info,
                               unsigned index) const;
    void x_CreateFeaturesChunk(SWGSCreateInfo& info,
                               unsigned index) const;

    TSeqPos x_GetQualityArraySize(void) const;
    void x_AddQualityChunkInfo(SWGSCreateInfo& info) const;
    void x_GetQualityAnnot(TAnnotSet& annot_set,
                           SWGSCreateInfo& info,
                           TSeqPos pos = 0,
                           TSeqPos len = kInvalidSeqPos) const;

    const Uint1* x_GetUnpacked4na(TSeqPos pos, TSeqPos len) const;

    // methods to be used if no gap information exist
    TSeqPos x_GetGapLengthExact(TSeqPos pos, TSeqPos len) const;
    TSeqPos x_Get2naLengthExact(TSeqPos pos, TSeqPos len) const;
    TSeqPos x_Get4naLengthExact(TSeqPos pos, TSeqPos len,
                                TSeqPos stop_2na_len,
                                TSeqPos stop_gap_len) const;

    // methods to be used with gap information
    bool x_AmbiguousBlock(TSeqPos block_index) const;
    TSeqPos x_Get2naLength(TSeqPos pos, TSeqPos len) const;
    TSeqPos x_Get4naLength(TSeqPos pos, TSeqPos len) const;

    struct SSegment {
        COpenRange<TSeqPos> range;
        bool is_gap;
        CRef<CSeq_literal> literal;
    };
    typedef vector<SSegment> TSegments;

    COpenRange<TSeqPos> x_NormalizeSeqRange(COpenRange<TSeqPos> range) const;
    void x_AddGap(TSegments& segments,
                  TSeqPos pos, TSeqPos len,
                  const TWGSContigGapInfo& gap_info) const;
    void x_SetDelta(CSeq_inst& inst, const TSegments& segments) const;
    void x_SetDeltaOrData(CSeq_inst& inst, const TSegments& segments) const;

    void x_GetSegments(TSegments& data,
                       COpenRange<TSeqPos> range,
                       TWGSContigGapInfo gap_info,
                       TInstSegmentFlags flags) const;
    void x_GetSegments(TSegments& segments,
                       COpenRange<TSeqPos> range) const;

    CRef<CSeq_inst> x_GetSeq_inst(SWGSCreateInfo& info) const;

private:
    CWGSDb m_Db;
    CRef<CWGSDb_Impl::SSeq0TableCursor> m_Cur0; // VDB seq table accessor
    CRef<CWGSDb_Impl::SSeqTableCursor> m_Cur; // VDB seq table accessor
    TVDBRowId m_CurrId, m_FirstGoodId, m_FirstBadId;
    SVersionSelector m_AccVersion;
    TIncludeFlags m_IncludeFlags;
    bool m_ClipByQuality;
};
DECLARE_SAFE_FLAGS(CWGSSeqIterator::EInstSegmentFlags);
DECLARE_SAFE_FLAGS(CWGSSeqIterator::EIncludeFlags);


class NCBI_SRAREAD_EXPORT CWGSScaffoldIterator : public SWGSDb_Defs
{
public:
    CWGSScaffoldIterator(void);
    explicit
    CWGSScaffoldIterator(const CWGSDb& wgs_db);
    CWGSScaffoldIterator(const CWGSDb& wgs_db, TVDBRowId row);
    CWGSScaffoldIterator(const CWGSDb& wgs_db, CTempString acc);
    ~CWGSScaffoldIterator(void);

    void Reset(void);
    CWGSScaffoldIterator(const CWGSScaffoldIterator& iter);
    CWGSScaffoldIterator& operator=(const CWGSScaffoldIterator& iter);

    CWGSScaffoldIterator& SelectRow(TVDBRowId row);

    DECLARE_OPERATOR_BOOL(m_CurrId < m_FirstBadId);

    CWGSScaffoldIterator& operator++(void) {
        ++m_CurrId;
        return *this;
    }

    TVDBRowId GetCurrentRowId(void) const {
        return m_CurrId;
    }
    TVDBRowId GetFirstGoodRowId(void) const {
        return m_FirstGoodId;
    }
    TVDBRowId GetFirstBadRowId(void) const {
        return m_FirstBadId;
    }
    TVDBRowCount GetRemainingCount(void) const {
        return GetFirstBadRowId() - GetCurrentRowId();
    }
    TVDBRowCount GetSize(void) const {
        return GetFirstBadRowId() - GetFirstGoodRowId();
    }

    CTempString GetAccession(void) const;
    int GetAccVersion(void) const;

    CRef<CSeq_id> GetAccSeq_id(void) const;
    CRef<CSeq_id> GetGiSeq_id(void) const;
    NCBI_DEPRECATED // use GetGeneralOrPatentSeq_id() instead
    CRef<CSeq_id> GetGeneralSeq_id(void) const;
    CRef<CSeq_id> GetGeneralOrPatentSeq_id(void) const;

    CTempString GetScaffoldName(void) const;

    TVDBRowIdRange GetLocFeatRowIdRange(void) const;

    NCBI_gb_state GetGBState(void) const;
    
    CRef<CSeq_id> GetId(TFlags flags = fDefaultFlags) const;
    void GetIds(CBioseq::TId& ids, TFlags flags = fDefaultFlags) const;

    bool HasSeq_descr(TFlags flags = fDefaultFlags) const;
    CRef<CSeq_descr> GetSeq_descr(TFlags flags = fDefaultFlags) const;

    TSeqPos GetSeqLength(void) const;

    bool IsCircular(void) const;

    CRef<CSeq_inst> GetSeq_inst(TFlags flags = fDefaultFlags) const;

    CRef<CBioseq> GetBioseq(TFlags flags = fDefaultFlags) const;
    // GetSeq_entry may create nuc-prot set if the sequence has products
    CRef<CSeq_entry> GetSeq_entry(TFlags flags = fDefaultFlags) const;

protected:
    void x_Init(const CWGSDb& wgs_db);

    CWGSDb_Impl& GetDb(void) const {
        return m_Db.GetNCObject();
    }
    
    void x_ReportInvalid(const char* method) const;
    void x_CheckValid(const char* method) const {
        if ( !*this ) {
            x_ReportInvalid(method);
        }
    }

    void x_CreateBioseq(SWGSCreateInfo& info) const;
    void x_CreateEntry(SWGSCreateInfo& info) const;

private:
    CWGSDb m_Db;
    CRef<CWGSDb_Impl::SScfTableCursor> m_Cur; // VDB scaffold table accessor
    TVDBRowId m_CurrId, m_FirstGoodId, m_FirstBadId;
};


class NCBI_SRAREAD_EXPORT CWGSGiIterator
{
public:
    enum ESeqType {
        eNuc  = 1 << 0,
        eProt = 1 << 1,
        eAll  = eNuc | eProt
    };
    CWGSGiIterator(void);
    explicit
    CWGSGiIterator(const CWGSDb& wgs_db, ESeqType seq_type = eAll);
    CWGSGiIterator(const CWGSDb& wgs_db, TGi gi, ESeqType seq_type = eAll);
    ~CWGSGiIterator(void);

    void Reset(void);
    CWGSGiIterator(const CWGSGiIterator& iter);
    CWGSGiIterator& operator=(const CWGSGiIterator& iter);

    DECLARE_OPERATOR_BOOL(m_CurrGi < m_FirstBadGi);

    CWGSGiIterator& operator++(void) {
        ++m_CurrGi;
        x_Settle();
        return *this;
    }

    // get currently selected gi
    TGi GetGi(void) const {
        return m_CurrGi;
    }

    // get currently selected gi type, eNuc or eProt
    ESeqType GetSeqType(void) const {
        return m_CurrSeqType;
    }

    // get currently selected gi row id in corresponding nuc or prot table
    TVDBRowId GetRowId(void) const {
        return m_CurrRowId;
    }

protected:
    void x_Init(const CWGSDb& wgs_db, ESeqType seq_type);

    CWGSDb_Impl& GetDb(void) const {
        return m_Db.GetNCObject();
    }

    void x_Settle(void);
    bool x_Excluded(void);
    
private:
    CWGSDb m_Db;
    CRef<CWGSDb_Impl::SIdxTableCursor> m_Cur; // VDB GI index table accessor
    TGi m_CurrGi, m_FirstBadGi;
    TVDBRowId m_CurrRowId;
    ESeqType m_CurrSeqType, m_FilterSeqType;
};


class NCBI_SRAREAD_EXPORT CWGSProteinIterator : public SWGSDb_Defs
{
public:
    CWGSProteinIterator(void);
    explicit
    CWGSProteinIterator(const CWGSDb& wgs_db);
    CWGSProteinIterator(const CWGSDb& wgs_db, TVDBRowId row);
    CWGSProteinIterator(const CWGSDb& wgs_db, CTempString acc);
    ~CWGSProteinIterator(void);

    void Reset(void);
    CWGSProteinIterator(const CWGSProteinIterator& iter);
    CWGSProteinIterator& operator=(const CWGSProteinIterator& iter);

    DECLARE_OPERATOR_BOOL(m_CurrId < m_FirstBadId);

    CWGSProteinIterator& operator++(void) {
        ++m_CurrId;
        return *this;
    }

    TVDBRowId GetCurrentRowId(void) const {
        return m_CurrId;
    }
    TVDBRowId GetFirstGoodRowId(void) const {
        return m_FirstGoodId;
    }
    TVDBRowId GetFirstBadRowId(void) const {
        return m_FirstBadId;
    }
    TVDBRowCount GetRemainingCount(void) const {
        return GetFirstBadRowId() - GetCurrentRowId();
    }
    TVDBRowCount GetSize(void) const {
        return GetFirstBadRowId() - GetFirstGoodRowId();
    }

    CWGSProteinIterator& SelectRow(TVDBRowId row);

    bool HasGi(void) const;
    CSeq_id::TGi GetGi(void) const;

    CTempString GetAccession(void) const;
    int GetAccVersion(void) const;

    CRef<CSeq_id> GetAccSeq_id(void) const;
    CRef<CSeq_id> GetGiSeq_id(void) const;
    NCBI_DEPRECATED // use GetGeneralOrPatentSeq_id() instead
    CRef<CSeq_id> GetGeneralSeq_id(void) const;
    CRef<CSeq_id> GetGeneralOrPatentSeq_id(void) const;

    CTempString GetProteinName(void) const;
    CTempString GetProductName(void) const;

    TVDBRowIdRange GetLocFeatRowIdRange(void) const;
    size_t GetProductFeatCount(void) const;
    TVDBRowId GetProductFeatRowId(size_t index) const;
    // return best product feature or 0 if none
    TVDBRowId GetBestProductFeatRowId(void) const;
    NCBI_DEPRECATED
    TVDBRowId GetProductFeatRowId(void) const; // single or 0 if none

    TVDBRowId GetReplacedByRowId(void) const;
    TVDBRowId GetReplacesRowId(void) const;

    CRef<CSeq_id> GetId(TFlags flags = fDefaultFlags) const;
    void GetIds(CBioseq::TId& ids, TFlags flags = fDefaultFlags) const;

    // reference protein accession WP_
    bool HasRefAcc(void) const;
    CTempString GetRefAcc(void) const;

    NCBI_gb_state GetGBState(void) const;

    bool HasPublicComment(void) const;
    CTempString GetPublicComment(void) const;

    TSeqPos GetSeqLength(void) const;

    bool HasSeq_descr(TFlags flags = fDefaultFlags) const;
    CRef<CSeq_descr> GetSeq_descr(TFlags flags = fDefaultFlags) const;

    bool HasTitle(void) const;
    CTempString GetTitle(void) const;

    bool HasAnnotSet(void) const;
    typedef CBioseq::TAnnot TAnnotSet;
    void GetAnnotSet(TAnnotSet& annot_set, TFlags flags = fDefaultFlags) const;

    CRef<CSeq_inst> GetSeq_inst(TFlags flags = fDefaultFlags) const;

    CRef<CBioseq> GetBioseq(TFlags flags = fDefaultFlags) const;
    // GetSeq_entry will always return seq entry
    CRef<CSeq_entry> GetSeq_entry(TFlags flags = fDefaultFlags) const;

protected:
    friend struct SWGSCreateInfo;
    friend struct SWGSFeatChunkInfo;

    void x_Init(const CWGSDb& wgs_db);
    void x_Cur() const;

    CWGSDb_Impl& GetDb(void) const {
        return m_Db.GetNCObject();
    }
    
    void x_ReportInvalid(const char* method) const;
    void x_CheckValid(const char* method) const {
        if ( !*this ) {
            x_ReportInvalid(method);
        }
    }

    void x_CreateBioseq(SWGSCreateInfo& info) const;
    void x_CreateEntry(SWGSCreateInfo& info) const;

private:
    CWGSDb m_Db;
    CRef<CWGSDb_Impl::SProt0TableCursor> m_Cur0; // VDB protein table accessor
    CRef<CWGSDb_Impl::SProtTableCursor> m_Cur; // VDB protein table accessor
    TVDBRowId m_CurrId, m_FirstGoodId, m_FirstBadId;
};


class NCBI_SRAREAD_EXPORT CWGSFeatureIterator
{
public:
    CWGSFeatureIterator(void);
    explicit
    CWGSFeatureIterator(const CWGSDb& wgs);
    CWGSFeatureIterator(const CWGSDb& wgs, TVDBRowId row);
    CWGSFeatureIterator(const CWGSDb& wgs, TVDBRowIdRange row_range);
    ~CWGSFeatureIterator(void);

    void Reset(void);
    CWGSFeatureIterator(const CWGSFeatureIterator& iter);
    CWGSFeatureIterator& operator=(const CWGSFeatureIterator& iter);

    DECLARE_OPERATOR_BOOL(m_CurrId < m_FirstBadId);

    CWGSFeatureIterator& operator++(void) {
        ++m_CurrId;
        return *this;
    }

    TVDBRowId GetCurrentRowId(void) const {
        return m_CurrId;
    }
    TVDBRowId GetFirstGoodRowId(void) const {
        return m_FirstGoodId;
    }
    TVDBRowId GetFirstBadRowId(void) const {
        return m_FirstBadId;
    }
    TVDBRowCount GetRemainingCount(void) const {
        return GetFirstBadRowId() - GetCurrentRowId();
    }
    TVDBRowCount GetSize(void) const {
        return GetFirstBadRowId() - GetFirstGoodRowId();
    }

    CWGSFeatureIterator& SelectRow(TVDBRowId row);
    CWGSFeatureIterator& SelectRowRange(TVDBRowIdRange row_range);

    NCBI_WGS_feattype GetFeatType(void) const;
    
    NCBI_WGS_seqtype GetLocSeqType(void) const;
    NCBI_WGS_seqtype GetProductSeqType(void) const;

    TVDBRowId GetLocRowId(void) const;
    TVDBRowId GetProductRowId(void) const;

    TSeqPos GetLocStart(void) const;
    TSeqPos GetLocLength(void) const;
    CRange<TSeqPos> GetLocRange(void) const;

    CTempString GetSeq_featBytes(void) const;
    CRef<CSeq_feat> GetSeq_feat() const;

protected:
    CWGSDb_Impl& GetDb(void) const {
        return m_Db.GetNCObject();
    }
    
    void x_Init(const CWGSDb& wgs_db);

    void x_ReportInvalid(const char* method) const;
    void x_CheckValid(const char* method) const {
        if ( !*this ) {
            x_ReportInvalid(method);
        }
    }

private:
    CWGSDb m_Db;
    CRef<CWGSDb_Impl::SFeatTableCursor> m_Cur; // VDB feature table accessor
    TVDBRowId m_CurrId, m_FirstGoodId, m_FirstBadId;
};


END_NAMESPACE(objects);
END_NCBI_NAMESPACE;

#endif // SRA__READER__SRA__WGSREAD__HPP
