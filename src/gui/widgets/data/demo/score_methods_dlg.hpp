/*  $Id: score_methods_dlg.hpp 25668 2012-04-19 15:54:28Z katargir $
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

#ifndef _SCORE_METHODS_DLG_H_
#define _SCORE_METHODS_DLG_H_

/*!
 * Includes
 */

#include <objtools/alnmgr/aln_explorer.hpp>

#include "wx/dialog.h"

////@begin includes
#include "wx/statline.h"
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
#define SYMBOL_CSCOREMETHODSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CSCOREMETHODSDLG_TITLE _("Score Methods")
#define SYMBOL_CSCOREMETHODSDLG_IDNAME ID_CALIGNSCOREPROPDLG
#define SYMBOL_CSCOREMETHODSDLG_SIZE wxDefaultSize
#define SYMBOL_CSCOREMETHODSDLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CScoreMethodsDlg class declaration
 */

class IScoringMethod;

class CScoreMethodsDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CScoreMethodsDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CScoreMethodsDlg();
    CScoreMethodsDlg( wxWindow* parent, wxWindowID id = SYMBOL_CSCOREMETHODSDLG_IDNAME, const wxString& caption = SYMBOL_CSCOREMETHODSDLG_TITLE, const wxPoint& pos = SYMBOL_CSCOREMETHODSDLG_POSITION, const wxSize& size = SYMBOL_CSCOREMETHODSDLG_SIZE, long style = SYMBOL_CSCOREMETHODSDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSCOREMETHODSDLG_IDNAME, const wxString& caption = SYMBOL_CSCOREMETHODSDLG_TITLE, const wxPoint& pos = SYMBOL_CSCOREMETHODSDLG_POSITION, const wxSize& size = SYMBOL_CSCOREMETHODSDLG_SIZE, long style = SYMBOL_CSCOREMETHODSDLG_STYLE );

    /// Destructor
    ~CScoreMethodsDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CScoreMethodsDlg event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE1
    void OnChoice1Selected( wxCommandEvent& event );

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX1
    void OnListbox1Selected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
    void OnButton2Click( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
    void OnButton1Click( wxCommandEvent& event );

////@end CScoreMethodsDlg event handler declarations

////@begin CScoreMethodsDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CScoreMethodsDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CScoreMethodsDlg member variables
    /// Control identifiers
    enum {
        ID_CALIGNSCOREPROPDLG = 10000,
        ID_CHOICE1 = 10004,
        ID_LISTBOX1 = 10001,
        ID_TEXTCTRL1 = 10005,
        ID_STATICLINE = 10003,
        ID_BUTTON2 = 10006,
        ID_BUTTON1 = 10002
    };
////@end CScoreMethodsDlg member variables
protected:
    void x_LoadTools();
    void x_UpdateDescription();
    IAlnExplorer::EAlignType x_GetSelectedType();

    vector<CIRef<IScoringMethod> > m_Tools;
};

END_NCBI_SCOPE

#endif
    // _SCORE_METHODS_DLG_H_