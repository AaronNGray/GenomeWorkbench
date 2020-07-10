/*  $Id: text_select_item.cpp 32393 2015-02-19 16:49:23Z katargir $
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

#include <gui/widgets/text_widget/text_select_item.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/text_widget/draw_styled_text.hpp>
#include <gui/widgets/text_widget/calc_textpos_stream.hpp>
#include <gui/widgets/text_widget/calc_caretpos_stream.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/dcclient.h>
#include <wx/txtstrm.h>

BEGIN_NCBI_SCOPE

void CTextSelectItem::MouseEvent(wxMouseEvent& event, CTextPanelContext& context)
{
    CTextItem::MouseEvent(event, context);
    if (!event.GetSkipped())
        return;

    if (event.Moving() || event.Dragging())
        MouseMove(event);
    else if (event.LeftDown())
        MouseLeftDown(event);
    else if (event.LeftUp())
        MouseLeftUp(event);
    else if (event.LeftDClick())
        MouseLeftDClick(event);
}

void CTextSelectItem::MouseMove(wxMouseEvent& event)
{
    CTextItemPanel* panel = static_cast<CTextItemPanel*>(event.GetEventObject());

    if (panel->Selecting()) {
        CTextPanelContext* context = panel->GetContext();
        int textX, textY, caretX, caretY;
        x_CalcTextPos(context, event.m_x, event.m_y, textX, textY, caretX, caretY);

        panel->SelMove(GetStartLine() + textY, textX);
        panel->SetCursor(wxCursor(wxCURSOR_IBEAM));
        wxPoint pos = GetPosition();
        panel->SetCaretPos(pos.x + caretX, pos.y + caretY);
    }
    else
        panel->SetCursor(wxCursor(wxCURSOR_IBEAM));
}



void CTextSelectItem::MouseLeftDown(wxMouseEvent& event)
{
    CTextItemPanel* panel = static_cast<CTextItemPanel*>(event.GetEventObject());
    CTextPanelContext* context = panel->GetContext();

    if (!panel->Selecting()) {
        if (!event.HasAnyModifiers())
            context->SelectItem(0, false, false);

        int textX, textY, caretX, caretY;
        x_CalcTextPos(context, event.m_x, event.m_y, textX, textY, caretX, caretY);

        if (textY < numeric_limits<int>::max()) {
            panel->SelStart(GetStartLine() + textY, textX, event.ShiftDown());
            wxPoint pos = GetPosition();
            panel->SetCaretPos(pos.x + caretX, pos.y + caretY);
        }
    }
}

void CTextSelectItem::MouseLeftUp(wxMouseEvent& event)
{
    CTextItemPanel* panel = static_cast<CTextItemPanel*>(event.GetEventObject());
    CTextPanelContext* context = panel->GetContext();

    if (panel->Selecting()) {
        int textX, textY, caretX, caretY;
        x_CalcTextPos(context, event.m_x, event.m_y, textX, textY, caretX, caretY);
        panel->SelEnd(GetStartLine() + textY, textX);
        wxPoint pos = GetPosition();
        panel->SetCaretPos(pos.x + caretX, pos.y + caretY);
    }
}

namespace // anonymous namespace
{
static bool IsAlpha(char ch)
{
    return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_';
}
} // anonymous namespace

void CTextSelectItem::MouseLeftDClick(wxMouseEvent& event)
{
    CTextItemPanel* panel = static_cast<CTextItemPanel*>(event.GetEventObject());
    CTextPanelContext* context = panel->GetContext();

    wxClientDC dc(panel);
    panel->InitDC(dc);
    CCalcTextPosOStream ostream(dc, context->GetLeftMargin(), 0,
                                dc.GetCharHeight(), context->GetIndentWidth(), event.m_x, event.m_y);
    x_RenderText(ostream, context);

    int textX, textY, caretX, caretY;
    ostream.GetTextPos(textX, textY);

    string line = ostream.GetCurrentLine();
    if (line.empty())
        return;

    if (textX >= line.length())
        textX = line.length() - 1;

    int selStart = textX, selEnd = textX + 1;
    if (line[selStart] == ' ') {
        while (selStart > 0 && line[selStart - 1] == ' ')
            --selStart;
        while (selEnd < line.length() && line[selEnd] == ' ')
            ++selEnd;
    }
    else if (IsAlpha(line[selStart])) {
        while (selStart > 0 && IsAlpha(line[selStart - 1]))
            --selStart;
        while (selEnd < line.length() && IsAlpha(line[selEnd]))
            ++selEnd;
    }

    panel->SelStart(GetStartLine() + textY, selStart, false);
    panel->SelEnd(GetStartLine() + textY, selEnd);


    CCalcCaretPosOStream caretStrm(dc, context->GetLeftMargin(), 0,
                                   dc.GetCharHeight(), context->GetIndentWidth(),
                                   selEnd, textY);
    x_RenderText(caretStrm, context);
    caretStrm.GetCaretPos(caretX, caretY);
    wxPoint pos = GetPosition();
    panel->SetCaretPos(pos.x + caretX, pos.y + caretY);
}

void CTextSelectItem::x_CalcTextPos(CTextPanelContext* context,
                                    int x, int y,
                                    int& textX, int& textY,
                                    int& caretX, int& caretY)
{
    CTextItemPanel* textPanel = context->GetPanel();
    wxClientDC dc(textPanel);
    textPanel->InitDC(dc);
    CCalcTextPosOStream ostream(dc, context->GetLeftMargin(), 0,
                                dc.GetCharHeight(), context->GetIndentWidth(), x, y);
    x_RenderText(ostream, context);
    ostream.GetTextPos(textX, textY);

    CCalcCaretPosOStream caretStrm(dc, context->GetLeftMargin(), 0,
                                   dc.GetCharHeight(), context->GetIndentWidth(),
                                   textX, textY);
    x_RenderText(caretStrm, context);
    caretStrm.GetCaretPos(caretX, caretY);

    CTextPosition pos(textY, textX);
    this->TextPosToExpanded(pos, context);
    textY = pos.GetRow();
    textX = pos.GetCol();
}

namespace // anonymous namespace
{

class CCopyText : public CStyledTextOStream
{
public:
    CCopyText(wxTextOutputStream& os,
              int startRow, int startCol,
              int endRow, int endCol) :
        m_OS(os),
        m_StartRow(startRow), m_StartCol(startCol),
        m_EndRow(endRow), m_EndCol(endCol), m_CurRow(0),
        m_CurrentLineLength(0) {}

    virtual StreamHint GetHint() const { return kCopyText; }
    virtual wxTextOutputStream* GetTextStream() const { return &m_OS; }
    virtual bool IsVisible() const;
    virtual void UpdatePosition(int shift) { m_CurrentLineLength += shift; }

    virtual CStyledTextOStream& operator<< (const string& text);

    virtual int CurrentRowLength() const { return m_CurrentLineLength; }

    virtual void SetStyle(const CTextStyle* /*style*/) {}
    virtual void SetDefaultStyle() {}
    virtual void Indent (size_t steps);
    virtual void NewLine();

private:
    wxTextOutputStream& m_OS;
    int m_StartRow;
    int m_StartCol;
    int m_EndRow;
    int m_EndCol;
    int m_CurRow;
    int m_CurrentLineLength;
};

bool CCopyText::IsVisible() const
{
    if (m_CurRow > m_StartRow && m_CurRow < m_EndRow)
        return true;

    if (m_CurRow == m_StartRow) {
        if (m_CurrentLineLength < m_StartCol)
            return false;
        if (m_CurRow == m_EndRow)
            return m_CurrentLineLength < m_EndCol;
        return true;
    }

    if (m_CurRow == m_EndRow)
        return m_CurrentLineLength < m_EndCol;

    return false;
}

void CCopyText::Indent (size_t steps)
{
    if (steps == 0)
        return;

    this->operator<<(string(steps, ' '));
}

void CCopyText::NewLine()
{
    if (m_CurRow >= m_StartRow && m_CurRow < m_EndRow)
        m_OS << endl;

    ++m_CurRow;
    m_CurrentLineLength = 0;
}

CStyledTextOStream& CCopyText::operator<< (const string& text)
{
    int length = text.length();
    int selStart = length, selEnd = length;

    if (m_CurRow == m_EndRow) {
        selStart = 0;
        selEnd = m_EndCol-m_CurrentLineLength;
    }
    if (m_CurRow == m_StartRow)
        selStart = m_StartCol-m_CurrentLineLength;

    if (m_CurRow > m_StartRow && m_CurRow < m_EndRow) {
        selStart = 0;
        selEnd = length;
    }

    if (selEnd < selStart)
        swap(selStart, selEnd);

    if (selStart < selEnd && selStart < length && selEnd > 0) {
        selStart = max(selStart, 0);
        selEnd = min(selEnd, length);
        m_OS << ToWxString(text.substr(selStart, selEnd - selStart));
    }

    m_CurrentLineLength += length;
    return *this;
}

} // anonymous namespace

void CTextSelectItem::GetText(wxTextOutputStream& os,
                              const CTextBlock& block,
                              CTextPanelContext* context)
{
    CTextBlock b(block);
    b.ShiftRow(-GetStartLine());
    CCopyText ostream(os, b.GetStart().GetRow(), b.GetStart().GetCol(),
                          b.GetEnd().GetRow(), b.GetEnd().GetCol());
    x_RenderText(ostream, context);
}

END_NCBI_SCOPE
