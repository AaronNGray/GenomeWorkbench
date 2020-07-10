#ifndef GUI_WIDGETS_ALN_SCORE___SCORING_METHODS_DLG__HPP
#define GUI_WIDGETS_ALN_SCORE___SCORING_METHODS_DLG__HPP

/*  $Id: scoring_methods_dlg.hpp 25739 2012-04-27 18:55:44Z voronov $
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
 * Authors:  Andrey Yazhuk, Roman Katargin
 *
 * File Description:
 */

#include <gui/gui_export.h>

#include <gui/widgets/wx/dialog.hpp>

#include <objtools/alnmgr/aln_explorer.hpp>

////@begin includes
#include "wx/statline.h"
////@end includes

////@begin forward declarations
////@end forward declarations

////@begin control identifiers
#define SYMBOL_CSCORINGMETHODSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CSCORINGMETHODSDLG_TITLE _("Alignment Scoring Methods")
#define SYMBOL_CSCORINGMETHODSDLG_IDNAME ID_CSCORINGMETHODSDLG
#define SYMBOL_CSCORINGMETHODSDLG_SIZE wxSize(400, 300)
#define SYMBOL_CSCORINGMETHODSDLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// CScoringMethodsDlg

class NCBI_GUIWIDGETS_ALNSCORE_EXPORT CScoringMethodsDlg: public CDialog
{
    DECLARE_DYNAMIC_CLASS( CScoringMethodsDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CScoringMethodsDlg();
    CScoringMethodsDlg( wxWindow* parent, wxWindowID id = SYMBOL_CSCORINGMETHODSDLG_IDNAME, const wxString& caption = SYMBOL_CSCORINGMETHODSDLG_TITLE, const wxPoint& pos = SYMBOL_CSCORINGMETHODSDLG_POSITION, const wxSize& size = SYMBOL_CSCORINGMETHODSDLG_SIZE, long style = SYMBOL_CSCORINGMETHODSDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSCORINGMETHODSDLG_IDNAME, const wxString& caption = SYMBOL_CSCORINGMETHODSDLG_TITLE, const wxPoint& pos = SYMBOL_CSCORINGMETHODSDLG_POSITION, const wxSize& size = SYMBOL_CSCORINGMETHODSDLG_SIZE, long style = SYMBOL_CSCORINGMETHODSDLG_STYLE );

    /// Destructor
    ~CScoringMethodsDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CScoringMethodsDlg event handler declarations

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX1
    void OnMethodSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for ID_LISTBOX1
    void OnListbox1DoubleClicked( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
    void OnPropertiesClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOkClick( wxCommandEvent& event );

////@end CScoringMethodsDlg event handler declarations

////@begin CScoringMethodsDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CScoringMethodsDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CScoringMethodsDlg member variables
    /// Control identifiers
    enum {
        ID_CSCORINGMETHODSDLG = 10000,
        ID_LISTBOX1 = 10001,
        ID_TEXTCTRL1 = 10002,
        ID_BUTTON1 = 10004,
        ID_STATICLINE = 10003
    };
////@end CScoringMethodsDlg member variables
public:
    void    Setup( const string& sel_method, IAlnExplorer::EAlignType type);
    wxString GetSelectedMethod() const  { return m_SelMethod; }

protected:
    void x_UpdateDescription();

protected:
    IAlnExplorer::EAlignType m_Type;
    wxString m_SelMethod;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_SCORE___SCORING_METHODS_DLG__HPP
