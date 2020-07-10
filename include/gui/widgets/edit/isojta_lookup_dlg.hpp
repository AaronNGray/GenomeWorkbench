#ifndef GUI_WIDGETS_EDIT___ISOJTA_LOOKUP_DLG__HPP
#define GUI_WIDGETS_EDIT___ISOJTA_LOOKUP_DLG__HPP

/*  $Id: isojta_lookup_dlg.hpp 34071 2015-10-29 16:46:32Z filippov $
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
 * Authors:  Colleen Bollin
 */

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/dialog.h>
#include <wx/choice.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CISOJTALOOKUPDLG 10031
#define ID_CHOICE1 10087
#define ID_ISO_JTA_REPORT_BTN 10093
#define SYMBOL_CISOJTALOOKUPDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CISOJTALOOKUPDLG_TITLE _("ISO JTA Lookup")
#define SYMBOL_CISOJTALOOKUPDLG_IDNAME ID_CISOJTALOOKUPDLG
#define SYMBOL_CISOJTALOOKUPDLG_SIZE wxSize(400, 300)
#define SYMBOL_CISOJTALOOKUPDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CISOJTALookupDlg class declaration
 */

class CISOJTALookupDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CISOJTALookupDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CISOJTALookupDlg();
    CISOJTALookupDlg( wxWindow* parent, const vector<string> &journals, wxWindowID id = SYMBOL_CISOJTALOOKUPDLG_IDNAME, const wxString& caption = SYMBOL_CISOJTALOOKUPDLG_TITLE, const wxPoint& pos = SYMBOL_CISOJTALOOKUPDLG_POSITION, const wxSize& size = SYMBOL_CISOJTALOOKUPDLG_SIZE, long style = SYMBOL_CISOJTALOOKUPDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CISOJTALOOKUPDLG_IDNAME, const wxString& caption = SYMBOL_CISOJTALOOKUPDLG_TITLE, const wxPoint& pos = SYMBOL_CISOJTALOOKUPDLG_POSITION, const wxSize& size = SYMBOL_CISOJTALOOKUPDLG_SIZE, long style = SYMBOL_CISOJTALOOKUPDLG_STYLE );

    /// Destructor
    ~CISOJTALookupDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CISOJTALookupDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOkClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
    void OnCancelClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ISO_JTA_REPORT_BTN
    void OnIsoJtaReportBtnClick( wxCommandEvent& event );

////@end CISOJTALookupDlg event handler declarations

////@begin CISOJTALookupDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CISOJTALookupDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CISOJTALookupDlg member variables
    wxChoice* m_JournalChoice;
////@end CISOJTALookupDlg member variables

    void SetJournalList(const vector<string> &journals);
    string GetISOJTA();
private:
    vector<string> m_titles;
};

END_NCBI_SCOPE

#endif
    // _ISOJTA_LOOKUP_DLG__HPP
