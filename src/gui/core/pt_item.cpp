/*  $Id: pt_item.cpp 34027 2015-10-22 21:00:44Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/core/pt_item.hpp>

#include <wx/treectrl.h>

BEGIN_NCBI_SCOPE

using namespace PT;

#ifdef _DEBUG
size_t CItem::m_InstanceCount = 0;
#endif

void CItem::LogInstanceCount()
{
#ifdef _DEBUG
    LOG_POST(Info << "Number of CItem instances: " << m_InstanceCount);
#endif
}


CItem::CItem()
{
#ifdef _DEBUG
    ++m_InstanceCount;
#endif
}

CItem::~CItem()
{
#ifdef _DEBUG
    --m_InstanceCount;
#endif
}


wxString CItem::GetLabel(wxTreeCtrl& treeCtrl) const
{
    return treeCtrl.GetItemText(m_TreeItemId);
}

CItem* CItem::GetParent(wxTreeCtrl& treeCtrl) const
{
    wxTreeItemId pId = treeCtrl.GetItemParent(m_TreeItemId);
    if (!pId.IsOk()) return 0;
    return dynamic_cast<CItem*>(treeCtrl.GetItemData(pId));
}


void CItem::AppendChildItem(wxTreeCtrl& treeCtrl, CItem& ch_item)
{
    ch_item.m_TreeItemId =
        treeCtrl.AppendItem(m_TreeItemId, wxT("item"), -1, -1, &ch_item);
}

void CItem::InsertChildItem(wxTreeCtrl& treeCtrl, CItem& ch_item, size_t pos)
{
    if (pos > treeCtrl.GetChildrenCount(m_TreeItemId, false))
        AppendChildItem(treeCtrl, ch_item);
    else {
        ch_item.m_TreeItemId =
            treeCtrl.InsertItem(m_TreeItemId, pos, wxT("item"), -1, -1, &ch_item);
    }
}

CChildIterator::CChildIterator(wxTreeCtrl& treeCtrl, const CItem& item)
    : m_TreeCtrl(treeCtrl), m_Item(item.GetTreeItemId())
{
    m_Child = m_TreeCtrl.GetFirstChild(m_Item, m_Cookie);
}

CChildIterator& CChildIterator::operator++ (void)
{
    if (!m_Child.IsOk())
        throw std::out_of_range("Invalid index");
    m_Child = m_TreeCtrl.GetNextChild(m_Item, m_Cookie);
    return *this;
}

CChildIterator::operator bool() const
{
    return m_Child.IsOk();
}

CItem& CChildIterator::operator* (void) const
{
    if (!m_Child.IsOk())
        throw std::out_of_range("Invalid index");

    CItem* item = dynamic_cast<CItem*>(m_TreeCtrl.GetItemData(m_Child));
    _ASSERT(item);
    return *item;
}

END_NCBI_SCOPE
