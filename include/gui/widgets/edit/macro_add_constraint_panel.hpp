#ifndef GUI_WIDGETS_EDIT___MACRO_ADD_CONSTRAINT_PANEL__HPP
#define GUI_WIDGETS_EDIT___MACRO_ADD_CONSTRAINT_PANEL__HPP
/*  $Id: macro_add_constraint_panel.hpp 44692 2020-02-21 19:41:22Z asztalos $
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
#include <objects/macro/Word_substitution.hpp>
#include <gui/widgets/edit/macro_constraint_panel.hpp>
#include <gui/widgets/edit/macro_item_constraint_panel.hpp>
#include <wx/listbook.h>
#include <wx/choicebk.h>
#include <wx/checklst.h>


BEGIN_NCBI_SCOPE

////@begin control identifiers
#define SYMBOL_CMACROADDCONSTRAINT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CMACROADDCONSTRAINT_TITLE _("Add Macro Constraint")
#define SYMBOL_CMACROADDCONSTRAINT_IDNAME wxID_ANY
#define SYMBOL_CMACROADDCONSTRAINT_SIZE wxDefaultSize
#define SYMBOL_CMACROADDCONSTRAINT_POSITION wxDefaultPosition
////@end control identifiers


class CMacroAddConstraint: public wxFrame, public CFieldNamePanelParent
{    
    DECLARE_DYNAMIC_CLASS(CMacroAddConstraint)
    DECLARE_EVENT_TABLE()

public:
    CMacroAddConstraint();
    CMacroAddConstraint(wxWindow* parent, objects::CSeq_entry_Handle seh, const pair<string, string>& target,
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

    ~CMacroAddConstraint();

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

    void SetEditId(int id) {m_id = id;}
    void SetNumConstraints(size_t num) {m_num = num;}
    void SetConstraint(const string &constraint);

    enum {
        ID_MACROADDCONSTRAINTCHOICEBK = 10500,
    };
private:
  void SetStringSelection(const string& sel);

////@begin CAddConstraint member variables
    wxChoicebook* m_Notebook;
    objects::CSeq_entry_Handle m_TopSeqEntry;
    pair<string, string> m_target;
    int m_id;
    size_t m_num;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___MACRO_ADD_CONSTRAINT_PANEL__HPP
