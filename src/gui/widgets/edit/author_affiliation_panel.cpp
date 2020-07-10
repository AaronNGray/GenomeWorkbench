/*  $Id: author_affiliation_panel.cpp 42280 2019-01-24 20:12:34Z asztalos $
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


#include <ncbi_pch.hpp>
#include <gui/utils/command_processor.hpp>
#include <gui/widgets/edit/edit_object.hpp>

#include <objects/biblio/Affil.hpp>

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>


#include <gui/widgets/edit/author_affiliation_panel.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*
 * CAuthorAffiliationPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAuthorAffiliationPanel, wxPanel )


/*
 * CAuthorAffiliationPanel event table definition
 */

BEGIN_EVENT_TABLE( CAuthorAffiliationPanel, wxPanel )

////@begin CAuthorAffiliationPanel event table entries
////@end CAuthorAffiliationPanel event table entries

END_EVENT_TABLE()


/*
 * CAuthorAffiliationPanel constructors
 */

CAuthorAffiliationPanel::CAuthorAffiliationPanel()
{
    Init();
}

CAuthorAffiliationPanel::CAuthorAffiliationPanel( wxWindow* parent, const CAffil& affil,
         wxWindowID id, const wxPoint& pos, const wxSize& size, long style )     
{
    m_Affil.Reset(new CAffil);
    m_Affil->Assign(affil);
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CAuthorAffiliationPanel creator
 */

bool CAuthorAffiliationPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAuthorAffiliationPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAuthorAffiliationPanel creation
    return true;
}


/*
 * CAuthorAffiliationPanel destructor
 */

CAuthorAffiliationPanel::~CAuthorAffiliationPanel()
{
////@begin CAuthorAffiliationPanel destruction
////@end CAuthorAffiliationPanel destruction
}


/*
 * Member initialisation
 */

void CAuthorAffiliationPanel::Init()
{
////@begin CAuthorAffiliationPanel member initialisation
    m_AffilCtrl = NULL;
    m_DepartmentCtrl = NULL;
    m_StreetCtrl = NULL;
    m_CityCtrl = NULL;
    m_SubCtrl = NULL;
    m_ZipCtrl = NULL;
    m_CountryCtrl = NULL;
////@end CAuthorAffiliationPanel member initialisation
}


/*
 * Control creation for CAuthorAffiliationPanel
 */

void CAuthorAffiliationPanel::CreateControls()
{    
////@begin CAuthorAffiliationPanel content construction
    CAuthorAffiliationPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Institution"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AffilCtrl = new wxTextCtrl( itemPanel1, ID_AUTH_AFFIL_INST, wxEmptyString, wxDefaultPosition, wxSize(300, -1), 0 );
    itemFlexGridSizer4->Add(m_AffilCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Department"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText7, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DepartmentCtrl = new wxTextCtrl( itemPanel1, ID_AUTH_AFFIL_DEPT, wxEmptyString, wxDefaultPosition, wxSize(300, -1), 0 );
    itemFlexGridSizer4->Add(m_DepartmentCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel1, wxID_STATIC, _("Street Address"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText9, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_StreetCtrl = new wxTextCtrl( itemPanel1, ID_AUTH_AFFIL_ADDRESS, wxEmptyString, wxDefaultPosition, wxSize(300, -1), 0 );
    itemFlexGridSizer4->Add(m_StreetCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemPanel1, wxID_STATIC, _("City"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText11, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CityCtrl = new wxTextCtrl( itemPanel1, ID_AUTH_AFFIL_CITY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), 0 );
    itemFlexGridSizer4->Add(m_CityCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemPanel1, wxID_STATIC, _("State/Province"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText13, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SubCtrl = new wxTextCtrl( itemPanel1, ID_AUTH_AFFIL_STATE, wxEmptyString, wxDefaultPosition, wxSize(300, -1), 0 );
    itemFlexGridSizer4->Add(m_SubCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText15 = new wxStaticText( itemPanel1, wxID_STATIC, _("Zip/Postal Code"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText15, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ZipCtrl = new wxTextCtrl( itemPanel1, ID_AUTH_AFFIL_CODE, wxEmptyString, wxDefaultPosition, wxSize(300, -1), 0 );
    itemFlexGridSizer4->Add(m_ZipCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText17 = new wxStaticText( itemPanel1, wxID_STATIC, _("Country"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText17, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CountryCtrl = new wxTextCtrl( itemPanel1, ID_AUTH_AFFIL_COUNTRY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), 0 );
    itemFlexGridSizer4->Add(m_CountryCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CAuthorAffiliationPanel content construction

}


#define AFFILPANEL_SETSTD(Field,Ctrl) \
    if (m_Affil->GetStd().IsSet##Field()) { \
        m_##Ctrl->SetValue (ToWxString (m_Affil->SetStd().Set##Field())); \
    } else { \
        m_##Ctrl->SetValue (wxEmptyString); \
    } 

bool CAuthorAffiliationPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    if (m_Affil->IsStd()) {
        AFFILPANEL_SETSTD(Affil,AffilCtrl);
        AFFILPANEL_SETSTD(Div,DepartmentCtrl);
        AFFILPANEL_SETSTD(Street,StreetCtrl);
        AFFILPANEL_SETSTD(City,CityCtrl);
        AFFILPANEL_SETSTD(Sub,SubCtrl);
        AFFILPANEL_SETSTD(Postal_code,ZipCtrl);
        AFFILPANEL_SETSTD(Country,CountryCtrl);
    } else if (m_Affil->IsStr()) {
        m_AffilCtrl->SetValue(ToWxString (m_Affil->GetStr()));
        m_DepartmentCtrl->SetValue(wxEmptyString);
        m_StreetCtrl->SetValue(wxEmptyString);
        m_CityCtrl->SetValue(wxEmptyString);
        m_SubCtrl->SetValue(wxEmptyString);
        m_ZipCtrl->SetValue(wxEmptyString);
        m_CountryCtrl->SetValue(wxEmptyString);
    } else {
        m_AffilCtrl->SetValue(wxEmptyString);
        m_DepartmentCtrl->SetValue(wxEmptyString);
        m_StreetCtrl->SetValue(wxEmptyString);
        m_CityCtrl->SetValue(wxEmptyString);
        m_SubCtrl->SetValue(wxEmptyString);
        m_ZipCtrl->SetValue(wxEmptyString);
        m_CountryCtrl->SetValue(wxEmptyString);
    }
    return true;
}


#define AFFILPANEL_RESETBLANK(Field) \
    if (NStr::IsBlank (m_Affil->GetStd().Get##Field())) { \
        m_Affil->SetStd().Reset##Field(); \
    }

bool CAuthorAffiliationPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    string affil = ToAsciiStdString (m_AffilCtrl->GetValue());
    string div = ToAsciiStdString (m_DepartmentCtrl->GetValue());
    string street = ToAsciiStdString (m_StreetCtrl->GetValue());
    string city = ToAsciiStdString (m_CityCtrl->GetValue());
    string sub = ToAsciiStdString (m_SubCtrl->GetValue());
    string zip = ToAsciiStdString (m_ZipCtrl->GetValue());
    string country = ToAsciiStdString (m_CountryCtrl->GetValue());

    if (NStr::IsBlank(div) && NStr::IsBlank(street) 
        && NStr::IsBlank(city) && NStr::IsBlank(sub) 
        && NStr::IsBlank(zip) && NStr::IsBlank(country)) {
        
        if (NStr::IsBlank(affil)) {
            m_Affil.Reset();
        } else {
            if (m_Affil.IsNull()) {
                m_Affil.Reset(new CAffil);
            }
            m_Affil->SetStr(affil);
        }
    } else {
        if (m_Affil.IsNull()) {
            m_Affil.Reset(new CAffil);
        }
        m_Affil->SetStd().SetAffil(affil);
        m_Affil->SetStd().SetDiv(div);
        m_Affil->SetStd().SetStreet(street);
        m_Affil->SetStd().SetCity(city);
        m_Affil->SetStd().SetSub(sub);
        m_Affil->SetStd().SetPostal_code(zip);
        m_Affil->SetStd().SetCountry(country);
        AFFILPANEL_RESETBLANK(Affil)
        AFFILPANEL_RESETBLANK(Div)
        AFFILPANEL_RESETBLANK(Street)
        AFFILPANEL_RESETBLANK(City)
        AFFILPANEL_RESETBLANK(Sub)
        AFFILPANEL_RESETBLANK(Postal_code)
        AFFILPANEL_RESETBLANK(Country)
    }
    return true;
}

void CAuthorAffiliationPanel::SetAffil(CRef<objects::CAffil> affil)
{
    if (affil) {
        m_Affil = affil;
    }
    else {
        m_Affil.Reset();
    }
    TransferDataToWindow();
}

CRef<CAffil> CAuthorAffiliationPanel::GetAffil() 
{
    if (TransferDataFromWindow() && m_Affil) {
        return m_Affil;
    }

    return CRef<CAffil>();
}



/*
 * Should we show tooltips?
 */

bool CAuthorAffiliationPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CAuthorAffiliationPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAuthorAffiliationPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAuthorAffiliationPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CAuthorAffiliationPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAuthorAffiliationPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAuthorAffiliationPanel icon retrieval
}
END_NCBI_SCOPE
