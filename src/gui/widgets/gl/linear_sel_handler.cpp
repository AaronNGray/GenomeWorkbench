/*  $Id: linear_sel_handler.cpp 42279 2019-01-24 19:10:40Z katargir $
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

#include <gui/widgets/gl/linear_sel_handler.hpp>
#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/irender.hpp>

#include <math.h>

#include <wx/window.h>


BEGIN_NCBI_SCOPE

static int kDragThreshold = 4; // make this configurable

////////////////////////////////////////////////////////////////////////////////
/// class CLinearSelHandler
BEGIN_EVENT_TABLE(CLinearSelHandler, wxEvtHandler)
    EVT_LEFT_DOWN(CLinearSelHandler::OnLeftDown)
    EVT_LEFT_DCLICK(CLinearSelHandler::OnLeftDoubleClick)
    EVT_LEFT_UP(CLinearSelHandler::OnLeftUp)
    EVT_MOTION(CLinearSelHandler::OnMotion)
    EVT_MOUSE_CAPTURE_LOST(CLinearSelHandler::OnMouseCaptureLost)
END_EVENT_TABLE()


CLinearSelHandler::CLinearSelHandler(EOrientation orient)
:   m_Orientation(orient),
    m_ExtState(eNoExt),
    m_OpType(eNoOp),
    m_bResizeCursor(false),
    m_ActiveSel(false),
    m_ShowCoordinates(true),
    m_Host(NULL),
    m_SelColor(0.5f, 0.5f, 0.5f, 0.25f),
    m_BorderColor(0.25f, 0.25f, 0.25f, 0.5f),
    m_PassiveSelColor(0.6f, 0.6f, 0.6f, 0.25f),
    m_SymbolColor(1.0f, 0.0f, 0.0f, 1.0f),
    m_TexFont(CGlTextureFont::eFontFace_Helvetica, 12)
{
}


CLinearSelHandler::~CLinearSelHandler()
{
}


void CLinearSelHandler::SetPane(CGlPane* pane)
{
    m_Pane = pane;
}


wxEvtHandler* CLinearSelHandler::GetEvtHandler()
{
    return this;
}


void CLinearSelHandler::SetHost(ISelHandlerHost* host)
{
    m_Host = host;
}


IGenericHandlerHost* CLinearSelHandler::GetGenericHost()
{
    return dynamic_cast<IGenericHandlerHost*>(m_Host);
}


void CLinearSelHandler::SetOrientation(EOrientation orient)
{
    m_Orientation = orient;
}


TSeqRange CLinearSelHandler::GetSelectionLimits()const
{
    return m_Selection.GetLimits();
}


const CLinearSelHandler::TRangeColl& CLinearSelHandler::GetSelection() const
{
    return m_Selection;
}


void CLinearSelHandler::SetSelection(const TRangeColl& C, bool redraw)
{
    m_Selection = C;
    if(redraw)    {
        m_Host->SHH_OnChanged();
    }
}


void CLinearSelHandler::ResetSelection(bool redraw)
{
    m_Selection.clear();
    if(redraw) {
        m_Host->SHH_OnChanged();
    }
}


void CLinearSelHandler::SetColor(EColorType type, const CRgbaColor& color)
{
    switch(type)    {
    case eSelection:
        m_SelColor = color;
        break;
    case ePasssiveSelection:
        m_PassiveSelColor = color;
        break;
    case eSymbol:
        m_SymbolColor = color;
        break;
    default:
        _ASSERT(false);
    }
}


void CLinearSelHandler::SetShowRangeCoords(bool flag)
{
    m_ShowCoordinates = flag;
}


////////////////////////////////////////////////////////////////////////////////
// event handlers
void CLinearSelHandler::OnLeftDown(wxMouseEvent& event)
{
    wxPoint ms_pos = event.GetPosition();

    // If modifier keys are used, allow this event to pass to other handlers (and ignore the following
    // mouse-up as well).  Make exception for control keys that may be enabled for linear selection.
    m_ActiveSel = true;

    int  k;
    for ( k='A'; k<='Z'; ++k) {
        if (wxGetKeyState((wxKeyCode)k))
            m_ActiveSel = false;
    }

    if (wxGetKeyState(WXK_ALT)) {
        m_ActiveSel = false;
    }

    if (!m_ActiveSel) {
        event.Skip();
        return;
    }


    CGUIEvent::EGUIState state = CGUIEvent::wxGetSelectState(event);

    x_OnStartSel(ms_pos, state);
    x_OnSelectCursor(ms_pos);
}


void CLinearSelHandler::OnLeftDoubleClick(wxMouseEvent& event)
{
    x_OnResetAll();
}


void CLinearSelHandler::OnLeftUp(wxMouseEvent& event)
{
    if (!m_ActiveSel) {
        event.Skip();
        return;
    }

    // Should we only set the operation type once?
    // Probablly, yes.  Now we only set m_OpType when selection
    // operation starts, and it won't get change before the selection
    // is done.
    //CGUIEvent::EGUIState state = CGUIEvent::wxGetSelectState(event);
    //m_OpType = x_GetOpTypeByState(state);
    wxPoint ms_pos = event.GetPosition();

    x_OnChangeSelRange(ms_pos);
    x_OnEndSelRange();
    x_OnSelectCursor(ms_pos);
}


void CLinearSelHandler::OnMotion(wxMouseEvent& event)
{
    bool handled = false;
    wxPoint ms_pos = event.GetPosition();

    if(event.Dragging())    {
        if(m_OpType != eNoOp)  {
            x_OnChangeSelRange(ms_pos);
            handled = true;
        }
    } else {
        x_OnSelectCursor(ms_pos);
    }
    if( ! handled)  {
        event.Skip();
    }
}


////////////////////////////////////////////////////////////////////////////////
/// Signal handlers
void CLinearSelHandler::x_OnStartSel(const wxPoint& ms_pos, CGUIEvent::EGUIState state)
{
    TSeqRange HitR;
    bool b_hit_start = false;
    x_HitTest(ms_pos, HitR, b_hit_start);

    if(HitR.NotEmpty()) {
        // hit a border of the selected range - resize
        m_OpType = eChange;
        m_ExtState = b_hit_start ? eExtRangeStart : eExtRangeEnd;
        m_CurrRange = HitR;

        x_RemoveFromSelection(HitR);
    } else {
        m_OpType = x_GetOpTypeByState(state);
        if(m_OpType == eReplace) {
            m_Selection.clear();
            m_ExtState = eNoExt;
        } else {
            m_ExtState = eExtRangeEnd;
        }

        TModelUnit m_Z = x_MouseToSeqPos(ms_pos);
        TSeqPos pos = (TSeqPos) floor(m_Z);

        m_CurrRange.SetFrom(pos);
        m_CurrRange.SetToOpen(pos);
    }

    GetGenericHost()->GHH_Redraw();
    GetGenericHost()->GHH_CaptureMouse();
}


void CLinearSelHandler::x_OnChangeSelRange(const wxPoint& ms_pos)
{
    TModelUnit m_Z = x_MouseToSeqPos(ms_pos);
    TModelUnit z = floor(m_Z);
    TSeqPos pos = (TSeqPos) x_ClipPosByRange(z);
    TSeqRange old_r = m_CurrRange;

    if(m_ExtState == eNoExt  &&  pos != m_CurrRange.GetFrom())   {
        if(pos > m_CurrRange.GetFrom()) {
            m_CurrRange.SetToOpen(pos);
            m_ExtState = eExtRangeEnd;
        } else  {
            m_CurrRange.SetToOpen(m_CurrRange.GetFrom());
            m_CurrRange.SetFrom(pos);
            m_ExtState = eExtRangeStart;
        }
    } else if(m_ExtState == eExtRangeEnd &&  pos != m_CurrRange.GetToOpen()) {
        if(pos > m_CurrRange.GetFrom())  {
            m_CurrRange.SetToOpen(pos);
        } else { //flip
            m_CurrRange.SetToOpen(m_CurrRange.GetFrom());
            m_CurrRange.SetFrom(pos);
            m_ExtState = eExtRangeStart;
        }
    } else if(m_ExtState == eExtRangeStart  &&  pos != m_CurrRange.GetFrom())    {
        if(pos <= m_CurrRange.GetToOpen())  {
            m_CurrRange.SetFrom(pos);
        } else {
            m_CurrRange.SetFrom(m_CurrRange.GetToOpen());
            m_CurrRange.SetToOpen(pos);
            m_ExtState = eExtRangeEnd;
        }
    }
    if(m_CurrRange != old_r) {
        GetGenericHost()->GHH_Redraw();
    }
}


void CLinearSelHandler::x_OnEndSelRange()
{
    if(! m_CurrRange.Empty())   {
        switch(m_OpType)   {
        case eAdd:
        case eReplace:
        case eChange:
            x_AddToSelection(m_CurrRange);
            break;
        case eRemove:
            x_RemoveFromSelection(m_CurrRange);
            break;
        case eNoOp: break;
        }
    }
    m_CurrRange.SetLength(0);

    m_ExtState = eNoExt;
    m_OpType = eNoOp;

    m_Host->SHH_OnChanged();
    GetGenericHost()->GHH_ReleaseMouse();
}


void CLinearSelHandler::x_OnResetAll()
{
    bool b_update = ! m_Selection.empty();
    m_Selection.clear();

    m_ExtState = eNoExt;
    m_OpType = eNoOp;

    if(b_update) {
        m_Host->SHH_OnChanged();
    }
}


void CLinearSelHandler::x_OnSelectCursor(const wxPoint& ms_pos)
{
    switch(m_OpType)    {
    case eNoOp:
        {{
             TSeqRange HitR;
             bool b_hit_start = false;
             x_HitTest(ms_pos, HitR, b_hit_start);

             m_bResizeCursor = HitR.NotEmpty();
         }}
        break;
    case eAdd:
    case eRemove:
    case eChange:
        m_bResizeCursor = true;
        break;

    case eReplace:
        break;

    default:
        _ASSERT(false);
        break;
    }
    x_SetCursor();
}


void CLinearSelHandler::x_SetCursor()
{
    wxStockCursor cur_id = wxCURSOR_ARROW;
    if(m_bResizeCursor) {
        cur_id = (m_Orientation == eHorz) ? wxCURSOR_SIZEWE : wxCURSOR_SIZENS;
    }
    wxCursor cursor(cur_id);
    GetGenericHost()->GHH_SetCursor(cursor);
}


////////////////////////////////////////////////////////////////////////////////
/// helper functions

// translate modificators to operation type
CLinearSelHandler::EOpType CLinearSelHandler::x_GetOpTypeByState(CGUIEvent::EGUIState state ) const
{
    switch(state)    {
    case CGUIEvent::eSelectState:
        return eReplace;
    case CGUIEvent::eSelectIncState:
        return eAdd;
    case CGUIEvent::eSelectExtState:
        return eRemove;
    default:
        return eNoOp;
    }
}


TModelUnit CLinearSelHandler::x_MouseToSeqPos(const wxPoint& pos)
{
    _ASSERT(m_Pane);
    int z = (m_Orientation == eHorz) ? pos.x : pos.y;
    TModelUnit r = m_Host->SHH_GetModelByWindow(z, m_Orientation) + 0.5;
    return r;
}


void CLinearSelHandler::x_HitTest(const wxPoint& pos, TSeqRange& range, bool& b_hit_start)
{
    _ASSERT(m_Pane);
    int z = (m_Orientation == eHorz) ? pos.x : pos.y;

    int min_D = -1;
    bool b_min_start = false;
    const TSeqRange* p_min_range = 0;

    const TRangeColl& C = m_Selection;
    ITERATE(TRangeColl, it, C)  {
        const TSeqRange& R = *it;
        int from_Z = m_Host->SHH_GetWindowByModel(R.GetFrom(), m_Orientation);
        int to_Z = m_Host->SHH_GetWindowByModel(R.GetToOpen(), m_Orientation);
        int D = abs(z - from_Z);

        if(min_D < 0 || min_D > D)    {
            min_D = D;
            b_min_start = true;
            p_min_range = &R;
        }
        D = abs(z - to_Z);
        if(min_D > D)    {
            min_D = D;
            b_min_start = false;
            p_min_range = &R;
        }
    }

    if(min_D > -1  && min_D <= kDragThreshold) {
        b_hit_start = b_min_start;
        _ASSERT(p_min_range);
        range  = *p_min_range;
    } else range.SetLength(0);
}


TModelUnit CLinearSelHandler::x_ClipPosByRange(TModelUnit pos)
{
    TModelRect rc_lim = m_Pane->GetModelLimitsRect();
    TModelUnit max_pos, min_pos;
    if(m_Orientation == eHorz) {
        max_pos = max(rc_lim.Right(), rc_lim.Left());
        min_pos = min(rc_lim.Right(), rc_lim.Left());
    } else {
        max_pos = max(rc_lim.Top(), rc_lim.Bottom());
        min_pos = min(rc_lim.Top(), rc_lim.Bottom());
    }

    pos = max(pos, min_pos);
    pos = min(pos, max_pos);
    return pos;
}


void    CLinearSelHandler::x_AddToSelection(const TSeqRange& range)
{
    m_Selection.CombineWith(range);
}


void    CLinearSelHandler::x_RemoveFromSelection(const TSeqRange& range)
{
    m_Selection.Subtract(range);
}


inline void RenderRect(const TModelRect& rc, TModelUnit offset_x, TModelUnit offset_y,
                       CRgbaColor& color, GLenum type)
{
    IRender& gl = GetGl();

    gl.ColorC(color);
    gl.LineWidth(1.0f);

    if (type == GL_LINE) {
        gl.Begin(GL_LINE_STRIP);
            gl.Vertex2d(rc.Left() - offset_x,  rc.Top() - offset_y);
            gl.Vertex2d(rc.Right() - offset_x, rc.Top() - offset_y);
            gl.Vertex2d(rc.Right() - offset_x, rc.Bottom() - offset_y);
            gl.Vertex2d(rc.Left() - offset_x,  rc.Bottom() - offset_y);
            gl.Vertex2d(rc.Left() - offset_x,  rc.Top() - offset_y);
        gl.End();
    }
    else {
        gl.PolygonMode(GL_FRONT_AND_BACK, type);
        gl.Rectd(rc.Left() - offset_x,  rc.Top() - offset_y,
                 rc.Right() - offset_x, rc.Bottom() - offset_y);
    }
}


static void s_RenderSymbol(TModelUnit x, TModelUnit y,
                           TModelUnit w, TModelUnit line_w,
                           TModelUnit scale_x, TModelUnit scale_y,
                           CLinearSelHandler::EOpType type, bool b_horz)
{
    IRender& gl = GetGl();

    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    gl.Enable(GL_LINE_SMOOTH);
                    
    TModelUnit f = floor((line_w - 1.0) * 0.5);
    x -= scale_x * f;
    y -= scale_y * f;

    TModelUnit half_w = (ceil)(w * 0.5);

    TModelUnit lw_x = line_w * scale_x;
    TModelUnit lw_y = line_w * scale_y;

    TModelUnit half_w_x = half_w * scale_x;
    TModelUnit half_w_y = half_w * scale_y;

    switch(type) {
    case CLinearSelHandler::eAdd:
        gl.Begin(GL_QUADS);
            gl.Vertex2d(x - half_w_x, y);
            gl.Vertex2d(x - half_w_x, y + lw_y);
            gl.Vertex2d(x + lw_x + half_w_x, y + lw_y);
            gl.Vertex2d(x + lw_x + half_w_x, y);

            gl.Vertex2d(x, y - half_w_y);
            gl.Vertex2d(x, y);
            gl.Vertex2d(x + lw_x, y);
            gl.Vertex2d(x + lw_x, y - half_w_y);

            gl.Vertex2d(x, y + lw_y);
            gl.Vertex2d(x, y + lw_y + half_w_y);
            gl.Vertex2d(x + lw_x, y + lw_y + half_w_y);
            gl.Vertex2d(x + lw_x, y + lw_y);

        gl.End();
        break;
    case CLinearSelHandler::eRemove:
        gl.Begin(GL_QUADS);
        if (b_horz) {
            gl.Vertex2d(x - half_w_x, y);
            gl.Vertex2d(x - half_w_x, y + lw_y);
            gl.Vertex2d(x + lw_x + half_w_x, y + lw_y);
            gl.Vertex2d(x + lw_x + half_w_x, y);
        } else {
            gl.Vertex2d(x, y - half_w_y);
            gl.Vertex2d(x + lw_x, y - half_w_y);
            gl.Vertex2d(x + lw_x, y + lw_y + half_w_y);
            gl.Vertex2d(x, y + lw_y + half_w_y);
        }
        gl.End();
        break;
    default:
        break;
    }
}

static const int kSymbolBorder = 6;

void CLinearSelHandler::Render(CGlPane& pane, ERenderingOption option)
{
    IRender& gl = GetGl();

    CGlAttrGuard AttrGuard(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_HINT_BIT);

    TModelRect rc_vis = pane.GetVisibleRect();

    if(! rc_vis.IsEmpty()) {
        CGlPaneGuard GUARD(pane, CGlPane::eOrtho);

        TModelUnit offset_x = pane.GetOffsetX();
        TModelUnit offset_y = pane.GetOffsetY();

        CRgbaColor& fill_color = (option == eActiveState) ? m_SelColor : m_PassiveSelColor;
        bool b_horz = (m_Orientation == eHorz);

        // draw exisiting selection
        TModelRect rc(rc_vis);

        const TRangeColl& C = m_Selection;
        ITERATE(TRangeColl, it, C)  { // for every segment
            const TSeqRange& r = *it;
            switch(m_Orientation)   {
            case eHorz:
                rc.SetHorz(r.GetFrom(), r.GetToOpen());
                break;
            case eVert:
                rc.SetVert(r.GetFrom(), r.GetToOpen());
                break;
            }
            RenderRect(rc, offset_x, offset_y, fill_color, GL_FILL);
            RenderRect(rc, offset_x, offset_y, m_BorderColor, GL_LINE);
        }

        if(m_OpType != eNoOp)   { // draw current range
            TModelRect rc_curr(rc_vis);

            if(b_horz)  {
                TModelUnit min_pos = min(rc_vis.Left(), rc_vis.Right());
                TModelUnit max_pos = max(rc_vis.Left(), rc_vis.Right());
                rc_curr.SetLeft(max(min_pos, (TModelUnit) m_CurrRange.GetFrom()));
                rc_curr.SetRight(min(max_pos, (TModelUnit) m_CurrRange.GetToOpen()) );
            } else {
                TModelUnit min_pos = min(rc_vis.Bottom(), rc_vis.Top());
                TModelUnit max_pos = max(rc_vis.Bottom(), rc_vis.Top());
                rc_curr.SetBottom( max(min_pos, (TModelUnit) m_CurrRange.GetFrom()) );
                rc_curr.SetTop( min(max_pos, (TModelUnit) m_CurrRange.GetToOpen()) );
            }

            RenderRect(rc_curr, offset_x, offset_y, fill_color, GL_FILL);

            CRgbaColor& border_c = (option == eActiveState) ? m_SymbolColor : m_BorderColor;
            RenderRect(rc_curr, offset_x, offset_y, border_c, GL_LINE);

            if(option == eActiveState)  {
                // drwaing Operation symbol
                TModelUnit symbol_w = 0.0;
                switch(m_OpType) {
                case eAdd:
                    symbol_w = 12.0;
                    break;
                case eRemove:
                    symbol_w = 12.0;
                    break;
                default:
                    break;
                }

                CRgbaColor bk_color(1.0f, 1.0f, 1.0f, 0.8f);
                TModelUnit scale_y = pane.GetScaleY();
                TModelUnit scale_x = pane.GetScaleX();

                if (symbol_w > 0  &&
                    ((b_horz  &&  fabs((symbol_w + kSymbolBorder) * scale_x) < fabs(rc_curr.Width()))  ||
                    (!b_horz  &&  fabs((symbol_w + kSymbolBorder) * scale_y) < fabs(rc_curr.Height()))) ) {

                        TModelPoint pos_world((rc_curr.Left() + rc_curr.Right()) / 2 - offset_x,
                            (rc_curr.Bottom() + rc_curr.Top()) / 2 - offset_y);
                        TVPPoint pos_screen = pane.Project(pos_world.X(), pos_world.Y());
                        pos_world = pane.UnProject(pos_screen.X(), pos_screen.Y());

                        gl.ColorC(bk_color);
                        s_RenderSymbol(pos_world.X(), pos_world.Y(),
                            symbol_w, 4.0, scale_x, scale_y, m_OpType, b_horz);
                        gl.ColorC(border_c);
                        s_RenderSymbol(pos_world.X(), pos_world.Y(),
                            symbol_w, 2.0, scale_x, scale_y, m_OpType, b_horz);
                } else {
                    // No operation symbol
                    symbol_w = 0.0;
                }
                symbol_w += kSymbolBorder;

                // render range coordinates
                if (m_ExtState != eNoExt  &&  m_ShowCoordinates) {
                    TSeqPos pos_s = m_CurrRange.GetFrom() + 1;
                    TSeqPos pos_e = m_CurrRange.GetToOpen();
                    string pos_str_s = NStr::NumericToString(pos_s, NStr::fWithCommas);
                    string pos_str_e = NStr::NumericToString(pos_e, NStr::fWithCommas);
                    TModelUnit x_s = 0.0, y_s = 0.0, x_e = 0.0, y_e = 0.0;
                    {
                        TModelUnit text_pos_s = 0.0;
                        TModelUnit text_pos_e = 0.0;
                        TModelUnit scale = b_horz ? scale_x : scale_y;
                        TModelUnit text_w_s = m_TexFont.TextWidth(pos_str_s.c_str()) * scale;
                        TModelUnit text_w_e = m_TexFont.TextWidth(pos_str_e.c_str()) * scale;
                        TModelUnit text_h = m_TexFont.TextHeight() * (b_horz ? scale_y : scale_x);

                        // check if there is enougth space to draw number inside range
                        const TModelUnit text_gap = 2.0 * fabs(scale);
                        TModelUnit space =
                            (m_CurrRange.GetLength() -
                            (symbol_w + kSymbolBorder) * fabs(scale)) * 0.5 - text_gap;

                        if (space > fabs(text_w_s)) {
                            text_pos_s = m_CurrRange.GetFrom() + text_gap - (scale < 0 ? text_w_s : 0.0);
                        } else {
                            text_pos_s = m_CurrRange.GetFrom() - (scale < 0 ? 0.0 : text_w_s) - text_gap;
                        }

                        if (space > fabs(text_w_e)) {
                            text_pos_e = m_CurrRange.GetToOpen() - (scale < 0 ? 0.0 : text_w_e) - text_gap;
                        } else {
                            text_pos_e = m_CurrRange.GetToOpen() + text_gap - (scale < 0 ? text_w_e : 0.0);
                        }

                        TModelUnit x_off = 0.0, y_off = 0.0;
                        TModelUnit h_s = 0.0, w_s = 0.0, h_e = 0.0, w_e = 0.0;
                        if (b_horz) {
                            x_s = text_pos_s - offset_x;
                            y_s = (rc_curr.Bottom() + rc_curr.Top() - text_h) * 0.5 - offset_y;
                            x_e = text_pos_e - offset_x;
                            y_e = y_s;
                            x_off = scale_x;
                            y_off = 2.0 * scale_y;
                            h_s = h_e = text_h;
                            w_s = text_w_s;
                            w_e = text_w_e;
                        } else {
                            x_s = (rc_curr.Left() + rc_curr.Right() + text_h) * 0.5 - offset_x;
                            y_s = text_pos_s - offset_y;
                            x_e = x_s;
                            y_e = text_pos_e - offset_y;
                            x_off = -2.0 * scale_x;
                            y_off = scale_y;
                            h_s = text_w_s;
                            h_e = text_w_e;
                            w_s = w_e = -text_h;
                        }

                        // draw text background
                        gl.ColorC(bk_color);
                        {
                            gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                            gl.Rectd(x_s - x_off , y_s - y_off, x_s + w_s + x_off, y_s + h_s + y_off);
                            gl.Rectd(x_e - x_off , y_e - y_off, x_e + w_e + x_off, y_e + h_e + y_off);
                        }
                    }

                    gl.ColorC(border_c);

                    float rotation = 0.0f;
                    if (m_Orientation == eVert)
                        rotation = 90.0f;

                    gl.BeginText(&m_TexFont);                   
                    gl.WriteText(x_s, y_s, pos_str_s.c_str(), rotation);
                    gl.WriteText(x_e, y_e, pos_str_e.c_str(), rotation);
                    gl.EndText();
                }
            }
        }
    }
}



void CLinearSelHandler::OnMouseCaptureLost(wxMouseCaptureLostEvent& evt)
{
    // should we do anything here?
}


END_NCBI_SCOPE
