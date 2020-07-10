/*  $Id: filter_columns_dlg.hpp 34132 2015-11-09 14:07:36Z evgeniev $
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
#ifndef _FILTER_COLUMNS_DLG_H_
#define _FILTER_COLUMNS_DLG_H_

#include <corelib/ncbistd.hpp>
#include <set>
#include <wx/dialog.h>
#include <gui/objutils/grid_table_adapter.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

class wxBitmap;
class wxIcon;
class wxCheckListBox;
class wxTextCtrl;
class wxButton;

/*!
 * Control identifiers
 */

BEGIN_NCBI_SCOPE

////@begin control identifiers
#define ID_FILTER_COLUMNS_DLG 10000
#define ID_COLUMNS_CHECKLISTBOX 10004
#define ID_SELECT_ALL_BTN 10003
#define ID_DESELECT_BTN 10005
#define ID_RANGE_TEXTCTRL 10001
#define ID_SELECT_BTN 10002
#define SYMBOL_CFILTERCOLUMNSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CFILTERCOLUMNSDLG_TITLE _("Filter Columns")
#define SYMBOL_CFILTERCOLUMNSDLG_IDNAME ID_FILTER_COLUMNS_DLG
#define SYMBOL_CFILTERCOLUMNSDLG_SIZE wxSize(300, 180)
#define SYMBOL_CFILTERCOLUMNSDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CFilterColumnsDlg class declaration
 */

class CFilterColumnsDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CFilterColumnsDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFilterColumnsDlg();
    CFilterColumnsDlg(wxWindow* parent, wxWindowID id = SYMBOL_CFILTERCOLUMNSDLG_IDNAME, const wxString& caption = SYMBOL_CFILTERCOLUMNSDLG_TITLE, const wxPoint& pos = SYMBOL_CFILTERCOLUMNSDLG_POSITION, const wxSize& size = SYMBOL_CFILTERCOLUMNSDLG_SIZE, long style = SYMBOL_CFILTERCOLUMNSDLG_STYLE);

    void SetGridAdapter(IGridTableAdapter* gridAdapter);

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFILTERCOLUMNSDLG_IDNAME, const wxString& caption = SYMBOL_CFILTERCOLUMNSDLG_TITLE, const wxPoint& pos = SYMBOL_CFILTERCOLUMNSDLG_POSITION, const wxSize& size = SYMBOL_CFILTERCOLUMNSDLG_SIZE, long style = SYMBOL_CFILTERCOLUMNSDLG_STYLE );

    /// Destructor
    ~CFilterColumnsDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CFilterColumnsDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SELECT_ALL_BTN
    void OnSelectAllBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DESELECT_BTN
    void OnDeselectBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_RANGE_TEXTCTRL
    void OnRangeTextctrlTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SELECT_BTN
    void OnSelectBtnClick( wxCommandEvent& event );

////@end CFilterColumnsDlg event handler declarations

////@begin CFilterColumnsDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFilterColumnsDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CFilterColumnsDlg member variables
    wxCheckListBox* m_ColumnsLstBox;
    wxTextCtrl* m_RangeText;
    wxButton* m_SelectBtn;
////@end CFilterColumnsDlg member variables
protected:
    void x_ProcessSelectionPattern();

private:
    IGridTableAdapter*  m_GridAdapter;
};

END_NCBI_SCOPE

#endif
    // _FILTER_COLUMNS_DLG_H_
