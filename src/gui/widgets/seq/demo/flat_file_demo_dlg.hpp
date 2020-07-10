#ifndef GUI_SEQ_DEMO___FLATFILE_DEMO_DLG_HPP
#define GUI_SEQ_DEMO___FLATFILE_DEMO_DLG_HPP

/*  $Id: flat_file_demo_dlg.hpp 28342 2013-06-21 16:26:06Z katargir $
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

/*!
 * Includes
 */

#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>

#include <wx/dialog.h>
#include <wx/textctrl.h>

////@begin includes
#include "wx/statline.h"
////@end includes

/*!
 * Forward declarations
 */


class wxStaticText;
////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CFLATFILEDEMODLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxMAXIMIZE_BOX|wxMINIMIZE_BOX
#define SYMBOL_CFLATFILEDEMODLG_TITLE _("CFlatFileDemoDlg")
#define SYMBOL_CFLATFILEDEMODLG_IDNAME ID_HIT_MATRIX_DEMO_DLG
#define SYMBOL_CFLATFILEDEMODLG_SIZE wxSize(400, 368)
#define SYMBOL_CFLATFILEDEMODLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CFlatFileDemoDlg class declaration
 */

class CFlatFileDemoDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CFlatFileDemoDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFlatFileDemoDlg();
    CFlatFileDemoDlg( wxWindow* parent, wxWindowID id = SYMBOL_CFLATFILEDEMODLG_IDNAME, const wxString& caption = SYMBOL_CFLATFILEDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CFLATFILEDEMODLG_POSITION, const wxSize& size = SYMBOL_CFLATFILEDEMODLG_SIZE, long style = SYMBOL_CFLATFILEDEMODLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFLATFILEDEMODLG_IDNAME, const wxString& caption = SYMBOL_CFLATFILEDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CFLATFILEDEMODLG_POSITION, const wxSize& size = SYMBOL_CFLATFILEDEMODLG_SIZE, long style = SYMBOL_CFLATFILEDEMODLG_STYLE );

    /// Destructor
    ~CFlatFileDemoDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CFlatFileDemoDlg event handler declarations

    /// wxEVT_CLOSE_WINDOW event handler for ID_HIT_MATRIX_DEMO_DLG
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_FILE
    void OnLoadFileClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ACC
    void OnAccClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
    void OnSaveText( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
    void OnCancelClick( wxCommandEvent& event );

////@end CFlatFileDemoDlg event handler declarations

////@begin CFlatFileDemoDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFlatFileDemoDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CFlatFileDemoDlg member variables
    wxStaticText* m_LoadedStatus;
    wxTextCtrl* m_AccInput;
    /// Control identifiers
    enum {
        ID_HIT_MATRIX_DEMO_DLG = 10001,
        ID_WINDOW = 10000,
        ID_FILE = 10007,
        ID_ACC_TEXT = 10004,
        ID_ACC = 10005,
        ID_BUTTON1 = 10002
    };
////@end CFlatFileDemoDlg member variables

private:
    CRef<CObjectManager> m_ObjMgr;
    CRef<CScope> m_Scope;
};

END_NCBI_SCOPE

#endif  // GUI_SEQ_DEMO___FLATFILE_DEMO_DLG_HPP
