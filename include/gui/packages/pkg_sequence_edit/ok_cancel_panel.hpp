/*  $Id: ok_cancel_panel.hpp 38626 2017-06-05 13:53:11Z asztalos $
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
#ifndef _OKCANCELPANEL_H_
#define _OKCANCELPANEL_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/checkbox.h>
#include <wx/panel.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_COKCANCELPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_COKCANCELPANEL_TITLE _("OkCancelPanel")
#define SYMBOL_COKCANCELPANEL_IDNAME ID_COKCANCELPANEL
#define SYMBOL_COKCANCELPANEL_SIZE wxSize(400, 300)
#define SYMBOL_COKCANCELPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * COkCancelPanel class declaration
 */

class COkCancelPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( COkCancelPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    COkCancelPanel();
    COkCancelPanel( wxWindow* parent, wxWindowID id = SYMBOL_COKCANCELPANEL_IDNAME, const wxPoint& pos = SYMBOL_COKCANCELPANEL_POSITION, const wxSize& size = SYMBOL_COKCANCELPANEL_SIZE, long style = SYMBOL_COKCANCELPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_COKCANCELPANEL_IDNAME, const wxPoint& pos = SYMBOL_COKCANCELPANEL_POSITION, const wxSize& size = SYMBOL_COKCANCELPANEL_SIZE, long style = SYMBOL_COKCANCELPANEL_STYLE );

    /// Destructor
    ~COkCancelPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin COkCancelPanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ACCEPT_BTN
    void OnAcceptBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
    void OnClickCancel( wxCommandEvent& event );

////@end COkCancelPanel event handler declarations

////@begin COkCancelPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end COkCancelPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    bool GetLeaveUp(void) {return m_LeaveUp->GetValue();}

    /// Control identifiers
    enum {
        ID_COKCANCELPANEL = wxID_HIGHEST + 1,
        ID_ACCEPT_BTN = 6001,
        ID_LEAVE_DIALOG_UP_CHKBOX = 6002
    };

private:
    void x_ReportUsage(void);

////@begin COkCancelPanel member variables
    wxCheckBox* m_LeaveUp;
    
////@end COkCancelPanel member variables
};

END_NCBI_SCOPE

#endif
    // _OKCANCELPANEL_H_
