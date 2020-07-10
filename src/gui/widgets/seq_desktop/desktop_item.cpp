/*  $Id: desktop_item.cpp 37370 2016-12-29 21:44:57Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <serial/typeinfo.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/seq_desktop/desktop_item.hpp>

#include <wx/dcmemory.h>

BEGIN_NCBI_SCOPE

namespace {
    const wxFont s_DesktopFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Consolas"));
}

CDesktopItem::CDesktopItem(CConstRef<IDesktopDataItem> item)
    : m_Pos(wxPoint(0, 0)), m_TextHeight(0), m_Origin(wxPoint(20, 20)), 
    m_Item(item), m_Show(false), m_ExpandLevel(0), m_Indent(33), m_Selected(false)
{
    if (!m_Item) return;
    Init();
}

void CDesktopItem::Init()
{
    m_Text = m_Item->GetDescription();
    m_FrameCol = m_Item->GetFrameColor();
    m_TextCol = m_Item->GetTextColor();
    m_BkgdBrush = m_Item->GetBackgroundBrush();
}

void CDesktopItem::Draw(wxDC& dc, bool highlight) const
{
    dc.SetPen(m_FrameCol);
    dc.SetBrush(m_BkgdBrush);

    int height = (m_Item->ShouldBeReduced()) ? m_Size.GetHeight() - 1 : m_Size.GetHeight();
    wxRect rect(m_Pos, wxSize(m_Size.GetWidth(), height));
    
    dc.DrawRectangle(rect);
    dc.SetTextForeground(m_TextCol);
    dc.DrawLabel(m_CurrentText, rect, wxALIGN_TOP | wxALIGN_LEFT);

    if (highlight) {
        dc.SetPen(*wxWHITE_PEN);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRectangle(GetRect());
    }
    if (m_Selected) {
        dc.SetPen(wxPen(*wxBLACK, 2));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        if (m_Item->ShouldBeReduced()) {
            dc.DrawRectangle(GetRect());
        }
        else {
            wxRect rect(GetRect());
            dc.DrawRectangle(
                wxRect(rect.GetPosition(),
                wxSize(rect.GetSize().GetWidth(), rect.GetSize().GetHeight() + 1)));
        }
    }
}

const string& CDesktopItem::GetType() const
{
    _ASSERT(m_Item);
    return m_Item->GetType();
}

const CConstRef<CObject> CDesktopItem::GetAssociatedObject() const
{
    return ConstRef(m_Item->GetObject());
}

bool CDesktopItem::FindSearchItem(const string& search, bool case_sensitive) const
{
    for (auto& it : m_Text) {
        SIZE_TYPE pos = NStr::Find(it, search, (case_sensitive) ? NStr::eCase : NStr::eNocase);
        if (pos != NPOS) {
            return true;
        }
    }
    return false;
}

void CDesktopItem::x_SetTextToDrawAndSize()
{
    wxMemoryDC dc;
    // the size of the text determines the size of the rectangle
    wxSize textSize = x_GetVisibleTextSize(dc);
    m_Size = textSize + wxSize(20, 5);
}

static wxSize s_GetMultiLineSize(wxDC& dc, const wxString& text)
{
    wxCoord width, height;
    dc.GetMultiLineTextExtent(text, &width, &height, NULL, &s_DesktopFont);
    return wxSize(width, height);
}

//  CSimpleDesktopItem

CSimpleDesktopItem::CSimpleDesktopItem(CConstRef<IDesktopDataItem> item)
    : CDesktopItem(item)
{
    x_SetTextToDrawAndSize();
}

void CSimpleDesktopItem::Expand()
{
    m_ExpandLevel++;
    x_SetTextToDrawAndSize();
}

void CSimpleDesktopItem::Collapse()
{
    if (m_ExpandLevel <= 0) return;

    m_ExpandLevel--;
    x_SetTextToDrawAndSize();
}

void CSimpleDesktopItem::UpdateSize()
{
}


IDesktopItem* CSimpleDesktopItem::FindItem(const wxPoint& pt) const
{
    if (GetRect().Contains(pt.x, pt.y)) {
        return (IDesktopItem*)this;
    }
    return nullptr;
}

wxSize CSimpleDesktopItem::x_GetVisibleTextSize(wxDC& dc)
{
    string text;
    if (m_ExpandLevel >= (int)m_Text.size()) {
        // draw all lines
        for (const auto& it : m_Text) {
            text.append(it);
            text.append("\n");
        }
    }
    else {
        for (auto i = 0; i < m_ExpandLevel + 1; ++i) {
            text.append(m_Text[i]);
            text.append("\n");
        }
    }

    text.pop_back();
    m_CurrentText = ToWxString(text);
    wxSize size = s_GetMultiLineSize(dc, m_CurrentText);
    m_TextHeight = size.y + 2;  // 2 - to have vertical white space around the text
    return size;
}

void CSimpleDesktopItem::UpdatePositions(int* cumY, int depth)
{
    m_Pos.x = depth*m_Indent + m_Origin.x;
    m_Pos.y = *cumY;
    *cumY += GetRelevantHeight();
}

//  CCompositeDesktopItem

CCompositeDesktopItem::CCompositeDesktopItem(CConstRef<IDesktopDataItem> item)
    : CDesktopItem(item)
{
    x_SetTextToDrawAndSize();
}

void CCompositeDesktopItem::Add(IDesktopItem& item)
{
    m_ItemList.push_back(CIRef<IDesktopItem>(&item));
}

void CCompositeDesktopItem::Expand()
{
    m_ExpandLevel++;
    if (m_ExpandLevel == 1) {
        for (auto& it : m_ItemList) {
            it->Show(true);
            if (it->GetType() == objects::CBioseq::GetTypeInfo()->GetName()
                || it->GetType() == objects::CBioseq_set::GetTypeInfo()->GetName()) {
                it->Expand();
            }
        }
    }
    else {
        for (auto& it : m_ItemList) {
            it->Expand();
        }
    }
    x_SetTextToDrawAndSize();
}

void CCompositeDesktopItem::Collapse()
{
    if (m_ExpandLevel <= 0) return;
    
    m_ExpandLevel--;
    if (m_ExpandLevel == 0) {
        for (auto& it : m_ItemList) {
            if (it->GetType() == objects::CBioseq::GetTypeInfo()->GetName()
                || it->GetType() == objects::CBioseq_set::GetTypeInfo()->GetName()) {
                it->Collapse();
            }
            else {
                it->Show(false);
            }
        }
    }
    else  {
        for (auto& it : m_ItemList) {
            it->Collapse();
        }
    }
    x_SetTextToDrawAndSize();
}

void CCompositeDesktopItem::UpdateSize()
{
    m_Size.y = 0;
    // contribution from children items

    // flag to signal whether the last child of this composite item has a white frame color 
    bool last_child_white_frame = false;
    for (auto& it : m_ItemList) {
        if (it->IsShown()) {
            it->UpdateSize();
            m_Size.y += it->GetSize().GetHeight();
            int new_width = it->GetSize().GetWidth() + (it->GetPosition().x - m_Pos.x);
            if (new_width >= m_Size.x) {
                new_width = new_width + 2;  // for making the outline visible
            }
            m_Size.x = max(new_width, m_Size.x);
            last_child_white_frame = (it->GetFrameColor() == *wxWHITE) ? true : false;
        }
    }

    // contribution of the actual item
    m_Size.y += m_TextHeight;
}

bool CCompositeDesktopItem::Traverse(IDesktopCompositeTraverser& traverser)
{
    CDesktopItem::Traverse(traverser);
    for (auto& it : m_ItemList) {
        if (!it->Traverse(traverser))
            return false;
    }
    return true;
}

IDesktopItem* CCompositeDesktopItem::FindItem(const wxPoint& pt) const
{
    for (auto& it : m_ItemList) {
        if (it->IsShown()) {
            IDesktopItem* hit = it->FindItem(pt);
            if (hit) {
                return hit;
            }
        }
    }

    if (GetRect().Contains(pt.x, pt.y)) {
        return (IDesktopItem*)this;
    }
    return nullptr;
}

void CCompositeDesktopItem::ShowTillBioseq(bool value)
{
    for (auto& it : m_ItemList) {
        if (it->GetType() == objects::CBioseq::GetTypeInfo()->GetName()
            || it->GetType() == objects::CBioseq_set::GetTypeInfo()->GetName()) {
            it->Show(value);
            it->ShowTillBioseq(value);
        }
    }
}

int CCompositeDesktopItem::GetRelevantHeight() const
{
    return m_TextHeight;
}

wxSize CCompositeDesktopItem::x_GetVisibleTextSize(wxDC& dc)
{
    string text;
    if (m_ExpandLevel >= (int)m_Text.size()) {
        // draw all lines
        for (const auto& it : m_Text) {
            text.append(it);
            text.append("\n");
        }
    }
    else {
        for (auto i = 0; i < m_ExpandLevel + 1; ++i) {
            text.append(m_Text[i]);
            text.append("\n");
        }
    }

    text.pop_back();
    m_CurrentText = ToWxString(text);
    wxSize size = s_GetMultiLineSize(dc, m_CurrentText);
    m_TextHeight = size.y + 2; // 2 - to have vertical white space around the text
    return size;
}

void CCompositeDesktopItem::UpdatePositions(int* cumY, int depth)
{
    m_Pos.x = depth*m_Indent + m_Origin.x;
    m_Pos.y = *cumY;
    *cumY += GetRelevantHeight();
    for (auto& it : m_ItemList) {
        if (it->IsShown()) {
            it->UpdatePositions(cumY, depth + 1);
        }
    }
}

//  CRootDesktopItem

CRootDesktopItem::CRootDesktopItem(CConstRef<IDesktopDataItem> item)
    : CCompositeDesktopItem(item)
{
    Init();
}

void CRootDesktopItem::Init()
{
    m_Pos = m_Origin;
    m_Text = m_Item->GetDescription(true);
    m_FrameCol = *wxBLUE;
    m_TextCol = *wxBLUE;
    m_BkgdBrush = *wxWHITE_BRUSH;
    x_SetTextToDrawAndSize();
    m_TextHeight = m_Size.GetHeight();
    m_OrigSize = m_Size;
}

void CRootDesktopItem::Expand()
{
    m_ExpandLevel++;
    if (m_ExpandLevel == 1) {
        // gather all children up till bioseq
        for (auto& it : m_ItemList) {
            if (it->GetType() == objects::CBioseq::GetTypeInfo()->GetName()
                || it->GetType() == objects::CBioseq_set::GetTypeInfo()->GetName()) {
                it->Show(true);
                it->ShowTillBioseq(true);
            }
        }
    }
    else if (m_ExpandLevel == 2) {
        for (auto& it : m_ItemList) {
            it->Show(true);
            if (it->GetType() == objects::CBioseq::GetTypeInfo()->GetName()
                || it->GetType() == objects::CBioseq_set::GetTypeInfo()->GetName()) {
                it->Expand();
            }
        }
    }
    else {
        for (auto& it : m_ItemList) {
            it->Expand();
        }
    }
    x_SetTextToDrawAndSize();
}

void CRootDesktopItem::Collapse()
{
    if (m_ExpandLevel <= 0) return;

    m_ExpandLevel--;
    if (m_ExpandLevel == 0) {
        // all children should not show up
        for (auto& it : m_ItemList) {
            it->Show(false);
            it->ShowTillBioseq(false);
        }
    }
    else if (m_ExpandLevel == 1) {
        for (auto& it : m_ItemList) {
            if (it->GetType() == objects::CBioseq::GetTypeInfo()->GetName()
                || it->GetType() == objects::CBioseq_set::GetTypeInfo()->GetName()) {
                it->Collapse();
            }
            else {
                it->Show(false);
            }
        }
    }
    else {
        for (auto& it : m_ItemList) {
            it->Collapse();
        }
    }

    x_SetTextToDrawAndSize();
}

void CRootDesktopItem::UpdateSize()
{
    m_Size.y = 0;
    // contribution from children items
    for (auto& it : m_ItemList) {
        if (it->IsShown()) {
            it->UpdateSize();
            m_Size.y += it->GetSize().GetHeight();
            m_Size.x = max(it->GetSize().GetWidth() + (it->GetPosition().x - m_Pos.x), m_Size.x);
        }
    }
    // contribution of the original item
    m_Size.y += m_OrigSize.GetHeight();
}

void CRootDesktopItem::UpdatePositions(int* cumY, int depth)
{
    depth = 0;
    *cumY = m_Origin.y + m_OrigSize.GetHeight();
    for (auto& it : m_ItemList) {
        if (it->IsShown()) {
            if (it->GetType() == objects::CContact_info::GetTypeInfo()->GetName()
                || it->GetType() == objects::CCit_sub::GetTypeInfo()->GetName()) {
                depth = 1;
            }
            else {
                depth = 0;
            }
            it->UpdatePositions(cumY, depth);
        }
    }
}

bool CRootDesktopItem::Traverse(IDesktopCompositeTraverser& traverser)
{
    return CCompositeDesktopItem::Traverse(traverser);
}

wxSize CRootDesktopItem::x_GetVisibleTextSize(wxDC& dc)
{
    string text;
    for (const auto& it : m_Text) {
        text.append(it);
        text.append("\n");
    }

    text.pop_back();
    m_CurrentText = ToWxString(text);
    return s_GetMultiLineSize(dc, m_CurrentText);
}

END_NCBI_SCOPE

