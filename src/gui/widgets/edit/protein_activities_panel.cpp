/*  $Id: protein_activities_panel.cpp 25347 2012-03-01 18:22:54Z katargir $
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

#include "protein_activities_panel.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*!
 * CProteinActivitiesPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CProteinActivitiesPanel, wxPanel )


/*!
 * CProteinActivitiesPanel event table definition
 */

BEGIN_EVENT_TABLE( CProteinActivitiesPanel, wxPanel )

////@begin CProteinActivitiesPanel event table entries
////@end CProteinActivitiesPanel event table entries

END_EVENT_TABLE()


/*!
 * CProteinActivitiesPanel constructors
 */

CProteinActivitiesPanel::CProteinActivitiesPanel()
{
    Init();
}

CProteinActivitiesPanel::CProteinActivitiesPanel( wxWindow* parent, CSerialObject& object, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Object(&object)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CProteinActivitiesPanel creator
 */

bool CProteinActivitiesPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CProteinActivitiesPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CProteinActivitiesPanel creation
    return true;
}


/*!
 * CProteinActivitiesPanel destructor
 */

CProteinActivitiesPanel::~CProteinActivitiesPanel()
{
////@begin CProteinActivitiesPanel destruction
////@end CProteinActivitiesPanel destruction
}


/*!
 * Member initialisation
 */

void CProteinActivitiesPanel::Init()
{
////@begin CProteinActivitiesPanel member initialisation
////@end CProteinActivitiesPanel member initialisation
}


/*!
 * Control creation for CProteinActivitiesPanel
 */

void CProteinActivitiesPanel::CreateControls()
{    
////@begin CProteinActivitiesPanel content construction
    CProteinActivitiesPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 0);

    CStringListCtrl* itemScrolledWindow4 = new CStringListCtrl( itemPanel1, ID_WINDOW, wxDefaultPosition, itemPanel1->ConvertDialogToPixels(wxSize(66, 61)), wxSUNKEN_BORDER|wxScrolledWindowStyle|wxTAB_TRAVERSAL );
    itemBoxSizer3->Add(itemScrolledWindow4, 1, wxGROW|wxALL, 0);

    // Set validators
    itemScrolledWindow4->SetValidator( CSerialStringListValidator(*m_Object, "activity") );
////@end CProteinActivitiesPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CProteinActivitiesPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CProteinActivitiesPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CProteinActivitiesPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CProteinActivitiesPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CProteinActivitiesPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CProteinActivitiesPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CProteinActivitiesPanel icon retrieval
}

END_NCBI_SCOPE

