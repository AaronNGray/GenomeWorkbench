/*  $Id: loc_constraint_panel.hpp 39649 2017-10-24 15:22:12Z asztalos $
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

#ifndef LOC_CONSTRAINT_PANEL__HPP
#define LOC_CONSTRAINT_PANEL__HPP


#include <corelib/ncbistd.hpp>
#include <objects/macro/Location_constraint.hpp>
#include <objects/macro/Location_pos_constraint.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/item_constraint_panel.hpp>
/*!
 * Includes
 */

////@begin includes
#include "wx/panel.h"
////@end includes

BEGIN_NCBI_SCOPE

struct SFieldTypeAndMatcher;

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CLOCATIONCONSTRAINT wxID_ANY
#define SYMBOL_CLOCATIONCONSTRAINT_STYLE wxTAB_TRAVERSAL 
#define SYMBOL_CLOCATIONCONSTRAINT_IDNAME ID_CLOCATIONCONSTRAINT
#define SYMBOL_CLOCATIONCONSTRAINT_SIZE wxDefaultSize
#define SYMBOL_CLOCATIONCONSTRAINT_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CLocationConstraintPanel class declaration
 */

class CLocationConstraintPanel: public wxPanel, public CItemConstraintPanel
{    
    DECLARE_CLASS( CLocationConstraintPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CLocationConstraintPanel();
    CLocationConstraintPanel( wxWindow* parent, wxWindowID id = SYMBOL_CLOCATIONCONSTRAINT_IDNAME, const wxPoint& pos = SYMBOL_CLOCATIONCONSTRAINT_POSITION, const wxSize& size = SYMBOL_CLOCATIONCONSTRAINT_SIZE, long style = SYMBOL_CLOCATIONCONSTRAINT_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CLOCATIONCONSTRAINT_IDNAME, const wxPoint& pos = SYMBOL_CLOCATIONCONSTRAINT_POSITION, const wxSize& size = SYMBOL_CLOCATIONCONSTRAINT_SIZE, long style = SYMBOL_CLOCATIONCONSTRAINT_STYLE );

    /// Destructor
    ~CLocationConstraintPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CLocationConstraintPanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for wxID_ANY
    void OnAnySelected( wxCommandEvent& event );

////@end CLocationConstraintPanel event handler declarations

////@begin CLocationConstraintPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CLocationConstraintPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual pair<string, SFieldTypeAndMatcher > GetItem(CFieldNamePanel::EFieldType field_type);
    virtual void SetItem(const SFieldTypeAndMatcher &item);

    CRef<objects::CLocation_constraint> GetLocationConstraint();
    string GetLabel();
private:
////@begin CLocationConstraintPanel member variables
    wxChoice* m_Strand;
    wxChoice* m_NucOrProt;
    wxChoice* m_5Partial;
    wxChoice* m_3Partial;
    wxChoice* m_LocType;
    wxChoice* m_5DistType;
    wxTextCtrl* m_5Dist;
    wxChoice* m_3DistType;
    wxTextCtrl* m_3Dist;
////@end CLocationConstraintPanel member variables
};

END_NCBI_SCOPE
#endif
    // _LOC_CONSTRAINT_PANEL_H_
