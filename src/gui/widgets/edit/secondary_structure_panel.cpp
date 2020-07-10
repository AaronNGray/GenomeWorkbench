/*  $Id: secondary_structure_panel.cpp 28514 2013-07-23 17:29:46Z bollin $
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
////@end includes

#include <gui/widgets/edit/secondary_structure_panel.hpp>
#include <wx/sizer.h>
#include <wx/icon.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CSecondaryStructurePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSecondaryStructurePanel, wxPanel )


/*!
 * CSecondaryStructurePanel event table definition
 */

BEGIN_EVENT_TABLE( CSecondaryStructurePanel, wxPanel )

////@begin CSecondaryStructurePanel event table entries
////@end CSecondaryStructurePanel event table entries

END_EVENT_TABLE()


/*!
 * CSecondaryStructurePanel constructors
 */

CSecondaryStructurePanel::CSecondaryStructurePanel()
{
    Init();
}

CSecondaryStructurePanel::CSecondaryStructurePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CSecondaryStructurePanel creator
 */

bool CSecondaryStructurePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSecondaryStructurePanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSecondaryStructurePanel creation
    return true;
}


/*!
 * CSecondaryStructurePanel destructor
 */

CSecondaryStructurePanel::~CSecondaryStructurePanel()
{
////@begin CSecondaryStructurePanel destruction
////@end CSecondaryStructurePanel destruction
}


/*!
 * Member initialisation
 */

void CSecondaryStructurePanel::Init()
{
////@begin CSecondaryStructurePanel member initialisation
    m_Type = NULL;
////@end CSecondaryStructurePanel member initialisation
}


/*!
 * Control creation for CSecondaryStructurePanel
 */

void CSecondaryStructurePanel::CreateControls()
{    
////@begin CSecondaryStructurePanel content construction
    CSecondaryStructurePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxArrayString m_TypeStrings;
    m_TypeStrings.Add(_("Helix"));
    m_TypeStrings.Add(_("Sheet"));
    m_TypeStrings.Add(_("Turn"));
    m_Type = new wxChoice( itemPanel1, ID_CHOICE17, wxDefaultPosition, wxDefaultSize, m_TypeStrings, 0 );
    m_Type->SetStringSelection(_("Helix"));
    itemBoxSizer2->Add(m_Type, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CSecondaryStructurePanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CSecondaryStructurePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSecondaryStructurePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSecondaryStructurePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSecondaryStructurePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSecondaryStructurePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSecondaryStructurePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSecondaryStructurePanel icon retrieval
}


CSeqFeatData::EPsec_str CSecondaryStructurePanel::GetSecondaryStructure()
{
    CSeqFeatData::EPsec_str rval = CSeqFeatData::ePsec_str_helix;

    switch (m_Type->GetSelection()) {
        case 0:
            rval = CSeqFeatData::ePsec_str_helix;
            break;
        case 1:
            rval = CSeqFeatData::ePsec_str_sheet;
            break;
        case 2:
            rval = CSeqFeatData::ePsec_str_turn;
            break;
    }
    return rval;
}


void CSecondaryStructurePanel::SetSecondaryStructure(objects::CSeqFeatData::EPsec_str struc)
{
    switch (struc) {
        case CSeqFeatData::ePsec_str_helix:
            m_Type->SetSelection(0);
            break;
        case CSeqFeatData::ePsec_str_sheet:
            m_Type->SetSelection(1);
            break;
        case CSeqFeatData::ePsec_str_turn:
            m_Type->SetSelection(2);
            break;
        default:
            m_Type->SetSelection(0);
            break;
    }
}

END_NCBI_SCOPE
