/*  $Id: edit_job.cpp 30185 2014-04-11 19:13:34Z katargir $
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
 * Authors:  Colleen Bollin, based on a file by Andrey Yazhuk
 *
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_sequence_edit/edit_job.hpp>

#include <gui/core/project_selector_dlg.hpp>

#include <objects/gbproj/ProjectItem.hpp>
#include <objects/gbproj/ProjectDescr.hpp>
#include <objects/gbproj/ProjectFolder.hpp>
#include <objects/gbproj/FolderInfo.hpp>

#include <objects/general/Date.hpp>

#include <corelib/ncbi_system.hpp>

#include <wx/dirdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
/// CDataLoadingAppJob
CEditAppJob::CEditAppJob(CProjectService* service)
:   m_Service(service)
{
}


CEditAppJob::~CEditAppJob()
{
}


CProjectService* CEditAppJob::GetProjectService()
{
    return m_Service.GetPointer();
}


IAppJob::EJobState CEditAppJob::Run()
{
    static const char* kLogMsg = "CEditAppJob::Run() - exception in x_Run() ";

    //LOG_POST("CEditAppJob::Run()  Started  " << m_Descr);

    x_ResetState(); // reset state before execution

    string err_msg;
    try {
        x_Run();
    }
    catch (CException& e) {
        err_msg = kLogMsg + GetDescr() + ". " + e.GetMsg();
        LOG_POST(Error << err_msg);
        LOG_POST(Error << e.ReportAll());
    } catch (std::exception& ee) {
        err_msg = kLogMsg + GetDescr() + ". " + ee.what();
        LOG_POST(Error << err_msg);
    } 

    CFastMutexGuard lock(m_Mutex);

    if(err_msg.empty())  {
        LOG_POST(Info << "CEditAppJob::Run()  Finished  " << m_Descr);
        return eCompleted;
    } else {
        m_Error.Reset(new CAppJobError(err_msg));
        return eFailed;
    }
}


void CEditAppJob::x_ResetState()
{
    CFastMutexGuard lock(m_Mutex);

    m_TextResult = kEmptyStr;
    m_Error.Reset();

    m_Status = "Starting...";
}


void CEditAppJob::x_Run()
{

    x_MakeChanges ();

}


CRef<CObject> CEditAppJob::GetResult()
{
    CRef<CObject>   res;
    CFastMutexGuard lock(m_Mutex);
    if( ! m_TextResult.empty()) {
        res.Reset(new CAppJobTextResult(m_TextResult));
    }
    return res;
}


///////////////////////////////////////////////////////////////////////////////
/// CEditAppTask

CEditAppTask::CEditAppTask(CEditAppJob& job, const string& descr)
:   CAppJobTask(job, true, descr)
{
    if(descr == kEmptyStr)  {
        m_Descr = job.GetDescr();
    }
}


IAppTask::ETaskState CEditAppTask::Run()
{
    ETaskState state = CAppJobTask::Run();

    if(state == eCompleted) {
        IAppJob* job = GetJob().GetPointer();
        CEditAppJob* data_job = dynamic_cast<CEditAppJob*>(job);
        _ASSERT(data_job);
    }
    return state;
}


END_NCBI_SCOPE
