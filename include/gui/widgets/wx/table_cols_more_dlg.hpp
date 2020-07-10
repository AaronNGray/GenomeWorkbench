#ifndef GUI_WIDGETS_WX__TABLE_COLS_MORE_DLG__HPP
#define GUI_WIDGETS_WX__TABLE_COLS_MORE_DLG__HPP

/*  $Id: table_cols_more_dlg.hpp 23822 2011-06-08 22:14:04Z voronov $
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
 * Authors:  Roman Katargin
 */
#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */
#include <gui/widgets/wx/dialog.hpp>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <gui/widgets/wx/table_listctrl.hpp>

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

BEGIN_NCBI_SCOPE

////@begin control identifiers
#define SYMBOL_CMORETABLECOLSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CMORETABLECOLSDLG_TITLE _("More on Table Columns")
#define SYMBOL_CMORETABLECOLSDLG_IDNAME ID_MORECOLDLG
#define SYMBOL_CMORETABLECOLSDLG_SIZE wxSize(399, 299)
#define SYMBOL_CMORETABLECOLSDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CMoreTableColsDlg class declaration
 */

class CMoreTableColsDlg: public CDialog
{    
    DECLARE_DYNAMIC_CLASS( CMoreTableColsDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMoreTableColsDlg();
    CMoreTableColsDlg( wxWindow* parent, wxWindowID id = SYMBOL_CMORETABLECOLSDLG_IDNAME, const wxString& caption = SYMBOL_CMORETABLECOLSDLG_TITLE, const wxPoint& pos = SYMBOL_CMORETABLECOLSDLG_POSITION, const wxSize& size = SYMBOL_CMORETABLECOLSDLG_SIZE, long style = SYMBOL_CMORETABLECOLSDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CMORETABLECOLSDLG_IDNAME, const wxString& caption = SYMBOL_CMORETABLECOLSDLG_TITLE, const wxPoint& pos = SYMBOL_CMORETABLECOLSDLG_POSITION, const wxSize& size = SYMBOL_CMORETABLECOLSDLG_SIZE, long style = SYMBOL_CMORETABLECOLSDLG_STYLE );

    /// Destructor
    ~CMoreTableColsDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

private:
    void SetTable( CwxTableListCtrl* table );
    void x_UpdateModel();
    void x_UpdateButtons();

public:
////@begin CMoreTableColsDlg event handler declarations

    /// wxEVT_COMMAND_LIST_ITEM_SELECTED event handler for ID_COLTABLE
    void OnColTableSelected( wxListEvent& event );

    /// wxEVT_COMMAND_LIST_ITEM_DESELECTED event handler for ID_COLTABLE
    void OnColTableDeselected( wxListEvent& event );

    /// wxEVT_COMMAND_LIST_ITEM_FOCUSED event handler for ID_COLTABLE
    void OnColTableRowFocused( wxListEvent& event );

    /// wxEVT_COMMAND_LIST_KEY_DOWN event handler for ID_COLTABLE
    void OnColTableKeyDown( wxListEvent& event );

    /// wxEVT_COMMAND_LIST_COL_RIGHT_CLICK event handler for ID_COLTABLE
    void OnColTableHeaderMenu( wxListEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SHOW
    void OnShowClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_HIDE
    void OnSortingClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CLEAR_SORT
    void OnClearSortClick( wxCommandEvent& event );

////@end CMoreTableColsDlg event handler declarations

    /// wxEVT_LEFT_DOWN event handler for ID_COLTABLE
    //! It is out of DialogBlocks because of Table ID Linux issue [YV]
    void OnColTableLeftDown( wxMouseEvent& event );

    void EndModal( int retCode );

////@begin CMoreTableColsDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMoreTableColsDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

private:
////@begin CMoreTableColsDlg member variables
    CwxTableListCtrl* m_ColTable;
    wxButton* m_ShowBtn;
    wxButton* m_SortBtn;
    wxButton* m_ClearBtn;
    CTextTableModel m_Model;
    CwxTableListCtrl* m_BaseTable;
    vector<bool> m_Shown;
    vector<CwxTableListCtrl::TSortedCol> m_Sorted;
    /// Control identifiers
    enum {
        ID_MORECOLDLG = 10014,
        ID_COLTABLE = 10003,
        ID_SHOW = 10004,
        ID_HIDE = 10006,
        ID_CLEAR_SORT = 10003
    };
////@end CMoreTableColsDlg member variables
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX__TABLE_COLS_MORE_DLG__HPP
