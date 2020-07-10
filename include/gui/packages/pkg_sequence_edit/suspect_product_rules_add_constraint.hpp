#ifndef GUI_PKG_EDIT_SUSPECT_ADD_CONSTRAINT_PANEL__HPP
#define GUI_PKG_EDIT_SUSPECT_ADD_CONSTRAINT_PANEL__HPP
/*  $Id: suspect_product_rules_add_constraint.hpp 39741 2017-10-31 20:17:38Z filippov $
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


#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */


////@begin includes
#include <wx/listbook.h>
#include <wx/choicebk.h>
#include <wx/checklst.h>
////@end includes
#include <objects/macro/Word_substitution.hpp>
#include <objects/macro/Word_substitution_set.hpp>
#include <objects/macro/Word_substitution_set.hpp>
#include <objects/macro/String_constraint.hpp>
#include <gui/widgets/edit/word_substitute_dlg.hpp>

BEGIN_NCBI_SCOPE

////@begin control identifiers
#define SYMBOL_CMACROADDCONSTRAINT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CMACROADDCONSTRAINT_TITLE _("Constraint")
#define SYMBOL_CMACROADDCONSTRAINT_IDNAME wxID_ANY
#define SYMBOL_CMACROADDCONSTRAINT_SIZE wxDefaultSize
#define SYMBOL_CMACROADDCONSTRAINT_POSITION wxDefaultPosition
////@end control identifiers


class CSuspectAddConstraint: public wxFrame, public CWordSubstitutionSetConsumer 
{    
    DECLARE_DYNAMIC_CLASS(CSuspectAddConstraint)
    DECLARE_EVENT_TABLE()

public:
    CSuspectAddConstraint();
    CSuspectAddConstraint(wxWindow* parent, 
        wxWindowID id = SYMBOL_CMACROADDCONSTRAINT_IDNAME, 
        const wxString& caption = SYMBOL_CMACROADDCONSTRAINT_TITLE, 
        const wxPoint& pos = SYMBOL_CMACROADDCONSTRAINT_POSITION, 
        const wxSize& size = SYMBOL_CMACROADDCONSTRAINT_SIZE, 
        long style = SYMBOL_CMACROADDCONSTRAINT_STYLE);

    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CMACROADDCONSTRAINT_IDNAME, 
        const wxString& caption = SYMBOL_CMACROADDCONSTRAINT_TITLE, 
        const wxPoint& pos = SYMBOL_CMACROADDCONSTRAINT_POSITION,
        const wxSize& size = SYMBOL_CMACROADDCONSTRAINT_SIZE,
        long style = SYMBOL_CMACROADDCONSTRAINT_STYLE);

    ~CSuspectAddConstraint();

    void Init();

    void CreateControls();

////@begin CAddConstraint event handler declarations

////@end CAddConstraint event handler declarations

////@begin CAddConstraint member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CAddConstraint member function declarations

    static bool ShowToolTips();

    void OnAccept( wxCommandEvent& event );
    void OnCancel( wxCommandEvent& event );

    void SetConstraint(const objects::CConstraint_choice &constraint);

///@begin CMacroAdvStringConstraintPanel event handler declarations
    void OnClear( wxCommandEvent& event );    
    void OnMatchTypeSelected( wxCommandEvent& event );
    void OnWordSubstitution( wxCommandEvent& event );
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
    void SetChoices(const vector<string> &choices);
    void ShowChoiceOrText(void);
    CRef<objects::CString_constraint> GetStringConstraint();
    string GetDescription();
    static string GetDescription(CRef<objects::CString_constraint> c);
    static string GetDescription(const objects::CConstraint_choice &constraint);
    virtual void AddWordSubstSet(CRef<objects::CWord_substitution_set> word_subst);
    void SetEditId(int id) {m_id = id;}
private:
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
        ID_STRING_CONSTRAINT_WORDSUBST_BUTTON
    };
    
    wxBoxSizer* m_TextSizer;
    bool m_is_choice;
    CRef<objects::CWord_substitution_set> m_word_subst;
    int m_id;
};


END_NCBI_SCOPE

#endif  // GUI_PKG_EDIT_SUSPECT_ADD_CONSTRAINT_PANEL__HPP
