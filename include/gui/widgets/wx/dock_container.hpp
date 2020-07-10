#ifndef GUI_WX_DEMO___DOCK_CONTAINER__HPP
#define GUI_WX_DEMO___DOCK_CONTAINER__HPP

/*  $Id: dock_container.hpp 34941 2016-03-03 15:58:19Z katargir $
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
 *  Dock Container - window container that holds docked windows.        
 */


#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/widgets/wx/dock_window.hpp>
#include <gui/widgets/wx/dock_layout.hpp>

#include <objects/general/User_field.hpp>

#include <wx/panel.h>

BEGIN_NCBI_SCOPE

// uncomment this line to enable logging in Window Manager
//#define WM_LOGGING

#ifdef WM_LOGGING
    #define WM_POST(text) LOG_POST(Info << text)
#else
    #define WM_POST(text)
#endif

class CWindowManager;
class CDockPanel;
class CDockMarkerWindow;
class CFloatingFrame;
class CDockNotebook;
class CDockManager;


BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_LOCAL_EVENT_TYPE(USER_EVT_BEGIN_DRAG_OUTSIDE, 1)
END_DECLARE_EVENT_TYPES()

class CBeginDragOutsideEvent : public wxEvent
{
public:
    CBeginDragOutsideEvent() : wxEvent(0, USER_EVT_BEGIN_DRAG_OUTSIDE), m_Window(0) {}
    CBeginDragOutsideEvent(IDockableWindow* window) :
        wxEvent(0, USER_EVT_BEGIN_DRAG_OUTSIDE), m_Window(window) {}
    IDockableWindow* GetWindow() const { return m_Window; }

    virtual wxEvent* Clone() const
        { return new CBeginDragOutsideEvent(m_Window); }

private:
    DECLARE_DYNAMIC_CLASS(CBeginDragOutsideEvent)
    IDockableWindow* m_Window;
};

typedef void (wxEvtHandler::*BeginDragOutsideEventFunction)(CBeginDragOutsideEvent&);

#define BeginDragOutsideEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(BeginDragOutsideEventFunction, &func)

#define EVT_BEGIN_DRAG_OUTSIDE(fn) \
    wx__DECLARE_EVT0(USER_EVT_BEGIN_DRAG_OUTSIDE, BeginDragOutsideEventHandler(fn))

class CDockContainerDescr : public CObject
{
public:
    CDockContainerDescr(wxPoint pos, wxSize size, bool maximized, CRef<CDockLayoutTree> tree) :
        m_Rect(pos, size), m_Maximized(maximized), m_Tree(tree) {}
    CDockContainerDescr(wxRect rect, bool maximized, CRef<CDockLayoutTree> tree) :
        m_Rect(rect), m_Maximized(maximized), m_Tree(tree) {}

    wxPoint GetPos()  const { return m_Rect.GetPosition(); }
    wxSize  GetSize() const { return m_Rect.GetSize(); }
    wxRect  GetRect() const { return m_Rect; }
    bool GetMaximized() const { return m_Maximized; }
    CDockLayoutTree* GetTree() { return m_Tree.GetPointerOrNull(); }

    objects::CUser_object* ToUserObject() const;
    static CDockContainerDescr* FromUserObject(const objects::CUser_object& obj);

private:
    wxRect m_Rect;
    bool   m_Maximized;
    CRef<CDockLayoutTree> m_Tree;
};


///////////////////////////////////////////////////////////////////////////////
/// CDockContainer is a window that hosts docked windows. Dock Container is
/// a top-level window that lives in Window Manager or in a Floating Frame.
/// (CDockContainer cannot hosts other CDockContainers).
/// Each Dock Container holds a Dock Layout data structure that describes the
/// hierarchy of Splitters, Tabbed Panes and Dock Panels hosted in the container.

class NCBI_GUIWIDGETS_WX_EXPORT  CDockContainer :
    public wxPanel
{
public:
    typedef wxPanel TParent;
    typedef CDockLayoutTree::CNode TNode;
    typedef CDockLayoutTree::TNodeVector TNodeVector;
    typedef IWMClient::CFingerprint TFingerprint;

    /// describes
    enum  ERemoveAction {
        eDestroyWin,  // delete window; preserve layout nodes
        eMinimizeWin, // detach window, but node delete; preserve layout nodes
        eMoveWin,     // detach window, but node delete; detach layout nodes
    };

public:
    CDockContainer(CDockManager& manager, wxWindow* parent, bool create_tab);
    virtual ~CDockContainer();

    CDockManager& GetDockManager();

    /// returns floating frame hosting this container
    CFloatingFrame*  GetDockFrame();

    /// @name Client manipulation
    /// @{
    void    AddClientToDefaultLocation(IWMClient& client);
    void    AddClientToCentralPane(CRef<TNode> full_client);
    void    AddClient(CRef<TNode> full_client, EDockEffect effect, wxWindow* target_w);

    CRef<TNode> RemoveWindow(IDockableWindow& dockable, ERemoveAction action);

    void    GetAllClients(vector<IWMClient*>& clients);
    void    GetClientsInWindow(wxWindow* window, vector<IWMClient*>& clients);

    void    ActivateClients(vector<IWMClient*>& clients);
    /// @}

    /// returns window representing the root of the layout hosted by the container
    wxWindow*   GetRootWindow();

    /// initialize the container with the layout tree
    void    SetFullTree(CDockLayoutTree& full_tree);

    CRef<CDockLayoutTree>   DisconnectTrees();

    /// returns true if Dock Container does not have any IWMClients as children
    bool    HasNoWMClients();

    bool    IsRootWindow(wxWindow* window);

    bool    NeedToSavePositions();
    bool    HasDefaultPositionFor(const TFingerprint& fingerprint);

    /// @name Dragging interface - used by Dock Manager
    /// @{
    void OnBeginDragOver(const wxPoint& sc_mouse_pt);
    void OnDragOver(const wxPoint& sc_mouse_pt);
    EDockEffect OnEndDragOver(wxWindow*& target);
    bool    IsHintShown() const;
    /// @}

    // called by Notebook
    void    OnTabClosePressed(wxWindow* page);
    void    OnTabBeginDragOutside(CBeginDragOutsideEvent& event);
    void    OnUpdateTabOrder(CDockNotebook& notebook);

    void    OnLeftDown(wxMouseEvent& event);
    void    OnLeftUp(wxMouseEvent& event);
    void    OnMouseMove(wxMouseEvent& event);
    void    OnMouseCaptureLost(wxMouseCaptureLostEvent& event);

    /// @name Layout saving / restoring
    /// @{
    void    ApplyLayout(CDockLayoutTree& tree);
    objects::CUser_object*   SaveLayout(wxFrame* mainFrame);
    /// @}

    virtual bool    Layout();

    void    LogPost();
    void    LogPostTrees(const string& test);

private:
    DECLARE_EVENT_TABLE();

    enum    EInstFlags   {
        fCreateWindow = 1,
        fAddChildWindows = 2,
        fAll = fCreateWindow | fAddChildWindows
    };

    void    x_CreateControls();
    void    x_SetRootWindow(wxWindow* window);

    string  x_GetPageNameByWindow(wxWindow* window);

    void    x_GetClientsInNode(TNode& node, vector<IWMClient*>& clients);

    CDockPanel*   x_GetDockPanel(IWMClient& client);
    CDockPanel*   x_DockPanelForClient(IWMClient& client);

    void    x_AddClientInTab(CRef<TNode> full_client, CRef<TNode> full_target);
    void    x_AddClientInSplitter(CRef<TNode> full_client, CRef<TNode> full_target, EDockEffect effect);
    TNode*  x_AddClientToHiddenPos(IWMClient& client, CRef<TNode> full_node);

    void    x_Full_RemoveNode(TNode& full_node, ERemoveAction action);

    void    x_Full_DisconnectChildren(TNode& full_node);
    void    x_Full_ReduceContainer(TNode& full_cont);
    void    x_Full_ReduceHiddenContainer_IfNeeded(TNode& full_cont);
    void    x_Full_UnhideContainer_AddChildren(TNode& full_cont,
                                               TNode& full_vis_child,
                                               TNode& full_new_child);

    void    x_Full_CleanLayoutTree(CDockLayoutTree& full_ree);
    void    x_Full_CleanLayoutTreeBranch(TNode* centralPane, TNode& full_node);

    void    x_Visible_RemoveNode(TNode& node, ERemoveAction action);
    void    x_Visible_AddChildToContainer(TNode& cont, TNode& child);
    void    x_Visible_ReplaceChildWithContainer(TNode& child, TNode& cont);
    void    x_Visible_ReplaceContainerWithChild(TNode& cont);

    // for a given node return a fully constructed window corresponding to the node
    void    x_InstantiateNode(TNode& full_node, bool recursive, EInstFlags flags = fAll);
    void    x_InstantiateNode_AddChildWindows(TNode& full_node);
    wxWindow*   x_CreateContainerWindow(CDockLayoutTree::ENodeType type);
    void    x_DestroyNode(TNode& full_node, ERemoveAction action);

    void    BlockSplittersLayout(bool block); //TODO add x_

    bool    x_IsElastic(TNode& node);
    bool    x_IsElastic(wxWindow* win);

    IDockDropTarget*    x_FindDockDropTargetByPos(const wxPoint& screen_pt);
    EDockEffect     x_HitTest(const wxPoint& screen_pt,
                              wxWindow*& target, bool split_tab_pane);
    wxRect x_CalculateHintRect();


    void    x_DestroyEmptyLayout();

    void    x_InitDefaultTree(bool create_tab);

    TNode*  x_FindNodeByWindow(wxWindow* window);

    typedef map<wxWindow*, TNode*>  TWindowToNode;

    TWindowToNode     m_WindowToNode;
    CDockManager&     m_DockManager;
    CWindowManager&   m_WindowManager;
    CFloatingFrame*   m_DockFrame; // floating frame hosting this dock container

    CRef<CDockLayoutTree>   m_FullTree;
    CRef<CDockLayoutTree>   m_VisibleTree;

    wxWindow*   m_RootWindow;

    // D&D State variables
    CDockMarkerWindow*  m_MarkersWindow; // docking markers
    EDockEffect m_Effect; // drop effect
    wxWindow*   m_DragTarget; // window that will be affected if drop happens
    wxWindow*    m_HintWindow; // semi-transparent hint window

    IDockableWindow* m_TabDockable; // if not null than Dock Container needs to handle mouse events
    // to support dragging of the given component
};



END_NCBI_SCOPE


#endif  // GUI_WX_DEMO___DOCK_CONTAINER__HPP
