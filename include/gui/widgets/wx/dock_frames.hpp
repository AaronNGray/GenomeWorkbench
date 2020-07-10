#ifndef GUI_WX_DEMO___DOCK_FRAMES__HPP
#define GUI_WX_DEMO___DOCK_FRAMES__HPP

/*  $Id: dock_frames.hpp 43814 2019-09-05 16:11:53Z katargir $
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
 *  Dock Marker Window and Floating Frame used by Dock Manager. 
 */

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/dock_window.hpp>


#include <wx/platform.h>
#include <wx/frame.h>

#if defined( __WXMSW__ ) || defined( __WXMAC__ ) ||  defined( __WXGTK__ )
    #include "wx/minifram.h"
#endif

BEGIN_NCBI_SCOPE

class CDockManager;
class CDockContainer;

///////////////////////////////////////////////////////////////////////////////
/// CDockMarkerWindow - top level window with docking markers displayed
/// on top of the application window. This window consists of 5 regions
/// (4 rectangles and 1 cross-like). Rectangular regions represent marker for
/// docking a window into the Root container (left, right, top, bottom).
/// The central region contains 5 markers for docking a window inside the
/// Target Window (a container inside a Dock Container).

class NCBI_GUIWIDGETS_WX_EXPORT CDockMarkerWindow
    : public wxTopLevelWindow
{
    typedef wxTopLevelWindow TParent;
public:
    CDockMarkerWindow(CDockContainer& doc_cont, wxWindow* parent);
    ~CDockMarkerWindow();

    // Sets the rectangle for the Target Window (in screen coordinates)
    void    SetTargetRect(const wxRect& root_rc, const wxRect& target_rc);

    void    OnPaint(wxPaintEvent& event);

    /// determines whether a given position is inside one of the 9 markers
    /// and returns the corresponding dock effect
    virtual EDockEffect HitTest(const wxPoint& screen_pt);
protected:
    void    x_InitBitmaps();
    bool    x_UpdateLayout(const wxRect& root_rc, const wxRect& target_rc);
    void    x_DrawButton(wxDC& dc, const wxRect& btn_rc, int index);
    void    x_SetShape();

    DECLARE_EVENT_TABLE();

protected:
    CDockContainer& m_DockContainer;
    wxBitmap    m_Bitmaps[5];
    wxColour    m_BtnFrameColor;
    wxColour    m_BtnFillColor;
    bool        m_SetShape;
    wxRegion    m_ClipRegion;

public:
    wxRect  m_RootRect;
    wxRect  m_TargetRect;

    // Rectangles for the 9 Markers

    wxRect  m_LeftRect;
    wxRect  m_RightRect;
    wxRect  m_TopRect;
    wxRect  m_BottomRect;
    wxRect  m_CenterRect;

    wxRect  m_CenterLeftRect;
    wxRect  m_CenterRightRect;
    wxRect  m_CenterTopRect;
    wxRect  m_CenterBottomRect;
    wxRect  m_CenterTabRect;
};


///////////////////////////////////////////////////////////////////////////////
/// CFloatingFrame - a top level floating frame that can host a Dock Container.

#if defined( __WXMSW__ )
    #define CFloatingFrameBaseClass wxFrame
#elif defined( __WXMAC__ )
    #define CFloatingFrameBaseClass wxMiniFrame
#elif defined( __WXGTK__ )
    #define CFloatingFrameBaseClass wxMiniFrame
#else
    #define CFloatingFrameBaseClass wxFrame
#endif


class NCBI_GUIWIDGETS_WX_EXPORT     CFloatingFrame
    : public CFloatingFrameBaseClass,
      public IDockableWindow
{
public:
    CFloatingFrame(CDockManager& manager,
                     wxWindow* parent,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize);
    virtual ~CFloatingFrame();

    virtual CDockContainer*   GetDockContainer();
    virtual void    SetDockContainer(CDockContainer* dock_cont);

    virtual bool Destroy();

#ifdef __WXMSW__
    virtual WXDWORD MSWGetStyle(long flags, WXDWORD *exstyle = NULL) const;
#endif

protected:
    virtual void OnMoveStart();
    virtual void OnMoving();

    virtual void OnMoveFinished(bool drop);

    void    OnClose(wxCloseEvent& event);
    void    OnMovingEvent(wxMoveEvent& event);
    void    OnIdle(wxIdleEvent& event);
    void    OnActivate(wxActivateEvent& event);
    void    OnMotion(wxMouseEvent& evt);
    void    OnKeyDown(wxKeyEvent& event);
    void    OnKeyUp(wxKeyEvent& event);

    static bool isMouseDown();

private:
    CDockManager*     m_DockManager;
    CDockContainer*   m_DockContainer;

    // mouse shift relative to the frame position at the time of the first wmMoveEvent
    wxPoint     m_OrigMouseShift;

    bool m_solid_drag;          // true if system uses solid window drag
    bool m_moving;

    wxRect m_last_rect;
    wxRect m_last2_rect;
    wxRect m_last3_rect;
    wxSize m_last_size;
#ifdef __WXOSX_COCOA__
    wxPoint m_move_start_mouse_pos;
    wxPoint m_prev_mouse_pos;
    bool    m_titlebar_move;
#endif

    DECLARE_EVENT_TABLE()
    DECLARE_CLASS(CFloatingFrame)
};


END_NCBI_SCOPE


#endif  // GUI_WX_DEMO___DOCK_FRAMES__HPP
