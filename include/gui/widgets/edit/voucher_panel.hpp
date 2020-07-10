/*  $Id: voucher_panel.hpp 29060 2013-10-01 19:21:17Z bollin $
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
#ifndef _VOUCHER_PANEL_H_
#define _VOUCHER_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/srcmod_edit_panel.hpp>
#include <wx/textctrl.h>

/*!
 * Includes
 */

////@begin includes
////@end includes

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
#define ID_CVOUCHERPANEL 10005
#define ID_VOUCHER_INST 10009
#define ID_VOUCHER_COLL 10016
#define ID_VOUCHER_TEXT 10017
#define SYMBOL_CVOUCHERPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CVOUCHERPANEL_TITLE _("VoucherPanel")
#define SYMBOL_CVOUCHERPANEL_IDNAME ID_CVOUCHERPANEL
#define SYMBOL_CVOUCHERPANEL_SIZE wxDefaultSize
#define SYMBOL_CVOUCHERPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CVoucherPanel class declaration
 */

class CVoucherPanel: public CSrcModEditPanel 
{    
    DECLARE_DYNAMIC_CLASS( CVoucherPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CVoucherPanel();
    CVoucherPanel( wxWindow* parent, wxWindowID id = SYMBOL_CVOUCHERPANEL_IDNAME, const wxPoint& pos = SYMBOL_CVOUCHERPANEL_POSITION, const wxSize& size = SYMBOL_CVOUCHERPANEL_SIZE, long style = SYMBOL_CVOUCHERPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CVOUCHERPANEL_IDNAME, const wxPoint& pos = SYMBOL_CVOUCHERPANEL_POSITION, const wxSize& size = SYMBOL_CVOUCHERPANEL_SIZE, long style = SYMBOL_CVOUCHERPANEL_STYLE );

    /// Destructor
    ~CVoucherPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CVoucherPanel event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_VOUCHER_INST
    void OnVoucherInstTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_VOUCHER_COLL
    void OnVoucherCollTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_VOUCHER_TEXT
    void OnVoucherTextTextUpdated( wxCommandEvent& event );

////@end CVoucherPanel event handler declarations

////@begin CVoucherPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CVoucherPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CVoucherPanel member variables
    wxTextCtrl* m_Inst;
    wxTextCtrl* m_Coll;
    wxTextCtrl* m_ID;
////@end CVoucherPanel member variables

    virtual string GetValue();
    virtual void SetValue(const string& val);
    virtual bool IsWellFormatted(const string& val);
};

END_NCBI_SCOPE

#endif
    // _VOUCHER_PANEL_H_
