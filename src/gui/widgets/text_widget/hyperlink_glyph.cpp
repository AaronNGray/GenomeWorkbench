/*  $Id: hyperlink_glyph.cpp 38288 2017-04-21 20:55:35Z katargir $
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

#include <gui/widgets/text_widget/hyperlink_glyph.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>

#include <wx/dcclient.h>

BEGIN_NCBI_SCOPE

void CHyperlinkGlyph::Draw(wxDC& dc, CTextPanelContext* WXUNUSED(context))
{
    wxColour saveTextForeground = dc.GetTextForeground();
    wxFont saveFont = dc.GetFont();

    dc.SetTextForeground(*wxBLUE);
    wxFont font(m_PointSize, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, true, wxT("Tahoma"));
    dc.SetFont(font);
    dc.DrawText(m_Text, 0, 0);

    dc.SetTextForeground(saveTextForeground);
    dc.SetFont(saveFont);
}

void CHyperlinkGlyph::CalcSize(wxDC& dc, CTextPanelContext* context)
{
    wxFont saveFont = dc.GetFont();
    wxFont font(m_PointSize, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, true, wxT("Tahoma"));
    dc.SetFont(font);
    m_Size = dc.GetTextExtent(m_Text);
    dc.SetFont(saveFont);
}

void CHyperlinkGlyph::MouseEvent(wxMouseEvent& event)
{
    if (event.Moving())
        MouseMove(event);
    else if (event.LeftDown())
        MouseLeftDown(event);
    else if (event.RightDown())
        MouseLeftDown(event);
}

void CHyperlinkGlyph::MouseLeftDown(wxMouseEvent& event)
{
    if (m_OnClick) {
        CTextItemPanel* panel = (CTextItemPanel*)event.GetEventObject();
        m_OnClick->OnHyperlinkClick (panel->GetContext());
    }
}

void CHyperlinkGlyph::MouseMove(wxMouseEvent& event)
{
    CTextItemPanel* panel = static_cast<CTextItemPanel*>(event.GetEventObject());
    panel->SetCursor (wxCursor(wxCURSOR_HAND));
}

END_NCBI_SCOPE
