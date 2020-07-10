/*  $Id: unverified_panel.cpp 40461 2018-02-20 18:48:16Z filippov $
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
////@begin includes
////@end includes

#include <gui/widgets/edit/unverified_panel.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>

////@begin XPM images
////@end XPM images

#include <wx/sizer.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CUnverifiedPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CUnverifiedPanel, wxPanel )


/*!
 * CUnverifiedPanel event table definition
 */

BEGIN_EVENT_TABLE( CUnverifiedPanel, wxPanel )

////@begin CUnverifiedPanel event table entries
////@end CUnverifiedPanel event table entries

END_EVENT_TABLE()


/*!
 * CUnverifiedPanel constructors
 */

CUnverifiedPanel::CUnverifiedPanel()
{
    Init();
}

CUnverifiedPanel::CUnverifiedPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CUnverifiedPanel creator
 */

bool CUnverifiedPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CUnverifiedPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CUnverifiedPanel creation
    return true;
}


/*!
 * CUnverifiedPanel destructor
 */

CUnverifiedPanel::~CUnverifiedPanel()
{
////@begin CUnverifiedPanel destruction
////@end CUnverifiedPanel destruction
}


/*!
 * Member initialisation
 */

void CUnverifiedPanel::Init()
{
////@begin CUnverifiedPanel member initialisation
    m_Organism = NULL;
    m_Features = NULL;
    m_Misassembled = NULL;
    m_Contamination = NULL;
////@end CUnverifiedPanel member initialisation
}


/*!
 * Control creation for CUnverifiedPanel
 */

void CUnverifiedPanel::CreateControls()
{    
////@begin CUnverifiedPanel content construction
    CUnverifiedPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_Organism = new wxCheckBox( itemPanel1, ID_CHECKBOX14, _("Organism"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Organism->SetValue(false);
    itemBoxSizer2->Add(m_Organism, 0, wxALIGN_LEFT|wxALL, 5);

    m_Features = new wxCheckBox( itemPanel1, ID_CHECKBOX15, _("Features"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Features->SetValue(false);
    itemBoxSizer2->Add(m_Features, 0, wxALIGN_LEFT|wxALL, 5);

    m_Misassembled = new wxCheckBox( itemPanel1, ID_CHECKBOX16, _("Misassembled"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Misassembled->SetValue(false);
    itemBoxSizer2->Add(m_Misassembled, 0, wxALIGN_LEFT|wxALL, 5);

    m_Contamination = new wxCheckBox( itemPanel1, ID_CHECKBOX17, _("Contamination"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Contamination->SetValue(false);
    itemBoxSizer2->Add(m_Contamination, 0, wxALIGN_LEFT|wxALL, 5);
////@end CUnverifiedPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CUnverifiedPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CUnverifiedPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CUnverifiedPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CUnverifiedPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CUnverifiedPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CUnverifiedPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CUnverifiedPanel icon retrieval
}


void CUnverifiedPanel::SetUser_object(CRef<CUser_object> user)
{
    if (user) {
        m_User.Reset(new CUser_object());
        m_User->Assign(*user);
    } else {
        m_User.Reset(NULL);
    }
    bool is_organism = false;
    bool is_feature = false;
    bool is_misassembled = false;
    bool is_contamination = false;

    if (m_User) {
        // search for fields in user object
        is_feature = m_User->IsUnverifiedFeature();
        is_organism = m_User->IsUnverifiedOrganism();
        is_misassembled = m_User->IsUnverifiedMisassembled();
	is_contamination = m_User->IsUnverifiedContaminant();
    }
    m_Organism->SetValue(is_organism);
    m_Features->SetValue(is_feature);
    m_Misassembled->SetValue(is_misassembled);
    m_Contamination->SetValue(is_contamination);
}


CRef<CUser_object> CUnverifiedPanel::GetUser_object()
{
    CRef<CUser_object> user(new CUser_object());
    
    if (m_User) {
        user->Assign(*m_User);
    } else {
        user->SetObjectType(CUser_object::eObjectType_Unverified);
    }
    
    // remove previous Type fields
    user->RemoveUnverifiedFeature();
    user->RemoveUnverifiedOrganism();
    user->RemoveUnverifiedMisassembled();
    user->RemoveUnverifiedContaminant();

    // add fields based on checkboxes
    if (m_Organism->GetValue()) {
        user->AddUnverifiedOrganism();
    }
    
    if (m_Features->GetValue()) {
        user->AddUnverifiedFeature();
    }

    if (m_Misassembled->GetValue()) {
        user->AddUnverifiedMisassembled();
    }

    if (m_Contamination->GetValue()) {
        user->AddUnverifiedContaminant();
    }

    return user;
}


void CUnverifiedPanel::ChangeSeqdesc(const objects::CSeqdesc& desc)
{
    CRef<CUser_object> tmp(new CUser_object());
    tmp->Assign(desc.GetUser());
    SetUser_object(tmp);
    TransferDataToWindow();
}


void CUnverifiedPanel::UpdateSeqdesc(objects::CSeqdesc& desc)
{
    TransferDataFromWindow();
    CRef<CUser_object> user = GetUser_object();
    desc.SetUser(*user);
}


END_NCBI_SCOPE
