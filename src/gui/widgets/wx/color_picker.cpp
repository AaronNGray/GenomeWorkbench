/*  $Id: color_picker.cpp 21074 2010-03-19 17:58:40Z tereshko $
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

#include <wx/dcclient.h>
#include <gui/widgets/wx/color_picker.hpp>
#include <wx/colordlg.h>

#include <corelib/ncbistd.hpp>

#ifdef NCBI_OS_DARWIN_NEVER
#include <Carbon/Carbon.h>

#include <wx/wx.h>
#include <wx/settings.h>

#include <wx/clrpicker.h> // wxColourPickerEvent

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///  CColorPickerMac

BEGIN_EVENT_TABLE(CColorPickerMac, wxControl)
    EVT_PAINT(CColorPickerMac::OnPaint)
    EVT_MOUSE_EVENTS(CColorPickerMac::OnMouseEvent)
END_EVENT_TABLE()


IMPLEMENT_DYNAMIC_CLASS(CColorPickerMac, wxControl)

CColorPickerMac::CColorPickerMac()
{
}

CColorPickerMac::CColorPickerMac(wxWindow *parent, wxWindowID id,
        const wxColour& colour, const wxPoint& pos, const wxSize& size,
        long style, const wxValidator& validator, const wxString& name)
:   wxControl(parent, id, pos, size, style, validator, name)
{
    m_ColourData.SetColour(colour);
    m_Down = false;
}

CColorPickerMac::~CColorPickerMac()
{
}

void CColorPickerMac::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    wxRect rect = GetClientRect();
    
    ThemeButtonDrawInfo info;
    info.value = kThemeButtonOff;
    info.state = m_Down ? kThemeStatePressed : kThemeStateActive;
                 //active_r() ? kThemeStateActive : kThemeStateInactive;
    info.adornment = kThemeAdornmentNone;//Fl::focus() == this ? kThemeAdornmentFocus : kThemeAdornmentNone;

    Rect bounds;
    SetRect (&bounds, rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
    DrawThemeButton(&bounds, kThemeBevelButton, &info, NULL, NULL, NULL, 0);

    wxBrush brush( m_ColourData.GetColour() );
    wxPen pen( m_ColourData.GetColour() );
    dc.SetBrush(brush);
    dc.SetPen(pen);

    dc.DrawRectangle(rect.x+3, rect.y+3, rect.x+rect.width-6, rect.y+rect.height-6);
}

void CColorPickerMac::OnMouseEvent(wxMouseEvent& event)
{
    if (event.LeftDown()) {
        m_Down = true;
        Refresh();
    } else if (event.LeftUp()) {
        m_Down = false;
        Refresh();
        wxWindow* parent = GetParent();
        while (parent != NULL &&
                !parent->IsKindOf(CLASSINFO(wxDialog)) &&
                !parent->IsKindOf(CLASSINFO(wxFrame)))
            parent = parent->GetParent();

        wxColourDialog dlg(parent, &m_ColourData);
        if (dlg.ShowModal() == wxID_OK) {
            m_ColourData = dlg.GetColourData();
            Refresh();
            wxCommandEvent event(wxEVT_COMMAND_COLOURPICKER_CHANGED, GetId());
            event.SetEventObject(this);
            GetEventHandler()->ProcessEvent(event);
        }
    }
}

wxColour CColorPickerMac::GetColour () const
{
    return m_ColourData.GetColour();
}

void CColorPickerMac::SetColour(const wxColour &colour)
{
    m_ColourData.SetColour(colour);
    Refresh();
}


END_NCBI_SCOPE
#endif
