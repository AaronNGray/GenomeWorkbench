/*  $Id: string_list_ctrl.cpp 40968 2018-05-04 16:42:02Z katargir $
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
#include <corelib/ncbistd.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/hyperlink.hpp>


////@begin includes
////@end includes

#include "string_list_ctrl.hpp"

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/recguard.h>

BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images


/*!
 * CStringListCtrl type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CStringListCtrl, wxScrolledWindow )


/*!
 * CStringListCtrl event table definition
 */

BEGIN_EVENT_TABLE( CStringListCtrl, wxScrolledWindow )

////@begin CStringListCtrl event table entries
////@end CStringListCtrl event table entries

    EVT_TEXT(wxID_ANY, CStringListCtrl::OnTextCtrlUpdated)
    EVT_HYPERLINK(wxID_ANY, CStringListCtrl::OnDelete)


END_EVENT_TABLE()


/*!
 * CStringListCtrl constructors
 */

CStringListCtrl::CStringListCtrl()
{
    Init();
}

CStringListCtrl::CStringListCtrl( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CStringListCtrl creator
 */

bool CStringListCtrl::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CStringListCtrl creation
    wxScrolledWindow::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CStringListCtrl creation

    SetScrollRate(5, 5);

    return true;
}


/*!
 * CStringListCtrl destructor
 */

CStringListCtrl::~CStringListCtrl()
{
////@begin CStringListCtrl destruction
////@end CStringListCtrl destruction
}


/*!
 * Member initialisation
 */

void CStringListCtrl::Init()
{
////@begin CStringListCtrl member initialisation
    m_CurrentStringId = 0;
    m_GridSizer = NULL;
////@end CStringListCtrl member initialisation
}


/*!
 * Control creation for CStringListCtrl
 */

void CStringListCtrl::CreateControls()
{
////@begin CStringListCtrl content construction
    CStringListCtrl* itemScrolledWindow1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemScrolledWindow1->SetSizer(itemBoxSizer2);

    m_GridSizer = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(m_GridSizer, 1, wxGROW|wxALL, 5);

    m_GridSizer->AddGrowableCol(0);

////@end CStringListCtrl content construction
}


/*!
 * Should we show tooltips?
 */

bool CStringListCtrl::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CStringListCtrl::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CStringListCtrl bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CStringListCtrl bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CStringListCtrl::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CStringListCtrl icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CStringListCtrl icon retrieval
}

static wxRecursionGuardFlag s_flagReentrancy;

bool CStringListCtrl::AddString(const string& value)
{
    wxRecursionGuard guard(s_flagReentrancy);

    int id = FindSlot();
    if (id == kStringIdBase + kMaxStringsCount)
        return false;

    if (m_CurrentStringId > 0) {
        wxHyperlinkCtrl* itemHyperlinkCtrl = new CHyperlink( this,
            id + kMaxStringsCount, wxT("Delete"), wxT(""), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
        m_GridSizer->Add(itemHyperlinkCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);
    }

    wxTextCtrl* itemTextCtrl = new wxTextCtrl( this, id, ToWxString(value), wxDefaultPosition, wxDefaultSize, 0 );
    m_GridSizer->Add(itemTextCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);
    m_CurrentStringId = id;

    return true;
}

int CStringListCtrl::FindSlot()
{
    int id = kStringIdBase;
    for (;id < kStringIdBase + kMaxStringsCount;++id)
    {
        wxWindow* window = FindWindow(id);
        if (window == 0)
            break;
    }
    return id;
}

void CStringListCtrl::OnTextCtrlUpdated (wxCommandEvent& event)
{
    wxRecursionGuard guard(s_flagReentrancy);
    if (guard.IsInside())
        return;

    if (event.GetId() == m_CurrentStringId) {
            AddString("");
            FitInside();

            int h, hv;
            GetClientSize(0, &h);
            GetVirtualSize(0, &hv);
            if (hv > h) Scroll(-1, (hv - h + 5)/5);
    }
}

void CStringListCtrl::OnDelete (wxHyperlinkEvent& event)
{
    wxWindow* window = FindWindow(event.GetId());
    if (window == 0)
        return;

    int index = 0;
    wxSizerItemList::iterator node = m_GridSizer->GetChildren().begin();
    for ( ;  node != m_GridSizer->GetChildren().end();  ++node, ++index) {
        wxSizerItem* item = *node;
        if (item->IsWindow()&& item->GetWindow() == window)
            break;
    }

    if (node == m_GridSizer->GetChildren().end())
        return;

    m_GridSizer->GetItem(index)->DeleteWindows();
    m_GridSizer->GetItem(index-1)->DeleteWindows();
    m_GridSizer->Remove(index);
    m_GridSizer->Remove(index-1);

    wxTextCtrl* textCtrl = (wxTextCtrl*)FindWindow(m_CurrentStringId);
    if (textCtrl != 0) {
        wxString value = textCtrl->GetValue();
        if (!value.empty())
            AddString("");
    }
    FitInside();
}


void CStringListCtrl::Clear()
{
    Layout();
    while(m_GridSizer->GetItemCount() > 0) {
        m_GridSizer->GetItem((size_t)0)->DeleteWindows();
        m_GridSizer->Remove(0);
    }
    m_GridSizer->Clear();
    FitInside();
    m_CurrentStringId = 0;
}


CStringListCtrl::CConstIterator::CConstIterator (CStringListCtrl& control)
    : m_Control(control), m_Id(kStringIdBase-1), m_TextCtrl(0)
{
    this->operator++();
}

CStringListCtrl::CConstIterator& CStringListCtrl::CConstIterator::operator++()
{
    if (m_Id == kStringIdBase + kMaxStringsCount)
        return *this;

    for(;;) {
        if (++m_Id == kStringIdBase + kMaxStringsCount) {
            m_TextCtrl = 0;
            break;
        }

        m_TextCtrl = (wxTextCtrl*)m_Control.FindWindow(m_Id);
        if (m_TextCtrl != 0)
            break;
    }

    return *this;
}

string CStringListCtrl::CConstIterator::GetValue()
{
    if (m_TextCtrl == 0)
        return "";

    string value = ToStdString(m_TextCtrl->GetValue());
    return value;
}

END_NCBI_SCOPE



