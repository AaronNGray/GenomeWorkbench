/*  $Id: project_tree_panel.cpp 44121 2019-10-31 15:23:32Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/core/project_tree_panel.hpp>

#include <gui/core/commands.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/project_task.hpp>
#include <gui/core/fname_validator_imp.hpp>
#include <gui/core/app_dialogs.hpp>
#include <gui/core/pt_icon_list.hpp>
#include <gui/core/pt_project.hpp>
#include <gui/core/pt_workspace.hpp>
#include <gui/core/pt_project_item.hpp>
#include <gui/core/pt_view.hpp>
#include <gui/core/pt_data_source.hpp>
#include <gui/core/pt_root.hpp>
#include <gui/core/pt_folder.hpp>
#include <gui/core/pt_utils.hpp>
#include <gui/core/app_explorer_data_object.hpp>

#include <objects/gbproj/ProjectDescr.hpp>
#include <objects/gbproj/FolderInfo.hpp>
#include <objects/gbproj/GBProject_ver2.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/framework/view_manager_service.hpp>
#include <gui/framework/window_manager_service.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/framework/menu_service.hpp>

#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/utils/extension_impl.hpp>


#include <wx/artprov.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/menu.h>
#include <wx/wupdlock.h>
#include <wx/frame.h>
#include <wx/msgdlg.h>
#include <wx/clipbrd.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CProjectTreeViewDropTarget
CProjectTreeViewDropTarget::CProjectTreeViewDropTarget(CProjectTreePanel* panel)
:   m_Panel(panel),
    m_AcceptableData(false)
{
    wxDataObjectComposite* composite = new wxDataObjectComposite();
    composite->Add(new wxFileDataObject);
    composite->Add(new CAppExplorerDataObject, true);
    SetDataObject(composite);
}


wxDragResult CProjectTreeViewDropTarget::OnEnter(wxCoord x, wxCoord y, wxDragResult def)
{
    wxDataObjectComposite* composite = dynamic_cast<wxDataObjectComposite*>(m_dataObject);
    wxDataFormat format = composite->GetReceivedFormat();
    if (format == wxDF_FILENAME)
        return def;

    m_AcceptableData = true;
    return OnDragOver(x, y, def);
}


wxDragResult CProjectTreeViewDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
    wxDataObjectComposite* composite = dynamic_cast<wxDataObjectComposite*>(m_dataObject);
    wxDataFormat format = composite->GetReceivedFormat();
    if (format == wxDF_FILENAME)
        return def;

    if(m_AcceptableData)    {
        return m_Panel->OnDragOver(x, y, def);
    }
    return wxDragNone;
}


wxDragResult CProjectTreeViewDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
    GetData();
    wxDataObjectComposite* composite = dynamic_cast<wxDataObjectComposite*>(m_dataObject);
    wxDataFormat format = composite->GetReceivedFormat();
    if (format == wxDF_FILENAME) {
        wxFileDataObject *fileDataObj = dynamic_cast<wxFileDataObject*>
            (composite->GetObject(wxDF_FILENAME));

        if(fileDataObj &&  (def == wxDragMove  ||  def == wxDragCopy || def == wxDragLink)) {
            wxArrayString filenames = fileDataObj->GetFilenames();

            vector<wxString> names;
            size_t n = filenames.GetCount();
            names.reserve(n);
            for( size_t i = 0; i < n; i++ ){
                names.push_back (filenames[i]);
            }

            CAppDialogs::COpenDialog(m_Panel->m_Workbench, NcbiEmptyString, names);
        }

        return def;
    }
    CAppExplorerDataObject* data_obj = dynamic_cast<CAppExplorerDataObject*>
        (composite->GetObject(format));
    if (data_obj) {
        m_Panel->OnDrop(x, y, def, *data_obj);
        return def;
    }

    return def;
}

BEGIN_EVENT_TABLE(CProjectTreeCtrl, wxTreeCtrl)
    EVT_LEFT_UP( CProjectTreeCtrl::OnLeftDown)
    EVT_KEY_UP(CProjectTreeCtrl::OnKeyUp)
    EVT_MOTION( CProjectTreeCtrl::OnMove )
END_EVENT_TABLE()
///////////////////////////////////////////////////////////////////////////////
/// CProjectTreeCtrl
wxTextCtrl *CProjectTreeCtrl::EditLabel(const wxTreeItemId& item, wxClassInfo* textCtrlClass)
{
    wxTextCtrl* t = wxTreeCtrl::EditLabel(item, textCtrlClass);
    
    // In windows, when an item in the tree is selected for editing, all characters
    // are selected by default. This is not true on the Mac, but the EditLabel function
    // does return the edit control so we can use that to select the text.  The 
    // documentation however says that EditLabel returns NULL.  Restrict it to
    // to MAC only since that is where it is needed and it is not validated on other platforms.
#ifdef NCBI_OS_DARWIN
    if (t!=NULL)
        t->SelectAll();
#endif

    return t;
}

void CProjectTreeCtrl::OnLeftDown(wxMouseEvent& event)
{
    CProjectTreePanel* tree_panel = dynamic_cast<CProjectTreePanel*>(GetParent());
    if( tree_panel ){
        wxTreeEvent tree_event;
        tree_panel->OnSelectionChanged( tree_event );
    }

    event.Skip();
}

void CProjectTreeCtrl::OnKeyUp(wxKeyEvent& event)
{
    // Keep track of cmd(osx)/ctrl(other os) key status since it changes whether
    // drag and drop is a copy or a move (cmd/ctrl down=> copy)
    CProjectTreePanel* tree_panel = dynamic_cast<CProjectTreePanel*>(GetParent());
    bool b = event.CmdDown(); 
    tree_panel->SetCopyMode(b);
    event.Skip();
}

void CProjectTreeCtrl::OnMove(wxMouseEvent& event)
{
#ifdef __WXOSX_COCOA__
    // Tree's built-in drag and drop does not have a 'isdragging' event so
    // we call the 'OnDragOver' on all mouse moves and it checks if a 
    // drag is actually in progress.
    CProjectTreePanel* tree_panel = dynamic_cast<CProjectTreePanel*>(GetParent());
    if( tree_panel ){       
        // The result is ignored (it is used by the other drag method - this method is only
        // for the osx/cocoa case.)
        wxDragResult ignore_me = wxDragMove;
        wxCoord x,y;
        event.GetPosition(&x,&y);
        tree_panel->OnDragOver(x, y, ignore_me);
    }
#endif

    event.Skip();
}

///////////////////////////////////////////////////////////////////////////////
/// CProjectTreePanel

BEGIN_EVENT_TABLE(CProjectTreePanel, wxPanel)
    EVT_CONTEXT_MENU(CProjectTreePanel::OnContextMenu)

    EVT_TREE_ITEM_COLLAPSED(wxID_ANY, CProjectTreePanel::OnItemExpandedCollapsed)
    EVT_TREE_ITEM_EXPANDED(wxID_ANY, CProjectTreePanel::OnItemExpandedCollapsed)
    EVT_TREE_ITEM_ACTIVATED(wxID_ANY, CProjectTreePanel::OnItemActivated)
    EVT_TREE_KEY_DOWN(wxID_ANY, CProjectTreePanel::OnTreeKeyDown)
    EVT_TREE_BEGIN_LABEL_EDIT(wxID_ANY, CProjectTreePanel::OnBeginLabelEdit)
    EVT_TREE_END_LABEL_EDIT(wxID_ANY, CProjectTreePanel::OnEndLabelEdit)

    EVT_TREE_SEL_CHANGED(wxID_ANY, CProjectTreePanel::OnSelectionChanged)

    EVT_TREE_BEGIN_DRAG(wxID_ANY, CProjectTreePanel::OnBeginDrag)
    EVT_TREE_END_DRAG(wxID_ANY, CProjectTreePanel::OnEndDrag)

    EVT_LEFT_UP( CProjectTreePanel::OnLeftDown)

    EVT_MENU(wxID_CUT, CProjectTreePanel::OnCut)
    EVT_UPDATE_UI(wxID_CUT, CProjectTreePanel::OnUpdateCut)
    EVT_MENU(wxID_COPY, CProjectTreePanel::OnCopy)
    EVT_UPDATE_UI(wxID_COPY, CProjectTreePanel::OnUpdateCopy)
    EVT_MENU(wxID_PASTE, CProjectTreePanel::OnPaste)
    EVT_UPDATE_UI(wxID_PASTE, CProjectTreePanel::OnUpdatePaste)

    EVT_MENU(wxID_DELETE, CProjectTreePanel::OnRemove)
    EVT_UPDATE_UI(wxID_DELETE, CProjectTreePanel::OnUpdateRemove)

    EVT_MENU(eCmdShowHideDisabledItems, CProjectTreePanel::OnShowHideDisabledItems)
    EVT_UPDATE_UI(eCmdShowHideDisabledItems, CProjectTreePanel::OnUpdateShowHideDisabledItems)

    EVT_MENU(eCmdEnableDisable, CProjectTreePanel::OnEnableDisable)
    EVT_UPDATE_UI(eCmdEnableDisable, CProjectTreePanel::OnUpdateEnableDisable)
    EVT_MENU(eCmdNewFolder, CProjectTreePanel::OnNewFolder)
    EVT_UPDATE_UI(eCmdNewFolder, CProjectTreePanel::OnUpdateNewFolder)

    EVT_MENU( eCmdCloseProjectView, CProjectTreePanel::OnCloseView )
    EVT_UPDATE_UI( eCmdCloseProjectView, CProjectTreePanel::OnUpdateCloseView )
    EVT_MENU(eCmdActivateClients, CProjectTreePanel::OnActivateClients)
    EVT_UPDATE_UI(eCmdActivateClients, CProjectTreePanel::OnUpdateActivateClients)
    EVT_MENU(wxID_PROPERTIES, CProjectTreePanel::OnProperties)
    EVT_UPDATE_UI(wxID_PROPERTIES, CProjectTreePanel::OnUpdateProperties)

    EVT_MENU(eCmdUnLoadProject, CProjectTreePanel::OnUnLoadProject)
    EVT_UPDATE_UI(eCmdUnLoadProject, CProjectTreePanel::OnUpdateUnLoadProject)

    EVT_MENU(eCmdLoadProject, CProjectTreePanel::OnLoadProject)
    EVT_UPDATE_UI(eCmdLoadProject, CProjectTreePanel::OnUpdateLoadProject)

    EVT_MENU(eCmdRemoveProject, CProjectTreePanel::OnRemoveProject)
    EVT_UPDATE_UI(eCmdRemoveProject, CProjectTreePanel::OnUpdateRemoveProject)

    EVT_MENU(eCmdProjectTableView, CProjectTreePanel::OnProjectTableView)
    EVT_UPDATE_UI(eCmdProjectTableView, CProjectTreePanel::OnUpdateProjectTableView)
END_EVENT_TABLE()


CProjectTreePanel::CProjectTreePanel()
:   m_DataObject(NULL),
    m_Workbench(NULL),
    m_Tree(NULL),
    m_CopyMode(false)
{
    Init();
}


CProjectTreePanel::~CProjectTreePanel()
{
    PT::CItem::LogInstanceCount();
    delete m_DataObject;
    m_DataObject = NULL;
}


void CProjectTreePanel::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    wxPanel::Create(parent, id, pos, size, wxBORDER_NONE);
    CreateControls();
}


void CProjectTreePanel::Init()
{
}


void CProjectTreePanel::CreateControls()
{
    wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);

    long style = wxTR_HAS_BUTTONS | wxTR_MULTIPLE | wxTR_EDIT_LABELS 
        | wxTR_HIDE_ROOT | wxBORDER_NONE | wxTR_GBENCH_LINES
    ;

    // create Tree Control
    m_Tree = new CProjectTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);
    sizer->Add(m_Tree, 1, wxEXPAND);

    m_Tree->SetDropTarget(new CProjectTreeViewDropTarget(this));
    m_Tree->SetImageList(&PT::CPTIcons::GetInstance().GetImageList());
}


void CProjectTreePanel::SetWorkbench(IWorkbench* workbench)
{
    m_Workbench = workbench;
    if (m_Workbench) {
        ReloadProjectTree();
    } else {
        wxTreeItemId rootId = m_Tree->GetRootItem();
        if (rootId.IsOk()) {
            m_Tree->DeleteChildren(rootId);
            PT::CItem::LogInstanceCount();
        }
    }
}

WX_DEFINE_MENU(kContextMenu)
    WX_MENU_SEPARATOR_L("Top Actions")
    WX_MENU_SEPARATOR_L("Actions")
    WX_MENU_SEPARATOR_L("Edit")
    WX_MENU_SEPARATOR_L("Properties")
WX_END_MENU()


void CProjectTreePanel::OnContextMenu(wxContextMenuEvent& event)
{
    SetFocus();

    CUICommandRegistry& cmd_reg = m_Workbench->GetUICommandRegistry();

    PT::TItems sel_items;
    GetSelectedItems(sel_items);

    // create menu backbone
    unique_ptr<wxMenu> menu(cmd_reg.CreateMenu(kContextMenu));

        IExplorerItemCmdContributor::TContribution
                contrib = x_GetContextMenu(sel_items);
    wxMenu* obj_menu = contrib.first;
    if (obj_menu) {
        Merge(*menu, *obj_menu);
        delete obj_menu;
        /// register provided handler
        wxEvtHandler* handler = contrib.second;
        if (handler) {
            m_ContributedHandlers.push_back(handler);
            PushEventHandler(handler);
        }
    }

    // get contributed menus
    vector< CIRef<IExplorerItemCmdContributor> > contributors;
    static string id("project_tree_view::context_menu::item_cmd_contributor");
    GetExtensionAsInterface(id, contributors);

    // Merge contributed menus into the main Menu
    for( size_t i = 0; i < contributors.size(); i++ ) {
        IExplorerItemCmdContributor& obj = *contributors[i];
        IExplorerItemCmdContributor::TContribution
            contrib = obj.GetMenu(*m_Tree, sel_items);
        /// update menu
        wxMenu* obj_menu = contrib.first;
        if(obj_menu)    {
            Merge(*menu, *obj_menu);
            delete obj_menu;

            /// register provided handler
            wxEvtHandler* handler = contrib.second;
            if(handler)  {
                m_ContributedHandlers.push_back(handler);
                PushEventHandler(handler);
            }
        } else {
            _ASSERT(contrib.second == NULL);
            delete contrib.second;
        }
    }

    CleanupSeparators(*menu);   // Remove empty groups

    PopupMenu(menu.get());


    /// disconnect and destroy contributed handlers
    for(  size_t i = 0;  i < m_ContributedHandlers.size();  i++ )   {
        wxEvtHandler* handler = PopEventHandler();
        _ASSERT(handler == m_ContributedHandlers[i]);
        delete handler;
    }
    m_ContributedHandlers.clear();
}

void CProjectTreePanel::x_RefreshProjectTables (int projectId)
{
    if (!m_WS || projectId == -1) return;

    CGBDocument* doc = dynamic_cast<CGBDocument*>(m_WS->GetProjectFromId(projectId));
    if (!doc || !doc->IsLoaded()) return;
    const CGBProject_ver2* gbproj = dynamic_cast<const CGBProject_ver2*>(&doc->GetProject());

    vector< CIRef<IProjectView> > views = doc->GetViews();

    NON_CONST_ITERATE(vector< CIRef<IProjectView> >, it_view, views) {
        IProjectView& view = **it_view;
        TConstScopedObjects objs;
        view.GetMainObject(objs);
        if (objs.size() == 1 && objs[0].object == gbproj) {
            CEventHandler* handler = dynamic_cast<CEventHandler*>(&view);
            if (handler) {
                CRef<CEvent> ev(new CProjectViewEvent(projectId, CProjectViewEvent::eData));
                handler->Send(ev);
            }
        }
    }
}

static void s_RemoveDependentItems(wxTreeCtrl& treeCtrl, wxArrayTreeItemIds& items)
{
    // build an index for fast look-up
    set<wxTreeItemId> items_set;
    ITERATE (wxArrayTreeItemIds, it, items)
        items_set.insert(*it);

    wxArrayTreeItemIds topItems;
    ITERATE (wxArrayTreeItemIds, it, items) {
        wxTreeItemId parent = treeCtrl.GetItemParent(*it);
        while (parent.IsOk()) {
            if (items_set.find(parent) != items_set.end())
                break;
            parent = treeCtrl.GetItemParent(parent);
        }

        if (!parent.IsOk())
            topItems.push_back(*it);
    }

    items = topItems;
}

CAppExplorerDataObject* CProjectTreePanel::x_CreateDataObject(bool cut)
{
    wxArrayTreeItemIds sel_ids;
    m_Tree->GetSelections(sel_ids);
    if (sel_ids.empty()) return 0;

    ITERATE (wxArrayTreeItemIds, it, sel_ids) {
        PT::CItem* item = x_GetExplorerItem(*it);
        if (!item->CanCopyToClipboard(*m_Tree))
            return 0;
    }
    s_RemoveDependentItems(*m_Tree, sel_ids);

    return new CAppExplorerDataObject (m_Tree, sel_ids, cut);
}

void CProjectTreePanel::x_CutOrCopyToClipboard(bool cut)
{
    CAppExplorerDataObject* data_object = x_CreateDataObject(cut);
    if (!data_object) return;

    wxClipboardLocker clipLocker;
    if (!clipLocker) return;
    wxTheClipboard->SetData(data_object);
}

void CProjectTreePanel::x_ClearClipboardData()
{
    wxClipboardLocker clipLocker;
    if (!clipLocker) return;

    if (wxTheClipboard->IsSupported(CAppExplorerDataObject::m_ItemsFormat))
        wxTheClipboard->Clear();
}

void CProjectTreePanel::ReloadProjectTree()
{
    //CStopWatch sw; sw.Start();
    PT::CItem::LogInstanceCount();

    x_ClearClipboardData();

    CProjectService* prj_srv = m_Workbench->GetServiceByType<CProjectService>();
    m_WS = prj_srv->GetGBWorkspace();

    wxTreeItemId rootId = m_Tree->GetRootItem();
    if (rootId.IsOk()) {
        m_Tree->DeleteChildren(rootId);
    } else {
        PT::CRoot* item = new PT::CRoot(0);
        rootId = m_Tree->AddRoot(wxT(""), -1, -1, item);
        item->SetTreeItemId(rootId);
    }

    PT::CItem::LogInstanceCount();

    PT::CRoot* rootItem = x_GetRootItem();
    rootItem->Initialize(*m_Tree, m_WS);
    rootItem->UpdateDataSources(*m_Tree, *m_Workbench);
    rootItem->UpdateAllViews(*m_Tree, m_WS);
    //LOG_POST(Info << "CProjectTreePanel::ReloadProjectTree()" << int(1000 * sw.Elapsed()) << " ms");
}

PT::CRoot* CProjectTreePanel::x_GetRootItem()
{
    wxTreeItemId rootId = m_Tree->GetRootItem();
    if (!rootId.IsOk()) return 0;
    return dynamic_cast<PT::CRoot*>(m_Tree->GetItemData(rootId));
}

PT::CWorkspace* CProjectTreePanel::x_GetWorkspaceItem()
{
    PT::CRoot* rootItem = x_GetRootItem();
    return rootItem ? rootItem->GetWorkspaceItem(*m_Tree) : 0;
}

PT::CItem* CProjectTreePanel::x_GetExplorerItem(const wxTreeItemId& id)
{
    wxTreeItemData* data = m_Tree->GetItemData(id);
    PT::CItem* item = dynamic_cast<PT::CItem*>(data);
    _ASSERT(item);
    return item;
}


// returns selection as a vector of pointers to CItem
void CProjectTreePanel::GetSelectedItems(PT::TItems& items)
{
    wxArrayTreeItemIds sel_ids;
    m_Tree->GetSelections(sel_ids);

    x_GetItemsFromIds(sel_ids, items);
}

void CProjectTreePanel::x_CollectItemIds( wxTreeItemId root, wxArrayTreeItemIds &ids )
{
    wxTreeItemIdValue cookie;
    wxTreeItemId search;
    wxTreeItemId item = m_Tree->GetFirstChild( root, cookie );
    wxTreeItemId child;

    while( item.IsOk() )
    {
        ids.Add(item);
        if( m_Tree->ItemHasChildren( item ) )
        {
            x_CollectItemIds( item, ids );
        }
        item = m_Tree->GetNextChild( root, cookie);
    }
}

// returns selection as a vector of pointers to CItem
void CProjectTreePanel::GetAllItems(PT::TItems& items)
{
    wxArrayTreeItemIds ids;
    x_CollectItemIds(m_Tree->GetRootItem(), ids);
    x_GetItemsFromIds(ids, items);
}


void CProjectTreePanel::x_GetItemsFromIds(const wxArrayTreeItemIds& ids,
                                          PT::TItems& items)
{
    for(  size_t i = 0;  i < ids.GetCount();  i++ ) {
        wxTreeItemId id = ids[i];
        PT::CItem* item = x_GetExplorerItem(id);
        items.push_back(item);
    }
}


PT::CItem* CProjectTreePanel::x_GetSingleSelectedItem()
{
    wxArrayTreeItemIds sel_ids;
    m_Tree->GetSelections(sel_ids);

    if(sel_ids.size() == 1) {
        PT::CItem* item = x_GetExplorerItem(sel_ids[0]);
        return item;
    }
    return NULL;
}


void CProjectTreePanel::GetSelectedProjectIds(TProjectIdVector& ids)
{
    PT::TItems sel_items;
    GetSelectedItems(sel_items);

    NON_CONST_ITERATE(PT::TItems, it, sel_items)  {
        PT::CItem& item = **it;
        PT::CProject* project_item = dynamic_cast<PT::CProject*>(&item);
        if(project_item)    {
            TProjectId id = project_item->GetData()->GetId();
            ids.push_back(id);
        }
    }
}


void CProjectTreePanel::OnItemExpandedCollapsed(wxTreeEvent& event)
{
    wxTreeItemId id = event.GetItem();
    PT::CItem* item = x_GetExplorerItem(id);
    item->OnItemExpandedCollapsed(*m_Tree);
}


void CProjectTreePanel::OnItemActivated(wxTreeEvent& event)
{
    wxTreeItemId id = event.GetItem();
    if (!id.IsOk()) return;

    PT::CItem* item = x_GetExplorerItem(id);
    if (!item) return;

    CIRef<IMenuService> menu_srv = m_Workbench->GetServiceByType<IMenuService>();

    int type = item->GetType();

    switch(type)    {
    case PT::eWorkspace:
    case PT::eProjectFolder:
        // OnProperties(item);
        break;

    case PT::eProject:
        break;

    case PT::eProjectItem: {{
        menu_srv->AddPendingCommand(eCmdOpenViewDefault);
        break;
    }}
    case PT::eView:    {{
        menu_srv->AddPendingCommand(eCmdActivateClients);
        break;
    }}
    case PT::eDataSource: {{
        PT::CDataSource* ds_item =
            dynamic_cast<PT::CDataSource*>(item);

        if(ds_item) {
            CIRef<IUIDataSource> ds = ds_item->GetData();
            int def_command = ds->GetDefaultCommand();
            if( def_command ){
                menu_srv->AddPendingCommand(def_command);

                wxEvtHandler* handler = ds->CreateEvtHandler();
                if( handler ){
                    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, def_command);
                    handler->ProcessEvent(event);

                    delete handler;
                }
            }
        }
    }}
    case PT::eHiddenItems: {{
        PT::sm_HideDisabledItems = !PT::sm_HideDisabledItems;
        UpdateDisabledItems();
    }}
    default:
        break;
    }
}


void CProjectTreePanel::OnSelectionChanged(wxTreeEvent& event)
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged);
    Send(&evt, ePool_Parent);
}

void CProjectTreePanel::OnLeftDown(wxMouseEvent& event)
{
    event.Skip();
}


// this functions inititates and performs the whole D&D session
void CProjectTreePanel::OnBeginDrag(wxTreeEvent& event)
{
    // create a wxDataObject for the selected items
    m_DataObject = x_CreateDataObject(false);
    if (!m_DataObject) return;

    // reset D&D state
    m_DropItemId = wxTreeItemId();
    m_DropTimerActive = false;

	// For osx cocoa, we use wxTreeCtrl's built-in D&D.  For others we continue to
    // use the original (general) drag and drop and approach.  They seem
    // to be equivalent in behavior in this case afik)
#ifdef __WXOSX_COCOA__
    // Tree D&D does not take over the event loop...
    event.Allow();
#else
    wxDropSource source(*m_DataObject, this);

    // the whole D&D session happens in this call
    wxDragResult res = source.DoDragDrop(wxDragCopy | wxDragMove);

    // reset D&D state
	if(m_DropItemId.IsOk()) {
		m_Tree->SetItemDropHighlight(m_DropItemId, false);
	}
	m_DropItemId = wxTreeItemId();
	m_DropItemTimer.Stop();
	m_DropTimerActive = false;

    delete m_DataObject;
    m_DataObject = NULL;

    // at this point session has ended and Drag Traget already has done all necessary work
    if(res == wxDragError)  {
        NcbiErrorBox("Unexpected error while performing D&D");
        ERR_POST("CProjectTreePanel::OnBeginDrag() - Unexpected error while performing D&D");
    }
#endif
}

// end drag for the trees built-in drag and drop (used for osx cocoa)
// never called by other os's since they don't call event.Allow() when
// D&D starts.)
void CProjectTreePanel::OnEndDrag(wxTreeEvent& event)
{
    int flags = 0;
    wxTreeItemId id = m_Tree->HitTest( event.GetPoint(), flags);

    if( id.IsOk() ){
        PT::CItem* item = x_GetExplorerItem(id);
        PT::TItems items;
        m_DataObject->GetItems(*m_Tree, items);
        item->Paste(*m_Tree, items, !m_CopyMode);
    }

    // reset D&D state
    m_DropItemId = wxTreeItemId();
    m_DropItemTimer.Stop();
    m_DropTimerActive = false;
    delete m_DataObject;
    m_DataObject = NULL;
}

void CProjectTreePanel::x_SetSelections(wxArrayTreeItemIds& ids)
{
    wxArrayTreeItemIds sel_ids;
    m_Tree->GetSelections(sel_ids);

    //reset current selection
    for(  size_t i = 0;  i < sel_ids.size();  i++ ) {
        wxTreeItemId id = sel_ids[i];
        m_Tree->SelectItem(id, false);
    }
    // set new selection
    for(  size_t i = 0;  i < ids.size();  i++ ) {
        wxTreeItemId id = ids[i];
        m_Tree->SelectItem(id, true);
    }
}


wxDragResult CProjectTreePanel::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
    // If null, a drag is not in progress.
    if (m_DataObject == NULL)
        return def;

    static const double kExpandDelay = 0.5;
    // get the item under the mouse pointer
    int flags = 0;
    wxTreeItemId id = m_Tree->HitTest(wxPoint(x, y), flags);

    if(id != m_DropItemId)  {
        // current item have changed - need to update highlight and "expand" timer

        // stop the timer
        m_DropItemTimer.Stop();
        m_DropTimerActive = false;

        // change highlight
        if(m_DropItemId.IsOk()) {
        	m_Tree->SetItemDropHighlight(m_DropItemId, false);
        }

        m_DropItemId = id;

        if(m_DropItemId.IsOk()) {
            m_Tree->SetItemDropHighlight(m_DropItemId, true);

            // launch "expand" timer
            if(m_Tree->ItemHasChildren(m_DropItemId)  &&  ! m_Tree->IsExpanded(m_DropItemId)) {
                // start the timer
                m_DropItemTimer.Restart();
                m_DropTimerActive = true;
            }
        }
    } else if(m_DropTimerActive)    {
        // current item has not changed and timer is active
        if(m_DropItemTimer.Elapsed() >= kExpandDelay) {
            m_Tree->Expand(m_DropItemId);

            m_DropItemTimer.Stop();
            m_DropTimerActive = false;
        }
    }

    if(id.IsOk())    {
        PT::CItem* item = x_GetExplorerItem(id);
        if (item->CanPaste(*m_Tree))
            return def;
    }
    return wxDragNone;
}

// drag end for standard (general) drag and drop used on non-cocoa platforms
wxDragResult CProjectTreePanel::OnDrop(
    wxCoord x, wxCoord y, wxDragResult def, CAppExplorerDataObject& data
){
    if( def == wxDragCopy || def == wxDragMove ){
        int flags = 0;
        wxTreeItemId id = m_Tree->HitTest( wxPoint(x, y), flags );

        if( id.IsOk() ){
            PT::CItem* item = x_GetExplorerItem(id);

            if (item->CanPaste(*m_Tree)) {
                bool move = (def == wxDragMove);
                PT::TItems items;
                data.GetItems(*m_Tree, items);
                item->Paste(*m_Tree, items, move);
                return def;
            }
        }
    }

    return wxDragNone;
}

void CProjectTreePanel::OnTreeKeyDown(wxTreeEvent& event)
{
    const wxKeyEvent& keyEvent = event.GetKeyEvent();
    m_CopyMode = keyEvent.CmdDown(); 

    if (WXK_F2 != keyEvent.GetKeyCode() || keyEvent.GetModifiers() != wxMOD_NONE) {
        event.Skip();
        return;
    }

    wxArrayTreeItemIds items;
    if (m_Tree->GetSelections(items) != 1)
        return;

    wxTreeItemId item = items[0];
    if (!item.IsOk())
        return;

    (void)m_Tree->EditLabel(item);
}

void CProjectTreePanel::OnBeginLabelEdit(wxTreeEvent& event)
{
    wxTreeItemId item = event.GetItem();
    PT::CItem* eitem = x_GetExplorerItem(item);
    eitem->BeginLabelEdit(*m_Tree, event);
}

void CProjectTreePanel::OnEndLabelEdit(wxTreeEvent& event)
{
    wxTreeItemId item = event.GetItem();
    PT::CItem* eitem = x_GetExplorerItem(item);
    if (!eitem)
        return;

    if (eitem->EndLabelEdit(*m_Tree, event))
        x_RefreshProjectTables(PT::GetProjectId(*m_Tree, *eitem));
}

void CProjectTreePanel::OnPaste(wxCommandEvent& event)
{
    PT::TItems sel_items;
    GetSelectedItems(sel_items);

    if (sel_items.size() == 1) {
        wxClipboardLocker clipLocker;
        if (!clipLocker) return;

        CAppExplorerDataObject data_object;
        if (!wxTheClipboard->GetData(data_object))
            return;
        wxTheClipboard->Clear();

        PT::TItems items;
        data_object.GetItems(*m_Tree, items);
        sel_items[0]->Paste(*m_Tree, items, data_object.IsCut());
    } else _ASSERT(false);
}

void CProjectTreePanel::OnUpdatePaste(wxUpdateUIEvent& event)
{
    PT::TItems sel_items;
    GetSelectedItems(sel_items);

    bool en = false;
    if (sel_items.size() == 1 && sel_items[0]->CanPaste(*m_Tree)) {
        wxClipboardLocker clipLocker;
        if (!!clipLocker)
            en = wxTheClipboard->IsSupported(CAppExplorerDataObject::m_ItemsFormat);
    }
    event.Enable(en);
}

void CProjectTreePanel::OnCut(wxCommandEvent& event)
{
    x_CutOrCopyToClipboard (true);
}

void CProjectTreePanel::OnUpdateCut(wxUpdateUIEvent& event)
{
    PT::TItems sel_items;
    GetSelectedItems(sel_items);

    if (sel_items.empty()) {
        event.Enable(false);
        return;
    }

    ITERATE (PT::TItems, it, sel_items) {
        if (!(*it)->CanCutToClipboard(*m_Tree)) {
            event.Enable(false);
            return;
        }
    }

    event.Enable(true);
}


void CProjectTreePanel::OnCopy(wxCommandEvent& event)
{
    x_CutOrCopyToClipboard (false);
}


void CProjectTreePanel::OnUpdateCopy(wxUpdateUIEvent& event)
{
    PT::TItems sel_items;
    GetSelectedItems(sel_items);

    if (sel_items.empty()) {
        event.Enable(false);
        return;
    }

    ITERATE (PT::TItems, it, sel_items) {
        if (!(*it)->CanCopyToClipboard(*m_Tree)) {
            event.Enable(false);
            return;
        }
    }

    event.Enable(true);
}

void CProjectTreePanel::OnRemove(wxCommandEvent& event)
{
    wxArrayTreeItemIds sel_ids;
    m_Tree->GetSelections(sel_ids);
    if (sel_ids.empty()) return;

    wxString quest;
    if (sel_ids.size() == 1) {
        quest = wxT("Delete selected item?");
    } else {
        quest.Printf(wxT("Delete %lu selected items?"), (unsigned long)sel_ids.size());
    }
    int answer = 
        wxMessageBox(quest, wxT("Confirm"), 
                        wxYES_NO | wxCANCEL | wxICON_QUESTION );
    if (answer != wxYES)
        return;

    s_RemoveDependentItems(*m_Tree, sel_ids);

    PT::TItems sel_items;
    x_GetItemsFromIds(sel_ids, sel_items);

    set<int> projIds;

    typedef map<PT::CProject*, vector<CProjectItem*> > TItemMap;
    TItemMap toDelete;

    NON_CONST_ITERATE (PT::TItems, it, sel_items) {
        if ((*it)->GetType() == PT::eProjectItem) {
            PT::CProjectItem& item = static_cast<PT::CProjectItem&>(**it);
            PT::CProject* project = PT::GetProject (*m_Tree, **it);
            if (project) {
                CRef<CProjectItem> prjItem = item.GetData();
                toDelete[project].push_back(prjItem);
            }
        } else {
            int projId = PT::GetProjectId(*m_Tree, **it);
            if ((*it)->DoRemove(*m_Tree))
                projIds.insert(projId);
        }
    }

    ITERATE(TItemMap, it, toDelete) {
        CGBDocument* doc = it->first->GetData();
        if (doc) {
            if (doc->RemoveProjectItems(it->second)) {
                projIds.insert(doc->GetId());
            }
        }
    }

    ITERATE(set<int>, it, projIds)
        x_RefreshProjectTables(*it);

    PT::CWorkspace* wsItem = x_GetWorkspaceItem();
    if (wsItem) wsItem->UpdateHiddenItems(*m_Tree);
    m_Tree->UnselectAll();
}


void CProjectTreePanel::OnUpdateRemove(wxUpdateUIEvent& event)
{
    // check if we can delete this selection
    PT::TItems sel_items;
    GetSelectedItems(sel_items);
    if (sel_items.empty()) {
        event.Enable(false);
        return;
    }

    ITERATE(PT::TItems, it, sel_items) {
        if (!(*it)->CanDoRemove(*m_Tree)) {
            event.Enable(false);
            return;
        }
    }

    event.Enable(true);
}

void CProjectTreePanel::OnShowHideDisabledItems(wxCommandEvent& event)
{
    PT::sm_HideDisabledItems = !PT::sm_HideDisabledItems;
    UpdateDisabledItems();
}

void CProjectTreePanel::OnUpdateShowHideDisabledItems(wxUpdateUIEvent& event)
{
    event.SetText(PT::sm_HideDisabledItems ? wxT("Show Disabled Items") : wxT("Hide Disabled Items"));
    event.Enable(true);
}

void CProjectTreePanel::OnEnableDisable(wxCommandEvent& event)
{
    PT::TItems sel_items;
    GetSelectedItems(sel_items);

    PT::CWorkspace* wsItem = x_GetWorkspaceItem();
    if (!wsItem) return;

    typedef map<PT::CProject*, vector<CProjectItem*> > TItemMap;
    TItemMap toEnable, toDisable;

    NON_CONST_ITERATE(PT::TItems, it, sel_items) {
        if ((*it)->GetType() == PT::eProjectItem) {
            PT::CProjectItem& item = static_cast<PT::CProjectItem&>(**it);
            PT::CProject* project = PT::GetProject (*m_Tree, **it);
            if (project) {
                CRef<CProjectItem> prjItem = item.GetData();
                if (prjItem->IsEnabled())
                    toDisable[project].push_back(prjItem);
                else
                    toEnable[project].push_back(prjItem);
            }
        }
    }

    ITERATE(TItemMap, it, toEnable) {
        CGBDocument* doc = it->first->GetData();
        if (doc) {
            doc->AttachProjectItems(it->second);
            it->first->UpdateProjectItems(*m_Tree);
        }
    }

    ITERATE(TItemMap, it, toDisable) {
        CGBDocument* doc = it->first->GetData();
        if (doc) {
            doc->DetachProjectItems(it->second);
            it->first->UpdateProjectItems(*m_Tree);
        }
    }

    wsItem->UpdateHiddenItems(*m_Tree);
}

void CProjectTreePanel::OnUpdateEnableDisable(wxUpdateUIEvent& event)
{
    PT::TItems sel_items;
    GetSelectedItems(sel_items);

    int types = PT::GetItemTypes(sel_items);
    if (types == PT::eProjectItem) {
        // only Project Items or Data Loaders
        bool en = false, dis = false;
        for (size_t i = 0; i < sel_items.size(); i++) {
            PT::CProjectItem& item = static_cast<PT::CProjectItem&>(*sel_items[i]);
            if(item.GetData()->IsEnabled())    {
                en = true;
            } else {
                dis = true;
            }
        }
        wxString text = (en == dis) ? wxT("Enable / Disable") : (en ? wxT("Disable") : wxT("Enable"));
        event.SetText(text);
        event.Enable(true);
    } else {
        event.Enable(false);
    }
}

void CProjectTreePanel::OnNewFolder(wxCommandEvent& event)
{
    PT::TItems sel_items;
    GetSelectedItems(sel_items);

    if(sel_items.size() == 1) {
        PT::CItem& item = *sel_items[0];
        item.DoNewFolder(*m_Tree);
    } else _ASSERT(false);
}


void CProjectTreePanel::OnUpdateNewFolder(wxUpdateUIEvent& event)
{
    PT::TItems sel_items;
    GetSelectedItems(sel_items);

    bool en = false;
    if (sel_items.size() == 1)
        en = sel_items[0]->CanDoNewFolder();
    event.Enable(en);
}

static void s_ItemsToViews(PT::TItems& items, vector<CIRef<IProjectView> >& views)
{
    set<IProjectView*> st_sel_views; // to avoid dupplication

    for (size_t i = 0; i < items.size(); ++i) {
        PT::CItem& item = *items[i];
        if (item.GetType() == PT::eView) {
            IProjectView* view = static_cast<PT::CView*>(&item)->GetData();
            st_sel_views.insert(view);
        }
    }
    // copy from st_sel_views to sel_views
    NON_CONST_ITERATE(set<IProjectView*>, it, st_sel_views) {
        views.push_back(CIRef<IProjectView>(*it));
    }
}

void CProjectTreePanel::OnCloseView(wxCommandEvent& event)
{
    if (!m_Workbench) return;

    vector<CIRef<IProjectView> > views;

    PT::TItems sel_items;
    GetSelectedItems(sel_items);

    s_ItemsToViews(sel_items, views);

    CProjectService* prj_srv =
        m_Workbench->GetServiceByType<CProjectService>();

    for(  size_t i = 0; i < views.size();  i++) {
        IProjectView& view = *views[i];
        prj_srv->RemoveProjectView(view);
    }
}


void CProjectTreePanel::OnUpdateCloseView(wxUpdateUIEvent& event)
{
    vector<CIRef<IProjectView> > views;

    PT::TItems sel_items;
    GetSelectedItems(sel_items);

    s_ItemsToViews(sel_items, views);
    event.Enable(views.size() > 0);
}


void CProjectTreePanel::OnActivateClients(wxCommandEvent& event)
{
    if (!m_Workbench) return;

    vector<CIRef<IProjectView> > views;

    PT::TItems sel_items;
    GetSelectedItems(sel_items);

    s_ItemsToViews(sel_items, views);

    vector<IWMClient*>  clients;
    for(  size_t i = 0; i < views.size();  i++) {
        IProjectView* view = views[i].GetPointer();
        clients.push_back(view);
    }

    IWindowManagerService* wm_srv =
        m_Workbench->GetServiceByType<IWindowManagerService>();

    wm_srv->ActivateClients(clients);
}


void CProjectTreePanel::OnUpdateActivateClients(wxUpdateUIEvent& event)
{
    vector<CIRef<IProjectView> > views;

    PT::TItems sel_items;
    GetSelectedItems(sel_items);

    s_ItemsToViews(sel_items, views);

    event.Enable(views.size() > 0);
}


void CProjectTreePanel::OnProperties(wxCommandEvent& event)
{
    PT::TItems sel_items;
    GetSelectedItems(sel_items);

    if (sel_items.size() == 1) {
        PT::CItem& item = *sel_items[0];
        if (item.DoProperties(*m_Tree)) {
            x_RefreshProjectTables(PT::GetProjectId(*m_Tree, item));
        }
    } else _ASSERT(false);
}


void CProjectTreePanel::OnUpdateProperties(wxUpdateUIEvent& event)
{
    PT::TItems sel_items;
    GetSelectedItems(sel_items);

    bool en = false;
    if(sel_items.size() == 1) 
        en = sel_items[0]->CanDoProperties();
    event.Enable(en);
}

void CProjectTreePanel::OnUpdateUnLoadProject(wxUpdateUIEvent& event)
{
    event.Enable(false);

    TProjectIdVector ids;
    GetSelectedProjectIds(ids);

    if (!m_WS) return;

    ITERATE(TProjectIdVector, it, ids) {
        CGBDocument* doc = dynamic_cast<CGBDocument*>(m_WS->GetProjectFromId(*it));
        if (doc && (doc->IsLoaded() || doc->IsLoading())) {
            event.Enable(true);
            return;
        }
    }
}

void CProjectTreePanel::OnUnLoadProject(wxCommandEvent& event)
{
    TProjectIdVector ids, toUnload;
    GetSelectedProjectIds(ids);

    if (!m_WS) return;
    ITERATE(TProjectIdVector, it, ids) {
        CGBDocument* doc = dynamic_cast<CGBDocument*>(m_WS->GetProjectFromId(*it));
        if (doc && (doc->IsLoaded() || doc->IsLoading())) {
            toUnload.push_back(*it);
        }
    }

    if (toUnload.empty())
        return;

    CProjectTask::UnLoadProjects(m_Workbench, toUnload);
}

void CProjectTreePanel::OnLoadProject(wxCommandEvent& event)
{
    TProjectIdVector ids, toLoad;
    GetSelectedProjectIds(ids);

    if (!m_WS) return;
    ITERATE(TProjectIdVector, it, ids) {
        CGBDocument* doc = dynamic_cast<CGBDocument*>(m_WS->GetProjectFromId(*it));
        if (doc && !(doc->IsLoaded() || doc->IsLoading())) {
            toLoad.push_back(*it);
        }
    }

    if (toLoad.empty())
        return;

    CProjectTask::LoadProjects(m_Workbench, toLoad);
}

void CProjectTreePanel::OnUpdateLoadProject(wxUpdateUIEvent& event)
{
    event.Enable(false);

    TProjectIdVector ids;
    GetSelectedProjectIds(ids);

    if (!m_WS) return;
    ITERATE(TProjectIdVector, it, ids) {
        CGBDocument* doc = dynamic_cast<CGBDocument*>(m_WS->GetProjectFromId(*it));
        if (doc && !(doc->IsLoaded() || doc->IsLoading())) {
            event.Enable(true);
            return;
        }
    }
}

void CProjectTreePanel::OnRemoveProject(wxCommandEvent& event)
{
    TProjectIdVector ids, toRemove;
    GetSelectedProjectIds(ids);
    if (ids.empty()) return;

    if (!m_WS) return;
    ITERATE(TProjectIdVector, it, ids) {
        CGBDocument* doc = dynamic_cast<CGBDocument*>(m_WS->GetProjectFromId(*it));
        if (doc) {
            toRemove.push_back(*it);
        }
    }

    if (toRemove.empty()) return;

    CProjectTask::RemoveProjects(m_Workbench, toRemove);
}

void CProjectTreePanel::OnUpdateRemoveProject(wxUpdateUIEvent& event)
{
    event.Enable(false);

    TProjectIdVector ids;
    GetSelectedProjectIds(ids);

    if (!m_WS) return;
    ITERATE(TProjectIdVector, it, ids) {
        CGBDocument* doc = dynamic_cast<CGBDocument*>(m_WS->GetProjectFromId(*it));
        if (doc) {
            event.Enable(true);
            return;
        }
    }
}

void CProjectTreePanel::OnProjectTableView(wxCommandEvent& event)
{
    TProjectIdVector ids;
    GetSelectedProjectIds(ids);

    if (!m_WS) return;

    CProjectService* projectService = m_Workbench->GetServiceByType<CProjectService>();

    ITERATE(TProjectIdVector, it, ids) {
        CGBDocument* doc = dynamic_cast<CGBDocument*>(m_WS->GetProjectFromId(*it));
        if (doc && doc->IsLoaded()) {
            const CGBProject_ver2* gbproj = dynamic_cast<const CGBProject_ver2*>(&doc->GetProject());
            if (gbproj) {
                SConstScopedObject obj(gbproj, doc->GetScope());
                projectService->AddProjectView( "Project Table View", obj, 0 );
            }
        }
    }
}

void CProjectTreePanel::OnUpdateProjectTableView(wxUpdateUIEvent& event)
{
    event.Enable(false);

    TProjectIdVector ids;
    GetSelectedProjectIds(ids);

    if (!m_WS) return;
    ITERATE(TProjectIdVector, it, ids) {
        CGBDocument* doc = dynamic_cast<CGBDocument*>(m_WS->GetProjectFromId(*it));
        if (doc && doc->IsLoaded()) {
            event.Enable(true);
            return;
        }
    }
}

static bool s_IsProjectDataFolder(wxTreeCtrl& treeCtrl, const PT::CItem& item)
{
    if (item.GetType() != PT::eProjectFolder)
        return false;

    const PT::CItem* parent_item = item.GetParent(treeCtrl);
    if (parent_item != 0 && parent_item->GetType() == PT::eProject)
        return true;

    return false;
}

// returns true if Load / Unload command can be performed on the given set of items
// all items must represent Projects and at least one must allow the operation
static bool s_CanLoadUnloadProjects(CGBWorkspace* ws, PT::TItems& items, bool load)
{
    if (!ws) return false;

    for (size_t i = 0; i < items.size(); ++i) {
        PT::CItem& item = *items[i];
        PT::CProject* project_item = dynamic_cast<PT::CProject*>(&item);

        if(project_item)    {
            CGBDocument* doc = project_item->GetData();
            if (doc && !doc->IsLoading() && doc->IsLoaded() != load) {
                return true; // found a project on which we can perform the operation
            }
        } else {
            return false; // not a Project
        }
    }
    return false;
}

static const int kSaveTypesMask =
    PT::eWorkspace |
    PT::eProject;

static const int kClipboardTypesMask =
    PT::eProjectFolder |
    PT::eProjectItem |
    PT::eDataLoader;

static const int kPropertyTypesMask =
    PT::eProjectFolder |
    PT::eProject |
    PT::eProjectItem |
    PT::eDataLoader |
    PT::eDataSource;

IExplorerItemCmdContributor::TContribution
    CProjectTreePanel::x_GetContextMenu(PT::TItems& items)
{
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();

    int types = PT::GetItemTypes(items);

    bool projectDataFolder = false;
    ITERATE(PT::TItems, it, items) {
        if (s_IsProjectDataFolder(*m_Tree, **it)) {
            projectDataFolder = true;
            break;
        }
    }

    wxMenu* menu = new wxMenu;

    // Save and Save As commands for Workspace and Projects
    if( (types & kSaveTypesMask)  &&  (types & ~kSaveTypesMask) == 0)    {
        bool ws = types & PT::eWorkspace;
        if(ws  || s_CanLoadUnloadProjects(m_WS, items, false))    {
            menu->Append(wxID_SEPARATOR, wxT("Top Actions"));
            cmd_reg.AppendMenuItem(*menu, wxID_SAVE);
            cmd_reg.AppendMenuItem(*menu, wxID_SAVEAS);
        }
        menu->Append(wxID_SEPARATOR, wxT("Actions"));
        cmd_reg.AppendMenuItem(*menu, eCmdShowHideDisabledItems);
    }

    // View commands
    if(types  ==  PT::eView)    {
        menu->Append(wxID_SEPARATOR, wxT("Actions"));
        cmd_reg.AppendMenuItem( *menu, eCmdActivateClients );
        cmd_reg.AppendMenuItem( *menu, eCmdCloseProjectView );
        cmd_reg.AppendMenuItem( *menu, eCmdCloseAllProjectViews );

    } else if(items.size() == 1 &&
              items[0]->GetType() == PT::eFolder &&
              static_cast<PT::CFolder*>(items[0])->GetData() == wxT("All Views"))
    {
        menu->Append(wxID_SEPARATOR, wxT("Actions"));
        cmd_reg.AppendMenuItem(*menu, eCmdCloseAllProjectViews);
    }

    // Project Item commands
    if( types == PT::eProjectItem ){

        menu->Append(wxID_SEPARATOR, wxT("Top Actions"));
        cmd_reg.AppendMenuItem(*menu, eCmdOpenView);
        menu->Append(wxID_SEPARATOR, wxT("Actions"));
        cmd_reg.AppendMenuItem(*menu, eCmdEnableDisable);
        menu->Append(wxID_SEPARATOR, wxT("Actions"));
        cmd_reg.AppendMenuItem(*menu, eCmdShowHideDisabledItems);
    }

    // Project commands
    if( types == PT::eProject ){
        menu->Append(wxID_SEPARATOR, wxT("Actions"));
        cmd_reg.AppendMenuItem(*menu, eCmdLoadProject);
        cmd_reg.AppendMenuItem(*menu, eCmdUnLoadProject);
        cmd_reg.AppendMenuItem(*menu, eCmdProjectTableView);
    }

    // Edit commands
    TCmdID  clp_cmds[4] = { wxID_CUT, wxID_COPY, wxID_PASTE, wxID_DELETE };

    if((types & kClipboardTypesMask) == types)  {
        menu->Append(wxID_SEPARATOR, wxT("Edit"));

        if (projectDataFolder) {
            cmd_reg.AppendMenuItem( *menu, wxID_PASTE );

        } else {
            cmd_reg.AppendMenuItems(*menu, clp_cmds, sizeof(clp_cmds) / sizeof(TCmdID));
        }

        if (types == PT::eProjectFolder)
            cmd_reg.AppendMenuItem( *menu, eCmdNewFolder );
    }

    if( types == PT::eProject ){
        menu->Append(wxID_SEPARATOR, wxT("Edit"));
        cmd_reg.AppendMenuItem( *menu, eCmdRemoveProject );

    }

    // Properties
    if( (types & kPropertyTypesMask) == types ){
        menu->Append( wxID_SEPARATOR, wxT("Properties") );
        cmd_reg.AppendMenuItem( *menu, wxID_PROPERTIES );
    }

    return IExplorerItemCmdContributor::TContribution(menu, (wxEvtHandler*)NULL);
}

void CProjectTreePanel::ProjectStateChanged(CGBDocument& doc)
{
    PT::CWorkspace* wsItem = x_GetWorkspaceItem();
    if (wsItem) {
        wxWindowUpdateLocker locker(m_Tree);
        wsItem->ProjectStateChanged(*m_Tree, doc);
    }
}

void CProjectTreePanel::ProjectViewsChanged(CGBDocument& doc)
{
    PT::CRoot* rootItem = x_GetRootItem();
    if (rootItem) {
        wxWindowUpdateLocker locker(m_Tree);
        PT::CWorkspace* wsItem = rootItem->GetWorkspaceItem(*m_Tree);
        if (wsItem) wsItem->UpdateViews(*m_Tree, &doc);
        rootItem->UpdateAllViews(*m_Tree, m_WS);
    }
}

void CProjectTreePanel::ProjectAdded(CGBDocument& doc)
{
    PT::CWorkspace* wsItem = x_GetWorkspaceItem();
    if (wsItem) {
        wxWindowUpdateLocker locker(m_Tree);
        wsItem->ProjectAdded(*m_Tree, doc);
    }
}

void CProjectTreePanel::UpdateDisabledItems()
{
    wxBusyCursor wait;

    PT::CWorkspace* wsItem = x_GetWorkspaceItem();
    wxWindowUpdateLocker locker(m_Tree);
    if (wsItem) wsItem->UpdateDisabledItems(*m_Tree);
}

void CProjectTreePanel::ProjectRemoved(size_t id)
{
    PT::CWorkspace* wsItem = x_GetWorkspaceItem();
    wxWindowUpdateLocker locker(m_Tree);
    if (wsItem) wsItem->ProjectRemoved(*m_Tree, id);
}

void CProjectTreePanel::UpdateWorkspaceLabel()
{
    PT::CWorkspace* wsItem = x_GetWorkspaceItem();
    if (wsItem) wsItem->UpdateLabel(*m_Tree);
}

void CProjectTreePanel::ProjectUpdateItems(CGBDocument& doc)
{
    wxBusyCursor wait;

    PT::CWorkspace* wsItem = x_GetWorkspaceItem();
    if (wsItem) {
        wxWindowUpdateLocker locker(m_Tree);
        wsItem->UpdateProjectItems(*m_Tree, doc);
    }
}

void CProjectTreePanel::ProjectUpdateLabel(CGBDocument& doc)
{
    PT::CWorkspace* wsItem = x_GetWorkspaceItem();
    if (wsItem) wsItem->UpdateProjectLabel(*m_Tree, doc);
}

void CProjectTreePanel::UpdateViewLabel(CGBDocument& doc, IProjectView& view)
{
    PT::CRoot* rootItem = x_GetRootItem();
    if (rootItem) {
        rootItem->UpdateViewLabel(*m_Tree, view);
        PT::CWorkspace* wsItem = rootItem->GetWorkspaceItem(*m_Tree);
        if (wsItem) wsItem->UpdateViewLabel(*m_Tree, doc, view);
    }
}

END_NCBI_SCOPE
