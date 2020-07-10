/*  $Id: loading_app_job.cpp 33751 2015-09-14 15:47:01Z katargir $
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
 * Authors:  Andrey Yazhuk, Liangshou Wu
 *
 */

#include <ncbi_pch.hpp>

#include <gui/core/loading_app_job.hpp>
#include <gui/core/prefetch_seq_descr.hpp>

#include <gui/core/project_selector_dlg.hpp>
#include <gui/widgets/loaders/job_results_dlg.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <corelib/ncbi_system.hpp>

#include <wx/dirdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CDataLoadingAppJob
CDataLoadingAppJob::CDataLoadingAppJob() : m_TextResult()
{
}


CDataLoadingAppJob::~CDataLoadingAppJob()
{
}

void CDataLoadingAppJob::x_SetTextResult( const string& result ){ 
    
    CFastMutexGuard lock(m_Mutex);
    m_TextResult = result; 
}

IAppJob::EJobState CDataLoadingAppJob::Run()
{
    static const char* kLogMsg = "CDataLoadingAppJob::x_Run() - exception ";

    x_ResetState();

    string err_msg;
    bool err_user_level = false;
    try {
        x_Run();

    } catch( CException& e ){
        LOG_POST(Error << kLogMsg << GetDescr() << ". " << e);
        e.ReportAll();

        err_msg = e.GetMsg();
        err_user_level = true;

    } catch( std::exception& ee ){
        err_msg = kLogMsg + GetDescr() + ". " + ee.what();
        LOG_POST(Error << err_msg );

        err_user_level = false;
    } 

    CFastMutexGuard lock(m_Mutex);

    if( !err_msg.empty() ){
        m_Error.Reset( new CAppJobError( err_msg, err_user_level ) );
    }

    if( m_Error ){
        return eFailed;
    }

    LOG_POST(Info 
        << "CDataLoadingAppJob::Run(): Finished job " << m_Descr
    );
    return eCompleted;
}


void CDataLoadingAppJob::x_ResetState()
{
    CAppJob::x_ResetState();

    x_SetTextResult( m_Descr + ": no items created." );
}


void CDataLoadingAppJob::x_Run()
{
    // if we do not have project items - we need to create them
    // otherwise - skip this step and add existing items to a project
    if( m_Data.empty() ){
        CLockerGuard guard = x_GetGuard();
        if (IsCanceled()) return;
        x_SetStatusText( "Creating data items..." );
        x_CreateProjectItems(); /// populate m_Items
        if (IsCanceled()) {
            return;
        }
    }

    if (!m_Data.empty()) {
        if (IsCanceled()) return;
        x_SetTextResult("");
    }
}

void CDataLoadingAppJob::ClearItems()
{
    m_Data.clear();
}

void CDataLoadingAppJob::AddProjectItem(CProjectItem& item)
{
    CRef<CLoaderDescriptor> loader;
    m_Data[loader].push_back(CRef<CProjectItem>(&item));
}

void CDataLoadingAppJob::AddProjectItem(CProjectItem& item, CLoaderDescriptor& loader)
{
    m_Data[CRef<CLoaderDescriptor>(&loader)].push_back(CRef<CProjectItem>(&item));
}

void CDataLoadingAppJob::AddDataLoader(objects::CLoaderDescriptor& loader)
{
    m_Data[CRef<CLoaderDescriptor>(&loader)];
}

CRef<CObject> CDataLoadingAppJob::GetResult()
{
    CFastMutexGuard lock(m_Mutex);

    if( !m_TextResult.empty() ){
        return CRef<CObject>( new CAppJobTextResult( m_TextResult ) );
    }

    return CRef<CObject>();
}

///////////////////////////////////////////////////////////////////////////////
/// CDataLoadingAppTask

CDataLoadingAppTask::CDataLoadingAppTask(CProjectService* service, CSelectProjectOptions& options, CDataLoadingAppJob& job, const string& descr)
    : CAppJobTask( job, true, descr.empty() ? job.GetDescr() : descr ), m_Service(service), m_Options(options)
{
}


IAppTask::ETaskState CDataLoadingAppTask::Run()
{
    ETaskState state = CAppJobTask::Run();

    if (state == eCompleted) {
        IAppJob* job = GetJob().GetPointer();
        CDataLoadingAppJob* data_job = dynamic_cast<CDataLoadingAppJob*>(job);
        if (data_job->IsCanceled()) {
            return state;
        }

        _ASSERT(data_job);

        const CSelectProjectOptions::TData& data = data_job->GetData();

        if (!data.empty()) {
            if (!m_Options.AddItemsToWorkspace(m_Service, data))
                return eCanceled;

            vector<CRef<CProjectItem> >  items;
            ITERATE(CSelectProjectOptions::TData, it, data) {
                ITERATE(CSelectProjectOptions::TItems, it2, it->second)
                    items.push_back(*it2);
            }

            CPrefetchSeqDescr::PrefetchSeqDescr(m_Service->GetServiceLocator(), items);
        }

        wxString resTitle, resHTML;
        data_job->GetHTMLResults(resTitle, resHTML);

        if (!resHTML.empty()) {
            CJobResultsDlg dlg;
            dlg.SetHTML(resHTML);
            dlg.Create(NULL, wxID_ANY, resTitle);
            dlg.ShowModal();
        }
    }

    return state;
}

END_NCBI_SCOPE
