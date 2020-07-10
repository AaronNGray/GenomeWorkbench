/*  $Id: string_constraint_panel.hpp 36143 2016-08-16 18:21:37Z filippov $
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
#ifndef _STRING_CONSTRAINT_PANEL_H_
#define _STRING_CONSTRAINT_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <objects/macro/String_constraint.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/combobox.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSTRINGCONSTRAINTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSTRINGCONSTRAINTPANEL_TITLE _("StringConstraintPanel")
#define SYMBOL_CSTRINGCONSTRAINTPANEL_IDNAME ID_CSTRINGCONSTRAINTPANEL
#define SYMBOL_CSTRINGCONSTRAINTPANEL_SIZE wxSize(400,300)
#define SYMBOL_CSTRINGCONSTRAINTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CStringConstraintPanel class declaration
 */

class CStringConstraintPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CStringConstraintPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CStringConstraintPanel();
    CStringConstraintPanel( wxWindow* parent, bool parentAECRDlg = false, 
                            wxWindowID id = SYMBOL_CSTRINGCONSTRAINTPANEL_IDNAME, 
                            const wxPoint& pos = SYMBOL_CSTRINGCONSTRAINTPANEL_POSITION, 
                            const wxSize& size = SYMBOL_CSTRINGCONSTRAINTPANEL_SIZE, 
                            long style = SYMBOL_CSTRINGCONSTRAINTPANEL_STYLE,
                            bool cap_constraints = false );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSTRINGCONSTRAINTPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSTRINGCONSTRAINTPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSTRINGCONSTRAINTPANEL_SIZE, 
        long style = SYMBOL_CSTRINGCONSTRAINTPANEL_STYLE );

    /// Destructor
    ~CStringConstraintPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CStringConstraintPanel event handler declarations
    void OnClear( wxCommandEvent& event );    
    void OnMatchTypeSelected( wxCommandEvent& event );
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
        eMatchType_DoesNotEndWith,

        eMatchType_Is_all_caps,
        eMatchType_Is_first_each_cap,
        eMatchType_IsFirstCap
    };

    CRef<edit::CStringConstraint> GetStringConstraint();
    bool Match(const string &str);

    void SetStringSelection(const wxString &str) { m_MatchType->SetStringSelection(str);}

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

////@begin CStringConstraintPanel member variables
    wxChoice* m_MatchType;
    wxTextCtrl* m_MatchText;
    wxCheckBox* m_IgnoreCase;
    wxCheckBox* m_IgnoreSpace;
    wxComboBox* m_MatchChoice;

////@end CStringConstraintPanel member variables
    
    enum {
        ID_CSTRINGCONSTRAINTPANEL = 6300,
        ID_STRING_CONSTRAINT_MATCH_CHOICE,
        ID_STRING_CONSTRAINT_MATCH_TEXT,
        ID_STRING_CONSTRAINT_CHECKBOX5,
        ID_STRING_CONSTRAINT_CHECKBOX6,
        ID_STRING_CONSTRAINT_CLEAR_BUTTON
    };
    /// flag is set if parent is the AECRDlg, for having specific features
    bool m_ParentAECRDlg; 

    bool m_CapConstraints;
    wxBoxSizer* m_TextSizer;
    bool m_is_choice;
};

END_NCBI_SCOPE

#endif
    // _STRING_CONSTRAINT_PANEL_H_
