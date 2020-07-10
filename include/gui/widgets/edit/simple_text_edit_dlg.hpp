#ifndef GUI_WIDGETS_EDIT___SIMPLE_TEXT_EDIT__HPP
#define GUI_WIDGETS_EDIT___SIMPLE_TEXT_EDIT__HPP
/*  $Id: simple_text_edit_dlg.hpp 40897 2018-04-26 20:46:56Z filippov $
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
#include <wx/dialog.h>

class wxButton;
class wxStaticText;
class wxTextCtrl;

BEGIN_NCBI_SCOPE


/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_SIMPLE_TEXT_EDIT_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxRESIZE_BORDER
#define SYMBOL_SIMPLE_TEXT_EDIT_TITLE _("Multi-line Editor")
#define SYMBOL_SIMPLE_TEXT_EDIT_IDNAME wxID_ANY
#define SYMBOL_SIMPLE_TEXT_EDIT_SIZE wxSize(600, 400)
#define SYMBOL_SIMPLE_TEXT_EDIT_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSimpleTextEditor class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CSimpleTextEditor : public wxDialog
{    
    DECLARE_CLASS( CSimpleTextEditor )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSimpleTextEditor();
    CSimpleTextEditor( wxWindow* parent, const wxString& text,
        wxWindowID id = SYMBOL_SIMPLE_TEXT_EDIT_IDNAME, 
        const wxString& caption = SYMBOL_SIMPLE_TEXT_EDIT_TITLE, 
        const wxPoint& pos = SYMBOL_SIMPLE_TEXT_EDIT_POSITION, 
        const wxSize& size = SYMBOL_SIMPLE_TEXT_EDIT_SIZE, 
        long style = SYMBOL_SIMPLE_TEXT_EDIT_STYLE );

    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_SIMPLE_TEXT_EDIT_IDNAME, 
        const wxString& caption = SYMBOL_SIMPLE_TEXT_EDIT_TITLE, 
        const wxPoint& pos = SYMBOL_SIMPLE_TEXT_EDIT_POSITION, 
        const wxSize& size = SYMBOL_SIMPLE_TEXT_EDIT_SIZE, 
        long style = SYMBOL_SIMPLE_TEXT_EDIT_STYLE );

    /// Destructor
    ~CSimpleTextEditor();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSimpleTextEditor event handler declarations
    void OnKey(wxKeyEvent & event);
////@end CSimpleTextEditor event handler declarations

////@begin CSimpleTextEditor member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();
////@end CSimpleTextEditor member function declarations

    /// Control identifiers
    enum {
        ID_SIMPLE_TEXT_EDIT_PANEL = 10042,
    };

    wxString GetValue();
private:
    wxString m_Text;
    wxTextCtrl* m_Panel;
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___SIMPLE_TEXT_EDIT__HPP
