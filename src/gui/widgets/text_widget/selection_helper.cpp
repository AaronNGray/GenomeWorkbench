/*  $Id: selection_helper.cpp 39189 2017-08-17 15:12:36Z katargir $
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
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/text_widget/selection_helper.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>

#include <wx/dcclient.h>
#include <wx/txtstrm.h>

BEGIN_NCBI_SCOPE

BEGIN_EVENT_TABLE(CSelectionHelper, wxEvtHandler)
    EVT_MOTION(CSelectionHelper::OnMouseMove)
    EVT_LEFT_DOWN(CSelectionHelper::OnLeftDown)
    EVT_LEFT_DCLICK(CSelectionHelper::OnLeftDClick)
END_EVENT_TABLE()

CSelectionHelper& CSelectionHelper::GetInstance()
{
    static CSelectionHelper _instance(2);
    return _instance;
}

void CSelectionHelper::Draw(wxDC& dc, const wxRect& updateRect, CTextPanelContext* context)
{
    _ASSERT(m_Item);
    if (m_Item->GetAssosiatedObject())
        x_DrawSelectionArea(dc, updateRect, context);
}

wxRect CSelectionHelper::x_GetSelectionRect(CTextPanelContext* context) const
{
    int x = m_Item->GetTextLeftMargin(context);
    int w = context->GetWWidth();
    return wxRect (wxPoint(x-m_Offset*w,0), wxSize(w, m_Item->GetSize().GetHeight()));
}

void CSelectionHelper::x_DrawSelectionArea(wxDC& dc, const wxRect& updateRect, CTextPanelContext* context)
{
    wxBrush saveBrush = dc.GetBrush();
    wxPen savePen = dc.GetPen();

    wxRect rect = m_Item->GetRect();
    wxRect itemSelRect = x_GetSelectionRect(context);

    wxPoint dcOrigin = context->GetDeviceOrigin();
    context->SetDeviceOrigin(wxPoint(dcOrigin.x + updateRect.GetLeft(), dcOrigin.y + updateRect.GetTop()));
    context->SetDeviceOrigin(dc);

    itemSelRect = itemSelRect.Intersect(updateRect);
    itemSelRect.Offset(-updateRect.GetLeft(), -updateRect.GetTop());
    rect.SetPosition(wxPoint(-updateRect.GetLeft(), -updateRect.GetTop()));

    if (context->IsItemSelected(m_Item)) {
        wxColor color(128,128,64);
        dc.SetBrush(wxBrush(color));
        dc.SetPen(wxPen(color, 1));
    } else {
        wxColor color = dc.GetTextBackground().ChangeLightness(95);
        dc.SetBrush(wxBrush(color));
        dc.SetPen(wxPen(color, 1));
    }
    dc.DrawRectangle(itemSelRect);

    if (context->IsItemSelected(m_Item) || context->GetHighlightedItem() == m_Item) {
        wxColor color(128,128,64);
        dc.SetPen(wxPen(color, 1));
        int yt = rect.GetTop();
        int yb = rect.GetBottom();
        if (yt < updateRect.GetHeight() && yb > 0) {
            if (yt >= 0)
                dc.DrawLine(rect.GetLeft(), yt, rect.GetRight(), yt);
            if (yb < updateRect.GetHeight())
                dc.DrawLine(rect.GetLeft(), yb, rect.GetRight(), yb);
            yt = max(yt, 0);
            yb = min(yb, updateRect.GetHeight() - 1);
            dc.DrawLine(rect.GetLeft(), yt, rect.GetLeft(), yb);
            dc.DrawLine(rect.GetRight(), yt, rect.GetRight(), yb);
        }
    }

    dc.SetBrush(saveBrush);
    dc.SetPen(savePen);
    
    context->SetDeviceOrigin(dcOrigin);
}

void CSelectionHelper::OnMouseMove(wxMouseEvent& event)
{
    CTextItemPanel* panel = static_cast<CTextItemPanel*>(event.GetEventObject());
    CTextPanelContext* context = panel->GetContext();

    if (!panel->Selecting()) {
        wxRect itemSelRect = x_GetSelectionRect(context);

        if (itemSelRect.Contains(event.m_x, event.m_y)) {
            if (context->GetHighlightedItem() != m_Item) {
                context->SetHighlightedItem(m_Item);
                panel->Refresh();
            }
            panel->SetCursor(wxCursor(wxCURSOR_ARROW));
            return;
        }
        else {
            if (context->GetHighlightedItem() != 0) {
                context->SetHighlightedItem(0);
                panel->Refresh();
            }
            panel->SetCursor(wxCursor(wxCURSOR_IBEAM));
        }
    }
    event.Skip();
}

void CSelectionHelper::OnLeftDown(wxMouseEvent& event)
{
    CTextItemPanel* panel = static_cast<CTextItemPanel*>(event.GetEventObject());
    CTextPanelContext* context = panel->GetContext();

    wxRect itemSelRect = x_GetSelectionRect(context);
    if (itemSelRect.Contains(event.m_x, event.m_y)) {
        context->SelectItem(m_Item, event.ControlDown(), event.ShiftDown());
    } else {
        wxRect leftSpace(0, itemSelRect.GetTop(), itemSelRect.GetRight(), itemSelRect.GetHeight());
        if (leftSpace.Contains(event.m_x, event.m_y))
            context->SelectItem(0, event.ControlDown(), event.ShiftDown());
        else
            event.Skip();
    }
}

void CSelectionHelper::OnLeftDClick(wxMouseEvent& event)
{
    CTextItemPanel* panel = static_cast<CTextItemPanel*>(event.GetEventObject());
    CTextPanelContext* context = panel->GetContext();

    wxRect itemSelRect = (context->IsEditingEnabled() || event.ControlDown()) ?
        wxRect (wxPoint(0,0), m_Item->GetSize()) : x_GetSelectionRect(context);

    if (itemSelRect.Contains(event.m_x, event.m_y)) {
        context->SelectItem(m_Item, false, event.ShiftDown());
        context->EditItem(m_Item, false, event.ShiftDown());
    }
    else
        event.Skip();
}

END_NCBI_SCOPE
