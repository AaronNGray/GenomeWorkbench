#ifndef GUI_FRAMEWORK___WORKBENCH_MAIN_FRAME__HPP
#define GUI_FRAMEWORK___WORKBENCH_MAIN_FRAME__HPP

/*  $Id: workbench_frame.hpp 43956 2019-09-25 21:02:14Z katargir $
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
 * Authors: Andrey Yazhuk
 *
 * File Description:
 *   CWorkbenchFrame - main application frame for Workbench-based applications.
 */

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/widgets/wx/main_frame.hpp>
#include <gui/widgets/wx/dock_manager.hpp>


BEGIN_NCBI_SCOPE

class IWorkbench;

///////////////////////////////////////////////////////////////////////////////
/// CWorkbenchFrame
/// Main Application Frame for Workbench-based applications.

class NCBI_GUIFRAMEWORK_EXPORT  CWorkbenchFrame :
    public CMainFrame,
    public IDockManagerKeyHook
{
    DECLARE_EVENT_TABLE()

public:
    typedef CMainFrame TParent;

    /// Constructors
    CWorkbenchFrame();
    CWorkbenchFrame(wxWindow* parent, wxWindowID id, const wxString& caption,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  long style = wxDEFAULT_FRAME_STYLE );
    ~CWorkbenchFrame();

    virtual void    SetWorkbench(IWorkbench* workbench);
    virtual void    SetDockManager(CDockManager* c) { m_DockManager = c; }

    /// IDockManagerKeyHook implementation
    virtual void    ForwardKeyEventsTo(CDockManager* manager);

    bool    ProcessEvent(wxEvent& event);

    /// handle request to close window
    void    OnCloseWindow(wxCloseEvent& event);
    void    OnActivate(wxActivateEvent& event);

    void    OnKeyDown(wxKeyEvent& event);
    void    OnKeyUp(wxKeyEvent& event);
    void    OnMove(wxMoveEvent& event);
    void    OnIdle(wxIdleEvent& event);

    void    SetPagerMessage(const string& message);
    void    SetShowPagerMessage() { m_ShowPagerMessage = true; }
    bool    HasPagerMessage() const { return !m_PagerMessage.empty(); }

protected:
    void    x_ShowPagerMessage();

    IWorkbench*   m_Workbench;
    CDockManager* m_DockManager;

    bool m_ForwardKeyEvents;
    bool m_IsMoving;

    string m_PagerMessage;
    bool   m_ShowPagerMessage = false;
};


END_NCBI_SCOPE

/* @} */

#endif // GUI_FRAMEWORK___WORKBENCH_MAIN_FRAME__HPP
