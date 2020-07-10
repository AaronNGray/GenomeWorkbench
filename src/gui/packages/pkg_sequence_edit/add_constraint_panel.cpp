/*  $Id: add_constraint_panel.cpp 45101 2020-05-29 20:53:24Z asztalos $
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

#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <gui/widgets/edit/source_field_name_panel.hpp>
#include <gui/widgets/edit/rna_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/misc_field_panel.hpp>
#include <gui/packages/pkg_sequence_edit/gene_field_panel.hpp>
#include <gui/packages/pkg_sequence_edit/protein_field_panel.hpp>
#include <gui/packages/pkg_sequence_edit/molinfoedit_util.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/edit/pub_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/pub_field.hpp>
#include <gui/packages/pkg_sequence_edit/dblink_field.hpp>
#include <gui/packages/pkg_sequence_edit/generalid_panel.hpp>
#include <gui/widgets/edit/struct_comm_field_panel.hpp>
#include <gui/widgets/edit/struct_fieldvalue_panel.hpp>
#include <gui/packages/pkg_sequence_edit/dbxref_name_panel.hpp>

#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>
#include <gui/packages/pkg_sequence_edit/aecr_frame.hpp>
#include <gui/packages/pkg_sequence_edit/constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/loc_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/add_constraint_panel.hpp>

BEGIN_NCBI_SCOPE

template <typename T>
class CTripleConstraintPanel: public wxPanel, public CItemConstraintPanel
{    
    wxDECLARE_NO_COPY_CLASS(CTripleConstraintPanel);
public:
    CTripleConstraintPanel();
    CTripleConstraintPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CTripleConstraintPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    
    virtual pair<string, SFieldTypeAndMatcher > GetItem(CFieldNamePanel::EFieldType field_type);
    virtual void SetItem(const SFieldTypeAndMatcher &item);

    void SetFieldName(const string& field);
    void PopulateFeatureListbox(objects::CSeq_entry_Handle seh);
    void SetFieldNames(const vector<string>& field_names);

private:
    wxNotebook* m_Notebook;
};

template<typename T>
class CPresentMatcherPanel : public CMatcherPanelBase, public wxPanel
{
   wxDECLARE_NO_COPY_CLASS( CPresentMatcherPanel );

public:
    CPresentMatcherPanel();
    CPresentMatcherPanel(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CPresentMatcherPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();

    virtual SFieldTypeAndMatcher GetMatcher(CFieldNamePanel::EFieldType field_type);
    virtual void SetMatcher(const SFieldTypeAndMatcher &item);
    virtual string GetLabel(void);
    virtual void SetFieldName(const string& field);
    virtual void PopulateFeatureListbox(objects::CSeq_entry_Handle seh);
    virtual void SetFieldNames(const vector<string>& field_names);
private:
    T *m_panel;
};

template<typename T>
class CSameMatcherPanel : public CMatcherPanelBase, public wxPanel
{
    wxDECLARE_NO_COPY_CLASS( CSameMatcherPanel );

public:
    CSameMatcherPanel();
    CSameMatcherPanel(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CSameMatcherPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();

    virtual SFieldTypeAndMatcher GetMatcher(CFieldNamePanel::EFieldType field_type);
    virtual void SetMatcher(const SFieldTypeAndMatcher &item);
    virtual string GetLabel(void);
    virtual void SetFieldName(const string& field);
    virtual void PopulateFeatureListbox(objects::CSeq_entry_Handle seh);
    virtual void SetFieldNames(const vector<string>& field_names);
private:
    T *m_panel1;
    T *m_panel2;
};

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
 * CAdvStringConstraintPanel class declaration
 */
template<typename T>
class CAdvStringConstraintPanel:  public CMatcherPanelBase, public wxPanel, public CFieldNamePanelParent, public CWordSubstitutionSetConsumer
{    
    wxDECLARE_NO_COPY_CLASS( CAdvStringConstraintPanel );
public:
    /// Constructors
    CAdvStringConstraintPanel();
    CAdvStringConstraintPanel( wxWindow* parent,
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
    ~CAdvStringConstraintPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CStringConstraintPanel event handler declarations
    void OnClear( wxCommandEvent& event );    
    void OnMatchTypeSelected( wxCommandEvent& event );
    void OnWordSubstitution( wxCommandEvent& event );
////@end CStringConstraintPanel event handler declarations

////@begin CStringConstraintPanel member function declarations

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
    void SetChoices(const vector<string> &choices);
    void ShowChoiceOrText(void);
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CStringConstraintPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual SFieldTypeAndMatcher GetMatcher(CFieldNamePanel::EFieldType field_type);
    virtual void SetMatcher(const SFieldTypeAndMatcher &item);
    virtual string GetLabel(void);


    // inherited from CFieldNamePanelParent
    virtual void UpdateEditor(void);

    CRef<objects::CString_constraint> GetStringConstraint();
    string GetDescription();

    virtual void AddWordSubstSet(CRef<objects::CWord_substitution_set> word_subst);
    virtual void SetFieldName(const string& field);
    virtual void PopulateFeatureListbox(objects::CSeq_entry_Handle seh);
    virtual void SetFieldNames(const vector<string>& field_names);
private:
    T *m_panel;

////@begin CStringConstraintPanel member variables
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
////@end CStringConstraintPanel member variables
    
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
        ID_STRING_CONSTRAINT_WORDSUBST_BUTTON
    };
    
    wxBoxSizer* m_TextSizer;
    bool m_is_choice;
    CRef<objects::CWord_substitution_set> m_word_subst;
};

IMPLEMENT_DYNAMIC_CLASS( CAddConstraint, wxFrame )

BEGIN_EVENT_TABLE( CAddConstraint, wxFrame )

////@begin CAddConstraint event table entries
    EVT_BUTTON( wxID_OK, CAddConstraint::OnAccept )
    EVT_BUTTON( wxID_CANCEL, CAddConstraint::OnCancel )
////@end CAddConstraint event table entries

END_EVENT_TABLE()

CAddConstraint::CAddConstraint()
{
    Init();
}

CAddConstraint::CAddConstraint( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh), m_id(-1)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CAddConstraint::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAddConstraint creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);
////@end CAddConstraint creation
    return true;
}

CAddConstraint::~CAddConstraint()
{
////@begin CAddConstraint destruction
////@end CAddConstraint destruction
}

void CAddConstraint::Init()
{
////@begin CAddConstraint member initialisation
    m_Notebook = NULL;
////@end CAddConstraint member initialisation
}

void CAddConstraint::CreateControls()
{    
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer1);

    wxPanel* itemCBulkCmdDlg1 = new wxPanel(this, wxID_ANY);
    itemBoxSizer1->Add(itemCBulkCmdDlg1, 1, wxGROW, 0);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    m_Notebook = new wxChoicebook( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCHB_TOP );

    itemBoxSizer2->Add(m_Notebook, 1, wxGROW|wxALL, 5);

    CTripleConstraintPanel<CSourceFieldNamePanel> *panel1 = new CTripleConstraintPanel<CSourceFieldNamePanel>(m_Notebook);
    m_Notebook->AddPage(panel1,_("Taxname"));
    panel1->SetFieldName("taxname");

    CTripleConstraintPanel<CSourceFieldNamePanel> *panel2 = new CTripleConstraintPanel<CSourceFieldNamePanel>(m_Notebook); 
    m_Notebook->AddPage(panel2,_("Source"));

    CTripleConstraintPanel<CFeatureFieldNamePanel> *panel3 =  new CTripleConstraintPanel<CFeatureFieldNamePanel>(m_Notebook);
    m_Notebook->AddPage(panel3,_("Feature"));
    panel3->PopulateFeatureListbox(m_TopSeqEntry);

    CTripleConstraintPanel<CCDSGeneProtFieldNamePanel> *panel4 = new CTripleConstraintPanel<CCDSGeneProtFieldNamePanel>(m_Notebook);
    m_Notebook->AddPage(panel4,_("CDS-Gene-Prot-mRNA"));
    panel4->SetFieldName("protein name");

    CTripleConstraintPanel<CRNAFieldNamePanel> *panel5 = new CTripleConstraintPanel<CRNAFieldNamePanel>(m_Notebook);
    m_Notebook->AddPage(panel5,_("RNA"));
    panel5->SetFieldName("product");

    CTripleConstraintPanel<CFieldHandlerNamePanel> *panel6 =  new CTripleConstraintPanel<CFieldHandlerNamePanel>(m_Notebook);
    panel6->SetFieldNames(CMolInfoField::GetFieldNames());
    m_Notebook->AddPage(panel6,_("MolInfo"));
    panel6->SetFieldName("molecule");

    CTripleConstraintPanel<CFieldHandlerNamePanel> *panel7 =  new CTripleConstraintPanel<CFieldHandlerNamePanel>(m_Notebook);
    panel7->SetFieldNames(CPubField::GetFieldNames());
    m_Notebook->AddPage(panel7,_("Pub"));
    panel7->SetFieldName("title");

    CTripleConstraintPanel<CStructCommentFieldPanel> *panel8 =  new CTripleConstraintPanel<CStructCommentFieldPanel>(m_Notebook);
    m_Notebook->AddPage(panel8,_("Structured Comment"));

    CTripleConstraintPanel<CFieldHandlerNamePanel> *panel9 =  new CTripleConstraintPanel<CFieldHandlerNamePanel>(m_Notebook);
    panel9->SetFieldNames(CDBLinkField::GetFieldNames());
    m_Notebook->AddPage(panel9,_("DBLink"));
    panel9->SetFieldName("title");

    CTripleConstraintPanel<CFieldHandlerNamePanel> *panel10 =  new CTripleConstraintPanel<CFieldHandlerNamePanel>(m_Notebook);
    panel10->SetFieldNames(CMiscFieldPanel::GetStrings());
    m_Notebook->AddPage(panel10,_("Misc"));
    panel10->SetFieldName(kDefinitionLineLabel);

    CLocationConstraintPanel *panel11 =  new CLocationConstraintPanel(m_Notebook);
    m_Notebook->AddPage(panel11,_("Location"));

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL, 0);

    wxButton* itemButton13 = new wxButton( itemCBulkCmdDlg1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemCBulkCmdDlg1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

}

bool CAddConstraint::ShowToolTips()
{
    return true;
}
wxBitmap CAddConstraint::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAddConstraint bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAddConstraint bitmap retrieval
}
wxIcon CAddConstraint::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAddConstraint icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAddConstraint icon retrieval
}

void CAddConstraint::OnCancel( wxCommandEvent& event )
{
    Close();
}

void CAddConstraint::OnAccept( wxCommandEvent& event )
{
    wxWindow* win = m_Notebook->GetCurrentPage();
    CItemConstraintPanel* panel = dynamic_cast<CItemConstraintPanel*>(win);
    pair<string, SFieldTypeAndMatcher > item = panel->GetItem(CFieldNamePanel::GetFieldTypeFromName(m_Notebook->GetPageText(m_Notebook->GetSelection()).ToStdString()));
    if (!item.second.enabled)
    {
        event.Skip();
        return;
    }
    item.second.top_level = m_Notebook->GetSelection();
    wxWindow* parent = GetParent();
    CCompoundConstraintPanel* compound_panel = dynamic_cast<CCompoundConstraintPanel*>(parent);
    compound_panel->AddConstraint(item, m_id);
    Close();
}

void CAddConstraint::SetSelection(int page)
{
    if (page != wxNOT_FOUND)
    {
        m_Notebook->ChangeSelection(page);     
    }
}

void CAddConstraint::SetConstraint(const SFieldTypeAndMatcher &constraint)
{
    m_Notebook->SetSelection(constraint.top_level);
    wxWindow* win = m_Notebook->GetCurrentPage();
    CItemConstraintPanel* panel = dynamic_cast<CItemConstraintPanel*>(win);
    panel->SetItem(constraint);
}

/*!
 * CTripleConstraintPanel constructors
 */
template <typename T>
CTripleConstraintPanel<T>::CTripleConstraintPanel()
{
    Init();
}

template <typename T>
CTripleConstraintPanel<T>::CTripleConstraintPanel( wxWindow* parent,  wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CTripleConstraintPanel creator
 */
template <typename T>
bool CTripleConstraintPanel<T>::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CTripleConstraintPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CTripleConstraintPanel creation
    return true;
}


/*!
 * CTripleConstraintPanel destructor
 */
template <typename T>
CTripleConstraintPanel<T>::~CTripleConstraintPanel()
{
////@begin CTripleConstraintPanel destruction
////@end CTripleConstraintPanel destruction
}


/*!
 * Member initialisation
 */
template <typename T>
void CTripleConstraintPanel<T>::Init()
{
////@begin CTripleConstraintPanel member initialisation
////@end CTripleConstraintPanel member initialisation
    m_Notebook = NULL;
}


/*!
 * Control creation for CTripleConstraintPanel
 */
template <typename T>
void CTripleConstraintPanel<T>::CreateControls()
{    
////@begin CTripleConstraintPanel content construction
    CTripleConstraintPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_Notebook = new wxNotebook(itemPanel1, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    itemBoxSizer2->Add(m_Notebook, 1, wxGROW|wxALL, 0);
    
    CPresentMatcherPanel<T>* panel1 = new CPresentMatcherPanel<T>(m_Notebook);
    m_Notebook->AddPage(panel1,_("When qualifier present"));

    CAdvStringConstraintPanel<T>* panel2 = new CAdvStringConstraintPanel<T>(m_Notebook);
    m_Notebook->AddPage(panel2,_("String constraint"));

    CSameMatcherPanel<T>* panel3 = new CSameMatcherPanel<T>(m_Notebook);
    m_Notebook->AddPage(panel3,_("When qualifiers match"));    
}

/*!
 * Should we show tooltips?
 */
template <typename T>
bool CTripleConstraintPanel<T>::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */
template <typename T>
wxBitmap CTripleConstraintPanel<T>::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CTripleConstraintPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CTripleConstraintPanel bitmap retrieval
}

/*!
 * Get icon resources
 */
template <typename T>
wxIcon CTripleConstraintPanel<T>::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CTripleConstraintPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CTripleConstraintPanel icon retrieval
}

template <typename T>
pair<string, SFieldTypeAndMatcher > CTripleConstraintPanel<T>::GetItem(CFieldNamePanel::EFieldType field_type)
{
    wxWindow *win =  m_Notebook->GetCurrentPage();
    SFieldTypeAndMatcher field_type_matcher;
    field_type_matcher.enabled = false;

    CMatcherPanelBase* matcher_panel = dynamic_cast<CMatcherPanelBase*>(win);
    if (!matcher_panel)
        return make_pair(kEmptyStr, field_type_matcher);

    string label = matcher_panel->GetLabel();
    field_type_matcher = matcher_panel->GetMatcher(field_type);
    field_type_matcher.second_level = m_Notebook->GetSelection();
    
    return make_pair(label, field_type_matcher);
}

template <typename T>
void CTripleConstraintPanel<T>::SetItem(const SFieldTypeAndMatcher &item)
{
    m_Notebook->SetSelection(item.second_level);
    wxWindow *win =  m_Notebook->GetCurrentPage();
    CMatcherPanelBase* matcher_panel = dynamic_cast<CMatcherPanelBase*>(win);
    matcher_panel->SetMatcher(item);
}

template <typename T>
void  CTripleConstraintPanel<T>::SetFieldName(const string& field)
{
    for (size_t i = 0; i < m_Notebook->GetPageCount(); i++)
    {
        wxWindow *win = m_Notebook->GetPage(i);
        CMatcherPanelBase* matcher_panel = dynamic_cast<CMatcherPanelBase*>(win);
        if (matcher_panel)
            matcher_panel->SetFieldName(field);
    }
}

template <typename T>
void  CTripleConstraintPanel<T>::PopulateFeatureListbox(objects::CSeq_entry_Handle seh)
{
    for (size_t i = 0; i < m_Notebook->GetPageCount(); i++)
    {
        wxWindow *win = m_Notebook->GetPage(i);
        CMatcherPanelBase* matcher_panel = dynamic_cast<CMatcherPanelBase*>(win);
        if (matcher_panel)
            matcher_panel->PopulateFeatureListbox(seh);
    }
}

template <typename T>
void  CTripleConstraintPanel<T>::SetFieldNames(const vector<string>& field_names)
{
    for (size_t i = 0; i < m_Notebook->GetPageCount(); i++)
    {
        wxWindow *win = m_Notebook->GetPage(i);
        CMatcherPanelBase* matcher_panel = dynamic_cast<CMatcherPanelBase*>(win);
        if (matcher_panel)
            matcher_panel->SetFieldNames(field_names);
    }
}


template<typename T>
CPresentMatcherPanel<T>::CPresentMatcherPanel()
{
    Init();
}

template<typename T>
CPresentMatcherPanel<T>::CPresentMatcherPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

template<typename T>
bool CPresentMatcherPanel<T>::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    return true;
}

template<typename T>
CPresentMatcherPanel<T>::~CPresentMatcherPanel()
{
}

template<typename T>
void CPresentMatcherPanel<T>::Init()
{
    m_panel = NULL;
}

template<typename T>
void CPresentMatcherPanel<T>::CreateControls()
{    
    CPresentMatcherPanel<T>* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer1);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_CENTER_HORIZONTAL, 0);
    
    m_panel= new T(itemPanel1);
    itemBoxSizer2->Add(m_panel, 0, wxALIGN_TOP|wxALL, 5);
}


template<typename T>
bool CPresentMatcherPanel<T>::ShowToolTips()
{
    return true;
}

template<typename T>
wxBitmap CPresentMatcherPanel<T>::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

template<typename T>
wxIcon CPresentMatcherPanel<T>::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}

template <typename T>
SFieldTypeAndMatcher CPresentMatcherPanel<T>::GetMatcher(CFieldNamePanel::EFieldType field_type)
{
    CFieldNamePanel* field_name_panel = dynamic_cast<CFieldNamePanel*>(m_panel);
    SFieldTypeAndMatcher field_type_matcher;
    string field = field_name_panel ? field_name_panel->GetFieldName(true) : CFieldNamePanel::GetFieldTypeName(field_type);
    field_type_matcher.field = field;
    field_type_matcher.subtype = GetSubtype(field_name_panel);
    field_type_matcher.field_type = field_type;
    field_type_matcher.matcher = CRef<CConstraintMatcher>(new CPresentConstraintMatcher);
    field_type_matcher.enabled = !field.empty();
    field_type_matcher.full_field = field_name_panel->GetFieldName(false);
    return field_type_matcher;
}

template <typename T>
void CPresentMatcherPanel<T>::SetMatcher(const SFieldTypeAndMatcher &matcher)
{
    m_panel->SetFieldName(matcher.full_field);
}

template <typename T>
string CPresentMatcherPanel<T>::GetLabel()
{
    CFieldNamePanel* field_name_panel = dynamic_cast<CFieldNamePanel*>(m_panel);
    if (field_name_panel)
    {
        string field = field_name_panel->GetFieldName(false);
        return "where " + field + " is present";
    }
    return kEmptyStr;
}

template <typename T>
void CPresentMatcherPanel<T>::SetFieldName(const string& field) 
{
    m_panel->SetFieldName(field);
}

template <typename T>
void CPresentMatcherPanel<T>::PopulateFeatureListbox(objects::CSeq_entry_Handle seh) 
{
    CFeatureFieldNamePanel *panel = dynamic_cast<CFeatureFieldNamePanel*>(m_panel);
    if (panel)
        panel->ListPresentFeaturesFirst(seh);
}

template <typename T>
void CPresentMatcherPanel<T>::SetFieldNames(const vector<string>& field_names) 
{
    CFieldHandlerNamePanel *panel = dynamic_cast<CFieldHandlerNamePanel*>(m_panel);
    if (panel)
        panel->SetFieldNames(field_names);
}


// Dual Source Panel

template<typename T>
CSameMatcherPanel<T>::CSameMatcherPanel()
{
    Init();
}

template<typename T>
CSameMatcherPanel<T>::CSameMatcherPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

template<typename T>
bool CSameMatcherPanel<T>::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    return true;
}

template<typename T>
CSameMatcherPanel<T>::~CSameMatcherPanel()
{
}

template<typename T>
void CSameMatcherPanel<T>::Init()
{
    m_panel1 = NULL;
    m_panel2 = NULL;
}

template<typename T>
void CSameMatcherPanel<T>::CreateControls()
{    
    CSameMatcherPanel<T>* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer1);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_CENTER_HORIZONTAL, 0);
    
    m_panel1 = new T(itemPanel1);
    itemBoxSizer2->Add(m_panel1, 0, wxALIGN_TOP|wxALL, 5);

    m_panel2 = new T(itemPanel1);
    itemBoxSizer2->Add(m_panel2, 0, wxALIGN_TOP|wxALL, 5);
}


template<typename T>
bool CSameMatcherPanel<T>::ShowToolTips()
{
    return true;
}

template<typename T>
wxBitmap CSameMatcherPanel<T>::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

template<typename T>
wxIcon CSameMatcherPanel<T>::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}

template <typename T>
SFieldTypeAndMatcher CSameMatcherPanel<T>::GetMatcher(CFieldNamePanel::EFieldType field_type)
{
    SFieldTypeAndMatcher field_type_matcher;
    CFieldNamePanel* field_name_panel1 = dynamic_cast<CFieldNamePanel*>(m_panel1);
    string field1 = field_name_panel1 ? field_name_panel1->GetFieldName(true) : CFieldNamePanel::GetFieldTypeName(field_type); 
    field_type_matcher.field = field1;
    field_type_matcher.subtype = GetSubtype(field_name_panel1);
    field_type_matcher.field_type = field_type;

    CFieldNamePanel* field_name_panel2 = dynamic_cast<CFieldNamePanel*>(m_panel2);
    string field2 = field_name_panel2 ? field_name_panel2->GetFieldName(true) : CFieldNamePanel::GetFieldTypeName(field_type); 
    field_type_matcher.matcher = CRef<CConstraintMatcher>(new CSameConstraintMatcher(field2, field_type, GetSubtype(field_name_panel2) ));
    field_type_matcher.enabled = !field1.empty()  && !field2.empty();

    field_type_matcher.field2 = field2;
    field_type_matcher.subtype2 = GetSubtype(field_name_panel2);
    field_type_matcher.field_type2 = field_type;
    field_type_matcher.matcher2 = CRef<CConstraintMatcher>(new CSameConstraintMatcher(field1, field_type, GetSubtype(field_name_panel1) ));
    field_type_matcher.enabled2 = !field1.empty()  && !field2.empty();

    field_type_matcher.full_field = field_name_panel1->GetFieldName(false);
    field_type_matcher.full_field2 = field_name_panel2->GetFieldName(false);

    return field_type_matcher;
}

template <typename T>
void CSameMatcherPanel<T>::SetMatcher(const SFieldTypeAndMatcher &matcher)
{
    m_panel1->SetFieldName(matcher.full_field);
    m_panel2->SetFieldName(matcher.full_field2);
}

template <typename T>
string CSameMatcherPanel<T>::GetLabel()
{
    CFieldNamePanel* field_name_panel1 = dynamic_cast<CFieldNamePanel*>(m_panel1);
    CFieldNamePanel* field_name_panel2 = dynamic_cast<CFieldNamePanel*>(m_panel2);
    if (field_name_panel1 && field_name_panel2)
    {
        string field1 = field_name_panel1->GetFieldName(false);
        string field2 = field_name_panel2->GetFieldName(false);
        return "where " + field1 + " matches " + field2;
    }
    return kEmptyStr;
}

template <typename T>
void CSameMatcherPanel<T>::SetFieldName(const string& field) 
{
    m_panel1->SetFieldName(field);
    m_panel2->SetFieldName(field);
}

template <typename T>
void CSameMatcherPanel<T>::PopulateFeatureListbox(objects::CSeq_entry_Handle seh) 
{
    CFeatureFieldNamePanel *panel1 = dynamic_cast<CFeatureFieldNamePanel*>(m_panel1);
    if (panel1)
        panel1->ListPresentFeaturesFirst(seh);
    CFeatureFieldNamePanel *panel2 = dynamic_cast<CFeatureFieldNamePanel*>(m_panel2);
    if (panel2)
        panel2->ListPresentFeaturesFirst(seh);
}

template <typename T>
void CSameMatcherPanel<T>::SetFieldNames(const vector<string>& field_names) 
{
    CFieldHandlerNamePanel *panel1 = dynamic_cast<CFieldHandlerNamePanel*>(m_panel1);
    if (panel1)
        panel1->SetFieldNames(field_names);
    CFieldHandlerNamePanel *panel2 = dynamic_cast<CFieldHandlerNamePanel*>(m_panel2);
    if (panel2)
        panel2->SetFieldNames(field_names);
}



/*!
 * CAdvStringConstraintPanel constructors
 */
template<typename T>
CAdvStringConstraintPanel<T>::CAdvStringConstraintPanel()
{
    Init();
}

template<typename T>
CAdvStringConstraintPanel<T>::CAdvStringConstraintPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CAdvStringConstraintPanel creator
 */
template<typename T>
bool CAdvStringConstraintPanel<T>::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAdvStringConstraintPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAdvStringConstraintPanel creation
    ShowChoiceOrText();
    
    return true;
}


/*!
 * CAdvStringConstraintPanel destructor
 */
template<typename T>
CAdvStringConstraintPanel<T>::~CAdvStringConstraintPanel()
{
////@begin CAdvStringConstraintPanel destruction
////@end CAdvStringConstraintPanel destruction
}


/*!
 * Member initialisation
 */
template<typename T>
void CAdvStringConstraintPanel<T>::Init()
{
////@begin CAdvStringConstraintPanel member initialisation
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
    m_is_choice = false;
////@end CAdvStringConstraintPanel member initialisation
}


/*!
 * Control creation for CAdvStringConstraintPanel
 */
template<typename T>
void CAdvStringConstraintPanel<T>::CreateControls()
{    
////@begin CAdvStringConstraintPanel content construction
    //CAdvStringConstraintPanel* itemPanel1 = this;
    wxPanel* parentPanel = this;
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    parentPanel->SetSizer(itemBoxSizer1);

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
    m_MatchType->Bind(wxEVT_CHOICE, &CAdvStringConstraintPanel<T>::OnMatchTypeSelected, this);
    
    m_TextSizer = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(m_TextSizer, 0, wxALIGN_TOP, 0);

    m_MatchText = new wxTextCtrl( parentPanel, ID_STRING_CONSTRAINT_MATCH_TEXT, wxEmptyString, wxDefaultPosition, wxSize(300, -1), wxTE_MULTILINE );
    m_TextSizer->Add(m_MatchText, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 5);   
    wxArrayString choice_strings;
    m_MatchChoice = new wxComboBox(parentPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), choice_strings, 0 );
    m_MatchChoice->Hide();


    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_IgnoreCase = new wxCheckBox( parentPanel, ID_STRING_CONSTRAINT_CHECKBOX5, _("Ignore Case"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreCase->SetValue(false);
    itemBoxSizer6->Add(m_IgnoreCase, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_IgnoreSpace = new wxCheckBox( parentPanel, ID_STRING_CONSTRAINT_CHECKBOX6, _("Ignore Space"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreSpace->SetValue(false);
    itemBoxSizer6->Add(m_IgnoreSpace, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_IgnorePunct = new wxCheckBox( parentPanel, ID_STRING_CONSTRAINT_CHECKBOX8, _("Ignore Punctuation"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnorePunct->SetValue(false);
    itemBoxSizer6->Add(m_IgnorePunct, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);    

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_WholeWord = new wxCheckBox( parentPanel, ID_STRING_CONSTRAINT_CHECKBOX7, _("Whole Word"), wxDefaultPosition, wxDefaultSize, 0 );
    m_WholeWord->SetValue(false);
    itemBoxSizer7->Add(m_WholeWord, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);   

    m_IgnoreSyn = new wxCheckBox( parentPanel, ID_STRING_CONSTRAINT_CHECKBOX9, _("Ignore 'putative' synonyms"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreSyn->SetValue(false);
    itemBoxSizer7->Add(m_IgnoreSyn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_AnyLetters = new wxRadioButton( parentPanel, ID_STRING_CONSTRAINT_RADIO0, _("Any letters"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AnyLetters->SetValue(true);
    itemBoxSizer8->Add(m_AnyLetters, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AllUpper = new wxRadioButton( parentPanel, ID_STRING_CONSTRAINT_RADIO1, _("All letters are upper case"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AllUpper->SetValue(false);
    itemBoxSizer8->Add(m_AllUpper, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AllLower = new wxRadioButton( parentPanel, ID_STRING_CONSTRAINT_RADIO2, _("All letters are lower case"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AllLower->SetValue(false);
    itemBoxSizer8->Add(m_AllLower, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AllPunct = new wxRadioButton( parentPanel, ID_STRING_CONSTRAINT_RADIO3, _("All characters are punctuation"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AllPunct->SetValue(false);
    itemBoxSizer8->Add(m_AllPunct, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
   
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ClearButton = new wxButton( parentPanel, ID_STRING_CONSTRAINT_CLEAR_BUTTON, _("Clear Constraint"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(m_ClearButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    //m_ClearButton->Bind(wxEVT_BUTTON, &CAdvStringConstraintPanel<T>::OnClear, this);
    m_ClearButton->Connect(ID_STRING_CONSTRAINT_CLEAR_BUTTON, wxEVT_BUTTON, wxCommandEventHandler(CAdvStringConstraintPanel<T>::OnClear), NULL, this);

    m_WordSubst = new wxButton( parentPanel, ID_STRING_CONSTRAINT_WORDSUBST_BUTTON, _("Word Substitutions"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(m_WordSubst, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    //m_WordSubst->Bind(wxEVT_BUTTON, &CAdvStringConstraintPanel<T>::OnWordSubstitution, this); 
    m_WordSubst->Connect(ID_STRING_CONSTRAINT_WORDSUBST_BUTTON, wxEVT_BUTTON, wxCommandEventHandler(CAdvStringConstraintPanel<T>::OnWordSubstitution), NULL, this);

    ////@end CAdvStringConstraintPanel content construction

    CFieldNamePanel* panel = dynamic_cast<CFieldNamePanel*>(m_panel);
    if (panel)
    {
        bool allow_other = true;
        vector<string> choices = panel->GetChoices(allow_other); 
        SetChoices(choices);      
    }
}

template<typename T>
void CAdvStringConstraintPanel<T>::OnClear( wxCommandEvent& event )
{
    ClearValues();  
}

template<typename T>
void CAdvStringConstraintPanel<T>::ClearValues()
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
void CAdvStringConstraintPanel<T>::SetChoices(const vector<string> &choices)
{
    wxArrayString choice_strings;
    ITERATE(vector<string>, it, choices) 
    {
        choice_strings.Add(ToWxString(*it));
    }
    m_MatchChoice->Set(choice_strings);
    ShowChoiceOrText();
}

template<typename T>
void CAdvStringConstraintPanel<T>::ShowChoiceOrText()
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
void CAdvStringConstraintPanel<T>::OnMatchTypeSelected( wxCommandEvent& event )
{
    ShowChoiceOrText();
    event.Skip();
}

/*!
 * Should we show tooltips?
 */
template<typename T>
bool CAdvStringConstraintPanel<T>::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */
template<typename T>
wxBitmap CAdvStringConstraintPanel<T>::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAdvStringConstraintPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAdvStringConstraintPanel bitmap retrieval
}

/*!
 * Get icon resources
 */
template<typename T>
wxIcon CAdvStringConstraintPanel<T>::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAdvStringConstraintPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAdvStringConstraintPanel icon retrieval
}

template <typename T>
void CAdvStringConstraintPanel<T>::UpdateEditor()
{
    CFieldNamePanel* panel = dynamic_cast<CFieldNamePanel*>(m_panel);
    if (panel)
    {
        bool allow_other = true;
        vector<string> choices = panel->GetChoices(allow_other); 
        SetChoices(choices);      
    }
}

template <typename T>
SFieldTypeAndMatcher CAdvStringConstraintPanel<T>::GetMatcher(CFieldNamePanel::EFieldType field_type)
{
    CFieldNamePanel* field_name_panel = dynamic_cast<CFieldNamePanel*>(m_panel);
    SFieldTypeAndMatcher field_type_matcher;
    string field = field_name_panel ? field_name_panel->GetFieldName(true) : CFieldNamePanel::GetFieldTypeName(field_type); 
    field_type_matcher.field = field;
    field_type_matcher.subtype = GetSubtype(field_name_panel);
    field_type_matcher.field_type = field_type;
    CRef<CString_constraint> sc = GetStringConstraint();
    field_type_matcher.matcher =  CRef<CConstraintMatcher>(new CAdvancedConstraintMatcher(sc));
    field_type_matcher.enabled = !field.empty() && !sc->Empty(); 
    field_type_matcher.full_field = field_name_panel->GetFieldName(false);
    field_type_matcher.string_constraint = sc;
    return field_type_matcher;
}

template <typename T>
string CAdvStringConstraintPanel<T>::GetLabel()
{
    CFieldNamePanel* field_name_panel = dynamic_cast<CFieldNamePanel*>(m_panel);
    if (field_name_panel)
    {
        string field = field_name_panel->GetFieldName(false);
        return "where " + field + " " + GetDescription();
    }
    return kEmptyStr;
}

template <typename T>
CRef<CString_constraint> CAdvStringConstraintPanel<T>::GetStringConstraint()
{
    string match_text; 
    if (!m_is_choice)
        match_text = NStr::Replace(NStr::Replace(ToStdString(m_MatchText->GetValue()),"\r"," "),"\n"," ");
    else 
        match_text = NStr::Replace(NStr::Replace(ToStdString(m_MatchChoice->GetValue()),"\r"," "),"\n"," ");

    CRef<CString_constraint> c(new CString_constraint);
    c->SetMatch_text(match_text);
    c->SetCase_sensitive(!m_IgnoreCase->GetValue());
    c->SetIgnore_space(m_IgnoreSpace->GetValue());
    c->SetIgnore_punct(m_IgnorePunct->GetValue());
    c->SetWhole_word(m_WholeWord->GetValue());
    c->SetIs_all_caps(m_AllUpper->GetValue());
    c->SetIs_all_lower(m_AllLower->GetValue());
    c->SetIs_all_punct(m_AllPunct->GetValue());
    c->SetIgnore_weasel(m_IgnoreSyn->GetValue());

    if (m_word_subst && m_word_subst->IsSet() && !m_word_subst->Get().empty())
    {
        c->SetIgnore_words(*m_word_subst);
    }

    switch(m_MatchType->GetSelection()) {
        case eMatchType_Contains:
            c->SetMatch_location(eString_location_contains);
            break;
        case eMatchType_DoesNotContain:
            c->SetMatch_location(eString_location_contains);
            c->SetNot_present(true);
            break;
        case eMatchType_Equals:
            c->SetMatch_location(eString_location_equals);
            break;
        case eMatchType_DoesNotEqual:
            c->SetMatch_location(eString_location_equals);
            c->SetNot_present(true);
            break;
        case eMatchType_StartsWith:
            c->SetMatch_location(eString_location_starts);
            break;
        case eMatchType_EndsWith:
            c->SetMatch_location(eString_location_ends);
            break;
        case eMatchType_IsOneOf:
            c->SetMatch_location(eString_location_inlist);
            break;
        case eMatchType_IsNotOneOf:
            c->SetMatch_location(eString_location_inlist);
            c->SetNot_present(true);
            break;
        case eMatchType_DoesNotStartWith:
            c->SetMatch_location(eString_location_starts);
            c->SetNot_present(true);
            break;
        case eMatchType_DoesNotEndWith:
            c->SetMatch_location(eString_location_ends);
            c->SetNot_present(true);
            break;
    default: c.Reset(); break;
    }
  
    return c;
}

template <typename T>
string CAdvStringConstraintPanel<T>::GetDescription()
{
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

    return str;
}

template <typename T>
void CAdvStringConstraintPanel<T>::OnWordSubstitution( wxCommandEvent& event )
{
    CWordSubstitutionDlg * dlg = new CWordSubstitutionDlg(this, m_word_subst); 
    dlg->Show(true);
}

template <typename T>
void CAdvStringConstraintPanel<T>::AddWordSubstSet(CRef<objects::CWord_substitution_set> word_subst)
{
    m_word_subst = word_subst;
}

template <typename T>
void CAdvStringConstraintPanel<T>::SetFieldName(const string& field) 
{
    m_panel->SetFieldName(field);
}

template <typename T>
void CAdvStringConstraintPanel<T>::SetMatcher(const SFieldTypeAndMatcher &item)
{
    m_panel->SetFieldName(item.full_field);
    CRef<CString_constraint> c = item.string_constraint;

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
void CAdvStringConstraintPanel<T>::PopulateFeatureListbox(objects::CSeq_entry_Handle seh) 
{
    CFeatureFieldNamePanel *panel = dynamic_cast<CFeatureFieldNamePanel*>(m_panel);
    if (panel)
        panel->ListPresentFeaturesFirst(seh);
}

template <typename T>
void CAdvStringConstraintPanel<T>::SetFieldNames(const vector<string>& field_names) 
{
    CFieldHandlerNamePanel *panel = dynamic_cast<CFieldHandlerNamePanel*>(m_panel);
    if (panel)
        panel->SetFieldNames(field_names);
}

CTripleConstraintPanel_CSourceFieldNamePanel_Wrapper::CTripleConstraintPanel_CSourceFieldNamePanel_Wrapper(wxWindow *parent)
{
    wxPanel::Create( parent);
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    SetSizer(itemBoxSizer2);
    CTripleConstraintPanel<CSourceFieldNamePanel> *win = new CTripleConstraintPanel<CSourceFieldNamePanel>(this); 
    itemBoxSizer2->Add(win, 1, wxGROW|wxALL, 0);
    m_panel = win;

    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
}

END_NCBI_SCOPE
