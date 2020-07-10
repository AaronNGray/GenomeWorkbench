/*  $Id: macro_authors_panel.cpp 41345 2018-07-12 18:04:13Z asztalos $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software / database is a "United States Government Work" under the
*  terms of the United States Copyright Act.It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.This software / database is freely available
*  to the public for use.The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data.The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors: Andrea Asztalos
*
*/

#include <ncbi_pch.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/biblio/Auth_list.hpp>

#include <gui/widgets/wx/unfocused_controls.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/singleauthor_panel.hpp>
#include <gui/widgets/edit/macro_authors_panel.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/scrolwin.h>
#include <wx/button.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CMacroAuthorNamesPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CMacroAuthorNamesPanel, CAuthorNamesContainer)


/*
 * CMacroAuthorNamesPanel event table definition
 */

BEGIN_EVENT_TABLE( CMacroAuthorNamesPanel, CAuthorNamesContainer)

    EVT_BUTTON( ID_IMPORTAUTHORSBTN, CMacroAuthorNamesPanel::OnImportAuthorsClicked )
    EVT_HYPERLINK(wxID_ANY, CMacroAuthorNamesPanel::OnDelete)

END_EVENT_TABLE()


/*
 * CMacroAuthorNamesPanel constructors
 */

CMacroAuthorNamesPanel::CMacroAuthorNamesPanel()
    : CAuthorNamesContainer()
{
    Init();
}

CMacroAuthorNamesPanel::CMacroAuthorNamesPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : CAuthorNamesContainer()
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CMacroAuthorNamesPanel creator
 */

bool CMacroAuthorNamesPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMacroAuthorNamesPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CMacroAuthorNamesPanel creation
    return true;
}


/*
 * CMacroAuthorNamesPanel destructor
 */

CMacroAuthorNamesPanel::~CMacroAuthorNamesPanel()
{
////@begin CMacroAuthorNamesPanel destruction
////@end CMacroAuthorNamesPanel destruction
}


/*
 * Member initialisation
 */

void CMacroAuthorNamesPanel::Init()
{
    m_ScrolledWindow = nullptr;
    m_Sizer = nullptr;
}

/*
 * Control creation for CMacroAuthorNamesPanel
 */

void CMacroAuthorNamesPanel::CreateControls()
{    
////@begin CMacroAuthorNamesPanel content construction
    CMacroAuthorNamesPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("First name"), wxDefaultPosition, wxSize(80, -1), 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Middle initials"), wxDefaultPosition, wxSize(90, -1), 0 );
    itemBoxSizer3->Add(itemStaticText5, 0, wxALIGN_LEFT |wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Last name"), wxDefaultPosition, wxSize(90, -1), 0 );
    itemBoxSizer3->Add(itemStaticText6, 0, wxALIGN_LEFT |wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Suffix"), wxDefaultPosition, wxSize(170, -1), 0 );
    itemBoxSizer3->Add(itemStaticText7, 0, wxALIGN_LEFT |wxALL, 5);

    m_ScrolledWindow = new wxScrolledWindow( itemPanel1, ID_MACROAUTHORSSCROLLEDWND, wxDefaultPosition, wxSize(515, 140), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemBoxSizer2->Add(m_ScrolledWindow, 0, wxALIGN_CENTER_HORIZONTAL |wxALL, 5);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);

    wxButton* itemButton11 = new wxButton( itemPanel1, ID_IMPORTAUTHORSBTN, _("Import authors"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemButton11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Sizer = new wxFlexGridSizer(0, 2, 0, 0);
    m_ScrolledWindow->SetSizer(m_Sizer);

    for (size_t index = 0; index < m_Rows; ++index) {
        x_AddEmptyRow();
    }

////@end CMacroAuthorNamesPanel content construction
}

void CMacroAuthorNamesPanel::x_AddEmptyRow()
{
    CRef<CAuthor> new_auth(new CAuthor);
    CSingleAuthorPanel* row = new CSingleAuthorPanel(m_ScrolledWindow, *new_auth);
    if (m_RowHeight == 0) {
        int row_width;
        row->GetClientSize(&row_width, &m_RowHeight);
    }
    x_AddRowToWindow(row);
}

void CMacroAuthorNamesPanel::x_AddRowToWindow(wxWindow* row)
{
    m_Sizer->Add(row, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 0);
    wxHyperlinkCtrl* itemHyperLink = new CNoTabHyperlinkCtrl(m_ScrolledWindow, wxID_ANY, wxT("Delete"), wxT("delete"));
    itemHyperLink->SetVisitedColour(itemHyperLink->GetNormalColour());
    m_Sizer->Add(itemHyperLink, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 0);
    m_ScrolledWindow->SetScrollRate(0, m_RowHeight);
    m_ScrolledWindow->FitInside();
    m_ScrolledWindow->Refresh();
}

wxSizerItem* CMacroAuthorNamesPanel::x_FindSingleAuthor(wxWindow* wnd, wxSizerItemList& itemList)
{
    if (wnd == nullptr) {
        return nullptr;
    }

    wxSizerItem* auth = nullptr;
    for (wxSizerItemList::iterator it = itemList.begin(); it != itemList.end(); ++it) {
        wxWindow* child = (**it).GetWindow();
        if (child) {
            if (dynamic_cast<CSingleAuthorPanel*> (child)) {
                auth = *it;
            }
            if (child == wnd)
                break;
        }
    }
    return auth;
}

static bool s_IsLastRow(wxWindow* wnd, wxSizerItemList& itemList)
{
    bool is_last = false;
    if (NULL == wnd)
        return false;

    for (wxSizerItemList::iterator it = itemList.begin(); it != itemList.end(); ++it)
    {
        wxWindow* child = (**it).GetWindow();
        if (child)
        {
            if (child == wnd)
            {
                is_last = true;
            }
            else
            {
                CSingleAuthorPanel* auth1 = dynamic_cast<CSingleAuthorPanel*> (child);
                if (auth1 && is_last)
                {
                    is_last = false;
                    break;
                }
            }
        }
    }

    return is_last;
}


void CMacroAuthorNamesPanel::AddLastAuthor(wxWindow* link)
{
    // find the row that holds the author being changed
    wxSizerItemList& itemList = m_Sizer->GetChildren();

    if (s_IsLastRow(link, itemList)) {
        CRef<CAuthor> auth(new CAuthor());
        wxWindow* row = new CSingleAuthorPanel(m_ScrolledWindow, *auth);
        x_AddRowToWindow(row);
    }
}

/*
 * Should we show tooltips?
 */

bool CMacroAuthorNamesPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CMacroAuthorNamesPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMacroAuthorNamesPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMacroAuthorNamesPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CMacroAuthorNamesPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMacroAuthorNamesPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMacroAuthorNamesPanel icon retrieval
}

CMacroAuthorNamesPanel::TAuthorsVec CMacroAuthorNamesPanel::GetAuthorNames() const
{
    TAuthorsVec m_AuthorNames;
    wxSizerItemList::iterator node = m_Sizer->GetChildren().begin();
    for (; node != m_Sizer->GetChildren().end(); ++node) {
        if (wxWindow *w = (*node)->GetWindow()) {
            CSingleAuthorPanel* singleauth_panel = dynamic_cast<CSingleAuthorPanel*> (w);
            if (singleauth_panel) {
                m_AuthorNames.push_back(
                    make_tuple(ToStdString(singleauth_panel->m_LastNameCtrl->GetValue())
                        , ToStdString(singleauth_panel->m_FirstNameCtrl->GetValue())
                        , ToStdString(singleauth_panel->m_MiddleInitial->GetValue())
                        , ToStdString(singleauth_panel->m_Suffix->GetStringSelection())));
            }
        }
    }
    return m_AuthorNames;
}

void CMacroAuthorNamesPanel::OnImportAuthorsClicked(wxCommandEvent& event)
{
    ImportAuthorsFromFile();
}

void CMacroAuthorNamesPanel::OnDelete(wxHyperlinkEvent& event)
{
    if (event.GetURL() != wxT("delete")) {
        event.Skip();
        return;
    }
    DeleteRow((wxWindow*)event.GetEventObject());
    event.Skip();
}

void CMacroAuthorNamesPanel::SetAuthors(const CAuth_list& auth_list)
{
    Freeze();
    if (auth_list.IsSetNames() && auth_list.GetNames().IsStd()) {
        while (m_Sizer->GetItemCount() > 0) {
            size_t pos = 0;
            m_Sizer->GetItem(pos)->DeleteWindows();
            m_Sizer->Remove(pos);
        }
    }

    CAuth_list& non_const_authlist = const_cast<CAuth_list&>(auth_list);
    for (auto& it : non_const_authlist.SetNames().SetStd()) {
        wxWindow* row = new CSingleAuthorPanel(m_ScrolledWindow, *it);
        x_AddRowToWindow(row);
    }

    wxPanel::TransferDataToWindow();
    Thaw();
}

void CMacroAuthorNamesPanel::InsertAuthorBefore(wxWindow* link)
{
    // find the row that holds the author being changed
    int index = 0;
    wxSizerItemList::iterator it = m_Sizer->GetChildren().begin();
    while (it != m_Sizer->GetChildren().end()) {
        wxWindow *win = (**it).GetWindow();
        CSingleAuthorPanel* auth = dynamic_cast<CSingleAuthorPanel*> (win);
        if (auth && link == win)
            break;
        ++index;
        ++it;
    }

    CRef<CAuthor> auth(new CAuthor());
    wxWindow* row = new CSingleAuthorPanel(m_ScrolledWindow, *auth);
    x_InsertRowtoWindow(row, index);
    
    if (link) {
        row->MoveBeforeInTabOrder(link);
    }
}

void CMacroAuthorNamesPanel::InsertAuthorAfter(wxWindow* link)
{
    // find the row that holds the author being changed
    size_t index = 0;
    wxSizerItemList::iterator it = m_Sizer->GetChildren().begin();
    while (it != m_Sizer->GetChildren().end())
    {
        wxWindow *win = (**it).GetWindow();
        CSingleAuthorPanel* auth = dynamic_cast<CSingleAuthorPanel*> (win);
        if (auth && link == win)
            break;
        ++index;
        ++it;
    }
    ++index;
    ++it;
    while (it != m_Sizer->GetChildren().end())
    {
        wxWindow *win = (**it).GetWindow();
        CSingleAuthorPanel* auth = dynamic_cast<CSingleAuthorPanel*> (win);
        if (auth)
            break;
        ++index;
        ++it;
    }

    CRef<CAuthor> auth(new CAuthor());
    wxWindow* row = new CSingleAuthorPanel(m_ScrolledWindow, *auth);
    if (index == m_Sizer->GetChildren().size()) {
        x_AddRowToWindow(row);
    }
    else {
        x_InsertRowtoWindow(row, index);
    }

    if (link)
    {
        row->MoveAfterInTabOrder(link);
    }
}

void CMacroAuthorNamesPanel::x_InsertRowtoWindow(wxWindow* row, size_t index)
{
    m_Sizer->Insert(index, row, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 0);
    wxHyperlinkCtrl* itemHyperLink = new CNoTabHyperlinkCtrl(m_ScrolledWindow, wxID_ANY, wxT("Delete"), wxT("delete"));
    itemHyperLink->SetVisitedColour(itemHyperLink->GetNormalColour());
    m_Sizer->Insert(index + 1, itemHyperLink, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 0);
    m_ScrolledWindow->FitInside();
    m_ScrolledWindow->Refresh();
}

void CMacroAuthorNamesPanel::x_MoveRowUp(int row_num)
{
    if (row_num < 1) {
        return;
    }

    wxSizerItemList& itemList = m_Sizer->GetChildren();

    int row = 0;
    wxSizerItemList::iterator it = itemList.begin();
    wxSizerItemList::iterator before = itemList.end();

    while (it != itemList.end())
    {
        CSingleAuthorPanel* auth = dynamic_cast<CSingleAuthorPanel*> ((**it).GetWindow());
        if (auth)
        {
            if (row == row_num - 1)
                before = it;
            ++row;
        }
        ++it;
    }
    if (row_num == row - 1)
        return;

    if (before != itemList.end())
    {
        wxSizerItemList::iterator after = before;
        ++after;
        while (after != itemList.end())
        {
            CSingleAuthorPanel* auth = dynamic_cast<CSingleAuthorPanel*> ((**after).GetWindow());
            if (auth)
                break;
            ++after;
        }

        if (after != itemList.end())
        {
            swap(*before, *after);
            (**before).GetWindow()->MoveBeforeInTabOrder((**after).GetWindow());
        }

    }

    m_Sizer->Layout();
    m_ScrolledWindow->Refresh();
}

int CMacroAuthorNamesPanel::x_FindRow(wxWindow* wnd, wxSizerItemList& itemList)
{
    if (NULL == wnd)
        return -1;

    int row_num = -1;
    for (wxSizerItemList::iterator it = itemList.begin(); it != itemList.end(); ++it)
    {
        wxWindow* child = (**it).GetWindow();
        if (child)
        {
            CSingleAuthorPanel* auth = dynamic_cast<CSingleAuthorPanel*> (child);
            if (auth)
                row_num++;
            if (child == wnd)
                break;
        }
    }

    return row_num;
}


END_NCBI_SCOPE
