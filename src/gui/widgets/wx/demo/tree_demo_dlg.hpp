/////////////////////////////////////////////////////////////////////////////
// Name:        tree_demo_dlg.hpp
// Purpose:
// Author:      Vladimir Tereshkov
// Modified by:
// Created:     05/02/2008 14:43:25
// RCS-ID:
// Copyright:
// Licence:
/////////////////////////////////////////////////////////////////////////////

#ifndef _TREE_DEMO_DLG_H_
#define _TREE_DEMO_DLG_H_


#include <corelib/ncbistd.hpp>

#include "wx/dialog.h"

/*!
 * Includes
 */

////@begin includes
#include "wx/treectrl.h"
#include "wx/statline.h"
////@end includes

#include <wx/dialog.h>

/*!
 * Forward declarations
 */

BEGIN_NCBI_SCOPE
////@begin forward declarations
class CwxTreeCtrlEx;
////@end forward declarations
END_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CTREEDEMODLG 10001
#define ID_TREECTRL1 10002
#define SYMBOL_CTREEDEMODLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CTREEDEMODLG_TITLE _("Tree With Checkboxes")
#define SYMBOL_CTREEDEMODLG_IDNAME ID_CTREEDEMODLG
#define SYMBOL_CTREEDEMODLG_SIZE wxSize(400, 300)
#define SYMBOL_CTREEDEMODLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CTreeDemoDlg class declaration
 */

class CwxTreeCtrlEx;

class CTreeDemoDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CTreeDemoDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CTreeDemoDlg();
    CTreeDemoDlg( wxWindow* parent, wxWindowID id = SYMBOL_CTREEDEMODLG_IDNAME, const wxString& caption = SYMBOL_CTREEDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CTREEDEMODLG_POSITION, const wxSize& size = SYMBOL_CTREEDEMODLG_SIZE, long style = SYMBOL_CTREEDEMODLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTREEDEMODLG_IDNAME, const wxString& caption = SYMBOL_CTREEDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CTREEDEMODLG_POSITION, const wxSize& size = SYMBOL_CTREEDEMODLG_SIZE, long style = SYMBOL_CTREEDEMODLG_STYLE );

    /// Destructor
    ~CTreeDemoDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CTreeDemoDlg event handler declarations

////@end CTreeDemoDlg event handler declarations

////@begin CTreeDemoDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTreeDemoDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CTreeDemoDlg member variables
    CwxTreeCtrlEx* m_Tree;
////@end CTreeDemoDlg member variables

};

END_NCBI_SCOPE

#endif
    // _TREE_DEMO_DLG_H_
