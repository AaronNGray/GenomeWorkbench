/*  $Id: expand_glyph.cpp 37393 2017-01-04 22:12:16Z katargir $
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

#include <gui/widgets/text_widget/glyph.hpp>
#include <gui/widgets/text_widget/expand_glyph.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>

#include <wx/dcclient.h>

BEGIN_NCBI_SCOPE

void CExpandGlyph::Draw(wxDC& dc, CTextPanelContext* WXUNUSED(context))
{
    wxPen savePen = dc.GetPen();
    dc.SetPen(wxPen(wxColor(255,0,0),1));

    int size = min(m_Size.GetWidth(), m_Size.GetHeight());
    size = (size-1)|1; // make odd
    int x = (m_Size.GetWidth() - size)/2;
    int y = (m_Size.GetHeight() - size + 1)/2;
    //dc.DrawRectangle(x, y, size, size);
#ifdef __WXMAC__
    dc.DrawLine(x, y + size / 2, x + size - 1, y + size / 2);
    if (!m_Expandable->IsExpanded())
        dc.DrawLine(x + size / 2, y, x + size / 2, y + size - 1);
#else
    dc.DrawLine(x, y + size / 2, x + size, y + size / 2);
    if (!m_Expandable->IsExpanded())
        dc.DrawLine(x + size / 2, y, x + size / 2, y + size);
#endif

    dc.SetPen(savePen);
}

void CExpandGlyph::MouseEvent(wxMouseEvent& event)
{
    if (event.Moving())
        MouseMove(event);
    else if (event.LeftDown())
        MouseLeftDown(event);
}

void CExpandGlyph::MouseLeftDown(wxMouseEvent& event)
{
    CTextItemPanel* panel = (CTextItemPanel*)event.GetEventObject();
    CTextPanelContext* context = panel->GetContext();
    m_Expandable->Toggle(context, true);
}

void CExpandGlyph::MouseMove(wxMouseEvent& event)
{
    CTextItemPanel* panel = static_cast<CTextItemPanel*>(event.GetEventObject());
    panel->SetCursor(wxCursor(wxCURSOR_HAND));
}

END_NCBI_SCOPE
