/*  $Id: test_client.cpp 40279 2018-01-19 17:48:49Z katargir $
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

#include <ncbi_pch.hpp>

#include "test_client.hpp"
#include <gui/utils/event_translator.hpp>

#include <corelib/ncbi_system.hpp>


USING_NCBI_SCOPE;

////////////////////////////////////////////////////////////////////////////////
/// CPrimeNumbersJob

CPrimeNumbersJob::CPrimeNumbersJob(int max)
:   m_MaxNumber(max),
    m_StopRequested(false),
    m_Finished(false)
{
}


IAppJob::EJobState CPrimeNumbersJob::Run()
{
    LOG_POST("CPrimeNumbersJob::Run() Started");
    m_Finished = false;
    m_Progress.Reset(new CAppJobProgress(0.0, ""));
    m_Result = new CResult();
    m_Error.Reset();

    if(m_MaxNumber <= 1) {
        string s = "Invalid input argument " + NStr::Int8ToString(m_MaxNumber);
        m_Error.Reset(new CAppJobError(s));
        return eFailed;
    }

    int last_num = -1;
    for( int i = 1; i < m_MaxNumber  &&  ! m_StopRequested; i++ )   {
        bool prime = true;
        for( int j = 2; j <= i / 2;  j++ )   {
            if(i % j == 0)  {
                prime = false;
                break;
            }
        }
        if(prime)   {
            m_Result->m_Numbers.push_back(i);
            last_num = i;
        }


        if( i % 20 == 0)   {
            float norm = float(i) / m_MaxNumber;
            string s = "Last prime number " + NStr::IntToString(last_num);
            CAppJobProgress* pr = new CAppJobProgress(norm, s);
            m_Progress.Reset(pr);

            //LOG_POST("CPrimeNumbersJob::Run() - progress updated");

            SleepMilliSec(100);
        }
    }
    m_Finished = ! m_StopRequested;
    LOG_POST("CPrimeNumbersJob::Run() - Finished");
    return m_StopRequested ? eCanceled : eCompleted;
}


void CPrimeNumbersJob::RequestCancel()
{
    m_StopRequested = true;
}


CConstIRef<IAppJobProgress> CPrimeNumbersJob::GetProgress()
{
    return m_Progress;
}


CRef<CObject> CPrimeNumbersJob::GetResult()
{
    CRef<CObject> ref(m_Finished ? m_Result.GetPointer() : NULL);
    return ref;
}


CConstIRef<IAppJobError> CPrimeNumbersJob::GetError()
{
    return m_Error;
}


string CPrimeNumbersJob::GetDescr() const
{
    string s("Find all prime numbers between 1 and ");
    s += NStr::IntToString(m_MaxNumber);
    return s;
}

////////////////////////////////////////////////////////////////////////////////
/// CPeriodicTestJob

//static const double kStartPeriod = 8.0;
static const double kDecrement = 2.0;
static const long kSleepTime_ms = 100;

CPeriodicTestJob::CPeriodicTestJob(int period_sec)
:   m_Period((double)period_sec), // seconds
    m_StopRequested(false),
    m_Finished(false)
{
    m_Progress.Reset(new CAppJobProgress(0.0, ""));
    m_Error.Reset();
}


IAppJob::EJobState CPeriodicTestJob::Run()
{
    LOG_POST("CPeriodicTestJob::Run() Started");

    if(m_StopRequested) {
        LOG_POST("CPeriodicTestJob::Run() - Canceled");
        return eCanceled;
    }

    CTime t;
    t.SetCurrent();
    m_Period -= CTimeSpan(kDecrement);

        LOG_POST("CPeriodicTestJob::Run() - Running, current time " << t.AsString() <<
                 ", period " << m_Period.AsString());
        SleepMilliSec(kSleepTime_ms);
    if(m_Period.GetSign() == ePositive) {
        return eRunning;
    } else {
        LOG_POST("CPeriodicTestJob::Run() - Completed");
        return eCompleted;
    }
}


void CPeriodicTestJob::RequestCancel()
{
    m_StopRequested = true;
}

bool CPeriodicTestJob::IsCanceled() const
{
    return m_StopRequested;
}


CConstIRef<IAppJobProgress> CPeriodicTestJob::GetProgress()
{
    return m_Progress;
}


CRef<CObject> CPeriodicTestJob::GetResult()
{
    CRef<CObject> ref;
    return ref;
}


CConstIRef<IAppJobError> CPeriodicTestJob::GetError()
{
    return m_Error;
}


string CPeriodicTestJob::GetDescr() const
{
    string s("Periodic Job");
    return s;
}


CTimeSpan CPeriodicTestJob::GetWaitPeriod()
{
    return m_Period;
}

////////////////////////////////////////////////////////////////////////////////
/// CPrimeNumberClient

CPrimeNumberClient::CPrimeNumberClient(CAppJobDispatcher& disp)
:   m_Disp(disp),
    m_LastNumber(500)
{
}


void CPrimeNumberClient::StartJobs(int n)
{
    /*for( int i = 0; i < n;  i++ )   {
        CRef<CPrimeNumbersJob>  job(new CPrimeNumbersJob(-1));//m_LastNumber++));
        int job_id = m_Disp.StartJob(*job, "ThreadPool", *this);
        m_JobIDs.insert(job_id);

        LOG_POST("CPrimeNumberClient - started job \"" << job->GetDescr() << "\" Job ID = " << job_id);
    }*/

    // start periodic jobs
    for( int i = 0; i < n;  i++ )   {
        int period = 5 + i * 2;
        CRef<CPeriodicTestJob>  job(new CPeriodicTestJob(period));
        int job_id = m_Disp.StartJob(*job, "Scheduler", *this);
        m_JobIDs.insert(job_id);
        SleepMilliSec(250);

        LOG_POST("CPrimeNumberClient - started job \"" << job->GetDescr() << "\" Job ID = " << job_id);
    }
}


void CPrimeNumberClient::CancelJobs()
{
    /*while( ! m_JobIDs.empty())  {
        int job_id = *m_JobIDs.begin();
        m_Disp.CancelJob(job_id);
        LOG_POST("CPrimeNumberClient - Canceled job " << job_id);

        m_JobIDs.erase(m_JobIDs.begin());
    }*/
}

void CPrimeNumberClient::CheckStatus()
{
    //LOG_POST(" ");
    //LOG_POST("CPrimeNumberClient::CheckStatus()");

    for(set<int>::iterator it = m_JobIDs.begin();
            it != m_JobIDs.end();  ) {
        int job_id = *it;
        IAppJob::EJobState state = m_Disp.GetJobState(job_id);
        string s = CAppJobDispatcher::StateToStr(state);

        LOG_POST("ID " << job_id << " State " << s);

        if(CAppJobDispatcher::IsTerminal(state))   {
            m_Disp.DeleteJob(job_id);
            set<int>::iterator er_it = it;
            it++;
            m_JobIDs.erase(er_it);
            LOG_POST("Deleted Job " << job_id);
        } else {
            it++;
        }
    }
}


void CPrimeNumberClient::CheckProgress()
{
    //LOG_POST(" ");
    //LOG_POST("CPrimeNumberClient::CheckProgress()");

    ITERATE(set<int>, it, m_JobIDs) {
        int job_id = *it;
        CConstIRef<IAppJobProgress> pr = m_Disp.GetJobProgress(job_id);
        if(pr)  {
            string t = pr->GetText();
            LOG_POST("ID " << job_id << ", Progress " << pr->GetNormDone() << "  " << t);
        } else {
            LOG_POST("ID " << job_id << " Progress unknown");
        }
    }
}


BEGIN_EVENT_MAP(CPrimeNumberClient, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged,
             &CPrimeNumberClient::OnAppJobNotification)
END_EVENT_MAP()


void CPrimeNumberClient::OnAppJobNotification(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);
    if(notn)    {
        IAppJob::EJobState state = notn->GetState();
        if(state == IAppJob::eCompleted) {
            CConstIRef<IAppJobError> err = notn->GetError();
        }
    }
}
