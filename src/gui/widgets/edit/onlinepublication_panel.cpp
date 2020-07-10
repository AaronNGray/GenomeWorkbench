/*  $Id: onlinepublication_panel.cpp 31553 2014-10-22 16:28:19Z katargir $
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
#include <gui/widgets/wx/number_validator.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <objects/biblio/Cit_gen.hpp>

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>

////@begin includes
////@end includes
////@begin includes
////@end includes

#include "onlinepublication_panel.hpp"

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
 * COnlinePublicationPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( COnlinePublicationPanel, wxPanel )


/*
 * COnlinePublicationPanel event table definition
 */

BEGIN_EVENT_TABLE( COnlinePublicationPanel, wxPanel )

////@begin COnlinePublicationPanel event table entries
////@end COnlinePublicationPanel event table entries

END_EVENT_TABLE()


/*
 * COnlinePublicationPanel constructors
 */

COnlinePublicationPanel::COnlinePublicationPanel()
{
    Init();
}

COnlinePublicationPanel::COnlinePublicationPanel( wxWindow* parent, CSerialObject& object,
         wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
     m_Object(0)
{
    Init();
    m_Object = dynamic_cast<CCit_gen*>(&object);
    const CCit_gen& citgen = dynamic_cast<const CCit_gen&>(*m_Object);
    m_EditedCit.Reset((CSerialObject*)CCit_gen::GetTypeInfo()->Create());
    m_EditedCit->Assign(citgen);
    if (citgen.IsSetSerial_number()) {
        m_SerialNumber = citgen.GetSerial_number();
    } else {
        m_SerialNumber = 0;
    }
    Create(parent, id, pos, size, style);
}


/*
 * COnlinePublicationPanel creator
 */

bool COnlinePublicationPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin COnlinePublicationPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end COnlinePublicationPanel creation
    return true;
}


/*
 * COnlinePublicationPanel destructor
 */

COnlinePublicationPanel::~COnlinePublicationPanel()
{
////@begin COnlinePublicationPanel destruction
////@end COnlinePublicationPanel destruction
}


/*
 * Member initialisation
 */

void COnlinePublicationPanel::Init()
{
////@begin COnlinePublicationPanel member initialisation
////@end COnlinePublicationPanel member initialisation
}


/*
 * Control creation for COnlinePublicationPanel
 */

void COnlinePublicationPanel::CreateControls()
{    
////@begin COnlinePublicationPanel content construction
    COnlinePublicationPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Title"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl5 = new wxTextCtrl( itemPanel1, ID_ONLINEPUBLICATION_TITLE, wxEmptyString, wxDefaultPosition, wxSize(400, -1), wxTE_MULTILINE );
    itemFlexGridSizer3->Add(itemTextCtrl5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Serial Number"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl7 = new wxTextCtrl( itemPanel1, ID_ONLINEPUBLICATION_SERIAL_NUMBER, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end COnlinePublicationPanel content construction
    CCit_gen& citgen = dynamic_cast<CCit_gen&>(*m_EditedCit);
    itemTextCtrl5->SetValidator( CSerialTextValidator(citgen, "title") );
    m_SerialNumber = 0;
    if (citgen.IsSetSerial_number()) {
        m_SerialNumber = citgen.GetSerial_number();
    }
    itemTextCtrl7->SetValidator( CNumberOrBlankValidator(&m_SerialNumber) );

}

bool COnlinePublicationPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    CCit_gen& citgen = dynamic_cast<CCit_gen&>(*m_EditedCit);
    if (m_SerialNumber < 1) {
        citgen.ResetSerial_number();
    } else {
        citgen.SetSerial_number(m_SerialNumber);
    }

    return true;
}


/*
 * Should we show tooltips?
 */

bool COnlinePublicationPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap COnlinePublicationPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin COnlinePublicationPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end COnlinePublicationPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon COnlinePublicationPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin COnlinePublicationPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end COnlinePublicationPanel icon retrieval
}
END_NCBI_SCOPE
