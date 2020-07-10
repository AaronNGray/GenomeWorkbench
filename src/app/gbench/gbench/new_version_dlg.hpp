#ifndef GUI_APP_GBENCH_NEW___NEW_VERSION_DLG__HPP
#define GUI_APP_GBENCH_NEW___NEW_VERSION_DLG__HPP

/*  $Id: new_version_dlg.hpp 34700 2016-02-02 11:37:31Z evgeniev $
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
#include "wx/valgen.h"
#include "wx/hyperlink.h"
#include "wx/statline.h"
////@end includes

#include <wx/dialog.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxHyperlinkCtrl;
////@end forward declarations

class wxStaticBitmap;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CNEWVERSIONDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CNEWVERSIONDLG_TITLE _("New version notification")
#define SYMBOL_CNEWVERSIONDLG_IDNAME ID_CNEWVERSIONDLG
#define SYMBOL_CNEWVERSIONDLG_SIZE wxSize(400, 300)
#define SYMBOL_CNEWVERSIONDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CNewVersionDlg class declaration
 */

class CNewVersionDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CNewVersionDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CNewVersionDlg();
    CNewVersionDlg( wxWindow* parent, wxWindowID id = SYMBOL_CNEWVERSIONDLG_IDNAME, const wxString& caption = SYMBOL_CNEWVERSIONDLG_TITLE, const wxPoint& pos = SYMBOL_CNEWVERSIONDLG_POSITION, const wxSize& size = SYMBOL_CNEWVERSIONDLG_SIZE, long style = SYMBOL_CNEWVERSIONDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CNEWVERSIONDLG_IDNAME, const wxString& caption = SYMBOL_CNEWVERSIONDLG_TITLE, const wxPoint& pos = SYMBOL_CNEWVERSIONDLG_POSITION, const wxSize& size = SYMBOL_CNEWVERSIONDLG_SIZE, long style = SYMBOL_CNEWVERSIONDLG_STYLE );

    /// Destructor
    ~CNewVersionDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CNewVersionDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_APPLY
    void OnApplyClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_POSTPONE_BTN
    void OnPostponeBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_IGNORE
    void OnIgnoreClick( wxCommandEvent& event );

////@end CNewVersionDlg event handler declarations

////@begin CNewVersionDlg member function declarations

    wxString GetCurrentVersion() const { return m_CurrentVersion ; }
    void SetCurrentVersion(wxString value) { m_CurrentVersion = value ; }

    wxString GetDownloadURL() const { return m_DownloadURL ; }
    void SetDownloadURL(wxString value) { m_DownloadURL = value ; }

    bool GetInstallationEnabled() const { return m_InstallationEnabled ; }
    void SetInstallationEnabled(bool value) { m_InstallationEnabled = value ; }

    wxString GetLatestVersion() const { return m_LatestVersion ; }
    void SetLatestVersion(wxString value) { m_LatestVersion = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CNewVersionDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CNewVersionDlg member variables
    wxStaticBitmap* m_Image;
    wxHyperlinkCtrl* m_Hyperlink;
    wxHyperlinkCtrl* m_RNotesHyperlink;
    wxButton* m_InstallBtn;
private:
    wxString m_CurrentVersion;
    wxString m_DownloadURL;
    bool m_InstallationEnabled;
    wxString m_LatestVersion;
    /// Control identifiers
    enum {
        ID_CNEWVERSIONDLG = 10032,
        ID_HYPERLINKCTRL = 10033,
        ID_RNOTES_HYPERLINKCTRL = 10000,
        ID_POSTPONE_BTN = 10039
    };
////@end CNewVersionDlg member variables
};

END_NCBI_SCOPE

#endif  // GUI_APP_GBENCH_NEW___NEW_VERSION_DLG__HPP
