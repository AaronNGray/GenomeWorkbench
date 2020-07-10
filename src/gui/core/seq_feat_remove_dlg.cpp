/*  $Id: seq_feat_remove_dlg.cpp 25493 2012-03-27 18:28:58Z kuznets $
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

#include "seq_feat_remove_dlg.hpp"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/radiobut.h>
#include <wx/icon.h>
#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/textctrl.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CSeqFeatRemoveDlg, wxDialog )

BEGIN_EVENT_TABLE( CSeqFeatRemoveDlg, wxDialog )

////@begin CSeqFeatRemoveDlg event table entries
////@end CSeqFeatRemoveDlg event table entries

END_EVENT_TABLE()

CSeqFeatRemoveDlg::CSeqFeatRemoveDlg()
{
    Init();
}

CSeqFeatRemoveDlg::CSeqFeatRemoveDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CSeqFeatRemoveDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSeqFeatRemoveDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSeqFeatRemoveDlg creation
    return true;
}

CSeqFeatRemoveDlg::~CSeqFeatRemoveDlg()
{
////@begin CSeqFeatRemoveDlg destruction
////@end CSeqFeatRemoveDlg destruction
}

void CSeqFeatRemoveDlg::Init()
{
////@begin CSeqFeatRemoveDlg member initialisation
////@end CSeqFeatRemoveDlg member initialisation
}

void CSeqFeatRemoveDlg::CreateControls()
{
////@begin CSeqFeatRemoveDlg content construction
    CSeqFeatRemoveDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Do you really want to delete the following:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl4 = new wxTextCtrl( itemDialog1, ID_TEXTCTRL1, wxT(""), wxDefaultPosition, itemDialog1->ConvertDialogToPixels(wxSize(250, 120)), wxTE_MULTILINE );
    itemBoxSizer2->Add(itemTextCtrl4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer5 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer5, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton6);

    wxButton* itemButton7 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton7);

    itemStdDialogButtonSizer5->Realize();

    // Set validators
    itemTextCtrl4->SetValidator( wxTextValidator(wxFILTER_NONE, & m_Text) );
////@end CSeqFeatRemoveDlg content construction
}

bool CSeqFeatRemoveDlg::ShowToolTips()
{
    return true;
}
wxBitmap CSeqFeatRemoveDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSeqFeatRemoveDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSeqFeatRemoveDlg bitmap retrieval
}
wxIcon CSeqFeatRemoveDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSeqFeatRemoveDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSeqFeatRemoveDlg icon retrieval
}

END_NCBI_SCOPE
