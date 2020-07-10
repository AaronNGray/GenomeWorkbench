/*  $Id: agp_load_page.cpp 38672 2017-06-07 21:13:40Z katargir $
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
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/radiobox.h>
#include <wx/valtext.h>
#include <wx/valgen.h>
#include <wx/stattext.h>
#include <wx/bmpbuttn.h>

#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/filedlg.h>
#include <wx/artprov.h>

////@begin includes
////@end includes

#include <gui/widgets/loaders/agp_load_page.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CAgpLoadPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAgpLoadPage, wxPanel )


/*!
 * CAgpLoadPage event table definition
 */

BEGIN_EVENT_TABLE( CAgpLoadPage, wxPanel )

////@begin CAgpLoadPage event table entries
    EVT_BUTTON( ID_BITMAPBUTTON1, CAgpLoadPage::OnFASTASeqsBrowse )

////@end CAgpLoadPage event table entries

END_EVENT_TABLE()


/*!
 * CAgpLoadPage constructors
 */

CAgpLoadPage::CAgpLoadPage()
{
    Init();
}

CAgpLoadPage::CAgpLoadPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CAgpLoadPage creator
 */

bool CAgpLoadPage::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAgpLoadPage creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAgpLoadPage creation
    return true;
}


/*!
 * CAgpLoadPage destructor
 */

CAgpLoadPage::~CAgpLoadPage()
{
////@begin CAgpLoadPage destruction
////@end CAgpLoadPage destruction
}


/*!
 * Member initialisation
 */

void CAgpLoadPage::Init()
{
////@begin CAgpLoadPage member initialisation
////@end CAgpLoadPage member initialisation
}


/*!
 * Control creation for CAgpLoadPage
 */

void CAgpLoadPage::CreateControls()
{
////@begin CAgpLoadPage content construction
    CAgpLoadPage* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("AGP Load Parameters"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText3->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxFONTWEIGHT_BOLD, wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName()));
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_LEFT|wxALL, 5);

    wxArrayString itemRadioBox5Strings;
    itemRadioBox5Strings.Add(_("Try to parse ID"));
    itemRadioBox5Strings.Add(_("Always make a local ID"));
    wxRadioBox* itemRadioBox5 = new wxRadioBox( itemPanel1, ID_RADIOBOX2, _("Component IDs"), wxDefaultPosition, wxDefaultSize, itemRadioBox5Strings, 1, wxRA_SPECIFY_COLS );
    itemRadioBox5->SetSelection(0);
    itemBoxSizer4->Add(itemRadioBox5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox6 = new wxCheckBox( itemPanel1, ID_CHECKBOX16, _("Set gap info"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox6->SetValue(false);
    itemBoxSizer4->Add(itemCheckBox6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("FASTA sequences"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemTextCtrl9, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton10 = new wxBitmapButton( itemPanel1, ID_BITMAPBUTTON1, itemPanel1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    itemBitmapButton10->SetHelpText(_("Select FASTA file"));
    if (CAgpLoadPage::ShowToolTips())
        itemBitmapButton10->SetToolTip(_("Select FASTA file"));
    itemBoxSizer7->Add(itemBitmapButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemRadioBox5->SetValidator( wxGenericValidator(& GetData().m_ParseIDs) );
    itemCheckBox6->SetValidator( wxGenericValidator(& GetData().m_SetGapInfo) );
    itemTextCtrl9->SetValidator( wxTextValidator(wxFILTER_NONE, & GetData().m_FastaFile) );
////@end CAgpLoadPage content construction
}


/*!
 * Should we show tooltips?
 */

bool CAgpLoadPage::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAgpLoadPage::GetBitmapResource( const wxString& name )
{
    return wxArtProvider::GetBitmap(name);
}

/*!
 * Get icon resources
 */

wxIcon CAgpLoadPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAgpLoadPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAgpLoadPage icon retrieval
}

/*!
 * Transfer data to the window
 */

bool CAgpLoadPage::TransferDataToWindow()
{
    return wxPanel::TransferDataToWindow();
}

/*!
 * Transfer data from the window
 */

bool CAgpLoadPage::TransferDataFromWindow()
{
    return wxPanel::TransferDataFromWindow();
}

void CAgpLoadPage::OnFASTASeqsBrowse( wxCommandEvent& WXUNUSED(event) )
{
    wxTextCtrl* textCtrl = (wxTextCtrl*)FindWindow(ID_TEXTCTRL6);

    wxString path = textCtrl->GetValue();

    wxFileDialog dlg(this, wxT("Select a FASTA file"), wxT(""), wxT(""), wxALL_FILES_PATTERN,
        wxFD_OPEN);

    dlg.SetPath(path);

    if (dlg.ShowModal() != wxID_OK)
        return;

    path = dlg.GetPath();
    textCtrl->SetValue(path);
}

END_NCBI_SCOPE
