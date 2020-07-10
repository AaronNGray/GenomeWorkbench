/*  $Id: project_locked_dlg.cpp 31531 2014-10-20 19:56:06Z katargir $
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

#include "project_locked_dlg.hpp"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CProjectLockedDlg, wxDialog )

BEGIN_EVENT_TABLE( CProjectLockedDlg, wxDialog )

////@begin CProjectLockedDlg event table entries
////@end CProjectLockedDlg event table entries

END_EVENT_TABLE()

CProjectLockedDlg::CProjectLockedDlg()
{
    Init();
}

CProjectLockedDlg::CProjectLockedDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CProjectLockedDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CProjectLockedDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CProjectLockedDlg creation
    return true;
}

CProjectLockedDlg::~CProjectLockedDlg()
{
////@begin CProjectLockedDlg destruction
////@end CProjectLockedDlg destruction
}

void CProjectLockedDlg::Init()
{
////@begin CProjectLockedDlg member initialisation
////@end CProjectLockedDlg member initialisation
}

void CProjectLockedDlg::CreateControls()
{    
////@begin CProjectLockedDlg content construction
    CProjectLockedDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("The project you are trying to modify is locked by a view  or a tool.\n"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxButton* itemButton5 = new wxButton( itemDialog1, wxID_OK, _("Retry"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CProjectLockedDlg content construction
}

bool CProjectLockedDlg::ShowToolTips()
{
    return true;
}
wxBitmap CProjectLockedDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CProjectLockedDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CProjectLockedDlg bitmap retrieval
}
wxIcon CProjectLockedDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CProjectLockedDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CProjectLockedDlg icon retrieval
}

END_NCBI_SCOPE
