/*  $Id: seqmark_handler.cpp 45030 2020-05-11 18:30:27Z evgeniev $
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
 * File Description:
 *
 */

#include <ncbi_pch.hpp>


#include <gui/widgets/gl/seqmark_handler.hpp>
#include <gui/widgets/wx/gui_event.hpp>
#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/irender.hpp>

#include <math.h>


BEGIN_NCBI_SCOPE


const int kMarkerSize = 6;
////////////////////////////////////////////////////////////////////////////////
/// class CSeqMarkHandler
BEGIN_EVENT_TABLE(CSeqMarkHandler, wxEvtHandler)
    EVT_LEFT_DOWN(CSeqMarkHandler::OnLeftDown)
    EVT_LEFT_DCLICK(CSeqMarkHandler::OnLeftDoubleClick)
    EVT_LEFT_UP(CSeqMarkHandler::OnLeftUp)
    EVT_MOTION(CSeqMarkHandler::OnMotion)
    EVT_MOUSE_CAPTURE_LOST(CSeqMarkHandler::OnMouseCaptureLost)
END_EVENT_TABLE()


CSeqMarkHandler::CSeqMarkHandler()
    : m_Host(NULL)
    , m_MarkerType(ePoint)
    , m_MarkDrag(false)
    , m_ShowCoord(true)
    , m_DragPos(-1.0)
    , m_ExtendedPos(-1.0)
    , m_PosOff(0.0)
    , m_Font_Helv10(CGlTextureFont::eFontFace_Helvetica, 10)
    , m_LabelFont(CGlTextureFont::eFontFace_Helvetica, 10)
    , m_Color(1.0f, 0.0f, 0.0f, 0.3f)
    , m_HitMe(false)
    , m_Flipped(false)
{
}


CSeqMarkHandler::~CSeqMarkHandler()
{
}


void CSeqMarkHandler::SetPane(CGlPane* pane)
{
    m_Pane = pane;
}


wxEvtHandler* CSeqMarkHandler::GetEvtHandler()
{
    return this;
}


void CSeqMarkHandler::SetHost(ISeqMarkHandlerHost* host)
{
    m_Host = host;
}


IGenericHandlerHost* CSeqMarkHandler::GetGenericHost()
{
    return dynamic_cast<IGenericHandlerHost*>(m_Host);
}

void CSeqMarkHandler::SetMarkerType(EMarkerType t)
{
    m_MarkerType = t;
}

void CSeqMarkHandler::SetFlipped(bool flipped)
{
    m_Flipped = flipped;
}

void CSeqMarkHandler::SetPos(TModelUnit pos)
{ 
    // When it's a range marker, maintain m_DragPos as the smaller value
    // (unless m_ExtendedPos is uninitialized).
    if (m_MarkerType == eRange && 
        m_ExtendedPos >= 0.0f &&
        pos > m_ExtendedPos) {
            m_DragPos = m_ExtendedPos;
            m_ExtendedPos = pos;
    }
    else {
         m_DragPos = (int)pos; 
    }
}

TModelUnit CSeqMarkHandler::GetPosLeft() const
{ 
    if (m_MarkerType == ePoint)
        return m_DragPos;

    if (!m_Flipped)
        return m_DragPos;
    else
        return m_ExtendedPos;
}

void  CSeqMarkHandler::SetExtendedPos(TModelUnit pos)
{ 
    // When it's a range marker, maintain m_DragPos as the smaller value
    if (m_MarkerType == eRange && 
        m_DragPos > pos) {
            m_ExtendedPos = m_DragPos;
            m_DragPos = pos;
    }
    else {
        m_ExtendedPos = (int)pos;
    }
}

void CSeqMarkHandler::SetRange(TSeqRange range)
{
    m_DragPos = range.GetFrom();
    m_ExtendedPos = range.GetTo();

    if (m_ExtendedPos < m_DragPos)
        std::swap(m_DragPos, m_ExtendedPos);
}

TModelUnit CSeqMarkHandler::GetPosRight() const
{ 
    // Only applies to range markers
    if (!m_Flipped)
        return m_ExtendedPos;
    else
        return m_DragPos;
}

////////////////////////////////////////////////////////////////////////////////
// event handlers

void CSeqMarkHandler::OnLeftDown(wxMouseEvent& event)
{
    _ASSERT(m_Pane);

    CGUIEvent::EGUIState state = CGUIEvent::wxGetSelectState(event);
    m_PosOff = 0.0;
    if (state == CGUIEvent::eSelectState)  {
        wxPoint pos = event.GetPosition();
        TModelUnit x_pos = m_Host->SMHH_GetModelByWindow(pos.x, eHorz);
        TModelUnit y_pos = m_Host->SMHH_GetModelByWindow(pos.y, eVert);

        TModelPoint click_point = TModelPoint(x_pos, y_pos);
        TModelRect rc = x_GetMarkRect(*m_Pane);

        if (rc.PtInRect(click_point) ) {
            m_MarkDrag = true;
            TSeqPos pos = m_Host->SMHH_GetSeqMark();
            if (pos != (TSeqPos)(-1)) {
                SetPos((TModelUnit)pos);
            }
            m_PosOff = GetPosLeft() - x_pos;
            GetGenericHost()->GHH_CaptureMouse();
            GetGenericHost()->GHH_Redraw();
            return;
        }
    }
    event.Skip();
}


void CSeqMarkHandler::OnLeftDoubleClick(wxMouseEvent& event)
{
    CGUIEvent::EGUIState state = CGUIEvent::wxGetSelectState(event);

    if (state == CGUIEvent::eSelectState)  {
        wxPoint pos = event.GetPosition();
        TModelUnit x_pos = m_Host->SMHH_GetModelByWindow(pos.x, eHorz);
        TModelUnit y_pos = m_Host->SMHH_GetModelByWindow(pos.y, eVert);

        TModelPoint click_point = TModelPoint(x_pos, y_pos);
        TModelRect rc = x_GetMarkRect(*m_Pane);

        if (rc.PtInRect(click_point) ) {
            m_MarkDrag = false;
            SetPos(0);
            m_Host->SMHH_SetSeqMark(0); // reset to 0
            m_Host->SMHH_OnReset(GetId());
            GetGenericHost()->GHH_Redraw();
            return;
        }
    }
    event.Skip();
}

TModelRect CSeqMarkHandler::x_GetMarkRect(CGlPane& pane) const
{
    TModelRect rc     = pane.GetVisibleRect();
    TModelUnit half_x = pane.UnProjectWidth(kMarkerSize);
    TModelUnit half_y = pane.UnProjectHeight(kMarkerSize);
    TModelUnit pos = GetPosLeft();
    TSeqPos marker_pos = m_Host->SMHH_GetSeqMark();
    if (marker_pos != (TSeqPos)(-1)) {
        pos = (TModelUnit)marker_pos;
    }

    if (m_Flipped)
        pos += 1.0;

    IRender& gl = GetGl();
    TModelUnit l_w = gl.TextWidth(&m_LabelFont, m_Label.c_str()) + kMarkerSize;
    TModelRect res;    
    l_w = pane.UnProjectWidth(l_w);
    res = TModelRect(pos - half_x, rc.Top() - half_y * 2,
                     pos + half_x + l_w, rc.Top());

    return res;
}


void CSeqMarkHandler::OnMotion(wxMouseEvent& event)
{
    m_HitMe = false;
    if(event.Dragging())    {
        if (m_MarkDrag) {
            wxPoint ms_pos = event.GetPosition();
            TModelUnit prev_pos = GetPosLeft();
            TModelUnit new_pos = 0.0;
            	
            // Don't let mouse drag marker outside of viewport
            // (but in a range marker, it could be partly outside)
            GLint vp[4];
            glGetIntegerv(GL_VIEWPORT, vp);
            if (ms_pos.x < vp[0])
                ms_pos.x = vp[0];
            else if (ms_pos.x > vp[0] + vp[2])
                ms_pos.x = vp[0] + vp[2];
          
            new_pos = m_PosOff + m_Host->SMHH_GetModelByWindow(ms_pos.x, eHorz);
        
            // Move both endpoints of range markers
            if (m_MarkerType == eRange) {
                TModelUnit delta =  new_pos - prev_pos;                

                if (!m_Flipped) {
                    m_DragPos = new_pos;
                    m_ExtendedPos += delta;
                }
                else {
                    m_ExtendedPos = new_pos;
                    m_DragPos += delta;
                }

            }
            else {
                m_DragPos = new_pos;
            }
            GetGenericHost()->GHH_Redraw();
        }
        // always handle drags
    } else {
        wxPoint pos = event.GetPosition();
        TModelUnit x_pos = m_Host->SMHH_GetModelByWindow(pos.x, eHorz);
        TModelUnit y_pos = m_Host->SMHH_GetModelByWindow(pos.y, eVert);

        TModelPoint click_point = TModelPoint(x_pos, y_pos);
        TModelRect rc = x_GetMarkRect(*m_Pane);

        if ( rc.PtInRect(click_point) ) {
            m_HitMe = true;
        }

        event.Skip();
    }
}


void CSeqMarkHandler::OnLeftUp(wxMouseEvent& event)
{
    if (m_MarkDrag) {
        x_OnEndAction();
    }
}

void CSeqMarkHandler::OnMouseCaptureLost(wxMouseCaptureLostEvent& evt)
{
    if (m_MarkDrag) {
        x_OnEndAction();
    }
}

void CSeqMarkHandler::x_OnEndAction()
{
    m_MarkDrag = false;
    if (m_MarkerType == eRange)
        m_ExtendedPos = floor(m_ExtendedPos < 0 ? 0 : m_ExtendedPos + 0.5);

    SetPos(TModelUnit(GetPos()));
    m_Host->SMHH_SetSeqMark( x_SeqDragPos() );
    GetGenericHost()->GHH_ReleaseMouse();
    GetGenericHost()->GHH_Redraw();
}

////////////////////////////////////////////////////////////////////////////////
/// helper functions

TSeqPos CSeqMarkHandler::x_SeqDragPos() const
{
    // round position to nearest integer coordinate
    return (TSeqPos) floor(m_DragPos < 0 ? 0 : m_DragPos+0.5);
}


void CSeqMarkHandler::x_DrawMarker(TModelUnit x_c, TModelUnit y_c, int half) const
{
    IRender& gl = GetGl();

    gl.Begin(GL_TRIANGLE_STRIP);

    double incr = M_PI / 10;

    gl.Vertex2d(x_c, y_c - half);
    gl.Vertex2d(x_c, y_c + half);

    double a = M_PI / 2 + incr;
    for (int i = 0; i < 4; ++i) {
        float sina = half*sin(a), cosa = half*cos(a);
        gl.Vertex2d(x_c + cosa, y_c - sina);
        gl.Vertex2d(x_c + cosa, y_c + sina);
        a += incr;
    }

    gl.Vertex2d(x_c - half, y_c);

    gl.End();
}

void CSeqMarkHandler::x_DrawLabel(TModelUnit x, TModelUnit y, int half) const
{
    IRender& gl = GetGl();
    TModelUnit w = gl.TextWidth(&m_LabelFont, m_Label.c_str()) + kMarkerSize;
    TModelUnit h = gl.TextHeight(&m_LabelFont) + 1.0;

    gl.Rectd(x, y-half, x+w, y+half);

    if (!m_Label.empty()) {
        y = y - (h / 2) + 1;
        x += 2.0;

        gl.Color4f(1.0f, 1.0f, 1.0f, 1.0f);
        gl.BeginText(&m_LabelFont);
        gl.WriteText(x, y, m_Label.c_str());
        gl.EndText();
    }
}


void CSeqMarkHandler::Render(CGlPane& pane, ERenderingOption option)
{
    CGlAttrGuard AttrGuard(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_HINT_BIT);
    CGlPaneGuard GUARD(pane, CGlPane::ePixels);

    const TVPRect& rcV = pane.GetViewport();
    IRender& gl = GetGl();
    
    TModelUnit pos = GetPosLeft();
    TModelUnit end_pos = GetPosRight();
    if (m_MarkerType == ePoint)
        end_pos = pos;

    /// I think this is not used (at lest not with seqgraphic_pane)
    if ( !m_DragPos ) {
        TSeqPos marker_pos = m_Host->SMHH_GetSeqMark();
        if (marker_pos != (TSeqPos)(-1)) {
            pos = (TModelUnit)marker_pos + 0.5;
        }
    }

    TModelPoint p;
    string text_out = x_GetText(TSeqPos(pos + 1.0));

    TModelUnit width = gl.TextWidth(&m_Font_Helv10, text_out.c_str()) + kMarkerSize;
    TModelUnit off = option == ePassiveState ? 0 : kMarkerSize * 1.6;

    m_Color.SetAlpha(0.3f);
    gl.ColorC(m_Color);
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // draw the markers so that they include the base-pair characters 
    // so we draw the first marker at the position and the end is
    // set to the last position + 1
    TModelUnit cur_pos;
    TModelUnit cur_end_pos;
    TModelUnit text_pos;

    if (m_Flipped) {
        cur_pos = pane.ProjectX(pos + 1.0);
        cur_end_pos = pane.ProjectX(end_pos);
    }
    else {
        cur_pos = pane.ProjectX(pos);
        cur_end_pos = pane.ProjectX(end_pos + 1.0);
    }
    text_pos = cur_pos;

    m_Color.SetAlpha(0.2f);
    gl.ColorC(m_Color);
    gl.Rectd(cur_pos, rcV.Top() - off, cur_end_pos, rcV.Bottom() - 1);

    m_Color.SetAlpha(0.4f);
    gl.ColorC(m_Color);

    gl.Begin(GL_LINES); // make lines at edges a little darker
    gl.Vertex2d(cur_pos, rcV.Bottom());
    gl.Vertex2d(cur_pos, rcV.Top() - off);
    gl.Vertex2d(cur_end_pos, rcV.Bottom());
    gl.Vertex2d(cur_end_pos, rcV.Top() - off);
    gl.End();

    if (option == ePassiveState  &&  m_ShowCoord) { // show current position
        // Want to keep text glyph on left-hand side of marker always
        text_pos -= width / 2.0;
        if (text_pos < 1.0)
            text_pos = 1.0;

        m_Color.SetAlpha(1.0f);
        gl.ColorC(m_Color);
        gl.BeginText(&m_Font_Helv10);
        gl.WriteText(text_pos, rcV.Bottom() + 2, text_out.c_str());
        gl.EndText();
    }
    p = TModelPoint(cur_pos, rcV.Top() - kMarkerSize - 1);


    // Active State (pane)
    if(option == eActiveState) {  // Draw mark texture
        m_Color.SetAlpha(1.0f);
        gl.ColorC(m_Color);
        x_DrawMarker(p.X(), p.Y(), kMarkerSize);

        m_Color.SetAlpha(0.6f);
        gl.ColorC(m_Color);
        x_DrawLabel(p.X(), p.Y(), kMarkerSize);
    }  // option == eActiveState
}

string CSeqMarkHandler::x_GetText(TSeqPos pos) 
{
    return NStr::SizetToString( (unsigned long) pos, NStr::fWithCommas);
}



END_NCBI_SCOPE
