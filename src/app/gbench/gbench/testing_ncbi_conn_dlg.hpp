#ifndef GUI_CORE___TESTING_NCBI_CONN_DLG__HPP
#define GUI_CORE___TESTING_NCBI_CONN_DLG__HPP

/*  $Id: testing_ncbi_conn_dlg.hpp 23228 2011-02-23 16:16:41Z katargir $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Roman Katargin
 */

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

#include <wx/dialog.h>
#include <wx/timer.h>

/*!
 * Control identifiers
 */

BEGIN_NCBI_SCOPE

class CConnTestThread;

////@begin control identifiers
#define ID_CTESTINGNCBICONNDLG 10028
#define ID_TEXTCTRL2 10029
#define ID_BUTTON2 10030
#define SYMBOL_CTESTINGNCBICONNDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxTAB_TRAVERSAL
#define SYMBOL_CTESTINGNCBICONNDLG_TITLE _("Testing NCBI Connection")
#define SYMBOL_CTESTINGNCBICONNDLG_IDNAME ID_CTESTINGNCBICONNDLG
#define SYMBOL_CTESTINGNCBICONNDLG_SIZE wxSize(400, 300)
#define SYMBOL_CTESTINGNCBICONNDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CTestingNcbiConnDlg class declaration
 */

class CTestingNcbiConnDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CTestingNcbiConnDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CTestingNcbiConnDlg();
    CTestingNcbiConnDlg( wxWindow* parent, wxWindowID id = SYMBOL_CTESTINGNCBICONNDLG_IDNAME, const wxString& caption = SYMBOL_CTESTINGNCBICONNDLG_TITLE, const wxPoint& pos = SYMBOL_CTESTINGNCBICONNDLG_POSITION, const wxSize& size = SYMBOL_CTESTINGNCBICONNDLG_SIZE, long style = SYMBOL_CTESTINGNCBICONNDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTESTINGNCBICONNDLG_IDNAME, const wxString& caption = SYMBOL_CTESTINGNCBICONNDLG_TITLE, const wxPoint& pos = SYMBOL_CTESTINGNCBICONNDLG_POSITION, const wxSize& size = SYMBOL_CTESTINGNCBICONNDLG_SIZE, long style = SYMBOL_CTESTINGNCBICONNDLG_STYLE );

    void SetConnThread(CConnTestThread* connThread) { m_ConnThread = connThread; }

    /// Destructor
    ~CTestingNcbiConnDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CTestingNcbiConnDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
    void OnCancelClick( wxCommandEvent& event );

////@end CTestingNcbiConnDlg event handler declarations

    void OnTimer(wxTimerEvent& event);

////@begin CTestingNcbiConnDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTestingNcbiConnDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CTestingNcbiConnDlg member variables
////@end CTestingNcbiConnDlg member variables
private:
    CConnTestThread* m_ConnThread;
    wxTimer m_Timer;
};

END_NCBI_SCOPE

#endif  // GUI_CORE___TESTING_NCBI_CONN_DLG__HPP
