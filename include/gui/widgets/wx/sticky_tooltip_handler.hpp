#ifndef GGUI_WIDGETS_WX___STICKY_TOOL_HANDLER__HPP
#define GGUI_WIDGETS_WX___STICKY_TOOL_HANDLER__HPP
 /* ===========================================================================
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
 * Authors:  Bob Falk
 *
 * File Description:
 * 
 * For a class to support sticky tool tips, it needs to:
 * 
 * 1.  Be derived from IStickyTooltipHandlerHost.  CGlWidgetPane (gl_widget_base.hpp)
 *     already is, so GL windows derived from CGlWidgetPane already have this. This
 *     derived class defines the virtual functions TTHH_GetWindow and 
 *     TTHH_TTHH_PopupMenuDisplayed.
 * 
 * 2.  Call the IStickyTooltipHandlerHost TTHH_Init() after construction (to get
 *     a pointer to the underlying window).  CGlWidgetPane already does this.
 * 
 * 3.  In the implementation of the class that needs sticky tool tip support,
 *     e.g. CPhyloTreePane (phylo_tree_pane.hpp), implement the virtual functions
 *     that determine if a tool tip is needed: TTHH_NeedTooltip(const wxPoint & pt)
 *     and that returns the contents (string with line breaks) for the tool tip:
 *     TTHH_GetTooltip(const wxRect & rect).  Note that the "NeedTooltip" function
 *     must return a string that serves as a unique id within the window that 
 *     is implementing tip support. 
 *
 *     TTHH_GetTooltip returns an object of type CTooltipInfo in which both the
 *     tip text and the supported buttons (defaults to all) are saved.
 *     The text, in addition to having line breaks for each new line can have 
 *     URLs that, when clicked on, will bring up the user's default browser 
 *     with a new page for that link.  The urls format is standard html, e.g.:
 *
 *     "This is text for a tooltip. The next line has a url.
 *      A url: <a href="https://www.ncbi.nlm.nih.gov">Visit NCBI</a> in a tip"
 * 
 *     To implement these two functions, it may be most appropriate to forward
 *     the request to the pane's rendering object, e.g. IPhyloTreeRender, since
 *     that has the information to determine if a location (x,y) on the pane 
 *     has a graphics object that requires tip support.  The rendering objects
 *     are NOT derived from IStickyTooltipHandlerHost however.  This is a  change
 *     from the previous tool tip handler, ITooltipHandlerHost.
 *
 *  4. In the same class, decare an instance of CStickyTooltipHandler and register
 *     it the same way other mouse/interaction handlers are registered:
 *          m_TooltipManager.SetHost(static_cast<IStickyTooltipHandlerHost*>(this));
 *          x_RegisterHandler(dynamic_cast<IGlEventHandler*>(&m_TooltipManager),
 *                            fMatrixArea, &m_MatrixPane);
 *
 * 5.  Implement support for buttons on the tooltip windows. Note pin/unpin
 *     support is built in, but the commands for search, zoom, and info are
 *     not.  To add support for these, add event table macros to the class
 *     derived from CGlWidgetPane (and therefore IStickyTooltipHandlerHost) for
 *     the respective command events, e.g.:
 *          EVT_BUTTON(eCmdSearchTip, CPhyloTreeWidget::OnSearchTip)
 *          EVT_BUTTON(eCmdZoomTip, CPhyloTreeWidget::OnZoomTip)
 *          EVT_BUTTON(eCmdInfoTip, CPhyloTreeWidget::OnInfoTip)
 *
 *     Also addd support, if desired, for highlighting the screen element to
 *     which the tip is attached via the following two commands:
 *          EVT_BUTTON(eCmdTipActive, CPhyloTreeWidget::OnTipActivated)
 *          EVT_BUTTON(eCmdTipInactive, CPhyloTreeWidget::OnTipDeactivated)
 *
 *     It is also possible to keep track of tips that are currently attached to
 *     the current view by catching the following events which are fired when
 *     a new tip is added to the view or is removed from the view.
 *          EVT_BUTTON(eCmdTipAdded, CPhyloTreeWidget::OnTipAdded)
 *          EVT_BUTTON(eCmdTipRemoved, CPhyloTreeWidget::OnTipRemoved)
 *
 *     When implementing these commands, it will be useful to get a pointer to
 *     the tooltip, particularly so you can get its unique ID string.  The
 *     pointer to the tip is stored in the command event and you can get it via:
 *
 *          void CPhyloTreeWidget::OnSearchTip(wxCommandEvent & evt)
 *          {
 *              // Need the tool tip window for the event - we stored its pointer in
 *              // the button's (event objects) client data when the button was created
 *              wxEvtHandler* obj = dynamic_cast<wxEvtHandler*>(evt.GetEventObject());
 *              if (obj != NULL) {
 *                  CTooltipFrame* f = static_cast<CTooltipFrame*>(obj->GetClientData());
 *                  std::string unique_id = f->GetTipInfo().GetTipID();
 *              }
 *          }
 * 
 * 6.  The sticky tooltips are top-level dialog windows, and may pop-up in front
 *     of other windows based on when each window was created or raise()'d.  
 *     Other windows can be forced in front of tips by calling Raise() on them
 *     when needed. 
 *
 *     To keep the normal undocked windows (CFloatingFrame) windows in front of
 *     tooltips on the main window, you have to pass in a pointer to the IServiceLocator
 *     which gives sticky tooltips access to CDockManager which can raise all
 *     the CFloatingFrame objects in correct Z order when needed.  For PhyTree
 *     this was added to the view object (CPhyTreeView) in the InitView() function:
 *
 *  bool CPhyTreeView::InitView(TConstScopedObjects& objects)
 *  {
 *      m_PhyWidget->SetServiceLocator(m_Workbench);
 *      .....
 *
 */

#include <corelib/ncbistl.hpp>
#include <ncbiconf.h>

#include <gui/gui_export.h>
#include <gui/widgets/gl/ievent_handler.hpp>
#include <gui/widgets/wx/sticky_tooltip_wnd.hpp>
#include <gui/opengl/glrect.hpp>
#include <wx/timer.h>

#include <vector>
using std::vector;

#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
#include <Carbon/Carbon.h>
#endif

BEGIN_NCBI_SCOPE

class CStickyTooltipHandler;
class IServiceLocator;

///////////////////////////////////////////////////////////////////////////////
/// IStickyTooltipHandlerHost - Class from which windows that want to 
/// support sticky tooltips ultimately need to be derived.  Classes
/// then need to add support for the TTHH_* functions to suppport 
/// tool tips.
class NCBI_GUIWIDGETS_WX_EXPORT IStickyTooltipHandlerHost
{
public:
    struct TipLocation {
        TipLocation() {}
        TipLocation( const CGlRect<float>& r, const std::string& id) 
            : TipRect(r)
            , TipID(id) {}
        CGlRect<float> TipRect;
        std::string    TipID;
    };


public:
    IStickyTooltipHandlerHost();
    virtual ~IStickyTooltipHandlerHost();

    /// Init calls virtual functions so can't be called from ctor
    void TTHH_Init();

    /// Return id of the underlying element to dispaly a tooltip, otherwise ""
    virtual string    TTHH_NeedTooltip(const wxPoint & /*pt*/) { return ""; }
    /// Return the contents to be displayed in the tool tip
    virtual CTooltipInfo    TTHH_GetTooltip(const wxRect & /*rect*/) { return CTooltipInfo(); }
    /// Return the pointer to the underlying window
    virtual wxWindow* TTHH_GetWindow() { return NULL; }
    /// Return true if underlying window is currently displaying a popup menu
    virtual bool      TTHH_PopupMenuDisplayed() { return false; }
    /// Return true if the specified URL has been processed. Used for custom commands.
    virtual bool TTHH_ProcessURL(const wxString & /*href*/) { return false; }

    /// Return id and position information for currently displayed tips.  
    /// This allows windows showing tips to add visual elements linking the
    /// tip window on screen to its corresponding graphical element.
    std::vector<TipLocation> GetDisplayedTips() { return m_CurrentTips; }

    /// Set service locator so we can find other views and adjust z order
    void              SetServiceLocator(IServiceLocator* s) { m_ServiceLocator = s; }

protected:
    friend class CStickyTooltipHandler;

    /// Sets information on current tips that can be queried by the user.
    void x_SetCurrentTips(std::vector<TipLocation>& tips) { m_CurrentTips = tips; }

    /// Return the undering wxFrame parent for a wxWindow object
    wxFrame* GetUnderlyingFrame();
    /// Return true if the hosting window is ultimately enclosed in a
    /// CFloatingFrame class (meaning its in a floating window)
    bool IsFloatingWindow();
    /// If the tip window is docked in the main window (not a CFloatingFrame)
    /// this raises all the CFloatingFrame windows to make sure none of them
    /// are overlapped by tooltips
    void RaiseOverlappingWindows(wxRect r);
    /// Retruns true if Any window is currently being dragged
    bool IsDragging();
    /// Mac only - create or return window group for this window
    void InitWindowGroup(int dock_count=0);

    /// Needed to find other views
    IServiceLocator* m_ServiceLocator;

    std::vector<TipLocation>  m_CurrentTips;

#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
    /// Get window group
    WindowGroupRef GetWindowGroup() { return m_TipGroupWinRef; }
    WindowGroupRef m_TipGroupWinRef;

    struct CWindowRef {
        CWindowRef() : m_Ref(NULL), m_DockCount(0) {}
        CWindowRef(WindowGroupRef r, int dc) : m_Ref(r), m_DockCount(dc) {}
        WindowGroupRef m_Ref;
        int m_DockCount;
    };

    static std::vector<std::pair<wxWindow*,CWindowRef> > sm_Windows;
#endif
};

///////////////////////////////////////////////////////////////////////////////
/// CStickyTooltipHandler : Class that manages all the tooltips
/// for a specific window. Primarily it responds to move, size, and
/// dock/undock messages of the hosting window in order to make
/// the tips look as if they are attached to the window. The tooltip
/// windows are toplevel windows just like their hosting windows. 
/// This means quite a bit is also done to make sure that the tips always
/// appear in front of their parent window, and never jump in front
/// of another application window that is on top of their parent
/// window.  The details of managing this feature are in some cases platform
/// dependent.
class NCBI_GUIWIDGETS_WX_EXPORT CStickyTooltipHandler                                
                                : public wxTimer
                                , public IGlEventHandler
                                , public ILinkEventHandler
{
public:
    CStickyTooltipHandler();
    virtual ~CStickyTooltipHandler();

    // initialize support for menu commands (popup on tip windows)
    static void RegisterCommands();

    virtual void    SetHost(IStickyTooltipHandlerHost* host) { m_Host = host; }

    /// @name IGlEventHandler implementaion
    /// @{
    virtual void    SetPane(CGlPane* pane) { m_Pane = pane; }
    virtual wxEvtHandler*    GetEvtHandler() { return this; }
    /// @}

    /// @name ILinkEventHandler implementaion
    /// @{
    virtual bool OnLinkClicked(const wxString &href) { _ASSERT(m_Host); return m_Host->TTHH_ProcessURL(href); }
    /// @}

    /// Add tool tip to current window
    virtual void AddTip(CTooltipFrame* tt);
    /// Add an unpinned tip to the window. Since there can be only one,
    /// delete the current upinned tip (m_UnpinnedTip) if any.
    virtual void AddUnpinned(CTooltipFrame* tt);
    /// Take the current upinned tip and pin it (add it to m_ToolTips)
    virtual void PinUnpinned(CTooltipFrame* tt);
    /// Remove a tip from m_ToolTips, and set m_UnpinnedTip to the tip
    virtual void UnpinPinned(CTooltipFrame* tt);
    /// Delete the current unpinned tip (if any)
    virtual void ClearUnpinned();

    /// Move all the tooltip windows along with the parent
    virtual void ParentMove(const wxPoint& screen_pos);
    /// Called when parent is unhidden to show all tips
    virtual void ParentShow(bool show);
    /// Clips all tip windows to parent window
    virtual void Resize();
    /// Determines which tip mouse is on (if any) and brings it to front
    virtual void OnTimer(wxTimerEvent& event);

    /// Hides tips and then copies them from m_ToolTips to m_SuspendedTips -
    /// called before docking starts.
    virtual void Suspend();
    /// Recreates suspended tips (old tips are deleted).  
    virtual void ReCreate(int dock_count = 0);
    /// Currently does nothing.
    virtual void OnSetFocus();

    /// Checks if a new tip should be popped up for the current location
    void CheckForNewTip(wxPoint pos);

    /// Commands propogated from parent windows to initiate move, resize,
    /// docking (suspend/recreate), hide, and show of current tips
    void OnMotion(wxMouseEvent& event);
    void OnRightMouseDown(wxMouseEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnLeaveWindow(wxMouseEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnWindowMoveCmd(wxCommandEvent& event);
    void OnSuspendCmd(wxCommandEvent& event);
    void OnReCreateCmd(wxCommandEvent& event);
    void OnMainWindowDockCmd(wxCommandEvent& event);
    void OnActivateCmd(wxCommandEvent& event);
    void OnWindowShowCmd(wxCommandEvent& event);
    void OnWindowHideCmd(wxCommandEvent& event);
    void OnTipRelease(wxCommandEvent& event);
    void OnTipMove(wxCommandEvent& event);

    /// Commands from buttons on individual tips
    void OnCloseTip(wxCommandEvent& event);
    void OnPinTip(wxCommandEvent& event);

    /// Any command that updates tip position should use this to keep
    /// the list of user-accessible tip information up-to-date.
    void UpdateTips();

    // wxTimer function
    void Notify();

    float& GetMoveThreshold() { return m_MouseMoveThreshold; }
    float& GetUnpinnedMoveThreshold() { return m_UnpinnedMouseMoveThreshold; }
    int& GetClearUnpinnedDelay() { return m_ClearUnpinnedDelay; }
    float& GetPopupDistance() { return m_PopupDistance; }
    int& GetTipPopupDelay() { return m_TipPopupDelay; }
    
protected:
    bool checkChildFocus(wxWindow* fwin, wxWindow* checkwin);

    /// Host window for tips
    IStickyTooltipHandlerHost* m_Host;   

    /// At a given time, only 1 tip can be unpinned
    CTooltipFrame* m_UnpinnedTip;
    /// position of mouse when need for new tip indicated
    wxPoint m_TipInitPos;

    /// If the mouse pointer is inside the boundries of a tip,that tip
    /// is m_ActiveTip.  Otherwise nULL
    CTooltipFrame* m_ActiveTip;
    /// Timer to watch mouse and change active tip as mouse moves
    wxTimer m_Timer;
    /// Tracks suspend count for window for debugging
    int m_SuspendCount;
    /// Timer to clear away unpinned tip if mouse starts moving
    wxTimer m_TimerClearUnpinned;

    /// All tooltips for current window (m_Host)
    std::vector<CTooltipFrame*>  m_ToolTips;
    /// Hidden tooltips (tips are all hidden when window is being (un)docked)
    std::vector<CTooltipFrame*>  m_SuspendedTips;

    /// Mouse move threshold for disabling tip before it pops up
    float m_MouseMoveThreshold;
    /// Mouse move threshold for disabling tip after it pops up
    float m_UnpinnedMouseMoveThreshold;
    /// When the mouse starts moving after a tip pops up, this is the delay
    /// until the tip is removed (mouse must get inside tip before this time)
    int m_ClearUnpinnedDelay;
    /// Distance from tip that the tooltip pops up
    float m_PopupDistance;
    /// Delay between seeing the need for a tip and displaying it
    int   m_TipPopupDelay;

    CGlPane*  m_Pane;

    DECLARE_EVENT_TABLE()
};

END_NCBI_SCOPE

#endif // GGUI_WIDGETS_WX___STICKY_TOOL_HANDLER__HPP
