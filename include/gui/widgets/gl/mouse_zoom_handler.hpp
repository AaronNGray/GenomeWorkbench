#ifndef __GUI_WIDGETS_GL___MOUSE_ZOOM_HANDLER__HPP
#define __GUI_WIDGETS_GL___MOUSE_ZOOM_HANDLER__HPP

/*  $Id: mouse_zoom_handler.hpp 42213 2019-01-11 21:00:33Z katargir $
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

#include <corelib/ncbistl.hpp>

#include <gui/widgets/gl/ievent_handler.hpp>

#include <gui/utils/rgba_color.hpp>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// Interface IMouseZoomHandlerHost represents a context in which CMouseZoomHandler
/// functions.

class NCBI_GUIWIDGETS_GL_EXPORT IMouseZoomHandlerHost

{
public:
    enum EScaleType {
        eMin,       /// minimal scale
        eCurrent,   /// current scale
        eMax        /// maximal scale
    };

    virtual TModelUnit  MZHH_GetScale(EScaleType type) = 0;
    virtual void        MZHH_SetScale(TModelUnit scale, const TModelPoint& point) = 0;
    virtual void        MZHH_ZoomPoint(const TModelPoint& point, TModelUnit factor) = 0;
    virtual void        MZHH_ZoomRect(const TModelRect& rc) = 0;
    virtual void        MZHH_Scroll(TModelUnit d_x, TModelUnit d_y) = 0;

    // indicates that an operation has ended and results can be posted to the history
    virtual void        MZHH_EndOp() = 0;

    /// converts window coord to Viewport coord
    virtual TVPUnit     MZHH_GetVPPosByY(int y) const = 0;

    virtual ~IMouseZoomHandlerHost()    {}
};


////////////////////////////////////////////////////////////////////////////////
/// CMouseZoomHandler provides support for mouse-driven zoom and pan operations.
class NCBI_GUIWIDGETS_GL_EXPORT CMouseZoomHandler :
            public wxEvtHandler,
            public IGlEventHandler
{
    DECLARE_EVENT_TABLE()
public:
    enum    EMode   {
        eHorz,
        eVert,
        e2D
    };

    enum    EPanMode   {
        eLmouse,
        ePkey
    };

    CMouseZoomHandler();
    virtual ~CMouseZoomHandler();

    virtual void    SetMode(EMode mode);
    virtual void    SetHost(IMouseZoomHandlerHost* pHost);
    virtual IGenericHandlerHost*    GetGenericHost();
    virtual void    Render(CGlPane& Pane);
    virtual void    SetPanMode(EPanMode mode) { m_PanMode = mode; }
    virtual EPanMode GetPanMode() const { return m_PanMode; }
    virtual void    SetPanOnMouseDown(bool b) { m_PanOnMouseDown = b; }
    virtual bool    IsScaleMode() const { return (m_State==eScale || m_State==eReadyScale); }

    /// @name IGlEventHandler implementaion
    /// @{
    virtual void    SetPane(CGlPane* pane);
    virtual wxEvtHandler*    GetEvtHandler();
    /// @}

    /// @name wxWidgets event handling
    /// @{
    void    OnLeftDown(wxMouseEvent& event);
    void    OnLeftUp(wxMouseEvent& event);
    void    OnMotion(wxMouseEvent& event);
    void    OnMouseWheel(wxMouseEvent& event);
    void    OnOtherMouseEvents(wxMouseEvent& event);
    void    OnMiddleDown(wxMouseEvent& event);
    void    OnMiddleUp(wxMouseEvent& event);
    void    OnMouseCaptureLost(wxMouseCaptureLostEvent& evt);
    void    OnKeyDown(wxKeyEvent& event);
    void    OnKeyUp(wxKeyEvent& event);
    void    OnKillFocus(wxFocusEvent& event);
    /// @}

protected:
    enum EKeyState {
        eDefaultState,
        eZoomState,
        ePanState,
        eZoomRectState
    };

    enum    EState {
        eIdle,
        eReadyScale,
        eReadyZoomRect,
        eReadyPan,
        eScale,
        eZoomRect,
        ePan
    };

    EKeyState   x_GetKeyState();
    EKeyState   x_GetKeyState(wxKeyEvent& event);

    void    x_OnCancel();

    /// signal handlers - functions doing the real job
    void    x_SwitchToReadyState(EState new_state, const wxPoint& ms_pos);
    void    x_SwitchToActiveState(EState state, const wxPoint& ms_pos);

    void    x_OnChangeScale(int d_y);
    void    x_OnEndScale(EState new_state);

    void    x_OnChangeZoomRectPan(const wxPoint& pos);
    void    x_OnEndZoomRect(EState new_state);
    void    x_OnEndPan(EState new_state);
    void    x_OnChangePan(const wxPoint& ms_pos);

protected:
    // helper functions
    void    x_OnSelectCursor(void);

    TModelUnit  x_ScaleToNorm(TModelUnit scale) const;
    TModelUnit  x_NormToScale(TModelUnit norm) const;
    int         x_NormToPixels(TModelUnit norm) const;

    void    x_RenderScale(CGlPane& pane);
    void    x_RenderZoomRect(CGlPane& pane);
    void    x_RenderPan(CGlPane& pane);

    void    x_DrawTicks(int center_x, int y, int tick_w);
    void    x_DrawMarker(bool fill, int x_c, int y_c, int half);

    const IMouseZoomHandlerHost*    x_GetHost(void) const {   return m_Host; }
    IMouseZoomHandlerHost*    x_GetHost(void) {   return m_Host; }

    void    x_CaptureMouse();
    void    x_ReleaseMouse();

protected:
    EMode   m_Mode;
    IMouseZoomHandlerHost*  m_Host;
    CGlPane*    m_Pane;

    EState   m_State;
    wxStockCursor m_CursorId;
    EPanMode m_PanMode;
    bool     m_PanOnMouseDown; // start pan on down instead of after moving.

    // Members used in "Scale" mode
    bool    m_Reversed;
    int     m_PixPerNorm;
    TModelUnit  m_MaxNorm;
    TModelUnit  m_MinNorm;
    TModelUnit  m_StartNorm;
    TModelUnit  m_CurrNorm; // current norm value

    wxPoint m_MarkerPos; // point where marker is drawn

    wxPoint m_CurrMouse; // current mouse position

    wxPoint m_MouseStart;
    wxPoint m_CurrPos;  // mouse position corresponding to m_CurrNurm;
    TModelPoint m_ptStart; // referrence point for Zoom

    bool    m_HasCapture; // indicated whether the handler has Mouse Capture on the host

    int m_WheelTotalShift;

    // scale in logarithmic units
    CRgbaColor    m_ScaleColor;
    CRgbaColor    m_TickColor;
    CRgbaColor    m_RectColor;
};


END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_GL___MOUSE_ZOOM_HANDLER__HPP
