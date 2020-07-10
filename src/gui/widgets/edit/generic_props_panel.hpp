#ifndef GUI_WIDGETS_EDIT___GENERIC_PROPS_PANEL__HPP
#define GUI_WIDGETS_EDIT___GENERIC_PROPS_PANEL__HPP

/*  $Id: generic_props_panel.hpp 42689 2019-04-03 15:09:17Z filippov $
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
 * Authors:  Roman Katargin
 */

#include <corelib/ncbiobj.hpp>

/*!
 * Includes
 */

#include <wx/panel.h>
#include <wx/radiobox.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/treebook.h>

////@begin includes
////@end includes

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CSeq_feat;
    class CScope;
END_SCOPE(objects)

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CGENERICPROPSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CGENERICPROPSPANEL_TITLE _("Gene Properties")
#define SYMBOL_CGENERICPROPSPANEL_IDNAME ID_CGENEPROPSPANEL
#define SYMBOL_CGENERICPROPSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CGENERICPROPSPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CGenericPropsPanel class declaration
 */

class CGenericPropsPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CGenericPropsPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CGenericPropsPanel();
    CGenericPropsPanel( wxWindow* parent, CObject& object, objects::CScope& scope,
                        wxWindowID id = SYMBOL_CGENERICPROPSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CGENERICPROPSPANEL_POSITION, const wxSize& size = SYMBOL_CGENERICPROPSPANEL_SIZE, long style = SYMBOL_CGENERICPROPSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CGENERICPROPSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CGENERICPROPSPANEL_POSITION, const wxSize& size = SYMBOL_CGENERICPROPSPANEL_SIZE, long style = SYMBOL_CGENERICPROPSPANEL_STYLE );

    /// Destructor
    ~CGenericPropsPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    bool TransferDataFromWindow();

////@begin CGenericPropsPanel event handler declarations

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_EXPLANATION
    void OnExplanationSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_EXPLANATION
    void OnExplanationUpdated( wxCommandEvent& event );

    void OnPseudoChanged( wxCommandEvent& event );

////@end CGenericPropsPanel event handler declarations

////@begin CGenericPropsPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGenericPropsPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CGenericPropsPanel member variables
    wxChoice* m_PseudogeneChoice;
    wxComboBox* m_Explanation;
    wxRadioBox* m_Evidence;
    wxCheckBox* m_Partial;
    /// Control identifiers
    enum {
        ID_CGENEPROPSPANEL = 10009,
        ID_PSEUDOGENE_CHOICE = 10172,
        ID_EXPLANATION = 10014,
        ID_RADIOBOX3 = 10010,
        ID_RADIOBOX2 = 10002
    };
////@end CGenericPropsPanel member variables
    bool IsPseudo();
    bool PseudoSelectionChanged() const;
    string GetPseudoChoice();
    static void ModifyPseudo(objects::CSeq_feat &feat, const string &pseudo_choice);
    void ResetPseudo();

private:
    objects::CSeq_feat* m_Object;
    objects::CScope* m_Scope;

    bool x_IsGene() const;
    bool x_IsRefSeq();
    bool x_IsCds() const;
    wxTreebook* x_GetTree();
    void x_UpdateDisableOnRetranslate();
    wxString m_OriginalPseudoSelection;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___GENERIC_PROPS_PANEL__HPP
