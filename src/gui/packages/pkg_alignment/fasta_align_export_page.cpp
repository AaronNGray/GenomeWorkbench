/*  $Id: fasta_align_export_page.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors: Vladislav Evgeniev
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/valgen.h>
#include <wx/valtext.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/artprov.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

#include "fasta_align_export_page.hpp"

#include <objects/seqloc/Seq_loc.hpp>

#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CFastaAlignExportPage, wxPanel )

BEGIN_EVENT_TABLE( CFastaAlignExportPage, wxPanel )

////@begin CFastaAlignExportPage event table entries
    EVT_BUTTON( ID_BITMAPBUTTON, CFastaAlignExportPage::OnSelectFileClick )
////@end CFastaAlignExportPage event table entries
    
    EVT_LIST_ITEM_SELECTED(ID_PANEL4, CFastaAlignExportPage::OnObjectSelected)

END_EVENT_TABLE()

CFastaAlignExportPage::CFastaAlignExportPage() :
    m_FileSelected(false)
{
    Init();
}

CFastaAlignExportPage::CFastaAlignExportPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
    m_FileSelected(false)
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CFastaAlignExportPage::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFastaAlignExportPage creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFastaAlignExportPage creation
    return true;
}

CFastaAlignExportPage::~CFastaAlignExportPage()
{
////@begin CFastaAlignExportPage destruction
////@end CFastaAlignExportPage destruction
}

void CFastaAlignExportPage::Init()
{
////@begin CFastaAlignExportPage member initialisation
    m_ObjectSel = NULL;
////@end CFastaAlignExportPage member initialisation
}

void CFastaAlignExportPage::CreateControls()
{    
////@begin CFastaAlignExportPage content construction
    CFastaAlignExportPage* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_ObjectSel = new CObjectListWidget( itemPanel1, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL|wxLC_SINGLE_SEL );
    itemBoxSizer2->Add(m_ObjectSel, 1, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("File name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl6 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL14, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemTextCtrl6, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton7 = new wxBitmapButton( itemPanel1, ID_BITMAPBUTTON, itemPanel1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    itemBitmapButton7->SetHelpText(_("Select File"));
    if (CFastaAlignExportPage::ShowToolTips())
        itemBitmapButton7->SetToolTip(_("Select File"));
    itemBoxSizer4->Add(itemBitmapButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemTextCtrl6->SetValidator( wxTextValidator(wxFILTER_NONE, & GetData().m_FileName) );
    // Connect events and objects
    itemTextCtrl6->Connect(ID_TEXTCTRL14, wxEVT_CHAR, wxKeyEventHandler(CFastaAlignExportPage::OnFileNameEdited), NULL, this);
////@end CFastaAlignExportPage content construction

    m_ObjectSel->DisableMultipleSelection();

    m_SaveFile.reset(new CSaveFileHelper(this, *itemTextCtrl6));
}

bool CFastaAlignExportPage::ShowToolTips()
{
    return true;
}
wxBitmap CFastaAlignExportPage::GetBitmapResource( const wxString& name )
{
    return wxArtProvider::GetBitmap(name);
}
wxIcon CFastaAlignExportPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFastaAlignExportPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFastaAlignExportPage icon retrieval
}

bool CFastaAlignExportPage::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    TConstScopedObjects objects;
    m_ObjectSel->GetSelection(objects);

    if (objects.empty()) {
        wxMessageBox(wxT("Please, select an object to export"), wxT("Error"), wxOK | wxICON_ERROR, this);
        FindWindow(ID_PANEL4)->SetFocus();
        return false;
    }

    GetData().SetObject() = objects[0];

    wxString path = GetData().GetFileName();
    _ASSERT(m_SaveFile);
    return m_SaveFile->Validate(path);
}


void CFastaAlignExportPage::SetObjects(TConstScopedObjects& objects)
{
    m_ObjectSel->SetObjects(objects);
    
    if (1 == objects.size())
        m_ObjectSel->SelectAll();
}


void CFastaAlignExportPage::x_SanitizePath(wxString &path)
{
    const char invalid_file_name_chars[] = { '~', '#', '%', '&', '*', '{', '}', '\\', ':', '<', '>', '?', '/', '+', '|', '"' };
    for (char c : invalid_file_name_chars)
        path.erase(std::remove(path.begin(), path.end(), c), path.end());
    if (path[path.length() - 1] == '.')
        path = path.substr(0, path.length() - 1);

}


void CFastaAlignExportPage::OnObjectSelected(wxListEvent& event)
{
    if (m_FileSelected)
        return;
    TConstScopedObjects objects;
    m_ObjectSel->GetSelection(objects);
    if (objects.empty())
        return;

    long selected = m_ObjectSel->GetFirstSelected();
    if (-1 == selected)
        return;

    wxString label = m_ObjectSel->GetItemText(selected).ToStdString();

    if (label.empty())
        return;
 
    x_SanitizePath(label);
    
    wxStandardPaths& standard_paths = wxStandardPaths::Get();
    wxString path = standard_paths.GetDocumentsDir();
    path += wxFileName::GetPathSeparator();
    path += label;
    path += ".fa";
    GetData().SetFileName(path);
    TransferDataToWindow();
}


static const char* kObjectList = "ObjectList";

void CFastaAlignExportPage::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}

void CFastaAlignExportPage::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kObjectList);
        CRegistryWriteView table_view = gui_reg.GetWriteView(reg_path);
        m_ObjectSel->SaveTableSettings(table_view);
    }
}

void CFastaAlignExportPage::LoadSettings()
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

void CFastaAlignExportPage::OnSelectFileClick( wxCommandEvent& event )
{
    _ASSERT(m_SaveFile);
    if (m_SaveFile->ShowSaveDialog(CFileExtensions::kFASTA))
        m_FileSelected = true;
}

void CFastaAlignExportPage::OnFileNameEdited( wxKeyEvent& event )
{
    m_FileSelected = true;
    event.Skip();
 }

END_NCBI_SCOPE