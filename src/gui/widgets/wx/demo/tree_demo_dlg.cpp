/////////////////////////////////////////////////////////////////////////////
// Name:        tree_demo_dlg.cpp
// Purpose:
// Author:      Vladimir Tereshkov
// Modified by:
// Created:     05/02/2008 14:43:40
// RCS-ID:
// Copyright:
// Licence:
/////////////////////////////////////////////////////////////////////////////

#include <ncbi_pch.hpp>


#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>

////@begin includes
#include "wx/imaglist.h"
////@end includes

#include "tree_demo_dlg.hpp"
#include <gui/widgets/wx/treectrl_ex.hpp>

////@begin XPM images
////@end XPM images



BEGIN_NCBI_SCOPE


/*!
 * CTreeDemoDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CTreeDemoDlg, wxDialog )


/*!
 * CTreeDemoDlg event table definition
 */

BEGIN_EVENT_TABLE( CTreeDemoDlg, wxDialog )

////@begin CTreeDemoDlg event table entries
////@end CTreeDemoDlg event table entries

END_EVENT_TABLE()


/*!
 * CTreeDemoDlg constructors
 */

CTreeDemoDlg::CTreeDemoDlg()
{
    Init();
}

CTreeDemoDlg::CTreeDemoDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CTreeDemoDlg creator
 */

bool CTreeDemoDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CTreeDemoDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CTreeDemoDlg creation
    return true;
}


/*!
 * CTreeDemoDlg destructor
 */

CTreeDemoDlg::~CTreeDemoDlg()
{
////@begin CTreeDemoDlg destruction
////@end CTreeDemoDlg destruction
}


/*!
 * Member initialisation
 */

void CTreeDemoDlg::Init()
{
////@begin CTreeDemoDlg member initialisation
    m_Tree = NULL;
////@end CTreeDemoDlg member initialisation
}


/*!
 * Control creation for CTreeDemoDlg
 */

void CTreeDemoDlg::CreateControls()
{
////@begin CTreeDemoDlg content construction
    CTreeDemoDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_Tree = new CwxTreeCtrlEx( itemDialog1, ID_TREECTRL1, wxDefaultPosition, wxSize(300, 300), wxTR_HAS_BUTTONS |wxTR_MULTIPLE );
    itemBoxSizer2->Add(m_Tree, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticLine* itemStaticLine4 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine4, 0, wxGROW|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer5 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer5, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton6);

    wxButton* itemButton7 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton7);

    itemStdDialogButtonSizer5->Realize();

////@end CTreeDemoDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CTreeDemoDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CTreeDemoDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CTreeDemoDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CTreeDemoDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CTreeDemoDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CTreeDemoDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CTreeDemoDlg icon retrieval
}


END_NCBI_SCOPE
