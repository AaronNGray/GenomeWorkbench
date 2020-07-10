/*  $Id: rmtfilestatus.cpp 41935 2018-11-15 18:19:14Z rudnev $
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
 * Authors:  Dmitry Rudnev
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>
#include <corelib/ncbimisc.hpp>
#include <util/checksum.hpp>

#include <gui/utils/rmtfilestatus.hpp>



BEGIN_NCBI_SCOPE

unique_ptr<CNetICacheClient> CRmtFileStatus::m_pCacheClient;
CRmtFileStatus::TCounter CRmtFileStatus::m_MaxCount;
CRmtFileStatus::TTimeStamp CRmtFileStatus::m_MaxCheckInterval;
bool CRmtFileStatus::m_isStatusNotAvailable;

CNetICacheClient* CRmtFileStatus::x_Init()
{
    if(m_isStatusNotAvailable) {
        return NULL;
    }
    if(!m_pCacheClient) {
        try {
            const CNcbiRegistry& reg = CNcbiApplication::Instance()->GetConfig();
            m_MaxCount = reg.GetInt("RmtFileStatusCache", "max_count", 0);
            m_MaxCheckInterval = reg.GetInt("RmtFileStatusCache", "max_check_interval", 0);
            if(m_MaxCount != 0 && m_MaxCheckInterval != 0) {
                string cache_svc = reg.GetString("RmtFileStatusCache", "service", "IC_SV_TrackHubs_PROD");
                string cache_name = reg.GetString("RmtFileStatusCache", "cache", "rmttrackstatus");
                m_pCacheClient.reset(new CNetICacheClient(cache_svc, cache_name, "sviewer"));
                // the following will provoke an exception if the service does not exist/is not available so we will not have to 
                // try to access it again
                if(m_pCacheClient) {
                    int probe(0);
                    m_pCacheClient->Store("probe", 0, "", &probe, sizeof(probe));
                }
            } else {
                m_isStatusNotAvailable = true;
                return NULL;
            }
        } catch(...) {
            m_isStatusNotAvailable = true;
            return NULL;
        }
    }
    return m_pCacheClient.get();
}

bool CRmtFileStatus::x_Read(CNetICacheClient* cache, const string& sNCKey, int version, const string& sSubKey, void* data, size_t data_size)
{
    if(!cache) {
        return false;
    }
    try {
        return cache->Read(sNCKey, version, sSubKey, data, data_size);
    } catch(...) {
        return false;
    }
}

void CRmtFileStatus::x_ResetSkipLimits(const string& sNCKey)
{
    CNetICacheClient* cache(x_Init());
    if(!cache) {
        return;
    }
    TCounter count_zero(0);
    cache->Store(sNCKey, 0, "counter", &count_zero, sizeof(count_zero));
    TTimeStamp time_stamp(time(NULL));
    cache->Store(sNCKey, 0, "time_stamp", &time_stamp, sizeof(time_stamp));
    // initial check time interval is 10 min / 600 seconds
    TTimeStamp time_interval(600);
    cache->Store(sNCKey, 0, "time_interval", &time_interval, sizeof(time_interval));
}

// gives suggested course of action for a given file
CRmtFileStatus::ESuggestedAction CRmtFileStatus::Check(const string& sKey)
{
    LOG_POST(Warning<< "CRmtFileStatus::Check(" << sKey << ")");
    CNetICacheClient* cache(x_Init());

    // all checks are off if nothing is configured in .ini file or NC is not available
    if(!cache) {
        LOG_POST(Warning<< "CRmtFileStatus::Check(): " << " return forced Access");
        return ESuggestedAction_Access;
    }
    string sNCKey(x_KeyToNCKey(sKey));
    ESuggestedAction CurrentAccessibility;
    bool isCurrentAccessibilityKnown{x_Read(cache, sNCKey, 0, "access_action", &CurrentAccessibility, sizeof(CurrentAccessibility))};

    if(CurrentAccessibility == ESuggestedAction_Access ||
       !isCurrentAccessibilityKnown) {
        LOG_POST(Warning<< "CRmtFileStatus::Check(): " << "return Access, CurrentAccessibility: " << CurrentAccessibility << ", isCurrentAccessibilityKnown: " << isCurrentAccessibilityKnown);
        return ESuggestedAction_Access;
    }

    TCounter CurrentCounter(0);
    bool isCurrentCounterKnown{x_Read(cache, sNCKey, 0, "counter", &CurrentCounter, sizeof(CurrentCounter))};
    if(!isCurrentCounterKnown) {
        CurrentCounter = 0;
    }
    if(m_MaxCount != 0) {
        if(++CurrentCounter > m_MaxCount) {
            x_ResetSkipLimits(sNCKey);
            LOG_POST(Warning<< "CRmtFileStatus::Check(): " << "return Access, CurrentCounter: " << CurrentCounter);
            return ESuggestedAction_Access;
        }
        cache->Store(sNCKey, 0, "counter", &CurrentCounter, sizeof(CurrentCounter));
    }
    TTimeStamp TimeStamp(0);
    bool isCurrentTimeStampKnown{x_Read(cache, sNCKey, 0, "time_stamp", &TimeStamp, sizeof(TimeStamp))};
    TTimeStamp TimeInterval(0);
    bool isCurrentTimeIntervalKnown{x_Read(cache, sNCKey, 0, "time_interval", &TimeInterval, sizeof(TimeInterval))};

    if(m_MaxCheckInterval != 0) {
        if(!isCurrentTimeStampKnown || !isCurrentTimeIntervalKnown) {
            x_ResetSkipLimits(sNCKey);
            LOG_POST(Warning<< "CRmtFileStatus::Check(): " << "return Access, no time record");
            return ESuggestedAction_Access;
        }
        if(time(NULL) - TimeStamp > TimeInterval) {
            LOG_POST(Warning<< "CRmtFileStatus::Check(): " << "return Access, time stamp: " << TimeStamp << ", time interval: " << TimeInterval << ", actual time diff: " << (time(NULL) - TimeStamp));
            TimeStamp = time(NULL);
            cache->Store(sNCKey, 0, "time_stamp", &TimeStamp, sizeof(TimeStamp));
            TimeInterval *= 2;
            if(TimeInterval < m_MaxCheckInterval) {
                cache->Store(sNCKey, 0, "time_interval", &TimeInterval, sizeof(TimeInterval));
            }
            return ESuggestedAction_Access;
        }
    }
    LOG_POST(Warning<< "CRmtFileStatus::Check(): " << "return Skip, CurrentCounter: " << CurrentCounter << ", time stamp: " << TimeStamp << ", time interval: " << TimeInterval << ", actual time diff: " << (time(NULL) - TimeStamp));
    return ESuggestedAction_Skip;
}

// this must be called when accessibility of the file is known
void CRmtFileStatus::Set(const string& sKey, 
                                CRmtFileStatus::ESuggestedAction KnownAccessibility)
{
    CNetICacheClient* cache(x_Init());

    if(!cache) {
        return;
    }
    string sNCKey(x_KeyToNCKey(sKey));
    ESuggestedAction CurrentAccessibility;
    bool isCurrentAccessibilityKnown{x_Read(cache, sNCKey, 0, "access_action", &CurrentAccessibility, sizeof(CurrentAccessibility))};
    cache->Store(sNCKey, 0, "access_action", &KnownAccessibility, sizeof(KnownAccessibility));
    if(KnownAccessibility == ESuggestedAction_Access ||
       !isCurrentAccessibilityKnown ||
       (KnownAccessibility == ESuggestedAction_Skip && CurrentAccessibility == ESuggestedAction_Access)) {
        x_ResetSkipLimits(sNCKey);
    }
    LOG_POST(Warning<< "CRmtFileStatus::Set(" << sKey << ") was " << CurrentAccessibility << ", set to: " << KnownAccessibility);
}

// convert any string to a key that can be an NC blob key
string CRmtFileStatus::x_KeyToNCKey(const string& sKey) 
{
    CChecksum cs(CChecksum::eMD5);
    cs.AddLine(sKey);
    string result(cs.GetResultHex());
    return result;
}


time_t CRmtScriptTimeout::m_Timeout;

time_t CRmtScriptTimeout::Get()
{
    if(!m_Timeout) {
        const CNcbiRegistry& reg = CNcbiApplication::Instance()->GetConfig();
        m_Timeout = reg.GetInt("RmtPipeline", "script_timeout", 160);
    }
    return m_Timeout;
}


END_NCBI_SCOPE

