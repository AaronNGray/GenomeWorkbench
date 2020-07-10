#ifndef GUI_WIDGETS_EDIT___RELAXED_LOOKUP_DLG__HPP
#define GUI_WIDGETS_EDIT___RELAXED_LOOKUP_DLG__HPP

/*  $Id: relaxed_lookup_dlg.hpp 37916 2017-03-03 13:56:13Z filippov $
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
#include <objects/seq/Pubdesc.hpp>
#include <objects/pub/Pub.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/choice.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CRELAXEDLOOKUPDLG 10092
#define ID_MAX_MATCH_TXT 10094
#define ID_USE_FIRST_AUTHOR_BTN 10095
#define ID_FIRST_AUTHOR_TXT 10096
#define ID_USE_LAST_AUTHOR_BTN 10097
#define ID_LAST_AUTHOR_TXT 10098
#define ID_USE_JOURNAL_BTN 10099
#define ID_JOURNAL_TXT 10100
#define ID_USE_YEAR_BTN 10101
#define ID_YEAR_TXT 10103
#define ID_YEAR_FUZZ_BTN 10102
#define ID_USE_VOLUME_BTN 10104
#define ID_USE_VOLUME_TXT 10105
#define ID_USE_PAGE_BTN 10106
#define ID_PAGE_TXT 10107
#define ID_ARTICLE_TITLE_TXT 10108
#define ID_USE_EXTRA_TERMS_BTN 10109
#define ID_EXTRA_TERMS_TXT 10110
#define ID_NEW_QUERY_CHOICE 10111
#define ID_SEND_QUERY 10112
#define ID_SCROLLEDWINDOW 10113
#define SYMBOL_CRELAXEDLOOKUPDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CRELAXEDLOOKUPDLG_TITLE _("In-press citation update")
#define SYMBOL_CRELAXEDLOOKUPDLG_IDNAME ID_CRELAXEDLOOKUPDLG
#define SYMBOL_CRELAXEDLOOKUPDLG_SIZE wxSize(400, 300)
#define SYMBOL_CRELAXEDLOOKUPDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CRelaxedLookupDlg class declaration
 */

class CRelaxedLookupDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CRelaxedLookupDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRelaxedLookupDlg();
    CRelaxedLookupDlg( wxWindow* parent, wxWindowID id = SYMBOL_CRELAXEDLOOKUPDLG_IDNAME, const wxString& caption = SYMBOL_CRELAXEDLOOKUPDLG_TITLE, const wxPoint& pos = SYMBOL_CRELAXEDLOOKUPDLG_POSITION, const wxSize& size = SYMBOL_CRELAXEDLOOKUPDLG_SIZE, long style = SYMBOL_CRELAXEDLOOKUPDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CRELAXEDLOOKUPDLG_IDNAME, const wxString& caption = SYMBOL_CRELAXEDLOOKUPDLG_TITLE, const wxPoint& pos = SYMBOL_CRELAXEDLOOKUPDLG_POSITION, const wxSize& size = SYMBOL_CRELAXEDLOOKUPDLG_SIZE, long style = SYMBOL_CRELAXEDLOOKUPDLG_STYLE );

    /// Destructor
    ~CRelaxedLookupDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CRelaxedLookupDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEND_QUERY
    void OnSendQueryClick( wxCommandEvent& event );

////@end CRelaxedLookupDlg event handler declarations
    void OnPubTextClick( wxMouseEvent& event );
    void OnPubTextDClick( wxMouseEvent& event );

////@begin CRelaxedLookupDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CRelaxedLookupDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CRelaxedLookupDlg member variables
    wxTextCtrl* m_MaxMatchTxt;
    wxCheckBox* m_UseFirstAuthorBtn;
    wxTextCtrl* m_FirstAuthorTxt;
    wxCheckBox* m_UseLastAuthorBtn;
    wxTextCtrl* m_LastAuthorTxt;
    wxCheckBox* m_UseJournalBtn;
    wxTextCtrl* m_JournalTxt;
    wxCheckBox* m_UseYearBtn;
    wxTextCtrl* m_YearTxt;
    wxCheckBox* m_YearFuzzBtn;
    wxCheckBox* m_UseVolumeBtn;
    wxTextCtrl* m_UseVolumeTxt;
    wxCheckBox* m_UsePageBtn;
    wxTextCtrl* m_UsePageTxt;
    wxTextCtrl* m_ArticleTitleTxt;
    wxCheckBox* m_UseExtraTermsBtn;
    wxTextCtrl* m_ExtraTermsTxt;
    wxChoice* m_NewQueryChoice;
    wxScrolledWindow* m_ScrolledWindow;
    wxBoxSizer* m_Sizer;
////@end CRelaxedLookupDlg member variables
    void SetPubdesc(const objects::CPubdesc& pubdesc);
    void Clear();
    void SendQuery(bool use_hydra = false);
    CRef<objects::CPubdesc> GetPubdesc();
    void SetStrict(void);

protected:
    string x_GetSearchTerms();
    void x_GetPubIds(const string& terms);
    string x_GetSearchTermsHydra();
    void x_GetPubIdsHydra(const string& terms);
    void x_GetPubIdsMla();

    vector<int>     m_Ids;
    list<string>    m_Labels;
    vector<CRef<objects::CPubdesc> > m_Pubs;
    CRef<objects::CPub> m_CitMatch;
    set<string> m_AuthorsExist;
    vector<string> m_Authors;
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___RELAXED_LOOKUP_DLG__HPP
