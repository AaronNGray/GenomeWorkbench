#ifndef GUI_WX_DEMO___DOCK_HINT_FRAME__HPP
#define GUI_WX_DEMO___DOCK_HINT_FRAME__HPP

/*  $Id: dock_hint_frame.hpp 37486 2017-01-13 20:44:49Z katargir $
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

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/dock_window.hpp>

#include <wx/platform.h>
#include <wx/frame.h>

#if defined( __WXMSW__ ) || defined( __WXMAC__ ) ||  defined( __WXGTK__ )
    #include "wx/minifram.h"
#endif

BEGIN_NCBI_SCOPE

class CDockManager;

///////////////////////////////////////////////////////////////////////////////
/// CPseudoTransparentFrame - this is a transparent window that is used as
/// a visual hint during D&D docking. The frame is displayed as a transparent
/// overlay on top of the Window Manager.

class NCBI_GUIWIDGETS_WX_EXPORT CPseudoTransparentFrame : public wxFrame
{
    DECLARE_EVENT_TABLE()
public:
    CPseudoTransparentFrame(wxWindow* parent, CDockManager& manager);

    virtual bool SetTransparent(wxByte alpha);

    void    OnKeyUp(wxKeyEvent& event);
    void    OnKeyDown(wxKeyEvent& event);
protected:
    CDockManager&   m_DockManager;

/// for all platforms except for GTK 20 adding the code below
#ifndef __WXGTK20__
    void    OnPaint(wxPaintEvent& event);

#ifdef __WXGTK__
    void OnWindowCreate(wxWindowCreateEvent& event);
#endif

    void OnSize(wxSizeEvent& event);

protected:
    wxByte  m_Amount;
    int     m_MaxWidth;
    int     m_MaxHeight;
    bool    m_CanSetShape;
    int     m_LastWidth;
    int     m_LastHeight;

    wxRegion m_Region;
#endif // __WXGTK20__
};


/// For platforms that support transparent frames - we derive our Hint Frame
/// form different base classes on different platforms.
#if defined(__WXMSW__) || defined(__WXGTK__)
    typedef wxFrame     TTransFrameBase;
#elif defined(__WXMAC__)
    typedef wxMiniFrame TTransFrameBase;
#endif


///////////////////////////////////////////////////////////////////////////////
/// CTransparentFrame - this is frame that is used on platform supporting
/// transparency. We subclass TTransFrameBase in order to forward keyboard
/// events to Dock Manager.
class  NCBI_GUIWIDGETS_WX_EXPORT  CTransparentFrame : public TTransFrameBase
{
    DECLARE_EVENT_TABLE();
public:
    CTransparentFrame(wxWindow* parent, CDockManager& manager);
    ~CTransparentFrame();

    void    OnKeyUp(wxKeyEvent& event);
    void    OnKeyDown(wxKeyEvent& event);

protected:
    CDockManager&   m_DockManager;
};


/// This functions create a platform-dependent Hint Window for Dock Manager.
wxWindow*    CreateDockHintFrame(wxWindow& parent, CDockManager& manager);


END_NCBI_SCOPE


#endif  // GUI_WX_DEMO___DOCK_HINT_FRAME__HPP
