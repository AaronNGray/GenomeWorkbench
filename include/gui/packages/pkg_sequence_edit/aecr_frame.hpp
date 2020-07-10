/*  $Id: aecr_frame.hpp 44393 2019-12-10 21:13:12Z filippov $
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
 * Authors:  Igor Filippov
 *
 * File Description:
 *
 */

#ifndef AECR_FRAME__HPP
#define AECR_FRAME__HPP


#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */


////@begin includes
#include <wx/listbook.h>
#include <wx/choicebk.h>
////@end includes
#include <objects/macro/String_constraint.hpp>
#include <gui/widgets/edit/cds_gene_prot_field_name_panel.hpp>
#include <gui/widgets/edit/rna_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/parse_text_options_dlg.hpp>
#include <gui/widgets/edit/edit_text_options_panel.hpp>
#include <gui/packages/pkg_sequence_edit/remove_text_outside_string_panel.hpp>
#include <gui/packages/pkg_sequence_edit/cap_change_panel.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/constraint_panel.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxListbook;
class wxComboBox;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CAECRFRAME_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CAECRFRAME_TITLE _("Apply Edit Convert Remove")
#define SYMBOL_CAECRFRAME_IDNAME wxID_ANY
#define SYMBOL_CAECRFRAME_SIZE wxDefaultSize
#define SYMBOL_CAECRFRAME_POSITION wxDefaultPosition
#define ID_AECR_FRAME_LISTBOOK 14001
#define ID_AECR_FRAME_LISTVIEW 14002
#define ID_AECR_AUTOPOPULATE_APPLY_CHECKBOX 14003
#define ID_AECR_AUTOPOPULATE_APPLY_COMBOBOX 14004
#define ID_AECR_AUTOPOPULATE_EDIT_CHECKBOX 14005
#define ID_AECR_LEAVE_ORIGINAL_CONVERT_CHECKBOX 14006
#define ID_AECR_STRIP_NAME_CONVERT_CHECKBOX 14007
////@end control identifiers

/*!
 * CAECRFrame class declaration
 */

class COkCancelPanel;
class IActionPanel;

class CAECRFrame: public CBulkCmdDlg, public CFieldNamePanelParent
{    
    DECLARE_DYNAMIC_CLASS( CAECRFrame )
    DECLARE_EVENT_TABLE()

public:
    CAECRFrame();
    CAECRFrame( wxWindow* parent, IWorkbench* wb, 
                wxWindowID id = SYMBOL_CAECRFRAME_IDNAME, const wxString& caption = SYMBOL_CAECRFRAME_TITLE, const wxPoint& pos = SYMBOL_CAECRFRAME_POSITION, const wxSize& size = SYMBOL_CAECRFRAME_SIZE, long style = SYMBOL_CAECRFRAME_STYLE );

    bool Create( wxWindow* parent, IWorkbench* wb,
                 wxWindowID id = SYMBOL_CAECRFRAME_IDNAME, const wxString& caption = SYMBOL_CAECRFRAME_TITLE, const wxPoint& pos = SYMBOL_CAECRFRAME_POSITION, const wxSize& size = SYMBOL_CAECRFRAME_SIZE, long style = SYMBOL_CAECRFRAME_STYLE );

    virtual ~CAECRFrame();

    void Init();

    void CreateControls();

////@begin CAECRFrame event handler declarations

////@end CAECRFrame event handler declarations

////@begin CAECRFrame member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CAECRFrame member function declarations


    static bool ShowToolTips();

    // Inherited from CBulkCmdDlg
    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage() {return kEmptyStr;}
    
    void UpdateConstraintFieldType( wxCommandEvent& event );   
    void BeforePageChange(wxBookCtrlEvent& event);
    void AfterPageChange(wxBookCtrlEvent& event);
    void OnKeyDown(wxKeyEvent& event);

    enum EActionType {
        eActionType_Apply = 0,
        eActionType_Edit,
        eActionType_Convert,
        eActionType_Copy,
        eActionType_Swap,
        eActionType_Parse,
        eActionType_Remove,
        eActionType_RemoveTextOutside
    };

    enum EFieldType {
        eFieldType_Taxname = 0,
        eFieldType_Source,
        eFieldType_Feature,
        eFieldType_CDSGeneProt,
        eFieldType_RNA,
        eFieldType_MolInfo,
        eFieldType_Pub,
        eFieldType_StructuredComment,
        eFieldType_DBLink,
        eFieldType_Misc
    };

    void SetInitParameters(EActionType action_type, EFieldType field_type, const string &field = kEmptyStr, const wxString &title = wxEmptyString);
	
    // inherited from CFieldNamePanelParent
    virtual void UpdateEditor(void);
    CSeq_entry_Handle GetTopSeqEntryHandle(void) {GetTopLevelSeqEntryAndProcessor(); return m_TopSeqEntry;}
    CConstRef<objects::CSeq_submit> GetSeqSubmit(void) {return m_SeqSubmit;}
    CAdvancedConstraintPanel* GetConstraintPanel(void) {return m_Constraint;}
private:

    void UpdateChildrenFeaturePanels( wxWindow* win, vector<const CFeatListItem *> *featlist );
    void UpdateStructuredCommentFields( wxWindow* win, const set<string> &fields );

    virtual void SetRegistryPath(const string& reg_path);  
    virtual void LoadSettings();
    virtual void SaveSettings() const;

    void RemoveNotebook();
    void UnbindNotebookPageChanged();

////@begin CAECRFrame member variables
    IActionPanel* m_ActionPanel;
    wxListbook* m_Listbook;
    CAdvancedConstraintPanel* m_Constraint;
    wxCheckBox* m_ClearWhenChanging;
    int m_QualPage;
    string m_CurrentField1;
    string m_CurrentField2;
    COkCancelPanel* m_OkCancel;
    string m_RegPath;
};

class ISpecificQual
{
public:
    virtual ~ISpecificQual() {}
    virtual vector<string> GetChoices(bool& allow_other) const = 0;    
    virtual string GetField1(bool subfield = true) const {return kEmptyStr;}
    virtual string GetField2(bool subfield = true) const {return kEmptyStr;}
    virtual CFieldNamePanel* GetFieldNamePanel1(void) {return NULL;}
    virtual CFieldNamePanel* GetFieldNamePanel2(void) {return NULL;}
    virtual void UpdateMRNAbutton(void) {}
    virtual void UpdateRetranslateCds(void) {}
    virtual bool SetFieldName(const string& field) {return false;}
    virtual bool SetFieldName2(const string& field) {return false;}
    virtual bool GetUpdateMRNAProductFlag(void) const {return false;}
    virtual bool GetRetranslateCds(void) const {return false;}
    virtual void SetShowLegalOnly(bool show) {}
};

class IQualPanel
{
public:
    virtual ~IQualPanel() {}
    int GetSelection(void) const;
    void SetSelection(int page, const string &field1 = kEmptyStr, const string &field2 = kEmptyStr);
    void RemoveNotebook();
    vector<string> GetChoices(bool& allow_other) const;
    string GetField1(bool subfield = true) const;
    string GetField2(bool subfield = true) const;
    string GetFieldType(void) const;
    CFieldNamePanel* GetFieldNamePanel1(void);
    CFieldNamePanel* GetFieldNamePanel2(void);
    ISpecificQual* GetSpecificQualPanel(void);
    virtual void UnbindNotebookPageChanged() {}
protected:
    wxNotebook* m_Notebook;
    mutable ISpecificQual* m_SpecificQual;
    mutable string m_FieldType;
};

class IActionPanel
{
public:
    virtual ~IActionPanel() {}
    int GetSelection(void) const {return m_Panel->GetSelection();}
    void SetSelection(int page, const string &field1 = kEmptyStr, const string &field2 = kEmptyStr) {m_Panel->SetSelection(page, field1, field2);}
    virtual void UpdateQualChoices(void);
    string GetAutopopulateValue(wxWindow *parent_win);
    CAECRFrame *GetBaseFrame(wxWindow *win);
    string GetField1(bool subfield = true) const;
    string GetField2(bool subfield = true) const;
    string GetFieldType(void) const;
    CFieldNamePanel* GetFieldNamePanel1(void);
    CFieldNamePanel* GetFieldNamePanel2(void);
    virtual void SetupAction(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2, edit::EExistingText existing_text) {}
    bool GetUpdateMRNAProductFlag(void) const;
    bool GetRetranslateCds(void) const;
    CIRef<IEditingAction> CreateAction1(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit);
    virtual CIRef<IEditingAction> CreateAction2(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit) {return CIRef<IEditingAction>(NULL);}
    CIRef<IEditingAction> CreateAction2_imp(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit);
    virtual int CheckForExistingText(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2) {return 0;}
    virtual void ClearValues(void) {}
    virtual bool IsAction2Required(void) {return false;}
    void RemoveNotebook() { m_Panel->RemoveNotebook();}
protected:
    IQualPanel* m_Panel; 
};


class CQualApplyPanel: public wxPanel, public IActionPanel
{    
    DECLARE_DYNAMIC_CLASS( CQualApplyPanel )
    DECLARE_EVENT_TABLE()

public:
    CQualApplyPanel();
    CQualApplyPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    virtual ~CQualApplyPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    void OnUpdateQualChoices(wxCommandEvent& event);
    void OnTextEnter(wxCommandEvent& event);
    void OnAutopopulateOptClick( wxCommandEvent& event );
    void x_Autopopulate(void);

    // inherited from IActionPanel
    virtual void UpdateQualChoices(void);
    virtual void SetupAction(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2, edit::EExistingText existing_text);
    virtual int CheckForExistingText(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2) {return action1->CheckForExistingText(action2, IEditingAction::eActionType_Apply);}
    virtual void ClearValues(void);

private:
    wxCheckBox* m_AutopopulateOpt;
    wxComboBox* m_ApplyChoiceBox;
    bool m_allow_other;
};

class CQualEditPanel: public wxPanel, public IActionPanel
{    
    DECLARE_DYNAMIC_CLASS( CQualEditPanel )
    DECLARE_EVENT_TABLE()

public:
    CQualEditPanel();
    CQualEditPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    virtual ~CQualEditPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    void OnAutopopulateOptClick( wxCommandEvent& event );
    void x_Autopopulate(void);
    void OnUpdateQualChoices(wxCommandEvent& event);

    // inherited from IActionPanel
    virtual void UpdateQualChoices(void);
    virtual void SetupAction(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2, edit::EExistingText existing_text);
    virtual int CheckForExistingText(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2) {return action1->CheckForExistingText(action2, IEditingAction::eActionType_Edit);}
    virtual void ClearValues(void);
private:
    wxCheckBox* m_AutopopulateOpt;
    CEditTextOptionsPanel* m_EditOptions;
};

class CQualConvertPanel: public wxPanel, public IActionPanel
{    
    DECLARE_DYNAMIC_CLASS( CQualConvertPanel )
    DECLARE_EVENT_TABLE()

public:
    CQualConvertPanel();
    CQualConvertPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    virtual ~CQualConvertPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();

    // inherited from IActionPanel
    virtual void SetupAction(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2, edit::EExistingText existing_text);
    virtual CIRef<IEditingAction> CreateAction2(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit) {return CreateAction2_imp(seh, submit);}
    virtual bool IsAction2Required(void) {return true;}
    virtual int CheckForExistingText(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2) {return action1->CheckForExistingText(action2, IEditingAction::eActionType_ConvertFrom);}
    virtual void ClearValues(void);
private:
    wxCheckBox* m_LeaveOriginal;
    wxCheckBox* m_StripNameFromText;
    CCapChangePanel* m_CapChangeOptions;
};

class CQualParsePanel: public wxPanel, public IActionPanel
{    
    DECLARE_DYNAMIC_CLASS( CQualParsePanel )
    DECLARE_EVENT_TABLE()

public:
    CQualParsePanel();
    CQualParsePanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    virtual ~CQualParsePanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    virtual void SetupAction(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2, edit::EExistingText existing_text);
    virtual CIRef<IEditingAction> CreateAction2(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit) {return CreateAction2_imp(seh, submit);}
    virtual bool IsAction2Required(void) {return true;}
    virtual int CheckForExistingText(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2) {return action1->CheckForExistingText(action2, IEditingAction::eActionType_ParseFrom, *m_ParseOptions->GetParseOptions());}
    virtual void ClearValues(void);
private:
    CParseTextOptionsDlg* m_ParseOptions;
};

class CQualRemoveOutsidePanel: public wxPanel, public IActionPanel
{    
    DECLARE_DYNAMIC_CLASS( CQualRemoveOutsidePanel )
    DECLARE_EVENT_TABLE()

public:
    CQualRemoveOutsidePanel();
    CQualRemoveOutsidePanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    virtual ~CQualRemoveOutsidePanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();

    // inherited from IActionPanel
    virtual void SetupAction(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2, edit::EExistingText existing_text);
    virtual int CheckForExistingText(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2) {return action1->CheckForExistingText(action2, IEditingAction::eActionType_RemoveOutside);}
    virtual void ClearValues(void);
private:
    CRemoveTextOutsideStringPanel* m_RemoveTextOutsideOptions;
};

class CQualCopyPanel: public wxPanel, public IActionPanel
{    
    DECLARE_DYNAMIC_CLASS( CQualCopyPanel )
    DECLARE_EVENT_TABLE()

public:
    CQualCopyPanel();
    CQualCopyPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    virtual ~CQualCopyPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();

    // inherited from IActionPanel
    virtual void SetupAction(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2, edit::EExistingText existing_text);
    virtual CIRef<IEditingAction> CreateAction2(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit) {return CreateAction2_imp(seh, submit);}
    virtual bool IsAction2Required(void) {return true;}
    virtual int CheckForExistingText(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2) {return action1->CheckForExistingText(action2, IEditingAction::eActionType_CopyFrom);}
    virtual void ClearValues(void);
};

class CQualSwapPanel: public wxPanel, public IActionPanel
{    
    DECLARE_DYNAMIC_CLASS( CQualSwapPanel )
    DECLARE_EVENT_TABLE()

public:
    CQualSwapPanel();
    CQualSwapPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    virtual ~CQualSwapPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();

    // inherited from IActionPanel
    virtual void SetupAction(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2, edit::EExistingText existing_text);
    virtual CIRef<IEditingAction> CreateAction2(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit) {return CreateAction2_imp(seh, submit);}
    virtual bool IsAction2Required(void) {return true;}
    virtual int CheckForExistingText(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2) {return action1->CheckForExistingText(action2, IEditingAction::eActionType_SwapFrom);}
    virtual void ClearValues(void);
};

class CQualRemovePanel: public wxPanel, public IActionPanel
{    
    DECLARE_DYNAMIC_CLASS( CQualRemovePanel )
    DECLARE_EVENT_TABLE()

public:
    CQualRemovePanel();
    CQualRemovePanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    virtual ~CQualRemovePanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();

    // inherited from IActionPanel
    virtual void SetupAction(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2, edit::EExistingText existing_text);
    virtual int CheckForExistingText(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2) {return action1->CheckForExistingText(action2, IEditingAction::eActionType_Remove);}
    virtual void ClearValues(void);
};


END_NCBI_SCOPE

#endif  // AECR_FRAME__HPP
