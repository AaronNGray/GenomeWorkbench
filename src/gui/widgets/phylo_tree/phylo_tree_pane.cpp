/*  $Id: phylo_tree_pane.cpp 43510 2019-07-19 18:26:44Z katargir $
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
 * Authors:  Vladimir Tereshkov
 *
 * File Description: 
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbitime.hpp>

#include <gui/widgets/phylo_tree/phylo_tree_pane.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_widget.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_pane.hpp>

#include <gui/widgets/gl/attrib_menu.hpp>   
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/registry.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/irender.hpp>
#include <corelib/ncbitime.hpp>

#include <wx/wxchar.h>
#include <wx/dcclient.h>

BEGIN_NCBI_SCOPE

#define AXIS_AREA_W 50
#define AXIS_AREA_H 30


BEGIN_EVENT_TABLE(CPhyloTreePane, CGlWidgetPane)
    EVT_SIZE(CPhyloTreePane::OnSize)
    EVT_LEFT_DOWN(CPhyloTreePane::OnLeftDown)
    EVT_KEY_DOWN(CPhyloTreePane::OnKeyEvent)
END_EVENT_TABLE()

CPhyloTreePane::CPhyloTreePane(CPhyloTreeWidget * parent)
:   CGlWidgetPane(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0),
    m_CurrRenderer(-1),
    m_BackColor(0.95f, 1.0f, 0.95f),
    m_pTextFont(NULL),
    m_pLblFont(NULL),
    m_VertSelHandler(eVert),
    m_BottomRuler(true),
    m_TopRuler(true),
    m_LeftRuler(false),
    m_RightRuler(false)
{
    m_Gen.SetIntegerMode(true, true);
    m_Gen.EnableOneBased(true, true);
    m_Grid.EnableIntegerCentering(true);

    m_pTextFont = new CGlTextureFont(CGlTextureFont::eFontFace_Helvetica, 12); 
    m_pLblFont = new CGlTextureFont(CGlTextureFont::eFontFace_Helvetica, 10); 

    m_TooltipManager.SetHost(static_cast<IStickyTooltipHandlerHost*>(this));
    x_RegisterHandler(dynamic_cast<IGlEventHandler*>(&m_TooltipManager),
        fMatrixArea, &m_MatrixPane);

    m_MouseZoomHandler.SetHost(static_cast<IMouseZoomHandlerHost*>(this));
    x_RegisterHandler(dynamic_cast<IGlEventHandler*>(&m_MouseZoomHandler),
        fMatrixArea, &m_MatrixPane);

    m_MinimapHandler.SetHost(static_cast<IMinimapHandlerHost*>(this));
    x_RegisterHandler(dynamic_cast<IGlEventHandler*>(&m_MinimapHandler),
        fMatrixArea, &m_MatrixPane);

    x_Layout();
    x_SetupAxes();

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();

    CRegistryReadView view =
            gui_reg.GetReadView("GBENCH.Mouse.ModePhylo");

    std::string pan_mode = view.GetString("Pan");
    if (pan_mode == "lmouse") {
        m_MouseZoomHandler.SetPanMode(CMouseZoomHandler::eLmouse);
    }
    else {
        m_MouseZoomHandler.SetPanMode(CMouseZoomHandler::ePkey);
    }
}

CPhyloTreeWidget* CPhyloTreePane::x_GetWidget()  const
{
    return static_cast<CPhyloTreeWidget*>(GetParent());
}

void    CPhyloTreePane::x_SetupAxes()
{
    m_TopRuler.SetHorizontal(true, CRuler::eTop);
    m_RightRuler.SetHorizontal(false, CRuler::eRight);
}


TVPPoint CPhyloTreePane::GetPortSize(void)
{
    return TVPPoint(m_rcMatrix.Width(), m_rcMatrix.Height());
}

void CPhyloTreePane::UpdatePane(const TModelRect& r)
{
    m_MatrixPane.SetModelLimitsRect(r);
    m_MatrixPane.SetVisibleRect(r);
    m_MatrixPane.AdjustToLimits();
}

void CPhyloTreePane::RemoveCurrentDataSource()
{
    // This is for the purpose of cancelling any background jobs
    // dependent on the data source.  We do not re-display based on this.
    if (m_CurrRenderer>=0) {
        if (m_CurrRenderer < (int)m_Renderers.size())
            GetCurrRenderer()->RemoveCurrentDataSource();
    }
}

void CPhyloTreePane::UpdateDataSource()
{
    CPhyloTreeWidget* phyWidget = x_GetWidget();
    for (auto &rr : m_Renderers)
        rr->UpdateDataSource(*phyWidget->GetDS(), m_MatrixPane);
}

void    CPhyloTreePane::OnSize(wxSizeEvent& event)
{
    // If the event was not forced by the windows dialog gl fix
    if (!m_PseudoSized) {
        x_Layout();

        event.Skip();        
    }
    else {
        m_PseudoSized = false;
    }
}
/*
// For debugging - redo layout after a key event.
void CPhyloTreePane::OnKeyEvent(wxKeyEvent& event)
{
#ifdef ATTRIB_MENU_SUPPORT
    if (event.GetEventType() == wxEVT_KEY_DOWN) {
        if (CAttribMenuInstance::GetInstance().KeyPress(event.GetKeyCode())) {
            x_Layout();
            Refresh();
            return;
        }
    }
#endif

    x_HandleAccels(event);

    //trace = true;
    if (!x_Handlers_handle(event, 0xFFFFFFFF, false))  {
        event.Skip();
    }
}
*/

void CPhyloTreePane::x_Render(void)
{
    if (m_MatrixPane.GetProjMode()!=CGlPane::eNone) {
        // recursive call - something wrong or renderer crashed
        //m_MatrixPane.Close();
        return;
    }

    CPhyloTreeWidget* phyWidget = x_GetWidget();
    // Beware of empty trees
    if (!phyWidget->GetDS() || 
        !phyWidget->GetDS()->GetTree() ||
        phyWidget->GetDS()->GetTree()->GetRootIdx() == CPhyloTree::Null())
        return;

    CStopWatch  render_total;
    render_total.Start();

    CRgbaColor bg(1.0f, 1.0f, 1.0f, 1.0f);
    if (phyWidget->HasScheme())
        bg = phyWidget->GetScheme().SetColor(
            CPhyloTreeScheme::eTree, CPhyloTreeScheme::eBgColor);
    glClearColor(bg.GetRed(), bg.GetGreen(), bg.GetBlue(), bg.GetAlpha());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Make sure layout has been called (special case for first-time render
    // of newly created view)
    if (m_CurrRenderer >= 0 &&
        m_CurrRenderer < (int)m_Renderers.size() &&
        phyWidget->GetDS()) {
        if (m_Renderers[m_CurrRenderer]->ValidLayout() ==
            IPhyloTreeRender::eNeedLayout)
            SoftUpdate();
        else if (m_Renderers[m_CurrRenderer]->ValidLayout() ==
            IPhyloTreeRender::eNeedLayoutAndSize)
            Update();
    }

    const CGlPane& VP = phyWidget->GetPort();

    // prepare CGlPanes

    // The limits rect is set in Layout() and OnSize().  But
    // we need updates to the visible rect since it captures
    // zoom.  Not sure why the design called for a CGlPane in
    // both this (CPhyloTreePane) and CPhyloTreeWidget, forcing
    // us to keep them in sync.....
    TModelRect rc_V = VP.GetVisibleRect();
    m_MatrixPane.SetVisibleRect(rc_V);
    m_MatrixPane.SetModelLimitsRect(VP.GetModelLimitsRect());

    // now rendering

    if (m_CurrRenderer >= 0 &&
        m_CurrRenderer < (int)m_Renderers.size() &&
        phyWidget->GetDS()) {

        /// Stop rendering if Render() returns false (can return false
        /// in force layout due to active data synchronization)              
        if (!m_Renderers[m_CurrRenderer]->Render(
            m_MatrixPane, *phyWidget->GetDS())) {
            return;
        }
    }

    x_RenderMouseZoomHandler(m_MatrixPane);
#ifdef ATTRIB_MENU_SUPPORT
    CAttribMenuInstance::GetInstance().DrawMenu();
#endif

    phyWidget->GetDS()->GetModel().m_TotalTime = render_total.Elapsed();
}


void    CPhyloTreePane::x_OnShowPopup()
{
    x_GetWidget()->OnShowPopup();
}


void    CPhyloTreePane::Update()
{
    CPhyloTreeWidget* phyWidget = x_GetWidget();
    _ASSERT(phyWidget);

    if (m_CurrRenderer>=0 && phyWidget && phyWidget->GetDS()) {
        x_SetupGLContext();

        // Make sure we have a valid tree first:
        if (phyWidget->GetDS()->GetTree() == NULL ||
            phyWidget->GetDS()->GetTree()->GetRootIdx() == CPhyloTree::Null())
            return;

        if (m_CurrRenderer < (int)m_Renderers.size())
            GetCurrRenderer()->Layout(*phyWidget->GetDS(), m_MatrixPane);
        m_NavHistory.Reset(m_MatrixPane);

        phyWidget->SetPortLimits(m_MatrixPane.GetModelLimitsRect(), true);
    }
}


void    CPhyloTreePane::SoftUpdate()
{
    CPhyloTreeWidget* phyWidget = x_GetWidget();
    _ASSERT(phyWidget);

    if (m_CurrRenderer>=0 && phyWidget && phyWidget->GetDS()) {
        if (m_CurrRenderer < (int)m_Renderers.size()) {
            GetCurrRenderer()->Layout(*phyWidget->GetDS(), m_MatrixPane);

            m_NavHistory.Reset(m_MatrixPane);
            phyWidget->SetPortLimits(m_MatrixPane.GetModelLimitsRect(), false);

            // Synchronize matrix pane with pane in widget (again not quite sure why
            // both have a pane) Then update collision grid since it depends on pane
            // zoom especially when zooming only vertically or horizontally.
            // InitCollisionInfo also called from BufferedRender but that call can give
            // incorrect results when visible rect is not yet set.
            m_MatrixPane.SetVisibleRect(phyWidget->GetPort().GetVisibleRect());
            m_MatrixPane.SetModelLimitsRect(phyWidget->GetPort().GetModelLimitsRect());
            GetCurrRenderer()->InitCollisionInfo();            
        }
    }
}

const CPhyloTreePane::TRangeColl & CPhyloTreePane::GetSubjectSelection() const
{
    return m_HorzSelHandler.GetSelection();
}


const CPhyloTreePane::TRangeColl & CPhyloTreePane::GetQuerySelection() const
{
    return m_VertSelHandler.GetSelection();
}


CGlPane CPhyloTreePane::TreeNavHistory::Current() const
{ 
    return Empty() ? CGlPane() : m_History[m_BackForwardPos]; 
}

bool CPhyloTreePane::TreeNavHistory::EqualPanes(const CGlPane& lhs, const CGlPane& rhs) const
{
    // compute roughly the pct scrolling that has taken place in order
    // to ignore very (imperceptibly) small changes
    TModelRect l = lhs.GetVisibleRect();
    TModelRect r = rhs.GetVisibleRect();

    float delta = fabs(l.Left() - r.Left()) +
                  fabs(l.Right() - r.Right()) +
                  fabs(l.Top() - r.Top()) +
                  fabs(l.Bottom() - r.Bottom());

    float m = l.Width() + l.Height() + r.Width() + r.Height();

    float delta_pct = delta/m;

    return delta_pct < 0.0001f;        
}

void CPhyloTreePane::TreeNavHistory::Reset(const CGlPane& p) 
{
    m_History.clear(); 
    m_History.push_back(p); 
    m_BackForwardPos = 0;
}

void CPhyloTreePane::TreeNavHistory::Add(const CGlPane& p) 
{
    if (m_BackForwardPos < m_History.size()) {
        m_History.erase(m_History.begin() + m_BackForwardPos +1,
                        m_History.end());
    }
  
    m_History.push_back(p);
    m_BackForwardPos = m_History.size() - 1;
}

void CPhyloTreePane::SaveCurrentView()
{
    CPhyloTreeWidget* phyWidget = x_GetWidget();
    if (m_NavHistory.Empty() || 
        !m_NavHistory.EqualPanes(phyWidget->GetPort(), m_NavHistory.Current())) {
            m_NavHistory.Add(phyWidget->GetPort());
    }
}

bool CPhyloTreePane::CanGoBack() const
{
    return m_NavHistory.CanGoBack();
}


bool CPhyloTreePane::CanGoForward() const
{
    return m_NavHistory.CanGoForward();
}


void CPhyloTreePane::GoBack(void)
{
    CPhyloTreeWidget* phyWidget = x_GetWidget();
    if (CanGoBack()) {
        CGlPane p = m_NavHistory.Back();
        //m_MatrixPane = p;
        m_MatrixPane.SetVisibleRect(p.GetVisibleRect());
        m_MatrixPane.SetModelLimitsRect(p.GetModelLimitsRect());
        phyWidget->GetPort().SetModelLimitsRect(m_MatrixPane.GetModelLimitsRect());
        phyWidget->GetPort().SetVisibleRect(m_MatrixPane.GetVisibleRect());
        phyWidget->UpdateViewingArea();
        phyWidget->Refresh();
    }
}


void CPhyloTreePane::GoForward(void)
{
    CPhyloTreeWidget* phyWidget = x_GetWidget();
    if (CanGoForward()) {
        CGlPane p = m_NavHistory.Forward();
        //m_MatrixPane = p;
        m_MatrixPane.SetVisibleRect(p.GetVisibleRect());
        m_MatrixPane.SetModelLimitsRect(p.GetModelLimitsRect());
        phyWidget->GetPort().SetVisibleRect(m_MatrixPane.GetVisibleRect());
        phyWidget->UpdateViewingArea();
        phyWidget->Refresh();
    }
}


void CPhyloTreePane::x_Layout(void)
{
    TModelRect vis = m_MatrixPane.GetVisibleRect();
    m_rcMatrix.Init(0, 0, GetClientSize().GetWidth() - 1, GetClientSize().GetHeight() - 1);    
    m_MatrixPane.SetViewport(m_rcMatrix);

    CPhyloTreeWidget* phyWidget = x_GetWidget();
    if (m_CurrRenderer >= 0 && phyWidget->GetDS() != NULL) {
        m_MatrixPane.SetVisibleRect(vis);

        if (m_CurrRenderer < (int)m_Renderers.size()) {
            GetCurrRenderer()->ComputeViewingLimits(m_MatrixPane, false, false);


            phyWidget->SetPortLimits(m_MatrixPane.GetModelLimitsRect(), false);

            // Synchronize matrix pane with pane in widget (again not quite sure why
            // both have a pane) Then update collision grid since it depends on pane
            // zoom especially when zooming only vertically or horizontally. 
            // InitCollisionInfo also called from BufferedRender but that call can give
            // incorrect results when visible rect is not yet set.
            m_MatrixPane.SetVisibleRect(phyWidget->GetPort().GetVisibleRect());
            m_MatrixPane.SetModelLimitsRect(phyWidget->GetPort().GetModelLimitsRect());
            GetCurrRenderer()->InitCollisionInfo();
        }
    }

    m_NavHistory.Reset(m_MatrixPane);
}

void CPhyloTreePane::SetupHardcopyRender()
{
    if (m_CurrRenderer >= 0  &&
        m_CurrRenderer < (int)m_Renderers.size() &&
        x_GetWidget()->GetDS()) {
            GetCurrRenderer()->ComputeViewingLimits(m_MatrixPane, false);
    }
}

void CPhyloTreePane::RenderHardcopy(void)
{
    CPhyloTreeWidget* phyWidget = x_GetWidget();
    if (m_CurrRenderer >= 0  &&
        m_CurrRenderer < (int)m_Renderers.size() &&
        phyWidget->GetDS()) {
            GetCurrRenderer()->RenderForHardcopy(
                m_MatrixPane, *phyWidget->GetDS());
    }
}

void CPhyloTreePane::RenderPdf(CRef<CPdf> pdf, bool render_tooltips)
{
    CPhyloTreeWidget* phyWidget = x_GetWidget();
    if (m_CurrRenderer >= 0  &&
        m_CurrRenderer < (int)m_Renderers.size() &&
        phyWidget->GetDS()) {
        phyWidget->GetDS()->GetTree()->GetGraphicsModel().SetRenderTooltips(render_tooltips);
            GetCurrRenderer()->RenderPdf(
                m_MatrixPane, *phyWidget->GetDS(), pdf);
    }
}

void CPhyloTreePane::BufferedRender(void)
{
    CPhyloTreeWidget* phyWidget = x_GetWidget();
    if (m_CurrRenderer >= 0  &&
        m_CurrRenderer < (int)m_Renderers.size() &&
        phyWidget->GetDS()){
            GetCurrRenderer()->BufferedRender(
                m_MatrixPane, *phyWidget->GetDS(), false);
    }
}

void    CPhyloTreePane::x_AdjustToMasterPane(CGlPane& pane,
                                             bool b_model_x,
                                             bool b_model_y)
{
    const CGlPane& VP = x_GetWidget()->GetPort();

    TModelRect rc_vis = VP.GetVisibleRect();
    TModelRect rc_lim = VP.GetModelLimitsRect();

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

int     CPhyloTreePane::x_GetAreaByVPPos(TVPUnit vp_x, TVPUnit vp_y)
{
    if(m_rcMatrix.PtInRect(vp_x, vp_y)) {
        return fMatrixArea;
    } else if(m_rcBottomSeq.PtInRect(vp_x, vp_y)) {
        return fSubjectArea;
    } else if(m_rcLeftSeq.PtInRect(vp_x, vp_y)) {
        return fQueryArea;
    }
    return fOther;
}

void    CPhyloTreePane::x_RenderMouseZoomHandler(CGlPane& pane)
{
    IRender& gl = GetGl();
    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_MouseZoomHandler.Render(pane);
    m_MinimapHandler.Render(pane);


    gl.Disable(GL_BLEND);
}

bool CPhyloTreePane::x_GrabFocus()
{
    // Grab the focus unless user is in a text control.  This is primarly here
    // for the case where the user is typing in the search control, and may use
    // the mouse to move the cursor around.  We do not want to lose focus for
    // the text control in this case.
    if ( CGlWidgetPane::x_GrabFocus() )
        return true;

    return false;

    // Maybe not - I think you only want to take focus from certain controls, 
    // such as maybe those (other than the text edit control) in the search bar.
    /*
    wxWindow* w = wxWindow::FindFocus();

    if (w != NULL) {
        //_TRACE("Focus Parent: " << w->GetClassInfo()->GetClassName()    << " " << w->GetName());
    
        if (wxString(w->GetClassInfo()->GetClassName()) == wxT("wxTextCtrl") ||
            wxString(w->GetClassInfo()->GetClassName()) == wxT("wxRichTextCtrl") ||
            wxString(w->GetClassInfo()->GetClassName()) == wxT("wxComboBox"))
            return false;
    }    

    return true;    
    */
}
////////////////////////////////////////////////////////////////////////////////
/// IAlnMarkHandlerHost implementation

TModelUnit  CPhyloTreePane::MZHH_GetScale(EScaleType type)
{
    CPhyloTreeWidget* phyWidget = x_GetWidget();
    const CGlPane& VP = phyWidget->GetPort();

    if (m_CurrRenderer < 0)
        return -1;

    switch(type)    {
    case IMouseZoomHandlerHost::eCurrent:   {
        return phyWidget->GetScheme().GetZoomBehavior(GetCurrRenderer()->GetDescription())==CPhyloTreeScheme::eZoomY ?
            VP.GetScaleY() : VP.GetScaleX();
    }
    case IMouseZoomHandlerHost::eMin: {
        return phyWidget->GetScheme().GetZoomBehavior(GetCurrRenderer()->GetDescription())==CPhyloTreeScheme::eZoomY?
            VP.GetMinScaleY() : VP.GetMinScaleX();
    }
    case IMouseZoomHandlerHost::eMax:  {
        return phyWidget->GetScheme().GetZoomBehavior(GetCurrRenderer()->GetDescription())==CPhyloTreeScheme::eZoomY?
            VP.GetZoomAllScaleY() : VP.GetZoomAllScaleX();
    }
    default: _ASSERT(false); return -1;
    }
}

void    CPhyloTreePane::MZHH_SetScale(TModelUnit scale,
                                      const TModelPoint& point)
{
    x_GetWidget()->OnSetScaleXY(scale, point);
}

void     CPhyloTreePane::MZHH_ZoomRect(const TModelRect& rc)
{
    x_GetWidget()->ZoomRect(rc);
}


void     CPhyloTreePane::MZHH_ZoomPoint(const TModelPoint& point,
                                        TModelUnit factor)
{
    x_GetWidget()->ZoomPoint(point, factor);
}


void     CPhyloTreePane::MZHH_EndOp()
{
    SaveCurrentView();
}


void     CPhyloTreePane::MZHH_Scroll(TModelUnit d_x, TModelUnit d_y)
{
    x_GetWidget()->Scroll(d_x, d_y);
}


TVPUnit  CPhyloTreePane::MZHH_GetVPPosByY(int y) const
{
    return GetClientSize().GetHeight() - 1  - y;
}





TVPUnit  CPhyloTreePane::HMGH_GetVPPosByY(int y) const
{
    return GetClientSize().GetHeight() - 1  - y;
}



void    CPhyloTreePane::HMGH_UpdateLimits(bool force_redraw)
{
    CPhyloTreeWidget* phyWidget = x_GetWidget();
    _ASSERT(phyWidget);

    if (m_CurrRenderer>=0 && phyWidget && phyWidget->GetDS()) {
        phyWidget->GetPort().SetModelLimitsRect(m_MatrixPane.GetModelLimitsRect());
        phyWidget->GetPort().SetVisibleRect(m_MatrixPane.GetVisibleRect());
        phyWidget->UpdateViewingArea();

        if ( force_redraw) {
            phyWidget->Refresh();

            // Mac tends not redraw the display as much as needed when a force-based
            // layout is in progress. Update helps.
#if defined(__WXMAC__) 
            phyWidget->Update();
#endif
        }

        m_NavHistory.Reset(m_MatrixPane);
    }
}



////////////////////////////////////////////////////////////////////////////////
/// ISelHandlerHost implementation


void CPhyloTreePane::SHH_OnChanged()
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged);
    Send(&evt, ePool_Parent);
    
    Refresh();
}

void CPhyloTreePane::SHH_SetCursor(const wxCursor& cursor)
{
    SetCursor(cursor);
}


TModelUnit  CPhyloTreePane::SHH_GetModelByWindow(int z, EOrientation orient)
{
    switch(orient)  {
    case eHorz: return m_MatrixPane.UnProjectX(z);
    case eVert: return m_MatrixPane.UnProjectY(GetClientSize().GetHeight() - z);
    default:    _ASSERT(false); return -1;
    }
}


TVPUnit     CPhyloTreePane::SHH_GetWindowByModel(TModelUnit z, EOrientation orient)
{
    switch(orient)  {
    case eHorz: return m_MatrixPane.ProjectX(z);
    case eVert: return GetClientSize().GetHeight() - m_MatrixPane.ProjectY(z);
    default:    _ASSERT(false); return -1;
    }
}


void CPhyloTreePane::AddRenderer(CRef<IPhyloTreeRender> &&renderer)
{
    renderer->SetZoomHandler(&m_MouseZoomHandler);
    renderer->SetFont(m_pLblFont);
    renderer->SetRegenerateTexture(true);

    m_Renderers.push_back(renderer);

    renderer->SetHost(static_cast<IPhyloTreeRenderHost*>(this));

    if (m_CurrRenderer<0) SetCurrRendererIdx(0);
}

void CPhyloTreePane::SetCurrRendererIdx(int idx)
{
    if (m_CurrRenderer != idx) {

        if (m_CurrRenderer >= 0) {
            if (m_CurrRenderer < (int)m_Renderers.size()) {
                x_UnregisterHandler(dynamic_cast<IGlEventHandler*>(GetCurrRenderer()));
                m_Renderers[m_CurrRenderer]->StartRendering(false);
            }  
        }
        
        m_CurrRenderer = idx;

        if (m_CurrRenderer < (int)m_Renderers.size()) {
            m_Renderers[m_CurrRenderer]->StartRendering(true);
            if (m_MouseZoomHandler.GetPanMode() == CMouseZoomHandler::eLmouse) {
                x_RegisterHandler(dynamic_cast<IGlEventHandler*>(GetCurrRenderer()),
                    fMatrixArea, &m_MatrixPane, 1);
            }
            else {
                x_RegisterHandler(dynamic_cast<IGlEventHandler*>(GetCurrRenderer()),
                    fMatrixArea, &m_MatrixPane);
            }
        }
    }
}

string  CPhyloTreePane::TTHH_NeedTooltip(const wxPoint & pt)
{
    // Don't pop up tool tips while zooming in and out(this can
    // be called if zoom key (z) is down but mouse isn't)
    if (m_MouseZoomHandler.IsScaleMode())
        return "";

    return GetCurrRenderer()->TTHH_NeedTooltip(pt);
}

CTooltipInfo  CPhyloTreePane::TTHH_GetTooltip(const wxRect & rect)
{
    return GetCurrRenderer()->TTHH_GetTooltip(rect);
}

void  CPhyloTreePane::FireCBEvent(void)
{
    x_GetWidget()->SendSelChangedEvent();
}

void  CPhyloTreePane::FireEditEvent(EPhyloTreeEditCommand ec)
{
    x_GetWidget()->SendEditEvent(ec);
}

TModelUnit  CPhyloTreePane::MMHH_GetScale(EMMScaleType type)
{
    const CGlPane& VP = x_GetWidget()->GetPort();

    switch(type)    {
    case IMinimapHandlerHost::eCurrent:   return VP.GetScaleX();
    case IMinimapHandlerHost::eMin: return VP.GetMinScaleX();
    case IMinimapHandlerHost::eMax: return VP.GetZoomAllScaleX();
    default: _ASSERT(false); return -1;
    }
}

void     CPhyloTreePane::MMHH_EndOp()
{
    SaveCurrentView();
}


void     CPhyloTreePane::MMHH_Scroll(TModelUnit d_x, TModelUnit d_y)
{
    x_GetWidget()->Scroll(d_x, d_y);
}


TVPUnit  CPhyloTreePane::MMHH_GetVPPosByY(int y) const
{
    return GetClientSize().GetHeight() - 1  - y;
}


I3DTexture* CPhyloTreePane::MMHH_GetTexture(float& xcoord_limit,
                                            float& ycoord_limit)
{
    return GetCurrRenderer()->GetTexture(xcoord_limit, ycoord_limit);
}


void CPhyloTreePane::HMGH_OnChanged(void)
{
    x_SetupGLContext();
    x_GetWidget()->SendSelChangedEvent();
}

void CPhyloTreePane::HMGH_OnRefresh(void)
{
    Refresh();
}

void CPhyloTreePane::OnLeftDown(wxMouseEvent& evt)
{   
    SetFocus();
    
    bool handled = false; 

    if(m_pCurrHandlerRec)  {               
        if (dynamic_cast<CMouseZoomHandler*>(m_pCurrHandlerRec->m_pHandler) != NULL &&
            m_MouseZoomHandler.IsScaleMode()) {
            handled = x_DispatchEventToHandler(evt, m_pCurrHandlerRec);
        }
    }    

    if (!handled) {
        wxPoint pos = evt.GetPosition();
        int area = x_GetAreaByWindowPos(pos);
        if( ! x_Handlers_handle(evt, area)) {
            evt.Skip();
        }
    }
}

END_NCBI_SCOPE
