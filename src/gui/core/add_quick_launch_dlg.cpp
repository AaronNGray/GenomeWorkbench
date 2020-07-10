/*  $Id: add_quick_launch_dlg.cpp 26394 2012-09-11 17:31:43Z katargir $
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

////@begin includes
////@end includes

#include "add_quick_launch_dlg.hpp"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/icon.h>
#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/msgdlg.h>

#include <gui/widgets/wx/wx_utils.hpp>


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CAddQuickLaunchDlg, CDialog )

BEGIN_EVENT_TABLE( CAddQuickLaunchDlg, CDialog )

////@begin CAddQuickLaunchDlg event table entries
////@end CAddQuickLaunchDlg event table entries

END_EVENT_TABLE()

CAddQuickLaunchDlg::CAddQuickLaunchDlg()
{
    Init();
}

CAddQuickLaunchDlg::CAddQuickLaunchDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CAddQuickLaunchDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAddQuickLaunchDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAddQuickLaunchDlg creation
    return true;
}

CAddQuickLaunchDlg::~CAddQuickLaunchDlg()
{
////@begin CAddQuickLaunchDlg destruction
////@end CAddQuickLaunchDlg destruction
}

void CAddQuickLaunchDlg::Init()
{
    SetRegistryPath("Dialogs.CAddQuickLaunchDlg");
////@begin CAddQuickLaunchDlg member initialisation
    m_DescrCtrl = NULL;
////@end CAddQuickLaunchDlg member initialisation
}

void CAddQuickLaunchDlg::CreateControls()
{    
////@begin CAddQuickLaunchDlg content construction
    CAddQuickLaunchDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Please, enter short description of the parameter set:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_DescrCtrl = new wxTextCtrl( itemCDialog1, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_DescrCtrl, 0, wxGROW|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer5 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxButton* itemButton6 = new wxButton( itemCDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton6);

    wxButton* itemButton7 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton7);

    itemStdDialogButtonSizer5->Realize();

////@end CAddQuickLaunchDlg content construction

    m_DescrCtrl->SetFocus();
}

bool CAddQuickLaunchDlg::TransferDataFromWindow()
{
    if (!CDialog::TransferDataFromWindow())
        return false;

    wxTextCtrl* textCtrl = (wxTextCtrl*)FindWindow(ID_TEXTCTRL3);
    m_Descr = NStr::TruncateSpaces(ToStdString(textCtrl->GetValue()));

    if (m_Descr.empty()) {
        wxMessageBox(wxT("Please, enter non empty description"), wxT("Error"),
                     wxOK | wxICON_ERROR, this);
        m_DescrCtrl->SetFocus();
        return false;
    }

    return true;
}

bool CAddQuickLaunchDlg::ShowToolTips()
{
    return true;
}
wxBitmap CAddQuickLaunchDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAddQuickLaunchDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAddQuickLaunchDlg bitmap retrieval
}
wxIcon CAddQuickLaunchDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAddQuickLaunchDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAddQuickLaunchDlg icon retrieval
}

END_NCBI_SCOPE
