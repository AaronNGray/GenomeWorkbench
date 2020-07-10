/*  $Id: dock_manager.cpp 43826 2019-09-05 21:13:31Z katargir $
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

#include <gui/widgets/wx/dock_manager.hpp>
#include <gui/widgets/wx/window_manager.hpp>

#include <gui/widgets/wx/dock_container.hpp>
#include <gui/widgets/wx/dock_panel.hpp>
#include <gui/widgets/wx/min_panel_container.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/wm_client.hpp>
#include <gui/widgets/wx/ui_command.hpp>

#include <gui/widgets/gl/gl_widget_base.hpp>

#include <objects/general/User_object.hpp>
#include <objects/general/Object_id.hpp>

#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/utils.h>
#include <wx/menu.h>

#ifdef __WXMAC__
#import <AppKit/NSWindow.h>
#endif

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/// for debugging purpose
/*
static int s_CheckDragCount = 0;

#define CHECK_DRAG {\
    if( m_DragFrame ) { \
        if( !m_DragFrame->IsActive() ){              \
            ERR_POST( Error << "Drag frame lost its activation! [" << s_CheckDragCount++ << "]" ); \
        }\
    } else {\
        ERR_POST( Error << "No drag frame! [" << s_CheckDragCount++ << "]" ); \
    } \
}
#define RESET_CHECK_DRAG { s_CheckDragCount = 0; }
*/

////////////////////////////////////////////////////////////////////////////////
/// CDockManager

CDockManager::CDockManager(CWindowManager& manager, wxWindow& parent)
:   m_ParentWindow(parent),
    m_WindowManager(manager),
    m_FrameHook(NULL),
    m_MinContainer(NULL),
    m_DragClient(NULL),
    m_DragFrame(NULL),
    m_ActiveDragContainer(NULL),
    m_FocusedChild(0)
{
    CDockContainer* container = new CDockContainer(*this, &parent, true);
    m_Containers.push_back(container);

    m_MinContainer = new CMinPanelContainer(&parent, *this);
    //WM_POST("CDockManager constructor  m_Containers.push_back()"  << this);
}


CDockManager::~CDockManager()
{
    // delete all frames
    TContainers conts = m_Containers; // make a copy to avoid side effects
    for( size_t i = 0;  i < conts.size();  i++   )   {
        CDockContainer* cont = conts[i];
        if(cont->GetDockFrame())    {
            x_DestroyFrameAndContainer(cont);
        }
    }

    _ASSERT(m_Containers.size() == 1);
    m_Containers[0]->Destroy();
    m_Containers.clear();
}


void CDockManager::SetKeyboardHook(wxFrame* frame)
{
    m_FrameHook = dynamic_cast<IDockManagerKeyHook*>(frame);
}


CWindowManager& CDockManager::GetWindowManager()
{
    return m_WindowManager;
}


CDockContainer* CDockManager::GetMainContainer()
{
    return m_Containers.front();
}


const CDockContainer* CDockManager::GetMainContainer() const
{
    return m_Containers.front();
}


CMinPanelContainer* CDockManager::GetMinPanelContainer()
{
    return m_MinContainer;
}


// tries to find a saved position for the given window and place the window
// in the position. If no position found - adds window to the default container
void CDockManager::AddToDefaultLocation (IWMClient& client, bool bFloat)
{
    CDockContainer* cont = NULL; // container that will host the client

    TFingerprint fingerprint = client.GetFingerprint();

    // check positions in visible containers
    for( size_t i = 0; ! cont  &&  i < m_Containers.size(); i++  ){
        CDockContainer* c = m_Containers[i];
        if( c->HasDefaultPositionFor(fingerprint) ){
            cont = c;
        }
    }

    CFloatingFrame* frame = NULL;
    if( ! cont ){        
        // check positions in hidden floating frames
        TDescrMRUList::TItems& descrs = m_DeadFrameDescrs.SetItems();
        NON_CONST_ITERATE(TDescrMRUList::TItems, it, descrs)  {
            CDockContainerDescr& descr = **it;
            CDockLayoutTree& tree = *descr.GetTree();

            // FIXME: this fixes situation, which should not happen, but still happens 
            //        somehow during complex layout operations and leads to crash
            //        ALL clients should be HIDDEN in hidden floating frame
            tree.HideAll();

            if(tree.FindByFingerprint(fingerprint))   {
                frame = x_CreateFrameAndContainer(descr.GetPos(), descr.GetSize(), &tree);
                cont = frame->GetDockContainer();
                descrs.erase(it);
                break;
            }
        }
    }
    if( ! cont) {
        if (bFloat) {
            wxRect panel_rc = GetScreenRect(m_ParentWindow);
            wxPoint pos = panel_rc.GetLeftTop();
            pos += wxPoint(40, 40);
            frame = x_CreateFrameAndContainer(pos, panel_rc.GetSize(), NULL);
            cont = frame->GetDockContainer();
        }
        else
            cont = GetMainContainer();
    }
    cont->AddClientToDefaultLocation(client);

    if(frame)   {
        frame->Show();
        frame->SetFocus();
    }
    client.GetWindow()->SetFocus();
}


void CDockManager::Remove(IWMClient& client)
{
    CDockPanel* panel = x_GetDockPanel(client);
    _ASSERT(panel);

    if(panel)   {
        Remove(*panel);
    }
}


// removes IDockableWindow (IWMCLient in Dock Panel or Dock Container)
// from Dock Manager and destroys its window, does not send any
// notifications to Window Manager
void CDockManager::Remove(IDockableWindow& dockable)
{
    CDockContainer* cont = dockable.GetDockContainer();
    if(cont)    {
        cont->RemoveWindow(dockable, CDockContainer::eDestroyWin); // destroy client

        if(GetMainContainer() != cont  &&  cont->HasNoWMClients()) {
            x_DestroyFrameAndContainer(cont);
        }
    } else {
        // this is a minimized window or a logical error
        CDockPanel* panel = dynamic_cast<CDockPanel*>(&dockable);
        _ASSERT(m_MinContainer->HasClient(*panel));

        if(m_MinContainer)  {
            m_MinContainer->RemoveClient(*panel);
            panel->RemoveClient(m_MinContainer);
            panel->Destroy();
        }
    }
}


/// moves the given client into the Main Tab Control ("Reset")
void CDockManager::MoveToMainTab(IWMClient& client)
{
    IDockableWindow* dockable = x_GetDockPanel(client);
    CDockContainer* cont = dockable->GetDockContainer();

    if(cont)    {
        // the client is in container
        CRef<TNode> full_node = cont->RemoveWindow(*dockable, CDockContainer::eMoveWin);

        if(GetMainContainer() != cont  &&  cont->HasNoWMClients()) {
            x_DestroyFrameAndContainer(cont);
        }
        GetMainContainer()->AddClientToCentralPane(full_node);
    } else {
        // this is a minimized window
        CDockPanel* panel = dynamic_cast<CDockPanel*>(dockable);
        _ASSERT(m_MinContainer->HasClient(*panel));

        if(m_MinContainer)  {
            m_MinContainer->RemoveClient(*panel);
        }

        CRef<TNode> full_node(new TNode(CDockLayoutTree::eClient, panel));
        full_node->Link(*new TNode(*full_node));

        TFingerprint p = client.GetFingerprint();
        full_node->SetFingerprint(p);
        full_node->GetClone()->SetFingerprint(p);

        GetMainContainer()->AddClientToCentralPane(full_node);
    }
}


wxFrame* CDockManager::MoveToFloatingFrame(IWMClient& client)
{
    CDockPanel* panel = x_GetDockPanel(client);
    _ASSERT(panel);

    if (IsFloating(client))
        return panel->GetDockContainer()->GetDockFrame();

    wxFrame* frame = x_MoveToFloatingFrame(*panel, true);

    if (m_FocusedChild) {
        m_FocusedChild->SetFocus();
        m_FocusedChild = 0;
    }

    return frame;
}


void CDockManager::Minimize(IWMClient& client)
{
    CDockPanel* panel = x_GetDockPanel(client);

    CDockContainer* cont = panel->GetDockContainer();
    cont->RemoveWindow(*panel, CDockContainer::eMinimizeWin);

    m_MinContainer->AddClient(*panel);

    if(GetMainContainer() != cont  &&  cont->HasNoWMClients()) {
        x_DestroyFrameAndContainer(cont);
    }
}


void CDockManager::Restore( IWMClient& client )
{
    CDockPanel* panel = x_GetDockPanel(client);

    if( panel ){
        m_MinContainer->RemoveClient( *panel );
    }

    AddToDefaultLocation( client, false );
}


bool CDockManager::IsInMainTab(IWMClient& client) const
{
    CDockPanel* panel = x_GetDockPanel(client);

    _ASSERT(panel);

    CDockContainer* cont = panel->GetDockContainer();
    return cont == GetMainContainer();
}


bool CDockManager::IsFloating(IWMClient& client) const
{
    CDockPanel* panel = x_GetDockPanel(client);

    _ASSERT(panel);

    CDockContainer* cont = panel->GetDockContainer();
    return cont  &&  cont->IsRootWindow(panel);
}


bool CDockManager::IsMinimized(IWMClient& client) const
{
    CDockPanel* panel = x_GetDockPanel(client);
    return panel ? m_MinContainer->HasClient(*panel) : false;
}


CDockPanel* CDockManager::x_GetDockPanel(IWMClient& client)
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


// Make clients visible and moves focus to the first client in the list.
void CDockManager::ActivateClients(TClients& clients)
{
    // first restore clients if needed
    NON_CONST_ITERATE(TClients, it, clients)    {
        IWMClient& the_client = **it;
        if(IsMinimized(the_client))    {
            Restore(the_client);
        }
    }

    // iterate by Dock Container, activate clients in each container
    for( size_t i = 0; i < m_Containers.size(); i++  )   {
        CDockContainer* cont = m_Containers[i];
        cont->ActivateClients(clients);
    }
    // focus the first one
    if(clients.size())  {
        IWMClient* client = *clients.begin();
        client->GetWindow()->SetFocus();

        #ifdef _DEBUG
        wxRect winloc = client->GetWindow()->GetRect();
        wxPoint scrpnt = client->GetWindow()->GetScreenPosition();
        ERR_POST( 
            Info << "Client location is "
            "(" << winloc.x << "/" << scrpnt.x << ", " << winloc.y <<  "/" << scrpnt.y << ")"
            "[" << winloc.width << ", " << winloc.height << "]" 
        );
        #endif
    }
}

 void CDockManager::OnMove(wxMoveEvent& event)
 {
    // iterate by Dock Container, sending a move event to each one
    // so that it can in turn move attached client frames (sticky tool tips).
    for( size_t i = 0; i < m_Containers.size(); i++  )   {
        CDockContainer* cont = m_Containers[i];
        BroadcastCommandToChildWindows(cont, eCmdParentMove);
    }
 }


wxWindow* CDockManager::GetTopAppWindow()
{
    wxWindow* w = &m_ParentWindow;
    for(  ;w->GetParent();  w = w->GetParent()) {
    }
    return w;
}

void CDockManager::OnBeginDrag(IDockableWindow& dockable, const wxPoint& sc_mouse_pos)
{
    //WM_POST("CDockManager::OnBeginDrag()");

    if( m_DragClient ){
        return; // already dragging
    }

    m_DragClient = &dockable;
    m_DragFrame = dynamic_cast<CFloatingFrame*>(&dockable);
    m_PrevDragPos = sc_mouse_pos;

    m_FocusedChild = 0;

    // if (m_DragFrame) -  we move the whole frame
    if( !m_DragFrame ){
        wxWindow* window = dynamic_cast<wxWindow*>(&dockable);

        CDockContainer* cont = dockable.GetDockContainer();
        if(cont->IsRootWindow(window))    {
            // dockable is the Root Window for this container, so we do not need to create
            // a special Floating Frame - we will use the Frame that hosts this Dock Container
            m_DragFrame = cont->GetDockFrame();
        }
        if( ! m_DragFrame)  {
            // create a new Floating Frame to host dockable
            m_DragFrame = x_MoveToFloatingFrame(dockable, false);
        }
    }

    m_DragFrame->SetFocus();

    m_ActiveDragContainer = NULL;

    // now start handling dragging events
    OnDrag( dockable, sc_mouse_pos );

    if(m_FrameHook) {
        m_FrameHook->ForwardKeyEventsTo(this);
    }

    //WM_POST("CDockManager::OnBeginDrag  END");
}


bool CDockManager::OnDrag(IDockableWindow& dockable, const wxPoint& sc_mouse_pos)
{
    //WM_POST("CDockManager::OnDrag()");

    if(m_DragFrame)    {
        _ASSERT(&dockable == m_DragClient);

        // find the container that handles the dragging
        CDockContainer* active_cont = x_GetActiveContainerByPos(sc_mouse_pos);

        if(active_cont != m_ActiveDragContainer)    {
            // new active container - deactivate the old container and initiate the new one

            // stop dragging in the currently active container
            if(m_ActiveDragContainer)   {
                wxWindow* dummy = NULL;
                m_ActiveDragContainer->OnEndDragOver(dummy);
            }

            m_ActiveDragContainer = active_cont;

            // activate new container
            if(m_ActiveDragContainer)   {
                m_ActiveDragContainer->OnBeginDragOver(sc_mouse_pos);
            }
        } else {
            // active container is the same - forward the event to the container
            if(m_ActiveDragContainer)   {
                m_ActiveDragContainer->OnDragOver(sc_mouse_pos);
            }
        }

        if(m_DragClient != m_DragFrame) {
            // move frame only if it is not being moved by the system
            x_MoveDragFrame(sc_mouse_pos);
        }
        return true;
    }
    return false;
    //WM_POST("CDockManager::OnDrag()  END");
}



// handles the ending of a D&D docking session
// if result == eCancel we simply end the session, but do not
// change window layout
void CDockManager::OnEndDrag(IDockableWindow& dockable, EDragResult result,
                               const wxPoint& sc_mouse_pos)
{
    //WM_POST("CDockManager::OnEndDrag()  result " << result);

    if( m_DragFrame ){
        // handle the last mouse movement
        OnDrag( dockable, sc_mouse_pos );

        if( m_FrameHook ){
            m_FrameHook->ForwardKeyEventsTo(NULL);
        }

        if (m_FocusedChild)
            m_FocusedChild->SetFocus();
        else
            m_DragFrame->SetFocus();

       if( m_ActiveDragContainer ){
            wxWindow* target = NULL;
            EDockEffect effect = m_ActiveDragContainer->OnEndDragOver(target);

            //m_DragFrame->Refresh();

            if(/*(result != eCancel)  &&*/  effect != eNoEffect)  {
                // handle Drop
                _ASSERT(m_DragClient);

                CDockContainer* cont = m_DragClient->GetDockContainer();

                IDockableWindow* dock_client = &dockable;
                if(m_DragClient == m_DragFrame)    {
                    // we drag a frame, get the root window from the frame
                    dock_client = dynamic_cast<IDockableWindow*>(cont->GetRootWindow());
                } else {
                    // we drag a child of a frame, cast the child
                    dock_client = &dockable;
                }


                // Let subwindows know window is about to be docked
                wxWindow* window = dynamic_cast<wxWindow*>(dock_client);               
                BroadcastCommandToChildWindows(window, eCmdSuspend);

                CRef<TNode> full_node = cont->RemoveWindow(*dock_client, CDockContainer::eMoveWin);

                // add the client to the new Dock Container
                m_ActiveDragContainer->AddClient(full_node, effect, target);                

                if(cont->HasNoWMClients())    {
                    // prevent main window activation
                    CFloatingFrame* frame = cont->GetDockFrame();
                    frame->SetWindowStyleFlag(frame->GetWindowStyleFlag()&~wxFRAME_FLOAT_ON_PARENT);
                    x_DestroyFrameAndContainer(cont); // we do not need this container anymore                  
                }

                if (m_FocusedChild)
                    m_FocusedChild->SetFocus();
                else
                    full_node->GetWindow()->SetFocus();

                // Parent of target will enclose the dropped window (target itself
                // does not always enclose dropped window).
                wxWindow* w = target;
                if (target->GetParent() != NULL)
                    w = target->GetParent();

	            bool is_main_window = false;

                wxWindow* p =  w;
                CFloatingFrame* ff = NULL;				
                while (p != NULL && ff == NULL) {
                     ff = dynamic_cast<CFloatingFrame*>(p);
                     p = p->GetParent();
                }
                if (ff!=NULL)
                    is_main_window = false;
                else
                    is_main_window = true;

                if (is_main_window) {
                    // Let all subwindows know dock has been completed.  This is only needed due
                    // to problems with tooltip windows on mac (tooltips in main window can
                    // disappear after a docking event)
                    static int drop_count = 0;
                    ++drop_count;
                    BroadcastCommandToChildWindows(GetMainContainer(), eCmdSuspend);
                    BroadcastCommandToChildWindows(GetMainContainer(), eCmdDockMainWindow, drop_count);
                }
                else {
                    // Let subwindows know dock has been completed
                    BroadcastCommandToChildWindows(w, eCmdReCreate, 0);
                }               
            }
        }

        m_DragClient = NULL;
        m_DragFrame = NULL;
        m_ActiveDragContainer = NULL;
        m_FocusedChild = 0;
    }

    //WM_POST("CDockManager::OnEndDrag() END");
}


void CDockManager::OnCancelDrag()
{
    //WM_POST("CDockManager::OnCancelDrag()");
    OnEndDrag(*m_DragClient, eCancel, ::wxGetMousePosition());
}


/// Creates a new top-level Floating Frame and a Dock Container that will be
/// hosted in the frame.
CFloatingFrame*
    CDockManager::x_CreateFrameAndContainer(const wxPoint& pos,
                                              const wxSize& size,
                                              CDockLayoutTree* tree)
{
    // create a Frame
    CFloatingFrame* frame = new CFloatingFrame(*this, GetTopAppWindow(), pos, size);
    string title = m_WindowManager.GetNewFloatingFrameTitle();
    frame->SetTitle(ToWxString(title));

    // create a Dock Container
    CDockContainer* frame_cont = new CDockContainer(*this, frame, false);
    if(tree)    {
        frame_cont->SetFullTree(*tree);
    }
    m_Containers.push_back(frame_cont);

    // bind Container to the Frame
    frame->SetDockContainer(frame_cont);
    frame_cont->Reparent(frame);

    return frame;
}


/// Destroys a Docking Container togather with its Floating Frame.
void CDockManager::x_DestroyFrameAndContainer(CDockContainer* cont)
{
    bool can_do = cont  &&  cont->HasNoWMClients();
    _ASSERT(can_do);

    if(can_do)  {
        CFloatingFrame* frame = cont->GetDockFrame();

        // save layout of this Dock Container if needed
        if(cont->NeedToSavePositions()) {
            CRef<CDockContainerDescr> descr(
                new CDockContainerDescr(frame->GetPosition(), frame->GetSize(), frame->IsMaximized(),
                                        cont->DisconnectTrees()));
            m_DeadFrameDescrs.AddItem(descr);
        }
        // remove this Dock Container from m_Containers
        TContainers::iterator it = find(m_Containers.begin(), m_Containers.end(), cont);
        m_Containers.erase(it);

        // Destroy the Frame (together with the Container)
        //WM_POST(" Destroy the Frame (together with the Container) " << frame);
        frame->Destroy();
    }
}


// Moves IDockableWindow that is already managed by the Dock Manager into a
// separate floating frame
CFloatingFrame*
    CDockManager::x_MoveToFloatingFrame(IDockableWindow& dockable, bool shift)
{
    wxWindow* window = dynamic_cast<wxWindow*>(&dockable);

    m_FocusedChild = 0;
    wxWindow* focused = wxWindow::FindFocus();
    wxWindow* w = focused;
    while (w) {
        if (w == window) {
            m_FocusedChild = focused;
            break;
        }
        if (w->IsTopLevel())
            break;
        w = w->GetParent();
    }

    CDockContainer* cont = dockable.GetDockContainer();
    wxPoint sc_origin = window->GetScreenPosition();

    //This warns subwindows that the window will be docked
    BroadcastCommandToChildWindows(window, eCmdSuspend);

    // create Frame and Dock Container and bind them together
    wxRect panel_rc = GetScreenRect(*window);
    CDockPanel* panel = dynamic_cast<CDockPanel*>(&dockable);
    wxPoint pos = panel_rc.GetLeftTop();
    if(shift) {
        pos += wxPoint(40, 40);
    }
    wxSize size = panel_rc.GetSize();

    CFloatingFrame* frame = x_CreateFrameAndContainer(pos, size, NULL);
    CDockContainer* frame_cont = frame->GetDockContainer();

    IWMClient* wmClient = panel->GetClient();

    if(cont)    {
        // the client is in container
        // remove the client from its current container, but do not destroy it
        CRef<TNode> full_node = cont->RemoveWindow(dockable, CDockContainer::eMoveWin);

        // add the client to the new container
        frame_cont->AddClientToCentralPane(full_node);
    } else {
        // this is a minimized window
        _ASSERT(m_MinContainer->HasClient(*panel));

        if(m_MinContainer)  {
            m_MinContainer->RemoveClient(*panel);
        }

        CRef<TNode> full_node(new TNode(CDockLayoutTree::eClient, panel));
        full_node->Link(*new TNode(*full_node));

        TFingerprint p = wmClient->GetFingerprint();
        full_node->SetFingerprint(p);
        full_node->GetClone()->SetFingerprint(p);

        frame_cont->AddClientToCentralPane(full_node);
    }


#ifndef __WXGTK__
    // adjust position so that dockable stays in the same place on screen
    wxPoint sc_new_origin = window->GetScreenPosition();
    wxPoint frame_pos = frame->GetScreenPosition();

    wxPoint delta = sc_origin - sc_new_origin;
    frame_pos += delta;
    frame->Move(frame_pos);
#endif

    window->Show();
    frame->Show();
    frame->SetFocus();

    //Tell subwindows that the dock has been completed
    BroadcastCommandToChildWindows(window, eCmdReCreate);

    if (wmClient->IsAForm()) {
        size = wmClient->GetWindow()->GetVirtualSize();
        size = panel->ClientToWindowSize(size);
        size = frame->ClientToWindowSize(size);
        frame->SetSize(size);
    }

    return frame;
}


void CDockManager::x_MoveDragFrame(const wxPoint& sc_mouse_pos)
{
    if(sc_mouse_pos != m_PrevDragPos) {
        wxPoint delta = sc_mouse_pos - m_PrevDragPos;

        //WM_POST("CDockManager::x_MoveDragDrame() m_PrevDragPos.y " << m_PrevDragPos.y << ",  sc_mouse_pos.y " << sc_mouse_pos.y);

        wxPoint pos = m_DragFrame->GetPosition();

#ifdef __WXMAC__
        int maxy = 4096;
        int maxx;
        int menubar_height;

        GetMacDragWindowBounds(m_DragFrame, menubar_height, maxy, maxx);
        
    // Do not move the window if the mouse is in the system menubar area
        if (!(//sc_mouse_pos.y < m_DragFrame->GetRect().GetTop() || 
              sc_mouse_pos.y < menubar_height)) {
                if (pos.y + delta.y > menubar_height && pos.y + delta.y < maxy) {
                    //_TRACE("New Position: (" << pos.x << ", " << pos.y << ")");
                    pos += delta;
                    m_DragFrame->SetPosition(pos);

                    m_PrevDragPos = sc_mouse_pos;
                }
                else {
                    //_TRACE("Update rejected. Height = " << menubar_height);
                    pos += delta;
                    if (pos.y < menubar_height)
                         pos.y = menubar_height;
                    if (pos.y > maxy)
                         pos.y = maxy;
                    m_DragFrame->SetPosition(pos);

                    m_PrevDragPos = sc_mouse_pos;
                }
        }
#else
        pos += delta;
        m_DragFrame->SetPosition(pos);

        m_PrevDragPos = sc_mouse_pos;
#endif

    } //else         WM_POST("CDockManager::x_MoveDragDrame - skipping");
}


CDockContainer* CDockManager::x_GetActiveContainerByPos(const wxPoint& sc_mouse_pos)
{
    CDockContainer* dragged_cont = m_DragFrame->GetDockContainer();

    for(int i = (int)m_Containers.size()-1;  i >= 0;  i-- )   {
        CDockContainer* cont = m_Containers[i];

        if(cont != dragged_cont)    {
            wxRect rc = GetScreenRect(*cont);

            if(rc.Contains(sc_mouse_pos))   {
                return cont;
            }
        }
    }
    return NULL;
}


void CDockManager::OnFloatingPaneBeginMove(CFloatingFrame& frame, const wxPoint& sc_mouse_pos)
{
    //WM_POST("CDockManager::OnFloatingPaneBeginMove()");

    if(m_DragClient == NULL) {
        OnBeginDrag(frame, sc_mouse_pos);
    }
}


void CDockManager::OnFloatingPaneMoving(CFloatingFrame& frame, const wxPoint& sc_mouse_pos)
{
    //WM_POST("CDockManager::OnFloatingPaneMoving()");
    if(m_DragClient == &frame)   {
        OnDrag(frame, sc_mouse_pos);
    }
}


void CDockManager::OnFloatingPaneEndMove(CFloatingFrame& frame, CDockManager::EDragResult result, const wxPoint& sc_mouse_pos)
{
    WM_POST("CDockManager::OnFloatingPaneEndMove");
    if(m_DragClient == &frame)   {
        OnEndDrag(frame, result, sc_mouse_pos);
    }
}


wxMenu* CDockManager::GetDockPanelMenu(CDockPanel& panel)
{
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    IWMClient* client = panel.GetClient();

    wxMenu* menu = new wxMenu();

    if(IsMinimized(*client))   {
        cmd_reg.AppendMenuItem(*menu, eCmdWindowRestore);
    } else {
        cmd_reg.AppendMenuItem(*menu, eCmdWindowMinimize);
    }
    if( ! IsFloating(*client))  {
        cmd_reg.AppendMenuItem(*menu, eCmdWindowFloat);
    }
    if( ! IsInMainTab(*client)) {
        cmd_reg.AppendMenuItem(*menu, eCmdMoveToMainTab);
    }
    if(client  &&  client->GetColor())  {
        cmd_reg.AppendMenuItem(*menu, eCmdSetClientColor);
    }

    return menu;
}


bool CDockManager::OnDockPanelCommand(CDockPanel& panel, TCmdID cmd)
{
    return m_WindowManager.OnCaptionPanelCommand(panel, cmd);
}


void CDockManager::OnKeyDown(wxKeyEvent& event)
{
    //WM_POST("CDockManager::OnKeyDown");

    int code = event.GetKeyCode();
    if(code == WXK_ESCAPE)    {
        OnCancelDrag();
    } else if(code == WXK_SHIFT  &&  m_ActiveDragContainer)    {
        wxPoint sc_mouse = wxGetMousePosition();
        m_ActiveDragContainer->OnDragOver(sc_mouse);
    } else {
        event.Skip();
    }
}


void CDockManager::OnKeyUp(wxKeyEvent& event)
{
    //WM_POST("CDockManager::OnKeyUp");

    int code = event.GetKeyCode();
    if(code == WXK_SHIFT  &&  m_ActiveDragContainer)    {
        wxPoint sc_mouse = wxGetMousePosition();
        m_ActiveDragContainer->OnDragOver(sc_mouse);
    } else {
        event.Skip();
    }
}


void CDockManager::OnCloseDockable(IDockableWindow& dockable)
{
    CDockPanel* panel = dynamic_cast<CDockPanel*>(&dockable);
    _ASSERT(panel);

    m_WindowManager.OnCaptionPanelCommand(*panel, eCmdCloseDockPanel);
}


// a handler for frame's close button
void CDockManager::OnFrameClosePressed(CFloatingFrame* frame)
{
    //WM_POST("CDockManager::OnCloseFrame()");

    frame->Hide(); // do not close it yet

    CDockContainer* cont = frame->GetDockContainer();
    
    vector<IWMClient*> clients;
    cont->GetAllClients(clients);

    // place a request to close all clients in the frame
    // when it happens, the frame will be closed automatically
    m_WindowManager.OnCloseClientsRequest(clients);
}


void CDockManager::OnTabClosePressed(const vector<IWMClient*>& clients)
{
    //WM_POST("CDockManager::OnTabClosePressed()");
    m_WindowManager.OnCloseClientsRequest(clients);
}

void CDockManager::RaiseFloatingInZOrder()
{
    // Order m_Containers may be updated during the Raise() command
    // via an activate event that calls OnFloatingFrameActivateEvent,
    // so make a copy of the container first.
    TContainers cont_copy = m_Containers;
    TContainers::iterator it;

    for (it=cont_copy.begin(); it!=cont_copy.end(); ++it) {      
        wxWindow* w = *it;      
        CFloatingFrame* f = NULL;

        while (w!=NULL && f==NULL) {
            f = dynamic_cast<CFloatingFrame*>(w);       
            if (f!=NULL) {              
                f->Raise();
            }
            w = w->GetParent();
        }                    
    }
}

void CDockManager::RaiseFloatingInZOrder(wxRect r)
{
    // See if any windows overlap "r" and if so call RaiseFloatingInZOrder
    TContainers::iterator it;
    bool overlap = false;

    for (it=m_Containers.begin(); it!=m_Containers.end() && !overlap; ++it) {        
        wxWindow* w = *it;
        CFloatingFrame* f = NULL;

        while (w!=NULL && f==NULL) {
            f = dynamic_cast<CFloatingFrame*>(w);            
            w = w->GetParent();
        }       
        if (f!=NULL) {
            if (f->GetRect().Intersects(r))
                overlap = true;
        }
    }

    if (overlap)
        RaiseFloatingInZOrder();

}

void CDockManager::OnFloatingFrameActivateEvent(CFloatingFrame* frame, bool active)
{
    _ASSERT(frame);

    if(frame  &&  active)   {
        // frame is activate - bring it to the top in Z-order
        CDockContainer* cont = frame->GetDockContainer();
        _ASSERT(cont);

        //WM_POST(" CDockManager::OnFloatingFrameActivateEvent()  cont " << cont);

        TContainers::iterator it =
            std::find(m_Containers.begin(), m_Containers.end(), cont);

#ifndef __WXGTK__
        _ASSERT(it != m_Containers.end());
#endif
        if (it != m_Containers.end()) {
            m_Containers.erase(it);
            m_Containers.push_back(cont);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
/// Saving and Loading layouts

static const char*  kContainersTag  = "Containers";
static const char*  kLayoutTag2      = "Dock Manager Layout 2";
static const char*  kVideoIdTag      = "VideoId";

const int kMaxContainers = 10;

CUser_object* CDockManager::SaveLayout()
{
    CRef<CUser_object> layout(new CUser_object());
    layout->SetType().SetStr(kLayoutTag2);

    wxFrame* mainFrame = m_WindowManager.GetMainWindow();

    vector<CRef<CUser_object> > descrs;

    for (size_t i = 0; i < m_Containers.size(); i++) {
        CDockContainer* cont = m_Containers[i];
        CRef<CUser_object> contDesc(cont->SaveLayout(mainFrame));
        if (contDesc)
            descrs.push_back(contDesc);
        else if (i == 0) {
            _ASSERT(false);
            return 0;
        }
    }

    ITERATE(TDescrMRUList::TItems, it, m_DeadFrameDescrs.GetItems())  {
        if (descrs.size() >= kMaxContainers)
            break;

        CRef<CUser_object> contDesc((*it)->ToUserObject());
        if (contDesc) descrs.push_back(contDesc);
    }

    layout->AddField(kVideoIdTag, GetVideoId());
    layout->AddField(kContainersTag, descrs);

    return layout.Release();
}

bool CDockManager::x_ReadLayout(const objects::CUser_object& object, vector<CRef<CDockContainerDescr> >& descriptors, string& videoId)
{
    if (!object.GetType().IsStr() || object.GetType().GetStr() != kLayoutTag2) {
        LOG_POST(Error << "Invalid layout object");
        return false;
    }

    bool result = false;
    try {
        videoId = object.GetField(kVideoIdTag).GetData().GetStr();
        const vector<CRef<CUser_object> >& descrs = 
            object.GetField(kContainersTag).GetData().GetObjects();
        for (size_t i = 0; i < descrs.size(); ++i) {
            descriptors.push_back(CRef<CDockContainerDescr>(CDockContainerDescr::FromUserObject(*descrs[i])));
        }
        result = true;
    } NCBI_CATCH("CDockManager::x_ReadLayout()");

    return result;
}

void CDockManager::ApplyLayout(const CUser_object& object)
{
    vector<CRef<CDockContainerDescr> > descrs;
    string videoId;

    if (!x_ReadLayout(object, descrs, videoId))
        return;

    if (descrs.empty()) {
        LOG_POST(Error << "Failed to read layout: " << "Empty layout");
        return;
    }

    vector<IWMClient*> allClients;
    for( size_t i = 0;  i < m_Containers.size();  i++)  {
        CDockContainer* cont = m_Containers[i];

        vector<IWMClient*> clients;
        cont->GetAllClients(clients);
        allClients.insert(allClients.end(), clients.begin(), clients.end());

        ITERATE(vector<IWMClient*>, it, clients)
            Minimize(**it);
    }

    m_DeadFrameDescrs.SetItems().clear();

    CRef<CDockContainerDescr> mainDescr = descrs[0];
    descrs.erase(descrs.begin());

    wxFrame* mainFrame = m_WindowManager.GetMainWindow();
    if (videoId == GetVideoId()) {
        mainFrame->SetSize(mainDescr->GetRect());
    } else {
        LOG_POST(Info << "Applying laoyout: VideoId mismatch");
    }
    if (mainDescr->GetMaximized() && !mainFrame->IsMaximized())
        mainFrame->Maximize();
    GetMainContainer()->ApplyLayout(*mainDescr->GetTree());

    REVERSE_ITERATE(vector<CRef<CDockContainerDescr> >, it, descrs)
        m_DeadFrameDescrs.AddItem(*it);

    ITERATE(vector<IWMClient*>, it, allClients)
        Restore(**it);
}

class FClientFinder
{
public:
    FClientFinder(set<string>& views) : m_Views(views) {}

    void    operator()(CDockLayoutTree::CNode& node)
    {
        if (!node.IsContainer() && node.IsVisible()) {
            IWMClient::CFingerprint fingerprint = node.GetFingerprint();
            if (!fingerprint.IsEmpty()) {
                m_Views.insert(fingerprint.GetId());
            }
        }
    }

protected:
    set<string>& m_Views;
};


void CDockManager::LoadLayout(const CUser_object& object, IWMClientFactory& factory)
{
    vector<CRef<CDockContainerDescr> > descrs;
    string videoId;

    if (!x_ReadLayout(object, descrs, videoId))
        return;

    if (descrs.empty()) {
        LOG_POST(Error << "Failed to read layout: " << "Empty layout");
        return;
    }

    set<string> viewsToCreate;
    NON_CONST_ITERATE(vector<CRef<CDockContainerDescr> >, it, descrs) {
        FClientFinder finder(viewsToCreate);
        (*it)->GetTree()->DepthFirstForEach(finder);
    }

    m_DeadFrameDescrs.SetItems().clear();

    CRef<CDockContainerDescr> mainDescr = descrs[0];
    descrs.erase(descrs.begin());

    wxFrame* mainFrame = m_WindowManager.GetMainWindow();
    GetMainContainer()->ApplyLayout(*mainDescr->GetTree());

    REVERSE_ITERATE(vector<CRef<CDockContainerDescr> >, it, descrs)
        m_DeadFrameDescrs.AddItem(*it);

    ITERATE(set<string>, it, viewsToCreate) {
        IWMClient::CFingerprint fingerprint(*it, true);
        IWMClient* client = factory.CreateClient(fingerprint, mainFrame);
        if (!client) continue;
        m_WindowManager.AddClient(*client, false);
    }
}


END_NCBI_SCOPE
