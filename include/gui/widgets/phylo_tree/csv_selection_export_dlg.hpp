/*  $Id: csv_selection_export_dlg.hpp 39737 2017-10-31 17:03:07Z katargir $
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
 * Authors:  Vladislav Evgeniev
 */
#ifndef _CSV_EXPORT_DLG_H_
#define _CSV_EXPORT_DLG_H_

#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/valtext.h"
#include "wx/valgen.h"
////@end includes
#include <wx/checklst.h>
#include <wx/button.h>

#include <vector>
#include <gui/widgets/wx/dialog.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

class wxChoice;
class wxBoxSizer;
class wxTextCtrl;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CWXCSVEXPORTDLG 10007
#define ID_TEXTCTRL 10008
#define ID_BITMAPBUTTON 10009
#define ID_HEADER_CHECKBOX 10012
#define ID_LEAVES_ONLY_CHECKBOX 10013
#define ID_CHECKLISTBOX 10000
#define ID_SELECT_ALL_BTN 10001
#define ID_DESELECT_ALL_BTN 10002
#define SYMBOL_CCSVSELECTIONEXPORTDLG_STYLE wxCAPTION|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CCSVSELECTIONEXPORTDLG_TITLE _("Export To CSV")
#define SYMBOL_CCSVSELECTIONEXPORTDLG_IDNAME ID_CWXCSVEXPORTDLG
#define SYMBOL_CCSVSELECTIONEXPORTDLG_SIZE wxSize(400, 300)
#define SYMBOL_CCSVSELECTIONEXPORTDLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CCSVSelectionExportDlg class declaration
 */

class CCSVSelectionExportDlg: public CDialog
{    
    DECLARE_DYNAMIC_CLASS( CCSVSelectionExportDlg )
    DECLARE_EVENT_TABLE()

public:
    CCSVSelectionExportDlg();
    CCSVSelectionExportDlg( wxWindow* parent, wxWindowID id = SYMBOL_CCSVSELECTIONEXPORTDLG_IDNAME, const wxString& caption = SYMBOL_CCSVSELECTIONEXPORTDLG_TITLE, const wxPoint& pos = SYMBOL_CCSVSELECTIONEXPORTDLG_POSITION, const wxSize& size = SYMBOL_CCSVSELECTIONEXPORTDLG_SIZE, long style = SYMBOL_CCSVSELECTIONEXPORTDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCSVSELECTIONEXPORTDLG_IDNAME, const wxString& caption = SYMBOL_CCSVSELECTIONEXPORTDLG_TITLE, const wxPoint& pos = SYMBOL_CCSVSELECTIONEXPORTDLG_POSITION, const wxSize& size = SYMBOL_CCSVSELECTIONEXPORTDLG_SIZE, long style = SYMBOL_CCSVSELECTIONEXPORTDLG_STYLE );

    ~CCSVSelectionExportDlg();

    void Init();

    void CreateControls();
    
    /// Sets the columns lists
    void SetColumnsList(const vector<wxString> &columns);

    /// Gets the columns, selected by the user
    void GetSelectedColumns(vector<wxString> &columns) const;

////@begin CCSVSelectionExportDlg event handler declarations

    void OnSelectFileClick( wxCommandEvent& event );

    void OnSelectAllBtnClick( wxCommandEvent& event );

    void OnDeselectAllBtnClick( wxCommandEvent& event );

////@end CCSVSelectionExportDlg event handler declarations

////@begin CCSVSelectionExportDlg member function declarations

    wxString GetFileName() const { return m_FileName ; }
    void SetFileName(wxString value) { m_FileName = value ; }

    bool GetLeavesOnly() const { return m_LeavesOnly ; }
    void SetLeavesOnly(bool value) { m_LeavesOnly = value ; }

    bool GetWithHeaders() const { return m_WithHeaders ; }
    void SetWithHeaders(bool value) { m_WithHeaders = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CCSVSelectionExportDlg member function declarations

    static bool ShowToolTips();

////@begin CCSVSelectionExportDlg member variables
    wxTextCtrl* m_txtFilename;
    wxCheckListBox* m_ColumnList;
private:
    wxString m_FileName;
    bool m_LeavesOnly;
    bool m_WithHeaders;
////@end CCSVSelectionExportDlg member variables
protected:
    virtual void    x_LoadSettings(const CRegistryReadView& view);
    virtual void    x_SaveSettings(CRegistryWriteView view) const;
};

END_NCBI_SCOPE

#endif
    // _CSV_EXPORT_DLG_H_
