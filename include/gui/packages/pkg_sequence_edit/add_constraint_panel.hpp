/*  $Id: add_constraint_panel.hpp 39739 2017-10-31 18:00:13Z filippov $
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

#ifndef ADD_CONSTRAINT_PANEL__HPP
#define ADD_CONSTRAINT_PANEL__HPP


#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */


////@begin includes
#include <wx/listbook.h>
#include <wx/choicebk.h>
#include <wx/checklst.h>
////@end includes
#include <objects/macro/String_constraint.hpp>
#include <objects/macro/Word_substitution_set.hpp>
#include <objects/macro/Word_substitution.hpp>
#include <gui/widgets/edit/cds_gene_prot_field_name_panel.hpp>
#include <gui/widgets/edit/rna_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/parse_text_options_dlg.hpp>
#include <gui/widgets/edit/edit_text_options_panel.hpp>
#include <gui/packages/pkg_sequence_edit/remove_text_outside_string_panel.hpp>
#include <gui/packages/pkg_sequence_edit/cap_change_panel.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/item_constraint_panel.hpp>
#include <gui/widgets/edit/word_substitute_dlg.hpp>

BEGIN_NCBI_SCOPE

////@begin control identifiers
#define SYMBOL_CADDCONSTRAINT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CADDCONSTRAINT_TITLE _("Constraint")
#define SYMBOL_CADDCONSTRAINT_IDNAME wxID_ANY
#define SYMBOL_CADDCONSTRAINT_SIZE wxDefaultSize
#define SYMBOL_CADDCONSTRAINT_POSITION wxDefaultPosition
////@end control identifiers


class CAddConstraint: public wxFrame, public CFieldNamePanelParent
{    
    DECLARE_DYNAMIC_CLASS( CAddConstraint )
    DECLARE_EVENT_TABLE()

public:
    CAddConstraint();
    CAddConstraint( wxWindow* parent, objects::CSeq_entry_Handle seh,
                wxWindowID id = SYMBOL_CADDCONSTRAINT_IDNAME, const wxString& caption = SYMBOL_CADDCONSTRAINT_TITLE, const wxPoint& pos = SYMBOL_CADDCONSTRAINT_POSITION, const wxSize& size = SYMBOL_CADDCONSTRAINT_SIZE, long style = SYMBOL_CADDCONSTRAINT_STYLE );

    bool Create( wxWindow* parent, 
                 wxWindowID id = SYMBOL_CADDCONSTRAINT_IDNAME, const wxString& caption = SYMBOL_CADDCONSTRAINT_TITLE, const wxPoint& pos = SYMBOL_CADDCONSTRAINT_POSITION, const wxSize& size = SYMBOL_CADDCONSTRAINT_SIZE, long style = SYMBOL_CADDCONSTRAINT_STYLE );

    ~CAddConstraint();

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

  // inherited from CFieldNamePanelParent
    virtual void UpdateEditor(void) {}

    void SetSelection(int page);   
    void SetEditId(int id) {m_id = id;}
    void SetConstraint(const SFieldTypeAndMatcher &constraint);
private:

////@begin CAddConstraint member variables
    wxChoicebook* m_Notebook;
    objects::CSeq_entry_Handle m_TopSeqEntry;
    int m_id;
};

class CMatcherPanelBase
{
public:
    virtual ~CMatcherPanelBase() {}
    virtual SFieldTypeAndMatcher GetMatcher(CFieldNamePanel::EFieldType field_type) = 0;
    virtual void SetMatcher(const SFieldTypeAndMatcher &item) = 0;
    virtual string GetLabel(void) = 0;
    virtual void SetFieldName(const string& field) {}
    virtual void PopulateFeatureListbox(objects::CSeq_entry_Handle seh) {}
    virtual void SetFieldNames(const vector<string>& field_names) {}
};

class CTripleConstraintPanel_CSourceFieldNamePanel_Wrapper : public wxPanel
{
    wxDECLARE_NO_COPY_CLASS(CTripleConstraintPanel_CSourceFieldNamePanel_Wrapper);
public:
    CTripleConstraintPanel_CSourceFieldNamePanel_Wrapper() : m_panel(NULL) {}
    CTripleConstraintPanel_CSourceFieldNamePanel_Wrapper( wxWindow* parent);

    ~CTripleConstraintPanel_CSourceFieldNamePanel_Wrapper() {}

 
    pair<string, SFieldTypeAndMatcher > GetItem(CFieldNamePanel::EFieldType field_type) {return m_panel->GetItem(field_type);}
private:
    CItemConstraintPanel *m_panel;
};

END_NCBI_SCOPE

#endif  // ADD_CONSTRAINT_PANEL__HPP
