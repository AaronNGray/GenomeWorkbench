/*  $Id: gbqualtext_panel.cpp 35729 2016-06-15 20:31:00Z asztalos $
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
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
////@end includes

#include "gbqualtext_panel.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CGBQualTextPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CGBQualTextPanel, CFormattedQualPanel )


/*!
 * CGBQualTextPanel event table definition
 */

BEGIN_EVENT_TABLE( CGBQualTextPanel, CFormattedQualPanel )

////@begin CGBQualTextPanel event table entries
////@end CGBQualTextPanel event table entries

END_EVENT_TABLE()


/*!
 * CGBQualTextPanel constructors
 */

CGBQualTextPanel::CGBQualTextPanel()
{
    Init();
}

CGBQualTextPanel::CGBQualTextPanel( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CGBQualTextPanel creator
 */

bool CGBQualTextPanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGBQualTextPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CFormattedQualPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGBQualTextPanel creation
    return true;
}


/*!
 * CGBQualTextPanel destructor
 */

CGBQualTextPanel::~CGBQualTextPanel()
{
////@begin CGBQualTextPanel destruction
////@end CGBQualTextPanel destruction
}


/*!
 * Member initialisation
 */

void CGBQualTextPanel::Init()
{
////@begin CGBQualTextPanel member initialisation
    m_theValue = NULL;
////@end CGBQualTextPanel member initialisation
}


/*!
 * Control creation for CGBQualTextPanel
 */

void CGBQualTextPanel::CreateControls()
{    
////@begin CGBQualTextPanel content construction
    CGBQualTextPanel* itemCFormattedQualPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCFormattedQualPanel1->SetSizer(itemBoxSizer2);

    m_theValue = new wxTextCtrl( itemCFormattedQualPanel1, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(280, -1), 0 );
    itemBoxSizer2->Add(m_theValue, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

////@end CGBQualTextPanel content construction
}


void CGBQualTextPanel::SetValue(string val)
{
    m_theValue->SetValue (ncbi::ToWxString(val));
}


string CGBQualTextPanel::GetValue()
{
    string val = ncbi::ToStdString(m_theValue->GetValue());
    return val;
}


/*!
 * Should we show tooltips?
 */

bool CGBQualTextPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CGBQualTextPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGBQualTextPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGBQualTextPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CGBQualTextPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGBQualTextPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGBQualTextPanel icon retrieval
}

END_NCBI_SCOPE

