/*  $Id: glpanewidget_child_demo.cpp 42140 2018-12-27 19:19:17Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 */

#include <ncbi_pch.hpp>

#include "glpanewidget_child_demo.hpp"
#include "glpanewidget_demo.hpp"

#include <gui/opengl/irender.hpp>

BEGIN_NCBI_SCOPE

BEGIN_EVENT_TABLE(CGlPaneWidgetChildDemo, CGlWidgetPane)
    EVT_SIZE(CGlPaneWidgetChildDemo::OnSize)
END_EVENT_TABLE()


CGlPaneWidgetChildDemo::CGlPaneWidgetChildDemo(CGlWidgetDemo* parent, wxWindowID id)
:   CGlWidgetPane(parent, id, wxDefaultPosition, wxDefaultSize, 0),
    m_Pane(CGlPane::eAlwaysUpdate)
{
    SetBackgroundColour(wxColour(230, 230, 255));

    m_MouseZoomHandler.SetHost(static_cast<IMouseZoomHandlerHost*>(this));
    x_RegisterHandler(&m_MouseZoomHandler, 0xFFFF, &m_Pane);

    m_HorzSelHandler.SetHost(static_cast<ISelHandlerHost*>(this));
    x_RegisterHandler(&m_HorzSelHandler, 0xFFFF, &m_Pane);
}


CGlPaneWidgetChildDemo::~CGlPaneWidgetChildDemo()
{
}


TVPPoint CGlPaneWidgetChildDemo::GetPortSize()
{
    return TVPPoint(1000, 1000);
}


void CGlPaneWidgetChildDemo::x_Render()
{
    C3DCanvas::x_Render();

    IRender& gl = GetGl();

    // clear background
    wxSize size = GetClientSize();
    int h = size.GetHeight();
    int w = size.GetWidth();

    gl.Viewport(0, 0, w, h);

    // Render graphics
    CGlWidgetDemo* the_parent = dynamic_cast<CGlWidgetDemo*>(GetParent());
    m_Pane = the_parent->GetPort();

    m_Pane.OpenOrtho();

    gl.Color3d(1.0, 0.0, 0.0);
    gl.Rectd(0.0, 0.0, 10.0, 10.0);

    gl.Color3d(0.0, 1.0, 0.0);
    gl.Rectd(10.0, 10.0, 20.0, 20.0);

    gl.Color3d(0.0, 0.0, 1.0);
    gl.Rectd(20.0, 20.0, 30.0, 30.0);

    m_Pane.Close();

    x_RenderSelHandler();
}


void CGlPaneWidgetChildDemo::x_RenderSelHandler()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_HorzSelHandler.Render(m_Pane);
    m_MouseZoomHandler.Render(m_Pane);

    glDisable(GL_BLEND);
}


CGlWidgetDemo* CGlPaneWidgetChildDemo::x_GetParent()
{
    return dynamic_cast<CGlWidgetDemo*>(GetParent());
}


void CGlPaneWidgetChildDemo::OnSize(wxSizeEvent& WXUNUSED(event))
{
    m_Pane = x_GetParent()->GetPort();
}


////////////////////////////////////////////////////////////////////////////////
/// ISelHandlerHost implementation
void CGlPaneWidgetChildDemo::SHH_OnChanged()
{
    //CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged);
    //Send(&evt, ePool_Parent);
    Refresh();
}


TModelUnit CGlPaneWidgetChildDemo::SHH_GetModelByWindow(int z, EOrientation orient)
{
    switch(orient)  {
    case eHorz:
        return m_Pane.UnProjectX(z);
    case eVert:
        return m_Pane.UnProjectY(GetSize().y - z);
    default:
        _ASSERT(false); return -1;
    }
}


TVPUnit CGlPaneWidgetChildDemo::SHH_GetWindowByModel(TModelUnit z, EOrientation orient)
{
    switch(orient)  {
    case eHorz:
        return m_Pane.ProjectX(z);
    case eVert:
        return GetSize().y - m_Pane.ProjectY(z);
    default:
        _ASSERT(false); return -1;
    }
}


////////////////////////////////////////////////////////////////////////////////
/// IMouseZoomHandlerHost implementation

TModelUnit CGlPaneWidgetChildDemo::MZHH_GetScale(EScaleType type)
{
    const CGlPane& VP = x_GetParent()->GetPort();

    switch(type)    {
    case eCurrent:
        return VP.GetScaleX();
    case eMin:
        return VP.GetMinScaleX();
    case eMax:
        return VP.GetZoomAllScaleX();
    default:
        _ASSERT(false); return -1;
    }
}


void CGlPaneWidgetChildDemo::MZHH_SetScale(TModelUnit scale, const TModelPoint& point)
{
    x_GetParent()->SetScale(scale, point);
}


void  CGlPaneWidgetChildDemo::MZHH_ZoomRect(const TModelRect& rc)
{
    x_GetParent()->ZoomRect(rc);

    // translate the notification to the standard message
    x_GetParent()->NotifyVisibleRangeChanged();
}


void  CGlPaneWidgetChildDemo::MZHH_ZoomPoint(const TModelPoint& point, TModelUnit factor)
{
    x_GetParent()->ZoomPoint(point, factor);

    // translate the notification to the standard message
    x_GetParent()->NotifyVisibleRangeChanged();
}


void  CGlPaneWidgetChildDemo::MZHH_EndOp()
{
    // translate the notification to the standard message
    x_GetParent()->NotifyVisibleRangeChanged();
}


void  CGlPaneWidgetChildDemo::MZHH_Scroll(TModelUnit d_x, TModelUnit d_y)
{
    x_GetParent()->Scroll(d_x, d_y);

    // translate the notification to the standard message
    x_GetParent()->NotifyVisibleRangeChanged();
}


TVPUnit  CGlPaneWidgetChildDemo::MZHH_GetVPPosByY(int y) const
{
    return GetSize().y - 1 - y;
}


END_NCBI_SCOPE
