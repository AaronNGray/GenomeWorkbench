/*  $Id: srceditdialog.hpp 43832 2019-09-09 13:50:37Z filippov $
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
#ifndef _SRCEDITDIALOG_H_
#define _SRCEDITDIALOG_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/PCRPrimerSet.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_select.hpp>
#include <gui/packages/pkg_sequence_edit/apply_edit_convert_panel.hpp>
#include <gui/packages/pkg_sequence_edit/seq_grid_table_navigator.hpp>
#include <gui/framework/workbench.hpp>

#include <wx/dialog.h>
#include <wx/choice.h>
#include <wx/menu.h>
#include <wx/event.h>
#include <wx/stattext.h>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

BEGIN_NCBI_SCOPE

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_SRCEDITDIALOG 10013
#define ID_QUALCHOICE 10017
#define ID_ADD_QUAL_BUTTON 10018
#define ID_LOADQUALS 10084
#define ID_EXPORT_QUALS 10085
#define SYMBOL_SRCEDITDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_SRCEDITDIALOG_TITLE _("Source Editing")
#define SYMBOL_SRCEDITDIALOG_IDNAME ID_SRCEDITDIALOG
#define SYMBOL_SRCEDITDIALOG_SIZE wxDefaultSize
#define SYMBOL_SRCEDITDIALOG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * SrcEditDialog class declaration
 */



class SrcEditDialog : public CReportEditingDialog, public CSeqGridTableNav
{    
    DECLARE_DYNAMIC_CLASS( SrcEditDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    SrcEditDialog();
    SrcEditDialog( wxWindow* parent, objects::CSeq_entry_Handle seh,  IWorkbench* workbench, CConstRef<objects::CSeq_submit> seqSubmit,
                   wxWindowID id = SYMBOL_SRCEDITDIALOG_IDNAME, 
                   const wxString& caption = SYMBOL_SRCEDITDIALOG_TITLE, 
                   const wxPoint& pos = SYMBOL_SRCEDITDIALOG_POSITION, 
                   const wxSize& size = SYMBOL_SRCEDITDIALOG_SIZE, 
                   long style = SYMBOL_SRCEDITDIALOG_STYLE );

    SrcEditDialog( wxWindow* parent, const vector<CBioseq_Handle>& bioseq_handles, IWorkbench* workbench = NULL,
                   wxWindowID id = SYMBOL_SRCEDITDIALOG_IDNAME, 
                   const wxString& caption = SYMBOL_SRCEDITDIALOG_TITLE, 
                   const wxPoint& pos = SYMBOL_SRCEDITDIALOG_POSITION, 
                   const wxSize& size = SYMBOL_SRCEDITDIALOG_SIZE, 
                   long style = SYMBOL_SRCEDITDIALOG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_SRCEDITDIALOG_IDNAME, 
        const wxString& caption = SYMBOL_SRCEDITDIALOG_TITLE, 
        const wxPoint& pos = SYMBOL_SRCEDITDIALOG_POSITION, 
        const wxSize& size = SYMBOL_SRCEDITDIALOG_SIZE, 
        long style = SYMBOL_SRCEDITDIALOG_STYLE );

    /// Destructor
    ~SrcEditDialog();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin SrcEditDialog event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_QUALCHOICE
    void OnQualchoiceSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ADD_QUAL_BUTTON
    void OnAddQual( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_LOADQUALS
    void OnLoadqualsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EXPORT_QUALS
    void OnExportQualsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CLEAR_QUALS_BTN
    void OnClearQuals( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnClickOk( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
    void OnClickCancel( wxCommandEvent& event );

////@end SrcEditDialog event handler declarations

    void OnCopyCells( wxCommandEvent& event );
    void OnPasteCells( wxCommandEvent& event );
    void OnPasteAppendCells( wxCommandEvent& event );
    void OnCopyCellsFromId( wxCommandEvent& event );


////@begin SrcEditDialog member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end SrcEditDialog member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    void GetQualChoices(wxArrayString& srcModNameStrings);
    void x_RepopulateAddQualList();

    int GetCollapsible() {return 0;}
    bool IsReadOnlyColumn(string column_name) { if (column_name == "location" || column_name.empty() || column_name == "expand") return true; return false; };

////@begin SrcEditDialog member variables
    wxChoice* m_QualList;
    wxButton* m_AddQualBtn;
////@end SrcEditDialog member variables
    wxGrid* m_Grid;

    CRef<CCmdComposite> GetCommand();
    void ChangeColumnName(int col, string qual_name);
    void ResetSubPanels();
    int GetNewColumn(wxString new_col);
    virtual const CObject* RowToScopedObjects(int row, TConstScopedObjects &objects, CBioseq_Handle &bsh);

    void SetWorkDir(const wxString& workDir) { m_SaveFileDir = workDir; }

private:
    vector<CBioseq_Handle> m_BioseqHandles;
    objects::CSeq_entry_Handle m_SEH;
    CRef<objects::CSeq_table> m_Copied;
    CSeqTableGridPanel* m_GridPanel;
    wxString m_SaveFileDir;
    wxString m_SaveFileName;
    CStringConstraintSelect *m_StringConstraintPanel;
    CApplyEditconvertPanel *m_AecrPanel;
    objects::CSeq_table::TColumns  m_columns;

    int  x_FindColumn( const wxString& name );
    CRef<objects::CSeq_table> x_GetSourceTableChoices (CRef<objects::CSeq_table> values);
    CRef<CCmdComposite> ApplySrcTableToSeqEntry (CRef<objects::CSeq_table>values_table);
    bool OneRowOk (CRef<objects::CSeq_id> id, CRef<objects::CSeqTable_column> id_col);
    string FindBadRows (CRef<objects::CSeq_table> src, CRef<objects::CSeq_table> dst);
    CBioseq_Handle GetBioseqHandle(int row);
    CRef<objects::CSeq_table> x_GetValuesTableFromGridPanel();
    //CRef<CSeq_table> GetSeqTableFromSeqEntry (CSeq_entry_Handle seh);
    //string GetPrimerSetNameValues(const objects::CPCRPrimerSet& primer_set);
    //string GetPrimerSetSeqValues(const objects::CPCRPrimerSet& primer_set);
    CRef<objects::CSeqTable_column> FindSeqIDColumn(const objects::CSeq_table& table);
    void CombineColumns (CRef<objects::CSeqTable_column> dst_id, CRef<objects::CSeqTable_column> dst_col,
                         CRef<objects::CSeqTable_column> src_id, CRef<objects::CSeqTable_column> src_col);
    int CombineTables (CRef<objects::CSeq_table> dst, const objects::CSeq_table& src);
    bool SaveTableFile (wxWindow *parent, wxString& save_file_dir, wxString& save_file_name, CRef<objects::CSeq_table> values_table);
    int FindRowForSeqId (CRef<objects::CSeqTable_column> id_col, CRef<objects::CSeq_id> id);
    int CountColumnRowConflicts (CRef<objects::CSeqTable_column> dst_id, CRef<objects::CSeqTable_column> dst_col,
                                 CRef<objects::CSeqTable_column> src_id, CRef<objects::CSeqTable_column> src_col);
    int CountTableColumnConflicts (CRef<objects::CSeq_table> dst, const objects::CSeq_table& src);
    void FixTableAfterImport (CRef<objects::CSeq_table> input_table);
    void DeleteTableRow (CRef<objects::CSeq_table> table, int row);
    string GetLabelForTitle(string sTitle);
    string FindBadColumns (CRef<objects::CSeq_table> table);

};

END_NCBI_SCOPE

#endif
    // _SRCEDITDIALOG_H_
