/*  $Id: export_tree_dlg.cpp 39318 2017-09-12 16:00:18Z evgeniev $
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
#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/filedlg.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/statbox.h>
#include <wx/artprov.h>
#include <wx/msgdlg.h>
#include <wx/file.h>

#include <gui/packages/pkg_alignment/export_tree_dlg.hpp>

BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images

IMPLEMENT_DYNAMIC_CLASS( CExportTreeDlg, wxDialog )

BEGIN_EVENT_TABLE( CExportTreeDlg, wxDialog )

////@begin CExportTreeDlg event table entries
    EVT_BUTTON( ID_FILEOPEN_BTN, CExportTreeDlg::OnSelectFileClick )
////@end CExportTreeDlg event table entries

END_EVENT_TABLE()

CExportTreeDlg::CExportTreeDlg()
{
    Init();
}

CExportTreeDlg::CExportTreeDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CExportTreeDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CExportTreeDlg creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CExportTreeDlg creation
    return true;
}

CExportTreeDlg::~CExportTreeDlg()
{
////@begin CExportTreeDlg destruction
////@end CExportTreeDlg destruction
}

void CExportTreeDlg::Init()
{
////@begin CExportTreeDlg member initialisation
    m_txtFileName = NULL;
    m_FormatChoice = NULL;
////@end CExportTreeDlg member initialisation
}

void CExportTreeDlg::CreateControls()
{    
////@begin CExportTreeDlg content construction
    CExportTreeDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("File Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_txtFileName = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemBoxSizer3->Add(m_txtFileName, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton6 = new wxBitmapButton( itemDialog1, ID_FILEOPEN_BTN, itemDialog1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    if (CExportTreeDlg::ShowToolTips())
        itemBitmapButton6->SetToolTip(_("Select File"));
    itemBoxSizer3->Add(itemBitmapButton6, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Export Tree Format"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_FormatChoiceStrings;
    m_FormatChoiceStrings.Add(_("Newick"));
    m_FormatChoiceStrings.Add(_("Nexus"));
    m_FormatChoice = new wxChoice( itemDialog1, ID_FORMAT_CHOICE, wxDefaultPosition, wxDefaultSize, m_FormatChoiceStrings, 0 );
    m_FormatChoice->SetStringSelection(_("Newick"));
    itemBoxSizer7->Add(m_FormatChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer10 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer10, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton11 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer10->AddButton(itemButton11);

    wxButton* itemButton12 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer10->AddButton(itemButton12);

    itemStdDialogButtonSizer10->Realize();

    // Set validators
    m_txtFileName->SetValidator( wxTextValidator(wxFILTER_NONE, & m_FileName) );
////@end CExportTreeDlg content construction

    m_SaveFile.reset(new CSaveFileHelper(this, *m_txtFileName));
}

bool CExportTreeDlg::ShowToolTips()
{
    return true;
}

void CExportTreeDlg::OnSelectFileClick( wxCommandEvent& event )
{
    CFileExtensions::EFileType file_type;

    if (m_FormatChoice != NULL) {
        wxString s = m_FormatChoice->GetStringSelection();
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

bool CExportTreeDlg::TransferDataToWindow()
{
    m_FormatChoice->SetStringSelection(m_data.GetFileFormat());

    return wxDialog::TransferDataToWindow();
}

bool CExportTreeDlg::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow())
        return false;

    m_data.SetFileFormat(m_FormatChoice->GetStringSelection());

    wxString path(m_txtFileName->GetValue());
    _ASSERT(m_SaveFile);
    if (!m_SaveFile->Validate(path))
        return false;
    m_data.SetFileName(path);

    return true;
}

wxBitmap CExportTreeDlg::GetBitmapResource(const wxString& name)
{
    return wxArtProvider::GetBitmap(name);
}

wxIcon CExportTreeDlg::GetIconResource(const wxString& name)
{
    // Icon retrieval
    ////@begin CExportTreeDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
    ////@end CExportTreeDlg icon retrieval
}


END_NCBI_SCOPE
