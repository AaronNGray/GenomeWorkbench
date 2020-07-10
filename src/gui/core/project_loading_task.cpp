/*  $Id: project_loading_task.cpp 40276 2018-01-19 17:40:36Z katargir $
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

#include <gui/core/project_loading_task.hpp>
#include <gui/core/prefetch_seq_descr.hpp>

#include <gui/core/project_service.hpp>
#include <gui/core/document.hpp>
#include <gui/core/loading_app_job.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/framework/app_job_task.hpp>

#include <gui/objects/GBWorkspace.hpp>
#include <objects/gbproj/ProjectDescr.hpp>
#include <gui/objects/WorkspaceFolder.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <corelib/ncbifile.hpp>
#include <serial/iterator.hpp>

#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

const wxChar* kPRJ_Wildcard = wxT("GBENCH Project (*.gbp)|*.gbp"); //TODO reuse

///////////////////////////////////////////////////////////////////////////////
/// CProjectLoadingTask

static const wxChar* kAlreadyExist =
wxT("\" has been opened. Please choose a different filename.");

SWFileDlgData
    CProjectLoadingTask::sm_ProjectDlgData(wxT("Save Project As"),
                                          kPRJ_Wildcard,
                                          wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

BEGIN_EVENT_MAP(CProjectLoadingTask, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged,
             &CProjectLoadingTask::OnAppJobNotification)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eProgress,
             &CProjectLoadingTask::OnAppJobProgress)
END_EVENT_MAP()

CProjectLoadingTask::CProjectLoadingTask(int projId,
                                         CProjectService* srv,
                                         bool add_to_mru)
:   CAppTask("Loading project..."),
    m_TaskName("Load Project"),
    m_ProjId(projId),
    m_ProjectService(srv),
    m_AddToMRU(add_to_mru),
    m_JobID(CAppJobDispatcher::eInvalidJobID),
    m_JobState(IAppJob::eInvalid)
{
}

CProjectLoadingTask::~CProjectLoadingTask()
{
    if (m_JobID != CAppJobDispatcher::eInvalidJobID) {
        CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
        disp.DeleteJob(m_JobID);
    }
}

void CProjectLoadingTask::CancelBackgrounded()
{
    if(m_State == IAppTask::eBackgrounded)   {
        _ASSERT(m_JobID != CAppJobDispatcher::eInvalidJobID);
        CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
        disp.DeleteJob(m_JobID);
    }

    CRef<CGBWorkspace> ws = m_ProjectService->GetGBWorkspace();
    if (!ws) return;

    CRef<CGBDocument> doc(dynamic_cast<CGBDocument*>(ws->GetProjectFromId(m_ProjId)));
    if (!doc) return;
    m_ProjectService->RemoveProject(*doc);

    if (m_Listener)  {
        CRef<CEvent> evt(new CAppTaskServiceEvent(*this, CAppTaskServiceEvent::eTaskCanceled));
        m_Listener->Post(evt);
    }
}

IAppTask::ETaskState CProjectLoadingTask::Run()
{
    switch (m_State) {
    case eInitial:
        _ASSERT(m_JobID == CAppJobDispatcher::eInvalidJobID);
        _ASSERT(m_JobState == IAppJob::eInvalid);

        m_State = x_StartJob();
        m_JobState = IAppJob::eRunning;
        break;

    case eBackgrounded:
        // Job has been launched already - check whether its status has changed
        m_State = x_OnWakeUp();
        break;

    default:
        _ASSERT(false); // unexpected
        m_State = eInvalid;
        break;
    }
    return m_State;
}

IAppTask::ETaskState CProjectLoadingTask::x_StartJob()
{
    if (m_JobID != CAppJobDispatcher::eInvalidJobID)
        return eFailed;

    CRef<CGBWorkspace> ws = m_ProjectService->GetGBWorkspace();
    if (!ws) return eFailed;

    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromId(m_ProjId));
    if (!doc) return eFailed;

    wxString abs_path = doc->GetFileName();
    CIRef<IAppJob> job(doc->CreateLoadJob(abs_path));
    if (!job) {
        return eFailed;
    }

    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    m_JobID = disp.StartJob(*job, "ThreadPool", *this, 5, false);
    if (m_JobID == CAppJobDispatcher::eInvalidJobID)
        return eFailed;

    wxString fileName;
    if (abs_path.StartsWith(wxT("http://")) || abs_path.StartsWith(wxT("https://")) || abs_path.StartsWith(wxT("ftp://"))) {
        fileName = abs_path;
    }
    else {
        fileName = wxFileName(abs_path).GetFullName();
    }

    m_Descr = "Loading project: " + fileName;

    return eBackgrounded;
}

IAppTask::ETaskState CProjectLoadingTask::x_OnWakeUp()
{
    if (m_JobState == IAppJob::eRunning || m_JobState == IAppJob::eSuspended)
        return eBackgrounded;

    m_JobID = CAppJobDispatcher::eInvalidJobID;

    if (m_JobState == IAppJob::eCanceled)
        return eCanceled;

    CRef<CGBWorkspace> ws = m_ProjectService->GetGBWorkspace();
    if (!ws) return eFailed;

    CRef<CGBDocument> doc(dynamic_cast<CGBDocument*>(ws->GetProjectFromId(m_ProjId)));
    if (!doc) return eFailed;

    doc->ResetLoading();

    if (m_JobState == IAppJob::eCompleted) {
        doc->AttachData();

        wxString mru_path;

        if(m_AddToMRU)  {
            //TODO synchronization needed here ?
            mru_path = doc->GetFileName();
        }
        if (!mru_path.empty()) {
            m_ProjectService->AddToProjectWorkspaceMRUList(mru_path);
        }

        CSelectProjectOptions::TItems items;
        for (CTypeIterator<CProjectItem> it(doc->SetData()); it; ++it) {
            CRef<CProjectItem> item(&*it);
            items.push_back(item);
        }
        CPrefetchSeqDescr::PrefetchSeqDescr(m_ProjectService->GetServiceLocator(), items);

        return eCompleted;
    }
    else {
        // loading has not been successful - ask user what to do
        wxString prj_filename = doc->GetFileName();

        wxString msg = wxT("Cannot load project \"") + prj_filename;
        msg += wxT("\". Would you like to specify a different filename?");

        TDialogType type = eDialog_OkCancel | eDialog_Modal;
        EDialogReturnValue res = NcbiMessageBoxW(msg, type, eIcon_Exclamation,
                                                ToWxString(m_TaskName), eWrap);
        switch(res) {
        case eOK:
            break;
        default:
            m_ProjectService->RemoveProject(*doc);
            return eFailed;
        }

        // keep asking until a correct file name is specified or operation is canceled
        while(true) {
            sm_ProjectDlgData.m_Style = wxFD_OPEN | wxFD_FILE_MUST_EXIST;
            sm_ProjectDlgData.m_Title =
                    ToWxString(m_TaskName) + wxT(" - choose a new filename for project");

            if(NcbiFileBrowser(sm_ProjectDlgData, NULL) == wxID_OK)  {
                // users specified a new filename
                wxString abs_path = sm_ProjectDlgData.GetFilename();

                // look is we already have a project with this file name
                if (dynamic_cast<CGBDocument*>(ws->SetWorkspace().FindProjectByFilename(FnToStdString(abs_path)))) {
                    // a project with the selected file name already exists
                    msg = wxT("Project \"") + abs_path;
                    msg += kAlreadyExist;

                    TDialogType type = eDialog_Ok | eDialog_Modal;
                    NcbiMessageBoxW(msg, type, eIcon_Exclamation, ToWxString(m_TaskName), eWrap);
                    // remain in the loop to ask again
                } else {
                    // the filename is good - can use it to load the project
                    doc->SetFileName(abs_path);
                    break; // exit the loop to load the project
                }
            } else {
                m_ProjectService->RemoveProject(*doc);
                return eFailed;
            }
        } // while
    }

    return x_StartJob();
}

void CProjectLoadingTask::OnAppJobNotification( CEvent* evt )
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);

    if( notn ){
        if( m_JobID !=  notn->GetJobID() ){
            _ASSERT(false);
            ERR_POST(
                "CProjectLoadingTask::OnAppJobNotification() - unknown or invalid Job ID " 
                << notn->GetJobID()
            );
            return;
        }
        m_JobState = notn->GetState();

        if (m_JobState == IAppJob::eCompleted ||
            m_JobState == IAppJob::eFailed ||
            m_JobState == IAppJob::eCanceled) {

            _ASSERT(m_Listener);
            if(m_Listener)  {
                CAppTaskServiceEvent::EType type = CAppTaskServiceEvent::eWakeUpSignal;
                if(m_JobState == IAppJob::eCanceled) {
                    type = CAppTaskServiceEvent::eTaskCanceled;
                }

                CRef<CEvent> evt(new CAppTaskServiceEvent(*this, type));
                m_Listener->Post(evt);
            }
        }
    }
}

void CProjectLoadingTask::OnAppJobProgress(CEvent* evt)
{
    CAppJobNotification* notn =
        dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);

    if(notn)    {
        int job_id = notn->GetJobID();

        if(m_JobID != job_id) {
            _ASSERT(false);
            ERR_POST("CAppJobTask::OnAppJobNotification() - unknown or invalid Job ID " << job_id);
        } else {
            CConstIRef<IAppJobProgress> prg = notn->GetProgress();
        }
    }
}

END_NCBI_SCOPE

