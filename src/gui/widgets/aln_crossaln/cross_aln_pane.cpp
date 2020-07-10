/*  $Id: cross_aln_pane.cpp 42772 2019-04-11 14:14:30Z katargir $
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
 * Authors:  Vlad Lebedev
 *
 */


#include <ncbi_pch.hpp>
#include "cross_aln_pane.hpp"

#include <gui/opengl/irender.hpp>

#include <algorithm>
#include <math.h>


BEGIN_NCBI_SCOPE


const TModelUnit kMouseZoomFactor = 25.0;   // per pixel of mouse movement

BEGIN_EVENT_TABLE(CCrossAlnPane, CGlWidgetPane)
    EVT_LEFT_DOWN(CCrossAlnPane::OnLeftDown)
    EVT_LEFT_DCLICK(CCrossAlnPane::OnLeftDblClick)
END_EVENT_TABLE()

CCrossAlnPane::CCrossAlnPane(CCrossAlnWidget* parent)
    :CGlWidgetPane(parent, wxID_ANY)
    ,m_DS(NULL)
    ,m_QueryHalf(false)
{
    SetClearColor(0.95f, 1.0f, 0.95f, 0.95f);

    m_Renderer.Reset(new CCrossAlnRenderer());

    // setup Event Handlers
    m_SelHandler_Q.SetHost(this);
    x_RegisterHandler(&m_SelHandler_Q, fRuler1Area, &m_Renderer->GetRP_Q() );

    m_SelHandler_S.SetHost(this);
    x_RegisterHandler(&m_SelHandler_S, fRuler2Area, &m_Renderer->GetRP_S() );

    m_MouseZoomHandler.SetHost(this);
    x_RegisterHandler(&m_MouseZoomHandler, fCrossAlign, &m_Renderer->GetCP());

    m_MouseZoomHandler.SetMode(CMouseZoomHandler::eHorz);

    m_TooltipHandler.SetMode(CTooltipHandler::eHideOnMove);
    m_TooltipHandler.SetHost(this);
    x_RegisterHandler(&m_TooltipHandler, fAllAreas, &m_Renderer->GetCP());

    Layout();
}


CCrossAlnPane::~CCrossAlnPane()
{
}


// CGlWidgetPane requirements
TVPPoint CCrossAlnPane::GetPortSize(void)
{
    //cout << "CCrossAlnPane::GetPortSize" << endl;
    return TVPPoint(1000,1000);
}


void CCrossAlnPane::SetDataSource(ICrossAlnDataSource* ds)
{
    CIRef<ICrossAlnDataSource> guard = m_DS; // keep it alive until update is completed
    m_DS = ds;

    int w, h;
    this->GetClientSize(&w, &h);
    TVPRect rcVP(0, 0, w - 1, h - 1);


    m_SelHandler_Q.ResetSelection(false);
    m_SelHandler_S.ResetSelection(false);

    ResetObjectSelection();

    m_Renderer->SetDataSource(rcVP, ds);
    Update();
}



ICrossAlnDataSource* CCrossAlnPane::GetDataSource()
{
    return m_DS;
}


void CCrossAlnPane::ColorBySegments()
{
    m_Renderer->ColorBySegments();
}


void CCrossAlnPane::ColorByScore(CConstRef<CObject_id> score_id)
{
    m_Renderer->ColorByScore(score_id);
}


CConstRef<CObject_id> CCrossAlnPane::GetScoreId() const
{
    return m_Renderer->GetScoreId();
}


void CCrossAlnPane::ResetObjectSelection()
{
    m_Renderer->ResetObjectSelection();
}


void CCrossAlnPane::GetObjectSelection(TConstObjects& objs) const
{
    m_Renderer->GetObjectSelection(objs);
}


void CCrossAlnPane::SetObjectSelection(const vector<const CSeq_align*> sel_aligns)
{
    m_Renderer->SetObjectSelection(sel_aligns);
}



const CCrossAlnPane::TRangeColl& CCrossAlnPane::GetQueryRangeSelection() const
{
    return m_SelHandler_Q.GetSelection();
}


const CCrossAlnPane::TRangeColl& CCrossAlnPane::GetSubjectRangeSelection() const
{
    return m_SelHandler_S.GetSelection();
}


void CCrossAlnPane::SetQueryRangeSelection(const TRangeColl& coll)
{
    m_SelHandler_Q.SetSelection(coll, true);
}


void CCrossAlnPane::SetSubjectRangeSelection(const TRangeColl& coll)
{
    m_SelHandler_S.SetSelection(coll, true);
}


void CCrossAlnPane::Update()
{
    m_Renderer->Update();
    Layout();
}

static const int kScrollRange = 1000000;
double CCrossAlnPane::NormalizeQ(int pos)
{
    const TModelRect& rcMQ = m_Renderer->GetRP_Q().GetModelLimitsRect();
    const TModelRect& rcVQ = m_Renderer->GetRP_Q().GetVisibleRect();

    CNormalizer norm(rcMQ.Left(), rcMQ.Right(), kScrollRange);
    return norm.IntToReal(pos) - rcVQ.Left();
}

double CCrossAlnPane::NormalizeS(int pos)
{
    const TModelRect& rcMS = m_Renderer->GetRP_S().GetModelLimitsRect();
    const TModelRect& rcVS = m_Renderer->GetRP_S().GetVisibleRect();

    CNormalizer norm(rcMS.Left(), rcMS.Right(), kScrollRange);
    return norm.IntToReal(pos) - rcVS.Left();;
}

// virtual from CGlWidgetPane
int CCrossAlnPane::x_GetAreaByVPPos(int vp_x, int vp_y)
{
    if(m_Renderer->GetRP_Q().GetViewport().PtInRect(vp_x, vp_y)) {
        return fRuler1Area;
    } else if(m_Renderer->GetRP_S().GetViewport().PtInRect(vp_x, vp_y)) {
        return fRuler2Area;
    } else {
        return fCrossAlign;
    }
    return fOther;
}



bool CCrossAlnPane::x_IsQueryHalf(wxPoint point)
{
    TVPRect rc = m_Renderer->GetCP().GetViewport();
    TVPUnit hg = rc.Height() / 2;

    return (point.y < hg);
}


void CCrossAlnPane::x_Render()
{
    if (!m_DS) {
        return;
    }

    m_Renderer->Render();

    IRender& gl = GetGl();

    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ZoomHandler indicator
    m_MouseZoomHandler.Render(m_Renderer->GetRP_Q());
    m_MouseZoomHandler.Render(m_Renderer->GetCP());
    m_MouseZoomHandler.Render(m_Renderer->GetRP_S());

    // Draw linear selection
    m_SelHandler_Q.Render(m_Renderer->GetRP_Q());
    m_SelHandler_S.Render(m_Renderer->GetRP_S());

    gl.Disable(GL_BLEND);
}


void CCrossAlnPane::OnLeftDblClick(wxMouseEvent& event)
{
    CheckOverlayTimer();

    wxPoint pos = event.GetPosition();
    TModelRect rc = m_Renderer->HitTest(pos.x, pos.y);
    SquareOnRect(rc);
}


void CCrossAlnPane::OnLeftDown(wxMouseEvent& event)
{
    CheckOverlayTimer();

    wxPoint pos = event.GetPosition();
    if (x_GetAreaByVPPos(pos.x, pos.y) != fCrossAlign) {
        event.Skip();
        return;
    }

    if (wxGetKeyState(wxKeyCode('Z')) ||
            wxGetKeyState(wxKeyCode('R')) ||
            wxGetKeyState(wxKeyCode('P')) )
    {
        event.Skip();
        return;
    }


    CGUIEvent::EGUIState state = CGUIEvent::wxGetSelectState(event);
    int x = pos.x;
    int y = GetClientSize().y - pos.y;

    TRangeColl coll_q = m_SelHandler_Q.GetSelection();
    TRangeColl coll_s = m_SelHandler_S.GetSelection();

    // Distinguish selection and incremental selection
    if (state == CGUIEvent::eSelectState) {
        ResetObjectSelection();
        coll_q.clear();
        coll_s.clear();
    }


    TModelRect rc = m_Renderer->SelectHitTest(x, y);

    coll_q.CombineWith(TSeqRange(TSeqPos(rc.Left()),
                       TSeqPos(rc.Right()-1)));
    coll_s.CombineWith(TSeqRange(TSeqPos(rc.Bottom()),
                                 TSeqPos(rc.Top()-1)));

    m_SelHandler_Q.SetSelection(coll_q, true);
    m_SelHandler_S.SetSelection(coll_s, true);

    // broadcast selection change
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged);
    Send(&evt, ePool_Parent);

    event.Skip();
}


//void CCrossAlnPane::x_SetPortLimits()
//{
    //cout << "CCrossAlnPane::x_SetPortLimits" << endl;
//}


// IMouseZoomHandlerHost implementation
TModelUnit CCrossAlnPane::MZHH_GetScale(EScaleType type)
{
    wxPoint mouse_pos = ScreenToClient(wxGetMousePosition());
    m_QueryHalf = x_IsQueryHalf(mouse_pos);

    CGlPane& tmp = m_QueryHalf ? m_Renderer->GetRP_Q() :
                                 m_Renderer->GetRP_S();

    switch(type) {
        case eCurrent: return tmp.GetScaleX();
        case eMin:     return tmp.GetMinScaleX();
        case eMax:     return tmp.GetZoomAllScaleX();
        default: _ASSERT(false); return -1;
    }
}


void CCrossAlnPane::MZHH_SetScale(TModelUnit scale, const TModelPoint& point)
{
    bool lock = wxGetKeyState(WXK_SHIFT);
    TVPPoint t_point = m_Renderer->GetCP().Project(point.X(), point.Y());

    TModelUnit scale_yQ = m_Renderer->GetRP_Q().GetScaleY();
    TModelUnit scale_yS = m_Renderer->GetRP_S().GetScaleY();

    TModelPoint pQ = m_Renderer->GetRP_Q().UnProject(t_point.X(), t_point.Y());
    TModelPoint pS = m_Renderer->GetRP_S().UnProject(t_point.X(), t_point.Y());

    if (!lock) {
        TModelUnit sc_x1 = m_Renderer->GetRP_Q().GetScaleX();
        TModelUnit sc_x2 = m_Renderer->GetRP_S().GetScaleX();

        TModelUnit scale1, scale2;
        TModelPoint point1, point2;
        if (m_QueryHalf) {
            scale1 = scale;
            scale2 = scale1 * sc_x2 / sc_x1;
        } else {
            scale2 = scale;
            scale1 = scale2 * sc_x1 / sc_x2;
        }

        m_Renderer->SetScaleRef_Q(scale1, scale_yQ, pQ);
        m_Renderer->SetScaleRef_S(scale2, scale_yS, pS);

    } else {
        if (m_QueryHalf) {
            m_Renderer->SetScaleRef_Q(scale, scale_yQ, pQ);
        } else {
            m_Renderer->SetScaleRef_S(scale, scale_yS, pS);
        }
    }

    x_UpdateOnZoom();
}


void CCrossAlnPane::MZHH_ZoomRect(const TModelRect& rect)
{
    TVPUnit leftCP  = m_Renderer->GetCP().ProjectX(rect.Left());
    TVPUnit rightCP = m_Renderer->GetCP().ProjectX(rect.Right());

    TModelUnit l1, l2, r1, r2;

    l1 = m_Renderer->GetRP_Q().UnProjectX(leftCP);
    r1 = m_Renderer->GetRP_Q().UnProjectX(rightCP);

    l2 = m_Renderer->GetRP_S().UnProjectX(leftCP);
    r2 = m_Renderer->GetRP_S().UnProjectX(rightCP);


    ZoomOnRange_Q(l1, r1);
    ZoomOnRange_S(l2, r2);

    x_UpdateOnZoom();
}


void CCrossAlnPane::MZHH_ZoomPoint(const TModelPoint& point, TModelUnit factor)
{
    ERR_POST("CCrossAlnPane::MZHH_ZoomPoint() not implemented!");
    x_UpdateOnZoom();
}

void CCrossAlnPane::MZHH_Scroll(TModelUnit d_x, TModelUnit d_y)
{
    bool lock = wxGetKeyState(WXK_SHIFT);

    wxPoint mouse_pos = ScreenToClient(wxGetMousePosition());

    TVPUnit dx  = m_Renderer->GetCP().ProjectX(d_x);
    TModelUnit dxQ = m_Renderer->GetRP_Q().UnProjectWidth(dx);
    TModelUnit dxS = m_Renderer->GetRP_S().UnProjectWidth(dx);

    if (!lock) {
        Scroll(dxQ, dxS);
    } else {

        if (x_IsQueryHalf(mouse_pos)) {
            Scroll(dxQ, 0);
        } else {
            Scroll(0, dxS);
        }
    }

    x_UpdateOnZoom();
}


TVPUnit CCrossAlnPane::MZHH_GetVPPosByY(int y) const
{
    return GetClientSize().y - 1  - y;
}


void CCrossAlnPane::MZHH_EndOp()
{
    x_UpdateOnZoom();
}
// IMouseZoomHandlerHost end



// ISelHandlerHost
TModelUnit CCrossAlnPane::SHH_GetModelByWindow(int z, EOrientation orient)
{
    wxPoint mouse_pos = ScreenToClient(wxGetMousePosition());
    m_QueryHalf = x_IsQueryHalf(mouse_pos);

    CGlPane& tmp = m_QueryHalf ? m_Renderer->GetRP_Q() :
                                 m_Renderer->GetRP_S();
    switch (orient) {
        case eHorz: return tmp.UnProjectX(z);
        case eVert: return tmp.UnProjectY(GetClientSize().y - z);
        default:    _ASSERT(false); return -1;
    }
}


TVPUnit CCrossAlnPane::SHH_GetWindowByModel(TModelUnit z, EOrientation orient)
{
    wxPoint mouse_pos = ScreenToClient(wxGetMousePosition());
    m_QueryHalf = x_IsQueryHalf(mouse_pos);

    CGlPane& tmp = m_QueryHalf ? m_Renderer->GetRP_Q() :
                                 m_Renderer->GetRP_S();

    switch (orient) {
        case eHorz: return tmp.ProjectX(z);
        case eVert: return GetClientSize().y - tmp.ProjectY(z); // h
        default:    _ASSERT(false); return -1;
    }
}


void CCrossAlnPane::SHH_OnChanged()
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged);
    Send(&evt, ePool_Parent);
    Refresh();
    if (!GetQueryRangeSelection().size()  || !GetSubjectRangeSelection().size())
    {
        ResetObjectSelection();
    }
}
// ISelHandlerHost end


///////////////////////////////////////////////////////////////////////////////
/// ITooltip Implementation
bool CCrossAlnPane::TC_NeedTooltip(const wxPoint& point)
{

    m_Renderer->GetTooltip(point.x, GetClientSize().y - point.y, &m_TooltipText);
    return m_TooltipText.length() > 0;
}


string CCrossAlnPane::TC_GetTooltip(const wxRect &)
{
    return m_TooltipText;
}

// ITooltip end

void CCrossAlnPane::x_UpdateOnZoom()
{
    x_GetParent()->UpdateScrollbars();
    x_GetParent()->NotifyVisibleRangeChanged();
    Refresh();
}


// --- Scrolling --------------------------------------------------
void CCrossAlnPane::Scroll(TModelUnit dxQ, TModelUnit dxS)
{
    m_Renderer->Scroll(dxQ, dxS);
}


void CCrossAlnPane::SquareOnRect(TModelRect rc)
{
    if (rc.Width() == 0 || rc.Height() == 0) return;

    m_Renderer->SquareOnRect(rc);
    x_UpdateOnZoom();
}


void CCrossAlnPane::ZoomOnRect(TModelRect rc)
{
    if (rc.Width() == 0 || rc.Height() == 0) return;

    ZoomOnRange_Q( rc.Left(), rc.Right() );
    ZoomOnRange_S( rc.Bottom(), rc.Top() );

    x_UpdateOnZoom();
}


void CCrossAlnPane::ZoomOnRange_Q(TModelUnit from, TModelUnit to)
{
    m_Renderer->ZoomOnRange_Q(from, to);
    x_UpdateOnZoom();
}

void CCrossAlnPane::ZoomOnRange_S(TModelUnit from, TModelUnit to)
{
    m_Renderer->ZoomOnRange_S(from, to);
    x_UpdateOnZoom();
}


void CCrossAlnPane::ZoomIn_Q()
{
    m_Renderer->ZoomIn_Q();
    x_UpdateOnZoom();
}


void CCrossAlnPane::ZoomOut_Q()
{
    m_Renderer->ZoomOut_Q();
    x_UpdateOnZoom();
}

void CCrossAlnPane::ZoomAll(void)
{
    ZoomAll_Q();
    ZoomAll_S();
}


void CCrossAlnPane::ZoomIn(void)
{
    ZoomIn_Q();
    ZoomIn_S();
}

void CCrossAlnPane::ZoomOut(void)
{
    ZoomOut_Q();
    ZoomOut_S();
}


void CCrossAlnPane::ZoomAll_Q(void)
{
    m_Renderer->ZoomAll_Q();
    x_UpdateOnZoom();
}

void CCrossAlnPane::ZoomToSeq_Q(void)
{
    m_Renderer->ZoomToSeq_Q();
    x_UpdateOnZoom();
}


// Subject
void CCrossAlnPane::ZoomIn_S()
{
    m_Renderer->ZoomIn_S();
    x_UpdateOnZoom();
}


void CCrossAlnPane::ZoomOut_S()
{
    m_Renderer->ZoomOut_S();
    x_UpdateOnZoom();
}


void CCrossAlnPane::ZoomAll_S(void)
{
    m_Renderer->ZoomAll_S();
    x_UpdateOnZoom();
}

void CCrossAlnPane::ZoomToSeq_S(void)
{
    m_Renderer->ZoomToSeq_S();
    x_UpdateOnZoom();
}



END_NCBI_SCOPE
