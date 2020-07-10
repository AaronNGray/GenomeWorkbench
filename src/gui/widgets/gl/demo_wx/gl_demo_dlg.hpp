/////////////////////////////////////////////////////////////////////////////
// Name:        gl_demo_dlg.hpp
// Purpose:
// Author:      Roman Katargin
// Modified by:
// Created:     07/08/2007 16:16:39
// RCS-ID:
// Copyright:
// Licence:
/////////////////////////////////////////////////////////////////////////////

#ifndef _GL_DEMO_DLG_H_
#define _GL_DEMO_DLG_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

#include "wx/dialog.h"

////@begin includes
#include "wx/notebook.h"
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
#define ID_CGLDEMODLG 10000
#define ID_NOTEBOOK1 10001
#define SYMBOL_CGLDEMODLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CGLDEMODLG_TITLE _("wxWidgets GL Demo")
#define SYMBOL_CGLDEMODLG_IDNAME ID_CGLDEMODLG
#define SYMBOL_CGLDEMODLG_SIZE wxDefaultSize
#define SYMBOL_CGLDEMODLG_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

/*!
 * CGLDemoDlg class declaration
 */

class CGLDemoDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CGLDemoDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CGLDemoDlg();
    CGLDemoDlg( wxWindow* parent, wxWindowID id = SYMBOL_CGLDEMODLG_IDNAME, const wxString& caption = SYMBOL_CGLDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CGLDEMODLG_POSITION, const wxSize& size = SYMBOL_CGLDEMODLG_SIZE, long style = SYMBOL_CGLDEMODLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CGLDEMODLG_IDNAME, const wxString& caption = SYMBOL_CGLDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CGLDEMODLG_POSITION, const wxSize& size = SYMBOL_CGLDEMODLG_SIZE, long style = SYMBOL_CGLDEMODLG_STYLE );

    /// Destructor
    ~CGLDemoDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CGLDemoDlg event handler declarations

    /// wxEVT_CLOSE_WINDOW event handler for ID_CGLDEMODLG
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
    void OnCloseClick( wxCommandEvent& event );

////@end CGLDemoDlg event handler declarations

////@begin CGLDemoDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGLDemoDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CGLDemoDlg member variables
////@end CGLDemoDlg member variables
};

END_NCBI_SCOPE

#endif
    // _GL_DEMO_DLG_H_
