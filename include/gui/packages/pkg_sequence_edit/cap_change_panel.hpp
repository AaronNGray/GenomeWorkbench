#ifndef _CAP_CHANGE_PANEL_H_
#define _CAP_CHANGE_PANEL_H_
/*  $Id: cap_change_panel.hpp 41872 2018-10-31 15:16:50Z asztalos $
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
#include <objtools/cleanup/capitalization_string.hpp>

#include <wx/panel.h>
class wxRadioButton;

BEGIN_NCBI_SCOPE
/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CCAPCHANGEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CCAPCHANGEPANEL_TITLE _("CapitalizationChangePanel")
#define SYMBOL_CCAPCHANGEPANEL_IDNAME ID_CCAPCHANGEPANEL
#define SYMBOL_CCAPCHANGEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CCAPCHANGEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CCapChangePanel class declaration
 */

class CCapChangePanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CCapChangePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCapChangePanel();
    CCapChangePanel( wxWindow* parent, wxWindowID id = SYMBOL_CCAPCHANGEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CCAPCHANGEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CCAPCHANGEPANEL_SIZE, 
        long style = SYMBOL_CCAPCHANGEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCAPCHANGEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CCAPCHANGEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CCAPCHANGEPANEL_SIZE, 
        long style = SYMBOL_CCAPCHANGEPANEL_STYLE );

    /// Destructor
    ~CCapChangePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CCapChangePanel event handler declarations

////@end CCapChangePanel event handler declarations

////@begin CCapChangePanel member function declarations

    void ClearValues(void);

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCapChangePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    objects::ECapChange GetCapitalizationRequest();
////@begin CCapChangePanel member variables
    wxRadioButton* m_CapNochange;
    wxRadioButton* m_CapToupper;
    wxRadioButton* m_CapFirstcap_nochange;
    wxRadioButton* m_Capwords_space;
    wxRadioButton* m_CapTolower;
    wxRadioButton* m_CapFirstcap_restlow;
    wxRadioButton* m_CapFirstlow_nochange;
    wxRadioButton* m_Capwords_spacepunct;
    /// Control identifiers
    enum {
        ID_CCAPCHANGEPANEL = 6230,
        ID_CAPCHANGE_NOCHANGE = 6231,
        ID_CAPCHANGE_UPPER = 6232,
        ID_CAPCHANGE_FCAP_NOCH = 6233,
        ID_CAPCHANGE_CAPWORDS_SP = 6234,
        ID_CAPCHANGE_LOWER = 6235,
        ID_CAPCHANGE_FCAP_LOW = 6236,
        ID_CAPCHANGE_FLOW_NOCH = 6237,
        ID_CAPCHANGE_CAPWORDS_SPPUNCT = 6238
    };
////@end CCapChangePanel member variables

};

END_NCBI_SCOPE

#endif
    // _CAP_CHANGE_PANEL_H_

