#ifndef __GUI_WIDGETS_ALNMULTI___ALIGN_FEATURE_GRAPH__HPP
#define __GUI_WIDGETS_ALNMULTI___ALIGN_FEATURE_GRAPH__HPP

/*  $Id: feature_graph.hpp 35505 2016-05-16 15:18:00Z shkeda $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <gui/opengl/glpane.hpp>
#include <gui/widgets/gl/irenderable.hpp>
#include <gui/widgets/aln_multiple/alnvec_row_graph.hpp>

#include <gui/objutils/density_map.hpp>

#include <gui/utils/app_job.hpp>
#include <gui/utils/app_job_impl.hpp>
#include <gui/utils/app_job_dispatcher.hpp>

#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/seqfeat/Seq_feat.hpp>

#include <gui/widgets/gl/graph_container.hpp>
#include <gui/widgets/gl/histogram_graph.hpp>

#include <gui/widgets/seq_graphic/feature_glyph.hpp>

#include <gui/opengl/gltexturefont.hpp>

#include <util/range_coll.hpp>

#include <objmgr/feat_ci.hpp>
#include <objmgr/annot_selector.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/annot_selector.hpp>
#include <objmgr/seq_loc_mapper.hpp>
#include <objmgr/mapped_feat.hpp>


BEGIN_NCBI_SCOPE

class CAppJobNotification;
class IAlignRowHandle;

BEGIN_SCOPE(objects)
    class CSeq_loc;
    class CScope;
END_SCOPE(objects)


////////////////////////////////////////////////////////////////////////////////
///
struct NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT SMappedFeatLoc {
    objects::CMappedFeat m_Feature;
    //CConstRef<objects::CSeq_feat> m_Feature;
    CRef<objects::CSeq_loc>  m_MappedLoc;

    SMappedFeatLoc()
        {}
    SMappedFeatLoc(const objects::CMappedFeat& feat, objects::CSeq_loc& loc)
        : m_Feature(feat), m_MappedLoc(&loc)    {}
//    SMappedFeatLoc(const objects::CSeq_feat& feat, objects::CSeq_loc& loc)
//        : m_Feature(&feat), m_MappedLoc(&loc)    {}
};


////////////////////////////////////////////////////////////////////////////////
/// CFeatureLoadingJobResult
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CFeatureLoadingJobResult :
    public CObject
{
public:
    typedef vector< CIRef<IRenderable> >    TGraphs;

    string  m_Descr;
    //TFeatures   m_Features;
    TGraphs m_Graphs;
};


////////////////////////////////////////////////////////////////////////////////
/// CFeatureLoadingJob
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CFeatureLoadingJob : public CJobCancelable
{
public:
    typedef objects::CMappingRanges TMapRanges;

    CFeatureLoadingJob(const objects::CBioseq_Handle& handle,
                       objects::SAnnotSelector& sel,
                       const CRange<TSeqPos>& total_range,
                       TMapRanges& ranges,
                       const string& descr,
                       bool separate_types,
                       bool link_genes,
                       int track_order);
    virtual ~CFeatureLoadingJob();

    /// @name IAppJob implementation
    /// @{
    virtual EJobState   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>   GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string    GetDescr() const;
    /// @}};

protected:
    typedef vector< CConstRef<objects::CSeq_feat> >    TFeatures;
    typedef vector<SMappedFeatLoc>  TMappedFeatLocs;

    CIRef<IRenderable>
            x_CreateGraph(TMappedFeatLocs& feat_locs, const string& descr);

protected:
    objects::CBioseq_Handle m_Handle;
    objects::SAnnotSelector m_Sel;
    CRange<TSeqPos>     m_TotalRange;
    CRef<TMapRanges>    m_MapRanges;

    bool    m_SeparateTypes; ///< separate features by different graphs
    bool    m_LinkGenes; ///< links genes with CDS and mRNA

    CMutex  m_Mutex; ///< synchronizes access to the Job members

    string  m_Descr;
    int     m_Order;
    CRef<CFeatureLoadingJobResult>  m_Result;
    CIRef<IAppJobError>    m_Error;
};


////////////////////////////////////////////////////////////////////////////////
/// CFeatureGraphProperties

class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CFeatureGraphProperties :
    public IAlnRowGraphProperties
{
public:
    typedef objects::CFeatListItem  TFeatTypeItem;
    typedef set<TFeatTypeItem>      TFeatTypeItemSet;

    /// defines what feature types / subtypes shall be shown
    TFeatTypeItemSet    m_FeatureTypes;
};


////////////////////////////////////////////////////////////////////////////////
/// CFeatureGraph
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CFeatureGraph :
    public CGraphContainer,
    public IAlnVecRowGraph
{
public:
    typedef CFeatureGraphProperties::TFeatTypeItem   TFeatTypeItem;
    typedef CFeatureGraphProperties::TFeatTypeItemSet   TFeatTypeItemSet;
    typedef objects::CMappingRanges     TMapRanges;
    typedef vector<SMappedFeatLoc>  TMappedFeatLocs;
    typedef CAppJobDispatcher::TJobID   TJobID;

    CFeatureGraph(const IAlignRowHandle& row_handle, bool isDataReadSync);

    /// @name IRenderable implementation
    /// @{
    virtual TVPPoint    PreferredSize();
    virtual void    Layout();
    /// @}

    /// @name IAlnVecRowGraph implementaion
    /// @{
    virtual ~CFeatureGraph();

    virtual bool    IsCreated() const;
    virtual bool    Create();
    virtual void    Destroy();
    virtual void    Update(double start, double stop);

    virtual const IAlnRowGraphProperties*     GetProperties() const;
    virtual void    SetProperties(IAlnRowGraphProperties* props);

    virtual void    Render(CGlPane& pane, IAlnSegmentIterator& it);
    /// @}

    void    OnAJNotification(CEvent* evt);

protected:
    DECLARE_EVENT_MAP();

    typedef set<objects::CSeqFeatData::ESubtype> TSubtypeSet;

    void    x_RenderStatusText(CGlPane& pane);

    CRef<TMapRanges>    x_CreateMapRanges();

    void    x_StartJob(const TFeatTypeItemSet& feat_set,
                       const TSubtypeSet& filter,
                       bool positive,
                       CRange<TSeqPos>& total_range,
                       TMapRanges& map_ranges,
                       const string& descr,
                       bool separate_types,
                       bool link_genes,
                       int order);

    bool    x_StartJob(objects::SAnnotSelector& sel,
                       CRange<TSeqPos>& total_range,
                       TMapRanges& map_ranges,
                       const string& descr,
                       bool separate_types,
                       bool link_genes,
                       int order);

    bool    x_CancelJob(TJobID job_id);
    /// Cancel all active jobs
    void    x_CancelJobs();

    void    x_OnJobCompleted(CAppJobNotification& notify);
    void    x_OnJobFailed(CAppJobNotification& notify);
    void    x_Create(double start, double stop);

protected:
    typedef map<TJobID, CRef<CFeatureLoadingJob> >    TJobMap;

    const IAlignRowHandle&    m_RowHandle;
    bool        m_Created;
    TJobMap     m_Jobs;

    string  m_StatusText;
    CGlTextureFont   m_Font;

    CFeatureGraphProperties m_Properties;

private:
    bool m_isDataReadSync;
    bool m_Updated; // to prevent multiple updates in synchronous mode

    /// Candidate graphs waiting for replacing the current ones.
    TGraphs     m_PendingGraphs;
};



////////////////////////////////////////////////////////////////////////////////
/// CFeatureGraph

class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CAlignedFeatureGraph :
    public CObjectEx,
    public CRenderableImpl
{
public:
    typedef vector<SMappedFeatLoc>  TMappedFeatLocs;

    CAlignedFeatureGraph();

    CAlignedFeatureGraph(TMappedFeatLocs& feat_locs,
                         objects::CScope& scope,
                         const string& label,
                         bool link_genes);

    virtual ~CAlignedFeatureGraph();

    /// @name IRenderable implementation
    /// @{
    virtual void Render(CGlPane& pane);

    virtual TVPPoint PreferredSize();

    virtual bool    NeedTooltip(CGlPane& pane, int vp_x, int vp_y);
    virtual string  GetTooltip();
    /// @}

    struct SFeatRec {

    public:
        SFeatRec()
            : m_ClusterIndex(-1) {}
        SFeatRec(SMappedFeatLoc& feat_loc, int cluster_index = -1)
            : m_FeatLoc(feat_loc), m_ClusterIndex(cluster_index)   {
        }
        SFeatRec(CFeatGlyph& glyph, int cluster_index = -1)
            : m_ClusterIndex(-1)
            , m_FeatGlyph(&glyph)
        {
            m_FeatLoc.m_Feature = glyph.GetMappedFeature();
            m_FeatLoc.m_MappedLoc.Reset(new objects::CSeq_loc);
            m_FeatLoc.m_MappedLoc->Assign(glyph.GetMappedFeature().GetLocation());
        }
        const objects::CSeq_feat& GetFeature(void) const
        {
            return m_FeatLoc.m_Feature.GetOriginalFeature();
        }
        const objects::CSeq_loc& GetLocation(void) const {
            return *m_FeatLoc.m_MappedLoc;
        }
        void SetClusterIndex(int value) {
            m_ClusterIndex = value;
        }
        int GetClusterIndex() const {
            return m_ClusterIndex;
        }

        CFeatGlyph& GetGlyph() const {
            if (!m_FeatGlyph) {
                m_FeatGlyph = new CFeatGlyph(m_FeatLoc.m_Feature, *m_FeatLoc.m_MappedLoc);
                
                CRef<CFeatureParams> conf(new CFeatureParams);
                conf->m_BarHeight = 10.;
                conf->m_HeadHeight = 1.;
                conf->m_TailHeight = 1;
                conf->m_LineWidth = 1;
                conf->m_LabelPos = CFeatureParams::ePos_Inside;
                m_FeatGlyph->SetConfig(conf);

            }
            return *m_FeatGlyph;
        }
    protected:
        SMappedFeatLoc  m_FeatLoc;
        int m_ClusterIndex;
        mutable CRef<CFeatGlyph> m_FeatGlyph;
    };
    typedef vector<SFeatRec>    TFeatRecs;
    typedef vector<SFeatRec*>   TFeatRecPtrs;


    // defines a cluster consisting of a Gene with associated mRNAs and CDS-es
    // Cluster occupies 1 or more consequent layers. The horz. range belonging
    // to the cluster may different for every layer, it is defined as a union of
    // all features in cluster that are placed on this particular layer
    // plus m_Offset additional positions on every side

    struct SCluster    {
        int m_FirstLayer;
        int m_LastLayer;
        int m_Offset; // horz size in residues of zone

        SCluster(int first, int last, int offset)
            : m_FirstLayer(first), m_LastLayer(last), m_Offset(offset)  {}
    };

    // define a horizontal feature layer
    struct SLayer
    {
        TFeatRecPtrs   m_FeatRecPtrs; // pointers to SFeatRecs
    };

    typedef vector<SCluster>    TClusters;
    typedef vector<SLayer*> TLayers;
    typedef CRangeCollection<TSeqPos> TColl;
    typedef vector<TColl*>  TCollVector;
    typedef pair<SFeatRec*, TSeqPos>    TFeatRecPtrLenPair;
    typedef list< CConstRef<objects::CSeq_feat> >    TFeatList;

    typedef map<const objects::CSeq_feat*, pair<SFeatRec*, bool> >   TFeatToRecFlag;

    virtual void x_Init(TMappedFeatLocs& feat_locs,
        objects::CScope& scope,
        const string& label,
        bool link_genes);
    void x_Init(CSeqGlyph::TObjects& glyphs,
        objects::CScope& scope,
        const string& label,
        bool link_genes);

    void    x_Clear();

    void    x_Layout();
    void    x_SimpleLayout(TFeatRecPtrs& feat_recs);
    void    x_LinkedLayout(TFeatRecPtrs& feat_recs);

    void    x_GatherCluster(SFeatRec& feat_rec,
                            TFeatRecPtrs& cluster);
    void    x_GetRecsSortedByLength(const TFeatList& input,
                                    vector<TFeatRecPtrLenPair>& pairs);
    void    x_PlaceCluster(TFeatRecPtrs& cluster);
    bool    x_mRNA_CDS_ToBePlaced(const objects::CSeq_feat& feat);
    void    x_TryPlaceCDSFeature(SFeatRec& rec);

    void    x_PlaceFeature(SFeatRec& rec);

    void    x_RenderFeature(CGlPane& pane, const SFeatRec& rec, int layer,
                            TModelUnit top, TModelUnit bottom);

    virtual string  x_GetTooltip(const SFeatRec& rec);

protected:
    CRef<objects::CScope>    m_Scope;
    string  m_Label;
    bool    m_LinkGenes;

    TFeatRecs   m_FeatRecs;  //  mapped feature records
    TClusters   m_Clusters; // feature clusters
    TLayers     m_Layers;

    TFeatToRecFlag  m_FeatToRecFlag;
    TCollVector     m_Occupied; // layer index -> collection of ranges occupied by features

    SFeatRec* m_TooltipRec;
    int m_LayerH;

};


///////////////////////////////////////////////////////////////////////////////
/// CFeatHistogramDS
class CFeatHistogramDS
    : public CObject
    , public IHistogramGraphDS
{
public:
    typedef vector<SMappedFeatLoc>  TMappedFeatLocs;

    CFeatHistogramDS(TMappedFeatLocs& feat_locs, const string& label);

    /// @name IHistogramDS implementation
    /// @{
    virtual double  GetLimit() const;

    virtual double  GetStart() const;
    virtual double  GetStop() const;
    virtual double  GetStep() const;

    virtual size_t     GetCount() const;
    virtual double  GetValue(size_t index);

    virtual double  GetMaxValue();

    virtual string  GetLabel() const;

    virtual void    Update(double start, double stop);
    /// @}

protected:
    typedef CRange<TSeqPos>     TFeatRange;
    typedef CDensityMap<int>    TMap;

    string      m_Label;
    TMappedFeatLocs   m_FeatLocs;
    TFeatRange  m_FeatRange;
    auto_ptr<TMap>   m_Map;
};


END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_ALNMULTI___ALIGN_FEATURE_GRAPH__HPP
