/*  $Id: wx_main_frame.hpp 21484 2010-06-08 00:18:11Z voronov $
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

// Generated by DialogBlocks (unregistered), 23/04/2007 14:32:00

#ifndef _WX_MAIN_FRAME_H_
#define _WX_MAIN_FRAME_H_

/*!
 * Includes
 */

////@begin includes
#include "wx/frame.h"
#include "wx/statusbr.h"
////@end includes


#include <gui/wx_demo/test_client.hpp>
#include <gui/widgets/seq/sequence_goto_dlg.hpp>
#include <gui/widgets/wx/table_listctrl.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

//TODO this is temporary
#include <gui/utils/event_handler.hpp>

class wxNotebook;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CWXMAINFRAME 10000
#define ID_TEST_MSG_BOXES 10007
#define ID_POST_EVENTS 10013
#define ID_MENUITEM1 10027
#define ID_TEST_DIAG_PANEL 10038
#define ID_CREATE_MAP_CONTROL 10002
#define ID_CREATE_GROUP_MAP_WIDGET 10001
#define ID_CREATE_TABLE_CONTROL 10003
#define ID_STATUSBAR1 10004
#define SYMBOL_CWXMAINFRAME_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxMINIMIZE_BOX|wxMAXIMIZE_BOX|wxCLOSE_BOX
#define SYMBOL_CWXMAINFRAME_TITLE _("wx Demo App")
#define SYMBOL_CWXMAINFRAME_IDNAME ID_CWXMAINFRAME
#define SYMBOL_CWXMAINFRAME_SIZE wxSize(800, 600)
#define SYMBOL_CWXMAINFRAME_POSITION wxDefaultPosition
////@end control identifiers

#define ID_MENU_OPEN 11000
#define ID_OPEN_DATA 11001
#define ID_OPEN_PROJECT 11002
#define ID_OPEN_WORKSPACE 11003

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

class CMapControl;
class CGroupMapWidget;

/*!
 * CwxMainFrame class declaration
 */

class CwxMainFrame:     public wxFrame,
                        public CEventHandler
{
    DECLARE_CLASS( CwxMainFrame )
    DECLARE_EVENT_TABLE()

    DECLARE_EVENT_MAP(); //TODO

    typedef wxFrame TParent;

public:
    /// Constructors
    CwxMainFrame();
    CwxMainFrame(wxWindow* parent,
                 wxWindowID id = SYMBOL_CWXMAINFRAME_IDNAME,
                 const wxString& caption = SYMBOL_CWXMAINFRAME_TITLE,
                 const wxPoint& pos = SYMBOL_CWXMAINFRAME_POSITION,
                 const wxSize& size = SYMBOL_CWXMAINFRAME_SIZE,
                 long style = SYMBOL_CWXMAINFRAME_STYLE );

    bool Create(wxWindow* parent,
                wxWindowID id = SYMBOL_CWXMAINFRAME_IDNAME,
                const wxString& caption = SYMBOL_CWXMAINFRAME_TITLE,
                const wxPoint& pos = SYMBOL_CWXMAINFRAME_POSITION,
                const wxSize& size = SYMBOL_CWXMAINFRAME_SIZE,
                long style = SYMBOL_CWXMAINFRAME_STYLE );

    /// Destructor
    ~CwxMainFrame();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CwxMainFrame event handler declarations

    /// wxEVT_CLOSE_WINDOW event handler for ID_CWXMAINFRAME
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EXIT
    void OnEXITClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_TEST_MSG_BOXES
    void OnTestMsgBoxesClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_POST_EVENTS
    void OnPostEventsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM1
    void OnGoToSequence( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_TEST_DIAG_PANEL
    void OnTestDiagPanelClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_TEST_DIAG_PANEL
    void OnTestDiagPanelUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_CREATE_MAP_CONTROL
    void OnCreateMapControlClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_CREATE_GROUP_MAP_WIDGET
    void OnCreateGroupMapWidgetClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_CREATE_TABLE_CONTROL
    void OnCreateTableControlClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_ABOUT
    void OnABOUTClick( wxCommandEvent& event );

////@end CwxMainFrame event handler declarations

////@begin CwxMainFrame member function declarations

    int GetCreateMode() const { return m_CreateMode ; }
    void SetCreateMode(int value) { m_CreateMode = value ; }

    bool GetDiagnosticsTesting() const { return m_DiagnosticsTesting ; }
    void SetDiagnosticsTesting(bool value) { m_DiagnosticsTesting = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CwxMainFrame member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void    OnTestEvent(CEvent* evt);

    void    OnLaunchJobs( wxCommandEvent& event );
    void    OnCancelJobs( wxCommandEvent& event );
    void    OnCheckJobsStatus( wxCommandEvent& event );
    void    OnCheckJobsProgress( wxCommandEvent& event );

    void    OnSeqGoto( CSequenceGotoEvent& event );
    void    OnSeqGotoClose( CSequenceGotoEvent& event );

    void    OnCreateMapControlItems();
    void    OnCreateTableControlItems();

protected:
    void    x_SetupStatusBar();
    void    x_RegisterAliases();
    void    x_CreateNextItem();

 protected:
    auto_ptr<CPrimeNumberClient> m_PrimeClient;

    CSequenceGotoData m_SequenceGotoData;
    CSequenceGotoDlg* m_SequenceGotoDlg;

////@begin CwxMainFrame member variables
private:
    bool m_DiagnosticsTesting;
protected:
    int m_CreateMode;
    CGroupMapWidget* m_GroupMapWidget;
    CMapControl* m_MapControl;
    CwxTableListCtrl* m_TableControl;
////@end CwxMainFrame member variables

    wxNotebook* m_Notebook;

    int m_ItemsCounter;
};

END_NCBI_SCOPE

#endif
    // _WX_MAIN_FRAME_H_
