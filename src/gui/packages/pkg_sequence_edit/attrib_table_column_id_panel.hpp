/*  $Id: attrib_table_column_id_panel.hpp 45097 2020-05-29 15:15:00Z asztalos $
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
#ifndef _ATTRIB_TABLE_COLUMN_ID_PANEL_H_
#define _ATTRIB_TABLE_COLUMN_ID_PANEL_H_

#include <corelib/ncbistd.hpp>
#include "gui/widgets/loaders/table_import_listctrl.hpp"
#include <gui/packages/pkg_sequence_edit/field_choice_panel.hpp>
#include <gui/utils/object_loader.hpp>
#include <objtools/edit/seqid_guesser.hpp>
#include <wx/listctrl.h>
#include <wx/bookctrl.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
class CTableImportListCtrl;
class CConstraintPanel;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CATTRIBTABLECOLUMNIDPANEL 10230
#define ID_COLUMNNAMETXT 10231
#define ID_SKIPBTN 10232
#define ID_MATCH_COLUMN 10261
#define ID_MATCHTYPE 10371
#define ID_REPLACE_BTN 10379
#define ID_APPEND_BTN 10132
#define ID_PREFIX_BTN 10133
#define ID_IGNORE_BTN 10134
#define ID_ADD_QUAL_BTN 10260
#define ID_SEMICOLON_BTN 10135
#define ID_SPACE_BTN 10136
#define ID_COLON_BTN 10137
#define ID_COMMA_BTN 10138
#define ID_NODELIMITER_BTN 10139
#define ID_IGNORE_BLANKS_BTN 10373
#define ID_ERASE_EXISTING_VALUE_BTN 10374
#define ID_COLUMNIDLISTPANEL 10233
#define ID_CTABLEIMPORTLISTCTRL3 10234
#define ID_SPLIT_FIRST_COL 10372
#define ID_MULTISPACE_TO_TAB_BTN 10375
#define ID_MERGEIDCOLS 10301
#define ID_MAIL_REPORT_CHECKBOX 10292
#define ID_APPLY_NO_CLOSE 10368
#define SYMBOL_CATTRIBTABLECOLUMNIDPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CATTRIBTABLECOLUMNIDPANEL_TITLE _("Set Attribute Column Format")
#define SYMBOL_CATTRIBTABLECOLUMNIDPANEL_IDNAME ID_CATTRIBTABLECOLUMNIDPANEL
#define SYMBOL_CATTRIBTABLECOLUMNIDPANEL_SIZE wxDefaultSize
#define SYMBOL_CATTRIBTABLECOLUMNIDPANEL_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CAttribTableColumnIdPanel class declaration
 */

class CAttribTableColumnIdPanel: public wxPanel
                               , public CFieldNamePanelParent
{    
    DECLARE_DYNAMIC_CLASS( CAttribTableColumnIdPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAttribTableColumnIdPanel();
    CAttribTableColumnIdPanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CATTRIBTABLECOLUMNIDPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CATTRIBTABLECOLUMNIDPANEL_POSITION, 
        const wxSize& size = SYMBOL_CATTRIBTABLECOLUMNIDPANEL_SIZE, 
        long style = SYMBOL_CATTRIBTABLECOLUMNIDPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CATTRIBTABLECOLUMNIDPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CATTRIBTABLECOLUMNIDPANEL_POSITION, 
        const wxSize& size = SYMBOL_CATTRIBTABLECOLUMNIDPANEL_SIZE, 
        long style = SYMBOL_CATTRIBTABLECOLUMNIDPANEL_STYLE );

    /// Destructor
    ~CAttribTableColumnIdPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Override Show() to allow initialization each time window is displayed
    bool Show(bool show);

    void SetMainTitle(const wxString& title);

    void SetImportDataSource(CRef<CTableImportDataSource> ds) { m_ImportedTableData = ds; }
    void SetTopLevelEntry(CSeq_entry_Handle seq_entry);
    void SetFileNames(vector<wxString> filenames) { m_FileNames = filenames; }
    void SetCommandProcessor(ICommandProccessor* cp) { m_CmdProccessor = cp; }

    void UpdateCol();
    bool IsInputValid();


    void SetRegistryPath( const string& path );
    void LoadSettings();
    void SaveSettings() const;

    void ProcessUpdateFeatEvent( wxCommandEvent& event );
    void ProcessUpdateStructCommentEvent( wxCommandEvent& event );

////@begin CAttribTableColumnIdPanel event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_COLUMNNAMETXT
    void OnColumnNameTxtTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_SKIPBTN
    void OnSkipBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_MATCH_COLUMN
    void OnMatchColumnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBOX_SELECTED event handler for ID_MATCHTYPE
    void OnMatchTypeSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_REPLACE_BTN
    void OnReplaceBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_APPEND_BTN
    void OnAppendBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_PREFIX_BTN
    void OnPrefixBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_IGNORE_BTN
    void OnIgnoreBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ADD_QUAL_BTN
    void OnAddQualBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_SEMICOLON_BTN
    void OnSemicolonBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_SPACE_BTN
    void OnSpaceBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_COLON_BTN
    void OnColonBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_COMMA_BTN
    void OnCommaBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_NODELIMITER_BTN
    void OnNodelimiterBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_IGNORE_BLANKS_BTN
    void OnIgnoreBlanksBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ERASE_EXISTING_VALUE_BTN
    void OnEraseExistingValueBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_LIST_COL_CLICK event handler for ID_CTABLEIMPORTLISTCTRL3
    void OnCtableImportListctrlColLeftClick( wxListEvent& event );

    /// wxEVT_COMMAND_LIST_COL_DRAGGING event handler for ID_CTABLEIMPORTLISTCTRL3
    void OnCtableImportListCtrlColDragging( wxListEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SPLIT_FIRST_COL
    void OnSplitFirstColClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_MULTISPACE_TO_TAB_BTN
    void OnMultispaceToTabBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_MERGEIDCOLS
    void OnMergeIDColsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_MAIL_REPORT_CHECKBOX
    void OnMailReportCheckboxClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_APPLY_NO_CLOSE
    void OnApplyNoCloseClick( wxCommandEvent& event );
    
    /// Refresh the parent dialog upon changing the field type in the constraint panel 
    void OnPageChanged(wxBookCtrlEvent& event);

////@end CAttribTableColumnIdPanel event handler declarations

////@begin CAttribTableColumnIdPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAttribTableColumnIdPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void UpdateEditor();
    bool ShouldMakeMailReport() { return m_MakeMailReport->GetValue(); };
    void x_TableReaderCommon(CIRef<IObjectLoader> object_loader, const wxString& msg, bool modal = false);

private:
////@begin CAttribTableColumnIdPanel member variables
    wxStaticBox* m_ColumnPropertiesSizer;
    wxStaticText* m_ColumnNameStaticTxt;
    wxTextCtrl* m_ColumnNameTxtCtrl;
    wxCheckBox* m_SkipFormatBtn;
    wxCheckBox* m_MatchColumn;
    wxRadioBox* m_MatchType;

    CFieldChoicePanel* m_Fields;

    wxRadioButton* m_Replace;
    wxRadioButton* m_Append;
    wxRadioButton* m_Prefix;
    wxRadioButton* m_LeaveOld;
    wxRadioButton* m_AddQual;

    wxRadioButton* m_Semicolon;
    wxRadioButton* m_Space;
    wxRadioButton* m_Colon;
    wxRadioButton* m_Comma;
    wxRadioButton* m_NoDelimiter;

    wxRadioButton* m_IgnoreBlanks;
    wxRadioButton* m_EraseValue;

    CTableImportListCtrl* m_ColumnIdList;
    CConstraintPanel* m_Constraint;

    wxButton* m_MergeIDCols;
    wxCheckBox* m_MakeMailReport;
////@end CAttribTableColumnIdPanel member variables

protected:
    /// Guess column qualifiers - called on Show() or if table updated significantly
    void x_GuessQualifiers();

    /// If a column is currently selected, the controls to edit its type and name
    /// should be available, otherwise they should be grayed out.
    void x_EnableColumnEditControls(bool b);

    /// Enable/ disable fields for match column
    void x_EnableMachColumnFields(bool match_column);

    /// Enables all controls except the format buttons since they
    /// can be used to skip a column (which greys-out other buttons)
    void x_EnableNonFormatEditControls(bool b);

    /// Lists the features present in the top seq-entry first in the feature listbox
    void UpdateChildrenFeaturePanels( wxWindow* win );
    void UpdateStructuredCommentFields( wxWindow* win, const set<string> &fields );

    string x_GetConstraintField(void);

    void x_RefreshParentWnd();

    string m_RegPath;

    /// Index within listctrl/datasource of column being edited or -1
    int  m_CurrentColumnIdx;

    /// Contents of tabular file
    CRef<CTableImportDataSource> m_ImportedTableData; 

    /// Top level seq submit entry
    CSeq_entry_Handle m_TopSeqEntry;
    shared_ptr<edit::CSeqIdGuesser>  m_id_guesser;

    /// Fields to remember type information for col set to seq-id so that
    /// the values can be restored if seq-id is unset
    string m_PrevColName;
    string m_PrevQualType;
    string m_PrevQual;

    /// Previous size of m_Fields - used to resize dialog
    /// as m_Fields layout changes
    wxSize m_FieldsSize;

    vector<wxString> m_FileNames;
    ICommandProccessor* m_CmdProccessor;
};




#define ID_CLONGMESSAGEDLG wxID_ANY
#define SYMBOL_CLONGMESSAGEDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CLONGMESSAGEDLG_TITLE _("ID Errors")
#define SYMBOL_CLONGMESSAGEDLG_IDNAME ID_CLONGMESSAGEDLG
#define SYMBOL_CLONGMESSAGEDLG_SIZE wxSize(400, 300)
#define SYMBOL_CLONGMESSAGEDLG_POSITION wxDefaultPosition

class CLongMessageDialog: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CLongMessageDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CLongMessageDialog();
    CLongMessageDialog( wxWindow* parent,  wxString msg, 
                        wxWindowID id = SYMBOL_CLONGMESSAGEDLG_IDNAME, 
                        const wxString& caption = SYMBOL_CLONGMESSAGEDLG_TITLE, 
                        const wxPoint& pos = SYMBOL_CLONGMESSAGEDLG_POSITION, 
                        const wxSize& size = SYMBOL_CLONGMESSAGEDLG_SIZE, 
                        long style = SYMBOL_CLONGMESSAGEDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
                wxWindowID id = SYMBOL_CLONGMESSAGEDLG_IDNAME, 
                const wxString& caption = SYMBOL_CLONGMESSAGEDLG_TITLE, 
                const wxPoint& pos = SYMBOL_CLONGMESSAGEDLG_POSITION, 
                const wxSize& size = SYMBOL_CLONGMESSAGEDLG_SIZE, 
                long style = SYMBOL_CLONGMESSAGEDLG_STYLE );

    /// Destructor
    ~CLongMessageDialog();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CGenericReportDlg event handler declarations


////@end CGenericReportDlg event handler declarations
    
////@begin CGenericReportDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGenericReportDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void OnClose(wxCloseEvent& event);
    void OnYesClick( wxCommandEvent& event );
    void OnNoClick( wxCommandEvent& event );

private:
    wxString m_Text;
};

END_NCBI_SCOPE

#endif // _ATTRIB_TABLE_COLUMN_ID_PANEL_H_
