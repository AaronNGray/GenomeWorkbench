#ifndef GUI_WIDGETS_EDIT___MACRO_SELECTQUAL_DLG__HPP
#define GUI_WIDGETS_EDIT___MACRO_SELECTQUAL_DLG__HPP
/*  $Id: macro_selectqual_dlg.hpp 44787 2020-03-13 19:03:59Z asztalos $
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
 * Authors:  Andrea Asztalos
 */

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>
#include <gui/widgets/edit/macro_editor_context.hpp>
#include <wx/dialog.h>
#include <wx/sizer.h>

class wxChoice;

BEGIN_NCBI_SCOPE

class CFieldNamePanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSELECTMATCHFIELDDLG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSELECTMATCHFIELDDLG_TITLE _("Select Qualifier")
#define SYMBOL_CSELECTMATCHFIELDDLG_IDNAME ID_CSELECTMATCHFIELDDLG
#define SYMBOL_CSELECTMATCHFIELDDLG_SIZE wxSize(413, 258)
#define SYMBOL_CSELECTMATCHFIELDDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSelectQualdDlg class declaration
 */

class CSelectQualdDlg : public wxDialog, public CFieldNamePanelParent
{    
    DECLARE_DYNAMIC_CLASS(CSelectQualdDlg)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSelectQualdDlg();
    CSelectQualdDlg( wxWindow* parent,
        EMacroFieldType field_type, EMacroFieldType match_type,
        wxWindowID id = SYMBOL_CSELECTMATCHFIELDDLG_IDNAME, 
        const wxString& caption = SYMBOL_CSELECTMATCHFIELDDLG_TITLE, 
        const wxPoint& pos = SYMBOL_CSELECTMATCHFIELDDLG_POSITION, 
        const wxSize& size = SYMBOL_CSELECTMATCHFIELDDLG_SIZE, 
        long style = SYMBOL_CSELECTMATCHFIELDDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSELECTMATCHFIELDDLG_IDNAME, 
        const wxString& caption = SYMBOL_CSELECTMATCHFIELDDLG_TITLE, 
        const wxPoint& pos = SYMBOL_CSELECTMATCHFIELDDLG_POSITION, 
        const wxSize& size = SYMBOL_CSELECTMATCHFIELDDLG_SIZE, 
        long style = SYMBOL_CSELECTMATCHFIELDDLG_STYLE );

    /// Destructor
    ~CSelectQualdDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSelectQualdDlg event handler declarations
    void OnSelect(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnFieldTypeSelected(wxCommandEvent& event);
////@end CSelectQualdDlg event handler declarations

////@begin CSelectQualdDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSelectQualdDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void SetFieldType(CFieldNamePanel::EFieldType field_type);
    EMacroFieldType GetMatchType() const { return m_MatchType; }
    SFieldFromTable GetSelection() const { return m_SelectedField; }
    
    virtual void UpdateEditor(void);
////@begin CSelectQualdDlg member variables
    /// Control identifiers
    enum {
        ID_CSELECTMATCHFIELDDLG = 11235,
        ID_SELECTMATCHFIELD
    };
////@end CSelectQualdDlg member variables
private:
    static CFieldNamePanel::EFieldType s_ConvertFieldtype(EMacroFieldType macro_field_type);
    void x_SetTarget();

    wxChoice* m_FieldChoice;
    wxBoxSizer* m_FieldPicker;
    CFieldNamePanel* m_Field;
    EMacroFieldType m_FieldType{ EMacroFieldType::eNotSet };
    EMacroFieldType m_MatchType{ EMacroFieldType::eNotSet };
    pair<string, string> m_Target;
    SFieldFromTable m_SelectedField;
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_SELECTQUAL_DLG__HPP

