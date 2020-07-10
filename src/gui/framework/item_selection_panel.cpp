/*  $Id: item_selection_panel.cpp 42573 2019-03-21 17:53:20Z shkeda $
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

#include <gui/framework/item_selection_panel.hpp>

#include <gui/widgets/wx/splitter.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/registry.hpp>


#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/srchctrl.h>
#include <wx/button.h>
#include <wx/icon.h>

////@begin includes
////@end includes

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CItemSelectionPanel, wxPanel )

BEGIN_EVENT_TABLE( CItemSelectionPanel, wxPanel )
////@begin CItemSelectionPanel event table entries
    EVT_KEY_DOWN( CItemSelectionPanel::OnKeyDown )

    EVT_TEXT( ID_SEARCH, CItemSelectionPanel::OnSearchUpdated )

    EVT_BUTTON( ID_RESET, CItemSelectionPanel::OnResetClick )

////@end CItemSelectionPanel event table entries
    EVT_LISTBOX(wxID_ANY, CItemSelectionPanel::OnItemSelected)
END_EVENT_TABLE()


CItemSelectionPanel::CItemSelectionPanel()
{
    Init();
}


CItemSelectionPanel::CItemSelectionPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CItemSelectionPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CItemSelectionPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CItemSelectionPanel creation
    return true;
}


CItemSelectionPanel::~CItemSelectionPanel()
{
////@begin CItemSelectionPanel destruction
////@end CItemSelectionPanel destruction
}


void CItemSelectionPanel::Init()
{
////@begin CItemSelectionPanel member initialisation
    m_SearchCtrl = NULL;
    m_ResetBtn = NULL;
    m_StatusText = NULL;
    m_Splitter = NULL;
////@end CItemSelectionPanel member initialisation
}


void CItemSelectionPanel::CreateControls()
{
////@begin CItemSelectionPanel content construction
    CItemSelectionPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxLEFT|wxRIGHT, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SearchCtrl = new wxTextCtrl( itemPanel1, ID_SEARCH, wxT(""), wxDefaultPosition, wxSize(itemPanel1->ConvertDialogToPixels(wxSize(120, -1)).x, -1), 0 );
    itemBoxSizer3->Add(m_SearchCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ResetBtn = new wxButton( itemPanel1, ID_RESET, _("Reset"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_ResetBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_StatusText = new wxStaticText( itemPanel1, ID_STATUS, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_StatusText, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Splitter = new CSplitter( itemPanel1, ID_SPLITTER, wxDefaultPosition, wxSize(50, 360), 0 );
    itemBoxSizer2->Add(m_Splitter, 1, wxGROW|wxALL, 5);

////@end CItemSelectionPanel content construction
    int sizes[3] = { 300, 60, -1 };
    m_Splitter->Split(CSplitter::eVertical, sizes);
    m_Splitter->SetResizableCell(0, 0);

    m_MapWidget = new CGroupMapWidget( m_Splitter, ID_MAP_WIDGET, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

    m_DescrCtrl = new wxTextCtrl( m_Splitter, IDDESCR, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );

    m_Splitter->InsertToCell(m_MapWidget, 0, 0);
    m_Splitter->InsertToCell(m_DescrCtrl, 0, 1);

    CMapControl* map_control = m_MapWidget->GetMapControl();
    CMapControl::SProperties& props = map_control->GetProperties();
    props.m_ColumnWidth = 240;
    map_control->SetFocus();
}


void CItemSelectionPanel::SetItemTypeLabel(const string& label)
{
    m_ItemTypeLabel = label;
}

typedef CGroupMapWidget::SGroupDescr TGroupDescr;

void CItemSelectionPanel::SetItems(vector<TItemRef>& Items)
{
    m_AllItems = Items;
    m_Items = m_AllItems;
    x_FilterItems("");
}


// populates m_MapWidget with items for a set of objects in m_Items
void CItemSelectionPanel::x_UpdateMapWidget()
{
    CGroupMapWidget::TGroupDescrVector grp_descrs;

    for( size_t i = 0;  i < m_Items.size();  i++ ) {
        TItemRef Item = m_Items[i];
        x_AddItem(grp_descrs, *Item);
    }

    // add number of items to the group name
    string s;
    for( size_t i = 0;  i < grp_descrs.size();  i++ ) {
        CGroupMapWidget::SGroupDescr& descr = grp_descrs[i];

        vector<string>::const_iterator it =
            find(m_CollapsedGroups.begin(), m_CollapsedGroups.end(), descr.m_Label);
        if (it != m_CollapsedGroups.end())
            descr.m_Expanded = false;

        size_t n = descr.m_Items.size();
        if(n == 0)  {
            s = "  ( empty )";
        } else if(n == 1)  {
            s = "  ( 1 " + m_ItemTypeLabel + " )";
        } else {
            s = "  ( " + NStr::SizetToString(n) + " " + m_ItemTypeLabel + "s )";
        }
        descr.m_Label += s;
    }

    m_CollapsedGroups.clear();

    m_MapWidget->Init(grp_descrs);
}


// selected Items from m_AllItems that match the given query and
// place them in m_Items

void CItemSelectionPanel::x_FilterItems(const string& query)
{
    bool reset = query.empty();

    if(reset)   {
        m_Items = m_AllItems;
    } else {
        m_Items.clear();

        for( size_t i = 0;  i < m_AllItems.size();  i++ ) {
            TItemRef item = m_AllItems[i];
            const IUIObject& ui_obj = item->GetDescriptor();

            const string& label = ui_obj.GetLabel();
            if(NStr::FindNoCase(label, query) != string::npos)   {
                m_Items.push_back(item);
                continue;
            }

            const string& desc = ui_obj.GetDescription();
            if(NStr::FindNoCase(desc, query) != string::npos)   {
                m_Items.push_back(item);
                continue;
            }
        }
    }

    x_UpdateMapWidget();

    // Update Filter status text
    string status = reset ? "All " : "Filtered - ";
    size_t n = m_Items.size();

    if( ! reset  &&  n == 0)    {
        status += "no matches";
    } else {
        status += NStr::SizetToString(n);
        status += " " + m_ItemTypeLabel + (n != 1 ? "s are" : " is");
        status += " shown";
    }

    m_StatusText->SetLabel(ToWxString(status));
}


void CItemSelectionPanel::x_AddItem(CGroupMapWidget::TGroupDescrVector& descrs,
                                    IItemWithDescription& item)
{
    string group = item.GetCategory();

    CGroupMapWidget::SItemDescr descr;
    const IUIObject& ui_obj = item.GetDescriptor();
    descr.m_Label = ui_obj.GetLabel();
    descr.m_Description = ui_obj.GetDescription();
    descr.m_IconAlias = ui_obj.GetIconAlias();
    descr.m_ShowDefalt = item.ShowDefault();

    TGroupDescr * grp_descr = NULL;
    // find group
    for( size_t i = 0;  i < descrs.size();  i++ )    {
        if(descrs[i].m_Label == group)  {
            grp_descr = &descrs[i];
            break;
        }
    }

    if(grp_descr == NULL)   {   // create a new group
        descrs.push_back(TGroupDescr(group));
        grp_descr = &descrs.back();
    }

    grp_descr->m_Items.push_back(descr);
}


size_t CItemSelectionPanel::x_FindItemByLabel(const string& label)
{
    for( size_t i = 0;  i < m_Items.size();  i++ ) {
        TItemRef item = m_Items[i];
        const IUIObject& ui_obj = item->GetDescriptor();
        if(ui_obj.GetLabel() == label)  {
            return i;
        }
    }
    return (size_t) -1;
}



void CItemSelectionPanel::OnSearchUpdated( wxCommandEvent& event )
{
    wxString s_val = m_SearchCtrl->GetValue();

    string query = ToStdString(s_val);
    x_FilterItems(query);
}


void CItemSelectionPanel::OnResetClick( wxCommandEvent& event )
{
    m_SearchCtrl->SetValue(wxT(""));
}


CItemSelectionPanel::TItemRef CItemSelectionPanel::GetSelectedItemRef()
{
    TItemRef item;

    string label = m_MapWidget->GetSelectedItem();
    size_t index = x_FindItemByLabel(label);
    if(index != (size_t) -1) {
         item = m_Items[index];
    }
    return item;
}

void CItemSelectionPanel::OnItemSelected(wxCommandEvent& event)
{
    string descr;

    CConstRef<CGroupMapWidget::CGroupItem> curItem = m_MapWidget->GetCurrentItem();
    if (curItem) {
        descr = curItem->GetFullDescr();
    }

    m_DescrCtrl->SetValue(ToWxString(descr));
}


void CItemSelectionPanel::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}


static const char* kSplitterWidthsTag = "SplitterWidths";
static const char* kCollapsedGroupsTag = "CollapsedGroups";

void CItemSelectionPanel::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        // save splitter configuration
        vector<int> heights;
        m_Splitter->GetHeights(heights);
        view.Set(kSplitterWidthsTag, heights);

        vector<string> groups, collapsed;
        m_MapWidget->GetGroupLabels(groups);
        ITERATE(vector<string>, it, groups) {
            if (!m_MapWidget->IsExpanded(*it)) {
                string::size_type pos = it->find("  (");
                string str = (pos == string::npos) ? *it : it->substr(0, pos);
                collapsed.push_back(str);
            }
        }

        view.Set(kCollapsedGroupsTag, collapsed);
    }
}


void CItemSelectionPanel::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        // load splitter configuration
        vector<int> heights;
        view.GetIntVec(kSplitterWidthsTag, heights);
        if(heights.size() == 2)  {
            m_Splitter->SetHeights(heights);
        }

        view.GetStringVec(kCollapsedGroupsTag, m_CollapsedGroups);
    }
}


bool CItemSelectionPanel::ShowToolTips()
{
    return true;
}


wxBitmap CItemSelectionPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CItemSelectionPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CItemSelectionPanel bitmap retrieval
}


wxIcon CItemSelectionPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CItemSelectionPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CItemSelectionPanel icon retrieval
}

void CItemSelectionPanel::OnKeyDown(wxKeyEvent& event)
{
    int code = event.GetKeyCode();
    if(event.ControlDown() && (code == 'F'  ||  code == 'f'))   {
        m_SearchCtrl->SetFocus();
    } else {
        event.Skip();
    }

}

END_NCBI_SCOPE


