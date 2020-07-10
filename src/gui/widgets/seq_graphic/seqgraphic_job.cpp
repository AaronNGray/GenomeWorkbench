/*  $Id: seqgraphic_job.cpp 44110 2019-10-28 21:45:38Z shkeda $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Informationion
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
 * Authors:  Vlad Lebedev, Liangshou Wu, Victor Joukov
 *
 * File Description:
 *
 */


#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job_result.hpp>
#include <objmgr/graph_ci.hpp>
#include <objects/seqres/Byte_graph.hpp>
#include <objects/seqres/Int_graph.hpp>
#include <objects/seqres/Real_graph.hpp>

#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CSeqGraphicJob
///////////////////////////////////////////////////////////////////////////////

CSeqGraphicJob::CSeqGraphicJob(const string& desc)
    : m_Desc(desc)
    , m_Token(kInvalidJobToken)
    , m_TaskTotal(0)
    , m_TaskCompleted(0)
{
    m_Error.Reset(new CAppJobError("Job failed"));
    SetTaskName("Loading...");
}



IAppJob::EJobState CSeqGraphicJob::Run()
{
    m_Result.Reset();
    return x_Execute();
}


CConstIRef<IAppJobProgress> CSeqGraphicJob::GetProgress()
{
    CFastMutexGuard lock(m_Mutex);
    float progress = 0.0;
    if (m_TaskTotal != 0) {
        progress = (float)m_TaskCompleted/m_TaskTotal;
    }
    return CConstIRef<IAppJobProgress>
        (new CAppJobProgress(progress, m_TaskName));
}


CRef<CObject> CSeqGraphicJob::GetResult()
{
    return m_Result;
}


CConstIRef<IAppJobError> CSeqGraphicJob::GetError()
{
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}


string CSeqGraphicJob::GetDescr() const
{
    return m_Desc;
}


void CSeqGraphicJob::SetTaskName(const string& name)
{
    CFastMutexGuard lock(m_Mutex);
    m_TaskName = name;
}


void CSeqGraphicJob::SetTaskCompleted(int completed)
{
    CFastMutexGuard lock(m_Mutex);
    m_TaskCompleted = completed;
}


void CSeqGraphicJob::AddTaskCompleted(int delta)
{
    CFastMutexGuard lock(m_Mutex);
    m_TaskCompleted += delta;
}


void CSeqGraphicJob::SetTaskTotal(int total)
{
    CFastMutexGuard lock(m_Mutex);
    m_TaskTotal = total;
}


bool CSeqGraphicJob::StopRequested() const
{
    return IsCanceled();
}



///////////////////////////////////////////////////////////////////////////////
/// CSGAnnotJob
///////////////////////////////////////////////////////////////////////////////

CSGAnnotJob::CSGAnnotJob(const string& desc, CBioseq_Handle handle,
                         const SAnnotSelector& sel, const TSeqRange& range)
    : CSeqGraphicJob(desc)
    , m_Handle(handle)
    , m_Sel(sel)
    , m_Range(range)
    , m_GraphLevel(0)
{}


IAppJob::EJobState
CSGAnnotJob::x_LoadCoverageGraph(CSeqGlyph::TObjects& glyphs,
                                 int level, TModelUnit window,
                                 const string& title, bool fixed_scale, const TAxisLimits& y_limits)
{
    EJobState state = eFailed;
    SetTaskName("Loading coverage graph...");
    SAnnotSelector sel = m_Sel;
    sel.SetAnnotType(CSeq_annot::C_Data::e_Graph);
    sel.ResetAnnotsNames();

    ITERATE (SAnnotSelector::TNamedAnnotAccessions,
        iter, sel.GetNamedAnnotAccessions()) {
        sel.IncludeNamedAnnotAccession(iter->first, level);
        sel.AddNamedAnnots(CombineWithZoomLevel(iter->first, level));
    }

    state = x_CreateHistFromGraph(glyphs, sel, window, fixed_scale, y_limits);
    return state;
}


void CSGAnnotJob::x_AddGraphObject(map<string, SGraphObject>& d_maps, 
                                   const CMappedGraph& gr,
                                   TModelUnit window,
                                   bool fixed_scale)
{

    // If we are showing the coverage, we need to pick the right
    // graph to show in case there are multiple graphs, such as
    // percentil graphs generated and stored in VDB graph
    // referred tickets include SV-1646, CXX-4124, ID-1100, SADB-80
    // and SV-596

    // skip the grpah that says it is a percentile coverage graph,
    // but not 100 percentile.

    if (x_PercentileButNotMax(gr))
        return;

    string g_title = x_GetGraphName(gr);
    string g_name = CSeqUtils::GetUnnamedAnnot();
    if (gr.GetAnnot().IsNamed()) {
        g_name = gr.GetAnnot().GetName();
    }
    auto iter = d_maps.find(g_title);
    if (iter == d_maps.end()) {
        string g_desc = CSeqUtils::GetAnnotComment(gr.GetAnnot());
        iter = d_maps.insert(
            pair<string, SGraphObject>(
            g_title,
            SGraphObject(
            m_Range.GetFrom(),
            m_Range.GetTo(),
            (float)window,
            new CHistogramGlyph::max_func,
            g_desc, g_name))).first;
    }
    SGraphObject& g_obj = iter->second;
    switch (gr.GetGraph().Which()) {
    case CSeq_graph::TGraph::e_Byte:
    {{
            const CByte_graph& graph = gr.GetGraph().GetByte();
            x_AddGraphToMap(g_obj.m_Map, gr, graph,
                            g_obj.m_Max, g_obj.m_Min, fixed_scale);

            //x_AddGraphToMap(g_obj.m_Map, gr, graph.GetValues(),
            //                graph.GetMax(), graph.GetMin(), g_obj.m_Max,
            //                g_obj.m_Min, fixed_scale);
            break;
    }}
    case CSeq_graph::TGraph::e_Int:
    {{
            const CInt_graph& graph = gr.GetGraph().GetInt();
            x_AddGraphToMap(g_obj.m_Map, gr, graph,
                            g_obj.m_Max, g_obj.m_Min, fixed_scale);
            break;
        }}
    case CSeq_graph::TGraph::e_Real:
    {{
            const CReal_graph& graph = gr.GetGraph().GetReal();
            x_AddGraphToMap(g_obj.m_Map, gr, graph,
                            g_obj.m_Max, g_obj.m_Min, fixed_scale);
            break;
        }}
    default:
        break;
    }

}


IAppJob::EJobState
CSGAnnotJob::x_CreateHistFromGraph(CSeqGlyph::TObjects& glyphs,
                                   const SAnnotSelector& sel,
                                   TModelUnit window,
                                   bool fixed_scale,
                                   const TAxisLimits& y_limits)
{
    CConstRef<CSeq_loc> loc(
        m_Handle.GetRangeSeq_loc(m_Range.GetFrom(), m_Range.GetTo()) );
    CGraph_CI graph_iter(m_Handle.GetScope(), *loc, sel);
    // We want to merge graphs with the same graph name.
    // Graphs will have the same name if they have the same annotation
    // name and graph title .
    typedef map<string, SGraphObject> TDMaps;
    TDMaps d_maps;

    for (; graph_iter; ++graph_iter) {
        if (IsCanceled()) 
            return eCanceled;
        x_AddGraphObject(d_maps, *graph_iter, window, fixed_scale);
    }

    NON_CONST_ITERATE (TDMaps, iter, d_maps) {
        SGraphObject& g_obj = iter->second;
        CRef<CHistogramGlyph> hist = Ref(new CHistogramGlyph(g_obj.m_Map, iter->first));
        //CRef<CSeqGlyph> fref(hist);
        hist->SetDesc(g_obj.m_Desc);
        hist->SetAnnotName(g_obj.m_AnnotName);
        hist->SetAxisMaxFixed(max(y_limits.max, g_obj.m_Max));
        hist->SetAxisMinFixed(min(y_limits.min, g_obj.m_Min));
        hist->SetFixedScale(fixed_scale);
        glyphs.emplace_back(hist.GetPointer());
    }

    return eCompleted;
}

/*
template <typename ValType>
void CSGAnnotJob::x_AddGraphToMap(CDensityMap<float>& the_map,
                                  const CMappedGraph& gr,
                                  const vector<ValType>& vec,
                                  double l_max, double l_min,
                                  double& g_max, double& g_min,
                                  bool fixed_scale)
{
    if (vec.empty()) return;

    typedef CHistogramGlyph::TDataType TDataType;
    TSeqRange r  = gr.GetLoc().GetTotalRange();
    if (r.GetLength() == 0)
        return;
    TSeqRange map_r = r.IntersectionWith(m_Range);
    TSeqPos pos  = r.GetFrom();
    double a = gr.IsSetA() ? gr.GetA() : 1.0;
    double b = gr.IsSetB() ? gr.GetB() : 0.0;

    TSeqPos comp = gr.IsSetComp() ? gr.GetComp() : 1;
    if (comp == 0) {
        // Ignore this graph.
        // The 'comp' value is set incorrectly. Do not try to reinterprete it
        // in any other way. See GP-4721 (comment-2638485)
        return;
    }

    // skip the uninterested range
    typename vector<ValType>::const_iterator iter_gr = vec.begin();
    int offset = (map_r.GetFrom() - pos) / comp;
    advance(iter_gr, min(offset, (int)vec.size()));
    pos += offset * comp;

    // if the data type is char, we need to convert them into unsigned char
    bool unsigned_char = false;
    if (sizeof(ValType) == 1) unsigned_char = true;
    TDataType val;
    while (iter_gr != vec.end()  &&  pos < map_r.GetToOpen()) {
        TSeqRange range = TSeqRange(pos, pos + comp - 1);
        //if (unsigned_char) {
            //val = (TDataType)((unsigned char)(*iter_gr) * a + b);
        //} else {
            //val = (TDataType)((*iter_gr) * a + b);
        //}
        if (unsigned_char) {
            val = (TDataType)(unsigned char)(*iter_gr);
        } else {
            val = (TDataType)(*iter_gr);
        }
        if (val < l_min)
            val = l_min;
        else if (val > l_max) 
            val = l_max;

        val = val * a + b;
        the_map.AddRange(range, val);
        pos += comp;
        ++iter_gr;
    }

    //if (fixed_scale) {
        g_max = max(l_max * a + b, g_max);
        g_min = min(l_min * a + b, g_min);
    //}
}
*/

template <typename TGraphType>
void CSGAnnotJob::x_AddGraphToMap(CDensityMap<float>& the_map,
                                  const CMappedGraph& gr,
                                  const TGraphType& graph,
                                  double& g_max, double& g_min,
                                  bool fixed_scale)
{
    const auto& vec = graph.GetValues();
    if (vec.empty()) return;
    auto l_max = graph.GetMax();
    auto l_min = graph.GetMin();

    typedef CHistogramGlyph::TDataType TDataType;
    TSeqRange r  = gr.GetLoc().GetTotalRange();
    if (r.GetLength() == 0)
        return;
    TSeqRange map_r = r.IntersectionWith(m_Range);
    TSeqPos pos  = r.GetFrom();
    double a = gr.IsSetA() ? gr.GetA() : 1.0;
    double b = gr.IsSetB() ? gr.GetB() : 0.0;

    TSeqPos comp = gr.IsSetComp() ? gr.GetComp() : 1;
    if (comp == 0) {
        // Ignore this graph.
        // The 'comp' value is set incorrectly. Do not try to reinterprete it
        // in any other way. See GP-4721 (comment-2638485)
        return;
    }

    // skip the uninterested range
    //typename vector<ValType>::const_iterator 
    auto iter_gr = vec.begin();
    int offset = (map_r.GetFrom() - pos) / comp;
    advance(iter_gr, min(offset, (int)vec.size()));
    pos += offset * comp;

    // if the data type is char, we need to convert them into unsigned char
    bool unsigned_char = false;
    typedef typename TGraphType::TValues TGraphValues;
    typedef typename TGraphValues::value_type TGraphValueType;
    if (sizeof(TGraphValueType) == 1) unsigned_char = true;
    TDataType val;
    while (iter_gr != vec.end()  &&  pos < map_r.GetToOpen()) {
        TSeqRange range = TSeqRange(pos, pos + comp - 1);
        //if (unsigned_char) {
            //val = (TDataType)((unsigned char)(*iter_gr) * a + b);
        //} else {
            //val = (TDataType)((*iter_gr) * a + b);
        //}
        if (unsigned_char) {
            val = (TDataType)(unsigned char)(*iter_gr);
        } else {
            val = (TDataType)(*iter_gr);
        }
        if (val < l_min)
            val = l_min;
        else if (val > l_max) 
            val = l_max;

        val = val * a + b;
        the_map.AddRange(range, val);
        pos += comp;
        ++iter_gr;
    }

    //if (fixed_scale) {
        g_max = max(l_max * a + b, g_max);
        g_min = min(l_min * a + b, g_min);
    //}
}


string CSGAnnotJob::x_GetGraphName(const CMappedGraph& gr) const
{
    string name = kEmptyStr;

    /// We should favor title over annotation name.
    if (gr.IsSetTitle()) {
        name = gr.GetTitle();
    } else {
        name = CSeqUtils::GetAnnotName(gr.GetAnnot());
    }

    return name;
}


bool CSGAnnotJob::x_PercentileButNotMax(const CMappedGraph& gr) const
{
    CSeq_annot_Handle annot_handle = gr.GetAnnot();
    CConstRef<CSeq_annot> annot = annot_handle.GetCompleteSeq_annot();
    if (annot->CanGetDesc()) {
        ITERATE (CSeq_annot::TDesc::Tdata, iter, annot->GetDesc().Get()) {
            if ((*iter)->IsUser()  &&  (*iter)->GetUser().GetType().IsStr()  &&
                (*iter)->GetUser().GetType().GetStr() == "AnnotationTrack") {
                CConstRef<CUser_field> field = (*iter)->GetUser().GetFieldRef("StatisticsType");
                if (field  &&  field->GetData().IsStr()  &&
                    field->GetData().GetStr() == "Percentiles"  &&
                    gr.IsSetComment()  &&  gr.GetComment() != "100%") {
                        return true;
                }
            }
        }
    }

    return false;
}


END_NCBI_SCOPE
