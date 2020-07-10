/*  $Id: calc_textpos_stream.cpp 25755 2012-05-03 19:14:11Z katargir $
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

#include <gui/widgets/text_widget/calc_textpos_stream.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

CCalcTextPosOStream::CCalcTextPosOStream(wxDC& dc, int x, int y,
                                         size_t rowHeight, size_t indentStep,
                                         int mouseX, int mouseY) :
    CBaseTextOStream(dc, x, y, rowHeight, indentStep),
        m_MouseX(mouseX), m_MouseY(mouseY), m_TextPosX(0),
        m_NBStartChar(0), m_NBRow(0)

{
    m_TextPosY = (m_MouseY >= m_RowHeight) ? 1 : 0;
}

void CCalcTextPosOStream::NewLine()
{
    TParent::NewLine();

    if (m_MouseY >= (m_Row + 1)*m_RowHeight)
        m_TextPosY = m_Row + 1;
}

CStyledTextOStream& CCalcTextPosOStream::operator<< (const string& text_in)
{
    wxString text = ToWxString(text_in);
    int length = text.length();
    if (length == 0)
        return *this;

    wxArrayInt widths;
    m_DC.GetPartialTextExtents(text, widths);

    if (m_MouseY >= (int)(m_Row*m_RowHeight) &&
        m_MouseY < (int)((m_Row + 1)*m_RowHeight)) {

        m_Ostr << text_in;

        m_TextPosY = m_Row;

        if (!x_InNB() && m_MouseX >= m_x) {
            wxArrayInt::iterator it;
            if (length < 5)
                for (it = widths.begin();
                     it != widths.end() && m_MouseX-m_x > *it;
                     ++it);
            else
                it = lower_bound(widths.begin(), widths.end(), m_MouseX-m_x);

            if (it != widths.end()) {
                int charWidth = *it;
                if (it != widths.begin())
                    charWidth -= *(it-1);
                if (m_MouseX-m_x + charWidth/2 >= *it)
                    ++it;
            }
            m_TextPosX  = it - widths.begin() + m_CurrentLineLength;
            if (it != widths.begin())
                --it;
        }
    }

    x_UpdatePos(length, widths[length-1]);
    if (x_InNB()) x_UpdateNBRect(wxSize(widths[length-1], m_RowHeight));

    return *this;
}

void CCalcTextPosOStream::Indent (size_t steps)
{
    if (steps == 0)
        return;

    size_t w = m_IndentStep*steps;

    if (m_MouseY >= (int)(m_Row*m_RowHeight) &&
        m_MouseY < (int)((m_Row + 1)*m_RowHeight)) {

        m_Ostr << string(steps, ' ');

        m_TextPosY = m_Row;

        if (!x_InNB() && m_MouseX >= m_x) {
            size_t index = (m_MouseX - m_x + m_IndentStep/2)/m_IndentStep;
            m_TextPosX  = min(index, steps) + m_CurrentLineLength;
        }
    }

    x_UpdatePos(steps, w);
    if (x_InNB()) x_UpdateNBRect(wxSize(w, m_RowHeight));
}

void CCalcTextPosOStream::x_OnNBStart()
{
    m_NBRow = m_Row;
    m_NBStartChar = m_CurrentLineLength;
}

void CCalcTextPosOStream::x_OnNBEnd()
{
    wxRect rect = GetNBRect();

    if (m_MouseY >= rect.GetTop() && m_MouseY <= rect.GetBottom() && 
        m_MouseX >= rect.GetLeft()) {

        m_TextPosY = m_Row;

        if ((m_MouseX - rect.GetLeft())*2 < rect.width)
            m_TextPosX = m_NBStartChar;
        else 
            m_TextPosX = m_CurrentLineLength;
    }
}

END_NCBI_SCOPE
