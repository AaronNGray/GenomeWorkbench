/*  $Id: general_panel.cpp 43053 2019-05-09 18:23:44Z filippov $
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
#include <gui/widgets/edit/general_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <objtools/edit/dblink_field.hpp>
#include <objects/general/Date.hpp>
#include <objects/general/Date_std.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <objects/general/User_object.hpp>

#include <wx/datectrl.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/radiobut.h>
#include <wx/textctrl.h>
#include <wx/datetime.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE
/*
 * CGeneralPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CGeneralPanel, wxPanel )


/*
 * CGeneralPanel event table definition
 */

BEGIN_EVENT_TABLE( CGeneralPanel, wxPanel )

////@begin CGeneralPanel event table entries
    EVT_RADIOBUTTON( ID_SUBMITTER_IMMEDIATE, CGeneralPanel::OnSubmitterImmediateSelected )
    EVT_RADIOBUTTON( ID_SUBMITTER_HUP, CGeneralPanel::OnSubmitterHupSelected )
////@end CGeneralPanel event table entries

END_EVENT_TABLE()


/*
 * CGeneralPanel constructors
 */

CGeneralPanel::CGeneralPanel()
{
    Init();
}

CGeneralPanel::CGeneralPanel( wxWindow* parent, ICommandProccessor* cmdproc, objects::CSeq_entry_Handle seh,wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: m_CmdProcessor(cmdproc), m_Seh(seh)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CGeneralPanel creator
 */

bool CGeneralPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGeneralPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGeneralPanel creation
    return true;
}


/*
 * CGeneralPanel destructor
 */

CGeneralPanel::~CGeneralPanel()
{
////@begin CGeneralPanel destruction
////@end CGeneralPanel destruction
}


/*
 * Member initialisation
 */

void CGeneralPanel::Init()
{
////@begin CGeneralPanel member initialisation
    m_Bioproject = NULL;
    m_Biosample = NULL;
    m_ImmediateRelease = NULL;
    m_HUP = NULL;
    m_Label = NULL;
    m_Date = NULL;
////@end CGeneralPanel member initialisation
}


/*
 * Control creation for CGeneralPanel
 */

void CGeneralPanel::CreateControls()
{    
////@begin CGeneralPanel content construction
    CGeneralPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText2 = new wxStaticText( itemPanel1, wxID_STATIC, _("BioProject"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("BioSample"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Bioproject = new wxTextCtrl( itemPanel1, ID_SUBMITTER_BIOPROJECT, wxEmptyString, wxDefaultPosition, wxSize(150, -1), 0 );
    itemFlexGridSizer1->Add(m_Bioproject, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Biosample = new wxTextCtrl( itemPanel1, ID_SUBMITTER_BIOSAMPLE, wxEmptyString, wxDefaultPosition, wxSize(150, -1), 0 );
    itemFlexGridSizer1->Add(m_Biosample, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer1Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Release date"));
    wxStaticBoxSizer* itemStaticBoxSizer1 = new wxStaticBoxSizer(itemStaticBoxSizer1Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText1 = new wxStaticText( itemPanel1, wxID_STATIC, _("When should this submission be released to the public?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer1->Add(itemStaticText1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemStaticBoxSizer1->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ImmediateRelease = new wxRadioButton( itemPanel1, ID_SUBMITTER_IMMEDIATE, _("Immediately after processing"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_ImmediateRelease->SetValue(true);
    itemBoxSizer3->Add(m_ImmediateRelease, 0, wxALIGN_LEFT|wxALL, 5);

    m_HUP = new wxRadioButton( itemPanel1, ID_SUBMITTER_HUP, _("On specified date or upon publication, whichever is first:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_HUP->SetValue(false);
    itemBoxSizer3->Add(m_HUP, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Label = new wxStaticText( itemPanel1, wxID_STATIC, _("Release date*"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Label->Enable(false);
    itemBoxSizer3->Add(m_Label, 0, wxALIGN_LEFT|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    m_Date = new wxDatePickerCtrl( itemPanel1, ID_DATEPICKERCTRL, wxDateTime(), wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    m_Date->Enable(false);
    itemBoxSizer3->Add(m_Date, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Note: Please inform GenBank when the accession number or any portion\nof the sequence is published, as published data must be released."), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer1->Add(itemStaticText8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CGeneralPanel content construction
}

/*
 * Should we show tooltips?
 */

bool CGeneralPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CGeneralPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGeneralPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGeneralPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CGeneralPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGeneralPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGeneralPanel icon retrieval
}


/*
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_SUBMITTER_HUP
 */

void CGeneralPanel::OnSubmitterHupSelected( wxCommandEvent& event )
{
    m_Label->Enable(true);
    m_Date->Enable(true);
}

/*
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_SUBMITTER_IMMEDIATE
 */

void CGeneralPanel::OnSubmitterImmediateSelected( wxCommandEvent& event )
{
    m_Label->Enable(false);
    m_Date->Enable(false);
}

void CGeneralPanel::ApplySubmitBlock(objects::CSubmit_block& block)
{
    m_Block.Reset(&block);
    x_UpdateBlockControls();

}

void CGeneralPanel::ApplyDescriptor(objects::CSeqdesc& desc)
{
    if (desc.IsUser() && desc.GetUser().GetObjectType() == objects::CUser_object::eObjectType_DBLink) {
        m_Dblink.Reset(&desc);
        x_UpdateDescControls();
    }
}


void CGeneralPanel::x_Reset()
{
    m_ImmediateRelease->SetValue(true);
    m_HUP->SetValue(false);
    m_Date->Enable(false);
    m_Bioproject->SetValue(wxT(""));
    m_Biosample->SetValue(wxT(""));
}


void CGeneralPanel::x_UpdateBlockControls()
{
    if (m_Block && m_Block->IsSetHup() && m_Block->GetHup()) {
        m_HUP->SetValue(true);
        m_ImmediateRelease->SetValue(false);
        m_Date->Enable(true);
        if (m_Block->IsSetReldate() && m_Block->GetReldate().IsStd() &&
            m_Block->GetReldate().GetStd().IsSetYear()) {
            const auto& reldate = m_Block->GetReldate().GetStd();
            int year = 1900;
            int  month = 1;
            int day = 1;
            if (reldate.IsSetYear())
                year = reldate.GetYear();
            if (reldate.IsSetMonth())
                month = reldate.GetMonth();
            if (reldate.IsSetDay())
                day = reldate.GetDay();
            wxDateTime dmy(day, wxDateTime::Month(wxDateTime::Jan + month - 1), year, 0, 0, 0, 0);
            m_Date->SetValue(dmy);
        }
    }
}


void CGeneralPanel::x_UpdateDescControls()
{
    if (m_Dblink) {
        const objects::CUser_object& user = m_Dblink->GetUser();
        vector<string> bioprojects = objects::edit::CDBLink::GetBioProject(user);
        string project_val = NStr::Join(bioprojects, ";");
        m_Bioproject->SetValue(ToWxString(project_val));

        vector<string> biosamples = objects::edit::CDBLink::GetBioSample(user);
        string biosample_val = NStr::Join(biosamples, ";");
        m_Biosample->SetValue(ToWxString(biosample_val));
    }

}


bool CGeneralPanel::TransferDataToWindow()
{
    x_Reset();

    x_UpdateBlockControls();

    x_UpdateDescControls();
    return true;
}


void SetField(objects::CUser_object& user, const string& field_name, const string& val)
{
    vector<string> val_list;
    NStr::Split(val, ";", val_list);
    if (val_list.size() > 0) {
        objects::CUser_field& field = user.SetField(field_name);
        field.ResetData();
        for (auto it : val_list) {
            field.SetData().SetStrs().push_back(it);
        }
    }
    else {
        user.RemoveNamedField(field_name);
    }

}


bool CGeneralPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow()) {
        return false;
    }

    if (m_Block) {
        if (m_HUP->GetValue()) {
            m_Block->SetHup(true);
            wxDateTime dt = m_Date->GetValue();
            m_Block->SetReldate().SetStd().SetYear(dt.GetYear());
            m_Block->SetReldate().SetStd().SetMonth(dt.GetMonth() - wxDateTime::Jan + 1);
            m_Block->SetReldate().SetStd().SetDay(dt.GetDay());
        }
        else {
            m_Block->SetHup(false);
        }
    }

    if (m_Dblink) {
        objects::CUser_object& user = m_Dblink->SetUser();

        string biosample_val = ToStdString(m_Biosample->GetValue());
        SetField(user, "BioSample", biosample_val);

        string bioproject_val = ToStdString(m_Bioproject->GetValue());
        SetField(user, "BioProject", bioproject_val);
    }
    return true;
}

void CGeneralPanel::ApplyDescriptorCommand()
{
    if (!m_Dblink)
        return;

    CRef<objects::CSeqdesc> empty(new objects::CSeqdesc());
    empty->SetUser().SetObjectType(objects::CUser_object::eObjectType_DBLink);

    if (m_Dblink->Equals(*empty))
        return;

    CRef<CCmdComposite> cmd(new CCmdComposite("update dblink"));
    bool any_changes = false;

    for (objects::CBioseq_CI bi(m_Seh, objects::CSeq_inst::eMol_na); bi; ++bi) 
    {
        bool found = false;
        for (objects::CSeqdesc_CI di(*bi, objects::CSeqdesc::e_User); di; ++di)
        {
            if (di && di->IsUser() && di->GetUser().GetObjectType() == objects::CUser_object::eObjectType_DBLink)
            {
            
                // edit existing descriptor
                CRef<objects::CSeqdesc> cpy(new objects::CSeqdesc());
                cpy->Assign(*m_Dblink);
                if (!di->Equals(*cpy)) 
                {
                    CRef<CCmdChangeSeqdesc> chg(new CCmdChangeSeqdesc(di.GetSeq_entry_Handle(), *di, *cpy));
                    cmd->AddCommand(*chg);
                    any_changes = true;
                }
                found = true;
            } 
        }
        
        if (!found) 
        {
            // create new source descriptor on this sequence or on the nuc-prot that contains it
            CRef<objects::CSeqdesc> new_desc(new objects::CSeqdesc());

            new_desc->Assign(*m_Dblink);
            objects::CBioseq_set_Handle parent = bi->GetParentBioseq_set();
            auto entry = bi->GetParentEntry();
            if (parent && parent.IsSetClass() && parent.GetClass() == objects::CBioseq_set::eClass_nuc_prot) 
                entry = parent.GetParentEntry();
            CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(entry, *new_desc));
            cmd->AddCommand(*cmdAddDesc);
            any_changes = true;
        }
    }
    
    if (any_changes) 
    {
        m_CmdProcessor->Execute(cmd);
    }
}

void CGeneralPanel::ApplyCommand()
{
    if (!TransferDataFromWindow())
        return;
    ApplyDescriptorCommand();
    GetParentWizard(this)->ApplySubmitCommand();    
}

END_NCBI_SCOPE
