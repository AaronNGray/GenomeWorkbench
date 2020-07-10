/*  $Id: calc_size_stream.cpp 19853 2009-08-13 18:27:23Z katargir $
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

#include <gui/widgets/text_widget/calc_size_stream.hpp>
#include <gui/widgets/text_widget/calc_size_cache.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

CCalcSizeOStream::CCalcSizeOStream(wxDC& dc, CCalcSizeCache& cache,
                                   size_t rowHeight, size_t indentStep) :
    CBaseTextOStream(dc, 0, 0, rowHeight, indentStep),
    m_SizeCache(cache), m_Size(0, 0), m_LineWidth(0),
    m_LineHeight(m_RowHeight), m_LineCount(0), m_CachedStyle()
{
}

void CCalcSizeOStream::SetStyle(const CTextStyle* style)
{
    m_CachedStyle = style;
}

void CCalcSizeOStream::SetDefaultStyle()
{
    m_CachedStyle = 0;
}

void CCalcSizeOStream::NewLine()
{
    TParent::NewLine();
    m_Size.IncBy(0, m_RowHeight);

    if (m_LineWidth > (size_t)m_Size.GetWidth())
        m_Size.SetWidth(m_LineWidth);

    m_LineWidth = 0;
    m_LineHeight = m_RowHeight;
    ++m_LineCount;
}

CStyledTextOStream& CCalcSizeOStream::operator<< (const string& text_in)
{
    wxString text = ToWxString(text_in);
    int length = text.length();

    wxSize size = m_SizeCache.Get(text_in, m_CachedStyle);
    if (!size.IsFullySpecified()) {
        CBaseTextOStream::SetStyle(m_CachedStyle);
        size = m_DC.GetTextExtent(text);
        m_SizeCache.Put(text_in, m_CurrentStyle, size);
    }

    wxCoord w = size.GetWidth(), h = size.GetHeight();
    m_LineWidth += w;
    if ((size_t)h > m_LineHeight) m_LineHeight = h;

    x_UpdatePos(length, w);

    return *this;
}

void CCalcSizeOStream::Indent (size_t steps)
{
    if (steps == 0)
        return;

    size_t w = m_IndentStep*steps;
    m_LineWidth += w;
    if ((size_t)m_RowHeight > m_LineHeight) m_LineHeight = m_RowHeight;
    x_UpdatePos(steps, w);
}

wxSize CCalcSizeOStream::GetSize() const
{
    wxSize size = m_Size;
    if (m_LineWidth > 0) size.IncBy(m_LineWidth, m_RowHeight);
    return size;
}

size_t CCalcSizeOStream::GetLineCount() const
{
    return (m_LineWidth > 0) ? m_LineCount + 1 : m_LineCount;
}

END_NCBI_SCOPE
