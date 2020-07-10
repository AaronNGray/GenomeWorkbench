/*  $Id: phylo_tree_demo_dlg.cpp 25623 2012-04-13 17:17:09Z katargir $
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
 * Authors:  Vladimir tereshkov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbitime.hpp>

#include "phylo_tree_demo_dlg.hpp"

#include <gui/widgets/phylo_tree/phylo_tree_widget.hpp>

// Register commands
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/commands.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

#include <objmgr/scope.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CPhyloTreeDemoDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CPhyloTreeDemoDlg, wxDialog )


/*!
 * CPhyloTreeDemoDlg event table definition
 */

BEGIN_EVENT_TABLE( CPhyloTreeDemoDlg, wxDialog )

////@begin CPhyloTreeDemoDlg event table entries
    EVT_CLOSE( CPhyloTreeDemoDlg::OnCloseWindow )

////@end CPhyloTreeDemoDlg event table entries

END_EVENT_TABLE()


/*!
 * CPhyloTreeDemoDlg constructors
 */

CPhyloTreeDemoDlg::CPhyloTreeDemoDlg()
{
    Init();
}

CPhyloTreeDemoDlg::CPhyloTreeDemoDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CPhyloTreeDemoDlg creator
 */

bool CPhyloTreeDemoDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CPhyloTreeDemoDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CPhyloTreeDemoDlg creation
    return true;
}


/*!
 * CPhyloTreeDemoDlg destructor
 */

CPhyloTreeDemoDlg::~CPhyloTreeDemoDlg()
{
////@begin CPhyloTreeDemoDlg destruction
////@end CPhyloTreeDemoDlg destruction
}


/*!
 * Member initialisation
 */

void CPhyloTreeDemoDlg::Init()
{
////@begin CPhyloTreeDemoDlg member initialisation
    m_Widget = NULL;
////@end CPhyloTreeDemoDlg member initialisation

    wxFileArtProvider* provider = GetDefaultFileArtProvider();
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    WidgetsWx_RegisterCommands(cmd_reg, *provider);
    CPhyloTreeWidget::RegisterCommands(cmd_reg, *provider);
}


/*!
 * Control creation for CPhyloTreeDemoDlg
 */

void CPhyloTreeDemoDlg::CreateControls()
{
////@begin CPhyloTreeDemoDlg content construction
    CPhyloTreeDemoDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_Widget = new CPhyloTreeWidget( itemDialog1, ID_PANEL1, wxDefaultPosition, wxSize(800, 600), wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_Widget, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, wxID_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CPhyloTreeDemoDlg content construction


    CPhyloPhylipReader  * reader = new CPhyloPhylipReader();
    CScope              * scope = NULL;


    // setting up scheme
    CPhyloTreeScheme * scheme = new CPhyloTreeScheme();
    //scheme->SetSize(CPhyloTreeScheme::eLineWidth) = 1;
    //scheme->SetSize(CPhyloTreeScheme::eNodeSize) = 3;

    //m_Widget->SetScheme(*scheme);
    //m_AlnWidget->SetPopupMenuItems(CreateMenuItems(PopupMenu));

    m_Widget->Create();
    m_Widget->SetDataSource(new CPhyloTreeDataSource(reader->GetTree(), *scope));
    m_Widget->SetScheme(*scheme);
}


/*!
 * wxEVT_CLOSE_WINDOW event handler for ID_CHITMATRIXDEMODLG
 */

void CPhyloTreeDemoDlg::OnCloseWindow( wxCloseEvent& event )
{
////@begin wxEVT_CLOSE_WINDOW event handler for ID_CHITMATRIXDEMODLG in CPhyloTreeDemoDlg.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_CLOSE_WINDOW event handler for ID_CHITMATRIXDEMODLG in CPhyloTreeDemoDlg.
}




/*!
 * Should we show tooltips?
 */

bool CPhyloTreeDemoDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CPhyloTreeDemoDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CPhyloTreeDemoDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CPhyloTreeDemoDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CPhyloTreeDemoDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CPhyloTreeDemoDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CPhyloTreeDemoDlg icon retrieval
}


END_NCBI_SCOPE
