/*  $Id: subprepintropanel.hpp 27517 2013-02-28 13:25:54Z bollin $
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
#ifndef _SUBPREPINTROPANEL_H_
#define _SUBPREPINTROPANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/biblio/Cit_gen.hpp>

/*!
 * Includes
 */
#include <gui/packages/pkg_sequence_edit/subprep_panel.hpp>

////@begin includes
#include "wx/hyperlink.h"
////@end includes
#include <wx/hyperlink.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/listbox.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

class CSubPrep_panel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CSUBPREPINTROPANEL 10090
#define ID_HYPERLINKCTRL5 10000
#define ID_SUBMISSION_TYPE 10003
#define ID_INTRO_TEXT 10091
#define SYMBOL_CSUBPREPINTROPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSUBPREPINTROPANEL_TITLE _("SubmissionPrepIntroPanel")
#define SYMBOL_CSUBPREPINTROPANEL_IDNAME ID_CSUBPREPINTROPANEL
#define SYMBOL_CSUBPREPINTROPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSUBPREPINTROPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSubPrepIntroPanel class declaration
 */

class CSubPrepIntroPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CSubPrepIntroPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSubPrepIntroPanel();
    CSubPrepIntroPanel( wxWindow* parent, IWorkbench* workbench, wxWindowID id = SYMBOL_CSUBPREPINTROPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSUBPREPINTROPANEL_POSITION, const wxSize& size = SYMBOL_CSUBPREPINTROPANEL_SIZE, long style = SYMBOL_CSUBPREPINTROPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSUBPREPINTROPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSUBPREPINTROPANEL_POSITION, const wxSize& size = SYMBOL_CSUBPREPINTROPANEL_SIZE, long style = SYMBOL_CSUBPREPINTROPANEL_STYLE );

    /// Destructor
    ~CSubPrepIntroPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSubPrepIntroPanel event handler declarations

    /// wxEVT_LEFT_DOWN event handler for wxID_STATIC
    void OnClickSubmitterSummary( wxMouseEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_HYPERLINKCTRL5
    void OnEditSubmitterInfoClicked( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_SUBMISSION_TYPE
    void OnSubmissionTypeSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_URL event handler for ID_INTRO_TEXT
    void OnIntroTextClickUrl( wxTextUrlEvent& event );

////@end CSubPrepIntroPanel event handler declarations

////@begin CSubPrepIntroPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSubPrepIntroPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSubPrepIntroPanel member variables
    wxStaticText* m_StarSubmitter;
    wxTextCtrl* m_SubmitterLabel;
    wxStaticText* m_StarType;
    wxListBox* m_SubmissionType;
    wxTextCtrl* m_IntroText;
////@end CSubPrepIntroPanel member variables

    // for communicating with parent
    void RefreshSubmitterInfoStatus();
    CSourceRequirements::EWizardType GetWizardTypeFromCtrl ();
    void SetWizardTypeCtrl();
    bool IsComplete();

private:
    IWorkbench* m_Workbench;

    void x_LaunchSubmitEditor();
    CSourceRequirements::EWizardType x_GetWizardTypeFromSelection (int sel);

    // for communicating with parent
    CSubPrep_panel* x_GetParent();
    void x_SetSubmissionNotReady(string error);
    void x_UpdateSubmitBlock(CRef<objects::CSubmit_block> new_block, CRef<objects::CCit_gen> new_gen, string alt_email);
    void x_SetIntroText(CSourceRequirements::EWizardType wizard_type);

};

END_NCBI_SCOPE

#endif
    // _SUBPREPINTROPANEL_H_
