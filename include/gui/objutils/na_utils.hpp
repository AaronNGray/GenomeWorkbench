#ifndef GUI_OBJUTILS___NA_UTILS__HPP
#define GUI_OBJUTILS___NA_UTILS__HPP

/*  $Id: na_utils.hpp 38366 2017-04-28 17:06:14Z rudnev $
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
 * Authors:  Liangshou Wu, Dmitry Rudnev
 *
 * File Description:
 *    Name annotation utility class.
 *
 */

#include <gui/objutils/annot_info.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/objutils/utils.hpp>

/** @addtogroup GUI_OBJUTILS
 *
 * @{
 */


BEGIN_NCBI_SCOPE

class ICanceled;

BEGIN_SCOPE(objects)
    class CScope;
    class CSeq_id;
    class CSeq_align;
    class CSeq_annot;
    class CSeq_annot_Handle;
END_SCOPE(objects)



///////////////////////////////////////////////////////////////////////////////
/// Utilities for data/track type discovery
class NCBI_GUIOBJUTILS_EXPORT CDataTrackUtils : public CObject
{
public:
    /// Get alignment type.
    /// The alignment types include:
    ///   - nucleotide: nucleotide-to-nucleotide alignment
    ///   - protein:    protein-to-protein alignment
    ///   - mixed:      protein-to-nucleotide alignment
    static string GetAlignType(const objects::CSeq_align& align,
                               objects::CScope& scope);

    /// Collect unique list of feature subtypes for all features in a seq-annot.
    static void GetFeatSubtypes(const objects::CSeq_annot& annot,
                                set<string>& subtypes);

    /// Does feature belong to gene model track
    /// Gene, RNA, cdregion, exon, C_region, and VDJ segments
    static bool IsGeneModelFeature(int type, int subtype);

    /// Convert feature subtypes to track subtypes.
    /// If the feature subtype list contains 'gene' feature,
    /// all gene, RNA, cdregion and exon feature subtypes will
    /// be combined into one track subtype: gene_model. If
    /// the feature type is variation_ref, the track type will
    /// be dbVar. If the feature tyep is variation, the track
    /// type will be dbSNP. For anything else, track type
    /// will be just treated as a feature track subtype.
    static void FeatSubtypes2TrackSubtypes(set<string>& subtypes,
                                           const set<string>& feat_subtypes);

    /// Collect all column header names for a given seq-table.
    /// The header name is from either field-name or field-id.
    static void GetColumnHeader(set<string>& headers,
                                const objects::CSeq_annot& annot);

    /// Guess the track subtype based on seq-table headers.
    static string GetSeqTableSubtype(const set<string>& headers);

    /// Get data track information from a seq-annot.
    static CTrackInfo::TTrackInfoList
        GetTrackInfo(objects::CSeq_annot_Handle annot_handle);


    static CTrackInfo::TTrackInfoList
        GetTrackInfo(const objects::CSeq_annot& annot, objects::CScope& scope);

    /// Map data type to track type.
    static void DataType2TrackType(const string& annot_type,
        const string& subtype,
        string& track_key,
        string& subkey);
};

class NCBI_GUIOBJUTILS_EXPORT CNAUtils
{
public:
    /// typedefs for NAA meta-data
    typedef string TNAA;
    typedef list<TNAA> TNAAs;
    typedef map<string, CRef<CAnnotMetaData> > TNAMetaDataSet;
    typedef CTrackInfo::TTrackInfoList TTrackInfoList;

    /// eutils databases that can be used
    enum EEntrezDB {
        EEDB_Undef,
        EEDB_Nucleotide,
        EEDB_Protein,
        EEDB_All        ///< try both nucleotide and protein databases, merge results
    };
    /// source of obtained NA metadata
    /// were internal caches hit, partially hit or totally missed during some cached operations
    enum EMetaDataSource {
        EMetaDataSource_Undef,
        EMetaDataSource_Cache,
        EMetaDataSource_Mixed,
        EMetaDataSource_Eutils,
        EMetaDataSource_EmptyRequest,
        EMetaDataSource_EmptyAfterFilteringRequest
    };

    enum EUidsSource {
        EUidsSource_Undef,
        EUidsSource_Cache,
        EUidsSource_Mixed,
        EUidsSource_Eutils,
        EUidsSource_EmptyRequest
    };

    /// Default ctor.
    /// No Sequence id provided. That means the target named
    /// annotations won't be filtered by any sequence id.
    CNAUtils();

    /// Ctor with target sequence id.
    /// Only the named annotations associated with the target
    /// sequence will be returned. However, the target sequence
    /// is ignored if the requested NAAs are explicitly listed.
    CNAUtils(const objects::CSeq_id& id);

    /// Ctor with target sequence id and a scope.
    /// @sa CNAUtils(const objects::CSeq_id& id).
    CNAUtils(const objects::CSeq_id& id, objects::CScope& scope);

    /// Get a list of NAAs associated with the target sequence.
    /// It returns only the named annotation names only (no meta-data).
    /// If the target sequence is not set, it returns nothing.
    /// @param naas data structure for returned NA names.
    /// @param context is a viewer context that predefines a set
    ///        of NAAs applied only under the given viewer context.
    ///        Maybe, we can remove it when TMS is fully integrated.
    void GetAllNAAs(TNAAs& naas, const string& context = "") const;

    /// get all GIs for a given NA accession
    typedef set<TGi> TGis;
    static void GetAllGIs(const TNAA& naa, EEntrezDB eedb, TGis& gis,
                       EUidsSource* pUidsSource = NULL);

    /// Get meta-data for a specific NA.
    /// The meta-data include both the core information from NA
    /// meta-data and Entrez links.
    /// @param na the target named annotation.
    /// @filtering determines if results are filtered against
    ///            on the target sequence
    void GetNAMetaData(TNAMetaDataSet& md_set,
                       const string& na,
                       bool filtering = false,
                       EMetaDataSource* pMDSource = NULL,
                       EUidsSource* pUidsSource = NULL,
                       bool isGetLinks = true) const;

    /// Get meta-data for a list of NAAs.
    /// The meta-data include both the core informaton from NA
    /// meta-data and Entrez links.
    /// @param naas the target input NAA list
    /// @param md_set data structure for returned na meta-data
    /// @filtering determines if results are filtered against
    ///            on the target seqquience
    void GetNAMetaData(TNAMetaDataSet& md_set,
                       const TNAAs& naas,
                       bool filtering = false,
                       EMetaDataSource* pMDSource = NULL,
                       EUidsSource* pUidsSource = NULL,
                       bool isGetLinks = true) const;

    /// Get meta-data for a given NAA associated with seq_id.
    /// Links stored in NA meta-data will be included, but links
    /// from Entez are not.
    /// @param md_set data structure for returned na meta-data
    /// @param context is a viewer context that predefines a set
    ///        of NAAs applied only under the given viewer context.
    ///        Maybe, we can remove it when TMS is fully integrated.
    void GetAllNAMetaData(TNAMetaDataSet& md_set,
                          const string& context = "",
                          ICanceled* canceledCallback = 0) const;

    /// Do data track discovery for a given NA.
    /// This method relies on NA meta-data to do data discovery
    /// completely or partially depending on the annotation type.
    /// For example, for seq-table, it will need to look at the data
    /// to report the column names. For alignment, it will need to
    /// check the alignment type: protein or nucleotide.
    void GetNATrackInfo(TTrackInfoList& track_info, const string& naa) const;

    /// Do data track discovery for a given NA with meta-data.
    /// @sa void GetNATrackInfo(TTrackInfoList& track_info, const string& naa) const;
    void GetNATrackInfo(TTrackInfoList& track_info,
        const string& naa, const TNAMetaDataSet& md_set) const;

    /// Do data track discovery for a list of NAs.
    /// @sa void GetNATrackInfo(TTrackInfoList& track_info, const string& naa) const;
    void GetNATrackInfo(TTrackInfoList& track_info,
        const TNAAs& naas) const;

    /// Do data track discovery for a list of NAs with meta-data.
    /// @sa void GetNATrackInfo(TTrackInfoList& track_info, const string& naa) const;
    void GetNATrackInfo(TTrackInfoList& track_info,
        const TNAAs& naas, const TNAMetaDataSet& md_set) const;

    void SetMaxNAs(int size);

private:
    /// Initialize some internal states.
    void x_Init();

    /// Get all available NA uids w/wo a context.
    void x_GetNAIds(CSeqUtils::TUids &uids, const string& context = "") const;

    /// get an id for a given NAA string
    /// returns false if no id is found
    static bool x_GetNAId(CSeqUtils::TUid &uid, const string& naa,
                       EUidsSource* pUidsSource = NULL);

    /// get all available NA uids for a given gi.
    void x_GetAllNAIds(CSeqUtils::TUids &uids) const;

    /// filter a given list of NA uids and keep only the ones that are related to a given gi (m_GI)
    void x_FilterNAIds(CSeqUtils::TUids &uids,
                       EUidsSource* pUidsSource = NULL) const;

    /// returns true if an uid is related to  a given gi (m_GI) or GI is ZERO_GI
    bool x_NAIdMatchesGI(CSeqUtils::TUid uid,
                         EUidsSource* pUidsSource = NULL) const;

    /// get all available NA ids applied to a viewer context for a given gi.
    void x_GetAllNAIdsWithContext(CSeqUtils::TUids &uids, const string& context) const;

    /// Retrieve NAAs for a list of NA uids.
    void x_GetNAAs(TNAAs& naas, const CSeqUtils::TUids &uids) const;

    /// get all GIs for a given uid
    /// this needs a given database where to search for GIs
    /// @param if isIncremental == true, the newly found GIs are appended to gis, not erasing it
    static void x_GetAllGIs(CSeqUtils::TUid uid, EEntrezDB eedb, TGis& gis,
                       EUidsSource* pUidsSource = NULL,
                       bool isIncremental = false);
    /// this uses m_SeqDB
    void x_GetAllGIs(CSeqUtils::TUid uid, TGis& gis,
                       EUidsSource* pUidsSource = NULL) const;

    /// Retrieve NA meta data for a list of NA uids.
    void x_GetNAMetaData(TNAMetaDataSet& md_set,
                         const CSeqUtils::TUids &uids,
                         bool isGetLinks,
                         EMetaDataSource* pSource = NULL) const;

    /// Search all NAAs for a given term.
    void x_SearchNAIds(CSeqUtils::TUids &uids, const TNAAs& naas, bool filtering,
                       EUidsSource* pUidsSource = NULL) const;

    /// Parse the meta-data xml DocSum.
    void x_ParseNAMetaData(CAnnotMetaData& data, const char* xml_str) const;

    /// Retrieve Entrez links for a given NA.
    void x_GetNAEntrezLinks(CAnnotMetaData& data) const;

private:
    /// The target sequence associating with the NAAs.
    /// This is optional. If it is set, we only look at the NAAs
    /// associated with the target sequence. If it is not set,
    /// all requested/found NAAs will be returned. However, the
    /// target sequence may be ignored for some cases, such as
    /// searching meta-data for a speicfic NA.
    CConstRef<objects::CSeq_id>  m_TargetSeq;

    /// Scope helping resolve sequences and retrieve annotations.
    /// If not set, a default one will be created if necessary.
    CRef<objects::CScope>   m_Scope;

    /// Derived values which make sense only if m_TargetSeq is set
    objects::CBioseq_Handle m_BioseqHandle;
    objects::CSeq_id_Handle m_SeqIdHandle;       ///< sequence handles
    TGi    m_Gi;       ///< GI of m_TargetSeq; it may be ZERO_GI for one of two reasons: either a target seq is not given at all or
                       ///< it does not have a GI
    EEntrezDB m_SeqDB{EEDB_Undef};   ///< Entrez db name for m_TargetSeq: nucleotide/protein or Undef if no TargetSeq is given

    /// maximal number of NAAs for retrieving meta-data.
    int    m_MaxNAMeta;

    /// caching of eutils-related data:
    /// x_GetNAId():
    /// NAA string to its numeric uid
    typedef map<TNAA, CSeqUtils::TUid> TNAAtoUidCache;
    DECLARE_CLASS_STATIC_FAST_MUTEX(sm_NAAtoUidCacheMutex);
    static TNAAtoUidCache sm_NAAtoUidCache;

    /// x_GetNAMetaData()
    /// NAA numeric uid to CAnnotMetaData
    typedef map<CSeqUtils::TUid, CRef<CAnnotMetaData> > TUidtoMetaDataCache;
    DECLARE_CLASS_STATIC_FAST_MUTEX(sm_UidtoMetaDataCacheMutex);
    static TUidtoMetaDataCache sm_UidtoMetaDataCache;

    /// x_GetAllGIs():
    /// NA uid+dbfrom to GIs which are associated with this NA
    typedef map<string, TGis> TUidtoGiCache;
    DECLARE_CLASS_STATIC_FAST_MUTEX(sm_UidtoGiCacheMutex);
    static TUidtoGiCache sm_UidtoGiCache;
};


END_NCBI_SCOPE

/* @} */

#endif  /// GUI_OBJUTILS___NA_UTILS__HPP
