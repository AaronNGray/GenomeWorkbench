#ifndef GUI_WIDGETS_GL_WX_DEMO___GLPANEWIDGET_CHILD_DEMO__HPP
#define GUI_WIDGETS_GL_WX_DEMO___GLPANEWIDGET_CHILD_DEMO__HPP


/*  $Id: glpanewidget_child_demo.hpp 17877 2008-09-24 13:03:09Z dicuccio $
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
 * Authors:  Roman Katargin, Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/widgets/gl/gl_widget_base.hpp>

#include <gui/widgets/gl/linear_sel_handler.hpp>
#include <gui/widgets/gl/mouse_zoom_handler.hpp>


BEGIN_NCBI_SCOPE

class CGlWidgetDemo;

///////////////////////////////////////////////////////////////////////////////
///
class CGlPaneWidgetChildDemo :
    public CGlWidgetPane,
    public ISelHandlerHost,
    public IMouseZoomHandlerHost
{
    DECLARE_EVENT_TABLE();
public:
    /// Constructors
    CGlPaneWidgetChildDemo(CGlWidgetDemo* parent, wxWindowID id = wxID_ANY);
    virtual ~CGlPaneWidgetChildDemo();

    /// CGlWidgetPane overridables
    virtual TVPPoint GetPortSize();

    void    OnSize(wxSizeEvent& event);

    /// @name ISelHandlerHost implementation
    /// @{
    virtual void    SHH_OnChanged();
    virtual TModelUnit  SHH_GetModelByWindow(int z, EOrientation orient);
    virtual TVPUnit     SHH_GetWindowByModel(TModelUnit z, EOrientation orient);
    /// @}

    /// @name IMouseZoomHandlerHost implementation
    /// @{
    virtual TModelUnit  MZHH_GetScale(EScaleType type);
    virtual void        MZHH_SetScale(TModelUnit scale, const TModelPoint& point);
    virtual void        MZHH_ZoomRect(const TModelRect& rc);
    virtual void        MZHH_ZoomPoint(const TModelPoint& point, TModelUnit factor);
    virtual void        MZHH_Scroll(TModelUnit d_x, TModelUnit d_y);
    virtual void        MZHH_EndOp();
    virtual TVPUnit     MZHH_GetVPPosByY(int y) const;
    /// @}

protected:
    /// CGlWidgetPane overridables
    virtual void    x_Render();
    virtual void    x_RenderSelHandler();

    CGlWidgetDemo* x_GetParent();

protected:
    CGlPane m_Pane;

    CMouseZoomHandler   m_MouseZoomHandler;
    CLinearSelHandler   m_HorzSelHandler;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_GL_WX_DEMO___GLPANEWIDGET_CHILD_DEMO__HPP
