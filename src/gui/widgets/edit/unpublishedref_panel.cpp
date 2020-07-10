/*  $Id: unpublishedref_panel.cpp 43202 2019-05-28 18:05:59Z filippov $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software / database is a "United States Government Work" under the
*  terms of the United States Copyright Act.It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.This software / database is freely available
*  to the public for use.The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data.The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors: Andrea Asztalos
*
*/

#include <ncbi_pch.hpp>
#include <gui/widgets/edit/unpublishedref_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE

/*
 * CUnpublishedRefPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CUnpublishedRefPanel, wxPanel )


/*
 * CUnpublishedRefPanel event table definition
 */

BEGIN_EVENT_TABLE( CUnpublishedRefPanel, wxPanel )

////@begin CUnpublishedRefPanel event table entries
////@end CUnpublishedRefPanel event table entries

END_EVENT_TABLE()


/*
 * CUnpublishedRefPanel constructors
 */

CUnpublishedRefPanel::CUnpublishedRefPanel()
{
    Init();
}

CUnpublishedRefPanel::CUnpublishedRefPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CUnpublishedRefPanel creator
 */

bool CUnpublishedRefPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CUnpublishedRefPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CUnpublishedRefPanel creation
    return true;
}


/*
 * CUnpublishedRefPanel destructor
 */

CUnpublishedRefPanel::~CUnpublishedRefPanel()
{
////@begin CUnpublishedRefPanel destruction
////@end CUnpublishedRefPanel destruction
}


/*
 * Member initialisation
 */

void CUnpublishedRefPanel::Init()
{
////@begin CUnpublishedRefPanel member initialisation
    m_ReferenceTitle = NULL;
////@end CUnpublishedRefPanel member initialisation
}


/*
 * Control creation for CUnpublishedRefPanel
 */

void CUnpublishedRefPanel::CreateControls()
{    
////@begin CUnpublishedRefPanel content construction
    CUnpublishedRefPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText1 = new wxStaticText( itemPanel1, wxID_STATIC, _("Reference title*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText1, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP, 5);

    m_ReferenceTitle = new wxTextCtrl( itemPanel1, ID_REFTITLETXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemBoxSizer2->Add(m_ReferenceTitle, 0, wxALIGN_LEFT|wxALL, 5);

////@end CUnpublishedRefPanel content construction
}


/*
 * Should we show tooltips?
 */

bool CUnpublishedRefPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CUnpublishedRefPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CUnpublishedRefPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CUnpublishedRefPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CUnpublishedRefPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CUnpublishedRefPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CUnpublishedRefPanel icon retrieval
}


void CUnpublishedRefPanel::SetCitGen(const objects::CCit_gen& gen)
{
    if (gen.IsSetTitle()) {
        m_ReferenceTitle->SetValue(ToWxString(gen.GetTitle()));
    } else {
        m_ReferenceTitle->SetValue(wxT(""));
    }
}


void CUnpublishedRefPanel::UpdateCitGen(objects::CCit_gen& gen)
{
    if (m_ReferenceTitle->IsEmpty()) {
        gen.ResetTitle();
    } else {
        gen.SetTitle(ToStdString(m_ReferenceTitle->GetValue()));
    }
}

void CUnpublishedRefPanel::ReportMissingFields(string &text)
{
    if (m_ReferenceTitle->GetValue().IsEmpty())
        text += "Reference Title\n";
}

END_NCBI_SCOPE
