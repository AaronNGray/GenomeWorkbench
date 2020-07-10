/*  $Id: macro_edit_action_panel.cpp 44720 2020-02-28 16:37:19Z asztalos $
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
 * Authors:  Andrea Asztalos

 */


#include <ncbi_pch.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/macro_biodata.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <gui/widgets/edit/macro_editor.hpp>
#include <gui/widgets/edit/macro_action_tree.hpp>
#include <gui/widgets/edit/macro_constraint_panel.hpp>
#include <gui/widgets/edit/macro_edit_action_panel.hpp>

#include <wx/statbox.h>
#include <wx/wupdlock.h>
#include <wx/tipwin.h>
#include <wx/dcclient.h>
#include <wx/tglbtn.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CMacroEditingActionPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CMacroEditingActionPanel, wxPanel )


/*!
 * CMacroEditingActionPanel event table definition
 */

BEGIN_EVENT_TABLE( CMacroEditingActionPanel, wxPanel )

    EVT_TREE_SEL_CHANGED(ID_MACROEDIT_TREECTRL, CMacroEditingActionPanel::OnActionTreeItemChanged)
    EVT_SEARCHCTRL_SEARCH_BTN(ID_MACROEDIT_SEARCHCTRL, CMacroEditingActionPanel::OnSearchCtrlClick)

    EVT_CHECKLISTBOX(wxID_ANY, CMacroEditingActionPanel::OnControlInSubPanelChanged)
    EVT_LISTBOX(wxID_ANY, CMacroEditingActionPanel::OnControlClick)
    EVT_CHOICE(wxID_ANY, CMacroEditingActionPanel::OnControlClick)
    EVT_TEXT(wxID_ANY, CMacroEditingActionPanel::OnControlInSubPanelChanged)
    EVT_RADIOBOX(wxID_ANY, CMacroEditingActionPanel::OnControlClick)
    EVT_CHECKBOX(wxID_ANY, CMacroEditingActionPanel::OnControlInSubPanelChanged)
    EVT_RADIOBUTTON(wxID_ANY, CMacroEditingActionPanel::OnControlInSubPanelChanged)
    EVT_BUTTON(wxID_ANY, CMacroEditingActionPanel::OnControlInSubPanelChanged)
    EVT_TOGGLEBUTTON(wxID_ANY, CMacroEditingActionPanel::OnControlInSubPanelChanged)
    EVT_COMBOBOX(wxID_ANY, CMacroEditingActionPanel::OnControlInSubPanelChanged)
    EVT_HYPERLINK(wxID_ANY, CMacroEditingActionPanel::OnHyperlinkClicked)

END_EVENT_TABLE()


/*!
 * CMacroEditingActionPanel constructors
 */

CMacroEditingActionPanel::CMacroEditingActionPanel()
{
    Init();
}

CMacroEditingActionPanel::CMacroEditingActionPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CMacroEditingActionPanel creator
 */

bool CMacroEditingActionPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMacroEditingActionPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CMacroEditingActionPanel creation
    return true;
}


/*!
 * CMacroEditingActionPanel destructor
 */

CMacroEditingActionPanel::~CMacroEditingActionPanel()
{
////@begin CMacroEditingActionPanel destruction
////@end CMacroEditingActionPanel destruction
}


/*!
 * Member initialisation
 */

void CMacroEditingActionPanel::Init()
{
////@begin CMacroEditingActionPanel member initialisation
    m_SearchCtrl = NULL;
    m_ActionTree = NULL;
    m_ActionSizer = NULL;
    m_ConstraintSizer = NULL;
    m_ConstraintPanel = NULL;
    m_SelectedItem = nullptr;
    m_ParentFrame = nullptr;
////@end CMacroEditingActionPanel member initialisation
}


/*!
 * Control creation for CMacroEditingActionPanel
 */

void CMacroEditingActionPanel::CreateControls()
{    
////@begin CMacroEditingActionPanel content construction
    CMacroEditingActionPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 1, wxEXPAND|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer3->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer5, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Action search:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SearchCtrl = new wxSearchCtrl( itemPanel1, ID_MACROEDIT_SEARCHCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    if (CMacroEditingActionPanel::ShowToolTips())
        m_SearchCtrl->SetToolTip(_("Type action to search for"));
    itemBoxSizer5->Add(m_SearchCtrl, 1, wxGROW|wxALL, 5);
    m_SearchCtrl->Bind(wxEVT_TEXT_ENTER, &CMacroEditingActionPanel::OnSearchCtrlEnter, this);

    m_ActionTree = new CwxTreeCtrl(itemPanel1, ID_MACROEDIT_TREECTRL, wxDefaultPosition, wxSize(260, -1), wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT | wxTR_SINGLE);
    itemBoxSizer4->Add(m_ActionTree, 1, wxEXPAND|wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer3->Add(itemBoxSizer9, 0, wxGROW|wxALL, 0);

    wxStaticBox* itemStaticBoxSizer10Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Action"));
    m_ActionSizer = new wxStaticBoxSizer(itemStaticBoxSizer10Static, wxVERTICAL);
    itemBoxSizer9->Add(m_ActionSizer, 1, wxEXPAND | wxALL, 0);
    m_ActionSizer->SetMinSize(665, 450);

    wxStaticBox* itemStaticBoxSizer13Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Constraints"));
    m_ConstraintSizer = new wxStaticBoxSizer(itemStaticBoxSizer13Static, wxHORIZONTAL);
    itemBoxSizer9->Add(m_ConstraintSizer, 0, wxEXPAND| wxALL, 0);

    CSeq_entry_Handle seh;
    m_ConstraintPanel = new CMacroCompoundConstraintPanel(itemPanel1, seh, ID_MACROEDIT_CONSTRAINTPNL, wxDefaultPosition, wxSize(-1, 100), wxBORDER_NONE);
    m_ConstraintSizer->Add(m_ConstraintPanel, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    x_PopulateTreeCtrl();
////@end CMacroEditingActionPanel content construction
}


void CMacroEditingActionPanel::x_PopulateTreeCtrl()
{
    CConstRef<CMActionTree> action_tree = CMActionTreeBuilder::BuildActionTree();
    const CMActionNode& root = *action_tree->GetTopNode();

    wxTreeItemId root_id = m_ActionTree->AddRoot(root.GetLabel());
    CMTreeItemDataBuilder builder;
    x_Build(root, root_id, builder);

    wxTreeItemIdValue cookie;
    m_TopId = m_ActionTree->GetFirstChild(root_id, cookie);

    _ASSERT(action_tree->ReferencedOnlyOnce());

    m_ActionTree->CollapseAll();
}

void CMacroEditingActionPanel::x_Build(const CMActionNode& root, wxTreeItemId root_id, const CMTreeItemDataBuilder& builder)
{
    for (auto& child : root.GetChildren()) {
        wxTreeItemId child_id = m_ActionTree->AppendItem(root_id, child->GetLabel(), -1, -1, builder.GetTreeItemData(*child));
        x_Build(*child, child_id, builder);
    }
}

void CMacroEditingActionPanel::OnActionTreeItemChanged(wxTreeEvent& event)
{
    wxTreeItemId item_id = event.GetItem();
    if (!item_id.IsOk()) {
        return;
    }

    wxTreeItemId parent_id = m_ActionTree->GetItemParent(item_id);
    if (!parent_id.IsOk()) {
        return;
    }

    wxTreeItemData* data = m_ActionTree->GetItemData(item_id);
    if (IMacroActionItemData* edit_item = dynamic_cast<IMacroActionItemData*>(data)) {
#if defined __WINDOWS__
        wxWindowUpdateLocker locker(this);
#endif
        if (m_SelectedItem) {
            if (!m_ActionSizer->GetChildren().empty()) {
                m_SelectedItem->DeleteParamPanel();
            }
        }
        m_ConstraintPanel->ClearValues(true);

        m_SelectedItem = edit_item;
        _ASSERT(m_ParentFrame);
        
        m_SelectedItem->CreateParamPanel(*m_ActionSizer, this);
        m_ParentFrame->Update();
    }
    else if (dynamic_cast<CMIteratorItemData*>(data)) {
        m_ConstraintPanel->ClearValues(false);
        m_ParentFrame->Clear();
    }
}

void CMacroEditingActionPanel::OnControlInSubPanelChanged(wxCommandEvent& event)
{
    if (event.GetId() == ID_MACROEDIT_SEARCHCTRL) {
        return;
    }
    UpdateParentFrame();
}

void CMacroEditingActionPanel::OnControlClick(wxCommandEvent& event)
{
    if (m_SelectedItem->TransferDataFromPanel() && m_SelectedItem->UpdateTarget()) {
        m_ConstraintPanel->ClearValues(true);
    }
    UpdateParentFrame();
}

void CMacroEditingActionPanel::OnHyperlinkClicked(wxHyperlinkEvent& event)
{
    UpdateParentFrame();
}

void CMacroEditingActionPanel::UpdateParentFrame()
{
    // when updating the target, the constraints should also be updated!!!
    if (m_SelectedItem) {
        m_SelectedItem->UpdateTarget(m_ConstraintPanel->GetConstraints());
        if (m_ParentFrame) {
            m_ParentFrame->Update();
        }
    }
}

pair<string, string> CMacroEditingActionPanel::GetForTarget()
{
    if (!m_SelectedItem)
        return { kEmptyStr, kEmptyStr };

    return m_SelectedItem->GetTarget();
}

string CMacroEditingActionPanel::GetMacro()
{
    if (!m_SelectedItem) return kEmptyStr;

    m_SelectedItem->ResetWarning();
    const string& iter_label = m_SelectedItem->GetTarget().first;

    if (iter_label.empty())
        return kEmptyStr;

    m_SelectedItem->TransferDataFromPanel();
    string description = m_SelectedItem->GetMacroDescription();

    string macro_script = "MACRO Macro_name \"" + description;
    vector<string> constraints_description = m_ConstraintPanel->GetDescription();
    if (!constraints_description.empty())
        macro_script += " where " + NStr::Join(constraints_description, " and ");
    macro_script +=  "\"\n";

    if (m_SelectedItem->HasVariables() || m_ConstraintPanel->HasVariables()) {
        macro_script += "VAR\n";
    }
    if (m_SelectedItem->HasVariables()) {
        macro_script += m_SelectedItem->GetVariables() + "\n";
    }
    if (m_ConstraintPanel->HasVariables()) {
        macro_script += m_ConstraintPanel->GetVariables();
    }
    
    macro_script += "FOR EACH " + iter_label + "\n";
    vector<pair<string,string> > constraints = m_ConstraintPanel->GetConstraints();
    string do_function = m_SelectedItem->GetFunction(constraints); // create do_function based on constraint, remove used constraint from the vector;

    if (!constraints.empty()) {
        vector<string> pure_constraints;
        pure_constraints.reserve(constraints.size());
        for (auto p : constraints) {
            pure_constraints.push_back(p.second);
        }
        
        macro_script += "WHERE " + NStr::Join(pure_constraints, "\n      AND ") + "\n";
    }

    macro_script += "DO\n";
    macro_script += do_function + "\n";
    macro_script += "DONE\n";
    macro_script += "---------------------------------------------------\n";
    
    return macro_script;
}

const string& CMacroEditingActionPanel::GetWarning() const
{
    return m_SelectedItem->GetWarning();
}

void CMacroEditingActionPanel::OnSearchCtrlClick(wxCommandEvent& event)
{
    x_FindText(ToStdString(event.GetString()));
}

void CMacroEditingActionPanel::OnSearchCtrlEnter(wxCommandEvent& event)
{
    x_FindText(ToStdString(m_SearchCtrl->GetValue()));
    event.Skip();
}

void CMacroEditingActionPanel::x_FindText(const string& search)
{
    if (search.empty()) {
        NcbiMessageBox("Search string is empty");
        return;
    }

    if (NStr::EqualNocase(search, m_RecentSearch)) {
        // this has been searched for previously
        ++m_RecentIt;
        if (m_RecentIt == m_RecentItemIds.end()) {
            m_RecentIt = m_RecentItemIds.begin();
        }
        x_FocusFoundItem();
    }
    else {
        m_RecentSearch = search;
        m_RecentItemIds.resize(0);

        wxTreeItemId root_item = m_ActionTree->GetRootItem();
        x_FindItem(search, root_item);

        if (m_RecentItemIds.empty()) {
            NcbiInfoBox("String was not found");
            m_RecentSearch.resize(0);
            m_ActionTree->UnselectAll();
        }
        else {
            m_RecentIt = m_RecentItemIds.begin();
            x_FocusFoundItem();
        }
    }
}

void CMacroEditingActionPanel::x_FindItem(const string& search, const wxTreeItemId& root)
{
    wxTreeItemIdValue cookie;
    wxTreeItemId child, item_id = m_ActionTree->GetFirstChild(root, cookie);

    while (item_id.IsOk()) {
        const string item_text = ToStdString(m_ActionTree->GetItemText(item_id));
        if (NStr::FindNoCase(item_text, search) != NPOS) {
            m_RecentItemIds.push_back(item_id);
        }
        else if (wxTreeItemData* data = m_ActionTree->GetItemData(item_id)) {
            IMacroActionItemData* edit_data = dynamic_cast<IMacroActionItemData*>(data);
            if (edit_data) {
                const string& description = edit_data->GetGeneralDescription();
                if (NStr::FindNoCase(description, search) != NPOS) {
                    m_RecentItemIds.push_back(item_id);
                }
                else {
                    const vector<string>& keywords = edit_data->GetKeywords();
                    if (!keywords.empty() && 
                        find_if(keywords.begin(), keywords.end(), [&search](const string& it) { return NStr::FindNoCase(it, search) != NPOS; }) != keywords.end()) {
                        m_RecentItemIds.push_back(item_id);
                    }
                }
            }
        }

        if (m_ActionTree->ItemHasChildren(item_id)) {
            x_FindItem(search, item_id);
        }
        item_id = m_ActionTree->GetNextChild(root, cookie);
    }
}

void CMacroEditingActionPanel::x_FocusFoundItem()
{
    if (m_RecentIt != m_RecentItemIds.end() && m_RecentIt->IsOk()) {
        m_ActionTree->UnselectAll();
        m_ActionTree->SelectItem(*m_RecentIt, true);
        m_ActionTree->EnsureVisible(*m_RecentIt);
    }
}

/*!
 * Should we show tooltips?
 */

bool CMacroEditingActionPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CMacroEditingActionPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMacroEditingActionPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMacroEditingActionPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CMacroEditingActionPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMacroEditingActionPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMacroEditingActionPanel icon retrieval
}

IMPLEMENT_DYNAMIC_CLASS(CwxTreeCtrl, wxTreeCtrl)

BEGIN_EVENT_TABLE(CwxTreeCtrl, wxTreeCtrl)
    EVT_MOTION(CwxTreeCtrl::OnMouseMove)
    EVT_TIMER(-1, CwxTreeCtrl::OnTimer)
    EVT_LEFT_DOWN(CwxTreeCtrl::OnLeftDown)

    EVT_CONTEXT_MENU(CwxTreeCtrl::OnContextMenu)
    EVT_MENU(ID_MACROEDIT_POPUPEXPAND, CwxTreeCtrl::OnExpandAllMenuItem)
    EVT_MENU(ID_MACROEDIT_POPUPCOLLAPSE, CwxTreeCtrl::OnCollapseAllMenuItem)
END_EVENT_TABLE()

CwxTreeCtrl::~CwxTreeCtrl()
{
    m_Timer.Stop();
    x_ShowTooltip(false);
}

void CwxTreeCtrl::OnLeftDown(wxMouseEvent& event)
{
    event.Skip();
    if (m_SelItem.IsOk()) {
        SelectItem(m_SelItem);
    }
}

void CwxTreeCtrl::OnMouseMove(wxMouseEvent& event) 
{ 
    m_Timer.Stop();
    int flags = 0;
    m_SelItem = HitTest(event.GetPosition(), flags);
    if (flags & (wxTREE_HITTEST_ONITEMLABEL | wxTREE_HITTEST_ONITEMSTATEICON | wxTREE_HITTEST_ONITEMINDENT)) {
        m_Timer.StartOnce(500);
    }
    
    event.Skip();
}

void CwxTreeCtrl::OnTimer(wxTimerEvent& event)
{
    if (m_Timer.IsRunning())
        return;

    x_ShowTooltip(true);
}

void CwxTreeCtrl::x_ShowTooltip(bool show)
{
    static wxTipWindow* s_tipWindow = nullptr;

    if (s_tipWindow)
    {
        s_tipWindow->SetTipWindowPtr(nullptr);
        s_tipWindow->Close();
        s_tipWindow = nullptr;
    }

    if (!show)
        return;
    
    if (m_SelItem.IsOk()) {
        wxTreeItemData* data = GetItemData(m_SelItem);
        IMacroActionItemData* edit_item = dynamic_cast<IMacroActionItemData*>(data);
        if (edit_item) {
            m_ToolTipText = ToWxString(edit_item->GetGeneralDescription());
        }
        else {
            return;
        }
    }

    if (m_ToolTipText.IsEmpty())
        return;

    wxClientDC dc(this);
    int w = dc.GetTextExtent(m_ToolTipText).GetWidth();

    wxRect r;
    GetBoundingRect(m_SelItem, r);
    // Convert from window to screen coordinates
    r.SetPosition(ClientToScreen(r.GetPosition()));

    s_tipWindow = new wxTipWindow(this, m_ToolTipText, w, &s_tipWindow, &r);
}

void CwxTreeCtrl::OnContextMenu(wxContextMenuEvent& event)
{
    m_Timer.Stop();
    wxMenu item;
    item.Append(ID_MACROEDIT_POPUPEXPAND, wxT("Expand all"));
    item.Append(ID_MACROEDIT_POPUPCOLLAPSE, wxT("Collapse all"));
    PopupMenu(&item);
}

void CwxTreeCtrl::OnExpandAllMenuItem(wxCommandEvent& event)
{
    ExpandAll();
    UnselectAll();
}

void CwxTreeCtrl::OnCollapseAllMenuItem(wxCommandEvent& event)
{
    CollapseAll();
    UnselectAll();
}

END_NCBI_SCOPE
