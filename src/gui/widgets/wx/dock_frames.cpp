/*  $Id: dock_frames.cpp 43821 2019-09-05 19:43:37Z katargir $
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


#include <gui/widgets/wx/dock_frames.hpp>

#include <gui/widgets/wx/dock_manager.hpp>
#include <gui/widgets/wx/dock_container.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/gl/gl_widget_base.hpp>

#include <wx/dcclient.h>
#include <wx/settings.h>

#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
#include <Carbon/Carbon.h>
#endif


BEGIN_NCBI_SCOPE


BEGIN_EVENT_TABLE(CDockMarkerWindow, CDockMarkerWindow::TParent)
    EVT_PAINT(CDockMarkerWindow::OnPaint)
END_EVENT_TABLE()

static long kMarkersStyle = wxSTAY_ON_TOP | wxFRAME_TOOL_WINDOW |
                            wxFRAME_NO_TASKBAR | wxBORDER_NONE | wxFRAME_SHAPED;

CDockMarkerWindow::CDockMarkerWindow(CDockContainer& dock_cont, wxWindow* parent)
:   m_DockContainer(dock_cont),
    m_BtnFrameColor(128, 128, 128),
    m_BtnFillColor(224, 224, 224),
    m_SetShape(true)
{
    Create(parent, wxID_ANY, wxT("Dock Markers Window"),
           wxPoint(6, 6), wxSize(666, 666), kMarkersStyle);

    // debugging code for GTK
    //g_signal_connect (m_widget, "size_allocate", G_CALLBACK (gtk_frame_size_callback), this);

    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    Enable(false);

    x_InitBitmaps();
}

CDockMarkerWindow::~CDockMarkerWindow()
{
    // prevent main window activation
    SetWindowStyleFlag(GetWindowStyleFlag()&~wxFRAME_FLOAT_ON_PARENT);
}

void CDockMarkerWindow::x_InitBitmaps()
{
    static bool initialized = false;

    wxFileArtProvider* provider = GetDefaultFileArtProvider();

    if( ! initialized)  {
        // Register images only once
        provider->RegisterFileAlias(wxT("wm_dock_center"), wxT("wm_dock_center.png"));
        provider->RegisterFileAlias(wxT("wm_dock_left"), wxT("wm_dock_left.png"));
        provider->RegisterFileAlias(wxT("wm_dock_right"), wxT("wm_dock_right.png"));
        provider->RegisterFileAlias(wxT("wm_dock_top"), wxT("wm_dock_top.png"));
        provider->RegisterFileAlias(wxT("wm_dock_bottom"), wxT("wm_dock_bottom.png"));

        initialized = true;
    }

    m_Bitmaps[0] = provider->GetBitmap(wxT("wm_dock_center"));
    m_Bitmaps[1] = provider->GetBitmap(wxT("wm_dock_left"));
    m_Bitmaps[2] = provider->GetBitmap(wxT("wm_dock_right"));
    m_Bitmaps[3] = provider->GetBitmap(wxT("wm_dock_top"));
    m_Bitmaps[4] = provider->GetBitmap(wxT("wm_dock_bottom"));
}


static int kBtnLength = 32;
static int kBtnWidth = 29;
static int kCenterSize = 88;
static int kCenterBtnSize = 25;
static int kCenterOffset = 4;
static int kBtnSpace = 10;

void CDockMarkerWindow::x_SetShape()
{
    if (!m_SetShape)
        return;

    // make a cross-like region from the central pane

    // horizontal strip
    wxRect rc(m_CenterRect);
    int c_y = rc.y + rc.height / 2;
    rc.y = c_y - kBtnWidth / 2;
    rc.height = kBtnWidth - 1;
     
    // create a region that combines all rectangles
    wxRegion rgn(rc);

    // vertical strip
    rc = m_CenterRect;
    int c_x = rc.x + rc.width / 2;
    rc.x = c_x - kBtnWidth / 2;
    rc.width = kBtnWidth - 1;
    rgn.Union(rc);

    // add regions for the side markers
    rgn.Union(m_LeftRect);
    rgn.Union(m_RightRect);
    rgn.Union(m_TopRect);
    rgn.Union(m_BottomRect);

    rgn.Offset(-m_RootRect.GetPosition());

    m_ClipRegion = rgn;

    m_SetShape = false;

    SetShape(rgn);
}

void CDockMarkerWindow::SetTargetRect(const wxRect& root_rc, const wxRect& target_rc)
{
    if(m_RootRect != root_rc)   {
        wxSize size = ClientToWindowSize(root_rc.GetSize());
        SetSize(wxRect(root_rc.GetPosition(), size));
    }

    bool update = x_UpdateLayout(root_rc, target_rc);
    if(update)  {
        m_SetShape = true;

        /// Cocoa doesn't refresh properly (visual artifacts) if we do not
        // hide before (re)setting the shape.
#ifdef __WXOSX_COCOA__
        Hide();         
#endif

        if( ! IsShown())    {
            // Mac (cocoa) is unhappy (at least in debug mode) if Show() is called before SetShape.
#ifdef __WXOSX_COCOA__
    	    x_SetShape();
            m_SetShape = true;
#endif
            Show();
        }
        Refresh();
    }
    //WM_POST("CDockMarkerWindow::SetTargetRect m_CenterRect  " <<  sToString(m_CenterRect));
}


bool CDockMarkerWindow::x_UpdateLayout(const wxRect& root_rc, const wxRect& target_rc)
{
    //wxRect rc = GetRect();
    //WM_POST(" CDockMarkerWindow::x_UpdateLayout()  GetRect() " << sToString(rc));

    static const wxSize sz_btn_h(kBtnLength, kBtnWidth);
    static const wxSize sz_btn_v(kBtnWidth, kBtnLength);

    bool root_changed = (m_RootRect != root_rc);

    if(root_changed)   {
        m_RootRect = root_rc;

        // update side buttons
        int kRootMinSize = kBtnLength * 2 + kBtnWidth * 4;

        // layout side buttons
        bool side_visible = (root_rc.height >= kRootMinSize  &&  root_rc.width >= kRootMinSize);
        if(side_visible)   {
            int c_x = root_rc.x + root_rc.width / 2;
            int c_y = root_rc.y + root_rc.height / 2;

            m_LeftRect.x = root_rc.GetLeft() + kBtnSpace;
            m_LeftRect.y = c_y - kBtnWidth / 2;
            m_LeftRect.SetSize(sz_btn_h);

            m_RightRect = m_LeftRect;
            m_RightRect.x = root_rc.GetRight() - kBtnSpace - kBtnLength;

            m_TopRect.x = c_x - kBtnWidth / 2;
            m_TopRect.y = root_rc.GetTop() + kBtnSpace;
            m_TopRect.SetSize(sz_btn_v);

            m_BottomRect = m_TopRect;
            m_BottomRect.y = root_rc.GetBottom() - kBtnSpace - kBtnLength;
        } else {
            m_LeftRect = m_RightRect = m_TopRect = m_BottomRect = m_CenterRect = wxRect(0, 0, -1, -1);
        }
    }

    bool center_visible = (target_rc.width >= kCenterSize  && target_rc.height >= kCenterSize);

    bool target_changed = (m_TargetRect != target_rc);
    if(target_changed)  {
        m_TargetRect = target_rc;
        // update central pane
        if(center_visible) {
            // central pane is visible
            m_CenterRect.SetSize(wxSize(kCenterSize, kCenterSize));
            m_CenterRect = m_CenterRect.CenterIn(target_rc);
        } else {
            m_CenterRect = wxRect(0, 0, -1, -1);
        }
    }

    if(root_changed || target_changed)  {
        // resolve possible clashes between side buttons and central pane
        // hide clashing side buttons
        wxSize sz_empty(-1, -1);

        // hide some of the buttons if there is not enough space
        m_LeftRect.SetSize(m_CenterRect.Intersects(m_LeftRect) ? sz_empty : sz_btn_h);
        m_RightRect.SetSize(m_CenterRect.Intersects(m_RightRect) ? sz_empty : sz_btn_h);
        m_TopRect.SetSize(m_CenterRect.Intersects(m_TopRect) ? sz_empty : sz_btn_v);
        m_BottomRect.SetSize(m_CenterRect.Intersects(m_BottomRect) ? sz_empty : sz_btn_v);

        // layout areas inside the central pane
        if(center_visible)  {
            m_CenterTabRect.SetSize(wxSize(kCenterBtnSize, kCenterBtnSize));
            m_CenterTabRect = m_CenterTabRect.CenterIn(m_CenterRect);

            // find the center of m_CenterRect
            int c_x = m_CenterRect.x + m_CenterRect.width / 2;
            int c_y = m_CenterRect.y + m_CenterRect.height / 2;

            m_CenterLeftRect.x = m_CenterRect.x + kCenterOffset;
            m_CenterLeftRect.y = c_y - kBtnWidth / 2;
            m_CenterLeftRect.SetSize(sz_btn_h);

            m_CenterRightRect.x = m_CenterRect.GetRight() - kBtnLength - kCenterOffset;
            m_CenterRightRect.y = m_CenterLeftRect.y;
            m_CenterRightRect.SetSize(sz_btn_h);

            m_CenterTopRect.x = c_x - kBtnWidth / 2;
            m_CenterTopRect.y = m_CenterRect.y + kCenterOffset;
            m_CenterTopRect.SetSize(sz_btn_v);

            m_CenterBottomRect.x = m_CenterTopRect.x;
            m_CenterBottomRect.y = m_CenterRect.GetBottom() - kBtnLength - kCenterOffset;
            m_CenterBottomRect.SetSize(sz_btn_v);
        }
    }
    return root_changed || target_changed;
}


void CDockMarkerWindow::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);

    // set origin so that we can draw in screen coordinates
    wxPoint orig = ClientToScreen(wxPoint(0,0));

    if (orig != m_RootRect.GetPosition()) {
        Move(GetPosition() + m_RootRect.GetPosition() - orig);
    }

    x_SetShape();

#ifdef __WXOSX_COCOA__
    // On cocoa the central four-arrow drop hint may have the four non-button corners show up black if
    // the clip region is not used to restrict the drawing.  This has not happened consistently 
    // however, and seems to be similar to a wxWidgets defect that was fixed for 2.9.3, #13340

    // This has now been fixed in 2.9.3 (and leaving this in now causes corners to be clipped)
    //dc.SetDeviceClippingRegion(m_ClipRegion);
#endif

    dc.SetDeviceOrigin(-m_RootRect.GetPosition().x, -m_RootRect.GetPosition().y);

    if( ! m_CenterRect.IsEmpty()) {
        x_DrawButton(dc, m_CenterRect, 0);
    }

    x_DrawButton(dc, m_LeftRect, 1);
    x_DrawButton(dc, m_RightRect, 2);
    x_DrawButton(dc, m_TopRect, 3);
    x_DrawButton(dc, m_BottomRect, 4);
}

void CDockMarkerWindow::x_DrawButton(wxDC& dc, const wxRect& btn_rc, int bmp_index)
{
    if( ! btn_rc.IsEmpty())    {
        wxBitmap& bmp = m_Bitmaps[bmp_index];
        if(bmp.IsOk())  {
            dc.DrawBitmap(bmp, btn_rc.x, btn_rc.y);
        } else {
            // error - no bitmap, draw a simple rectangle
            wxPen pen(m_BtnFrameColor);
            dc.SetPen(pen);
        
            wxBrush brush(m_BtnFillColor);
            dc.SetBrush(brush);

            dc.DrawRectangle(btn_rc);
        }
    }
}


EDockEffect CDockMarkerWindow::HitTest(const wxPoint& screen_pt)
{
    if(m_LeftRect.Contains(screen_pt))  {
        return eSplitLeft;
    }
    if(m_RightRect.Contains(screen_pt))  {
        return eSplitRight;
    }
    if(m_TopRect.Contains(screen_pt))  {
        return eSplitTop;
    }
    if(m_BottomRect.Contains(screen_pt))  {
        return eSplitBottom;
    }

    if(m_CenterRect.Contains(screen_pt))  {
        if(m_CenterLeftRect.Contains(screen_pt))  {
            return eSplitTargetLeft;
        }
        if(m_CenterRightRect.Contains(screen_pt))  {
            return eSplitTargetRight;
        }
        if(m_CenterTopRect.Contains(screen_pt))  {
            return eSplitTargetTop;
        }
        if(m_CenterBottomRect.Contains(screen_pt))  {
            return eSplitTargetBottom;
        }
        if(m_CenterTabRect.Contains(screen_pt))  {
            return ePutInTab;
        }
    }
    return eNoEffect;
}



///////////////////////////////////////////////////////////////////////////////
/// CFloatingFrame

BEGIN_EVENT_TABLE(CFloatingFrame, CFloatingFrameBaseClass)
    EVT_SIZE(CFloatingFrame::OnSize)
    EVT_MOVE(CFloatingFrame::OnMovingEvent)
    EVT_MOVING(CFloatingFrame::OnMovingEvent)
    EVT_CLOSE(CFloatingFrame::OnClose)
    EVT_IDLE(CFloatingFrame::OnIdle)
    EVT_ACTIVATE(CFloatingFrame::OnActivate)
    EVT_MOTION(CFloatingFrame::OnMotion)
    EVT_KEY_DOWN(CFloatingFrame::OnKeyDown)
    EVT_KEY_UP(CFloatingFrame::OnKeyUp)
END_EVENT_TABLE()


IMPLEMENT_CLASS(CFloatingFrame, CFloatingFrameBaseClass) //TODO

const static long kFloatFrameStyle =
#if defined(__WXMSW__)
    wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX |
    wxFRAME_FLOAT_ON_PARENT | wxCLIP_CHILDREN ;
#elif defined(__WXOSX_COCOA__)
    wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX |
    wxFRAME_NO_TASKBAR | wxFRAME_FLOAT_ON_PARENT ;
#else
    wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX |
    wxFRAME_NO_TASKBAR;
#endif

CFloatingFrame::CFloatingFrame(CDockManager& manager,
                                   wxWindow* parent,
                                   const wxPoint& pos,
                                   const wxSize& size)
:   CFloatingFrameBaseClass(),
    m_DockManager(&manager),
    m_DockContainer(NULL)
{
    // window shall not be created by base class, we must create here so that
    // our version of MSWGetStyle() is called and we het a chance to modify
    // the styles
    Create(parent, wxID_ANY, wxEmptyString, pos, size, kFloatFrameStyle);

    m_moving = false;
    m_solid_drag = true;
#ifdef __WXOSX_COCOA__
    m_titlebar_move = false;
#endif

    // find out if the system supports solid window drag.
    // on non-msw systems, this is assumed to be the case
#ifdef __WXMSW__
    BOOL b = TRUE;
    SystemParametersInfo(38 /*SPI_GETDRAGFULLWINDOWS*/, 0, &b, 0);
    m_solid_drag = b ? true : false;
#endif

#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
    // On mac, floating windows that display sticky tool tips have to have
    // their window class set to kFloatingWindowClass (with some additional 
    // parent/child hierarchy stuff).  So we have to do it for all floating 
    // otherwise windows with tip support will always be on top of windows 
    // without (on mac)
    WindowGroupRef group_class = GetWindowGroupOfClass(kFloatingWindowClass);	
    SetWindowGroup((WindowRef)MacGetTopLevelWindowRef(), group_class);
#endif

    SetExtraStyle(wxWS_EX_PROCESS_IDLE);

    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();

    cmd_reg.ApplyAccelerators( this );
}


CFloatingFrame::~CFloatingFrame()
{
}


void CFloatingFrame::SetDockContainer(CDockContainer* dock_cont)
{
    m_DockContainer = dock_cont;
}


CDockContainer* CFloatingFrame::GetDockContainer()
{
    return m_DockContainer;
}

bool CFloatingFrame::Destroy()
{
    m_DockManager = NULL;
    m_DockContainer = NULL;

    return CFloatingFrameBaseClass::Destroy();
}


#ifdef NCBI_OS_MSWIN
// we override these functions to modify windows styles before the window is
// created (WS_POPUP cannot be easily set after a window has been created).
WXDWORD CFloatingFrame::MSWGetStyle(long flags, WXDWORD *exstyle ) const
{
    WXDWORD ms_styles = CFloatingFrameBaseClass::MSWGetStyle(flags, exstyle);
    //ms_styles |= WS_POPUP;
    //if(exstyle)    {
        //*exstyle |= WS_EX_OVERLAPPEDWINDOW;
    //}
    return ms_styles;
}
#endif


void CFloatingFrame::OnClose(wxCloseEvent& evt)
{
    if(m_DockManager)   {
        m_DockManager->OnFrameClosePressed(this);
    }
}

void CFloatingFrame::OnMovingEvent(wxMoveEvent& event)
{
    //WM_POST("CFloatingFrame::OnMovingEvent()");

// Cocoa doesn't send window-move events (EVT_MOVE/EVT_MOVING) when the user moves a window
// using the title bar.  Since the window manager does the move, it doesn't require the
// app to be 'active' so you never have frozen windows.  At the end of the move (when the
// mouse stops moving) we get an EVT_MOVE event.  To get around this for drag and drop (where
// we need interactive updates), we must patch wxWidgets so that we also get a start-moving
// event as an initial EVT_MOVE. After that we can just look at the mouse position updates
// to get the windows position.  The neecessary patch is:
/*
Index: osx/cocoa/nonownedwnd.mm
===================================================================
--- osx/cocoa/nonownedwnd.mm    (revision 69613)
+++ osx/cocoa/nonownedwnd.mm    (working copy)
@@ -260,6 +260,7 @@
 - (void)windowDidResignKey:(NSNotification *)notification;
 - (void)windowDidBecomeKey:(NSNotification *)notification;
 - (void)windowDidMove:(NSNotification *)notification;
+- (void)windowWillMove:(NSNotification *)notification;
 - (BOOL)windowShouldClose:(id)window;
 - (BOOL)windowShouldZoom:(NSWindow *)window toFrame:(NSRect)newFrame;
 
@@ -407,6 +408,19 @@
     }
 }
 
+- (void)windowWillMove:(NSNotification *)notification
+{
+    wxNSWindow* window = (wxNSWindow*) [notification object];
+    wxNonOwnedWindowCocoaImpl* windowimpl = [window WX_implementation];
+    if ( windowimpl )
+    {
+        wxNonOwnedWindow* wxpeer = windowimpl->GetWXPeer();
+        if ( wxpeer )
+            // Use a negative time to indicate this is the first move
+            wxpeer->HandleMoved(-1.0);
+    }
+}
+
*/


#ifdef __WXOSX_COCOA__
    if (event.GetTimestamp() < 0) {
        m_move_start_mouse_pos = ::wxGetMousePosition();
        m_titlebar_move = true;
    }
#endif

    // Let child windows know parent has moved (for sticky tooltips)
    BroadcastCommandToChildWindows(this, eCmdParentMove);

    if( ! m_moving) { // this is the first move
        //m_OrigMouseShift = ::wxGetMousePosition() - GetPosition();

        // Don't ask why, this is empirical knowledge.
        m_OrigMouseShift = ::wxGetMousePosition() - event.GetPosition();

        //_TRACE( "OrigMouseShift: ( " << m_OrigMouseShift.x << ", " << m_OrigMouseShift.y << " )" );

        //LOG_POST( Info << "Orig Pos: ( " << GetPosition().x << ", " << GetPosition().y << " )" );
        //LOG_POST( Info << "Orig Event pos: ( " << event.GetPosition().x << ", " << event.GetPosition().y << " )" );
    } else {
        //LOG_POST( Info << "----- Pos: ( " << GetPosition().x << ", " << GetPosition().y << " )" );
        //LOG_POST( Info << "Event pos: ( " << event.GetPosition().x << ", " << event.GetPosition().y << " )" );
    }


    if ( ! m_solid_drag)
    {
        // systems without solid window dragging need to be
        // handled slightly differently, due to the lack of
        // the constant stream of EVT_MOVING events
        if ( ! isMouseDown())   {
            //WM_POST("CFloatingFrame::OnMovingEvent() -  ! isMouseDown()");
            return;
        }
        OnMoveStart();
        OnMoving();
        m_moving = true;
        return;
    }

    wxRect win_rect = GetRect();

// Since cocoa doesn't update the windows rectangle (position) while moving via a tile-bar grab,
// we modify the position by adding mouse offset from the windows position at the beginning
// of the move
#ifdef __WXOSX_COCOA__
    wxPoint pos = ::wxGetMousePosition();
    wxPoint delta = pos - m_move_start_mouse_pos;
    win_rect.Offset(delta);
#endif

    if (win_rect == m_last_rect)    {
        //WM_POST("CFloatingFrame::OnMovingEvent() - win_rect == m_last_rect");
        return;
    }

    // skip the first move event
    if (m_last_rect.IsEmpty())  {
        m_last_rect = win_rect;
        //WM_POST("CFloatingFrame::OnMovingEvent() - skip the first move event");
        return;
    }

    // skip if moving too fast to avoid massive redraws and jumping hint windows
    // but not for cocoa since it sometimes has jumps due to too few events
#ifndef __WXOSX_COCOA__
    constexpr int kMaxMoveDelta = 30;

    if ((abs(win_rect.x - m_last_rect.x) > kMaxMoveDelta) ||
        (abs(win_rect.y - m_last_rect.y) > kMaxMoveDelta))
    {
        m_last3_rect = m_last2_rect;
        m_last2_rect = m_last_rect;
        m_last_rect = win_rect;
        //WM_POST("CFloatingFrame::OnMovingEvent() - skip if moving too fast ");
        return;
    }
#endif

    // prevent frame redocking during resize
    if (m_last_rect.GetSize() != win_rect.GetSize())
    {
        m_last3_rect = m_last2_rect;
        m_last2_rect = m_last_rect;
        m_last_rect = win_rect;
        //WM_POST("CFloatingFrame::OnMovingEvent() - prevent frame redocking during resize");
        return;
    }

    m_last3_rect = m_last2_rect;
    m_last2_rect = m_last_rect;
    m_last_rect = win_rect;

    if ( ! isMouseDown()) {
        //WM_POST("CFloatingFrame::OnMovingEvent() -  ! isMouseDown()");
        return;
    }

    if ( ! m_moving)    {
        OnMoveStart();
        m_moving = true;
    }

    if (m_last3_rect.IsEmpty()) {
        //WM_POST("CFloatingFrame::OnMovingEvent() - m_last3_rect.IsEmpty()");
        return;
    }

    OnMoving();
}


void CFloatingFrame::OnIdle(wxIdleEvent& event)
{
    if (m_moving)   {
 
        // Simulate window moving for Cocoa which (when using the title bar) does not normally send
        // moving events
#ifdef __WXOSX_COCOA__
        if (m_titlebar_move == true) {
            wxPoint pos = ::wxGetMousePosition();
            if (pos != m_prev_mouse_pos) {
                m_prev_mouse_pos = pos;
                wxMoveEvent dummy(pos, wxEVT_MOVING);
                OnMovingEvent(dummy);
            }
        }
#endif
        if ( ! isMouseDown()) {
            m_moving = false;
#ifdef __WXOSX_COCOA__
            m_titlebar_move = false;
#endif

            // if Esc has been pressed the offset most like is not the same,
            // this is an workaround for wxWidgets feature request #1755061
            wxPoint offset = ::wxGetMousePosition() - GetPosition();

            //_TRACE( Info << "Offset: ( " << offset.x << ", " << offset.y << " )" );

            int delta_x = (m_OrigMouseShift.x - offset.x);
            int delta_y = (m_OrigMouseShift.y - offset.y);
            int max_x = wxSystemSettings::GetMetric(wxSYS_DRAG_X) / 2;
            int max_y = wxSystemSettings::GetMetric(wxSYS_DRAG_Y) / 2;
            bool drop = abs(delta_x) <= max_x  &&  abs(delta_y) <= max_y;

            OnMoveFinished(drop);
        } else    {
            event.RequestMore();
        }
    }
}


void CFloatingFrame::OnMoveStart()
{
    //WM_POST("CFloatingFrame::OnMoveStart()");
    if(m_DockManager)   {
        m_DockManager->OnFloatingPaneBeginMove(*this, wxGetMousePosition());
    }
}


void CFloatingFrame::OnMoving()
{
    //WM_POST("CFloatingFrame::OnMoving()");
    if(m_DockManager)   {
        m_DockManager->OnFloatingPaneMoving(*this, wxGetMousePosition());
    }
}


void CFloatingFrame::OnMoveFinished(bool drop)
{
    //WM_POST("CFloatingFrame::OnMoveFinished()  drop " << drop);
    if(m_DockManager)   {
        wxPoint sc_mouse_pos = ::wxGetMousePosition();
        CDockManager::EDragResult result =
            drop ? CDockManager::eDrop : CDockManager::eCancel;
        m_DockManager->OnFloatingPaneEndMove(*this, result, sc_mouse_pos);
    }
}

void CFloatingFrame::OnActivate(wxActivateEvent& event)
{
    bool active = event.GetActive();
    //WM_POST("CFloatingFrame::OnActivate() " << this << " active " << active);

    // This messes up GTK which interleaves the activate message with the tool tip creation,
    // which causes the unpinned tip to be deleted.  Could make this GTK only, or fix the
    // problem with special processing...    (and windows...)
#if !defined(NCBI_OS_LINUX) && !defined(NCBI_OS_MSWIN)
    //_TRACE("Suspend/ReCreate all via Activate.");
    //BroadcastCommandToChildWindows(this, eCmdParentActivate);   
#endif

    if(m_DockManager)   {
        m_DockManager->OnFloatingFrameActivateEvent(this, active);
    }
    event.Skip();
}


void CFloatingFrame::OnMotion(wxMouseEvent& evt)
{
    //wxPoint pt = evt.GetPosition();
}


void CFloatingFrame::OnKeyDown(wxKeyEvent& event)
{
    m_DockContainer->GetDockManager().OnKeyDown(event);
}


void CFloatingFrame::OnKeyUp(wxKeyEvent& event)
{
    m_DockContainer->GetDockManager().OnKeyUp(event);
}


// utility function which determines the state of the mouse button
// (independent of having a wxMouseEvent handy) - ultimately a better
// mechanism for this should be found (possibly by adding the
// functionality to wxWidgets itself)
bool CFloatingFrame::isMouseDown()
{
    return wxGetMouseState().LeftIsDown();
}


END_NCBI_SCOPE
