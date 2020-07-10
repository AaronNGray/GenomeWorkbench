/* $Id: rename_column_dlg.cpp 34945 2016-03-03 20:31:51Z asztalos $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Igor Filippov
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/packages/pkg_sequence_edit/rename_column_dlg.hpp>

#include <wx/button.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS( CRenameColumnDlg, wxDialog )



BEGIN_EVENT_TABLE( CRenameColumnDlg, wxDialog )

////@begin CRenameColumnDlg event table entries
////@end CRenameColumnDlg event table entries

END_EVENT_TABLE()


/*!
 * CRenameColumnDlg constructors
 */

CRenameColumnDlg::CRenameColumnDlg()
{
    Init();
}

CRenameColumnDlg::CRenameColumnDlg( wxWindow* parent, wxArrayString& srcModNameStrings,
				wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : m_srcModNameStrings(srcModNameStrings)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CRenameColumnDlg creator
 */

bool CRenameColumnDlg::Create( wxWindow* parent, 
			     wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRenameColumnDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRenameColumnDlg creation

    return true;
}


/*!
 * CRenameColumnDlg destructor
 */

CRenameColumnDlg::~CRenameColumnDlg()
{
////@begin CRenameColumnDlg destruction
////@end CRenameColumnDlg destruction
}


/*!
 * Member initialisation
 */

void CRenameColumnDlg::Init()
{
////@begin CRenameColumnDlg member initialisation
    m_QualList = NULL;
////@end CRenameColumnDlg member initialisation
}


/*!
 * Control creation for CRenameColumnDlg
 */

void CRenameColumnDlg::CreateControls()
{    
////@begin CRenameColumnDlg content construction
    CRenameColumnDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_QualList = new wxChoice( itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_srcModNameStrings, 0 );
    itemBoxSizer2->Add(m_QualList, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    if (!m_srcModNameStrings.IsEmpty())
        m_QualList->SetSelection(0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxEXPAND|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CRenameColumnDlg content construction
    
}


/*!
 * Should we show tooltips?
 */

bool CRenameColumnDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRenameColumnDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRenameColumnDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRenameColumnDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CRenameColumnDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRenameColumnDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRenameColumnDlg icon retrieval
}

wxString CRenameColumnDlg::GetSelection()
{
    if (m_QualList)
        return m_QualList->GetStringSelection();
    else
        return wxEmptyString;
}

END_NCBI_SCOPE
