/*  $Id: country_conflict.hpp 42309 2019-01-30 21:23:04Z asztalos $
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
#ifndef _COUNTRY_CONFLICT_H_
#define _COUNTRY_CONFLICT_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/packages/pkg_sequence_edit/seq_table_grid.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_select.hpp>


BEGIN_NCBI_SCOPE


#define SYMBOL_COUNTRY_CONFLICT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_COUNTRY_CONFLICT_TITLE _("Country Conflict Tool")
#define SYMBOL_COUNTRY_CONFLICT_IDNAME wxID_ANY
#define SYMBOL_COUNTRY_CONFLICT_SIZE wxDefaultSize
#define SYMBOL_COUNTRY_CONFLICT_POSITION wxDefaultPosition


class CCountryConflict : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS(  CCountryConflict )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCountryConflict();
    CCountryConflict( wxWindow* parent, objects::CSeq_entry_Handle seh, 
              wxWindowID id = SYMBOL_COUNTRY_CONFLICT_IDNAME, const wxString& caption = SYMBOL_COUNTRY_CONFLICT_TITLE, const wxPoint& pos = SYMBOL_COUNTRY_CONFLICT_POSITION, const wxSize& size = SYMBOL_COUNTRY_CONFLICT_SIZE, long style = SYMBOL_COUNTRY_CONFLICT_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_COUNTRY_CONFLICT_IDNAME, const wxString& caption = SYMBOL_COUNTRY_CONFLICT_TITLE, const wxPoint& pos = SYMBOL_COUNTRY_CONFLICT_POSITION, const wxSize& size = SYMBOL_COUNTRY_CONFLICT_SIZE, long style = SYMBOL_COUNTRY_CONFLICT_STYLE );

    /// Destructor
    ~CCountryConflict();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    CRef<objects::CSeq_table> GetValuesTableFromSeqEntry();
    CRef<CCmdComposite> GetCommandFromValuesTable(CRef<objects::CSeq_table>);
    CRef<CCmdComposite> GetCommand();
    CRef<objects::CSeq_table> GetChoices(CRef<objects::CSeq_table> values_table) { CRef<objects::CSeq_table> choices; return choices; } ;
    int GetCollapsible() {return 0;}

    string GetErrorMessage();
    bool IsReadOnlyColumn(string column_name) { if (column_name == "Current Lat-Lon" || column_name.empty() || column_name == "expand") return true; return false; };
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );
    
    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
    
    /// Should we show tooltips?
    static bool ShowToolTips();
    void OnClickOk( wxCommandEvent& event );
    void OnClickCancel( wxCommandEvent& event );
private:
    void x_FindBioSource(objects::CSeq_entry_Handle tse, CCmdComposite* composite = NULL);
    void x_ApplyToDescriptors(const objects::CSeq_entry& se, CCmdComposite* composite);
    bool x_ApplyToBioSource(objects::CBioSource& biosource);
    void x_GatherLatLon(const objects::CBioSource& biosource);
  
    wxGrid* m_Grid;
    vector<string> m_LatLon;
    vector<string> m_Country;
    vector<string> m_ReplaceLatLon;
    map<pair<string, string>, pair<string, string> > m_LatLonCountry;
    objects::CSeq_entry_Handle m_TopSeqEntry;
    bool m_Found;
    CSeqTableGridPanel* m_GridPanel;
    CStringConstraintSelect* m_StringConstraintPanel;
};

END_NCBI_SCOPE
#endif
