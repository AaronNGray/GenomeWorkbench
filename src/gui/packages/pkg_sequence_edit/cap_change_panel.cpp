/*  $Id: cap_change_panel.cpp 41872 2018-10-31 15:16:50Z asztalos $
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


#include <ncbi_pch.hpp>
#include <objmgr/scope.hpp>
#include <gui/packages/pkg_sequence_edit/cap_change_panel.hpp>

#include <wx/statbox.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/icon.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
/*!
 * CCapChangePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CCapChangePanel, wxPanel )


/*!
 * CCapChangePanel event table definition
 */

BEGIN_EVENT_TABLE( CCapChangePanel, wxPanel )
END_EVENT_TABLE()


/*!
 * CCapChangePanel constructors
 */

CCapChangePanel::CCapChangePanel()
{
    Init();
}

CCapChangePanel::CCapChangePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CCapChangePanel creator
 */

bool CCapChangePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CCapChangePanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CCapChangePanel creation
    return true;
}


/*!
 * CCapChangePanel destructor
 */

CCapChangePanel::~CCapChangePanel()
{
////@begin CCapChangePanel destruction
////@end CCapChangePanel destruction
}


/*!
 * Member initialisation
 */

void CCapChangePanel::Init()
{
////@begin CCapChangePanel member initialisation
    m_CapNochange = NULL;
    m_CapToupper = NULL;
    m_CapFirstcap_nochange = NULL;
    m_Capwords_space = NULL;
    m_CapTolower = NULL;
    m_CapFirstcap_restlow = NULL;
    m_CapFirstlow_nochange = NULL;
    m_Capwords_spacepunct = NULL;
////@end CCapChangePanel member initialisation
}


/*!
 * Control creation for CCapChangePanel
 */

void CCapChangePanel::CreateControls()
{    
////@begin CCapChangePanel content construction
    CCapChangePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Capitalization"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(0, 4, 0, 0);
    itemStaticBoxSizer3->Add(itemFlexGridSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_CapNochange = new wxRadioButton( itemStaticBoxSizer3->GetStaticBox(), ID_CAPCHANGE_NOCHANGE, _("No change"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_CapNochange->SetValue(true);
    itemFlexGridSizer4->Add(m_CapNochange, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CapToupper = new wxRadioButton( itemStaticBoxSizer3->GetStaticBox(), ID_CAPCHANGE_UPPER, _("To upper"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CapToupper->SetValue(false);
    itemFlexGridSizer4->Add(m_CapToupper, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CapFirstcap_nochange = new wxRadioButton( itemStaticBoxSizer3->GetStaticBox(), ID_CAPCHANGE_FCAP_NOCH, _("First cap, rest no change"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CapFirstcap_nochange->SetValue(false);
    itemFlexGridSizer4->Add(m_CapFirstcap_nochange, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Capwords_space = new wxRadioButton( itemStaticBoxSizer3->GetStaticBox(), ID_CAPCHANGE_CAPWORDS_SP, _("Cap words, start at spaces"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Capwords_space->SetValue(false);
    itemFlexGridSizer4->Add(m_Capwords_space, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CapTolower = new wxRadioButton( itemStaticBoxSizer3->GetStaticBox(), ID_CAPCHANGE_LOWER, _("To lower"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CapTolower->SetValue(false);
    itemFlexGridSizer4->Add(m_CapTolower, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CapFirstcap_restlow = new wxRadioButton( itemStaticBoxSizer3->GetStaticBox(), ID_CAPCHANGE_FCAP_LOW, _("First cap, rest lower"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CapFirstcap_restlow->SetValue(false);
    itemFlexGridSizer4->Add(m_CapFirstcap_restlow, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CapFirstlow_nochange = new wxRadioButton( itemStaticBoxSizer3->GetStaticBox(), ID_CAPCHANGE_FLOW_NOCH, _("First lower, rest no change"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CapFirstlow_nochange->SetValue(false);
    itemFlexGridSizer4->Add(m_CapFirstlow_nochange, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Capwords_spacepunct = new wxRadioButton( itemStaticBoxSizer3->GetStaticBox(), ID_CAPCHANGE_CAPWORDS_SPPUNCT, _("Cap words, start at spaces or punct."), wxDefaultPosition, wxDefaultSize, 0 );
    m_Capwords_spacepunct->SetValue(false);
    itemFlexGridSizer4->Add(m_Capwords_spacepunct, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CCapChangePanel content construction
}

void CCapChangePanel::ClearValues()
{
    m_CapNochange->SetValue(true);
    m_CapToupper->SetValue(false);
    m_CapFirstcap_nochange->SetValue(false);
    m_Capwords_space->SetValue(false);
    m_CapTolower->SetValue(false);
    m_CapFirstcap_restlow->SetValue(false);
    m_CapFirstlow_nochange->SetValue(false);
    m_Capwords_spacepunct->SetValue(false);
}

/*!
 * Should we show tooltips?
 */

bool CCapChangePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCapChangePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCapChangePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCapChangePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCapChangePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCapChangePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCapChangePanel icon retrieval
}


ECapChange CCapChangePanel::GetCapitalizationRequest()
{
    ECapChange cap_change = eCapChange_none;

    // read out which radiobutton is set 
    if (m_CapNochange->GetValue()) {
        cap_change = eCapChange_none;
    } else if (m_CapToupper->GetValue()) {
        cap_change = eCapChange_toupper;
    } else if (m_CapTolower->GetValue()) {
        cap_change = eCapChange_tolower;
    } else if (m_CapFirstcap_restlow->GetValue()) {
        cap_change = eCapChange_firstcap_restlower;
    } else if (m_CapFirstcap_nochange->GetValue()) {
        cap_change = eCapChange_firstcap_restnochange;
    } else if (m_CapFirstlow_nochange->GetValue()) {
        cap_change = eCapChange_firstlower_restnochange;
    } else if (m_Capwords_space->GetValue()) {
        cap_change = eCapChange_capword_afterspace;
    } else if (m_Capwords_spacepunct->GetValue()) {
        cap_change = eCapChange_capword_afterspacepunc;
    }
    return cap_change;
}


END_NCBI_SCOPE

