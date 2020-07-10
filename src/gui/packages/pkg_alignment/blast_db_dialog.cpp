/*  $Id: blast_db_dialog.cpp 44121 2019-10-31 15:23:32Z katargir $
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
 * Authors: Andrey Yazhuk
 *
 * File Description:
 *      A dialog for browsing BLAST Databases.
 */


#include <ncbi_pch.hpp>


#include <gui/packages/pkg_alignment/blast_db_dialog.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/treectrl.h>
#include <wx/wupdlock.h>


////@begin XPM images
////@end XPM images


BEGIN_NCBI_SCOPE

class CBLASTDlgItemData : public wxTreeItemData
{
public:
    CBLASTDlgItemData(CBLAST_Dlg_Item* item)  :   m_Item(item)   {}

    CRef<CBLAST_Dlg_Item>   m_Item;
};

///////////////////////////////////////////////////////////////////////////////
///
CBLAST_Dlg_Item::CBLAST_Dlg_Item()
:   m_LabelPos(string::npos),
    m_LabelLength(0),
    m_Visible(true),
    m_Expanded(false),
    m_Parent(NULL),
    m_ChildItems(NULL)
{
}

CBLAST_Dlg_Item::CBLAST_Dlg_Item(const string& label, const string& path, bool is_database)
:   //m_Label(label),
    m_Path(path),
    m_IsDatabase(is_database),
    m_Visible(true),
    m_Expanded(false),
    m_Parent(NULL),
    m_ChildItems(NULL)
{
    m_LabelPos = path.find(label);
    _ASSERT(m_LabelPos != string::npos);
    m_LabelLength = label.size();

}

wxString CBLAST_Dlg_Item::GetLabel() const {
    if (m_LabelLength == 0)
        return wxT("");
    return wxString(&m_Path[m_LabelPos], m_LabelLength); 
}


CBLAST_Dlg_Item::~CBLAST_Dlg_Item()
{
    m_Parent = NULL;
    delete m_ChildItems;
}

CBLAST_Dlg_Item* CBLAST_Dlg_Item::GetChildByLabel(const string& label)
{
    if(m_ChildItems)    {
        string l = label;
        TNameToItemMap::iterator it = m_ChildItems->find(l);
        if(it != m_ChildItems->end()) {
            return it->second;
        }
    }
    return NULL;
}


void CBLAST_Dlg_Item::AddChild(CBLAST_Dlg_Item& item)
{
    _ASSERT(item.m_Parent == NULL);
//    _ASSERT(m_ChildItems == NULL  ||  m_ChildItems->find(item.m_Label) == m_ChildItems->end());
    _ASSERT(m_ChildItems == NULL  ||  m_ChildItems->find(item.GetLabel()) == m_ChildItems->end());

    if(m_ChildItems == NULL)    {
        m_ChildItems = new TNameToItemMap;
    }
    m_ChildItems->insert(TNameToItemMap::value_type(item.GetLabel(), CRef<CBLAST_Dlg_Item>(&item)));
//    m_ChildItems->insert(TNameToItemMap::value_type(item.m_Label, CRef<CBLAST_Dlg_Item>(&item)));
    item.m_Parent = this;
}


///////////////////////////////////////////////////////////////////////////////
///
IMPLEMENT_DYNAMIC_CLASS( CBLAST_DB_Dialog, CDialog )

BEGIN_EVENT_TABLE( CBLAST_DB_Dialog, CDialog )
////@begin CBLAST_DB_Dialog event table entries
    EVT_INIT_DIALOG( CBLAST_DB_Dialog::OnInitDialog )
    EVT_TEXT( ID_SEARCH, CBLAST_DB_Dialog::OnSearchTextUpdated )
    EVT_BUTTON( ID_RESET_BTN, CBLAST_DB_Dialog::OnResetBtnClick )
    EVT_TREE_SEL_CHANGED( ID_TREECTRL, CBLAST_DB_Dialog::OnTreectrlSelChanged )
    EVT_TREE_ITEM_ACTIVATED( ID_TREECTRL, CBLAST_DB_Dialog::OnTreectrlItemActivated )
    EVT_TREE_ITEM_COLLAPSED( ID_TREECTRL, CBLAST_DB_Dialog::OnTreectrlItemCollapsed )
    EVT_TREE_ITEM_EXPANDED( ID_TREECTRL, CBLAST_DB_Dialog::OnTreectrlItemExpanded )
    EVT_BUTTON( wxID_OK, CBLAST_DB_Dialog::OnOkClick )
////@end CBLAST_DB_Dialog event table entries
END_EVENT_TABLE()


CBLAST_DB_Dialog::CBLAST_DB_Dialog()
{
    Init();
}


CBLAST_DB_Dialog::CBLAST_DB_Dialog( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CBLAST_DB_Dialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CBLAST_DB_Dialog creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CBLAST_DB_Dialog creation
    return true;
}


CBLAST_DB_Dialog::~CBLAST_DB_Dialog()
{
////@begin CBLAST_DB_Dialog destruction
////@end CBLAST_DB_Dialog destruction
}


void CBLAST_DB_Dialog::Init()
{
////@begin CBLAST_DB_Dialog member initialisation
    m_SearchCtrl = NULL;
    m_StatusText = NULL;
    m_TreeCtrl = NULL;
    m_OKBtn = NULL;
////@end CBLAST_DB_Dialog member initialisation
    m_DbMap = NULL;
}


void CBLAST_DB_Dialog::CreateControls()
{
////@begin CBLAST_DB_Dialog content construction
    CBLAST_DB_Dialog* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(2, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer3->Add(itemBoxSizer5, 0, wxGROW|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    m_SearchCtrl = new wxTextCtrl( itemCDialog1, ID_SEARCH, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(m_SearchCtrl, 1, wxGROW|wxALL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    wxButton* itemButton7 = new wxButton( itemCDialog1, ID_RESET_BTN, _("Clear"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    itemFlexGridSizer3->Add(wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x, wxDLG_UNIT(itemCDialog1, wxSize(-1, 5)).y, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    m_StatusText = new wxStaticText( itemCDialog1, wxID_STATUS, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(m_StatusText, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    itemFlexGridSizer3->AddGrowableCol(1);

    m_TreeCtrl = new wxTreeCtrl( itemCDialog1, ID_TREECTRL, wxDefaultPosition, wxDLG_UNIT(itemCDialog1, wxSize(240, 160)), wxTR_HAS_BUTTONS |wxTR_FULL_ROW_HIGHLIGHT|wxTR_HIDE_ROOT|wxTR_SINGLE|wxTR_GBENCH_LINES );
    itemBoxSizer2->Add(m_TreeCtrl, 1, wxGROW|wxALL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer11 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer11, 0, wxALIGN_RIGHT|wxALL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);
    m_OKBtn = new wxButton( itemCDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer11->AddButton(m_OKBtn);

    wxButton* itemButton13 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer11->AddButton(itemButton13);

    itemStdDialogButtonSizer11->Realize();

////@end CBLAST_DB_Dialog content construction
}


bool CBLAST_DB_Dialog::ShowToolTips()
{
    return true;
}


wxBitmap CBLAST_DB_Dialog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBLAST_DB_Dialog bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBLAST_DB_Dialog bitmap retrieval
}


wxIcon CBLAST_DB_Dialog::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBLAST_DB_Dialog icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBLAST_DB_Dialog icon retrieval
}


void CBLAST_DB_Dialog::SetToolName(const string& tool)
{
    string s = tool + " - Select BLAST Databases";
    SetTitle(ToWxString(s));
}


void CBLAST_DB_Dialog::SetDBMap(const TDbMap& map)
{
    m_DbMap = &map;
}


void CBLAST_DB_Dialog::SelectDatabases(vector<string>& databases)
{
    if( ! databases.empty())   {
        m_SelDatabase = databases[0];
    }
}


void CBLAST_DB_Dialog::GetSelectedDatabases(vector<string>& databases)
{
    databases.push_back(m_SelDatabase);
}


void CBLAST_DB_Dialog::OnSearchTextUpdated( wxCommandEvent& event )
{
    wxString s_val = m_SearchCtrl->GetValue();

    string query = ToStdString(s_val);
    x_FilterItems(query);
}


void CBLAST_DB_Dialog::OnResetBtnClick( wxCommandEvent& event )
{
    m_SearchCtrl->SetValue(wxT(""));
    x_FilterItems(kEmptyStr);
}


void CBLAST_DB_Dialog::OnInitDialog( wxInitDialogEvent& event )
{
    CDialog::OnInitDialog(event);

    GUI_AsyncExec([this](ICanceled&) { this->x_CreateItems(); }, wxT("Building list of BLAST DBs..."));

    x_FilterItems(kEmptyStr);

    // select databases
    NON_CONST_ITERATE(TItemDeque, it, m_DbItems)   {
        CBLAST_Dlg_Item& ch_item = **it;
        if(ch_item.GetPath() == m_SelDatabase) {
            m_TreeCtrl->SelectItem(ch_item.GetId());
        }
    }
}


void CBLAST_DB_Dialog::x_CreateItems()
{
    // clean old data, this should delete all previously created items
    m_Root.Reset(new CBLAST_Dlg_Item);
    m_CatItems.clear();
    m_DbItems.clear();

    if (!m_DbMap)
        return;

    vector<string> tokens;

    // build the new item hierarchy
    ITERATE(TDbMap, it, *m_DbMap)   {
        const string& path = it->first;
        tokens.clear();
        NStr::Split(path, "/", tokens);
        CBLAST_Dlg_Item* item = m_Root.GetPointer();
        CBLAST_Dlg_Item* new_item = NULL;
        size_t n = tokens.size();
        for (size_t i = 0; i < n; ++i)  {
            const string& label = tokens[i];
            if (i < n - 1)  {
                // try to find the item for category
                new_item = item->GetChildByLabel(label);
                if (new_item == NULL)    {
                    // create a category item
                    new_item = new CBLAST_Dlg_Item(label, path);
                    item->AddChild(*new_item);
                    m_CatItems.push_back(new_item);
                }
            }
            else {
                // last item - create a DB item
                new_item = new CBLAST_Dlg_Item(label, path, true);
                item->AddChild(*new_item);
                m_DbItems.push_back(new_item);
            }
            item = new_item;
        }
    }
}


void CBLAST_DB_Dialog::x_BuildTreeItems(CBLAST_Dlg_Item& item)
{
    if(item.IsVisible())    {
        const CBLAST_Dlg_Item* parent = item.GetParent();
//        const string& label = item.GetLabel();
        wxString label = item.GetLabel();

        // create wxTreeItem for this item
        wxTreeItemId id;
        CBLASTDlgItemData* data = new CBLASTDlgItemData(&item);
        if(parent)  {
            id = m_TreeCtrl->AppendItem(parent->GetId(), label,
                                        -1, -1, data);
            item.SetId(id);
        } else {
            id = m_TreeCtrl->AddRoot(label, -1, -1, data);
            item.SetId(id);
        }

        // process child items recursively
        CBLAST_Dlg_Item::TNameToItemMap* items = item.GetChildItems();
        if(items)   {
            NON_CONST_ITERATE(CBLAST_Dlg_Item::TNameToItemMap, it, *items)   {
                CBLAST_Dlg_Item& ch_item = *it->second;
                if(ch_item.IsVisible()) {
                    x_BuildTreeItems(ch_item);
                }
            }
            if(parent)  {
                if(item.IsExpanded())    {
                    m_TreeCtrl->Expand(id);
                } else {
                    m_TreeCtrl->Collapse(id);
                }
            }
        }
    }
}


// completely rebuilds all wxTreeItems in the Tree Control
void CBLAST_DB_Dialog::x_CreateTreeItems()
{
    wxWindowUpdateLocker locker(m_TreeCtrl);

    if(m_Root)  {
        m_TreeCtrl->UnselectAll();
        m_TreeCtrl->DeleteAllItems();

        x_BuildTreeItems(*m_Root);
    }
}


void CBLAST_DB_Dialog::x_FilterItems(const string& query)
{
    if(m_Root)  {
        wxBusyCursor wait;
        wxWindowUpdateLocker locker(m_TreeCtrl);
        bool reset = query.empty();
        int n_vis = 0;

        // make all categories invisible by default
        NON_CONST_ITERATE(TItemDeque, it, m_CatItems)   {
            CBLAST_Dlg_Item& ch_item = **it;
            ch_item.SetVisible(false);
        }

        // iterate on all DB items and set Visible flag
        NON_CONST_ITERATE(TItemDeque, it, m_DbItems)   {
            CBLAST_Dlg_Item& ch_item = **it;
            bool vis = true;
            if( ! reset)   {
        //        const string& label = ch_item.GetLabel();
                string label = ToStdString(ch_item.GetLabel());

                vis = (NStr::FindNoCase(label, query) != string::npos);
            }
            ch_item.SetVisible(vis);

            if(vis) {
                n_vis++;

                // mark parent category items as visible
                CBLAST_Dlg_Item* parent = ch_item.GetParent();
                while(parent  &&  ! parent->IsVisible())    {
                    parent->SetVisible(true);
                    parent = parent->GetParent();
                }
            }
        }

        x_CreateTreeItems();

        x_UpdateFilterStatusText(reset, n_vis);
    }
}


void CBLAST_DB_Dialog::x_UpdateFilterStatusText(bool reset, int n)
{
    string status = reset ? "All " : "Filtered - ";
    if( ! reset  &&  n == 0)    {
        status += "no matches";
    } else {
        status += NStr::IntToString(n);
        status += " Database";
        status += (n != 1 ? "s are shown" : " is shown");
    }

    m_StatusText->SetLabel(ToWxString(status));
}


void CBLAST_DB_Dialog::OnTreectrlItemCollapsed( wxTreeEvent& event )
{
    x_UpdateCollapsedState(event.GetItem());
}


void CBLAST_DB_Dialog::OnTreectrlItemExpanded( wxTreeEvent& event )
{
    x_UpdateCollapsedState(event.GetItem());
}


void CBLAST_DB_Dialog::OnTreectrlSelChanged( wxTreeEvent& event )
{
    bool db_item = false;

    wxTreeItemId id = m_TreeCtrl->GetSelection();
    if(id.IsOk())   {
        wxTreeItemData* data = m_TreeCtrl->GetItemData(id);
        CBLASTDlgItemData* ex_data = dynamic_cast<CBLASTDlgItemData*>(data);
        db_item = ex_data->m_Item->IsDatabase();
    }

    m_OKBtn->Enable(db_item);
}


void CBLAST_DB_Dialog::OnTreectrlItemActivated( wxTreeEvent& event )
{
    x_OnDatabaseSelected();
}


void CBLAST_DB_Dialog::OnOkClick( wxCommandEvent& event )
{
    x_OnDatabaseSelected();
}


void CBLAST_DB_Dialog::x_OnDatabaseSelected()
{
    CBLAST_Dlg_Item* item = x_GetSelectedDBItem();
    if(item)    {
        m_SelDatabase = item->GetPath();
        EndModal(wxID_OK);
    }
}


CBLAST_Dlg_Item* CBLAST_DB_Dialog::x_GetSelectedDBItem()
{
    wxTreeItemId id = m_TreeCtrl->GetSelection();

    if(id.IsOk())   {
        wxTreeItemData* data = m_TreeCtrl->GetItemData(id);
        CBLASTDlgItemData* ex_data = dynamic_cast<CBLASTDlgItemData*>(data);
        CBLAST_Dlg_Item* item = ex_data->m_Item.GetPointer();
        if (item->IsDatabase())
            return item;
    }
    return NULL;
}


void CBLAST_DB_Dialog::x_UpdateCollapsedState(wxTreeItemId id)
{
    bool open = m_TreeCtrl->IsExpanded(id);

    wxTreeItemData* data = m_TreeCtrl->GetItemData(id);
    CBLASTDlgItemData* ex_data = dynamic_cast<CBLASTDlgItemData*>(data);
    ex_data->m_Item->Expand(open);
}


END_NCBI_SCOPE
