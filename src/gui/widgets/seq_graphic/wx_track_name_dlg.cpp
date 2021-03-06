/*  $Id: wx_track_name_dlg.cpp 23952 2011-06-23 19:12:28Z wuliangs $
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
 * Authors:
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>////@begin includes
////@end includes

#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/button.h>

#include "wx_track_name_dlg.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS( CwxTrackNameDlg, wxDialog )

BEGIN_EVENT_TABLE( CwxTrackNameDlg, wxDialog )

////@begin CwxTrackNameDlg event table entries
    EVT_BUTTON( wxID_OK, CwxTrackNameDlg::OnOkClick )

////@end CwxTrackNameDlg event table entries

END_EVENT_TABLE()

CwxTrackNameDlg::CwxTrackNameDlg()
{
    Init();
}

CwxTrackNameDlg::CwxTrackNameDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    CreateX(parent, id, caption, pos, size, style);
}

bool CwxTrackNameDlg::CreateX( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CwxTrackNameDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CwxTrackNameDlg creation
    return true;
}

CwxTrackNameDlg::~CwxTrackNameDlg()
{
////@begin CwxTrackNameDlg destruction
////@end CwxTrackNameDlg destruction
}

void CwxTrackNameDlg::Init()
{
////@begin CwxTrackNameDlg member initialisation
    m_Msg = NULL;
    m_TrackName = NULL;
////@end CwxTrackNameDlg member initialisation
}

void CwxTrackNameDlg::CreateControls()
{    
////@begin CwxTrackNameDlg content construction
    // Generated by DialogBlocks, 10/05/2011 16:07:11 (unregistered)

    CwxTrackNameDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_Msg = new wxStaticText( itemDialog1, wxID_STATIC, _("Please input a track title/name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_Msg, 0, wxGROW|wxTOP|wxBOTTOM, 0);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Track title:"), wxDefaultPosition, wxDefaultSize, 0 );
    if (CwxTrackNameDlg::ShowToolTips())
        itemStaticText6->SetToolTip(_("Track title/name shown on the track title bar. It doesn't need to be unique, but a unique name may help differentiate it from others."));
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_TrackName = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    if (CwxTrackNameDlg::ShowToolTips())
        m_TrackName->SetToolTip(_("Track title/name shown on the track title bar. It doesn't need to be unique, but a unique name may help differentiate it from others."));
    itemBoxSizer5->Add(m_TrackName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer8 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer8, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton9 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton9->SetDefault();
    itemStdDialogButtonSizer8->AddButton(itemButton9);

    wxButton* itemButton10 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer8->AddButton(itemButton10);

    itemStdDialogButtonSizer8->Realize();

////@end CwxTrackNameDlg content construction
}

bool CwxTrackNameDlg::ShowToolTips()
{
    return true;
}
wxBitmap CwxTrackNameDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CwxTrackNameDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CwxTrackNameDlg bitmap retrieval
}
wxIcon CwxTrackNameDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxTrackNameDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxTrackNameDlg icon retrieval
}

void CwxTrackNameDlg::OnOkClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in CwxTrackNameDlg.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in CwxTrackNameDlg. 
}

void CwxTrackNameDlg::SetMsg(const wxString& msg)
{
    m_Msg->SetLabel(msg);
}

void CwxTrackNameDlg::SetTrackName(const wxString& name)
{
    m_TrackName->SetValue(name);
}

wxString CwxTrackNameDlg::GetTrackName() const
{
    return m_TrackName->GetValue();
}

END_NCBI_SCOPE
