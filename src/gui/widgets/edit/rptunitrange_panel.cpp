/*  $Id: rptunitrange_panel.cpp 25454 2012-03-21 12:23:18Z bollin $
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

#include "rptunitrange_panel.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CRptUnitRangePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CRptUnitRangePanel, CFormattedQualPanel )


/*!
 * CRptUnitRangePanel event table definition
 */

BEGIN_EVENT_TABLE( CRptUnitRangePanel, CFormattedQualPanel )

////@begin CRptUnitRangePanel event table entries
////@end CRptUnitRangePanel event table entries

END_EVENT_TABLE()


/*!
 * CRptUnitRangePanel constructors
 */

CRptUnitRangePanel::CRptUnitRangePanel()
{
    Init();
}

CRptUnitRangePanel::CRptUnitRangePanel( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CRptUnitRangePanel creator
 */

bool CRptUnitRangePanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRptUnitRangePanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CFormattedQualPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRptUnitRangePanel creation
    return true;
}


/*!
 * CRptUnitRangePanel destructor
 */

CRptUnitRangePanel::~CRptUnitRangePanel()
{
////@begin CRptUnitRangePanel destruction
////@end CRptUnitRangePanel destruction
}


/*!
 * Member initialisation
 */

void CRptUnitRangePanel::Init()
{
////@begin CRptUnitRangePanel member initialisation
    m_StartCtrl = NULL;
    m_StopCtrl = NULL;
////@end CRptUnitRangePanel member initialisation
}


/*!
 * Control creation for CRptUnitRangePanel
 */

void CRptUnitRangePanel::CreateControls()
{    
////@begin CRptUnitRangePanel content construction
    CRptUnitRangePanel* itemCFormattedQualPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemCFormattedQualPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemCFormattedQualPanel1, wxID_STATIC, _("Start"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_StartCtrl = new wxTextCtrl( itemCFormattedQualPanel1, ID_TEXTCTRL29, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_StartCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemCFormattedQualPanel1, wxID_STATIC, _("Stop"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_StopCtrl = new wxTextCtrl( itemCFormattedQualPanel1, ID_TEXTCTRL30, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_StopCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CRptUnitRangePanel content construction
}


void CRptUnitRangePanel::SetValue(string val)
{
    wxString start = wxEmptyString;
    wxString stop = wxEmptyString;

    int pos = NStr::Find(val, "..");
    if (pos == string::npos) {
        pos = NStr::Find(val, ".");
        if (pos == string::npos) {
            start = ToWxString (val);
        } else {
            start = ToWxString (val.substr(0, pos));
            stop = ToWxString (val.substr(pos + 1));
        }
    } else {
        start = ToWxString (val.substr(0, pos));
        stop = ToWxString (val.substr(pos + 2));
    }
    m_StartCtrl->SetValue(start);
    m_StopCtrl->SetValue(stop);
}


string CRptUnitRangePanel::GetValue()
{
    string part1 = ToStdString (m_StartCtrl->GetValue());
    string part2 = ToStdString (m_StopCtrl->GetValue());
    string val = "";
    if (NStr::IsBlank(part1) && NStr::IsBlank(part2)) {
        val = "";
    } else if (NStr::IsBlank(part2)) {
        val = part1;
    } else {
        val = part1 + ".." + part2;
    }
    return val;
}


/*!
 * Should we show tooltips?
 */

bool CRptUnitRangePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRptUnitRangePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRptUnitRangePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRptUnitRangePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CRptUnitRangePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRptUnitRangePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRptUnitRangePanel icon retrieval
}

END_NCBI_SCOPE
