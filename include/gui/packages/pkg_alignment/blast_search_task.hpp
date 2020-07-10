#ifndef PKG_ALIGNMENT___BLAST_SEARCH_TASK__HPP
#define PKG_ALIGNMENT___BLAST_SEARCH_TASK__HPP

/*  $Id: blast_search_task.hpp 30782 2014-07-23 19:31:13Z katargir $
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

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/framework/app_task_impl.hpp>

#include <gui/packages/pkg_alignment/blast_jobs.hpp>
#include <gui/packages/pkg_alignment/net_blast_ui_data_source.hpp>


BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CProjectService;

///////////////////////////////////////////////////////////////////////////////
/// TODO
class CBlastSearchTask : public CCompositeAppTask
{
public:
    typedef vector< CRef<CNetBlastJobDescriptor> >   TDescriptors;

    CBlastSearchTask(IServiceLocator* srv_locator,
                     CNetBLASTUIDataSource& ds,
                     const string& tool_name);
    virtual ~CBlastSearchTask();

    /// setup the task for submitting and monitoring BLAST jobs, and for
    /// retrieving results
    virtual void    Init_SubmitSearch(const CBLASTParams& params);

    /// setup the task for monitoring BLAST jobs and for retrieving results
    virtual void    Init_Monitoring(TDescriptors& descriptors);

    /// setup the task for retrieving results
    virtual void    Init_RetrieveRID(const vector<string>& RIDs);

    /// define how the task results shall be handled
    virtual void    SetLoadingOptions(CSelectProjectOptions& options);

    /// overriding CAppTask::GetStatusText()
    virtual string  GetStatusText() const;

protected:
    enum ECommand   {
        eInvalid = -1,
        eSubmit,   /// submit request, monitor and retrieve results
        eMonitor,  /// monitor and retrieve resutlts
        eRetrieve, /// only retrieve resutlts
        eFinished
    };

protected:
    void    x_InitMonitoringStage(TDescriptors& descriptors);

    /// @name CCompositeAppTask overridables
    /// @{
    virtual CIRef<IAppTask> x_GetCurrentSubTask();
    virtual CIRef<IAppTask> x_NextSubTask();
    virtual ETaskState      x_RunCurrentSubTask();
    /// @}

    void    x_ReportErrors(vector<string>& errors);

protected:
    IServiceLocator*  m_SrvLocator;
    CRef<CNetBLASTUIDataSource> m_NetBlastDS;

    /// name of the tool that launched the task; used in UI
    string m_ToolName;

    /// the command associated with the task
    ECommand    m_Command;

    CSelectProjectOptions m_LoadingOptions;
    string  m_ProjectFolderName;
    CRef<objects::CGBProjectHandle>  m_TargetProject; // the project to add to

    /// defines the current stage of the task
    ECommand    m_Stage;
    CRef<CNetBlastSubmittingJob>    m_SubmittingJob;
    CRef<CNetBlastMonitoringJob>    m_MonitoringJob;
    CRef<CNetBlastLoadingJob>       m_LoadingJob;
    CIRef<IAppTask>     m_CurrTask;
};


/* @} */

END_NCBI_SCOPE;


#endif  // PKG_ALIGNMENT___BLAST_SEARCH_TASK__HPP

