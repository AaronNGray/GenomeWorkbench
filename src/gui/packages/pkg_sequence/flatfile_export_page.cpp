/*  $Id: flatfile_export_page.cpp 39318 2017-09-12 16:00:18Z evgeniev $
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

#include <gui/widgets/object_list/object_list_widget_sel.hpp>

////@begin includes
////@end includes

#include "flatfile_export_page.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CFlatFileExportPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CFlatFileExportPage, wxPanel )


/*!
 * CFlatFileExportPage event table definition
 */

BEGIN_EVENT_TABLE( CFlatFileExportPage, wxPanel )

////@begin CFlatFileExportPage event table entries
    EVT_BUTTON( ID_BITMAPBUTTON4, CFlatFileExportPage::OnSelectFileClick )

////@end CFlatFileExportPage event table entries

END_EVENT_TABLE()


/*!
 * CFlatFileExportPage constructors
 */

CFlatFileExportPage::CFlatFileExportPage()
{
    Init();
}

CFlatFileExportPage::CFlatFileExportPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, long style )
{
    Init();
    Create(parent, id, pos, style);
}


/*!
 * CFlatFileExportPage creator
 */

bool CFlatFileExportPage::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, long style )
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
 * CFlatFileExportPage destructor
 */

CFlatFileExportPage::~CFlatFileExportPage()
{
////@begin CFlatFileExportPage destruction
////@end CFlatFileExportPage destruction
}


/*!
 * Member initialisation
 */

void CFlatFileExportPage::Init()
{
////@begin CFlatFileExportPage member initialisation
    m_LocationSel = NULL;
////@end CFlatFileExportPage member initialisation
}


/*!
 * Control creation for CFlatFileExportPage
 */

void CFlatFileExportPage::CreateControls()
{
////@begin CFlatFileExportPage content construction
    CFlatFileExportPage* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_LocationSel = new CObjectListWidgetSel( itemPanel1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL|wxLC_SINGLE_SEL );
    itemBoxSizer2->Add(m_LocationSel, 1, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_LEFT|wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Flat File Mode"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemChoice6Strings;
    itemChoice6Strings.Add(_("Genome Workbench"));
    itemChoice6Strings.Add(_("Entrez"));
    itemChoice6Strings.Add(_("Release"));
    itemChoice6Strings.Add(_("Dump"));
    wxChoice* itemChoice6 = new wxChoice( itemPanel1, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, itemChoice6Strings, 0 );
    itemChoice6->SetStringSelection(_("Genome Workbench"));
    itemBoxSizer4->Add(itemChoice6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("File name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL7, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemTextCtrl9, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton10 = new wxBitmapButton( itemPanel1, ID_BITMAPBUTTON4, itemPanel1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    itemBitmapButton10->SetHelpText(_("Select File"));
    if (CFlatFileExportPage::ShowToolTips())
        itemBitmapButton10->SetToolTip(_("Select File"));
    itemBoxSizer7->Add(itemBitmapButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemChoice6->SetValidator( wxGenericValidator(& GetData().m_Mode) );
    itemTextCtrl9->SetValidator( wxTextValidator(wxFILTER_NONE, & GetData().m_FileName) );
////@end CFlatFileExportPage content construction

    m_LocationSel->SetTitle(wxT("Select Location"));
    m_SaveFile.reset(new CSaveFileHelper(this, *itemTextCtrl9));
}

void CFlatFileExportPage::SetObjects(map<string, TConstScopedObjects>* objects)
{
    m_LocationSel->SetObjects(objects);
}

/*!
 * Should we show tooltips?
 */

bool CFlatFileExportPage::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CFlatFileExportPage::GetBitmapResource( const wxString& name )
{
    return wxArtProvider::GetBitmap(name);
}

/*!
 * Get icon resources
 */

wxIcon CFlatFileExportPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFlatFileExportPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFlatFileExportPage icon retrieval
}

/*!
 * Transfer data from the window
 */

bool CFlatFileExportPage::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    TConstScopedObjects selection = m_LocationSel->GetSelection();
    m_data.m_SeqLoc = selection[0];

    wxString path = GetData().GetFileName();
    _ASSERT(m_SaveFile);
    return m_SaveFile->Validate(path);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON4
 */

void CFlatFileExportPage::OnSelectFileClick( wxCommandEvent& WXUNUSED(event) )
{
    _ASSERT(m_SaveFile);
    m_SaveFile->ShowSaveDialog(CFileExtensions::kGenBank);
}

static const char* kLocationList = ".LocationList";

void CFlatFileExportPage::SetRegistryPath(const string& path)
{
    m_RegPath = path;
    m_LocationSel->SetRegistryPath(m_RegPath + kLocationList);
}

void CFlatFileExportPage::SaveSettings() const
{
    m_LocationSel->SaveSettings();
}

void CFlatFileExportPage::LoadSettings()
{
    m_LocationSel->LoadSettings();
}

END_NCBI_SCOPE
