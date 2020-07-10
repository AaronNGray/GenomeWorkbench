/*  $Id: trna_recognized_codons_panel.cpp 25347 2012-03-01 18:22:54Z katargir $
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

#include <gui/widgets/edit/codons_stringlist_validator.hpp>

#include "trna_recognized_codons_panel.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CtRNARecognizedCodonsPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CtRNARecognizedCodonsPanel, wxPanel )


/*!
 * CtRNARecognizedCodonsPanel event table definition
 */

BEGIN_EVENT_TABLE( CtRNARecognizedCodonsPanel, wxPanel )

////@begin CtRNARecognizedCodonsPanel event table entries
////@end CtRNARecognizedCodonsPanel event table entries

END_EVENT_TABLE()


/*!
 * CtRNARecognizedCodonsPanel constructors
 */

CtRNARecognizedCodonsPanel::CtRNARecognizedCodonsPanel()
{
    Init();
}

CtRNARecognizedCodonsPanel::CtRNARecognizedCodonsPanel( wxWindow* parent, CSerialObject& object, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Object(0)
{
    m_Object = &object;
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CtRNARecognizedCodonsPanel creator
 */

bool CtRNARecognizedCodonsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CtRNARecognizedCodonsPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CtRNARecognizedCodonsPanel creation
    return true;
}


/*!
 * CtRNARecognizedCodonsPanel destructor
 */

CtRNARecognizedCodonsPanel::~CtRNARecognizedCodonsPanel()
{
////@begin CtRNARecognizedCodonsPanel destruction
////@end CtRNARecognizedCodonsPanel destruction
}


/*!
 * Member initialisation
 */

void CtRNARecognizedCodonsPanel::Init()
{
////@begin CtRNARecognizedCodonsPanel member initialisation
////@end CtRNARecognizedCodonsPanel member initialisation
}


/*!
 * Control creation for CtRNARecognizedCodonsPanel
 */

void CtRNARecognizedCodonsPanel::CreateControls()
{    
////@begin CtRNARecognizedCodonsPanel content construction
    CtRNARecognizedCodonsPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 0);

    CStringListCtrl* itemScrolledWindow4 = new CStringListCtrl( itemPanel1, ID_WINDOW, wxDefaultPosition, itemPanel1->ConvertDialogToPixels(wxSize(22, 37)), wxSUNKEN_BORDER|wxScrolledWindowStyle|wxTAB_TRAVERSAL );
    itemBoxSizer3->Add(itemScrolledWindow4, 1, wxGROW|wxALL, 0);

    // Set validators
    itemScrolledWindow4->SetValidator( CCodonsStringListValidator(*m_Object) );
////@end CtRNARecognizedCodonsPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CtRNARecognizedCodonsPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CtRNARecognizedCodonsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CtRNARecognizedCodonsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CtRNARecognizedCodonsPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CtRNARecognizedCodonsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CtRNARecognizedCodonsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CtRNARecognizedCodonsPanel icon retrieval
}

END_NCBI_SCOPE
