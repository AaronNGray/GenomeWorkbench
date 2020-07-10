/*  $Id: ec_numbers_panel.cpp 25347 2012-03-01 18:22:54Z katargir $
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
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>
////@begin includes
#include "string_list_ctrl.hpp"
////@end includes

#include <gui/widgets/edit/serial_member_stringlist_validator.hpp>

#include "ec_numbers_panel.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CECNumbersPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CECNumbersPanel, wxPanel )


/*!
 * CECNumbersPanel event table definition
 */

BEGIN_EVENT_TABLE( CECNumbersPanel, wxPanel )

////@begin CECNumbersPanel event table entries
////@end CECNumbersPanel event table entries

END_EVENT_TABLE()


/*!
 * CECNumbersPanel constructors
 */

CECNumbersPanel::CECNumbersPanel()
{
    Init();
}

CECNumbersPanel::CECNumbersPanel( wxWindow* parent, CSerialObject& object, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Object(&object)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CECNumbersPanel creator
 */

bool CECNumbersPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CECNumbersPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CECNumbersPanel creation
    return true;
}


/*!
 * CECNumbersPanel destructor
 */

CECNumbersPanel::~CECNumbersPanel()
{
////@begin CECNumbersPanel destruction
////@end CECNumbersPanel destruction
}


/*!
 * Member initialisation
 */

void CECNumbersPanel::Init()
{
////@begin CECNumbersPanel member initialisation
////@end CECNumbersPanel member initialisation
}


/*!
 * Control creation for CECNumbersPanel
 */

void CECNumbersPanel::CreateControls()
{    
////@begin CECNumbersPanel content construction
    CECNumbersPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 0);

    CStringListCtrl* itemScrolledWindow4 = new CStringListCtrl( itemPanel1, ID_WINDOW, wxDefaultPosition, itemPanel1->ConvertDialogToPixels(wxSize(44, 37)), wxSUNKEN_BORDER|wxScrolledWindowStyle|wxTAB_TRAVERSAL );
    itemBoxSizer3->Add(itemScrolledWindow4, 1, wxGROW|wxALL, 0);

    // Set validators
    itemScrolledWindow4->SetValidator( CSerialStringListValidator(*m_Object, "ec") );
////@end CECNumbersPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CECNumbersPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CECNumbersPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CECNumbersPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CECNumbersPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CECNumbersPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CECNumbersPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CECNumbersPanel icon retrieval
}

END_NCBI_SCOPE

