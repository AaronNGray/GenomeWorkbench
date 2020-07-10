/*  $Id: hit_matrix_renderer.cpp 44757 2020-03-05 18:58:50Z evgeniev $
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

#include <ncbi_pch.hpp>

#include <gui/widgets/hit_matrix/hit_matrix_renderer.hpp>

#include <gui/widgets/hit_matrix/hit_matrix_ds.hpp>
#include <gui/widgets/hit_matrix/density_map_ds.hpp>

#include <gui/opengl/gltexturefont.hpp>
#include <gui/opengl/glutils.hpp>

#include <objmgr/util/sequence.hpp>
#include <gui/objutils/label.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CHitMatrixRenderer::CHitMatrixRenderer()
:   m_ShowRulers(true),
    m_ShowGrid(true),
    m_BottomRuler(true),
    m_TopRuler(true),
    m_LeftRuler(false),
    m_RightRuler(false),
    m_QueryGraph(NULL),
    m_SubjectGraph(NULL),
    m_BottomContainer(CGraphContainer::eHorzStack),
    m_LeftContainer(CGraphContainer::eVertStack),
    m_BackColor(1.0f, 1.0f, 1.0f),
    m_TextFont(NULL),
    m_SeqFont(NULL),
    m_HitColoringParams(NULL)
{
    m_LeftRuler.SetLabelOrientation(CGlTextureFont::fFontRotateBase, 90);
    m_BottomRuler.SetLabelOrientation(CGlTextureFont::fFontRotateBase, 0);

    m_MatrixPane.EnableOffset(true);
    m_SubjectPane.EnableOffset(true);
    m_QueryPane.EnableOffset(true);

    m_Gen.SetIntegerMode(true, true);
    m_Gen.EnableOneBased(false, false);
    m_Grid.EnableIntegerCentering(true);

    m_SeqFont = new CGlTextureFont(CGlTextureFont::eFontFace_BPMono, 12);

    m_SubjectGraph = new CHitMatrixSeqGraph(true);
    m_SubjectGraph->SetFonts(m_SeqFont);

    m_QueryGraph = new CHitMatrixSeqGraph(false);
    m_QueryGraph->SetFonts(m_SeqFont);

    x_SetupGraphTypes();
    x_SetupAxes();
}


CHitMatrixRenderer::~CHitMatrixRenderer()
{
    x_DestroyGraphs(m_BottomContainer);
    x_DestroyGraphs(m_LeftContainer);
}


void CHitMatrixRenderer::ShowRulers(bool show)
{
    m_ShowRulers = show;
}


void CHitMatrixRenderer::ShowGrid(bool show)
{
    m_ShowGrid = show;
}


void CHitMatrixRenderer::SetBackgroundColor(CRgbaColor& color)
{
    m_BackColor = color;
}


CGlPane& CHitMatrixRenderer::GetMatrixPane()
{
    return m_MatrixPane;
}


CGlPane& CHitMatrixRenderer::GetSubjectPane()
{
    return m_SubjectPane;
}


CGlPane& CHitMatrixRenderer::GetQueryPane()
{
    return m_QueryPane;
}


TVPRect CHitMatrixRenderer::GetRect(EArea area)
{
    switch(area)    {
    case fMatrixArea:
        return m_rcMatrix;

    case fSubjectGraphs:
        return m_rcBottomContainer;

    case fSubjectRuler:
        return m_rcBottomRuler;

    case fQueryGraphs:
        return m_rcLeftContainer;

    case fQueryRuler:
        return m_rcLeftRuler;

    default:
        _ASSERT(false);
        return TVPRect();
    }
}


// TODO keep here
void CHitMatrixRenderer::x_SetupAxes()
{
    m_BottomRuler.SetColor(CRuler::eBackground, m_BackColor);
    m_TopRuler.SetColor(CRuler::eBackground, m_BackColor);
    m_LeftRuler.SetColor(CRuler::eBackground, m_BackColor);
    m_RightRuler.SetColor(CRuler::eBackground, m_BackColor);

    m_LeftRuler.SetDisplayOptions(CRuler::fShowTextLabel);
    m_BottomRuler.SetDisplayOptions(CRuler::fShowTextLabel);

    m_TopRuler.SetHorizontal(true, CRuler::eTop);
    m_RightRuler.SetHorizontal(false, CRuler::eRight);

    m_LeftRuler.SetLabelOrientation(CGlTextureFont::fFontRotateBase, 90);
    m_BottomRuler.SetLabelOrientation(CGlTextureFont::fFontRotateBase, 0);

    m_RightRuler.SetLabelOrientation(CGlTextureFont::fFontRotateBase, 90); 
    m_TopRuler.SetLabelOrientation(CGlTextureFont::fFontRotateBase, 0);

}


void CHitMatrixRenderer::Render(const CGlPane& port, bool pdf)
{
    if (!pdf) {
        glClearColor(m_BackColor.GetRed(), m_BackColor.GetGreen(),
                     m_BackColor.GetBlue(), m_BackColor.GetAlpha());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLineWidth(10.0f);
    }

    // prepare CGlPanes
    TModelRect rc_M = port.GetModelLimitsRect();
    m_MatrixPane.SetModelLimitsRect(rc_M);

    TModelRect rc_V = port.GetVisibleRect();
    m_MatrixPane.SetVisibleRect(rc_V);

    if (m_ShowGrid  ||  m_ShowRulers) {
        x_AdjsutToMasterPane(port, m_SubjectPane, true, false);
        x_AdjsutToMasterPane(port, m_QueryPane, false, true);

        // now rendering
        x_RenderAxisAndGrid(port, m_MatrixPane);
    }

    x_RenderContainers(port);

    m_Graph.Render(m_MatrixPane);
}


void CHitMatrixRenderer::Update(IHitMatrixDataSource* ds, const CGlPane& port)
{
    m_Graph.DeleteGlyphs();

    m_DataSource = ds;

    if(m_DataSource) {
        CBioseq_Handle q_handle = m_DataSource->GetQueryHandle();
        CBioseq_Handle s_handle = m_DataSource->GetSubjectHandle();

        const IHitMatrixDataSource::THitAdapterCont&   hits = m_DataSource->GetHits();
        ITERATE (IHitMatrixDataSource::THitAdapterCont, it, hits) {
            m_Graph.CreateGlyph(**it);
        }

        m_Graph.AssignColorsByScore(m_HitColoringParams);

        m_SubjectGraph->SetBioseqHandle(s_handle);
        m_QueryGraph->SetBioseqHandle(q_handle);

        x_SetRulerText(m_DataSource);
    } else {
        CBioseq_Handle h_null;
        m_SubjectGraph->SetBioseqHandle(h_null);
        m_QueryGraph->SetBioseqHandle(h_null);
    }

    Layout(port);
}


static const size_t kMaxRulerLabel = 60;

void CHitMatrixRenderer::x_SetRulerText(IHitMatrixDataSource* ds)
{
    string s_label, q_label;
    if(ds) {
        CBioseq_Handle q_handle = ds->GetQueryHandle();
        CBioseq_Handle s_handle = ds->GetSubjectHandle();

        /// subject label
        s_label = ds->GetLabel(ds->GetSubjectId());
        //LOG_POST(Info << "CHitMatrixRenderer::x_SetRulerText() subject " << s_label);
        if (s_handle)    {
            s_label += ": ";
            s_label += sequence::CDeflineGenerator().GenerateDefline(s_handle);
        }
        if(s_label.size() > kMaxRulerLabel)   {
            s_label.resize(kMaxRulerLabel);
            s_label += "...";
        }

        /// query label
        q_label = ds->GetLabel(ds->GetQueryId());
        if (q_handle)    {
            q_label += ": ";
            q_label += sequence::CDeflineGenerator().GenerateDefline(q_handle);
        }
        //LOG_POST(Info << "CHitMatrixRenderer::x_SetRulerText() query " << q_label);
        if(q_label.size() > kMaxRulerLabel)   {
            q_label.resize(kMaxRulerLabel);
            q_label += "...";
        }
    }

    m_BottomRuler.SetTextLabel(s_label);
    m_LeftRuler.SetTextLabel(q_label);
}


void CHitMatrixRenderer::x_DestroyGraphs(CGraphContainer& cont)
{
    size_t n = cont.GetGraphsCount();
    for( size_t ct = 0;  ct < n;  ct++ )    {
        IRenderable* r = cont.GetGraph(ct);

        CHistogramGraph* histogram = dynamic_cast<CHistogramGraph*>(r);
        if(histogram)   {
            histogram->SetDataSource(NULL);
        }
    }
    cont.RemoveAllGraphs();
}


CHistogramGraph* CHitMatrixRenderer::x_CreateHistogram(ESequence seq, const SGraphDescr& descr)
{
    CHistogramGraph* graph = new CHistogramGraph(seq == eSubject);

    // set color
    CHistogramGraph::SProperties props;
    props.m_MaxColor = descr.m_Color;
    graph->SetProperties(props);

    // create data source
    CBioseq_Handle handle = (seq == eSubject) ? m_DataSource->GetSubjectHandle()
                                              : m_DataSource->GetQueryHandle();
    CDensityMapDS* ds = new CDensityMapDS(handle, descr.m_Type);
    graph->SetDataSource(ds);
    return graph;
}


void CHitMatrixRenderer::ColorByScore(const SHitColoringParams* params)
{
    m_HitColoringParams = params;

    if(m_DataSource) {
        m_Graph.AssignColorsByScore(m_HitColoringParams);
    }
}


const string& CHitMatrixRenderer::GetScoreName() const
{
    static string s_emp;
    return m_HitColoringParams ? m_HitColoringParams->m_ScoreName : s_emp;
}


TModelRect  CHitMatrixRenderer::GetSelectedHitElemsRect() const
{
    const TElemGlyphSet& glyphs = m_Graph.GetSelectedGlyphs();
    bool b_first = true;

    int left, right, top, bottom;
    ITERATE( CHitMatrixRenderer::TElemGlyphSet, it_G, glyphs)   {
        const IHitElement& elem = (*it_G)->GetHitElem();

        int s_from = elem.GetSubjectStart();
        int q_from = elem.GetQueryStart();
        int s_len = elem.GetSubjectLength();
        int q_len = elem.GetQueryLength();
        int s_to = s_from + s_len;
        int q_to = q_from + q_len;

        if(q_from > q_to)
            swap(q_from, q_to);
        if(s_from > s_to)
            swap(s_from, s_to);

        if(b_first) {
            left = s_from;            right = s_to;
            bottom = q_from;          top = q_to;
            b_first = false;
        } else {
            left = min(left, s_from);
            right = max(right, s_to);
            bottom = min(bottom, q_from);
            top = max(top, q_to);
        }
    }
    return b_first ? TModelRect(0, 0) : TModelRect(left, bottom, right + 1, top + 1);
}

/*
Preserve for possible future use

// populates the given collection with projections of the selected hit
// elements on the Subject sequence
void CHitMatrixRenderer::ProjectSelectedElems(TRangeColl& coll, ESequence seq) const
{
    coll.clear();
    const TElemGlyphSet& glyphs = m_Graph.GetSelectedGlyphs();

    ITERATE( CHitMatrixRenderer::TElemGlyphSet, it_G, glyphs)   {
        const IHitElement& elem = (*it_G)->GetHitElem();

        int from, to;
        if(seq == eSubject) {
            from = elem.GetSubjectStart();
            to = from + elem.GetSubjectLength() - 1;
        } else {
            from = elem.GetQueryStart();
            to = from + elem.GetQueryLength() - 1;
        }

        if(from > to)   {
            swap(from, to);
        }
        TRangeColl::TRange r(from, to);
        coll.CombineWith(r);
    }
}
*/
static const char* kSeqGraph = "Sequence";

void CHitMatrixRenderer::x_SetupGraphTypes()
{
    vector<string> types;
    types.push_back(kSeqGraph);
    CDensityMapDS::GetGraphTypes(types);

    m_GraphTypes.resize(types.size());

    for( size_t ct = 0;  ct < types.size();  ct++ )  {
        m_GraphTypes[ct].m_Type = types[ct];
        m_GraphTypes[ct].m_HasColor = true;
    }

    m_GraphTypes[0].m_HasColor = false; // seq graph
}


void CHitMatrixRenderer::GetGraphTypes(vector<SGraphDescr>& types) const
{
    ITERATE(TGraphTypes, it, m_GraphTypes)  {
        types.push_back(*it);
    }
}


void CHitMatrixRenderer::SetGraphColor(const string& name,
                                      const CRgbaColor& color)
{
    NON_CONST_ITERATE(TGraphTypes, it, m_GraphTypes)  {
        if(it->m_Type == name)  {
            it->m_Color = color;
            return;
        }
    }
}


void CHitMatrixRenderer::GetGraphs(ESequence seq, vector<string>& graphs) const
{
    const CGraphContainer& cont = (seq == eSubject) ? m_BottomContainer : m_LeftContainer;

    size_t n = cont.GetGraphsCount();
    for(size_t ct = 0; ct < n;  ct++ )   {
        const IRenderable* r = cont.GetGraph(ct);
        const CHistogramGraph* graph = dynamic_cast<const CHistogramGraph*>(r);
        string s = graph ? graph->GetDataSource()->GetLabel() : string(kSeqGraph);
        graphs.push_back(s);
    }
    std::reverse(graphs.begin(), graphs.end());
}


void CHitMatrixRenderer::SetGraphs(ESequence seq, const vector<string>& graphs)
{
    x_SetGraphs(seq, graphs);
}


void CHitMatrixRenderer::x_SetGraphs(ESequence seq, const vector<string>& graphs)
{
    bool horz = (seq == eSubject);
    CGraphContainer& cont = horz ? m_BottomContainer : m_LeftContainer;

    typedef map<string, CIRef<IRenderable> > TMap;
    TMap name_to_graph;

    size_t n = cont.GetGraphsCount();
    for(size_t ct = 0; ct < n; ct++ )   {
        IRenderable* r = cont.GetGraph(ct);
        CHistogramGraph* graph = dynamic_cast<CHistogramGraph*>(r);
        string s = graph ? graph->GetDataSource()->GetLabel() : string(kSeqGraph);
        name_to_graph[s] = CIRef<IRenderable>(r);
    }
    cont.RemoveAllGraphs();

    // create graphs
    for( int ct = (int) graphs.size() - 1;  ct >= 0; ct-- )   {
        const string& name = graphs[ct];
        TMap::iterator it = name_to_graph.find(name);

        CIRef<IRenderable> graph;
        if(it != name_to_graph.end())   {
            graph = it->second;
            x_UpdateGraphColor(graph.GetPointer());
            name_to_graph.erase(it);
        } else {
            if(name == kSeqGraph)   {
                graph = horz ? m_SubjectGraph : m_QueryGraph;
            } else {
                ITERATE(TGraphTypes, iter, m_GraphTypes)  {
                    const string& type = iter->m_Type;
                    if(type == name)  {
                        graph = x_CreateHistogram(seq, *iter);
                    }
                }
            }
        }

        cont.AddGraph(graph.GetPointer());
    }

    cont.Layout();
}


void CHitMatrixRenderer::x_UpdateGraphColor(IRenderable* graph)
{
    CHistogramGraph* histogram = dynamic_cast<CHistogramGraph*>(graph);
    if(histogram)   {
        CRgbaColor color;

        const string& name = histogram->GetDataSource()->GetLabel();
        ITERATE(TGraphTypes, it, m_GraphTypes)  {
            if(it->m_Type == name)  {
                color = it->m_Color;
                break;
            }
        }

        CHistogramGraph::SProperties props = histogram->GetProperties();
        props.m_MaxColor = color;
        histogram->SetProperties(props);
    }
}


void CHitMatrixRenderer::Resize(int w, int h, const CGlPane& port)
{
    m_Size = TVPPoint(w, h);

    Layout(port); //TODO ?
}


/// recalculates recatngles for all intrnal objects so that they are positioned
/// correctly with regard to the new windows size
void CHitMatrixRenderer::Layout(const CGlPane& port)
{
    TVPPoint subj_cont_size = m_BottomContainer.PreferredSize();
    TVPPoint query_cont_size = m_LeftContainer.PreferredSize();

    TVPPoint sz_left_ruler, sz_bottom_ruler;
    const TModelRect& rc_m = port.GetModelLimitsRect();
    if (m_ShowRulers  &&  rc_m.Width() > 0  &&  rc_m.Height() > 0) {
        int max_num_y = (int) ceil(rc_m.Top());
        sz_left_ruler = m_LeftRuler.GetPreferredSize(max_num_y);
        int max_num_x = (int) ceil(rc_m.Right());
        sz_bottom_ruler = m_BottomRuler.GetPreferredSize(max_num_x);
    }

    // calculate bounds of the matrix
    int mx_left = query_cont_size.X()+ sz_left_ruler.X();
    int mx_bottom = subj_cont_size.Y() + sz_bottom_ruler.Y() + 2;
    int mx_right = m_Size.m_X - 2; // - sz_left_ruler.X();
    int mx_top = m_Size.m_Y - 2; // - sz_bottom_ruler.Y();

    m_rcMatrix.Init(mx_left, mx_bottom, mx_right, mx_top);
    m_rcAxes.Init(query_cont_size.X(), subj_cont_size.Y(), m_Size.m_X - 1, m_Size.m_Y - 1);

    // set Rulers rects
    m_rcBottomRuler.Init(mx_left, m_rcAxes.Bottom(), mx_right, mx_bottom - 1);
    m_rcLeftRuler.Init(m_rcAxes.Left(), mx_bottom, mx_left - 1, mx_top);

    // set Graph Containers
    m_rcBottomContainer.Init(mx_left, 0, mx_right, m_rcAxes.Bottom() - 1);
    m_BottomContainer.SetVPRect(m_rcBottomContainer);

    m_rcLeftContainer.Init(0, mx_bottom, m_rcAxes.Left() - 1, mx_top);
    m_LeftContainer.SetVPRect(m_rcLeftContainer);

    // adjust panes
    m_MatrixPane.SetViewport(m_rcMatrix);

    {
        TVPRect rc_vp = m_rcBottomRuler;
        rc_vp.CombineWith(m_rcBottomContainer);
        m_SubjectPane.SetViewport(rc_vp);
        x_AdjsutToMasterPane(port, m_SubjectPane, true, false);

        rc_vp = m_rcLeftRuler;
        rc_vp.CombineWith(m_rcLeftContainer);
        m_QueryPane.SetViewport(rc_vp);
        x_AdjsutToMasterPane(port, m_QueryPane, false, true);
    }
}

// TODO keep here
void CHitMatrixRenderer::x_RenderAxisAndGrid(const CGlPane& port, CGlPane& gr_pane)
{
    if (gr_pane.GetViewport().Width() < 20 || 
        gr_pane.GetViewport().Height() < 20)
            return;

    CGlPane pane(CGlPane::eAlwaysUpdate);
    pane.EnableOffset(true);
    pane.SetViewport(m_rcAxes);

    if (m_ShowGrid) {
        x_AdjsutToMasterPane(port, pane, true, true);
        m_Grid.Render(&pane, &gr_pane, &m_Gen);
    }

    if (m_ShowRulers) {
        // draw horizontal rulers
        // draw bottom ruler
        TVPRect rc_VP(m_rcMatrix.Left(), m_rcAxes.Bottom(),
                      m_rcMatrix.Right(), m_rcMatrix.Bottom() - 1);
        pane.SetViewport(rc_VP);
        x_AdjsutToMasterPane(port, pane, true, false);

        m_BottomRuler.Render(pane);

        // draw vertical rulers
        // draw left ruler
        rc_VP.Init(m_rcAxes.Left(), m_rcMatrix.Bottom(),
                   m_rcMatrix.Left() - 1, m_rcMatrix.Top());
        pane.SetViewport(rc_VP);
        x_AdjsutToMasterPane(port, pane, false, true);

        m_LeftRuler.Render(pane);
    }
}


void CHitMatrixRenderer::x_RenderContainers(const CGlPane& port)
{
    CGlPane pane;
    pane.EnableOffset(true);

    pane.SetViewport(m_rcBottomContainer);
    x_AdjsutToMasterPane(port, pane, true, false);
    m_BottomContainer.SetModelRect(pane.GetVisibleRect());
    m_BottomContainer.Render(pane);

    pane.SetViewport(m_rcLeftContainer);
    x_AdjsutToMasterPane(port, pane, false, true);
    m_LeftContainer.SetModelRect(pane.GetVisibleRect());

    m_LeftContainer.Render(pane);
}


void CHitMatrixRenderer::x_AdjsutToMasterPane(const CGlPane& port, CGlPane& pane, bool b_model_x, bool b_model_y)
{
    TModelRect rc_vis = port.GetVisibleRect();
    TModelRect rc_lim = port.GetModelLimitsRect();

    // assuming that Viewport in the pane has been set properly
    if(! b_model_x) { // adjust horz range to represent pixels
        int max_x = pane.GetViewport().Width() - 1;
        rc_lim.SetHorz(0, max_x);
        rc_vis.SetHorz(0, max_x);
    }

    if(! b_model_y) { // adjust vert range to represent pixels
        int max_y = pane.GetViewport().Height() - 1;
        rc_lim.SetVert(0, max_y);
        rc_vis.SetVert(0, max_y);
    }
    pane.SetModelLimitsRect(rc_lim);
    pane.SetVisibleRect(rc_vis);
}


int CHitMatrixRenderer::GetAreaByVPPos(TVPUnit vp_x, TVPUnit vp_y)
{
    if(m_rcMatrix.PtInRect(vp_x, vp_y)) {
        return fMatrixArea;
    } else if(m_rcBottomRuler.PtInRect(vp_x, vp_y)) {
        return fSubjectRuler;
    } else if(m_rcLeftRuler.PtInRect(vp_x, vp_y)) {
        return fQueryRuler;
    } else if(m_rcBottomContainer.PtInRect(vp_x, vp_y)) {
        return fSubjectGraphs;
    } else if(m_rcLeftContainer.PtInRect(vp_x, vp_y)) {
        return fQueryGraphs;
    }
    return fOther;
}


bool CHitMatrixRenderer::NeedTooltip(int area, int vp_x, int vp_y)
{
    switch(area)    {
    case CHitMatrixRenderer::fSubjectGraphs:
        return m_BottomContainer.NeedTooltip(m_SubjectPane, vp_x, vp_y);

    case CHitMatrixRenderer::fQueryGraphs:
        return m_LeftContainer.NeedTooltip(m_QueryPane, vp_x, vp_y);

        default:
        return false;
    };
}


string CHitMatrixRenderer::GetTooltip(int area)
{
    switch(area)    {
    case CHitMatrixRenderer::fSubjectGraphs:
        return m_BottomContainer.GetTooltip();

    case CHitMatrixRenderer::fQueryGraphs:
        return m_LeftContainer.GetTooltip();

    default:
        _ASSERT(false);
        return "";
    };
}


END_NCBI_SCOPE
