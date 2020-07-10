/*  $Id: subprepmainpanel.hpp 27920 2013-04-25 13:04:34Z bollin $
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
#ifndef _SUBPREPMAINPANEL_H_
#define _SUBPREPMAINPANEL_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */
#include <gui/packages/pkg_sequence_edit/subprep_panel.hpp>
#include <gui/packages/pkg_sequence_edit/subprepfieldeditor.hpp>
#include <gui/packages/pkg_sequence_edit/chimera_panel.hpp>
#include <gui/packages/pkg_sequence_edit/virus_molecule_questions_panel.hpp>

////@begin includes
#include "wx/hyperlink.h"
////@end includes
#include <wx/stattext.h>
#include <wx/choice.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxHyperlinkCtrl;
class wxBoxSizer;
class wxFlexGridSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

class CSubPrep_panel;
class CSubPrepFieldEditor;
class CChimeraPanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CSUBMISSIONPREPMAINPANEL 10001
#define ID_TEXTCTRL 10004
#define ID_HYPERLINKCTRL1 10005
#define ID_SOURCETYPE 10010
#define ID_TEXTCTRL2 10008
#define ID_HYPERLINKCTRL2 10009
#define ID_TEXTCTRL1 10007
#define ID_HYPERLINKCTRL3 10028
#define ID_HYPERLINKCTRL4 10081
#define ID_REMOVE_FEAT 10108
#define SYMBOL_CSUBMISSIONPREPMAINPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSUBMISSIONPREPMAINPANEL_TITLE _("SubmissionPrepMainPanel")
#define SYMBOL_CSUBMISSIONPREPMAINPANEL_IDNAME ID_CSUBMISSIONPREPMAINPANEL
#define SYMBOL_CSUBMISSIONPREPMAINPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSUBMISSIONPREPMAINPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSubmissionPrepMainPanel class declaration
 */

class CSubmissionPrepMainPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CSubmissionPrepMainPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSubmissionPrepMainPanel();
    CSubmissionPrepMainPanel( wxWindow* parent, wxWindowID id = SYMBOL_CSUBMISSIONPREPMAINPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSUBMISSIONPREPMAINPANEL_POSITION, const wxSize& size = SYMBOL_CSUBMISSIONPREPMAINPANEL_SIZE, long style = SYMBOL_CSUBMISSIONPREPMAINPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSUBMISSIONPREPMAINPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSUBMISSIONPREPMAINPANEL_POSITION, const wxSize& size = SYMBOL_CSUBMISSIONPREPMAINPANEL_SIZE, long style = SYMBOL_CSUBMISSIONPREPMAINPANEL_STYLE );

    /// Destructor
    ~CSubmissionPrepMainPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSubmissionPrepMainPanel event handler declarations

    /// wxEVT_LEFT_DOWN event handler for ID_TEXTCTRL
    void OnClickSeqTechSummary( wxMouseEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_HYPERLINKCTRL1
    void OnEditSequencingTechnologyClicked( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_SOURCETYPE
    void OnSourceTypeSelected( wxCommandEvent& event );

    /// wxEVT_LEFT_DOWN event handler for ID_TEXTCTRL2
    void OnClickSrcSummary( wxMouseEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_HYPERLINKCTRL2
    void OnEditSourceClicked( wxHyperlinkEvent& event );

    /// wxEVT_LEFT_DOWN event handler for ID_TEXTCTRL1
    void OnClickFeatureSummary( wxMouseEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_HYPERLINKCTRL3
    void OnAddFeaturesClicked( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_HYPERLINKCTRL4
    void OnImportFeatureTableFile( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_REMOVE_FEAT
    void OnRemoveFeatHyperlinkClicked( wxHyperlinkEvent& event );

////@end CSubmissionPrepMainPanel event handler declarations

////@begin CSubmissionPrepMainPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSubmissionPrepMainPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSubmissionPrepMainPanel member variables
    wxStaticText* m_StarTech;
    wxTextCtrl* m_SequencingTechLabel;
    wxHyperlinkCtrl* m_SeqTechEdit;
    wxStaticText* m_StarSourceType;
    wxChoice* m_SourceType;
    wxStaticText* m_StarSource;
    wxTextCtrl* m_SourceLabel;
    wxHyperlinkCtrl* m_EditSourceLink;
    wxStaticText* m_StarFeatures;
    wxTextCtrl* m_FeaturesLabel;
    wxHyperlinkCtrl* m_FeatureAddBtn;
    wxHyperlinkCtrl* m_FeatureImportBtn;
    wxHyperlinkCtrl* m_RemoveFeaturesBtn;
    wxBoxSizer* m_WizardExtras;
    wxFlexGridSizer* m_ExtraGrid;
    wxBoxSizer* m_ChimeraSizer;
    wxBoxSizer* m_VirusMolSizer;
////@end CSubmissionPrepMainPanel member variables

    typedef pair<CSourceRequirements::EWizardSrcType, string> TWizardSrcTypeName;
    typedef vector<TWizardSrcTypeName> TWizardSrcTypeNameList;

    // for communicating with parent
    void UpdateSeqTechCtrls (CSourceRequirements::EWizardType wizard_type);
    void SetSourceTypeOptions (CSourceRequirements::EWizardType wizard_type);
    CSourceRequirements::EWizardSrcType GetWizardSrcTypeFromCtrls (CSourceRequirements::EWizardType wizard_type);
    void SetSourceType (CSourceRequirements::EWizardSrcType src_type);
    CSourceRequirements::EWizardSrcType GetWizardSrcTypeFromName(string name);
    void SetSourceLabel (string label);
    void ShowSourceLabelStar (bool val);
    void SetFeaturesLabel (string label);
    void ShowAddFeatureBtn (bool val); 
    void ShowFeaturesStar (bool val);

    void ResetFieldsInUse();
    void ClearUnusedFields();
    void AddField(objects::CSeq_entry_Handle seh, 
                  ICommandProccessor* processor,
                  IWorkbench* workbench,
                  string label, string column_name, 
                  CTableFieldCommandConverter * converter, 
                  bool must_choose = false, bool required = false, 
                  string default_val = "", string url = "",
                  bool allow_mix = true);

    void CreateSetChoice (CSourceRequirements::EWizardType wizard_type, objects::CSeq_entry_Handle seh);
    void ShowChimera (objects::CSeq_entry_Handle seh, 
                      ICommandProccessor* processor,
                      IWorkbench* workbench, bool show);
    void CreateVirusMoleculeOptions (objects::CSeq_entry_Handle seh, ICommandProccessor* processor);

private:

    TWizardSrcTypeNameList m_SrcTypeNames;
  
  // for communicating with parent
    CSubPrep_panel* x_GetParent();


    wxStaticText* x_AddStar(wxFlexGridSizer* sizer);

    vector<CSubPrepFieldEditor * > m_ExtraFields;
    vector<bool> m_FieldsInUse;

    CChimeraPanel* m_ChimeraPanel;
    CVirusMoleculeQuestionsPanel* m_VirusMolQuestions;

};

END_NCBI_SCOPE

#endif
    // _SUBPREPMAINPANEL_H_
