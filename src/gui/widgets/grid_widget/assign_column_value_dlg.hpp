#ifndef GUI_WIDGETS_GRID_WIDGET___ASSIGN_COLUMN_VALUE_DLG__HPP
#define GUI_WIDGETS_GRID_WIDGET___ASSIGN_COLUMN_VALUE_DLG__HPP

/*  $Id: assign_column_value_dlg.hpp 26151 2012-07-26 18:02:26Z katargir $
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

#include <gui/widgets/wx/dialog.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/valtext.h"
#include "wx/valgen.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CASSIGNCOLUMNVALUEDLG 10003
#define ID_COMBOBOX 10004
#define ID_COMBOBOX1 10005
#define ID_RADIOBOX1 10000
#define SYMBOL_CASSIGNCOLUMNVALUEDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CASSIGNCOLUMNVALUEDLG_TITLE _("Assign column value dlg")
#define SYMBOL_CASSIGNCOLUMNVALUEDLG_IDNAME ID_CASSIGNCOLUMNVALUEDLG
#define SYMBOL_CASSIGNCOLUMNVALUEDLG_SIZE wxSize(400, 300)
#define SYMBOL_CASSIGNCOLUMNVALUEDLG_POSITION wxDefaultPosition
////@end control identifiers

class wxComboBox;

BEGIN_NCBI_SCOPE

class ITableData;
class IEditCommand;

/*!
 * CAssignColumnValueDlg class declaration
 */

class CAssignColumnValueDlg: public CDialog
{    
    DECLARE_DYNAMIC_CLASS( CAssignColumnValueDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAssignColumnValueDlg();
    CAssignColumnValueDlg( wxWindow* parent, ITableData& table );

    /// @name IRegSettings interface
    /// @{
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CASSIGNCOLUMNVALUEDLG_IDNAME, const wxString& caption = SYMBOL_CASSIGNCOLUMNVALUEDLG_TITLE, const wxPoint& pos = SYMBOL_CASSIGNCOLUMNVALUEDLG_POSITION, const wxSize& size = SYMBOL_CASSIGNCOLUMNVALUEDLG_SIZE, long style = SYMBOL_CASSIGNCOLUMNVALUEDLG_STYLE );

    /// Destructor
    ~CAssignColumnValueDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    IEditCommand* GetEditCommand();

////@begin CAssignColumnValueDlg event handler declarations

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX
    void OnComboboxSelected( wxCommandEvent& event );

////@end CAssignColumnValueDlg event handler declarations

////@begin CAssignColumnValueDlg member function declarations

    wxString GetValue() const { return m_Value ; }
    void SetValue(wxString value) { m_Value = value ; }

    int GetColumn() const { return m_Column ; }
    void SetColumn(int value) { m_Column = value ; }

    int GetOp() const { return m_Op ; }
    void SetOp(int value) { m_Op = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAssignColumnValueDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual bool TransferDataFromWindow();

////@begin CAssignColumnValueDlg member variables
    wxComboBox* m_Columns;
    wxComboBox* m_Values;
private:
    wxString m_Value;
    int m_Column;
    int m_Op;
////@end CAssignColumnValueDlg member variables

private:
    void x_UpdateValues();

    ITableData* m_Table;

public:
    static bool CanAssignColumn(ITableData& table);
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_GRID_WIDGET___ASSIGN_COLUMN_VALUE_DLG__HPP
