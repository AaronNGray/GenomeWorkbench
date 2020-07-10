/*  $Id: text_line_iterator.cpp 35541 2016-05-18 19:29:42Z katargir $
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

#include <gui/widgets/text_widget/text_line_iterator.hpp>

#include <gui/widgets/text_widget/expand_item.hpp>
#include <gui/widgets/text_widget/composite_text_item.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/text_widget/draw_styled_text.hpp>


BEGIN_NCBI_SCOPE

namespace // anonymous namespace
{

class CCopyLines : public CStyledTextOStream
{
public:
    CCopyLines(string& line, int lineNum)
        : m_Line(line), m_LineNum(lineNum), m_CurLine(), m_CurrentLineLength(0) {}

    virtual StreamHint GetHint() const { return kCopyText; }
    virtual void UpdatePosition(int shift) { m_CurrentLineLength += shift; }

    virtual CStyledTextOStream& operator<< (const string& text);

    virtual int CurrentRowLength() const { return m_CurrentLineLength; }

    virtual void SetStyle(const CTextStyle* /*style*/) {}
    virtual void SetDefaultStyle() {}
    virtual void Indent (size_t steps);
    virtual void NewLine();

private:
    string& m_Line;
    int     m_LineNum;
    int     m_CurLine;
    int     m_CurrentLineLength;
};

void CCopyLines::Indent (size_t steps)
{
    if (steps == 0)
        return;

    this->operator<<(string(steps, ' '));
}

void CCopyLines::NewLine()
{
    ++m_CurLine;
    m_CurrentLineLength = 0;
}

CStyledTextOStream& CCopyLines::operator<< (const string& text)
{
    if (m_CurLine == m_LineNum) {
        m_Line.append(text); 
    }
    m_CurrentLineLength += text.length();
    return *this;
}

} // anonymous namespace

CTextLineIterator::CTextLineIterator(CTextItemPanel* panel, const CTextPosition& pos, bool forward)
: m_Panel(panel), m_StartPos(pos), m_Forward(forward), m_Begin(true), m_End(false)
{
    ITextItem* root = m_Panel->GetRootItem();
    size_t lines = root->GetLineCount();
    int row = m_StartPos.GetRow();
    if (row < 0 || row >= (int)lines) {
        m_StartPos = CTextPosition(0, 0);
    }

    m_CurLine = m_StartPos.GetRow();

    CTextItem* item = dynamic_cast<CTextItem*>(root->GetItemByLine(m_CurLine, true));
    if (!item) {
        m_End = true;
        return;
    }

    CCopyLines ostream(m_Line, m_CurLine - item->GetStartLine());
    item->RenderText(ostream, m_Panel->GetContext());

    int startCol = m_StartPos.GetCol();
    if (m_Forward) {
        if (startCol >= m_Line.length())
            m_Line.erase();
        else
            m_Line = m_Line.substr(startCol);

    } else {
        if (startCol < m_Line.length())
            m_Line = m_Line.substr(0, startCol);
    }
}

string CTextLineIterator::operator* (void) const
{
    return m_Line;
}

CTextPosition CTextLineIterator::GetPosition() const
{
    if (m_CurLine == m_StartPos.GetRow()) {
        if ((m_Forward && m_Begin) || !(m_Forward || m_Begin))
            return m_StartPos;
    }

    return CTextPosition(m_CurLine, 0);
}

void CTextLineIterator::x_Next(void)
{
    if (m_End)
        return;

    m_Line.erase();

    if (!m_Begin && m_CurLine == m_StartPos.GetRow()) {
        m_End = true;
        return;
    }

    m_Begin = false;

    if (m_Forward)
        ++m_CurLine;
    else
        --m_CurLine;

    ITextItem* root = m_Panel->GetRootItem();
    size_t lines = root->GetLineCount();
    if (m_CurLine < 0)
        m_CurLine = lines - 1;
    else if (m_CurLine >= lines)
        m_CurLine = 0;

    CTextItem* item = dynamic_cast<CTextItem*>(root->GetItemByLine(m_CurLine, true));
    if (!item) {
        m_End = true;
        return;
    }

    CCopyLines ostream(m_Line, m_CurLine - item->GetStartLine());
    item->RenderText(ostream, m_Panel->GetContext());

    if (m_CurLine == m_StartPos.GetRow()) {
        int startCol = m_StartPos.GetCol();
        if (m_Forward) {
            if (startCol < m_Line.length())
                m_Line = m_Line.substr(0, startCol);
        } else {
            if (startCol >= m_Line.length())
                m_Line.erase();
            else
                m_Line = m_Line.substr(startCol);
        }
    }
}

bool CTextLineIterator::x_Valid(void) const
{
    return !m_End;
}

CTextLineIterator& CTextLineIterator::operator++ (void)
{
    if (x_Valid()) {
        x_Next();
    } else {
        throw std::out_of_range("Invalid index");
    }

    return *this;
}

CTextLineIterator::operator bool() const
{
    return x_Valid();
}

END_NCBI_SCOPE
