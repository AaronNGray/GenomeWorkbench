/*  $Id: macro_add_constraint_panel.cpp 44731 2020-03-03 15:53:48Z asztalos $
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
#include <ncbi_pch.hpp>

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <objects/macro/Word_substitution_set.hpp>
#include <objects/macro/CDSGenePro_qual_constraint.hpp>
#include <objects/macro/Source_constraint.hpp>
#include <objects/macro/Source_qual_choice.hpp>
#include <objects/macro/CDSGenePr_constraint_field.hpp>
#include <gui/objutils/macro_biodata.hpp>
#include <gui/objutils/macro_fn_where.hpp>
#include <gui/widgets/edit/source_field_name_panel.hpp>
#include <gui/widgets/edit/pub_field_name_panel.hpp>
#include <gui/widgets/edit/struct_comm_field_panel.hpp>
#include <gui/widgets/edit/rna_field_name_panel.hpp>

#include <gui/widgets/edit/molinfo_fieldtype.hpp>
#include <gui/widgets/edit/pub_fieldtype.hpp>
#include <gui/widgets/edit/dblink_fieldtype.hpp>
#include <gui/widgets/edit/misc_fieldtype.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>
#include <gui/widgets/edit/seqid_field_name_panel.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/widgets/edit/macro_loc_constraint_panel.hpp>
#include <gui/widgets/edit/macro_feats_on_seq_constraint_panel.hpp>
#include <gui/widgets/edit/macro_choicetype_constraint_panel.hpp>
#include <gui/widgets/edit/macro_editor_context.hpp>
#include <gui/widgets/edit/word_substitute_dlg.hpp>
#include <gui/widgets/edit/macro_add_constraint_panel.hpp>

#include <wx/statline.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/valnum.h>
#include <wx/artprov.h>
#include <wx/filedlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CADVSTRINGCONSTRAINTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CADVSTRINGCONSTRAINTPANEL_TITLE _("StringConstraintPanel")
#define SYMBOL_CADVSTRINGCONSTRAINTPANEL_IDNAME ID_CADVSTRINGCONSTRAINTPANEL
#define SYMBOL_CADVSTRINGCONSTRAINTPANEL_SIZE wxDefaultSize
#define SYMBOL_CADVSTRINGCONSTRAINTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CMacroAdvStringConstraintPanel class declaration
 */
template<typename T>
class CMacroAdvStringConstraintPanel : public CMacroMatcherPanelBase, public wxPanel, public CFieldNamePanelParent, public CWordSubstitutionSetConsumer
{    
    wxDECLARE_NO_COPY_CLASS(CMacroAdvStringConstraintPanel);
public:
    /// Constructors
    CMacroAdvStringConstraintPanel();
    CMacroAdvStringConstraintPanel(wxWindow* parent,
        wxWindowID id = SYMBOL_CADVSTRINGCONSTRAINTPANEL_IDNAME,
        const wxPoint& pos = SYMBOL_CADVSTRINGCONSTRAINTPANEL_POSITION,
        const wxSize& size = SYMBOL_CADVSTRINGCONSTRAINTPANEL_SIZE,
        long style = SYMBOL_CADVSTRINGCONSTRAINTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CADVSTRINGCONSTRAINTPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CADVSTRINGCONSTRAINTPANEL_POSITION, 
        const wxSize& size = SYMBOL_CADVSTRINGCONSTRAINTPANEL_SIZE, 
        long style = SYMBOL_CADVSTRINGCONSTRAINTPANEL_STYLE );

    /// Destructor
    ~CMacroAdvStringConstraintPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CMacroAdvStringConstraintPanel event handler declarations
    void OnClear( wxCommandEvent& event );    
    void OnMatchTypeSelected( wxCommandEvent& event );
    void OnWordSubstitution( wxCommandEvent& event );
    void OnConstraintType( wxCommandEvent& event );
    void OnOpenFile(wxCommandEvent& event);
////@end CMacroAdvStringConstraintPanel event handler declarations

////@begin CMacroAdvStringConstraintPanel member function declarations

    enum EMatchType {
        eMatchType_Contains = 0,
        eMatchType_DoesNotContain,
        eMatchType_Equals,
        eMatchType_DoesNotEqual,
        eMatchType_StartsWith,
        eMatchType_EndsWith,
        eMatchType_IsOneOf,
        eMatchType_IsNotOneOf,
        eMatchType_DoesNotStartWith,
        eMatchType_DoesNotEndWith
    };


    void ClearValues(void);
    void SetChoices(const vector<string> &choices, const vector<string> &values);
    void ShowChoiceOrText(void);
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMacroAdvStringConstraintPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual pair<string, string> GetMatcher(const pair<string, string>& target, size_t num);
    virtual void SetMatcher(const string &item);
    virtual void GetVars(vector<string> &vars, size_t num);
    virtual string GetQualName(const pair<string, string>& target);

    // inherited from CFieldNamePanelParent
    virtual void UpdateEditor(void);

    string GetDescription(const string& field);
    string GetTextConstraint(const string& field, size_t num);

    virtual void AddWordSubstSet(CRef<objects::CWord_substitution_set> word_subst);
    virtual void SetFieldName(const string& field);
    virtual void PopulateFeatureListbox(objects::CSeq_entry_Handle seh);
    virtual void SetFieldNames(const vector<string>& field_names);
    void SetMacroFieldNames(const vector<string>& field_names);
    void SetMacroSelf(const string& self);

private:
    void EnableStringConstraint(bool enable);
    string ReplaceChoices(const string &orig);

    T *m_panel;

////@begin CMacroAdvStringConstraintPanel member variables
    wxChoice* m_MatchType;
    wxTextCtrl* m_MatchText;
    wxCheckBox* m_IgnoreCase;
    wxCheckBox* m_IgnoreSpace;
    wxCheckBox* m_WholeWord;
    wxCheckBox* m_IgnorePunct;
    wxCheckBox* m_IgnoreSyn;
    wxRadioButton* m_AnyLetters;
    wxRadioButton*  m_AllUpper;
    wxRadioButton*  m_AllLower;
    wxRadioButton*  m_AllPunct;
    wxComboBox* m_MatchChoice;
    wxButton* m_ClearButton;
    wxButton* m_WordSubst;
    wxRadioBox* m_ConstraintType;
    wxTextCtrl* m_Tablename;
    wxTextCtrl* m_Column;
////@end CMacroAdvStringConstraintPanel member variables
    
    enum {
        ID_CADVSTRINGCONSTRAINTPANEL = 6300,
        ID_STRING_CONSTRAINT_MATCH_CHOICE,
        ID_STRING_CONSTRAINT_MATCH_TEXT,
        ID_STRING_CONSTRAINT_CHECKBOX5,
        ID_STRING_CONSTRAINT_CHECKBOX6,
        ID_STRING_CONSTRAINT_CHECKBOX7,
        ID_STRING_CONSTRAINT_CHECKBOX8,
        ID_STRING_CONSTRAINT_CHECKBOX9,
        ID_STRING_CONSTRAINT_RADIO0,
        ID_STRING_CONSTRAINT_RADIO1,
        ID_STRING_CONSTRAINT_RADIO2,
        ID_STRING_CONSTRAINT_RADIO3,
        ID_STRING_CONSTRAINT_CLEAR_BUTTON,
        ID_STRING_CONSTRAINT_WORDSUBST_BUTTON,
        ID_CONSTRAINT_TYPE,
        ID_OPENFILEBTMBTN,
        ID_CONSTRAINT_TABLENAME,
        ID_CONSTRAINT_TABLECOL
    };
    
    wxBoxSizer* m_TextSizer;
    wxFlexGridSizer* m_TableSizer;
    bool m_is_choice;
    CRef<objects::CWord_substitution_set> m_word_subst;
    map<string, string> m_choice_to_value;
};

IMPLEMENT_DYNAMIC_CLASS( CMacroAddConstraint, wxFrame )

BEGIN_EVENT_TABLE( CMacroAddConstraint, wxFrame )

////@begin CMacroAddConstraint event table entries
    EVT_BUTTON( wxID_OK, CMacroAddConstraint::OnAccept )
    EVT_BUTTON( wxID_CANCEL, CMacroAddConstraint::OnCancel )
////@end CMacroAddConstraint event table entries

END_EVENT_TABLE()

CMacroAddConstraint::CMacroAddConstraint()
{
    Init();
}

CMacroAddConstraint::CMacroAddConstraint( wxWindow* parent, objects::CSeq_entry_Handle seh, const pair<string, string>& target, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh), m_target(target), m_id(-1), m_num(0)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CMacroAddConstraint::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMacroAddConstraint creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);
////@end CMacroAddConstraint creation
    return true;
}

CMacroAddConstraint::~CMacroAddConstraint()
{
////@begin CMacroAddConstraint destruction
////@end CMacroAddConstraint destruction
}

void CMacroAddConstraint::Init()
{
////@begin CMacroAddConstraint member initialisation
    m_Notebook = NULL;
////@end CMacroAddConstraint member initialisation
}

void CMacroAddConstraint::CreateControls()
{    
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer1);

    wxPanel* itemCBulkCmdDlg1 = new wxPanel(this, wxID_ANY);
    itemBoxSizer1->Add(itemCBulkCmdDlg1, 1, wxGROW, 0);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    m_Notebook = new wxChoicebook( itemCBulkCmdDlg1, ID_MACROADDCONSTRAINTCHOICEBK, wxDefaultPosition, wxDefaultSize, wxCHB_TOP );

    itemBoxSizer2->Add(m_Notebook, 1, wxGROW|wxALL, 5);

    const CTempString target = m_target.first;
    bool special_target = (target == macro::CMacroBioData::sm_SeqAlign 
        || target == macro::CMacroBioData::sm_TSEntry 
        || target == macro::CMacroBioData::sm_SeqSet);

    if (!special_target) {
        CMacroAdvStringConstraintPanel<CSourceFieldNamePanel> *panel1 = new CMacroAdvStringConstraintPanel<CSourceFieldNamePanel>(m_Notebook);
        m_Notebook->AddPage(panel1,_("Taxname"));
        panel1->SetFieldName("taxname");
    
        CMacroAdvStringConstraintPanel<CSourceFieldNamePanel> *panel2 = new CMacroAdvStringConstraintPanel<CSourceFieldNamePanel>(m_Notebook); 
        m_Notebook->AddPage(panel2,_("Source"));

        CMacroAdvStringConstraintPanel<CFeatureFieldNamePanel> *panel3 = new CMacroAdvStringConstraintPanel<CFeatureFieldNamePanel>(m_Notebook);
        m_Notebook->AddPage(panel3, _("Feature"));
        panel3->PopulateFeatureListbox(m_TopSeqEntry);
        if (CFieldNamePanel::IsFeature(target)) {
            SetStringSelection("Feature");
            panel3->SetFieldName(target);
        }

        CMacroAdvStringConstraintPanel<CCDSGeneProtFieldNamePanel> *panel4 = new CMacroAdvStringConstraintPanel<CCDSGeneProtFieldNamePanel>(m_Notebook);
        m_Notebook->AddPage(panel4, _("CDS-Gene-Prot-mRNA"));
        panel4->SetFieldName("protein name");

        CMacroAdvStringConstraintPanel<CRNAFieldNamePanel> *panel5 = new CMacroAdvStringConstraintPanel<CRNAFieldNamePanel>(m_Notebook);
        m_Notebook->AddPage(panel5, _("RNA"));
        panel5->SetFieldName("product");
    }

    bool molinfo_target = (target == macro::CMacroBioData::sm_MolInfo
        || target == macro::CMacroBioData::sm_BioSource
        || NStr::StartsWith(target, macro::CMacroBioData::sm_Seq)
        || CFieldNamePanel::IsFeature(target));
    if (molinfo_target) {
        CMacroAdvStringConstraintPanel<CFieldHandlerNamePanel> *panel6 =  new CMacroAdvStringConstraintPanel<CFieldHandlerNamePanel>(m_Notebook);
        panel6->SetFieldNames(CMolInfoFieldType::GetFieldNames());
        panel6->SetMacroFieldNames(CMolInfoFieldType::GetMacroFieldNames());
        panel6->SetMacroSelf(macro::CMacroBioData::sm_MolInfo);
        m_Notebook->AddPage(panel6,_("MolInfo"));
        panel6->SetFieldName("molecule");
    }

    if (target == macro::CMacroBioData::sm_Pubdesc) {
        CMacroAdvStringConstraintPanel<CFieldHandlerNamePanel> *panel7 =  new CMacroAdvStringConstraintPanel<CFieldHandlerNamePanel>(m_Notebook);
        bool extended = true;
        panel7->SetFieldNames(CPubFieldType::GetFieldNames(extended));
        panel7->SetMacroFieldNames(CPubFieldType::GetMacroFieldNames());
        panel7->SetMacroSelf(macro::CMacroBioData::sm_Pubdesc);
        m_Notebook->AddPage(panel7,_("Pub"));
        panel7->SetFieldName("title");
    }

    if (target == macro::CMacroBioData::sm_StrComm 
        || target == macro::CMacroBioData::sm_SeqNa) {
        CMacroAdvStringConstraintPanel<CStructCommentFieldPanel> *panel8 =  new CMacroAdvStringConstraintPanel<CStructCommentFieldPanel>(m_Notebook);
        m_Notebook->AddPage(panel8,_("Structured Comment"));
    }

    if (target == macro::CMacroBioData::sm_DBLink 
        || NStr::StartsWith(target, macro::CMacroBioData::sm_Seq)
        || target == macro::CMacroBioData::sm_StrComm
        || target == macro::CMacroBioData::sm_BioSource
        || target == macro::CMacroBioData::sm_MolInfo) {
        CMacroAdvStringConstraintPanel<CFieldHandlerNamePanel> *panel9 =  new CMacroAdvStringConstraintPanel<CFieldHandlerNamePanel>(m_Notebook);
        panel9->SetFieldNames(CDBLinkFieldType::GetFieldNames());
        panel9->SetMacroFieldNames(CDBLinkFieldType::GetFieldNames());
        panel9->SetMacroSelf("DBLink");
        m_Notebook->AddPage(panel9,_("DBLink"));
        panel9->SetFieldName("title");
    }

    if (target == macro::CMacroBioData::sm_SeqNa
        || target == macro::CMacroBioData::sm_Seq
        || target == macro::CMacroBioData::sm_Seqdesc 
        || target == macro::CMacroBioData::sm_UserObject)
    {
        CMacroAdvStringConstraintPanel<CFieldHandlerNamePanel> *panel10 =  new CMacroAdvStringConstraintPanel<CFieldHandlerNamePanel>(m_Notebook);
        panel10->SetFieldNames(CMiscFieldType::GetStrings());
        panel10->SetMacroFieldNames(CMiscFieldType::GetMacroFieldNames());
        panel10->SetMacroSelf("Misc");
        m_Notebook->AddPage(panel10,_("Misc"));
        panel10->SetFieldName(kDefinitionLineLabel);
    }

    if (CFieldNamePanel::IsFeature(target))
    {
        CMacroLocationConstraintPanel *panel11 = new CMacroLocationConstraintPanel(m_Notebook);
        m_Notebook->AddPage(panel11,_("Location"));
    }

    CMacroFeatsOnSeqPanel *panel12 = new CMacroFeatsOnSeqPanel(m_Notebook);
    m_Notebook->AddPage(panel12,_("Sequence and Features"));

    if (target == macro::CMacroBioData::sm_SeqFeat 
        || target == macro::CMacroBioData::sm_Seqdesc
        || target == macro::CMacroBioData::sm_UserObject
        || target == macro::CMacroBioData::sm_ImpFeat
        || target == macro::CMacroBioData::sm_RNA
        || target == macro::CMacroBioData::sm_SeqSet)
    {
        CMacroChoiceTypePanel *panel13 = new CMacroChoiceTypePanel(m_Notebook, target);
        m_Notebook->AddPage(panel13,_("Choice type"));
    }

    CMacroAdvStringConstraintPanel<CSeqIdPanel> *panel14 =  new CMacroAdvStringConstraintPanel<CSeqIdPanel>(m_Notebook);
    m_Notebook->AddPage(panel14,_("Seq-Id"));

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL, 0);

    wxButton* itemButton13 = new wxButton( itemCBulkCmdDlg1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemCBulkCmdDlg1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    if (target == macro::CMacroBioData::sm_RNA ||
        target == macro::CMacroBioData::sm_rRNA ||
        target == macro::CMacroBioData::sm_mRNA ||
        target == macro::CMacroBioData::sm_miscRNA) {
        SetStringSelection("RNA");
    }
    if (target == macro::CMacroBioData::sm_BioSource)
    {
        SetStringSelection("Source");
    }
    if (target == macro::CMacroBioData::sm_MolInfo || NStr::StartsWith(target, macro::CMacroBioData::sm_Seq))
    {
        SetStringSelection("MolInfo");
    }
    if (target == macro::CMacroBioData::sm_Pubdesc)
    {
        SetStringSelection("Pub");
    }
    if (target == macro::CMacroBioData::sm_StrComm)
    {
        SetStringSelection("Structured Comment");
    }
    if (target == macro::CMacroBioData::sm_DBLink)
    {
        SetStringSelection("DBLink");
    }
    if (target == macro::CMacroBioData::sm_SeqNa)
    {
        SetStringSelection("Seq-Id");
    }
}

void CMacroAddConstraint::SetStringSelection(const string& sel)
{
    for (size_t i = 0; i < m_Notebook->GetPageCount(); i++)
    {
        string label = m_Notebook->GetPageText(i).ToStdString();
        if (label  == sel)
        {
            m_Notebook->SetSelection(i);
            break;
        }
    }
}

bool CMacroAddConstraint::ShowToolTips()
{
    return true;
}
wxBitmap CMacroAddConstraint::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMacroAddConstraint bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMacroAddConstraint bitmap retrieval
}
wxIcon CMacroAddConstraint::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMacroAddConstraint icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMacroAddConstraint icon retrieval
}

void CMacroAddConstraint::OnCancel( wxCommandEvent& event )
{
    Close();
}

void CMacroAddConstraint::OnAccept( wxCommandEvent& event )
{
    wxWindow* win = m_Notebook->GetCurrentPage();
    CMacroMatcherPanelBase* panel = dynamic_cast<CMacroMatcherPanelBase*>(win);
    vector<string> vars;
    pair<string, string> item = panel->GetMatcher(m_target, m_num);
#ifdef _DEBUG
    LOG_POST(Info << item.first << " " << item.second);
#endif

    panel->GetVars(vars, m_num);
    string qual_name = panel->GetQualName(m_target);
    if (item.first.empty() || item.second.empty()) 
    {
        event.Skip();
        return;
    }

    wxWindow* parent = GetParent();
    CMacroCompoundConstraintPanel* compound_panel = dynamic_cast<CMacroCompoundConstraintPanel*>(parent);
    compound_panel->AddConstraint(item, m_id, vars, qual_name);
    NMacroStats::ReportUsage(this->GetLabel(), "add constraint");
    Close();
}


void CMacroAddConstraint::SetConstraint(const string &constraint)
{
//    m_Notebook->SetSelection(constraint.top_level); // TODO
    wxWindow* win = m_Notebook->GetCurrentPage();
    CMacroMatcherPanelBase* panel = dynamic_cast<CMacroMatcherPanelBase*>(win);
    panel->SetMatcher(constraint);
}


/*!
 * CMacroAdvStringConstraintPanel constructors
 */
template<typename T>
CMacroAdvStringConstraintPanel<T>::CMacroAdvStringConstraintPanel()
{
    Init();
}

template<typename T>
CMacroAdvStringConstraintPanel<T>::CMacroAdvStringConstraintPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CMacroAdvStringConstraintPanel creator
 */
template<typename T>
bool CMacroAdvStringConstraintPanel<T>::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMacroAdvStringConstraintPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CMacroAdvStringConstraintPanel creation
    ShowChoiceOrText();
    
    return true;
}


/*!
 * CMacroAdvStringConstraintPanel destructor
 */
template<typename T>
CMacroAdvStringConstraintPanel<T>::~CMacroAdvStringConstraintPanel()
{
////@begin CMacroAdvStringConstraintPanel destruction
////@end CMacroAdvStringConstraintPanel destruction
}


/*!
 * Member initialisation
 */
template<typename T>
void CMacroAdvStringConstraintPanel<T>::Init()
{
////@begin CMacroAdvStringConstraintPanel member initialisation
    m_panel = NULL;
    m_MatchType = NULL;
    m_MatchText = NULL;
    m_IgnoreCase = NULL;
    m_IgnoreSpace = NULL;
    m_WholeWord = NULL;
    m_IgnorePunct = NULL;
    m_IgnoreSyn = NULL;
    m_AnyLetters = NULL;
    m_AllUpper = NULL;
    m_AllLower = NULL;
    m_AllPunct = NULL;
    m_MatchChoice = NULL;
    m_TextSizer = NULL;
    m_ClearButton = NULL;
    m_WordSubst = NULL;
    m_ConstraintType = NULL;
    m_is_choice = false;
    m_Tablename = NULL;
    m_Column = NULL;
////@end CMacroAdvStringConstraintPanel member initialisation
}


/*!
 * Control creation for CMacroAdvStringConstraintPanel
 */
template<typename T>
void CMacroAdvStringConstraintPanel<T>::CreateControls()
{    
////@begin CMacroAdvStringConstraintPanel content construction
    //CMacroAdvStringConstraintPanel* itemPanel1 = this;
    wxPanel* parentPanel = this;

    wxBoxSizer* itemBoxSizer0 = new wxBoxSizer(wxVERTICAL);
    parentPanel->SetSizer(itemBoxSizer0);

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer0->Add(itemBoxSizer1, 0, wxGROW|wxALL, 0);

    m_panel = new T(parentPanel);
    itemBoxSizer1->Add(m_panel, 0, wxALIGN_TOP|wxALL, 5);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_TOP|wxALL, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_MatchTypeStrings;
    m_MatchTypeStrings.Add(_("Contains"));
    m_MatchTypeStrings.Add(_("Does not contain"));
    m_MatchTypeStrings.Add(_("Equals"));
    m_MatchTypeStrings.Add(_("Does not equal"));
    m_MatchTypeStrings.Add(_("Starts with"));
    m_MatchTypeStrings.Add(_("Ends with"));
    m_MatchTypeStrings.Add(_("Is one of"));
    m_MatchTypeStrings.Add(_("Is not one of"));
    m_MatchTypeStrings.Add(_("Does not start with"));
    m_MatchTypeStrings.Add(_("Does not end with"));
    
    m_MatchType = new wxChoice( parentPanel, ID_STRING_CONSTRAINT_MATCH_CHOICE, wxDefaultPosition, wxDefaultSize, m_MatchTypeStrings, 0 );
    m_MatchType->SetStringSelection(_("Contains"));
    itemBoxSizer3->Add(m_MatchType, 0, wxALIGN_TOP|wxLEFT|wxRIGHT|wxBOTTOM, 5);
    m_MatchType->Bind(wxEVT_CHOICE, &CMacroAdvStringConstraintPanel<T>::OnMatchTypeSelected, this);
    
    m_TextSizer = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(m_TextSizer, 0, wxALIGN_TOP, 0);

    m_MatchText = new wxTextCtrl( parentPanel, ID_STRING_CONSTRAINT_MATCH_TEXT, wxEmptyString, wxDefaultPosition, wxSize(300, -1), wxTE_MULTILINE );
    m_TextSizer->Add(m_MatchText, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 5);   
    wxArrayString choice_strings;
    m_MatchChoice = new wxComboBox(parentPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), choice_strings, 0 );
    m_MatchChoice->Hide();


    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer6->Add(itemBoxSizer7, 0, wxALIGN_TOP|wxALL, 5);

    m_AnyLetters = new wxRadioButton( parentPanel, ID_STRING_CONSTRAINT_RADIO0, _("Any letters"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AnyLetters->SetValue(true);
    itemBoxSizer7->Add(m_AnyLetters, 0, wxALIGN_LEFT|wxALL, 5);

    m_AllUpper = new wxRadioButton( parentPanel, ID_STRING_CONSTRAINT_RADIO1, _("All letters are upper case"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AllUpper->SetValue(false);
    itemBoxSizer7->Add(m_AllUpper, 0, wxALIGN_LEFT|wxALL, 5);

    m_AllLower = new wxRadioButton( parentPanel, ID_STRING_CONSTRAINT_RADIO2, _("All letters are lower case"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AllLower->SetValue(false);
    itemBoxSizer7->Add(m_AllLower, 0, wxALIGN_LEFT|wxALL, 5);

    m_AllPunct = new wxRadioButton( parentPanel, ID_STRING_CONSTRAINT_RADIO3, _("All characters are punctuation"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AllPunct->SetValue(false);
    itemBoxSizer7->Add(m_AllPunct, 0, wxALIGN_LEFT|wxALL, 5);
   

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer6->Add(itemBoxSizer8, 0, wxALIGN_TOP|wxALL, 5);

    m_IgnoreCase = new wxCheckBox( parentPanel, ID_STRING_CONSTRAINT_CHECKBOX5, _("Ignore Case"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreCase->SetValue(false);
    itemBoxSizer8->Add(m_IgnoreCase, 0, wxALIGN_LEFT|wxALL, 5);

    m_IgnoreSpace = new wxCheckBox( parentPanel, ID_STRING_CONSTRAINT_CHECKBOX6, _("Ignore Space"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreSpace->SetValue(false);
    itemBoxSizer8->Add(m_IgnoreSpace, 0, wxALIGN_LEFT|wxALL, 5);

    m_IgnorePunct = new wxCheckBox( parentPanel, ID_STRING_CONSTRAINT_CHECKBOX8, _("Ignore Punctuation"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnorePunct->SetValue(false);
    itemBoxSizer8->Add(m_IgnorePunct, 0, wxALIGN_LEFT|wxALL, 5);       

    m_WholeWord = new wxCheckBox( parentPanel, ID_STRING_CONSTRAINT_CHECKBOX7, _("Whole Word"), wxDefaultPosition, wxDefaultSize, 0 );
    m_WholeWord->SetValue(false);
    itemBoxSizer8->Add(m_WholeWord, 0, wxALIGN_LEFT|wxALL, 5);   

    m_IgnoreSyn = new wxCheckBox( parentPanel, ID_STRING_CONSTRAINT_CHECKBOX9, _("Ignore 'putative' synonyms"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreSyn->SetValue(false);
    itemBoxSizer8->Add(m_IgnoreSyn, 0, wxALIGN_LEFT|wxALL, 5);   
    

    m_ClearButton = new wxButton( parentPanel, ID_STRING_CONSTRAINT_CLEAR_BUTTON, _("Clear Constraint"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_ClearButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_ClearButton->Connect(ID_STRING_CONSTRAINT_CLEAR_BUTTON, wxEVT_BUTTON, wxCommandEventHandler(CMacroAdvStringConstraintPanel<T>::OnClear), NULL, this);

// TODO ???
//    m_WordSubst = new wxButton( parentPanel, ID_STRING_CONSTRAINT_WORDSUBST_BUTTON, _("Word Substitutions"), wxDefaultPosition, wxDefaultSize, 0 );
//    itemBoxSizer9->Add(m_WordSubst, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
//    m_WordSubst->Connect(ID_STRING_CONSTRAINT_WORDSUBST_BUTTON, wxEVT_BUTTON, wxCommandEventHandler(CMacroAdvStringConstraintPanel<T>::OnWordSubstitution), NULL, this);

    ////@end CMacroAdvStringConstraintPanel content construction

    wxBoxSizer* hor_sizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer0->Add(hor_sizer, 0, wxALIGN_LEFT | wxALL, 0);

    wxArrayString present_not_present;
    present_not_present.Add(_("Match value"));
    present_not_present.Add(_("Present"));
    present_not_present.Add(_("Not present"));
    present_not_present.Add(_("From table"));
    m_ConstraintType = new wxRadioBox(parentPanel, ID_CONSTRAINT_TYPE, wxEmptyString, wxDefaultPosition, wxDefaultSize, present_not_present, 1, wxRA_SPECIFY_ROWS);
    m_ConstraintType->SetSelection(0);
    hor_sizer->Add(m_ConstraintType, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT, 5);
    m_ConstraintType->Connect(ID_CONSTRAINT_TYPE, wxEVT_RADIOBOX, wxCommandEventHandler(CMacroAdvStringConstraintPanel<T>::OnConstraintType), NULL, this);

    m_TableSizer = new wxFlexGridSizer(0, 2, 0, 0);
    hor_sizer->Add(m_TableSizer, 0, wxALIGN_BOTTOM | wxRESERVE_SPACE_EVEN_IF_HIDDEN | wxALL, 0);

    wxStaticText* itemStaticText10 = new wxStaticText(parentPanel, wxID_STATIC, _("File name"), wxDefaultPosition, wxDefaultSize, 0);
    m_TableSizer->Add(itemStaticText10, 0, wxALIGN_LEFT | wxRESERVE_SPACE_EVEN_IF_HIDDEN | wxLEFT, 5);

    wxStaticText* itemStaticText11 = new wxStaticText(parentPanel, wxID_STATIC, _("Column"), wxDefaultPosition, wxDefaultSize, 0);
    m_TableSizer->Add(itemStaticText11, 0, wxALIGN_LEFT | wxRESERVE_SPACE_EVEN_IF_HIDDEN | wxLEFT, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    m_TableSizer->Add(itemBoxSizer11, 0, wxALIGN_LEFT | wxRESERVE_SPACE_EVEN_IF_HIDDEN | wxALL, 0);

    m_Tablename  = new wxTextCtrl(parentPanel, ID_CONSTRAINT_TABLENAME, wxEmptyString, wxDefaultPosition, wxSize(270,-1));
    itemBoxSizer11->Add(m_Tablename, 0, wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN | wxALL, 5);

    wxButton* itemButton12 = new wxBitmapButton(parentPanel, ID_OPENFILEBTMBTN, wxArtProvider::GetBitmap(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    itemButton12->SetToolTip(wxT("Choose a table file..."));
    itemBoxSizer11->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN| wxALL, 2);
    itemButton12->Bind(wxEVT_BUTTON, &CMacroAdvStringConstraintPanel<T>::OnOpenFile, this);

    wxIntegerValidator<unsigned> validator;
    validator.SetMin(1);
    m_Column = new wxTextCtrl(parentPanel, ID_CONSTRAINT_TABLECOL, wxEmptyString, wxDefaultPosition, wxSize(30, -1), 0, validator);
    m_TableSizer->Add(m_Column, 0, wxALIGN_LEFT | wxRESERVE_SPACE_EVEN_IF_HIDDEN | wxALL, 5);

    hor_sizer->Hide(m_TableSizer, true);


    CFieldNamePanel* panel = dynamic_cast<CFieldNamePanel*>(m_panel);
    if (panel)
    {
        bool allow_other = true;
        vector<string> choices = panel->GetChoices(allow_other); 
        vector<string> values = panel->GetMacroValues();
        SetChoices(choices, values);      
    }
    CSourceFieldNamePanel* src_panel = dynamic_cast<CSourceFieldNamePanel*>(m_panel);
    if (src_panel)
    {
        src_panel->HideSourceType();
    }
}

template<typename T>
void CMacroAdvStringConstraintPanel<T>::OnClear( wxCommandEvent& event )
{
    ClearValues();  
}

template<typename T>
void CMacroAdvStringConstraintPanel<T>::ClearValues()
{
    if (m_is_choice)
    {
        m_MatchChoice->Hide();
        m_TextSizer->Replace(m_MatchChoice, m_MatchText);
        m_MatchText->Show();
        m_is_choice = false;
        Layout();
        Fit();
    }
    m_MatchType->SetStringSelection(_("Contains"));
    m_MatchText->SetValue(wxEmptyString);
    m_IgnoreCase->SetValue(false);
    m_IgnoreSpace->SetValue(false);
    m_WholeWord->SetValue(false);
    m_IgnorePunct->SetValue(false);
    m_IgnoreSyn->SetValue(false);
    m_AnyLetters->SetValue(true);
    m_AllUpper->SetValue(false);
    m_AllLower->SetValue(false);
    m_AllPunct->SetValue(false);
    m_word_subst.Reset();
}

template<typename T>
void CMacroAdvStringConstraintPanel<T>::OnConstraintType( wxCommandEvent& event )
{
    int sel = m_ConstraintType->GetSelection();
    bool enable = (sel == 0);
    EnableStringConstraint(enable);

    enable = (sel == 3);
    m_TableSizer->ShowItems(enable);
}

template<typename T>
void  CMacroAdvStringConstraintPanel<T>::EnableStringConstraint(bool enable)
{
 if (m_MatchChoice->IsShown())
     m_MatchChoice->Enable(enable);
 else
     m_MatchText->Enable(enable);

    m_MatchType->Enable(enable);
    m_MatchText->Enable(enable);
    m_IgnoreCase->Enable(enable);
    m_IgnoreSpace->Enable(enable);
    m_WholeWord->Enable(enable);
    m_IgnorePunct->Enable(enable);
    m_IgnoreSyn->Enable(enable);
    m_AnyLetters->Enable(enable);
    m_AllUpper->Enable(enable);
    m_AllLower->Enable(enable);
    m_AllPunct->Enable(enable);
    m_ClearButton->Enable(enable);
    if (m_WordSubst)
        m_WordSubst->Enable(enable);
}

template<typename T>
void CMacroAdvStringConstraintPanel<T>::SetChoices(const vector<string> &choices, const vector<string> &values)
{
    wxArrayString choice_strings;
    ITERATE(vector<string>, it, choices) 
    {
        choice_strings.Add(ToWxString(*it));
    }
    m_MatchChoice->Set(choice_strings);
    _ASSERT(values.empty() || choices.size() == values.size());
    m_choice_to_value.clear();
    for (size_t i = 0; i < values.size(); i++)
    m_choice_to_value[choices[i]] = values[i];

    ShowChoiceOrText();
}

template<typename T>
void CMacroAdvStringConstraintPanel<T>::ShowChoiceOrText()
{
   if (!m_MatchChoice->IsListEmpty() && (m_MatchType->GetSelection() == eMatchType_Equals || m_MatchType->GetSelection() == eMatchType_DoesNotEqual) )
    {
        if (!m_is_choice)
        {
            m_MatchText->Hide();
            m_TextSizer->Replace(m_MatchText, m_MatchChoice);
            m_MatchChoice->Show();
        }
        m_is_choice = true;
    }
    else
    {
        if (m_is_choice)
        {
            m_MatchChoice->Hide();
            m_TextSizer->Replace(m_MatchChoice, m_MatchText);
            m_MatchText->Show();
        }
        m_is_choice = false;
    }
    Layout();
    Fit();

}

template<typename T>
void CMacroAdvStringConstraintPanel<T>::OnMatchTypeSelected( wxCommandEvent& event )
{
    ShowChoiceOrText();
    event.Skip();
}

template<typename T>
void CMacroAdvStringConstraintPanel<T>::OnOpenFile(wxCommandEvent& event)
{
    if (event.GetId() != ID_OPENFILEBTMBTN)
        return;

    wxFileDialog dlg(this, wxT("Select table file"), wxEmptyString, wxEmptyString, _("All files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() == wxID_OK) {
        auto mapped_path = CMacroEditorContext::GetInstance().GetAbsolutePath(dlg.GetPath());
        m_Tablename->SetValue(mapped_path);
        m_Tablename->SetInsertionPoint(m_Tablename->GetLastPosition());
    }
}

/*!
 * Should we show tooltips?
 */
template<typename T>
bool CMacroAdvStringConstraintPanel<T>::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */
template<typename T>
wxBitmap CMacroAdvStringConstraintPanel<T>::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMacroAdvStringConstraintPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMacroAdvStringConstraintPanel bitmap retrieval
}

/*!
 * Get icon resources
 */
template<typename T>
wxIcon CMacroAdvStringConstraintPanel<T>::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMacroAdvStringConstraintPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMacroAdvStringConstraintPanel icon retrieval
}

template <typename T>
void CMacroAdvStringConstraintPanel<T>::UpdateEditor()
{
    m_choice_to_value.clear();
    CFieldNamePanel* panel = dynamic_cast<CFieldNamePanel*>(m_panel);
    if (panel)
    {
        bool allow_other = true;
        vector<string> choices = panel->GetChoices(allow_other); 
        vector<string> values = panel->GetMacroValues();
        SetChoices(choices, values);  
    }
}

template <typename T>
pair<string, string> CMacroAdvStringConstraintPanel<T>::GetMatcher(const pair<string, string> &target, size_t num)
{
    CFieldNamePanel* field_name_panel = dynamic_cast<CFieldNamePanel*>(m_panel);
    if (field_name_panel)
    {
        string field = field_name_panel->GetMacroFieldName(target.first, target.second);
        string label = field_name_panel->GetMacroFieldLabel(target.first, target.second);

        string field_name = field_name_panel->GetFieldName(false);
        vector<string> text_constraint;
        if (!label.empty())
            text_constraint.push_back(label);
        text_constraint.push_back(GetTextConstraint(field, num));
        return make_pair(GetDescription(field_name), NStr::Join(text_constraint, " AND "));
    }
    return make_pair(kEmptyStr, kEmptyStr);
}

template <typename T>
string CMacroAdvStringConstraintPanel<T>::GetQualName(const pair<string, string> &target)
{
    CFieldNamePanel* field_name_panel = dynamic_cast<CFieldNamePanel*>(m_panel);
    if (field_name_panel)
    {
        string field = field_name_panel->GetMacroFieldName(target.first, target.second);
        return field;
    }
    return kEmptyStr;
}

template <typename T>
string CMacroAdvStringConstraintPanel<T>::GetDescription(const string& field)
{
    int constraint_sel = m_ConstraintType->GetSelection();
    if (constraint_sel == 1) 
        return field + " is present";
    if (constraint_sel == 2)
        return field + " is not present";
    if (constraint_sel == 3)
        return field + " is used as identifier";

    string str;
    string match_text; 
    if (!m_is_choice)
        match_text = NStr::Replace(NStr::Replace(ToStdString(m_MatchText->GetValue()),"\r"," "),"\n"," ");
    else 
        match_text = NStr::Replace(NStr::Replace(ToStdString(m_MatchChoice->GetValue()),"\r"," "),"\n"," ");

    if (!match_text.empty())
    {
        switch(m_MatchType->GetSelection()) {
        case eMatchType_Contains:
            str = "contains ";
            break;
        case eMatchType_DoesNotContain:
            str = "does not contain ";
            break;
        case eMatchType_Equals:
            str = "equals ";
            break;
        case eMatchType_DoesNotEqual:
            str = "does not equal ";
            break;
        case eMatchType_StartsWith:
            str = "starts with ";
            break;
        case eMatchType_EndsWith:
            str = "ends with ";
            break;
        case eMatchType_IsOneOf:
            str = "is one of ";
            break;
        case eMatchType_IsNotOneOf:
            str = "is not one of ";
            break;
        case eMatchType_DoesNotStartWith:
            str = "does not start with ";
            break;
        case eMatchType_DoesNotEndWith:
            str = "does not ends with ";
            break;
        default:  break;
        }

  
        str += "'" + match_text + "' ";
        
        vector<string> subs;
        if (!m_IgnoreCase->GetValue())
            subs.push_back("case-sensitive");
        
        if (m_IgnoreSpace->GetValue())
            subs.push_back("ignore spaces");
        
        if (m_IgnorePunct->GetValue())
            subs.push_back("ignore punctuation");
        
        if (m_WholeWord->GetValue())
            subs.push_back("whole word");
        
        if (m_IgnoreSyn->GetValue())
            subs.push_back("ignore 'putative' synonyms");
        
        if (!subs.empty())
        {
            string sub = NStr::Join(subs, ", ");
            str += "(" + sub + ")";
        }
        
        if (m_word_subst && m_word_subst->IsSet())
        {
            for (auto ws : m_word_subst->Set())
            {
                string label = CWordSubstitutionDlg::GetDescription(ws);
                str += ", " + label;
            }
        }
        
    }
    
    
    string sub;
    if (m_AllUpper->GetValue())
        sub = "all letters are upper case";

    if (m_AllLower->GetValue())
        sub = "all letters are lower case";

    if (m_AllPunct->GetValue())
        sub = "all characters are punctiation";
    
    if (!sub.empty())
    {
        if (!str.empty())
            str += ", ";
        str += sub;        
    }

    return field + " " + str;
}

template <typename T>
string CMacroAdvStringConstraintPanel<T>::GetTextConstraint(const string& field, size_t num)
{
    if (field.empty())
        return kEmptyStr;

    wxString wx_num;
    wx_num << num + 1;
    string num_str = wx_num.ToStdString();
    
    int constraint_sel = m_ConstraintType->GetSelection();
    if (constraint_sel == 1)
        return macro::CMacroFunction_IsPresent::GetFuncName() + "(" + field + ")";
    if (constraint_sel == 2)
        return "NOT " + macro::CMacroFunction_IsPresent::GetFuncName() + "(" + field + ")";
    if (constraint_sel == 3) {
        if (m_Tablename->IsShown() && !m_Tablename->IsEmpty()) {
            string filename = "\"" + ToStdString(m_Tablename->GetValue()) + "\"";
            string func = macro::CMacroFunction_InTable::GetFuncName();
            func += "(" + field + ", " + filename + ", " + ToStdString(m_Column->GetValue()) + ", \"\\t\")";
            return func;
        }
    }

    
    string str;
    string match_text;
    string match_var;
    if (!m_is_choice)
        match_text = NStr::Replace(NStr::Replace(ToStdString(m_MatchText->GetValue()),"\r"," "),"\n"," ");
    else 
        match_text = NStr::Replace(NStr::Replace(ToStdString(m_MatchChoice->GetValue()),"\r"," "),"\n"," ");

    if (!match_text.empty())
    {
        switch(m_MatchType->GetSelection()) {
        case eMatchType_Contains:
            str = "CONTAINS";
        match_var = "contains";
            break;
        case eMatchType_DoesNotContain:
            str = "NOT CONTAINS";
        match_var = "not_contains";
            break;
        case eMatchType_Equals:
            str = "EQUALS";
        match_var = "equals";
            break;
        case eMatchType_DoesNotEqual:
            str = "NOT EQUALS";
        match_var = "not_equals";
            break;
        case eMatchType_StartsWith:
            str = "STARTS";
        match_var = "starts";
            break;
        case eMatchType_EndsWith:
            str = "ENDS";
        match_var = "ends";
            break;
        case eMatchType_IsOneOf:
            str = "INLIST";
        match_var = "inlist";
            break;
        case eMatchType_IsNotOneOf:
            str = "NOT INLIST";
        match_var = "not_inlist";
            break;
        case eMatchType_DoesNotStartWith:
            str = "NOT STARTS";
        match_var = "not_starts";
            break;
        case eMatchType_DoesNotEndWith:
            str = "NOT ENDS";
        match_var = "not_ends";
            break;
        default:  break;
        }
        string case_sensitive("false"), ignore_space("false"), ignore_punctuation("false"), whole_word("false"), ignore_weasel("false");
        if (!m_IgnoreCase->GetValue())
            case_sensitive = "true";
        
        if (m_IgnoreSpace->GetValue())
            ignore_space = "true";
        
        if (m_IgnorePunct->GetValue())
            ignore_punctuation = "true";
        
        if (m_WholeWord->GetValue())
            whole_word = "true";
        
        if (m_IgnoreSyn->GetValue())
            ignore_weasel = "true";

        if (!str.empty())
        {
      str += "(" + field + "," + match_var + num_str + "," +  case_sensitive + "," + ignore_space + "," + ignore_punctuation + "," + whole_word + "," + ignore_weasel + ")";
        }
    }

    string sub;
    if (m_AllUpper->GetValue())
        sub = "ISUPPER(" + field + ")";

    if (m_AllLower->GetValue())
        sub = "ISLOWER(" + field + ")";

    if (m_AllPunct->GetValue())
        sub = "ISPUNCTUATION(" + field + ")";
    
    if (!sub.empty())
    {
        if (!str.empty())
            str += " AND ";
        str += sub;        
    }

    return str;
}

template <typename T>
void CMacroAdvStringConstraintPanel<T>::GetVars(vector<string> &vars, size_t num)
{
    vars.clear();

    wxString wx_num;
    wx_num << num + 1;
    string num_str = wx_num.ToStdString();


    string match_text;
    string match_var;
    if (!m_is_choice)
        match_text = NStr::Replace(NStr::Replace(ToStdString(m_MatchText->GetValue()),"\r"," "),"\n"," ");
    else 
        match_text = ReplaceChoices(NStr::Replace(NStr::Replace(ToStdString(m_MatchChoice->GetValue()),"\r"," "),"\n"," "));

    if (!match_text.empty())
    {
        switch(m_MatchType->GetSelection()) {
        case eMatchType_Contains:
        match_var = "contains";
            break;
        case eMatchType_DoesNotContain:
        match_var = "not_contains";
            break;
        case eMatchType_Equals:
        match_var = "equals";
            break;
        case eMatchType_DoesNotEqual:
        match_var = "not_equals";
            break;
        case eMatchType_StartsWith:
        match_var = "starts";
            break;
        case eMatchType_EndsWith:
        match_var = "ends";
            break;
        case eMatchType_IsOneOf:
        match_var = "inlist";
            break;
        case eMatchType_IsNotOneOf:
        match_var = "not_inlist";
            break;
        case eMatchType_DoesNotStartWith:
        match_var = "not_starts";
            break;
        case eMatchType_DoesNotEndWith:
        match_var = "not_ends";
            break;
        default:  break;
        }
    if (!match_var.empty())
        vars.push_back(match_var + num_str + " = %" + match_text + "%");
    }
}

template <typename T>
string CMacroAdvStringConstraintPanel<T>::ReplaceChoices(const string &orig)
{
    string result = orig;
    auto it = m_choice_to_value.find(orig);
    if (it != m_choice_to_value.end())
    result = it->second;
    return result;
}

template <typename T>
void CMacroAdvStringConstraintPanel<T>::OnWordSubstitution( wxCommandEvent& event )
{
    CWordSubstitutionDlg * dlg = new CWordSubstitutionDlg(this, m_word_subst); 
    dlg->Show(true);
}

template <typename T>
void CMacroAdvStringConstraintPanel<T>::AddWordSubstSet(CRef<objects::CWord_substitution_set> word_subst)
{
    m_word_subst = word_subst;
}

template <typename T>
void CMacroAdvStringConstraintPanel<T>::SetFieldName(const string& field) 
{
    m_panel->SetFieldName(field);
}

template <typename T>
void CMacroAdvStringConstraintPanel<T>::SetMatcher(const string &item)
{
//    m_panel->SetFieldName(item.full_field); // TODO
    CRef<CString_constraint> c;// TODO = item.string_constraint;
    if (!c)
        return;
    string match_text = c->GetMatch_text();
   
    m_IgnoreCase->SetValue(!c->GetCase_sensitive());
    m_IgnoreSpace->SetValue(c->GetIgnore_space());
    m_IgnorePunct->SetValue(c->GetIgnore_punct());
    m_WholeWord->SetValue(c->GetWhole_word());
    m_AllUpper->SetValue(c->GetIs_all_caps());
    m_AllLower->SetValue(c->GetIs_all_lower());
    m_AllPunct->SetValue(c->GetIs_all_punct());
    m_IgnoreSyn->SetValue(c->GetIgnore_weasel());
    
    if (c->IsSetIgnore_words())
    {
        m_word_subst.Reset(new objects::CWord_substitution_set);
        m_word_subst->Assign(c->GetIgnore_words());
    }

    if (c->IsSetNot_present() && c->GetNot_present())
    {
        switch (c->GetMatch_location())
        {
        case eString_location_contains : m_MatchType->SetSelection(eMatchType_DoesNotContain); break;
        case eString_location_equals : m_MatchType->SetSelection(eMatchType_DoesNotEqual); break;
        case eString_location_starts : m_MatchType->SetSelection(eMatchType_DoesNotStartWith); break;
        case eString_location_ends : m_MatchType->SetSelection(eMatchType_DoesNotEndWith); break;
        case eString_location_inlist : m_MatchType->SetSelection(eMatchType_IsNotOneOf); break;
        default : break;
        }
    }
    else
    {
        switch (c->GetMatch_location())
        {
        case eString_location_contains : m_MatchType->SetSelection(eMatchType_Contains); break;
        case eString_location_equals : m_MatchType->SetSelection(eMatchType_Equals); break;
        case eString_location_starts : m_MatchType->SetSelection(eMatchType_StartsWith); break;
        case eString_location_ends : m_MatchType->SetSelection(eMatchType_EndsWith); break;
        case eString_location_inlist : m_MatchType->SetSelection(eMatchType_IsOneOf); break;
        default : break;
        }
    }
    
    ShowChoiceOrText();

    if (!m_is_choice)
        m_MatchText->SetValue(wxString(match_text));
    else 
        m_MatchChoice->SetValue(wxString(match_text));
}

template <typename T>
void CMacroAdvStringConstraintPanel<T>::PopulateFeatureListbox(objects::CSeq_entry_Handle seh) 
{
    CFeatureFieldNamePanel *panel = dynamic_cast<CFeatureFieldNamePanel*>(m_panel);
    if (panel)
    {
        panel->ListPresentFeaturesFirst(seh);
    }
}

template <typename T>
void CMacroAdvStringConstraintPanel<T>::SetFieldNames(const vector<string>& field_names) 
{
    CFieldHandlerNamePanel *panel = dynamic_cast<CFieldHandlerNamePanel*>(m_panel);
    if (panel)
        panel->SetFieldNames(field_names);
}

template <typename T>
void CMacroAdvStringConstraintPanel<T>::SetMacroFieldNames(const vector<string>& field_names) 
{
    CFieldHandlerNamePanel *panel = dynamic_cast<CFieldHandlerNamePanel*>(m_panel);
    if (panel)
        panel->SetMacroFieldNames(field_names);
}

template <typename T>
void CMacroAdvStringConstraintPanel<T>::SetMacroSelf(const string& self) 
{
    CFieldHandlerNamePanel *panel = dynamic_cast<CFieldHandlerNamePanel*>(m_panel);
    if (panel)
        panel->SetMacroSelf(self);
}

END_NCBI_SCOPE
