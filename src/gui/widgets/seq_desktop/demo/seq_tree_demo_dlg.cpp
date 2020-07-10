/*  $Id: seq_tree_demo_dlg.cpp 28407 2013-07-03 19:44:45Z katargir $
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
 * Authors:  
 */


#include <ncbi_pch.hpp>

#include <wx/sizer.h>

////@begin includes
////@end includes

#include "seq_tree_demo_dlg.hpp"

#include <gui/widgets/seq_tree/seq_tree_widget.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CSeqTreeDemoDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSeqTreeDemoDlg, wxDialog )


/*!
 * CSeqTreeDemoDlg event table definition
 */

BEGIN_EVENT_TABLE( CSeqTreeDemoDlg, wxDialog )

////@begin CSeqTreeDemoDlg event table entries
    EVT_CLOSE( CSeqTreeDemoDlg::OnCloseWindow )

    EVT_BUTTON( wxID_CANCEL, CSeqTreeDemoDlg::OnCancelClick )

////@end CSeqTreeDemoDlg event table entries

END_EVENT_TABLE()


/*!
 * CSeqTreeDemoDlg constructors
 */

CSeqTreeDemoDlg::CSeqTreeDemoDlg()
{
    Init();
}

CSeqTreeDemoDlg::CSeqTreeDemoDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CSeqTreeDemoDlg creator
 */

bool CSeqTreeDemoDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSeqTreeDemoDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSeqTreeDemoDlg creation
    return true;
}


/*!
 * CSeqTreeDemoDlg destructor
 */

CSeqTreeDemoDlg::~CSeqTreeDemoDlg()
{
////@begin CSeqTreeDemoDlg destruction
////@end CSeqTreeDemoDlg destruction
}


/*!
 * Member initialisation
 */

void CSeqTreeDemoDlg::Init()
{
////@begin CSeqTreeDemoDlg member initialisation
    m_Widget = NULL;
////@end CSeqTreeDemoDlg member initialisation
}


/*!
 * Control creation for CSeqTreeDemoDlg
 */

void CSeqTreeDemoDlg::CreateControls()
{    
////@begin CSeqTreeDemoDlg content construction
    CSeqTreeDemoDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_Widget = new CSeqTreeWidget( itemDialog1, ID_FOREIGN, wxDefaultPosition, wxSize(300, 300), wxSIMPLE_BORDER );
    itemBoxSizer2->Add(m_Widget, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CSeqTreeDemoDlg content construction

    m_Widget->Create();
    m_Widget->ZoomRect(TModelRect(0.0, 0.0, 200.0, 200.0));
}


/*!
 * Should we show tooltips?
 */

bool CSeqTreeDemoDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSeqTreeDemoDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSeqTreeDemoDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSeqTreeDemoDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSeqTreeDemoDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSeqTreeDemoDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSeqTreeDemoDlg icon retrieval
}

void CSeqTreeDemoDlg::OnCancelClick( wxCommandEvent& event )
{
    Destroy();
}

void CSeqTreeDemoDlg::OnCloseWindow( wxCloseEvent& event )
{
    Destroy();
}

END_NCBI_SCOPE
