/*  $Id: calc_caretpos_stream.cpp 23747 2011-05-25 18:09:23Z katargir $
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

#include <gui/widgets/text_widget/calc_caretpos_stream.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

CCalcCaretPosOStream::CCalcCaretPosOStream(wxDC& dc, int x, int y, size_t rowHeight, size_t indentStep,
                                           int textPosX, int textPosY) :
    CBaseTextOStream(dc, x, y, rowHeight, indentStep),
        m_TextPosX(textPosX), m_TextPosY(textPosY)
{
    m_CaretPosX = m_StartX;

    if (m_TextPosY < 0)
        m_CaretPosY = m_StartY - rowHeight;
    else if (m_TextPosY > 0)
        m_CaretPosY = m_StartY + rowHeight;
    else
        m_CaretPosY = m_StartY;
}

void CCalcCaretPosOStream::NewLine()
{
    TParent::NewLine();

    if (m_Row < m_TextPosY)
        m_CaretPosY += m_RowHeight;
}

CStyledTextOStream& CCalcCaretPosOStream::operator<< (const string& text_in)
{
    wxString text = ToWxString(text_in);
    int length = text.length();
    if (length == 0)
        return *this;

    wxArrayInt widths;
    m_DC.GetPartialTextExtents(text, widths);

    if (m_Row == m_TextPosY) {
        if (m_CurrentLineLength + length <= m_TextPosX)
            m_CaretPosX = m_x + widths[length-1];
        else if (m_CurrentLineLength < m_TextPosX)
            m_CaretPosX = m_x + widths[m_TextPosX - m_CurrentLineLength - 1];
    }

    x_UpdatePos(length, widths[length-1]);

    return *this;
}

void CCalcCaretPosOStream::Indent (size_t steps)
{
    if (steps == 0)
        return;

    size_t w = m_IndentStep*steps;

    if (m_Row == m_TextPosY) {
        if (m_CurrentLineLength + (int)steps <= m_TextPosX)
            m_CaretPosX = m_x + w;
        else if (m_CurrentLineLength < m_TextPosX)
            m_CaretPosX = m_x + m_IndentStep*(m_TextPosX - m_CurrentLineLength);
    }

    x_UpdatePos(steps, w);
}

END_NCBI_SCOPE
