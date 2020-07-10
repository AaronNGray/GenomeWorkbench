/*  $Id: suspect_product_rule_editor.hpp 39739 2017-10-31 18:00:13Z filippov $
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

#ifndef SUSPECT_PRODUCT_RULE_EDITOR__HPP
#define SUSPECT_PRODUCT_RULE_EDITOR__HPP


#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */


////@begin includes
////@end includes
#include <objects/macro/Suspect_rule.hpp>
#include <gui/widgets/edit/rearrange_list.hpp>
#include <objects/macro/String_constraint.hpp>
#include <objects/macro/Search_func.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/parse_text_options_dlg.hpp>
#include <gui/widgets/edit/edit_text_options_panel.hpp>
#include <gui/packages/pkg_sequence_edit/remove_text_outside_string_panel.hpp>
#include <gui/packages/pkg_sequence_edit/cap_change_panel.hpp>
#include <gui/widgets/edit/word_substitute_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/suspect_product_rules_constraint_panel.hpp>

#include <wx/valtext.h>
#include <wx/wxchar.h>
#include <wx/choicebk.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

#define ID_SUSPECT_PRODUCT_RULE wxID_ANY
#define SYMBOL_SUSPECT_PRODUCT_RULE_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_SUSPECT_PRODUCT_RULE_TITLE _("Suspect Product Rule Editor")
#define SYMBOL_SUSPECT_PRODUCT_RULE_IDNAME ID_SUSPECT_PRODUCT_RULE
#define SYMBOL_SUSPECT_PRODUCT_RULE_SIZE wxSize(704, 504)
#define SYMBOL_SUSPECT_PRODUCT_RULE_POSITION wxDefaultPosition

#define ID_LOAD_RULE_LIST_MENU 11401
#define ID_ADD_RULE_LIST_MENU 11402
#define ID_SAVE_RULE_LIST_MENU 11403
#define ID_SAVE_AS_RULE_LIST_MENU 11404
#define ID_QUIT_RULE_LIST_MENU 11405
#define ID_LIST_MATCHES_RULE_LIST_BUTTON 11406
#define ID_APPLY_FIXES_RULE_LIST_BUTTON 11407
#define ID_SHOW_DIFFS_RULE_LIST_BUTTON 11410
#define ID_DISPLAY_TEXT_RULE_LIST_BUTTON 11411
#define ID_ADD_BEFORE_RULE_LIST_BUTTON 11412
#define ID_ADD_AFTER_RULE_LIST_BUTTON 11413
#define ID_DELETE_SELECTED_RULE_LIST_BUTTON 11414
#define ID_SORT_FIND_RULE_LIST_MENU 11415
#define ID_SORT_CAT_RULE_LIST_MENU 11416

class  CSuspectProductRulesEditor : public wxFrame, public CCheckListBoxListener, public CDoubleClickConsumer
{    
    DECLARE_DYNAMIC_CLASS( CSuspectProductRulesEditor )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSuspectProductRulesEditor();

    CSuspectProductRulesEditor( wxWindow* parent, IWorkbench* workbench,
                                wxWindowID id = SYMBOL_SUSPECT_PRODUCT_RULE_IDNAME, const wxString& caption = SYMBOL_SUSPECT_PRODUCT_RULE_TITLE, const wxPoint& pos = SYMBOL_SUSPECT_PRODUCT_RULE_POSITION, const wxSize& size = SYMBOL_SUSPECT_PRODUCT_RULE_SIZE, long style = SYMBOL_SUSPECT_PRODUCT_RULE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_SUSPECT_PRODUCT_RULE_IDNAME, const wxString& caption = SYMBOL_SUSPECT_PRODUCT_RULE_TITLE, const wxPoint& pos = SYMBOL_SUSPECT_PRODUCT_RULE_POSITION, const wxSize& size = SYMBOL_SUSPECT_PRODUCT_RULE_SIZE, long style = SYMBOL_SUSPECT_PRODUCT_RULE_STYLE );

    /// Destructor
    virtual ~CSuspectProductRulesEditor();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    void OnFileLoad( wxCommandEvent& event );
    void OnFileAdd( wxCommandEvent& event );
    void OnFileSave( wxCommandEvent& event );
    void CanFileSave(wxUpdateUIEvent &event);
    void OnFileSaveAs( wxCommandEvent& event );
    void CanFileSaveAs(wxUpdateUIEvent &event);
    void OnQuit( wxCommandEvent& event );
    void CanSort(wxUpdateUIEvent &event);
    void OnSortByFind( wxCommandEvent& event );
    void OnSortByCat( wxCommandEvent& event );
    void OnDisplay( wxCommandEvent& event );
    void OnShowDiffs( wxCommandEvent& event );
    void OnApplyFixes( wxCommandEvent& event );
    void OnListMatches( wxCommandEvent& event );
    void OnDeleteRules( wxCommandEvent& event );
    void OnAddRuleBefore( wxCommandEvent& event );
    void OnAddRuleAfter( wxCommandEvent& event );

    virtual void UpdateCheckedState(size_t checked);
    void AddRule(CRef<CSuspect_rule> rule, bool before);
    void ReplaceRule(CRef<CSuspect_rule> rule, int n);
    virtual void DoubleClick(int n);

private:
    void x_SetUpTSEandUnDoManager();
    bool MatchConstraint(CRef<CSuspect_rule> rule, const string &str);

    IWorkbench*     m_Workbench;
    objects::CSeq_entry_Handle m_TopSeqEntry;
    ICommandProccessor* m_CmdProcessor;
    CRearrangeCtrl *m_Panel;
    wxString m_File;
    wxString m_DefaultDir;
    vector< CRef<CSuspect_rule> > m_Rules;
    wxButton *m_DeleteButton;
};


////@begin control identifiers
#define SYMBOL_CADD_SUSPECT_RULE_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CADD_SUSPECT_RULE_TITLE _("Suspect Rule")
#define SYMBOL_CADD_SUSPECT_RULE_IDNAME wxID_ANY
#define SYMBOL_CADD_SUSPECT_RULE_SIZE wxDefaultSize
#define SYMBOL_CADD_SUSPECT_RULE_POSITION wxDefaultPosition
////@end control identifiers


class CAddSuspectProductRule: public wxFrame
{    
    DECLARE_DYNAMIC_CLASS( CAddSuspectProductRule )
    DECLARE_EVENT_TABLE()

public:
    CAddSuspectProductRule();
    CAddSuspectProductRule( wxWindow* parent, bool before, objects::CSeq_entry_Handle seh,
                wxWindowID id = SYMBOL_CADD_SUSPECT_RULE_IDNAME, const wxString& caption = SYMBOL_CADD_SUSPECT_RULE_TITLE, const wxPoint& pos = SYMBOL_CADD_SUSPECT_RULE_POSITION, const wxSize& size = SYMBOL_CADD_SUSPECT_RULE_SIZE, long style = SYMBOL_CADD_SUSPECT_RULE_STYLE );

    bool Create( wxWindow* parent, 
                 wxWindowID id = SYMBOL_CADD_SUSPECT_RULE_IDNAME, const wxString& caption = SYMBOL_CADD_SUSPECT_RULE_TITLE, const wxPoint& pos = SYMBOL_CADD_SUSPECT_RULE_POSITION, const wxSize& size = SYMBOL_CADD_SUSPECT_RULE_SIZE, long style = SYMBOL_CADD_SUSPECT_RULE_STYLE );

    ~CAddSuspectProductRule();

    void Init();

    void CreateControls();

////@begin CAddSuspectProductRule event handler declarations

////@end CAddSuspectProductRule event handler declarations

////@begin CAddSuspectProductRule member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CAddSuspectProductRule member function declarations

    static bool ShowToolTips();

    void OnAccept( wxCommandEvent& event );
    void OnCancel( wxCommandEvent& event );

    void SetRule(CRef<CSuspect_rule> rule, int pos);
private:
    wxChoice *m_Type;
    wxChoicebook *m_Matches;
    wxChoicebook *m_DoesNotMatch;
    wxChoicebook *m_Action;
    wxCheckBox *m_OriginalToNote;
    wxCheckBox *m_Fatal;
    wxTextCtrl *m_Description;
    bool m_before;
    bool m_replace;
    CRef<CSuspect_rule> m_rule;
    int m_pos;
    CSuspectCompoundConstraintPanel* m_CompoundConstraintPanel;
    objects::CSeq_entry_Handle m_TopSeqEntry;
};

class CSearch_func_provider
{
public:
    virtual CRef<CSearch_func> GetSearchFunc() = 0;
    virtual void SetSearchFunc(const CSearch_func &search_func) = 0;
    virtual ~CSearch_func_provider() {}
};

////@begin control identifiers
#define SYMBOL_CMACROSTRINGCONSTRAINTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CMACROSTRINGCONSTRAINTPANEL_TITLE _("StringConstraintPanel")
#define SYMBOL_CMACROSTRINGCONSTRAINTPANEL_IDNAME wxID_ANY
#define SYMBOL_CMACROSTRINGCONSTRAINTPANEL_SIZE wxDefaultSize
#define SYMBOL_CMACROSTRINGCONSTRAINTPANEL_POSITION wxDefaultPosition
#define SYMBOL_CMACROSTRINGCONSTRAINTPANEL_CLEAR_ID 12010
#define SYMBOL_CMACROSTRINGCONSTRAINTPANEL_WORD_SUBST_ID 12011
////@end control identifiers


/*!
 * CMacroStringConstraintPanel class declaration
 */
class CMacroStringConstraintPanel:  public wxPanel, public CWordSubstitutionSetConsumer, public CSearch_func_provider
{    
    DECLARE_DYNAMIC_CLASS( CMacroStringConstraintPanel )
    DECLARE_EVENT_TABLE()
public:
    /// Constructors
    CMacroStringConstraintPanel();
    CMacroStringConstraintPanel( wxWindow* parent,
                            wxWindowID id = SYMBOL_CMACROSTRINGCONSTRAINTPANEL_IDNAME, 
                            const wxPoint& pos = SYMBOL_CMACROSTRINGCONSTRAINTPANEL_POSITION, 
                            const wxSize& size = SYMBOL_CMACROSTRINGCONSTRAINTPANEL_SIZE, 
                            long style = SYMBOL_CMACROSTRINGCONSTRAINTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CMACROSTRINGCONSTRAINTPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CMACROSTRINGCONSTRAINTPANEL_POSITION, 
        const wxSize& size = SYMBOL_CMACROSTRINGCONSTRAINTPANEL_SIZE, 
        long style = SYMBOL_CMACROSTRINGCONSTRAINTPANEL_STYLE );

    /// Destructor
    ~CMacroStringConstraintPanel();

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

    CRef<objects::CString_constraint> GetStringConstraint();

    virtual void AddWordSubstSet(CRef<objects::CWord_substitution_set> word_subst);
    virtual CRef<CSearch_func> GetSearchFunc();
    virtual void SetSearchFunc(const CSearch_func &search_func);
private:

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
    
    wxBoxSizer* m_TextSizer;
    bool m_is_choice;
    CRef<objects::CWord_substitution_set> m_word_subst;
};

class CMacroBlankConstraintPanel:  public wxPanel, public CSearch_func_provider
{
    wxDECLARE_NO_COPY_CLASS(CMacroBlankConstraintPanel);
public:
    /// Constructors
    CMacroBlankConstraintPanel();
    CMacroBlankConstraintPanel( wxWindow* parent, CRef<CSearch_func> search_func,
                                wxWindowID id = wxID_ANY,
                                const wxPoint& pos = wxDefaultPosition,
                                const wxSize& size = wxDefaultSize,
                                long style = wxTAB_TRAVERSAL)
        : m_Search_func(search_func)
        {
            wxPanel::Create( parent, id, pos, size, style );
        }
    
    /// Destructor
    virtual ~CMacroBlankConstraintPanel() {}
    
    virtual CRef<CSearch_func> GetSearchFunc() {return m_Search_func;}
    virtual void SetSearchFunc(const CSearch_func &search_func) {}
private:
    CRef<CSearch_func> m_Search_func;
};

class CMacroIntConstraintPanel:  public wxPanel, public CSearch_func_provider
{
    wxDECLARE_NO_COPY_CLASS( CMacroIntConstraintPanel );
public:
    /// Constructors
    CMacroIntConstraintPanel();
    CMacroIntConstraintPanel( wxWindow* parent, CRef<CSearch_func> search_func, int *val,
                                wxWindowID id = wxID_ANY,
                                const wxPoint& pos = wxDefaultPosition,
                                const wxSize& size = wxDefaultSize,
                                long style = wxTAB_TRAVERSAL)
        : m_Search_func(search_func), m_val(val)
        {
            wxPanel::Create( parent, id, pos, size, style );
            wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
            SetSizer(itemBoxSizer2);
            m_Text = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
            itemBoxSizer2->Add(m_Text, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
            m_Text->SetValidator( wxTextValidator( wxFILTER_NUMERIC ) );
            if (GetSizer())
            {
                GetSizer()->SetSizeHints(this);
            }
            Centre();
        }
    
    /// Destructor
    virtual ~CMacroIntConstraintPanel() {}
    
    virtual CRef<CSearch_func> GetSearchFunc() 
        {
            if (m_Text->GetValue().IsEmpty())
                return CRef<CSearch_func>(NULL);
            *m_val = wxAtoi(m_Text->GetValue());
            return m_Search_func;
        }
    virtual void SetSearchFunc(const CSearch_func &search_func)
        {
            m_Search_func->Assign(search_func);
            wxString val;
            m_val = NULL;
            if (m_Search_func->IsN_or_more_brackets_or_parentheses())
            {
                m_val = &m_Search_func->SetN_or_more_brackets_or_parentheses();
            }
            if (m_Search_func->IsToo_long())
            {
                m_val = &m_Search_func->SetToo_long();
            }
            if (m_val)
                val << *m_val;            
            m_Text->SetValue(val);
        }
private:
    CRef<CSearch_func> m_Search_func;
    int *m_val;
    wxTextCtrl *m_Text;
};

class CMacroStrConstraintPanel:  public wxPanel, public CSearch_func_provider
{
    wxDECLARE_NO_COPY_CLASS( CMacroStrConstraintPanel );
public:
    /// Constructors
    CMacroStrConstraintPanel();
    CMacroStrConstraintPanel( wxWindow* parent, CRef<CSearch_func> search_func, string *val,
                                wxWindowID id = wxID_ANY,
                                const wxPoint& pos = wxDefaultPosition,
                                const wxSize& size = wxDefaultSize,
                                long style = wxTAB_TRAVERSAL)
        : m_Search_func(search_func), m_val(val)
        {
            wxPanel::Create( parent, id, pos, size, style );
            wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
            SetSizer(itemBoxSizer2);
            m_Text = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
            itemBoxSizer2->Add(m_Text, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
            if (GetSizer())
            {
                GetSizer()->SetSizeHints(this);
            }
            Centre();
        }
    
    /// Destructor
    virtual ~CMacroStrConstraintPanel() {}
    
    virtual CRef<CSearch_func> GetSearchFunc() 
        {
            if (m_Text->GetValue().IsEmpty())
                return CRef<CSearch_func>(NULL);
            *m_val = NStr::TruncateSpaces(m_Text->GetValue().ToStdString());
            return m_Search_func;
        }
    virtual void SetSearchFunc(const CSearch_func &search_func)
        {
            m_Search_func->Assign(search_func);
            wxString val;
            m_val = NULL;
            if (m_Search_func->IsPrefix_and_numbers())
            {
                m_val = &m_Search_func->SetPrefix_and_numbers();
            }
            if (m_Search_func->IsHas_term())
            {
                m_val = &m_Search_func->SetHas_term();
            }
            if (m_val)
                val << *m_val;            
            m_Text->SetValue(val);
        }
private:
    CRef<CSearch_func> m_Search_func;
    string *m_val;
    wxTextCtrl *m_Text;
};

class CReplace_rule_provider
{
public:
    virtual CRef<CReplace_rule> GetReplace() = 0;
    virtual void SetReplace(const CReplace_rule &replace) = 0;
    virtual ~CReplace_rule_provider() {}
};

class CReplacementActionNone:  public wxPanel, public CReplace_rule_provider
{    
    DECLARE_DYNAMIC_CLASS( CReplacementActionNone )
    DECLARE_EVENT_TABLE()
public:
    CReplacementActionNone();
    CReplacementActionNone( wxWindow* parent,
                            wxWindowID id = wxID_ANY, 
                            const wxPoint& pos = wxDefaultPosition, 
                            const wxSize& size = wxDefaultSize,
                            long style = wxTAB_TRAVERSAL);

    bool Create( wxWindow* parent, 
                 wxWindowID id = wxID_ANY, 
                 const wxPoint& pos = wxDefaultPosition, 
                 const wxSize& size = wxDefaultSize,
                 long style = wxTAB_TRAVERSAL);

    ~CReplacementActionNone();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    virtual CRef<CReplace_rule> GetReplace();
    virtual void SetReplace(const CReplace_rule &replace) {}
};

class CReplacementActionSimple:  public wxPanel, public CReplace_rule_provider
{    
    DECLARE_DYNAMIC_CLASS( CReplacementActionSimple )
    DECLARE_EVENT_TABLE()
public:
    CReplacementActionSimple();
    CReplacementActionSimple( wxWindow* parent,
                            wxWindowID id = wxID_ANY, 
                            const wxPoint& pos = wxDefaultPosition, 
                            const wxSize& size = wxDefaultSize,
                            long style = wxTAB_TRAVERSAL);

    bool Create( wxWindow* parent, 
                 wxWindowID id = wxID_ANY, 
                 const wxPoint& pos = wxDefaultPosition, 
                 const wxSize& size = wxDefaultSize,
                 long style = wxTAB_TRAVERSAL);

    ~CReplacementActionSimple();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    virtual CRef<CReplace_rule> GetReplace();
    virtual void SetReplace(const CReplace_rule &replace);
private:
    wxTextCtrl *m_Text;
    wxCheckBox *m_Replace;
    wxCheckBox *m_Retain;
};

class CReplacementActionHaem:  public wxPanel, public CReplace_rule_provider
{    
    DECLARE_DYNAMIC_CLASS( CReplacementActionHaem )
    DECLARE_EVENT_TABLE()
public:
    CReplacementActionHaem();
    CReplacementActionHaem( wxWindow* parent,
                            wxWindowID id = wxID_ANY, 
                            const wxPoint& pos = wxDefaultPosition, 
                            const wxSize& size = wxDefaultSize,
                            long style = wxTAB_TRAVERSAL);

    bool Create( wxWindow* parent, 
                 wxWindowID id = wxID_ANY, 
                 const wxPoint& pos = wxDefaultPosition, 
                 const wxSize& size = wxDefaultSize,
                 long style = wxTAB_TRAVERSAL);

    ~CReplacementActionHaem();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    virtual CRef<CReplace_rule> GetReplace();
    virtual void SetReplace(const CReplace_rule &replace);
private:
    wxTextCtrl *m_Text;
};

class CReplacementActionHypothetical:  public wxPanel, public CReplace_rule_provider
{    
    DECLARE_DYNAMIC_CLASS( CReplacementActionHypothetical )
    DECLARE_EVENT_TABLE()
public:
    CReplacementActionHypothetical();
    CReplacementActionHypothetical( wxWindow* parent,
                            wxWindowID id = wxID_ANY, 
                            const wxPoint& pos = wxDefaultPosition, 
                            const wxSize& size = wxDefaultSize,
                            long style = wxTAB_TRAVERSAL);

    bool Create( wxWindow* parent, 
                 wxWindowID id = wxID_ANY, 
                 const wxPoint& pos = wxDefaultPosition, 
                 const wxSize& size = wxDefaultSize,
                 long style = wxTAB_TRAVERSAL);

    ~CReplacementActionHypothetical();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    virtual CRef<CReplace_rule> GetReplace();
    virtual void SetReplace(const CReplace_rule &replace) {}
};


END_NCBI_SCOPE

#endif  // SUSPECT_PRODUCT_RULE_EDITOR__HPP
