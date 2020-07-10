#ifndef __GUI_WIDGETS_GL___MINIMAP_HANDLER__HPP
#define __GUI_WIDGETS_GL___MINIMAP_HANDLER__HPP

/*  $Id: minimap_handler.hpp 42705 2019-04-04 16:54:47Z katargir $
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
 * Authors:  Vladimir Tereshkov, Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>

#include <gui/widgets/gl/ievent_handler.hpp>

#include <gui/opengl/glpane.hpp>
#include <gui/utils/rgba_color.hpp>

#include <math.h>

BEGIN_NCBI_SCOPE

class I3DTexture;

////////////////////////////////////////////////////////////////////////////////
/// Interface IMinimapHandlerHost represents a context in which CMinimapHandler
/// functions.

class IMinimapHandlerHost
{
public:
    enum EMMScaleType {
        eMin,       /// minimal scale
        eCurrent,   /// current scale
        eMax        /// maximal scale
    };

    virtual void        MMHH_Scroll(TModelUnit d_x, TModelUnit d_y) = 0;
    virtual TModelUnit  MMHH_GetScale(EMMScaleType type) = 0;

    // indicates that an operation has ended and results can be posted to the history
    virtual void        MMHH_EndOp() = 0;

    /// converts window coord to Viewport coord
    virtual TVPUnit     MMHH_GetVPPosByY(int y) const = 0;

    /// gets a texture and its coordinate limits for the minmap
    virtual I3DTexture* MMHH_GetTexture(float& /*xcoord_limit*/,
                                        float& /*ycoord_limit*/) { return NULL; }

    virtual ~IMinimapHandlerHost()    {}
};


////////////////////////////////////////////////////////////////////////////////
/// CMinimapHandler provides support for mouse-driven zoom and pan operations.
class NCBI_GUIWIDGETS_GL_EXPORT CMinimapHandler :
        public wxEvtHandler,
        public IGlEventHandler
{
    DECLARE_EVENT_TABLE()
public:
    CMinimapHandler();
    virtual ~CMinimapHandler();

    /// @name IGlEventHandler implementaion
    /// @{
    virtual void    SetPane(CGlPane* pane);
    virtual wxEvtHandler*   GetEvtHandler();
    /// @}

    void    SetHost(IMinimapHandlerHost* pHost);
    IGenericHandlerHost*    GetGenericHost();
    void    Render(CGlPane& Pane);

    /// IGlEventHandler implementation.
    void    OnLeftDown(wxMouseEvent& event);
    void    OnLeftUp(wxMouseEvent& event);
    void    OnMotion(wxMouseEvent& event);
    void    OnMouseWheel(wxMouseEvent& event);
    void    OnMouseCaptureLost(wxMouseCaptureLostEvent& evt);
    void    OnKeyDown(wxKeyEvent& event);
    void    OnKeyUp(wxKeyEvent& event);

protected:
    enum EState {
        eIdle,
        eReadyMinimap,
        eMinimap
    };

    enum ESubstate {
        eNormal,
        eJumpTo,
        eCursor
    };

    typedef ESubstate TSubstate;

    /// signal handlers - functions doing the real job
    void    x_SwitchToReadyState(EState new_state, const wxPoint& ms_pos);
    void    x_SwithToActiveState(EState state, const wxPoint& ms_pos);

    TSubstate x_GetSubstate(const wxPoint& ms_pos);

    void    x_OnChangeZoomRectPan(int x, int y);
    void    x_OnEndMinimap(EState new_state);
    void    x_OnChangePan(const wxPoint& ms_pos);

    void    x_OnSelectCursor(const wxPoint& ms_pos);

    void    x_RenderMinimap(CGlPane& pane);

    const IMinimapHandlerHost*    x_GetHost(void) const {   return m_Host; }
    IMinimapHandlerHost*    x_GetHost(void) {   return m_Host; }

    TModelUnit  x_ScaleToNorm(TModelUnit scale) const;
    TModelUnit  x_NormToScale(TModelUnit norm) const;

protected:
    I3DTexture * m_pTexture;
    float        m_XCoordLimit;
    float        m_YCoordLimit;
    IMinimapHandlerHost*  m_Host;
    CGlPane*    m_Pane;
    EState      m_State;
    TSubstate   m_Substate;
    int m_CursorId;

    TVPRect m_Minimap;
    TVPRect m_MiniCursor;

    // Members used in "Scale" mode
    int m_PixPerNorm;
    TModelUnit  m_MaxNorm;
    TModelUnit  m_MinNorm;
    TModelUnit  m_StartNorm;
    TModelUnit  m_CurrNorm; // current norm value

    wxPoint m_MarkerPos; // point where marker is drawn

    wxPoint m_MouseStart;
    wxPoint m_CurrPos; // mouse position corresponding to m_CurrNurm;
    wxPoint m_LastMouse;  // Tracks current mouse position via OnMotion
    TModelPoint m_ptStart; // referrence point for Zoom


    int m_WheelTotalShift;

    int m_InnerRectX, m_InnerRectY;
    int m_InnerRectW, m_InnerRectH;

    double m_scalex, m_scaley;

    // scale in logarithmic units
    CRgbaColor    m_ScaleColor;
    CRgbaColor    m_TickColor;
    CRgbaColor    m_RectColor;
};


END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_GL___MINIMAP_HANDLER__HPP
