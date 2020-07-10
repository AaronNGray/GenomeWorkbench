/*  $Id: cds_protein_panel.cpp 28136 2013-05-15 15:07:51Z bollin $
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

#include "cds_protein_panel.hpp"

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>
#include <gui/widgets/edit/serial_member_stringlist_validator.hpp>
#include <serial/serialbase.hpp>
#include <serial/typeinfo.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CCDSProteinPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CCDSProteinPanel, wxPanel )


/*!
 * CCDSProteinPanel event table definition
 */

BEGIN_EVENT_TABLE( CCDSProteinPanel, wxPanel )

////@begin CCDSProteinPanel event table entries
////@end CCDSProteinPanel event table entries

END_EVENT_TABLE()


/*!
 * CCDSProteinPanel constructors
 */

CCDSProteinPanel::CCDSProteinPanel()
{
    Init();
}

CCDSProteinPanel::CCDSProteinPanel( wxWindow* parent, CSerialObject& object, objects::CScope&,
                                    wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Object(0)
{
    m_Object = &object;
    _ASSERT(m_Object->GetThisTypeInfo()->GetName() == "Prot-ref");
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CCDSProteinPanel creator
 */

bool CCDSProteinPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CCDSProteinPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CCDSProteinPanel creation
    return true;
}


/*!
 * CCDSProteinPanel destructor
 */

CCDSProteinPanel::~CCDSProteinPanel()
{
////@begin CCDSProteinPanel destruction
////@end CCDSProteinPanel destruction
}


/*!
 * Member initialisation
 */

void CCDSProteinPanel::Init()
{
////@begin CCDSProteinPanel member initialisation
    m_AdjustmRNAProductName = NULL;
////@end CCDSProteinPanel member initialisation
}


/*!
 * Control creation for CCDSProteinPanel
 */

void CCDSProteinPanel::CreateControls()
{
////@begin CCDSProteinPanel content construction
    CCDSProteinPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(2, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Protein Name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_RIGHT|wxALIGN_TOP|wxALL, 5);

    CStringListCtrl* itemScrolledWindow5 = new CStringListCtrl( itemPanel1, ID_WINDOW1, wxDefaultPosition, itemPanel1->ConvertDialogToPixels(wxSize(100, 100)), wxDOUBLE_BORDER );
    itemFlexGridSizer3->Add(itemScrolledWindow5, 1, wxGROW|wxGROW|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Description"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl7 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL12, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl7, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->AddGrowableRow(0);
    itemFlexGridSizer3->AddGrowableCol(1);

    wxStaticLine* itemStaticLine8 = new wxStaticLine( itemPanel1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine8, 0, wxGROW|wxALL, 5);

    m_AdjustmRNAProductName = new wxCheckBox( itemPanel1, ID_CHECKBOX4, _("Make overlapping mRNA product match protein name"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AdjustmRNAProductName->SetValue(false);
    itemBoxSizer2->Add(m_AdjustmRNAProductName, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    // Set validators
    itemScrolledWindow5->SetValidator( CSerialStringListValidator(*m_Object, "name", true, "protein name") );
    itemTextCtrl7->SetValidator( CSerialTextValidator(*m_Object, "desc") );
////@end CCDSProteinPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CCDSProteinPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCDSProteinPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCDSProteinPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCDSProteinPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCDSProteinPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCDSProteinPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCDSProteinPanel icon retrieval
}


END_NCBI_SCOPE
