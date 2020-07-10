/*  $Id: gene_panel.cpp 38906 2017-07-05 16:52:18Z bollin $
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
////@end includes

#include "gene_panel.hpp"
#include <objects/seqfeat/Gene_ref.hpp>
#include <gui/widgets/edit/serial_member_primitive_validators.hpp>
#include <serial/serialbase.hpp>
#include <serial/typeinfo.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE


////@begin XPM images
////@end XPM images


/*!
 * CGenePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CGenePanel, wxPanel )


/*!
 * CGenePanel event table definition
 */

BEGIN_EVENT_TABLE( CGenePanel, wxPanel )

////@begin CGenePanel event table entries
////@end CGenePanel event table entries

END_EVENT_TABLE()

/*!
 * CGenePanel constructors
 */

 CGenePanel::CGenePanel() : m_Object(0)
{
    Init();
}

CGenePanel::CGenePanel( wxWindow* parent, CSerialObject& object, objects::CScope&,
                        wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Object(0)
{
    m_Object = &object;
    _ASSERT(m_Object->GetThisTypeInfo()->GetName() == "Gene-ref");
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CGenePanel creator
 */

bool CGenePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGenePanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGenePanel creation
    return true;
}


/*!
 * CGenePanel destructor
 */

CGenePanel::~CGenePanel()
{
////@begin CGenePanel destruction
////@end CGenePanel destruction
}


/*!
 * Member initialisation
 */

void CGenePanel::Init()
{
////@begin CGenePanel member initialisation
////@end CGenePanel member initialisation
}


/*!
 * Control creation for CGenePanel
 */

void CGenePanel::CreateControls()
{
////@begin CGenePanel content construction
    CGenePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Locus"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxTextCtrl* itemTextCtrl5 = new wxTextCtrl( itemPanel1, ID_TEXT1_GENEPNL, wxEmptyString, wxDefaultPosition, wxSize(itemPanel1->ConvertDialogToPixels(wxSize(150, -1)).x, -1), 0 );
    itemFlexGridSizer3->Add(itemTextCtrl5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Allele"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxTextCtrl* itemTextCtrl7 = new wxTextCtrl( itemPanel1, ID_TEXT2_GENEPNL, wxEmptyString, wxDefaultPosition, wxSize(itemPanel1->ConvertDialogToPixels(wxSize(150, -1)).x, -1), 0 );
    itemFlexGridSizer3->Add(itemTextCtrl7, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Description"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemPanel1, ID_TEXT3_GENEPNL, wxEmptyString, wxDefaultPosition, wxSize(itemPanel1->ConvertDialogToPixels(wxSize(150, -1)).x, -1), 0 );
    itemFlexGridSizer3->Add(itemTextCtrl9, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel1, wxID_STATIC, _("Locus Tag"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxTextCtrl* itemTextCtrl13 = new wxTextCtrl( itemPanel1, ID_TEXT5_GENEPNL, wxEmptyString, wxDefaultPosition, wxSize(itemPanel1->ConvertDialogToPixels(wxSize(150, -1)).x, -1), 0 );
    itemFlexGridSizer3->Add(itemTextCtrl13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText10 = new wxStaticText(itemPanel1, wxID_STATIC, _("Map Location"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    itemFlexGridSizer3->Add(itemStaticText10, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 1);

    wxTextCtrl* itemTextCtrl11 = new wxTextCtrl(itemPanel1, ID_TEXT4_GENEPNL, wxEmptyString, wxDefaultPosition, wxSize(itemPanel1->ConvertDialogToPixels(wxSize(150, -1)).x, -1), 0);
    itemFlexGridSizer3->Add(itemTextCtrl11, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, 1);

    objects::CGene_ref *gene = dynamic_cast<objects::CGene_ref*>(m_Object);
    if (gene->IsSetLocus())
        itemTextCtrl5->SetValue(gene->GetLocus());
    if (gene->IsSetAllele())
        itemTextCtrl7->SetValue(gene->GetAllele());
    if (gene->IsSetDesc())
        itemTextCtrl9->SetValue(gene->GetDesc());
    if (gene->IsSetLocus_tag())
        itemTextCtrl13->SetValue(gene->GetLocus_tag());
    if (gene->IsSetMaploc()) {
        itemTextCtrl11->SetValue(gene->GetMaploc());
    } else {
        itemTextCtrl11->Show(false);
        itemStaticText10->Show(false);
    }

    // Set validators
    itemTextCtrl5->SetValidator( CSerialTextValidator(*m_Object, "locus") );
    itemTextCtrl7->SetValidator( CSerialTextValidator(*m_Object, "allele") );
    itemTextCtrl9->SetValidator( CSerialTextValidator(*m_Object, "desc") );
    itemTextCtrl11->SetValidator( CSerialTextValidator(*m_Object, "maploc"));
    itemTextCtrl13->SetValidator( CSerialTextValidator(*m_Object, "locus-tag") );
////@end CGenePanel content construction
    
}

/*!
 * Should we show tooltips?
 */

bool CGenePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CGenePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGenePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGenePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CGenePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGenePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGenePanel icon retrieval
}

END_NCBI_SCOPE
