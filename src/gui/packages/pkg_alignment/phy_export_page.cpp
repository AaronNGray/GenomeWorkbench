/*  $Id: phy_export_page.cpp 39318 2017-09-12 16:00:18Z evgeniev $
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

#include <corelib/ncbifile.hpp>

#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/wx/save_file_helper.hpp>

////@begin includes
////@end includes

#include "phy_export_page.hpp"

////@begin XPM images
////@end XPM images


BEGIN_NCBI_SCOPE

/*!
 * CPhyExportPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CPhyExportPage, wxPanel )


/*!
 * CPhyExportPage event table definition
 */

BEGIN_EVENT_TABLE( CPhyExportPage, wxPanel )

////@begin CPhyExportPage event table entries
EVT_BUTTON(ID_BITMAPBUTTON1, CPhyExportPage::OnButton2Click)

////@end CPhyExportPage event table entries

END_EVENT_TABLE()


/*!
 * CPhyExportPage constructors
 */

CPhyExportPage::CPhyExportPage()
{
    Init();
}

CPhyExportPage::CPhyExportPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CAgpPExportPage creator
 */

bool CPhyExportPage::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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
////@end CPhyExportPage creation
    return true;
}


/*!
 * CPhyExportPage destructor
 */

CPhyExportPage::~CPhyExportPage()
{
////@begin CPhyExportPage destruction
////@end CPhyExportPage destruction
}


/*!
 * Member initialisation
 */

void CPhyExportPage::Init()
{
////@begin CPhyExportPage member initialisation
    m_LocationSel = NULL;
////@end CPhyExportPage member initialisation
}


/*!
 * Control creation for CAgpPExportPage
 */

void CPhyExportPage::CreateControls()
{


////@begin CPhyExportPage content construction
    CPhyExportPage* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_LocationSel = new CObjectListWidget( itemPanel1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL|wxLC_SINGLE_SEL );
    itemBoxSizer2->Add(m_LocationSel, 1, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(2, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer4, 0, wxALIGN_LEFT|wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Export Tree Format"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemChoice6Strings;
    itemChoice6Strings.Add(_("Newick"));
    itemChoice6Strings.Add(_("Nexus"));
    wxChoice* itemChoice6 = new wxChoice( itemPanel1, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, itemChoice6Strings, 0 );
    itemFlexGridSizer4->Add(itemChoice6, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    
    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer10, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText11 = new wxStaticText( itemPanel1, wxID_STATIC, _("File name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemStaticText11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl12 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL5, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemTextCtrl12, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton13 = new wxBitmapButton(itemPanel1, ID_BITMAPBUTTON1, itemPanel1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    itemBitmapButton13->SetHelpText(_("Select File"));
    if (CPhyExportPage::ShowToolTips())
        itemBitmapButton13->SetToolTip(_("Select File"));
    itemBoxSizer10->Add(itemBitmapButton13, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    // Set validators
    itemTextCtrl12->SetValidator( wxTextValidator(wxFILTER_NONE, GetData().SetFileName()) );
////@end CPhyExportPage content construction

//    m_LocationSel->SetTitle(wxT("Objects To Export"));
    m_SaveFile.reset(new CSaveFileHelper(this, *itemTextCtrl12));
}

void CPhyExportPage::SetObjects(TConstScopedObjects * objects)
{
    m_LocationSel->SetObjects(*objects);
}

/*!
 * Should we show tooltips?
 */

bool CPhyExportPage::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CPhyExportPage::GetBitmapResource( const wxString& name )
{
    return wxArtProvider::GetBitmap(name);
}

/*!
 * Get icon resources
 */

wxIcon CPhyExportPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CPhyExportPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CPhyExportPage icon retrieval
}

/*!
 * Transfer data to the window
 */

bool CPhyExportPage::TransferDataToWindow()
{
    wxChoice* choice = (wxChoice*)FindWindow(ID_CHOICE2);
    choice->SetStringSelection(GetData().GetFileFormat());

    return wxPanel::TransferDataToWindow();
}

/*!
 * Transfer data from the window
 */

bool CPhyExportPage::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    wxChoice* choice = (wxChoice*)FindWindow(ID_CHOICE2);
    GetData().SetFileFormat(choice->GetStringSelection());

    wxString path = GetData().GetFileName();
    _ASSERT(m_SaveFile);
    return m_SaveFile->Validate(path);
}

//static const char* kLocationList = ".LocationList";

void CPhyExportPage::SetRegistryPath(const string& path)
{
    m_RegPath = path;
//    m_LocationSel->SetRegistryPath(m_RegPath + kLocationList);
}

void CPhyExportPage::SaveSettings() const
{
//    m_LocationSel->SaveSettings();
}

void CPhyExportPage::LoadSettings()
{
 //   m_LocationSel->LoadSettings();
}

void CPhyExportPage::OnButton2Click( wxCommandEvent& event )
{
    // Update file-choice dialog to show the file type selected by the user:
    wxChoice* choice = (wxChoice*)FindWindow(ID_CHOICE2);
    CFileExtensions::EFileType file_type;

    if (choice != NULL) {
        wxString s = choice->GetStringSelection();
        if (s == _("Newick")) {
            file_type = CFileExtensions::kNewick;
        }
        else if (s == _("Nexus")) {
            file_type = CFileExtensions::kNexus;
        }
        else {
            // Unsupported tree type
            _ASSERT(false);
        }
    }

    _ASSERT(m_SaveFile);
    m_SaveFile->ShowSaveDialog(file_type);
}

END_NCBI_SCOPE
