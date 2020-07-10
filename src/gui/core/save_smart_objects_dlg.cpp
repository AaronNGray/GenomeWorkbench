/*  $Id: save_smart_objects_dlg.cpp 28638 2013-08-08 19:42:37Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include "save_smart_objects_dlg.hpp"

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CSaveSmartObjectsDlg, CDialog )

BEGIN_EVENT_TABLE( CSaveSmartObjectsDlg, CDialog )

////@begin CSaveSmartObjectsDlg event table entries
////@end CSaveSmartObjectsDlg event table entries

END_EVENT_TABLE()

CSaveSmartObjectsDlg::CSaveSmartObjectsDlg()
{
    Init();
}

CSaveSmartObjectsDlg::CSaveSmartObjectsDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CSaveSmartObjectsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSaveSmartObjectsDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSaveSmartObjectsDlg creation
    return true;
}

CSaveSmartObjectsDlg::~CSaveSmartObjectsDlg()
{
////@begin CSaveSmartObjectsDlg destruction
////@end CSaveSmartObjectsDlg destruction
}

void CSaveSmartObjectsDlg::Init()
{
////@begin CSaveSmartObjectsDlg member initialisation
    m_ProjectListrCtrl = NULL;
////@end CSaveSmartObjectsDlg member initialisation
}

void CSaveSmartObjectsDlg::CreateControls()
{    
////@begin CSaveSmartObjectsDlg content construction
    CSaveSmartObjectsDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Do you want to send edited Smart data to the Smart client\nand close corresponding projects?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_ProjectListrCtrlStrings;
    m_ProjectListrCtrl = new wxListBox( itemCDialog1, ID_LISTBOX, wxDefaultPosition, wxSize(-1, 100), m_ProjectListrCtrlStrings, wxLB_SINGLE );
    itemBoxSizer2->Add(m_ProjectListrCtrl, 0, wxGROW|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer5 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer5, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton6 = new wxButton( itemCDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton6);

    wxButton* itemButton7 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton7);

    itemStdDialogButtonSizer5->Realize();

////@end CSaveSmartObjectsDlg content construction
}

bool CSaveSmartObjectsDlg::ShowToolTips()
{
    return true;
}
wxBitmap CSaveSmartObjectsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSaveSmartObjectsDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSaveSmartObjectsDlg bitmap retrieval
}
wxIcon CSaveSmartObjectsDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSaveSmartObjectsDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSaveSmartObjectsDlg icon retrieval
}

END_NCBI_SCOPE
