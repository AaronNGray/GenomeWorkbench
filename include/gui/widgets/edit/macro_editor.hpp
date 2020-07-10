#ifndef GUI_WIDGETS_EDIT___MACRO_EDITOR_FRAME__HPP
#define GUI_WIDGETS_EDIT___MACRO_EDITOR_FRAME__HPP
/*  $Id: macro_editor.hpp 42239 2019-01-17 16:46:51Z filippov $
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
 * Authors:  Andrea Asztalos
 */

#include <corelib/ncbistd.hpp>
#include <gui/objutils/macro_engine.hpp>
#include <wx/frame.h>

class wxButton;
class wxStaticText;

BEGIN_NCBI_SCOPE

class CMacroEditingActionPanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CMACROEDITOR_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CMACROEDITOR_TITLE _("Add New Macro")
#define SYMBOL_CMACROEDITOR_IDNAME ID_CMACROEDITOR
#define SYMBOL_CMACROEDITOR_SIZE wxDefaultSize
#define SYMBOL_CMACROEDITOR_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CMacroEditor class declaration
 */

class CMacroEditor : public wxFrame
{    
    DECLARE_CLASS( CMacroEditor )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMacroEditor();
    CMacroEditor( wxWindow* parent, 
                  int w, int h, int x, int y,
                  wxWindowID id = SYMBOL_CMACROEDITOR_IDNAME, 
                  const wxString& caption = SYMBOL_CMACROEDITOR_TITLE, 
                  const wxPoint& pos = SYMBOL_CMACROEDITOR_POSITION, 
                  const wxSize& size = SYMBOL_CMACROEDITOR_SIZE, 
                  long style = SYMBOL_CMACROEDITOR_STYLE );

    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CMACROEDITOR_IDNAME, 
        const wxString& caption = SYMBOL_CMACROEDITOR_TITLE, 
        const wxPoint& pos = SYMBOL_CMACROEDITOR_POSITION, 
        const wxSize& size = SYMBOL_CMACROEDITOR_SIZE, 
        long style = SYMBOL_CMACROEDITOR_STYLE );

    /// Destructor
    ~CMacroEditor();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CMacroEditor event handler declarations

    void OnOkClick(wxCommandEvent& event);

    void OnCloseClick(wxCommandEvent& event);

////@end CMacroEditor event handler declarations

////@begin CMacroEditor member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();
////@end CMacroEditor member function declarations

    void Update();
    void Clear();
    
    /// Control identifiers
    enum {
        ID_CMACROEDITOR = 10040,
        ID_PANEL = 10041,
        ID_MACROACTIONPANEL = 10042,
        ID_MACROVALIDTEXTCTRL = 10043
    };

private:
    void DisplayNotice();
    void SetPositionAndSize();

    CMacroEditingActionPanel* m_ActionPanel;
    wxButton* m_AddToScript;
    wxStaticText* m_Status;

    CRef<macro::CMacroRep> m_MacroRep;
    macro::CMacroEngine m_MEngine;
    int m_width;
    int m_height;
    int m_pos_x;
    int m_pos_y;
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_EDITOR_FRAME__HPP
