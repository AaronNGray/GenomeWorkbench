#ifndef GUI_WX_APP_DEMO___TEST_CLIENT__HPP
#define GUI_WX_APP_DEMO___TEST_CLIENT__HPP

/*  $Id: test_client.hpp 29661 2014-01-24 14:48:36Z falkrb $
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
 */


#include <gui/utils/app_job_impl.hpp>
#include <gui/utils/app_job_dispatcher.hpp>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// CPrimeNumbersJob

class CPrimeNumbersJob : public CObject, public IAppJob
{
public:
    class CResult   : public CObject
    {
    public:
        vector<int> m_Numbers;
    };

    CPrimeNumbersJob(int max);

    /// @name IAppJob implementation
    /// @{
    virtual IAppJob::EJobState    Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>   GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string    GetDescr() const;
    virtual void RequestCancel();
    virtual bool IsCanceled() const  { return m_StopRequested; }
    /// @}

protected:
    int     m_MaxNumber;
    bool    m_StopRequested;
    bool    m_Finished;

    CConstIRef<IAppJobProgress> m_Progress;
    CRef<CResult>               m_Result;
    CConstIRef<IAppJobError>    m_Error;
};


////////////////////////////////////////////////////////////////////////////////
/// CPrimeNumberClient
class CPrimeNumberClient : public CEventHandler
{
public:
    CPrimeNumberClient(CAppJobDispatcher& disp);

    void    StartJobs(int n);
    void    CancelJobs();

    void    CheckStatus();
    void    CheckProgress();

protected:
    DECLARE_EVENT_MAP();

    /// Job Dispatcher notification handler
    void    OnAppJobNotification(CEvent* evt);

protected:
    CAppJobDispatcher&  m_Disp;
    int     m_LastNumber;
    set<int>    m_JobIDs;
};

END_NCBI_SCOPE;

#endif  // GUI_WX_APP_DEMO___TEST_CLIENT__HPP
