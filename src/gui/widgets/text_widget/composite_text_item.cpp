/*  $Id: composite_text_item.cpp 35549 2016-05-20 15:37:51Z katargir $
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

#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/text_widget/composite_text_item.hpp>
#include <gui/widgets/text_widget/composite_traverser.hpp>
#include <gui/widgets/text_widget/expand_item.hpp>

#include <wx/dcclient.h>

BEGIN_NCBI_SCOPE

CCompositeTextItem::~CCompositeTextItem()
{
	if (m_DeleteChildren) {
		vector<ITextItem*>::iterator it;
		for (it = m_Items.begin(); it != m_Items.end(); ++it)
			delete *it;
	}
}

bool CCompositeTextItem::Traverse(ICompositeTraverser& traverser)
{
    for (vector<ITextItem*>::iterator it = m_Items.begin(); it != m_Items.end(); ++it) {
        ITextItem* item = m_Items[it - m_Items.begin()];
        if (!item->Traverse(traverser))
            return false;
    }
    return true;
}

void CCompositeTextItem::CalcSize(wxDC& dc, CTextPanelContext* context)
{
    for (auto i : m_Items) {
        if (context->IsCanceled())
            return;
        if (i) i->CalcSize(dc, context);
    }

    UpdatePositions();
}

ITextItem* CCompositeTextItem::GetItemByLine(int lineNum, bool expanded)
{
    if (lineNum < 0 || lineNum >= GetLineCount() || m_Items.empty())
        return 0;

    vector<int>::iterator it = upper_bound(m_AccumLines.begin(), m_AccumLines.end(), lineNum);
    if (it == m_AccumLines.end())
        return 0;
    size_t index = it - m_AccumLines.begin();
    if (index > 0)
        lineNum -= m_AccumLines[index - 1];

    return m_Items[index]->GetItemByLine(lineNum, expanded);
}

wxPoint CCompositeTextItem::GetItemPosition(int index) const
{
    wxPoint pos = GetPosition();

    if (index > 0 && (size_t)index < m_AccumHeight.size())
        return pos + wxPoint(0, m_AccumHeight[index - 1]);
    return pos;
}

int CCompositeTextItem::GetItemStartLine(int index) const
{
    int startLine = GetStartLine();

    if (index > 0 && (size_t)index < m_AccumLines.size())
        return startLine + m_AccumLines[index - 1];
    return startLine;
}

void CCompositeTextItem::ItemSizeChanged(int index)
{
    UpdatePositions();

    if (m_Container)
        m_Container->ItemSizeChanged(m_Index);
}

void CCompositeTextItem::Draw(wxDC& dc, const wxRect& updateRect, CTextPanelContext* context)
{
    wxPoint dcOrigin = context->GetDeviceOrigin();

    vector<int>::iterator posIt = upper_bound(m_AccumHeight.begin(), m_AccumHeight.end(), updateRect.GetTop());

    for (size_t index = posIt - m_AccumHeight.begin(); index < m_Items.size(); ++index) {
        ITextItem* item = m_Items[index];
        int top = (index > 0) ? m_AccumHeight[index - 1] : 0;
        if (top > updateRect.GetBottom())
            break;

        wxRect rect(updateRect);
        rect.Offset(0, -top);
        context->SetDeviceOrigin(wxPoint(dcOrigin.x, dcOrigin.y + top));
        item->Draw(dc, rect, context);
    }

    context->SetDeviceOrigin(dcOrigin);
}

void CCompositeTextItem::MouseEvent(wxMouseEvent& event, CTextPanelContext& context)
{
// This is only container, it should only distribute events
//  TParent::MouseEvent(event, context);
//  if (!event.GetSkipped())
//      return;

    if (event.Moving()) {
        context.MouseMoveInContainer(this);
    }

    CTextItemPanel* panel = static_cast<CTextItemPanel*>(event.GetEventObject());

    if (m_Items.empty()) return;

    int x = event.m_x, y = event.m_y;

    vector<int>::iterator posIt = upper_bound(m_AccumHeight.begin(), m_AccumHeight.end(), y);
    if (posIt == m_AccumHeight.end()) --posIt;
    size_t index = posIt - m_AccumHeight.begin();

    ITextItem* item = m_Items[index];

    if (index > 0)
        y -= m_AccumHeight[index - 1];

    wxSize size = item->GetSize();
    if (panel->Selecting() || (0 <= y && y < size.GetHeight())) {
        wxMouseEvent ev(event);
        ev.m_x = x;
        ev.m_y = y;
        item->MouseEvent(ev, context);
    }
    else
        panel->SetCursor(wxCursor(wxCURSOR_ARROW));
}

void CCompositeTextItem::AddItem (ITextItem* item, bool updatePositions)
{
    InsertItems (m_Items.size(), &item, 1, updatePositions);
}

ITextItem* CCompositeTextItem::GetItem(size_t index)
{
    return (index < m_Items.size()) ? m_Items[index] : 0;
}

void CCompositeTextItem::GetText(wxTextOutputStream& os,
                                 const CTextBlock& block,
                                 CTextPanelContext* context)
{
    for (size_t i = 0; i < m_Items.size(); ++i) {
        CTextBlock itemBlock = m_Items[i]->GetItemBlock();

        if (itemBlock.GetEnd().GetRow() <= block.GetStart().GetRow())
            continue;
        if (itemBlock.GetStart().GetRow() > block.GetEnd().GetRow())
            break;

        m_Items[i]->GetText(os, block, context);
    }
}

void CCompositeTextItem::InsertItems (size_t index, ITextItem* const * items, size_t count, bool updatePositions)
{
    if (index > m_Items.size()) index = m_Items.size();
    m_Items.insert(m_Items.begin() + index, items, items + count);

    m_AccumLines.resize(m_AccumLines.size() + count);
    m_AccumHeight.resize(m_AccumHeight.size() + count);

    if (updatePositions) {
        UpdatePositions();
        if (m_Container)
            m_Container->ItemSizeChanged(m_Index);
    }
}

void CCompositeTextItem::RemoveItem (size_t index)
{
    if (index >= m_Items.size()) return;
	if (m_DeleteChildren) {
		delete m_Items[index];
	}
    m_Items.erase(m_Items.begin() + index);
    m_AccumLines.erase(m_AccumLines.begin() + index);
    m_AccumHeight.erase(m_AccumHeight.begin() + index);
    UpdatePositions();

    if (m_Container)
        m_Container->ItemSizeChanged(m_Index);
}

void CCompositeTextItem::UpdatePositions()
{
    m_LineCount = 0;
    m_Size = wxSize(0,0);

    for (size_t i = 0; i < m_AccumLines.size(); ++i) {
        ITextItem* item = m_Items[i];
        if (!item) continue;
        item->SetContainer(this, i);
		IContainer* container = dynamic_cast<IContainer*>(item);
		if (container) {
			container->UpdatePositions();
		}
        m_LineCount += item->GetLineCount();
        m_AccumLines[i] = m_LineCount;
        
        wxSize childSize = item->GetSize();
        m_Size.SetWidth(max(m_Size.GetWidth(), childSize.GetWidth()));
        m_Size.IncBy(0, childSize.GetHeight());

        m_AccumHeight[i] = m_Size.GetHeight();
    }
}

END_NCBI_SCOPE
