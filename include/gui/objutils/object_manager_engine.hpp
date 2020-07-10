#ifndef GUI_OBJUTILS___APP_JOB_ENGINE_OM__HPP
#define GUI_OBJUTILS___APP_JOB_ENGINE_OM__HPP

/*  $Id: object_manager_engine.hpp 24876 2011-12-16 16:03:41Z kuznets $
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
 * Authors:  Andrey Yazhuk, Eugene Vasilchenko
 *
 * File Description:
 *   Implementation of Application Job Engine using object manager prefetcher.
 *
 */


#include <gui/utils/app_job_engine.hpp>

#include <corelib/ncbiobj.hpp>
#include <objmgr/prefetch_manager.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CAppJobDispatcher
class NCBI_GUIUTILS_EXPORT CObjectManagerEngine :
    public CObject,
    public objects::IPrefetchListener,
    public IAppJobEngine
{
public:
    CObjectManagerEngine(unsigned max_threads = 3);
    ~CObjectManagerEngine();

    virtual bool    IsActive();

    virtual void    SetListener(IAppJobEngineListener* listener);

    virtual void    StartJob(IAppJob& job, IEngineParams* params = NULL);

    virtual void    CancelJob(IAppJob& job);
    virtual void    SuspendJob(IAppJob& job);
    virtual void    ResumeJob(IAppJob& job);

    virtual TJobState   GetJobState(IAppJob& job) const;

    virtual void    ShutDown();
    virtual void    RequestCancel();

protected:
    void PrefetchNotify(CRef<objects::CPrefetchRequest> token, EEvent event);

    mutable CMutex m_Mutex;
    objects::CPrefetchManager m_PrefetchManager;
    IAppJobEngineListener* m_Listener;
    typedef map<IAppJob*, CRef<objects::CPrefetchRequest> > TJobHandles;
    TJobHandles m_Jobs;

private:
    CObjectManagerEngine(const CObjectManagerEngine&);
    void operator=(const CObjectManagerEngine&);
};


END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___APP_JOB_ENGINE_OM__HPP
