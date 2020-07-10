#ifndef __GUI_WIDGETS_GL___TOOLTIP_HANDLER__HPP
#define __GUI_WIDGETS_GL___TOOLTIP_HANDLER__HPP

/*  $Id: tooltip_handler.hpp 21488 2010-06-08 13:54:09Z falkrb $
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
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>

#include <gui/widgets/gl/ievent_handler.hpp>

#include <math.h>

#include <wx/timer.h>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
///
class ITooltipHandlerHost
{
public:
   public:
    virtual ~ITooltipHandlerHost() { }

    /// Returns "true" if client wants to dispaly a tooltip. "x" and "y"  are
    /// cordinates of the mouse pointer
    virtual bool    TC_NeedTooltip(const wxPoint & pt) = 0;

    /// Returns tooltip string and coordinates for area tootlip associated
    /// with.  Client may choose not modify x, y, w, z, in that case tooltip
    /// will be associated with current mouse position.
    virtual string  TC_GetTooltip(const wxRect & rect) = 0;

    /// Returns pointer to the widget hosting ITooltipClient.
    /// Default implementation returns "this";
    virtual wxWindow*  TC_GetWindow() = 0;

};

class CTooltipWnd;


// tooltip class
class CTooltip : public wxTimer
{
protected:  
    bool                  m_bNewText;
    wxString              m_Text;
    ITooltipHandlerHost * m_Host;
    CTooltipWnd*          m_TooltipWnd;
    
    // Screen coordniates of mouse when tip enabled
    wxPoint               m_ScreenPos;
    // Mouse position on Window when tip enabled
    wxPoint               m_BasePos;
    // Offset between m_BasePos and position tip shown
    wxPoint               m_PosDelta;

    bool                  m_On;
public:
    CTooltip() : m_bNewText(false), m_TooltipWnd(), m_On(false) {}
    ~CTooltip();   

    // sets text for tooltip
    void SetText(const string & text);

    // host is needed to access tooltip-ed wxWindow
    void SetHost(ITooltipHandlerHost* host)
    {
        m_Host = host;
    }

    // new or old text
    bool NewText() const { return m_bNewText; }

    // switch tooltip off (should work on mac/gtk)
    void Off();
    // switch tooltip On
    void On(wxPoint point);
    void Move();
    void WinMove();
    bool IsOn() const { return m_On; }
    bool IsShown() const;
    wxPoint GetScreenPos() const { return m_ScreenPos; }

    // mouse move default tooltip handle
    void HideAndShow(wxPoint point);
    void Hide();
    // wxTimer
    void Notify();
};

////////////////////////////////////////////////////////////////////////////////
/// CTooltipHandler

class NCBI_GUIWIDGETS_GL_EXPORT CTooltipHandler :
            public wxEvtHandler,
            public IGlEventHandler          
{
    DECLARE_EVENT_TABLE()
public:
     enum EMode {
        eHideOnMove,    /// if mouse moves tooltip hides and reappears only after delay
        eTrackOnMove, /// if mouse moves tooltip reappears immediatly in a new position
                      /// if text is the same
        eStayOnMove   /// if tooltip area and text remain the same - tooltip will remain on screen
                      /// and will not move
     };

    CTooltipHandler();
    virtual ~CTooltipHandler();

    virtual void    SetMode(EMode mode);    
    virtual void    SetHost(ITooltipHandlerHost* host);
    IGenericHandlerHost*    GetGenericHost();

    /// @name IGlEventHandler implementaion
    /// @{
    virtual void    SetPane(CGlPane* pane);
    virtual wxEvtHandler*    GetEvtHandler();
    /// @}

    void    OnLeftDown(wxMouseEvent& event);
    void    OnRightDown(wxMouseEvent& event);
    void    OnMotion(wxMouseEvent& event);
    void    OnMouseCaptureLost(wxMouseCaptureLostEvent& evt);
    void    OnKeyDown(wxKeyEvent& event);
    void    OnKillFocus(wxFocusEvent& event);
    void    OnSize(wxSizeEvent& event);
    void    OnTimer(wxTimerEvent& event);

    
protected:
    EMode                 m_Mode;
    ITooltipHandlerHost * m_Host;
    CGlPane*              m_Pane;   
    CTooltip              m_Tip;
    wxTimer               m_PosUpdateTimer;
};


END_NCBI_SCOPE

#endif
