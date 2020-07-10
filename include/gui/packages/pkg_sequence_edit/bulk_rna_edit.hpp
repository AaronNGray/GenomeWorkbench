/*  $Id: bulk_rna_edit.hpp 38632 2017-06-05 17:16:13Z asztalos $
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
#ifndef _BULK_RNA_EDIT_H_
#define _BULK_RNA_EDIT_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/seq_table_grid.hpp>
#include <gui/packages/pkg_sequence_edit/tbl_edit_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/seq_grid_table_navigator.hpp>

BEGIN_NCBI_SCOPE

#define SYMBOL_RNAEDITDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_RNAEDITDIALOG_TITLE _("RNA Editing")
#define SYMBOL_RNAEDITDIALOG_IDNAME wxID_ANY
#define SYMBOL_RNAEDITDIALOG_SIZE wxDefaultSize
#define SYMBOL_RNAEDITDIALOG_POSITION wxDefaultPosition


class  CBulkRna : public CReportEditingDialog, public CSeqGridTableNav
{    
    DECLARE_DYNAMIC_CLASS(  CBulkRna )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CBulkRna();
    CBulkRna( wxWindow* parent, objects::CSeq_entry_Handle seh, IWorkbench* workbench, CConstRef<objects::CSeq_submit> seqSubmit,
               wxWindowID id = SYMBOL_RNAEDITDIALOG_IDNAME, const wxString& caption = SYMBOL_RNAEDITDIALOG_TITLE, const wxPoint& pos = SYMBOL_RNAEDITDIALOG_POSITION, const wxSize& size = SYMBOL_RNAEDITDIALOG_SIZE, long style = SYMBOL_RNAEDITDIALOG_STYLE );
    CBulkRna( wxWindow* parent,  const vector<CSeq_feat_Handle> &feat_handles, IWorkbench* workbench = NULL,
              wxWindowID id = SYMBOL_RNAEDITDIALOG_IDNAME, const wxString& caption = SYMBOL_RNAEDITDIALOG_TITLE, const wxPoint& pos = SYMBOL_RNAEDITDIALOG_POSITION, const wxSize& size = SYMBOL_RNAEDITDIALOG_SIZE, long style = SYMBOL_RNAEDITDIALOG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_RNAEDITDIALOG_IDNAME, const wxString& caption = SYMBOL_RNAEDITDIALOG_TITLE, const wxPoint& pos = SYMBOL_RNAEDITDIALOG_POSITION, const wxSize& size = SYMBOL_RNAEDITDIALOG_SIZE, long style = SYMBOL_RNAEDITDIALOG_STYLE );

    /// Destructor
    ~CBulkRna();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    CRef<objects::CSeq_table> GetValuesTableFromSeqEntry();
    CRef<CCmdComposite> GetCommandFromValuesTable(CRef<objects::CSeq_table>);
    string GetErrorMessage();
    bool IsReadOnlyColumn(string column_name) { if (column_name == "location" || column_name.empty() || column_name == "expand") return true; return false; };
    int GetCollapsible() {return 0;}
    CRef<CCmdComposite> GetCommand();
    CRef<objects::CSeq_table> GetChoices(CRef<objects::CSeq_table> values_table) { CRef<objects::CSeq_table> choices; return choices; } ;

    wxBitmap GetBitmapResource( const wxString& name );
    
    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
    
    /// Should we show tooltips?
    static bool ShowToolTips();
    void OnClickOk( wxCommandEvent& event );
    void OnClickCancel( wxCommandEvent& event );
    virtual const CObject* RowToScopedObjects(int row, TConstScopedObjects &objects, CBioseq_Handle &bsh);
private:
    wxGrid* m_Grid;
    vector<CSeq_feat_Handle> m_FeatHandles;
    CSeqTableGridPanel* m_GridPanel;
    bool m_Modified;
    CApplyEditconvertPanel *m_AecrPanel;
};

END_NCBI_SCOPE
#endif
