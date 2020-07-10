#ifndef _ABOUTDLG_H_
#define _ABOUTDLG_H_

/*  $Id: aboutdlg.hpp 30445 2014-05-20 20:08:44Z katargir $
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

#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/statbmp.h>

////@begin includes
#include "wx/hyperlink.h"
#include "wx/statline.h"
////@end includes

////@begin forward declarations
////@end forward declarations

////@begin control identifiers
#define SYMBOL_CABOUTDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CABOUTDLG_TITLE _("About Genome Workbench")
#define SYMBOL_CABOUTDLG_IDNAME ID_ABOUTDLG
#define SYMBOL_CABOUTDLG_SIZE wxSize(400, 300)
#define SYMBOL_CABOUTDLG_POSITION wxDefaultPosition
////@end control identifiers
///

BEGIN_NCBI_SCOPE

class CAboutDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CAboutDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAboutDlg();
    CAboutDlg( wxWindow* parent, wxWindowID id = SYMBOL_CABOUTDLG_IDNAME, const wxString& caption = SYMBOL_CABOUTDLG_TITLE, const wxPoint& pos = SYMBOL_CABOUTDLG_POSITION, const wxSize& size = SYMBOL_CABOUTDLG_SIZE, long style = SYMBOL_CABOUTDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CABOUTDLG_IDNAME, const wxString& caption = SYMBOL_CABOUTDLG_TITLE, const wxPoint& pos = SYMBOL_CABOUTDLG_POSITION, const wxSize& size = SYMBOL_CABOUTDLG_SIZE, long style = SYMBOL_CABOUTDLG_STYLE );

    /// Destructor
    ~CAboutDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAboutDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
    void OnOK( wxCommandEvent& event );

////@end CAboutDlg event handler declarations

////@begin CAboutDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAboutDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAboutDlg member variables
    wxStaticBitmap* m_Image;
    wxStaticText* m_VersionString;
    wxStaticText* m_BuildDate;
    /// Control identifiers
    enum {
        ID_ABOUTDLG = 10021,
        m_HYPERLINKCTRL = 10036,
        ID_BUTTON = 10022
    };
////@end CAboutDlg member variables
};

END_NCBI_SCOPE

#endif  // _ABOUTDLG_H_
