/*  $Id: expand_item.cpp 39653 2017-10-24 18:50:24Z katargir $
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

#include <gui/widgets/text_widget/expand_item.hpp>
#include <gui/widgets/text_widget/selection_helper.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>

#include <gui/widgets/text_widget/calc_nbpos_stream.hpp>

#include <wx/dcclient.h>

BEGIN_NCBI_SCOPE

CExpandItem::CExpandItem(CTextItem* collapsedItem, CTextItem* expandedItem, bool expand)
    : m_Glyph2(), m_Selectable(true), m_Container(), m_Index(0)
{
    _ASSERT(collapsedItem);
    _ASSERT(expandedItem);

    m_Expanded = expand ? 1 : 0;
    m_Items[0] = collapsedItem;
    collapsedItem->SetContainer(this, 0);
    m_Items[1] = expandedItem;
    expandedItem->SetContainer(this, 1);

    m_Glyph = new CExpandGlyph(this);
}

CExpandItem::~CExpandItem()
{
    delete m_Items[0];
    if (m_Items[1] != m_Items[0])
        delete m_Items[1];

    delete m_Glyph;
    delete m_Glyph2;
}

void CExpandItem::AddGlyph(CGlyph* glyph)
{
    delete m_Glyph2;
    m_Glyph2 = glyph;
}

const CConstRef<CObject> CExpandItem::GetAssosiatedObject() const
{
    CConstRef<CObject> obj = m_Items[1]->GetAssosiatedObject();
    if (!obj) {
        obj = m_Items[0]->GetAssosiatedObject();
    }
    return obj;
}

void CExpandItem::Toggle(CTextPanelContext* context, bool update)
{
    m_Expanded ^= 1;

    if (update) {
        if ((m_Expanded & 1) == 0)
            x_LimitCollapsedSelection(context);

        if (m_Container)
            m_Container->ItemSizeChanged(m_Index);

        CTextItemPanel* textPanel = context->GetPanel();
        textPanel->Layout();
        textPanel->Refresh();
        textPanel->UpdateCaretPos();
    }
}

string CExpandItem::GetToolTipText(CTextPanelContext* context) const
{
    return m_Items[m_Expanded]->GetToolTipText(context);
}

void CExpandItem::CalcSize(wxDC& dc, CTextPanelContext* context)
{
    m_Items[0]->CalcSize(dc, context);
    m_Items[1]->CalcSize(dc, context);
    wxSize size = context->GetWSize();
    m_Glyph->SetRect(wxRect(m_Items[0]->GetTextLeftMargin(context)- size.x, 0, size.x, size.y));
    m_Items[0]->SetLineCount(m_Items[1]->GetLineCount());
}

void CExpandItem::Draw(wxDC& dc, const wxRect& updateRect, CTextPanelContext* context)
{
    if (IsSelectable()) {
        CSelectionHelper& selHelper = CSelectionHelper::GetInstance();
        selHelper.SetItem(this);
        selHelper.Draw(dc, updateRect, context);
    }
 
    m_Items[m_Expanded]->Draw(dc, updateRect, context);

    wxRect rect = m_Glyph->GetRect();
    if (updateRect.Intersects(rect)) {
        wxPoint dcOrigin = context->GetDeviceOrigin();
        context->SetDeviceOrigin(wxPoint(dcOrigin.x + rect.GetLeft(), dcOrigin.y + rect.GetTop()));
        context->SetDeviceOrigin(dc);
        m_Glyph->Draw(dc, context);
        context->SetDeviceOrigin(dcOrigin);
    }

    if (m_Glyph2) {
        wxRect rect = m_Glyph2->GetRect();
        if (updateRect.Intersects(rect)) {
            wxPoint dcOrigin = context->GetDeviceOrigin();
            context->SetDeviceOrigin(wxPoint(dcOrigin.x + rect.GetLeft(), dcOrigin.y + rect.GetTop()));
            context->SetDeviceOrigin(dc);
            m_Glyph2->Draw(dc, context);
            context->SetDeviceOrigin(dcOrigin);
        }
    }
}

void CExpandItem::GetText(wxTextOutputStream& os,
                   const CTextBlock& block,
                   CTextPanelContext* context)
{
    m_Items[1]->GetText(os, block, context);
}

void CExpandItem::MouseEvent(wxMouseEvent& event, CTextPanelContext& context)
{
    wxRect rect = m_Glyph->GetRect();
    if (rect.Contains(event.m_x, event.m_y)) {
        wxMouseEvent ev(event);
        ev.m_x -= rect.x;
        ev.m_y -= rect.y;
        m_Glyph->MouseEvent(ev);
        return;
    }

    if (m_Glyph2) {
        wxRect rect = m_Glyph2->GetRect();
        if (rect.Contains(event.m_x, event.m_y)) {
            wxMouseEvent ev(event);
            ev.m_x -= rect.x;
            ev.m_y -= rect.y;
            m_Glyph2->MouseEvent(ev);
            return;
        }
    }

    if (IsSelectable()) {
        CSelectionHelper& selHelper = CSelectionHelper::GetInstance();
        selHelper.SetItem(this);
        if (selHelper.ProcessEvent(event))
            return;
    }

    m_Items[m_Expanded]->MouseEvent(event, context);
}

void CExpandItem::UpdatePositions()
{
	for (size_t i = 0; i < 2; ++i) {
		m_Items[i]->SetContainer(this, i);
		IContainer* container = dynamic_cast<IContainer*>(m_Items[i]);
		if (container)
			container->UpdatePositions();
	}
}

bool CExpandItem::Traverse(ICompositeTraverser& traverser)
{
    if (!traverser.ProcessItem(*this))
        return false;

    return m_Items[1]->Traverse(traverser);
}

void CExpandItem::x_LimitCollapsedSelection(CTextPanelContext* context) const
{
    CCalcNBPosOStream ostr;
    m_Items[0]->RenderText(ostr, context);
    int nbStart, nbEnd, nbRow;
    ostr.GetNBPos(nbStart, nbEnd, nbRow);

    CTextItemPanel* textPanel = context->GetPanel();
    CTextBlock block = textPanel->GetTextSelection();

    int startLine = m_Items[0]->GetStartLine();

    block.ShiftRow(-startLine);
    size_t lineCount = m_Items[0]->GetLineCount();
    int indent = m_Items[0]->GetIndent();

    CTextPosition blockStart(0, nbStart);
    CTextPosition blockEnd(lineCount - 1, indent);

    int pos1 = 0, pos2 = 0;

    if (block.GetStart() > blockStart)
        pos1 = (block.GetStart() >= blockEnd) ? 2 : 1;

    if (block.GetEnd() > blockStart)
        pos2 = (block.GetEnd() >= blockEnd) ? 2 : 1;


    blockStart.ShiftRow(startLine);
    block.ShiftRow(startLine);

    if (pos1 != 1 && pos2 != 1) return;

    if (pos1 == 1 && pos2 == 1)
        textPanel->SetSelection(CTextBlock(blockStart, blockStart));
    else if (pos1 == 2)
        textPanel->SetSelection(CTextBlock(block.GetStart(), blockStart));
    else if (pos2 == 2)
        textPanel->SetSelection(CTextBlock(blockStart, block.GetEnd()));
    else if (pos1 == 0)
        textPanel->SetSelection(CTextBlock(block.GetStart(), blockStart));
    else if (pos2 == 0)
        textPanel->SetSelection(CTextBlock(blockStart, block.GetEnd()));
}

END_NCBI_SCOPE
 
