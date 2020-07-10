/*  $Id: authornames_panel.cpp 43642 2019-08-13 14:41:50Z asztalos $
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
#include <gui/utils/command_processor.hpp>
#include <gui/widgets/edit/edit_object.hpp>

#include <objects/pub/Pub_equiv.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/biblio/Cit_book.hpp>
#include <objects/biblio/Cit_jour.hpp>
#include <objects/biblio/Cit_let.hpp>
#include <objects/biblio/Cit_pat.hpp>
#include <objects/biblio/Cit_proc.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/biblio/Cit_gen.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>
#include <objects/submit/Contact_info.hpp>

#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/seq_entry_ci.hpp>

#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_composite.hpp>

#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <util/line_reader.hpp>

#include <gui/widgets/wx/unfocused_controls.hpp>
#include <gui/widgets/edit/singleauthor_panel.hpp>
#include <gui/widgets/edit/serial_member_primitive_validators.hpp>

////@begin includes
////@end includes

#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/scrolwin.h>
#include <wx/hyperlink.h>
#include <wx/statline.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/notebook.h>

#include <gui/widgets/edit/edit_obj_view_dlg.hpp>
#include <gui/widgets/edit/edit_object_seq_desc.hpp>
#include <gui/widgets/edit/authornames_panel.hpp>
#include <gui/widgets/edit/author_affiliation_panel.hpp>
#include <gui/widgets/edit/consortium_panel.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CAuthorNamesPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAuthorNamesPanel, CAuthorNamesContainer)


/*
 * CAuthorNamesPanel event table definition
 */

BEGIN_EVENT_TABLE( CAuthorNamesPanel, CAuthorNamesContainer)

////@begin CAuthorNamesPanel event table entries
    EVT_HYPERLINK( ID_ADDAUTH, CAuthorNamesPanel::OnAddauthHyperlinkClicked )
    EVT_HYPERLINK( ID_ADDCONSORTIUM, CAuthorNamesPanel::OnAddconsortiumHyperlinkClicked )
    EVT_HYPERLINK( ID_IMPORT_AUTHORS, CAuthorNamesPanel::OnImportAuthorsHyperlinkClicked )
    EVT_HYPERLINK( ID_IMPORT_AUTHORS_AFFILIATION, CAuthorNamesPanel::OnImportAuthorsAffiliationHyperlinkClicked )
    EVT_HYPERLINK(wxID_ANY, CAuthorNamesPanel::OnDelete)

    EVT_BUTTON(ID_REPLACE_ALL_AUTHORS, CAuthorNamesPanel::OnReplaceAllAuthors)
    EVT_BUTTON(ID_REPLACE_SAME_AUTHORS, CAuthorNamesPanel::OnReplaceSameAuthors)
////@end CAuthorNamesPanel event table entries
END_EVENT_TABLE()


/*
 * CAuthorNamesPanel constructors
 */

CAuthorNamesPanel::CAuthorNamesPanel()
    : CAuthorNamesContainer()
{
    Init();
}

CAuthorNamesPanel::CAuthorNamesPanel( wxWindow* parent, CSerialObject& object, bool show_replace_buttons,
         wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
    CAuthorNamesContainer(), m_Object(0), m_show_replace_buttons(show_replace_buttons)
{
    Init();
    m_Object = dynamic_cast<CAuth_list*>(&object);
    const CAuth_list& auth_list = dynamic_cast<const CAuth_list&>(*m_Object);
    m_EditedAuthList.Reset((CSerialObject*)CAuth_list::GetTypeInfo()->Create());
    m_EditedAuthList->Assign(auth_list);

    Create(parent, id, pos, size, style);
}


/*
 * CAuthorNamesPanel creator
 */

bool CAuthorNamesPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAuthorNamesPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAuthorNamesPanel creation
    return true;
}


/*
 * CAuthorNamesPanel destructor
 */

CAuthorNamesPanel::~CAuthorNamesPanel()
{
////@begin CAuthorNamesPanel destruction
////@end CAuthorNamesPanel destruction
}


/*
 * Member initialisation
 */

void CAuthorNamesPanel::Init()
{
////@begin CAuthorNamesPanel member initialisation
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
////@end CAuthorNamesPanel member initialisation
}


/*
 * Control creation for CAuthorNamesPanel
 */

void CAuthorNamesPanel::CreateControls()
{    
////@begin CAuthorNamesPanel content construction
    CAuthorNamesPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl4 = new wxHyperlinkCtrl( itemPanel1, ID_ADDAUTH, _("Add Author"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer3->Add(itemHyperlinkCtrl4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl5 = new wxHyperlinkCtrl( itemPanel1, ID_ADDCONSORTIUM, _("Add Consortium"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer3->Add(itemHyperlinkCtrl5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl6 = new wxHyperlinkCtrl( itemPanel1, ID_IMPORT_AUTHORS, _("Import Authors"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer3->Add(itemHyperlinkCtrl6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl7 = new wxHyperlinkCtrl( itemPanel1, ID_IMPORT_AUTHORS_AFFILIATION, _("Import Authors and Affiliation"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer3->Add(itemHyperlinkCtrl7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticLine* itemStaticLine8 = new wxStaticLine( itemPanel1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine8, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer10, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer10->Add(itemBoxSizer9, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("First Name"), wxDefaultPosition, wxSize(90, -1), wxALIGN_CENTRE );
    itemBoxSizer9->Add(itemStaticText10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemPanel1, wxID_STATIC, _("M.I."), wxDefaultPosition, wxSize(90, -1), wxALIGN_CENTRE );
    itemBoxSizer9->Add(itemStaticText11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel1, wxID_STATIC, _("Last Name"), wxDefaultPosition, wxSize(90, -1), wxALIGN_CENTRE );
    itemBoxSizer9->Add(itemStaticText12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemPanel1, wxID_STATIC, _("Suffix"), wxDefaultPosition, wxSize(50, -1), wxALIGN_CENTRE );
    itemBoxSizer9->Add(itemStaticText13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText14 = new wxStaticText( itemPanel1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer9->Add(itemStaticText14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ScrolledWindow = new wxScrolledWindow( itemPanel1, ID_PANEL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    m_ScrolledWindow->SetMinSize(wxSize(510, 126));
    itemBoxSizer10->Add(m_ScrolledWindow, 1, wxGROW|wxALL, 5);

//    m_Sizer = new wxBoxSizer(wxVERTICAL);
    m_Sizer = new wxFlexGridSizer(0, 2, 0, 0);
    m_ScrolledWindow->SetSizer(m_Sizer);

   CAuth_list& edited_auth_list = dynamic_cast<CAuth_list&>(*m_EditedAuthList);
   bool name_list_exists =  (edited_auth_list.IsSetNames() && edited_auth_list.GetNames().IsStd() && !edited_auth_list.GetNames().GetStd().empty()) && m_show_replace_buttons;
   if (name_list_exists)
   {
       wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
       itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
       
       wxButton* itemButton1 = new wxButton( itemPanel1, ID_REPLACE_ALL_AUTHORS, _("Replace All Authors"), wxDefaultPosition, wxDefaultSize, 0 );
       itemBoxSizer4->Add(itemButton1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
       
       wxButton* itemButton2 = new wxButton( itemPanel1, ID_REPLACE_SAME_AUTHORS, _("Replace Same Authors"), wxDefaultPosition, wxDefaultSize, 0 );
       itemBoxSizer4->Add(itemButton2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
   }



    ////@end CAuthorNamesPanel content construction
}


bool CAuthorNamesPanel::TransferDataToWindow()
{
    Freeze();
    while (m_Sizer->GetItemCount() > 0) {
        size_t pos = 0;
        m_Sizer->GetItem(pos)->DeleteWindows();
        m_Sizer->Remove(pos);
    }

    m_RowHeight = 0;
    int total_height = 0;
    int total_width = 0;
    int num_rows = 0;
    int max_rows_displayed = 6;

    CAuth_list& edited_auth_list = dynamic_cast<CAuth_list&>(*m_EditedAuthList);
    if (edited_auth_list.IsSetNames() && edited_auth_list.GetNames().IsStd() && !edited_auth_list.GetNames().GetStd().empty()) {
        for (list< CRef< CAuthor > >::iterator auth = edited_auth_list.SetNames().SetStd().begin();
             auth != edited_auth_list.SetNames().SetStd().end();
             auth++) {    
            if (CSingleAuthorPanel::IsPlaceholder(**auth)) {
                // don't add
            } else if ((*auth)->GetName().IsName()) {
                wxWindow* row = new CSingleAuthorPanel(m_ScrolledWindow, **auth);
                m_Sizer->Add(row, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0); 
                wxHyperlinkCtrl* itemHyperLink = new CNoTabHyperlinkCtrl(m_ScrolledWindow, ID_DELETELINK, wxT("Delete"), wxT("delete"));
                itemHyperLink->SetVisitedColour(itemHyperLink->GetNormalColour());
                m_Sizer->Add(itemHyperLink, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

                int row_height, row_width;
                row->GetClientSize(&row_width, &row_height);
                if (row_width > total_width) {
                    total_width = row_width;
                }
                if (num_rows < max_rows_displayed) {
                    total_height += row_height;
                }
                num_rows++;
                if (m_RowHeight == 0 || m_RowHeight > row_height) {
                    m_RowHeight = row_height;
                }
            } else if ((*auth)->GetName().IsConsortium()) {
                wxWindow* row = new CConsortiumPanel (m_ScrolledWindow, **auth);
                m_Sizer->Add(row, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
                wxHyperlinkCtrl* itemHyperLink = new CNoTabHyperlinkCtrl(m_ScrolledWindow, ID_DELETELINK, wxT("Delete"), wxT("delete"));
                itemHyperLink->SetVisitedColour(itemHyperLink->GetNormalColour());
                m_Sizer->Add(itemHyperLink, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

                int row_height, row_width;
                row->GetClientSize(&row_width, &row_height);
                if (row_width > total_width) {
                    total_width = row_width;
                }
                if (num_rows < max_rows_displayed) {
                    total_height += row_height;
                }
                num_rows++;
                if (m_RowHeight == 0 || m_RowHeight > row_height) {
                    m_RowHeight = row_height;
                }
            }
        }
    }

    // now add one empty row for user to fill in

    CRef<CAuthor> auth(new CAuthor());
    edited_auth_list.SetNames().SetStd().push_back(auth);
    wxWindow* row = new CSingleAuthorPanel(m_ScrolledWindow, *auth);
    x_AddRowToWindow(row);
    int row_height, row_width;
    row->GetClientSize(&row_width, &row_height);
    if (row_width > total_width) {
        total_width = row_width;
    }
    if (num_rows < max_rows_displayed) {
        total_height += row_height;
    }
    if (m_RowHeight == 0 || m_RowHeight > row_height) {
        m_RowHeight = row_height;
    }

        
    m_ScrolledWindow->SetVirtualSize(total_width + 10, total_height);
    m_ScrolledWindow->SetScrollRate(0, m_RowHeight);

    m_ScrolledWindow->FitInside();
    Layout();
    Thaw();

    if (!wxPanel::TransferDataToWindow())
        return false;

    return true;
}


bool CAuthorNamesPanel::IsAuthorEmpty (const CAuthor& auth)
{
    bool is_empty = true;

    if (!auth.IsSetName()) {
        is_empty = true;
    } else if (auth.GetName().IsName()) {
        if (auth.GetName().GetName().IsSetFirst()
            || auth.GetName().GetName().IsSetInitials()
            || auth.GetName().GetName().IsSetLast())
        {
            is_empty = false;
        }
    } else if (auth.GetName().IsConsortium()) {
        if (!NStr::IsBlank(auth.GetName().GetConsortium())) 
        {
            is_empty = false;
        }
    }
    return is_empty;
}


bool CAuthorNamesPanel::x_GetAuthors(bool keep_blank)
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    CAuth_list& edited_auth_list = dynamic_cast<CAuth_list&>(*m_EditedAuthList);
    edited_auth_list.ResetNames();

    wxSizerItemList::iterator node = m_Sizer->GetChildren().begin();
    for (; node != m_Sizer->GetChildren().end(); ++node) {
        wxWindow *w = (*node)->GetWindow();
        if (!w) {
            continue;
        }
        CRef<CAuthor> edited_auth;
        CSingleAuthorPanel* singleauth_panel = dynamic_cast<CSingleAuthorPanel*> (w);
        if (singleauth_panel) {
            edited_auth = singleauth_panel->GetAuthor();
        } else {
            CConsortiumPanel *consort_panel = dynamic_cast<CConsortiumPanel*> (w);
            if (consort_panel) {
                edited_auth = consort_panel->GetAuthor();
            }
        }
        // only take the entries that aren't empty
        if (edited_auth && (keep_blank || !IsAuthorEmpty(*edited_auth))) {
            CRef<CAuthor> new_auth(new CAuthor());
            new_auth->Assign(*edited_auth);
            edited_auth_list.SetNames().SetStd().push_back(new_auth);
        }

    }

    // If all names were deleted create a dummy last name, see GB-3111
    if (!edited_auth_list.IsSetNames() || ! edited_auth_list.SetNames().IsStd() || edited_auth_list.SetNames().SetStd().empty())
    {
        CRef<CAuthor> new_auth(new CAuthor());
        new_auth->SetName().SetName().SetLast("?");
        edited_auth_list.SetNames().SetStd().push_back(new_auth);
    }
    return true;
}


bool CAuthorNamesPanel::TransferDataFromWindow()
{
    return x_GetAuthors();
}


void CAuthorNamesPanel::PopulateAuthors (CAuth_list& auth_list)
{
    TransferDataFromWindow();
    CAuth_list& edited_auth_list = dynamic_cast<CAuth_list&>(*m_EditedAuthList);
    if (edited_auth_list.IsSetNames()) {
        auth_list.SetNames(edited_auth_list.SetNames());      
    } else {
        auth_list.ResetNames();
    }
}


void CAuthorNamesPanel::SetAuthors(const CAuth_list& auth_list)
{
    CAuth_list& edited_auth_list = dynamic_cast<CAuth_list&>(*m_EditedAuthList);
    edited_auth_list.Assign(auth_list);
    TransferDataToWindow();
}


int CAuthorNamesPanel::x_FindRow(wxWindow* wnd, wxSizerItemList& itemList)
{
    if (NULL == wnd)
        return -1;

    int row_num = -1;
    for(wxSizerItemList::iterator it = itemList.begin(); it != itemList.end(); ++it) 
    {
        wxWindow* child = (**it).GetWindow();
        if (child)
        {
            CSingleAuthorPanel* auth1 = dynamic_cast<CSingleAuthorPanel*> (child);
            CConsortiumPanel* auth2 = dynamic_cast<CConsortiumPanel*> (child);
            if (auth1 || auth2) 
                row_num++;
            if (child == wnd)
                break;
        }
    }

    return row_num;
}

wxSizerItem* CAuthorNamesPanel::x_FindSingleAuthor(wxWindow* wnd, wxSizerItemList& itemList)
{
    wxSizerItem* auth = NULL;
    if (NULL == wnd)
        return auth;

    for(wxSizerItemList::iterator it = itemList.begin(); it != itemList.end(); ++it) 
    {
        wxWindow* child = (**it).GetWindow();
        if (child)
        {
            CSingleAuthorPanel* auth1 = dynamic_cast<CSingleAuthorPanel*> (child);
            CConsortiumPanel* auth2 = dynamic_cast<CConsortiumPanel*> (child);
            if (auth1 || auth2) 
            {
                auth = *it;
            }
            if  (child == wnd)
                break;;
        }
        
    }
   
    return auth;
}

static bool s_IsLastRow(wxWindow* wnd, wxSizerItemList& itemList) 
{
    bool is_last = false;
    if (NULL == wnd)
        return false;

    for(wxSizerItemList::iterator it = itemList.begin(); it != itemList.end(); ++it) 
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
                CConsortiumPanel* auth2 = dynamic_cast<CConsortiumPanel*> (child);
                if ((auth1 || auth2) && is_last)
                {
                    is_last = false;
                    break;
                }
            }
        }
    }

    return is_last;
}


void CAuthorNamesPanel::AddLastAuthor (wxWindow* link)
{
    // find the row that holds the author being changed
    wxSizerItemList& itemList = m_Sizer->GetChildren();

    if (s_IsLastRow(link, itemList)) {
        CAuth_list& edited_auth_list = dynamic_cast<CAuth_list&>(*m_EditedAuthList);
        CRef<CAuthor> auth(new CAuthor());
        edited_auth_list.SetNames().SetStd().push_back(auth);
        
        wxWindow* row = new CSingleAuthorPanel(m_ScrolledWindow, *auth);
        x_AddRowToWindow(row);
    }
}

void CAuthorNamesPanel::OnDelete(wxHyperlinkEvent& event)
{
    if (event.GetURL() != wxT("delete")) {
        event.Skip();
        return;
    }
    DeleteRow((wxWindow*)event.GetEventObject());
}

void CAuthorNamesPanel::x_MoveRowUp(int row_num) 
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
        CSingleAuthorPanel* auth1 = dynamic_cast<CSingleAuthorPanel*> ((**it).GetWindow());
        CConsortiumPanel* auth2 = dynamic_cast<CConsortiumPanel*> ((**it).GetWindow());
        if (auth1 || auth2) 
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
            CSingleAuthorPanel* auth1 = dynamic_cast<CSingleAuthorPanel*> ((**after).GetWindow());
            CConsortiumPanel* auth2 = dynamic_cast<CConsortiumPanel*> ((**after).GetWindow());
            if (auth1 || auth2)
                break;
            ++after;
        }

        if (after != itemList.end())   
        {
            swap (*before, *after);
            (**before).GetWindow()->MoveBeforeInTabOrder((**after).GetWindow());
        }
        
    }

    m_Sizer->Layout();
    m_ScrolledWindow->Refresh();

}

void CAuthorNamesPanel::InsertAuthorBefore (wxWindow* link)
{
    // find the row that holds the author being changed
    wxSizerItemList& itemList = m_Sizer->GetChildren();

    int index = 0;
    wxSizerItemList::iterator it = itemList.begin();
    while (it != itemList.end()) 
    {
        wxWindow *win = (**it).GetWindow();
        CSingleAuthorPanel* auth1 = dynamic_cast<CSingleAuthorPanel*> (win);
        CConsortiumPanel* auth2 = dynamic_cast<CConsortiumPanel*> (win);
        if ((auth1 || auth2) && link == win)
            break;
        ++index;
        ++it;
    }

    CRef<CAuthor> auth(new CAuthor());    
    wxWindow* row = new CSingleAuthorPanel(m_ScrolledWindow, *auth);
    x_InsertRowtoWindow(row, index);
    if (link)
    {
        row->MoveBeforeInTabOrder(link);
    }
}


void CAuthorNamesPanel::InsertAuthorAfter (wxWindow* link)
{
    // find the row that holds the author being changed
    wxSizerItemList& itemList = m_Sizer->GetChildren();

    size_t index = 0;
    wxSizerItemList::iterator it = itemList.begin();
    while (it != itemList.end()) 
    {
        wxWindow *win = (**it).GetWindow();
        CSingleAuthorPanel* auth1 = dynamic_cast<CSingleAuthorPanel*> (win);
        CConsortiumPanel* auth2 = dynamic_cast<CConsortiumPanel*> (win);
        if ((auth1 || auth2) && link == win)
            break;
        ++index;
        ++it;
    }
    ++index;
    ++it;
    while (it != itemList.end()) 
    {
        wxWindow *win = (**it).GetWindow();
        CSingleAuthorPanel* auth1 = dynamic_cast<CSingleAuthorPanel*> (win);
        CConsortiumPanel* auth2 = dynamic_cast<CConsortiumPanel*> (win);
        if ((auth1 || auth2))
            break;
        ++index;
        ++it;
    }

    CRef<CAuthor> auth(new CAuthor());    
    wxWindow* row = new CSingleAuthorPanel(m_ScrolledWindow, *auth);
    if (index == itemList.size()) {
        x_AddRowToWindow (row);
    } else {
        x_InsertRowtoWindow(row, index);
    }

    if (link)
    {
        row->MoveAfterInTabOrder(link);
    }
}

void CAuthorNamesPanel::x_InsertRowtoWindow(wxWindow* row, size_t index)
{
    m_Sizer->Insert(index, row, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 0);
    wxHyperlinkCtrl* itemHyperLink = new CNoTabHyperlinkCtrl(m_ScrolledWindow, wxID_ANY, wxT("Delete"), wxT("delete"));
    itemHyperLink->SetVisitedColour(itemHyperLink->GetNormalColour());
    m_Sizer->Insert(index + 1, itemHyperLink, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 0);
    x_AdjustScroll(row, false);
}

/*
 * Should we show tooltips?
 */

bool CAuthorNamesPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CAuthorNamesPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAuthorNamesPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAuthorNamesPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CAuthorNamesPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAuthorNamesPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAuthorNamesPanel icon retrieval
}

void CAuthorNamesPanel::x_AddNewEmptyAuthor()
{
    Freeze();
    CAuth_list& edited_auth_list = dynamic_cast<CAuth_list&>(*m_EditedAuthList);
    CRef<CAuthor> auth(new CAuthor());
    edited_auth_list.SetNames().SetStd().push_back(auth);
    
    wxWindow* row = new CSingleAuthorPanel(m_ScrolledWindow, *auth);
    x_AddRowToWindow (row);
    Thaw();
}

/*
 * wxEVT_COMMAND_HYPERLINK event handler for ID_ADDAUTH
 */

void CAuthorNamesPanel::OnAddauthHyperlinkClicked( wxHyperlinkEvent& event )
{
    x_AddNewEmptyAuthor();
}


/*
 * wxEVT_COMMAND_HYPERLINK event handler for ID_ADDCONSORTIUM
 */

void CAuthorNamesPanel::OnAddconsortiumHyperlinkClicked( wxHyperlinkEvent& event )
{
    CAuth_list& edited_auth_list = dynamic_cast<CAuth_list&>(*m_EditedAuthList);
    CRef<CAuthor> auth(new CAuthor());
    edited_auth_list.SetNames().SetStd().push_back(auth);
    
    wxWindow* row = new CConsortiumPanel(m_ScrolledWindow, *auth);
    x_AddRowToWindow (row);
}


// utility function for adding new control to window
// adjusts scroll rate if necessary
void CAuthorNamesPanel::x_AddRowToWindow(wxWindow* row)
{
    m_Sizer->Add(row, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxHyperlinkCtrl* itemHyperLink = new CNoTabHyperlinkCtrl(m_ScrolledWindow, wxID_ANY, wxT("Delete"), wxT("delete"));
    itemHyperLink->SetVisitedColour(itemHyperLink->GetNormalColour());
    m_Sizer->Add(itemHyperLink, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);
    x_AdjustScroll(row);
}


void CAuthorNamesPanel::x_AdjustScroll(wxWindow* row, bool scroll_to_end)
{
    int row_height;
    row->GetClientSize(0, &row_height);
    if (m_RowHeight == 0 || row_height < m_RowHeight) {
        m_RowHeight = row_height;
        m_ScrolledWindow->SetScrollRate(0, row_height);
    }
    m_ScrolledWindow->FitInside();

    if (scroll_to_end) {
        int h, hv;
        m_ScrolledWindow->GetClientSize(0, &h);
        m_ScrolledWindow->GetVirtualSize(0, &hv);

        if (hv > h) {
            int xUnit, yUnit;
            m_ScrolledWindow->GetScrollPixelsPerUnit(&xUnit, &yUnit);
            if (yUnit > 0) {
                m_ScrolledWindow->Scroll(-1, (hv - h + yUnit)/yUnit);
            }
        }
    }
    m_ScrolledWindow->Refresh();
}

void CAuthorNamesPanel::OnImportAuthorsHyperlinkClicked( wxHyperlinkEvent& event )
{
    ImportAuthorsFromFile();
}

/*
* wxEVT_COMMAND_HYPERLINK event handler for ID_IMPORT_AUTHORS_AFFILIATION
*/

void CAuthorNamesPanel::OnImportAuthorsAffiliationHyperlinkClicked(wxHyperlinkEvent& event)
{
    wxFileDialog asn_open_file(this, wxT("Import authors and affiliation from file"), m_WorkDir, wxEmptyString,
                                CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
                                CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                                wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (asn_open_file.ShowModal() == wxID_OK) {
        wxString path = asn_open_file.GetPath();
        if (path.IsEmpty()) return;

        CNcbiIfstream istr(path.fn_str());
        try
        {
            OnImport(istr);
            x_SetAffiliation();
        }
        catch (const CException& e) {
            string msg = "Authors and affiliation could not be imported";
            wxMessageBox(ToWxString(msg), wxT("Error"), wxOK | wxICON_ERROR);
            LOG_POST(Info << msg << ": " << e.GetMsg());
        }
    }
}

bool CAuthorNamesPanel::IsImportEnabled()
{   
    return true;
}

bool CAuthorNamesPanel::IsExportEnabled()
{
    return true;
}

CRef<CSerialObject> CAuthorNamesPanel::OnExport()
{
    CRef<CSerialObject> so((CSerialObject*)(new CAuth_list));
    PopulateAuthors (dynamic_cast<CAuth_list&>(*so));
    return so;   
}

void CAuthorNamesPanel::OnImport( CNcbiIfstream &istr)
{
    CRef<CAuth_list> new_auth_list(new CAuth_list);
    istr >> MSerial_AsnText >> *new_auth_list;
    SetAuthors(*new_auth_list);            
}

void CAuthorNamesPanel::x_SetAffiliation()
{
    wxNotebook* notebook = dynamic_cast<wxNotebook*>(this->GetParent());
    if (!notebook) {
        return;
    }

    CAuthorAffiliationPanel* affil_panel = 0;

    for (size_t i = 0; i < notebook->GetPageCount(); ++i) {
        affil_panel = dynamic_cast<CAuthorAffiliationPanel*>(notebook->GetPage(i));
        if (affil_panel) {
            break;
        }
    }

    if (!affil_panel) {
        return;
    }

    CAuth_list& edited_auth_list = dynamic_cast<CAuth_list&>(*m_EditedAuthList);
    if (edited_auth_list.IsSetAffil()) {
        affil_panel->SetAffil(CRef<CAffil>(&(edited_auth_list.SetAffil())));
    }
    else {
        affil_panel->SetAffil(CRef<CAffil>());
    }
}

ICommandProccessor* CAuthorNamesPanel::GetUndoManager()
{
    ICommandProccessor *cmd_processor = NULL;
    wxWindow *win = GetParent();
    while(win)
    {
        CEditObjViewDlg *parent = dynamic_cast<CEditObjViewDlg*>(win);
        if (parent)
        {
            cmd_processor = parent->GetUndoManager();
            break;
        }
        CEditObjViewDlgModal *parent2 = dynamic_cast<CEditObjViewDlgModal*>(win);
        if (parent2)
        {
            cmd_processor = parent2->GetUndoManager();
            break;
        }
        win = win->GetParent();
    }

    return cmd_processor;
}

CSeq_entry_Handle CAuthorNamesPanel::GetSeqEntryHandle()
{
    CSeq_entry_Handle seh;
    wxWindow *win = GetParent();
    while(win)
    {
        CSehHolderPanel *parent = dynamic_cast<CSehHolderPanel*>(win);
        if (parent)
        {
            seh = parent->GetSEH();
            break;
        }
        win = win->GetParent();
    }
    if (seh)
        seh = seh.GetTopLevelEntry();
    return seh;
}


void CAuthorNamesPanel::OnReplaceAllAuthors( wxCommandEvent& event )
{
    TransferDataFromWindow();
    CAuth_list& edited_auth_list = dynamic_cast<CAuth_list&>(*m_EditedAuthList);
    if (!edited_auth_list.IsSetNames())
        return;
    CAuth_list::TNames& edited_names = edited_auth_list.SetNames();

    ICommandProccessor *cmd_processor = GetUndoManager();
    if (!cmd_processor)
        return;
    CSeq_entry_Handle seh = GetSeqEntryHandle();
    if (!seh)
        return;

    CAuth_list* original = dynamic_cast<CAuth_list*>(m_Object);
    if (!original->IsSetNames())
        return;

    CRef<CCmdComposite> comp_cmd(new CCmdComposite("Replace All Authors"));
    bool modified = false;
    for ( CSeq_entry_CI entry_it(seh, CSeq_entry_CI::fRecursive|CSeq_entry_CI::fIncludeGivenEntry); entry_it; ++entry_it ) 
    {
        for ( CSeqdesc_CI pub_it(*entry_it, CSeqdesc::e_Pub, 1); pub_it; ++pub_it) 
        {
            CRef<CSeqdesc> replacement(new CSeqdesc());
            replacement->Assign(*pub_it);

            NON_CONST_ITERATE (CPub_equiv::Tdata, pub, replacement->SetPub().SetPub().Set()) 
            {
                switch ((*pub)->Which()) 
                {
                case CPub::e_Gen:
                    (*pub)->SetGen().SetAuthors().SetNames(edited_names);
                    break;
                case CPub::e_Article:
                    (*pub)->SetArticle().SetAuthors().SetNames(edited_names);
                    break;
                case CPub::e_Book:
                    (*pub)->SetBook().SetAuthors().SetNames(edited_names);
                    break;
                case CPub::e_Proc:
                    (*pub)->SetProc().SetBook().SetAuthors().SetNames(edited_names);
                    break;
                case CPub::e_Sub:
                    (*pub)->SetSub().SetAuthors().SetNames(edited_names);
                    break;
                case CPub::e_Patent:
                    (*pub)->SetPatent().SetAuthors().SetNames(edited_names);
                    break;
                case CPub::e_Man:
                    (*pub)->SetMan().SetCit().SetAuthors().SetNames(edited_names);
                    break;
                default:
                    (*pub)->SetGen().SetAuthors().SetNames(edited_names);
                    break;
                }
                modified = true;
            }
            if (modified)
            {
                CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(pub_it.GetSeq_entry_Handle(), *pub_it, *replacement));
                comp_cmd->AddCommand(*cmd);
            }
        }
    }
    if (modified)
    {
        cmd_processor->Execute(comp_cmd);
        CRef<CAuth_list::TNames> new_names(new CAuth_list::TNames);
        new_names->Assign(edited_names);
        original->SetNames(*new_names);
    }
}

void CAuthorNamesPanel::OnReplaceSameAuthors( wxCommandEvent& event )
{
    TransferDataFromWindow();
    CAuth_list& edited_auth_list = dynamic_cast<CAuth_list&>(*m_EditedAuthList);
    if (!edited_auth_list.IsSetNames())
        return;
    CAuth_list::TNames& edited_names = edited_auth_list.SetNames();

    ICommandProccessor *cmd_processor = GetUndoManager();
    if (!cmd_processor)
        return;
    CSeq_entry_Handle seh = GetSeqEntryHandle();
    if (!seh)
        return;

    CAuth_list* original = dynamic_cast<CAuth_list*>(m_Object);
    if (!original->IsSetNames())
        return;
    const CAuth_list::TNames& original_names = original->GetNames();

    CRef<CCmdComposite> comp_cmd(new CCmdComposite("Replace Same Authors"));
    bool modified = false;
    for ( CSeq_entry_CI entry_it(seh, CSeq_entry_CI::fRecursive|CSeq_entry_CI::fIncludeGivenEntry); entry_it; ++entry_it ) 
    {
        for ( CSeqdesc_CI pub_it(*entry_it, CSeqdesc::e_Pub, 1); pub_it; ++pub_it) 
        {
            CRef<CSeqdesc> replacement(new CSeqdesc());
            replacement->Assign(*pub_it);

            NON_CONST_ITERATE (CPub_equiv::Tdata, pub, replacement->SetPub().SetPub().Set()) 
            {
                switch ((*pub)->Which()) 
                {
                case CPub::e_Gen:
                    if ((*pub)->SetGen().SetAuthors().SetNames().Equals(original_names))
                    {
                        (*pub)->SetGen().SetAuthors().SetNames(edited_names);
                        modified = true;
                    }
                    break;
                case CPub::e_Article:
                    if ((*pub)->SetArticle().SetAuthors().SetNames().Equals(original_names))
                    {
                        (*pub)->SetArticle().SetAuthors().SetNames(edited_names);
                        modified = true;
                    }
                    break;
                case CPub::e_Book:
                    if ((*pub)->SetBook().SetAuthors().SetNames().Equals(original_names))
                    {
                        (*pub)->SetBook().SetAuthors().SetNames(edited_names);
                        modified = true;
                    }
                    break;
                case CPub::e_Proc:
                    if ((*pub)->SetProc().SetBook().SetAuthors().SetNames().Equals(original_names))
                    {
                        (*pub)->SetProc().SetBook().SetAuthors().SetNames(edited_names);
                        modified = true;
                    }
                    break;
                case CPub::e_Sub:
                    if ((*pub)->SetSub().SetAuthors().SetNames().Equals(original_names))
                    {
                        (*pub)->SetSub().SetAuthors().SetNames(edited_names);
                        modified = true;
                    }
                    break;
                case CPub::e_Patent:
                    if ((*pub)->SetPatent().SetAuthors().SetNames().Equals(original_names))
                    {
                        (*pub)->SetPatent().SetAuthors().SetNames(edited_names);
                        modified = true;
                    }
                    break;
                case CPub::e_Man:
                    if ((*pub)->SetMan().SetCit().SetAuthors().SetNames().Equals(original_names))
                    {
                        (*pub)->SetMan().SetCit().SetAuthors().SetNames(edited_names);
                        modified = true;
                    }
                    break;
                default:
                    if ((*pub)->SetGen().SetAuthors().SetNames().Equals(original_names))
                    {
                        (*pub)->SetGen().SetAuthors().SetNames(edited_names);
                        modified = true;
                    }
                    break;
                }
            }
            if (modified)
            {
                CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(pub_it.GetSeq_entry_Handle(), *pub_it, *replacement));
                comp_cmd->AddCommand(*cmd);
            }
        }
    }
    if (modified)
    {
        cmd_processor->Execute(comp_cmd);
        CRef<CAuth_list::TNames> new_names(new CAuth_list::TNames);
        new_names->Assign(edited_names);
        original->SetNames(*new_names);
    }
}

void CAuthorNamesPanel::ApplySubmitBlock(objects::CSubmit_block& block)
{
    m_SubmitBlock.Reset(&block);
    SetAuthors(m_SubmitBlock->SetCit().SetAuthors());        
}

void CAuthorNamesPanel::ApplyCommand()
{
    if (!m_SubmitBlock)
        return;
    PopulateAuthors(m_SubmitBlock->SetCit().SetAuthors());
    CRef<objects::CCit_sub> empty(new objects::CCit_sub);
    empty->SetAuthors();
    if (m_SubmitBlock->IsSetCit() && m_SubmitBlock->GetCit().Equals(*empty))
        return;
    if (!m_SubmitBlock->IsSetCit() || !m_SubmitBlock->GetCit().IsSetAuthors() || !m_SubmitBlock->GetCit().GetAuthors().IsSetAffil()) {
        if (m_SubmitBlock->IsSetContact() && m_SubmitBlock->GetContact().IsSetContact() && m_SubmitBlock->GetContact().GetContact().IsSetAffil()) {
            m_SubmitBlock->SetCit().SetAuthors().SetAffil().Assign(m_SubmitBlock->GetContact().GetContact().GetAffil());
        }
    }
    GetParentWizard(this)->ApplySubmitCommand();    
}

END_NCBI_SCOPE

