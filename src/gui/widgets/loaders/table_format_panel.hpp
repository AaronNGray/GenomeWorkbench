/*  $Id: table_format_panel.hpp 43701 2019-08-14 20:37:42Z katargir $
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

#ifndef _TABLE_FORMAT_PANEL_H_
#define _TABLE_FORMAT_PANEL_H_

/*!
 * Includes
 */

////@begin includes
#include "wx/spinctrl.h"
#include "wx/listctrl.h"
////@end includes

#include "wx/panel.h"

#include <gui/widgets/loaders/table_import_data_source.hpp>

/*!
 * Forward declarations
 */

class wxStaticText;
class wxRadioButton;
class wxCheckBox;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CTABLEFORMATPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CTABLEFORMATPANEL_TITLE _("Select Table Format")
#define SYMBOL_CTABLEFORMATPANEL_IDNAME ID_CTABLEFORMATPANEL
#define SYMBOL_CTABLEFORMATPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CTABLEFORMATPANEL_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

class CTableImportListCtrl;

/*!
 * CTableFormatPanel class declaration
 */

class CTableFormatPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CTableFormatPanel )
    DECLARE_EVENT_TABLE()

    friend class CTableImportWizard;

public:
    CTableFormatPanel();
    CTableFormatPanel( wxWindow* parent, wxWindowID id = SYMBOL_CTABLEFORMATPANEL_IDNAME, const wxString& caption = SYMBOL_CTABLEFORMATPANEL_TITLE, const wxPoint& pos = SYMBOL_CTABLEFORMATPANEL_POSITION, const wxSize& size = SYMBOL_CTABLEFORMATPANEL_SIZE, long style = SYMBOL_CTABLEFORMATPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTABLEFORMATPANEL_IDNAME, const wxString& caption = SYMBOL_CTABLEFORMATPANEL_TITLE, const wxPoint& pos = SYMBOL_CTABLEFORMATPANEL_POSITION, const wxSize& size = SYMBOL_CTABLEFORMATPANEL_SIZE, long style = SYMBOL_CTABLEFORMATPANEL_STYLE );

    ~CTableFormatPanel();

    void Init();

    void CreateControls();

    void SetMainTitle(const wxString& title);

    void SetImportDataSource(CRef<CTableImportDataSource> ds) { m_ImportedTableData = ds; }

    bool IsInputValid();

    void SetRegistryPath( const string& path );
    void LoadSettings();
    void SaveSettings() const;

    /// Return true if delimited button selected, false if fixed btn selected
    bool IsDelimitedFile() const;

    /// Load the selected file and displays its contents in the list
    bool PreviewData();

////@begin CTableFormatPanel event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_DELIMITEDBUTTON
    void OnDelimitedButtonSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_FIXEDWIDTHBUTTON
    void OnFixedwidthButtonSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_SPINCTRL_UPDATED event handler for ID_FIRSTLINESPINCTRL
    void OnFirstLineSpinCtrlUpdated( wxSpinEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_COMMENTCHARTXTCTRL
    void OnCommentCharTxtCtrlTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_PARSECOLUMNNAMESBTN
    void OnParseColumnNamesBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_LIST_ITEM_SELECTED event handler for ID_LISTCTRL
    void OnListCtrlSelected( wxListEvent& event );

////@end CTableFormatPanel event handler declarations

////@begin CTableFormatPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTableFormatPanel member function declarations

    static bool ShowToolTips();

////@begin CTableFormatPanel member variables
    wxRadioButton* m_DelimitedBtn;
    wxRadioButton* m_FixedWidthBtn;
    wxStaticText* m_ImportStaticText;
    wxSpinCtrl* m_DataImportLine;
    wxStaticText* m_CommentStaticText;
    wxTextCtrl* m_CommentCharTextCtrl;
    wxCheckBox* m_ParseColumnNamesBtn;
    wxStaticText* m_HeaderImportRow;
    wxStaticText* m_PreviewResultsInfoTxt;
    wxPanel* m_ListCtrlPanel;
    CTableImportListCtrl* m_TableListCtrl;
    /// Control identifiers
    enum {
        ID_CTABLEFORMATPANEL = 10057,
        ID_DELIMITEDBUTTON = 10067,
        ID_FIXEDWIDTHBUTTON = 10068,
        ID_FIRSTLINESPINCTRL = 10093,
        ID_COMMENTCHARTXTCTRL = 10099,
        ID_PARSECOLUMNNAMESBTN = 10108,
        wxID_COLUMNNAMESROW = 10111,
        wxID_PREVIEWRESULTSTXT = 10097,
        ID_PANEL4 = 10083,
        ID_LISTCTRL = 10115
    };
////@end CTableFormatPanel member variables
protected:  

    string  m_RegPath;
    CRef<CTableImportDataSource> m_ImportedTableData;

    wxString m_PreviewFname;
};

END_NCBI_SCOPE

#endif // _TABLE_FORMAT_PANEL_H_
