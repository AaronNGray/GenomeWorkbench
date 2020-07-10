/*  $Id: site_panel.cpp 27154 2012-12-31 20:33:35Z filippov $
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


#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

#include "site_panel.hpp"


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images



IMPLEMENT_DYNAMIC_CLASS( CSitePanel, wxPanel )


BEGIN_EVENT_TABLE( CSitePanel, wxPanel )

END_EVENT_TABLE()



CSitePanel::CSitePanel()
{
    Init();
}

CSitePanel::CSitePanel( wxWindow* parent, CSeqFeatData::ESite site,
                      wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: m_Site(site)
{
    Init();
    Create(parent, id, pos, size, style);
}


bool CSitePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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



CSitePanel::~CSitePanel()
{
}


void CSitePanel::Init()
{
}


void CSitePanel::CreateControls()
{
    CSitePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxArrayString types;
    CSiteList list;
    int selection = 0;
    int j = 0;
    for (CSiteList::const_iterator i = list.begin(); i !=  list.end(); ++i)
    {
        types.Add(ToWxString(i->first));
        if (i->second == m_Site) selection = j;
        j++;
    }

    m_Choice = new wxChoice(itemPanel1, ID_SITEPANEL_CHOICE, wxDefaultPosition, wxDefaultSize, types ,0,wxDefaultValidator,"Site");
    m_Choice->SetSelection(selection);
    itemBoxSizer2->Add(m_Choice, 0, wxALIGN_LEFT|wxALL, 5);
   
}



bool CSitePanel::TransferDataFromWindow()
{
    bool result = wxPanel::TransferDataFromWindow();
    int selection = m_Choice->GetSelection();
    CSiteList list;
    int j=0;
    for (CSiteList::const_iterator i = list.begin(); i !=  list.end(); ++i)
    {
        if (j == selection) m_Site = i->second;
        j++;
    }

    return result;
}


bool CSitePanel::ShowToolTips()
{
    return true;
}

wxBitmap CSitePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CSitePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

CSeqFeatData::ESite CSitePanel::GetSite()
{
    return m_Site;
}

END_NCBI_SCOPE

