/*  $Id: export_table.hpp 41087 2018-05-21 21:29:57Z filippov $
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
#ifndef _EXPORT_TABLE_H_
#define _EXPORT_TABLE_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/widgets/data/report_dialog.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/checkbox.h>
#include <wx/listbox.h>

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
#define SYMBOL_EXPORT_TABLE_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_EXPORT_TABLE_TITLE _("Export Table")
#define SYMBOL_EXPORT_TABLE_IDNAME ID_EXPORT_TABLE_DLG
#define SYMBOL_EXPORT_TABLE_SIZE wxDefaultSize
#define SYMBOL_EXPORT_TABLE_POSITION wxDefaultPosition
////@end control identifiers

class CExportTableDlg : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CExportTableDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CExportTableDlg();
    CExportTableDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = SYMBOL_EXPORT_TABLE_IDNAME, const wxString& caption = SYMBOL_EXPORT_TABLE_TITLE, const wxPoint& pos = SYMBOL_EXPORT_TABLE_POSITION, const wxSize& size = SYMBOL_EXPORT_TABLE_SIZE, long style = SYMBOL_EXPORT_TABLE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_EXPORT_TABLE_IDNAME, const wxString& caption = SYMBOL_EXPORT_TABLE_TITLE, const wxPoint& pos = SYMBOL_EXPORT_TABLE_POSITION, const wxSize& size = SYMBOL_EXPORT_TABLE_SIZE, long style = SYMBOL_EXPORT_TABLE_STYLE );

    /// Destructor
    ~CExportTableDlg();

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

    void SaveTable(const wxString& workDir = wxEmptyString);
    void SaveTableFile (wxWindow *parent, wxString& save_file_dir, wxString& save_file_name,  const list< map<string, string> > &table, const  set<string> &columns);
    static void RemoveEmptyColsAndRows(CRef<CSeq_table> values_table);

private:
    void AddBioSource(const CBioSource& biosource);
    void GetDesc(const CSeq_entry& se);
    void FindBioSource(objects::CSeq_entry_Handle tse);
    void AddDeflinesToTable(list< map<string, string> > &table, set<string> &columns);
    void AddCdsGeneMrnaTable(list< map<string, string> > &table, set<string> &columns);
    void AddFeaturesToTable(list< map<string, string> > &table, set<string> &columns);
    void AddPubsToTable(list< map<string, string> > &table, set<string> &columns);
    void AddDblinkToTable(list< map<string, string> > &table, set<string> &columns);
    void AddRnaToTable( list< map<string, string> > &table, set<string> &columns);
    void AddSourceToTable( list< map<string, string> > &table, set<string> &columns);
    void RunExcel(wxString FileName);


    objects::CSeq_entry_Handle m_TopSeqEntry;
    wxCheckBox *m_SourceCheck, *m_ExcelCheck;
    wxRadioButton *m_NoneRB, *m_FeatureRB,*m_CdsGeneProtRB,*m_RnaRB,*m_PubRB,*m_DbLinkRB,*m_DeflineRB;
    wxListBox *m_SourceChoice;
    set<string> m_source_choices;
    wxString m_SaveFileDir, m_SaveFileName;

    enum {
        ID_EXPORT_TABLE_DLG = 12100,
        ID_EXPTABLE_CHKBOX1,
        ID_EXPTABLE_LISTBOX,
        ID_EXPTABLE_RADBTN1,
        ID_EXPTABLE_RADBTN2,
        ID_EXPTABLE_RADBTN3,
        ID_EXPTABLE_RADBTN4,
        ID_EXPTABLE_RADBTN5,
        ID_EXPTABLE_RADBTN6,
        ID_EXPTABLE_RADBTN7,
        ID_EXPTABLE_CHKBOX2
    };
};

END_NCBI_SCOPE

#endif
    // _EXPORT_TABLE_H_
