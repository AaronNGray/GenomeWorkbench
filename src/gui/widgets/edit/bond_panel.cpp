/*  $Id: bond_panel.cpp 27153 2012-12-31 19:59:43Z filippov $
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

#include "bond_panel.hpp"


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images



IMPLEMENT_DYNAMIC_CLASS( CBondPanel, wxPanel )


BEGIN_EVENT_TABLE( CBondPanel, wxPanel )

END_EVENT_TABLE()



CBondPanel::CBondPanel()
{
    Init();
}

CBondPanel::CBondPanel( wxWindow* parent, CSeqFeatData::EBond bond,
                      wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: m_Bond(bond)
{
       Init();
    Create(parent, id, pos, size, style);
}


bool CBondPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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



CBondPanel::~CBondPanel()
{
}


void CBondPanel::Init()
{
}


void CBondPanel::CreateControls()
{
    CBondPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxArrayString types;
    CBondList list;
    int selection = 0;
    int j = 0;
    for (CBondList::const_iterator i = list.begin(); i !=  list.end(); ++i)
    {
        types.Add(ToWxString(i->first));
        if (i->second == m_Bond) selection = j;
        j++;
    }

    m_Choice = new wxChoice(itemPanel1, ID_BONDPANEL_CHOICE, wxDefaultPosition, wxDefaultSize, types ,0,wxDefaultValidator,"Bond");
    m_Choice->SetSelection(selection);
    itemBoxSizer2->Add(m_Choice, 0, wxALIGN_LEFT|wxALL, 5);
   
}



bool CBondPanel::TransferDataFromWindow()
{
    bool result = wxPanel::TransferDataFromWindow();
    int selection = m_Choice->GetSelection();
    CBondList list;
    int j=0;
    for (CBondList::const_iterator i = list.begin(); i !=  list.end(); ++i)
    {
        if (j == selection) m_Bond = i->second;
        j++;
    }

    return result;
}


bool CBondPanel::ShowToolTips()
{
    return true;
}

wxBitmap CBondPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CBondPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

CSeqFeatData::EBond CBondPanel::GetBond()
{
    return m_Bond;
}

END_NCBI_SCOPE

