/*  $Id: feature_graph.cpp 41823 2018-10-17 17:34:58Z katargir $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a 'United States Government Work' under the
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

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>

#include <gui/widgets/aln_multiple/aln_vec_iterator.hpp>
#include <gui/widgets/aln_multiple/feature_graph.hpp>
#include <gui/widgets/aln_multiple/alnmulti_ds.hpp>

#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/widgets/seq_graphic/feature_glyph.hpp>

#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/types.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>

#include <gui/utils/event_translator.hpp>

#include <objmgr/util/sequence.hpp>
#include <objmgr/impl/synonyms.hpp>

#include <math.h>
#include <memory>
#include <vector>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
/// CFeatureLoadingJob
CFeatureLoadingJob::CFeatureLoadingJob(const CBioseq_Handle&  handle,
                                       SAnnotSelector&        sel,
                                       const CRange<TSeqPos>& total_range,
                                       TMapRanges&            ranges,
                                       const string&          descr,
                                       bool                   separate_types,
                                       bool                   link_genes,
                                       int track_order)
:   m_Handle(handle),
    m_Sel(sel),
    m_TotalRange(total_range),
    m_MapRanges(&ranges),
    m_SeparateTypes(separate_types),
    m_LinkGenes(link_genes),
    m_Descr(descr),
    m_Order(track_order)
{
    _ASSERT(! m_SeparateTypes  || ! m_LinkGenes);
}


CFeatureLoadingJob::~CFeatureLoadingJob()
{
}


IAppJob::EJobState CFeatureLoadingJob::Run()
{
    //LOG_POST("CFeatureLoadingJob::Run() STARTED " << m_Descr);
    {
        CMutexGuard Guard(m_Mutex); // synchronize
        m_Result.Reset();
        m_Error.Reset();
    }

    /// load features and separate them by subtypes if needed

    typedef map<CSeqFeatData::ESubtype, TMappedFeatLocs>  TMap;
    TMap    subtype_to_feats;   // subtype -> mapped features

    // cache TMappedFeatLocs* to avoid lookups for every feature
    CSeqFeatData::ESubtype subtype, prev_subtype = CSeqFeatData::eSubtype_bad;
    TMappedFeatLocs* locs = NULL;

    CSeq_loc_Mapper mapper(m_MapRanges.GetPointer());

    // iterate by features
    CFeat_CI feat_it(m_Handle, m_TotalRange, m_Sel);
    for ( ;  feat_it  &&  !IsCanceled();  ++feat_it) {
        const CMappedFeat& mp_feat = *feat_it;
        const CSeq_feat& feat = mp_feat.GetMappedFeature();
        // remap the original feature through the alignment
        CRef<CSeq_loc> mapped_loc = mapper.Map(feat.GetLocation());

        // if mapping is successful - add feature to the map
        if( ! mapped_loc->IsEmpty()  &&  ! mapped_loc->IsNull())   {
            // choose container based on settings and subtype
            subtype = m_SeparateTypes ? feat.GetData().GetSubtype()
                                    : CSeqFeatData::eSubtype_any;
            if(subtype != prev_subtype) {
                TMap::iterator it = subtype_to_feats.find(subtype);
                if(it == subtype_to_feats.end())    {
                    it = subtype_to_feats.insert(
                        TMap::value_type(subtype, TMappedFeatLocs())).first;
                }
                locs = & it->second;
                prev_subtype = subtype;
            }

            // add the feature to the container
//            locs->push_back(SMappedFeatLoc(feat, *mapped_loc));
            locs->push_back(SMappedFeatLoc(mp_feat, *mapped_loc));
        }
    }

    // create Result object
    if( ! IsCanceled())  {
        CFeatureLoadingJobResult* res = new CFeatureLoadingJobResult();
        res->m_Descr = m_Descr;

        // create graphs
        NON_CONST_ITERATE(TMap, it, subtype_to_feats) {
            if (IsCanceled()) {
                return eCanceled;
            }
            TMappedFeatLocs& locs = it->second;
            if( ! locs.empty())    {
                CIRef<IRenderable> graph = x_CreateGraph(locs, m_Descr);
                res->m_Graphs.push_back(graph);

                locs.clear();
            }
        }

        CMutexGuard Guard(m_Mutex);
        m_Result.Reset(res);
    }

    //LOG_POST(Info << "------ CFeatureLoadingJob::Run() FINISHED    " << m_Descr);

    return IsCanceled() ? eCanceled : eCompleted;
}


// create a Graph from the given set of features
CIRef<IRenderable>
    CFeatureLoadingJob::x_CreateGraph(TMappedFeatLocs& feat_locs, const string& descr)
{
    static const size_t kHistThreshold = 1000;
    CIRef<IRenderable> graph;

    size_t n_feat = feat_locs.size();
    if(n_feat > kHistThreshold)  {
        // too many features - create a Histogram
        CHistogramGraph* hist = new CHistogramGraph(true);
        graph.Reset(hist);

        CHistogramGraph::SProperties props;
        props.m_Margin = 1;
        hist->SetProperties(props);

        CFeatHistogramDS* ds = new CFeatHistogramDS(feat_locs, descr);
        hist->SetDataSource(ds);

        //LOG_POST("Histogram Graph created - " << descr);
    } else if(n_feat > 0)   {
        bool link_genes = ! m_SeparateTypes  &&  m_LinkGenes;
        graph = new CAlignedFeatureGraph(feat_locs, m_Handle.GetScope(),
                                         descr, link_genes);
        //LOG_POST("Layered Graph created - " << descr);
    }
    graph->SetOrder(m_Order);
    return graph;
}


CConstIRef<IAppJobProgress> CFeatureLoadingJob::GetProgress()
{
    CMutexGuard Guard(m_Mutex); // synchronize
    return CConstIRef<IAppJobProgress>();
}


CRef<CObject> CFeatureLoadingJob::GetResult()
{
    CMutexGuard Guard(m_Mutex); // synchronize
    return CRef<CObject>(m_Result.GetPointer());
}


CConstIRef<IAppJobError> CFeatureLoadingJob::GetError()
{
    CMutexGuard Guard(m_Mutex); // synchronize
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}


string CFeatureLoadingJob::GetDescr() const
{
    return m_Descr;
}


////////////////////////////////////////////////////////////////////////////////
/// CFeatureGraph

const static int kGradColors = 32;

CFeatureGraph::CFeatureGraph(const IAlignRowHandle& row_handle, bool isDataReadSync)
    : CGraphContainer(eHorzStack),
    m_RowHandle(row_handle),
    m_Created(false),
    m_Font(CGlTextureFont::eFontFace_Helvetica, 10),
    m_isDataReadSync(isDataReadSync),
    m_Updated(false)
{
    // add all feature types to the set
    const CFeatList& list = *CSeqFeatData::GetFeatList();
    TFeatTypeItemSet& set = m_Properties.m_FeatureTypes;

    ITERATE(CFeatList, ft_it, list) {
        const CFeatListItem& item = *ft_it;
        set.insert(item);
    }
}


CFeatureGraph::~CFeatureGraph()
{
    Destroy();
}


bool CFeatureGraph::IsCreated() const
{
    return m_Created;
}


// performs basic initilization and laucnhes async jobs that will create graphs
bool CFeatureGraph::Create()
{
    if (m_ModelRect.Width() > 0.0  &&  m_ModelRect.Left() >= 0.0) {
        x_Create(m_ModelRect.Left(), m_ModelRect.Right());
        m_Created = true;
        return true;
    }
    return false;
}
 

void CFeatureGraph::x_StartJob(const TFeatTypeItemSet& feat_set,
                               const TSubtypeSet& filter,
                               bool positive,
                               CRange<TSeqPos>& total_range,
                               TMapRanges& map_ranges,
                               const string& descr,
                               bool separate_types,
                               bool link_genes,
                               int order)
{
    // apply "filter" to "feat_set", results are accumulated in "final_set"
    TFeatTypeItemSet final_set;
    bool include_snp = false;

    ITERATE(TFeatTypeItemSet, it, feat_set) {
        CSeqFeatData::ESubtype subtype =
            (CSeqFeatData::ESubtype) it->GetSubtype();

        if(subtype != CSeqFeatData::eSubtype_any)   {
            bool found = (filter.find(subtype) != filter.end());
            if(found == positive)   {
                final_set.insert(*it);
                if(subtype == CSeqFeatData::eSubtype_variation)    {
                    include_snp = true;
                }
            }
        }
    }

    // create SAnnotSelector  according to the "final_set" and launch the job
    if( ! final_set.empty())    {
        SAnnotSelector sel = CSeqUtils::GetAnnotSelector();

        ITERATE(TFeatTypeItemSet, it, final_set) {
            CSeqFeatData::ESubtype subtype =
                (CSeqFeatData::ESubtype) it->GetSubtype();
            sel.IncludeFeatSubtype(subtype);
        }
        if(include_snp) {
            sel.AddNamedAnnots("SNP");
        } else {
            sel.ExcludeNamedAnnots("SNP");
        }

        x_StartJob(sel, total_range, map_ranges, descr, separate_types, link_genes, order);
    }
}


void CFeatureGraph::Destroy()
{
    x_CancelJobs();

    RemoveAllGraphs();
    m_Created = false;
}


void CFeatureGraph::Update(double start, double stop)
{
    if (((TSeqPos)start == (TSeqPos)m_ModelRect.Left()  &&
        (TSeqPos)stop == (TSeqPos)m_ModelRect.Right())  ||
        start < 0.0  ||   start > stop) {
            return;
    }
    if (m_isDataReadSync && m_Updated) return;
    m_Updated = true;

    x_CancelJobs();
    x_Create(start, stop);
}


const IAlnRowGraphProperties* CFeatureGraph::GetProperties() const
{
    return NULL;
}


void CFeatureGraph::SetProperties(IAlnRowGraphProperties* props)
{
    CFeatureGraphProperties* gr_props =
        dynamic_cast<CFeatureGraphProperties*>(props);
    _ASSERT(gr_props);
    if(gr_props)    {
        m_Properties = *gr_props;
    }
}


void CFeatureGraph::Render(CGlPane& pane, IAlnSegmentIterator& it)
{
    //LOG_POST(Info << " ");
    //LOG_POST(Info << this << "  CFeatureGraph::Render()  m_StatusText = " << m_StatusText);
    //LOG_POST(Info << "    Viewport " << m_VPRect.ToString() << "\n    Visible " << m_ModelRect.ToString());

    //CGlAttrGuard AttrGuard(GL_ENABLE_BIT  | GL_COLOR_BUFFER_BIT | GL_HINT_BIT
    //                        | GL_LINE_SMOOTH | GL_POLYGON_MODE | GL_LINE_BIT);

    CGraphContainer::Render(pane);

    x_RenderStatusText(pane);
}


static const int kTextOff = 2;

void CFeatureGraph::x_RenderStatusText(CGlPane& pane)
{
    //LOG_POST("CFeatureGraph::x_RenderStatusText() m_StatusText = "  << m_StatusText);
    if( ! m_StatusText.empty())  {
        //CGlAttrGuard guard(GL_LINE_BIT |  GL_ENABLE_BIT  | GL_COLOR_BUFFER_BIT );

        IRender& gl = GetGl();

        gl.Enable(GL_BLEND);
        gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        gl.Enable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

        // calculate bounding rectangle
        TVPRect rc = m_VPRect;
        rc.Inflate(-kTextOff, -kTextOff);
        TVPUnit h = (TVPUnit) m_Font.TextHeight()+ 1 + kTextOff * 2;
        rc.SetBottom(rc.Top() - h);

        pane.OpenPixels();

        double alpha = 0.5;

        // render text in rectangular frame
        gl.Color4d(0.8, 0.8, 0.8, alpha);
        gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        gl.RectC(rc);

        gl.Color4d(0.2, 0.2, 0.2, alpha);
        gl.PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        gl.RectC(rc);
		gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        rc.Inflate(-kTextOff, -kTextOff);

        m_Font.TextOut(rc.Left(), rc.Bottom(), rc.Right(), rc.Top(),
                       m_StatusText.c_str(), IGlFont::eAlign_Left);

        pane.Close();
    }
}


BEGIN_EVENT_MAP(CFeatureGraph, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged,
             &CFeatureGraph::OnAJNotification)
END_EVENT_MAP()


TVPPoint CFeatureGraph::PreferredSize()
{
    TVPPoint size = CGraphContainer::PreferredSize();
    if( ! m_StatusText.empty()) {
        TVPUnit h = (TVPUnit) m_Font.TextHeight() +  4 * kTextOff;
        size.m_Y = max(size.m_Y, h);
    }
    return size;
}

void CFeatureGraph::Layout()
{
    CGraphContainer::Layout();

    /// adjust model space for graphs
    /// the trik is that model space is in the same units as viewport (pixels), however
    /// it is flipped so that range [a, b] in viewport corresponds to [b, a] in model space
    switch(m_LayoutPolicy)  {
    case eHorzStack:    {
        TVPRect rc_vp(m_VPRect);
        TVPUnit shift = 0;
        TModelRect rc_vis(m_ModelRect);

        NON_CONST_ITERATE(TGraphs, it, m_Graphs)  {
            TGraph& graph = **it;
            TVPUnit size = graph.PreferredSize().Y();

            // adjust viewport rect
            TVPUnit vp_bottom = m_VPRect.Bottom()  + shift;
            TVPUnit vp_top = vp_bottom + (size - 1);
            rc_vp.SetVert(vp_bottom, vp_top);
            graph.SetVPRect(rc_vp);

            // adjust model rect
            TModelUnit m_bottom = m_ModelRect.Bottom() - shift;
            TModelUnit m_top = m_bottom - (size - 1);
            rc_vis.SetVert(m_bottom, m_top);
            graph.SetModelRect(rc_vis);

            shift += size;
        }
        break;
    }
    case eVertStack:
    default:
        _ASSERT(false);
        break;
    }
}


// creates CMappingRanges based on m_RowHandle
CRef<CMappingRanges> CFeatureGraph::x_CreateMapRanges()
{
    typedef IAlnExplorer::TSignedRange   TSignedRange;

    // get a segment iterator
    TSignedRange total_r;
    total_r.SetFrom(m_RowHandle.GetSeqAlnStart());
    total_r.SetTo(m_RowHandle.GetSeqAlnStop());

    auto_ptr<IAlnSegmentIterator>
        it(m_RowHandle.CreateSegmentIterator(total_r, IAlnSegmentIterator::eSkipInserts));

    // prepare ID handles
    CBioseq_Handle handle = m_RowHandle.GetBioseqHandle();
    CConstRef<CSeq_id> seq_id = handle.GetSeqId();
    CScope& scope = m_RowHandle.GetBioseqHandle().GetScope();
    CConstRef<CSynonymsSet> syns = scope.GetSynonyms(*seq_id);

    CSeq_id_Handle aln_idh = sequence::GetId(handle, sequence::eGetId_Canonical);

    // create CMappingRanges
    CRef<CMappingRanges> ranges(new CMappingRanges());

    // iterate by the segments in the alignment and populate CMappingRanges
    for( ; *it; ++(*it))    {
        const IAlnSegment& seg = **it;
        if(seg.IsAligned()) {
            TSignedRange aln_range = seg.GetAlnRange();
            TSignedRange range = seg.GetRange();
            ENa_strand aln_strand = seg.IsReversed() ? eNa_strand_minus : eNa_strand_plus;

            // add a range for every Seq-id synonym, this is necessary because features
            // can refer to different Seq-ids
            ITERATE(CSynonymsSet, it_s, *syns)  {
                CSeq_id_Handle seq_idh = CSynonymsSet::GetSeq_id_Handle(it_s);
                ranges->AddConversion(seq_idh, range.GetFrom(), range.GetLength(),
                                    eNa_strand_plus,
                                    aln_idh, aln_range.GetFrom(),
                                    aln_strand);
            }
        }
    }
    return ranges;
}


bool CFeatureGraph::x_StartJob(SAnnotSelector& sel, 
                               CRange<TSeqPos>& total_range,
                               TMapRanges& map_ranges,
                               const string& descr,
                               bool separate_types, 
                               bool link_genes,
                               int order)
{
    //LOG_POST("CFeatureGraph::x_StartJob(CFeatureLoadingJob& job)");

    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    CRef<CFeatureLoadingJob> job(
        new CFeatureLoadingJob(m_RowHandle.GetBioseqHandle(), sel, total_range, map_ranges,
                               descr, separate_types, link_genes, order));

    int job_id = -1;
    if (m_isDataReadSync) {
        // do everything synchronously
        if(IAppJob::eCompleted == job->Run()) {
            CAppJobNotification    notification(-1, job->GetResult().GetPointer());
            x_OnJobCompleted( notification );
            m_StatusText = "";
            return true;
        } else {
            CAppJobError    errorNotification("Synchronous feature loading job failed.");
            IAppJobError &  rErrorNotification( errorNotification );
            CAppJobNotification    notification(-1, rErrorNotification);
            x_OnJobFailed( notification );
            m_StatusText = "";
            return false;
        }

        job.Reset();
    } else {
        try {
            /// launch on ObjManagerEngine, receive notifications, no progress reports
            /// delete the Job when completed
            job_id =
                disp.StartJob(*job, "ObjManagerEngine", *this, -1, true);
            m_Jobs[job_id] = job;
            //LOG_POST("Started job id = " << job_id << " " << job->GetDescr());
            return true;
        } catch(CAppJobException& e)  {
            ERR_POST("CFeatureGraph::x_StartJob - Failed");
            LOG_POST(e.ReportAll());
            return false;
        }
    }

    return false;
}

void CFeatureGraph::x_CancelJobs()
{
    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    NON_CONST_ITERATE(TJobMap, it, m_Jobs) {
        TJobID job_id = it->first;
        try {
            disp.DeleteJob(job_id);
        } catch(CException&) {            
        }
    }
    m_Jobs.clear();
    m_PendingGraphs.clear();
}

bool CFeatureGraph::x_CancelJob(TJobID job_id)
{
    TJobMap::iterator it = m_Jobs.find(job_id);
    if(it != m_Jobs.end())  {
        CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
        m_Jobs.erase(it);
        try {
            disp.DeleteJob(job_id);
        } catch(CAppJobException& e)  {
            switch(e.GetErrCode())  {
            case CAppJobException::eUnknownJob:
            case CAppJobException::eEngine_UnknownJob:
                /// this is fine - job probably already finished
                break;
            default:
                // something wrong
                ERR_POST("CFeatureGraph::x_CancelJob() " << "Error canceling job");
                LOG_POST(e.ReportAll());
                return false;
            }
        }
        return true;
    } else {
        _ASSERT(false);
        ERR_POST("CFeatureGraph::x_CancelJob - invalid job id" << job_id);
        return false;
    }
    return false;
}


void CFeatureGraph::OnAJNotification(CEvent* evt)
{
    //LOG_POST("CFeatureGraph::OnAJNotification(CEvent* evt)");
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);

    if(notn)    {
        int job_id = notn->GetJobID();
        TJobMap::iterator it = m_Jobs.find(job_id);

        //if(notn->GetState() != IAppJob::eRunning)    {
        //    string s = CAppJobDispatcher::StateToStr(notn->GetState());
        //    LOG_POST("OnAJNotification(),   id = " << job_id << ",  State " << s);
        //}

        if(it == m_Jobs.end())  {
            // Maybe, the job has been removed/canceled.
            return;
        }

        size_t old_graphs_n = GetGraphsCount();

        switch(notn->GetState())    {
        case IAppJob::eCompleted:
            x_OnJobCompleted(*notn);
            break;
        case IAppJob::eFailed:
            x_OnJobFailed(*notn);
            break;
        case IAppJob::eCanceled:
            break;
        case IAppJob::eRunning:
            return;
        default:
            _ASSERT(false);
            return;
        }

        m_Jobs.erase(it);
        if(m_Jobs.empty())  {
            RemoveAllGraphs();
            if (size_t n = m_PendingGraphs.size()) {
                for (size_t i = 0; i < n; ++i) {
                    AddGraph(m_PendingGraphs[i].GetPointer());
                }
            }
            m_StatusText = "";
        }

        if(GetGraphsCount() != old_graphs_n  ||  m_Jobs.empty())   {
            // notify parent graph about the changes
            CGraphNotification nt(CGraphNotification::eContentChanged);
            Send(&nt, ePool_Parent);
        }
    }
}


void CFeatureGraph::x_OnJobCompleted(CAppJobNotification& notify)
{
    //LOG_POST("CFeatureGraph::x_OnJobCompleted(CAppJobNotification& notify)");
    CRef<CObject> res_obj = notify.GetResult();
    CFeatureLoadingJobResult* result =
        dynamic_cast<CFeatureLoadingJobResult*>(res_obj.GetPointer());

    if(result)  {
        CFeatureLoadingJobResult::TGraphs& graphs = result->m_Graphs;
        size_t n = graphs.size();
        //LOG_POST("CFeatureGraph::x_OnJobCompleted() " << n << " graphs arrived");
        if(n)  {
            std::copy(graphs.begin(), graphs.end(), back_inserter(m_PendingGraphs));
        }
    } else {
        ERR_POST("CFeatureGraph::x_OnJobCompleted() notification for job "
                 << notify.GetJobID() << " does not contain results.");
    }
}


void CFeatureGraph::x_OnJobFailed(CAppJobNotification& notify)
{
    CConstIRef<IAppJobError> err = notify.GetError();
    if(err) {
        // TODO report result in graph
    } else {
        ERR_POST("CFeatureGraph::x_OnJobFailed() notification for job "
                 << notify.GetJobID() << " does not have an error object");
    }
}


void CFeatureGraph::x_Create(double start, double stop)
{
    //LOG_POST(Info);
    m_StatusText = "Features Graph - Loading data...";

    // setup groups
    TSubtypeSet gene_set;  // Genes, mRNAsm CDS-es
    gene_set.insert(CSeqFeatData::eSubtype_gene);
    gene_set.insert(CSeqFeatData::eSubtype_mRNA);
    gene_set.insert(CSeqFeatData::eSubtype_cdregion);

    TSubtypeSet snp_set; // SNPs
    snp_set.insert(CSeqFeatData::eSubtype_variation);

    TSubtypeSet sts_set; // STS-es
    sts_set.insert(CSeqFeatData::eSubtype_STS);

    TSubtypeSet others_set; // all other types - negative set
    others_set.insert(CSeqFeatData::eSubtype_gene);
    others_set.insert(CSeqFeatData::eSubtype_cdregion);
    others_set.insert(CSeqFeatData::eSubtype_mRNA);
    others_set.insert(CSeqFeatData::eSubtype_variation);
    others_set.insert(CSeqFeatData::eSubtype_STS);

    // here we launch all the Feature Loading Jobs
    // groupped by features types with filter
    CRef<TMapRanges> map_ranges = x_CreateMapRanges();

    //CRange<TSeqPos>  total_range(m_RowHandle.GetSeqStart(), 
    //                             m_RowHandle.GetSeqStop());
    TSignedSeqPos f = m_RowHandle.GetSeqPosFromAlnPos(start, IAlnExplorer::eLeft);
    TSignedSeqPos t = m_RowHandle.GetSeqPosFromAlnPos(stop, IAlnExplorer::eRight);
    if (f > t) {
        swap(f, t);
    }
    CRange<TSeqPos>  total_range((TSeqPos)f, (TSeqPos)t);
    TFeatTypeItemSet& load_set = m_Properties.m_FeatureTypes;

    x_StartJob(load_set, gene_set,   true,  total_range, *map_ranges, "Gene, CDS, mRNA", false, true, 10);
    //x_StartJob(load_set, snp_set,    true,  total_range, *map_ranges, "SNP", false, false);
    x_StartJob(load_set, sts_set,    true,  total_range, *map_ranges, "STS", false, false, 20);
    x_StartJob(load_set, others_set, false, total_range, *map_ranges, "Other feature types", false, false, 30);
    if (m_isDataReadSync) {
        // Piece from OnAJNotification suitable for sync execution
        //RemoveAllGraphs();
        if (size_t n = m_PendingGraphs.size()) {
            while (n > 0) {
                AddGraph(m_PendingGraphs[--n].GetPointer());
            }
        }
        m_StatusText = "";
        CGraphNotification nt(CGraphNotification::eContentChanged);
        Send(&nt, ePool_Parent);
    }
}



///////////////////////////////////////////////////////////////////////////////
/// CAlignedFeatureGraph
static const int kLayerH = 12;
static const int kLayerOffY = 2;

CAlignedFeatureGraph::CAlignedFeatureGraph()
   : m_Scope(0),
     m_Label(NcbiEmptyString),
     m_LinkGenes(false),
     m_TooltipRec(NULL),
     m_LayerH(kLayerH)
{
}

CAlignedFeatureGraph::CAlignedFeatureGraph(TMappedFeatLocs& feat_locs,
                                           CScope& scope,
                                           const string& label,
                                           bool link_genes)
:   m_Scope(&scope),
    m_Label(label),
    m_LinkGenes(link_genes),
    m_TooltipRec(NULL),
    m_LayerH(kLayerH)
{
    x_Init(feat_locs, scope, label, link_genes);        
}

void CAlignedFeatureGraph::x_Init(TMappedFeatLocs& feat_locs,
                             CScope& scope,
                             const string& label,
                             bool link_genes)
{
    x_Clear();
    m_Scope = &scope;
    m_Label = label;
    m_LinkGenes = link_genes;
    m_FeatRecs.clear();
    size_t n = feat_locs.size();
    m_FeatRecs.resize(n);
    for( size_t i = 0;  i < n;  i++)    {
        SFeatRec s(feat_locs[i]);
        m_FeatRecs[i] = s;
    }
    x_Layout();

}

void CAlignedFeatureGraph::x_Init(CSeqGlyph::TObjects& glyphs,
                                  CScope& scope,
                                  const string& label,
                                  bool link_genes)
{
    x_Clear();
    m_Scope = &scope;
    m_Label = label;
    m_LinkGenes = link_genes;
    m_FeatRecs.clear();
    m_FeatRecs.reserve(glyphs.size());
    NON_CONST_ITERATE(CSeqGlyph::TObjects, it, glyphs) {
        CFeatGlyph* feat_glyph = dynamic_cast<CFeatGlyph*>(&**it);
        if (!feat_glyph)
            continue;
        SFeatRec s(*feat_glyph);
        m_FeatRecs.push_back(s);
    }
    x_Layout();

}

CAlignedFeatureGraph::~CAlignedFeatureGraph()
{
    x_Clear();
}


typedef CRange<TSeqPos>     TFeatRange;

void CAlignedFeatureGraph::Render(CGlPane& pane)
{
    //LOG_POST(Info << "  CAlignedFeatureGraph::Render()");
    //LOG_POST(Info << "    Viewport " << m_VPRect.ToString() << "\n    Visible " << m_ModelRect.ToString());

    //CGlAttrGuard AttrGuard(GL_ENABLE_BIT  | GL_COLOR_BUFFER_BIT | GL_HINT_BIT
    //                        | GL_LINE_SMOOTH | GL_POLYGON_MODE | GL_LINE_BIT);

    IRender& gl = GetGl();

    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // render bounds rectangle
    pane.OpenPixels();
    gl.Color3d(0.97, 0.97, 0.97);
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    gl.RectC(m_VPRect);
    pane.Close();
    // END of render bounds rectangle

    pane.OpenOrtho();

    gl.Color3d(1, 0.5, 0.5);
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //LOG_POST(Info << "  Rendering "  << m_Layers.size() << " layers");
    TModelUnit top = m_ModelRect.Top() - pane.GetOffsetY();
    TModelUnit bottom = top + m_LayerH;

    // render all Layers
    for( size_t i = 0;  i < m_Layers.size(); i++ )  {
        TFeatRecPtrs& recs = m_Layers[i]->m_FeatRecPtrs;

        // render Layer with index i
        for( size_t j = 0; j < recs.size();  j++ ) {
            SFeatRec& rec = *recs[j];
            x_RenderFeature(pane, rec, (int)i, top, bottom);
        }

        top += m_LayerH;
        bottom += m_LayerH;
    }

    pane.Close();
}


CRgbaColor GetRandomColor(int i_color)
{
    static CRgbaColor c_red(1.0f, 0.5f, 0.5f);
    int angle = i_color * 131;
    return CRgbaColor(CRgbaColor::RotateColor(c_red, (float) angle));
}


CRgbaColor& GetColor(CSeqFeatData::ESubtype subtype)
{
    typedef map<CSeqFeatData::ESubtype, CRgbaColor> TTypeToColor;

    static TTypeToColor s_map;
    if(s_map.empty())   {
        s_map[CSeqFeatData::eSubtype_gene] = CRgbaColor(0.0f, 0.8f, 0.0f);
        s_map[CSeqFeatData::eSubtype_mRNA] = CRgbaColor(0.0f, 0.0f, 0.8f);
        s_map[CSeqFeatData::eSubtype_cdregion] = CRgbaColor(0.8f, 0.0f, 0.0f);
    }

    TTypeToColor::iterator it = s_map.find(subtype);
    if(it == s_map.end())   {
        size_t i = s_map.size();
        CRgbaColor color = GetRandomColor((int)i);
        it = s_map.insert(TTypeToColor::value_type(subtype, color)).first;
    }
    return it->second;
}



// assuming CGlPane::OpenOrtho()
void CAlignedFeatureGraph::x_RenderFeature(CGlPane& pane, const SFeatRec& rec, int layer,
                                           TModelUnit top, TModelUnit bottom)
{
    IRender& gl = GetGl();

//    const CSeq_feat& feat = *rec.m_FeatLoc.m_Feature;
    const CSeq_feat& feat = rec.GetFeature();
    const CSeq_loc& loc = rec.GetLocation();

    TFeatRange r = loc.GetTotalRange();

    const TModelRect& rc_vis = pane.GetVisibleRect();
    if(r.GetFrom() > rc_vis.Right()  || r.GetToOpen() < rc_vis.Left())  {
        return; // feature does not overlap visible range
    }

    TModelUnit off_x = pane.GetOffsetX();
    TModelUnit scale_x = pane.GetScaleX();

    TModelUnit y1, y2;
    TModelUnit x1 = r.GetFrom() - off_x;
    TModelUnit x2 = r.GetToOpen() - off_x;

    // render cluster's backgorund
    if(rec.GetClusterIndex() >= 0) {
        _ASSERT(rec.GetClusterIndex() < (int) m_Clusters.size());
        const SCluster& cluster = m_Clusters[rec.GetClusterIndex()];

        y1 = top + ((layer == cluster.m_FirstLayer) ? +2 : 0);
        y2 = bottom + ((layer == cluster.m_LastLayer) ? -2 : 0);

        gl.Color3d(0.90, 0.90, 0.90);
        TModelUnit cl_off = std::min(cluster.m_Offset / 2.0, scale_x * 3.0);
        gl.Rectd(x1 - cl_off, y1, x2 + cl_off, y2);
    }

    // render total range
    TModelUnit y_c = (top + bottom) / 2;

    CRgbaColor& c = GetColor(feat.GetData().GetSubtype());
    gl.ColorC(c);

    gl.Begin(GL_LINES);
    gl.Vertex2d(x1, y_c);
    gl.Vertex2d(x2, y_c);
    gl.End();

    // render segments
    for( CSeq_loc_CI it(loc); it; ++it) {
        CSeq_loc::TRange r = it.GetRange();

        x1 = r.GetFrom() - off_x;
        x2 = r.GetToOpen() - off_x;

        if((x2 - x1) / scale_x < 1.0)   {
            // render at least 1 pixel
            x1 = (x1 + x2 - scale_x) / 2;
            x2 = x1 + scale_x;
        }

        gl.Rectd(x1, top + kLayerOffY, x2, bottom - kLayerOffY);
    }
}


TVPPoint CAlignedFeatureGraph::PreferredSize()
{
    TVPUnit h = m_LayerH * (TVPUnit)m_Layers.size();
    return TVPPoint(0, h);
}

static const int kTootipPix = 2;

bool CAlignedFeatureGraph::NeedTooltip(CGlPane& pane, int vp_x, int vp_y)
{
    m_TooltipRec = NULL;

    if(m_VPRect.PtInRect(vp_x, vp_y))   {
        int layer = (m_VPRect.Top() - vp_y) / m_LayerH;
        if(layer >= 0  &&  layer < (int) m_Layers.size())   {
            pane.OpenOrtho();
            pane.Close();
            TModelUnit pos = pane.UnProjectX(vp_x);

            // find the closet feature
            TModelUnit min_dist = -1, dist;
            SFeatRec* min_rec = NULL;

            TFeatRecPtrs& recs = m_Layers[layer]->m_FeatRecPtrs;
            NON_CONST_ITERATE(TFeatRecPtrs, it, recs) {
                SFeatRec& rec = **it;
//                CSeq_loc& loc = *rec.m_FeatLoc.m_MappedLoc;
                const CSeq_loc& loc = rec.GetLocation();

                TFeatRange r = loc.GetTotalRange();

                // calculate the horx distance from mouse pos to the feature
                if(pos < r.GetFrom())   {
                    dist = r.GetFrom() - pos;
                } else {
                    if(pos > r.GetToOpen())  {
                        dist = pos - r.GetToOpen();
                    } else {
                        dist = 0.0;
                    }
                }

                if(min_dist < 0  || dist < min_dist)    {
                    min_rec = &rec;
                    min_dist = dist;
                }
            }

            TModelUnit d = pane.GetScaleX() * kTootipPix;
            if(min_dist < d)    {
                m_TooltipRec = min_rec;
            }
        }
        return true;
    }
    _ASSERT(false);
    return false;
}


string CAlignedFeatureGraph::GetTooltip()
{
    if(m_TooltipRec)    {
        string s = x_GetTooltip(*m_TooltipRec);
        //s += "\n\nDEBUG   Feat ptr";
        //s += NStr::PtrToString(&feat);
        return s;
    }
    return string("Feature Graph - ") + m_Label;
}


// copy of CDefaultPolicy::x_GetTitle()
// TODO - code reuse
string CAlignedFeatureGraph::x_GetTooltip(const SFeatRec& rec)
{
    string title;
    const CSeq_feat& feat = rec.GetFeature();
    const CSeq_feat::TData& data = feat.GetData();
    title = "Feature";
    const CFeatList& feats(*CSeqFeatData::GetFeatList());
    CFeatListItem item;
    if (feats.GetItemBySubtype(feat.GetData().GetSubtype(), item)) {
        title += "[" + item.GetStoragekey() + "]";
    }
    title += ": ";

    //    const CSeq_feat& feat = *rec.m_FeatLoc.m_Feature;
    string label;
    CLabel::GetLabel(feat, &label, CLabel::eContent, m_Scope.GetPointer());
    title += label;


    string feat_title;
    // Brief description goes here
    if (data.IsSeq()) {
        CLabel::GetLabel(feat, &feat_title, CLabel::eDescription, m_Scope.GetPointer());
    } else if (data.IsGene()  &&  data.GetGene().IsSetDesc()) {
        feat_title = data.GetGene().GetDesc();
    } else {
        feature::GetLabel(feat, &feat_title, feature::fFGL_Content, m_Scope.GetPointer());
    }
    if (feat_title != label) {
        // avoid showing the duplicated information
        title += "\nTitle: " + feat_title;
    }

    //CLabel::GetLabel(feat, &title, CLabel::eDescriptionBrief,
    //    m_Scope.GetPointer());

    /// add on a statement about feature lengths
    title += "\nTotal length: ";
    title += NStr::IntToString
        (feat.GetLocation().GetTotalRange().GetLength(), NStr::fWithCommas);
    if ( ! feat.GetLocation().IsInt()  &&
        !feat.GetLocation().IsPnt() ) {
            try {
                title += ";  Processed length: ";
                title += NStr::IntToString
                    (sequence::GetLength(feat.GetLocation(), m_Scope.GetPointer()),
                    NStr::fWithCommas);
            }
            catch (CException&) {
            }
    }
    if (feat.IsSetProduct()) {
        try {
            string prod_len_str;
            CBioseq_Handle h = m_Scope->GetBioseqHandle(*feat.GetProduct().GetId());
            prod_len_str += ";  Product length: ";
            prod_len_str += NStr::IntToString(h.GetBioseqLength(),
                NStr::fWithCommas);
            title += prod_len_str;
        }
        catch (CException&) {
        }
    }

    // report mapped location
    //    const CSeq_loc& loc = *rec.m_FeatLoc.m_MappedLoc;
    const CSeq_loc& loc = rec.GetLocation();

    TFeatRange r = loc.GetTotalRange();
    title += "\nPosition in alignment: start ";
    title += NStr::IntToString(r.GetFrom() + 1, NStr::fWithCommas);
    title += ", end ";
    title += NStr::IntToString(r.GetTo() + 1, NStr::fWithCommas);

    return title;
}


void CAlignedFeatureGraph::x_Clear()
{
    ITERATE(TLayers, it, m_Layers) {
        delete *it;
    }
    m_Layers.clear();
}



typedef pair<size_t, TSeqPos> TIndexLenPair; // rec index -> length
static bool s_ILPLonger(const TIndexLenPair& p1, const TIndexLenPair& p2)
{
    return p1.second > p2.second;
}

void CAlignedFeatureGraph::x_Layout()
{
    //LOG_POST("CAlignedFeatureGraph::x_Layout() - started");
    size_t feat_n = m_FeatRecs.size();

    //sort features by length of mapped locations
    typedef pair<size_t, TSeqPos>   TIndexLenPair; // rec index -> length
    vector<TIndexLenPair> feat_lens;
    feat_lens.reserve(feat_n);

    for( size_t i = 0;  i < feat_n;  i++ )   {
//        const SMappedFeatLoc& feat_loc = m_FeatRecs[i].m_FeatLoc;
//        const CSeq_loc& loc = *feat_loc.m_MappedLoc;

        const CSeq_loc& loc = m_FeatRecs[i].GetLocation();

        TFeatRange r = loc.GetTotalRange();
        int len = r.GetLength();

        feat_lens.push_back(TIndexLenPair(i, len));
    }

    // stable sort by length
    std::stable_sort(feat_lens.begin(), feat_lens.end(), s_ILPLonger);

    // place record pointers into a vector in the sorted order
    TFeatRecPtrs sorted_recs(feat_n, NULL);
    for( size_t i = 0;  i < feat_n;  i++ )   {
        size_t index = feat_lens[i].first;
        sorted_recs[i] = &m_FeatRecs[index];
    }

    // layout features
    if(m_LinkGenes) {
        x_LinkedLayout(sorted_recs);
    } else {
        x_SimpleLayout(sorted_recs);
    }

    //clean-up temporary structures
    NON_CONST_ITERATE(TCollVector, it, m_Occupied)  {
        delete *it;
    }
    m_Occupied.clear();
    m_FeatToRecFlag.clear();
}


void CAlignedFeatureGraph::x_SimpleLayout(TFeatRecPtrs& feat_recs)
{
    //LOG_POST("x_SimpleLayout()  - " << m_Label << ", " << feat_recs.size() << " features");

    size_t n = feat_recs.size();
    for( size_t i = 0;  i < n;  i++ )   {
        SFeatRec& rec = *feat_recs[i];
        x_PlaceFeature(rec);
    }
}


void CAlignedFeatureGraph::x_LinkedLayout(TFeatRecPtrs& feat_recs)
{
    //LOG_POST("x_LinkedLayout() - " << m_Label << ", " << feat_recs.size() << " features");

    size_t feat_n = feat_recs.size();

    // preapre m_FeatToRecFlag
    for( size_t i = 0;  i < feat_n;  i++ )   {
        SFeatRec& feat_rec = *feat_recs[i];
//        const CSeq_feat* feat = feat_rec.m_FeatLoc.m_Feature.GetPointer();
        const CSeq_feat* feat = &feat_rec.GetFeature();

        CSeqFeatData::ESubtype subtype = feat->GetData().GetSubtype();
        if(subtype == CSeqFeatData::eSubtype_mRNA ||
                subtype == CSeqFeatData::eSubtype_cdregion)  {
            m_FeatToRecFlag[feat] = TFeatToRecFlag::mapped_type(&feat_rec, false);
        }
    }

    // place Genes clusters (Gene + mRNAs and CDS-es)
    TFeatRecPtrs cluster;
    for( size_t i = 0;  i < feat_n;  i++ )   {
        SFeatRec& feat_rec = *feat_recs[i];
//        const CSeq_feat* feat = feat_rec.m_FeatLoc.m_Feature.GetPointer();
        const CSeq_feat* feat = &feat_rec.GetFeature();

        if(feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_gene)    {
            x_GatherCluster(feat_rec, cluster);
            x_PlaceCluster(cluster);
            cluster.clear();
        }
    }

    // place mRNA-s clusters
    for( size_t i = 0;  i < feat_n;  i++ )   {
        SFeatRec& feat_rec = *feat_recs[i];
//        const CSeq_feat* feat = feat_rec.m_FeatLoc.m_Feature.GetPointer();
        const CSeq_feat* feat = &feat_rec.GetFeature();

        if(feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_mRNA)    {
            if(x_mRNA_CDS_ToBePlaced(*feat)) {
                x_GatherCluster(feat_rec, cluster);
                x_PlaceCluster(cluster);
                cluster.clear();
            }
        }
    }

    // place CDS-es that are not linked
    for( size_t i = 0;  i < feat_n;  i++ )   {
        SFeatRec& feat_rec = *feat_recs[i];
//        const CSeq_feat* feat = feat_rec.m_FeatLoc.m_Feature.GetPointer();
        const CSeq_feat* feat = &feat_rec.GetFeature();

        if(feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_cdregion)    {
            x_TryPlaceCDSFeature(feat_rec);
        }
    }

    // place everything that has not been placed yet
    for( size_t i = 0;  i < feat_n;  i++ )   {
        SFeatRec& feat_rec = *feat_recs[i];
        const CSeq_feat* feat = &feat_rec.GetFeature();
//        const CSeq_feat* feat = feat_rec.m_FeatLoc.m_Feature.GetPointer();

        CSeqFeatData::ESubtype subtype = feat->GetData().GetSubtype();
        switch(subtype) {
        case CSeqFeatData::eSubtype_gene:
        case CSeqFeatData::eSubtype_mRNA:
        case CSeqFeatData::eSubtype_cdregion:   {
            break; // should be already placed
        }
        default:
            x_PlaceFeature(feat_rec);
        }
    }
}


// for a given features gathers or linked features and packs all features into
// provided container in proper order
void CAlignedFeatureGraph::x_GatherCluster(SFeatRec& feat_rec,
                                           TFeatRecPtrs& cluster)
{
    //CSeqFeatData::ESubtype subtype = feat_rec.m_FeatLoc.m_Feature->GetData().GetSubtype();
    CSeqFeatData::ESubtype subtype = feat_rec.GetFeature().GetData().GetSubtype();

    switch(subtype) {
    case CSeqFeatData::eSubtype_gene:   {
        cluster.push_back(&feat_rec); // add gene record

        // process linked mRNAs
        TFeatList mrna_feats;
//        const CSeq_feat& feat = *feat_rec.m_FeatLoc.m_Feature;
        const CSeq_feat& feat = feat_rec.GetFeature();


        sequence::GetMrnasForGene(feat, *m_Scope, mrna_feats);

        vector<TFeatRecPtrLenPair> pairs;
        x_GetRecsSortedByLength(mrna_feats, pairs);

        ITERATE(vector<TFeatRecPtrLenPair>, it_mrna, pairs)   {
            SFeatRec* mrna_feat_rec = it_mrna->first;
//            const CSeq_feat& mrna_feat = *mrna_feat_rec->m_FeatLoc.m_Feature;
            const CSeq_feat& mrna_feat = mrna_feat_rec->GetFeature();


            if(x_mRNA_CDS_ToBePlaced(mrna_feat)) {
                x_GatherCluster(*mrna_feat_rec, cluster);
            }
        }

        // process CDS-es linked directly to the Gene
        TFeatList cds_feats;
        sequence::GetCdssForGene(feat, *m_Scope, cds_feats);

        pairs.clear();
        x_GetRecsSortedByLength(cds_feats, pairs);

        ITERATE(vector<TFeatRecPtrLenPair>, it_cds, pairs)   {
            SFeatRec* cds_feat_rec = it_cds->first;
            //const CSeq_feat& cds_feat = *cds_feat_rec->m_FeatLoc.m_Feature;
            const CSeq_feat& cds_feat = cds_feat_rec->GetFeature();


            // check if this CDS is already included in the cluster
            TFeatRecPtrs::const_iterator it =
                std::find(cluster.begin(), cluster.end(), cds_feat_rec);
            // if not in cluster and not placed yet - place it
            if((it == cluster.end())  &&  x_mRNA_CDS_ToBePlaced(cds_feat))  {
                x_GatherCluster(*cds_feat_rec, cluster);
            }
        }
        break;
    }
    case CSeqFeatData::eSubtype_mRNA:   {
        cluster.push_back(&feat_rec); // add mRNA

        // get linked CDS
//        const CSeq_feat& feat = *feat_rec.m_FeatLoc.m_Feature;
        const CSeq_feat& feat = feat_rec.GetFeature();


        CConstRef<CSeq_feat> cds_feat = sequence::GetBestCdsForMrna(feat, *m_Scope);

        TFeatToRecFlag::iterator it = m_FeatToRecFlag.find(cds_feat.GetPointer());
        if((it != m_FeatToRecFlag.end())  &&  ! it->second.second)  {
            SFeatRec* cds_feat_rec = it->second.first;
            x_GatherCluster(*cds_feat_rec, cluster);
        }
        break;
    }
    case CSeqFeatData::eSubtype_cdregion:   {
        cluster.push_back(&feat_rec); // add CDS
        break;
    }
    default:
        break;
    }
}


static bool s_FRPLPLonger(const CAlignedFeatureGraph::TFeatRecPtrLenPair& p1,
                          const CAlignedFeatureGraph::TFeatRecPtrLenPair& p2)
{
    return p1.second > p2.second;
}

// takes a list of features, finds records corresponding to the features
// and sorts them by length of their mapped locations
void CAlignedFeatureGraph::x_GetRecsSortedByLength(const TFeatList& input,
                                        vector<TFeatRecPtrLenPair>& pairs)
{
    pairs.clear();

    ITERATE(TFeatList, it_f, input)   {
        const CSeq_feat* feat = *it_f;
        TFeatToRecFlag::iterator it = m_FeatToRecFlag.find(feat);

        if(it != m_FeatToRecFlag.end()) {
            SFeatRec* rec = it->second.first;
//            TFeatRange r = rec->m_FeatLoc.m_MappedLoc->GetTotalRange();
            TFeatRange r = rec->GetLocation().GetTotalRange();

            int len = r.GetLength();

            pairs.push_back(TFeatRecPtrLenPair(rec, len));
        }
    }
    std::stable_sort(pairs.begin(), pairs.end(), s_FRPLPLonger);
}


// returns true if the given feature is mRNA or CDS, belongs to this
// graph and have not been placed yet
bool CAlignedFeatureGraph::x_mRNA_CDS_ToBePlaced(const CSeq_feat& feat)
{
    TFeatToRecFlag::iterator it = m_FeatToRecFlag.find(&feat);
    bool found = it != m_FeatToRecFlag.end();
    return (found  &&  ! it->second.second);
}


// places all features in the cluster
void CAlignedFeatureGraph::x_PlaceCluster(TFeatRecPtrs& cluster)
{
    size_t cl_n = cluster.size();

    // calculate cluster bounds
    TFeatRange total_r;
    vector<TFeatRange> cluster_ranges(cl_n); // extents of the features

    for( size_t i = 0;  i < cluster.size();  i++ )  {
        SFeatRec* rec = cluster[i];
//        TFeatRange r = rec->m_FeatLoc.m_MappedLoc->GetTotalRange();
        TFeatRange r = rec->GetLocation().GetTotalRange();


        cluster_ranges[i] = r;
        if(i == 0)  {
            total_r = r;
        } else {
            total_r.CombineWith(r);
        }
    }

    // Extend ranges on both sides to separate this cluster from other features
    // offset on each side is the larger of 4% of total_r and kMinOffset
    static const TSeqPos kMinOffset = 500;
    TSeqPos percent = TSeqPos(total_r.GetLength() / 25); // 4%
    TSeqPos offset = std::max(percent, kMinOffset);

    for( size_t i = 0;  i < cluster.size();  i++ )  {
        TFeatRange& r = cluster_ranges[i];
        r.SetFrom(r.GetFrom() > offset ? r.GetFrom() - offset : 0);
        r.SetTo(r.GetTo() + offset);
    }

    // now find the area where we can place the whole cluster, so that all
    // features will be located in the proper order on neighbouring layers
    size_t layer_index = m_Layers.size(); // invalid index

    _ASSERT(cl_n > 0);
    size_t occ_n = m_Occupied.size();
    for( size_t j = 0; j < occ_n;  j++ ) {
        // iterate by features in the cluster
        bool clash = false;
        size_t k_end = min(cl_n, occ_n - j);
        for(  size_t k = 0;  ! clash  &&  k < k_end;  k++ )   {
            TColl& coll = *m_Occupied[j + k];
            SFeatRec* rec = cluster[k];
//            const TFeatRange& r = rec->m_FeatLoc.m_MappedLoc->GetTotalRange();
            TFeatRange r = rec->GetLocation().GetTotalRange();
            
            clash = coll.IntersectingWith(r);
        }
        if( ! clash)    {
            // all features in cluster can be placed
            layer_index = j;
            break;
        }
    }

    if(layer_index + cl_n > m_Layers.size())  {
        // need to create new layers
        size_t k_end = layer_index + cl_n;
        for(  size_t k = m_Layers.size();  k < k_end;  k++ )   {
            m_Layers.push_back(new SLayer());
            m_Occupied.push_back(new TColl);
        }
    }

    // create cluster
    m_Clusters.push_back(SCluster((int)layer_index, (int)(layer_index + cl_n - 1), (int)offset));
    size_t cluster_index = m_Clusters.size() - 1;

    // add features to the layers
    for(  size_t k = 0;  k < cl_n;  k++ )   {
        size_t index = size_t(layer_index + k);
        SFeatRec* rec = cluster[k];
        rec->SetClusterIndex((int)cluster_index);
        m_Layers[index]->m_FeatRecPtrs.push_back(rec);

        const TFeatRange& r = cluster_ranges[k];

        m_Occupied[index]->CombineWith(r); // reserve occupied space

        // mark CDS or mRNA as placed
//        const CSeq_feat* feat = rec->m_FeatLoc.m_Feature.GetPointer();
        const CSeq_feat* feat = &rec->GetFeature();


        TFeatToRecFlag::iterator it = m_FeatToRecFlag.find(feat);
        if(it != m_FeatToRecFlag.end())    {
            it->second.second = true;
        }
    }
}


void CAlignedFeatureGraph::x_TryPlaceCDSFeature(SFeatRec& rec)
{
//    const CSeq_feat* feat = rec.m_FeatLoc.m_Feature.GetPointer();
    const CSeq_feat* feat = &rec.GetFeature();


    TFeatToRecFlag::iterator it = m_FeatToRecFlag.find(feat);

    if(it != m_FeatToRecFlag.end()  &&  ! it->second.second)    {
        // this is our feature and it is not placed yet
        x_PlaceFeature(rec);
        it->second.second = true; // now placed
    }
}


// places the provided feature and updates m_Occupied accordingly
// if needed a new layer is created
void CAlignedFeatureGraph::x_PlaceFeature(SFeatRec& rec)
{
    //TFeatRange r = rec.m_FeatLoc.m_MappedLoc->GetTotalRange();
    TFeatRange r = rec.GetLocation().GetTotalRange();
    // extend the range by 1 base on both side to make 
    // sure abutting features are on different layers
    if (r.GetFrom() > 0) r.SetFrom(r.GetFrom() - 1);
    r.SetTo(r.GetTo() + 1);

    // index of the layer where the feature will be placed
    size_t layer_index = m_Layers.size(); // init with out of range value

    // iterate existing layers and try to find a place for the feature
    for( size_t j = 0; j < m_Occupied.size();  j++ ) {
        TColl& coll = *m_Occupied[j];
        if( ! coll.IntersectingWith(r)) {
            layer_index = j; // found a spot
            break;
        }
    }

    if(layer_index == m_Layers.size())  {
        // no space available - create a new layer
        m_Layers.push_back(new SLayer());
        m_Occupied.push_back(new TColl);
    }

    // add feature record to the layer
    m_Layers[layer_index]->m_FeatRecPtrs.push_back(&rec);

    // reserve occupied space
    m_Occupied[layer_index]->CombineWith(r);
}


///////////////////////////////////////////////////////////////////////////////
/// CFeatHistogramDS

CFeatHistogramDS::CFeatHistogramDS(TMappedFeatLocs& feat_locs, const string& label)
:   m_Label(label),
    m_FeatLocs(feat_locs)
{
    ITERATE(TMappedFeatLocs, it, feat_locs)    {
        const CSeq_loc& loc= *it->m_MappedLoc;
        TFeatRange r = loc.GetTotalRange();
        if(m_FeatRange.Empty())  {
            m_FeatRange = r;
        } else {
            m_FeatRange.CombineWith(r);
        }
    }
    Update(m_FeatRange.GetFrom(), m_FeatRange.GetToOpen());
}


double CFeatHistogramDS::GetLimit() const
{
    return m_FeatRange.GetToOpen();
}


double CFeatHistogramDS::GetStart() const
{
    return m_Map->GetStart();
}

double CFeatHistogramDS::GetStop() const
{
    return m_Map->GetStop();
}


double CFeatHistogramDS::GetStep() const
{
    return m_Map->GetWindow();
}


size_t CFeatHistogramDS::GetCount() const
{
    return m_Map->GetBins();
}


double  CFeatHistogramDS::GetValue(size_t index)
{
    return (*m_Map)[index];
}


double CFeatHistogramDS::GetMaxValue()
{
    return m_Map->GetMax();
}


string CFeatHistogramDS::GetLabel() const
{
    return m_Label;
}

static const int kBins = 2048;

void CFeatHistogramDS::Update(double start, double stop)
{
    //LOG_POST("CFeatHistogramDS::Update() " << start << " " << stop);

    _ASSERT(start >= 0  &&  stop <= m_FeatRange.GetToOpen() &&  start < stop);

    TSeqPos pos_start = (TSeqPos) floor(start);
    TSeqPos pos_stop = (TSeqPos) ceil(stop);

    if(m_Map.get() == NULL  ||  pos_start != m_Map->GetStart()  ||  pos_stop != m_Map->GetStop()) {
        float window = float(pos_stop - start) / float(kBins);
        m_Map.reset(new TMap(pos_start, pos_stop, window));

        ITERATE(TMappedFeatLocs, it, m_FeatLocs)    {
            const CSeq_loc& loc= *it->m_MappedLoc;
            m_Map->AddLocation(loc);
        }
    }
}


END_NCBI_SCOPE
