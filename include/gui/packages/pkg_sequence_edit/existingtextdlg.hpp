/*  $Id: existingtextdlg.hpp 34511 2016-01-19 16:21:35Z asztalos $
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
 * Authors:  Colleen Bollin
 */
#ifndef _EXISTINGTEXTDLG_H_
#define _EXISTINGTEXTDLG_H_

#include <corelib/ncbistd.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/dialog.h>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CEXISTINGTEXTDLG 10131
#define ID_APPEND_BTN 10132
#define ID_PREFIX_BTN 10133
#define ID_IGNORE_BTN 10134
#define ID_ADD_QUAL_BTN 10260
#define ID_SEMICOLON_BTN 10135
#define ID_SPACE_BTN 10136
#define ID_COLON_BTN 10137
#define ID_COMMA_BTN 10138
#define ID_NODELIMITER_BTN 10139
#define SYMBOL_CEXISTINGTEXTDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CEXISTINGTEXTDLG_TITLE _("How to Add New Text")
#define SYMBOL_CEXISTINGTEXTDLG_IDNAME ID_CEXISTINGTEXTDLG
#define SYMBOL_CEXISTINGTEXTDLG_SIZE wxSize(400, 300)
#define SYMBOL_CEXISTINGTEXTDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CExistingTextDlg class declaration
 */

class CExistingTextDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CExistingTextDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CExistingTextDlg();
    CExistingTextDlg( wxWindow* parent, wxWindowID id = SYMBOL_CEXISTINGTEXTDLG_IDNAME, const wxString& caption = SYMBOL_CEXISTINGTEXTDLG_TITLE, const wxPoint& pos = SYMBOL_CEXISTINGTEXTDLG_POSITION, const wxSize& size = SYMBOL_CEXISTINGTEXTDLG_SIZE, long style = SYMBOL_CEXISTINGTEXTDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CEXISTINGTEXTDLG_IDNAME, const wxString& caption = SYMBOL_CEXISTINGTEXTDLG_TITLE, const wxPoint& pos = SYMBOL_CEXISTINGTEXTDLG_POSITION, const wxSize& size = SYMBOL_CEXISTINGTEXTDLG_SIZE, long style = SYMBOL_CEXISTINGTEXTDLG_STYLE );

    /// Destructor
    ~CExistingTextDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CExistingTextDlg event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_APPEND_BTN
    void OnAppendBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_PREFIX_BTN
    void OnPrefixBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_IGNORE_BTN
    void OnIgnoreBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ADD_QUAL_BTN
    void OnAddQualBtnSelected( wxCommandEvent& event );

////@end CExistingTextDlg event handler declarations

////@begin CExistingTextDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CExistingTextDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CExistingTextDlg member variables
    wxBoxSizer* m_Position;
    wxRadioButton* m_Append;
    wxRadioButton* m_Prefix;
    wxRadioButton* m_LeaveOld;
    wxRadioButton* m_AddQual;
    wxBoxSizer* m_Delimiters;
    wxRadioButton* m_Semicolon;
    wxRadioButton* m_Space;
    wxRadioButton* m_Colon;
    wxRadioButton* m_Comma;
    wxRadioButton* m_NoDelimiter;
////@end CExistingTextDlg member variables

    objects::edit::EExistingText GetExistingTextHandler();
    void AllowAdditionalQual(bool val);

private:
    void x_EnableDelimiters(bool val);
};

END_NCBI_SCOPE

#endif
    // _EXISTINGTEXTDLG_H_
