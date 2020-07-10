#ifndef GUI_APP_GBENCH_NEW___NETWORK_OPTIONS_PAGE__HPP
#define GUI_APP_GBENCH_NEW___NETWORK_OPTIONS_PAGE__HPP

/*  $Id: network_options_page.hpp 32604 2015-03-31 18:45:53Z katargir $
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

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

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
#define SYMBOL_CNETWORKOPTIONSPAGE_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CNETWORKOPTIONSPAGE_TITLE _("Network")
#define SYMBOL_CNETWORKOPTIONSPAGE_IDNAME ID_CNETWORKOPTIONSPAGE
#define SYMBOL_CNETWORKOPTIONSPAGE_SIZE wxSize(400, 300)
#define SYMBOL_CNETWORKOPTIONSPAGE_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CNetworkOptionsPage class declaration
 */

class CNetworkOptionsPage: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CNetworkOptionsPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CNetworkOptionsPage();
    CNetworkOptionsPage( wxWindow* parent, wxWindowID id = SYMBOL_CNETWORKOPTIONSPAGE_IDNAME, const wxPoint& pos = SYMBOL_CNETWORKOPTIONSPAGE_POSITION, const wxSize& size = SYMBOL_CNETWORKOPTIONSPAGE_SIZE, long style = SYMBOL_CNETWORKOPTIONSPAGE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CNETWORKOPTIONSPAGE_IDNAME, const wxPoint& pos = SYMBOL_CNETWORKOPTIONSPAGE_POSITION, const wxSize& size = SYMBOL_CNETWORKOPTIONSPAGE_SIZE, long style = SYMBOL_CNETWORKOPTIONSPAGE_STYLE );

    /// Destructor
    ~CNetworkOptionsPage();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CNetworkOptionsPage event handler declarations

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX1
    void OnUseProxyClick( wxCommandEvent& event );

////@end CNetworkOptionsPage event handler declarations

////@begin CNetworkOptionsPage member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CNetworkOptionsPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CNetworkOptionsPage member variables
    /// Control identifiers
    enum {
        ID_CNETWORKOPTIONSPAGE = 10015,
        ID_RADIOBOX1 = 10016,
        ID_RADIOBOX3 = 10020,
        ID_CHECKBOX1 = 10031,
        ID_TEXTCTRL6 = 10017,
        ID_TEXTCTRL7 = 10018,
        ID_TEXTCTRL8 = 10025,
        ID_TEXTCTRL9 = 10026
    };
////@end CNetworkOptionsPage member variables

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

private:
    void UpdateProxyControls(bool enable);
};

END_NCBI_SCOPE

#endif  // GUI_APP_GBENCH_NEW___NETWORK_OPTIONS_PAGE__HPP
