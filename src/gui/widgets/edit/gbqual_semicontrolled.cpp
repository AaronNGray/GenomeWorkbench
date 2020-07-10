/*  $Id: gbqual_semicontrolled.cpp 36176 2016-08-19 17:28:23Z bollin $
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

#include "gbqual_semicontrolled.hpp"
#include <gui/widgets/wx/wx_utils.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CGBQualSemicontrolledTextPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CGBQualSemicontrolledTextPanel, CFormattedQualPanel )


/*!
 * CGBQualSemicontrolledTextPanel event table definition
 */

BEGIN_EVENT_TABLE( CGBQualSemicontrolledTextPanel, CFormattedQualPanel )

////@begin CGBQualSemicontrolledTextPanel event table entries
////@end CGBQualSemicontrolledTextPanel event table entries

END_EVENT_TABLE()


/*!
 * CGBQualSemicontrolledTextPanel constructors
 */

CGBQualSemicontrolledTextPanel::CGBQualSemicontrolledTextPanel()
{
    Init();
}

CGBQualSemicontrolledTextPanel::CGBQualSemicontrolledTextPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CGBQualTextPanel creator
 */

bool CGBQualSemicontrolledTextPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGBQualSemicontrolledTextPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CFormattedQualPanel::Create( parent, id, pos, size, style );

    CreateControls();
    Centre();
////@end CGBQualSemicontrolledTextPanel creation
    return true;
}


/*!
 * CGBQualSemicontrolledTextPanel destructor
 */

CGBQualSemicontrolledTextPanel::~CGBQualSemicontrolledTextPanel()
{
////@begin CGBQualSemicontrolledTextPanel destruction
////@end CGBQualSemicontrolledTextPanel destruction
}


/*!
 * Member initialisation
 */

void CGBQualSemicontrolledTextPanel::Init()
{
////@begin CGBQualSemicontrolledTextPanel member initialisation
    m_theValue = NULL;
////@end CGBQualSemicontrolledTextPanel member initialisation
}


/*!
 * Control creation for CGBQualTextPanel
 */

void CGBQualSemicontrolledTextPanel::CreateControls()
{    
////@begin CGBQualSemicontrolledTextPanel content construction
    CGBQualSemicontrolledTextPanel* itemCFormattedQualPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCFormattedQualPanel1->SetSizer(itemBoxSizer2);

    wxArrayString m_theValueStrings;
    m_theValue = new wxComboBox( itemCFormattedQualPanel1, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_theValueStrings, wxCB_DROPDOWN );
    itemBoxSizer2->Add(m_theValue, 0, wxGROW|wxALL, 0);

////@end CGBQualSemicontrolledTextPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CGBQualSemicontrolledTextPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CGBQualSemicontrolledTextPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGBQualSemicontrolledTextPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGBQualSemicontrolledTextPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CGBQualSemicontrolledTextPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGBQualSemicontrolledTextPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGBQualSemicontrolledTextPanel icon retrieval
}


void CGBQualSemicontrolledTextPanel::SetValue(string val)
{
    m_theValue->SetValue (ncbi::ToWxString(val));
}


string CGBQualSemicontrolledTextPanel::GetValue()
{
    string val = ncbi::ToStdString(m_theValue->GetValue());
    return val;
}


void CGBQualSemicontrolledTextPanel::SetControlledList (vector<string> choices)
{
    m_theValue->Clear();
    m_theValue->Append(wxEmptyString);
    ITERATE (vector<string>, it, choices) {
        m_theValue->Append(ToWxString (*it));
    }
}


END_NCBI_SCOPE
