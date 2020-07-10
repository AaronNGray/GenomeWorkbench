#ifndef GUI_OBJUTILS___UTILS__HPP
#define GUI_OBJUTILS___UTILS__HPP

/*  $Id: utils.hpp 44783 2020-03-12 19:40:03Z shkeda $
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
 * Authors:  Mike DiCuccio, Liangshou Wu
 *
 * File Description:
 *    General utility classes for GUI projects.
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbimisc.hpp>
#include <util/range_coll.hpp>
#include <objmgr/annot_selector.hpp>
#include <objmgr/mapped_feat.hpp>
#include <gui/gui.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/objutils/registry.hpp>
#include <connect/ncbi_http_connector.h>

#include <objtools/readers/iidmapper.hpp>
#include <algo/id_mapper/id_mapper.hpp>


#include <limits>

/** @addtogroup GUI_OBJUTILS
 *
 * @{
 */


BEGIN_SCOPE(xml)
    class document;
    class node;
END_SCOPE(xml)

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CSeq_loc;
    class CBioseq_Handle;
    class CProjectItem;
    class CGC_Assembly;
END_SCOPE(objects)

static const string kCoverageStr = "coverage";

class NCBI_GUIOBJUTILS_EXPORT CLinkedFeature : public CObject
{
public:
    typedef list< CRef<CLinkedFeature> >  TLinkedFeats;
    CLinkedFeature(const objects::CMappedFeat& feat) : m_Feat(feat) {}

    const objects::CMappedFeat& GetMappedFeature() const { return m_Feat; }

    objects::CMappedFeat& GetMappedFeature(){ return m_Feat; }

    const objects::CSeq_feat& GetFeature() const { return m_Feat.GetOriginalFeature(); }

    const TLinkedFeats& GetChildren() const { return m_Children; }

    TLinkedFeats& GetChildren() { return m_Children; }

    void AddChild(CLinkedFeature* feat)
    { m_Children.push_back(CRef<CLinkedFeature>(feat)); }

    void AddChild(const objects::CMappedFeat& feat)
    { m_Children.push_back(CRef<CLinkedFeature>(new CLinkedFeature(feat))); }

private:
    objects::CMappedFeat    m_Feat;
    TLinkedFeats            m_Children;
};


/// Task clients implement this callback interface.
class NCBI_GUIOBJUTILS_EXPORT ISeqTaskProgressCallback
{
public:
    virtual void SetTaskName      (const string& name) = 0;
    /// set total finished task number.
    virtual void SetTaskCompleted (int completed)      = 0;
    /// set to add newly finished task number.
    virtual void AddTaskCompleted (int delta)          = 0;
    virtual void SetTaskTotal     (int total)          = 0;
    virtual bool StopRequested    () const             = 0;
    virtual ~ISeqTaskProgressCallback() {}
};

#define __GUI_SEQ_UTILS_DEFINED__
class NCBI_GUIOBJUTILS_EXPORT CSeqUtils
{
public:
    typedef vector< CRef<objects::CSeq_loc> > TLocVec;

    typedef int TUid;
    typedef vector<TUid> TUids;

    typedef vector<TGi> TGis;
    typedef vector<objects::CSeq_id_Handle> TSeqIdHandles;

    /// flags for classifying annotation names.
    enum EAnnotationNameType
    {
        eAnnot_Unnamed,           ///< unnamed annotation
        eAnnot_Named,             ///< all named annotations
        eAnnot_All,               ///< all annotations
        eAnnot_Other              ///< any given named annots
    };

    typedef EAnnotationNameType     TAnnotNameType;

    static TAnnotNameType NameTypeStrToValue(const string& type);
    static const string& NameTypeValueToStr(TAnnotNameType type);

    /// flags controlling feature retrieval
    enum {
        fFeature_LinkFeatures = 0x1,
        fFeature_Default = 0
    };
    typedef int TFeatureFlags;

    /// flags controlling use of annotation selectors
    enum {
        //< don't apply any settings that affect the retrieval or presentation
        //< of named annotation sets
        fAnnot_UnsetNamed = 0x01,

        //< don't apply any settings that affect the depth of annotation retrieval
        fAnnot_UnsetDepth = 0x02
    };
    typedef int TAnnotFlags;

    typedef CRef<objects::CSeq_interval> TProdInterval;
    typedef CRef<objects::CSeq_interval> TGenomicInterval;
    typedef pair<TProdInterval, TGenomicInterval> TMappedInt;
    typedef vector<TMappedInt> TMappingInfo;

    // set of assembly ids
    typedef set<string> TIds;
    // set of assembly accessions
    typedef set<string> TAccs;

    /// request an annotation selector for a given type
    static objects::SAnnotSelector
        GetAnnotSelector(TAnnotFlags flags = 0);

    static objects::SAnnotSelector
    GetAnnotSelector(objects::SAnnotSelector::TAnnotType choice,
                     TAnnotFlags flags = 0);

    static objects::SAnnotSelector
    GetAnnotSelector(objects::SAnnotSelector::TFeatType,
                     TAnnotFlags flags = 0);

    static objects::SAnnotSelector
    GetAnnotSelector(objects::SAnnotSelector::TFeatSubtype,
                     TAnnotFlags flags = 0);

    /// create a annotation selector for collecting annotation names only.
    /// @param annots
    static objects::SAnnotSelector
        GetAnnotSelector(const vector<string>& annots);

    static objects::SAnnotSelector
        GetAnnotSelector(const vector<string>& annots,
        bool adaptive, int depth);

    /// help function for setting up an annotation.
    static void SetAnnot(objects::SAnnotSelector& sel, const string& annot);

    /// helper functions to read selector-related tune-up info (mostly segment limits) from registry:
    static CRegistryReadView GetSelectorRegistry();
    static int GetMaxSearchSegments(const CRegistryReadView& view);
    static objects::SAnnotSelector::EMaxSearchSegmentsAction GetMaxSearchSegmentsAction(const CRegistryReadView& view);
    static int GetMaxSearchTime(const CRegistryReadView& view);

    /// check actual number of segments against max and perform the action if the actual number is more than max
    /// returns true if actual is more than max
    static bool CheckMaxSearchSegments(int actual, int max, objects::SAnnotSelector::EMaxSearchSegmentsAction action);

    /// help function for setting selector resolve depth.
    /// @param sel target annotation selector
    /// @param adaptive adaptive selector if true, otherwise, exact selector
    /// @param depth resolve depth. For adaptive selector, it resolves all
    ///        if depth < 0, otherwie resolve up to the specified depth.
    ///        For exact selector, depth is the exact the resovle depth (>=0).
    static void SetResolveDepth(objects::SAnnotSelector& sel,
        bool adaptive, int depth = -1);

    typedef int TFeatLinkingMode;

    /// Link features into a hierarchical list.  This function will transform a
    /// linear, feature-order-sorted list of CLinkedFeature into a
    /// hierarchically arranged list in which genes are linked to mRNAs, mRNAs
    /// to CDSs, etc.
    /// @param mode 0:eFeatId_ignore, 1:eFeatId_by_type, 2:eFeatId_always
    static bool LinkFeatures(CLinkedFeature::TLinkedFeats& feats,
        TFeatLinkingMode mode = 1, // TFeatLinkingMode
        ISeqTaskProgressCallback* p_cb = NULL);

    /// remap a location to a parent location.  The child location is split
    /// such that all of its relative positions will map correctly to the
    /// parent location's positions
    static CRef<objects::CSeq_loc>
    RemapChildToParent(const objects::CSeq_loc& parent,
                       const objects::CSeq_loc& child,
                       objects::CScope* scope = NULL);

    /// check to see if two seq-ids are identical.  If provided with a scope,
    /// this function will use the synonyms available through a scope to check
    /// for identicality.  Note that this may require network access, and may
    /// be slow.
    static bool Match(const objects::CSeq_id& id1,
                      const objects::CSeq_id& id2,
                      objects::CScope* scope = NULL);

    static bool Match(const objects::CSeq_id_Handle& id1,
                      const objects::CSeq_id_Handle& id2,
                      objects::CScope* scope = NULL);

    // create a CSeq_loc from the given CSeq_id and a collection of ranges
    static CRef<objects::CSeq_loc>
        CreateSeq_loc(const objects::CSeq_id& id,
                      const CRangeCollection<TSeqPos>& ranges);

    // extracts from the given map all segments corresponding to the given id
    // and puts them to the "ranges" collection. Retuns false if id is not found
    // in the map.
    static bool   GetRangeCollection(const objects::CSeq_id& id,
                                     const objects::CHandleRangeMap& map,
                                     CRangeCollection<TSeqPos>& ranges);

    /// Construct a bioseq to fit a given location.  If the location is of
    /// type 'whole', then a duplicate of the original will be returned;
    /// otherwise, a new bioseq will be constructed to cover just the
    /// region indicated in the seq-loc
    static objects::CBioseq* SeqLocToBioseq(objects::CScope& scope,
                                            const objects::CSeq_loc& loc);

    /// Create a new seq-loc with a unique seq-id from a "mixed" loc.
    /// "Mixed" means mixed seq-ids, e.g. a seq-loc with multi seq-ids.
    static CRef<objects::CSeq_loc>
        MixLocToLoc(const objects::CSeq_loc& mix_loc,
                    const objects::CBioseq_Handle& handle);

    static string GetAnnotName(const objects::CSeq_annot_Handle& annot_handle);

    static string GetAnnotName(const objects::CSeq_annot& annot);

    static string GetAnnotComment(const objects::CSeq_annot_Handle& annot_handle);

    static string GetAnnotComment(const objects::CSeq_annot& annot);


    /// check if a given annotation is a named annotation accession[.version][#number]
    /// when isSctrict == false, includes extended NAs since it is considered no harm to relax the rules for all NAs
    static bool IsNAA(const string& annot, bool isStrict = false);
    /// check if a given annotation is a named annotation name with '.' replaced with '_'.
    static bool IsNAA_Name(const string& annot);

    /// check if a given annotation is an extended NAA (named accession[.version][#number], as used in e.g. SNP2)
    /// when isSctrict == false, regular NAs are now included into this as well i.e. the result of calling this is now the same as IsNAA
    static bool IsExtendedNAA(const string& annot, bool isStrict = false);

    /// check if a given annotation is an extended NAA name with '.' replaced with '_'.
    static bool IsExtendedNAA_Name(const string& annot);

    /// check if a given annotation is AlignDb (potentially suffixed with batch identication string after a '#')
    static bool IsAlignDb(const string& annot);
    /// get a batch string from  AlignDb annotation suffixed with batch identication string after a '#'
    static string GetAlignDbBatch(const string& annot);
    /// get a is_source_assembly_query string from AlignDb annotation suffixed after a second '#'
    static string GetAlignDbIsQuery(const string& annot);

    /// create an annotation name for a remote file pipeline, appending sSuffix
    static string MakeRmtAnnotName(const string& sSuffix);
    /// check if a given annotation was created by a remote file pipeline
    static bool isRmtAnnotName(const string& sAnnotname);

    /// check if a file type is one of remote file types
    static bool isRmtPipelineFileType(const string& sFileType);

    /// check if a given annotation is a unnamed annotation.
    static bool IsUnnamed(const string& annot);

    /// Get the commonly used symbol representing a unnnamed annotation.
    static const string& GetUnnamedAnnot();

    /// Check if string starts with ("SRA", "SRR", "DRR", "ERR")
    static bool IsVDBAccession(const string& acc);

    static bool IsPseudoFeature(const objects::CSeq_feat& feat);
    static bool IsPartialFeature(const objects::CSeq_feat& feat);
    static bool IsPartialStart(const objects::CSeq_loc& loc);
    static bool IsPartialStop(const objects::CSeq_loc& loc);
    static bool IsSameStrands(const objects::CSeq_loc& loc);
    static bool IsException(const objects::CSeq_feat& feat);
    static string GetNcbiBaseUrl();

    /// help methods for creating HTML text
    static string CreateTableStart();
    static string CreateTableEnd();
    static string CreateTableRow(const string& tag = "", const string& value = "");
    static string CreateSectionRow(const string& tag);
    static string CreateLinkRow(const string& tag,
        const string& label, const string& url);

    /// Retrieve mapped-up sequences.
    /// Help method for retrieving upper level sequences for a given gi.
    /// @param gi: the source sequence gi
    static TLocVec GetGiPlacements(TGi gi, int time_out_sec  = 5, THTTP_Flags flags = fHTTP_AutoReconnect);

    /// check that a given seq-id can potentially have placements (to weed out cases like local ids)
    static bool CanHavePlacements(const objects::CSeq_id& seqid);
    /// Retrieve mapped-up sequences.
    /// Help method for retrieving upper level sequences for a given accession.
    static TLocVec GetAccessionPlacements(const objects::CSeq_id &id, objects::CScope &scope, int time_out_sec = 1, THTTP_Flags flags = fHTTP_AutoReconnect);
    /// same, with a timeout in milliseconds
    static TLocVec GetAccessionPlacementsMsec(const objects::CSeq_id &id, objects::CScope &scope, unsigned long time_out_msec = 1000, THTTP_Flags flags = fHTTP_AutoReconnect);

	/// Retrieve locations on mapped-up sequences
    /// Help method for retrieving upper level sequences for a given loc
    /// @param loc
    ///   source location (expected to be a pnt or int)
    /// @return
    ///   list of locations represented as intervals
    static TLocVec GetLocPlacements(const objects::CSeq_loc& loc, int time_out_sec  = 1);

    /// Convert a range string to a range.
    /// @return flag indicating if the input range string is valid
    static bool StringToRange(const string& range_str, long& from, long& to);

    /// try to get a chromosome from a GI
	///
	/// @param gi
	///   GI to try to match to a chromosome
	/// @return
	///   - if the given GI is a chromosome GI, then a string containing the chr number, or X/Y
	///   - empty string if the GI is not a chromosome
	static string GetChrGI(TGi gi);

	/// same as GetChrGI(), but takes an string with id (that must correspond to some GI)
    static string GetChrId(const string& id_str, objects::CScope& scope);

    /// get all assembly ids associated with a gi where this gi is a chromosome
    /// @note transplanted from w_loaders/assm_info.cpp because this is needed for GetChrGI()
	///
	/// @param gi
	///   GI to try to match to a chromosome in any assembly
	/// @param gc_ids
	///   - will be reset and filled with list of assemblies where the given GI is a chromosome
    ///   - will become empty if there is no assembly where this GI is a chromosome
    static void GetAssmIds_GIChr(TUids& gc_ids, TGi gi);

    /// get all assembly accessions corresponding to a GI
    static void GetAssmAccs_Gi(TAccs& accs, TGi gi);

    /// get all assembly ids associated with a gi.
    static void GetAssmIds_GI(TUids& gc_ids, TGi gi);

    // uses a GenColl assembly to find out whether a given sequence is listed as top level in that assembly
    // if scope is given, will consider synonyms for seq-ids found in the assembly; this may give more precise 
    // results at cost of some time
    static bool isTopLevel(const objects::CSeq_id& seq_id, const string& assm_acc, objects::CScope* scope = NULL);
    /// Helper function to generate mapping info between the
    /// specified product sequence and genomic sequence using
    /// the anotated alignment.
    static TMappingInfo GetRnaMappingInfo(const objects::CSeq_loc& feat_loc,
                                          const objects::CMappedFeat& feat,
                                          const objects::CBioseq_Handle& handle);


    /// Derive the CDS feature mapping information based on
    /// its parent RNA feature mapping info.
    static TMappingInfo GetCdsMappingInfoFromRna(
        const TMappingInfo& rna_mapping_info,
        const objects::CSeq_id& rna_product_id,
        const objects::CSeq_loc& feat_loc,
        const objects::CSeq_loc& product,
        objects::CScope& scope);

    // Assumes that the feature and the product have exactly the same number of intervals, and
    // each has the same length
    // feat_offset is used for CDS features to correclty map CDS that have cds_frame > 0
    static TMappingInfo GetMappingInfoFromLocation(
            const objects::CSeq_loc &feat_loc,
            const objects::CSeq_id  &product_id,
            int feat_offset = 0);

    // Gets a feature location on the master sequence
    static CConstRef<objects::CSeq_loc> GetFeatLocation(const objects::CSeq_feat& feat, const objects::CBioseq_Handle& bsh);

    // Finds the parent RNA feature of a given CDS feature
    static objects::CMappedFeat GetMrnaForCds(const objects::CMappedFeat &cds_feat, const string &named_acc = string());

    // Calculates the mapping information of a given RNA/CDS/V Segment feature
    static void GetMappingInfo(const objects::CMappedFeat &mapped_feat, const objects::CBioseq_Handle& bsh, TMappingInfo &info, const string &annot = string());

    /// ad-hoc GI extraction (for misformed seq-id strings like: "gi|55823257|ref|YP_141698.1"
    /// (data coming from some other ad-hoc scripts)
    ///
    /// @param sid - input string
    /// @param gi_str - output string ("gi|123")
    /// @return true if input matches the profile
    ///
    static bool GetGIString(const string& sid, string* gi_str);

    /// Convert a list of ids into a comma-delimited string
    static string CreateIdStr(const TUids& uids);

    /// Queries elink.fcgi with a vector of uids/seq-ids (seq-ids preferred for future compatibility) and returns a vector of uids, filtered by the xpath query
    /// note that return is still uids only since at this point trying to support seq-ids in return is fruitless
    static void ELinkQuery(const string &db_from, const string &db_to, const TUids &uids_from, TUids &uids_to, const string &cmd = "neighbor", const string &xpath = "//Link/Id/text()");
#ifdef NCBI_INT8_GI
    NCBI_DEPRECATED static void ELinkQuery(const string &db_from, const string &db_to, const TGis &uids_from, TGis &uids_to, const string &cmd = "neighbor", const string &xpath = "//Link/Id/text()");
    NCBI_DEPRECATED static void ELinkQuery(const string &db_from, const string &db_to, const TGis &uids_from, TUids &uids_to, const string &cmd = "neighbor", const string &xpath = "//Link/Id/text()");
    NCBI_DEPRECATED static void ELinkQuery(const string &db_from, const string &db_to, const TUids &uids_from, TGis &uids_to, const string &cmd = "neighbor", const string &xpath = "//Link/Id/text()");
    static void ELinkQuery(const string &db_from, const string &db_to, const TSeqIdHandles& uids_from, TGis& uids_to, const string &cmd = "neighbor", const string &xpath = "//Link/Id/text()");
#endif
    static void ELinkQuery(const string &db_from, const string &db_to, const TSeqIdHandles& uids_from, TUids& uids_to, const string &cmd = "neighbor", const string &xpath = "//Link/Id/text()");

    /// Queries elink.fcgi and returns an xml document
    static void ELinkQuery(const string &db_from, const string &db_to, const TUids &uids_from, xml::document& linkset, const string &cmd = "neighbor");
#ifdef NCBI_INT8_GI
    NCBI_DEPRECATED static void ELinkQuery(const string &db_from, const string &db_to, const TGis &uids_from, xml::document& linkset, const string &cmd = "neighbor");
#endif
    static void ELinkQuery(const string &db_from, const string &db_to, const TSeqIdHandles& uids_from, xml::document& linkset, const string &cmd = "neighbor");

	/// Queries esearch.fcgi and returns a vector of uids/seq-ids (seq-ids preferred for future compatibility), filtered by the xpath query
	static void ESearchQuery(const string &db, const string &term, TUids &uids, size_t &count, const int ret_max, const string &xpath = "//IdList/Id/text()");
#ifdef NCBI_INT8_GI
    NCBI_DEPRECATED static void ESearchQuery(const string &db, const string &term, TGis &uids, size_t &count, const int ret_max, const string &xpath = "//IdList/Id/text()");
#endif
	/// Queries esearch.fcgi and returns a vector of uids, filtered by the xpath query
	static void ESearchQuery(const string &db, const string &term, const string &web_env, const string &query_key, TUids &uids, size_t &count, int retstart = 0, const string &xpath = "//IdList/Id/text()");
#ifdef NCBI_INT8_GI
    NCBI_DEPRECATED static void ESearchQuery(const string &db, const string &term, const string &web_env, const string &query_key, TGis &uids, size_t &count, int retstart = 0, const string &xpath = "//IdList/Id/text()");
#endif
    /// Queries esearch.fcgi and returns an xml document
    static void ESearchQuery(const string &db, const string &term, const string &web_env, const string &query_key, xml::document &searchset, int retstart = 0);

    /// Returns the contents of the specified child node
    static std::string GetXmlChildNodeValue(const xml::node& parent, const std::string& name);

    /// Returns Bioseq's Genetic Code
    static int GetGenCode(const objects::CBioseq_Handle& handle);

    static void CreateCoverageAnnotName(const string& annot_name, string& coverage_annot);
    static bool IsCoverageAnnotName(const string& annot_name);

    // parse a ranges string into TRanges
    // string format:
    // from-to[,from-to...]	 comma-delimited list of position ranges, zero-based, end-inclusive
    typedef vector<TSeqRange> TRanges;

    static void ParseRanges(const string& sRanges, TRanges& ranges);
        
    

private:
    /// get all assembly ids associated with a gi.
    static void x_GetAssmIds_GI(string& gc_ids, TGi gi);

};


/// Adapter class to convert CGencollIdMapper to IIdMapper interface
class NCBI_GUIOBJUTILS_EXPORT CGencollIdMapperAdapter : public objects::IIdMapper
{
public:
    CGencollIdMapperAdapter(CRef<CGencollIdMapper> mapper, const CGencollIdMapper::SIdSpec& spec);
    virtual objects::CSeq_id_Handle Map(const objects::CSeq_id_Handle& idh) override;
    virtual CRef<objects::CSeq_loc> Map(const objects::CSeq_loc& loc) override;
    virtual void MapObject(CSerialObject& obj) override;

    static IIdMapper* GetIdMapper(CRef<objects::CGC_Assembly> assm);

private:
    CRef<CGencollIdMapper> m_Mapper;
    CGencollIdMapper::SIdSpec m_Spec;
};



inline
bool CSeqUtils::IsUnnamed(const string& annot)
{
    return annot == GetUnnamedAnnot();
}

inline
const string& CSeqUtils::GetUnnamedAnnot()
{
    static string kUnnamedAnnot = "Unnamed";
    return kUnnamedAnnot;
}

inline
void CSeqUtils::CreateCoverageAnnotName(const string& annot_name, string& coverage_annot)
{
    coverage_annot = annot_name;
    coverage_annot += " ";
    coverage_annot += kCoverageStr;
}

inline
bool CSeqUtils::IsCoverageAnnotName(const string& annot_name)
{
    return NStr::EndsWith(annot_name, kCoverageStr, NStr::eNocase);
}

NCBI_GUIOBJUTILS_EXPORT CConstRef<objects::CBioseq> GetBioseqForSeqdesc (CRef<objects::CScope> scope, const objects::CSeqdesc& seq_desc);
NCBI_GUIOBJUTILS_EXPORT objects::CBioseq_Handle GetBioseqForSeqFeat(const objects::CSeq_feat& f, objects::CScope& scope);
NCBI_GUIOBJUTILS_EXPORT objects::CSeq_feat_Handle GetSeqFeatHandleForBadLocFeature(const objects::CSeq_feat& feat, objects::CScope& scope);
NCBI_GUIOBJUTILS_EXPORT objects::CSeq_entry_Handle GetSeqEntryForPubdesc (CRef<objects::CScope> scope, const objects::CPubdesc& pubdesc);
NCBI_GUIOBJUTILS_EXPORT objects::CSeq_entry_Handle GetTopSeqEntryFromScopedObject (SConstScopedObject& obj);
NCBI_GUIOBJUTILS_EXPORT vector<const objects::CFeatListItem *> GetSortedFeatList(objects::CSeq_entry_Handle seh, size_t max = numeric_limits<size_t>::max());

END_NCBI_SCOPE


/* @} */

#endif  /// GUI_OBJUTILS___UTILS__HPP
