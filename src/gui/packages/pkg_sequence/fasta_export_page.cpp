/*  $Id: fasta_export_page.cpp 39318 2017-09-12 16:00:18Z evgeniev $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>

#include <wx/sizer.h>
#include <wx/checkbox.h>
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
#include <wx/radiobox.h>

#include <gui/widgets/object_list/object_list_widget_sel.hpp>

////@begin includes
////@end includes

#include "fasta_export_page.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CFastaExportPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CFastaExportPage, wxPanel )


/*!
 * CFastaExportPage event table definition
 */

BEGIN_EVENT_TABLE( CFastaExportPage, wxPanel )

////@begin CFastaExportPage event table entries
    EVT_BUTTON( ID_BITMAPBUTTON3, CFastaExportPage::OnSelectFileClick )
////@end CFastaExportPage event table entries

END_EVENT_TABLE()


/*!
 * CFastaExportPage constructors
 */

CFastaExportPage::CFastaExportPage()
{
    Init();
}

CFastaExportPage::CFastaExportPage( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * fasta_export_page creator
 */

bool CFastaExportPage::Create( wxWindow* parent, wxWindowID id, const wxString& WXUNUSED(caption), const wxPoint& pos, const wxSize& WXUNUSED(size), long style )
{
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
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


/*!
 * CFastaExportPage destructor
 */

CFastaExportPage::~CFastaExportPage()
{
////@begin CFastaExportPage destruction
////@end CFastaExportPage destruction
}


/*!
 * Member initialisation
 */

void CFastaExportPage::Init()
{
////@begin CFastaExportPage member initialisation
    m_LocationSel = NULL;
////@end CFastaExportPage member initialisation
}


/*!
 * Control creation for fasta_export_page
 */

void CFastaExportPage::CreateControls()
{    
////@begin CFastaExportPage content construction
    CFastaExportPage* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_LocationSel = new CObjectListWidgetSel( itemPanel1, ID_PANEL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_LocationSel, 1, wxGROW|wxALL, 0);

    wxCheckBox* itemCheckBox4 = new wxCheckBox( itemPanel1, ID_CHECKBOX, _("Save master record"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox4->SetValue(false);
    itemBoxSizer2->Add(itemCheckBox4, 0, wxALIGN_LEFT|wxALL, wxDLG_UNIT(itemPanel1, wxSize(5, -1)).x);

    wxArrayString itemRadioBox5Strings;
    itemRadioBox5Strings.Add(_("&Export unique locations"));
    itemRadioBox5Strings.Add(_("&Export the whole sequence(s)"));
    wxRadioBox* itemRadioBox5 = new wxRadioBox( itemPanel1, ID_RADIOBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, itemRadioBox5Strings, 1, wxRA_SPECIFY_ROWS );
    itemRadioBox5->SetSelection(0);
    itemBoxSizer2->Add(itemRadioBox5, 0, wxGROW|wxALL, wxDLG_UNIT(itemPanel1, wxSize(5, -1)).x);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("File name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemPanel1, wxSize(5, -1)).x);

    wxTextCtrl* itemTextCtrl8 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemTextCtrl8, 1, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemPanel1, wxSize(5, -1)).x);

    wxBitmapButton* itemBitmapButton9 = new wxBitmapButton( itemPanel1, ID_BITMAPBUTTON3, itemPanel1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    itemBitmapButton9->SetHelpText(_("Select FASTA File"));
    if (CFastaExportPage::ShowToolTips())
        itemBitmapButton9->SetToolTip(_("Select FASTA File"));
    itemBoxSizer6->Add(itemBitmapButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemPanel1, wxSize(5, -1)).x);

    // Set validators
    itemCheckBox4->SetValidator( wxGenericValidator(& GetData().m_SaveMasterRecord) );
    itemRadioBox5->SetValidator( wxGenericValidator(& GetData().m_ExportType) );
    itemTextCtrl8->SetValidator( wxTextValidator(wxFILTER_NONE, & GetData().m_FileName) );
////@end CFastaExportPage content construction

    m_LocationSel->SetDoSelectAll(true);

    m_SaveFile.reset(new CSaveFileHelper(this, *itemTextCtrl8));
}

void CFastaExportPage::SetObjects(map<string, TConstScopedObjects>* objects)
{
    m_LocationSel->SetObjects(objects);
}

/*!
 * Should we show tooltips?
 */

bool CFastaExportPage::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CFastaExportPage::GetBitmapResource( const wxString& name )
{
    return wxArtProvider::GetBitmap(name);
}

/*!
 * Get icon resources
 */

wxIcon CFastaExportPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFastaExportPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFastaExportPage icon retrieval
}

bool CFastaExportPage::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    TConstScopedObjects selection = m_LocationSel->GetSelection();

    if (selection.empty()) {
        wxMessageBox(wxT("Please, select at least one object to export"), wxT("Error"),
                     wxOK | wxICON_ERROR, this);
        FindWindow(ID_PANEL)->SetFocus();
        return false;
    }

    m_data.m_Objects = selection;

    wxString path = GetData().GetFileName();
    _ASSERT(m_SaveFile);
    return m_SaveFile->Validate(path);
}

static const char* kLocationList = ".LocationList";

void CFastaExportPage::SetRegistryPath(const string& path)
{
    m_RegPath = path;
    m_LocationSel->SetRegistryPath(m_RegPath + kLocationList);
}

void CFastaExportPage::SaveSettings() const
{
    m_LocationSel->SaveSettings();
}

void CFastaExportPage::LoadSettings()
{
    m_LocationSel->LoadSettings();
}

void CFastaExportPage::OnSelectFileClick( wxCommandEvent& WXUNUSED(event) )
{
    _ASSERT(m_SaveFile);
    m_SaveFile->ShowSaveDialog(CFileExtensions::kFASTA);
}

END_NCBI_SCOPE
