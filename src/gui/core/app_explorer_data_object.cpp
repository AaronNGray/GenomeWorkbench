/*  $Id: app_explorer_data_object.cpp 43778 2019-08-29 19:19:00Z katargir $
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

#include <gui/core/app_explorer_data_object.hpp>

#include <wx/treectrl.h>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
///  CAppExplorerDataObject

wxDataFormat CAppExplorerDataObject::m_ItemsFormat(wxT("gbench/app_explorer_items"));

CAppExplorerDataObject::CAppExplorerDataObject()
    : m_TreeCtrl() , m_Cut()
{
    SetFormat(m_ItemsFormat);
}

CAppExplorerDataObject::CAppExplorerDataObject(wxTreeCtrl* treeCtrl, wxArrayTreeItemIds& items, bool cut)
    : m_TreeCtrl(treeCtrl), m_Items(items), m_Cut(cut)
{
    SetFormat(m_ItemsFormat);
}


//TODO In all functions check whether text object is full (see if it can do it itself)

size_t CAppExplorerDataObject::GetDataSize() const
{
    return sizeof(TData) + sizeof(wxTreeItemId)*m_Items.size();
}

bool CAppExplorerDataObject::GetDataHere(void * buf) const
{
    TData* data = (TData*)buf;
    data->m_Pid = CCurrentProcess::GetPid();
    data->m_TreeCtrl = m_TreeCtrl;
    data->m_Cut = m_Cut;
    data->m_ItemCount = m_Items.size();
    wxTreeItemId* items = (wxTreeItemId*)(data + 1);
    std::copy(m_Items.begin(), m_Items.end(), items);
    return true;
}

bool CAppExplorerDataObject::SetData(size_t len, const void *buf)
{
    bool ok = buf && (len >= sizeof(TData));
    _ASSERT(ok);
    if (!ok) return false;

    m_Items.clear();

    const TData* data = (const TData*)buf;
    if (data->m_Pid != CCurrentProcess::GetPid())
        return false;

    m_TreeCtrl = data->m_TreeCtrl;
    m_Cut      = data->m_Cut;
    wxTreeItemId* items = (wxTreeItemId*)(data + 1);

    for (size_t i = 0; i < data->m_ItemCount; ++i)
        m_Items.push_back(items[i]);

    return true;
}

void CAppExplorerDataObject::GetItems(wxTreeCtrl& treeCtrl, PT::TItems& items)
{
    if (&treeCtrl != m_TreeCtrl) return;

    ITERATE (wxArrayTreeItemIds, it, m_Items) {
        wxTreeItemId id = *it;
        PT::CItem* item = dynamic_cast<PT::CItem*>(treeCtrl.GetItemData(id));
        if (item) items.push_back(item);
    }
}

END_NCBI_SCOPE
