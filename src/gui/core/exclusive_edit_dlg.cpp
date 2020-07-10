/*  $Id: exclusive_edit_dlg.cpp 34313 2015-12-11 19:02:20Z katargir $
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

#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include "exclusive_edit_dlg.hpp"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CExclusiveEditDlg, wxDialog )

BEGIN_EVENT_TABLE( CExclusiveEditDlg, wxDialog )

////@begin CExclusiveEditDlg event table entries
    EVT_BUTTON( ID_ACTIVATE_WINDOW, CExclusiveEditDlg::OnActivateWindowClick )

////@end CExclusiveEditDlg event table entries

END_EVENT_TABLE()

CExclusiveEditDlg::CExclusiveEditDlg()
{
    Init();
}

CExclusiveEditDlg::CExclusiveEditDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CExclusiveEditDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CExclusiveEditDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CExclusiveEditDlg creation
    return true;
}

CExclusiveEditDlg::~CExclusiveEditDlg()
{
////@begin CExclusiveEditDlg destruction
////@end CExclusiveEditDlg destruction
}

void CExclusiveEditDlg::Init()
{
////@begin CExclusiveEditDlg member initialisation
    m_TextCtrl = NULL;
////@end CExclusiveEditDlg member initialisation
}

void CExclusiveEditDlg::CreateControls()
{    
////@begin CExclusiveEditDlg content construction
    CExclusiveEditDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_TextCtrl = new wxStaticText( itemDialog1, wxID_WINDOW_DESCRIPTION, _("Window \"%s\" has aquired exclusive edit access to the project:\n%s."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_TextCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, ID_ACTIVATE_WINDOW, _("Bring window to top"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CExclusiveEditDlg content construction

    if (m_ExclusiveEdit) {
        wxString title = m_ExclusiveEdit->GetLabel();
        wxString format = m_TextCtrl->GetLabel();
        m_TextCtrl->SetLabel(wxString::Format(format, title, m_ExclusiveDescr));
    }
}

bool CExclusiveEditDlg::ShowToolTips()
{
    return true;
}
wxBitmap CExclusiveEditDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CExclusiveEditDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CExclusiveEditDlg bitmap retrieval
}
wxIcon CExclusiveEditDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CExclusiveEditDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CExclusiveEditDlg icon retrieval
}

void CExclusiveEditDlg::OnActivateWindowClick( wxCommandEvent& event )
{
    if (m_ExclusiveEdit) {
        m_ExclusiveEdit->Raise();
    }
    EndDialog(wxID_CANCEL);
}

END_NCBI_SCOPE
