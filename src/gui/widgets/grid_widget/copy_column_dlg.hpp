#ifndef GUI_WIDGETS_GRID_WIDGET___COPY_COLUMN_DLG__HPP
#define GUI_WIDGETS_GRID_WIDGET___COPY_COLUMN_DLG__HPP

/*  $Id: copy_column_dlg.hpp 26153 2012-07-26 18:15:25Z katargir $
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
 * Authors:  
 */

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/dialog.hpp>

/*!
 * Includes
 */

////@begin includes
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
#define ID_CCOPYCOLUMNDLG 10006
#define ID_COMBOBOX2 10007
#define ID_COMBOBOX3 10008
#define ID_RADIOBOX 10009
#define SYMBOL_CCOPYCOLUMNDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CCOPYCOLUMNDLG_TITLE _("Copy column")
#define SYMBOL_CCOPYCOLUMNDLG_IDNAME ID_CCOPYCOLUMNDLG
#define SYMBOL_CCOPYCOLUMNDLG_SIZE wxSize(400, 300)
#define SYMBOL_CCOPYCOLUMNDLG_POSITION wxDefaultPosition
////@end control identifiers


class wxComboBox;

BEGIN_NCBI_SCOPE

class ITableData;
class IEditCommand;

/*!
 * CCopyColumnDlg class declaration
 */

class CCopyColumnDlg: public CDialog
{    
    DECLARE_DYNAMIC_CLASS( CCopyColumnDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCopyColumnDlg();
    CCopyColumnDlg( wxWindow* parent, ITableData& table );

    /// @name IRegSettings interface
    /// @{
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCOPYCOLUMNDLG_IDNAME, const wxString& caption = SYMBOL_CCOPYCOLUMNDLG_TITLE, const wxPoint& pos = SYMBOL_CCOPYCOLUMNDLG_POSITION, const wxSize& size = SYMBOL_CCOPYCOLUMNDLG_SIZE, long style = SYMBOL_CCOPYCOLUMNDLG_STYLE );

    /// Destructor
    ~CCopyColumnDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    IEditCommand* GetEditCommand();

////@begin CCopyColumnDlg event handler declarations

////@end CCopyColumnDlg event handler declarations

////@begin CCopyColumnDlg member function declarations

    int GetOp() const { return m_Op ; }
    void SetOp(int value) { m_Op = value ; }

    int GetColumnFrom() const { return m_ColumnFrom ; }
    void SetColumnFrom(int value) { m_ColumnFrom = value ; }

    int GetColumnTo() const { return m_ColumnTo ; }
    void SetColumnTo(int value) { m_ColumnTo = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCopyColumnDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual bool TransferDataFromWindow();

////@begin CCopyColumnDlg member variables
    wxComboBox* m_ColumnFromCtrl;
    wxComboBox* m_ColumnToCtrl;
    int m_ColumnFrom;
private:
    int m_Op;
    int m_ColumnTo;
////@end CCopyColumnDlg member variables

    ITableData* m_Table;

public:
    static bool CanCopyColumn(ITableData& table);
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_GRID_WIDGET___COPY_COLUMN_DLG__HPP
