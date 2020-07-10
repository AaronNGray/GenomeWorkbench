/*  $Id: draw_text_stream.cpp 41546 2018-08-17 17:03:23Z katargir $
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

#include <gui/widgets/text_widget/draw_text_stream.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

CDrawTextOStream::CDrawTextOStream(
    wxDC& dc, int x, int y,
    size_t rowHeight, size_t indentStep) :
        CBaseTextOStream(dc, x, y, rowHeight, indentStep)
{
}

void CDrawTextOStream::AddMarker(int startRow, int startCol, int endRow, int endCol, IStyleModifier* style)
{
    m_Markers.push_back(SMarker(startRow, startCol, endRow, endCol, style));
}

CStyledTextOStream& CDrawTextOStream::operator<< (const string& text_in)
{
    wxString text = ToWxString(text_in);
    int length = text.length();
    if (length == 0)
        return *this;

    wxArrayInt widths;
    m_DC.GetPartialTextExtents(text, widths);
    int y = m_Row*(int)m_RowHeight;

    list<CStylePos> styles = x_GetStyles(length);

    for (list<CStylePos>::const_iterator it2 = styles.begin();;) {
        list<CStylePos>::const_iterator it1 = it2++;
        if (it2 == styles.end()) break;

        IStyleModifier* modifier = it1->GetStyle();
        if (modifier) modifier->SetStyle(m_DC);
        else {
            if (m_CurrentStyle != 0) {
                m_DC.SetBackgroundMode(m_CurrentStyle->GetBackgroundMode());
                m_DC.SetTextForeground(m_CurrentStyle->GetTextForeground());
                m_DC.SetTextBackground(m_CurrentStyle->GetTextBackground());
            }
            else {
                m_DC.SetBackgroundMode(m_BackgroundMode);
                m_DC.SetTextForeground(m_TextForeground);
                m_DC.SetTextBackground(m_TextBackground);
            }
        }

        int start = it1->GetTextPos(), end = it2->GetTextPos();
        wxString substr = text.substr(start, end - start);
        m_DC.DrawText(substr, m_x + (start > 0 ? widths[start - 1] : 0), y);
    }

    x_UpdatePos(length, widths[length-1]);

    if (x_InNB()) x_UpdateNBRect(wxSize(widths[length-1], m_RowHeight));

    return *this;
}

void CDrawTextOStream::Indent (size_t steps)
{
    if (steps == 0)
        return;

    int y = m_Row*(int)m_RowHeight;
    list<CStylePos> styles = x_GetStyles(steps);

    for (list<CStylePos>::const_iterator it2 = styles.begin();;) {
        list<CStylePos>::const_iterator it1 = it2++;
        if (it2 == styles.end()) break;

        IStyleModifier* modifier = it1->GetStyle();
        if (modifier) {
            modifier->SetStyle(m_DC);
            int start = m_IndentStep*it1->GetTextPos(), end = m_IndentStep*it2->GetTextPos();
            m_DC.DrawRectangle(m_x + start, y, end - start, m_RowHeight);
        }
    }

    x_UpdatePos(steps, steps*m_IndentStep);
}

void CDrawTextOStream::x_OnNBEnd()
{
    wxPen savePen = m_DC.GetPen();
    m_DC.SetPen(wxPen(wxColor(128,128,128),1));

    m_DC.DrawLine(m_NBRect.x + 2, m_NBRect.y, m_NBRect.x + m_NBRect.width - 2, m_NBRect.y);
    m_DC.DrawLine(m_NBRect.x + m_NBRect.width - 2, m_NBRect.y + 1,
                  m_NBRect.x + m_NBRect.width - 2, m_NBRect.y + m_NBRect.height - 1);
    m_DC.DrawLine(m_NBRect.x + 2, m_NBRect.y + m_NBRect.height - 1,
                  m_NBRect.x + m_NBRect.width - 2, m_NBRect.y + m_NBRect.height - 1);
    m_DC.DrawLine(m_NBRect.x + 1, m_NBRect.y + 1,
                  m_NBRect.x + 1, m_NBRect.y + m_NBRect.height - 1);

    m_DC.SetPen(savePen);
}

list<CDrawTextOStream::CStylePos> CDrawTextOStream::x_GetStyles(int textLength)
{
    list<CStylePos> styles;
    styles.push_back(CStylePos(0, 0));
    styles.push_back(CStylePos(textLength, 0));

    REVERSE_ITERATE(vector<SMarker>, mit, m_Markers) {
        if (m_Row >= mit->m_StartRow && m_Row <= mit->m_EndRow) {
            int start = (m_Row == mit->m_StartRow) ? mit->m_StartCol - m_CurrentLineLength : 0;
            int end   = (m_Row == mit->m_EndRow) ? mit->m_EndCol - m_CurrentLineLength : textLength;
            start = min(max(start, 0), textLength);
            end   = min(max(end, 0), textLength);

            if (start < end) {
                list<CStylePos>::iterator it1 = styles.begin(), it2 = --styles.end();
                IStyleModifier* prevStyle = it1->GetStyle();
                for (list<CStylePos>::iterator it = styles.begin(); it != styles.end();) {
                    int pos = it->GetTextPos();
                    if (pos <= start) {
                        it1 = it;
                        ++it;
                        continue;
                    }
                    else if (pos < end) {
                        prevStyle = it->GetStyle();
                        it = styles.erase(it);
                        continue;
                    }
                    it2 = it;
                    break;
                }

                if (end < it2->GetTextPos())
                    styles.insert(it2, CStylePos(end, prevStyle));

                if (it1->GetTextPos() == start)
                    it1 = styles.erase(it1);
                else
                    ++it1;
                styles.insert(it1, CStylePos(start, mit->m_Style));
            }
        }
    }
    return styles;
}

END_NCBI_SCOPE
