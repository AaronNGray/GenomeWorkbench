#ifndef GUI_WX_DEMO___ABOUT_DLG_HPP
#define GUI_WX_DEMO___ABOUT_DLG_HPP

/*  $Id: about_dlg.hpp 25633 2012-04-13 20:51:47Z katargir $
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


#include <corelib/ncbistl.hpp>

/*!
 * Includes
 */

#include <wx/dialog.h>


////@begin includes
#include "wx/html/htmlwin.h"
#include "wx/statline.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxHtmlWindow;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CWXABOUTDLG 10006
#define ID_HTMLWINDOW1 10007
#define ID_STATICLINE 10008
#define SYMBOL_CWXABOUTDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CWXABOUTDLG_TITLE _("About")
#define SYMBOL_CWXABOUTDLG_IDNAME ID_CWXABOUTDLG
#define SYMBOL_CWXABOUTDLG_SIZE wxSize(400, 300)
#define SYMBOL_CWXABOUTDLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * CwxAboutDlg class declaration
 */

BEGIN_NCBI_SCOPE;

class CwxAboutDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CwxAboutDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CwxAboutDlg();
    CwxAboutDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWXABOUTDLG_IDNAME, const wxString& caption = SYMBOL_CWXABOUTDLG_TITLE, const wxPoint& pos = SYMBOL_CWXABOUTDLG_POSITION, const wxSize& size = SYMBOL_CWXABOUTDLG_SIZE, long style = SYMBOL_CWXABOUTDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWXABOUTDLG_IDNAME, const wxString& caption = SYMBOL_CWXABOUTDLG_TITLE, const wxPoint& pos = SYMBOL_CWXABOUTDLG_POSITION, const wxSize& size = SYMBOL_CWXABOUTDLG_SIZE, long style = SYMBOL_CWXABOUTDLG_STYLE );

    /// Destructor
    ~CwxAboutDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CwxAboutDlg event handler declarations

////@end CwxAboutDlg event handler declarations

////@begin CwxAboutDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CwxAboutDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual bool TransferDataToWindow();

////@begin CwxAboutDlg member variables
    wxHtmlWindow* m_HTML;
////@end CwxAboutDlg member variables
};


END_NCBI_SCOPE;

#endif // GUI_WX_DEMO___ABOUT_DLG_HPP
