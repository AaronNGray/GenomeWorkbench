/*  $Id: 5col_export_page.cpp 39318 2017-09-12 16:00:18Z evgeniev $
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

#include "5col_export_page.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * C5ColExportPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( C5ColExportPage, wxPanel )


/*!
 * C5ColExportPage event table definition
 */

BEGIN_EVENT_TABLE( C5ColExportPage, wxPanel )

////@begin C5ColExportPage event table entries
    EVT_BUTTON( ID_BITMAPBUTTON2, C5ColExportPage::OnSelectFileClcik )

////@end C5ColExportPage event table entries

END_EVENT_TABLE()


/*!
 * C5ColExportPage constructors
 */

C5ColExportPage::C5ColExportPage()
{
    Init();
}

C5ColExportPage::C5ColExportPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * C5ColExportPage creator
 */

bool C5ColExportPage::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin C5ColExportPage creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end C5ColExportPage creation
    return true;
}


/*!
 * C5ColExportPage destructor
 */

C5ColExportPage::~C5ColExportPage()
{
////@begin C5ColExportPage destruction
////@end C5ColExportPage destruction
}


/*!
 * Member initialisation
 */

void C5ColExportPage::Init()
{
////@begin C5ColExportPage member initialisation
    m_LocationSel = NULL;
    m_FileNameCtrl = NULL;
////@end C5ColExportPage member initialisation
}


/*!
 * Control creation for C5ColExportPage
 */

void C5ColExportPage::CreateControls()
{    
////@begin C5ColExportPage content construction
    C5ColExportPage* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_LocationSel = new CObjectListWidgetSel( itemPanel1, ID_OBJECT_LIST, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL|wxLC_SINGLE_SEL );
    itemBoxSizer2->Add(m_LocationSel, 1, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_LEFT|wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Feature depth:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl6 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL10, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemTextCtrl6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox7 = new wxCheckBox( itemPanel1, ID_CHECKBOX17, _("Set exact feature level"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox7->SetValue(false);
    itemBoxSizer4->Add(itemCheckBox7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer8, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel1, wxID_STATIC, _("File name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FileNameCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL11, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(m_FileNameCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton11 = new wxBitmapButton( itemPanel1, ID_BITMAPBUTTON2, itemPanel1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    itemBitmapButton11->SetHelpText(_("Select File"));
    if (C5ColExportPage::ShowToolTips())
        itemBitmapButton11->SetToolTip(_("Select File"));
    itemBoxSizer8->Add(itemBitmapButton11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemTextCtrl6->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & GetData().m_FeatureDepth) );
    itemCheckBox7->SetValidator( wxGenericValidator(& GetData().m_ExactFlevel) );
    m_FileNameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_FileName) );
////@end C5ColExportPage content construction

    m_LocationSel->SetTitle(wxT("Select Location"));
    m_SaveFile.reset(new CSaveFileHelper(this, *m_FileNameCtrl));
}

void C5ColExportPage::SetObjects(map<string, TConstScopedObjects>* objects)
{
    m_LocationSel->SetObjects(objects);
}

/*!
 * Should we show tooltips?
 */

bool C5ColExportPage::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap C5ColExportPage::GetBitmapResource( const wxString& name )
{
    return wxArtProvider::GetBitmap(name);
}

/*!
 * Get icon resources
 */

wxIcon C5ColExportPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin C5ColExportPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end C5ColExportPage icon retrieval
}

/*!
 * Transfer data from the window
 */

bool C5ColExportPage::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    TConstScopedObjects selection = m_LocationSel->GetSelection();
    if (selection.empty()) {
        wxMessageBox(wxT("No objects were selected"), wxT("Error"),
                     wxOK | wxICON_ERROR, this);
        m_LocationSel->SetFocus();
        return false;
   }

    m_SeqLoc = selection[0];

    m_FileName.Trim(false);
    m_FileName.Trim(true);

    _ASSERT(m_SaveFile);
    return m_SaveFile->Validate(m_FileName);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON7
 */

void C5ColExportPage::OnSelectFileClcik( wxCommandEvent& event )
{
    _ASSERT(m_SaveFile);
    m_SaveFile->ShowSaveDialog(CFileExtensions::k5Column);
}

static const char* kLocationList = ".LocationList";

void C5ColExportPage::SetRegistryPath(const string& path)
{
    m_RegPath = path;
    m_LocationSel->SetRegistryPath(m_RegPath + kLocationList);
}

void C5ColExportPage::SaveSettings() const
{
    m_LocationSel->SaveSettings();
}

void C5ColExportPage::LoadSettings()
{
    m_LocationSel->LoadSettings();
}

END_NCBI_SCOPE
