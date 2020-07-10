/*  $Id: country_fixup.hpp 42307 2019-01-30 20:05:59Z asztalos $
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
#ifndef _COUNTRY_FIXUP_H_
#define _COUNTRY_FIXUP_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/packages/pkg_sequence_edit/seq_table_grid.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_select.hpp>
#include <gui/packages/pkg_sequence_edit/apply_edit_convert_panel.hpp>
#include <gui/packages/pkg_sequence_edit/seq_grid_table_navigator.hpp>

BEGIN_NCBI_SCOPE

class CCountryFixup
{
public:
    CRef<CCmdComposite> GetCommand(objects::CSeq_entry_Handle tse, bool capitalize_after_colon);
private:
    void x_ApplyToSeqAndFeat(CCmdComposite* composite);
    void x_ApplyToDescriptors(const objects::CSeq_entry& se, CCmdComposite* composite);
    bool x_ApplyToBioSource(objects::CBioSource& biosource);

    bool m_capitalize_after_colon;
    string m_accession;
    vector<pair<string, string> > m_invalid_countries;
    map<string,string> m_fixed_countries;
    objects::CSeq_entry_Handle m_Seh;
};


#define SYMBOL_COUNTRY_EDIT_DIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_COUNTRY_EDIT_DIALOG_TITLE _("Country Modifiers That Could Not Be Autocorrected")
#define SYMBOL_COUNTRY_EDIT_DIALOG_IDNAME wxID_ANY
#define SYMBOL_COUNTRY_EDIT_DIALOG_SIZE wxDefaultSize
#define SYMBOL_COUNTRY_EDIT_DIALOG_POSITION wxDefaultPosition


class  CBulkCountryEdit : public wxDialog
{    
    DECLARE_DYNAMIC_CLASS(  CBulkCountryEdit )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CBulkCountryEdit();
    CBulkCountryEdit( wxWindow* parent, const vector<pair<string,string> > &invalid_countries,
              wxWindowID id = SYMBOL_COUNTRY_EDIT_DIALOG_IDNAME, const wxString& caption = SYMBOL_COUNTRY_EDIT_DIALOG_TITLE, const wxPoint& pos = SYMBOL_COUNTRY_EDIT_DIALOG_POSITION, const wxSize& size = SYMBOL_COUNTRY_EDIT_DIALOG_SIZE, long style = SYMBOL_COUNTRY_EDIT_DIALOG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_COUNTRY_EDIT_DIALOG_IDNAME, const wxString& caption = SYMBOL_COUNTRY_EDIT_DIALOG_TITLE, const wxPoint& pos = SYMBOL_COUNTRY_EDIT_DIALOG_POSITION, const wxSize& size = SYMBOL_COUNTRY_EDIT_DIALOG_SIZE, long style = SYMBOL_COUNTRY_EDIT_DIALOG_STYLE );

    /// Destructor
    ~CBulkCountryEdit();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    CRef<objects::CSeq_table> GetValuesTable();
    map<string,string> GetValuesFromValuesTable(CRef<objects::CSeq_table>);
    map<string,string> GetValues();
    int GetCollapsible() {return 0;}

    string GetErrorMessage();
    CRef<objects::CSeq_table> GetChoices(CRef<objects::CSeq_table> values_table) { CRef<objects::CSeq_table> choices; return choices; } ;
    bool IsReadOnlyColumn(string column_name) { if (column_name == "Accession" || column_name.empty() || column_name == "expand") return true; return false; };
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );
    
    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
    
    /// Should we show tooltips?
    static bool ShowToolTips();
    void OnClickCancel( wxCommandEvent& event );

private:
    wxGrid* m_Grid;
    CSeqTableGridPanel* m_GridPanel;
    CStringConstraintSelect* m_StringConstraintPanel;
    CApplyEditconvertPanel *m_AecrPanel;
    vector<pair<string,string> > m_invalid_countries;
};

END_NCBI_SCOPE

#endif
    // _COUNTRY_FIXUP_H_
