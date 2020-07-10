/*  $Id: large_spin_ctrl.cpp 38182 2017-04-06 18:57:26Z filippov $
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

#include <gui/widgets/edit/large_spin_ctrl.hpp>
#include <wx/settings.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>

BEGIN_NCBI_SCOPE

DEFINE_EVENT_TYPE(wxEVT_LARGE_SPIN_CTRL_EVENT)

///////////////////////////////////////////////////////////////////////////////
///  CLargeSpinControl

BEGIN_EVENT_TABLE(CLargeSpinControl, wxControl)
    EVT_MOTION(CLargeSpinControl::OnMouseMove)
    EVT_LEFT_DOWN(CLargeSpinControl::OnLeftDown)
    EVT_LEFT_UP(CLargeSpinControl::OnLeftUp)
    EVT_ENTER_WINDOW(CLargeSpinControl::OnMouseMove)
    EVT_LEAVE_WINDOW(CLargeSpinControl::OnMouseLeave)
    EVT_PAINT(CLargeSpinControl::OnPaint)
END_EVENT_TABLE()


static const char up_xpm[]       = { "\x00\x00\x00\x08\x1C\x3E\x7F\x00\x00\x00\x00" };
static const char down_xpm[]     = { "\x00\x00\x00\x7F\x3E\x1C\x08\x00\x00\x00\x00" };
static const char add_up_xpm[]   = { "\x00\xFF\x00\x00\x00\xFF\x00\xFF\x00\xFF\x00" };
static const char add_down_xpm[] = { "\x00\xFF\x00\xFF\x00\xFF\x00\x00\x00\xFF\x00" };

CLargeSpinControl::CLargeSpinControl(wxWindow* parent, wxWindowID id, const wxPoint& pos)
: m_OffScreen(kWidth, kHeight),
  m_ArrUp(up_xpm, 8, 11), m_ArrDn(down_xpm, 8, 11), m_PlusUp(add_up_xpm, 8, 11), m_PlusDown(add_down_xpm, 8, 11), m_ImageSet(0),
  m_Highlight(kNone), m_Clicking(kNone)
{
    Create(parent, id, pos);
}

void CLargeSpinControl::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos)
{
    wxWindow::Create(parent, id, pos, wxSize(kWidth, kHeight), wxBORDER_NONE, wxT("spin_control"));
}

void CLargeSpinControl::UseImageSet(int set)
{
    m_ImageSet = (set < 0 || set > 1) ? 0 : set;
}

wxSize CLargeSpinControl::DoGetBestSize() const
{
    return wxSize(kWidth, kHeight);
}

void CLargeSpinControl::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxMemoryDC memdc;
    memdc.SelectObject( m_OffScreen );

    memdc.SetBackground( wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE)) );
    memdc.Clear();

    int bwidht = kWidth / 2;
    int bheight = kHeight;

    x_DrawButton(memdc, bwidht, bheight,
        ((m_ImageSet == 1) ? m_PlusUp : m_ArrUp), (m_Highlight == kBtnUp), (m_Clicking == kBtnUp));
    memdc.SetDeviceOrigin(bwidht, 0);
    x_DrawButton(memdc, bwidht, bheight,
        ((m_ImageSet == 1) ? m_PlusDown : m_ArrDn), (m_Highlight == kBtnDn), (m_Clicking == kBtnDn));

    memdc.SetPen( *wxBLACK_PEN );
    memdc.SetBrush( *wxWHITE_BRUSH );
    memdc.SelectObject( wxNullBitmap );

    wxPaintDC dc( this );
    dc.DrawBitmap( m_OffScreen, 0, 0 );
}

void CLargeSpinControl::x_DrawButton(wxDC& dc, int w, int h, wxBitmap& img, bool highlight, bool clicked)
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

void CLargeSpinControl::OnMouseMove(wxMouseEvent& event)
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

void CLargeSpinControl::OnMouseLeave(wxMouseEvent& WXUNUSED(event))
{
    if (kNone != m_Highlight || kNone != m_Clicking) {
        m_Highlight = kNone;
        m_Clicking = kNone;
        Refresh();
    }
}

void CLargeSpinControl::OnLeftDown(wxMouseEvent& event)
{
    EArea area = x_HitTest(event.m_x, event.m_y);
    if (area != m_Clicking) {
        m_Clicking = area;
        Refresh();
    }
}

void CLargeSpinControl::OnLeftUp(wxMouseEvent& WXUNUSED(event))
{
    if (kNone != m_Clicking) {
        wxCommandEvent evt( wxEVT_LARGE_SPIN_CTRL_EVENT, m_Clicking + m_ImageSet*2);
        evt.SetEventObject( this );
        GetEventHandler()->ProcessEvent( evt );
        m_Clicking = kNone;
        Refresh();
    }
}

CLargeSpinControl::EArea CLargeSpinControl::x_HitTest(int x, int y)
{
    if (x < 0 || x >= kWidth || y < 0 || y >= kHeight)
        return kNone;

    int bwidth = kWidth/2;
    if (x < bwidth)
        return kBtnUp;
    if (x > bwidth)
        return kBtnDn;

    return kNone;
}

/////////////////////////////////////////////////////////////
/// CLargeNoTabSpinControl
///

CLargeNoTabSpinControl::CLargeNoTabSpinControl(wxWindow* parent, wxWindowID id, const wxPoint& pos)
    : CLargeSpinControl(parent, id, pos)
{
}

END_NCBI_SCOPE
