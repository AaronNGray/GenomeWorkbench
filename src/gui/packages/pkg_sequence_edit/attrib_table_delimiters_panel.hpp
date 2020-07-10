/*  $Id: attrib_table_delimiters_panel.hpp 38460 2017-05-11 18:58:47Z evgeniev $
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
 * Authors:  Bob Falk
 */

#ifndef _ATTRIB_TABLE_DELIMITERS_PANEL_H_
#define _ATTRIB_TABLE_DELIMITERS_PANEL_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/listctrl.h"
#include "gui/widgets/loaders/table_import_listctrl.hpp"
////@end includes

#include <wx/panel.h>
#include <wx/stattext.h>

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CATTRIBTABLEDELIMITERSPANEL 10235
#define ID_TABDELIMCHECK 10236
#define ID_SEMICOLONDELIMCHECK 10237
#define ID_COMMADELIMCHECK 10238
#define ID_SPACEDELIMCHECK 10239
#define ID_MULTSPANCEONLYCHECK 10240
#define ID_OTHERDELIMCHECK 10241
#define ID_TABLEDELIMITERTEXTCTRL 10242
#define ID_ADJACENTDELIMITERSCHECK 10243
#define ID_TEXTQUALIFIER 10244
#define ID_PARSECOLUMNNAMESBTN 10245
#define ID_PANEL3 10246
#define ID_SPLITCOLBTN 10247
#define ID_MERGECOLSBTN 10248
#define ID_EXPORTBTN 10249
#define ID_TABMSBTN 10250
#define ID_SPACEMSBTN 10251
#define ID_WHITESPACEMSBTN 10299
#define ID_COLONMSBTN 10252
#define ID_SEMICOLONMSBTN 10253
#define ID_COMMAMSBTN 10254
#define ID_NONEMSBTN 10300
#define ID_OTHERMSBTN 10255
#define ID_TEXTMSDELIMITER 10256
#define ID_LISTCTRL 10257
#define ID_DELIMROWTEXT 10258
#define ID_REVERTUPDATEBUTTON 10259
#define SYMBOL_CATTRIBTABLEDELIMITERSPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CATTRIBTABLEDELIMITERSPANEL_TITLE _("Set Attribute Table Delimiters")
#define SYMBOL_CATTRIBTABLEDELIMITERSPANEL_IDNAME ID_CATTRIBTABLEDELIMITERSPANEL
#define SYMBOL_CATTRIBTABLEDELIMITERSPANEL_SIZE wxDefaultSize
#define SYMBOL_CATTRIBTABLEDELIMITERSPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

////@begin forward declarations
class CTableImportListCtrl;
class IUIToolManager;
////@end forward declarations

class CAttribTableDelimitersPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CAttribTableDelimitersPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAttribTableDelimitersPanel();
    CAttribTableDelimitersPanel( wxWindow* parent, wxWindowID id = SYMBOL_CATTRIBTABLEDELIMITERSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CATTRIBTABLEDELIMITERSPANEL_POSITION, const wxSize& size = SYMBOL_CATTRIBTABLEDELIMITERSPANEL_SIZE, long style = SYMBOL_CATTRIBTABLEDELIMITERSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CATTRIBTABLEDELIMITERSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CATTRIBTABLEDELIMITERSPANEL_POSITION, const wxSize& size = SYMBOL_CATTRIBTABLEDELIMITERSPANEL_SIZE, long style = SYMBOL_CATTRIBTABLEDELIMITERSPANEL_STYLE );

    /// Destructor
    ~CAttribTableDelimitersPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Override Show() to allow initialization each time window is displayed
    virtual bool Show(bool show = true);

    void SetMainTitle(const wxString& title);

    void SetImportDataSource(CRef<CTableImportDataSource> ds) { m_ImportedTableData = ds; }

    void SetLoadManager(IUIToolManager* tool_mgr) { m_ToolManager = tool_mgr; }

    bool IsInputValid();

    /// Called after any options that may update the partioning of the table
    /// into columns are updated
    void UpdateDelimiters();

    void SetRegistryPath( const string& path );
    void LoadSettings();
    void SaveSettings() const;

    /// Set the name of the table file
    void SetPreviewFileName( const wxString& fname) { m_PreviewFname = fname; }       

    /// Load the selected file and displays its contents in the list
    bool PreviewData();

////@begin CAttribTableDelimitersPanel event handler declarations

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_TABDELIMCHECK
    void OnTabDelimCheckClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_SEMICOLONDELIMCHECK
    void OnSemiColonDelimCheckClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_COMMADELIMCHECK
    void OnCommaDelimCheckClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_SPACEDELIMCHECK
    void OnSpaceDelimCheckClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_MULTSPANCEONLYCHECK
    void OnMultSpanceOnlyCheckClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_OTHERDELIMCHECK
    void OnOtherDelimCheckClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_TABLEDELIMITERTEXTCTRL
    void OnTableDelimiterTextCtrlTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_ADJACENTDELIMITERSCHECK
    void OnAdjacentDelimitersCheckClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBOX_SELECTED event handler for ID_TEXTQUALIFIER
    void OnTextQualifierSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_PARSECOLUMNNAMESBTN
    void OnParseColumnNamesBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SPLITCOLBTN
    void OnSplitColBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_MERGECOLSBTN
    void OnMergeColsBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EXPORTBTN
    void OnExportBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_WHITESPACEMSBTN
    void OnWhitespacemsBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_SEMICOLONMSBTN
    void OnSemicolonmsBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_COMMAMSBTN
    void OnCommamsBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_NONEMSBTN
    void OnNonemsBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_TEXTMSDELIMITER
    void OnTextMSDelimiterTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_LIST_ITEM_SELECTED event handler for ID_LISTCTRL
    void OnCTableImportListctrl2Selected( wxListEvent& event );

    /// wxEVT_COMMAND_LIST_COL_CLICK event handler for ID_LISTCTRL
    void OnListctrlColLeftClick( wxListEvent& event );

    /// wxEVT_COMMAND_LIST_COL_DRAGGING event handler for ID_LISTCTRL
    void OnCtableImportListctrl2ColDragging( wxListEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_DELIMROWTEXT
    void OnDelimRowTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_REVERTUPDATEBUTTON
    void OnRevertUpdateButtonClick( wxCommandEvent& event );

////@end CAttribTableDelimitersPanel event handler declarations

////@begin CAttribTableDelimitersPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAttribTableDelimitersPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAttribTableDelimitersPanel member variables
    wxCheckBox* m_TabDelimCheck;
    wxCheckBox* m_SemicolonDelimCheck;
    wxCheckBox* m_CommaDelimCheck;
    wxCheckBox* m_SpaceDelimCheck;
    wxCheckBox* m_MultipleSpacesOnly;
    wxCheckBox* m_OtherDelimCheck;
    wxTextCtrl* m_DelimTextCtrl;
    wxCheckBox* m_MergeAdjacentDelimiters;
    wxRadioBox* m_TextQualifier;
    wxCheckBox* m_ParseColumnNamesBtn;
    wxButton* m_SplitColBtn;
    wxButton* m_MergeColsBtn;
    wxStaticText* m_MergeSplitStaticText;
    wxRadioButton* m_TabMSBtn;
    wxRadioButton* m_SpaceMSBtn;
    wxRadioButton* m_WhitespaceMSBtn;
    wxRadioButton* m_ColonMSBtn;
    wxRadioButton* m_SemicolonMSBtn;
    wxRadioButton* m_CommaMSBtn;
    wxRadioButton* m_NoneMSBtn;
    wxRadioButton* m_OtherMSBtn;
    wxTextCtrl* m_DelimMSText;
    CTableImportListCtrl* m_DelimiterListCtrl;
    wxTextCtrl* m_DelimRowText;
    wxButton* m_RevertRowUpdate;
////@end CAttribTableDelimitersPanel member variables
protected:


    /// Disable/Enable widgets for splitting/merging specific columns after primary
    /// sptlit character(s) are chosen
    void x_EnableMergeSplitWidgets(bool b);

    /// Returns true if a character for merging or splitting table columns was
    /// selected. If is also a char used to split the table, duplicate is true.
    bool x_MergeSplitCharSelected(char& ch, bool& duplicate);


    string         m_RegPath;
    
    /// 
    bool m_DelimsInitialized;
    CRef<CTableImportDataSource> m_ImportedTableData;

    IUIToolManager* m_ToolManager;

    wxString m_PreviewFname;
    
    // In order to skip the first panel when we determine that the delimiter
    // is standard (tab) record whenever the file changes
    bool m_FileUpdated;

    int m_EditRowIdx;
    string m_EditRow;
};

END_NCBI_SCOPE

#endif // _ATTRIB_TABLE_DELIMITERS_PANEL_H_
