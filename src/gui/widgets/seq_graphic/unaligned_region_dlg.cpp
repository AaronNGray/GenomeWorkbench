/*  $Id: unaligned_region_dlg.cpp 36270 2016-09-02 19:17:13Z evgeniev $
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
 * File Description: Dialog, displaying unaligned regions.
 *
 */


#include <ncbi_pch.hpp>
#include <corelib/ncbidbg.hpp>

////@begin includes
////@end includes

#include "unaligned_region_dlg.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CUnalignedRegionDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CUnalignedRegionDlg, wxDialog )


/*!
 * CUnalignedRegionDlg event table definition
 */

BEGIN_EVENT_TABLE( CUnalignedRegionDlg, wxDialog )

////@begin CUnalignedRegionDlg event table entries
////@end CUnalignedRegionDlg event table entries

END_EVENT_TABLE()


/*!
 * CUnalignedRegionDlg constructors
 */

CUnalignedRegionDlg::CUnalignedRegionDlg()
{
    Init();
}

CUnalignedRegionDlg::CUnalignedRegionDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CUnalignedRegionDlg creator
 */

bool CUnalignedRegionDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CUnalignedRegionDlg creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CUnalignedRegionDlg creation
    return true;
}


/*!
 * CUnalignedRegionDlg destructor
 */

CUnalignedRegionDlg::~CUnalignedRegionDlg()
{
////@begin CUnalignedRegionDlg destruction
////@end CUnalignedRegionDlg destruction
}


/*!
 * Member initialisation
 */

void CUnalignedRegionDlg::Init()
{
////@begin CUnalignedRegionDlg member initialisation
    m_SequenceTextCtrl = NULL;
////@end CUnalignedRegionDlg member initialisation
}


/*!
 * Control creation for CUnalignedRegionDlg
 */

void CUnalignedRegionDlg::CreateControls()
{    
////@begin CUnalignedRegionDlg content construction
    CUnalignedRegionDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_SequenceTextCtrl = new wxRichTextCtrl( itemDialog1, ID_SEQUENCE_RICHTEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(400, 300), wxTE_READONLY|wxWANTS_CHARS );
    itemBoxSizer2->Add(m_SequenceTextCtrl, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, wxID_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CUnalignedRegionDlg content construction
}

void CUnalignedRegionDlg::WriteNormalText(const wxString &text)
{
    _ASSERT(m_SequenceTextCtrl);

    m_SequenceTextCtrl->BeginStyle(m_SequenceTextCtrl->GetDefaultStyle());
    m_SequenceTextCtrl->WriteText(text);
    m_SequenceTextCtrl->EndStyle();
}

void CUnalignedRegionDlg::WriteRedText(const wxString &text)
{
    _ASSERT(m_SequenceTextCtrl);

    wxTextAttr unalignedStyle (m_SequenceTextCtrl->GetDefaultStyle());
    wxColor color(196, 135, 147); // HTML pink3
    unalignedStyle.SetBackgroundColour(color);

    m_SequenceTextCtrl->BeginStyle(unalignedStyle);
    m_SequenceTextCtrl->WriteText(text);
    m_SequenceTextCtrl->EndStyle();
}

/*!
 * Should we show tooltips?
 */

bool CUnalignedRegionDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CUnalignedRegionDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CUnalignedRegionDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CUnalignedRegionDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CUnalignedRegionDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CUnalignedRegionDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CUnalignedRegionDlg icon retrieval
}

END_NCBI_SCOPE