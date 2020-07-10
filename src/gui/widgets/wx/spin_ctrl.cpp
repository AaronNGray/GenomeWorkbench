/*  $Id: spin_ctrl.cpp 30297 2014-04-25 19:41:26Z asztalos $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/spin_ctrl.hpp>
#include <wx/settings.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>

BEGIN_NCBI_SCOPE

DEFINE_EVENT_TYPE(wxEVT_SPIN_CTRL_EVENT)

///////////////////////////////////////////////////////////////////////////////
///  CSpinControl

BEGIN_EVENT_TABLE(CSpinControl, wxControl)
    EVT_MOTION(CSpinControl::OnMouseMove)
    EVT_LEFT_DOWN(CSpinControl::OnLeftDown)
    EVT_LEFT_UP(CSpinControl::OnLeftUp)
    EVT_ENTER_WINDOW(CSpinControl::OnMouseMove)
    EVT_LEAVE_WINDOW(CSpinControl::OnMouseLeave)
    EVT_PAINT(CSpinControl::OnPaint)
END_EVENT_TABLE()

static const char up_xbm[]   = { 0x04, 0x0E, 0x1F };
static const char dn_xbm[]   = { 0x1F, 0x0E, 0x04 };
static const char plus_xbm[] = { 0x0C, 0x3F, 0x0C };

CSpinControl::CSpinControl(wxWindow* parent, wxWindowID id, const wxPoint& pos)
: m_OffScreen(kWidth, kHeight),
  m_ArrUp(up_xbm, 5, 3), m_ArrDn(dn_xbm, 5, 3), m_Plus(plus_xbm, 6, 3), m_ImageSet(0),
  m_Highlight(kNone), m_Clicking(kNone)
{
    Create(parent, id, pos);
}

void CSpinControl::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos)
{
    wxWindow::Create(parent, id, pos, wxSize(kWidth, kHeight), wxBORDER_NONE, wxT("spin_control"));
}

void CSpinControl::UseImageSet(int set)
{
    m_ImageSet = (set < 0 || set > 1) ? 0 : set;
}

wxSize CSpinControl::DoGetBestSize() const
{
    return wxSize(kWidth, kHeight);
}

void CSpinControl::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxMemoryDC memdc;
    memdc.SelectObject( m_OffScreen );

    memdc.SetBackground( wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE)) );
    memdc.Clear();

    int bwidht = kWidth;
    int bheight = (kHeight + 1)/2;

    x_DrawButton(memdc, bwidht, bheight,
        ((m_ImageSet == 1) ? m_Plus : m_ArrUp), (m_Highlight == kBtnUp), (m_Clicking == kBtnUp));
    memdc.SetDeviceOrigin(0, bheight - 1);
    x_DrawButton(memdc, bwidht, bheight,
        ((m_ImageSet == 1) ? m_Plus : m_ArrDn), (m_Highlight == kBtnDn), (m_Clicking == kBtnDn));

    memdc.SetPen( *wxBLACK_PEN );
    memdc.SetBrush( *wxWHITE_BRUSH );
    memdc.SelectObject( wxNullBitmap );

    wxPaintDC dc( this );
    dc.DrawBitmap( m_OffScreen, 0, 0 );
}

void CSpinControl::x_DrawButton(wxDC& dc, int w, int h, wxBitmap& img, bool highlight, bool clicked)
{
    dc.SetPen( wxPen(wxColor(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT)),1) );
    dc.DrawLine(1, 0, w - 1, 0);
    dc.DrawLine(0, 1, 0, h - 1);
    dc.DrawLine(1, h - 1, w - 1, h - 1);
    dc.DrawLine(w - 1, 1, w - 1, h - 1);

    dc.SetPen( wxPen(wxColor(wxSystemSettings::GetColour(clicked ? wxSYS_COLOUR_3DSHADOW : wxSYS_COLOUR_3DHILIGHT)),1) );
    dc.DrawLine(1, 1, w - 1, 1);
    dc.DrawLine(1, 2, 1, h - 2);

    dc.SetPen( wxPen(wxColor(wxSystemSettings::GetColour(clicked ? wxSYS_COLOUR_3DHILIGHT : wxSYS_COLOUR_3DSHADOW)),1) );
    dc.DrawLine(1, h - 2, w - 1, h - 2);
    dc.DrawLine(w - 2, 2, w - 2, h - 2);

    dc.SetTextForeground( highlight ? *wxRED : *wxBLACK );
    dc.SetTextBackground( wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE) );
    int imgX = (w - img.GetWidth())/2;
    int imgY = (h - img.GetHeight())/2;
    dc.DrawBitmap( img, imgX, imgY, true);
}

void CSpinControl::OnMouseMove(wxMouseEvent& event)
{
    EArea area = x_HitTest(event.m_x, event.m_y);
    if (area != m_Highlight) {
        m_Highlight = area;
        Refresh();
    }
    if (area != m_Clicking) {
        m_Clicking = kNone;
        Refresh();
    }
}

void CSpinControl::OnMouseLeave(wxMouseEvent& WXUNUSED(event))
{
    if (kNone != m_Highlight || kNone != m_Clicking) {
        m_Highlight = kNone;
        m_Clicking = kNone;
        Refresh();
    }
}

void CSpinControl::OnLeftDown(wxMouseEvent& event)
{
    EArea area = x_HitTest(event.m_x, event.m_y);
    if (area != m_Clicking) {
        m_Clicking = area;
        Refresh();
    }
}

void CSpinControl::OnLeftUp(wxMouseEvent& WXUNUSED(event))
{
    if (kNone != m_Clicking) {
        wxCommandEvent evt( wxEVT_SPIN_CTRL_EVENT, m_Clicking + m_ImageSet*2);
        evt.SetEventObject( this );
        GetEventHandler()->ProcessEvent( evt );
        m_Clicking = kNone;
        Refresh();
    }
}

CSpinControl::EArea CSpinControl::x_HitTest(int x, int y)
{
    if (x < 0 || x >= kWidth || y < 0 || y >= kHeight)
        return kNone;

    int bheight = (kHeight + 1)/2;
    if (y < bheight)
        return kBtnUp;
    if (y > bheight)
        return kBtnDn;

    return kNone;
}

/////////////////////////////////////////////////////////////
/// CNoTabSpinControl
///

CNoTabSpinControl::CNoTabSpinControl(wxWindow* parent, wxWindowID id, const wxPoint& pos)
    : CSpinControl(parent, id, pos)
{
}

END_NCBI_SCOPE