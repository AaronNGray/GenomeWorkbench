/*  $Id: feature_ds.cpp 44986 2020-05-01 22:45:19Z rudnev $
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
 * Authors:  Liangshou Wu
 *
 */
#include <ncbi_pch.hpp>

#include <gui/widgets/seq_graphic/feature_ds.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job.hpp>
#include <gui/widgets/seq_graphic/cds_glyph.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <gui/widgets/seq_graphic/clone_placement_glyph.hpp>
#include <gui/widgets/seq_graphic/layout_group.hpp>
#include <gui/widgets/seq_graphic/layout_policy.hpp>
#include <gui/widgets/seq_graphic/named_group.hpp>
#include <gui/widgets/seq_graphic/gene_model_config.hpp>
#include <gui/widgets/seq_graphic/gene_model_group.hpp>
#include <gui/widgets/seq_graphic/feature_filter.hpp>
#include <gui/widgets/seq_graphic/comment_glyph.hpp>
#include <gui/widgets/seq_graphic/clone_feature_sorter.hpp>
#include <gui/widgets/seq_graphic/dbvar_feature_sorter.hpp>
#include <gui/widgets/seq_graphic/graph_cache.hpp>
#include <gui/widgets/seq_graphic/bed_coverage_graph.hpp>
#include <gui/widgets/seq_graphic/vcf_heatmap.hpp>

#include <gui/widgets/seq_graphic/graph_utils.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>

#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/seqalign/Std_seg.hpp>

// for gene_by_loc reply
#include <objects/genesbyloc/GBL_Reply.hpp>
#include <objects/genesbyloc/GBL_Data.hpp>
#include <objects/genesbyloc/GBL_Gene.hpp>

#include <gui/objutils/density_map.hpp>

#include <serial/serial.hpp>
#include <connect/ncbi_types.h>
#include <connect/ncbi_conn_stream.hpp>
#include <serial/objistr.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

// the maximal number of objects allowed to show without smearing
static const int kMaxObjNum = 250000;

// the maximal number of children in parent/child situations dictated by approximation of screen size limits
static const size_t kMaxChildNum = 500;
// the zoom level that allows to show sequence
static const float kSeqZoomLevel = 0.125;
// unique group for 'unsorted' (no sorting is needed) features
static const char* kUnsortedGroupName = "--unsorted--";

// create feature product mapping if scale (m_Window) is less than this value
static const float kMinScaleForMapping = 16.;


///////////////////////////////////////////////////////////////////////////////
/// CSGFeatureJob: a job class for loading annotated features from a given
/// sequence and visible range.
///////////////////////////////////////////////////////////////////////////////
class CSGFeatureJob : public CSGAnnotJob
{
public:
    typedef pair<string, CLinkedFeature::TLinkedFeats> TSortedFeats;
    typedef vector<TSortedFeats> TFeatGroups;
    typedef map<int, CLinkedFeature::TLinkedFeats> TFeatGroupMap;

    struct SLandmarkFeat
    {
        float score;
        CRef<CSeqGlyph> obj;
    };

    CSGFeatureJob(const string& desc, CBioseq_Handle handle,
        const SAnnotSelector& sel, const TSeqRange& range,
        TModelUnit window, TJobToken token);

    CSGFeatureJob(const string& desc, CBioseq_Handle handle,
                  const SAnnotSelector& sel, const TSeqRange& range,
                  TModelUnit window, int max_feat, ELinkedFeatDisplay LinkedFeatDisplay,
                  TJobToken token, ICoordMapper* mapper = NULL);

    void SetFilter(const string& filter)
    { m_Filter = filter; }

    void SetSortBy(const string& sortby);

    void SetBigBedCacheKey(const string& cache_key)
    {
        m_BigBedCacheKey = cache_key;
    }

    void SetVcfCacheKey(const string& cache_key)
    {
        m_VcfCacheKey = cache_key;
    }


    static void GetAnnotNames(const CBioseq_Handle& handle,
        const TSeqRange& range, SAnnotSelector& sel, TAnnotNameTitleMap& names);

    static void GetAnnotNames_var(const CBioseq_Handle& handle,
        const TSeqRange& range, SAnnotSelector& sel, TAnnotNameTitleMap& names);


protected:
    virtual EJobState x_Execute();

    /// @name feature glyph creators.
    /// Using different function names to get by hide-by-name semantics.
    /// Maybe should consider using 'using scope' directive in the
    /// derived class.
    /// @{
    /// shared creator.
    CRef<CSeqGlyph> x_CreateFeature1(const CMappedFeat& feat,
        CSeqGlyph* parent_glyph = NULL,
        const char* label_prefix = NULL);

    /// overridable creator.
    virtual CRef<CSeqGlyph> x_CreateFeature2(
        const CMappedFeat& feat, const CSeq_loc& loc,
        const char* label_prefix = NULL);
    /// @}

    CRef<CSeqGlyph> x_CreateCloneFeature(const CMappedFeat& feat,
                                         const CSeq_loc& loc);

    IAppJob::EJobState x_CreateFeature_Recursive(
        CLinkedFeature::TLinkedFeats& features, CLayoutGroup* group, int Level);

    IAppJob::EJobState x_CreateFeatures(CFeat_CI& feature_iter,
        CSeqGlyph::TObjects& objs);

    void x_CalcIntervals(CFeatGlyph& feat);

    EJobState x_CreateFeatHist(CFeat_CI& feat_iter,
        CSGJobResult* result, const string& title);

    EJobState x_CreateFeatHistFromGraph(CSGJobResult* result,
        int level, const string& title);

    IAppJob::EJobState x_CreateFeatSmear(CFeat_CI& feat_iter,
        CSGJobResult* result, const string& title);

    bool x_FilterFeature_recursive(
        CLinkedFeature::TLinkedFeats& features,
        CFeatureFilter& filter);

    void x_SortFeatures(CLinkedFeature::TLinkedFeats& feats,
        TFeatGroups& feat_groups);

    void x_SortFeature_recursive(CLinkedFeature::TLinkedFeats& feats,
        TFeatGroupMap& group_map);

    IAppJob::EJobState x_LinkFeatures(CFeat_CI& feature_iter, CSeqGlyph::TObjects& objs);

    bool x_ProcessCachedBigBed();
    bool x_ProcessCachedVcfTabix();

    
protected:
    TModelUnit  m_Window;       ///< current window for smear bars
    /// the maximum number of features allowed.
    /// -2: pack all features into one smear line
    /// -1: theoretically unlimited, practically there is one (kMaxObjNum)
    /// -0: pack all features into a histogram
    /// >0: any limit set from a user
    int         m_FeatLimit;

    /// controls creation of parent or child features
    ELinkedFeatDisplay m_LinkedFeat;

    string      m_Filter;
    CIRef<IFeatSorter>     m_Sorter;

    CIRef<ICoordMapper>    m_Mapper;

    string m_BigBedCacheKey;
    string m_VcfCacheKey;

};


///////////////////////////////////////////////////////////////////////////////
/// CGeneModelFeatJob: a job class for loading annotated main features,
/// including genes, RNAs, CDSs and exons from a given sequence and a given
/// visible range.
///////////////////////////////////////////////////////////////////////////////
class CGeneModelFeatJob : public CSGFeatureJob
{
public:
    enum EGeneFilterType {
        eFilter_invalid = -1,
        eFilter_dbref_all = 0,  ///< any dbref
        eFilter_dbref,      ///< one specific dbref
        eFilter_CCDS,       ///< genes with Consensus CDS
        eFilter_pseudo_genes,
        eFilter_ncRNAs,
        eFilter_npcGenes
    };

    typedef int TFilterType;

    CGeneModelFeatJob(const string& desc,
        CBioseq_Handle handle,
        const SAnnotSelector& lm_feat_sel,
        const SAnnotSelector& m_feat_sel,
        TJobToken lm_token, TJobToken main_token,
        int merge_style, bool landmark_feat,
        int min_overview_feat,
        const TSeqRange& range, TModelUnit window,
        int max_feat, bool show_histogram,
        ICoordMapper* mapper,
        int highlight_mode);

    static vector<string> GetFilters(const CBioseq_Handle& handle,
        const TSeqRange& range, SAnnotSelector& sel, const string& filter);

protected:
    virtual EJobState x_Execute();

    virtual CRef<CSeqGlyph> x_CreateFeature2(
        const CMappedFeat& feat, const CSeq_loc& loc,
        const char* label_prefix = NULL);

private:
    EJobState x_ProcessLandMarkFeatures(CSeqGlyph::TObjects& objs,
        CRef<CSeqGlyph>& hist_obj, CSeqGlyph::TObjects& comment_objs);

    /// Link features into a hierarchical list.  This function will transform a
    /// linear, feature-order-sorted list of CLayoutFeat objects into a
    /// hierarchically arranged list in which genes are linked to mRNAs, mRNAs
    /// to CDSs, etc.
    IAppJob::EJobState x_CreateGeneModels(CLinkedFeature::TLinkedFeats& feats,
        CSeqGlyph::TObjects& objs);

    /// create features when there is a filter set up.
    IAppJob::EJobState x_CreateFeaturesWithFilter(CFeat_CI& feature_iter,
        CSeqGlyph::TObjects& objs, TFilterType filter);

    static TFilterType x_FilterStrToValue(const string& filter);

    static bool x_IsDbref(const string& db, const CSeq_feat& feat)
    {
        if (feat.IsSetDbxref()) {
            ITERATE (CSeq_feat::TDbxref, iter, feat.GetDbxref()) {
                if (NStr::EqualNocase(db, (*iter)->GetDb())) {
                    return true;
                }
            }
        }
        return false;
    }

    static bool x_IsDbref(const string& db, const CLinkedFeature& linked_feat) {
        const CSeq_feat& feat = linked_feat.GetFeature();
        if (x_IsDbref(db, feat)) {
            return true;
        }

        ITERATE (CLinkedFeature::TLinkedFeats, iter, linked_feat.GetChildren()) {
            if (x_IsDbref(db, **iter)) {
                return true;
            }
        }

        return false;
    }

    static bool x_IsCCDS(const CLinkedFeature& linked_feat) {
        const CSeq_feat& feat = linked_feat.GetFeature();
        if (feat.GetData().IsGene()) {
            ITERATE (CLinkedFeature::TLinkedFeats, iter, linked_feat.GetChildren()) {
                const CSeq_feat& c_feat = (*iter)->GetFeature();
                if (c_feat.GetData().IsRna()) {
                    ITERATE (CLinkedFeature::TLinkedFeats, c_iter, (*iter)->GetChildren()) {
                        if (x_IsDbref("CCDS", (*c_iter)->GetFeature())) {
                            return true;
                        }
                    }
                } else if (c_feat.GetData().IsCdregion()) {
                    if (x_IsDbref("CCDS", c_feat)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    static bool x_IsPseudoGene(const CSeq_feat& feat)
    {
        return (feat.IsSetPseudo()  &&  feat.GetPseudo());
    }

    static bool x_IsncRNA(const CLinkedFeature& linked_feat) {
        const CSeq_feat& feat = linked_feat.GetFeature();
        bool has_rna = false;
        bool has_cds = false;
        if (feat.GetData().IsGene()) {
            ITERATE (CLinkedFeature::TLinkedFeats, iter, linked_feat.GetChildren()) {
                const CSeq_feat& c_feat = (*iter)->GetFeature();
                if (c_feat.GetData().IsRna()) {
                    has_rna = true;
                    if (!(*iter)->GetChildren().empty()) {
                        has_cds = true;
                    }
                }
            }
        } else if (feat.GetData().IsRna()) {
            has_rna = true;
            if (!linked_feat.GetChildren().empty()) {
                has_cds = true;
            }
        }
        return has_rna && !has_cds;
    }

    static bool x_IsnpcGene(const CLinkedFeature& linked_feat) {
        const CSeq_feat& feat = linked_feat.GetFeature();
        if (feat.GetData().IsGene()) {
            ITERATE (CLinkedFeature::TLinkedFeats, iter, linked_feat.GetChildren()) {
                const CSeq_feat& c_feat = (*iter)->GetFeature();
                if ( (c_feat.GetData().IsRna()  &&  !(*iter)->GetChildren().empty())  ||
                    c_feat.GetData().IsCdregion()) {
                    return false;
                }
            }
        }
        return true;
    }

private:
    SAnnotSelector m_MainFeatSel;  ///< annotation selector for all main features.
    TJobToken   m_MainFeatToken;            ///< Job token for loading main features.
    int         m_MergeStyle;               ///< style for merging RNA and CDS features.
    bool        m_LandmarkFeats;            ///< is overview mode?
    int         m_MinOverviewFeat;          ///< mininal landmark feat number
    bool        m_ShowHistogram;
    int         m_HighlightMode;
};


///////////////////////////////////////////////////////////////////////////////
/// CBatchFeatJob: a job class for loading annotated features from a given
/// a set of sequences within the corresponding visible ranges.
///////////////////////////////////////////////////////////////////////////////
class CBatchFeatJob : public CSGFeatureJob
{
public:
    CBatchFeatJob(const string& desc, CBioseq_Handle handle,
        const TSeqRange& range, TModelUnit window, TJobToken token,
        const TFeatBatchJobRequests& requests);

protected:
    virtual EJobState x_Execute();

private:
    virtual CRef<CSGJobResult> x_LoadFeatProducts(CRef<CFeatGlyph> feat,
        const SAnnotSelector& sel);

private:
    TFeatBatchJobRequests   m_Requests;
};


///////////////////////////////////////////////////////////////////////////////
/// CSGCalcFeatHistJob
///////////////////////////////////////////////////////////////////////////////
class CSGCalcFeatHistJob : public CSeqGraphicJob
{
public:
    CSGCalcFeatHistJob(const string& desc, const CSeqGlyph::TObjects& objs,
        const TSeqRange& range, TModelUnit window, TJobToken token)
        : CSeqGraphicJob(desc)
        , m_Objects(objs)
        , m_Range(range)
        , m_Window(window)
    {
        SetToken(token);
    }

protected:
    virtual EJobState x_Execute();

private:
    EJobState x_CalcFeatHistRecursive(CRef<CSeqGlyph> obj,
        CDensityMap<float>& the_map);

private:
    const CSeqGlyph::TObjects& m_Objects;
    TSeqRange   m_Range;            ///< target range
    TModelUnit  m_Window;           ///< current window for smear bars
};


///////////////////////////////////////////////////////////////////////////////
/// CSGFeatureDS
///////////////////////////////////////////////////////////////////////////////
CSGFeatureDS::CSGFeatureDS(CScope& scope, const CSeq_id& id)
    : CSGGenBankDS(scope, id)
{}

void CSGFeatureDS::LoadFeatures(SAnnotSelector& sel,
                                const TSeqRange& range, TModelUnit window,
                                TJobToken token, int max_feat,
                                ELinkedFeatDisplay LinkedFeat)
{
    CSeqUtils::SetResolveDepth(sel, m_Adaptive, m_Depth);
    CRef<CSGFeatureJob> job(
        new CSGFeatureJob("Feature", m_Handle, sel, range, window,
                          max_feat, LinkedFeat,
                          token, m_Mapper));
    job->SetGraphLevel(CGraphUtils::GetNearestLevel(m_GraphLevels, window));
    job->SetFilter(m_Filter);
    job->SetSortBy(m_SortBy);
    if (IsBigBed())
        job->SetBigBedCacheKey(m_GraphCacheKey);
    else if (IsVcfTabix())        
        job->SetVcfCacheKey(m_GraphCacheKey);
    x_LaunchJob(*job);
}


void CSGFeatureDS::LoadFeatures(const TSeqRange& range,
                                TModelUnit window,
                                TJobToken token,
                                const TFeatBatchJobRequests& requests)
{
    //CSeqUtils::SetResolveDepth(sel, m_Adaptive, m_Depth);
    CRef<CSGFeatureJob> job(new CBatchFeatJob(
        "Feature", m_Handle, range, window, token, requests));
    job->SetGraphLevel(CGraphUtils::GetNearestLevel(m_GraphLevels, window));
    if (IsBigBed())
        job->SetBigBedCacheKey(m_GraphCacheKey);
    else if (IsVcfTabix())        
        job->SetVcfCacheKey(m_GraphCacheKey);

    // no progress report
    x_LaunchJob(*job, -1);
}


void CSGFeatureDS::LoadMainFeatures(SAnnotSelector& lm_feat_sel,
                                    SAnnotSelector& m_feat_sel,
                                    TJobToken lm_token, TJobToken main_token,
                                    int merge_style, bool landmark_feat,
                                    int min_overview_feat,
                                    const TSeqRange& range,
                                    TModelUnit window, int max_feat,
                                    bool show_histogram, int highlight_mode)
{
    CSeqUtils::SetResolveDepth(lm_feat_sel, m_Adaptive, m_Depth);
    CSeqUtils::SetResolveDepth(m_feat_sel, m_Adaptive, m_Depth);
    CRef<CGeneModelFeatJob> job(new CGeneModelFeatJob(
        "Feature", m_Handle, lm_feat_sel, m_feat_sel, lm_token,
        main_token, merge_style, landmark_feat, min_overview_feat,
        range, window, max_feat, show_histogram, m_Mapper, highlight_mode) );
    job->SetFilter(m_Filter);
    job->SetSortBy(m_SortBy);
    x_LaunchJob(*job);
}


void CSGFeatureDS::CalcFeatHistogram(const CSeqGlyph::TObjects& objs,
                                     const TSeqRange& range,
                                     TModelUnit window, TJobToken token)
{
    CRef<CSGCalcFeatHistJob> job(new CSGCalcFeatHistJob(
        "Calculate feature histogram", objs,
        range, window, token));
    x_LaunchJob(*job);
}


void CSGFeatureDS::GetAnnotNames(SAnnotSelector& sel,
                                 const TSeqRange& range,
                                 TAnnotNameTitleMap& names) const
{
    CSeqUtils::SetResolveDepth(sel, m_Adaptive, m_Depth);
    CSGFeatureJob::GetAnnotNames(m_Handle, range, sel, names);
}


void CSGFeatureDS::GetAnnotNames_var(SAnnotSelector& sel,
                                     const TSeqRange& range,
                                     TAnnotNameTitleMap& names) const
{
    CSeqUtils::SetResolveDepth(sel, m_Adaptive, m_Depth);
    CSGFeatureJob::GetAnnotNames_var(m_Handle, range, sel, names);
}


vector<string> CSGFeatureDS::GetFilters(const string& annot_name,
                                        const TSeqRange& range) const
{
    SAnnotSelector sel(CSeqUtils::GetAnnotSelector());
    CSeqUtils::SetAnnot(sel, annot_name);
    return CGeneModelFeatJob::GetFilters(m_Handle, range, sel, m_Filter);
}


///////////////////////////////////////////////////////////////////////////////
/// CSGFeatureDSType
///////////////////////////////////////////////////////////////////////////////

ISGDataSource*
CSGFeatureDSType::CreateDS(SConstScopedObject& object) const
{
    const CSeq_id& id = dynamic_cast<const CSeq_id&>(object.object.GetObject());
//                cerr << "Creating DS for seq-id" << endl << 
//                      MSerial_AsnText << id << endl;
    return new CSGFeatureDS(object.scope.GetObject(), id);
}


string CSGFeatureDSType::GetExtensionIdentifier() const
{
    static string sid("seqgraphic_feature_ds_type");
    return sid;
}


string CSGFeatureDSType::GetExtensionLabel() const
{
    static string slabel("Graphical View Feature Data Source Type");
    return slabel;
}


bool CSGFeatureDSType::IsSharable() const
{
    return false;
}

//
//void CFeatFilter::x_AddToList(const string& type, bool shown)
//{
//    TFeatTypeIdx idx = CCustomFeatureColor::eDefault;
//    if (type == "cnv") {
//        idx = CCustomFeatureColor::eCNV;
//    } else if (type == "other") {
//        idx = CCustomFeatureColor::eOther;
//    } else if (type == "inversion") {
//        idx = CCustomFeatureColor::eInversion;
//    } else if (type == "loss") {
//        idx = CCustomFeatureColor::eLoss;
//    } else if (type == "gain") {
//        idx = CCustomFeatureColor::eGain;
//    } else if (type == "insertion") {
//        idx = CCustomFeatureColor::eInsertion;
//    } else if (type == "complex") {
//        idx = CCustomFeatureColor::eComplex;
//    } else if (type == "unknown") {
//        idx = CCustomFeatureColor::eUnknown;
//    } else if (type == "loh") {
//        idx = CCustomFeatureColor::eLOH;
//    } else if (type == "everted") {
//        idx = CCustomFeatureColor::eEverted;
//    } else if (type == "translocation") {
//        idx = CCustomFeatureColor::eTranschr;
//    } else if (type == "upd") {
//        idx = CCustomFeatureColor::eUPD;
//    }
//    if (idx != CCustomFeatureColor::eDefault) {
//        if (shown) {
//            m_ShowList.insert(idx);
//            m_HideList.clear();
//        } else {
//            m_HideList.insert(idx);
//            m_ShowList.clear();
//        }
//    }
//}


///////////////////////////////////////////////////////////////////////////////
/// CSGFeatureJob implementation
///////////////////////////////////////////////////////////////////////////////
struct SLandmarkFeatByScore
{
    bool operator()(const CSGFeatureJob::SLandmarkFeat& f1,
        const CSGFeatureJob::SLandmarkFeat& f2) const
    {
        return f1.score > f2.score;
    }
};

struct SLandmarkFeatBySeqPos
{
    bool operator()(const CSGFeatureJob::SLandmarkFeat& f1,
                    const CSGFeatureJob::SLandmarkFeat& f2) const
    {
        return f1.obj->LessBySeqPos(*f2.obj);
    }
};


CSGFeatureJob::CSGFeatureJob(const string& desc, CBioseq_Handle handle,
                             const SAnnotSelector& sel,
                             const TSeqRange& range, TModelUnit window,
                             TJobToken token)
    : CSGAnnotJob(desc, handle, sel, range)
    , m_Window(window)
    , m_FeatLimit(-1)
    , m_LinkedFeat(ELinkedFeatDisplay::eLFD_Default)
{
    SetToken(token);
}


CSGFeatureJob::CSGFeatureJob(const string& desc, CBioseq_Handle handle,
                             const SAnnotSelector& sel,
                             const TSeqRange& range, TModelUnit window,
                             int max_feat, ELinkedFeatDisplay LinkedFeatDisplay,
                             TJobToken token, ICoordMapper* mapper)
    : CSGAnnotJob(desc, handle, sel, range)
    , m_Window(window)
    , m_FeatLimit(max_feat)
    , m_LinkedFeat(LinkedFeatDisplay)
    , m_Mapper(mapper)
{
    SetToken(token);
}


void CSGFeatureJob::SetSortBy(const string& sortby)
{
    if (sortby.empty()) return;

    size_t found = sortby.find_first_of('|');
    string sorter_type = NStr::TruncateSpaces(sortby.substr(0, found));
    string sort_str = kEmptyStr;
    if (found != string::npos) {
        sort_str = sortby.substr(found + 1);
    }
    if (NStr::EqualNocase(sorter_type, CCloneConcordancySorter::GetID())) {
        m_Sorter.Reset(new CCloneConcordancySorter(sort_str));
    } else if (NStr::EqualNocase(sorter_type, CDbvarQualitySorter::GetID())) {
        m_Sorter.Reset(new CDbvarQualitySorter(sort_str));
    } else if (NStr::EqualNocase(sorter_type, CDbvarPilotSorter::GetID())) {
        m_Sorter.Reset(new CDbvarPilotSorter(sort_str));
    } else if (NStr::EqualNocase(sorter_type, CDbvarClinicalAsstSorter::GetID())) {
        m_Sorter.Reset(new CDbvarClinicalAsstSorter(sort_str));
    } else if (NStr::EqualNocase(sorter_type, CDbvarSamplesetTypeSorter::GetID())) {
        m_Sorter.Reset(new CDbvarSamplesetTypeSorter(sort_str));
    } else if (NStr::EqualNocase(sorter_type, CDbvarValidationSorter::GetID())) {
        m_Sorter.Reset(new CDbvarValidationSorter(sort_str));
    } else if (NStr::EqualNocase(sorter_type, CGRCStatusSorter::GetID())) {
        m_Sorter.Reset(new CGRCStatusSorter(sort_str));
    }
}


void CSGFeatureJob::GetAnnotNames(const CBioseq_Handle& handle,
                                  const TSeqRange& range,
                                  SAnnotSelector& sel,
                                  TAnnotNameTitleMap& names)
{
    sel.SetCollectNames();
    CFeat_CI feat_iter(handle, range, sel);
    ITERATE (CFeat_CI::TAnnotNames, iter, feat_iter.GetAnnotNames()) {
        if (iter->IsNamed()) {
            if (iter->GetName().find("@@") == string::npos) {
                names.insert(TAnnotNameTitleMap::value_type(iter->GetName(), ""));
            }
        } else {
            names.insert(TAnnotNameTitleMap::value_type(CSeqUtils::GetUnnamedAnnot(), ""));
        }
    }
}


void CSGFeatureJob::GetAnnotNames_var(const CBioseq_Handle& handle,
                                      const TSeqRange& range,
                                      SAnnotSelector& sel,
                                      TAnnotNameTitleMap& names)
{
    sel.SetCollectNames();
    CFeat_CI feat_iter(handle, range, sel);
    ITERATE (CFeat_CI::TAnnotNames, iter, feat_iter.GetAnnotNames()) {
        if (iter->IsNamed()) {
            if (iter->GetName().find("@@") == string::npos) {
                names.insert(TAnnotNameTitleMap::value_type(iter->GetName(), ""));
            }
        } else {
            names.insert(TAnnotNameTitleMap::value_type(CSeqUtils::GetUnnamedAnnot(), ""));
        }
    }
    /// TODO: look at the real feature for each annotaton to verify
    /// it is really structural variants, not SNP.
}

IAppJob::EJobState CSGFeatureJob::x_LinkFeatures(CFeat_CI& feature_iter, CSeqGlyph::TObjects& objs)
{
    SetTaskName("Linking features...");
    
    // Creating feature wrappers
    CLinkedFeature::TLinkedFeats features;
    for ( ;  feature_iter;  ++feature_iter) {
        if (IsCanceled()) {
            return eCanceled;
        }
        const CMappedFeat& feat = *feature_iter;
        //        CRef<CLinkedFeature> fref( new CLinkedFeature(feat) );
        features.emplace_back(new CLinkedFeature(feat));
//        cerr << "Got feature" << endl << 
//          MSerial_AsnText << feat.GetOriginalFeature() << endl;
    }
                    
    // linking features
    if ( !CSeqUtils::LinkFeatures(features,
                                  feature::CFeatTree::eFeatId_always, this) ) {
        return eCanceled;
    }
    
    // filtering features
    CFeatureFilter filter;
    filter.Init(m_Filter);
    if (filter.NeedFiltering()) {
        x_FilterFeature_recursive(features, filter);
        if (IsCanceled()) {
            return eCanceled;
        }
    }
    
    TFeatGroups feat_groups;
    x_SortFeatures(features, feat_groups);
    
    SetTaskName("Creating feature glyphs...");
    SetTaskCompleted(0);
    
    NON_CONST_ITERATE (TFeatGroups, grp_iter, feat_groups) {
        if (IsCanceled()) {
            return eCanceled;
        }
        
        CLinkedFeature::TLinkedFeats& curr_feats = grp_iter->second;
        NON_CONST_ITERATE (CLinkedFeature::TLinkedFeats, iter, curr_feats) {
            if(m_LinkedFeat == ELinkedFeatDisplay::eLFD_Hidden &&  !(*iter)->GetChildren().empty() ) {
                (*iter)->GetChildren().clear();
            }
        }
        CRef<CLayoutGroup> group(new CLayoutGroup(m_LinkedFeat));
        
        EJobState state = x_CreateFeature_Recursive(curr_feats, group.GetPointer(), 0);
        if (state != eCompleted) {
            return state;
        }
        
        if (curr_feats.size() == group->GetChildrenNum()) {
            // For most cases, we don't need the group container
            // added earlier. Instead, take the children only.
            if (grp_iter->first == kUnsortedGroupName) {
                // Sorting features is not needed in this case.
                // That means there should be only one group.
                _ASSERT(feat_groups.size() == 1);
                objs.swap(group->SetChildren());
            } else {
                // created a group with a title
                CRef<CNamedGroup> named_group((new CNamedGroup(grp_iter->first)));
                named_group->Set(group->SetChildren());
                objs.emplace_back(named_group.GetPointer());
            }
        } else {
            // There is one case we need to group the features:
            //    When all the features are linked together as one
            //    feature group with parent and child relationship.
            //    After x_CreateFeature_Recursive step, the parent
            //    feature and child feature(s) are put in one group
            //    which is this group container. We need it to hold
            //    them as a linked group.
            if (grp_iter->first == kUnsortedGroupName) {
                // Sorting features is not needed in this case.
                // That means there should be only one group.
                _ASSERT(feat_groups.size() == 1);
                objs.emplace_back(group.GetPointer());
            } else {
                // created a group with a title
                CRef<CNamedGroup> named_group((new CNamedGroup(grp_iter->first)));
                named_group->PushBack(group);
                objs.emplace_back(named_group.GetPointer());
            }
        }
    }
                
    return eCompleted;
}


bool CSGFeatureJob::x_ProcessCachedBigBed()
{
    _ASSERT(m_Result);
    if (!m_Result)
        return false;
    auto result = dynamic_cast<CSGJobResult*>(m_Result.GetPointer());
    if (!result) 
        return false;
    auto data = CGraphCache<CBedCoverageGraph>::GetInstance().GetData(m_BigBedCacheKey);
    if (!data) 
        return false;
    CHistogramGlyph::TMap the_map(m_Range.GetFrom(),
                                    m_Range.GetTo(),
                                    (float)m_Window,
                                    new plus_func<CHistogramGlyph::TDataType>());
    data->GetData(the_map);

    SAnnotSelector::TAnnotsNames annots;
    annots = m_Sel.GetIncludedAnnotsNames();
    for(const auto& annot : annots) {
        if(annot.IsNamed()) {
            result->m_Desc = annot.GetName();
            break;
        }
    }
    CRef<CHistogramGlyph> histogram(new CHistogramGlyph(the_map, result->m_Desc));
    result->m_ExtraObj.Reset(histogram.GetPointer());
    SetTaskCompleted(1);
    return true;
}

bool CSGFeatureJob::x_ProcessCachedVcfTabix()
{
    _ASSERT(m_Result);
    if (!m_Result)
        return false;
    auto result = dynamic_cast<CSGJobResult*>(m_Result.GetPointer());
    if (!result) 
        return false;
    auto data = CGraphCache<CVcfHeatmap>::GetInstance().GetCachedData(m_VcfCacheKey);
    if (!data) 
        return false;
    vector<TSeqRange> ranges;
    data->GetData(m_Range, ranges);    
    CHistogramGlyph::TMap the_map(m_Range.GetFrom(),
                                    m_Range.GetTo(),
                                    (float)m_Window,
                                    new max_func<CHistogramGlyph::TDataType>());
    for (const auto& r : ranges) {
        the_map.AddRange(r, 1);
    }

    SAnnotSelector::TAnnotsNames annots;
    annots = m_Sel.GetIncludedAnnotsNames();
    for(const auto& annot : annots) {
        if(annot.IsNamed()) {
            result->m_Desc = annot.GetName();
            break;
        }
    }
    CRef<CHistogramGlyph> histogram(new CHistogramGlyph(the_map, result->m_Desc));
    result->m_ExtraObj.Reset(histogram.GetPointer());
    SetTaskCompleted(1);
    return true;
}


IAppJob::EJobState CSGFeatureJob::x_Execute()
{
    CSGJobResult* result = new CSGJobResult();
    m_Result.Reset(result);

    CConstRef<CSeq_loc> loc(
        m_Handle.GetRangeSeq_loc(m_Range.GetFrom(), m_Range.GetTo()) );

    SetTaskName("Loading features...");
    SetTaskTotal(0);
    SetTaskCompleted(0);

    try {
        bool glyph_created = false;
        if (!m_BigBedCacheKey.empty())
            glyph_created = x_ProcessCachedBigBed();
        else if (!m_VcfCacheKey.empty()) 
            glyph_created = x_ProcessCachedVcfTabix();

        if (!glyph_created) {
            bool has_coverage_graph = GetGraphLevel() > 0;
            if (m_FeatLimit == 0  &&  has_coverage_graph) {
                // use coverage graph instead to inprove performance
                EJobState state = x_CreateFeatHistFromGraph(result, GetGraphLevel(), "");
                if (state != eCompleted) 
                    return state;
            } else {
                CFeat_CI feature_iter(m_Handle.GetScope(), *loc, m_Sel);
                int size = (int)feature_iter.GetSize();
                SetTaskTotal(size);
                if (feature_iter) {
                    // Get a more meaningful name
                    result->m_Desc = CSeqUtils::GetAnnotName(feature_iter->GetAnnot());
                }
                
                // use the real feature limit
                if (m_FeatLimit > 0) {
                    // we may want to adjust feature limit based on the
                    // actual screen size (in pixels) for this case
                    int new_limit = (int)(m_FeatLimit * m_Range.GetLength() / (1024 * m_Window));
                    //ERR_POST(Error << "FL adjusted from " << m_FeatLimit << " to " << new_limit << ",w:" << m_Window << ",r:" << m_Range.GetLength());
                    m_FeatLimit = max(m_FeatLimit, new_limit);
                } else if (m_FeatLimit == -1) {
                    // unlimited, but still set a upper limit to guard against problem.
                    m_FeatLimit = kMaxObjNum;
                }
                
                // based on different limit settings to create different glyphs
                if (m_FeatLimit == -2) {
                    // pack features into one line
                    SetTaskName("Packing features...");
                    EJobState state =
                        x_CreateFeatSmear(feature_iter, result, result->m_Desc);
                    if (state != eCompleted) 
                        return state;
                } else if (m_FeatLimit >= 0) {
                    EJobState state;
                    // in adaptive mode (m_FeatLimit != 0) and if number of feature is relatively small 
                    // return both hstogram and feat glyphs and let feature track decide what to show
                    // based on the height of resulting track
                    if (m_FeatLimit != 0 && size < m_FeatLimit) { //m_FeatLimit == 0 is packed mode
                        CSeqGlyph::TObjects objs;
                        auto state = x_LinkFeatures(feature_iter, objs);
                        if (state != eCompleted) 
                            return state;
                        result->m_ObjectList.swap(objs);
                        feature_iter.Rewind();
                    }
                    
                    // pack features into a histogram
                    if (has_coverage_graph) {
                        state = x_CreateFeatHistFromGraph(result,
                                                            GetGraphLevel(), result->m_Desc);
                    } else {
                        state = x_CreateFeatHist(feature_iter, result, result->m_Desc);
                    }
                    if (state != eCompleted) 
                        return state;

                } else {
                    //                    auto state = x_CreateFeatHist(feature_iter, result, result->m_Desc);
                    
                    CSeqGlyph::TObjects objs;
                    auto state = x_LinkFeatures(feature_iter, objs);
                    if (state != eCompleted) 
                        return state;
                    result->m_ObjectList.swap(objs);
                }
                SetTaskCompleted(size);
            }
        }
    } catch (CException& ex) {
        m_Error.Reset(new CAppJobError(ex.GetMsg()));
        return eFailed;
    } catch (std::exception& ex) {
        m_Error.Reset(new CAppJobError(ex.what()));
        return eFailed;
    }

    result->m_Token = m_Token;
    return eCompleted;
}


bool CSGFeatureJob::x_FilterFeature_recursive(
    CLinkedFeature::TLinkedFeats& features,
    CFeatureFilter& filter)
{
    bool passed = false;
    CLinkedFeature::TLinkedFeats::iterator p_iter = features.begin();
    while (p_iter != features.end()) {
        if (IsCanceled()) {
            return passed;
        }

        bool is_leaf = (*p_iter)->GetChildren().empty();

        // For features with child(ren), it will be treated as
        // passed if any of its children is passed.
        if ( (!is_leaf  &&  x_FilterFeature_recursive((*p_iter)->GetChildren(), filter)) ||
             filter.Pass(&(*p_iter)->GetMappedFeature())) {
                ++p_iter;
                passed = true;
        } else {
            p_iter = features.erase(p_iter);
        }
    }

    return passed;
}


void CSGFeatureJob::x_SortFeatures(
    CLinkedFeature::TLinkedFeats& feats,
    TFeatGroups& feat_groups)
{
    feat_groups.clear();
    if ( !m_Sorter ) {
        feat_groups.push_back(TFeatGroups::value_type(kUnsortedGroupName, feats));
        return;
    }

    TFeatGroupMap group_map;
    x_SortFeature_recursive(feats, group_map);
    NON_CONST_ITERATE (TFeatGroupMap, grp_iter, group_map) {
        feat_groups.push_back(
            TFeatGroups::value_type(m_Sorter->GroupIdxToName(grp_iter->first), grp_iter->second));
    }
}


void CSGFeatureJob::x_SortFeature_recursive(
    CLinkedFeature::TLinkedFeats& feats,
    TFeatGroupMap& group_map)
{
    CLinkedFeature::TLinkedFeats::iterator p_iter = feats.begin();
    while (p_iter != feats.end()) {
        if (IsCanceled()) {
            break;
        }

        bool is_leaf = (*p_iter)->GetChildren().empty();
        int idx = m_Sorter->GetGroupIdx((*p_iter)->GetMappedFeature());

        TFeatGroupMap child_group_map;
        if ( !is_leaf  &&  m_Sorter->ChildFeatureFirst() ) {
            // sort the child features first
            x_SortFeature_recursive((*p_iter)->GetChildren(), child_group_map);
        }

        size_t grp_num = child_group_map.size();
        if (grp_num > 1) {
            // Child features fall in different groups, we need to clone
            // the parent feature to go with each group of chilren
            NON_CONST_ITERATE (TFeatGroupMap, grp_iter, child_group_map) {
                CRef<CLinkedFeature> p_feat(new CLinkedFeature((*p_iter)->GetMappedFeature()));
                p_feat->GetChildren() = grp_iter->second;
                group_map[grp_iter->first].push_back(p_feat);
            }
            ++p_iter;
        } else if (idx > -1  ||  grp_num == 1) {
            if (grp_num == 1) {
                // only one group of children, using child's group idx
                idx = child_group_map.begin()->first;
            }
            // else {
                // there is no child or no child falls in any target group,
                // and the parent feature (this feature) is good.
            // }
            group_map[idx].push_back(*p_iter);
            ++p_iter;
        } else {
            // unwanted feature, remove it
            p_iter = feats.erase(p_iter);
        }
    }
}


IAppJob::EJobState
CSGFeatureJob::x_CreateFeature_Recursive(CLinkedFeature::TLinkedFeats& features,
                                         CLayoutGroup* group,
                                         int Level)
{
    group->SetLevel(Level);

    bool need_group = features.size() > 1;
    NON_CONST_ITERATE (CLinkedFeature::TLinkedFeats, iter, features) {
        if (IsCanceled()) return eCanceled;

        CRef<CSeqGlyph> glyph = x_CreateFeature1((*iter)->GetMappedFeature());
        glyph->SetLevel(Level);
        if ( !glyph ) continue;

        size_t num_children = (*iter)->GetChildren().size();
        EJobState state = eCompleted;

        // cut-off more than kMaxChildNum children, modifying the last glyph to show that cut-off
        if(num_children >= kMaxChildNum) {
            (*iter)->GetChildren().resize(kMaxChildNum);
            glyph->SetTearlineText(" (" + NStr::NumericToString(num_children - kMaxChildNum) + " features not shown)");
        }
        if (num_children == 0 ) {
            group->PushBack(glyph);
            if(Level == 0) {
                group->SetAsMaster(glyph);
            }
        } else if (num_children == 1  &&  m_LinkedFeat == ELinkedFeatDisplay::eLFD_ParentHidden) {
            // skip the parent feature (the created glyph)
            state = x_CreateFeature_Recursive((*iter)->GetChildren(), group, Level + 1);
        } else {
            CLayoutGroup* p_group = group;
            if (need_group) {
                CLayoutGroup* c_group = new CLayoutGroup(m_LinkedFeat);
                p_group->PushBack(c_group);
                p_group = c_group;
            }
            if (m_LinkedFeat != ELinkedFeatDisplay::eLFD_ParentHidden) {
                p_group->PushBack(glyph);
                if(Level == 0) {
                    p_group->SetAsMaster(glyph);
                }
            }

            if (num_children > 1 &&  m_LinkedFeat != ELinkedFeatDisplay::eLFD_Expandable) {
                CLayoutGroup* cc_group = new CLayoutGroup(m_LinkedFeat);
                p_group->PushBack(cc_group);
                p_group = cc_group;
            }

            state = x_CreateFeature_Recursive((*iter)->GetChildren(), p_group, Level + 1);
        }
        if (state != eCompleted) {
            return state;
        }

        AddTaskCompleted(1);
    }
    return eCompleted;
}


IAppJob::EJobState CSGFeatureJob::x_CreateFeatures(CFeat_CI& feature_iter,
                                                   CSeqGlyph::TObjects& objs)
{
    SetTaskName("Creating feature glyphs...");
    SetTaskTotal((int)feature_iter.GetSize());
    SetTaskCompleted(0);
    for (;  feature_iter ;  ++feature_iter) {
        if (IsCanceled()) {
            return eCanceled;
        }
        CRef<CSeqGlyph> glyph = x_CreateFeature1(*feature_iter);
        if (glyph) {
            objs.push_back(glyph);
        }
        AddTaskCompleted(1);
    }
    return eCompleted;
}


static 
size_t s_GetCDFrame(const CMappedFeat& feat)
{
    size_t frame = 0;
    _ASSERT(feat.GetData().Which() == CSeqFeatData::e_Cdregion);
    // This is exactly as it done in Sequin.
    const CCdregion& cdr = feat.GetData().GetCdregion();
    if (cdr.IsSetFrame()) {
        switch (cdr.GetFrame()) {
        case CCdregion::eFrame_one:
            frame = 0;
            break;
        case CCdregion::eFrame_two:
            frame = 1;
            break;
        case CCdregion::eFrame_three:
            frame = 2;
            break;
        default:
            break;
        }
    }
    return frame;
}


CRef<CSeqGlyph> CSGFeatureJob::x_CreateFeature1(const CMappedFeat& feat,
    CSeqGlyph* parent_glyph /*=NULL*/,
    const char* label_prefix /*=NULL*/)
{
    CConstRef<CSeq_loc> loc;
    if (m_Mapper) {
        try {
            loc = m_Mapper->Map(feat.GetLocation());
        } catch (std::exception&) {
            // ignore loc mapping exception.
        }
    } else {
        const CSeq_loc& loc_obj = feat.GetLocation();
        if (loc_obj.GetId()) {
            loc.Reset(&loc_obj);
        } else {
            // Very likely, we get a seq-loc with multiple seq-ids.
            // Extract the parts that matches with the viewed sequence
            loc = CSeqUtils::MixLocToLoc(loc_obj, m_Handle);
        }
    }

    CRef<CSeqGlyph> glyph;
    if (loc) {
        if (loc->Which() != CSeq_loc::e_Null  &&
            loc->Which() != CSeq_loc::e_Empty) {
            glyph = x_CreateFeature2(feat, *loc, label_prefix);

            // Try to find out if there is more accurate mapping
            // information available than the feature location itself.
            // For some features, such as RNA and CDS features, the
            // feature location might not be able to give the accureate mapping
            // between the RNA/CDS product sequence and the viewed sequence.
            // The inaccuracy prevent us from accurately showing the ruler
            // for the product sequence on the feature and mapping the
            // protein sequence or translated sequence to the feature intervals.
            // For performance consideration, we only do this when:
            //   - the zoom level is at the sequence level,
            //   - the feature has product sequence set,
            //   - the feature is not projected from other sequences, and
            //   - the feature is RNA or CDS feature.
            if (glyph  &&  m_Window < kMinScaleForMapping && !m_Mapper && CSeqUtils::IsSameStrands(*loc)) {

                CSeqUtils::TMappingInfo info;
                CSeqFeatData::E_Choice type = feat.GetData().Which();
                CSeqFeatData::ESubtype subtype = feat.GetData().GetSubtype();
                if (feat.IsSetProduct()) {
                    const CSeq_loc& product = feat.GetProduct();
                    const CSeq_id& product_id = *product.GetId();

                    if (feat.GetData().IsRna()) {
                        info = CSeqUtils::GetRnaMappingInfo(*loc, feat, m_Handle);
                    } else if (type == CSeqFeatData::e_Cdregion) {
                        if (nullptr != parent_glyph) {
                            // For CDS feature, we do this only when:
                            //   - the parent feature is a RNA feature, and
                            //   - the parent has mapping info set
                            CFeatGlyph* parent_feat = dynamic_cast<CFeatGlyph*>(parent_glyph);
                            if (parent_feat  &&  !parent_feat->GetMappingInfo().empty() &&
                                parent_feat->GetFeature().GetData().IsRna() && parent_feat->GetFeature().IsSetProduct()) {

                                info = CSeqUtils::GetCdsMappingInfoFromRna(parent_feat->GetMappingInfo(),
                                                                           *parent_feat->GetFeature().GetProduct().GetId(),
                                                                           *loc, product, m_Handle.GetScope());
                            }
							else {
								info = CSeqUtils::GetMappingInfoFromLocation(*loc, product_id);
							}
                        }
                        else {
                            info = CSeqUtils::GetMappingInfoFromLocation(*loc, product_id);
                        }
                    }
                } else { 
                    // no product 
                    if ((type == CSeqFeatData::e_Cdregion) || (subtype == CSeqFeatData::eSubtype_V_segment) || feat.GetData().IsRna()) {
                        static unsigned id_num{ 0 };
                        CRef<CSeq_id> prod_id(new CSeq_id("lcl|pseudo" + NStr::IntToString(++id_num)));
                        int offset = (type == CSeqFeatData::e_Cdregion) ? s_GetCDFrame(feat) : 0;
                        info = CSeqUtils::GetMappingInfoFromLocation(*loc,*prod_id, offset);
                    }
                }

                if ( !info.empty() ) {
                    CFeatGlyph* feat_glyph = dynamic_cast<CFeatGlyph*>(glyph.GetPointer());
                    if (feat_glyph) {
                        feat_glyph->SetMappingInfo(info);
                    }
                }
            }
        }
    } else {
        LOG_POST(Error << "CSGFeatureJob::x_CreateFeature1()"
            << "Failed to get seq-loc with unique seq-id!");
    }

    return glyph;
}


CRef<CSeqGlyph>
CSGFeatureJob::x_CreateFeature2(const CMappedFeat& feat,
                                const CSeq_loc& loc,
                                const char* label_prefix /*=NULL*/)
{
    const CSeqFeatData& data = feat.GetData();
    if (data.GetSubtype() == CSeqFeatData::eSubtype_user) {
        const string& object_type = data.GetUser().GetType().GetStr();
        if (object_type == "clone placement") {
            return x_CreateCloneFeature(feat, loc);
        }
    }

    if (data.GetSubtype() == CSeqFeatData::eSubtype_clone) {
        return x_CreateCloneFeature(feat, loc);
    }

    CRef<CSeqGlyph> glyph;
    CFeatGlyph* feat_glyph = new CFeatGlyph(feat, loc, m_LinkedFeat);
//    cerr << "CFeatGlyph created: " << (void *) feat_glyph << endl;
    feat_glyph->SetFilter(m_Filter);
    if (label_prefix) {
        feat_glyph->SetTopLabelPrefix(label_prefix);
    }
    glyph.Reset(feat_glyph);
    x_CalcIntervals(*feat_glyph);
    return glyph;
}


CRef<CSeqGlyph>
CSGFeatureJob::x_CreateCloneFeature(const CMappedFeat& feat,
                                    const CSeq_loc& loc)
{
    CRef<CSeqGlyph> glyph;
    CClonePlacementGlyph* cp = new CClonePlacementGlyph(feat, loc);
    glyph.Reset(cp);
    CClonePlacementGlyph::TIntervals& intervals = cp->SetIntervals();
    if (feat.GetLocation().IsMix()) {
        const CSeq_loc_mix::Tdata& seq_locs = feat.GetLocation().GetMix().Get();
        ITERATE (CSeq_loc_mix::Tdata, iter, seq_locs) {
            intervals.push_back((*iter)->GetTotalRange());
        }
    } else {
        intervals.push_back(feat.GetLocation().GetTotalRange());
    }
    return glyph;
}



IAppJob::EJobState CSGFeatureJob::x_CreateFeatHist(CFeat_CI& feat_iter,
                                                   CSGJobResult* result,
                                                   const string& title)
{
    if ( !feat_iter ) return eCompleted;


    SetTaskName("Packing features...");
    CSeqFeatData::ESubtype subtype =
        feat_iter->GetOriginalFeature().GetData().GetSubtype();
    CHistogramGlyph::TMap the_map(m_Range.GetFrom(), m_Range.GetTo(), (float)m_Window);

    CFeatureFilter filter;
    filter.Init(m_Filter);
    for (;  feat_iter ;  ++feat_iter) {
        if (IsCanceled()) {
            return eCanceled;
        }
        if ( !filter.NeedFiltering()  ||  filter.Pass(&(*feat_iter)) ) {
            try {
                the_map.AddRange(feat_iter->GetLocation().GetTotalRange(), 1, false);
            } catch (CException&) {
                /// skip the feature with mix location and multiple seq-ids.
            }
        }
        AddTaskCompleted(1);
    }
    result->m_ExtraObj.Reset(new CHistogramGlyph(the_map, subtype, title));
    return eCompleted;
}


IAppJob::EJobState
CSGFeatureJob::x_CreateFeatHistFromGraph(CSGJobResult* result,
                                         int level,
                                         const string& title)
{
    CSeqGlyph::TObjects glyphs;
    TAxisLimits tal;
    tal.min = 0.0;
    tal.max = 0.0;
    EJobState state = x_LoadCoverageGraph(glyphs, level, m_Window, title, false, tal);
    if (state == eCompleted  &&  !glyphs.empty()) {
        _ASSERT(glyphs.size() == 1);
        result->m_ExtraObj = glyphs.front();
    }
    return state;
}


IAppJob::EJobState CSGFeatureJob::x_CreateFeatSmear(CFeat_CI& feat_iter,
                                                    CSGJobResult* result,
                                                    const string& title)
{
    if ( !feat_iter ) return eCompleted;
    
    CHistogramGlyph::TMaps the_maps;
    for (;  feat_iter ;  ++feat_iter) {
        if (IsCanceled()) {
            return eCanceled;
        }

        size_t idx =
            CFeatGlyph::GetCustomColorIdx(feat_iter->GetOriginalFeature());
        string idx_str = NStr::IntToString((int)idx);
        CHistogramGlyph::TMaps::iterator iter = the_maps.find(idx_str);
        if (iter == the_maps.end()) {
            iter = the_maps.insert(
                CHistogramGlyph::TMaps::value_type(
                    idx_str,
                    CHistogramGlyph::TMap(
                        m_Range.GetFrom(),
                        m_Range.GetTo(),
                        (float)m_Window))).first;
        }

        try {
            iter->second.AddRange(
                feat_iter->GetLocation().GetTotalRange(), 1, false);
        } catch (CException&) {
            /// skip the feature with mix location and multiple seq-ids.
        }
        AddTaskCompleted(1);
    }

    if ( !the_maps.empty() ) {
        CRef<CHistogramGlyph> histogram(new CHistogramGlyph(the_maps, title));
        result->m_ExtraObj.Reset(histogram.GetPointer());
    }

    return eCompleted;
}


void CSGFeatureJob::x_CalcIntervals(CFeatGlyph& feat)
{
    CFeatGlyph::TIntervals& intervals = feat.SetIntervals();
    const CSeq_loc& loc = feat.GetLocation();
    intervals.clear();
    bool multi_ids = loc.IsMix()  &&  loc.GetId() == NULL;
    CSeq_loc_CI iter(loc);
    if (multi_ids) {
        if (m_Handle) {
            for ( ;  iter;  ++iter) {
                if (m_Handle.IsSynonym(iter.GetSeq_id())) {
                    intervals.push_back(iter.GetRange());
                }
            }
        }
    } else {
        bool rsite = false;
        if (feat.GetFeature().CanGetData())
            rsite = CSeqFeatData::eSubtype_rsite == feat.GetFeature().GetData().GetSubtype();
        TSeqRange rsiteRange;
        bool rsiteRangeFromSet = false;
        for ( ;  iter;  ++iter) {
            if (rsite) { // Assemble one range from the two points of the restriction site
                CConstRef<CSeq_loc> curr_loc = iter.GetRangeAsSeq_loc();

                if (curr_loc->IsPnt() && curr_loc->GetPnt().CanGetFuzz()) {
                    const CSeq_point_Base::TFuzz &fuzz = curr_loc->GetPnt().GetFuzz();
                    if (fuzz.IsLim() && (CInt_fuzz_Base::eLim_tl == fuzz.GetLim())) {
                        if (!rsiteRangeFromSet) {
                            rsiteRange.SetFrom(curr_loc->GetPnt().GetPoint());
                            rsiteRangeFromSet = true;
                        }
                        else {
                            rsiteRange.SetToOpen(curr_loc->GetPnt().GetPoint());
                            intervals.push_back(rsiteRange);
                            rsiteRangeFromSet = false;
                        }
                    }
                    continue;
                }
            }
            intervals.push_back(iter.GetRange());
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CGeneModelFeatJob
///////////////////////////////////////////////////////////////////////////////
CGeneModelFeatJob::
CGeneModelFeatJob(const string& desc, CBioseq_Handle handle,
        const SAnnotSelector& lm_feat_sel,
        const SAnnotSelector& m_feat_sel,
        TJobToken lm_token, TJobToken main_token,
        int merge_style, bool landmark_feat, int min_overview_feat,
        const TSeqRange& range, TModelUnit window,
        int max_feat, bool show_histogram, ICoordMapper* mapper,
        int highlight_mode)
        : CSGFeatureJob(desc, handle, lm_feat_sel, range,
                        window, max_feat, ELinkedFeatDisplay::eLFD_Default, lm_token, mapper)
        , m_MainFeatSel(m_feat_sel)
        , m_MainFeatToken(main_token)
        , m_MergeStyle(merge_style)
        , m_LandmarkFeats(landmark_feat)
        , m_MinOverviewFeat(min_overview_feat)
        , m_ShowHistogram(show_histogram)
        , m_HighlightMode(highlight_mode)
{}


IAppJob::EJobState CGeneModelFeatJob::x_Execute()
{
    SetTaskName("Loading features...");
    CSGJobResult* result = new CSGJobResult();
    m_Result.Reset(result);

    try {
        CConstRef<CSeq_loc> loc(
            m_Handle.GetRangeSeq_loc(m_Range.GetFrom(), m_Range.GetTo()) );
    //    cerr << "Features for " << loc->GetId()->GetSeqIdString(true) << endl;
//    cerr << "using loc " << endl << 
//            MSerial_AsnText << *loc << endl;
       CSeqGlyph::TObjects tmp;
        SAnnotSelector sel;
        if (m_LandmarkFeats) {
            sel = m_Sel;
            result->m_Token = m_Token;
        } else {
            sel = m_MainFeatSel;
            result->m_Token = m_MainFeatToken;
        }

        CFeat_CI feature_iter(m_Handle.GetScope(), *loc, sel);

        // if the visible genes are less than a certain number, we change the mode
        // to show all main features.
        if (m_LandmarkFeats  &&
            feature_iter.GetSize() < (size_t)m_MinOverviewFeat) {
            feature_iter = CFeat_CI(m_Handle.GetScope(), *loc, m_MainFeatSel);
            result->m_Token = m_MainFeatToken;
            m_LandmarkFeats = false;
        }

        // linking the features if not only landmark features are shown
        if (m_LandmarkFeats) {
            TFilterType f_type = x_FilterStrToValue(m_Filter);
            switch (f_type) {
                case eFilter_CCDS:
                    sel.IncludeFeatType(CSeqFeatData::e_Gene);
                    sel.IncludeFeatType(CSeqFeatData::e_Cdregion);
                    feature_iter = CFeat_CI(m_Handle.GetScope(), *loc, sel);
                    break;
                case eFilter_ncRNAs:
                case eFilter_dbref:
                    sel.IncludeFeatType(CSeqFeatData::e_Gene);
                    sel.IncludeFeatType(CSeqFeatData::e_Rna);
                    sel.IncludeFeatType(CSeqFeatData::e_Cdregion);
                    feature_iter = CFeat_CI(m_Handle.GetScope(), *loc, sel);
                    break;
                case eFilter_npcGenes:
                    sel.IncludeFeatType(CSeqFeatData::e_Gene);
                    sel.IncludeFeatType(CSeqFeatData::e_Cdregion);
                    feature_iter = CFeat_CI(m_Handle.GetScope(), *loc, sel);
                    break;
                default:
                    break;
            }

            EJobState state;
            if (f_type != eFilter_invalid) {
                state = x_CreateFeaturesWithFilter(feature_iter, tmp, f_type);
            } else {
                state = x_CreateFeatures(feature_iter, tmp);
            }
            if (state != eCompleted) {
                return state;
            }

            // if feature number is more than the limit, we need to create a gene
            // distribution histogram and pick the most significant ones.
            if(m_FeatLimit > -1) {
                state = x_ProcessLandMarkFeatures(tmp, result->m_ExtraObj, result->m_CommentObjs);
                if (state != eCompleted) {
                    return state;
                }
            }
        } else {

            CLinkedFeature::TLinkedFeats features;
//            cerr << "got features: " << feature_iter.GetSize() << endl;
            for ( ;  feature_iter;  ++feature_iter) {
                const CMappedFeat& feat = *feature_iter;
//        cerr << "Got feature" << endl << 
//          MSerial_AsnText << feat.GetOriginalFeature() << endl;
                CRef<CLinkedFeature> fref( new CLinkedFeature(feat) );
                features.push_back(fref);
            }

            if ( !CSeqUtils::LinkFeatures(features,
                feature::CFeatTree::eFeatId_by_type, this) ) {
                return eCanceled;
            }

            EJobState state = x_CreateGeneModels(features, tmp);
            if (state != eCompleted) {
                return state;
            }
        }
        result->m_ObjectList.swap(tmp);
    } catch (CException& ex) {
        m_Error.Reset(new CAppJobError(ex.GetMsg()));
        return eFailed;
    } catch (std::exception& ex) {
        m_Error.Reset(new CAppJobError(ex.what()));
        return eFailed;
    }

    return eCompleted;
}



CRef<CSeqGlyph> CGeneModelFeatJob::x_CreateFeature2(
    const CMappedFeat& feat, const CSeq_loc& loc, const char* label_prefix /*= NULL*/)
{
    CFeatGlyph* feat_glyph;
    if (feat.GetData().IsCdregion()) {
        feat_glyph = new CCdsGlyph(feat, loc);
    } else {
        feat_glyph = new CFeatGlyph(feat, loc, m_LinkedFeat);
    }
    if (label_prefix) {
        feat_glyph->SetTopLabelPrefix(label_prefix);
    }
    CRef<CSeqGlyph> glyph(feat_glyph);
    x_CalcIntervals(*feat_glyph);
    return glyph;
}


IAppJob::EJobState
CGeneModelFeatJob::x_CreateGeneModels(CLinkedFeature::TLinkedFeats& feats,
                                      CSeqGlyph::TObjects& objs)
{
    SetTaskName("Creating feature glyphs and gene models...");
    SetTaskTotal((int)feats.size());
    SetTaskCompleted(0);

    TFilterType f_type = x_FilterStrToValue(m_Filter);

    // Create glyph for each feature, and put the linked features
    // into right group according to the feature merging style
    NON_CONST_ITERATE (CLinkedFeature::TLinkedFeats, iter, feats) {
        AddTaskCompleted(1);
        if (IsCanceled()) return eCanceled;

        bool skip = false;
        switch (f_type) {
            case eFilter_dbref:
                if ( !x_IsDbref(m_Filter, **iter) ) {
                    skip = true;
                }
                break;
            case eFilter_CCDS:
                if ( !x_IsCCDS(**iter) ) {
                    skip = true;
                }
                break;
            case eFilter_pseudo_genes:
                if ( !x_IsPseudoGene((*iter)->GetFeature()) ) {
                    skip = true;
                }
                break;
            case eFilter_ncRNAs:
                if ( !x_IsncRNA(**iter) ) {
                    skip = true;
                }
                break;
            case eFilter_npcGenes:
                if ( !x_IsnpcGene(**iter) ) {
                    skip = true;
                }
                break;
            default:
                break;
        }

        if (skip) continue;

        CRef<CSeqGlyph> glyph = x_CreateFeature1((*iter)->GetMappedFeature());
        if ( !glyph ) continue;

        if ((*iter)->GetChildren().empty()) {
            objs.push_back(glyph);
            continue;
        }
        CSeqFeatData::ESubtype subtype =
            (*iter)->GetFeature().GetData().GetSubtype();
        if (subtype == CSeqFeatData::eSubtype_gene) {
            CGeneGroup* group = new CGeneGroup();
            objs.push_back(CRef<CSeqGlyph>(group));
            group->PushBack(glyph);
            CRef<CLayoutGroup> misc_group(new CLayoutGroup);
            CRef<CLinkedFeatsGroup> rnacds_group(new CLinkedFeatsGroup);
            CRef<CLayoutGroup> exon_group;
            string label_prefix;
            feature::GetLabel((*iter)->GetFeature(), &label_prefix, feature::fFGL_Content);

            NON_CONST_ITERATE (CLinkedFeature::TLinkedFeats,
                g_iter, (*iter)->GetChildren()) {
                CRef<CSeqGlyph> g_glyph =
                    x_CreateFeature1((*g_iter)->GetMappedFeature(), 0, label_prefix.c_str());
                if ( !g_glyph ) continue;

                CSeqFeatData::E_Choice s_type =
                    (*g_iter)->GetFeature().GetData().Which();
                CSeqFeatData::ESubtype s_subtype =
                    (*g_iter)->GetFeature().GetData().GetSubtype();
                if (s_subtype == CSeqFeatData::eSubtype_exon) {
                    if ( !exon_group ) {
                        exon_group.Reset(new CLayoutGroup);
                    }
                    exon_group->PushBack(g_glyph);
                } else if (s_subtype  == CSeqFeatData::eSubtype_cdregion) {
                    rnacds_group->PushBack(g_glyph);
                }
                else if ((s_type != CSeqFeatData::e_Rna && s_subtype != CSeqFeatData::eSubtype_misc_RNA) 
                    && (s_subtype != CSeqFeatData::eSubtype_V_segment) && (s_subtype != CSeqFeatData::eSubtype_D_segment)
                    && (s_subtype != CSeqFeatData::eSubtype_J_segment) && (s_subtype != CSeqFeatData::eSubtype_C_region)) {
                    misc_group->PushBack(g_glyph);
                } else { /// must be RNA
                    if (m_MergeStyle == CGeneModelConfig::eMerge_Pairs  &&
                        !(*g_iter)->GetChildren().empty()) {
                            CLinkedFeatsGroup* rna_group = new CLinkedFeatsGroup();
                            rnacds_group->PushBack(rna_group);
                            rna_group->PushBack(g_glyph);
                            NON_CONST_ITERATE (CLinkedFeature::TLinkedFeats,
                                r_iter, (*g_iter)->GetChildren()) {
                                CRef<CSeqGlyph> r_glyph =
                                    x_CreateFeature1((*r_iter)->GetMappedFeature(), g_glyph.GetPointer(), label_prefix.c_str());
                                if (r_glyph) {
                                    rna_group->PushBack(r_glyph);
                                }
                            }
                    } else { // eMerge_All, eMerge_No, eMerge_OneLine, and eMerge_Pairs when no CDSs present
                        rnacds_group->PushBack(g_glyph);
                        NON_CONST_ITERATE (CLinkedFeature::TLinkedFeats,
                            r_iter, (*g_iter)->GetChildren()) {
                            CRef<CSeqGlyph> r_glyph =
                                x_CreateFeature1((*r_iter)->GetMappedFeature(), g_glyph.GetPointer(), label_prefix.c_str());
                            if (r_glyph) {
                                rnacds_group->PushBack(r_glyph);
                            }
                        }
                    }
                }
            }
            if (misc_group->GetChildren().size() &&
                m_MergeStyle != CGeneModelConfig::eMerge_OneLine)
            {
                if (misc_group->GetChildren().size() > 1) {
                    group->PushBack(misc_group);
                } else {
                    group->Append(misc_group->SetChildren());
                }
            }
            if (rnacds_group->GetChildren().size()) {
                if (rnacds_group->GetChildren().size() == 1 ||
                    m_MergeStyle == CGeneModelConfig::eMerge_No  ||
                    m_MergeStyle == CGeneModelConfig::eMerge_Pairs)
                {
                    // Merge group children, dissolve the group
                    group->Append(rnacds_group->SetChildren());
                } else {
                    // Add as a group
                    group->PushBack(rnacds_group);
                }
            }
            if (exon_group  &&
                (m_MergeStyle != CGeneModelConfig::eMerge_OneLine  ||
                 rnacds_group->GetChildren().size() == 0))
            {
                if (exon_group->GetChildren().size() > 1) {
                    group->PushBack(exon_group);
                } else {
                    group->Append(exon_group->SetChildren());
                }
            }

        } else { // must be RNA or other misc features: VDJ segments, C regions, and enhancers
            if ((*iter)->GetChildren().empty()) {
                // no need to put it into a group
                objs.push_back(glyph);
            } else {
                CLayoutGroup* group = NULL;
                if (m_MergeStyle == CGeneModelConfig::eMerge_No) {
                    group = new CLayoutGroup();
                    objs.push_back(CRef<CSeqGlyph>(group));
                } else {
                    CLinkedFeatsGroup* rc_group = new CLinkedFeatsGroup();
                    objs.push_back(CRef<CSeqGlyph>(rc_group));
                    group = &rc_group->SetGroup();
                }
                group->PushBack(glyph);
                NON_CONST_ITERATE (CLinkedFeature::TLinkedFeats, r_iter,
                    (*iter)->GetChildren()) {
                    CRef<CSeqGlyph> r_glyph =
                        x_CreateFeature1((*r_iter)->GetMappedFeature(), glyph.GetPointer());
                    if (r_glyph) {
                        group->PushBack(r_glyph);
                    }
                }
            }
        }
    }

    return eCompleted;
}


IAppJob::EJobState
CGeneModelFeatJob::x_CreateFeaturesWithFilter(CFeat_CI& feature_iter,
                                              CSeqGlyph::TObjects& objs,
                                              TFilterType filter)
{
    if (filter == eFilter_pseudo_genes) {
        SetTaskName("Creating feature glyphs...");
        SetTaskTotal((int)feature_iter.GetSize());
        SetTaskCompleted(0);
        for (;  feature_iter ;  ++feature_iter) {
            if (IsCanceled()) {
                return eCanceled;
            }

            AddTaskCompleted(1);
            if ( !x_IsPseudoGene(feature_iter->GetOriginalFeature()) ) {
                continue;
            }

            CRef<CSeqGlyph> glyph = x_CreateFeature1(*feature_iter);
            if (glyph) {
                objs.push_back(glyph);
            }
        }
    } else {
        CLinkedFeature::TLinkedFeats features;
        for ( ;  feature_iter;  ++feature_iter) {
            const CMappedFeat& feat = *feature_iter;
            CRef<CLinkedFeature> fref( new CLinkedFeature(feat) );
            features.push_back(fref);
        }
        if ( !CSeqUtils::LinkFeatures(features,
            feature::CFeatTree::eFeatId_by_type, this) ) {
            return eCanceled;
        }

        SetTaskName("Creating feature glyphs...");
        SetTaskTotal((int)features.size());
        SetTaskCompleted(0);
        NON_CONST_ITERATE (CLinkedFeature::TLinkedFeats, iter, features) {
            AddTaskCompleted(1);

            if ( !(*iter)->GetFeature().GetData().IsGene() ) continue;

            if (IsCanceled()) return eCanceled;

            bool skip = false;
            switch (filter) {
            case eFilter_dbref:
                if ( !x_IsDbref(m_Filter, **iter) ) {
                    skip = true;
                }
                break;
            case eFilter_CCDS:
                if ( !x_IsCCDS(**iter) ) {
                    skip = true;
                }
                break;
            case eFilter_ncRNAs:
                if ( !x_IsncRNA(**iter) ) {
                    skip = true;
                }
                break;
            case eFilter_npcGenes:
                if ( !x_IsnpcGene(**iter) ) {
                    skip = true;
                }
                break;
            default:
                break;
            }

            if (skip) continue;

            CRef<CSeqGlyph> glyph = x_CreateFeature1((*iter)->GetMappedFeature());
            if (glyph) {
                objs.push_back(glyph);
            }
        }
    }
    return eCompleted;
}


//float s_GetGeneDocsumWeight(const CEntrez2_docsum& ds)
//{
//    float score = 0;
//    try {
//        string val = ds.GetValue("GeneWeight");
//        score = (float)NStr::StringToDouble(val);
//    }
//    catch (CException&) {
//    }
//    return score;
//}


typedef map<int, int> TId2Wt;

static const char* kGeneByLocUrl =
"http://www.ncbi.nlm.nih.gov/gutils/fgenes_by_loc.fcgi?sort=weight&retmode=asnb";

bool s_GetGeneWeight(TId2Wt& genes,
    TGi gi, const TSeqRange& r, size_t retmax)
{
    STimeout timeout;
    timeout.sec = 4;
    timeout.usec = 0;

    string url = kGeneByLocUrl;
    url += "&gi=" + NStr::NumericToString(gi);
    url += "&retmax=" + NStr::NumericToString(retmax);
    url += "&start=" + NStr::NumericToString(r.GetFrom());
    url += "&stop=" + NStr::NumericToString(r.GetTo());
    CConn_HttpStream stream(url, fHTTP_AutoReconnect, &timeout);

    auto_ptr<CObjectIStream> obj_stream(CObjectIStream::Open(eSerial_AsnBinary, stream));
    try {
        CGBL_Reply gbl_reply;
        //NcbiStreamCopy(cout, stream);
        *obj_stream >> gbl_reply;
        if (gbl_reply.IsError()) {
            NCBI_THROW(CException, eUnknown, gbl_reply.GetError());
        }
        const CGBL_Data& data = gbl_reply.GetData();
        if (data.CanGetGenes()) {
            int weight = data.GetReturned();
            ITERATE (CGBL_Data::TGenes, iter, data.GetGenes()) {
                genes.insert(map<int, int>::value_type((*iter)->GetGeneid(), weight--));
            }
        }
    } catch (CException& e) {
        LOG_POST(Error << "Failed to retrieve genes by location for gi|"
                 << gi << ", error: " << e.GetMsg());
        return false;
    }

    return true;
}


IAppJob::EJobState CGeneModelFeatJob::x_ProcessLandMarkFeatures(
    CSeqGlyph::TObjects& objs, CRef<CSeqGlyph>& hist_obj, CSeqGlyph::TObjects& comment_objs)
{
    SetTaskName("Packing features...");
    SetTaskTotal((int)objs.size());
    SetTaskCompleted(0);


    vector<SLandmarkFeat> landmark_feats;

    ///
    /// determine scores for landmark features
    ///

#if 0
    ///
    /// option 1: brain-dead calculation
    ///
    NON_CONST_ITERATE(CSeqGlyph::TObjects, iter, objs) {
        if (m_StopRequested) {
            return eCanceled;
        }

        CSeqGlyph* tmp = *iter;
        CFeatGlyph* feat  = dynamic_cast<CFeatGlyph*>(tmp);

        TSeqRange range = tmp->GetRange();

        float score = (float)range.GetLength();
        string s;//s.erase();
        feature::GetLabel(feat->GetFeature(), &s, feature::fFGL_Content);
        score /= s.length();

        SLandmarkFeat f;
        f.score = score;
        f.obj = *iter;

        landmark_feats.push_back(f);
    }

    ///
    /// end option 1
    ///
#endif

#if 0
    /// option 2: use gene weights
    /// NB: this currently uses an ineffective method, in that it
    /// queries Entrez for the information on the fly
    vector<int> gene_uids;
    NON_CONST_ITERATE(CSeqGlyph::TObjects, iter, objs) {
        if (m_StopRequested) {
            return eCanceled;
        }

        CSeqGlyph* tmp = *iter;
        CFeatGlyph* feat  = dynamic_cast<CFeatGlyph*>(tmp);
        const CSeq_feat& seqfeat = feat->GetFeature();
        CConstRef<CDbtag> gene_tag = seqfeat.GetNamedDbxref("GeneID");
        if ( !gene_tag ) {
            gene_tag = seqfeat.GetNamedDbxref("LocusID");
        }
        if (gene_tag  &&  gene_tag->GetTag().IsId()) {
            gene_uids.push_back(gene_tag->GetTag().GetId());
        }
    }

    typedef map<int, float> TScoreMap;
    TScoreMap gene_scores;

    /// screen for already retrieved gene IDs
    CEntrezCache& cache = CEntrezCache::GetInstance();
    {{
        vector<int>::iterator iter = gene_uids.begin();
        string db("gene");
        for ( ;  iter != gene_uids.end();  ) {
            CConstRef<CEntrez2_docsum> ds = cache.Get(db, *iter);
            if (ds) {
                gene_scores[ds->GetUid()] =
                    s_GetGeneDocsumWeight(*ds);
                iter = gene_uids.erase(iter);
            } else {
                ++iter;
            }
        }
    }}

    /// retrieve any new gene IDs
    if (gene_uids.size()) {
        CEntrez2Client cli;
        CRef<CEntrez2_docsum_list> ds = cli.GetDocsums(gene_uids, "gene");
        if (ds  &&  ds->IsSetList()) {
            cache.Put("gene", *ds);
            ITERATE (CEntrez2_docsum_list::TList, iter, ds->GetList()) {
                gene_scores[(*iter)->GetUid()] =
                    s_GetGeneDocsumWeight(**iter);
            }
        }
    }

    NON_CONST_ITERATE(CSeqGlyph::TObjects, iter, objs) {
        if (m_StopRequested) {
            return eCanceled;
        }

        CSeqGlyph* tmp = *iter;
        CFeatGlyph* feat  = dynamic_cast<CFeatGlyph*>(tmp);
        const CSeq_feat& seqfeat = feat->GetFeature();
        float score = 0;

        CConstRef<CDbtag> gene_tag = seqfeat.GetNamedDbxref("GeneID");
        if ( !gene_tag ) {
            gene_tag = seqfeat.GetNamedDbxref("LocusID");
        }
        if (gene_tag  &&  gene_tag->GetTag().IsId()) {
            int uid = gene_tag->GetTag().GetId();
            TScoreMap::iterator it = gene_scores.find(uid);
            if (it != gene_scores.end()) {
                score = it->second;
            }
        }

        SLandmarkFeat f;
        f.score = score;
        f.obj = *iter;

        landmark_feats.push_back(f);
    }

    ///
    /// end option 2
    ///
#endif

    TId2Wt gene_weights;
#if 0
    // Gene weights are available for current live gis only
    if (m_Handle.GetState() == CBioseq_Handle::fState_none) {
        // try to get the sorted (by weight) gene list
        CSeq_id_Handle idh = m_Handle.GetSeq_id_Handle();
        idh = sequence::GetId(idh, m_Handle.GetScope(), sequence::eGetId_ForceGi);
        if (idh  &&  idh.GetGi() > 0) {
            TGi gi = idh.GetGi();
            int number_genes = m_FeatLimit > 0 ? m_FeatLimit : (int)objs.size();
            typedef map<int, int> TId2Wt; // gene id to gene weight map
            if (number_genes > 0) {
                s_GetGeneWeight(gene_weights, gi, m_Range, number_genes);
            }
        }
    }
#endif

#if 1
    ///
    /// option 4: use gene weights from user objects
    ///
    NON_CONST_ITERATE(CSeqGlyph::TObjects, iter, objs) {
        if (IsCanceled()) {
            return eCanceled;
        }

        CSeqGlyph* tmp = *iter;
        CFeatGlyph* feat  = dynamic_cast<CFeatGlyph*>(tmp);
        const CSeq_feat& seqfeat = feat->GetFeature();
        float score = 0.0f;

        if (gene_weights.empty()) {
// weight use removed per SV-4623
/*            if (seqfeat.IsSetExt()) {
                const CUser_object& obj = seqfeat.GetExt();
                CConstRef<CUser_field> field = obj.GetFieldRef("GeneWeight");
                if (field) {
                    if (field->GetData().IsInt()) {
                        score = (float)field->GetData().GetInt();
                    } else if (field->GetData().IsReal()) {
                        score = (float)field->GetData().GetReal();
                    }
                }
            }
*/
            if ( !score ) {
                string s;
                feature::GetLabel(feat->GetFeature(), &s, feature::fFGL_Content);
                TSeqRange range = tmp->GetRange();

                score = (float)range.GetLength();
                score /= s.length();
                score -= 1e6f;
            }
        } else if (CConstRef<CDbtag> tag = seqfeat.GetNamedDbxref("GeneID")) {
            if (tag->GetTag().IsId()) {
                TId2Wt::const_iterator g_iter = gene_weights.find(tag->GetTag().GetId());
                if (g_iter != gene_weights.end()) {
                    score = (float)g_iter->second;
                }
            }
        }

        SLandmarkFeat f;
        f.score = score;
        f.obj = *iter;

        landmark_feats.push_back(f);
    }

    ///
    /// end option 3
    ///
#endif

    if (landmark_feats.empty()) return eCompleted;

    ///
    ///
    ///
    /// sort and filter
    std::sort(landmark_feats.begin(), landmark_feats.end(),
        SLandmarkFeatByScore());

    // Add Gene Histogram when there are genes that are not shown
    if ((int)landmark_feats.size() > m_FeatLimit  &&  m_ShowHistogram) {
        const CFeatGlyph* feat =
            dynamic_cast<const CFeatGlyph*>(landmark_feats.front().obj.GetPointer());
        string title = CSeqUtils::GetAnnotName(feat->GetMappedFeature().GetAnnot());

        CHistogramGlyph::TMap the_map(
            m_Range.GetFrom(), m_Range.GetTo(), (float)m_Window);
        ITERATE (vector<SLandmarkFeat>, iter, landmark_feats) {
            if (IsCanceled()) {
                return eCanceled;
            }

            TSeqRange range = iter->obj->GetRange();
            the_map.AddRange(range);
            AddTaskCompleted(1);
        }

        hist_obj.Reset(new CHistogramGlyph(the_map, CSeqFeatData::eSubtype_gene, title));
    }

    size_t len = landmark_feats.size() - 1;
    vector<SLandmarkFeat>::iterator prev =
        landmark_feats.begin() + ((int)len < m_FeatLimit ? len : m_FeatLimit);
    vector<SLandmarkFeat>::iterator s_iter = prev;
    vector<SLandmarkFeat>::iterator end = landmark_feats.end();
    //for (++iter;  iter != end;  ++iter) {
    //    if (iter->score != prev->score) {
    //        break;
    //    }
    //    prev = iter;
    //}
    if (IsCanceled()) {
        return eCanceled;
    }

    landmark_feats.erase(s_iter, end);

    int highlight_num = (int)(10 * m_Range.GetLength() / (1400 * m_Window));
    if (highlight_num > 40) {
        highlight_num = 40;
    } else if (highlight_num < 10) {
        highlight_num = 10;
    }

    if (m_HighlightMode == 1) {
        // highlight the top most important genes
        s_iter = landmark_feats.begin();
        while (highlight_num > 0  &&  s_iter != landmark_feats.end()) {
            s_iter->obj->SetHighlighted(true);
            --highlight_num;
            ++s_iter;
        }
    } else if (m_HighlightMode == 2) {
        // render label and feature independently
        s_iter = landmark_feats.begin();
        while (highlight_num > 0  &&  s_iter != landmark_feats.end()) {
            CFeatGlyph* feat  = dynamic_cast<CFeatGlyph*>(s_iter->obj.GetPointer());
            string label;
            feat->GetLabel(label, CLabel::eContent);
            CCommentGlyph* comment = new CCommentGlyph(label, s_iter->obj);
            CRef<CSeqGlyph> glyph(comment);
            TSeqRange range = feat->GetRange();
            comment->SetTargetPos(TModelPoint((range.GetFrom() + range.GetTo()) * 0.5, 0.0));
            comment_objs.push_back(glyph);
            --highlight_num;
            ++s_iter;
        }
    }

    std::sort(landmark_feats.begin(), landmark_feats.end(),
        SLandmarkFeatBySeqPos());

    if (IsCanceled()) {
        return eCanceled;
    }

    objs.clear();
    ITERATE (vector<SLandmarkFeat>, l_iter, landmark_feats) {
        objs.push_back(l_iter->obj);
    }

    return eCompleted;
}


vector<string> CGeneModelFeatJob::GetFilters(const CBioseq_Handle& handle,
                                             const TSeqRange& range,
                                             SAnnotSelector& sel,
                                             const string& filter)
{
    vector<string> filters;
    TFilterType f_type = x_FilterStrToValue(filter);
    switch (f_type) {
        case eFilter_dbref_all:
            sel.SetFeatType(CSeqFeatData::e_Gene);
            break;
        case eFilter_dbref:
        {{
            sel.IncludeFeatType(CSeqFeatData::e_Gene);
            sel.IncludeFeatType(CSeqFeatData::e_Rna);
            sel.IncludeFeatType(CSeqFeatData::e_Cdregion);
            CFeat_CI feat_iter(handle, range, sel);
            while (feat_iter) {
                if (x_IsDbref(filter, feat_iter->GetOriginalFeature())) {
                    filters.push_back(filter);
                    break;
                }
                ++feat_iter;
            }
            break;
        }}
        case eFilter_CCDS:
        {{
            sel.SetFeatType(CSeqFeatData::e_Cdregion);
            CFeat_CI feat_iter(handle, range, sel);
            while (feat_iter) {
                if (x_IsDbref(filter, feat_iter->GetOriginalFeature())) {
                    filters.push_back(filter);
                    break;
                }
                ++feat_iter;
            }
            break;
        }}
        case eFilter_pseudo_genes:
        {{
            sel.SetFeatType(CSeqFeatData::e_Gene);
            CFeat_CI feat_iter(handle, range, sel);
            while (feat_iter) {
                if (x_IsPseudoGene(feat_iter->GetOriginalFeature())) {
                    filters.push_back(filter);
                    break;
                }
                ++feat_iter;
            }
            break;
        }}
        case eFilter_ncRNAs:
        {{
            sel.IncludeFeatType(CSeqFeatData::e_Gene);
            sel.IncludeFeatType(CSeqFeatData::e_Rna);
            sel.IncludeFeatType(CSeqFeatData::e_Cdregion);
            CFeat_CI feat_iter(handle, range, sel);
            CLinkedFeature::TLinkedFeats features;
            for (;  feat_iter;  ++feat_iter) {
                const CMappedFeat& feat = *feat_iter;
                CRef<CLinkedFeature> fref( new CLinkedFeature(feat) );
                features.push_back(fref);
            }
            CSeqUtils::LinkFeatures(features,
                feature::CFeatTree::eFeatId_by_type, NULL);
            ITERATE (CLinkedFeature::TLinkedFeats, iter, features) {
                if (x_IsncRNA(**iter)) {
                    filters.push_back(filter);
                    break;
                }
            }
            break;
        }}
        case eFilter_npcGenes:
        {{
            sel.IncludeFeatType(CSeqFeatData::e_Gene);
            sel.IncludeFeatType(CSeqFeatData::e_Cdregion);
            CFeat_CI feat_iter(handle, range, sel);
            CLinkedFeature::TLinkedFeats features;
            for (;  feat_iter;  ++feat_iter) {
                const CMappedFeat& feat = *feat_iter;
                CRef<CLinkedFeature> fref( new CLinkedFeature(feat) );
                features.push_back(fref);
            }
            CSeqUtils::LinkFeatures(features,
                feature::CFeatTree::eFeatId_by_type, NULL);
            ITERATE (CLinkedFeature::TLinkedFeats, iter, features) {
                if (x_IsnpcGene(**iter)) {
                    filters.push_back(filter);
                    break;
                }
            }
            break;
        }}
        default:
            break;
    }
    return filters;
}


CGeneModelFeatJob::TFilterType
CGeneModelFeatJob::x_FilterStrToValue(const string& filter)
{
    /// anything unrecognizable will be treated as dbref
    TFilterType type = eFilter_invalid;
    if (NStr::EqualNocase(filter, "dbref")) {
        type = eFilter_dbref_all;
    } else if (NStr::EqualNocase(filter, "CCDS")) {
        type = eFilter_CCDS;
    } else if (NStr::EqualNocase(filter, "PseudoGenes")) {
        type = eFilter_pseudo_genes;
    } else if (NStr::EqualNocase(filter, "ncRNAs")) {
        type = eFilter_ncRNAs;
    } else if (NStr::EqualNocase(filter, "npcGenes")) {
        type = eFilter_npcGenes;
    } else if ( !filter.empty() ) {
        type = eFilter_dbref;
    }

    return type;
}


///////////////////////////////////////////////////////////////////////////////
/// CBatchFeatJob
///////////////////////////////////////////////////////////////////////////////
CBatchFeatJob::CBatchFeatJob(const string& desc,
                             CBioseq_Handle handle,
                             const TSeqRange& range, TModelUnit window,
                             TJobToken token, const TFeatBatchJobRequests& requests)
    : CSGFeatureJob(desc, handle, SAnnotSelector(), range, window, token)
    , m_Requests(requests)
{
}


IAppJob::EJobState CBatchFeatJob::x_Execute()
{
    try {
        CBatchJobResult* result = new CBatchJobResult();
        m_Result.Reset(result);
        result->m_Token = m_Token;

        NON_CONST_ITERATE(TFeatBatchJobRequests, iter, m_Requests) {
            if (IsCanceled()) {
                return eCanceled;
            }
            CRef<CSGJobResult> single_res =
                x_LoadFeatProducts(iter->m_Feat, iter->m_Sel);
            if (single_res) {
                result->m_Results.push_back(single_res);
            }
        }
    } catch (CException& ex) {
        m_Error.Reset(new CAppJobError(ex.GetMsg()));
        return eFailed;
    } catch (std::exception& ex) {
        m_Error.Reset(new CAppJobError(ex.what()));
        return eFailed;
    }

    return eCompleted;
}


CRef<CSGJobResult>
CBatchFeatJob::x_LoadFeatProducts(CRef<CFeatGlyph> feat,
                                  const SAnnotSelector& sel)
{
    CRef<CSGJobResult> result;
    CSeqGlyph::TObjects tmp;
    CRef<CSeqGlyph> dmap;

    const CSeq_loc& loc     = feat->GetLocation();
    const CSeq_loc& product = feat->GetFeature().GetProduct();
    try {
        // map visible range to product feature
        CSeq_loc_Mapper prot_range_mapper(loc, product, &GetScope());

        TSeqRange range(m_Range);
        range.IntersectWith(loc.GetTotalRange());
        CRef<CSeq_loc> tmp_loc(new CSeq_loc());
        tmp_loc->SetInt().SetFrom(range.GetFrom());
        tmp_loc->SetInt().SetTo  (range.GetTo());
        tmp_loc->SetId(*loc.GetId());

        // our new location for feature iterator
        CRef<CSeq_loc> prot_loc = prot_range_mapper.Map(tmp_loc.GetObject());
        prot_loc->SetId(*product.GetId());


        // might be slow if number of visible features in the frame are more than a 100
        // if slow, try not to recreate seq_mapper for each feature
        CRef<CSeq_loc_Mapper> mapper;
        CAlign_CI align_iter(GetScope(), loc, sel);

        while (align_iter  &&  !mapper) {
            // find the first seq-align that matches the current feature product sequence
            const CSeq_align& align = *align_iter;
            CSeq_align::TDim num_row = align.CheckNumRows();
            for (CSeq_align::TDim row = 0;  row < num_row;  ++row) {
                if (product.GetId()->Match(align.GetSeq_id(row))) {
                    mapper.Reset(
                        new CSeq_loc_Mapper(align, *loc.GetId(), &GetScope()));
                    break;
                }
            }
            ++align_iter;
        }

        if ( !mapper ) {
            mapper.Reset( new CSeq_loc_Mapper(product, loc, &GetScope()) );
        }

        // use newly created location to get the features
        CFeat_CI feat_iter(GetScope(), prot_loc.GetObject(), sel);
        size_t size = feat_iter.GetSize();

        // Only show variation in packed form in non-sequence level
        bool packed = feat_iter  &&
            feat_iter->GetFeatSubtype() == CSeqFeatData::eSubtype_variation  &&
            m_Window > kSeqZoomLevel  &&
            size * 1024 * m_Window / range.GetLength() > 50.0;

        if (packed) {
            CHistogramGlyph::TMap the_map(feat->GetRange().GetFrom(),
                feat->GetRange().GetTo(), (float)m_Window);

            for ( ;  feat_iter;  ++feat_iter) {
                CConstRef<CSeq_loc> mapped_loc =
                    mapper->Map(feat_iter->GetLocation());
                the_map.AddRange( mapped_loc->GetTotalRange() );
            }
            dmap.Reset(new CHistogramGlyph(
                           the_map, CSeqFeatData::eSubtype_variation,
                           CSeqUtils::GetUnnamedAnnot()));

        } else if (size > 0) {
            for ( ;  feat_iter;  ++feat_iter) {
                // see the comment about performance above
                CConstRef<CSeq_loc> mapped_loc = mapper->Map(feat_iter->GetLocation());
                if (mapped_loc->Which() != CSeq_loc::e_Null  &&
                    mapped_loc->Which() != CSeq_loc::e_Empty) {
                    const CMappedFeat mapped_feat = *feat_iter;
                    tmp.push_back(x_CreateFeature2(mapped_feat, *mapped_loc));
                }
            }
        }
    }
    catch (CAnnotMapperException&) {
        /// ignore errors from location mapping
        return result;
    }

    result.Reset(new CSGJobResult());
    if (dmap) result->m_ExtraObj.Reset(dmap);
    if ( !tmp.empty() ) result->m_ObjectList.swap(tmp);
    result->m_Token = m_Token;
    result->m_Owner = CRef<CSeqGlyph>(feat.GetPointer());
    return result;
}



///////////////////////////////////////////////////////////////////////////////
/// CSGCalcFeatHistJob
///////////////////////////////////////////////////////////////////////////////
IAppJob::EJobState CSGCalcFeatHistJob::x_Execute()
{
    try {
        CSGJobResult* result = new CSGJobResult();
        m_Result.Reset(result);

        CHistogramGlyph::TMap
            the_map(m_Range.GetFrom(), m_Range.GetTo(), (float)m_Window);
        ITERATE (CSeqGlyph::TObjects, iter, m_Objects) {
            if (IsCanceled()) {
                EJobState state = x_CalcFeatHistRecursive(*iter, the_map);
                if (state != eCompleted) {
                    return state;
                }
            }
        }

        CRef<CSeqGlyph> dmap;
        dmap.Reset(new CHistogramGlyph(the_map, CSeqFeatData::eSubtype_gene));

        result->m_ExtraObj.Reset(dmap);
        result->m_Token = m_Token;
    } catch (CException& ex) {
        m_Error.Reset(new CAppJobError(ex.GetMsg()));
        return eFailed;
    } catch (std::exception& ex) {
        m_Error.Reset(new CAppJobError(ex.what()));
        return eFailed;
    }

    return eCompleted;
}


IAppJob::EJobState
CSGCalcFeatHistJob::x_CalcFeatHistRecursive(CRef<CSeqGlyph> obj,
                                            CDensityMap<float>& the_map)
{
    CLayoutGroup* group = dynamic_cast<CLayoutGroup*>(obj.GetPointer());
    if (group) {
        ITERATE (CLayoutGroup::TObjectList, iter, group->GetChildren()) {
            if (IsCanceled()) {
                EJobState state = x_CalcFeatHistRecursive(*iter, the_map);
                if (state != eCompleted) {
                    return state;
                }
            }
        }
        return eCompleted;
    }

    CFeatGlyph* feat = dynamic_cast<CFeatGlyph*>(obj.GetPointer());
    if (feat) {
        const CSeqFeatData& data = feat->GetFeature().GetData();
        if ( data.GetSubtype() == CSeqFeatData::eSubtype_mRNA  ||
            data.GetSubtype() == CSeqFeatData::eSubtype_cdregion) {
                // collect intervals
                ITERATE (CFeatGlyph::TIntervals, iter, feat->GetIntervals()) {
                    TSeqRange range = *iter;

                    if (IsCanceled()) {
                        return eCanceled;
                    }

                    the_map.AddRange(range);
                }
        }
    }

    return eCompleted;
}


END_NCBI_SCOPE
