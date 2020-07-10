#ifndef GUI_PACKAGES_PKG_SEQUENCE_EDIT___EDIT_APP_JOB__HPP
#define GUI_PACKAGES_PKG_SEQUENCE_EDIT___EDIT_APP_JOB__HPP

/*  $Id: edit_job.hpp 30185 2014-04-11 19:13:34Z katargir $
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
 * Authors: Colleen Bollin, based on a file by Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbimtx.hpp>

#include <gui/core/project_service.hpp>

#include <gui/framework/app_job_task.hpp>

#include <gui/utils/app_job_impl.hpp>

#include <objmgr/scope.hpp>


BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CProjectFolder;
    class CScope;
END_SCOPE(objects)


///////////////////////////////////////////////////////////////////////////////
/// CEditAppJob - a base class for Jobs modifying existing data in projects.

class CEditAppJob : public CAppJob
{
public:
    typedef CRef<objects::CProjectItem>      TItemRef;

public:
    /// constructor for derived classes
    CEditAppJob(CProjectService* service);

    virtual ~CEditAppJob();

    CProjectService*    GetProjectService();

    /// @name IAppJob implementation
    /// @{
    virtual EJobState   Run();
    virtual CRef<CObject>               GetResult();
    /// @}

protected:
    
    /// prepares job for execution by clearing state variables
    virtual void    x_ResetState();

    virtual void    x_Run();

    /// override this function in derived classes, populate m_TextResult
    virtual void    x_MakeChanges()  = 0;

protected:
    CIRef<CProjectService>  m_Service;

    string  m_TextResult;

};


///////////////////////////////////////////////////////////////////////////////
/// CDataLoadingAppTask - a task that executes CDataLoadingAppJob.
class CEditAppTask : public CAppJobTask
{
public:
    CEditAppTask(CEditAppJob& job, const string& descr = kEmptyStr);

    /// @name IAppTask interface implementation
    /// @{

    /// we override this function because our logic is a bit more complex than
    /// one implemented in CAppJobTask::Run() - under some circumstances
    /// we may need to show a dialog and then relaunch the job.
    virtual ETaskState    Run();
    /// @}
};


END_NCBI_SCOPE


#endif
// GUI_PACKAGES_PKG_SEQUENCE_EDIT___EDIT_APP_JOB__HPP
