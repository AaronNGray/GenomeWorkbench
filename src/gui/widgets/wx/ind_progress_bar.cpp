/*  $Id: ind_progress_bar.cpp 35341 2016-04-26 18:51:00Z katargir $
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

#include <gui/widgets/wx/ind_progress_bar.hpp>

#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/dcmemory.h>
#include <wx/dcclient.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CMessageSlot

BEGIN_EVENT_TABLE(CIndProgressBar, wxControl)
    EVT_PAINT(CIndProgressBar::OnPaint)
    EVT_TIMER(-1, CIndProgressBar::OnTimer)
END_EVENT_TABLE()


CIndProgressBar::CIndProgressBar(wxWindow* parent, wxWindowID id, const wxPoint& pos, int width)
: m_Timer(this), m_CurPos(0), m_BmWidth(100)
{
    Create(parent, id, pos, width);
}

void CIndProgressBar::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, int width)
{
    static bool bitmap_registered = false;
    if (! bitmap_registered) {
        GetDefaultFileArtProvider()->RegisterFileAlias(
                                    wxT("progress_bar::barberpole"),
                                    wxT("barberpole.png"));
        bitmap_registered = true;
   }

    m_Bitmap = wxArtProvider::GetBitmap(wxT("progress_bar::barberpole"));

    int height = 9;
    if (m_Bitmap.IsOk()) {
        height = m_Bitmap.GetHeight();
        m_BmWidth = m_Bitmap.GetWidth();
    }

    wxControl::Create(parent, id, pos, wxSize(width, height + 4), wxBORDER_NONE, wxDefaultValidator, wxT("ind_progress_bar"));

    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    Enable(false);

    m_Timer.Start(50);
}


void CIndProgressBar::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxSize size = GetClientSize();
    wxCoord width = size.GetWidth(), height = size.GetHeight();

    wxColour bkgColor = GetParent()->GetBackgroundColour();

    wxBitmap bitmap(width, height);

    wxMemoryDC memdc;
    memdc.SelectObject( bitmap );
    memdc.SetPen(bkgColor);
    memdc.SetBrush(bkgColor);
    memdc.DrawRectangle(0, 0, width, height);

    if (m_Bitmap.IsOk())
        height = m_Bitmap.GetHeight() + 4;
    wxCoord y = (size.GetHeight() - height) / 2;

    memdc.SetPen( *wxBLACK_PEN );
    memdc.SetBrush( *wxWHITE_BRUSH );
    memdc.DrawRectangle(0, y, width, height);

    if (m_Bitmap.IsOk()) {
        memdc.SetClippingRegion( 2, y, width - 4, height);

        for (int pos = (m_CurPos - m_BmWidth + 1) + 2; pos < width - 2; pos += m_BmWidth) {
            memdc.DrawBitmap(m_Bitmap, pos, y + 2);
        }
    }

    memdc.SelectObject( wxNullBitmap );

    wxPaintDC dc( this );
    dc.DrawBitmap(bitmap, 0, 0);
}

void CIndProgressBar::OnTimer(wxTimerEvent& WXUNUSED(event))
{
    m_CurPos = (m_CurPos + 1) % m_BmWidth;
    Refresh();
}

END_NCBI_SCOPE
