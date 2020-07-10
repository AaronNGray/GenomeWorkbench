#ifndef GUI_WIDGETS_EDIT___JOURNAL_PANEL__HPP
#define GUI_WIDGETS_EDIT___JOURNAL_PANEL__HPP

/*  $Id: journal_panel.hpp 44555 2020-01-21 19:24:06Z filippov $
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

#include <objmgr/scope.hpp>
#include <objects/seq/Pubdesc.hpp>

#include <wx/panel.h>
#include <wx/choice.h>

#include <objects/biblio/Title.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <gui/widgets/edit/flexibledate_panel.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes


/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

class wxTextCtrl;

BEGIN_NCBI_SCOPE

class CSerialObject;

/*!
 * Control identifiers
 */


////@begin control identifiers
#define SYMBOL_CJOURNALPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CJOURNALPANEL_TITLE _("Journal")
#define SYMBOL_CJOURNALPANEL_IDNAME ID_JOURNAL
#define SYMBOL_CJOURNALPANEL_SIZE wxDefaultSize
#define SYMBOL_CJOURNALPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CJournalPanel class declaration
 */

class CJournalPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CJournalPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CJournalPanel();
    CJournalPanel( wxWindow* parent, CSerialObject& object, 
        long *pmid, long *muid, long *serial_number,
        wxWindowID id = SYMBOL_CJOURNALPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CJOURNALPANEL_POSITION, 
        const wxSize& size = SYMBOL_CJOURNALPANEL_SIZE, 
        long style = SYMBOL_CJOURNALPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CJOURNALPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CJOURNALPANEL_POSITION, 
        const wxSize& size = SYMBOL_CJOURNALPANEL_SIZE, 
        long style = SYMBOL_CJOURNALPANEL_STYLE );

    /// Destructor
    ~CJournalPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    bool TransferDataToWindow();
    bool TransferDataFromWindow();
    CRef<objects::CCit_jour> GetCit_jour() const;

////@begin CJournalPanel event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_JOURNAL_PMID
    void OnJournalPmidTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_JOURNAL_ERRATUM
    void OnJournalErratumSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_JOURNAL_LOOKUP_RELAXED
    void OnJournalLookupRelaxedClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_JOURNAL_LOOKUP_ISOJTA
    void OnJournalLookupIsojtaClick( wxCommandEvent& event );

    void OnArticleLookup( wxCommandEvent& event );

////@end CJournalPanel event handler declarations

////@begin CJournalPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CJournalPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CJournalPanel member variables
    wxTextCtrl* m_JournalTitle;
    wxTextCtrl* m_Volume;
    wxTextCtrl* m_Issue;
    wxTextCtrl* m_Pages;
    wxTextCtrl* m_PmidCtrl;
    wxTextCtrl* m_MuidCtrl;
    wxTextCtrl* m_SerialNoCtrl;
    wxChoice* m_ErratumCtrl;
    wxTextCtrl* m_ErratumExplanation;
    wxChoice* m_PubstatusCtrl;
    wxBoxSizer* m_PubDateSizer;
    /// Control identifiers
    enum {
        ID_JOURNAL = 10034,
        ID_JOURNAL_TITLE,
        ID_JOURNAL_VOLUME,
        ID_JOURNAL_ISSUE,
        ID_JOURNAL_PAGES,
        ID_JOURNAL_PMID,
        ID_JOURNAL_MUID,
        ID_JOURNAL_SERIALNO,
        ID_JOURNAL_ERRATUM,
        ID_JOURNAL_EXPLANATION,
        ID_JOURNAL_PUBSTATUS,
        ID_JOURNAL_LOOKUP_RELAXED,
        ID_JOURNAL_LOOKUP_ISOJTA,
        ID_PMID_LOOKUP,
        ID_ARTICLE_LOOKUP
    };
////@end CJournalPanel member variables
    CFlexibleDatePanel* m_PubDatePanel;

    void SetFinalValidation(bool val);

private:
    CSerialObject* m_Object;
    CRef<CSerialObject> m_EditedJournal;
    long* m_pPmid;
    long* m_pMuid;
    long* m_pSerialNumber;
    bool m_IsIsoJtaTitle;
   
    bool x_HasIsoJtaTitle(const objects::CTitle& title);
    void x_UpdateJournalTitle();
    void LookupIsjta(string old_title, vector<string> &titles);
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___JOURNAL_PANEL__HPP
