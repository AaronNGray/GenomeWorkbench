/*  $Id: text_item.cpp 38729 2017-06-13 15:50:41Z katargir $
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

#include <gui/widgets/text_widget/text_item.hpp>
#include <gui/widgets/text_widget/selection_helper.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/text_widget/draw_text_stream.hpp>

#include <wx/dcclient.h>

BEGIN_NCBI_SCOPE

#ifdef _DEBUG
CAtomicCounter_WithAutoInit gITextItemCount;
#endif

class NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CCalcSizeStream : public CStyledTextOStream
{
public:
    CCalcSizeStream() : m_LineSize(0), m_LineCount(0), m_MaxLineSize(0) {}

    virtual StreamHint GetHint() const { return kDisplay; }

    virtual CStyledTextOStream& operator<< (const string& text) { x_AddChars((int)text.length()); return *this; }

    virtual int CurrentRowLength() const { return m_LineSize; }

    virtual void SetStyle(const CTextStyle* WXUNUSED(style)) {}
    virtual void SetDefaultStyle() {}
    virtual void Indent (size_t steps) { x_AddChars((int)steps); }
    virtual void NewLine() { m_LineSize = 0; ++m_LineCount; }

    wxSize GetSize() const;

private:
    void x_AddChars(int count)
    {
        m_LineSize += count;
        if (m_LineSize > m_MaxLineSize)
            m_MaxLineSize = m_LineSize;
    }
    int    m_LineSize;
    int    m_LineCount;
    int    m_MaxLineSize;
};

wxSize CCalcSizeStream::GetSize() const
{
    int lineCount = (m_LineCount == 0 && m_MaxLineSize  > 0) ? 1 : m_LineCount;
    return wxSize(m_MaxLineSize, lineCount); 
}

int CTextItem::GetTextLeftMargin(CTextPanelContext* context) const
{
    return context->GetLeftMargin() + context->GetIndentWidth()*GetIndent();
}

void CTextItem::CalcSize(wxDC& /*dc*/, CTextPanelContext* context)
{
    CCalcSizeStream ostream;
    x_RenderText(ostream, context);
    wxSize size = ostream.GetSize();

    wxSize wSize = context->GetWSize();
    m_Size = wxSize(size.GetWidth()*wSize.GetWidth(), size.GetHeight()*wSize.GetHeight());
    m_Size.IncBy(context->GetLeftMargin(), 0);
    m_LineCount = size.GetHeight();
}

namespace {
class CSelectionModifier : public CDrawTextOStream::IStyleModifier
{
public:
    static CSelectionModifier& GetInstance()
    {
        static CSelectionModifier _inst;
        return _inst;
    }
    virtual void SetStyle(wxDC& dc)
    {
        wxColor background(49, 106, 197);
        dc.SetBackgroundMode(wxSOLID);
        dc.SetTextForeground(wxColor(255,255,255));
        dc.SetTextBackground(background);
        dc.SetBrush(wxBrush(background));
        dc.SetPen(wxPen(background));
    }
};
class CTestModifier : public CDrawTextOStream::IStyleModifier
{
public:
    static CDrawTextOStream::IStyleModifier& GetInstance()
    {
        static CTestModifier _inst;
        return _inst;
    }
    virtual void SetStyle(wxDC& dc)
    {
        dc.SetBackgroundMode(wxTRANSPARENT);
        dc.SetTextForeground(wxColor(0,222,0));
    }
};
}

void CTextItem::Draw(wxDC& dc, const wxRect& updateRect, CTextPanelContext* context)
{
    if (IsSelectable()) {
        CSelectionHelper& selHelper = CSelectionHelper::GetInstance();
        selHelper.SetItem(this);
        selHelper.Draw(dc, updateRect, context);
    }

    CTextBlock block = context->GetPanel()->GetTextSelection().Normalized();
    block.ShiftRow(-GetStartLine());

    CTextPosition start = block.GetStart();
    CTextPosition end = block.GetEnd();

    TextPosToCollapsed(start, context);
    TextPosToCollapsed(end, context);
    block = CTextBlock(start, end);

    context->SetDeviceOrigin(dc);
    CDrawTextOStream ostream(dc, context->GetLeftMargin(), 0,
                             dc.GetCharHeight(), context->GetIndentWidth());

    ostream.AddMarker(block.GetStart().GetRow(), block.GetStart().GetCol(),
        block.GetEnd().GetRow(), block.GetEnd().GetCol(), &CSelectionModifier::GetInstance());
    //ostream.AddMarker(1, 7, 2, 3, &CTestModifier::GetInstance());

    context->InitDrawStream(ostream, GetStartLine());

    x_RenderText(ostream, context);
}

void CTextItem::x_Indent(CStyledTextOStream& ostream) const
{
     ostream << Indent(GetIndent());
}

void CTextItem::MouseEvent(wxMouseEvent& event, CTextPanelContext& context)
{
    wxRect rect(wxPoint(0, 0), GetSize());
    if (rect.Contains(event.GetPosition())) {
        context.MouseOverItem(this);
    }

    if (IsSelectable()) {
        CSelectionHelper& selHelper = CSelectionHelper::GetInstance();
        selHelper.SetItem(this);
        if (selHelper.ProcessEvent(event))
            return;
    }
    event.Skip();
}

ITextItem* CTextItem::GetItemByLine(int lineNum, bool /*expanded*/)
{
    if (lineNum >= 0 && lineNum < GetLineCount())
        return this;
    return 0;
}

END_NCBI_SCOPE
