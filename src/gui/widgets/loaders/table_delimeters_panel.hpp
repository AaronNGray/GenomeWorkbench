/*  $Id: table_delimeters_panel.hpp 43700 2019-08-14 20:31:35Z katargir $
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

#ifndef _TABLE_DELIMETERS_PANEL_H_
#define _TABLE_DELIMETERS_PANEL_H_

/*!
 * Includes
 */

////@begin includes
////@end includes

#include "wx/listctrl.h"

#include <gui/widgets/loaders/table_import_listctrl.hpp>
#include <gui/widgets/loaders/table_import_data_source.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class CTableImportListCtrl;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CTABLEDELIMITERSPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CTABLEDELIMITERSPANEL_TITLE _("Set Table Delimiters")
#define SYMBOL_CTABLEDELIMITERSPANEL_IDNAME ID_CTABLEDELIMITERSPANEL
#define SYMBOL_CTABLEDELIMITERSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CTABLEDELIMITERSPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CTableDelimitersPanel class declaration
 */

class CTableDelimitersPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CTableDelimitersPanel )
    DECLARE_EVENT_TABLE()

public:
    CTableDelimitersPanel();
    CTableDelimitersPanel( wxWindow* parent, wxWindowID id = SYMBOL_CTABLEDELIMITERSPANEL_IDNAME, const wxString& caption = SYMBOL_CTABLEDELIMITERSPANEL_TITLE, const wxPoint& pos = SYMBOL_CTABLEDELIMITERSPANEL_POSITION, const wxSize& size = SYMBOL_CTABLEDELIMITERSPANEL_SIZE, long style = SYMBOL_CTABLEDELIMITERSPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTABLEDELIMITERSPANEL_IDNAME, const wxString& caption = SYMBOL_CTABLEDELIMITERSPANEL_TITLE, const wxPoint& pos = SYMBOL_CTABLEDELIMITERSPANEL_POSITION, const wxSize& size = SYMBOL_CTABLEDELIMITERSPANEL_SIZE, long style = SYMBOL_CTABLEDELIMITERSPANEL_STYLE );

    ~CTableDelimitersPanel();

    void Init();

    void CreateControls();

    /// Override Show() to allow initialization each time window is displayed
    virtual bool Show(bool show = true);

    void SetMainTitle(const wxString& title);

    void SetImportDataSource(CRef<CTableImportDataSource> ds) { m_ImportedTableData = ds; }

    bool IsInputValid();

    /// Called after any options that may update the partioning of the table
    /// into columns are updated
    void UpdateDelimiters();

    void SetRegistryPath( const string& path );
    void LoadSettings();
    void SaveSettings() const;

////@begin CTableDelimitersPanel event handler declarations

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_TABDELIMCHECK
    void OnTabDelimCheckClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_SEMICOLONDELIMCHECK
    void OnSemiColonDelimCheckClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_COMMADELIMCHECK
    void OnCommaDelimCheckClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_SPACEDELIMCHECK
    void OnSpaceDelimCheckClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_OTHERDELIMCHECK
    void OnOtherDelimCheckClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_TABLEDELIMITERTEXTCTRL
    void OnTableDelimiterTextCtrlTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_ADJACENTDELIMITERSCHECK
    void OnAdjacentDelimitersCheckClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBOX_SELECTED event handler for ID_RADIOBOX
    void OnRadioBoxSelected( wxCommandEvent& event );

////@end CTableDelimitersPanel event handler declarations

    void OnCtableImportListctrlColDragging( wxListEvent& event );

////@begin CTableDelimitersPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTableDelimitersPanel member function declarations

    static bool ShowToolTips();

////@begin CTableDelimitersPanel member variables
    wxCheckBox* m_TabDelimCheck;
    wxCheckBox* m_SemicolonDelimCheck;
    wxCheckBox* m_CommaDelimCheck;
    wxCheckBox* m_SpaceDelimCheck;
    wxCheckBox* m_OtherDelimCheck;
    wxTextCtrl* m_DelimTextCtrl;
    wxCheckBox* m_MergeAdjacentDelimiters;
    wxRadioBox* m_TextQualifier;
    wxPanel* m_ListCtrlPanel;
    CTableImportListCtrl* m_DelimiterListCtrl;
    /// Control identifiers
    enum {
        ID_CTABLEDELIMITERSPANEL = 10072,
        ID_TABDELIMCHECK = 10074,
        ID_SEMICOLONDELIMCHECK = 10075,
        ID_COMMADELIMCHECK = 10076,
        ID_SPACEDELIMCHECK = 10077,
        ID_OTHERDELIMCHECK = 10078,
        ID_TABLEDELIMITERTEXTCTRL = 10079,
        ID_ADJACENTDELIMITERSCHECK = 10073,
        ID_RADIOBOX = 10080,
        ID_PANEL3 = 10082,
        ID_CTABLEIMPORTLISTCTRL2 = 10071
    };
////@end CTableDelimitersPanel member variables
protected:  
    string         m_RegPath;
    
    /// 
    bool m_DelimsInitialized;
    CRef<CTableImportDataSource> m_ImportedTableData;
};

END_NCBI_SCOPE

#endif
    // _TABLE_DELIMETERS_PANEL_H_
