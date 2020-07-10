/*  $Id: project_service_test.cpp 35611 2016-06-01 20:01:20Z katargir $
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
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbifile.hpp>

#include <gui/core/project_service_test.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/document.hpp>

#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>
#include <objects/gbproj/ProjectDescr.hpp>
#include <objects/gbproj/GBProject_ver2.hpp>

#include <objects/general/Date.hpp>

#include <objects/seqloc/Seq_id.hpp>

#include <corelib/ncbi_system.hpp>

#include <serial/iterator.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CProjectServiceTestTask::CProjectServiceTestTask(IServiceLocator* srv_locator)
:   CAppTask("Project Service Concurrency Test", true),
    m_SrvLocator(srv_locator),
    m_Service(NULL)
{
    m_Service = m_SrvLocator->GetServiceByType<CProjectService>().GetPointer();

    _ASSERT(m_Service);
}


CProjectServiceTestTask::~CProjectServiceTestTask()
{
}


CProjectServiceTestTask::ETaskState CProjectServiceTestTask::x_Run()
{
    LOG_POST(Info << "CProjectServiceTestTask::x_Run() - BEGIN");

    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();

    CProjectServiceTestJob* job;

    job = new CProjectServiceTestJob(m_Service, CProjectServiceTestJob::ePrintItems, 10);
    disp.StartJob(*job, "ThreadPool");

    job = new CProjectServiceTestJob(m_Service, CProjectServiceTestJob::eCreateItems, 78);
    disp.StartJob(*job, "ThreadPool");

    job = new CProjectServiceTestJob(m_Service, CProjectServiceTestJob::eDeleteItems, 301);
    disp.StartJob(*job, "ThreadPool");

    job = new CProjectServiceTestJob(m_Service, CProjectServiceTestJob::eModifyItems, 13);
    disp.StartJob(*job, "ThreadPool");

    LOG_POST(Info << "CProjectServiceTestTask::x_Run() - BEGIN(completed)");
    return eCompleted;
}


///////////////////////////////////////////////////////////////////////////////
/// CProjectServiceTestJob

CProjectServiceTestJob::CProjectServiceTestJob(CProjectService* service,
                                               EType Type, int sleep_ms)
:   CAppJob(),
    m_ProjectService(service),
    m_Type(Type),
    m_SleepMs(sleep_ms)
{
    string sType;

    switch(m_Type)  {
    case eCreateItems:
        sType = "eCreateItems";
        break;

    case eModifyItems:
        sType = "eModifyItems";
        break;

    case eDeleteItems:
        sType = "eDeleteItems";
        break;

    case ePrintItems:
        sType = "ePrintItems";
        break;
    }

    m_Descr = "Test Job - " + sType;
}


IAppJob::EJobState CProjectServiceTestJob::Run()
{
    static const char* kLogMsg = "CProjectServiceTestJob::Run() - exception in x_Run() ";
    static string kErrorMsg = "Unhandled exception in CProjectServiceTestJob::Run()";

    x_ResetState(); // reset state before execution

    string err_msg;
    try {
        while(true) {
            if (IsCanceled())
                break;

            LOG_POST(Info << m_Descr << " - now working ");

            switch(m_Type)  {
        case eCreateItems:
            x_CreateItems();
            break;

        case eModifyItems:
            x_ModifyItems();
            break;

        case eDeleteItems:
            x_DeleteItems();
            break;

        case ePrintItems:
            x_PrintItems();
            break;

        default:
            _ASSERT(false); //unsupported
            }

            LOG_POST(Info << m_Descr << " - sleeping for " << m_SleepMs << " ms");
            SleepMilliSec(m_SleepMs);
        }
    } catch (CException& e) {
        err_msg = kLogMsg + GetDescr() + ". " + e.GetMsg();
        LOG_POST(Error << err_msg);
        LOG_POST(Error << e.ReportAll());
    }

    CFastMutexGuard lock(m_Mutex);

    if(err_msg.empty())  {
        return eCompleted;
    } else {
        string s = kErrorMsg + err_msg;
        m_Error.Reset(new CAppJobError(s));
        return eFailed;
    }
}

static vector<int> s_GetProjectIds(const CWorkspaceFolder& folder)
{
    vector<int> projectIds;
    for (CTypeConstIterator<CGBProjectHandle> it(folder); it; ++it) {
        const CGBDocument* doc = dynamic_cast<const CGBDocument*>(&*it);
        if (doc && doc->IsLoaded()) {
            projectIds.push_back(doc->GetId());
        }
    }
    return projectIds;
}

static CRef<CGBDocument> s_CreateNewProject(CProjectService* srv, CGBWorkspace& ws)
{
    CGBProject_ver2* prj_obj = new CGBProject_ver2();
    CRef<CGBDocument> doc(new CGBDocument(srv, CGBDocument::GetNextId(), *prj_obj));
    doc->CreateProjectScope();

    string new_name = ws.MakeUniqueProjectTitle("New Project");
    doc->SetDescr().SetTitle(new_name);
    CTime now(CTime::eCurrent);

    CDate date(now);
    doc->SetCreateDate(date);
    doc->SetModifiedDate(date);

    ws.SetWorkspace().AddProject(*doc);
    ws.SetDirty(true);

    srv->x_OnProjectChanged(doc->GetId());
    srv->x_OnWorkspaceChanged(ws.GetId());

    return doc;
}


void CProjectServiceTestJob::x_CreateItems()
{
    static int item_counter = 0;

    auto_ptr<ILocked> lock = m_ProjectService->GetWorkspaceLock();
    lock->GetLock();

    LOG_POST(Info << m_Descr << " get Write lock for Workspace and Projects");

    CRef<CGBWorkspace> ws = m_ProjectService->GetGBWorkspace();
    if (!ws) {
        LOG_POST(Info << m_Descr << " failed - no workspace");
        return;
    }

    CWorkspaceFolder& root_folder = ws->SetWorkspace();

    vector<int> projectIds = s_GetProjectIds(root_folder);

    if (projectIds.empty()) {
        LOG_POST(Info << "Creating a new project ...");
        s_CreateNewProject(m_ProjectService, *ws);
        s_GetProjectIds(root_folder);
        LOG_POST(Info << "Creating a new project ... Done");
    }

    for (size_t i = 0; i < projectIds.size(); i++) {
        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromId(projectIds[i]));
        if (!doc) continue;

        CProjectFolder& data_folder = doc->SetData();

        static int kItemsN = 2;
        LOG_POST(Info << "Creating " << kItemsN << " items in project  " << doc->GetDescr().GetTitle());

        for( int j = 0;  j < kItemsN;  j++ )  {
            CRef<objects::CSeq_id> id(new objects::CSeq_id);
            id->Set("19568015");
            string label = "Item # " + NStr::IntToString(++item_counter);

            CRef<CProjectItem> item(new CProjectItem());
            item->SetLabel(doc->MakeUniqueItemLabel(label));
            item->SetObject(*id);

		    doc->AddItem(*item, data_folder);
            doc->AttachProjectItem(item->GetId());
        }
        doc->SetDirty( true );
        doc->x_OnProjectChanged(CProjectViewEvent::eNone);
        m_ProjectService->x_OnProjectChanged(doc->GetId());

        LOG_POST(Info << "Items created");
    }
    LOG_POST(Info << m_Descr << "Release locks");
}


void CProjectServiceTestJob::x_ModifyItems()
{
    static int pass = 1;

    auto_ptr<ILocked> lock = m_ProjectService->GetWorkspaceLock();
    lock->GetLock();

    LOG_POST(Info << m_Descr << " get READ lock for Workspace and Write for Projects");

    CRef<CGBWorkspace> ws = m_ProjectService->GetGBWorkspace();
    if (!ws) {
        LOG_POST(Info << m_Descr << " failed - no workspace");
        return;
    }

    CWorkspaceFolder& root_folder = ws->SetWorkspace();

    vector<int> projectIds = s_GetProjectIds(root_folder);

    for (size_t i = 0; i < projectIds.size(); i++) {
        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromId(projectIds[i]));
        if (!doc) continue;
        LOG_POST(Info << "Renaming items " << doc->GetDescr().GetTitle());

        CProjectFolder& data_folder = doc->SetData();
        if(data_folder.CanGetItems()) {
            NON_CONST_ITERATE(CProjectFolder::TItems, it, data_folder.SetItems()) {
                string ver = ": v " + NStr::IntToString(pass);

                string name;
                if ((*it)->IsSetLabel())
                    name = (*it)->GetLabel();

                string::size_type pos = name.find_first_of(':');
                if(pos != string::npos) {
                    name.resize(pos);
                }
                name += ver;
                (*it)->SetLabel(doc->MakeUniqueItemLabel(name));
            }
        }

        doc->SetDirty( true );
        doc->x_OnProjectChanged(CProjectViewEvent::eNone);
        m_ProjectService->x_OnProjectChanged(doc->GetId());
    }

    pass++;

    LOG_POST(Info << m_Descr << "Release locks");
}


void CProjectServiceTestJob::x_DeleteItems()
{
    auto_ptr<ILocked> lock = m_ProjectService->GetWorkspaceLock();
    lock->GetLock();

    LOG_POST(Info << m_Descr << " get READ lock for Workspace and Write for Projects");

    CRef<CGBWorkspace> ws = m_ProjectService->GetGBWorkspace();
    if (!ws) {
        LOG_POST(Info << m_Descr << " failed - no workspace");
        return;
    }

    CWorkspaceFolder& root_folder = ws->SetWorkspace();

    vector<int> projectIds = s_GetProjectIds(root_folder);

    for (size_t i = 0; i < projectIds.size(); i++) {
        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromId(projectIds[i]));
        if (!doc) continue;
        LOG_POST(Info << "Deleting items from project " << doc->GetDescr().GetTitle());

        CProjectFolder& data_folder = doc->SetData();
        if(data_folder.CanGetItems()) {
            vector<int> projectItems;
            ITERATE(CProjectFolder::TItems, it, data_folder.GetItems())
                projectItems.push_back((*it)->GetId());

            ITERATE(vector<int>, it, projectItems) {
                CProjectItem* item = data_folder.FindProjectItemById(*it);
                if (item) {
                    LOG_POST(Info << "Deleting item  " << item->GetLabel());
                    doc->DetachProjectItem(*it);
                    doc->RemoveProjectItem(*it);
                }
            }
        }
        doc->SetDirty( true );
        doc->x_OnProjectChanged(CProjectViewEvent::eNone);
        m_ProjectService->x_OnProjectChanged(doc->GetId());
    }

    LOG_POST(Info << m_Descr << "Release locks");
}


void CProjectServiceTestJob::x_PrintItems()
{
    auto_ptr<ILocked> lock = m_ProjectService->GetWorkspaceLock();
    lock->GetLock();

    LOG_POST(Info << m_Descr << " get Read Lock for Workspace and Projects");

    CRef<CGBWorkspace> ws = m_ProjectService->GetGBWorkspace();
    if (!ws) {
        LOG_POST(Info << m_Descr << " failed - no workspace");
        return;
    }

    CWorkspaceFolder& root_folder = ws->SetWorkspace();

    vector<int> projectIds = s_GetProjectIds(root_folder);

    for (size_t i = 0; i < projectIds.size(); i++) {
        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromId(projectIds[i]));
        if (!doc) continue;
        LOG_POST(Info << m_Descr << " printing items in project " << doc->GetDescr().GetTitle());

        CProjectFolder& data_folder = doc->SetData();
        if(data_folder.CanGetItems()) {
            ITERATE(CProjectFolder::TItems, it, data_folder.GetItems()) {
                LOG_POST(Info << "\t- " << (*it)->GetLabel());
            }
        }
    }

    LOG_POST(Info << m_Descr << "Release locks");
}


END_NCBI_SCOPE

