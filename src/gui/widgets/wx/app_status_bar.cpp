/*  $Id: app_status_bar.cpp 23970 2011-06-27 18:15:09Z kuznets $
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
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/app_status_bar.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/gauge.h>
#include <wx/settings.h>

BEGIN_NCBI_SCOPE


BEGIN_EVENT_TABLE(CStatusBar, wxStatusBar)
    EVT_SIZE(CStatusBar::OnSize)
END_EVENT_TABLE()


CStatusBar::CStatusBar(wxWindow* parent, wxWindowID id, long style,
                           const wxString& name)
: wxStatusBar(parent, id, style | wxCLIP_CHILDREN, name)
{
}


CStatusBar::~CStatusBar()
{
}


void CStatusBar::SetFieldsCount( int number, const int* widths )
{
    m_FieldWindows.resize( number );
    m_FieldWidths.resize( number );
    if( widths ){
        for( int i = 0; i < number;  i++  ) {
            m_FieldWidths[i] = widths[i];
        }
    } else {
        m_FieldWidths.assign( number, -1 );
    }

    wxStatusBar::SetFieldsCount( number, &m_FieldWidths[0] );
}


void CStatusBar::SetStatusWidths( int number, const int* widths )
{
    m_FieldWidths.resize( number );
    for( int i = 0; i < number;  i++  ){
        m_FieldWidths[i] = widths[i];
    }
    wxStatusBar::SetStatusWidths( number, &m_FieldWidths[0] );
}


void CStatusBar::SetFieldWindow(int index, wxWindow* window)
{
    wxWindow* old_win = m_FieldWindows[index];
    if(old_win)  {
        //remove window
        old_win->Destroy();
        m_FieldWindows[index] = NULL;
    }

    if(window)  {
        m_FieldWindows[index] = window;
        window->Reparent(this);
    }
}


void CStatusBar::InsertFieldWindow(int index, wxWindow* window, int width)
{
    m_FieldWindows.insert(m_FieldWindows.begin() + index, window);
    m_FieldWidths.insert(m_FieldWidths.begin() + index, width);

    int n = (int)m_FieldWidths.size();
    wxStatusBar::SetFieldsCount(n, &m_FieldWidths[0]);
    if (window) window->Reparent(this);
    Layout();
    if (window) window->Refresh();
}


int CStatusBar::AddFieldWindow(wxWindow* window)
{
    int index = GetFieldsCount();
    SetFieldWindow(index, window);
    return index;
}


void CStatusBar::RemoveFieldWindow(int index)
{
    m_FieldWindows.erase(m_FieldWindows.begin() + index);
    m_FieldWidths.erase(m_FieldWidths.begin() + index);

    int n = (int)m_FieldWidths.size();
    wxStatusBar::SetFieldsCount(n, &m_FieldWidths[0]);
    wxStatusBar::SetStatusWidths(n, &m_FieldWidths[0]);
}


void CStatusBar::OnSize(wxSizeEvent& WXUNUSED(event))
{
    Layout();
}


bool CStatusBar::Layout()
{
    size_t n = m_FieldWindows.size();
    wxRect rect;

    for(  size_t i = 0;  i < n;  i++  ) {
        wxWindow* window = m_FieldWindows[i];
        if(window)  {
            GetFieldRect((int)i, rect);
            rect.SetX(rect.GetX() + 1);
            rect.SetY(rect.GetY() + 1);
            rect.SetWidth(rect.GetWidth() - 2);
            rect.SetHeight(rect.GetHeight() - 2);

            window->SetSize(rect);
        }
    }
    return true;
}


wxColour CStatusBar::GetBackColor(bool hot)
{
    wxColor cl = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
    if(hot) {
        wxColor cl_light = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT);
        wxColor cl_mix = GetAverage(cl, cl_light, 0.5);
        return cl_mix;
    }
    return cl;
}


wxColour CStatusBar::GetTextColor(bool hot)
{
    if(hot) {
        return wxColour(0, 0, 204);
    } else {
        return wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
    }
}


END_NCBI_SCOPE
