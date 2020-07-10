/*  $Id: filter_dialog.hpp 19460 2009-06-22 20:00:43Z wuliangs $
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
 * Authors:  Yury Voronov
 *
 * File Description:
 *
 */

#ifndef _FILTER_DIALOG_H_
#define _FILTER_DIALOG_H_


#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>
#include <gui/core/dm_search_tool.hpp>
#include <wx/dialog.h>

/*!
 * Includes
 */


////@begin includes
////@end includes



/*!
 * Forward declarations
 */

class wxCheckListBox;
class wxTextCtrl;

////@begin forward declarations
////@end forward declarations
class wxCheckListBox;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CFILTERDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CFILTERDIALOG_TITLE _("Filter Selection Dialog")
#define SYMBOL_CFILTERDIALOG_IDNAME ID_CFILTERDIALOG
#define SYMBOL_CFILTERDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_CFILTERDIALOG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CFilterDialog class declaration
 */


BEGIN_NCBI_SCOPE

class CFilterDialog: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CFilterDialog )
    DECLARE_EVENT_TABLE()

public:
    CFilterDialog();
    CFilterDialog( wxWindow* parent, wxWindowID id = SYMBOL_CFILTERDIALOG_IDNAME, const wxString& caption = SYMBOL_CFILTERDIALOG_TITLE, const wxPoint& pos = SYMBOL_CFILTERDIALOG_POSITION, const wxSize& size = SYMBOL_CFILTERDIALOG_SIZE, long style = SYMBOL_CFILTERDIALOG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFILTERDIALOG_IDNAME, const wxString& caption = SYMBOL_CFILTERDIALOG_TITLE, const wxPoint& pos = SYMBOL_CFILTERDIALOG_POSITION, const wxSize& size = SYMBOL_CFILTERDIALOG_SIZE, long style = SYMBOL_CFILTERDIALOG_STYLE );

    ~CFilterDialog();

    void Init();

    void CreateControls();
    
    IDMSearchTool::TFilters & GetFilters() { return m_Filters ; }    

    
    bool TransferDataToWindow();

    bool TransferDataFromWindow();


////@begin CFilterDialog event handler declarations

    void OnChecklistbox1Selected( wxCommandEvent& event );

    void OnClearClicked( wxCommandEvent& event );

////@end CFilterDialog event handler declarations

////@begin CFilterDialog member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CFilterDialog member function declarations

    static bool ShowToolTips();

////@begin CFilterDialog member variables
    wxCheckListBox* m_ListBox;
    wxTextCtrl* m_Name;
    wxTextCtrl* m_Expression;
    enum {
        ID_CFILTERDIALOG = 10046,
        ID_CHECKLISTBOX1 = 10047,
        ID_TEXTCTRL4 = 10052,
        ID_TEXTCTRL5 = 10053,
        ID_BUTTON2 = 10048
    };
////@end CFilterDialog member variables
    
    IDMSearchTool::TFilters m_Filters;
protected:
    void x_SelectionUpdated();
};

END_NCBI_SCOPE

#endif
    // _FILTER_DIALOG_H_
