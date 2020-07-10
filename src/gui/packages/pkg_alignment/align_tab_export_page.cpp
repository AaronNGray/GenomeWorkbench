/*  $Id: align_tab_export_page.cpp 39318 2017-09-12 16:00:18Z evgeniev $
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

#include "align_tab_export_page.hpp"

#include <objects/seqloc/Seq_loc.hpp>

#include <gui/widgets/object_list/object_list_widget_sel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CAlignTabExportPage, wxPanel )

BEGIN_EVENT_TABLE( CAlignTabExportPage, wxPanel )

////@begin CAlignTabExportPage event table entries
    EVT_BUTTON( ID_BITMAPBUTTON, CAlignTabExportPage::OnSelectFileClick )

////@end CAlignTabExportPage event table entries

END_EVENT_TABLE()

CAlignTabExportPage::CAlignTabExportPage()
{
    Init();
}

CAlignTabExportPage::CAlignTabExportPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CAlignTabExportPage::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAlignTabExportPage creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAlignTabExportPage creation
    return true;
}

CAlignTabExportPage::~CAlignTabExportPage()
{
////@begin CAlignTabExportPage destruction
////@end CAlignTabExportPage destruction
}

void CAlignTabExportPage::Init()
{
////@begin CAlignTabExportPage member initialisation
    m_LocationSel = NULL;
////@end CAlignTabExportPage member initialisation
}

void CAlignTabExportPage::CreateControls()
{    
////@begin CAlignTabExportPage content construction
    CAlignTabExportPage* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_LocationSel = new CObjectListWidgetSel( itemPanel1, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL|wxLC_SINGLE_SEL );
    itemBoxSizer2->Add(m_LocationSel, 1, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("File name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl6 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL14, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemTextCtrl6, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton7 = new wxBitmapButton( itemPanel1, ID_BITMAPBUTTON, itemPanel1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    itemBitmapButton7->SetHelpText(_("Select File"));
    if (CAlignTabExportPage::ShowToolTips())
        itemBitmapButton7->SetToolTip(_("Select File"));
    itemBoxSizer4->Add(itemBitmapButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemTextCtrl6->SetValidator( wxTextValidator(wxFILTER_NONE, & GetData().m_FileName) );
////@end CAlignTabExportPage content construction

    m_LocationSel->SetTitle(wxT("Select Location"));

    m_SaveFile.reset(new CSaveFileHelper(this, *itemTextCtrl6));
}

void CAlignTabExportPage::SetObjects(map<string, TConstScopedObjects>* objects)
{
    m_LocationSel->SetObjects(objects);
}


bool CAlignTabExportPage::ShowToolTips()
{
    return true;
}
wxBitmap CAlignTabExportPage::GetBitmapResource( const wxString& name )
{
    return wxArtProvider::GetBitmap(name);
}
wxIcon CAlignTabExportPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAlignTabExportPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAlignTabExportPage icon retrieval
}
bool CAlignTabExportPage::TransferDataToWindow()
{
    return wxPanel::TransferDataToWindow();
}
bool CAlignTabExportPage::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    TConstScopedObjects selection = m_LocationSel->GetSelection();
    if (selection.size() != 1) {
        wxMessageBox(wxT("Please select a single location"), wxT("Error"),
                     wxOK | wxICON_ERROR, this);
        m_LocationSel->SetFocus();
        return false;
    }

    const objects::CSeq_loc* seq_loc = dynamic_cast<const objects::CSeq_loc*>(selection[0].object.GetPointerOrNull());
    if (!seq_loc) {
        wxMessageBox(wxT("Internal error. The selections is not a location,\n")
                     wxT("please, try to select another location"), wxT("Internal Error"),
                     wxOK | wxICON_ERROR, this);
        m_LocationSel->SetFocus();
        return false;
    }

    GetData().SetObject() = selection[0];

    wxString path = GetData().GetFileName();
    _ASSERT(m_SaveFile);
    return m_SaveFile->Validate(path);
}

static const char* kLocationList = ".LocationList";

void CAlignTabExportPage::SetRegistryPath(const string& path)
{
    m_RegPath = path;
    m_LocationSel->SetRegistryPath(m_RegPath + kLocationList);
}

void CAlignTabExportPage::SaveSettings() const
{
    m_LocationSel->SaveSettings();
}

void CAlignTabExportPage::LoadSettings()
{
    m_LocationSel->LoadSettings();
}

void CAlignTabExportPage::OnSelectFileClick( wxCommandEvent& event )
{
    _ASSERT(m_SaveFile);
    m_SaveFile->ShowSaveDialog(CFileExtensions::kTxt);
}

END_NCBI_SCOPE
