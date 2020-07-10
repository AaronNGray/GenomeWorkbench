/*  $Id: hit_matrix_pane.cpp 42775 2019-04-11 14:59:28Z katargir $
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

#include <corelib/ncbistl.hpp>
#include <corelib/ncbitime.hpp>

#include <gui/widgets/hit_matrix/hit_matrix_pane.hpp>
#include <gui/widgets/hit_matrix/hit_matrix_widget.hpp>
#include <gui/widgets/hit_matrix/density_map_ds.hpp>
#include <gui/widgets/hit_matrix/dense_hit.hpp> //TODO

#include <gui/objutils/label.hpp>
#include <gui/utils/view_event.hpp>

#include <gui/opengl/glresmgr.hpp>

#include <objmgr/util/sequence.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

BEGIN_EVENT_TABLE(CHitMatrixPane, CGlWidgetPane)
    EVT_SIZE(CHitMatrixPane::OnSize)
END_EVENT_TABLE()


CHitMatrixPane::CHitMatrixPane(CHitMatrixWidget* parent)
:   CGlWidgetPane(parent, wxID_ANY),
    m_HorzSelHandler(eHorz),
    m_VertSelHandler(eVert),
    m_TooltipArea(0)
{
    if(GetParent())   {
        // TODO this is wrong, parent must establish conections
        CHitMatrixWidget* parent = x_GetParent();
        AddListener(parent, ePool_Parent);
    }

    // setup Event Handlers
    m_HorzSelHandler.SetHost(this);
    x_RegisterHandler(&m_HorzSelHandler,
                      CHitMatrixRenderer::fSubjectRuler | CHitMatrixRenderer::fSubjectGraphs,
                      &m_Renderer.GetSubjectPane());

    m_VertSelHandler.SetHost(this);
    x_RegisterHandler(&m_VertSelHandler,
                      CHitMatrixRenderer::fQueryRuler | CHitMatrixRenderer::fQueryGraphs,
                      &m_Renderer.GetQueryPane());

    CHitMatrixGraph& graph = m_Renderer.GetGraph();
    graph.SetHost(this);
    x_RegisterHandler(&graph, CHitMatrixRenderer::fMatrixArea, &m_Renderer.GetMatrixPane());

    m_MouseZoomHandler.SetHost(this);
    x_RegisterHandler(&m_MouseZoomHandler, CHitMatrixRenderer::fMatrixArea, &m_Renderer.GetMatrixPane());

    m_TooltipHandler.SetMode(CTooltipHandler::eHideOnMove);
    m_TooltipHandler.SetHost(this);
    x_RegisterHandler(&m_TooltipHandler, CHitMatrixRenderer::fAllAreas,
                      &m_Renderer.GetMatrixPane());
}


CHitMatrixPane::~CHitMatrixPane()
{
    CHitMatrixWidget* parent = x_GetParent();
    if(parent)   { // disconnect old parent
        RemoveListener(parent);
    }
}

TVPPoint CHitMatrixPane::GetPortSize(void)
{
    const TVPRect& rc = m_Renderer.GetRect(CHitMatrixRenderer::fMatrixArea);
    return TVPPoint(rc.Width(), rc.Height());
}


void CHitMatrixPane::OnSize(wxSizeEvent& event)
{
    //Layout();
    wxSize sz = GetSize();
    m_Renderer.Resize(sz.x, sz.y, x_GetParent()->GetPort());

    event.Skip();
}


void CHitMatrixPane::SetQueryVisibleRange(const TSeqRange& range)
{
    if (x_GetParent()) {
        const CGlPane& VP = x_GetParent()->GetPort();

        // prepare CGlPanes
        TModelRect rc_V = VP.GetVisibleRect();
        rc_V.SetBottom(range.GetFrom());
        rc_V.SetTop   (range.GetTo());
        x_GetParent()->ZoomRect(rc_V);
    }
}


void CHitMatrixPane::SetSubjectVisibleRange(const TSeqRange& range)
{
    if (x_GetParent()) {
        const CGlPane& VP = x_GetParent()->GetPort();

        // prepare CGlPanes
        TModelRect rc_V = VP.GetVisibleRect();
        rc_V.SetLeft (range.GetFrom());
        rc_V.SetRight(range.GetTo());
        x_GetParent()->ZoomRect(rc_V);
    }
}


void CHitMatrixPane::x_Render(void)
{
    x_RenderContent();
}


void CHitMatrixPane::Update()
{
    _ASSERT(x_GetParent());

    IHitMatrixDataSource* ds = x_GetParent()->GetDS();
    m_Renderer.Update(ds, x_GetParent()->GetPort());
}

const CHitMatrixPane::TRangeColl&  CHitMatrixPane::GetRangeSelection(ESequence seq) const
{
    switch(seq) {
    case eSubject:
        return m_HorzSelHandler.GetSelection();
    case eQuery:
        return m_VertSelHandler.GetSelection();
    default:
        _ASSERT(false);
        return m_VertSelHandler.GetSelection();
    }
}


void CHitMatrixPane::SetRangeSelection(const TRangeColl& coll, ESequence seq)
{
    switch(seq) {
    case eSubject:
        m_HorzSelHandler.SetSelection(coll, true);
        break;
    case eQuery:
        m_VertSelHandler.SetSelection(coll, true);
        break;
    }
}


void CHitMatrixPane::ResetObjectSelection()
{
    //TODO
    m_Renderer.GetGraph().ResetGlyphSelection();
}


void   CHitMatrixPane::GetObjectSelection(TConstObjects& objs) const
{
    set<const CSeq_align*>    aligns; // selected aligns

    const TElemGlyphSet& glyphs = m_Renderer.GetGraph().GetSelectedGlyphs();

    // build a set of CSeq_aligns corresponding to selected glyphs
    ITERATE(CHitMatrixPane::TElemGlyphSet, it_G, glyphs)   {
        const IHitElement& elem = (*it_G)->GetHitElem();
        const IHit& hit = elem.GetHit();
        aligns.insert(hit.GetSeqAlign());

    }

    objs.reserve(aligns.size());
    ITERATE(set<const CSeq_align*>, it_align, aligns) {
        objs.push_back(CConstRef<CObject>(*it_align));
    }
}


void CHitMatrixPane::SetObjectSelection(const vector<const CSeq_align*> sel_aligns)
{
    m_Renderer.GetGraph().ResetGlyphSelection();

    typedef CHitMatrixGraph::THitGlyphVector TGlyphs;
    const TGlyphs& glyphs = m_Renderer.GetGraph().GetGlyphs();
    ITERATE(TGlyphs, it_G, glyphs)  {
        const CHitGlyph& glyph = **it_G;
        const CSeq_align* align = glyph.GetHit().GetSeqAlign();

        if(std::find(sel_aligns.begin(), sel_aligns.end(), align)
                     != sel_aligns.end())   {
            m_Renderer.GetGraph().SelectGlyph(glyph);
        }
    }
}
 
const TVPRect & CHitMatrixPane::GetViewportRect () const
{
    return x_GetParent()->GetPort().GetViewport();
}


void CHitMatrixPane::RenderVectorGraphics(int vp_width, int vp_height)
{
    if (!x_GetParent() || !x_GetParent()->GetDS()) 
        return;
    
    m_Renderer.Render(x_GetParent()->GetPort(), true);
    x_RenderSelHandler();
    x_RenderMouseZoomHandler(m_Renderer.GetMatrixPane());
}

void CHitMatrixPane::UpdateVectorLayout ()
{
}

void CHitMatrixPane::AddTitle(bool b) 
{
}

void CHitMatrixPane::x_RenderContent(void)
{
    if (!x_GetParent() || !x_GetParent()->GetDS()) 
        return;

    CIRef<IRender>  mgr = CGlResMgr::Instance().
        GetRenderer( CGlResMgr::Instance().GetApiLevel());
    if (mgr.IsNull()) {
        LOG_POST(Error << "IRender object not available.");
        return;
    }
    CGlResMgr::Instance().SetCurrentRenderer(mgr);

    m_Renderer.Render(x_GetParent()->GetPort());
    x_RenderSelHandler();
    x_RenderMouseZoomHandler(m_Renderer.GetMatrixPane());
}


int  CHitMatrixPane::x_GetAreaByVPPos(TVPUnit vp_x, TVPUnit vp_y)
{
    return m_Renderer.GetAreaByVPPos(vp_x, vp_y);
}


void CHitMatrixPane::x_RenderSelHandler()
{
    CGlPane pane;
    pane.EnableOffset(true);

    // horizontal selection
    TVPRect rc_vp = m_Renderer.GetRect(CHitMatrixRenderer::fSubjectRuler);
    TVPRect rc_gr = m_Renderer.GetRect(CHitMatrixRenderer::fSubjectGraphs);
    rc_vp.CombineWith(rc_gr);
    pane.SetViewport(rc_vp);
    CGlPane& subject_pane = m_Renderer.GetSubjectPane();
    pane.SetModelLimitsRect(subject_pane.GetModelLimitsRect());
    pane.SetVisibleRect(subject_pane.GetVisibleRect());

    if ( m_Renderer.GetMatrixPane().GetViewport().Width() < 10 ||
         m_Renderer.GetMatrixPane().GetViewport().Height() < 10)
            return;

    if(rc_vp.Width() > 0  &&  rc_vp.Height() > 0)   {
        x_RenderSelHandler(true, pane, CLinearSelHandler::eActiveState);
    }
    x_RenderSelHandler(true, m_Renderer.GetMatrixPane(), CLinearSelHandler::ePassiveState);

    // vertical selection
    rc_vp = m_Renderer.GetRect(CHitMatrixRenderer::fQueryRuler);
    rc_gr = m_Renderer.GetRect(CHitMatrixRenderer::fQueryGraphs);
    rc_vp.CombineWith(rc_gr);
    pane.SetViewport(rc_vp);
    CGlPane& query_pane = m_Renderer.GetQueryPane();
    pane.SetModelLimitsRect(query_pane.GetModelLimitsRect());
    pane.SetVisibleRect(query_pane.GetVisibleRect());

    if(rc_vp.Width() > 0  &&  rc_vp.Height() > 0)   {
        x_RenderSelHandler(false, pane, CLinearSelHandler::eActiveState);
    }
    x_RenderSelHandler(false, m_Renderer.GetMatrixPane(), CLinearSelHandler::ePassiveState);
}


void CHitMatrixPane::x_RenderMouseZoomHandler(CGlPane& pane)
{
    IRender& gl = GetGl();
    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_MouseZoomHandler.Render(pane);

    gl.Disable(GL_BLEND);
}


void CHitMatrixPane::x_RenderSelHandler(bool b_horz, CGlPane& pane, CLinearSelHandler::ERenderingOption option)
{
    IRender& gl = GetGl();
    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    CLinearSelHandler& handler = b_horz ? m_HorzSelHandler : m_VertSelHandler;
    handler.Render(pane, option);

    gl.Disable(GL_BLEND);
}

////////////////////////////////////////////////////////////////////////////////
/// IMouseZoomHandlerHost implementation

TModelUnit  CHitMatrixPane::MZHH_GetScale(EScaleType type)
{
    const CGlPane& VP = x_GetParent()->GetPort();

    switch(type)    {
    case eCurrent:   return VP.GetScaleX();
    case eMin: return VP.GetMinScaleX();
    case eMax: return VP.GetZoomAllScaleX();
    default: _ASSERT(false); return -1;
    }
}


void CHitMatrixPane::MZHH_SetScale(TModelUnit scale, const TModelPoint& point)
{
    x_GetParent()->SetScaleX(scale, point);
}


void CHitMatrixPane::MZHH_ZoomRect(const TModelRect& rc)
{
    x_GetParent()->ZoomRect(rc);

    // translate the notification to the standard message
    x_GetParent()->NotifyVisibleRangeChanged();
}


void CHitMatrixPane::MZHH_ZoomPoint(const TModelPoint& point, TModelUnit factor)
{
    x_GetParent()->ZoomPoint(point, factor);

    // translate the notification to the standard message
    x_GetParent()->NotifyVisibleRangeChanged();
}


void CHitMatrixPane::MZHH_EndOp()
{
    // translate the notification to the standard message
    x_GetParent()->NotifyVisibleRangeChanged();
}


void CHitMatrixPane::MZHH_Scroll(TModelUnit d_x, TModelUnit d_y)
{
    x_GetParent()->Scroll(d_x, d_y);

    // translate the notification to the standard message
    x_GetParent()->NotifyVisibleRangeChanged();
}


TVPUnit CHitMatrixPane::MZHH_GetVPPosByY(int y) const
{
    return GetClientSize().y - 1  - y;
}


void CHitMatrixPane::HMGH_OnChanged(void)
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged);
    Send(&evt, ePool_Parent);

    Refresh();
}


TVPUnit  CHitMatrixPane::HMGH_GetVPPosByY(int y) const
{
    return GetClientSize().y - 1  - y;
}


////////////////////////////////////////////////////////////////////////////////
/// ISelHandlerHost implementation
void CHitMatrixPane::SHH_OnChanged()
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged);
    Send(&evt, ePool_Parent);
    Refresh();
}


TModelUnit  CHitMatrixPane::SHH_GetModelByWindow(int z, EOrientation orient)
{
    switch(orient)  {
    case eHorz:
        return m_Renderer.GetMatrixPane().UnProjectX(z);

    case eVert:
        return m_Renderer.GetMatrixPane().UnProjectY(GetSize().y - z);

    default:
        _ASSERT(false);
         return -1;
    }
}


TVPUnit  CHitMatrixPane::SHH_GetWindowByModel(TModelUnit z, EOrientation orient)
{
    switch(orient)  {
    case eHorz:
        return m_Renderer.GetMatrixPane().ProjectX(z);

    case eVert:
        return GetSize().y - m_Renderer.GetMatrixPane().ProjectY(z);

    default:
        _ASSERT(false);
        return -1;
    }
}


///////////////////////////////////////////////////////////////////////////////
/// ITooltip Implementation
/// TC_NeedTooltip() and TC_GetTooltip() is everything needed to show toolitps
/// in "active" mode

bool CHitMatrixPane::TC_NeedTooltip(const wxPoint & pt)
{
    m_TooltipArea = x_GetAreaByWindowPos(pt);
    int vp_y = GetSize().y - pt.y;

    return m_Renderer.NeedTooltip(m_TooltipArea, pt.x, vp_y);
}


string  CHitMatrixPane::TC_GetTooltip(const wxRect & rect)
{
    return m_Renderer.GetTooltip(m_TooltipArea);
}


END_NCBI_SCOPE
