/*  $Id: authorized_access_panel.cpp 31212 2014-09-12 12:45:01Z bollin $
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

#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>
#include <gui/widgets/edit/authorized_access_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/stattext.h>
#include <wx/sizer.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CAuthorizedAccessPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAuthorizedAccessPanel, wxPanel )


/*!
 * CAuthorizedAccessPanel event table definition
 */

BEGIN_EVENT_TABLE( CAuthorizedAccessPanel, wxPanel )

////@begin CAuthorizedAccessPanel event table entries
////@end CAuthorizedAccessPanel event table entries

END_EVENT_TABLE()


/*!
 * CAuthorizedAccessPanel constructors
 */

CAuthorizedAccessPanel::CAuthorizedAccessPanel()
{
    Init();
}

CAuthorizedAccessPanel::CAuthorizedAccessPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CAuthorizedAccessPanel creator
 */

bool CAuthorizedAccessPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAuthorizedAccessPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAuthorizedAccessPanel creation
    return true;
}


/*!
 * CAuthorizedAccessPanel destructor
 */

CAuthorizedAccessPanel::~CAuthorizedAccessPanel()
{
////@begin CAuthorizedAccessPanel destruction
////@end CAuthorizedAccessPanel destruction
}


/*!
 * Member initialisation
 */

void CAuthorizedAccessPanel::Init()
{
////@begin CAuthorizedAccessPanel member initialisation
    m_StudyTxt = NULL;
////@end CAuthorizedAccessPanel member initialisation
}


/*!
 * Control creation for CAuthorizedAccessPanel
 */

void CAuthorizedAccessPanel::CreateControls()
{    
////@begin CAuthorizedAccessPanel content construction
    CAuthorizedAccessPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Study"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_StudyTxt = new wxTextCtrl( itemPanel1, ID_TEXTCTRL22, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(m_StudyTxt, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CAuthorizedAccessPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CAuthorizedAccessPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAuthorizedAccessPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAuthorizedAccessPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAuthorizedAccessPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAuthorizedAccessPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAuthorizedAccessPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAuthorizedAccessPanel icon retrieval
}

static const string kStudy = "Study";

void CAuthorizedAccessPanel::SetUser_object(CRef<CUser_object> user)
{
    if (user) {
        m_User.Reset(new CUser_object());
        m_User->Assign(*user);
    } else {
        m_User.Reset(NULL);
    }
    
    string study = "";
    
    if (m_User) {
        // search for fields in user object
        ITERATE(CUser_object::TData, it, m_User->GetData()) {
            if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr()) {
                if (NStr::EqualNocase((*it)->GetLabel().GetStr(), kStudy)) {
                    if ((*it)->IsSetData() && (*it)->GetData().IsStr()) {
                        study = (*it)->GetData().GetStr();
                    }
                }
            }
        }
    }    

    m_StudyTxt->SetValue(ToWxString(study));
}


CRef<CUser_object> CAuthorizedAccessPanel::GetUser_object()
{
    CRef<CUser_object> user(new CUser_object());
    
    if (m_User) {
        user->Assign(*m_User);
    }

    user->SetType().SetStr("AuthorizedAccess");

    // remove previous fields
    // remove previous fields
    if (user->IsSetData()) {
        CUser_object::TData::iterator it = user->SetData().begin();
        while (it != user->SetData().end()) {
            if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr() && NStr::EqualNocase((*it)->GetLabel().GetStr(), kStudy)) {
                it = user->SetData().erase(it);
            } else {
                ++it;
            }
        }
    }
    string study = ToStdString(m_StudyTxt->GetValue());
    if (!NStr::IsBlank(study)) {
        CRef<CUser_field> new_field(new CUser_field());
        new_field->SetLabel().SetStr(kStudy);
        new_field->SetData().SetStr(study);
        user->SetData().push_back(new_field);
    }
        
    return user;
}


void CAuthorizedAccessPanel::ChangeSeqdesc(const objects::CSeqdesc& desc)
{
    CRef<CUser_object> tmp(new CUser_object());
    tmp->Assign(desc.GetUser());
    SetUser_object(tmp);
    TransferDataToWindow();
}


void CAuthorizedAccessPanel::UpdateSeqdesc(objects::CSeqdesc& desc)
{
    TransferDataFromWindow();
    CRef<CUser_object> user = GetUser_object();
    desc.SetUser(*user);
}


END_NCBI_SCOPE

