/*  $Id: wx_marker_name_dlg.cpp 27016 2012-12-07 21:34:14Z wuliangs $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
////@end includes

#include "wx_marker_name_dlg.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CwxMarkerNameDlg, wxDialog )

BEGIN_EVENT_TABLE( CwxMarkerNameDlg, wxDialog )

////@begin CwxMarkerNameDlg event table entries
    EVT_BUTTON( wxID_OK, CwxMarkerNameDlg::OnOkClick )

////@end CwxMarkerNameDlg event table entries

END_EVENT_TABLE()

CwxMarkerNameDlg::CwxMarkerNameDlg()
{
    Init();
}

CwxMarkerNameDlg::CwxMarkerNameDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CwxMarkerNameDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CwxMarkerNameDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CwxMarkerNameDlg creation
    return true;
}


void CwxMarkerNameDlg::SetMarkerName(const string& name)
{
    m_MarkerName->SetValue(ToWxString(name));
}


string CwxMarkerNameDlg::GetMarkerName() const
{
    return ToStdString(m_MarkerName->GetValue());
}


CwxMarkerNameDlg::~CwxMarkerNameDlg()
{
////@begin CwxMarkerNameDlg destruction
////@end CwxMarkerNameDlg destruction
}

void CwxMarkerNameDlg::Init()
{
////@begin CwxMarkerNameDlg member initialisation
    m_Msg = NULL;
    m_MarkerName = NULL;
////@end CwxMarkerNameDlg member initialisation
}

void CwxMarkerNameDlg::CreateControls()
{    
////@begin CwxMarkerNameDlg content construction
    CwxMarkerNameDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_Msg = new wxStaticText( itemDialog1, wxID_STATIC, _("Please Input a New Marker Name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_Msg, 0, wxGROW|wxTOP|wxBOTTOM, 0);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("New name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_MarkerName = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemBoxSizer5->Add(m_MarkerName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer8 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer8, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton9 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton9->SetDefault();
    itemStdDialogButtonSizer8->AddButton(itemButton9);

    wxButton* itemButton10 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer8->AddButton(itemButton10);

    itemStdDialogButtonSizer8->Realize();

////@end CwxMarkerNameDlg content construction
}

void CwxMarkerNameDlg::OnOkClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in CwxMarkerNameDlg.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in CwxMarkerNameDlg. 
}

bool CwxMarkerNameDlg::ShowToolTips()
{
    return true;
}
wxBitmap CwxMarkerNameDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CwxMarkerNameDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CwxMarkerNameDlg bitmap retrieval
}
wxIcon CwxMarkerNameDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxMarkerNameDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxMarkerNameDlg icon retrieval
}

END_NCBI_SCOPE