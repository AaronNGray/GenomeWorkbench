/*  $Id: agp_export_page.cpp 39318 2017-09-12 16:00:18Z evgeniev $
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

#include <gui/widgets/object_list/object_list_widget_sel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <objects/seqloc/Seq_loc.hpp>
#include <serial/typeinfo.hpp>

////@begin includes
////@end includes

#include "agp_export_page.hpp"

////@begin XPM images
////@end XPM images


BEGIN_NCBI_SCOPE

/*!
 * CAgpExportPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAgpExportPage, wxPanel )


/*!
 * CAgpExportPage event table definition
 */

BEGIN_EVENT_TABLE( CAgpExportPage, wxPanel )

////@begin CAgpExportPage event table entries
    EVT_BUTTON( ID_BITMAPBUTTON5, CAgpExportPage::OnSelectFileClick )

////@end CAgpExportPage event table entries

END_EVENT_TABLE()


/*!
 * CAgpExportPage constructors
 */

CAgpExportPage::CAgpExportPage()
{
    Init();
}

CAgpExportPage::CAgpExportPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, long style )
{
    Init();
    Create(parent, id, pos, style);
}


/*!
 * CAgpPExportPage creator
 */

bool CAgpExportPage::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, long style )
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
 * CAgpExportPage destructor
 */

CAgpExportPage::~CAgpExportPage()
{
////@begin CAgpExportPage destruction
////@end CAgpExportPage destruction
}


/*!
 * Member initialisation
 */

void CAgpExportPage::Init()
{
////@begin CAgpExportPage member initialisation
    m_LocationSel = NULL;
////@end CAgpExportPage member initialisation
}


/*!
 * Control creation for CAgpPExportPage
 */

void CAgpExportPage::CreateControls()
{
////@begin CAgpExportPage content construction
    CAgpExportPage* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_LocationSel = new CObjectListWidgetSel( itemPanel1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL|wxLC_SINGLE_SEL );
    itemBoxSizer2->Add(m_LocationSel, 1, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer4, 0, wxALIGN_LEFT|wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Gap type (col. 7)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemChoice6Strings;
    itemChoice6Strings.Add(_("fragment"));
    itemChoice6Strings.Add(_("clone"));
    itemChoice6Strings.Add(_("contig"));
    itemChoice6Strings.Add(_("split_finished"));
    itemChoice6Strings.Add(_("centromere"));
    itemChoice6Strings.Add(_("short_arm"));
    itemChoice6Strings.Add(_("heterochromatin"));
    itemChoice6Strings.Add(_("telomere"));
    wxChoice* itemChoice6 = new wxChoice( itemPanel1, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, itemChoice6Strings, 0 );
    itemFlexGridSizer4->Add(itemChoice6, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Alternate object id (column 1)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl8 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemTextCtrl8, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox9 = new wxCheckBox( itemPanel1, ID_CHECKBOX15, _("Assert evidence for linkage between components (\"yes\" in col. 8 of gap lines)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox9->SetValue(false);
    itemBoxSizer2->Add(itemCheckBox9, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer10, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText11 = new wxStaticText( itemPanel1, wxID_STATIC, _("File name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemStaticText11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl12 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemTextCtrl12, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton13 = new wxBitmapButton( itemPanel1, ID_BITMAPBUTTON5, itemPanel1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    itemBitmapButton13->SetHelpText(_("Select AGP File"));
    if (CAgpExportPage::ShowToolTips())
        itemBitmapButton13->SetToolTip(_("Select AGP File"));
    itemBoxSizer10->Add(itemBitmapButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemTextCtrl8->SetValidator( wxTextValidator(wxFILTER_NONE, & GetData().m_AltObjId) );
    itemCheckBox9->SetValidator( wxGenericValidator(& GetData().m_AssertEvidence) );
    itemTextCtrl12->SetValidator( wxTextValidator(wxFILTER_NONE, & GetData().m_FileName) );
////@end CAgpExportPage content construction

     m_LocationSel->SetTitle(wxT("Select Location"));
     m_SaveFile.reset(new CSaveFileHelper(this, *itemTextCtrl12));
}

void CAgpExportPage::SetObjects(map<string, TConstScopedObjects>* objects)
{
    _ASSERT(m_LocationSel);

    m_LocationSel->SetObjects(objects);
}

/*!
 * Should we show tooltips?
 */

bool CAgpExportPage::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAgpExportPage::GetBitmapResource( const wxString& name )
{
    return wxArtProvider::GetBitmap(name);
}

/*!
 * Get icon resources
 */

wxIcon CAgpExportPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAgpExportPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAgpExportPage icon retrieval
}

/*!
 * Transfer data to the window
 */

bool CAgpExportPage::TransferDataToWindow()
{
    wxChoice* choice = (wxChoice*)FindWindow(ID_CHOICE2);
    choice->SetStringSelection(ToWxString(GetData().m_GapType));

    return wxPanel::TransferDataToWindow();
}

/*!
 * Transfer data from the window
 */

bool CAgpExportPage::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    wxChoice* choice = (wxChoice*)FindWindow(ID_CHOICE2);
    GetData().m_GapType = ToStdString(choice->GetStringSelection());

    TConstScopedObjects selection;
    selection = m_LocationSel->GetSelection();
    m_data.m_SeqLoc = selection[0];

    wxString path = GetData().GetFileName();
    _ASSERT(m_SaveFile);
    return m_SaveFile->Validate(path);
}

static const char* kLocationList = ".LocationList";

void CAgpExportPage::SetRegistryPath(const string& path)
{
    m_RegPath = path;
    m_LocationSel->SetRegistryPath(m_RegPath + kLocationList);
}

void CAgpExportPage::SaveSettings() const
{
    m_LocationSel->SaveSettings();
}

void CAgpExportPage::LoadSettings()
{
    m_LocationSel->LoadSettings();
}

void CAgpExportPage::OnSelectFileClick( wxCommandEvent& WXUNUSED(event) )
{
    _ASSERT(m_SaveFile);
    m_SaveFile->ShowSaveDialog(CFileExtensions::kAGP);
}

END_NCBI_SCOPE
