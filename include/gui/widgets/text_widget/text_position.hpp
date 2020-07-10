#ifndef GUI_WIDGETS_TEXT_WIDGET___TEXT_POSITION__HPP
#define GUI_WIDGETS_TEXT_WIDGET___TEXT_POSITION__HPP

/*  $Id: text_position.hpp 27273 2013-01-18 18:33:40Z katargir $
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

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

BEGIN_NCBI_SCOPE

class CTextPosition
{
public:
    CTextPosition() : m_Row(0), m_Col(0) {}
    CTextPosition(int row, int col) : m_Row(row), m_Col(col) {}

    bool operator< (const CTextPosition& pos) const
        { return (m_Row < pos.m_Row || (m_Row == pos.m_Row && m_Col < pos.m_Col)); }
    bool operator> (const CTextPosition& pos) const { return (pos < *this); }
    bool operator== (const CTextPosition& pos) const
        { return m_Row == pos.m_Row && m_Col == pos.m_Col; }
    bool operator!= (const CTextPosition& pos) const { return !(pos == *this); }

    bool operator>= (const CTextPosition& pos) const
        { return !this->operator<(pos); }

    void ShiftRow(int delta) { m_Row += delta; }
    void ShiftCol(int delta) { m_Col += delta; }

    int GetRow() const { return m_Row; }
    int GetCol() const { return m_Col; }

private:
    int  m_Row;
    int  m_Col;
};

class CTextBlock
{
public:
    CTextBlock() {}
    CTextBlock(const CTextPosition& start, const CTextPosition& end)
        : m_Start(start), m_End(end) {}
    CTextBlock(int startLine, int lineCount)
        : m_Start(startLine, 0), m_End(startLine + lineCount, 0) {}

    CTextBlock Normalized() const
    { return (m_Start > m_End) ? CTextBlock(m_End, m_Start) : CTextBlock(*this); }
    
    bool IsEmpty() const { return m_Start == m_End; }

    bool Intersect(const CTextBlock& block)
    {
        if (IsEmpty() || block.IsEmpty()) return false;
        CTextBlock b1 = this->Normalized(), b2 = block.Normalized();
        return (b1.m_Start < b2.m_End && b2.m_Start < b1.m_End);
    }

    void ShiftRow(int delta) { m_Start.ShiftRow(delta); m_End.ShiftRow(delta); }

    CTextPosition GetStart() const { return m_Start; }
    CTextPosition GetEnd() const { return m_End; }

private:
    CTextPosition m_Start;
    CTextPosition m_End;
};

NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CNcbiOstream& operator<<(CNcbiOstream& strm, const CTextPosition& pos);
NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CNcbiOstream& operator<<(CNcbiOstream& strm, const CTextBlock& block);

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TEXT_WIDGET___TEXT_POSITION__HPP
