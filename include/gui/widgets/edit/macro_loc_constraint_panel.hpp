#ifndef GUI_WIDGETS_EDIT___MACRO_LOC_CONSTRAINT_PANEL__HPP
#define GUI_WIDGETS_EDIT___MACRO_LOC_CONSTRAINT_PANEL__HPP
/*  $Id: macro_loc_constraint_panel.hpp 40627 2018-03-21 15:34:12Z asztalos $
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

#include <objects/macro/Location_constraint.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>
#include <gui/widgets/edit/macro_item_constraint_panel.hpp>
#include <wx/panel.h>

class wxChoice;
class wxTextCtrl;

BEGIN_NCBI_SCOPE


/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CMACROLOCATIONCONSTRAINT wxID_ANY
#define SYMBOL_CMACROLOCATIONCONSTRAINT_STYLE wxTAB_TRAVERSAL 
#define SYMBOL_CMACROLOCATIONCONSTRAINT_IDNAME ID_CMACROLOCATIONCONSTRAINT
#define SYMBOL_CMACROLOCATIONCONSTRAINT_SIZE wxDefaultSize
#define SYMBOL_CMACROLOCATIONCONSTRAINT_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CMacroLocationConstraintPanel class declaration
 */

class CMacroLocationConstraintPanel : public wxPanel, public CMacroMatcherPanelBase
{    
    DECLARE_CLASS(CMacroLocationConstraintPanel)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMacroLocationConstraintPanel();
    CMacroLocationConstraintPanel(wxWindow* parent, 
        wxWindowID id = SYMBOL_CMACROLOCATIONCONSTRAINT_IDNAME,
        const wxPoint& pos = SYMBOL_CMACROLOCATIONCONSTRAINT_POSITION,
        const wxSize& size = SYMBOL_CMACROLOCATIONCONSTRAINT_SIZE,
        long style = SYMBOL_CMACROLOCATIONCONSTRAINT_STYLE);

    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CMACROLOCATIONCONSTRAINT_IDNAME,
        const wxPoint& pos = SYMBOL_CMACROLOCATIONCONSTRAINT_POSITION,
        const wxSize& size = SYMBOL_CMACROLOCATIONCONSTRAINT_SIZE,
        long style = SYMBOL_CMACROLOCATIONCONSTRAINT_STYLE);

    /// Destructor
    ~CMacroLocationConstraintPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CMacroLocationConstraintPanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for wxID_ANY
    void OnAnySelected( wxCommandEvent& event );

////@end CMacroLocationConstraintPanel event handler declarations

////@begin CMacroLocationConstraintPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMacroLocationConstraintPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual pair<string, string> GetMatcher(const pair<string, string>& target, size_t num);
    virtual void SetMatcher(const string &item);

    string GetLocationConstraint(const string &location);
    string GetDescription() const;
private:
////@begin CMacroLocationConstraintPanel member variables
    wxChoice* m_Strand;
    wxChoice* m_NucOrProt;
    wxChoice* m_5Partial;
    wxChoice* m_3Partial;
    wxChoice* m_LocType;
    wxChoice* m_5DistType;
    wxTextCtrl* m_5Dist;
    wxChoice* m_3DistType;
    wxTextCtrl* m_3Dist;
////@end CMacroLocationConstraintPanel member variables
};

END_NCBI_SCOPE
#endif
    // GUI_WIDGETS_EDIT___MACRO_LOC_CONSTRAINT_PANEL__HPP
