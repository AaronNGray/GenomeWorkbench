/*  $Id: status_message_slot_panel.cpp 39077 2017-07-24 20:50:01Z katargir $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/framework/status_message_slot_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
////@end includes

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>


BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CStatusMessageSlotPanel, wxPanel )

BEGIN_EVENT_TABLE( CStatusMessageSlotPanel, wxPanel )
////@begin CStatusMessageSlotPanel event table entries
    EVT_HYPERLINK( ID_EXTRA_LINK, CStatusMessageSlotPanel::OnExtraLinkHyperlinkClicked )

////@end CStatusMessageSlotPanel event table entries
END_EVENT_TABLE()


CStatusMessageSlotPanel::CStatusMessageSlotPanel()
{
    Init();
}


CStatusMessageSlotPanel::CStatusMessageSlotPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CStatusMessageSlotPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CStatusMessageSlotPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CStatusMessageSlotPanel creation
    return true;
}


CStatusMessageSlotPanel::~CStatusMessageSlotPanel()
{
////@begin CStatusMessageSlotPanel destruction
////@end CStatusMessageSlotPanel destruction
}


void CStatusMessageSlotPanel::Init()
{
////@begin CStatusMessageSlotPanel member initialisation
    m_TitleStatic = NULL;
    m_TimeStatic = NULL;
    m_DescrStatic = NULL;
    m_ExtraLink = NULL;
////@end CStatusMessageSlotPanel member initialisation
}


void CStatusMessageSlotPanel::CreateControls()
{
////@begin CStatusMessageSlotPanel content construction
    CStatusMessageSlotPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW, 5);

    m_TitleStatic = new wxStaticText( itemPanel1, wxID_STATIC, _("Title"), wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE|wxNO_BORDER );
    m_TitleStatic->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxFONTWEIGHT_BOLD, wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName()));
    itemBoxSizer3->Add(m_TitleStatic, 1, wxGROW|wxALL, 5);

    m_TimeStatic = new wxStaticText( itemPanel1, wxID_STATIC, _("Time"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_TimeStatic, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DescrStatic = new wxStaticText( itemPanel1, wxID_STATIC, _("Description"), wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE|wxNO_BORDER );
    itemBoxSizer2->Add(m_DescrStatic, 1, wxGROW|wxALL, 5);

    m_ExtraLink = new wxHyperlinkCtrl( itemPanel1, ID_EXTRA_LINK, _("Extra Link"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer2->Add(m_ExtraLink, 0, wxALIGN_LEFT|wxALL, 5);

    // Set validators
    m_TitleStatic->SetValidator( wxGenericValidator(& m_Title) );
    m_TimeStatic->SetValidator( wxGenericValidator(& m_Time) );
    m_DescrStatic->SetValidator( wxGenericValidator(& m_Descr) );
////@end CStatusMessageSlotPanel content construction
}


bool CStatusMessageSlotPanel::ShowToolTips()
{
    return true;
}


wxBitmap CStatusMessageSlotPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CStatusMessageSlotPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CStatusMessageSlotPanel bitmap retrieval
}


wxIcon CStatusMessageSlotPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CStatusMessageSlotPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CStatusMessageSlotPanel icon retrieval
}


void CStatusMessageSlotPanel::SetExtraLink(const string& label)
{
    m_ExtraLink->SetLabel(ToWxString(label));
    m_ExtraLink->Show( ! label.empty());
}


bool CStatusMessageSlotPanel::TransferDataToWindow()
{
    bool res = wxPanel::TransferDataToWindow();

    bool show = ! m_Descr.empty();
    m_DescrStatic->Show(show);

    return res;
}


void CStatusMessageSlotPanel::WrapText()
{
    wxRect rc = m_TitleStatic->GetRect();
    m_TitleStatic->Wrap(rc.width);
    wxSize sz = m_TitleStatic->GetBestSize();
    sz.x += 4;
    m_TitleStatic->SetMinSize(sz);

    rc = m_DescrStatic->GetRect();
    m_DescrStatic->Wrap(rc.width);
    sz = m_DescrStatic->GetBestSize();
    if(sz.x == 0)   {
        sz.y = 0;
    }
    m_DescrStatic->SetMinSize(sz);
}


void CStatusMessageSlotPanel::OnExtraLinkHyperlinkClicked( wxHyperlinkEvent& event )
{
    GetParent()->GetEventHandler()->ProcessEvent(event);
}

END_NCBI_SCOPE

