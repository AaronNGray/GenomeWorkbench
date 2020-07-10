/*  $Id: field_constraint_panel.hpp 35122 2016-03-24 20:57:42Z filippov $
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
#ifndef _PKG_SEQUENCE_EDITFIELD_CONSTRAINT_PANEL_H_
#define _PKG_SEQUENCE_EDITFIELD_CONSTRAINT_PANEL_H_

#include <corelib/ncbistd.hpp>

#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <gui/packages/pkg_sequence_edit/field_choice_panel.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/choice.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class CStringConstraintPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CFIELDCONSTRAINTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CFIELDCONSTRAINTPANEL_TITLE _("FieldConstraintPanel")
#define SYMBOL_CFIELDCONSTRAINTPANEL_IDNAME ID_CFIELDCONSTRAINTPANEL
#define SYMBOL_CFIELDCONSTRAINTPANEL_SIZE wxDefaultSize
#define SYMBOL_CFIELDCONSTRAINTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CFieldConstraintPanel class declaration
 */

class CFieldConstraintPanel: public CFieldNamePanel , public CFieldNamePanelParent 
{    
    DECLARE_DYNAMIC_CLASS( CFieldConstraintPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFieldConstraintPanel();
    CFieldConstraintPanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CFIELDCONSTRAINTPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CFIELDCONSTRAINTPANEL_POSITION, 
        const wxSize& size = SYMBOL_CFIELDCONSTRAINTPANEL_SIZE, 
        long style = SYMBOL_CFIELDCONSTRAINTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CFIELDCONSTRAINTPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CFIELDCONSTRAINTPANEL_POSITION, 
        const wxSize& size = SYMBOL_CFIELDCONSTRAINTPANEL_SIZE, 
        long style = SYMBOL_CFIELDCONSTRAINTPANEL_STYLE );

    /// Destructor
    ~CFieldConstraintPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CFieldConstraintPanel event handler declarations

////@end CFieldConstraintPanel event handler declarations

////@begin CFieldConstraintPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFieldConstraintPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CFieldConstraintPanel member variables
    CStringConstraintPanel* m_StringConstraintPanel;
////@end CFieldConstraintPanel member variables
    CFieldChoicePanel* m_FieldType;

    enum {
        ID_CFIELDCONSTRAINTPANEL = 6320,
        ID_CFIELDCONSTRAINTPANEL_FIELD,
        ID_CFIELDCONSTRAINTPANEL_CONSTRAINT
    };
    bool SetFieldType(CFieldNamePanel::EFieldType field_type);
    CFieldNamePanel::EFieldType GetFieldType() { return m_FieldType->GetFieldType(); }
    const vector<CFieldNamePanel::EFieldType> GetFieldTypes() { return m_FieldType->GetFieldTypes(); }
    virtual string GetFieldName(const bool subfield = false);
    virtual bool SetFieldName(const string& field);
    virtual vector<string> GetChoices(bool& allow_other);

    CRef<edit::CStringConstraint> GetStringConstraint();
    virtual void UpdateEditor();
    CFieldChoicePanel* GetFieldChoicePanel(void) {return m_FieldType;}
};

END_NCBI_SCOPE

#endif
    // _PKG_SEQUENCE_EDITFIELD_CONSTRAINT_PANEL_H_
