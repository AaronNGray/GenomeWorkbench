/*  $Id: netschedule_api_getjob.cpp 599189 2019-12-20 15:57:19Z sadyrovr $
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
 * Authors: Rafael Sadyrov
 *
 * File Description:
 *   NetSchedule API get/read job implementation.
 *
 */

#include <ncbi_pch.hpp>

#include "grid_worker_impl.hpp"
#include "netschedule_api_impl.hpp"
#include "netservice_api_impl.hpp"

#include "netschedule_api_getjob.hpp"

BEGIN_NCBI_SCOPE


template <class TImpl> class CAnyAffinityJob;
template <class TImpl> class CMostAffinityJob;

template <class TImpl>
CNetScheduleGetJob::EResult CNetScheduleGetJobImpl<TImpl>::GetJob(
        const CDeadline& deadline,
        CNetScheduleJob& job,
        CNetScheduleAPI::EJobStatus* job_status,
        bool any_affinity)
{
    if (any_affinity) {
        CAnyAffinityJob<TImpl> holder(job, job_status, m_ImmediateActions);
        return GetJobImpl(deadline, holder);
    } else {
        ReturnNotFullyCheckedServers();
        CMostAffinityJob<TImpl> holder(job, job_status, m_ImmediateActions, m_Impl);
        return GetJobImpl(deadline, holder);
    }
}

typedef list<CNetServer::SAddress> TServers;
typedef list<CNetScheduleGetJob::SEntry> TTimeline;
typedef TTimeline::iterator TIterator;

template <class TImpl>
class CAnyAffinityJob
{
public:
    CNetScheduleJob& job;
    CNetScheduleAPI::EJobStatus* job_status;

    CAnyAffinityJob(CNetScheduleJob& j, CNetScheduleAPI::EJobStatus* js,
            TTimeline& timeline) :
        job(j), job_status(js), m_Timeline(timeline)
    {}

    void Interrupt()                {}
    TIterator Begin()               { return m_Timeline.begin(); }
    TIterator Next(bool)            { return m_Timeline.begin(); }
    const string& Affinity() const  { return kEmptyStr; }
    bool Done()                     { return true; }
    bool HasJob() const             { return false; }

private:
    TTimeline& m_Timeline;
};

template <class TImpl>
class CMostAffinityJob
{
public:
    CNetScheduleJob& job;
    CNetScheduleAPI::EJobStatus* job_status;

    CMostAffinityJob(CNetScheduleJob& j, CNetScheduleAPI::EJobStatus* js,
            TTimeline& timeline, TImpl& get_job_impl) :
        job(j), job_status(js), m_JobPriority(numeric_limits<size_t>::max()),
        m_Timeline(timeline), m_Iterator(timeline.end()),
        m_GetJobImpl(get_job_impl)
    {
        _ASSERT(m_GetJobImpl.m_API->m_AffinityLadder.size());
    }

    void Interrupt()
    {
        if (HasJob()) {
            m_GetJobImpl.ReturnJob(job);
            job.Reset();
        }
    }

    TIterator Begin()
    {
        m_Iterator = m_Timeline.end();
        return m_Timeline.begin();
    }

    TIterator Next(bool increment)
    {
        if (increment) {
            if (m_Iterator == m_Timeline.end()) {
                m_Iterator = m_Timeline.begin();
            } else {
                ++m_Iterator;
            }

            // We've already got a job from an entry at m_Iterator + 1
            // (that is why increment is true), so must not happen
            _ASSERT(m_Iterator != m_Timeline.end());

        } else if (m_Iterator == m_Timeline.end()) {
            return m_Timeline.begin();
        }

        TIterator ret = m_Iterator;
        return ++ret;
    }

    const string& Affinity() const
    {
        // Must not happen, since otherwise Done() has returned true already
        _ASSERT(m_JobPriority);

        CNetScheduleGetJob::TAffinityLadder&
            affinity_ladder(m_GetJobImpl.m_API->m_AffinityLadder);

        if (HasJob()) {
            // Only affinities that are higher that current job's one
            return affinity_ladder[m_JobPriority - 1].second;
        } else {
            // All affinities
            return affinity_ladder.back().second;
        }
    }

    bool Done()
    {
        // Must not happen, since otherwise Done() has returned true already
        _ASSERT(m_JobPriority);

        // Return a less-priority job back
        if (HasJob()) {
            m_GetJobImpl.ReturnJob(m_PreviousJob);
        }

        m_PreviousJob = job;

        CNetScheduleGetJob::TAffinityLadder&
            affinity_ladder(m_GetJobImpl.m_API->m_AffinityLadder);

        size_t priority = min(affinity_ladder.size(), m_JobPriority) - 1;

        do {
            if (job.affinity == affinity_ladder[priority].first) {
                m_JobPriority = priority;

                // Return true, if job has the highest priority (zero)
                return !m_JobPriority;
            }
        } while (priority-- > 0);

        // Whether affinities not from the ladder are allowed
        if (m_GetJobImpl.m_API->m_AffinityPreference ==
                CNetScheduleExecutor::eAnyJob) {
            // Make it the least-priority
            m_JobPriority = affinity_ladder.size();
        } else {
            // Should not happen
            ERR_POST("Got a job " << job.job_id <<
                    " with unexpected affinity " << job.affinity);
            m_JobPriority = numeric_limits<size_t>::max();
        }

        return false;
    }

    bool HasJob() const
    {
        return m_JobPriority < numeric_limits<size_t>::max();
    }

private:
    size_t m_JobPriority;
    TTimeline& m_Timeline;
    TIterator m_Iterator;
    TImpl& m_GetJobImpl;
    CNetScheduleJob m_PreviousJob;
};

template <class TImpl>
template <class TJobHolder>
CNetScheduleGetJob::EResult CNetScheduleGetJobImpl<TImpl>::GetJobImmediately(TJobHolder& holder)
{
    TIterator i = holder.Begin();

    for (;;) {
        EState state = m_Impl.CheckState();

        if (state == eStopped) {
            holder.Interrupt();
            return eInterrupt;
        }
        
        if (state == eRestarted) {
            Restart();
            i = holder.Begin();
            continue;
        }

        // We must check i here to let state be checked before leaving loop
        if (i == m_ImmediateActions.end()) {
            return holder.HasJob() ? eJob : eAgain;
        }

        if (*i == m_DiscoveryAction) {
            NextDiscoveryIteration();
            i = holder.Begin();
            continue;
        }

        // Whether to move to the next entry
        // (false means we are already at the next entry due to splice/erase)
        bool increment = false;

        try {
            // Get prioritized affinity list and
            // a flag whether any affinity job is appropriate
            const string& prio_aff_list = holder.Affinity();
            const bool any_affinity = !holder.HasJob();

            if (m_Impl.CheckEntry(*i, prio_aff_list, any_affinity,
                        holder.job, holder.job_status)) {
                if (i == m_ImmediateActions.begin()) {
                    increment = true;
                } else {
                    // We have got a more prioritized job from this server.
                    // Move this server to the top of immediate actions,
                    // so we will have servers ordered (most-to-least)
                    // by affinities of the jobs they have returned last
                    m_ImmediateActions.splice(m_ImmediateActions.begin(),
                            m_ImmediateActions, i);
                }

                // A job has been returned; keep the server in
                // immediate actions because there can be more
                // jobs in the queue.
                if (holder.Done()) {
                    return eJob;
                }
            } else {
                // No job has been returned by this server;
                // query the server later.
                i->deadline = CDeadline(m_Impl.m_Timeout, 0);
                i->all_affinities_checked = any_affinity;
                m_ScheduledActions.splice(m_ScheduledActions.end(),
                        m_ImmediateActions, i);
            }
        }
        catch (CNetSrvConnException& e) {
            // Because a connection error has occurred, do not
            // put this server back to the timeline.
            m_ImmediateActions.erase(i);
            ERR_POST(Warning << e.GetMsg());
        }
        catch (...) {
            m_ImmediateActions.erase(i);

            if (holder.HasJob()) {
                return eJob;
            }

            throw;
        }

        // Check all servers that have timeout expired
        while (!m_ScheduledActions.empty() &&
                m_ScheduledActions.front().deadline.GetRemainingTime().IsZero()) {
            m_ImmediateActions.splice(m_ImmediateActions.end(),
                    m_ScheduledActions, m_ScheduledActions.begin());
        }

        // Check if there's a notification in the UDP socket.
        while (CNetServer server = m_Impl.ReadNotifications()) {
            MoveToImmediateActions(server);
        }

        i = holder.Next(increment);
    }
}

template <class TImpl>
template <class TJobHolder>
CNetScheduleGetJob::EResult CNetScheduleGetJobImpl<TImpl>::GetJobImpl(
        const CDeadline& deadline, TJobHolder& holder)
{
    for (;;) {
        EResult ret = GetJobImmediately(holder);

        if (ret != eAgain) {
            return ret;
        }

        auto entry_has_more_jobs = [&](const SEntry& entry) {
            return m_Impl.MoreJobs(entry);
        };

        // If MoreJobs() returned false for all entries of m_ScheduledActions
        if (find_if(m_ScheduledActions.begin(), m_ScheduledActions.end(),
                    entry_has_more_jobs) == m_ScheduledActions.end()) {
            return eNoJobs;
        }

        if (deadline.IsExpired())
            return eAgain;

        // At least, the discovery action must be there
        _ASSERT(!m_ScheduledActions.empty());

        // There's still time. Wait for notifications and query the servers.
        CDeadline next_event_time = m_ScheduledActions.front().deadline;
        bool last_wait = deadline < next_event_time;
        if (last_wait) next_event_time = deadline;

        if (CNetServer server = m_Impl.WaitForNotifications(next_event_time)) {
            do {
                MoveToImmediateActions(server);
            } while ((server = m_Impl.ReadNotifications()));
        } else if (last_wait) {
            return eAgain;
        } else {
            m_ImmediateActions.splice(m_ImmediateActions.end(),
                    m_ScheduledActions, m_ScheduledActions.begin());
        }
    }
}

inline void Filter(TTimeline& timeline, TServers& servers)
{
    TTimeline::iterator i = timeline.begin();

    while (i != timeline.end()) {
        const CNetServer::SAddress& address(i->server_address);
        TServers::iterator j = find(servers.begin(), servers.end(), address);

        // If this server is still valid
        if (j != servers.end()) {
            servers.erase(j);
            ++i;
        } else {
            timeline.erase(i++);
        }
    }
}

template <class TImpl>
void CNetScheduleGetJobImpl<TImpl>::Restart()
{
    // Rediscover all servers
    m_ImmediateActions.clear();
    m_ScheduledActions.clear();
    NextDiscoveryIteration();
}

template <class TImpl>
void CNetScheduleGetJobImpl<TImpl>::MoveToImmediateActions(SNetServerImpl* server_impl)
{
    SEntry entry(server_impl->m_ServerInPool->m_Address);

    TTimeline::iterator i = find(m_ScheduledActions.begin(),
            m_ScheduledActions.end(), entry);

    // Server was postponed, move to immediate
    if (i != m_ScheduledActions.end()) {
        m_ImmediateActions.splice(m_ImmediateActions.end(),
                m_ScheduledActions, i);
        return;
    }

    TTimeline::iterator j = find(m_ImmediateActions.begin(),
            m_ImmediateActions.end(), entry);

    // It's new server, add to immediate
    if (j == m_ImmediateActions.end()) {
        m_ImmediateActions.push_back(entry);
    }
}

template <class TImpl>
void CNetScheduleGetJobImpl<TImpl>::NextDiscoveryIteration()
{
    TServers servers;

    for (CNetServiceIterator it =
            m_Impl.m_API.GetService().Iterate(
                CNetService::eIncludePenalized); it; ++it) {
        servers.push_back((*it)->m_ServerInPool->m_Address);
    }

    // Keep up to date servers
    Filter(m_ImmediateActions, servers);
    Filter(m_ScheduledActions, servers);

    // Add newly discovered servers
    for (TServers::const_iterator i = servers.begin();
            i != servers.end(); ++i) {
        m_ImmediateActions.push_back(*i);
    }

    // Reschedule discovery after timeout
    m_DiscoveryAction.deadline = CDeadline(m_Impl.m_Timeout, 0);
    m_ScheduledActions.push_back(m_DiscoveryAction);
}

template <class TImpl>
void CNetScheduleGetJobImpl<TImpl>::ReturnNotFullyCheckedServers()
{
    // Return back to immediate actions
    // all servers that have not been checked for all possible affinities
    TIterator i = m_ScheduledActions.begin();

    while (i != m_ScheduledActions.end()) {
        if (i->all_affinities_checked) {
            ++i;
        } else {
            m_ImmediateActions.splice(m_ImmediateActions.end(),
                    m_ScheduledActions, i++);
        } 
    }
}


template class CNetScheduleGetJobImpl<CMainLoopThread::CImpl>;
template class CNetScheduleGetJobImpl<SNetScheduleJobReaderImpl::CImpl>;


END_NCBI_SCOPE
