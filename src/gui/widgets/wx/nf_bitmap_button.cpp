/*  $Id: nf_bitmap_button.cpp 37699 2017-02-08 17:51:05Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/nf_bitmap_button.hpp>

#include <wx/dcclient.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CNoFocusBitmapButton

BEGIN_EVENT_TABLE(CNoFocusBitmapButton, wxWindow)
    EVT_MOTION(CNoFocusBitmapButton::OnMotion)
    EVT_LEAVE_WINDOW(CNoFocusBitmapButton::OnMouseLeave)
    EVT_LEFT_DOWN(CNoFocusBitmapButton::OnMouseDown)
    EVT_LEFT_UP(CNoFocusBitmapButton::OnMouseUp)
    EVT_MOUSE_CAPTURE_LOST(CNoFocusBitmapButton::OnCaptureLost)
    EVT_PAINT(CNoFocusBitmapButton::OnPaint)
END_EVENT_TABLE()

CNoFocusBitmapButton::CNoFocusBitmapButton(
    wxWindow* parent,
    wxWindowID id,
    const wxBitmap& bitmap,
    const wxBitmap& hoverBitmap,
    const wxPoint& pos)
    : m_Bitmap(bitmap), m_HoverBitmap(hoverBitmap), m_MouseOver(false)
{
    Create(parent, id, pos, wxSize(m_Bitmap.GetWidth(), m_Bitmap.GetHeight()), wxBORDER_NONE);
}

void CNoFocusBitmapButton::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    bool hover = (m_MouseOver != HasCapture());
    dc.DrawBitmap(hover ? m_HoverBitmap : m_Bitmap, 0, 0);
}

void CNoFocusBitmapButton::OnMotion(wxMouseEvent& event)
{
    if (GetClientRect().Contains(event.GetPosition())) {
        if (!m_MouseOver) {
            m_MouseOver = true;
            Refresh();
        }
    }
    else {
        if (m_MouseOver) {
            m_MouseOver = false;
            Refresh();
        }
    }
}

void CNoFocusBitmapButton::OnMouseLeave(wxMouseEvent& WXUNUSED(event))
{
    m_MouseOver = false;
    Refresh();
}

void CNoFocusBitmapButton::OnMouseDown(wxMouseEvent& WXUNUSED(event))
{
    CaptureMouse();
    m_MouseOver = true;
    Refresh();
}

void CNoFocusBitmapButton::OnMouseUp(wxMouseEvent& event)
{
    if (HasCapture())
        ReleaseMouse();

    if (GetClientRect().Contains(event.GetPosition())) {
        wxCommandEvent btnEvent(wxEVT_BUTTON, GetId());
        btnEvent.SetEventObject(this);
        GetParent()->GetEventHandler()->AddPendingEvent(btnEvent);
    }

    m_MouseOver = false;
    Refresh();
}

void CNoFocusBitmapButton::OnCaptureLost(wxMouseCaptureLostEvent& WXUNUSED(event))
{
    m_MouseOver = false;
    Refresh();
}

END_NCBI_SCOPE
