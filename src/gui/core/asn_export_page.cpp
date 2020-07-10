/*  $Id: asn_export_page.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <ncbi_pch.hpp>////@begin includes
////@end includes

#include "asn_export_page.hpp"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/bmpbuttn.h>
#include <wx/radiobox.h>
#include <wx/valgen.h>
#include <wx/valtext.h>
#include <wx/icon.h>
#include <wx/bitmap.h>
#include <wx/textctrl.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/artprov.h>

#include <gui/widgets/object_list/object_list_widget.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CAsnExportPage, wxPanel )

BEGIN_EVENT_TABLE( CAsnExportPage, wxPanel )

////@begin CAsnExportPage event table entries
    EVT_BUTTON( ID_BITMAPBUTTON, CAsnExportPage::OnSelectFileClick )

////@end CAsnExportPage event table entries

END_EVENT_TABLE()

CAsnExportPage::CAsnExportPage()
{
    Init();
}

CAsnExportPage::CAsnExportPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, long style )
{
    Init();
    Create(parent, id, pos, style);
}

bool CAsnExportPage::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, long style )
{
    wxPanel::Create( parent, id, pos, wxSize(0,0), style );
    Hide();

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}

CAsnExportPage::~CAsnExportPage()
{
////@begin CAsnExportPage destruction
////@end CAsnExportPage destruction
}

void CAsnExportPage::Init()
{
////@begin CAsnExportPage member initialisation
    m_ObjectSel = NULL;
    m_FileNameCtrl = NULL;
////@end CAsnExportPage member initialisation
}

void CAsnExportPage::CreateControls()
{
////@begin CAsnExportPage content construction
    CAsnExportPage* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_ObjectSel = new CObjectListWidget( itemPanel1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_ObjectSel, 1, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_LEFT|wxALL, 0);

    wxArrayString itemRadioBox5Strings;
    itemRadioBox5Strings.Add(_("&Text"));
    itemRadioBox5Strings.Add(_("&Binary"));
    wxRadioBox* itemRadioBox5 = new wxRadioBox( itemPanel1, ID_RADIOBOX3, _("ASN Type"), wxDefaultPosition, wxDefaultSize, itemRadioBox5Strings, 1, wxRA_SPECIFY_ROWS );
    itemRadioBox5->SetSelection(0);
    itemBoxSizer4->Add(itemRadioBox5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("File name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FileNameCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(m_FileNameCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton9 = new wxBitmapButton( itemPanel1, ID_BITMAPBUTTON, itemPanel1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    itemBitmapButton9->SetHelpText(_("Select ASN.1 File"));
    if (CAsnExportPage::ShowToolTips())
        itemBitmapButton9->SetToolTip(_("Select ASN.1 File"));
    itemBoxSizer6->Add(itemBitmapButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemRadioBox5->SetValidator( wxGenericValidator(& GetData().m_AsnType) );
    m_FileNameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & GetData().m_FileName) );
////@end CAsnExportPage content construction

    m_SaveFile.reset(new CSaveFileHelper(this, *m_FileNameCtrl));
}

bool CAsnExportPage::ShowToolTips()
{
    return true;
}
wxBitmap CAsnExportPage::GetBitmapResource( const wxString& name )
{
    return wxArtProvider::GetBitmap(name);
}
wxIcon CAsnExportPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAsnExportPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAsnExportPage icon retrieval
}

bool CAsnExportPage::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    m_ObjectSel->SetObjects(m_data.m_Objects);
    m_ObjectSel->SelectAll();

    return true;
}
bool CAsnExportPage::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    TConstScopedObjects objects;
    m_ObjectSel->GetSelection(objects);

    if (objects.empty()) {
        wxMessageBox(wxT("Please, select at least one object to export"), wxT("Error"),
                     wxOK | wxICON_ERROR, this);
        FindWindow(ID_PANEL2)->SetFocus();
        return false;
    }

    GetData().m_Objects = objects;

    wxString path = GetData().GetFileName();
    _ASSERT(m_SaveFile);
    return m_SaveFile->Validate(path);
}

static const char* kObjectList = "ObjectList";

void CAsnExportPage::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}

void CAsnExportPage::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kObjectList);
        CRegistryWriteView table_view = gui_reg.GetWriteView(reg_path);
        m_ObjectSel->SaveTableSettings(table_view);
    }
}

void CAsnExportPage::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path;
        CRegistryReadView table_view;

        reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kObjectList);
        table_view = gui_reg.GetReadView(reg_path);
        m_ObjectSel->LoadTableSettings(table_view);
    }
}

void CAsnExportPage::OnSelectFileClick( wxCommandEvent& WXUNUSED(event) )
{
    _ASSERT(m_SaveFile);
    m_SaveFile->ShowSaveDialog(CFileExtensions::kASN);
}

END_NCBI_SCOPE
