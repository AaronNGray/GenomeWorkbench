/////////////////////////////////////////////////////////////////////////////
// Name:        hello_world_dlg.hpp
// Purpose:
// Author:      Roman Katargin
// Modified by:
// Created:     06/08/2007 16:56:20
// RCS-ID:
// Copyright:
// Licence:
/////////////////////////////////////////////////////////////////////////////

#ifndef _HELLO_WORLD_DLG_H_
#define _HELLO_WORLD_DLG_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

#include <wx/dialog.h>

////@begin includes
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
#define ID_CHELLOWORLDDLG 10000
#define SYMBOL_CHELLOWORLDDLG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CHELLOWORLDDLG_TITLE _("Empty wxWidets/C++ Toolkit app")
#define SYMBOL_CHELLOWORLDDLG_IDNAME ID_CHELLOWORLDDLG
#define SYMBOL_CHELLOWORLDDLG_SIZE wxSize(400, 300)
#define SYMBOL_CHELLOWORLDDLG_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

/*!
 * CHelloWorldDlg class declaration
 */

class CHelloWorldDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CHelloWorldDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CHelloWorldDlg();
    CHelloWorldDlg( wxWindow* parent, wxWindowID id = SYMBOL_CHELLOWORLDDLG_IDNAME, const wxString& caption = SYMBOL_CHELLOWORLDDLG_TITLE, const wxPoint& pos = SYMBOL_CHELLOWORLDDLG_POSITION, const wxSize& size = SYMBOL_CHELLOWORLDDLG_SIZE, long style = SYMBOL_CHELLOWORLDDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CHELLOWORLDDLG_IDNAME, const wxString& caption = SYMBOL_CHELLOWORLDDLG_TITLE, const wxPoint& pos = SYMBOL_CHELLOWORLDDLG_POSITION, const wxSize& size = SYMBOL_CHELLOWORLDDLG_SIZE, long style = SYMBOL_CHELLOWORLDDLG_STYLE );

    /// Destructor
    ~CHelloWorldDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CHelloWorldDlg event handler declarations

////@end CHelloWorldDlg event handler declarations

////@begin CHelloWorldDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CHelloWorldDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CHelloWorldDlg member variables
////@end CHelloWorldDlg member variables
};

END_NCBI_SCOPE

#endif
    // _HELLO_WORLD_DLG_H_
