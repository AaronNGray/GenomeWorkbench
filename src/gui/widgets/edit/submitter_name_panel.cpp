/*  $Id: submitter_name_panel.cpp 43412 2019-06-26 16:45:24Z filippov $
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
 * Authors:  Andrea Asztalos
 */


#include <ncbi_pch.hpp>
#include <objects/submit/Contact_info.hpp>
#include <objects/biblio/Affil.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>
#include <gui/objutils/macro_fn_pubfields.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/edit/singleauthor_panel.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/edit/submitter_name_panel.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/cshelp.h>
#include <wx/icon.h>
#include <wx/utils.h> 
#include <wx/filedlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CSubmitterNamePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS(CSubmitterNamePanel, wxPanel )


/*
 * CSubmitterNamePanel event table definition
 */

BEGIN_EVENT_TABLE(CSubmitterNamePanel, wxPanel )
    EVT_TEXT(ID_SUBMITTER_PRIMEMAIL, CSubmitterNamePanel::OnPrimeEmailChanged)
END_EVENT_TABLE()


/*
 * CSubmitterNamePanel constructors
 */

CSubmitterNamePanel::CSubmitterNamePanel()
{
    Init();
}

CSubmitterNamePanel::CSubmitterNamePanel( wxWindow* parent, CContact_info& contact_info, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Contact(&contact_info)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CSubmitterNamePanel creator
 */

bool CSubmitterNamePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSubmitterNamePanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSubmitterNamePanel creation
    return true;
}


/*
 * CSubmitterNamePanel destructor
 */

CSubmitterNamePanel::~CSubmitterNamePanel()
{
////@begin CSubmitterNamePanel destruction
////@end CSubmitterNamePanel destruction
}


/*
 * Member initialisation
 */

void CSubmitterNamePanel::Init()
{
////@begin CSubmitterNamePanel member initialisation
    m_SingleAuthor = NULL;
    m_EmailPrimary = NULL;
    m_EmailSecondary = NULL;
////@end CSubmitterNamePanel member initialisation
}


/*
 * Control creation for CSubmitterNamePanel
 */

void CSubmitterNamePanel::CreateControls()
{    
////@begin CSubmitterNamePanel content construction
    CSubmitterNamePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer1);

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    itemBoxSizer1->Add(itemFlexGridSizer2, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer2->Add(itemBoxSizer9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("First Name"), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE);
    itemBoxSizer9->Add(itemStaticText10, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemPanel1, wxID_STATIC, _("M.I."), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE);
    itemBoxSizer9->Add(itemStaticText11, 0, wxALIGN_LEFT |wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel1, wxID_STATIC, _("Last Name"), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE );
    itemBoxSizer9->Add(itemStaticText12, 0, wxALIGN_LEFT |wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemPanel1, wxID_STATIC, _("Suffix"), wxDefaultPosition, wxSize(50, -1), wxALIGN_CENTRE );
    itemBoxSizer9->Add(itemStaticText13, 0, wxALIGN_LEFT |wxALIGN_CENTER_VERTICAL|wxALL, 5);


    m_SingleAuthor = new CSingleAuthorPanel(itemPanel1, m_Contact->SetContact());
    m_SingleAuthor->m_FirstNameCtrl->SetMinSize(wxSize(120, -1));
    m_SingleAuthor->m_LastNameCtrl->SetMinSize(wxSize(120, -1));
    m_SingleAuthor->HideNonTextCtrls();
    itemFlexGridSizer2->Add(m_SingleAuthor, 0, wxALIGN_CENTER_VERTICAL |wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL |wxTOP, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Email (primary)*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    m_LabelSecondary = new wxStaticText( itemPanel1, wxID_STATIC, _("Email (secondary)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(m_LabelSecondary, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL| wxRIGHT | wxTOP | wxBOTTOM, 5);
    m_LabelSecondary->Disable();

    m_EmailPrimary = new wxTextCtrl( itemPanel1, ID_SUBMITTER_PRIMEMAIL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer3->Add(m_EmailPrimary, 0, wxALIGN_CENTER_VERTICAL| wxRIGHT | wxTOP | wxBOTTOM, 5);
    m_EmailPrimary->SetToolTip(wxEmptyString);
    
    m_EmailSecondary = new wxTextCtrl( itemPanel1, ID_SUBMITTER_SECEMAIL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer3->Add(m_EmailSecondary, 0, wxALIGN_CENTER_VERTICAL| wxRIGHT | wxTOP | wxBOTTOM, 5);
    m_EmailSecondary->Disable();    
    m_EmailSecondary->SetToolTip(wxEmptyString);
    
////@end CSubmitterNamePanel content construction
}


bool CSubmitterNamePanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow()) {
        return false;
    }
    if (m_Contact->IsSetContact()) {
        const auto& contact = m_Contact->GetContact();   
        if (CSingleAuthorPanel::IsPlaceholder(contact)) {
            CRef<CAuthor> blank(new CAuthor());
            m_SingleAuthor->SetAuthor(*blank);
        } else {        
            m_SingleAuthor->SetAuthor(contact);
        }

        if ((contact.IsSetAffil() && contact.GetAffil().IsStd()
            && contact.GetAffil().GetStd().IsSetEmail()) || m_Contact->IsSetEmail()) {
            vector<string> emails;
            if (m_Contact->IsSetEmail()) {
                NStr::Split(m_Contact->GetEmail(), ",;", emails);
            } else {
                NStr::Split(contact.GetAffil().GetStd().GetEmail(), ",;", emails);
            }
            if (!emails.empty()) {
                m_EmailPrimary->SetValue(ToWxString(emails[0]));
            }
            if (emails.size() > 1) {
                m_EmailSecondary->Enable();
                m_LabelSecondary->Enable();
                m_EmailSecondary->SetValue(ToWxString(emails[1]));
            }
        }
    }

    return true;
}

bool CSubmitterNamePanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow()) {
        return false;
    }

    m_SingleAuthor->TransferDataFromWindow();
    CRef<CAuthor> author = m_SingleAuthor->GetAuthor();
    if (!author) {
        author.Reset(new CAuthor());
    }
    if (!author->IsSetName() ||
        (author->GetName().IsName() && !author->GetName().GetName().IsSetLast())) {
        author->SetName().SetName().SetLast("?");
    }
    if (author)
        m_Contact->SetContact().Assign(*author);
    string email;
    if (!m_EmailPrimary->IsEmpty()) { 
        email += ToStdString(m_EmailPrimary->GetValue());        
    }    
    if (!m_EmailSecondary->IsEmpty()) {
        if (!email.empty()) {
            email += "; ";
        }
        email += ToStdString(m_EmailSecondary->GetValue());
    }
    if (!email.empty()) {
        m_Contact->SetEmail(email);
    }
    return true;
}

/*
 * Should we show tooltips?
 */

bool CSubmitterNamePanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSubmitterNamePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSubmitterNamePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSubmitterNamePanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSubmitterNamePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSubmitterNamePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSubmitterNamePanel icon retrieval
}

void CSubmitterNamePanel::ApplyContactInfo(CContact_info& contact_info)
{
    m_Contact.Reset(&contact_info);
    TransferDataToWindow();
}

void CSubmitterNamePanel::OnPrimeEmailChanged( wxCommandEvent& event )
{
    bool enable = m_EmailPrimary->GetValue().EndsWith(_("@qq.com")) || !m_EmailSecondary->IsEmpty();    
    m_EmailSecondary->Enable(enable);
    m_LabelSecondary->Enable(enable);
}

void CSubmitterNamePanel::ApplyCommand()
{
    if (!TransferDataFromWindow())
        return;
    GetParentWizard(this)->ApplySubmitCommand();    
}

void CSubmitterNamePanel::ReportMissingFields(string &text)
{
    m_SingleAuthor->TransferDataFromWindow();
    CRef<CAuthor> author = m_SingleAuthor->GetAuthor();
    if (!author || !author->IsSetName() || !author->GetName().IsName() || !author->GetName().GetName().IsSetFirst() 
        || author->GetName().GetName().GetFirst().empty() || author->GetName().GetName().GetFirst() == "?")
        text += "Submitter First Name\n";
    if (!author || !author->IsSetName() || !author->GetName().IsName() || !author->GetName().GetName().IsSetLast() 
        || author->GetName().GetName().GetLast().empty() || author->GetName().GetName().GetLast() == "?")
        text += "Submitter Last Name\n";   
    if (m_EmailPrimary->IsEmpty()) 
        text += "Submitter Primary Email Address\n";
}

END_NCBI_SCOPE


