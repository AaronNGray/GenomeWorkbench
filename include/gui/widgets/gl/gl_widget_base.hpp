#ifndef GUI_WIDGETS_GL___GLPANE_WIDGET__HPP
#define GUI_WIDGETS_GL___GLPANE_WIDGET__HPP

/*  $Id: gl_widget_base.hpp 42121 2018-12-21 21:49:09Z katargir $
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

#include <corelib/ncbiobj.hpp>
#include <corelib/ncbifloat.h>

#include <gui/widgets/gl/ievent_handler.hpp>
#include <gui/widgets/gl/tooltip_handler.hpp>
#include <gui/widgets/wx/sticky_tooltip_handler.hpp>
#include <gui/widgets/wx/3dcanvas.hpp>

#include <gui/opengl/gldlist.hpp>
#include <gui/opengl/glpane.hpp>

#include <gui/objutils/reg_settings.hpp>

#include <gui/utils/event_handler.hpp>
#include <gui/utils/view_event.hpp>

#include <wx/panel.h>

#define ncbi_round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

BEGIN_NCBI_SCOPE


/// CGlWidgetPane represent a window component residing in CGlWidgetBase
/// client area.
class NCBI_GUIWIDGETS_GL_EXPORT CGlWidgetPane :
    public C3DCanvas,
    public IGenericHandlerHost,
    public ITooltipHandlerHost,
    public IStickyTooltipHandlerHost
{
    DECLARE_EVENT_TABLE();
public:
    CGlWidgetPane(wxWindow* parent,
                  wxWindowID id,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  long style = 0);
    virtual ~CGlWidgetPane();

    /// returns size of the master pane in screen coordinates
    virtual TVPPoint GetPortSize(void) = 0;

    /// @name IGenericHandlerHost interface implementation
    /// @{
    virtual void    GHH_Redraw();
    virtual void    GHH_SetCursor(const wxCursor& cursor);
    virtual void    GHH_CaptureMouse();
    virtual void    GHH_ReleaseMouse();
    /// @}

    /// @name ITooltipHandlerHost implementation
    /// @{
    virtual bool    TC_NeedTooltip(const wxPoint & pt);
    virtual string  TC_GetTooltip(const wxRect & rect);
    virtual wxWindow*   TC_GetWindow();
    /// @}

    /// @name IStickyTooltipHandlerHost implementation
    /// @{
    virtual wxWindow*  TTHH_GetWindow() { return this; }
    virtual bool       TTHH_PopupMenuDisplayed() { return m_PopupMenuDisplayed; }
    /// @}


    /// @name wxWidgets event handlers
    /// @{
    void    OnEnterWindow(wxMouseEvent& event); 
    void    OnTimer(wxTimerEvent& event);
    void    OnMouseDown(wxMouseEvent& event);
    void    OnMouseUp(wxMouseEvent& event);
    void    OnMotion(wxMouseEvent& event);
    void    OnMouseWheel(wxMouseEvent& event);
    void    OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
    void    OnLeaveWindow(wxMouseEvent& event);
    void    OnKeyEvent(wxKeyEvent& event);
    void    OnSetFocus(wxFocusEvent& event);
    void    OnKillFocus(wxFocusEvent& event);
    void    OnSize(wxSizeEvent& event);    
    void    OnCmdEvent(wxCommandEvent& event);
	void    OnTipRemoved(wxCommandEvent& event);

    /// @}

    void    SetPopupMenuDisplayed(bool b) { m_PopupMenuDisplayed= b; }
    bool    GetPopupMenuDisplayed() const { return m_PopupMenuDisplayed; }
    void    DlgOverlayFix();
    void    CheckOverlayTimer();

protected:
    virtual int     x_GetAreaByWindowPos(const wxPoint& pos);
    virtual int     x_GetAreaByVPPos(TVPUnit vp_x, TVPUnit vp_y);

    // Return true if this control should grab focus when mouse enters it. Can
    // base decsion on what control currently has focus (this base class
    // implementation only takes focus from stickey tooltips)
    virtual bool    x_GrabFocus();

    /// @name IGlEventHandler management function
    /// @{
    struct  SHandlerRec
    {
        IGlEventHandler* m_pHandler;
        int         m_Area;
        CGlPane*    m_pPane;
    };

    bool    x_RegisterHandler(IGlEventHandler* handler, int area, CGlPane* pane, int index=-1);
    bool    x_UnregisterHandler(IGlEventHandler* handler);
    void    x_SetCurrHandler(SHandlerRec* rec);

    bool    x_Handlers_handle(wxEvent& event, int area, bool ignore_curr = true);
    bool    x_DispatchEventToHandler(wxEvent& event, SHandlerRec* rec);
    void    x_HandleAccels(wxKeyEvent& event);
    /// @}

protected:
    typedef list<SHandlerRec>    THandlerRecList;

    /// list of records for registered handlers
    THandlerRecList    m_lsHandlerRecs;

    /// pointer to record for last active handler
    SHandlerRec*       m_pCurrHandlerRec;

    /// Some handlers may need to know if popup (right click) menu is active
    bool               m_PopupMenuDisplayed;

    /// Windows vista bug screws up opengl after dlgs/popups displayed over window.
    /// Sometimes happens when mouse outside window so we reset after re-entering
    CStopWatch         m_EnterTimer;
    /// Need associated timer since some popups take a little time  to clear
    wxTimer            m_GLOverlapFixTimer;
    /// For windows (e.g. tree) that do not want to layout/update for the fake size event
    bool               m_PseudoSized;
};



/// this class converts model units to the integers distribited in [0, m_IntRange]
/// and vice versa
class CNormalizer
{
public:
    CNormalizer(double min, double max, int int_range)
        :   m_Min(min), m_Max(max), m_IntRange(int_range)   {}

    inline int RealToInt(double real)
    {
        double norm = (real - m_Min) / (m_Max - m_Min); // normalized in [0.0, 1.0]
        return (int)ncbi_round(norm * m_IntRange);
    }
    inline int SizeToInt(double size)
    {
        double norm = size / (m_Max - m_Min); // normalized in [0.0, 1.0]
        return (int)ncbi_round(norm * m_IntRange);
    }
    inline double IntToReal(int i)
    {
        double norm = double(i) / m_IntRange; // normalized in [0.0, 1.0]
        return norm * (m_Max - m_Min) + m_Min;
    }
    inline double IntToSize(int i)
    {
        double norm = double(i) / m_IntRange; // normalized in [0.0, 1.0]
        return (int)ncbi_round(norm * (m_Max - m_Min));
    }
protected:
    double m_Min, m_Max;
    int m_IntRange;
};



////////////////////////////////////////////////////////////////////////////////
/// class CGlWidgetBase

class NCBI_GUIWIDGETS_GL_EXPORT CGlWidgetBase :
        public wxPanel,
        public CEventHandler,
        public IRegSettings
{
    DECLARE_EVENT_TABLE()
public:
    enum {
        ID_GLCHILDPANE = wxID_HIGHEST + 200,
        ID_VSCROPLLBAR,
        ID_HSCROPLLBAR
    };

    CGlWidgetBase(wxWindow* parent,
                  wxWindowID id = wxID_ANY,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  long style = wxTAB_TRAVERSAL,
                  const wxString& name = wxT("panel"));

    virtual ~CGlWidgetBase();

    /// creates controls and performs basic initialization
    virtual void Create();

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    /// implement these 2 functions in derived classes
    virtual CGlPane&    GetPort() = 0;
    virtual const CGlPane&    GetPort() const = 0;

    /// @name wxWidgets event handlers
    /// @{
    void OnSize(wxSizeEvent& event);
    void OnScroll(wxScrollEvent& event);

    void OnZoomIn(wxCommandEvent& event);
    void OnZoomInX(wxCommandEvent& event);
    void OnZoomInY(wxCommandEvent& event);
    void OnZoomInMouse(wxCommandEvent& event);

    void OnZoomOut(wxCommandEvent& event);
    void OnZoomOutX(wxCommandEvent& event);
    void OnZoomOutY(wxCommandEvent& event);
    void OnZoomOutMouse(wxCommandEvent& event);

    void OnZoomAll(wxCommandEvent& event);
    void OnZoomAllX(wxCommandEvent& event);
    void OnZoomAllY(wxCommandEvent& event);

    void OnEnableCmdUpdate(wxUpdateUIEvent& event);
    /// @}

    virtual void    ZoomRect(const TModelRect&  rc);
    virtual void    ZoomPoint(const TModelPoint& point, 
                              TModelUnit factor,
                              CGlPane::EZoomOptions = CGlPane::fZoomXY);
    virtual void    Scroll(TModelUnit d_x, TModelUnit d_y);

    virtual void    NotifyVisibleRangeChanged();

    // Default events handler - reports unhandled event
    void OnAllEvents(CViewEvent::TEventObject evt);

protected:
    /// Override to record when (blocking) popup menu is active
    virtual bool DoPopupMenu(wxMenu *menu, int x, int y);

    /// creates Pane, Scrollbars and other child widgets, called from Create()
    virtual void    x_CreateControls(void);

    /// factory method creating master pane, called form x_CreateControls()
    virtual void    x_CreatePane() = 0;
    virtual CGlWidgetPane* x_GetPane();

    virtual void    x_RedrawControls(void);
    virtual void    x_Update();

    virtual void    x_SaveStates(){};

    /// updates model limits of the Master CGlPane

    virtual void    x_SetPortLimits(void) = 0;

    /// Zoom functions
    virtual void    x_ZoomIn(int options);
    virtual void    x_ZoomOut(int options);
    virtual void    x_ZoomAll(int options);
    virtual void    x_UpdateOnZoom();

    virtual void    x_UpdateOnHScroll();
    virtual void    x_UpdateOnVScroll();

    virtual void    x_UpdateScrollbars();
    virtual void    x_OnScrollX(int pos);
    virtual void    x_OnScrollY(int pos);

    ///
    virtual void    x_ShowDecoratedPopupMenu(wxMenu* menu);

protected:
    /// path to the widget's settings in GUI Registry
    string  m_RegPath;

    TModelPoint m_PopupPoint;
};

END_NCBI_SCOPE


#endif  // GUI_WIDGETS_GL___GLPANE_WIDGET__HPP
