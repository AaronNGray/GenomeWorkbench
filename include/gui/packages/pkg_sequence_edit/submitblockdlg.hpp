/*  $Id: submitblockdlg.hpp 27516 2013-02-28 12:38:52Z bollin $
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
#ifndef _SUBMITBLOCKDLG_H_
#define _SUBMITBLOCKDLG_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objects/submit/Submit_block.hpp>
#include <objects/biblio/Cit_gen.hpp>
#include <gui/widgets/edit/submitblockpanel.hpp>
#include <gui/widgets/edit/release_date_panel.hpp>
#include <gui/widgets/edit/contactpanel.hpp>
#include <gui/widgets/edit/authornames_panel.hpp>
#include <gui/widgets/edit/author_affiliation_panel.hpp>
#include <gui/widgets/edit/unpublished_panel.hpp>
#include <gui/widgets/edit/flexibledate_panel.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/framework/workbench.hpp>

#include <wx/dialog.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/button.h>

/*!
 * Includes
 */

////@begin includes
#include "wx/notebook.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
class wxNotebook;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_SUBMITBLOCKDLG 10015
#define ID_SUBMITBLOCKDLGLISTBOOK 10016
#define ID_CLEAR_BTN 10107
#define ID_BUTTON15 10066
#define ID_BUTTON14 10065
#define ID_IMPORT_BTN 10099
#define ID_EXPORT_BTN 10100
#define SYMBOL_CSUBMITBLOCKDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSUBMITBLOCKDLG_TITLE _("Submission Information")
#define SYMBOL_CSUBMITBLOCKDLG_IDNAME ID_SUBMITBLOCKDLG
#define SYMBOL_CSUBMITBLOCKDLG_SIZE wxSize(400, 300)
#define SYMBOL_CSUBMITBLOCKDLG_POSITION wxDefaultPosition
////@end control identifiers

#define ID_SUBMITBLOCKDLG_SAME_AUTHORS 10017
#define ID_SUBMITBLOCKDLG_SAME_AFFILS 10018
#define ID_COPY_SEQ_AUTH_TO_MAN_AUTH 10019

/*!
 * CSubmitBlockDlg class declaration
 */

class CSubmitBlockDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CSubmitBlockDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSubmitBlockDlg();
    CSubmitBlockDlg( wxWindow* parent, CRef<objects::CSubmit_block> block, CRef<objects::CCit_gen> gen, 
                     IWorkbench* workbench,
                     wxWindowID id = SYMBOL_CSUBMITBLOCKDLG_IDNAME, const wxString& caption = SYMBOL_CSUBMITBLOCKDLG_TITLE, const wxPoint& pos = SYMBOL_CSUBMITBLOCKDLG_POSITION, const wxSize& size = SYMBOL_CSUBMITBLOCKDLG_SIZE, long style = SYMBOL_CSUBMITBLOCKDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSUBMITBLOCKDLG_IDNAME, const wxString& caption = SYMBOL_CSUBMITBLOCKDLG_TITLE, const wxPoint& pos = SYMBOL_CSUBMITBLOCKDLG_POSITION, const wxSize& size = SYMBOL_CSUBMITBLOCKDLG_SIZE, long style = SYMBOL_CSUBMITBLOCKDLG_STYLE );

    /// Destructor
    ~CSubmitBlockDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSubmitBlockDlg event handler declarations

    /// wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED event handler for ID_SUBMITBLOCKDLGLISTBOOK
    void OnSubmitblockdlgPageChanged( wxNotebookEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CLEAR_BTN
    void OnClearBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON15
    void OnBackBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON14
    void OnNextBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_IMPORT_BTN
    void OnImportBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EXPORT_BTN
    void OnExportBtnClick( wxCommandEvent& event );

////@end CSubmitBlockDlg event handler declarations
    void OnSeparateAuthorsClick( wxCommandEvent& event );
    void OnCopySeqAuthToManAuthClick( wxCommandEvent& event );

////@begin CSubmitBlockDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSubmitBlockDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSubmitBlockDlg member variables
    wxBoxSizer* m_Updatable;
    wxNotebook* m_Book;
    wxBoxSizer* m_SameAuthSizer;
    wxButton* m_BackBtn;
    wxButton* m_NextBtn;
////@end CSubmitBlockDlg member variable
    
    void SetAlternateEmailAddress(string alt_email);

    CRef<objects::CSubmit_block> GetSubmitBlock();
    CRef<objects::CCit_gen> GetCitGen();
    string GetAlternateEmailAddress();
    string GetErrors();
    static string GetBlockProblems(CRef<objects::CSubmit_block> block);
    static string GetCitGenProblems(CRef<objects::CCit_gen> gen);
    void SetPageForError(string errors);
    void UnselectReleaseDateChoice();

private:
    CRef<objects::CSubmit_block> m_SubmitBlock;
    CRef<objects::CCit_gen> m_Gen;

    CReleaseDatePanel *m_ReleaseDatePanel;
    CAuthorNamesPanel *m_SequenceAuthorsPanel;
    CAuthorAffiliationPanel *m_AffilPanel;
    CContactPanel *m_ContactPanel;
    CAuthorNamesPanel *m_ManAuthorsPanel;
    CUnpublishedPanel *m_UnpublishedPanel;
    wxTextCtrl* m_AlternateEmail;

    wxCheckBox* m_AuthorsSame;
    wxCheckBox* m_AffilsSame;
    wxButton* m_CopySeqAuthToManAuth;

    bool x_CopyAuthors();
    void x_BuildPages();
    CRef<objects::CSubmit_block> x_CreateBlock();
    CRef<objects::CCit_gen> x_CreateCitGen();

    IWorkbench* m_Workbench;
    wxString m_SaveFileDir;
    wxString m_SaveFileName;
};

END_NCBI_SCOPE

#endif
    // _SUBMITBLOCKDLG_H_
