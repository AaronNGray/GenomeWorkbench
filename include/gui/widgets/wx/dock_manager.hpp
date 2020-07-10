#ifndef GUI_WIDGETS_WX___DOCK_MANAGER__HPP
#define GUI_WIDGETS_WX___DOCK_MANAGER__HPP

/*  $Id: dock_manager.hpp 37572 2017-01-24 18:30:50Z katargir $
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
 *      Dock Manager - manages Client windows that can be placed in Dock Containers.        
 */

#include <corelib/ncbiobj.hpp>

#include <gui/widgets/wx/dock_layout.hpp>
#include <gui/widgets/wx/dock_frames.hpp>
#include <gui/widgets/wx/dock_container.hpp>

#include <gui/utils/mru_list.hpp>
#include <gui/utils/command.hpp>

BEGIN_NCBI_SCOPE

class CDockLayoutTree;
class CWindowManager;
class IWMClientFactory;
class IWMClient;
class CDockPanel;
class CMinPanelContainer;

BEGIN_SCOPE(objects)
    class CUser_object;
END_SCOPE(objects)


class CDockManager;

///////////////////////////////////////////////////////////////////////////////
/// IDockManagerKeyHook
class IDockManagerKeyHook
{
public:
    virtual void    ForwardKeyEventsTo(CDockManager* manager) = 0;

    virtual ~IDockManagerKeyHook()  {}
};

///////////////////////////////////////////////////////////////////////////////
/// CDockManager
/// CDockManager sends requests to Window Manager, Window Manager
/// makes decisions about deleting clients and calls Dock Manager functions.

class  NCBI_GUIWIDGETS_WX_EXPORT  CDockManager
{
public:
    typedef CDockLayoutTree::CNode TNode;
    typedef vector< CRef<CDockLayoutTree> > TTreeVector;
    typedef vector<IWMClient*>  TClients;
    typedef IWMClient::CFingerprint TFingerprint;

    enum    EDragResult {
        eDrop,
        eCancel
    };

public:
    CDockManager(CWindowManager& manager, wxWindow& parent);
    virtual ~CDockManager();

    void    SetKeyboardHook(wxFrame* frame);

    CWindowManager&   GetWindowManager();
    CDockContainer*   GetMainContainer();
    const CDockContainer*   GetMainContainer() const;
    CMinPanelContainer* GetMinPanelContainer();

    wxWindow*         GetTopAppWindow();

    void    AddToDefaultLocation(IWMClient& client, bool bFloat);
    void    Remove(IWMClient& client);
    void    Remove(IDockableWindow& dockable);

    void    Minimize(IWMClient& client);
    void    Restore(IWMClient& client);
    wxFrame*    MoveToFloatingFrame(IWMClient& client);
    void        MoveToMainTab(IWMClient& client);

    bool    IsInMainTab(IWMClient& client) const;
    bool    IsFloating(IWMClient& client) const;
    bool    IsMinimized(IWMClient& client) const;

    void    ActivateClients(TClients& clients);
    void    OnMove(wxMoveEvent& event);

    /// @name Dragging interface
    /// @{
    void    OnBeginDrag(IDockableWindow& dockable, const wxPoint& sc_mouse_pos);
    bool    OnDrag(IDockableWindow& dockable, const wxPoint& sc_mouse_pos);
    void    OnEndDrag(IDockableWindow& dockable, EDragResult result, const wxPoint& sc_mouse_pos);

    void    OnCancelDrag();

    void    OnFloatingPaneBeginMove(CFloatingFrame& frame, const wxPoint& sc_mouse_pos);
    void    OnFloatingPaneMoving(CFloatingFrame& frame, const wxPoint& sc_mouse_pos);
    void    OnFloatingPaneEndMove(CFloatingFrame& frame, CDockManager::EDragResult result,
                                  const wxPoint& sc_mouse_pos);
    /// @}

    wxMenu* GetDockPanelMenu(CDockPanel& panel);
    bool    OnDockPanelCommand(CDockPanel& panel, TCmdID cmd);

    void    OnKeyDown(wxKeyEvent& event);
    void    OnKeyUp(wxKeyEvent& event);

    void    OnCloseDockable(IDockableWindow& dockable);
    void    OnFrameClosePressed(CFloatingFrame* frame);
    void    OnTabClosePressed(const vector<IWMClient*>& clients);
    void    OnFloatingFrameActivateEvent(CFloatingFrame* frame, bool active);

    void    RaiseFloatingInZOrder();
    void    RaiseFloatingInZOrder(wxRect);

    void    ApplyLayout(const objects::CUser_object& object);

    /// creates a new window layout based on the given Memento
    /// instantiates the clients as needed using the given factory
    void    LoadLayout(const objects::CUser_object& object, IWMClientFactory& factory);

    /// produces Memento describing the current window layout (including all clients)
    objects::CUser_object*   SaveLayout();

protected:
    static CDockPanel* x_GetDockPanel(IWMClient& client);

    void    x_OnEndDrag(IDockableWindow& dockable, bool apply);
    void    x_MoveDragFrame(const wxPoint& sc_mouse_pos);

    CFloatingFrame* x_CreateFrameAndContainer(const wxPoint& pos, const wxSize& size, CDockLayoutTree* tree);
    void    x_DestroyFrameAndContainer(CDockContainer* cont);

    CFloatingFrame* x_MoveToFloatingFrame(IDockableWindow& dockable, bool shift);

    CDockContainer* x_GetActiveContainerByPos(const wxPoint& sc_mouse_pos);

protected:
    typedef vector<CDockContainer*>   TContainers;
    typedef CMRUList< CRef<CDockContainerDescr> > TDescrMRUList;

protected:
    bool x_ReadLayout(const objects::CUser_object& object, vector<CRef<CDockContainerDescr> >& descriptors, string& videoId);

    /// a window that hosts the main dock container
    wxWindow&    m_ParentWindow;

    CWindowManager&     m_WindowManager;

    IDockManagerKeyHook*    m_FrameHook;

    /// A list of all Dock containers, the first container in the list is the main one
    TContainers m_Containers;

    /// holds minimized IWMClients
    CMinPanelContainer* m_MinContainer;

    // when a Floating Frame is closed, the layout tree corresponding to the frame
    // as well as the frame size and position are saved in m_DeadFrameDescrs. This
    // will allow for restoring the frame in future.
    TDescrMRUList   m_DeadFrameDescrs;

    // D&D state
    IDockableWindow*    m_DragClient; // dockable client being dragged
    CFloatingFrame*     m_DragFrame; // frame window being dragged
    wxPoint     m_PrevDragPos;
    CDockContainer*   m_ActiveDragContainer; // container that handles dragging events
    wxWindow*         m_FocusedChild;
};


END_NCBI_SCOPE


#endif  // GUI_WIDGETS_WX___DOCK_MANAGER__HPP
