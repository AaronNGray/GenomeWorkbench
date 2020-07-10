#ifndef GUI_WIDGETS_EDIT___MACRO_SIMPLE__HPP
#define GUI_WIDGETS_EDIT___MACRO_SIMPLE__HPP
/*  $Id: macro_simple.hpp 40876 2018-04-25 18:19:21Z filippov $
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
#include <gui/objutils/macro_engine.hpp>
#include <wx/dialog.h>

class wxButton;
class wxStaticText;
class wxTextCtrl;

BEGIN_NCBI_SCOPE


/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_MACRO_SIMPLE_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxRESIZE_BORDER
#define SYMBOL_MACRO_SIMPLE_TITLE _("Simple Macro Editor")
#define SYMBOL_MACRO_SIMPLE_IDNAME ID_MACRO_SIMPLE
#define SYMBOL_MACRO_SIMPLE_SIZE wxSize(600, 400)
#define SYMBOL_MACRO_SIMPLE_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CMacroSimple class declaration
 */

class CMacroSimple : public wxDialog
{    
    DECLARE_CLASS( CMacroSimple )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMacroSimple();
    CMacroSimple( wxWindow* parent, const string& text,
        wxWindowID id = SYMBOL_MACRO_SIMPLE_IDNAME, 
        const wxString& caption = SYMBOL_MACRO_SIMPLE_TITLE, 
        const wxPoint& pos = SYMBOL_MACRO_SIMPLE_POSITION, 
        const wxSize& size = SYMBOL_MACRO_SIMPLE_SIZE, 
        long style = SYMBOL_MACRO_SIMPLE_STYLE );

    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_MACRO_SIMPLE_IDNAME, 
        const wxString& caption = SYMBOL_MACRO_SIMPLE_TITLE, 
        const wxPoint& pos = SYMBOL_MACRO_SIMPLE_POSITION, 
        const wxSize& size = SYMBOL_MACRO_SIMPLE_SIZE, 
        long style = SYMBOL_MACRO_SIMPLE_STYLE );

    /// Destructor
    ~CMacroSimple();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CMacroSimple event handler declarations
    void OnUpdateText(wxCommandEvent& event);
    void OnKey(wxKeyEvent & event);
////@end CMacroSimple event handler declarations

////@begin CMacroSimple member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();
////@end CMacroSimple member function declarations

    /// Control identifiers
    enum {
        ID_MACRO_SIMPLE = 10040,
        ID_PANEL = 10041,
        ID_MACROACTIONPANEL = 10042,
        ID_MACROVALIDTEXTCTRL = 10043
    };

    CRef<macro::CMacroRep> GetMacro();
private:
    string m_Text;
    wxTextCtrl* m_ActionPanel;
    wxButton* m_AddToScript;
    wxStaticText* m_Status;

    macro::CMacroEngine m_MEngine;
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_EDITOR_FRAME__HPP
