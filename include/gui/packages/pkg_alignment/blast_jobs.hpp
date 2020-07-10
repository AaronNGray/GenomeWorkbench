#ifndef PKG_ALIGNMENT___BLAST_JOBS__HPP
#define PKG_ALIGNMENT___BLAST_JOBS__HPP

/*  $Id: blast_jobs.hpp 40209 2018-01-08 20:35:14Z joukovv $
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
 * Authors:  Andrey Yazhuk, Anatoliy Kuznetsov
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/packages/pkg_alignment/net_blast_job_descr.hpp>
#include <gui/core/loading_app_job.hpp>
#include <gui/objutils/objects.hpp>

#include <gui/utils/scheduler_engine.hpp> //TODO


BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

BEGIN_SCOPE(objects)
    class CSeq_align;
    class CSeq_align_set;
END_SCOPE(objects)


class CProjectService;
class CBLASTParams;
class CNetBLASTUIDataSource;

///////////////////////////////////////////////////////////////////////////////
/// CNetBlastSubmittingJob
class CNetBlastSubmittingJob : public CJobCancelable
{
public:
    typedef vector< CRef<CNetBlastJobDescriptor> >   TDescriptors;

    CNetBlastSubmittingJob(string tool_name,
                           const CBLASTParams& params,
                           CNetBLASTUIDataSource& ds);

    /// @name IAppJob implementation
    /// @{
    virtual EJobState   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string    GetDescr() const;
    /// @}

    void    GetDescriptors(TDescriptors& descriptors);
    void    GetErrors(vector<string>& errors) const;

protected:
    void    x_Run();
    void    x_SetStatusText(const string& text);
    void    x_ResetState();
    void    x_AddError(const string& error);

protected:
    CFastMutex m_Mutex; /// mutex to sync our internals

    string  m_Descr;
    string  m_Status;

    TConstScopedObjects  m_SeqLocs; // query sequences

    vector< CRef<CNetBlastJobDescriptor> >   m_Descriptors;
    vector<string>  m_Errors;

    CRef<CAppJobError>  m_Error;
};


///////////////////////////////////////////////////////////////////////////////
/// CNetBlastMonitoringJob

class IServiceLocator;

class CNetBlastMonitoringJob : public CJobCancelable, public IWaitPeriod
{
public:
    typedef vector< CRef<CNetBlastJobDescriptor> >   TDescriptors;

    CNetBlastMonitoringJob(CNetBLASTUIDataSource& ds,
                           IServiceLocator* srv_locator,
                           const string& tool_name,
                           TDescriptors& descriptors);

    virtual void    SetLoadingOptions(CSelectProjectOptions& options);

    /// @name IAppJob implementation
    /// @{
    virtual EJobState   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string    GetDescr() const;
    /// @}

    /// @name IWaitPeriod implementation
    /// @{
    virtual CTimeSpan   GetWaitPeriod();
    /// @}

protected:
    EJobState    x_Run();
    void    x_SetStatusText(const string& text);
    void    x_ResetState();

    void    x_StartRetrivingTask(const vector<string>& RIDs);

protected:
    CFastMutex m_Mutex; /// mutex to sync our internals

    string  m_ToolName;
    IServiceLocator*    m_SrvLocator;
    CRef<CNetBLASTUIDataSource> m_NetBlastDS;

    string  m_Descr;
    string  m_Status;

    CSelectProjectOptions m_LoadingOptions;

    int m_WaitPeriodIndex;

    TDescriptors    m_Descriptors;
    vector<string>  m_Errors;

    CRef<CAppJobError>  m_Error;
};


///////////////////////////////////////////////////////////////////////////////
/// CNetBlastLoadingJob - an application job for loading RIDs from
/// NCBI Net BLAST server.
class  CNetBlastLoadingJob : public CDataLoadingAppJob
{
public:
    typedef vector< CRef<CNetBlastJobDescriptor> >   TDescriptors;

    CNetBlastLoadingJob(CNetBLASTUIDataSource& ds,
                        const vector<string>* RIDs = NULL,
                        CScope *scope = NULL);

    void    SetRIDs(CNetBLASTUIDataSource& ds, const vector<string>& RIDs);
    bool    HasErrors() const;
    void    GetErrors(vector<string>& errors) const;

protected:
    virtual void    x_CreateProjectItems();

    void    x_CreateProjectItemsFromBlastResult(objects::CSeq_align_set& results,
                                                CRef<blast::CRemoteBlast> RemoteBlast);
protected:
    TDescriptors    m_Descriptors;

    vector<string> m_Errors;
    CScope        *m_Scope;
};



/* @} */

END_NCBI_SCOPE;


#endif  // PKG_ALIGNMENT___BLAST_JOBS__HPP

