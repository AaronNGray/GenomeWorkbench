/*  $Id: proceedings_panel.cpp 31553 2014-10-22 16:28:19Z katargir $
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
#include <objects/biblio/Cit_proc.hpp>
#include <objects/biblio/Imprint.hpp>
#include <objects/biblio/Title.hpp>
#include <objects/general/Date.hpp>
#include <objects/general/Date_std.hpp>

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>
#include <gui/widgets/wx/number_validator.hpp>

////@begin includes
////@end includes
////@begin includes
////@end includes

#include "proceedings_panel.hpp"

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
 * CProceedingsPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CProceedingsPanel, wxPanel )


/*
 * CProceedingsPanel event table definition
 */

BEGIN_EVENT_TABLE( CProceedingsPanel, wxPanel )

////@begin CProceedingsPanel event table entries
////@end CProceedingsPanel event table entries

END_EVENT_TABLE()


/*
 * CProceedingsPanel constructors
 */

CProceedingsPanel::CProceedingsPanel()
{
    Init();
}

CProceedingsPanel::CProceedingsPanel( wxWindow* parent, CRef<CCit_book> book,
         wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
     m_Book(book)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CProceedingsPanel creator
 */

bool CProceedingsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CProceedingsPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CProceedingsPanel creation
    return true;
}


/*
 * CProceedingsPanel destructor
 */

CProceedingsPanel::~CProceedingsPanel()
{
////@begin CProceedingsPanel destruction
////@end CProceedingsPanel destruction
}


/*
 * Member initialisation
 */

void CProceedingsPanel::Init()
{
////@begin CProceedingsPanel member initialisation
////@end CProceedingsPanel member initialisation
}


/*
 * Control creation for CProceedingsPanel
 */

void CProceedingsPanel::CreateControls()
{    
////@begin CProceedingsPanel content construction
    CProceedingsPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Proceedings Title"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl5 = new wxTextCtrl( itemPanel1, ID_PROCEEDINGS_TITLE, wxEmptyString, wxDefaultPosition, wxSize(400, -1), wxTE_MULTILINE );
    itemFlexGridSizer3->Add(itemTextCtrl5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Publication Year"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl7 = new wxTextCtrl( itemPanel1, ID_PROCEEDINGS_PUB_YEAR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Copyright Year"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemPanel1, ID_PROCEEDINGS_COPY_YEAR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CProceedingsPanel content construction

    CImprint& imprint = m_Book->SetImp();
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
    CSerialObject& title_object = m_Book->SetTitle();
    itemTextCtrl5->SetValidator( CSerialTitleValidator(title_object, CTitle::C_E::e_Name, "Proceedings Title") );
    itemTextCtrl7->SetValidator( CNumberOrBlankValidator(&m_PubYear));
    itemTextCtrl9->SetValidator( CNumberOrBlankValidator(&m_CopyrightYear));

}


bool CProceedingsPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;
    CImprint& imprint = m_Book->SetImp();
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
 

bool CProceedingsPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;
    if (m_PubYear > 0) {
        m_Book->SetImp().SetDate().SetStd().SetYear(m_PubYear);
    } else {
        m_Book->SetImp().ResetDate();
    }
    if (m_CopyrightYear > 0) {
        m_Book->SetImp().SetCprt().SetStd().SetYear(m_CopyrightYear);
    } else {
        m_Book->SetImp().ResetCprt();
    }
    return true;
}


/*
 * Should we show tooltips?
 */

bool CProceedingsPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CProceedingsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CProceedingsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CProceedingsPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CProceedingsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CProceedingsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CProceedingsPanel icon retrieval
}
END_NCBI_SCOPE
