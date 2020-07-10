/*  $Id: dock_hint_frame.cpp 37486 2017-01-13 20:44:49Z katargir $
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


#include <gui/widgets/wx/dock_hint_frame.hpp>

#include <gui/widgets/wx/dock_manager.hpp>

#include <wx/settings.h>
#include <wx/dcclient.h>
#include <wx/panel.h>

#if defined(__WXGTK20__)
    #include <gtk/gtk.h>
#endif

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CPseudoTransparentFrame


BEGIN_EVENT_TABLE(CPseudoTransparentFrame, wxFrame)
    EVT_KEY_DOWN(CPseudoTransparentFrame::OnKeyUp)
    EVT_KEY_UP(CPseudoTransparentFrame::OnKeyDown)

#ifndef __WXGTK20__
    EVT_PAINT(CPseudoTransparentFrame::OnPaint)

#ifdef __WXGTK__
    EVT_WINDOW_CREATE(CPseudoTransparentFrame::OnWindowCreate)
#endif

#endif //__WXGTK20__
END_EVENT_TABLE()



/// For GTK 20 Define a size callback and a special version of the constructor
/// and SetTransparent() function
#ifdef __WXGTK20__   ///////////////////////////////////////////////////////////

static void
gtk_pseudo_window_realized_callback( GtkWidget *m_widget, void *win )
{
    wxSize disp = wxGetDisplaySize();
    int amount = 128;
    wxRegion region;
    for (int y=0; y<disp.y; y++)
    {
        // Reverse the order of the bottom 4 bits
        int j=((y&8)?1:0)|((y&4)?2:0)|((y&2)?4:0)|((y&1)?8:0);
        if ((j*16+8)<amount)
            region.Union(0, y, disp.x, 1);
    }
    gdk_window_shape_combine_region(m_widget->window, region.GetRegion(), 0, 0);
}


const static long kTransWindowStyle =
                    wxFRAME_FLOAT_ON_PARENT | wxNO_BORDER | wxFRAME_NO_TASKBAR;

CPseudoTransparentFrame::CPseudoTransparentFrame(wxWindow* parent,
                                                     CDockManager& manager)
:   wxFrame(parent, wxID_ANY, wxT("Pseudo Transparent Frame"), wxDefaultPosition,
            wxDefaultSize, kTransWindowStyle, wxT("pseudo transparent frame")),
    m_DockManager(manager)
{
    g_signal_connect(m_widget, "realize",
                     G_CALLBACK(gtk_pseudo_window_realized_callback), this );

    GdkColor col;
    col.red = 128 * 256;
    col.green = 192 * 256;
    col.blue = 255 * 256;
    gtk_widget_modify_bg( m_widget, GTK_STATE_NORMAL, &col );
}


bool CPseudoTransparentFrame::SetTransparent(wxByte alpha)
{
    return true;
}


#else /// For platforms except GTK 2 (__WXGTK20__) /////////////////////////////

static const int kDefStyle = wxFRAME_TOOL_WINDOW | wxFRAME_FLOAT_ON_PARENT |
                             wxFRAME_NO_TASKBAR | wxNO_BORDER | wxFRAME_SHAPED;

CPseudoTransparentFrame::CPseudoTransparentFrame(wxWindow* parent,
                                                     CDockManager& manager)
:   wxFrame(parent, wxID_ANY, wxEmptyString, wxDefaultPosition,
            wxDefaultSize, kDefStyle, wxEmptyString),
    m_DockManager(manager)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    m_Amount = 0;
    m_MaxWidth = 0;
    m_MaxHeight = 0;
    m_LastWidth = 0;
    m_LastHeight = 0;

#ifdef __WXGTK__
    m_CanSetShape = false; // have to wait for window create event on GTK
#else
    m_CanSetShape = true;
#endif

    m_Region = wxRegion(0, 0, 0, 0);
    SetTransparent(0);
}


bool CPseudoTransparentFrame::SetTransparent(wxByte alpha)
{
    if (m_CanSetShape)  {
        int w = 100, h = 100;
        GetClientSize(&w, &h);

        m_MaxWidth = w;
        m_MaxHeight = h;
        m_Amount = alpha;
        m_Region.Clear();

        if (m_Amount)   {
            for (int y = 0; y < m_MaxHeight; y++)   {
                // Reverse the order of the bottom 4 bits
                int j=((y&8)?1:0)|((y&4)?2:0)|((y&2)?4:0)|((y&1)?8:0);
                if ((j*16 + 8) < m_Amount)
                    m_Region.Union(0, y, m_MaxWidth, 1);
            }
        }
        SetShape(m_Region);
        Refresh();
    }
    return true;
}


void CPseudoTransparentFrame::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);

    if (m_Region.IsEmpty())
        return;

#ifdef __WXMAC__
    dc.SetBrush(wxColour(128, 192, 255));
#else
    dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
#endif
    dc.SetPen(*wxTRANSPARENT_PEN);

    wxRegionIterator upd(GetUpdateRegion()); // get the update rect list

    while(upd) {
        wxRect rect(upd.GetRect());
        dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);

        upd++;
    }
}


/// Define OnWindowCreate() only for __WXGTK__
#ifdef __WXGTK__
void CPseudoTransparentFrame::OnWindowCreate(wxWindowCreateEvent& WXUNUSED(event))
{
    m_CanSetShape = true;
    SetTransparent(0);
}
#endif // Define OnWindowCreate()


void CPseudoTransparentFrame::OnSize(wxSizeEvent& event)
{
    // We sometimes get surplus size events
    if ((event.GetSize().GetWidth() == m_LastWidth) &&
        (event.GetSize().GetHeight() == m_LastHeight))
    {
        event.Skip();
        return;
    }
    m_LastWidth = event.GetSize().GetWidth();
    m_LastHeight = event.GetSize().GetHeight();

    SetTransparent(m_Amount);
    m_Region.Intersect(0, 0, event.GetSize().GetWidth(),
        event.GetSize().GetHeight());

    SetShape(m_Region);
    Refresh();

    event.Skip();
}


#endif // else #ifdef__WXGTK20__ ///////////////////////////////////////////////


/// Define functions that are the same for all platforms
void CPseudoTransparentFrame::OnKeyUp(wxKeyEvent& event)
{
    m_DockManager.OnKeyUp(event);
}


void CPseudoTransparentFrame::OnKeyDown(wxKeyEvent& event)
{
    m_DockManager.OnKeyDown(event);
}


// find out if the the system can do transparent frames
static bool sCanDoTransparent(wxWindow& parent)
{
    for(  wxWindow* w = &parent;  w;  w = w->GetParent()  )   {
        wxFrame* frame = dynamic_cast<wxFrame*>(w);
        if(frame)   {
            return frame->CanSetTransparent();
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
/// CTransparentFrame
#if defined(__WXMAC__)
CTransparentFrame::CTransparentFrame(wxWindow* parent, CDockManager& manager)
:   TTransFrameBase(parent, wxID_ANY, wxEmptyString, wxDefaultPosition,
                    wxSize(1,1),
                    wxFRAME_FLOAT_ON_PARENT | wxFRAME_TOOL_WINDOW | wxBORDER_SIMPLE),
    m_DockManager(manager)
{
    // The default wxSYS_COLOUR_ACTIVECAPTION color is a light silver
    // color that is really hard to see.
    
    // The behavior on mac carbon and mac cocoa is different - on carbon
    // we get trasparency but a nice border, and on cocoa we get the selected
    // color, but no border.  Either provides a good hint.
    this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW));
}
#else
CTransparentFrame::CTransparentFrame(wxWindow* parent, CDockManager& manager)
:   TTransFrameBase(parent, wxID_ANY, wxEmptyString, wxDefaultPosition,
                    wxSize(1,1),
                    wxFRAME_TOOL_WINDOW | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR |wxNO_BORDER),
    m_DockManager(manager)
{
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
}
#endif

CTransparentFrame::~CTransparentFrame()
{
    // prevent main window activation
    SetWindowStyleFlag(GetWindowStyleFlag()&~wxFRAME_FLOAT_ON_PARENT);
}

BEGIN_EVENT_TABLE(CTransparentFrame, TTransFrameBase)
    EVT_KEY_DOWN(CTransparentFrame::OnKeyUp)
    EVT_KEY_UP(CTransparentFrame::OnKeyDown)
END_EVENT_TABLE()


void CTransparentFrame::OnKeyUp(wxKeyEvent& event)
{
    m_DockManager.OnKeyUp(event);
}


void CTransparentFrame::OnKeyDown(wxKeyEvent& event)
{
    m_DockManager.OnKeyDown(event);
}


wxWindow* CreateDockHintFrame(wxWindow& parent, CDockManager& manager)
{
    wxWindow* wnd = NULL;
    bool can_do_transparent = sCanDoTransparent(parent);

    if (can_do_transparent) {
        wnd = new CTransparentFrame(&parent, manager);
    } else {
        wnd = new CPseudoTransparentFrame(&parent, manager);
    }
    wnd->SetTransparent(static_cast<wxByte>(96));
    return wnd;
}


END_NCBI_SCOPE
