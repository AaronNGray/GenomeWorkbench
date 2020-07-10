/*  $Id: dock_container.cpp 43819 2019-09-05 19:33:41Z katargir $
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

#include <gui/widgets/wx/dock_container.hpp>
#include <gui/widgets/wx/dock_notebook.hpp>
#include <gui/widgets/wx/dock_hint_frame.hpp>

#include <gui/widgets/wx/window_manager.hpp>
#include <gui/widgets/wx/dock_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/wm_client.hpp>

#include <objects/general/User_object.hpp>
#include <objects/general/Object_id.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

IMPLEMENT_DYNAMIC_CLASS(CBeginDragOutsideEvent, wxEvent)
DEFINE_EVENT_TYPE(USER_EVT_BEGIN_DRAG_OUTSIDE)

////////////////////////////////////////////////////////////////////////////////
/// CDockContainer

BEGIN_EVENT_TABLE(CDockContainer, CDockContainer::TParent)
    EVT_MOTION(CDockContainer::OnMouseMove)
    EVT_LEFT_DOWN(CDockContainer::OnLeftDown)
    EVT_LEFT_UP(CDockContainer::OnLeftUp)
    EVT_MOUSE_CAPTURE_LOST(CDockContainer::OnMouseCaptureLost)
    EVT_BEGIN_DRAG_OUTSIDE(CDockContainer::OnTabBeginDragOutside)
END_EVENT_TABLE()


CDockContainer::CDockContainer(CDockManager& manager, wxWindow* parent, bool create_tab)
:   TParent(parent, wxID_ANY, wxDefaultPosition, wxSize(0,0),
            wxCLIP_CHILDREN, _("dock_container")),
    m_DockManager(manager),
    m_WindowManager(manager.GetWindowManager()),
    m_DockFrame(NULL),
    m_RootWindow(NULL),
    m_MarkersWindow(NULL),
    m_DragTarget(NULL),
    m_HintWindow(NULL),
    m_TabDockable(NULL)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    m_DockFrame = dynamic_cast<CFloatingFrame*>(parent);

    x_CreateControls();

    x_InitDefaultTree(create_tab);
}

void CDockContainer::x_InitDefaultTree(bool create_tab)
{
    // create Full tree with a Notebook (for main container) or empty (for
    // floating frames)
    CRef<CDockLayoutTree> tree;
    if (create_tab) {
        TNode* node = new TNode(CDockLayoutTree::eTab);
        tree.Reset(new CDockLayoutTree(node, node));
    } else {
        tree.Reset(new CDockLayoutTree());
    }

    // initialize the container based on the full tree
    SetFullTree(*tree);
}

CDockContainer::~CDockContainer()
{
}


CDockManager& CDockContainer::GetDockManager()
{
    return  m_DockManager;
}


CFloatingFrame* CDockContainer::GetDockFrame()
{
    return m_DockFrame;
}


wxWindow* CDockContainer::GetRootWindow()
{
    if (m_VisibleTree  &&  m_VisibleTree->GetRoot()) {
        return m_VisibleTree->GetRoot()->GetWindow();
    }
    return NULL;
}


bool CDockContainer::Layout()
{
    return wxWindow::Layout();
}


void CDockContainer::x_CreateControls()
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);
}


// sets the new window as the Root window of the container
void CDockContainer::x_SetRootWindow(wxWindow* window)
{
    if (window != m_RootWindow) {
        if (m_RootWindow) {
            // disconnect the old window
            GetSizer()->Detach(m_RootWindow);
        }

        m_RootWindow = window;

        CDockPanel* panel = dynamic_cast<CDockPanel*>(m_RootWindow);
        if(panel)   {
            panel->ShowBorder(false);
        }

        if (m_RootWindow) {
            m_RootWindow->Reparent(this);
            GetSizer()->Add(m_RootWindow, 1, wxEXPAND);
            m_RootWindow->Show();

            GetSizer()->Layout();
        }
    }
}


// applies a layout tree to the container
void CDockContainer::SetFullTree(CDockLayoutTree& full_tree)
{
    _ASSERT(HasNoWMClients());

    m_FullTree.Reset(&full_tree);

    CRef<TNode> centralPane = m_FullTree->GetCentralPane();

    if (centralPane) {
        CRef<TNode> node(new TNode(CDockLayoutTree::eTab));
        m_VisibleTree.Reset(new CDockLayoutTree(node, node));
        node->Link(*centralPane);
    }
    else {
        m_VisibleTree.Reset(new CDockLayoutTree());
    }

    // traverse visible tree and create components as needed
    CRef<TNode> root = m_VisibleTree->GetRoot();
    if (root  &&  root->IsVisible()) {
        x_InstantiateNode(*root->GetClone(), true);

        wxWindow* win = root->GetWindow();
        if (win) {
            x_SetRootWindow(win);
        }

        BlockSplittersLayout(false);
    }
}


// Instantiates clients by fingerprints using the given factory.
class FClientCreator
{
public:
    FClientCreator(CDockContainer& cont, IWMClientFactory* factory)
    :   m_DockContainer(cont),
        m_Factory(factory)
    {}

    void    operator()(CDockLayoutTree::CNode& node)
    {
        if (m_Factory  && ! node.IsContainer()  &&  node.IsVisible()) {
            IWMClient* client = NULL;
            IWMClient::CFingerprint fingerprint = node.GetFingerprint();

            client = m_Factory->CreateClient(fingerprint, &m_DockContainer);

            if (client) {
                // client has been created - wrap it in a Dock Panel

                wxString pname = _("panel_");
                pname << ToWxString(fingerprint.GetId());

                CDockManager& manager = m_DockContainer.GetDockManager();
                CDockPanel* panel = new CDockPanel(&m_DockContainer, &manager,
                                                   client, pname);
                                                   
                node.SetWindow(panel);
            } else {
                // client cannot be created - hide the node
                node.SetHidden(true);
            }
        }
    }
protected:
    CDockContainer&   m_DockContainer;
    IWMClientFactory* m_Factory;
};


class FFindHiddenClient
{
public:
    bool operator()(CDockLayoutTree::CNode& node)
    {
        bool found = ! node.IsContainer()  &&  node.IsHidden();
        if(found)   {
            m_FoundNode.Reset(&node);
        }
        return found;
    }

    CRef<CDockContainer::TNode> m_FoundNode;
};


/// This clean-up normalizes the tree, so that it can be instantiated as a layout.
/// The function applies x_Full_CleanLayoutTreeBranch() recusrsively.
void CDockContainer::x_Full_CleanLayoutTree(CDockLayoutTree& full_tree)
{
    CRef<TNode> full_root = full_tree.GetRoot();
    if (full_root) {
        x_Full_CleanLayoutTreeBranch(full_tree.GetCentralPane(), *full_root);

        bool kill_root = false;
        if(full_root->IsClient())    {
            kill_root = full_root->IsHidden();
        } else {
            kill_root = ! full_root->HasChildren()  &&
                          (full_root != full_tree.GetCentralPane());
        }
        if(kill_root) {
            full_tree.m_Root.Reset();
        }
    }
}


/// Cleans the given tree starting from the given node.
/// This function removes all hidden client nodes, then it removes empty
/// containers. Containers that have exactly one child node are replaced with
/// this child node.
void  CDockContainer::x_Full_CleanLayoutTreeBranch(TNode* centralPane, TNode& full_node)
{
    if(full_node.IsContainer())  {
        CDockLayoutTree::TNodeVector&  children = full_node.GetChildren();
        for( size_t i = 0;  i < children.size();  )  {
            CRef<TNode> child = children[i];
            bool kill = false;

            if(child->IsContainer()) {
                // process containers recursively
                x_Full_CleanLayoutTreeBranch(centralPane, *child);

                // now see if this child container needs to be reduced
                bool central_pane = (centralPane == child);
                if( ! central_pane) {
                    if(child->HasOnlyOneChild())    {
                        // replace "child" with its only child (node's grand child)
                        CRef<TNode> grand_child = child->GetTheOnlyChild();
                        child->RemoveChild(*grand_child);
                        full_node.ReplaceChild(*child, *grand_child);

                        _ASSERT(grand_child->GetParent() == &full_node);
                    } else {
                        // if child has no children - kill it
                        kill = child->GetChildren().empty();
                    }
                }
                // Sometimes splitters get Hidden flag set to TRUE
                // If splitter contains some views
                // it may result in NULL pointer access an application exit
                // JIRA: GB-1280
                if (child->IsSplitter())
                    child->SetHidden(false);
            }

            if(kill)    {
                full_node.RemoveChild(i);
            } else {
                i++; // advance
            }
        }
    }
}


// destroys the visible tree and disconnects the full tree from the container
CRef<CDockLayoutTree> CDockContainer::DisconnectTrees()
{
    CRef<CDockLayoutTree> tree(m_FullTree);

    m_FullTree.Reset();
    m_VisibleTree->DisconnectAllNodes();

    return tree;
}


bool CDockContainer::HasNoWMClients()
{
    if (m_FullTree) {
        for (auto i : m_WindowToNode) {
            if (dynamic_cast<CDockPanel*>(i.first))
                return false;
        }
    }
    return true;
}

// get all IWMCient-s in the container
void CDockContainer::GetAllClients(vector<IWMClient*>& clients)
{
    if (m_FullTree) {
        for (auto i : m_WindowToNode) {
            CDockPanel* panel = dynamic_cast<CDockPanel*>(i.first);
            if (panel) {
                IWMClient* client = panel->GetClient();
                _ASSERT(client);
                clients.push_back(client);
            }
        }
    }
}


// returns true if the given window is the root window of this Dock Container
bool CDockContainer::IsRootWindow(wxWindow* window)
{
    CRef<TNode> root = m_VisibleTree->GetRoot();
    if (root) {
        return root->GetWindow() == window;
    }
    return false;
}


// helper function; converts Dock Effect into dock position
wxDirection sGetDirectionByEffect(EDockEffect effect)
{
    switch(effect) {
    case eSplitLeft:
    case eSplitTargetLeft:
        return wxLEFT;

    case eSplitRight:
    case eSplitTargetRight:
        return wxRIGHT;

    case eSplitTop:
    case eSplitTargetTop:
        return wxTOP;

    case eSplitBottom:
    case eSplitTargetBottom:
        return wxBOTTOM;

    default:
        return wxALL;
    }
}


bool sRootSplit(EDockEffect effect)
{
    switch(effect) {
    case eSplitLeft:
    case eSplitRight:
    case eSplitTop:
    case eSplitBottom:
        return true;
    default:
        return false;
    }
}


// diagnostics helper - dumps and validates Visible and Full trees
// we call this function before and after complex operations
void CDockContainer::LogPostTrees(const string& text)
{
#ifdef WM_LOGGING
    LOG_POST(Info << "LogPostTrees: " << text);

    LOG_POST(Info << "Visible tree");
    m_VisibleTree->LogPost();

    if (m_VisibleTree->m_Root)
        m_VisibleTree->Validate(*m_VisibleTree->m_Root, true);

    LOG_POST(Info << "Full tree");
    m_FullTree->LogPost();

    LOG_POST(Info << "\n");

    if (m_FullTree->m_Root)
        m_FullTree->Validate(*m_FullTree->m_Root, false);
#endif
}


// returns CDockPanel for a window (CDockPanel or IWMClient)
// create a new CDockPanel if needed
CDockPanel* CDockContainer::x_GetDockPanel(IWMClient& client)
{
    wxWindow* window = client.GetWindow();
    _ASSERT(window);

    CDockPanel* panel = dynamic_cast<CDockPanel*>(window);
    if (panel == NULL) {
        // window is not the panel
        wxWindow* parent = window->GetParent();
        panel = dynamic_cast<CDockPanel*>(parent);

        if(panel == NULL)   {
            // create a new one
            wxString pname = wxT("panel_");
            IWMClient::CFingerprint fp = client.GetFingerprint();
            string s = fp.GetId();
            pname << ToWxString(s);
            panel = new CDockPanel(this, &GetDockManager(), &client, pname);
        }
    }
    return panel;
}


// returns an existing Dock Panel for the given client
CDockPanel* CDockContainer::x_DockPanelForClient(IWMClient& client)
{
    wxWindow* window = client.GetWindow();
    for (;window;) {
        wxWindow* parent = window->GetParent();
        CDockPanel* panel = dynamic_cast<CDockPanel*>(parent);
        if (panel)
            return panel;
        window = parent;
    }
    return 0;
}


// Functor returns true if a node (visible or not) has a fingerprint
class FNodeFingerprintEquals
{
public:
    FNodeFingerprintEquals(const IWMClient::CFingerprint& print)
        : m_Print(print)    {}

    bool operator()(CDockLayoutTree::CNode& node) {
        bool res = false;

        if (!node.IsVisible()) 
            res = (node.GetFingerprint() == m_Print);

        if (res) {
            m_Node.Reset(&node);
        }
        return res;
    }

    IWMClient::CFingerprint m_Print;
    CRef<CDockLayoutTree::CNode>    m_Node;
};


// Functor tests whether a node is visible
struct FFindVisibleNode
{
    bool operator()(CDockLayoutTree::CNode& node)
 {
        if (node.IsVisible()) {
            m_Node.Reset(&node);
            return true;
        }
        return false;
    }
    CRef<CDockLayoutTree::CNode> m_Node;
};


// adds a new client into a reasonable default location
void CDockContainer::AddClientToDefaultLocation(IWMClient& client)
{
    //Freeze();

    BlockSplittersLayout(true);

    CRef<TNode> full_layout_node;

    // find a node that has the same fingerprint as the client
    CRef<TNode> full_node;
    TFingerprint p = client.GetFingerprint();
    if( ! p.IsEmpty())    {
        FNodeFingerprintEquals f(p);
        m_FullTree->DepthFirstSearch(f);
        full_node = f.m_Node;
    }

    if (full_node) {
        LogPostTrees("CDockContainer::AddClientToDefaultLocation() start");

        // the node with matching fingerprint is hidden, we need
        // to modify the layout tree so that this hidden node becomes visible
        // this may require unhiding some of the hidden parent containers
        TNode* full_cont = x_AddClientToHiddenPos(client, full_node);
        full_layout_node.Reset(full_cont);

        Layout();
        LogPostTrees("CDockContainer::AddClientToDefaultLocation() end");
    } else {
        // no positional information - add to the central Tab pane of the main container

        CDockPanel* panel = x_GetDockPanel(client);
        CRef<TNode> full_client(new TNode(CDockLayoutTree::eClient, panel));
        full_client->Link(*new TNode(*full_client));

        full_client->SetFingerprint(p);
        full_client->GetClone()->SetFingerprint(p);

        AddClientToCentralPane(full_client);
    }

    BlockSplittersLayout(false);

    //Thaw();

    // tereshko: this is the only thing that works to properly
    // initialize some views inserted to an _empty_ container
    // Refresh and Focus is not working
#ifdef __WXMAC__
    wxWindow* client_w = client.GetWindow();
    client_w->Hide();
    client_w->Show();
#endif
}


CDockContainer::TNode*
    CDockContainer::x_AddClientToHiddenPos(IWMClient& client, CRef<TNode> full_node)
{
    // instantiate the client node
    CDockPanel* panel = x_GetDockPanel(client);
    full_node->SetWindow(panel);
    full_node->Link(*new TNode(*full_node));

    TNode* full_cont = full_node->GetParent();
    TNode* full_cont_child = full_node;

    // we are traversing parent nodes looking for a hidden container that should
    // become visible or for a visible container
    while(full_cont) {
        if (full_cont->IsHidden()) {
            FFindVisibleNode test;
            if (full_cont->DepthFirstSearch(test)) {
                WM_POST("Unhiding a hidden container with one child");
                // a container having a visible child can be hidden only if it
                // has exactly one visible child
                TNode& full_vis_child = *test.m_Node;
                x_Full_UnhideContainer_AddChildren(*full_cont, full_vis_child, *full_node);
                break;
            }
        } else {
            WM_POST("Add the client to a visible parent container");
            // found a visible container - add child to it
            x_InstantiateNode(*full_node, false);
            x_Visible_AddChildToContainer(*full_cont->GetClone(), *full_node->GetClone());
            break;
        }
        // move one step up in the tree
        full_cont_child = full_cont;
        full_cont = full_cont->GetParent();
    }

    if (full_cont == NULL) {
        WM_POST("we did not find an appropriate parent container");
        // we did not find an appropriate parent container
        // instantiate the topmost hidden container
        full_cont = full_cont_child; // tompost container
        CRef<TNode> old_root = m_VisibleTree->GetRoot();

        if (old_root) {
            WM_POST("Unhide root container");
            x_Full_UnhideContainer_AddChildren(*full_cont, *old_root->GetClone(), *full_node);
        } else {
            WM_POST("Create as central component");
            // create as central component
            x_InstantiateNode(*full_node, true);

            m_VisibleTree->m_Root.Reset(full_node->GetClone());
            x_SetRootWindow(m_VisibleTree->GetRoot()->GetWindow());
            panel->Show();
        }
    }
    return full_cont;
}

// add the given client node to Dock Manager's Main Tabbed Pane
void CDockContainer::AddClientToCentralPane(CRef<TNode> full_client)
{
    //WM_POST("\nCwxDockContainer::AddClientToCentralPane(TNode)");

    CRef<TNode> pane = m_VisibleTree->GetCentralPane();
    if (pane) {
        _ASSERT(pane->IsTab());
        x_InstantiateNode(*full_client, true);
        x_AddClientInTab(full_client, CRef<TNode>(pane->GetClone()));
        x_SetRootWindow(m_VisibleTree->GetRoot()->GetWindow());
        Layout();
    } else {
        x_InstantiateNode(*full_client, true);

        if(m_VisibleTree->m_Root)   {
            CRef<TNode> full_root(m_VisibleTree->m_Root->GetClone());
            x_AddClientInTab(full_client, full_root);

            wxWindow* new_root_window = m_VisibleTree->GetRoot()->GetWindow();
            x_SetRootWindow(new_root_window);

            Layout();
        } else {
            // there is no central pane - client becomes the Root Window of this Dock Container

            m_FullTree->m_Root.Reset(full_client);
            m_VisibleTree->m_Root.Reset(full_client->GetClone());

            x_SetRootWindow(m_VisibleTree->GetRoot()->GetWindow());
        }
    }
}

void CDockContainer::AddClient(CRef<TNode> full_client, EDockEffect effect, wxWindow* target_w)
{
    //WM_POST("\nCwxDockContainer::AddClient() " << kStartSep);
    //WM_POST("\t\teffect " << effect << ", target window " << target_w);
    LogPostTrees("CDockContainer::AddClient() start");

    BlockSplittersLayout(true);

    // find the node that is the target of the drop operation
    CRef<TNode> full_target(x_FindNodeByWindow(target_w));

    x_InstantiateNode(*full_client, true);

    if (effect == ePutInTab) {
        x_AddClientInTab(full_client, full_target);
    } else {
        x_AddClientInSplitter(full_client, full_target, effect);
    }

    wxWindow* new_root_window = m_VisibleTree->GetRoot()->GetWindow();
    x_SetRootWindow(new_root_window);

    Layout();
    BlockSplittersLayout(false);

    LogPostTrees("CDockContainer::AddClient() end");
    //WM_POST("CDockContainer::AddClient() END " << kEndSep);
}


// add the given client in a tab with the given target
// client node must be already instantiated (window created)
void CDockContainer::x_AddClientInTab(CRef<TNode> full_client, CRef<TNode> full_target)
{
    _ASSERT(full_client->IsClient());

    TNode* fullTab = 0;

    if (full_target->IsTab()) {
        fullTab = full_target;
    }
    else {
        // target is not a Tab Control
        CRef<TNode> full_parent(full_target->GetParent());
        if (full_parent) {
            if (full_parent->IsTab()) {
                if (full_parent->IsHidden()) {
                    CRef<TNode> tab(new TNode(*full_parent));
                    tab->Link(*full_parent);
                    x_InstantiateNode(*full_parent, false);

                    x_Visible_ReplaceChildWithContainer(*full_target->GetClone(), *tab);
                    x_Visible_AddChildToContainer(*tab, *full_target->GetClone());
                }
                fullTab = full_parent;
            }
            else {
                CRef<TNode> fullNewTab(new TNode(CDockLayoutTree::eTab, NULL, true));
                fullNewTab->Link(*new TNode(*fullNewTab));
                x_InstantiateNode(*fullNewTab, false);

                full_parent->ReplaceChild(*full_target, *fullNewTab);
                fullNewTab->AddChild(*full_target);

                x_Visible_ReplaceChildWithContainer(*full_target->GetClone(), *fullNewTab->GetClone());
                x_Visible_AddChildToContainer(*fullNewTab->GetClone(), *full_target->GetClone());
                fullTab = fullNewTab;
            }
        }
        else { // full_target is a single node/window in container
            CRef<TNode> fullNewTab(new TNode(CDockLayoutTree::eTab, NULL, true));
            fullNewTab->Link(*new TNode(*fullNewTab));
            x_InstantiateNode(*fullNewTab, false);

            m_FullTree->m_Root.Reset(fullNewTab);
            fullNewTab->AddChild(*full_target);

            x_Visible_ReplaceChildWithContainer(*full_target->GetClone(), *fullNewTab->GetClone());
            x_Visible_AddChildToContainer(*fullNewTab->GetClone(), *full_target->GetClone());
            fullTab = fullNewTab;
        }
    }

    _ASSERT(fullTab);
    fullTab->AddChild(*full_client);
    x_Visible_AddChildToContainer(*fullTab->GetClone(), *full_client->GetClone());
    _ASSERT(full_client->GetParent());
}

void  sGetSplitSizes(int total_size, vector<int>& sizes, bool split_left)
{
    int s1 = total_size / 3;
    int s2 = total_size - s1;
    sizes.push_back(split_left ? s1 : s2);
    sizes.push_back(split_left ? s2 : s1);
}

// add the given client to the target with the specified split effect
void CDockContainer::x_AddClientInSplitter(CRef<TNode> full_client,
                                             CRef<TNode> full_target,
                                             EDockEffect effect)
{
    // need to split either target or root container
    wxDirection dir = sGetDirectionByEffect(effect);

    // create a node for the new splitter
    CDockLayoutTree::ENodeType type = (dir == wxLEFT  ||  dir == wxRIGHT)   ?
        CDockLayoutTree::eVertSplitter : CDockLayoutTree::eHorzSplitter;

    // TODO - revise - we may want to reuse hidden splitters
    CRef<TNode> full_sp_node(new TNode(type, NULL, true));
    full_sp_node->Link(*new TNode(*full_sp_node));
    x_InstantiateNode(*full_sp_node, false);

    // insert container instead of child_1 (update data structure)
    bool reverse = (dir == wxLEFT) || (dir == wxTOP);
    CRef<TNode> full_node_to_split = full_target;

    if (sRootSplit(effect)) {
        full_node_to_split.Reset(m_VisibleTree->GetRoot()->GetClone());
    }

    CRef<TNode> full_parent(full_node_to_split->GetParent());

    // disconnect full_node_to_split
    if (full_parent) {
        full_parent->ReplaceChild(*full_node_to_split, *full_sp_node);
    }
    else {
        // full_child_1 does not have a parent - it is the Root, update the Root
        m_FullTree->m_Root.Reset(full_sp_node);
    }

    x_Visible_ReplaceChildWithContainer(*full_node_to_split->GetClone(), *full_sp_node->GetClone());

    // remember the size
    wxSize size = full_node_to_split->GetWindow()->GetSize();

    TNode& cont = *full_sp_node->GetClone();

    if (reverse) {
        full_sp_node->AddChild(*full_client);
        x_Visible_AddChildToContainer(cont, *full_client->GetClone());

        full_sp_node->AddChild(*full_node_to_split);
        x_Visible_AddChildToContainer(cont, *full_node_to_split->GetClone());
    }
    else {
        full_sp_node->AddChild(*full_node_to_split);
        x_Visible_AddChildToContainer(cont, *full_node_to_split->GetClone());

        full_sp_node->AddChild(*full_client);
        x_Visible_AddChildToContainer(cont, *full_client->GetClone());
    }

    // correct splitter sizes TODO - need a more elegant solution
    wxWindow* cont_w = cont.GetWindow();

    switch (cont.GetType()) {
    case  CDockLayoutTree::eHorzSplitter: {
        CDockSplitter* splitter = dynamic_cast<CDockSplitter*>(cont_w);
        vector<int> heights;
        sGetSplitSizes(size.y, heights, reverse);
        splitter->SetHeights(heights);
        break;
    }
    case  CDockLayoutTree::eVertSplitter: {
        CDockSplitter* splitter = dynamic_cast<CDockSplitter*>(cont_w);
        vector<int> widths;
        sGetSplitSizes(size.x, widths, reverse);
        splitter->SetWidths(widths);
        break;
    }
    default:
        break;
    }
}


CRef<CDockContainer::TNode>
    CDockContainer::RemoveWindow(IDockableWindow& dockable, ERemoveAction action)
{
    //WM_POST("\nCwxDockContainer::RemoveWindow(): " << window << kStartSep);
    LogPostTrees("CDockContainer::RemoveWindow() start");

    wxWindow* window = dynamic_cast<wxWindow*>(&dockable);

    //Freeze();

    CRef<TNode> full_node(x_FindNodeByWindow(window));

    _ASSERT(full_node); // window must be in this container

    if (full_node) {
        m_FullTree->SaveSplitterLayout();

        TNode* node = full_node->GetClone();
        CRef<TNode> parent_node = node->GetParent();
        CRef<TNode> full_parent_node(full_node->GetParent());

        x_Full_RemoveNode(*full_node, action);

        // check whether we need to drop the full_node's visible parent container
        bool central_pane = (m_VisibleTree->GetCentralPane() == parent_node);

        // we never destroy the Central Pane
        if ( ! central_pane  &&  parent_node) {
            if (parent_node->HasOnlyOneChild()) {
                x_Full_ReduceContainer(*parent_node->GetClone());
            }
        }

        // may need to destroy its immediate hidden container node
        if (full_parent_node) {
            x_Full_ReduceHiddenContainer_IfNeeded(*full_parent_node);
        }
    }

    //Thaw();

    Layout();

    //WM_POST("\nAfter RemoveWindow() "  << kEndSep);
    LogPostTrees("CDockContainer::RemoveWindow() end");

    return full_node;
}


bool CDockContainer::HasDefaultPositionFor(const TFingerprint& fingerprint)
{
    FNodeFingerprintEquals f(fingerprint);
    bool ok = m_FullTree->DepthFirstSearch(f);
    return ok;
}


//// Functor returns true if a node (visible or not) has a fingerprint
class FNodeHasFingerprint
{
public:
    bool operator() (CDockLayoutTree::CNode& node)
    {
        if (node.IsVisible()) {
            CDockPanel* panel = dynamic_cast<CDockPanel*>(node.GetWindow());
            if (panel) {
                IWMClient* client = panel->GetClient();
                IWMClient::CFingerprint p = client->GetFingerprint();
                return  ! p.IsEmpty();
            }
        } else {
            return ! node.GetFingerprint().IsEmpty();
        }
        return false;
    }
};


// save positions if there are windows or hidden nodes with fingerprints
bool CDockContainer::NeedToSavePositions()
{
    if (m_DockManager.GetMainContainer() == this)
        return true;

    FNodeHasFingerprint f;
    bool ok = m_FullTree->DepthFirstSearch(f);
    return ok;
}


class FBlockSplitterLayout
{
public:
    FBlockSplitterLayout(bool block) : m_Block(block)   {}

    void operator()(CDockLayoutTree::CNode& node) {
        if (node.IsSplitter()) {
            CDockSplitter* splitter = dynamic_cast<CDockSplitter*>(node.GetWindow());
            splitter->BlockLayout(m_Block);
        }
    }
protected:
    bool m_Block;
};


/// Iterates through all splitters and blocks / unblocks layout.
void CDockContainer::BlockSplittersLayout(bool block)
{
    CRef<TNode> root = m_VisibleTree->GetRoot();
    if (root) {
        FBlockSplitterLayout blocker(block);
        root->DepthFirstForEach(blocker);
    }
}


///////////////////////////////////////////////////////////////////////////////
// Functions operating on the Full Tree (update visible tree and windows as well)
// Functions starting from Full_ update the full tree and then visible tree and
// windows to bring them in accordance with the full tree.

void sRestoreSplitterSizes(CDockLayoutTree::CNode& full_node)
{
    wxWindow* win = full_node.GetWindow();

    switch(full_node.GetType()) {
    case CDockLayoutTree::eHorzSplitter: {
        CDockSplitter* splitter = dynamic_cast<CDockSplitter*>(win);
        splitter->SetHeights(full_node.GetSplitterSizes());
        break;
    }
    case CDockLayoutTree::eVertSplitter: {
        CDockSplitter* splitter = dynamic_cast<CDockSplitter*>(win);
        splitter->SetWidths(full_node.GetSplitterSizes());
        break;
    }
    default:
        break;
    }
}

// Instantiates the given hidden container.
// Replaces vis_child (already visible) with the container and adds both
// visible and previously hidden child to the container.
// The children may be separated from the container by several hidden container nodes
// in the full tree.
void CDockContainer::x_Full_UnhideContainer_AddChildren(TNode& full_cont,
                                                          TNode& full_vis_child,
                                                          TNode& full_new_child)
{
    TNode& vis_child = *full_vis_child.GetClone();

    // create a container node and window, but do not add children
    CRef<TNode> cont(new TNode(full_cont));
    full_cont.Link(*cont);
    x_InstantiateNode(full_cont, false, fCreateWindow); // only create the container window

    // add child and client in the right order
    int vis_index = full_cont.GetBranchIndex(full_vis_child);
    int new_index = full_cont.GetBranchIndex(full_new_child);
    _ASSERT(vis_index >= 0  &&  new_index >= 0); // must be related

    // replace visible child with the container
    x_Visible_ReplaceChildWithContainer(vis_child, *cont);

    // instantiate new child node
    x_InstantiateNode(full_new_child, false);
    TNode& new_child = *full_new_child.GetClone();

    // establish parent-child relations in visible tree
    cont->RemoveAllChildren();
    if (vis_index < new_index) {
        cont->AddChild(vis_child);
        cont->AddChild(new_child);
    } else {
        cont->AddChild(new_child);
        cont->AddChild(vis_child);
    }

    // insert child windows into the container window
    x_InstantiateNode(full_cont, false, fAddChildWindows);

    sRestoreSplitterSizes(full_cont);
}


/// Disconnect all child nodes from the given node in all 3 trees (Full Tree,
/// Visible Tree and Window Tree).
/// The function does not destroy the children.
void CDockContainer::x_Full_DisconnectChildren(TNode& full_node)
{
    wxWindow* node_w = full_node.GetWindow();
    IDockContWindow* cont = dynamic_cast<IDockContWindow*>(node_w);

    // disconnect windows
    TNodeVector& children = full_node.GetChildren();
    for( size_t i = 0;  i < children.size(); i++) {
        TNode& full_child = *children[i];
        wxWindow* w = full_child.GetWindow();
        cont->Cont_Remove(w);
    }

    // disconnect nodes
    full_node.RemoveAllChildren();
    full_node.GetClone()->RemoveAllChildren();
}


// removes a node from all 3 trees
void CDockContainer::x_Full_RemoveNode(TNode& full_node, ERemoveAction action)
{
    //WM_POST("CDockContainer::x_Full_RemoveNode()");
    TFingerprint fingerprint;
    if (action != eMoveWin) {
        CDockPanel* panel = dynamic_cast<CDockPanel*>(full_node.GetWindow());
        _ASSERT(panel);

        IWMClient* client = panel->GetClient();
        fingerprint = client->GetFingerprint();
    }

    // remove visible node and destroy windows
    x_Visible_RemoveNode(*full_node.GetClone(), action);

    if (action != eMoveWin) {
        full_node.Unlink();
    }

    bool keep_node = false;

    if(action == eMinimizeWin)  {
        keep_node = true; // window is being minimized - preserve layout nodes
    } else {
        // windows is being moved or deleted

        if ( ! fingerprint.IsEmpty()) {
            keep_node = true;
        }
    }

    if (keep_node) {
        // keep the node in the Full tree as hidden to preserve this position
        // as the default position for clients with the same fingerprint
        full_node.SetFingerprint(fingerprint);
    } else {
        CRef<TNode> full_parent = full_node.GetParent();
        if (full_parent) {
            full_parent->RemoveChild(full_node);
        } else {
            // full_node is the root
            _ASSERT(m_FullTree->m_Root.GetPointer() == &full_node);
            m_FullTree->m_Root.Reset();
        }
    }
}


// replaces the given container with it's only child (lifts the child to the container's place)
// if the child became hidden - hide the container
// if the child was destroyed - destroy the container
void CDockContainer::x_Full_ReduceContainer(TNode& full_cont)
{
    //WM_POST("x_Full_ReduceContainer()...");

    _ASSERT(full_cont.IsVisible()); // precondition

    TNode& cont = *full_cont.GetClone();
    CRef<TNode> the_child = cont.GetTheOnlyChild();

    // modify visible tree
    x_Visible_ReplaceContainerWithChild(cont);

    full_cont.Unlink(); // unlink from visible node

    // update full tree
    if (full_cont.HasOnlyOneChild()) {
        // destroy the container as it has only 1 child in both trees
        CRef<TNode> full_child = full_cont.GetTheOnlyChild();
        full_cont.RemoveChild(*full_child);

        CRef<TNode> full_parent = full_cont.GetParent(); // the parent of this container
        if (full_parent) {
            // node's parent now should point to the node's only child
            full_parent->ReplaceChild(full_cont, *full_child);
            // child becomes connected to its previous grandparent
            _ASSERT(full_child->GetParent() == full_parent);
        }  else {
            _ASSERT(full_child->GetParent() == NULL); // child becomes the root
            m_FullTree->m_Root.Reset(full_child);
        }
    } else {
        // hide the container, but let it keep its children
        _ASSERT(full_cont.IsHidden());
    }
    // container must be disconnected or invisible now
    _ASSERT(full_cont.IsHidden()  ||  full_cont.GetParent() == NULL);
}



// remove a hidden container node from full tree only
void CDockContainer::x_Full_ReduceHiddenContainer_IfNeeded(TNode& full_cont)
{
    //WM_POST("x_Full_ReduceHiddenContainer_IfNeeded()...");

    // preconditions - container is hidden and has exactly one hidden child
    if (full_cont.IsHidden()  &&  full_cont.HasOnlyOneChild()) {
        CRef<TNode> full_child = full_cont.GetTheOnlyChild();

        if (full_child->IsHidden()  &&  full_cont.GetClone() == NULL) {
            // destroy the container as it has only 1 child in both trees
            CRef<TNode> full_parent = full_cont.GetParent();// the parent of this container
            full_cont.RemoveChild(*full_child);

            if (full_parent) {
                // node's parent now should point to the node's only child
                full_parent->ReplaceChild(full_cont, *full_child);
                // child becomes connects to its previous grandparent
                _ASSERT(full_child->GetParent() == full_parent);
            }  else {
                _ASSERT(full_child->GetParent() == NULL  && 
                        m_FullTree->m_Root.GetPointerOrNull() == &full_cont);
                // child becomes the root
                m_FullTree->m_Root.Reset(full_child);
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
/// Functions updating visible tree and windows based on the full tree.
/// Functions names start from Visible_.


void CDockContainer::x_Visible_RemoveNode(TNode& node, ERemoveAction action)
{
    CRef<TNode> parent = node.GetParent();

    if (parent) {
        // remove the node from the visible tree
        parent->RemoveChild(node);

        // disconnect from the parent container window
        wxWindow* win = node.GetWindow();
        wxWindow* parent_w = parent->GetWindow();

        IDockContWindow* parent_cont = dynamic_cast<IDockContWindow*>(parent_w);
        parent_cont->Cont_Remove(win);
    } else {
        _ASSERT(m_VisibleTree->m_Root.GetPointer() == &node);
        wxWindow* win = node.GetWindow();
        GetSizer()->Detach(win);
        m_VisibleTree->m_Root.Reset();
    }

    TNode& full_node = *node.GetClone();
    x_DestroyNode(full_node, action);
}

// Replaces the child node with the given container in visible tree in window hierarchy
// the child node becomes disconnected
void CDockContainer::x_Visible_ReplaceChildWithContainer(TNode& child, TNode& cont)
{
    wxWindow* child_w = child.GetWindow();
    wxWindow* cont_w = cont.GetWindow();

    _ASSERT(cont_w  &&  child_w); // both windows must exist

    CRef<TNode> parent = child.GetParent();

    if (parent) {
        wxWindow* parent_w = parent->GetWindow();
        /// remove the child window from the parent and replace it with container window
        IDockContWindow* parent_cont = dynamic_cast<IDockContWindow*>(parent_w);
        parent_cont->Cont_Replace(child_w, cont_w);

        // update visible tree - replace child the node with the container node
        parent->ReplaceChild(child, cont);

        // container now must be a child of the "parent" node
        _ASSERT(cont.GetParent() == parent.GetPointer());
    } else {
        // no - parent, child node is the root, update the root
        x_SetRootWindow(cont_w); // TODO move up
        m_VisibleTree->m_Root.Reset(&cont);
    }

    _ASSERT(child.GetParent() == NULL); // child node becomes diconnected
}


void sDockPanel_ShowBorder(wxWindow* w, bool show)
{
    CDockPanel* panel = dynamic_cast<CDockPanel*>(w);
    if(panel)   {
        panel->ShowBorder(show);
    }
}


/// Adds a child to a visible node. This function establishes parent-child
/// connection between visible nodes and betwee the windows corresponding to
/// the nodes.
void CDockContainer::x_Visible_AddChildToContainer(TNode& cont, TNode& child)
{
    _ASSERT(child.GetWindow()  &&  cont.IsContainer());

    wxWindow* cont_w = cont.GetWindow();
    wxWindow* child_w = child.GetWindow();

    // enable / disable border rendering in the panel depending on the container type
    sDockPanel_ShowBorder(child_w, cont.IsSplitter());

    switch(cont.GetType()) {
    case CDockLayoutTree::eHorzSplitter: {
        CDockSplitter* splitter = dynamic_cast<CDockSplitter*>(cont_w);
        size_t child_count = splitter->GetChildren().GetCount();
        if (child_count > 0) {
            splitter->AddRow();
        }
        int row = splitter->GetRowsCount() - 1;
        splitter->InsertToCell(child_w, 0, row);
        if (x_IsElastic(child)) {
            splitter->SetResizableCell(-1, row);
        }
        break;
    }
    case CDockLayoutTree::eVertSplitter: {
        CDockSplitter* splitter = dynamic_cast<CDockSplitter*>(cont_w);
        size_t child_count = splitter->GetChildren().GetCount();
        if (child_count > 0) {
            splitter->AddColumn();
        }
        int col = splitter->GetColumnsCount() - 1;
        splitter->InsertToCell(child_w, col, 0);
        if (x_IsElastic(child)) {
            splitter->SetResizableCell(col, -1);
        }
        break;
    }
    case CDockLayoutTree::eTab: {
        CDockNotebook* notebook = dynamic_cast<CDockNotebook*>(cont_w);
        string label = x_GetPageNameByWindow(child_w);
        notebook->InsertPageAtHitPoint(child_w, ToWxString(label));
        child_w->Reparent(notebook);

        size_t index = notebook->GetPageIndex(child_w);
        notebook->SetSelection(index);

        notebook->MakeSelectionVisible();

        break;
    }
    default:
        _ASSERT(false); // invalid container type
    }

    // establish parent-child connections between the nodes
    cont.AddChild(child);
}


// Replaces a container having exactly one child with the child;
// destroys the container
void CDockContainer::x_Visible_ReplaceContainerWithChild(TNode& cont)
{
    // can lift only containers having a single child
    CRef<TNode> child = cont.GetTheOnlyChild();
    CRef<TNode> parent = cont.GetParent();

    // update window connections
    wxWindow* child_w = child->GetWindow();
    wxWindow* cont_w = cont.GetWindow();
    cont_w->Hide();

    // disconnect child from cont in Visible and Window Trees
    cont.RemoveChild(*child);
    IDockContWindow* cont_dw = dynamic_cast<IDockContWindow*>(cont_w);
    cont_dw->Cont_Remove(child_w);

    /// remove cont_w window from Window Manager
    if (parent) {
        // node's parent now should point to the node's only child
        parent->ReplaceChild(cont, *child);

        // in the parent window replace container window with the child window
        IDockContWindow* parent_dw = dynamic_cast<IDockContWindow*>(parent->GetWindow());
        parent_dw->Cont_Replace(cont_w, child_w);

        // child becomes connect to its previous grandparent
        _ASSERT(child->GetParent() == parent);
    }  else {
        _ASSERT(child->GetParent() == NULL); // child becomes the root
        // this is a root node - update m_VisibleTree root

        m_VisibleTree->m_Root = child;
        x_SetRootWindow(child->GetWindow());
    }

    // now we can safely destroy the container node and window
    x_DestroyNode(cont, eDestroyWin);
}


/// Ensures that a node has a window and this window is properly connected with
/// its child and parent windows. This is a potentially recursive function
/// (controlled by "recursive" parameter).
/// For a container node it will create a window if needed.
void CDockContainer::x_InstantiateNode(TNode& full_node, bool recursive, EInstFlags flags)
{
    wxWindow* window = full_node.GetWindow();
    bool create = (flags & fCreateWindow) != 0;
    bool add_children = (flags == fAddChildWindows) ||
        ( (flags == (fCreateWindow | fAddChildWindows))  &&  window == NULL);

    if (full_node.IsContainer()) {
        // this is a Container window - inspect child nodes
        TNode& node = *full_node.GetClone();
        for( size_t i = 0;  i < node.GetChildren().size();  i++ ) {
            TNode& child_node = *node.GetChildren()[i];
            if (recursive) {
                // instantiate nodes for the child windows
                x_InstantiateNode(*child_node.GetClone(), recursive, flags);
            }
            // otherwise expect child window to exist
        }

        // create a window for "full_node" if needed
        if ( ! window  &&  create) {
            window = x_CreateContainerWindow(full_node.GetType());
            window->Hide();
        }
    }

    _ASSERT(window);

    if (create  &&  window) {
        TNode& node = *full_node.GetClone();
        full_node.SetWindow(window);
        node.SetWindow(window);

        // mark nodes as visble
        full_node.SetHidden(false);
        node.SetHidden(false);

        IDockableWindow* dockable = dynamic_cast<IDockableWindow*>(window);
        if (dockable) {
            dockable->SetDockContainer(this);
        }

        // update indexes
        m_WindowToNode[window] = &full_node;
    }

    /// optionally add child windows to this node's window
    if (add_children) {
        x_InstantiateNode_AddChildWindows(full_node);
    }
    _ASSERT(window  &&  full_node.GetClone());
}


wxWindow* CDockContainer::x_CreateContainerWindow(CDockLayoutTree::ENodeType type)
{
    switch(type) {
    case CDockLayoutTree::eHorzSplitter: {
        CDockSplitter* splitter = new CDockSplitter(*this, CDockSplitter::eHorizontal);
        splitter->BlockLayout(true);
        return splitter;
    }
    case CDockLayoutTree::eVertSplitter:{
        CDockSplitter* splitter = new CDockSplitter(*this, CDockSplitter::eVertical);
        splitter->BlockLayout(true);
        return splitter;
    }
    case CDockLayoutTree::eTab: {
        CDockNotebook* notebook = new CDockNotebook(*this);
        return notebook;
    }
    default:
        _ASSERT(false);
        return NULL;
    }
}


/// Iterates child windows of the given container node and inserts them into
/// the container window.
void CDockContainer::x_InstantiateNode_AddChildWindows(TNode& full_node)
{
    if (full_node.IsContainer()) {
        // accumulate windows for child nodes of the visible container node (real visible children)
        TNode& node = *full_node.GetClone();
        vector<wxWindow*> child_windows;
        for( size_t i = 0;  i < node.GetChildren().size();  i++ ) {
            TNode& child_node = *node.GetChildren()[i];
            child_windows.push_back(child_node.GetWindow());
        }
        size_t n = child_windows.size();

        // get the container window
        wxWindow* window = full_node.GetWindow();
        _ASSERT(window);

        // add child windows to the container window
        switch(node.GetType()) {
        case CDockLayoutTree::eHorzSplitter: {
            CDockSplitter* splitter = dynamic_cast<CDockSplitter*>(window);
            splitter->Split(0, (int)n);
            for( size_t i = 0;  i < n; i++ ) {
                wxWindow* child_w = child_windows[i];

                sDockPanel_ShowBorder(child_w, true);
                splitter->InsertToCell(child_w, 0, (int)i);
                if (x_IsElastic(child_w)) {
                    splitter->SetResizableCell(-1, (int)i);
                }
            }
            break;
        }
        case CDockLayoutTree::eVertSplitter:{
            CDockSplitter* splitter = dynamic_cast<CDockSplitter*>(window);
            splitter->Split((int)n, 0);
            for( size_t i = 0;  i < n; i++ ) {
                wxWindow* child_w = child_windows[i];

                sDockPanel_ShowBorder(child_w, true);
                splitter->InsertToCell(child_w, (int)i, 0);
                if (x_IsElastic(child_w)) {
                    splitter->SetResizableCell((int)i, -1);
                }
            }
            break;
        }
        case CDockLayoutTree::eTab: {
            CDockNotebook * notebook = dynamic_cast<CDockNotebook*>(window);
            for( size_t i = 0;  i < n; i++ ) {
                wxWindow* child_w = child_windows[i];

                sDockPanel_ShowBorder(child_w, false);

                child_w->Reparent(notebook);
                string label = x_GetPageNameByWindow(child_w);
                notebook->AddPage(child_w, ToWxString(label));

                int index = notebook->GetPageIndex(child_w);
                notebook->SetSelection(index);
            }
            break;
        }
        default:
            _ASSERT(false);
        }
    }
}


// disconnects node window from the node and destroys the window
// removes window for layout tree maps
void CDockContainer::x_DestroyNode(TNode& full_node, ERemoveAction action)
{
    wxWindow* win = full_node.GetWindow();
    _ASSERT(win);

    IWMClient* client = 0;
    for (wxWindow* w = win; w; w = w->GetParent()) {
        CDockPanel* panel = dynamic_cast<CDockPanel*>(w);
        if (panel) {
            client = panel->GetClient();
            break;
        }
    }

    if (client) {
        TFingerprint p = client->GetFingerprint();
        full_node.SetFingerprint(p);
    }

    IDockableWindow* dockable = dynamic_cast<IDockableWindow*>(win);
    if (dockable) {
        dockable->SetDockContainer(NULL);
    }

    if (full_node.IsContainer()) {
        _ASSERT(action != eMinimizeWin); // cannot minimize containers

        vector<wxWindow*> child_windows;
        for( size_t i = 0;  i < full_node.GetChildren().size();  i++ ) {
            TNode& full_child_node = *full_node.GetChildren()[i];
            if (full_child_node.IsVisible()) {
                x_DestroyNode(full_child_node, action);
            }
        }

        if (action == eDestroyWin) {
            //WM_POST("Destroying container window");
            win->Destroy();
        }
    } else {
        if (action == eDestroyWin) {
            CDockPanel* panel = dynamic_cast<CDockPanel*>(win);
            panel->RemoveClient(this);
            panel->Destroy();
        }
    }
    _ASSERT(full_node.GetClone());

    TNode& node = *full_node.GetClone();

    if (action != eMoveWin) {
        // mark nodes as invisible
        full_node.SetHidden(true);
        node.SetHidden(true);

        // disconnect nodes from the window
        full_node.SetWindow(NULL);
        node.SetWindow(NULL);
    }

    // remove window from tree index
    m_WindowToNode.erase(win);
}


// returns true if the given node belongs to the resizable branch
// (the branch of the visible tree containing the central tab control)
bool CDockContainer::x_IsElastic(TNode& node)
{
    TNode* curr_node = m_VisibleTree->GetCentralPane().GetPointer();
    while(curr_node) {
        if (curr_node == &node)
            return true;
        curr_node = curr_node->GetParent();
    }
    return false;
}


bool CDockContainer::x_IsElastic(wxWindow* win)
{
    TNode* curr_node = m_VisibleTree->GetCentralPane().GetPointer();
    if (curr_node) {
        wxWindow* curr_w = curr_node->GetWindow();
        while(curr_w) {
            if (curr_w == win)
                return true;
            curr_w = curr_w->GetParent();
        }
    }
    return false;
}

string CDockContainer::x_GetPageNameByWindow(wxWindow* window)
{
    if (window) {
        CDockPanel* panel = dynamic_cast<CDockPanel*>(window);
        if (panel) {
            return panel->GetClient()->GetClientLabel();
        }
        CDockSplitter* splitter = dynamic_cast<CDockSplitter*>(window);
        if (splitter) {
            return "Splitter";
        }
    }
    return "";
}


// get all IWMCient-s in a window hosted in the container
void CDockContainer::GetClientsInWindow(wxWindow* window, vector<IWMClient*>& clients)
{
    TNode* node = x_FindNodeByWindow(window);
    _ASSERT(node);
    if (node) {
        x_GetClientsInNode(*node, clients);
    }
}


// makes the given clients visible (if possible),
// if a client does not belong to the container it is simply ignored
void CDockContainer::ActivateClients(vector<IWMClient*>& clients)
{
    _ASSERT(m_VisibleTree); // the operation is meaningless otherwise

    // we can active only one tab in each Notebook, here we keep the list of
    // visited notebooks
    set<CDockNotebook*> visited;

    for( size_t i = 0; i < clients.size(); i++ ) {
        IWMClient* client = clients[i];
        _ASSERT(client);

        // get window and Node corresponding to the Client
        wxWindow* window = x_DockPanelForClient(*client);
        _ASSERT(window);

        if (x_FindNodeByWindow(window)) {
            // this is our client - iterate client's parents and activate tabs
            wxWindow* parent = window->GetParent();
            while(parent  &&  parent != this) {
                CDockNotebook* notebook = dynamic_cast<CDockNotebook*>(parent);

                if (notebook  &&  visited.find(notebook) == visited.end()) {
                    // activate the page
                    int index = notebook->GetPageIndex(window);
                    notebook->SetSelection(index);
                    visited.insert(notebook);
                }

                window = parent;
                parent = parent->GetParent();
            }
        }
    }
}


// get all IWMCient-s in a Node hosted in the container
void CDockContainer::x_GetClientsInNode(TNode& node, vector<IWMClient*>& clients)
{
    CDockPanel* panel = dynamic_cast<CDockPanel*>(node.GetWindow());
    if (panel) {
        IWMClient* client = panel->GetClient();
        _ASSERT(client);
        clients.push_back(client);
    } else if (node.IsContainer()) {
        // process child nodes recursively
        for( size_t i = 0;  i < node.GetChildren().size();  i++ ) {
            TNode& child_node = *node.GetChildren()[i];
            x_GetClientsInNode(child_node, clients);
        }
    }
}


// this function is called by Dock Manager during D&D docking session
// when mouse pointer enters the container
void CDockContainer::OnBeginDragOver(const wxPoint& sc_mouse_pos)
{
    //WM_POST("\nCwxDockContainer::OnBeginDragOver()  pos - " << sc_mouse_pos.x << ", " << sc_mouse_pos.y);

    wxWindow* parent = m_DockManager.GetTopAppWindow();

    m_MarkersWindow = new CDockMarkerWindow(*this, parent);

    m_HintWindow = CreateDockHintFrame(*parent, GetDockManager());
    m_DragTarget = NULL;

    OnDragOver(sc_mouse_pos);
}


// this function is called by Dock Manager during D&D docking session when
// mouse pointer moves inside the container
void CDockContainer::OnDragOver(const wxPoint& sc_mouse_pos)
{
    //WM_POST("CDockContainer::OnDragOver()  pos - " << sc_mouse_pos.x << ", " << sc_mouse_pos.y);

    wxMouseState ms_state = wxGetMouseState();
    bool split_tab_panes = ms_state.ShiftDown();

    // perform hit testing
    wxWindow* old_target = m_DragTarget;
    m_Effect = x_HitTest(sc_mouse_pos, m_DragTarget, split_tab_panes);

    if (m_DragTarget != old_target) {
        wxRect root_rc = ncbi::GetScreenRect(*m_RootWindow);
        wxRect target_rc(0, 0, 0, 0);
        if (m_DragTarget) {
            target_rc = ncbi::GetScreenRect(*m_DragTarget);
        }

        m_MarkersWindow->SetTargetRect(root_rc, target_rc);
    }

    EDockEffect marker_effect = m_MarkersWindow->HitTest(sc_mouse_pos);
    //WM_POST("m_MarkersWindow->HitTest(sc_mouse_pos) - effect " << marker_effect);
        
    if (marker_effect != eNoEffect) {
        m_Effect = marker_effect; // marker_effect overrides the target effect
    }

    // Update hint window
    if (m_Effect != eNoEffect) {
        // hint visible, may be requires updates
        wxRect rc_hint = x_CalculateHintRect();

        //WM_POST(" m_HintWindow->SetSize(rc_hint)   x "  << rc_hint.x << " y " << rc_hint.y);
        m_HintWindow->SetSize(rc_hint);

        if ( ! m_HintWindow->IsShown()) {
            m_HintWindow->Raise();
            m_HintWindow->Show();
        }
    } else {
        if(m_HintWindow->IsShown()) {
            m_HintWindow->Hide();
        }
    }
    //WM_POST("CDockContainer::OnDrag()  pos - " << sc_mouse_pos.x << ", " << sc_mouse_pos.y
    //         << "  m_Effect " << m_Effect);
}

// This function is called by Dock Manager during D&D docking session when
// mouse pointer leaves the container or if sessions ends (drop or cancel)
// The function returns Dock Effect and the target window that should be
// modified (drop target).
EDockEffect CDockContainer::OnEndDragOver(wxWindow*& target)
{
    //WM_POST("CDockContainer::OnEndDragOver()");

    EDockEffect effect = m_Effect;
    target = m_DragTarget;

    m_Effect = eNoEffect;
    m_DragTarget = NULL;

    m_MarkersWindow->Close();
    m_MarkersWindow = NULL;

    m_HintWindow->Close();
    m_HintWindow = NULL;

    return effect;
}


bool CDockContainer::IsHintShown() const
{
    return m_HintWindow  &&   m_HintWindow->IsShown();
}


void CDockContainer::LogPost()
{
    //WM_POST("CDockContainer #" << m_Number << " (total " << m_Count << ")");
    //wxRect rc = ncbi::GetScreenRect(*this);
    //WM_POST("    rect " << ToString(rc));
}


IDockDropTarget* CDockContainer::x_FindDockDropTargetByPos(const wxPoint& screen_pt)
{
    // traverse children recusrsively, find one that contains the point
    // and can perform hit testing
    IDockDropTarget* top_dock = NULL;

    for(  wxWindow* curr_w = m_RootWindow;  curr_w;  )  {
        IDockDropTarget* dock = dynamic_cast<IDockDropTarget*>(curr_w);
        if (dock) {
            top_dock = dock;
        }
        wxPoint pt = curr_w->ScreenToClient(screen_pt);

        // find a child window containing the mouse position
        wxWindowList& children = curr_w->GetChildren();
        wxWindow* child_w = NULL;
        for( wxWindowList::iterator it = children.begin(); it != children.end();  ++it ) {
            wxWindow* w = *it;
            if (w->IsShown()) {
                wxRect rc = w->GetRect();
                if (rc.Contains(pt)) {
                    child_w = w;
                    break;
                }
            }
        }
        curr_w = child_w; // continue with child window
    }
    return top_dock;
}


EDockEffect CDockContainer::x_HitTest(const wxPoint& screen_pt,
                                        wxWindow*& target,
                                        bool split_tab_pane)
{
    // the topmost in Z-order dock client that contains the given point
    IDockDropTarget* top_dock = x_FindDockDropTargetByPos(screen_pt);

    EDockEffect effect = eNoEffect;
    target = NULL;

    if (top_dock) {
        // perform hit testing in the found Dock Drop Target
        wxWindow* top_dock_w = dynamic_cast<wxWindow*>(top_dock);
        effect = top_dock->DropTest(screen_pt, target);

        if(effect != ePutInTab  &&  ! split_tab_pane)    {
            // do not split tabbed panes - split the tab control instead
            wxWindow* parent_w = top_dock_w->GetParent();

            if(parent_w  &&  dynamic_cast<CDockNotebook*>(parent_w))  {
                top_dock = dynamic_cast<IDockDropTarget*>(parent_w);
                top_dock_w = dynamic_cast<wxWindow*>(top_dock);

                effect = top_dock->DropTest(screen_pt, target);
                if(effect == ePutInTab) {
                    effect = eNoEffect;
                }
            }
        }

        target = top_dock_w;
    }
    //WM_POST("CDockContainer::x_HitTest() target " << target << "  " << typeid(*target).name());
    return effect;
}


// For a given Dock Effect calculates a rectangle for Hint Window (screen coords)
wxRect CDockContainer::x_CalculateHintRect()
{
    wxRect root_rc = ncbi::GetScreenRect(*m_RootWindow);
    wxRect rc(root_rc);

    switch(m_Effect) {
    case eSplitLeft: {
        rc.width = rc.width / 3;
        break;
    }
    case eSplitRight: {
        rc.width = rc.width / 3;
        rc.x = root_rc.GetRight() - rc.width;
        break;
    }
    case eSplitTop: {
        rc.height = rc.height / 3;
        break;
    }
    case eSplitBottom: {
        rc.height = rc.height / 3;
        rc.y = root_rc.GetBottom() - rc.height;
        break;
    }
    default: {
        if (m_DragTarget) {
            wxRect target_rc = ncbi::GetScreenRect(*m_DragTarget);
            rc = target_rc;

            switch(m_Effect) {
            case eSplitTargetLeft: {
                rc.width = rc.width / 3;
                break;
            }
            case eSplitTargetRight: {
                rc.width = rc.width / 3;
                rc.x = target_rc.GetRight() - rc.width;
                break;
            }
            case eSplitTargetTop: {
                rc.height = rc.height / 3;
                break;
            }
            case eSplitTargetBottom: {
                rc.height = rc.height / 3;
                rc.y = target_rc.GetBottom() - rc.height;
                break;
            }
            case ePutInTab: {
                // if this is a tab - select the whole Tab Control
                CDockNotebook* notebook = dynamic_cast<CDockNotebook*>(m_DragTarget->GetParent());
                if (notebook)
                    rc = ncbi::GetScreenRect(*notebook);
                break;
            }
            default:
                rc.SetSize(wxSize(-1, -1));
            }
        } else {
            rc.SetSize(wxSize(-1, -1));
        }
    }
    }

    //WM_POST("CDockContainer::x_CalculateHintRect()  effect " << effect << "   rect > " << ToString(rc));
    return rc;
}


void CDockContainer::OnTabClosePressed(wxWindow* page)
{
    vector<IWMClient*> clients;
    GetClientsInWindow(page, clients);

    _ASSERT(clients.size());

    m_DockManager.OnTabClosePressed(clients);
}


void CDockContainer::OnTabBeginDragOutside(CBeginDragOutsideEvent& event)
{
    //WM_POST("CDockContainer::OnTabBeginDragOutside()");

    // now we will be handling mouse events instead of the Tab Control
    m_TabDockable = event.GetWindow();

    //WM_POST("CDockContainer - capturing mouse...");
    CaptureMouse();

    wxPoint sc_mouse_pos = ::wxGetMousePosition();
    m_DockManager.OnBeginDrag(*m_TabDockable, sc_mouse_pos);
}


// order of tab pages has changed - rearrange nodes in layout trees
// according to the new order
void CDockContainer::OnUpdateTabOrder(CDockNotebook& notebook)
{
    TNode* node = x_FindNodeByWindow(&notebook);
    _ASSERT(node);

    if (node) {
        vector<wxWindow*> pages;
        for( size_t i = 0; i < notebook.GetPageCount();  i++ ) {
            pages.push_back(notebook.GetPage(i));
        }
        // update both Visible and Full trees
        node->EnforceChildOrder(pages);
        node->GetClone()->EnforceChildOrder(pages);
    }
}


///////////////////////////////////////////////////////////////////////////////
/// Mouse handling functions handle mouse events in case when a tab is dragged
/// from a tab control and the component that originaly handled mouse input is
/// destroyed as a result. In this case Dock Container captures mouse input
/// and relays events to Dock Manager to support the D&D session.

void CDockContainer::OnLeftDown(wxMouseEvent& event)
{
    //WM_POST("CDockContainer::OnLeftDown");
    event.Skip();
}


void CDockContainer::OnLeftUp(wxMouseEvent& event)
{
    //WM_POST("CDockContainer::OnLeftUp");

    if (HasCapture()) {
        //WM_POST("CDockContainer - releasing mouse...");
        ReleaseMouse();
    }

    if (m_TabDockable) {
        wxPoint sc_mouse_pos = ::wxGetMousePosition();

        m_DockManager.OnEndDrag(*m_TabDockable, CDockManager::eDrop, sc_mouse_pos);
    }
}


void CDockContainer::OnMouseMove(wxMouseEvent& event)
{
    //WM_POST("CDockContainer::OnMouseMove()");

    if (HasCapture()) {
        _ASSERT(m_TabDockable);

        if (m_TabDockable) {
            wxPoint sc_mouse_pos = ::wxGetMousePosition();
            m_DockManager.OnDrag(*m_TabDockable, sc_mouse_pos);
        }
    }
}


void CDockContainer::OnMouseCaptureLost(wxMouseCaptureLostEvent& WXUNUSED(event))
{
    //WM_POST("CDockContainer::OnMouseCaptureLost()");
    // we have lost mouse capture  and so we must cancel D&D session
    wxPoint sc_mouse_pos = ::wxGetMousePosition();
    m_DockManager.OnEndDrag(*m_TabDockable, CDockManager::eCancel, sc_mouse_pos);
}


///////////////////////////////////////////////////////////////////////////////
/// Saving and Loading Layouts
/// Loads a full layout tree from the given CUser_object
/// IWMClientFactory is used for instantiating the clients refered by the layout

void CDockContainer::x_DestroyEmptyLayout()
{
    if (m_VisibleTree  &&  m_VisibleTree->GetRoot()) {
        // delete the root window (no others should exist)
        CRef<TNode> vis_root = m_VisibleTree->GetRoot();
        x_DestroyNode(*vis_root->GetClone(), eDestroyWin);
        x_SetRootWindow(NULL);
    }
}

CDockContainer::TNode* CDockContainer::x_FindNodeByWindow(wxWindow* window)
{
    TWindowToNode::iterator it = m_WindowToNode.find(window);
    return (it == m_WindowToNode.end()) ? NULL : it->second;
}


void CDockContainer::ApplyLayout(CDockLayoutTree& tree)
{
    bool failed = false;
    //Freeze();
    try {
        x_DestroyEmptyLayout();
        //process the Full Tree, remove all hidden clients and containers that have less than one child node

        tree.HideAll();
        tree.GetCentralPane()->SetHidden(false);

        //WM_POST("Loaded Full Tree ");
        //tree.LogPost();

        SetFullTree(tree);

        //WM_POST("Trees after clean-up");
        LogPostTrees("CDockContainer::ApplyLayout()");

    } catch(std::exception&) {
        failed = true;
        ERR_POST("CDockContainer::ApplyLayout() - error loading layout - ");
    }

    if (failed)
        x_InitDefaultTree(m_DockManager.GetMainContainer() == this);

    //Thaw();
}

static const char* kWindowPos    = "WindowPos";
static const char* kXTag         = "x";
static const char* kYTag         = "y";
static const char* kWidthTag     = "width";
static const char* kHeightTag    = "height";
static const char* kMaximizedTag = "maximized";

static CUser_object* sWindowPosToUserObject(const wxRect& rc, bool maximized)
{
    CRef<CUser_object> pos(new CUser_object());
    pos->SetType().SetStr(kWindowPos);
    pos->AddField(kXTag, rc.x);
    pos->AddField(kYTag, rc.y);
    pos->AddField(kWidthTag, rc.width);
    pos->AddField(kHeightTag, rc.height);
    pos->AddField(kMaximizedTag, maximized);
    return pos.Release();
}

static void sWindowPosFromUserObject(const CUser_object& obj, wxRect& rc, bool& maximized)
{
    rc.x = obj.GetField(kXTag).GetData().GetInt();
    rc.y = obj.GetField(kYTag).GetData().GetInt();
    rc.width = obj.GetField(kWidthTag).GetData().GetInt();
    rc.height = obj.GetField(kHeightTag).GetData().GetInt();
    maximized = obj.GetField(kMaximizedTag).GetData().GetBool();
}

static const char* kDockContainer = "DockContainer";
static const char* kTreeTag    = "Tree";

CUser_object* CDockContainerDescr::ToUserObject() const
{
    CRef<CUser_object> container;

    CRef<CUser_object> tree(m_Tree->ToUserObject());
    if (tree) {
        container.Reset(new CUser_object());
        container->SetType().SetStr(kDockContainer);
        container->AddField(kTreeTag, *tree);

        CRef<CUser_object> windowPos(sWindowPosToUserObject(m_Rect, m_Maximized));
        container->AddField(kWindowPos, *windowPos);
    }

    return container.ReleaseOrNull();
}

CDockContainerDescr* CDockContainerDescr::FromUserObject(const CUser_object& obj)
{
    CRef<CDockLayoutTree> tree(CDockLayoutTree::FromUserObject(obj.GetField(kTreeTag).GetData().GetObject()));
    wxRect rc;
    bool maximized = false;
    sWindowPosFromUserObject(obj.GetField(kWindowPos).GetData().GetObject(), rc, maximized);
    return new CDockContainerDescr(rc, maximized, tree);
}

CUser_object* CDockContainer::SaveLayout(wxFrame* mainFrame)
{
    CRef<CUser_object> container;

    if (m_FullTree && NeedToSavePositions()) {
        m_FullTree->SaveSplitterLayout();

        CRef<CDockLayoutTree> treeClone(m_FullTree->CloneTree());
        x_Full_CleanLayoutTree(*treeClone);
        CRef<CUser_object> tree(treeClone->ToUserObject());
        if (tree) {
            container.Reset(new CUser_object());
            container->SetType().SetStr(kDockContainer);

            container->AddField(kTreeTag, *tree);

            wxFrame* frame = GetDockFrame();
            if (!frame) frame = mainFrame;

            if (frame) {
                CRef<CUser_object> windowPos(sWindowPosToUserObject(frame->GetRect(), frame->IsMaximized()));
                container->AddField(kWindowPos, *windowPos);
            }
        } else {
            _ASSERT(false);
        }
    }

    return container.ReleaseOrNull();
}

END_NCBI_SCOPE
