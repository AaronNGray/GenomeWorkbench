/*  $Id: synonyms_panel.cpp 25028 2012-01-12 17:26:13Z katargir $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>

////@begin includes
#include "string_list_ctrl.hpp"
////@end includes

#include <gui/widgets/edit/serial_member_stringlist_validator.hpp>

#include "synonyms_panel.hpp"

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images


/*!
 * CSynonymsPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSynonymsPanel, wxPanel )


/*!
 * CSynonymsPanel event table definition
 */

BEGIN_EVENT_TABLE( CSynonymsPanel, wxPanel )

////@begin CSynonymsPanel event table entries
////@end CSynonymsPanel event table entries

END_EVENT_TABLE()


/*!
 * CSynonymsPanel constructors
 */

CSynonymsPanel::CSynonymsPanel() : m_Object(0)
{
    Init();
}

CSynonymsPanel::CSynonymsPanel( wxWindow* parent, CSerialObject& object, objects::CScope&,
                                wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Object(&object)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CSynonymsPanel creator
 */

bool CSynonymsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSynonymsPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSynonymsPanel creation
    return true;
}


/*!
 * CSynonymsPanel destructor
 */

CSynonymsPanel::~CSynonymsPanel()
{
////@begin CSynonymsPanel destruction
////@end CSynonymsPanel destruction
}


/*!
 * Member initialisation
 */

void CSynonymsPanel::Init()
{
////@begin CSynonymsPanel member initialisation
////@end CSynonymsPanel member initialisation
}


/*!
 * Control creation for CSynonymsPanel
 */

void CSynonymsPanel::CreateControls()
{
////@begin CSynonymsPanel content construction
    CSynonymsPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 0);

    CStringListCtrl* itemScrolledWindow4 = new CStringListCtrl( itemPanel1, ID_WINDOW, wxDefaultPosition, itemPanel1->ConvertDialogToPixels(wxSize(100, 100)), wxSUNKEN_BORDER|wxScrolledWindowStyle|wxTAB_TRAVERSAL );
    itemBoxSizer3->Add(itemScrolledWindow4, 1, wxGROW|wxALL, 0);

    // Set validators
    itemScrolledWindow4->SetValidator( CSerialStringListValidator(*m_Object, "syn") );
////@end CSynonymsPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CSynonymsPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSynonymsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSynonymsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSynonymsPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSynonymsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSynonymsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSynonymsPanel icon retrieval
}

END_NCBI_SCOPE
