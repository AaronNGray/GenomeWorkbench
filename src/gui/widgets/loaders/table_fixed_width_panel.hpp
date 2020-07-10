/*  $Id: table_fixed_width_panel.hpp 43699 2019-08-14 20:17:05Z katargir $
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

#ifndef _TABLE_FIXED_WIDTH_PANEL_H_
#define _TABLE_FIXED_WIDTH_PANEL_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/tglbtn.h"
////@end includes

#include "wx/listctrl.h"

#include <gui/widgets/loaders/table_import_listctrl.hpp>
#include <gui/widgets/loaders/table_import_data_source.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxToggleButton;
class CTableImportListCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

BEGIN_NCBI_SCOPE

////@begin control identifiers
#define SYMBOL_CTABLEFIXEDWIDTHPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CTABLEFIXEDWIDTHPANEL_TITLE _("Set Table Column Widths")
#define SYMBOL_CTABLEFIXEDWIDTHPANEL_IDNAME ID_CTABLEFIXEDWIDTHPANEL
#define SYMBOL_CTABLEFIXEDWIDTHPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CTABLEFIXEDWIDTHPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CTableFixedWidthPanel class declaration
 */

class CTableFixedWidthPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CTableFixedWidthPanel )
    DECLARE_EVENT_TABLE()

public:
    CTableFixedWidthPanel();
    CTableFixedWidthPanel( wxWindow* parent, wxWindowID id = SYMBOL_CTABLEFIXEDWIDTHPANEL_IDNAME, const wxString& caption = SYMBOL_CTABLEFIXEDWIDTHPANEL_TITLE, const wxPoint& pos = SYMBOL_CTABLEFIXEDWIDTHPANEL_POSITION, const wxSize& size = SYMBOL_CTABLEFIXEDWIDTHPANEL_SIZE, long style = SYMBOL_CTABLEFIXEDWIDTHPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTABLEFIXEDWIDTHPANEL_IDNAME, const wxString& caption = SYMBOL_CTABLEFIXEDWIDTHPANEL_TITLE, const wxPoint& pos = SYMBOL_CTABLEFIXEDWIDTHPANEL_POSITION, const wxSize& size = SYMBOL_CTABLEFIXEDWIDTHPANEL_SIZE, long style = SYMBOL_CTABLEFIXEDWIDTHPANEL_STYLE );

    ~CTableFixedWidthPanel();

    void Init();

    void CreateControls();

    /// Override Show() to allow initialization each time window is displayed
    virtual bool Show(bool show = true);

    void SetMainTitle(const wxString& title);

    void SetImportDataSource(CRef<CTableImportDataSource> ds) { m_ImportedTableData = ds; }

    bool IsInputValid();

    void SetRegistryPath( const string& path );
    void LoadSettings();
    void SaveSettings() const;

////@begin CTableFixedWidthPanel event handler declarations

    /// wxEVT_IDLE event handler for ID_CTABLEFIXEDWIDTHPANEL
    void OnIdle( wxIdleEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_INSERTCOLUMNBTN
    void OnInsertColumnBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_DELETECOLUMNBTN
    void OnDeleteColumnBtnClick( wxCommandEvent& event );

////@end CTableFixedWidthPanel event handler declarations

    void OnCtableImportListctrlColLeftClick( wxListEvent& event );
    void OnCtableImportListctrlColBeginDrag( wxListEvent& event );
    void OnCTableImportListctrlColDragging( wxListEvent& event );
    void OnCtableImportListctrlColEndDrag( wxListEvent& event );

////@begin CTableFixedWidthPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTableFixedWidthPanel member function declarations

    static bool ShowToolTips();

////@begin CTableFixedWidthPanel member variables
    wxPanel* m_ListCtrlPanel;
    wxToggleButton* m_ClickToInsert;
    wxToggleButton* m_ClickToDelete;
    wxStaticText* m_InfoTxt;
    CTableImportListCtrl* m_FixedWidthListCtrl;
    /// Control identifiers
    enum {
        ID_CTABLEFIXEDWIDTHPANEL = 10084,
        ID_FIXEDWIDTHTABLEPANEL = 10082,
        ID_INSERTCOLUMNBTN = 10100,
        ID_DELETECOLUMNBTN = 10101,
        wxID_FIXEDWIDTHINSTRUCTIONS = 10102,
        ID_CTABLEIMPORTLISTCTRL1 = 10070
    };
////@end CTableFixedWidthPanel member variables
protected:  
    string         m_RegPath;
    CRef<CTableImportDataSource> m_ImportedTableData;
    vector<int>    m_ListWidgetColPrevWidths;

    bool m_ResizeColumnEventCheck;

    /// Bug on windows - not seeming to receive drag event
    bool m_IsDragging;
    wxListEvent m_DragEvent;
};

END_NCBI_SCOPE

#endif
    // _TABLE_FIXED_WIDTH_PANEL_H_
