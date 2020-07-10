/*  $Id: submitter_affil_panel.cpp 43206 2019-05-28 21:41:23Z asztalos $
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
#include <objects/submit/Contact_info.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/biblio/Affil.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objtools/cleanup/capitalization_string.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/submitter_affil_panel.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CSubmitterAffilPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSubmitterAffilPanel, wxPanel )


/*
 * CSubmitterAffilPanel event table definition
 */

BEGIN_EVENT_TABLE( CSubmitterAffilPanel, wxPanel )

////@begin CSubmitterAffilPanel event table entries
    EVT_COMBOBOX( ID_SUB_AFFIL_COUNTRY, CSubmitterAffilPanel::OnAffilCountrySelected )
////@end CSubmitterAffilPanel event table entries

END_EVENT_TABLE()


/*
 * CSubmitterAffilPanel constructors
 */

CSubmitterAffilPanel::CSubmitterAffilPanel()
{
    Init();
}

CSubmitterAffilPanel::CSubmitterAffilPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * SubmitterAffilPanel creator
 */

bool CSubmitterAffilPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSubmitterAffilPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSubmitterAffilPanel creation
    return true;
}


/*
 * CSubmitterAffilPanel destructor
 */

CSubmitterAffilPanel::~CSubmitterAffilPanel()
{
////@begin CSubmitterAffilPanel destruction
////@end CSubmitterAffilPanel destruction
}


/*
 * Member initialisation
 */

void CSubmitterAffilPanel::Init()
{
////@begin CSubmitterAffilPanel member initialisation
    m_AffilCtrl = NULL;
    m_DepartmentCtrl = NULL;
    m_StreetCtrl = NULL;
    m_CityCtrl = NULL;
    m_SubCtrl = NULL;
    m_ZipCtrl = NULL;
    m_CountryCtrl = NULL;
    m_PhoneCtrl = NULL;
////@end CSubmitterAffilPanel member initialisation
}


/*
 * Control creation for SubmitterAffilPanel
 */

void CSubmitterAffilPanel::CreateControls()
{    
////@begin CSubmitterAffilPanel content construction
    CSubmitterAffilPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer1, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    wxStaticText* itemStaticText2 = new wxStaticText( itemPanel1, wxID_STATIC, _("Institution*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText2, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AffilCtrl = new wxTextCtrl( itemPanel1, ID_SUB_AFFIL_INST, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer1->Add(m_AffilCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Department*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DepartmentCtrl = new wxTextCtrl( itemPanel1, ID_SUB_AFFIL_DEPT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(m_DepartmentCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Street*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_StreetCtrl = new wxTextCtrl( itemPanel1, ID_SUB_AFFIL_ADDRESS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(m_StreetCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("City*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CityCtrl = new wxTextCtrl( itemPanel1, ID_SUB_AFFIL_CITY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(m_CityCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("State/Province"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SubCtrl = new wxTextCtrl( itemPanel1, ID_SUB_AFFIL_STATE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(m_SubCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel1, wxID_STATIC, _("Postal code*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText12, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ZipCtrl = new wxTextCtrl( itemPanel1, ID_SUB_AFFIL_CODE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(m_ZipCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText14 = new wxStaticText( itemPanel1, wxID_STATIC, _("Country*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText14, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_CountryCtrlStrings;
    vector<CTempString> countries;
    for (unsigned int p = 0; !GetValidCountryCode(p).empty(); ++p) {
        countries.push_back(GetValidCountryCode(p));
    }
    for (auto&& it : countries) {
        m_CountryCtrlStrings.Add(ToWxString(it));
    }
    m_CountryCtrlStrings.Add(wxT("Other"));  // SP: Unknown or unspecified country
    m_CountryCtrl = new wxComboBox( itemPanel1, ID_SUB_AFFIL_COUNTRY, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_CountryCtrlStrings, wxCB_DROPDOWN );
    itemFlexGridSizer1->Add(m_CountryCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText16 = new wxStaticText( itemPanel1, wxID_STATIC, _("Phone"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText16, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_PhoneCtrl = new wxTextCtrl( itemPanel1, ID_SUB_AFFIL_PHONE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(m_PhoneCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CSubmitterAffilPanel content construction
}


/*
 * Should we show tooltips?
 */

bool CSubmitterAffilPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSubmitterAffilPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSubmitterAffilPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSubmitterAffilPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSubmitterAffilPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSubmitterAffilPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSubmitterAffilPanel icon retrieval
}

/*
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_SUB_AFFIL_COUNTRY
 */

void CSubmitterAffilPanel::OnAffilCountrySelected( wxCommandEvent& event )
{
    if (m_CountryCtrl->GetCurrentSelection() + 1 == (int)m_CountryCtrl->GetCount()) {
        m_CountryCtrl->ChangeValue(wxEmptyString);
    }
}


void CSubmitterAffilPanel::ApplySubmitBlock(objects::CSubmit_block& block)
{
    m_SubmitBlock.Reset(&block);
    TransferDataToWindow();
}


void CSubmitterAffilPanel::x_Reset()
{
    m_AffilCtrl->SetValue(wxT(""));
    m_DepartmentCtrl->SetValue(wxT(""));
    m_StreetCtrl->SetValue(wxT(""));
    m_CityCtrl->SetValue(wxT(""));
    m_SubCtrl->SetValue(wxT(""));
    m_ZipCtrl->SetValue(wxT(""));
    m_CountryCtrl->SetValue(wxT(""));
    m_PhoneCtrl->SetValue(wxT(""));
}


bool CSubmitterAffilPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow()) {
        return false;
    }

    x_Reset();
    if (!m_SubmitBlock)
        return false;

    const CAffil* affil = nullptr;
    if (m_SubmitBlock->IsSetCit() &&
        m_SubmitBlock->GetCit().IsSetAuthors() &&
        m_SubmitBlock->GetCit().GetAuthors().IsSetAffil()) {
        affil = &(m_SubmitBlock->GetCit().GetAuthors().GetAffil());
    }

    if (!affil && m_SubmitBlock->IsSetContact() && 
        m_SubmitBlock->GetContact().IsSetContact() && 
        m_SubmitBlock->GetContact().GetContact().IsSetAffil()) {
        affil = &(m_SubmitBlock->GetContact().GetContact().GetAffil());
    }
    
    if (affil) {
        if (affil->IsStr()) {
            m_AffilCtrl->SetValue(affil->GetStr());
        } else if (affil->IsStd()) {
            const auto& stad = affil->GetStd();
            if (stad.IsSetAffil()) {
                m_AffilCtrl->SetValue(ToWxString(stad.GetAffil()));
            }
            if (stad.IsSetDiv()) {
                m_DepartmentCtrl->SetValue(ToWxString(stad.GetDiv()));
            }
            if (stad.IsSetStreet()) {
                m_StreetCtrl->SetValue(ToWxString(stad.GetStreet()));
            }
            if (stad.IsSetCity()) {
                m_CityCtrl->SetValue(ToWxString(stad.GetCity()));
            }
            if (stad.IsSetSub()) {
                m_SubCtrl->SetValue(ToWxString(stad.GetSub()));
            }
            if (stad.IsSetPostal_code()) {
                m_ZipCtrl->SetValue(ToWxString(stad.GetPostal_code()));
            }
            if (stad.IsSetCountry()) {
                m_CountryCtrl->SetValue(ToWxString(stad.GetCountry()));
            }
            if (stad.IsSetPhone()) {
                m_PhoneCtrl->SetValue(ToWxString(stad.GetPhone()));
            }
        }
    }
    return true;
}


bool CSubmitterAffilPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow()) {
        return false;
    }
    if (!m_SubmitBlock)
        return false;

    auto& contact = m_SubmitBlock->SetContact().SetContact();
    auto& affil = contact.SetAffil().SetStd();

    if (m_AffilCtrl->GetValue().IsEmpty())
        affil.ResetAffil();
    else
        affil.SetAffil(ToStdString(m_AffilCtrl->GetValue()));

    if (m_DepartmentCtrl->GetValue().IsEmpty())
        affil.ResetDiv();
    else
        affil.SetDiv(ToStdString(m_DepartmentCtrl->GetValue()));

    if (m_StreetCtrl->GetValue().IsEmpty())
        affil.ResetStreet();
    else
        affil.SetStreet(ToStdString(m_StreetCtrl->GetValue()));
    
    if (m_CityCtrl->GetValue().IsEmpty())
        affil.ResetCity();
    else
        affil.SetCity(ToStdString(m_CityCtrl->GetValue()));

    if (m_SubCtrl->GetValue().IsEmpty())
        affil.ResetSub();
    else
        affil.SetSub(ToStdString(m_SubCtrl->GetValue()));

    if (m_ZipCtrl->GetValue().IsEmpty())
        affil.ResetPostal_code();
    else
        affil.SetPostal_code(ToStdString(m_ZipCtrl->GetValue()));

    if (m_CountryCtrl->GetValue().IsEmpty())
        affil.ResetCountry();
    else
        affil.SetCountry(ToStdString(m_CountryCtrl->GetValue()));

    if (m_PhoneCtrl->GetValue().IsEmpty())
        affil.ResetPhone();
    else
        affil.SetPhone(ToStdString(m_PhoneCtrl->GetValue()));

    if (!affil.IsSetAffil() && 
        !affil.IsSetDiv() && 
        !affil.IsSetStreet() && 
        !affil.IsSetCity() && 
        !affil.IsSetSub() &&
        !affil.IsSetPostal_code() &&
        !affil.IsSetCountry() &&
        !affil.IsSetPhone())
        contact.ResetAffil();

    // store the same information in the Cit-sub affiliation field
    x_CopyAffil(m_SubmitBlock->SetCit().SetAuthors());

    return true;
}

void CSubmitterAffilPanel::x_CopyAffil(objects::CAuth_list &auth_list)
{
    const auto& contact = m_SubmitBlock->GetContact().GetContact();
    if (contact.IsSetAffil()) {
        auth_list.SetAffil().Assign(contact.GetAffil());
    }
    else {
        auth_list.ResetAffil();
    }
}

void CSubmitterAffilPanel::ApplyCommand()
{
    TransferDataFromWindow();
    CRef<objects::CAffil> empty(new objects::CAffil);
    empty->SetStd();
    
    if (m_SubmitBlock->IsSetCit() &&
        m_SubmitBlock->GetCit().IsSetAuthors() &&
        m_SubmitBlock->GetCit().GetAuthors().IsSetAffil() &&
        m_SubmitBlock->GetCit().GetAuthors().GetAffil().Equals(*empty)) {
        return;
    }
    GetParentWizard(this)->ApplySubmitCommand();    
}

void CSubmitterAffilPanel::ReportMissingFields(string &text)
{
    if (m_AffilCtrl->GetValue().IsEmpty())
        text += "Submitter Institution\n";

    if (m_DepartmentCtrl->GetValue().IsEmpty())
        text += "Submitter Department\n";

    if (m_StreetCtrl->GetValue().IsEmpty())
        text += "Submitter Street\n";
    
    if (m_CityCtrl->GetValue().IsEmpty())
        text += "Submitter City\n";

    if (m_ZipCtrl->GetValue().IsEmpty())
        text += "Submitter Postal Code\n";

    if (m_CountryCtrl->GetValue().IsEmpty())
        text += "Submitter Country\n";;
}

END_NCBI_SCOPE
