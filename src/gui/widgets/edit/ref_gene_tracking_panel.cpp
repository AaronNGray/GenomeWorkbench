/*  $Id: ref_gene_tracking_panel.cpp 40132 2017-12-22 15:45:12Z bollin $
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

#include <gui/widgets/edit/ref_gene_tracking_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <objects/general/Object_id.hpp>

#include <wx/stattext.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CRefGeneTrackingPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CRefGeneTrackingPanel, wxPanel )


/*!
 * CRefGeneTrackingPanel event table definition
 */

BEGIN_EVENT_TABLE( CRefGeneTrackingPanel, wxPanel )

////@begin CRefGeneTrackingPanel event table entries
    EVT_TEXT( ID_TEXTCTRL9, CRefGeneTrackingPanel::OnGenomicSourceTextUpdated )

////@end CRefGeneTrackingPanel event table entries

END_EVENT_TABLE()


/*!
 * CRefGeneTrackingPanel constructors
 */

CRefGeneTrackingPanel::CRefGeneTrackingPanel()
{
    Init();
}

CRefGeneTrackingPanel::CRefGeneTrackingPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CRefGeneTrackingPanel creator
 */

bool CRefGeneTrackingPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRefGeneTrackingPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRefGeneTrackingPanel creation
    return true;
}


/*!
 * CRefGeneTrackingPanel destructor
 */

CRefGeneTrackingPanel::~CRefGeneTrackingPanel()
{
////@begin CRefGeneTrackingPanel destruction
////@end CRefGeneTrackingPanel destruction
}


/*!
 * Member initialisation
 */

void CRefGeneTrackingPanel::Init()
{
////@begin CRefGeneTrackingPanel member initialisation
    m_StatusSizer = NULL;
    m_InferredBtn = NULL;
    m_PredictedBtn = NULL;
    m_ProvisionalBtn = NULL;
    m_ValidatedBtn = NULL;
    m_ReviewedBtn = NULL;
    m_ModelBtn = NULL;
    m_WgsBtn = NULL;
    m_PipelineBtn = NULL;
    m_GeneratedBtn = NULL;
    m_CuratorTxt = NULL;
    m_GenomicSourceTxt = NULL;
    m_URLTxt = NULL;
    m_AccessionsPanel = NULL;
////@end CRefGeneTrackingPanel member initialisation
    m_User.Reset(NULL);
}


/*!
 * Control creation for CRefGeneTrackingPanel
 */

void CRefGeneTrackingPanel::CreateControls()
{    
////@begin CRefGeneTrackingPanel content construction
    CRefGeneTrackingPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_StatusSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(m_StatusSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_InferredBtn = new wxRadioButton( itemPanel1, ID_RADIOBUTTON, _("Inferred"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_InferredBtn->SetValue(false);
    m_StatusSizer->Add(m_InferredBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_PredictedBtn = new wxRadioButton( itemPanel1, ID_RADIOBUTTON1, _("Predicted"), wxDefaultPosition, wxDefaultSize, 0 );
    m_PredictedBtn->SetValue(false);
    m_StatusSizer->Add(m_PredictedBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProvisionalBtn = new wxRadioButton( itemPanel1, ID_RADIOBUTTON2, _("Provisional"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ProvisionalBtn->SetValue(false);
    m_StatusSizer->Add(m_ProvisionalBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ValidatedBtn = new wxRadioButton( itemPanel1, ID_RADIOBUTTON3, _("Validated"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ValidatedBtn->SetValue(false);
    m_StatusSizer->Add(m_ValidatedBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ReviewedBtn = new wxRadioButton( itemPanel1, ID_RADIOBUTTON4, _("Reviewed"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ReviewedBtn->SetValue(false);
    m_StatusSizer->Add(m_ReviewedBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ModelBtn = new wxRadioButton( itemPanel1, ID_RADIOBUTTON5, _("Model"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ModelBtn->SetValue(false);
    m_StatusSizer->Add(m_ModelBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_WgsBtn = new wxRadioButton( itemPanel1, ID_RADIOBUTTON6, _("WGS"), wxDefaultPosition, wxDefaultSize, 0 );
    m_WgsBtn->SetValue(false);
    m_StatusSizer->Add(m_WgsBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_PipelineBtn = new wxRadioButton( itemPanel1, ID_RADIOBUTTON7, _("Pipeline"), wxDefaultPosition, wxDefaultSize, 0 );
    m_PipelineBtn->SetValue(false);
    m_PipelineBtn->Enable(false);
    m_StatusSizer->Add(m_PipelineBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer12 = new wxFlexGridSizer(2, 5, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_GeneratedBtn = new wxCheckBox( itemPanel1, ID_CHECKBOX16, _("Generated"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GeneratedBtn->SetValue(false);
    itemFlexGridSizer12->Add(m_GeneratedBtn, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText14 = new wxStaticText( itemPanel1, wxID_STATIC, _("Curator"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer12->Add(itemStaticText14, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CuratorTxt = new wxTextCtrl( itemPanel1, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer12->Add(m_CuratorTxt, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText16 = new wxStaticText( itemPanel1, wxID_STATIC, _("Genomic Source"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer12->Add(itemStaticText16, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GenomicSourceTxt = new wxTextCtrl( itemPanel1, ID_TEXTCTRL9, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer12->Add(m_GenomicSourceTxt, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer12->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText19 = new wxStaticText( itemPanel1, wxID_STATIC, _("URL"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer12->Add(itemStaticText19, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_URLTxt = new wxTextCtrl( itemPanel1, ID_TEXTCTRL21, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer12->Add(m_URLTxt, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccessionsPanel = new CRefGeneTrackLocListPanel( itemPanel1, ID_FOREIGN, wxDefaultPosition, wxSize(100, 100), wxSIMPLE_BORDER );
    itemBoxSizer2->Add(m_AccessionsPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CRefGeneTrackingPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CRefGeneTrackingPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRefGeneTrackingPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRefGeneTrackingPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRefGeneTrackingPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CRefGeneTrackingPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRefGeneTrackingPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRefGeneTrackingPanel icon retrieval
}


void CRefGeneTrackingPanel::x_SetStatus(const string& val)
{
    size_t status_items = m_StatusSizer->GetItemCount();
    for (size_t pos = 0; pos < status_items; pos++) {
        wxRadioButton *but = dynamic_cast<wxRadioButton*>(m_StatusSizer->GetItem(pos)->GetWindow());
        if (but) {
            if (NStr::EqualNocase(ToStdString(but->GetLabel()), val)) {
                but->SetValue(true);
            } else {
                but->SetValue(false);
            }
        }
    }
    if (NStr::EqualNocase(val, "Pipeline")) {
        m_PipelineBtn->Enable(true);
    } else {
        m_PipelineBtn->Enable(false);
    }
}


string CRefGeneTrackingPanel::x_GetStatus()
{
    string status = "";
    size_t status_items = m_StatusSizer->GetItemCount();
    for (size_t pos = 0; pos < status_items; pos++) {
        wxRadioButton *but = dynamic_cast<wxRadioButton*>(m_StatusSizer->GetItem(pos)->GetWindow());
        if (but && but->GetValue()) {
            status = ToStdString(but->GetLabel());
            break;
        }
    }
    return status;
}


static const string kStatus = "Status";
static const string kGenomicSource = "GenomicSource";
static const string kGenerated = "Generated";
static const string kCollaborator = "Collaborator";
static const string kURL = "CollaboratorURL";

void CRefGeneTrackingPanel::SetUser_object(CRef<CUser_object> user)
{
    if (user) {
        m_User.Reset(new CUser_object());
        m_User->Assign(*user);
    } else {
        m_User.Reset(NULL);
    }
    string status = "";
    string genomic_source = "";
    string collaborator = "";
    string url = "";
    bool generated = false;

    if (m_User) {
        // search for fields in user object
        ITERATE(CUser_object::TData, it, m_User->GetData()) {
            if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr()) {
                if (NStr::EqualNocase((*it)->GetLabel().GetStr(), kStatus)) {
                    if ((*it)->IsSetData() && (*it)->GetData().IsStr()) {
                        status = (*it)->GetData().GetStr();
                    }
                } else if (NStr::EqualNocase((*it)->GetLabel().GetStr(), kGenomicSource)) {
                    if ((*it)->IsSetData() && (*it)->GetData().IsStr()) {
                        genomic_source = (*it)->GetData().GetStr();
                    }
                } else if (NStr::EqualNocase((*it)->GetLabel().GetStr(), kGenerated)) {
                    if ((*it)->IsSetData() && (*it)->GetData().IsBool() && (*it)->GetData().GetBool()) {
                        generated = true;
                    }
                } else if (NStr::EqualNocase((*it)->GetLabel().GetStr(), kCollaborator)) {
                    if ((*it)->IsSetData() && (*it)->GetData().IsStr()) {
                        collaborator = (*it)->GetData().GetStr();
                    }
                } else if (NStr::EqualNocase((*it)->GetLabel().GetStr(), kURL)) {
                    if ((*it)->IsSetData() && (*it)->GetData().IsStr()) {
                        url = (*it)->GetData().GetStr();
                    }
                }
            }
        }
    }
    x_SetStatus (status);
    m_GenomicSourceTxt->SetValue(ToWxString(genomic_source));
    m_CuratorTxt->SetValue(ToWxString(collaborator));
    m_URLTxt->SetValue(ToWxString(url));
    m_GeneratedBtn->SetValue(generated);
    m_AccessionsPanel->SetUser_object(m_User);
}


bool CRefGeneTrackingPanel::x_IsControlledField(const string& field)
{
    if (NStr::EqualNocase(field, kStatus)
        || NStr::EqualNocase(field, kGenomicSource)
        || NStr::EqualNocase(field, kGenerated)
        || NStr::EqualNocase(field, kCollaborator)
        || NStr::EqualNocase(field, kURL)){
        return true;
    }
    return false;
}


CRef<CUser_object> CRefGeneTrackingPanel::GetUser_object()
{
    TransferDataFromWindow();
    CRef<CUser_object> user(new CUser_object());
    
    if (m_User) {
        user->Assign(*m_User);
    } else {
        user->SetType().SetStr("RefGeneTracking");
    }

    // remove previous fields
    if (user->IsSetData()) {
        CUser_object::TData::iterator it = user->SetData().begin();
        while (it != user->SetData().end()) {
            if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr() && x_IsControlledField((*it)->GetLabel().GetStr())) {
                it = user->SetData().erase(it);
            } else {
                ++it;
            }
        }
    }
    string status = x_GetStatus();
    if (!NStr::IsBlank(status)) {
        CRef<CUser_field> new_field(new CUser_field());
        new_field->SetLabel().SetStr(kStatus);
        new_field->SetData().SetStr(status);
        user->SetData().push_back(new_field);
    }
    string genomic_source = ToStdString(m_GenomicSourceTxt->GetValue());
    if (!NStr::IsBlank(genomic_source)) {
        CRef<CUser_field> new_field(new CUser_field());
        new_field->SetLabel().SetStr(kGenomicSource);
        new_field->SetData().SetStr(genomic_source);
        user->SetData().push_back(new_field);
    }

    bool generated = m_GeneratedBtn->GetValue();
    if (generated) {
        CRef<CUser_field> new_field(new CUser_field());
        new_field->SetLabel().SetStr(kGenerated);
        new_field->SetData().SetBool(true);
        user->SetData().push_back(new_field);
    }
    string collaborator = ToStdString(m_CuratorTxt->GetValue());
    if (!NStr::IsBlank(collaborator)) {
        CRef<CUser_field> new_field(new CUser_field());
        new_field->SetLabel().SetStr(kCollaborator);
        new_field->SetData().SetStr(collaborator);
        user->SetData().push_back(new_field);
    }
    string url = ToStdString(m_URLTxt->GetValue());
    if (!NStr::IsBlank(url)) {
        CRef<CUser_field> new_field(new CUser_field());
        new_field->SetLabel().SetStr(kURL);
        new_field->SetData().SetStr(url);
        user->SetData().push_back(new_field);
    }

    m_AccessionsPanel->TransferDataFromWindow();
    m_AccessionsPanel->PopulateFields(*user);
    return user;
}


void CRefGeneTrackingPanel::ChangeSeqdesc(const objects::CSeqdesc& desc)
{
    CRef<CUser_object> tmp(new CUser_object());
    tmp->Assign(desc.GetUser());
    SetUser_object(tmp);
    m_AccessionsPanel->TransferDataToWindow();
    TransferDataToWindow();
}


void CRefGeneTrackingPanel::UpdateSeqdesc(objects::CSeqdesc& desc)
{
    TransferDataFromWindow();
    CRef<CUser_object> user = GetUser_object();
    desc.SetUser(*user);
}


/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_TEXTCTRL9
 */

void CRefGeneTrackingPanel::OnGenomicSourceTextUpdated( wxCommandEvent& event )
{
    if (m_GenomicSourceTxt->IsEmpty() || !m_AccessionsPanel->IsEmpty()) {
        m_AccessionsPanel->Enable(true);
    } else {
        m_AccessionsPanel->Enable(false);
    }
}

END_NCBI_SCOPE
