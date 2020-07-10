/*  $Id: phylo_save_selection_dlg.hpp 32608 2015-03-31 20:04:19Z falkrb $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */
#ifndef _PHYLO_SAVE_SELECTION_DLG_H_
#define _PHYLO_SAVE_SELECTION_DLG_H_

#define _WCHAR_H_CPLUSPLUS_98_CONFORMANCE_

/*!
 * Includes
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/widgets/phylo_tree/phylo_tree_ds.hpp>

////@begin includes
#include "wx/listctrl.h"
#include "wx/clrpicker.h"
#include "wx/dialog.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxListCtrl;
class wxColourPickerCtrl;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CPHYLOSAVESELECTIONDLG 10054
#define ID_SEL_LISTCTRL 10057
#define ID_SELECTION 10055
#define ID_SEL_COLOR_PICKER 10059
#define ID_BITMAPBUTTON_UP 10056
#define ID_BITMAPBUTTON_DOWN 10058
#define ID_DELETE_SEL_BTN 10060
#define ID_ADD_CURRENT_SEL_BTN 10061
#define SYMBOL_CPHYLOSAVESELECTIONDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CPHYLOSAVESELECTIONDLG_TITLE _("Save Selection")
#define SYMBOL_CPHYLOSAVESELECTIONDLG_IDNAME ID_CPHYLOSAVESELECTIONDLG
#define SYMBOL_CPHYLOSAVESELECTIONDLG_SIZE wxDefaultSize
#define SYMBOL_CPHYLOSAVESELECTIONDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CPhyloSaveSelectiondlg class declaration
 */

class CPhyloSaveSelectiondlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CPhyloSaveSelectiondlg )
    DECLARE_EVENT_TABLE()

public:
    CPhyloSaveSelectiondlg();
    CPhyloSaveSelectiondlg( wxWindow* parent, wxWindowID id = SYMBOL_CPHYLOSAVESELECTIONDLG_IDNAME, const wxString& caption = SYMBOL_CPHYLOSAVESELECTIONDLG_TITLE, const wxPoint& pos = SYMBOL_CPHYLOSAVESELECTIONDLG_POSITION, const wxSize& size = SYMBOL_CPHYLOSAVESELECTIONDLG_SIZE, long style = SYMBOL_CPHYLOSAVESELECTIONDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPHYLOSAVESELECTIONDLG_IDNAME, const wxString& caption = SYMBOL_CPHYLOSAVESELECTIONDLG_TITLE, const wxPoint& pos = SYMBOL_CPHYLOSAVESELECTIONDLG_POSITION, const wxSize& size = SYMBOL_CPHYLOSAVESELECTIONDLG_SIZE, long style = SYMBOL_CPHYLOSAVESELECTIONDLG_STYLE );

    ~CPhyloSaveSelectiondlg();

    void Init();

    void CreateControls();

    void SetSelections(CRef<CPhyloTreeDataSource> ds, string name);

    void UpdateSelections();

////@begin CPhyloSaveSelectiondlg event handler declarations

    void OnSelListctrlSelected( wxListEvent& event );

    void OnLeftDown( wxMouseEvent& event );

    void OnSelectionTextUpdated( wxCommandEvent& event );

    void OnSelColorPickerColourPickerChanged( wxColourPickerEvent& event );

    void OnBitmapbuttonUpClick( wxCommandEvent& event );

    void OnBitmapbuttonDownClick( wxCommandEvent& event );

    void OnDeleteSelBtnClick( wxCommandEvent& event );

    void OnAddCurrentSelBtnClick( wxCommandEvent& event );

    void OnOkClick( wxCommandEvent& event );

////@end CPhyloSaveSelectiondlg event handler declarations

////@begin CPhyloSaveSelectiondlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CPhyloSaveSelectiondlg member function declarations

    static bool ShowToolTips();

protected:
    void x_EnableCurrentSelection(bool b);

    void x_SetClusterIds();

////@begin CPhyloSaveSelectiondlg member variables
    wxListCtrl* m_SelList;
    wxTextCtrl* m_SelectionName;
    wxColourPickerCtrl* m_SelColorPicker;
    wxButton* m_ButtonUp;
    wxButton* m_ButtonDown;
    wxButton* m_DeleteBtn;
    wxButton* m_AddSelectionBtn;
////@end CPhyloSaveSelectiondlg member variables

    CRef<CPhyloTreeDataSource> m_DS;
    long m_CurrentItem;
};

#endif
    // _PHYLO_SAVE_SELECTION_DLG_H_
END_NCBI_SCOPE
