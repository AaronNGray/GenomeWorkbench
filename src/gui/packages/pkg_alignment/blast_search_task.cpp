/*  $Id: blast_search_task.cpp 40209 2018-01-08 20:35:14Z joukovv $
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

#include <gui/packages/pkg_alignment/blast_search_task.hpp>
#include <gui/packages/pkg_alignment/blast_search_params.hpp>
#include <gui/framework/app_job_task.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/msgdlg.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CBlastSearchTask

CBlastSearchTask::CBlastSearchTask(IServiceLocator* srv_locator,
                                   CNetBLASTUIDataSource& ds,
                                   const string& tool_name)
:   m_SrvLocator(srv_locator),
    m_NetBlastDS(&ds),
    m_ToolName(tool_name),
    m_Command(eInvalid),
    m_Stage(eInvalid)
{
}


CBlastSearchTask::~CBlastSearchTask()
{
}


void CBlastSearchTask::Init_SubmitSearch(const CBLASTParams& params)
{
    size_t n = params.GetSeqLocs().size();
    m_Descr = m_ToolName + " - ";

    string title = params.GetJobTitle();
    if(title.empty())   {
        m_Descr += NStr::NumericToString(n);
        m_Descr += (n ==1) ? " sequence" : " sequences";
    } else {
        m_Descr += title;
    }

    m_Command = eSubmit;

    m_SubmittingJob = new CNetBlastSubmittingJob(m_ToolName, params, *m_NetBlastDS);

    // create the first task in this sequence
    CAppJobTask* jtask = new CAppJobTask(*m_SubmittingJob, true);
    jtask->SetReportErrors( false );
    m_CurrTask.Reset( jtask );

        m_Stage = eSubmit;
}


void CBlastSearchTask::Init_Monitoring(TDescriptors& descriptors)
{
    size_t n =descriptors.size();
    m_Descr = m_ToolName + " - " + NStr::NumericToString(n);
    m_Descr += (n ==1) ? " sequence" : " sequences";

    m_Command = eMonitor;

    x_InitMonitoringStage(descriptors);
}


void CBlastSearchTask::Init_RetrieveRID(const vector<string>& RIDs)
{
    m_Descr = m_ToolName + " - retrieving ";
    size_t n = RIDs.size();
    m_Descr += NStr::NumericToString(n);
    m_Descr += (n == 1) ? " RID" : " RIDs";

    m_Command = eRetrieve;
    m_Stage = eMonitor;

    // create a loading job and a wrapping child task
    CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
    // Get project scope here and pass it to loading job to check local sequences
    // for duplicates
    CScope *scope = 0;
    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (ws) {
        auto project_id = m_LoadingOptions.GetTargetProjectId();
        CGBProjectHandle* project = ws->GetProjectFromId(project_id);
        if (project) {
            scope = project->GetScope();
        }
    }
    m_LoadingJob.Reset(new CNetBlastLoadingJob(*m_NetBlastDS, &RIDs, scope));
    //m_Descr = task->GetDescr();

    //CSelectProjectOptions options;
    m_CurrTask.Reset(new CDataLoadingAppTask(srv, m_LoadingOptions, *m_LoadingJob));
    m_Stage = eRetrieve;
}


/// preapres App Job for monitoring stage of the task
void CBlastSearchTask::x_InitMonitoringStage(TDescriptors& descriptors)
{
    m_Descr += " (";

    size_t i;
    for (i = 0; i < descriptors.size(); ++i) {
        if (i > 0)
            m_Descr += ",";
        m_Descr += descriptors[i]->GetRID();
        if (i == 2)
            break;
    }

    if (i < descriptors.size())
        m_Descr += ", ...";

    m_Descr += ")";

    m_MonitoringJob =
        new CNetBlastMonitoringJob(*m_NetBlastDS, m_SrvLocator, m_ToolName, descriptors);
    m_MonitoringJob->SetLoadingOptions(m_LoadingOptions);

    m_CurrTask.Reset(new CAppJobTask(*m_MonitoringJob, true, "", 5, "Scheduler"));
    m_Stage = eMonitor;
}

void CBlastSearchTask::SetLoadingOptions(CSelectProjectOptions& options)
{
    m_LoadingOptions = options;

    CDataLoadingAppTask* task = dynamic_cast<CDataLoadingAppTask*>(m_CurrTask.GetPointerOrNull());
    if (task)
        task->SetOptions(options);

    if(m_MonitoringJob) {
        m_MonitoringJob->SetLoadingOptions(m_LoadingOptions);
    }
}

/// overriding CAppTask::GetStatusText()
string CBlastSearchTask::GetStatusText() const
{
    switch(m_Stage) {
    case eSubmit:
        return "Submitting results to NCBI Net BLAST server";

    case eMonitor:
        return "Executing BLAST jobs remotely on NCBI Net BLAST server";

    case eRetrieve:
        return "Retrieving results from NCBI Net BLAST server";

    case eFinished:
        return "Finished";

    default:
        _ASSERT(false); // unexpected
        return "Bug";
    }
}


CIRef<IAppTask> CBlastSearchTask::x_GetCurrentSubTask()
{
    return m_CurrTask;
}


CIRef<IAppTask> CBlastSearchTask::x_NextSubTask()
{
    //LOG_POST("CBlastSearchTask::x_NextSubTask() " << m_Stage);
    switch(m_Stage) {
    case eSubmit:   {{
        // next stage - Monitoring
        if(m_SubmittingJob) {
            TDescriptors descriptors;
            m_SubmittingJob->GetDescriptors(descriptors);

            if( ! descriptors.empty()) {
                x_InitMonitoringStage(descriptors);
            } else {
                m_CurrTask.Reset();
                m_Stage = eFinished;
            }
            break;
        }
    }}
    case eMonitor:
        m_CurrTask.Reset();
        if(m_MonitoringJob) {
            m_Stage = eFinished;
        }
        break;

    case eRetrieve:
        m_CurrTask.Reset();
        m_Stage = eFinished;
        break;

    default:
        _ASSERT(false); // invalid
        break;
    }

    return m_CurrTask;
}


IAppTask::ETaskState CBlastSearchTask::x_RunCurrentSubTask()
{
    ETaskState state = CCompositeAppTask::x_RunCurrentSubTask();

    if(state == eCompleted)  {
        switch(m_Stage) {
        case eSubmit:   {{
            vector<string> errors;
            m_SubmittingJob->GetErrors(errors);
            if( ! errors.empty())   {
                x_ReportErrors(errors);
            }
            break;
        }}
        case eRetrieve:   {{
            vector<string> errors;
            m_LoadingJob->GetErrors(errors);
            if( ! errors.empty())   {
                x_ReportErrors(errors);
            }
            break;
        }}
        default:
            break;
        }
    }
    return state;
}


void CBlastSearchTask::x_ReportErrors(vector<string>& errors)
{
    // report errors interactively
    // TODO this probably should be done via Event Logging service
    // format the message
    size_t n = errors.size();
    string msg = NStr::NumericToString(n) + ((n == 1) ? " RID" : " RIDs");
    msg += " cannot be retrieved for the following reasons:";
    for( size_t i = 0;  i < n; i++) {
        msg += "\n  " + errors[i];
    }

    string title = (n == 1) ? "Error in" : "Errors in";
    title += m_ToolName;

    wxMessageBox(ToWxString(msg), ToWxString(title));
}


END_NCBI_SCOPE
