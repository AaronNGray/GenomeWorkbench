/*  $Id: alnmulti_pane.cpp 45038 2020-05-13 01:22:22Z evgeniev $
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

#include <objmgr/scope.hpp>

#include <gui/widgets/aln_multiple/alnmulti_pane.hpp>
#include <gui/widgets/aln_multiple/alnmulti_model.hpp>
#include <gui/widgets/aln_multiple/alnmulti_widget.hpp>

#include <gui/widgets/seq_graphic/url_tooltip_handler.hpp>

#include <gui/widgets/wx/async_call.hpp>

#include <gui/opengl/glutils.hpp>
#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/irender.hpp>

#include <gui/objects/gbench_version.hpp>

#include <gui/utils/command.hpp>

#include <algorithm>
#include <sstream>

BEGIN_NCBI_SCOPE


BEGIN_EVENT_TABLE(CAlnMultiPane, CGlWidgetPane)
    EVT_SIZE(CAlnMultiPane::OnSize)

    EVT_LEFT_DOWN(CAlnMultiPane::OnLeftDown)
    EVT_LEFT_UP(CAlnMultiPane::OnLeftUp)
    EVT_MOTION(CAlnMultiPane::OnMotion)
    EVT_LEFT_DCLICK(CAlnMultiPane::OnLeftDoubleClick)
    EVT_MOUSEWHEEL(CAlnMultiPane::OnMouseWheel)

    EVT_KEY_DOWN(CAlnMultiPane::OnKeyDown)
END_EVENT_TABLE()



CAlnMultiPane::CAlnMultiPane(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                             const wxSize& size, long style)
: CGlWidgetPane(parent, id, pos, size, style),
  m_Parent(NULL),
  m_Context(NULL),
  m_Model(NULL),
  m_Renderer(TVPRect(0, 0, size.x, size.y))
{
    // TODO
    //m_Tooltip.EnableActiveMode(this);
    //m_Tooltip.SetMode(CTooltip::eHideOnMove);

    m_HandlerPane.EnableOffset();

    m_Renderer.SetBackColor(CRgbaColor(0.95f, 1.0f, 0.95f));

    //setup Event Handlers
    m_SelHandler.SetHost(this);    
    x_RegisterHandler(&m_SelHandler, fAlignArea|fRulerArea, &m_HandlerPane);

    m_MouseZoomHandler.SetMode(CMouseZoomHandler::eHorz);
    m_MouseZoomHandler.SetHost(this);
    m_MouseZoomHandler.SetPanOnMouseDown(true);
    x_RegisterHandler(&m_MouseZoomHandler, fAlignArea|fRulerArea, &m_HandlerPane);

    m_MarkHandler.SetHost(this);
    x_RegisterHandler(&m_MarkHandler, fAlignArea, &m_HandlerPane);

    m_HeaderHandler.SetHost(this);
    x_RegisterHandler(&m_HeaderHandler, fHeaderArea, &m_HandlerPane);

    //m_TooltipHandler.SetHost(this);
    //m_TooltipHandler.SetMode(CTooltipHandler::eHideOnMove);
    //x_RegisterHandler(&m_TooltipHandler, fAllAreas, &m_HandlerPane);

    m_TooltipManager.SetHost(static_cast<IStickyTooltipHandlerHost*>(this));
    x_RegisterHandler( dynamic_cast<IGlEventHandler*>(&m_TooltipManager), fAllAreas, &m_HandlerPane);


    m_ProgressPanel.SetVisible(true);
}


CAlnMultiPane::~CAlnMultiPane()
{
}


TVPPoint CAlnMultiPane::GetPortSize(void)
{
    return TVPPoint();//TODO
}

void CAlnMultiPane::Init(objects::CScope* scope)
{
    m_SelectionVisitor.SetScope(scope);
}


void CAlnMultiPane::SetWidget(IAlnMultiPaneParent* parent)
{
    if (m_Parent)   { // disconnect old parent
        RemoveListener(dynamic_cast<CEventHandler*>(m_Parent));
    }

    m_Parent = parent;

    if(m_Parent)   {
        AddListener(dynamic_cast<CEventHandler*>(m_Parent), ePool_Parent);
    }
}


void CAlnMultiPane::SetContext(IAlnMultiRendererContext* context)
{
    m_Context = context;
    m_Renderer.SetContext(m_Context);

    if(m_Context)   {
        m_HandlerPane = x_GetContext()->GetAlignPort();
    }
}


void CAlnMultiPane::SetBackColor(const CRgbaColor& color)
{
    m_Renderer.SetBackColor(color);
}

void CAlnMultiPane::SetMasterBackColor(const CRgbaColor& color)
{
    m_Renderer.SetMasterBackColor(color);
}

CRuler& CAlnMultiPane::GetRuler()
{
    return m_Renderer.GetRuler();
}


CAlnMultiRenderer&  CAlnMultiPane::GetRenderer()
{
    return m_Renderer;
}


void CAlnMultiPane::OnSize(wxSizeEvent& event)
{
    CGlWidgetPane::OnSize(event);

    wxSize sz = GetClientSize();
    m_Renderer.Resize(TVPRect(0, 0, sz.x - 1, sz.y - 1));

    if(x_GetParent())   {
        x_GetParent()->OnChildResize();
    }

    event.Skip();
}


void CAlnMultiPane::OnLeftDown(wxMouseEvent& event)
{
    event.Skip();
    // rows can handle event in Icons column
    wxPoint ms_pos = event.GetPosition();
    int area = x_GetAreaByWindowPos(ms_pos);
    if (area == fIconsArea || area == fAlignArea) {
        x_OnRowMouseEvent(event);
        if (!event.GetSkipped())
            return;
    }
    CGlWidgetPane::OnMouseDown(event);

    if(event.GetSkipped())  {
        event.Skip(false);
        if (x_IsMasterLineByWindowY(ms_pos.y)) {
            TSelListModel* model = SLC_GetModel();
            if (model) {
                CGUIEvent::EGUIState state = CGUIEvent::wxGetSelectState(event);

                switch(state)   {
                case CGUIEvent::eSelectState:
                    model->SLM_SelectSingleItem(CAlnMultiModel::kMasterRowIndex);
                    break;
                case CGUIEvent::eSelectIncState:
                    m_ProcessMouseUp = true;
                    break;
                default:
                    break;
                }

                //Refresh();
                //NotifyParent(eSelRowChanged);
            }
        }
        else
            x_OnLeftDown(event);
    }
}


void CAlnMultiPane::OnLeftUp(wxMouseEvent& event)
{
    event.Skip();
    wxPoint ms_pos = event.GetPosition();
    int area = x_GetAreaByWindowPos(ms_pos);
    if (area == fIconsArea || area == fAlignArea) {
        x_OnRowMouseEvent(event);
        if (!event.GetSkipped())
            return;
    }

    CGlWidgetPane::OnMouseUp(event);

    if(event.GetSkipped())  {
        event.Skip(false);

        // let CSelListController handle remaining events
        wxPoint ms_pos = event.GetPosition();
        if (x_IsMasterLineByWindowY(ms_pos.y)) {
            TSelListModel* model = SLC_GetModel();
            if(model && m_ProcessMouseUp) {
                CGUIEvent::EGUIState state = CGUIEvent::wxGetSelectState(event);
                if(state == CGUIEvent::eSelectIncState)
                    model->SLM_InvertSingleItem(CAlnMultiModel::kMasterRowIndex);
                else if(state == CGUIEvent::eSelectState)
                    model->SLM_SelectSingleItem(CAlnMultiModel::kMasterRowIndex);

                Refresh();
                NotifyParent(eSelRowChanged);
            }
        }
        else
            x_OnLeftUp(event);
    }
}


void CAlnMultiPane::OnMotion(wxMouseEvent& event)
{

    event.Skip();
    wxPoint ms_pos = event.GetPosition();
    int area = x_GetAreaByWindowPos(ms_pos);
    if (area == fIconsArea || area == fAlignArea) {
        x_OnRowMouseEvent(event);
        if (!event.GetSkipped())
            return;
    }

    CGlWidgetPane::OnMotion(event);
    if(event.GetSkipped())  {
        event.Skip(false);
        x_OnMotion(event);
    }
}

void CAlnMultiPane::OnLeftDoubleClick(wxMouseEvent& event)
{
    event.Skip();
    wxPoint ms_pos = event.GetPosition();
    int area = x_GetAreaByWindowPos(ms_pos);
    if (fAlignArea)
        x_OnRowMouseEvent(event);
}


void CAlnMultiPane::OnMouseWheel(wxMouseEvent& event)
{
    CGlWidgetPane::OnMouseWheel(event);

    if(event.GetSkipped())  {
        // let CSelListController handle remaining events
        event.Skip(false);
        int d_y = event.GetWheelRotation();
        if (SLC_GetModel()->SLM_GetItemsCount() > 0) {
            // GetLineHeight increases if the row is expanded which I don't
            // think we want, so just use a standard (pixel) height for an average row
            // times # of scroll clicks (each click is 120)
            //int h = MHH_GetLineHeight(0);
            //d_y = (d_y / h) * h;
            int h = 15;
            d_y = h*(d_y/120);
        }
        x_GetParent()->Scroll(0, (TModelUnit) -d_y);
    }
}


void CAlnMultiPane::OnKeyDown(wxKeyEvent& event)
{
    if (HasCapture())
        return;
    CGlWidgetPane::OnKeyEvent(event);

    if(event.GetSkipped())  {
        // let CSelListController handle remaining events
        event.Skip(false);
        x_OnKeyDown(event);
    }
}


void CAlnMultiPane::x_OnRowMouseEvent(wxMouseEvent& event)
{
    auto is_being_handled =
        (wxGetKeyState(wxKeyCode('Z')) || wxGetKeyState(wxKeyCode('R')) || wxGetKeyState(wxKeyCode('P')));
    is_being_handled = is_being_handled
        && m_pCurrHandlerRec
        && dynamic_cast<CMouseZoomHandler*>(m_pCurrHandlerRec->m_pHandler) != 0;
    if (is_being_handled)
        return;
    if (m_MouseZoomHandler.IsScaleMode()) {
        event.Skip();
        return;
    }

    wxPoint pos = event.GetPosition();
    IAlignRow* p_row = 0;
    int vp_top_y = 0;
    bool is_master = x_IsMasterLineByWindowY(pos.y);
    if (is_master) {
        p_row = x_GetContext()->GetMasterRow();
        vp_top_y = m_Renderer.GetMasterArea().Top();
    }
    else {
        int i_line = x_GetLineByWindowY(pos.y);
        if(i_line >= 0)  {
            p_row = x_GetContext()->GetRowByLine(i_line);
            int row_top = m_Renderer.GetVPListTop() - x_GetContext()->GetLinePosY(i_line);

            const CGlPane& VP = x_GetContext()->GetAlignPort();
            vp_top_y = row_top + (int) VP.GetVisibleRect().Top();
        }
        else {
            event.Skip();
            return;
        }
    }

    _ASSERT(p_row);
    // grab focus so that track settings changes will be saved on KillFocus event 
    if (event.GetEventType() == wxEVT_LEFT_DOWN)
        SetFocus();

    int i_col = m_Renderer.GetColumnIndexByX(pos.x);
    IAlignRow::TColumnType type = (IAlignRow::EColumnType)
        (int) m_Renderer.GetColumn(i_col).m_UserData;

    CGlPane pane(x_GetContext()->GetAlignPort()); // (CGlPane::eNeverUpdate);
    pane.EnableOffset();
    if (is_master) {
        pane.GetViewport().SetVert(m_Renderer.GetMasterArea().Bottom(), m_Renderer.GetMasterArea().Top());
    //    pane.SetViewport(m_Renderer.GetMasterArea());
    } 
//    pane.SetViewport(port.GetViewport());
//    pane.SetVisibleRect(port.GetVisibleRect());
//    pane.SetModelLimitsRect(port.GetModelLimitsRect());

    //m_Renderer.SetupPaneForRow(pane, p_row, vp_top_y);
    m_Renderer.SetupPaneForColumn(pane, i_col);

    p_row->OnMouseEvent(event, type, pane);
}


/// Functor to retrieve tooltip (used for async execution)
///
struct CGetTooltip_local
{
    string operator()(CAlnMultiRenderer& renderer)
    {
        return renderer.GetTooltip();
    }
};

static int TTHH_id = 0;

string CAlnMultiPane::TTHH_NeedTooltip(const wxPoint& point)
{
    TVPPoint pt = x_GetVPPosByWindowPos( point.x, point.y );

    return m_Renderer.NeedTooltip(pt.X(), pt.Y()) ? NStr::IntToString( ++TTHH_id ) : "";
}

CTooltipInfo CAlnMultiPane::TTHH_GetTooltip(const wxRect& rect)
{
    CTooltipInfo info( CTooltipInfo::ePin );

    info.SetTipID("");
    info.SetTipText("");
    info.SetTitleText("");

    try { 
        //CGetTooltip_local func;
        //tt = 
        //    GUI_AsyncExec1<string, CAlnMultiRenderer&,
        //    CGetTooltip_local>(func, m_Renderer);
        string tt = m_Renderer.GetTooltip();
        if (!tt.empty()) {
            info.SetTipText( tt );
            info.SetTipID( NStr::IntToString( TTHH_id ) );
        }
    } catch (CException& e) {
        // Failed to show the tooltip, log it
        ERR_POST("CAlnMultiPane::TTHH_GetTooltip() " << e.GetMsg());        
    }
    //return tt;


    return info;
}

bool CAlnMultiPane::TTHH_ProcessURL(const wxString & href)
{
    objects::CScope *scope = m_SelectionVisitor.GetScope();
    if (nullptr == scope)
        return false;
    CURLTooltipHandler url_handler(*scope, *this);
    return url_handler.ProcessURL(href.ToStdString());
}

const TVPRect& CAlnMultiPane::GetViewportRect() const
{
    CAlnMultiWidget *parent = dynamic_cast<CAlnMultiWidget*>(m_Parent);
    _ASSERT(parent);
    return parent->GetPort().GetViewport();
}


void CAlnMultiPane::RenderVectorGraphics(int vp_width, int vp_height)
{
    if (!x_GetParent())
        return;

    IRender& gl = GetGl();

    // main rendering...
    string error;
    try {
        m_Renderer.SetFocused(false);
        int icons_index = m_Renderer.GetColumnIndexByType(IAlignRow::eIcons);
        bool icons_visible = m_Renderer.IsColumnVisible(icons_index);
        if (icons_visible)
            m_Renderer.SetColumnVisible(icons_index, false);
        m_Renderer.Render();

        if (m_Title) {

            CAlnMultiWidget* widget = dynamic_cast<CAlnMultiWidget*>(m_Parent);
            _ASSERT(widget);
            CGlTextureFont font = widget->GetModel()->GetDisplayStyle()->m_TextFont;
            font.SetFontSize(12);
            float title_height = font.TextHeight() - font.GetFontDescender();
            float vert_offset = title_height + 12.f;

            std::stringstream strm;        
            strm << "NCBI Genome Workbench, Version ";
            strm << CGBenchVersion::eMajor << "." << CGBenchVersion::eMinor << "." << CGBenchVersion::ePatchLevel;
            string title =  strm.str();

            gl.Viewport(0, vp_height - vert_offset, vp_width, vert_offset);
            gl.MatrixMode(GL_PROJECTION);
            gl.LoadIdentity();
            gl.Ortho(0, vp_width, 0, vert_offset, -1.0, 1.0);
            gl.MatrixMode(GL_MODELVIEW);
            gl.LoadIdentity();
            gl.Color3f(0.f, 0.f, 0.f);
            gl.BeginText(&font);
            TModelUnit yoffset = -font.GetFontDescender();
            gl.WriteText(0.0f, yoffset, float(vp_width), float(vert_offset - yoffset), title.c_str(), IGlFont::eAlign_Left | IGlFont::eAlign_VCenter);
            gl.EndText();
        }

        if (icons_visible)
            m_Renderer.SetColumnVisible(icons_index, true);
    }
    catch (CException& e) {
        error = e.GetMsg();
    }
    catch (std::exception& e) {
        error = e.what();
    }
    if (!error.empty()) {
        ERR_POST("CAlnMultiPane::Render() " << error);
    }
}


void CAlnMultiPane::UpdateVectorLayout()
{
    // No specific layout
}

void CAlnMultiPane::x_Render()
{
    string error;
    try {
        m_Renderer.SetFocused(FindFocus() == this);
        m_Renderer.Render();

        x_RenderSelection();
        x_RenderMark();
        x_RenderZoomHandler();
        x_RenderHeaderHandler();
        x_RenderProgressPanel();

    } catch(CException& e)  {
        error = e.GetMsg();
    } catch(std::exception& e)  {
        error = e.what();
    } 
    if( ! error.empty())    {
        ERR_POST("CAlnMultiPane::Render() " << error);
    }
}


void CAlnMultiPane::Update()
{
    m_Renderer.Update();
    UpdateHeaderSortState();

    if(x_GetParent())
        x_GetParent()->OnChildResize();
}


void CAlnMultiPane::UpdateOnDataChanged()
{
    m_Renderer.UpdateOnDataChanged();
    UpdateHeaderSortState();

    if(x_GetParent())
        x_GetParent()->OnChildResize();
}


CRange<TSeqPos> CAlnMultiPane::GetSelectionLimits()
{
    return m_SelHandler.GetSelectionLimits();
}


const CAlnMultiPane::TRangeColl&   CAlnMultiPane::GetSelection() const
{
    return m_SelHandler.GetSelection();
}


void CAlnMultiPane::SetSelection(const TRangeColl& C, bool redraw)
{
    m_SelHandler.SetSelection(C, redraw);
}


void CAlnMultiPane::ResetSelection(bool redraw)
{
    m_SelHandler.ResetSelection(redraw);
}


void CAlnMultiPane::MarkSelectedRows(const TRangeColl& C, bool bMark)
{
    m_MarkHandler.MarkSelectedRows(C, bMark);
}


void CAlnMultiPane::UnMarkAll()
{
    m_MarkHandler.UnMarkAll();
}


const   CAlnMultiPane::TRowToMarkMap&  CAlnMultiPane::GetMarks() const
{
    return m_MarkHandler.GetMarks();
}


int  CAlnMultiPane::GetAlignVPHeight() const
{
    return GetListAreaHeight();
}


int  CAlnMultiPane::GetAlignVPWidth() const
{
    int iAlign = m_Renderer.GetColumnIndexByType(IAlignRow::eAlignment);
    return m_Renderer.GetColumn(iAlign).m_Width;
}


////////////////////////////////////////////////////////////////////////////////
/// protected members

void CAlnMultiPane::x_RenderSelection()
{
    IRender& gl = GetGl();

    if(x_GetParent()) {
        const CGlPane& VP = x_GetContext()->GetAlignPort();
        if(! VP.GetVisibleRect().IsEmpty())
        {
            TVPRect rc_vp = m_Renderer.GetColumnRectByType(IAlignRow::eAlignment, false);
            if(rc_vp.Width() > 0)   {
                m_HandlerPane.EnableOffset();
                m_HandlerPane.SetViewport(rc_vp);

                TModelRect rcM = VP.GetModelLimitsRect();
                rcM.SetVert(0, 1);
                m_HandlerPane.SetModelLimitsRect(rcM);

                TModelRect rcV = VP.GetVisibleRect();
                rcV.SetVert(0, 1);
                m_HandlerPane.SetVisibleRect(rcV);

                //CGlAttrGuard guard(GL_ENABLE_BIT);
                gl.Enable(GL_BLEND);
                gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                m_SelHandler.Render(m_HandlerPane);
            }
        }
    }
}


void CAlnMultiPane::x_RenderMark()
{
    if(x_GetParent()) {
        const CGlPane& VP = x_GetContext()->GetAlignPort();

        IRender& gl = GetGl();

        TVPRect rc_vp = m_Renderer.GetColumnRectByType(IAlignRow::eAlignment, false);
        if(rc_vp.Width() > 0)   {
            rc_vp.SetTop(m_Renderer.GetVPListTop());
            m_HandlerPane.SetViewport(rc_vp);

            TModelRect rcM = VP.GetModelLimitsRect();
            m_HandlerPane.SetModelLimitsRect(rcM);

            TModelRect rcV = VP.GetVisibleRect();
            rcV.SetBottom(rcV.Top() + rc_vp.Height());
            m_HandlerPane.SetVisibleRect(rcV);

            //CGlAttrGuard guard(GL_ENABLE_BIT);
            gl.Enable(GL_BLEND);
            gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            m_MarkHandler.Render(m_HandlerPane);

            gl.Disable(GL_BLEND);
        }
    }
}


void CAlnMultiPane::x_RenderZoomHandler()
{
    if(x_GetParent()) {
        TVPRect rc_vp = m_Renderer.GetColumnRectByType(IAlignRow::eAlignment, false);
        if(rc_vp.Width() > 0)   {
            IRender& gl = GetGl();
            m_HandlerPane.SetViewport(rc_vp);

            //CGlAttrGuard guard(GL_ENABLE_BIT);
            gl.Enable(GL_BLEND);
            gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            m_MouseZoomHandler.Render(m_HandlerPane);

            gl.Disable(GL_BLEND);
        }
    }
}


void CAlnMultiPane::x_RenderHeaderHandler()
{
    if(m_HeaderHandler.IsActive()) {
        CGlPane pane(CGlPane::eNeverUpdate);
        pane.SetViewport(m_Renderer.GetRect());

        m_HeaderHandler.Render(pane);
    }
}


void CAlnMultiPane::x_RenderProgressPanel()
{
    if(m_ProgressPanel.IsVisible()) {
        CGlPane pane(CGlPane::eNeverUpdate);

        static int offset = 4;

        TVPRect rc(m_Renderer.GetRect());
        rc.Inflate(-offset, -offset);

        TVPPoint min_sz = m_ProgressPanel.PreferredSize();

        rc.SetTop(rc.Bottom() + min_sz.Y() - 1);
        pane.SetViewport(rc);

        m_ProgressPanel.SetVPRect(rc);
        m_ProgressPanel.Render(pane);
    }
}


TVPPoint CAlnMultiPane::x_GetVPPosByWindowPos(int x, int y)
{
    int vp_x = x;
    int vp_y = GetClientSize().y - 1 - y;
    return TVPPoint(vp_x, vp_y);
}


int CAlnMultiPane::x_GetLineByWindowY(int WinY) const
{
    int vpY = WinY - m_Renderer.GetListTop();
    int offset_y = (int) x_GetContext()->GetAlignPort().GetVisibleRect().Top();
    return x_GetContext()->GetLineByModelY(vpY + offset_y);
}

bool CAlnMultiPane::x_IsMasterLineByWindowY(int WinY)
{
    if (x_GetContext()->GetMasterRow() == 0)
        return false;

    int vp_y = GetClientSize().y - 1 - WinY;
    TVPRect rc = m_Renderer.GetMasterArea();
    return vp_y >= rc.Bottom() && vp_y <= rc.Top();
}

TVPRect CAlnMultiPane::x_GetLineRect(int index) const
{
    int top = 0, H = 0;
    if(x_GetParent()  &&  index >= 0) {
        int offset_y = (int) x_GetContext()->GetAlignPort().GetVisibleRect().Top();
        top = x_GetContext()->GetLinePosY(index) - offset_y;
        H = x_GetContext()->GetLineHeight(index);
    }
    int w = GetClientSize().x;
    return TVPRect(0, top + H -1, w - 1, top);
}


////////////////////////////////////////////////////////////////////////////////
// ISelListView implementation
void  CAlnMultiPane::SLV_SetModel(TSelListModel* pModel)
{
    m_Model = pModel;
}


void  CAlnMultiPane::SLV_UpdateItems(const TIndexVector& v_indexes)
{
    bool redraw = false;

    wxSize sz = GetClientSize();
    TVPRect rcVisible(0, sz.y - 1, sz.x - 1, 0);

    for( size_t j = 0;  j < v_indexes.size() &&  ! redraw;  j++ ) {
        int index = v_indexes[j];
        TVPRect rcItem = x_GetLineRect(index);
        redraw = rcItem.Intersects(rcVisible);
    }
    if(redraw) {
        Refresh();
    }

    NotifyParent(eSelRowChanged);
}


void  CAlnMultiPane::SLV_UpdateRange(int iFirstItem, int iLastItem)
{
    if(iLastItem >= iFirstItem) {
        TVPRect rc_first = x_GetLineRect(iFirstItem);
        TVPRect rc_last = x_GetLineRect(iLastItem);
        rc_first.CombineWith(rc_last);
        wxSize sz = GetClientSize();
        TVPRect rc_visible(0, sz.y - 1, sz.x - 1, 0);

        if(rc_first.Intersects(rc_visible))   {
            Refresh();
        }
    } else {
        Refresh();
    }

    NotifyParent(eSelRowChanged);
}


void  CAlnMultiPane::NotifyParent(EEventType type)
{
    CAlnMultiPane::CEvent evt(type);
    Send(&evt, CEventHandler::eDispatch_Default,
                    CEventHandler::ePool_Parent);
}


////////////////////////////////////////////////////////////////////////////////
/// CSelListController extension
void CAlnMultiPane::SetPane(CGlPane* pane)
{
    //NOP
}


wxEvtHandler* CAlnMultiPane::GetEvtHandler()
{
    return this;
}


CAlnMultiPane::TSelListModel* CAlnMultiPane::SLC_GetModel()
{
    return x_GetModel();
}


int CAlnMultiPane::SLC_GetHeight()
{
    return GetClientSize().y;
}


int CAlnMultiPane::SLC_GetLineByWindowY(int WinY, bool b_clip)
{
    int h = m_Renderer.GetRulerAreaHeight() + m_Renderer.GetMasterAreaHeight();
    if(! b_clip  ||  WinY >= h) {
        return  x_GetLineByWindowY(WinY);
    } else return -1;
}


void CAlnMultiPane::SLC_VertScrollToMakeVisible(int index)
{
    if(x_GetParent() && index >= 0) {
        const TModelRect& r = x_GetContext()->GetAlignPort().GetVisibleRect();
        int top = x_GetContext()->GetLinePosY(index);
        if (top < r.Top()) {
            x_GetParent()->Scroll(0, top - r.Top());
        } else {
            int last_row_top = (int)r.Bottom() - x_GetContext()->GetLineHeight(index);
            if (top > last_row_top)
                x_GetParent()->Scroll(0, top - last_row_top);
        }
    }

}


////////////////////////////////////////////////////////////////////////////////
/// ISelHandlerHost implementation
void CAlnMultiPane::SHH_OnChanged()
{
    Refresh();

    //broadcast selection
    NotifyParent(eLinearSelChanged);
}


TModelUnit  CAlnMultiPane::SHH_GetModelByWindow(int z, EOrientation orient)
{
    // Update pane first
    const CGlPane& VP = x_GetContext()->GetAlignPort();
    if (!VP.GetVisibleRect().IsEmpty()) {
        TVPRect rc_vp = m_Renderer.GetColumnRectByType(IAlignRow::eAlignment, false);
        if (rc_vp.Width() > 0) {
            m_HandlerPane.EnableOffset();
            rc_vp.SetVert(0, m_Renderer.GetListAreaHeight());

            m_HandlerPane.SetViewport(rc_vp);
            TModelRect rcM = VP.GetModelLimitsRect();
            m_HandlerPane.SetModelLimitsRect(rcM);

            TModelRect rcV = VP.GetVisibleRect();
            rcV.SetBottom(rcV.Top() + rc_vp.Height());
            m_HandlerPane.SetVisibleRect(rcV);

            m_HandlerPane.OpenOrtho();
            m_HandlerPane.Close();
        }
    }

    switch(orient)  {
    case eHorz: 
        return m_HandlerPane.UnProjectX(z);
    case eVert:
        return m_HandlerPane.UnProjectY(GetClientSize().y - z);
    default:
        _ASSERT(false);
        return -1;
    }
}


TVPUnit CAlnMultiPane::SHH_GetWindowByModel(TModelUnit z, EOrientation orient)
{
    switch(orient)  {
    case eHorz:
        return m_HandlerPane.ProjectX(z);
    case eVert:
        return GetClientSize().y - m_HandlerPane.ProjectY(z);
    default:
        _ASSERT(false);
        return -1;
    }
}


////////////////////////////////////////////////////////////////////////////////
/// IAlnMarkHandlerHost implementation
const IAlnMultiDataSource*      CAlnMultiPane::MHH_GetAlnDS() const
{
    return x_GetParent()->GetDataSource();
}


const CAlnMultiPane::TSelListModel*    CAlnMultiPane::MHH_GetSelListModel() const
{
    return x_GetModel();
}


CAlnMultiPane::TNumrow    CAlnMultiPane::MHH_GetRowByLine(int Index) const
{
    return x_GetParent()->GetRowNumByLine(Index);
}


int CAlnMultiPane::MHH_GetLineByRowNum(TNumrow Row) const
{
    return x_GetParent()->GetLineByRowNum(Row);
}


int CAlnMultiPane::MHH_GetLineByWindowY(int Y) const
{
    return x_GetLineByWindowY(Y);
}


int CAlnMultiPane::MHH_GetLinePosY(int Index) const
{
    return x_GetContext()->GetLinePosY(Index);
}


int CAlnMultiPane::MHH_GetLineHeight(int Index) const
{
    return x_GetContext()->GetLineHeight(Index);
}


///### refactor ??
TModelUnit  CAlnMultiPane::MHH_GetSeqPosByX(int X) const
{
    return m_HandlerPane.UnProjectX(X);
}


TModelUnit  CAlnMultiPane::MZHH_GetScale(EScaleType type)
{
    const CGlPane& port = x_GetContext()->GetAlignPort();

    switch(type)    {
    case eCurrent:   return port.GetScaleX();
    case eMin: return port.GetMinScaleX();
    case eMax: return port.GetZoomAllScaleX();
    default: _ASSERT(false); return -1;
    }
}


void CAlnMultiPane::MZHH_SetScale(TModelUnit scale, const TModelPoint& point)
{
    x_GetParent()->SetScaleX(scale, point);
}


void     CAlnMultiPane::MZHH_ZoomRect(const TModelRect& rc)
{
    x_GetParent()->ZoomRect(rc);
}


void     CAlnMultiPane::MZHH_ZoomPoint(const TModelPoint& point, TModelUnit factor)
{
    x_GetParent()->ZoomPoint(point, factor);
}


void CAlnMultiPane::MZHH_Scroll(TModelUnit d_x, TModelUnit d_y)
{
    x_GetParent()->Scroll(d_x, d_y);
}


void CAlnMultiPane::MZHH_EndOp()
{
    Refresh();
}


TVPUnit  CAlnMultiPane::MZHH_GetVPPosByY(int y) const
{
    return GetClientSize().y - 1  - y;
}


void CAlnMultiPane::ARH_OnRowChanged(IAlignRow* p_row)
{
    x_GetParent()->OnRowChanged(p_row);
}


TVPPoint CAlnMultiPane::ARH_GetVPPosByWindowPos(const wxPoint& pos)
{
    return x_GetVPPosByWindowPos(pos.x, pos.y);
}

void  CAlnMultiPane::ARH_OnSelectionChanged()
{
    NotifyParent(eSelRowChanged);
}

CSelectionVisitor* CAlnMultiPane::ARH_GetSelectionVisitor()
{
    return &m_SelectionVisitor;
}

int CAlnMultiPane::ARH_GetRowTop(IAlignRow* p_row) 
{
    if (x_GetContext()->GetMasterRow() == p_row) {
        const TVPRect& rc = m_Renderer.GetMasterArea();
        return rc.Top();
    }
    int index = x_GetContext()->GetLineByRowNum(p_row->GetRowNum());
    if (index < 0)
        return index;
    int y = m_Renderer.GetVPListTop() + (int)x_GetContext()->GetAlignPort().GetVisibleRect().Top();// -1;
    return y - x_GetContext()->GetLinePosY(index);
}


///////////////////////////////////////////////////////////////////////////////
///  IAMHeaderHandlerHost implementation
IAlnMultiHeaderContext* CAlnMultiPane::HHH_GetContext()
{
    return &m_Renderer;
}


TVPRect CAlnMultiPane::HHH_GetHeaderRect()
{
    return m_Renderer.GetHeader().GetVPRect();
}


TVPPoint CAlnMultiPane::HHH_GetVPPosByWindowPos(const wxPoint& pos)
{
   return x_GetVPPosByWindowPos(pos.x, pos.y);
}


void CAlnMultiPane::HHH_SetColumns(const TColumns& columns, int resizable_index)
{
    m_Renderer.SetColumns(columns, resizable_index);
    Update();
    Refresh();

    x_GetParent()->OnColumnsChanged();
}


void CAlnMultiPane::HHH_SortByColumn(int index)
{
    SortByColumn(index);
}

TVPPoint CAlnMultiPane::GCH_GetVPPosByWindowPos(const wxPoint& pos)
{
    return x_GetVPPosByWindowPos(pos.x, pos.y);
}

void CAlnMultiPane::SortByColumn(int index)
{
    TColumnType type = m_Renderer.GetColumnTypeByIndex(index);
    TColumn& column = m_Renderer.GetColumn(index);

    bool ok = false;

    switch(column.m_SortState)   {
    case IAlnExplorer::eUnSorted:
        ok = x_GetContext()->SortByColumn(type, true);
        break;
    case IAlnExplorer::eAscending:
        ok = x_GetContext()->SortByColumn(type, false);
        break;
    case IAlnExplorer::eDescending:
        ok = x_GetContext()->SortByColumn(IAlignRow::eInvalid, true);
        break;
    default:
        break;
    }

    if(ok)  {       // update column state
        UpdateHeaderSortState();
    }
}


void CAlnMultiPane::UpdateHeaderSortState()
{
    int n = m_Renderer.GetColumnsCount();
    for( int j = 0;  j < n;  j++ )  {
        TColumn& col = m_Renderer.GetColumn(j);
        IAlnExplorer::ESortState state = x_GetContext()->GetColumnSortState(col.m_UserData);
        col.m_SortState = state;
    }
}


void CAlnMultiPane::HHH_RenderColumnHeader(int index, const TVPRect& rc)
{
    m_Renderer.GetHeader().RenderColumnHeader(index, rc);
}


int CAlnMultiPane::x_GetAreaByWindowPos(const wxPoint& pos)
{
    int vp_y = GetClientSize().y - pos.y;
    return x_GetAreaByVPPos(pos.x, vp_y);
}


int CAlnMultiPane::x_GetAreaByVPPos(int vp_x, int vp_y)
{
    int list_h = m_Renderer.GetListAreaHeight();
    int ruler_h = m_Renderer.GetRulerAreaHeight();
    int master_h = m_Renderer.GetMasterAreaHeight();

    if(vp_y < list_h + master_h + ruler_h)  {
        TColumnType type = m_Renderer.GetColumnTypeByX(vp_x);

        switch(type)  {
        case IAlignRow::eDescr:
            return fDescrArea;
        case IAlignRow::eIcons:
        case IAlignRow::eIconStrand:
        case IAlignRow::eIconExpand:
            return fIconsArea;
        case IAlignRow::eAlignment:
            if (vp_y >list_h + master_h)
                return fRulerArea;
            else 
                return fAlignArea;
        default:
            return fOtherAreas;
        }
    } else if(vp_y < list_h + master_h + ruler_h)  {
        return fOtherAreas;
    } else {
        return fHeaderArea;
    }
}


void CAlnMultiPane::ShowProgressPanel(float progress, const string& msg)
{
    m_ProgressPanel.SetVisible(true);
    m_ProgressPanel.Update(progress, msg);
    Refresh();
}


void CAlnMultiPane::HideProgressPanel()
{
    m_ProgressPanel.SetVisible(false);
    Refresh();
}

bool CAlnMultiPane::HasSelectedObjects() const
{
    return m_SelectionVisitor.HasSelectedObjects();
}

void CAlnMultiPane::GetObjectSelection(TConstObjects &objs) const
{
    m_SelectionVisitor.GetObjectSelection(objs);
}


END_NCBI_SCOPE
