/*  $Id: patent_panel.cpp 35726 2016-06-15 18:09:45Z asztalos $
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

#include <objects/biblio/Cit_pat.hpp>
#include <objects/general/Date.hpp>

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>

////@begin includes
////@end includes

#include "patent_panel.hpp"
#include <gui/widgets/edit/flexibledate_panel.hpp>

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
 * CPatentPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CPatentPanel, wxPanel )


/*
 * CPatentPanel event table definition
 */

BEGIN_EVENT_TABLE( CPatentPanel, wxPanel )

////@begin CPatentPanel event table entries
////@end CPatentPanel event table entries

END_EVENT_TABLE()


/*
 * CPatentPanel constructors
 */

CPatentPanel::CPatentPanel()
{
    Init();
}

CPatentPanel::CPatentPanel( wxWindow* parent, CSerialObject& object,
         wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
     m_Object(0)
{
    Init();
    m_Object = dynamic_cast<CCit_pat*>(&object);
    const CCit_pat& patent = dynamic_cast<const CCit_pat&>(*m_Object);
    m_EditedPatent.Reset((CSerialObject*)CCit_pat::GetTypeInfo()->Create());
    m_EditedPatent->Assign(patent);
    Create(parent, id, pos, size, style);
}


/*
 * CPatentPanel creator
 */

bool CPatentPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CPatentPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CPatentPanel creation
    return true;
}


/*
 * CPatentPanel destructor
 */

CPatentPanel::~CPatentPanel()
{
////@begin CPatentPanel destruction
////@end CPatentPanel destruction
}


/*
 * Member initialisation
 */

void CPatentPanel::Init()
{
////@begin CPatentPanel member initialisation
////@end CPatentPanel member initialisation
}


/*
 * Control creation for CPatentPanel
 */

void CPatentPanel::CreateControls()
{    
////@begin CPatentPanel content construction
    CPatentPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Patent Title"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl5 = new wxTextCtrl( itemPanel1, ID_PATENT_TITLE, wxEmptyString, wxDefaultPosition, wxSize(400, 85), wxTE_MULTILINE );
    itemFlexGridSizer3->Add(itemTextCtrl5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Abstract"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl7 = new wxTextCtrl( itemPanel1, ID_PATENT_ABSTRACT, wxEmptyString, wxDefaultPosition, wxSize(400, 100), wxTE_MULTILINE );
    itemFlexGridSizer3->Add(itemTextCtrl7, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Country"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemPanel1, ID_PATENT_COUNTRY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("Document Type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl11 = new wxTextCtrl( itemPanel1, ID_PATENT_DOCTYPE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel1, wxID_STATIC, _("Patent Number"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText12, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl13 = new wxTextCtrl( itemPanel1, ID_PATENT_DOCNUM, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText14 = new wxStaticText( itemPanel1, wxID_STATIC, _("Issue Date"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText14, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer3->Add(itemBoxSizer15, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText16 = new wxStaticText( itemPanel1, wxID_STATIC, _("Application Number"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText16, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl17 = new wxTextCtrl( itemPanel1, ID_PATENT_APPNUM, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl17, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText18 = new wxStaticText( itemPanel1, wxID_STATIC, _("Application Date"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText18, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer19 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer3->Add(itemBoxSizer19, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CPatentPanel content construction
    CCit_pat& patent = dynamic_cast<CCit_pat&>(*m_EditedPatent);
    CRef<objects::CDate> issuedate(&(patent.SetDate_issue()));
    m_issuedate_ctrl = new CFlexibleDatePanel(itemPanel1, issuedate);
    itemBoxSizer15->Add(m_issuedate_ctrl);
    CRef<objects::CDate> appdate(&(patent.SetApp_date()));
    m_appdate_ctrl = new CFlexibleDatePanel(itemPanel1, appdate);
    itemBoxSizer19->Add(m_appdate_ctrl);


    itemTextCtrl5->SetValidator( CSerialTextValidator(patent, "title") );
    itemTextCtrl7->SetValidator( CSerialTextValidator(patent, "abstract") );
    itemTextCtrl9->SetValidator( CSerialTextValidator(patent, "country") );
    itemTextCtrl11->SetValidator( CSerialTextValidator(patent, "doc-type") );
    itemTextCtrl13->SetValidator( CSerialTextValidator(patent, "number") );
    itemTextCtrl17->SetValidator( CSerialTextValidator(patent, "app-number") );
}

bool CPatentPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow()) {
        return false;
    }
    m_issuedate_ctrl->TransferDataFromWindow();
    m_appdate_ctrl->TransferDataFromWindow();
    return true;
}

bool CPatentPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow()) {
        return false;
    }
    m_issuedate_ctrl->TransferDataToWindow();
    m_appdate_ctrl->TransferDataToWindow();
    return true;
}

CRef<CCit_pat> CPatentPanel::GetCit_pat(void) const
{
    const CCit_pat& pat = dynamic_cast<const CCit_pat&>(*m_EditedPatent);

    CRef<CCit_pat> ret(new CCit_pat());
    ret->Assign(pat);
    return ret;
}


/*
 * Should we show tooltips?
 */

bool CPatentPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CPatentPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CPatentPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CPatentPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CPatentPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CPatentPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CPatentPanel icon retrieval
}
END_NCBI_SCOPE
