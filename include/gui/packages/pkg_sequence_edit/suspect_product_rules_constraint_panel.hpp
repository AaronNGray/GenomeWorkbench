#ifndef GUI_PKG_EDIT_SUSPECT_CONSTRAINT_PANEL__HPP
#define GUI_PKG_EDIT_SUSPECT_CONSTRAINT_PANEL__HPP
/*  $Id: suspect_product_rules_constraint_panel.hpp 39741 2017-10-31 20:17:38Z filippov $
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
#include <objects/macro/Constraint_choice.hpp>


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


class CSuspectCompoundConstraintPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS(CSuspectCompoundConstraintPanel)
    DECLARE_EVENT_TABLE()

public:
    CSuspectCompoundConstraintPanel();
    CSuspectCompoundConstraintPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CSuspectCompoundConstraintPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    void ClearValues(void);

    void OnAddConstraint( wxCommandEvent& event );
    void OnRemoveConstraint( wxCommandEvent& event );
    void OnEditConstraint( wxCommandEvent& event );
    void AddConstraint(pair<string, CRef<objects::CConstraint_choice> > item, int id);
    void SetConstraints(const objects::CConstraint_choice_set& constraints);
    bool IsSetConstraints();
    CRef<objects::CConstraint_choice_set> GetConstraints();
private:
    wxCheckListBox *m_CheckListBox;
    map<string, CRef<objects::CConstraint_choice> > m_label_to_constraint;
};


END_NCBI_SCOPE

#endif  // GUI_PKG_EDIT_SUSPECT_CONSTRAINT_PANEL__HPP

