#ifndef CONNECT_SERVICES___BALANCING__HPP
#define CONNECT_SERVICES___BALANCING__HPP

/*  $Id: balancing.hpp 585321 2019-04-26 15:33:28Z sadyrovr $
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
 * File Description:
 *   Contains definitions related to server discovery by the load balancer.
 *
 * Authors:
 *   Dmitry Kazimirov
 *
 */

#include <corelib/ncbimtx.hpp>
#include <corelib/ncbitime.hpp>

BEGIN_NCBI_SCOPE

class CSimpleRebalanceStrategy
{
public:
    template <class TRegistry, class TSections>
    CSimpleRebalanceStrategy(TRegistry& registry, const TSections& sections) :
        m_MaxRequests(registry.Get(sections, "rebalance_requests", 5000)),
        m_MaxNs(static_cast<long>(registry.Get(sections, "rebalance_time", 10.0) * kNanoSecondsPerSecond))
    {
    }

    CSimpleRebalanceStrategy(int max_requests, double max_seconds) :
        m_MaxRequests(max_requests),
        m_MaxNs(static_cast<long>(max_seconds * kNanoSecondsPerSecond))
    {
    }

    CSimpleRebalanceStrategy(const CSimpleRebalanceStrategy& prototype) :
        m_MaxRequests(prototype.m_MaxRequests),
        m_MaxNs(prototype.m_MaxNs)
    {
    }

    bool NeedRebalance()
    {
        CFastMutexGuard g(m_Mutex);
        CTime current_time(GetFastLocalTime());
        if ((m_MaxNs > 0 && current_time >= m_NextRebalanceTime) ||
            (m_MaxRequests > 0 && m_RequestCounter >= m_MaxRequests)) {
            m_RequestCounter = 0;
            m_NextRebalanceTime = current_time;
            m_NextRebalanceTime.AddNanoSecond(m_MaxNs);
            return true;
        }
        return false;
    }

    void OnResourceRequested() {
        CFastMutexGuard g(m_Mutex);
        ++m_RequestCounter;
    }

private:
    const int m_MaxRequests;
    const long m_MaxNs;
    int        m_RequestCounter    = 0;
    CTime      m_NextRebalanceTime = CTime::eEmpty;
    CFastMutex m_Mutex;
};

END_NCBI_SCOPE

#endif  /* CONNECT_SERVICES___BALANCING__HPP */
