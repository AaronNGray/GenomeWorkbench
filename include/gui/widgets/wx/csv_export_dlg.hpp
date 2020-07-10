/*  $Id: csv_export_dlg.hpp 39737 2017-10-31 17:03:07Z katargir $
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
 * Authors:  Melvin Quintos
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
#define SYMBOL_CWXCSVEXPORTDLG_STYLE wxCAPTION|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWXCSVEXPORTDLG_TITLE _("Export To CSV")
#define SYMBOL_CWXCSVEXPORTDLG_IDNAME ID_CWXCSVEXPORTDLG
#define SYMBOL_CWXCSVEXPORTDLG_SIZE wxSize(400, 300)
#define SYMBOL_CWXCSVEXPORTDLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CwxCSVExportDlg class declaration
 */

class NCBI_GUIWIDGETS_WX_EXPORT CwxCSVExportDlg: public CDialog
{    
    DECLARE_DYNAMIC_CLASS( CwxCSVExportDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CwxCSVExportDlg();
    CwxCSVExportDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWXCSVEXPORTDLG_IDNAME, const wxString& caption = SYMBOL_CWXCSVEXPORTDLG_TITLE, const wxPoint& pos = SYMBOL_CWXCSVEXPORTDLG_POSITION, const wxSize& size = SYMBOL_CWXCSVEXPORTDLG_SIZE, long style = SYMBOL_CWXCSVEXPORTDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWXCSVEXPORTDLG_IDNAME, const wxString& caption = SYMBOL_CWXCSVEXPORTDLG_TITLE, const wxPoint& pos = SYMBOL_CWXCSVEXPORTDLG_POSITION, const wxSize& size = SYMBOL_CWXCSVEXPORTDLG_SIZE, long style = SYMBOL_CWXCSVEXPORTDLG_STYLE );

    /// Destructor
    ~CwxCSVExportDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Sets the columns lists
    void SetColumnsList(const vector<wxString> &columns);

    /// Gets the columns, selected by the user
    void GetSelectedColumns(vector<int> &columns) const;

////@begin CwxCSVExportDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON
    void OnSelectFileClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_SELECTED_CELLS_CHECKBOX
    void OnSelectedCellsCheckboxClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SELECT_ALL_BTN
    void OnSelectAllBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DESELECT_ALL_BTN
    void OnDeselectAllBtnClick( wxCommandEvent& event );

////@end CwxCSVExportDlg event handler declarations

////@begin CwxCSVExportDlg member function declarations

    wxString GetFileName() const { return m_FileName ; }
    void SetFileName(wxString value) { m_FileName = value ; }

    bool GetSelectedOnly() const { return m_SelectedOnly ; }
    void SetSelectedOnly(bool value) { m_SelectedOnly = value ; }

    bool GetWithHeaders() const { return m_WithHeaders ; }
    void SetWithHeaders(bool value) { m_WithHeaders = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CwxCSVExportDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CwxCSVExportDlg member variables
    wxTextCtrl* m_txtFilename;
    wxCheckListBox* m_ColumnList;
private:
    wxString m_FileName;
    bool m_SelectedOnly;
    bool m_WithHeaders;
    /// Control identifiers
    enum {
        ID_CWXCSVEXPORTDLG = 10007,
        ID_TEXTCTRL = 10008,
        ID_BITMAPBUTTON = 10009,
        ID_HEADER_CHECKBOX = 10012,
        ID_SELECTED_CELLS_CHECKBOX = 10013,
        ID_CHECKLISTBOX = 10000,
        ID_SELECT_ALL_BTN = 10001,
        ID_DESELECT_ALL_BTN = 10002
    };
////@end CwxCSVExportDlg member variables

protected:
    virtual void    x_LoadSettings(const CRegistryReadView& view);
    virtual void    x_SaveSettings(CRegistryWriteView view) const;
};

END_NCBI_SCOPE

#endif
    // _CSV_EXPORT_DLG_H_
