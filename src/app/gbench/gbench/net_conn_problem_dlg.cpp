/*  $Id: net_conn_problem_dlg.cpp 38225 2017-04-12 14:24:56Z evgeniev $
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

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/icon.h>

#include "net_conn_problem_dlg.hpp"
#include <gui/widgets/feedback/feedback.hpp>

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CNetConnProblemDlg, wxDialog )

BEGIN_EVENT_TABLE( CNetConnProblemDlg, wxDialog )

////@begin CNetConnProblemDlg event table entries
    EVT_BUTTON( ID_BUTTON1, CNetConnProblemDlg::OnSendFeedback )

////@end CNetConnProblemDlg event table entries

END_EVENT_TABLE()

CNetConnProblemDlg::CNetConnProblemDlg()
{
    Init();
}

CNetConnProblemDlg::CNetConnProblemDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CNetConnProblemDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CNetConnProblemDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CNetConnProblemDlg creation
    return true;
}

CNetConnProblemDlg::~CNetConnProblemDlg()
{
////@begin CNetConnProblemDlg destruction
////@end CNetConnProblemDlg destruction
}

void CNetConnProblemDlg::Init()
{
////@begin CNetConnProblemDlg member initialisation
    m_DescrCtrl = NULL;
////@end CNetConnProblemDlg member initialisation
}

void CNetConnProblemDlg::CreateControls()
{    
////@begin CNetConnProblemDlg content construction
    CNetConnProblemDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Problem description:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALL, 5);

    m_DescrCtrl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(350, 200), wxTE_MULTILINE|wxTE_READONLY );
    itemBoxSizer2->Add(m_DescrCtrl, 1, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("                \"If you are using the application behind a corporate firewall,\n                please point administrator to the following page:\"\n              "), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl6 = new wxHyperlinkCtrl( itemDialog1, ID_HYPERLINKCTRL1, _("NCBI Network Configuration"), wxT("http://www.ncbi.nlm.nih.gov/IEB/ToolBox/NETWORK/firewall.html"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer2->Add(itemHyperlinkCtrl6, 0, wxALIGN_RIGHT|wxRIGHT, 10);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButton8 = new wxButton( itemDialog1, ID_BUTTON1, _("Send feedback..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemDialog1, wxID_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_DescrCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_Descr) );
////@end CNetConnProblemDlg content construction
}

bool CNetConnProblemDlg::ShowToolTips()
{
    return true;
}
wxBitmap CNetConnProblemDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CNetConnProblemDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CNetConnProblemDlg bitmap retrieval
}
wxIcon CNetConnProblemDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CNetConnProblemDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CNetConnProblemDlg icon retrieval
}

void CNetConnProblemDlg::OnSendFeedback( wxCommandEvent& WXUNUSED(event) )
{
    ShowFeedbackDialog(false);
}

END_NCBI_SCOPE
