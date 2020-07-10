/*  $Id: assembly_sel_panel.cpp 44756 2020-03-05 18:44:36Z shkeda $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/widgets/loaders/assembly_sel_panel.hpp>
#include <gui/objutils/registry.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/loaders/select_assembly_dialog.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/icon.h>
#include <wx/bitmap.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

DEFINE_EVENT_TYPE(wxEVT_ASSEMBLY_CHANGED_EVENT)

IMPLEMENT_DYNAMIC_CLASS( CAssemblySelPanel, wxPanel )

BEGIN_EVENT_TABLE( CAssemblySelPanel, wxPanel )

////@begin CAssemblySelPanel event table entries
    EVT_CHECKBOX( ID_CHECKBOX6, CAssemblySelPanel::OnUseMappingClick )
    EVT_BUTTON( ID_BUTTON1, CAssemblySelPanel::OnSelectAssemBtnClick )
////@end CAssemblySelPanel event table entries

END_EVENT_TABLE()

CAssemblySelPanel::CAssemblySelPanel()
{
    Init();
}

CAssemblySelPanel::CAssemblySelPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CAssemblySelPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAssemblySelPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
////@end CAssemblySelPanel creation
    return true;
}

CAssemblySelPanel::~CAssemblySelPanel()
{
////@begin CAssemblySelPanel destruction
////@end CAssemblySelPanel destruction
}

void CAssemblySelPanel::Init()
{
////@begin CAssemblySelPanel member initialisation
    m_UseMappingCtrl = NULL;
    m_AssemblyName = NULL;
    m_AssemblyAccession = NULL;
    m_AssemblyDescription = NULL;
////@end CAssemblySelPanel member initialisation
}

void CAssemblySelPanel::CreateControls()
{    
////@begin CAssemblySelPanel content construction
    CAssemblySelPanel* itemPanel1 = this;

    wxStaticBox* itemStaticBoxSizer2Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Assembly to Map IDs to their Correct NCBI Accessions"));
    wxStaticBoxSizer* itemStaticBoxSizer2 = new wxStaticBoxSizer(itemStaticBoxSizer2Static, wxVERTICAL);
    itemPanel1->SetSizer(itemStaticBoxSizer2);

    m_UseMappingCtrl = new wxCheckBox( itemStaticBoxSizer2->GetStaticBox(), ID_CHECKBOX6, _("Use Mapping"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UseMappingCtrl->SetValue(false);
    itemStaticBoxSizer2->Add(m_UseMappingCtrl, 0, wxALIGN_LEFT|wxALL, wxDLG_UNIT(itemStaticBoxSizer2->GetStaticBox(), wxSize(5, -1)).x);

    wxStaticText* itemStaticText4 = new wxStaticText( itemStaticBoxSizer2->GetStaticBox(), wxID_STATIC, _("Assembly name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer2->Add(itemStaticText4, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP, wxDLG_UNIT(itemStaticBoxSizer2->GetStaticBox(), wxSize(5, -1)).x);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer2->Add(itemBoxSizer5, 0, wxGROW|wxALL, 0);

    m_AssemblyName = new wxStaticText( itemStaticBoxSizer2->GetStaticBox(), wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER );
    itemBoxSizer5->Add(m_AssemblyName, 1, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemStaticBoxSizer2->GetStaticBox(), wxSize(5, -1)).x);

    wxButton* itemButton7 = new wxButton( itemStaticBoxSizer2->GetStaticBox(), ID_BUTTON1, _("Find Assembly..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, wxDLG_UNIT(itemStaticBoxSizer2->GetStaticBox(), wxSize(5, -1)).x);

    wxStaticText* itemStaticText8 = new wxStaticText( itemStaticBoxSizer2->GetStaticBox(), wxID_STATIC, _("Accession:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer2->Add(itemStaticText8, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP, wxDLG_UNIT(itemStaticBoxSizer2->GetStaticBox(), wxSize(5, -1)).x);

    m_AssemblyAccession = new wxStaticText( itemStaticBoxSizer2->GetStaticBox(), wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT|wxSTATIC_BORDER );
    itemStaticBoxSizer2->Add(m_AssemblyAccession, 0, wxGROW|wxALL, wxDLG_UNIT(itemStaticBoxSizer2->GetStaticBox(), wxSize(5, -1)).x);

    wxStaticText* itemStaticText10 = new wxStaticText( itemStaticBoxSizer2->GetStaticBox(), wxID_STATIC, _("Description:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer2->Add(itemStaticText10, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP, wxDLG_UNIT(itemStaticBoxSizer2->GetStaticBox(), wxSize(5, -1)).x);

    m_AssemblyDescription = new wxStaticText( itemStaticBoxSizer2->GetStaticBox(), wxID_STATIC, _("                \"\n\n                \"\n              "), wxDefaultPosition, wxDLG_UNIT(itemStaticBoxSizer2->GetStaticBox(), wxSize(213, -1)), wxALIGN_LEFT|wxST_NO_AUTORESIZE|wxSTATIC_BORDER );
    itemStaticBoxSizer2->Add(m_AssemblyDescription, 1, wxGROW|wxALL, wxDLG_UNIT(itemStaticBoxSizer2->GetStaticBox(), wxSize(5, -1)).x);

    // Set validators
    m_UseMappingCtrl->SetValidator( wxGenericValidator(& GetData().m_UseMapping) );
////@end CAssemblySelPanel content construction
}

void CAssemblySelPanel::x_UpdateAssembly()
{
    m_AssemblyAccession->SetLabel(ToWxString(GetData().m_AssemblyAcc));
    m_AssemblyName->SetLabel(ToWxString(GetData().m_AssemblyName));
    m_AssemblyDescription->SetLabel(ToWxString(GetData().m_AssemblyDesc));
    int width = 0;
    m_AssemblyDescription->GetClientSize(&width, nullptr);
    m_AssemblyDescription->Wrap(width);
    Layout();
}

bool CAssemblySelPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    x_UpdateControls();
    x_UpdateAssembly();

    return true;
}

bool CAssemblySelPanel::ShowToolTips()
{
    return true;
}
wxBitmap CAssemblySelPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAssemblySelPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAssemblySelPanel bitmap retrieval
}
wxIcon CAssemblySelPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAssemblySelPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAssemblySelPanel icon retrieval
}

void CAssemblySelPanel::OnSelectAssemBtnClick( wxCommandEvent& event )
{
    CSelectAssemblyDialog dlg(this);

    dlg.SetRegistryPath("Dialogs.CSelectAssemblyDialog");
    dlg.SetAssmSearchTerm(NStr::TruncateSpaces(GetData().m_SearchTerm));

    if (dlg.ShowModal() == wxID_OK) {
        GetData().m_AssemblyAcc = dlg.GetSelectedAssembly(GetData().m_AssemblyName, GetData().m_AssemblyDesc);
        GetData().m_SearchTerm = dlg.GetAssmSearchTerm();
        x_UpdateAssembly();

        wxCommandEvent evt( wxEVT_ASSEMBLY_CHANGED_EVENT );
        evt.SetEventObject( this );
        GetEventHandler()->ProcessEvent( evt );
    }  
}

void CAssemblySelPanel::OnUseMappingClick( wxCommandEvent& event )
{
    x_UpdateControls();

    wxCommandEvent evt( wxEVT_ASSEMBLY_CHANGED_EVENT );
    evt.SetEventObject( this );
    GetEventHandler()->ProcessEvent( evt );
}

void CAssemblySelPanel::x_UpdateControls()
{
    if (m_UseMappingCtrl->IsChecked()) {
        FindWindow(ID_BUTTON1)->Enable(true);
        m_AssemblyName->Enable(true);
        m_AssemblyAccession->Enable(true);
        m_AssemblyDescription->Enable(true);
    }
    else {
        FindWindow(ID_BUTTON1)->Enable(false);
        m_AssemblyName->Enable(false);
        m_AssemblyAccession->Enable(false);
        m_AssemblyDescription->Enable(false);
    }
}

bool CAssemblySelPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    if (GetData().m_UseMapping && GetData().m_AssemblyAcc.empty()) {
        wxMessageBox(wxT("Please, select assembly for Mapping."), wxT("Error"),
                     wxOK | wxICON_ERROR, this);
        return false;
    }

    return true;
}

END_NCBI_SCOPE
