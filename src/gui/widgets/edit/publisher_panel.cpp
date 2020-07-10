/*  $Id: publisher_panel.cpp 38020 2017-03-15 17:51:59Z filippov $
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


#include <ncbi_pch.hpp>
#include <gui/utils/command_processor.hpp>
#include <gui/widgets/edit/edit_object.hpp>

#include <objects/biblio/Imprint.hpp>
#include <objects/biblio/Affil.hpp>
#include <objects/general/Date.hpp>
#include <objects/general/Date_std.hpp>

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>
#include <gui/widgets/wx/number_validator.hpp>

////@begin includes
////@end includes
////@begin includes
////@end includes

#include "publisher_panel.hpp"

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*
 * CPublisherPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CPublisherPanel, wxPanel )


/*
 * CPublisherPanel event table definition
 */

BEGIN_EVENT_TABLE( CPublisherPanel, wxPanel )

////@begin CPublisherPanel event table entries
////@end CPublisherPanel event table entries

END_EVENT_TABLE()


/*
 * CPublisherPanel constructors
 */

CPublisherPanel::CPublisherPanel()
{
    Init();
}

CPublisherPanel::CPublisherPanel( wxWindow* parent, CSerialObject& object, bool show_pages,
         wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
     m_Object(0), m_ShowPages(show_pages)
{
    Init();
    m_Object = dynamic_cast<CImprint*>(&object);

    const CImprint& imprint = dynamic_cast<const CImprint&>(*m_Object);
    m_EditedImprint.Reset((CSerialObject*)CImprint::GetTypeInfo()->Create());
    m_EditedImprint->Assign(imprint);
    Create(parent, id, pos, size, style);
}


/*
 * CPublisherPanel creator
 */

bool CPublisherPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CPublisherPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CPublisherPanel creation
    return true;
}


/*
 * CPublisherPanel destructor
 */

CPublisherPanel::~CPublisherPanel()
{
////@begin CPublisherPanel destruction
////@end CPublisherPanel destruction
}


/*
 * Member initialisation
 */

void CPublisherPanel::Init()
{
////@begin CPublisherPanel member initialisation
    m_GridSizer = NULL;
    m_AffilCtrl = NULL;
    m_StreetCtrl = NULL;
    m_CityCtrl = NULL;
    m_SubCtrl = NULL;
    m_ZipCtrl = NULL;
    m_CountryCtrl = NULL;
    m_PhoneCtrl = NULL;
    m_FaxCtrl = NULL;
    m_EmailCtrl = NULL;
    m_PubYearCtrl = NULL;
    m_CpyYearCtrl = NULL;
////@end CPublisherPanel member initialisation
}


/*
 * Control creation for CPublisherPanel
 */

void CPublisherPanel::CreateControls()
{    
////@begin CPublisherPanel content construction
    CPublisherPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_GridSizer = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(m_GridSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Publisher"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GridSizer->Add(itemStaticText4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AffilCtrl = new wxTextCtrl( itemPanel1, ID_PUBLISHER_NAME, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    m_GridSizer->Add(m_AffilCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Address"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GridSizer->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_StreetCtrl = new wxTextCtrl( itemPanel1, ID_PUBLISHER_ADDRESS, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    m_GridSizer->Add(m_StreetCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("City"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GridSizer->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CityCtrl = new wxTextCtrl( itemPanel1, ID_PUBLISHER_CITY, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    m_GridSizer->Add(m_CityCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("State/Province"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GridSizer->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SubCtrl = new wxTextCtrl( itemPanel1, ID_PUBLISHER_STATE, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    m_GridSizer->Add(m_SubCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel1, wxID_STATIC, _("Zip/Postal Code"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GridSizer->Add(itemStaticText12, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ZipCtrl = new wxTextCtrl( itemPanel1, ID_PUBLISHER_CODE, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    m_GridSizer->Add(m_ZipCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText14 = new wxStaticText( itemPanel1, wxID_STATIC, _("Country"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GridSizer->Add(itemStaticText14, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CountryCtrl = new wxTextCtrl( itemPanel1, ID_PUBLISHER_COUNTRY, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    m_GridSizer->Add(m_CountryCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText16 = new wxStaticText( itemPanel1, wxID_STATIC, _("Phone (*)"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GridSizer->Add(itemStaticText16, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_PhoneCtrl = new wxTextCtrl( itemPanel1, ID_PUBLISHER_PHONE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_GridSizer->Add(m_PhoneCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText18 = new wxStaticText( itemPanel1, wxID_STATIC, _("Fax (*)"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GridSizer->Add(itemStaticText18, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FaxCtrl = new wxTextCtrl( itemPanel1, ID_PUBLISHER_FAX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_GridSizer->Add(m_FaxCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText20 = new wxStaticText( itemPanel1, wxID_STATIC, _("Internet Access"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GridSizer->Add(itemStaticText20, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EmailCtrl = new wxTextCtrl( itemPanel1, ID_PUBLISHER_EMAIL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    m_GridSizer->Add(m_EmailCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText22 = new wxStaticText( itemPanel1, wxID_STATIC, _("Publication Year"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GridSizer->Add(itemStaticText22, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_PubYearCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_GridSizer->Add(m_PubYearCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText24 = new wxStaticText( itemPanel1, wxID_STATIC, _("Copyright Year"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GridSizer->Add(itemStaticText24, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CpyYearCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_GridSizer->Add(m_CpyYearCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText26 = new wxStaticText( itemPanel1, wxID_STATIC, _("(*) Please include country code for non-U.S. phone numbers"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText26, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CPublisherPanel content construction

    CImprint& imprint = dynamic_cast<CImprint&>(*m_EditedImprint);

    if (m_ShowPages) {
        wxStaticText* pages_label = new wxStaticText( itemPanel1, wxID_STATIC, _("Pages"), wxDefaultPosition, wxDefaultSize, 0 );
        m_GridSizer->Add(pages_label, 0,  wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        wxTextCtrl* pages_ctrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
        m_GridSizer->Add(pages_ctrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
        pages_ctrl->SetValidator( CSerialTextValidator(imprint, "pages") );
    }
                
    if (imprint.IsSetDate() && imprint.GetDate().IsStd() && imprint.GetDate().GetStd().IsSetYear()) {
        m_PubYear = imprint.GetDate().GetStd().GetYear();
    } else {
        m_PubYear = 0;
    }
    if (imprint.IsSetCprt() && imprint.GetCprt().IsStd() && imprint.GetCprt().GetStd().IsSetYear()) {
        m_CopyrightYear = imprint.GetCprt().GetStd().GetYear();
    } else {
        m_CopyrightYear = 0;
    }
    CAffil& pub_affil = imprint.SetPub();
    if (pub_affil.IsStr()) {
        string aff = pub_affil.GetStr();
        pub_affil.SetStd().SetAffil(aff);
    }
    CAffil::C_Std& affil_std = pub_affil.SetStd();

    m_AffilCtrl->SetValidator( CSerialTextValidator(affil_std, "affil") );
    m_StreetCtrl->SetValidator( CSerialTextValidator(affil_std, "street") );
    m_CityCtrl->SetValidator( CSerialTextValidator(affil_std, "city") );
    m_SubCtrl->SetValidator( CSerialTextValidator(affil_std, "sub") );
    m_ZipCtrl->SetValidator( CSerialTextValidator(affil_std, "postal-code") );
    m_CountryCtrl->SetValidator( CSerialTextValidator(affil_std, "country") );
    m_PhoneCtrl->SetValidator( CSerialTextValidator(affil_std, "phone") );
    m_FaxCtrl->SetValidator( CSerialTextValidator(affil_std, "fax") );
    m_EmailCtrl->SetValidator( CSerialTextValidator(affil_std, "email") );
    m_PubYearCtrl->SetValidator( CNumberOrBlankValidator(&m_PubYear));
    m_CpyYearCtrl->SetValidator( CNumberOrBlankValidator(&m_CopyrightYear));
    
}


bool CPublisherPanel::TransferDataToWindow()
{
    CImprint& imprint = dynamic_cast<CImprint&>(*m_EditedImprint);
    CAffil& pub_affil = imprint.SetPub();
    if (pub_affil.IsStr()) {
        string aff = pub_affil.GetStr();
        pub_affil.SetStd().SetAffil(aff);
    }

    if (!wxPanel::TransferDataToWindow())
        return false;
    if (imprint.IsSetDate() && imprint.GetDate().IsStd() && imprint.GetDate().GetStd().IsSetYear()) {
        m_PubYear = imprint.GetDate().GetStd().GetYear();
    } else {
        m_PubYear = 0;
    }
    if (imprint.IsSetCprt() && imprint.GetCprt().IsStd() && imprint.GetCprt().GetStd().IsSetYear()) {
        m_CopyrightYear = imprint.GetCprt().GetStd().GetYear();
    } else {
        m_CopyrightYear = 0;
    }
    return true;
}


bool CPublisherPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;
    CImprint& imprint = dynamic_cast<CImprint&>(*m_EditedImprint);
    if (m_PubYear > 0) {
        imprint.SetDate().SetStd().SetYear(m_PubYear);
    } else {
        imprint.ResetDate();
    }
    if (m_CopyrightYear > 0) {
        imprint.SetCprt().SetStd().SetYear(m_CopyrightYear);
    } else {
        imprint.ResetCprt();
    }
    return true;
}


void CPublisherPanel::PopulatePublisherFields (objects::CImprint& imprint)
{
    TransferDataFromWindow();

    if (m_PubYear > 0) {
        imprint.SetDate().SetStd().SetYear(m_PubYear);
    } else {
        imprint.ResetDate();
    }
    if (m_CopyrightYear > 0) {
        imprint.SetCprt().SetStd().SetYear(m_CopyrightYear);
    } else {
        imprint.ResetCprt();
    }

    CImprint& edited_imprint = dynamic_cast<CImprint&>(*m_EditedImprint);

    if (edited_imprint.IsSetPages()) {
        imprint.SetPages(edited_imprint.GetPages());
    } else {
        imprint.ResetPages();
    }
    if (edited_imprint.IsSetPub() && edited_imprint.GetPub().IsStd() && // Pub.Std is always set thanks to CreateControls
        (edited_imprint.GetPub().GetStd().IsSetAffil() || edited_imprint.GetPub().GetStd().IsSetDiv() || edited_imprint.GetPub().GetStd().IsSetCity() ||
         edited_imprint.GetPub().GetStd().IsSetSub() || edited_imprint.GetPub().GetStd().IsSetCountry() || edited_imprint.GetPub().GetStd().IsSetStreet() ||
         edited_imprint.GetPub().GetStd().IsSetEmail() || edited_imprint.GetPub().GetStd().IsSetFax() || edited_imprint.GetPub().GetStd().IsSetPhone() || 
         edited_imprint.GetPub().GetStd().IsSetPostal_code() )        
        ) {
        imprint.SetPub(edited_imprint.SetPub());
    } else {
        imprint.ResetPub();
    }
}


/*
 * Should we show tooltips?
 */

bool CPublisherPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CPublisherPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CPublisherPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CPublisherPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CPublisherPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CPublisherPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CPublisherPanel icon retrieval
}

END_NCBI_SCOPE
