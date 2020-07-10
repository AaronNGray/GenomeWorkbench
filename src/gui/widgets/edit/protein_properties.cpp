/*  $Id: protein_properties.cpp 39095 2017-07-27 19:56:04Z filippov $
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
#include "protein_properties.hpp"
#include "ec_numbers_panel.hpp"
#include "protein_activities_panel.hpp"
#include "comment_panel.hpp"


#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/statbox.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*!
 * CProteinPropertiesPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CProteinPropertiesPanel, wxPanel )


/*!
 * CProteinPropertiesPanel event table definition
 */

BEGIN_EVENT_TABLE( CProteinPropertiesPanel, wxPanel )

////@begin CProteinPropertiesPanel event table entries
////@end CProteinPropertiesPanel event table entries

END_EVENT_TABLE()


/*!
 * CProteinPropertiesPanel constructors
 */

CProteinPropertiesPanel::CProteinPropertiesPanel() : m_Object(0)
{
    Init();
}

CProteinPropertiesPanel::CProteinPropertiesPanel( wxWindow* parent, CSerialObject& object,  objects::CSeq_feat* edited_protein,
                              wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Object(&object), m_edited_protein(edited_protein)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * Commentpanel creator
 */

bool CProteinPropertiesPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CProteinPropertiesPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CProteinPropertiesPanel creation
    return true;
}


/*!
 * CProteinPropertiesPanel destructor
 */

CProteinPropertiesPanel::~CProteinPropertiesPanel()
{
////@begin CProteinPropertiesPanel destruction
////@end CProteinPropertiesPanel destruction
}


/*!
 * Member initialisation
 */

void CProteinPropertiesPanel::Init()
{
////@begin CProteinPropertiesPanel member initialisation
////@end CProteinPropertiesPanel member initialisation
}


/*!
 * Control creation for Commentpanel
 */

void CProteinPropertiesPanel::CreateControls()
{
////@begin CProteinPropertiesPanel content construction
    CProteinPropertiesPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Enzyme Commission Numbers"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 1, wxGROW|wxALL, 5);

    CECNumbersPanel *panel1 = new CECNumbersPanel(itemPanel1, *m_Object);
    itemStaticBoxSizer3->Add(panel1, 1, wxGROW|wxALL, 0);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Activity"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer4, 1, wxGROW|wxALL, 5);

    CProteinActivitiesPanel *panel2 = new CProteinActivitiesPanel(itemPanel1, *m_Object);
    itemStaticBoxSizer4->Add(panel2, 1, wxGROW|wxALL, 0);

    if (m_edited_protein)
    {
        wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Protein Comment"));
        wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxHORIZONTAL);
        itemBoxSizer2->Add(itemStaticBoxSizer5, 1, wxGROW|wxALL, 5);
        
        CCommentPanel *panel3 = new CCommentPanel(itemPanel1, *m_edited_protein);
        itemStaticBoxSizer5->Add(panel3, 1, wxGROW|wxALL, 0);
    }

////@end CProteinPropertiesPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CProteinPropertiesPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CProteinPropertiesPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CProteinPropertiesPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CProteinPropertiesPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CProteinPropertiesPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CProteinPropertiesPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CProteinPropertiesPanel icon retrieval
}



END_NCBI_SCOPE

