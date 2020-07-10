/*  $Id: workbench_frame.cpp 43956 2019-09-25 21:02:14Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include <wx/platform.h>

#include <gui/framework/workbench_frame.hpp>
#include <gui/framework/status_bar_service.hpp>
#include <gui/framework/menu_service.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/gl/gl_widget_base.hpp>

#include <util/checksum.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/app_status_bar.hpp>
#include <gui/widgets/feedback/feedback.hpp>

#include "pager_message_dlg.hpp"


#include <wx/menu.h>

// For OpenGL context creation via CGLPixelFormatObj 
// (our opengl header only includes gl.h)
#ifdef __WXMAC__
#include <OpenGL/OpenGL.h>
#endif

BEGIN_NCBI_SCOPE

BEGIN_EVENT_TABLE( CWorkbenchFrame, CWorkbenchFrame::TParent )
    EVT_CLOSE( CWorkbenchFrame::OnCloseWindow )
    EVT_KEY_DOWN(CWorkbenchFrame::OnKeyDown)
    EVT_KEY_UP(CWorkbenchFrame::OnKeyUp)
    EVT_MOVE(CWorkbenchFrame::OnMove)
    EVT_IDLE(CWorkbenchFrame::OnIdle)
    EVT_ACTIVATE(CWorkbenchFrame::OnActivate)
END_EVENT_TABLE()


CWorkbenchFrame::CWorkbenchFrame()
{
}


CWorkbenchFrame::CWorkbenchFrame( wxWindow* parent, wxWindowID id, const wxString& caption,
                           const wxPoint& pos, const wxSize& size, long style )
:   TParent(parent, id, caption, pos, size, style),
    m_Workbench(NULL),
    m_DockManager(NULL),
    m_ForwardKeyEvents(false),
    m_IsMoving(false)
{
#ifdef __WXMAC__
    // Some mac laptops come with both a discrete graphics card and graphics on the (intel) cpu.
    // These can switch between cards dynamically to save power, but that causes problems at least
    // in the current revision of the os. JIRA: GB-1636 for more details. 
    // Create and leak an opengl context to force cpu into discrete graphics mode from the start. 
    static CGLPixelFormatObj format = NULL;
    if (!format) {
        CGLPixelFormatAttribute attribs[1];
        attribs[0] = static_cast<CGLPixelFormatAttribute>(0);
        GLint num_pixel_formats = 0;
        CGLChoosePixelFormat(attribs, &format, &num_pixel_formats);
        _TRACE("OpenGL context created!");
    }
    EnableFullScreenView(false);
#endif
}

CWorkbenchFrame::~CWorkbenchFrame()
{
}


void CWorkbenchFrame::SetWorkbench(IWorkbench* workbench)
{
    m_Workbench = workbench;
}


void CWorkbenchFrame::ForwardKeyEventsTo(CDockManager* manager)
{    
    // There is only 1 dockmanager, but it is set here for key events
    // and elsewhere for updates while moving this window
    if (manager != NULL) {
        m_DockManager = manager;
        m_ForwardKeyEvents = true;
    }
    else {
        m_ForwardKeyEvents = false;
    }
}

bool CWorkbenchFrame::ProcessEvent(wxEvent& event)
{
    // Redirect events to active child first. Stops the same event being processed repeatedly
    static wxEventType inEvent = wxEVT_NULL;
    if (inEvent == event.GetEventType())
        return false;
    inEvent = event.GetEventType();

    bool res = false;

    if(m_Workbench  &&  event.IsCommandEvent())  {
        wxEventType type = event.GetEventType();

        // Sometimes accelerators do not seem to get propogated in mac - this 
        // propogates the selectall (GB-2626)
        if (event.GetId() == wxID_SELECTALL) {
            wxWindow* focused = wxWindow::FindFocus();
            if (focused != NULL) {
                res = focused->GetEventHandler()->ProcessEvent(event);
            }
        }

        if (!res && (type == wxEVT_UPDATE_UI  ||  type == wxEVT_COMMAND_MENU_SELECTED))    {
            // forward commands to Workbench

            wxEvtHandler* handler = dynamic_cast<wxEvtHandler*>(m_Workbench);
            if(handler) {
                res = handler->ProcessEvent(event);
            }
        }
    }

    if( ! res) {
        res = TParent::ProcessEvent(event);
    }

    inEvent = wxEVT_NULL;
    return res;
}


void CWorkbenchFrame::OnCloseWindow(wxCloseEvent& event)
{
    if(m_Workbench) {
        if(event.CanVeto()) {
            /// see whether the application can be closed now
            bool can = m_Workbench->CanShutDown();
            if( ! can) {
                event.Veto();
                return;
            }
        }
        // otherwise - ask Workbench to shutdown
        // it will eventually destroy the window, but we MUST NOT do it right now
        m_Workbench->ShutDown();
    }
    SetEvtHandlerEnabled(false);
}


// during D&D focus changes  unpredictably and often is reflected to the
// main frame, this is why we need to forward events back to Dock Manager
void CWorkbenchFrame::OnKeyDown(wxKeyEvent& event)
{
    if(m_ForwardKeyEvents)   {
        m_DockManager->OnKeyDown(event);
    } else {
        event.Skip();
    }
}


void CWorkbenchFrame::OnKeyUp(wxKeyEvent& event)
{
    if(m_ForwardKeyEvents)   {
        m_DockManager->OnKeyUp(event);
    } else {
        event.Skip();
    }
}

void CWorkbenchFrame::OnMove(wxMoveEvent& event)
{
    BroadcastCommandToChildWindows(this, eCmdParentMove);   

    if (!m_IsMoving) {
        m_IsMoving = true;

// This is to keep tooltip z order.  Tips when they get focus or are
// brought to the front (as during a move) they can get in front of
// other wnidows.  Not needed on MAC because it never lets the 
// tips in front of other windows in the first place.
#ifndef NCBI_OS_DARWIN
        if (m_DockManager)
            m_DockManager->RaiseFloatingInZOrder();
#endif
    }
}

void CWorkbenchFrame::OnIdle(wxIdleEvent& event)
{
    if (m_IsMoving)   {
#if(wxMAJOR_VERSION==2 && wxMINOR_VERSION<9)
        if ( !wxGetMouseState().LeftDown() ) {
#else
        if ( !wxGetMouseState().LeftIsDown() ) {
#endif
            m_IsMoving = false;
#ifndef NCBI_OS_DARWIN
            SetFocus();
#endif
        }
    }
#ifdef __WXMAC__
    
    // Make sure the window title bar doesn't go out of bounds - above
    // the menubar at the top of the window.  This can happen when
    // resizing after maximizing the window (especially after maximizing,
    // closing the program, restarting, and then maximizing again)
    // We put this in the idle event because Cocoa doesn't 
    // generate move events for these actions.
    // (Note that if the user has multiple monitors and the monitor
    // with the menubar is BELOW the other monitor, you will not
    // be able to put the app in the upper monitor)
    int mbar_height, maxy, maxx;
    GetMacDragWindowBounds(this, mbar_height, maxy, maxx);
    wxPoint pos = this->GetPosition();
    if (pos.y < mbar_height) {
        pos.y = mbar_height;
        this->SetPosition(pos);
    }
#endif

    if (m_ShowPagerMessage && IsActive())
        x_ShowPagerMessage();
}

void CWorkbenchFrame::OnActivate(wxActivateEvent& event)
{
    BroadcastCommandToChildWindows(this, eCmdParentActivate);
    event.Skip();
}

namespace
{
    class CPagerSlot : public wxPanel
    {
        DECLARE_EVENT_TABLE()
    public:
        CPagerSlot() {}
        bool Create(CWorkbenchFrame *parent);

        void OnPaint(wxPaintEvent& event);
        void OnMouseDown(wxMouseEvent& event);
        void OnMouseEnter(wxMouseEvent& event);
        void OnMouseLeave(wxMouseEvent& event);

    private:
        CWorkbenchFrame* m_Frame = nullptr;
        wxSizerItem* m_IconItem = nullptr;
        wxIcon m_Icon;
        bool m_Hot = false; // mouse inside
    };

    BEGIN_EVENT_TABLE(CPagerSlot, wxPanel)
        EVT_LEFT_DOWN(CPagerSlot::OnMouseDown)
        EVT_ENTER_WINDOW(CPagerSlot::OnMouseEnter)
        EVT_LEAVE_WINDOW(CPagerSlot::OnMouseLeave)
        EVT_PAINT(CPagerSlot::OnPaint)
    END_EVENT_TABLE()

    bool CPagerSlot::Create(CWorkbenchFrame *parent)
    {
        m_Frame = parent;

        long style = wxBORDER_NONE;

        if (!wxPanel::Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, style))
            return false;

        Hide();

        wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

        m_Icon = wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER, wxSize(16, 16));
        m_IconItem = sizer->Add(16, 16, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT, 5);

        SetSizer(sizer);
        Fit();

        return true;
    }

    void CPagerSlot::OnPaint(wxPaintEvent& event)
    {
        wxPaintDC dc(this);

        if (m_Hot) {
            wxBrush brush(CStatusBar::GetBackColor(true));
            dc.SetBrush(brush);
            dc.SetPen(*wxTRANSPARENT_PEN);
            wxRect rc = GetClientRect();
            dc.DrawRectangle(rc);
        }

        wxRect rc = m_IconItem->GetRect();
        dc.DrawIcon(m_Icon, rc.GetLeftTop());
    }

    void CPagerSlot::OnMouseDown(wxMouseEvent& /*event*/)
    {
        m_Frame->SetShowPagerMessage();
    }

    void CPagerSlot::OnMouseEnter(wxMouseEvent& event)
    {
        m_Hot = true;
        Refresh();
    }

    void CPagerSlot::OnMouseLeave(wxMouseEvent& event)
    {
        m_Hot = false;
        Refresh();
    }
}

static const wxChar* kPagerFile = wxT("<home>/pager_message");

void CWorkbenchFrame::SetPagerMessage(const string& message)
{
    m_PagerMessage = message;
    m_ShowPagerMessage = false;

    if (m_PagerMessage.empty())
        return;

    IMenuService* menu_srv = m_Workbench->GetMenuService();
    if (menu_srv)
        menu_srv->ResetMenuBar();

    IStatusBarService* sb_srv = m_Workbench->GetStatusBarService();
    if (sb_srv) {
        CPagerSlot* slot = new CPagerSlot();
        if (slot->Create(this)) {

#ifdef NCBI_OS_MSWIN  // account for divider
            int width = slot->GetSize().x;
#else // account for divider
            int width = slot->GetSize().x + 5;
#endif
            sb_srv->InsertSlot(1, slot, width);
            slot->Show();
            slot->SetToolTip(wxT("View Genome Workbench message"));
        }
        else
            delete slot;
    }

    m_ShowPagerMessage = true;

    wxString md5file = CSysPath::ResolvePath(kPagerFile);
    if (::wxFileExists(md5file)) {
        CChecksum cs(CChecksum::eMD5);
        cs.AddLine(message);
        string md5saved, md5 = cs.GetHexSum();

        try {
            CNcbiIfstream ifs(md5file.fn_str());
            ifs >> md5saved;
        } NCBI_CATCH("Reading pager_message md5:");

        m_ShowPagerMessage = (md5 != md5saved);
    }
}

void CWorkbenchFrame::x_ShowPagerMessage()
{
    m_ShowPagerMessage = false;

    if (m_PagerMessage.empty())
        return;

    CChecksum cs(CChecksum::eMD5);
    cs.AddLine(m_PagerMessage);
    string md5 = cs.GetHexSum();

    wxString md5file = CSysPath::ResolvePath(kPagerFile);

    bool doNotShowAgain = false;

    if (::wxFileExists(md5file)) {
        string md5saved;

        try {
            CNcbiIfstream ifs(md5file.fn_str());
            ifs >> md5saved;
        } NCBI_CATCH("Reading pager_message md5:");

        doNotShowAgain = (md5 == md5saved);
    }

    CPagerMessageDlg dlg;
    dlg.SetHTML(m_PagerMessage);
    dlg.SetDoNotShow(doNotShowAgain);
    dlg.Create(this);
    dlg.ShowModal();
    if (dlg.GetDoNotShow()) {
        try {
            CNcbiOfstream ofs(md5file.fn_str(), ios_base::out | ios_base::trunc);
            ofs << md5;
        } NCBI_CATCH("Writing pager_message md5:");
    }
    else
        ::wxRemoveFile(md5file);

    if (dlg.GetOpenFeedback())
        ShowFeedbackDialog(false);
}

END_NCBI_SCOPE
