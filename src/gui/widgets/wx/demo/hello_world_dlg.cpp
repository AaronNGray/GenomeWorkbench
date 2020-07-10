/////////////////////////////////////////////////////////////////////////////
// Name:        hello_world_dlg.cpp
// Purpose:
// Author:      Roman Katargin
// Modified by:
// Created:     06/08/2007 16:56:20
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
////@end includes

#include "hello_world_dlg.hpp"

////@begin XPM images
////@end XPM images


BEGIN_NCBI_SCOPE

/*!
 * CHelloWorldDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CHelloWorldDlg, wxDialog )


/*!
 * CHelloWorldDlg event table definition
 */

BEGIN_EVENT_TABLE( CHelloWorldDlg, wxDialog )

////@begin CHelloWorldDlg event table entries
////@end CHelloWorldDlg event table entries

END_EVENT_TABLE()


/*!
 * CHelloWorldDlg constructors
 */

CHelloWorldDlg::CHelloWorldDlg()
{
    Init();
}

CHelloWorldDlg::CHelloWorldDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CHelloWorldDlg creator
 */

bool CHelloWorldDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CHelloWorldDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CHelloWorldDlg creation
    return true;
}


/*!
 * CHelloWorldDlg destructor
 */

CHelloWorldDlg::~CHelloWorldDlg()
{
////@begin CHelloWorldDlg destruction
////@end CHelloWorldDlg destruction
}


/*!
 * Member initialisation
 */

void CHelloWorldDlg::Init()
{
////@begin CHelloWorldDlg member initialisation
////@end CHelloWorldDlg member initialisation
}


/*!
 * Control creation for CHelloWorldDlg
 */

void CHelloWorldDlg::CreateControls()
{
////@begin CHelloWorldDlg content construction
    CHelloWorldDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Hello World!"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer2->Add(50, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, wxID_CANCEL, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CHelloWorldDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CHelloWorldDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CHelloWorldDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CHelloWorldDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CHelloWorldDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CHelloWorldDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CHelloWorldDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CHelloWorldDlg icon retrieval
}

END_NCBI_SCOPE
