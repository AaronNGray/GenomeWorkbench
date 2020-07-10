/*  $Id: region_panel.cpp 39032 2017-07-21 16:04:03Z filippov $
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
 * Authors:  Roman Katargin and Igor Filippov
 */


#include <ncbi_pch.hpp>
////@begin includes
#include "wx/imaglist.h"
////@end includes

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include "region_panel.hpp"

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images



IMPLEMENT_DYNAMIC_CLASS( CRegionPanel, wxPanel )


BEGIN_EVENT_TABLE( CRegionPanel, wxPanel )

END_EVENT_TABLE()



CRegionPanel::CRegionPanel()
{
    Init();
}

CRegionPanel::CRegionPanel( wxWindow* parent, string region,
                      wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: m_Region(region)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CRNAPanel creator
 */

bool CRegionPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{

    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}




CRegionPanel::~CRegionPanel()
{
}


/*!
 * Member initialisation
 */

void CRegionPanel::Init()
{
    m_ConvertToMiscFeat = NULL;
}


void CRegionPanel::CreateControls()
{
    CRegionPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_TextCtrl = new wxTextCtrl( itemPanel1, ID_REGIONPANEL_TEXTCTRL, ToWxString(m_Region), wxDefaultPosition, wxSize(150, -1), 0 );
    itemBoxSizer2->Add(m_TextCtrl, 0, wxALIGN_LEFT|wxALL, 5);
   
    m_ConvertToMiscFeat = new wxCheckBox( itemPanel1, wxID_ANY, _("Convert to misc-feat"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_ConvertToMiscFeat, 0, wxALIGN_LEFT|wxALL, 5);
}



bool CRegionPanel::TransferDataFromWindow()
{
    bool result = wxPanel::TransferDataFromWindow();

    m_Region = ToStdString(m_TextCtrl->GetLineText(0));

    return result;
}


/*!
 * Should we show tooltips?
 */

bool CRegionPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRegionPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CRegionPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

string CRegionPanel::GetRegion()
{
    return m_Region;
}

END_NCBI_SCOPE

