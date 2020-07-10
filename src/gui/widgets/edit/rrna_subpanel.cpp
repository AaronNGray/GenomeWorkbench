/*  $Id: rrna_subpanel.cpp 29198 2013-11-13 20:34:31Z bollin $
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

#include "rrna_subpanel.hpp"

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>


BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images


/*!
 * CrRNASubPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CrRNASubPanel, wxPanel )


/*!
 * CrRNASubPanel event table definition
 */

BEGIN_EVENT_TABLE( CrRNASubPanel, wxPanel )

////@begin CrRNASubPanel event table entries
////@end CrRNASubPanel event table entries

END_EVENT_TABLE()


/*!
 * CrRNASubPanel constructors
 */

CrRNASubPanel::CrRNASubPanel()
{
    Init();
}

CrRNASubPanel::CrRNASubPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CrRNASubPanel creator
 */

bool CrRNASubPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CrRNASubPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CrRNASubPanel creation
    return true;
}


/*!
 * CrRNASubPanel destructor
 */

CrRNASubPanel::~CrRNASubPanel()
{
////@begin CrRNASubPanel destruction
////@end CrRNASubPanel destruction
}


/*!
 * Member initialisation
 */

void CrRNASubPanel::Init()
{
////@begin CrRNASubPanel member initialisation
    m_NameCtrl = NULL;
////@end CrRNASubPanel member initialisation
}


/*!
 * Control creation for CrRNASubPanel
 */

void CrRNASubPanel::CreateControls()
{
////@begin CrRNASubPanel content construction
    CrRNASubPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 2);

    m_NameCtrl = new CAutoCompleteTextCtrl( itemPanel1, ID_RRNA_NAME_CTRL, wxEmptyString, wxDefaultPosition, wxSize(itemPanel1->ConvertDialogToPixels(wxSize(100, -1)).x, -1), 0 );
    itemBoxSizer3->Add(m_NameCtrl, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 2);

////@end CrRNASubPanel content construction
    m_NameCtrl->AutoComplete(new CRNANameCompleter());
}


/*!
 * Should we show tooltips?
 */

bool CrRNASubPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CrRNASubPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CrRNASubPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CrRNASubPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CrRNASubPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CrRNASubPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CrRNASubPanel icon retrieval
}


wxString CrRNASubPanel::GetRnaName() const 
{ 
    return m_NameCtrl->GetValue();
}


void CrRNASubPanel::SetRnaName(wxString value) 
{
    m_NameCtrl->SetValue(value);
}



END_NCBI_SCOPE
