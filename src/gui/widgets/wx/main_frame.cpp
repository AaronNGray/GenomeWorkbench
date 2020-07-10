/*  $Id: main_frame.cpp 39675 2017-10-26 15:13:40Z katargir $
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

#include <gui/widgets/wx/main_frame.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/wnd_layout_registry.hpp>

#include <wx/menu.h>


BEGIN_NCBI_SCOPE


BEGIN_EVENT_TABLE( CMainFrame, CMainFrame::TParent )
    EVT_MENU(wxID_EXIT, CMainFrame::OnExitClick)
#ifdef NCBI_OS_MSWIN
    EVT_ACTIVATE(CMainFrame::OnActivate)
#endif
END_EVENT_TABLE()


CMainFrame::CMainFrame()
{
}


CMainFrame::CMainFrame( wxWindow* parent, wxWindowID id, const wxString& caption,
                           const wxPoint& pos, const wxSize& size, long style )
:   TParent(parent, id, caption, pos, size, style),
    m_HintListener(NULL)
#ifdef NCBI_OS_MSWIN
    , m_ModalCounter(0)
    , m_MenuFocusWnd()
#endif
{
}


CMainFrame::~CMainFrame()
{
}


void CMainFrame::SetHintListener(IHintListener* listener)
{
    m_HintListener = listener;
}


void CMainFrame::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}


static const char* kWinRectTag = "WindowRect";
static const char* kMaxTag = "Maximized";

void CMainFrame::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        // load window rect
        wxRect rc = GetRect();
        CRegistryReadView view =
            CWndLayoutReg::GetInstance().GetReadView(m_RegPath + "." + kWinRectTag);
        LoadWindowRectFromRegistry(rc, view);
        CorrectWindowRect(this, rc);
        SetSize(rc);

        bool max = view.GetBool(kMaxTag, true);
        if(max) {
            Maximize();
        }

    }
}


void CMainFrame::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        CRegistryWriteView view =
            CWndLayoutReg::GetInstance().GetWriteView(m_RegPath + "." + kWinRectTag);
        SaveWindowRectToRegistry(GetRect(), view);

        bool max = IsMaximized();
        view.Set(kMaxTag, max);
    }
}


void CMainFrame::DoGiveHelp(const wxString& text, bool show)
{
    /// instead of accessing Status Bar directly (this may be destructive)
    /// we forward messages to the provided interface
    if(m_HintListener)  {
        if(show)    {
            string s = ToStdString(text);
            m_HintListener->ShowCommandHint(s);
        } else {
            m_HintListener->HideCommandHint();
        }
    }
}


void CMainFrame::OnExitClick( wxCommandEvent& WXUNUSED(event) )
{
    wxFrame::Close(false);
}

#ifdef NCBI_OS_MSWIN

void CMainFrame::OnActivate(wxActivateEvent& event)
{
    if (m_MenuFocusWnd == 0)
        event.Skip();
}

/// To make modal dialogs truly modal we need to disable all top-level windows
/// in the application when a modal dialog box is shown. Enabling all windows
/// will guarantee that they do not get user input.
/// When the system show a modal dialog it send WM_ENABLE (false) event to the
/// main application window, when the dialog is closed WM_ENABLE (true) is sent.
/// We handle these events and enabled / disbale child frames.

WXLRESULT CMainFrame::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
    WXLRESULT   rc;

    if (message == WM_MOUSEACTIVATE && LOWORD(lParam) == HTMENU) {
        x_SaveMenuFocus();
    }

    // for most messages we should return 0 when we do process the message
    rc = TParent::MSWWindowProc(message, wParam, lParam);

    if( message == WM_ENABLE)   {
        _ASSERT(m_ModalCounter >= 0);

        bool enable = (wParam == TRUE);

        //LOG_POST("");
        //LOG_POST("Main frame - handling WM_ENABLE" << enable);

        wxWindowList& children = GetChildren();

        wxWindowList::compatibility_iterator node = children.GetFirst();
        while (node)
        {
            wxWindow *child = node->GetData();
            wxFrame* frame = dynamic_cast<wxFrame*>(child);
            if(frame)   {
                //LOG_POST("Enabling window " << frame->GetHWND() << "  " << enable);
                frame->Enable(enable);
            }

            node = node->GetNext();
        }
    }
    else if (message == WM_EXITMENULOOP) {
        x_RestoreMenuFocus();
    }

    return rc;
}

void CMainFrame::x_SaveMenuFocus()
{
    m_MenuFocusWnd = FindFocus();
    if (m_MenuFocusWnd == 0)
        return;

    if (wxGetTopLevelParent(m_MenuFocusWnd) == this)
        m_MenuFocusWnd = 0;
}

void CMainFrame::x_RestoreMenuFocus()
{
    if (m_MenuFocusWnd) {
        m_MenuFocusWnd->SetFocus();
        m_MenuFocusWnd = 0;
    }
}

#endif

END_NCBI_SCOPE
