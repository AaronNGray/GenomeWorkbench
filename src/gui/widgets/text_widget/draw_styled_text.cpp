/*  $Id: draw_styled_text.cpp 38729 2017-06-13 15:50:41Z katargir $
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

#include <gui/widgets/text_widget/draw_styled_text.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

void CStyledTextOStream::WriteMultiLineText(const string& text)
{
    if (!text.empty()) {
        list<string> lines;
        NStr::Split(text, "\n\r", lines, NStr::fSplit_Tokenize);
        ITERATE (list<string>, it, lines) {
            *this << *it;
            NewLine();
        }
    }
}

CBaseTextOStream::CBaseTextOStream(wxDC& dc, int x, int y,
                                   size_t rowHeight, size_t indentStep) :
        m_DC(dc), m_StartX(x), m_StartY(y),
        m_RowHeight(rowHeight), m_IndentStep(indentStep),
        m_x(x), m_Row(0), m_CurrentStyle(0), m_CurrentLineLength(0),
        m_NB(false), m_NBRect(x, y, 0, rowHeight)
{
    m_BackgroundMode = m_DC.GetBackgroundMode();
    m_TextForeground = m_DC.GetTextForeground();
    m_TextBackground = m_DC.GetTextBackground();
    m_SaveFont  = m_DC.GetFont();
    m_SaveBrush = m_DC.GetBrush();
    m_SavePen   = m_DC.GetPen();

}

CBaseTextOStream::~CBaseTextOStream()
{
    SetDefaultStyle();
}

void CBaseTextOStream::x_UpdatePos(int chars, int width)
{
    m_CurrentLineLength += chars;
    m_x += width;
}


void CBaseTextOStream::SetStyle(const CTextStyle* style)
{
    if (style == m_CurrentStyle)
        return;

    m_CurrentStyle = style;
    if (m_CurrentStyle == 0)
        SetDefaultStyle();
    else {
        m_DC.SetBackgroundMode(m_CurrentStyle->GetBackgroundMode());
        m_DC.SetTextForeground(m_CurrentStyle->GetTextForeground());
        m_DC.SetTextBackground(m_CurrentStyle->GetTextBackground());

        wxFont font;
        font.SetNativeFontInfo(m_SaveFont.GetNativeFontInfoDesc());
        font.SetStyle(
            m_CurrentStyle->IsTextItalic() ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL);
        font.SetWeight(
            m_CurrentStyle->IsTextBold() ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);

        m_DC.SetFont(font);
    }
}


void CBaseTextOStream::SetDefaultStyle()
{
    m_DC.SetBackgroundMode(m_BackgroundMode);
    m_DC.SetTextForeground(m_TextForeground);
    m_DC.SetTextBackground(m_TextBackground);
    m_DC.SetFont(m_SaveFont);
    m_DC.SetBrush(m_SaveBrush);
    m_DC.SetPen(m_SavePen);
}

void CBaseTextOStream::NewLine()
{
    ++m_Row;
    m_x = m_StartX;
    m_CurrentLineLength = 0;

    if (m_NB) {
        m_NB = false;
        x_OnNBEnd();
    }
}

void CBaseTextOStream::NBtoggle()
{
    if (m_NB) {
        m_NB = false;
        x_OnNBEnd();
    }
    else {
        m_NB = true;
        m_NBRect.x = m_x;
        m_NBRect.y = m_StartY + m_Row*m_RowHeight;
        m_NBRect.width = 0;
        m_NBRect.height = m_RowHeight;
        x_OnNBStart();
    }
}

void CBaseTextOStream::x_UpdateNBRect(wxSize size)
{
    if (m_NB) {
        if (m_NBRect.height < size.GetHeight())
            m_NBRect.height = size.GetHeight();
        m_NBRect.width += size.GetWidth();
    }
}

END_NCBI_SCOPE
