/*  $Id: subprep_panel.hpp 37342 2016-12-27 18:02:05Z katargir $
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
#ifndef _SUBPREP_PANEL_H_
#define _SUBPREP_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/seqdesc_ci.hpp>

#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/tbl_edit_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/subprepintropanel.hpp>
#include <gui/packages/pkg_sequence_edit/subprepmainpanel.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/general/User_object.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objmgr/seqdesc_ci.hpp>

#include <gui/utils/event_handler.hpp>
#include <gui/framework/workbench.hpp>

/*!
 * Includes
 */

#include <wx/hyperlink.h>
#include <wx/scrolwin.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/notebook.h>
#include <wx/button.h>

/*!
 * Forward declarations
 */
class wxChoice;


////@begin forward declarations
class wxNotebook;
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

class CSrcQual;
class CSourceRequirements;
class ICommandProccessor;
class CSubPrepIntroPanel;
class CSubmissionPrepMainPanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSUBPREP_PANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSUBPREP_PANEL_TITLE _("Submission Preparation Tool")
#define SYMBOL_CSUBPREP_PANEL_IDNAME ID_CSUBPREP_PANEL
#define SYMBOL_CSUBPREP_PANEL_SIZE wxDefaultSize
#define SYMBOL_CSUBPREP_PANEL_POSITION wxDefaultPosition
////@end control identifiers

#define ID_MOLTYPE_CHOICE   10100
#define ID_TOPOLOGY_CHOICE  10101
#define ID_SET_CHOICE       10104
#define ID_BULK_MOLINFO_EDIT 10203

const string kReleaseDateConfirmed = "Release Date confirmed";


/*!
 * CSubPrep_panel class declaration
 */

class CSubPrep_panel: public wxScrolledWindow, public CEventHandler
{    
    DECLARE_DYNAMIC_CLASS( CSubPrep_panel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSubPrep_panel();
    CSubPrep_panel( wxWindow* parent, wxWindowID id = SYMBOL_CSUBPREP_PANEL_IDNAME, const wxPoint& pos = SYMBOL_CSUBPREP_PANEL_POSITION, const wxSize& size = SYMBOL_CSUBPREP_PANEL_SIZE, long style = SYMBOL_CSUBPREP_PANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSUBPREP_PANEL_IDNAME, const wxPoint& pos = SYMBOL_CSUBPREP_PANEL_POSITION, const wxSize& size = SYMBOL_CSUBPREP_PANEL_SIZE, long style = SYMBOL_CSUBPREP_PANEL_STYLE );

    /// Destructor
    ~CSubPrep_panel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool Layout();

    void UpdateForSeqEntryChange();

    /// Transfer data to the window
    virtual bool TransferDataToWindow();

    /// @name IRegSettings interface implementation
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

////@begin CSubPrep_panel event handler declarations

    /// wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED event handler for ID_NOTEBOOK
    void OnNotebookPageChanged( wxNotebookEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON17
    void OnButton17Click( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ADD_SEQUENCE_BTN
    void OnAddSequenceBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_REORDER_SEQ
    void OnReorderSeqClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON16
    void OnRemoveSequencesClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON13
    void OnClickVectorTrim( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_PREV_BTN
    void OnPrevBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_NEXT_BTN
    void OnNextBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_FLATFILE
    void OnFlatfileClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_LOOKUP_TAXONOMY
    void OnLookupTaxonomyClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_VALIDATE
    void OnValidateClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SAVE_BTN
    void OnSaveBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_START_NEW
    void OnStartNewClick( wxCommandEvent& event );

////@end CSubPrep_panel event handler declarations

    void OnChangeSet( wxCommandEvent& event );

////@begin CSubPrep_panel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSubPrep_panel member function declarations

    void SetObjects(TConstScopedObjects* objects) { m_InputObjects = objects; }

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSubPrep_panel member variables
    wxStaticText* m_Status;
    wxNotebook* m_Book;
    wxStaticText* m_SequenceCount;
    wxButton* m_ReorderSequencesBtn;
    wxButton* m_RemoveSequencesBtn;
    wxButton* m_VectorTrimBtn;
    wxButton* m_PrevBtn;
    wxButton* m_NextBtn;
    wxBoxSizer* m_StarExplanation;
    wxButton* m_FlatfileButton;
    wxButton* m_TaxonomyButton;
    wxButton* m_ValidateButton;
    wxButton* m_StartNewBtn;
    /// Control identifiers
    enum {
        ID_CSUBPREP_PANEL = 10000,
        ID_NOTEBOOK = 10092,
        ID_BUTTON17 = 10119,
        ID_ADD_SEQUENCE_BTN = 10089,
        ID_REORDER_SEQ = 10098,
        ID_BUTTON16 = 10083,
        ID_BUTTON13 = 10076,
        ID_PREV_BTN = 10102,
        ID_NEXT_BTN = 10103,
        ID_FLATFILE = 10101,
        ID_LOOKUP_TAXONOMY = 10111,
        ID_VALIDATE = 10106,
        ID_SAVE_BTN = 10082,
        ID_START_NEW = 10110
    };
////@end CSubPrep_panel member variables
    TConstScopedObjects*    m_InputObjects;

    void SetUndoManager(ICommandProccessor* cmdProccessor) { m_CmdProcessor = cmdProccessor; }
    void SetWorkbench (IWorkbench* workbench) { m_Workbench = workbench; }

    typedef pair<CSourceRequirements::EWizardType, string> TWizardKeyword;
    typedef vector<TWizardKeyword> TWizardKeywordList;

    void OnDataChange ();

    // for communication with subtabs
    void SetSubmissionNotReady(string error);
    CRef<objects::CSubmit_block> GetSubmitBlock();
    objects::CSeqdesc_CI GetCitSubPub();
    void UpdateSubmitBlock(CRef<objects::CSubmit_block> new_block, CRef<objects::CCit_gen> new_gen, string alt_email);
    CSourceRequirements::EWizardType GetWizardType();
    CSourceRequirements::EWizardType SetWizardType(CSourceRequirements::EWizardType wizard_type);
    void SetSourceType(string src_type);
    CSourceRequirements::EWizardSrcType GetWizardSrcType() { return x_GetWizardSrcTypeFromCtrls(); };
    string GetWizardTypeField();
    string GetWizardSrcTypeField();
    string GetAltEmailAddress() { return GetAlternateEmailAddress(m_TopSeqEntry); };
    void SetAltEmailAddress(string alt_email) { SetAlternateEmailAddress(m_TopSeqEntry, alt_email); };


    void LaunchSeqTechEditor();
    void ClearSeqTechValues();
    bool OkToBulkEditSeqTech();
    void LaunchSourceEditor();
    void LaunchFeatureAdder();
    bool IsSequencingTechnologyRequired (CSourceRequirements::EWizardType wizard_type);
    CRef<objects::CSeq_table> GetSeqTechTable ();
    void ImportFeatureTableFile ();
    void HandleBulkCmdDlg (CBulkCmdDlg* dlg, string label = "");
    void RemoveAllFeatures ();

    static string GetWizardTypeName (unsigned int wizard_type);
    static CSourceRequirements::EWizardType GuessWizardTypefromSeqEntry(objects::CSeq_entry_Handle entry);
    static CSourceRequirements::EWizardType GetWizardTypefromSeqEntry(objects::CSeq_entry_Handle entry);
    // wizard user object functions
    static string GetWizardFieldFromSeqEntry (objects::CSeq_entry_Handle entry, string field_name);
    static CRef<CCmdComposite> SetWizardFieldInSeqEntry(objects::CSeq_entry_Handle entry, string field_name, string value);

    string GetWizardField(string field_name) { return GetWizardFieldFromSeqEntry(m_TopSeqEntry, field_name);};
    CRef<CCmdComposite> SetWizardField(string field_name, string value) { return SetWizardFieldInSeqEntry(m_TopSeqEntry, field_name, value);};
    CTableCommandConverter* GetConverterForFieldName (string field_name);

private:  
    TWizardKeywordList m_WizardNames; 
    TWizardKeywordList m_Keywords; 

    CSourceRequirements::EWizardType x_GetWizardTypeFromCtrl();

    CSourceRequirements::EWizardSrcType x_GetWizardSrcTypeFromCtrls ();
    CSourceRequirements::EWizardSrcType x_GetWizardSrcTypefromSeqEntry(objects::CSeq_entry_Handle entry);


    void x_SetSourceTypeOptionsForWizardType (CSourceRequirements::EWizardType wizard_type);
    void x_SetSourceType (CSourceRequirements::EWizardSrcType src_type);
    void x_RefreshSubmitterInfoStatus();
    void x_UpdateFeatureStatus(CSourceRequirements::EWizardType wizard_type);
    void x_CreateWizardExtras();
    CRef<CCmdComposite> x_ClearWizardSpecificData(CSourceRequirements::EWizardType wizard_type);

    bool x_NeedTopology(CSourceRequirements::EWizardType wizard_type, CMolInfoTableCommandConverter* converter);
    void x_CreateMoleculeOptions(CSourceRequirements::EWizardType wizard_type, 
                                 CSourceRequirements::EWizardSrcType src_type);
    void x_CreatePrimerOptions ();
    void x_CreateCommentOptions (const string& comment_label);
    void x_CreateChimeraOptions ();
    void x_CreateGenomeOptions (CSourceRequirements::EWizardType wizard_type, 
                                 CSourceRequirements::EWizardSrcType src_type);
    void x_CreateDBLinkOptions ();
    bool x_IsSequencingTechnologyRequired (objects::CSeq_entry_Handle entry, CSourceRequirements::EWizardType wizard_type);
    void x_SetSubmissionStatus();
    void x_LaunchVectorTrimEditor();
    void x_SetNextPrevBtns();
    void x_EnableSeqButtons();
    void x_UpdateSequenceCountLabel();
    void x_CheckValidation();

    bool m_IsSingle;
    vector<string> m_DBLinkFields;
    vector<string> m_DBLinkUrls;
    vector<bool> m_DBLinkRequired;

    string x_CheckSourceQuals(CSourceRequirements::EWizardType wizard_type);

    bool x_NeedUniqueMicrosatelliteNames();
    vector<size_t> x_GetSequenceLengths(CRef<objects::CSeq_table> table);
    CFeatureTableCommandConverter* x_GetMicrosatelliteTableConverter();

    CRef<objects::CSeq_table> x_BuildValuesTable();
    void x_ConvertToSeqSubmit();

    string m_RegPath;
    ICommandProccessor* m_CmdProcessor;
    IWorkbench* m_Workbench;
    objects::CSeq_entry_Handle m_TopSeqEntry;
    CRef<objects::CSeq_submit> m_SeqSubmit;

    CSubPrepIntroPanel* m_IntroPanel;
    CSubmissionPrepMainPanel* m_MainPanel;
    bool m_DataIsSaved;
    string m_SubmissionErrors;
    bool m_SubmissionOk;
    wxString m_SaveFileDir;
    wxString m_SaveFileName;
};


END_NCBI_SCOPE

#endif
    // _SUBPREP_PANEL_H_
