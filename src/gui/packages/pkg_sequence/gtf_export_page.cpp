/*  $Id: gtf_export_page.cpp 39318 2017-09-12 16:00:18Z evgeniev $
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
#include <wx/statbox.h>
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

#include "gtf_export_page.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE


/*!
 * CGtfExportPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CGtfExportPage, wxPanel )


/*!
 * CGtfExportPage event table definition
 */

BEGIN_EVENT_TABLE( CGtfExportPage, wxPanel )

////@begin CGtfExportPage event table entries
    EVT_BUTTON( ID_BITMAPBUTTON1, CGtfExportPage::OnSelectFileClick )

////@end CGtfExportPage event table entries

END_EVENT_TABLE()


/*!
 * CGtfExportPage constructors
 */

CGtfExportPage::CGtfExportPage()
{
    Init();
}

CGtfExportPage::CGtfExportPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CGtfExportPage creator
 */

bool CGtfExportPage::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGtfExportPage creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGtfExportPage creation
    return true;
}


/*!
 * CGtfExportPage destructor
 */

CGtfExportPage::~CGtfExportPage()
{
////@begin CGtfExportPage destruction
////@end CGtfExportPage destruction
}


/*!
 * Member initialisation
 */

void CGtfExportPage::Init()
{
////@begin CGtfExportPage member initialisation
    m_LocationSel = NULL;
////@end CGtfExportPage member initialisation
}


/*!
 * Control creation for CGtfExportPage
 */

void CGtfExportPage::CreateControls()
{    
////@begin CGtfExportPage content construction
    CGtfExportPage* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_LocationSel = new CObjectListWidgetSel( itemPanel1, ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_LocationSel, 1, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_LEFT|wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Feature depth:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl6 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemTextCtrl6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox7 = new wxCheckBox( itemPanel1, ID_CHECKBOX1, _("Set exact feature level"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox7->SetValue(false);
    itemBoxSizer4->Add(itemCheckBox7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer8, 0, wxALIGN_LEFT|wxALL, 0);

    wxCheckBox* itemCheckBox9 = new wxCheckBox( itemPanel1, ID_CHECKBOX2, _("Suppress Gene Features"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox9->SetValue(false);
    itemBoxSizer8->Add(itemCheckBox9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox10 = new wxCheckBox( itemPanel1, ID_CHECKBOX3, _("Suppress Exon Numbers"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox10->SetValue(false);
    itemBoxSizer8->Add(itemCheckBox10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer11, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel1, wxID_STATIC, _("File name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemStaticText12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl13 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemTextCtrl13, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton14 = new wxBitmapButton( itemPanel1, ID_BITMAPBUTTON1, itemPanel1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    itemBitmapButton14->SetHelpText(_("Select GTF File"));
    if (CGtfExportPage::ShowToolTips())
        itemBitmapButton14->SetToolTip(_("Select GTF File"));
    itemBoxSizer11->Add(itemBitmapButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemTextCtrl6->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & GetData().m_FeatureDepth) );
    itemCheckBox7->SetValidator( wxGenericValidator(& GetData().m_ExactFlevel) );
    itemCheckBox9->SetValidator( wxGenericValidator(& GetData().m_SuppressGeneFeatures) );
    itemCheckBox10->SetValidator( wxGenericValidator(& GetData().m_SuppressExonNumbers) );
    itemTextCtrl13->SetValidator( wxTextValidator(wxFILTER_NONE, & GetData().m_FileName) );
////@end CGtfExportPage content construction

    m_LocationSel->SetTitle(wxT("Select Objects"));
    m_SaveFile.reset(new CSaveFileHelper(this, *itemTextCtrl13));
}

void CGtfExportPage::SetObjects(map<string, TConstScopedObjects>* objects)
{
    m_LocationSel->SetObjects(objects);
}

/*!
 * Should we show tooltips?
 */

bool CGtfExportPage::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CGtfExportPage::GetBitmapResource( const wxString& name )
{
    return wxArtProvider::GetBitmap(name);
}

/*!
 * Get icon resources
 */

wxIcon CGtfExportPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGtfExportPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGtfExportPage icon retrieval
}

/*!
 * Transfer data to the window
 */

/*!
 * Transfer data from the window
 */

bool CGtfExportPage::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    TConstScopedObjects objects = m_LocationSel->GetSelection();

    if (objects.empty()) {
        wxMessageBox(wxT("Please, select at least one object to export"), wxT("Error"),
            wxOK | wxICON_ERROR, this);
        m_LocationSel->SetFocus();
        return false;
    }

    m_data.m_Objects = objects;

    wxString path = GetData().GetFileName();
    _ASSERT(m_SaveFile);
    return m_SaveFile->Validate(path);
}

static const char* kLocationList = ".LocationList";

void CGtfExportPage::SetRegistryPath(const string& path)
{
    m_RegPath = path;
    m_LocationSel->SetRegistryPath(m_RegPath + kLocationList);
}

void CGtfExportPage::SaveSettings() const
{
    m_LocationSel->SaveSettings();
}

void CGtfExportPage::LoadSettings()
{
    m_LocationSel->LoadSettings();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3
 */

void CGtfExportPage::OnSelectFileClick( wxCommandEvent& event )
{
    _ASSERT(m_SaveFile);
    m_SaveFile->ShowSaveDialog(CFileExtensions::kGTF);
}

END_NCBI_SCOPE
