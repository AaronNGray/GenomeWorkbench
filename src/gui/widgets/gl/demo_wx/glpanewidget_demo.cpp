/*  $Id: glpanewidget_demo.cpp 25560 2012-04-09 14:28:39Z katargir $
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

#include <wx/menu.h>

#include "glpanewidget_demo.hpp"
#include "glpanewidget_child_demo.hpp"

#include <gui/utils/command.hpp>

BEGIN_NCBI_SCOPE

BEGIN_EVENT_TABLE(CGlWidgetDemo, CGlWidgetBase)
    EVT_CONTEXT_MENU(CGlWidgetDemo::OnContextMenu)
END_EVENT_TABLE()

CGlWidgetDemo::CGlWidgetDemo(wxWindow* parent, wxWindowID id) :
    CGlWidgetBase(parent, id, wxDefaultPosition, wxDefaultSize, 0)
{
    CGlPane& port = GetPort();
    port.SetMinScaleX(0.001);
    port.SetMinScaleY(0.001);
}


CGlWidgetDemo::~CGlWidgetDemo()
{
}


void CGlWidgetDemo::x_CreatePane()
{
    new CGlPaneWidgetChildDemo(this, ID_GLCHILDPANE);
}


void CGlWidgetDemo::x_SetPortLimits()
{
    CGlPane& port = GetPort();
    port.SetModelLimitsRect(TModelRect(0, 0, 40.0, 40.0));
}


void CGlWidgetDemo::SetScale(TModelUnit scale, const TModelPoint& point)
{
    CGlPane& port = GetPort();
    port.SetScaleRefPoint(scale, scale, point);
    x_UpdateOnZoom();
}



CGlPane& CGlWidgetDemo::GetPort()
{
    return m_Port;
}


const CGlPane& CGlWidgetDemo::GetPort() const
{
    return m_Port;
}


void CGlWidgetDemo::OnContextMenu(wxContextMenuEvent& event)
{
    wxPoint point  = event.GetPosition();

    bool from_keyboard = (point == wxPoint(-1, -1));
    if (! from_keyboard) {
        point = ScreenToClient(point);
        CGlWidgetPane* child_pane = x_GetPane();
        wxPoint pt = child_pane->ScreenToClient(ClientToScreen(point));

        wxSize sz = child_pane->GetClientSize();

        CGlPane& port = GetPort();
        port.OpenOrtho();
        m_PopupPoint = port.UnProject(pt.x, sz.y - pt.y);
        port.Close();
    }
    wxMenu menu;

    menu.Append(eCmdZoomIn, wxT("Zoom &In"));
    menu.Append(eCmdZoomInX, wxT("Zoom In X"));
    menu.Append(eCmdZoomInY, wxT("Zoom In Y"));
    if (! from_keyboard)    {
        menu.Append(eCmdZoomInMouse, wxT("Zoom In Mouse"));
    }
    menu.AppendSeparator();

    menu.Append(eCmdZoomOut, wxT("Zoom &Out"));
    menu.Append(eCmdZoomOutX, wxT("Zoom Out X"));
    menu.Append(eCmdZoomOutY, wxT("Zoom Out Y"));
    if (! from_keyboard)    {
        menu.Append(eCmdZoomOutMouse, wxT("Zoom Out Mouse"));
    }
    menu.AppendSeparator();

    menu.Append(eCmdZoomAll, wxT("Zoom &All"));
    menu.Append(eCmdZoomAllX, wxT("Zoom All X"));
    menu.Append(eCmdZoomAllY, wxT("Zoom All Y"));

    PopupMenu(&menu, point);
}


END_NCBI_SCOPE
