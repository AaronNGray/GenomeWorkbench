/*  $Id: cmd_change_projectitem.cpp 43100 2019-05-14 19:24:34Z filippov $
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
 * Authors: Andrea Asztalos, based on a file by Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <objects/general/Date.hpp>

#include <gui/core/project_service.hpp>
#include <gui/core/open_view_task.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/core/cmd_change_projectitem.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CCmdChangeProjectItem::Execute()
{
    if (!m_OldItem || !m_Doc || !m_NewItem || !m_Workbench)
        return;

    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (!srv) return;

    vector<CIRef<IProjectView>> views;
    srv->FindViews(views, *m_OldItem->GetObject());
    set<string> view_names;
    for (auto& it : views) {
        string viewType = it->GetLabel(IProjectView::eType);
        if (!viewType.empty()) {
            view_names.insert(viewType);
        }
    }

    bool removed = m_Doc->RemoveProjectItem(m_OldItem);
    if (!removed) return;

    CProjectFolder& data_folder = m_Doc->SetData();
    m_Doc->AddItem(*m_NewItem, data_folder);
    m_Doc->AttachProjectItem(m_NewItem);

    m_Doc->SetModifiedDate(CTime(CTime::eCurrent));
    m_Doc->SetDirty(true);
    m_Doc->ProjectItemsChanged();

    const CSerialObject* so = m_NewItem->GetObject();
    if (so && !view_names.empty()) {
        CAppTaskService* taskService = m_Workbench->GetServiceByType<CAppTaskService>();
        SConstScopedObject object(so, m_Doc->GetScope());
        for (auto& it : view_names) {
            COpenViewTask* task = new COpenViewTask(m_Workbench, it, object, 0, false);
            taskService->AddTask(*task);
        }
    }

    swap(m_OldItem, m_NewItem);
}

void CCmdChangeProjectItem::Unexecute()
{
    Execute();
}

string CCmdChangeProjectItem::GetLabel()
{
    return "Change ProjectItem\n";
}

END_NCBI_SCOPE
