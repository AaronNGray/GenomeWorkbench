/*  $Id: rpttype_panel.cpp 36594 2016-10-12 20:17:36Z evgeniev $
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
#include <objects/seqfeat/Gb_qual.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
////@begin includes
////@end includes

#include "rpttype_panel.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CRptTypePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CRptTypePanel, CFormattedQualPanel )


/*!
 * CRptTypePanel event table definition
 */

BEGIN_EVENT_TABLE( CRptTypePanel, CFormattedQualPanel )

////@begin CRptTypePanel event table entries
////@end CRptTypePanel event table entries

END_EVENT_TABLE()


/*!
 * CRptTypePanel constructors
 */

CRptTypePanel::CRptTypePanel()
{
    Init();
}

CRptTypePanel::CRptTypePanel( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CRptTypePanel creator
 */

bool CRptTypePanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRptTypePanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CFormattedQualPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRptTypePanel creation
    return true;
}


/*!
 * CRptTypePanel destructor
 */

CRptTypePanel::~CRptTypePanel()
{
////@begin CRptTypePanel destruction
////@end CRptTypePanel destruction
}


/*!
 * Member initialisation
 */

static void s_PopulateValuesArray (wxArrayString& arr)
{
    arr.clear();
    const objects::CGb_qual::TLegalRepeatTypeSet& vals = objects::CGb_qual::GetSetOfLegalRepeatTypes();
    ITERATE(objects::CGb_qual::TLegalRepeatTypeSet, s, vals) {
        arr.Add(*s);
    }
}


void CRptTypePanel::Init()
{
////@begin CRptTypePanel member initialisation
    m_TheSizer = NULL;
////@end CRptTypePanel member initialisation

    s_PopulateValuesArray(m_AcceptedValues);
}


/*!
 * Control creation for CRptTypePanel
 */

void CRptTypePanel::CreateControls()
{    
////@begin CRptTypePanel content construction
    CRptTypePanel* itemCFormattedQualPanel1 = this;

    m_TheSizer = new wxFlexGridSizer(0, 4, 0, 0);
    itemCFormattedQualPanel1->SetSizer(m_TheSizer);

////@end CRptTypePanel content construction

    ITERATE (wxArrayString, it, m_AcceptedValues) {
        wxCheckBox* box = new wxCheckBox(this, wxID_ANY, *it);
        m_TheSizer->Add(box);
        m_Boxes.push_back(box);
    }
}


void CRptTypePanel::SetValue(string val)
{
    vector<string> tokens;
    NStr::Split(val, ";", tokens);
    for (unsigned int i = 0; i < m_Boxes.size(); i++) {
        m_Boxes[i]->SetValue(false);
    }
    for (unsigned int j = 0; j < tokens.size(); j++) {
        unsigned int i = 0;
        while (i < m_Boxes.size() && i < m_AcceptedValues.size()) {
            if (NStr::EqualNocase(tokens[j], ToStdString(m_AcceptedValues[i]))) {
                m_Boxes[i]->SetValue(true);
                break;
            }
            i++;
        }
    }
}


string CRptTypePanel::GetValue()
{
    string val = "";

    unsigned int i = 0;
    while (i < m_Boxes.size() && i < m_AcceptedValues.size()) {
        if (m_Boxes[i]->GetValue()) {
            if (!NStr::IsBlank(val)) {        
                val = val + ";";
            }
            val = val + ToStdString(m_AcceptedValues[i]);
        }
        i++;
    }
    return val;
}


/*!
 * Should we show tooltips?
 */

bool CRptTypePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRptTypePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRptTypePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRptTypePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CRptTypePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRptTypePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRptTypePanel icon retrieval
}

bool CRptTypePanel::IsParseable( const string& val )
{
    wxArrayString accepted;
    s_PopulateValuesArray(accepted);

    vector<string> tokens;

    NStr::Split(val, ";", tokens);
    ITERATE (vector<string>, tok, tokens) {
        bool found = false;
        ITERATE (wxArrayString, it, accepted) {
            if (NStr::EqualNocase(*tok, ToStdString(*it))) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

END_NCBI_SCOPE
