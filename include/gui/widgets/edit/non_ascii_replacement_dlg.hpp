#ifndef GUI_WIDGETS_EDIT___NON_ASCII_REPLACEMENT_DLG__HPP
#define GUI_WIDGETS_EDIT___NON_ASCII_REPLACEMENT_DLG__HPP

/*  $Id: non_ascii_replacement_dlg.hpp 40562 2018-03-09 14:59:21Z filippov $
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
 * Authors:  Igor Filippov
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

BEGIN_NCBI_SCOPE

NCBI_GUIWIDGETS_EDIT_EXPORT void TestForNonAsciiText(wxWindow *win);


////@begin control identifiers
#define ID_CNONASCIICHARACTERREPLACEMENT wxID_ANY
#define ID_SCROLLEDWINDOW wxID_ANY
#define SYMBOL_CNONASCIICHARACTERREPLACEMENT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CNONASCIICHARACTERREPLACEMENT_TITLE _("non-ascii character replacement")
#define SYMBOL_CNONASCIICHARACTERREPLACEMENT_IDNAME ID_CNONASCIICHARACTERREPLACEMENT
#define SYMBOL_CNONASCIICHARACTERREPLACEMENT_SIZE wxSize(400, 300)
#define SYMBOL_CNONASCIICHARACTERREPLACEMENT_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CNonAsciiCharacterReplacement class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CNonAsciiCharacterReplacement : public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CNonAsciiCharacterReplacement )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CNonAsciiCharacterReplacement();
    CNonAsciiCharacterReplacement( wxWindow* parent, const map<TUnicodeSymbol, string> &nonAsciiChars, const map<TUnicodeSymbol, set<wxString> > &nonAsciiContext, wxWindowID id = SYMBOL_CNONASCIICHARACTERREPLACEMENT_IDNAME, const wxString& caption = SYMBOL_CNONASCIICHARACTERREPLACEMENT_TITLE, const wxPoint& pos = SYMBOL_CNONASCIICHARACTERREPLACEMENT_POSITION, const wxSize& size = SYMBOL_CNONASCIICHARACTERREPLACEMENT_SIZE, long style = SYMBOL_CNONASCIICHARACTERREPLACEMENT_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CNONASCIICHARACTERREPLACEMENT_IDNAME, const wxString& caption = SYMBOL_CNONASCIICHARACTERREPLACEMENT_TITLE, const wxPoint& pos = SYMBOL_CNONASCIICHARACTERREPLACEMENT_POSITION, const wxSize& size = SYMBOL_CNONASCIICHARACTERREPLACEMENT_SIZE, long style = SYMBOL_CNONASCIICHARACTERREPLACEMENT_STYLE );

    /// Destructor
    ~CNonAsciiCharacterReplacement();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CNonAsciiCharacterReplacement event handler declarations

////@end CNonAsciiCharacterReplacement event handler declarations

////@begin CNonAsciiCharacterReplacement member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CNonAsciiCharacterReplacement member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    map<TUnicodeSymbol, string> GetReplacementMap();
    bool TransferDataFromWindow();
private:
////@begin CNonAsciiCharacterReplacement member variables
    wxScrolledWindow* m_ScrolledWindow;
////@end CNonAsciiCharacterReplacement member variables

    map<TUnicodeSymbol, string> m_NonAsciiChars;
    map<TUnicodeSymbol, set<wxString> > m_NonAsciiContext;
};

////@begin control identifiers
#define ID_CREPLACEMENTPANEL wxID_ANY
#define SYMBOL_CREPLACEMENTPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CREPLACEMENTPANEL_IDNAME ID_CREPLACEMENTPANEL
#define SYMBOL_CREPLACEMENTPANEL_SIZE wxDefaultSize
#define SYMBOL_CREPLACEMENTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CNonAsciiReplacementPanel class declaration
 */

class CNonAsciiReplacementPanel: public wxPanel
{    
    DECLARE_CLASS( CNonAsciiReplacementPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CNonAsciiReplacementPanel();
    CNonAsciiReplacementPanel( wxWindow* parent, wxUniChar orig, const string &replacement, const set<wxString> &contexts, wxWindowID id = SYMBOL_CREPLACEMENTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CREPLACEMENTPANEL_POSITION, const wxSize& size = SYMBOL_CREPLACEMENTPANEL_SIZE, long style = SYMBOL_CREPLACEMENTPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CREPLACEMENTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CREPLACEMENTPANEL_POSITION, const wxSize& size = SYMBOL_CREPLACEMENTPANEL_SIZE, long style = SYMBOL_CREPLACEMENTPANEL_STYLE );

    /// Destructor
    ~CNonAsciiReplacementPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CNonAsciiReplacementPanel event handler declarations

////@end CNonAsciiReplacementPanel event handler declarations

////@begin CNonAsciiReplacementPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CNonAsciiReplacementPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    pair<TUnicodeSymbol, string> GetReplacement();

private:
////@begin CNonAsciiReplacementPanel member variables
    wxStaticText* m_Char;
    wxTextCtrl* m_Replacement;
////@end CNonAsciiReplacementPanel member variables
    wxUniChar m_Orig;
    wxString m_ReplacementStr;
    set<wxString> m_Contexts;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___NON_ASCII_REPLACEMENT_DLG__HPP
