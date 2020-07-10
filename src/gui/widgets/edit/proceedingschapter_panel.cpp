/*  $Id: proceedingschapter_panel.cpp 31553 2014-10-22 16:28:19Z katargir $
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
#include <objects/biblio/Cit_art.hpp>
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

#include "proceedingschapter_panel.hpp"

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
 * CProceedingsChapterPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CProceedingsChapterPanel, wxPanel )


/*
 * CProceedingsChapterPanel event table definition
 */

BEGIN_EVENT_TABLE( CProceedingsChapterPanel, wxPanel )

////@begin CProceedingsChapterPanel event table entries
////@end CProceedingsChapterPanel event table entries

END_EVENT_TABLE()


/*
 * CProceedingsChapterPanel constructors
 */

CProceedingsChapterPanel::CProceedingsChapterPanel()
{
    Init();
}

CProceedingsChapterPanel::CProceedingsChapterPanel( wxWindow* parent, CSerialObject& object,
         wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
     m_Object(0)
{
    Init();
    m_Object = dynamic_cast<CCit_art*>(&object);
    const CCit_art& art = dynamic_cast<const CCit_art&>(*m_Object);
    m_EditedArt.Reset((CSerialObject*)CCit_art::GetTypeInfo()->Create());
    m_EditedArt->Assign(art);
    Create(parent, id, pos, size, style);
}


/*
 * CProceedingsChapterPanel creator
 */

bool CProceedingsChapterPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CProceedingsChapterPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CProceedingsChapterPanel creation
    return true;
}


/*
 * CProceedingsChapterPanel destructor
 */

CProceedingsChapterPanel::~CProceedingsChapterPanel()
{
////@begin CProceedingsChapterPanel destruction
////@end CProceedingsChapterPanel destruction
}


/*
 * Member initialisation
 */

void CProceedingsChapterPanel::Init()
{
////@begin CProceedingsChapterPanel member initialisation
////@end CProceedingsChapterPanel member initialisation
}


/*
 * Control creation for CProceedingsChapterPanel
 */

void CProceedingsChapterPanel::CreateControls()
{    
////@begin CProceedingsChapterPanel content construction
    CProceedingsChapterPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Abstract Title"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl5 = new wxTextCtrl( itemPanel1, ID_PROCEEDINGSCHAPTER_ABSTRACT_TITLE, wxEmptyString, wxDefaultPosition, wxSize(400, -1), wxTE_MULTILINE );
    itemFlexGridSizer3->Add(itemTextCtrl5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Proceedings Title"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl7 = new wxTextCtrl( itemPanel1, ID_PROCEEDINGSCHAPTER_PROCEEDINGS_TITLE, wxEmptyString, wxDefaultPosition, wxSize(400, -1), wxTE_MULTILINE );
    itemFlexGridSizer3->Add(itemTextCtrl7, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Pages"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemPanel1, ID_PROCEEDINGSCHAPTER_PAGES, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("Publication Year"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl11 = new wxTextCtrl( itemPanel1, ID_PROCEEDINGSCHAPTER_PUB_YEAR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel1, wxID_STATIC, _("Copyright Year"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText12, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl13 = new wxTextCtrl( itemPanel1, ID_PROCEEDINGSCHAPTER_COPY_YEAR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CProceedingsChapterPanel content construction

    CCit_art& art = dynamic_cast<CCit_art&>(*m_EditedArt);
    CCit_proc& proc = art.SetFrom().SetProc();
    CCit_book& book = proc.SetBook();
    CImprint& imprint = book.SetImp();
    itemTextCtrl5->SetValidator( CSerialTitleValidator(art.SetTitle(), CTitle::C_E::e_Name, "Abstract Title") );
    itemTextCtrl7->SetValidator( CSerialTitleValidator(book.SetTitle(), CTitle::C_E::e_Name, "Proceedings Title") );
    itemTextCtrl9->SetValidator( CSerialTextValidator(imprint, "pages") );
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
    itemTextCtrl11->SetValidator( CNumberOrBlankValidator(&m_PubYear));
    itemTextCtrl13->SetValidator( CNumberOrBlankValidator(&m_CopyrightYear));

}


bool CProceedingsChapterPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;
    CCit_art& art = dynamic_cast<CCit_art&>(*m_EditedArt);
    CImprint& imprint = art.SetFrom().SetProc().SetBook().SetImp();
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
 

bool CProceedingsChapterPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;
    CCit_art& art = dynamic_cast<CCit_art&>(*m_EditedArt);
    CImprint& imprint = art.SetFrom().SetProc().SetBook().SetImp();
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


/*
 * Should we show tooltips?
 */

bool CProceedingsChapterPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CProceedingsChapterPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CProceedingsChapterPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CProceedingsChapterPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CProceedingsChapterPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CProceedingsChapterPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CProceedingsChapterPanel icon retrieval
}
END_NCBI_SCOPE
