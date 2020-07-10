#ifndef GUI_CORE_DEMO___LOADING_APP_JOB__HPP
#define GUI_CORE_DEMO___LOADING_APP_JOB__HPP

/*  $Id: loading_app_job.hpp 33751 2015-09-14 15:47:01Z katargir $
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
 * Authors: Andrey Yazhuk, Liangshou Wu
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbimtx.hpp>

#include <gui/core/project_service.hpp>
#include <gui/core/select_project_options.hpp>

#include <objects/gbproj/LoaderDescriptor.hpp>

#include <gui/framework/app_job_task.hpp>

#include <gui/utils/app_job_impl.hpp>

#include <objmgr/scope.hpp>

#include <wx/string.h>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CDataLoadingAppJob - a base class for Jobs loading data into projects.

class NCBI_GUICORE_EXPORT CDataLoadingAppJob : public CAppJob
{
public:
    /// constructor for derived classes
    CDataLoadingAppJob();
    virtual ~CDataLoadingAppJob();

    /// @name IAppJob implementation
    /// @{
    virtual EJobState Run();
    virtual CRef<CObject> GetResult();
    /// @}

    const CSelectProjectOptions::TData& GetData() const { return m_Data; }
    const objects::CScope& GetScope() const { _ASSERT(m_Scope); return *m_Scope; }

    virtual void GetHTMLResults( wxString& /*title*/, wxString& /*html*/ ) {}

    void AddProjectItem(objects::CProjectItem& item);
    void AddProjectItem(objects::CProjectItem& item, objects::CLoaderDescriptor& loader);
    void AddDataLoader(objects::CLoaderDescriptor& loader);
    void ClearItems();

protected:
    
    /// prepares job for execution by clearing state variables
    virtual void x_ResetState();

    virtual void x_Run();

    /// override this function in derived classes and populate m_Items.
    /// If data loaders are needed, they should be created here too.
    virtual void x_CreateProjectItems() = 0;

    void x_SetTextResult( const string& result );

protected:
    string m_TextResult;

    CRef<objects::CScope>  m_Scope;

private:
    CSelectProjectOptions::TData m_Data;
};

///////////////////////////////////////////////////////////////////////////////
/// CDataLoadingAppTask - a task that executes CDataLoadingAppJob.
class NCBI_GUICORE_EXPORT  CDataLoadingAppTask : public CAppJobTask
{
public:
    CDataLoadingAppTask(CProjectService* service, CSelectProjectOptions& options, CDataLoadingAppJob& job, const string& descr = kEmptyStr);

    /// @name IAppTask interface implementation
    /// @{

    /// we override this function because our logic is a bit more complex than
    /// one implemented in CAppJobTask::Run() - under some circumstances
    /// we may need to show a dialog and then relaunch the job.
    virtual ETaskState Run();
    /// @}

    void SetOptions(const CSelectProjectOptions& options) { m_Options = options; }

protected:
    CIRef<CProjectService> m_Service;
    CSelectProjectOptions  m_Options;
};

END_NCBI_SCOPE


#endif
// GUI_CORE_DEMO___LOADING_APP_JOB__HPP
