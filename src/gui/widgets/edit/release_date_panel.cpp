/*  $Id: release_date_panel.cpp 27854 2013-04-12 20:52:44Z filippov $
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
#include <objects/submit/Submit_block.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/general/Date.hpp>
#include <objects/general/Date_std.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <gui/widgets/edit/release_date_panel.hpp>


BEGIN_NCBI_SCOPE


////@begin XPM images
////@end XPM images


/*
 * CReleaseDatePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CReleaseDatePanel, wxPanel )


/*
 * CReleaseDatePanel event table definition
 */

BEGIN_EVENT_TABLE( CReleaseDatePanel, wxPanel )

////@begin CReleaseDatePanel event table entries
    EVT_RADIOBUTTON( ID_RADIOBUTTON, CReleaseDatePanel::OnImmediateReleaseSelected )

    EVT_RADIOBUTTON( ID_RADIOBUTTON1, CReleaseDatePanel::OnHUPSelected )

////@end CReleaseDatePanel event table entries

END_EVENT_TABLE()


/*
 * CReleaseDatePanel constructors
 */

CReleaseDatePanel::CReleaseDatePanel()
{
    Init();
}

CReleaseDatePanel::CReleaseDatePanel( wxWindow* parent, CSerialObject& object, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Object(&object)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CReleaseDatePanel creator
 */

bool CReleaseDatePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CReleaseDatePanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CReleaseDatePanel creation
    return true;
}


/*
 * CReleaseDatePanel destructor
 */

CReleaseDatePanel::~CReleaseDatePanel()
{
////@begin CReleaseDatePanel destruction
////@end CReleaseDatePanel destruction
}


/*
 * Member initialisation
 */

void CReleaseDatePanel::Init()
{
////@begin CReleaseDatePanel member initialisation
    m_ImmediateRelease = NULL;
    m_HUP = NULL;
    m_DateSizer = NULL;
////@end CReleaseDatePanel member initialisation
}


/*
 * Control creation for CReleaseDatePanel
 */

void CReleaseDatePanel::CreateControls()
{    
////@begin CReleaseDatePanel content construction
    CReleaseDatePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("When may we release your sequence record?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, 0, 0);

    m_ImmediateRelease = new wxRadioButton( itemPanel1, ID_RADIOBUTTON, _("Immediately after processing"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_ImmediateRelease->SetValue(true);
    itemBoxSizer4->Add(m_ImmediateRelease, 0, wxALIGN_LEFT|wxALL, 5);

    m_HUP = new wxRadioButton( itemPanel1, ID_RADIOBUTTON1, _("Release date:"), wxDefaultPosition, wxDefaultSize );
    m_HUP->SetValue(false);
    itemBoxSizer4->Add(m_HUP, 0, wxALIGN_LEFT|wxALL, 5);

    m_DateSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(m_DateSizer, 0, 0, 0);

////@end CReleaseDatePanel content construction
    objects::CSubmit_block * block = dynamic_cast<objects::CSubmit_block*>(m_Object);
    CRef<objects::CDate> r_date(&(block->SetReldate()));
    CRef<objects::CDate> today( new objects::CDate(CTime(CTime::eCurrent), objects::CDate::ePrecision_day) );
    m_Reldate = new CFlexibleDatePanel(itemPanel1, r_date, today->GetStd().GetYear(), 10);
    m_DateSizer->Add ( m_Reldate, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_Reldate->Enable(false);
}


bool CReleaseDatePanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow()) {
        return false;
    }
    objects::CSubmit_block * block = dynamic_cast<objects::CSubmit_block*>(m_Object);
    if (block->IsSetHup() && block->GetHup() && block->IsSetReldate()) {
        m_ImmediateRelease->SetValue (false);
        m_HUP->SetValue (true);  
        m_Reldate->Enable(true);
        m_Reldate->SetDate(block->GetReldate());
        m_Reldate->TransferDataToWindow();
    } else {
        m_ImmediateRelease->SetValue (true);
        m_HUP->SetValue (false);
        m_Reldate->Enable(false);
    }

    return true;
}


bool CReleaseDatePanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow()) {
        return false;
    }
    objects::CSubmit_block * block = dynamic_cast<objects::CSubmit_block*>(m_Object);
    if (m_HUP->GetValue()) {
        block->SetHup(true);
        m_Reldate->TransferDataFromWindow();
        CRef<objects::CDate> r_date = m_Reldate->GetDate();
        if (!r_date) {
            block->SetHup(false);
            block->ResetReldate();
        } else {
            block->SetReldate(*r_date);
        }
    } else {
        block->SetHup(false);
        block->ResetReldate();
    }

    return true;
}


string CReleaseDatePanel::GetProblems()
{
    if (!(m_HUP->GetValue() && m_Reldate->GetDate()) && !m_ImmediateRelease->GetValue()) {
        return "You must choose immediate release or select a release date!";
    } else {
        return "";
    }
}


void CReleaseDatePanel::UnselectReleaseDateChoice()
{
    m_HUP->SetValue(false);
    m_ImmediateRelease->SetValue(false);
    m_Reldate->Clear();
    m_Reldate->Enable(false);
}


/*
 * Should we show tooltips?
 */

bool CReleaseDatePanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CReleaseDatePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CReleaseDatePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CReleaseDatePanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CReleaseDatePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CReleaseDatePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CReleaseDatePanel icon retrieval
}


/*
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON
 */

void CReleaseDatePanel::OnImmediateReleaseSelected( wxCommandEvent& event )
{
    m_Reldate->Enable(false);
}


/*
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON1
 */

void CReleaseDatePanel::OnHUPSelected( wxCommandEvent& event )
{
    m_Reldate->Enable(true);
}

END_NCBI_SCOPE
