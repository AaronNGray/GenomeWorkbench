/*  $Id: table_column_id_panel.hpp 43698 2019-08-14 20:00:53Z katargir $
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
 * Authors:  Bob Falk
 *
 * File Description:
 *
 */

#ifndef _TABLE_COLUMN_ID_PANEL_H_
#define _TABLE_COLUMN_ID_PANEL_H_


/*!
 * Includes
 */

////@begin includes
////@end includes

#include "wx/listctrl.h"
#include "gui/widgets/loaders/table_import_listctrl.hpp"

#include <gui/widgets/loaders/table_import_listctrl.hpp>
#include <gui/widgets/loaders/table_import_data_source.hpp>


/*!
 * Forward declarations
 */

BEGIN_NCBI_SCOPE

class CAssemblySelPanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CTABLECOLUMNIDPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CTABLECOLUMNIDPANEL_TITLE _("Set Table Column Format")
#define SYMBOL_CTABLECOLUMNIDPANEL_IDNAME ID_CTABLECOLUMNIDPANEL
#define SYMBOL_CTABLECOLUMNIDPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CTABLECOLUMNIDPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CTableColumnIdPanel class declaration
 */

class CTableColumnIdPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CTableColumnIdPanel )
    DECLARE_EVENT_TABLE()

public:
    CTableColumnIdPanel();
    CTableColumnIdPanel( wxWindow* parent, wxWindowID id = SYMBOL_CTABLECOLUMNIDPANEL_IDNAME, const wxString& caption = SYMBOL_CTABLECOLUMNIDPANEL_TITLE, const wxPoint& pos = SYMBOL_CTABLECOLUMNIDPANEL_POSITION, const wxSize& size = SYMBOL_CTABLECOLUMNIDPANEL_SIZE, long style = SYMBOL_CTABLECOLUMNIDPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTABLECOLUMNIDPANEL_IDNAME, const wxString& caption = SYMBOL_CTABLECOLUMNIDPANEL_TITLE, const wxPoint& pos = SYMBOL_CTABLECOLUMNIDPANEL_POSITION, const wxSize& size = SYMBOL_CTABLECOLUMNIDPANEL_SIZE, long style = SYMBOL_CTABLECOLUMNIDPANEL_STYLE );

    ~CTableColumnIdPanel();

    void Init();

    void CreateControls();

    /// Override Show() to allow initialization each time window is displayed
    bool Show(bool show);

    void SetMainTitle(const wxString& title);

    void SetImportDataSource(CRef<CTableImportDataSource> ds) { m_ImportedTableData = ds; }

    bool IsInputValid();

    bool NeedsAssemblyMapping();

    void SetRegistryPath( const string& path );
    void LoadSettings();
    void SaveSettings() const;

////@begin CTableColumnIdPanel event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_COLUMNNAMETXT
    void OnColumnNametxtTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_SEQIDFORMATBTN
    void OnSeqidFormatBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_TEXTFORMATBTN
    void OnTextFmtBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_NUMBERFORMATBTN
    void OnNumberFmtBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_ONEBASEDINTCHECK
    void OnOneBasedIntCheckClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_REALNUMBERFORMATBTN
    void OnRealNumberFormatBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_SKIPBTN
    void OnSkipBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_DATATYPECOMBO
    void OnDataTypePropertySelected( wxCommandEvent& event );

////@end CTableColumnIdPanel event handler declarations

    void OnCtableImportListctrlColLeftClick( wxListEvent& event );
    void OnCtableImportListctrlColDragging( wxListEvent& event );

    void OnAssemblyChanged ( wxCommandEvent& evt );

////@begin CTableColumnIdPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTableColumnIdPanel member function declarations

    static bool ShowToolTips();

////@begin CTableColumnIdPanel member variables
    wxStaticBox* m_ColumnPropertiesSizer;
    wxStaticText* m_ColumnNameStaticTxt;
    wxTextCtrl* m_ColumnNameTxtCtrl;
    wxRadioButton* m_SeqIdFormatBtn;
    wxRadioButton* m_TextFormatBtn;
    wxRadioButton* m_NumberFormatBtn;
    wxCheckBox* m_OneBasedIntCheck;
    wxRadioButton* m_RealNumberFormatBtn;
    wxRadioButton* m_SkipFormatBtn;
    wxListBox* m_DataTypeList;
    CAssemblySelPanel* m_AssemblyPanel;
    wxPanel* m_ColumnIdPanel;
    CTableImportListCtrl* m_ColumnIdList;
    /// Control identifiers
    enum {
        ID_CTABLECOLUMNIDPANEL = 10085,
        ID_COLUMNNAMETXT = 10081,
        ID_SEQIDFORMATBTN = 10086,
        ID_TEXTFORMATBTN = 10089,
        ID_NUMBERFORMATBTN = 10090,
        ID_ONEBASEDINTCHECK = 10094,
        ID_REALNUMBERFORMATBTN = 10098,
        ID_SKIPBTN = 10087,
        ID_DATATYPECOMBO = 10091,
        ID_PANEL5 = 10059,
        ID_COLUMNIDLISTPANEL = 10092,
        ID_CTABLEIMPORTLISTCTRL2 = 10071
    };
////@end CTableColumnIdPanel member variables

protected:  

    // Utility function to set some control values after a type btn clicked
    void x_OnColumnTypeUpdate();

    /// Set values in drop down list box based on data type
    void x_InitializePropertyList();

    /// If a column is currently selected, the controls to edit its type and name
    /// should be available, otherwise they should be grayed out.
    void x_EnableColumnEditControls(bool b);

    /// Enables all controls except the format buttons since they
    /// can be used to skip a column (which greys-out other buttons)
    void x_EnableNonFormatEditControls(bool b);

    void x_UpdateAssemblyPanel();

protected:
    string         m_RegPath;

    /// Index within listctrl/datasource of column being edited or -1
    int  m_CurrentColumnIdx;

    /// Contents of tabular file
    CRef<CTableImportDataSource> m_ImportedTableData; 

    list<string>   m_TermHistory;
};

END_NCBI_SCOPE

#endif
    // _TABLE_COLUMN_ID_PANEL_H_
