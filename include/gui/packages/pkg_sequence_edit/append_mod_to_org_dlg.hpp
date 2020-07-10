/*  $Id: append_mod_to_org_dlg.hpp 45081 2020-05-26 20:33:57Z asztalos $
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
 * Authors:  Igor Filippov
 */
#ifndef _APPEND_MOD_TO_ORG_DLG_H_
#define _APPEND_MOD_TO_ORG_DLG_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/edit/source_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/add_constraint_panel.hpp>



#include <wx/dialog.h>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/data/report_dialog.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE
using namespace objects;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_APPEND_MOD_TO_ORG_DLG 12000
#define SYMBOL_APPEND_MOD_TO_ORG_DLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_APPEND_MOD_TO_ORG_DLG_TITLE _("Append Mod To Org")
#define SYMBOL_APPEND_MOD_TO_ORG_DLG_IDNAME ID_APPEND_MOD_TO_ORG_DLG
#define SYMBOL_APPEND_MOD_TO_ORG_DLG_SIZE wxDefaultSize
#define SYMBOL_APPEND_MOD_TO_ORG_DLG_POSITION wxDefaultPosition
////@end control identifiers
#define ID_COMBOBOX_MOD 12001
#define ID_CHECKBOX_SP 12002
#define ID_CHECKBOX_CF 12003
#define ID_CHECKBOX_AFF 12004
#define ID_CHECKBOX_NR 12005
#define ID_CHECKBOX_NO_TAXID 12006
#define ID_CHECKBOX_ABBR 12007


/*!
 * CAppendModToOrgDlg class declaration
 */

class CAppendModToOrgDlg : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CAppendModToOrgDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAppendModToOrgDlg();
    CAppendModToOrgDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = SYMBOL_APPEND_MOD_TO_ORG_DLG_IDNAME, const wxString& caption = SYMBOL_APPEND_MOD_TO_ORG_DLG_TITLE, const wxPoint& pos = SYMBOL_APPEND_MOD_TO_ORG_DLG_POSITION, const wxSize& size = SYMBOL_APPEND_MOD_TO_ORG_DLG_SIZE, long style = SYMBOL_APPEND_MOD_TO_ORG_DLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_APPEND_MOD_TO_ORG_DLG_IDNAME, const wxString& caption = SYMBOL_APPEND_MOD_TO_ORG_DLG_TITLE, const wxPoint& pos = SYMBOL_APPEND_MOD_TO_ORG_DLG_POSITION, const wxSize& size = SYMBOL_APPEND_MOD_TO_ORG_DLG_SIZE, long style = SYMBOL_APPEND_MOD_TO_ORG_DLG_STYLE );

    /// Destructor
    ~CAppendModToOrgDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    CRef<CCmdComposite> GetCommand();
    void OnChoiceChanged(wxCommandEvent& event);

private:
    struct  Tchoice_item
    {
        bool is_org;
        int subtype;
        string abbr;
        Tchoice_item(bool org, int type, string ab) 
            {
                is_org = org;
                subtype = type;
                abbr = ab;
            }
        Tchoice_item() 
            {
                is_org = false;
                subtype = 0;
                abbr = "";
            }
    };
    CRef<CEditingActionConstraint> GetConstraint(pair<string, SFieldTypeAndMatcher > &item, int orgmod_subtype);

    objects::CSeq_entry_Handle m_TopSeqEntry;
    map<string, Tchoice_item> m_Types;
    wxChoice* m_Choice;
    wxCheckBox* m_CheckBox_sp;
    wxCheckBox* m_CheckBox_cf;
    wxCheckBox* m_CheckBox_aff;
    wxCheckBox* m_CheckBox_nr;
    wxCheckBox* m_CheckBox_no_taxid;
    wxCheckBox* m_CheckBox_abbr;
    CTripleConstraintPanel_CSourceFieldNamePanel_Wrapper *m_constraint;
};

END_NCBI_SCOPE

#endif
    // _APPEND_MOD_TO_ORG_DLG_H_
