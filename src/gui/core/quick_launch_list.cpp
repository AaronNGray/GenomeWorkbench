/*  $Id: quick_launch_list.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors: Roman Katargin
 *
 */

#include <ncbi_pch.hpp>

#include <gui/core/quick_launch_list.hpp>
#include <gui/objutils/registry.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/menu.h>

#include <set>

BEGIN_NCBI_SCOPE

static const string kQLPath = "Workbench.Tools.QuickLaunch";

CQuickLaunchList::CQuickLaunchList()
: m_ToolList(10)
{
}

CQuickLaunchList& CQuickLaunchList::GetInstance()
{
    static CRef<CQuickLaunchList> s_Instance;
    if (!s_Instance) {
        s_Instance.Reset(new CQuickLaunchList());
        s_Instance->x_Load();
    }
    return *s_Instance;
}

string CQuickLaunchList::GetItemPath(const string& item) const
{
    return kQLPath + ".Entries." + item;
}

void CQuickLaunchList::UpdateMenu(wxMenu& menu, int cmdStart, int cmdEnd, size_t pos)
{
    if (IsEmpty())
        return;

    wxMenu* subMenu = new wxMenu();
    const list<string>& items = m_ToolList.GetItems();
    int cmd = cmdStart;
    ITERATE(list<string>, it, items) {
        subMenu->Append(cmd, ToWxString(*it));
        if (cmd == cmdEnd)
            break;
        ++cmd;
    }

    if (pos == (size_t)-1)
        pos = menu.GetMenuItemCount();

    wxMenuItem* item = wxMenuItem::New(&menu, wxID_ANY,
                            wxT("Quick Launch"), wxEmptyString, wxITEM_NORMAL, subMenu);
    menu.Insert(pos, item);
}

void CQuickLaunchList::x_Load()
{
    CGuiRegistry::GetInstance().GetStringList(kQLPath + ".List", m_ToolList.SetItems());
}

void CQuickLaunchList::x_Save() const
{
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();

    list<string> items = m_ToolList.GetItems();
    gui_reg.Set(kQLPath + ".List", items);

    set<string> itemsSet;
    std::copy(items.begin(), items.end(), inserter(itemsSet, itemsSet.begin()));

    CRegistryReadView view = gui_reg.GetReadView(kQLPath + ".Entries");
    CRegistryReadView::TKeys entries;
    view.GetTopKeys(entries);
    ITERATE(CRegistryReadView::TKeys, it, entries) {
        const CRegistryReadView::SKeyInfo& key = *it;
        if (itemsSet.find(key.key) == itemsSet.end())
            gui_reg.DeleteField(kQLPath + ".Entries." + key.key);
    }
}

bool CQuickLaunchList::IsEmpty() const
{
    return m_ToolList.GetItems().empty();
}

const list<string>& CQuickLaunchList::GetItems() const
{
    return m_ToolList.GetItems();
}

void CQuickLaunchList::AddItem(const string& item)
{
    m_ToolList.AddItem(item);
    x_Save();
}

void CQuickLaunchList::RemoveItem(const string& item)
{
    m_ToolList.RemoveItem(item);
    x_Save();
}

END_NCBI_SCOPE
