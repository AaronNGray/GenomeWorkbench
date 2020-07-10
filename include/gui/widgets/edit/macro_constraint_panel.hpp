#ifndef GUI_WIDGETS_EDIT___MACRO_CONSTRAINT_PANEL__HPP
#define GUI_WIDGETS_EDIT___MACRO_CONSTRAINT_PANEL__HPP
/*  $Id: macro_constraint_panel.hpp 40417 2018-02-08 17:15:41Z filippov $
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
#include <objects/macro/Constraint_choice_set.hpp>
#include <objects/macro/String_constraint.hpp>
#include <objects/macro/Location_constraint.hpp>
#include <gui/widgets/edit/cds_gene_prot_field_name_panel.hpp>
#include <gui/widgets/edit/rna_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/parse_text_options_dlg.hpp>
#include <gui/widgets/edit/edit_text_options_panel.hpp>
#include <gui/packages/pkg_sequence_edit/remove_text_outside_string_panel.hpp>
#include <gui/packages/pkg_sequence_edit/cap_change_panel.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>


/*!
 * Forward declarations
 */

////@begin forward declarations
class wxListbook;
class wxComboBox;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */
#define ID_CONSTRAINT_CHOICEBOOK 14002
#define ID_CONSTRAINT_ADD 14003
#define ID_CONSTRAINT_REMOVE 14004
#define ID_CONSTRAINT_LIST 14005


class  NCBI_GUIWIDGETS_EDIT_EXPORT CMacroCompoundConstraintPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS(CMacroCompoundConstraintPanel)
    DECLARE_EVENT_TABLE()

public:
    CMacroCompoundConstraintPanel();
    CMacroCompoundConstraintPanel(wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CMacroCompoundConstraintPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    void ClearValues(bool enable_add);
    void OnAddConstraint( wxCommandEvent& event );
    void OnRemoveConstraint( wxCommandEvent& event );
    void OnEditConstraint( wxCommandEvent& event );
    void AddConstraint(pair<string,string> item, int id, const vector<string> &vars = vector<string>(), const string &qual_name = kEmptyStr);
    void SetConstraints(const vector<string>& constraints);
    bool IsSetConstraints();
    vector<pair<string, string> > GetConstraints();
    vector<string> GetDescription();
    bool HasVariables();
    string GetVariables();
private:
    wxCheckListBox *m_CheckListBox;
    wxButton* m_Add;
    wxButton* m_Clear;
    objects::CSeq_entry_Handle m_TopSeqEntry;
    map<string, string> m_label_to_constraint;
    map<string, vector<string> > m_label_to_vars;
    map<string, string> m_label_to_qual_name;
    size_t m_num;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___MACRO_CONSTRAINT_PANEL__HPP
