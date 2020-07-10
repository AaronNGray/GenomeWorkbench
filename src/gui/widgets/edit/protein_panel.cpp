/*  $Id: protein_panel.cpp 25638 2012-04-16 11:44:40Z bollin $
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
#include <objects/seqfeat/Prot_ref.hpp>
////@begin includes
#include "string_list_ctrl.hpp"
////@end includes

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>
#include <gui/widgets/edit/serial_member_stringlist_validator.hpp>
#include <serial/serialbase.hpp>
#include <serial/typeinfo.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include "protein_panel.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CProteinPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CProteinPanel, wxPanel )


/*!
 * CProteinPanel event table definition
 */

BEGIN_EVENT_TABLE( CProteinPanel, wxPanel )

////@begin CProteinPanel event table entries
////@end CProteinPanel event table entries

END_EVENT_TABLE()


/*!
 * CProteinPanel constructors
 */

CProteinPanel::CProteinPanel()
{
    Init();
}

CProteinPanel::CProteinPanel( wxWindow* parent, CSerialObject& object, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Object(0)
{
    m_Object = &object;
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CProteinPanel creator
 */

bool CProteinPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CProteinPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CProteinPanel creation
    return true;
}


/*!
 * CProteinPanel destructor
 */

CProteinPanel::~CProteinPanel()
{
////@begin CProteinPanel destruction
////@end CProteinPanel destruction
}


/*!
 * Member initialisation
 */

void CProteinPanel::Init()
{
////@begin CProteinPanel member initialisation
    m_ProcessedCtrl = NULL;
////@end CProteinPanel member initialisation
}


/*!
 * Control creation for CProteinPanel
 */

void CProteinPanel::CreateControls()
{    
////@begin CProteinPanel content construction
    CProteinPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 5);

    CStringListCtrl* itemScrolledWindow5 = new CStringListCtrl( itemPanel1, ID_WINDOW1, wxDefaultPosition, itemPanel1->ConvertDialogToPixels(wxSize(66, 61)), wxDOUBLE_BORDER );
    itemFlexGridSizer3->Add(itemScrolledWindow5, 1, wxALIGN_LEFT|wxGROW|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Description"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl7 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL12, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl7, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Processing"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_ProcessedCtrlStrings;
    m_ProcessedCtrlStrings.Add(wxEmptyString);
    m_ProcessedCtrlStrings.Add(_("Proprotein"));
    m_ProcessedCtrlStrings.Add(_("Mature"));
    m_ProcessedCtrlStrings.Add(_("Signal peptide"));
    m_ProcessedCtrlStrings.Add(_("Transit peptide"));
    m_ProcessedCtrl = new wxChoice( itemPanel1, ID_CHOICE15, wxDefaultPosition, wxDefaultSize, m_ProcessedCtrlStrings, 0 );
    itemFlexGridSizer3->Add(m_ProcessedCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->AddGrowableRow(0);
    itemFlexGridSizer3->AddGrowableCol(1);

    // Set validators
    itemScrolledWindow5->SetValidator( CSerialStringListValidator(*m_Object, "name") );
    itemTextCtrl7->SetValidator( CSerialTextValidator(*m_Object, "desc") );
////@end CProteinPanel content construction
}


bool CProteinPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    objects::CProt_ref & prot = dynamic_cast<objects::CProt_ref&>(*m_Object);
    if (prot.IsSetProcessed()) {
        switch (prot.GetProcessed()) {
            case objects::CProt_ref::eProcessed_preprotein:
                m_ProcessedCtrl->SetSelection(1);
                break;
            case objects::CProt_ref::eProcessed_mature:
                m_ProcessedCtrl->SetSelection(2);
                break;
            case objects::CProt_ref::eProcessed_signal_peptide:
                m_ProcessedCtrl->SetSelection(3);
                break;
            case objects::CProt_ref::eProcessed_transit_peptide:
                m_ProcessedCtrl->SetSelection(4);
                break;
            case objects::CProt_ref::eProcessed_not_set:
            default:
                m_ProcessedCtrl->SetSelection(1);
                break;
        }
    } else {
        m_ProcessedCtrl->SetSelection(0);
    }

    return true;
}


bool CProteinPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    objects::CProt_ref & prot = dynamic_cast<objects::CProt_ref&>(*m_Object);
    switch (m_ProcessedCtrl->GetSelection()) {
        case 1:
            prot.SetProcessed(objects::CProt_ref::eProcessed_preprotein);
            break;
        case 2:
            prot.SetProcessed(objects::CProt_ref::eProcessed_mature);
            break;
        case 3:
            prot.SetProcessed(objects::CProt_ref::eProcessed_signal_peptide);
            break;
        case 4:
            prot.SetProcessed(objects::CProt_ref::eProcessed_transit_peptide);
            break;
        case 0:
        default:
            prot.ResetProcessed();
            break;
    }
    return true;
}


/*!
 * Should we show tooltips?
 */

bool CProteinPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CProteinPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CProteinPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CProteinPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CProteinPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CProteinPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CProteinPanel icon retrieval
}

END_NCBI_SCOPE

