/*  $Id: locale_panel.cpp 34283 2015-12-08 21:44:29Z filippov $
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
#include <objects/biblio/Affil.hpp>
#include <objects/biblio/Meeting.hpp>

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include "wx/datectrl.h"

#include "locale_panel.hpp"
#include <gui/widgets/edit/flexibledate_panel.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*
 * CLocalePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CLocalePanel, wxPanel )


/*
 * CLocalePanel event table definition
 */

BEGIN_EVENT_TABLE( CLocalePanel, wxPanel )

////@begin CLocalePanel event table entries
////@end CLocalePanel event table entries

END_EVENT_TABLE()


/*
 * CLocalePanel constructors
 */

CLocalePanel::CLocalePanel()
{
    Init();
}

CLocalePanel::CLocalePanel( wxWindow* parent, CSerialObject& object,
         wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
     m_Object(0)
{
    Init();
    m_Object = dynamic_cast<CMeeting*>(&object);
    const CMeeting& meeting = dynamic_cast<const CMeeting&>(*m_Object);
    m_EditedMeeting.Reset((CSerialObject*)CMeeting::GetTypeInfo()->Create());
    m_EditedMeeting->Assign(meeting);
    Create(parent, id, pos, size, style);
}


/*
 * CLocalePanel creator
 */

bool CLocalePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CLocalePanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CLocalePanel creation
    return true;
}


/*
 * CLocalePanel destructor
 */

CLocalePanel::~CLocalePanel()
{
////@begin CLocalePanel destruction
////@end CLocalePanel destruction
}


/*
 * Member initialisation
 */

void CLocalePanel::Init()
{
////@begin CLocalePanel member initialisation
////@end CLocalePanel member initialisation
}


/*
 * Control creation for CLocalePanel
 */

void CLocalePanel::CreateControls()
{    
////@begin CLocalePanel content construction
    CLocalePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Location"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl5 = new wxTextCtrl( itemPanel1, ID_LOCALE_NAME, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer3->Add(itemTextCtrl5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Address"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl7 = new wxTextCtrl( itemPanel1, ID_LOCALE_ADDRESS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl7, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("City"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemPanel1, ID_LOCALE_CITY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl9, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("State/Province"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl11 = new wxTextCtrl( itemPanel1, ID_LOCALE_STATE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl11, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel1, wxID_STATIC, _("Country"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText12, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl13 = new wxTextCtrl( itemPanel1, ID_LOCALE_COUNTRY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl13, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText14 = new wxStaticText( itemPanel1, wxID_STATIC, _("Proceedings Number \n(i.e., 4th Ann or IV)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText14, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl15 = new wxTextCtrl( itemPanel1, ID_LOCALE_PROCNUM, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl15, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText16 = new wxStaticText( itemPanel1, wxID_STATIC, _("Date of Meeting"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText16, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CLocalePanel content construction

    CMeeting& meeting = dynamic_cast<CMeeting&>(*m_EditedMeeting);
    CAffil& affil = meeting.SetPlace();
    CAffil::C_Std& affil_std = affil.SetStd();
    itemTextCtrl5->SetValidator( CSerialTextValidator(affil_std, "affil") );
    itemTextCtrl7->SetValidator( CSerialTextValidator(affil_std, "street") );
    itemTextCtrl9->SetValidator( CSerialTextValidator(affil_std, "city") );
    itemTextCtrl11->SetValidator( CSerialTextValidator(affil_std, "sub") );
    itemTextCtrl13->SetValidator( CSerialTextValidator(affil_std, "country") );
    itemTextCtrl15->SetValidator( CSerialTextValidator(meeting, "number") );
    // date of meeting
    CRef<objects::CDate> p_date(&(meeting.SetDate()));
    m_PubDate = new CFlexibleDatePanel(itemPanel1, p_date);
    itemFlexGridSizer3->Add ( m_PubDate );

}

bool CLocalePanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow()) {
        return false;
    }
    m_PubDate->TransferDataFromWindow();

    return true;
}

bool CLocalePanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow()) {
        return false;
    }
    m_PubDate->TransferDataToWindow();

    return true;
}

CRef<CMeeting> CLocalePanel::GetMeeting() const
{
    const CMeeting& meeting = dynamic_cast<const CMeeting&>(*m_EditedMeeting);
    CRef<CMeeting> ret(new CMeeting());
    ret->Assign(meeting);
    return ret;
}


/*
 * Should we show tooltips?
 */

bool CLocalePanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CLocalePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CLocalePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CLocalePanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CLocalePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CLocalePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CLocalePanel icon retrieval
}
END_NCBI_SCOPE
