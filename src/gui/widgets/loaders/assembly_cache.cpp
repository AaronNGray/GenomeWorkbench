/*  $Id: assembly_cache.cpp 41426 2018-07-25 16:26:46Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/assembly_cache.hpp>

#include <serial/serial.hpp>
#include <serial/objistrasnb.hpp>

#include <gui/objects/SV_AssmAccs.hpp>
#include <objects/genomecoll/GC_Assemblies.hpp>

#include <gui/objutils/gui_http_session_request.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

//#define USE_DEV_TMS

static string s_GetAssmAccsParams(const string& seqAcc)
{
    string params =
        "&id=" + NStr::URLEncode(seqAcc) +
        "&req=getassmaccs" +
        "&ofmt=asnb";

    return params;
}

static string s_GetAssmInfoParams(const string& assmaccs)
{
    string params =
        "&assmaccs=" + NStr::URLEncode(assmaccs) +
        "&mode=AssemblyOnly" +
        "&req=getassminfos" +
        "&ofmt=asnb";

    return params;
}

static string s_GetHost()
{
#ifdef USE_DEV_TMS
    return  "dev.ncbi.nlm.nih.gov";
#else
    return  "www.ncbi.nlm.nih.gov";
#endif
}

static string s_GetPath()
{
    return "/projects/sviewer/seqconfig.cgi";
}

CAssemblyCache::CAssemblyCache() : m_StopRequested(false)
{
}

CAssemblyCache::~CAssemblyCache() = default;

CAssemblyCache& CAssemblyCache::GetInstance()
{
    static CAssemblyCache instance;
    return instance;
}

bool CAssemblyCache::GetAssemblies(const string& seqAcc, list<CRef<objects::CGC_Assembly> >& assAcc)
{
    if (seqAcc.empty())
        return true;

    {
        std::unique_lock<std::mutex> lock(m_DataMutex);
        x_StartThread();
        auto it = m_MolMap.find(seqAcc);
        if (it != m_MolMap.end()) {
            for (const auto& a : it->second)
                assAcc.push_back(m_AssMap[a]);
            return true;
        }
    }
    x_Load(seqAcc);
    return false;
}

bool CAssemblyCache::GetBestAssemblyAcc(const string& seqAcc, string& best_acc)
{
    if (seqAcc.empty())
        return true;

    {
        std::unique_lock<std::mutex> lock(m_DataMutex);
        x_StartThread();
        auto it = m_BestAccMap.find(seqAcc);
        if (it != m_BestAccMap.end()) {
            best_acc = it->second;
            return true;
        }
    }
    x_Load(seqAcc);
    return false;
}

void CAssemblyCache::Stop()
{
    if (m_WorkerThread) {
        {
            std::unique_lock<std::mutex> locker(m_QueueMutex);
            m_StopRequested = true;
        }
        m_QueueCheck.notify_one();
        m_WorkerThread.Reset();
    }
}

static void s_GetAccessions(const string& seqAcc, list<string>& accessions, string& best_acc)
{
    string host = s_GetHost();
    string path = s_GetPath();
    string params = s_GetAssmAccsParams(seqAcc);
    string url = "https://" + host + path + "?" + params;
    string errMsg = "s_GetAccessions: " + url;

    try {
        CGuiHttpSessionRequest httpRequest(url);
        unique_ptr<CObjectIStream> obj_strm(new CObjectIStreamAsnBinary(httpRequest.GetResponseStream()));

        CRef<CSV_AssmAccs> res(new CSV_AssmAccs());
        *obj_strm >> *res;

        if (res) {
            if (res->CanGetAssm_accs())
                accessions = res->GetAssm_accs();
            if (res->CanGetAssm_acc_best())
                best_acc = res->GetAssm_acc_best();
        }
    } NCBI_CATCH(errMsg);
}

static void s_GetAssemblies(const vector<string>& accessions,
                            list<CRef<objects::CGC_Assembly> >& assemblies)
{
    string assmaccs;
    for (const auto& acc : accessions) {
        if (!assmaccs.empty()) assmaccs += ",";
        assmaccs += acc;
    }

    string host = s_GetHost();
    string path = s_GetPath();
    string params = s_GetAssmInfoParams(assmaccs);
    string url = "https://" + host + path + "?" + params;
    string errMsg = "s_GetAssemblies: " + url;

    try {
        CGuiHttpSessionRequest httpRequest(url);
        unique_ptr<CObjectIStream> obj_strm(new CObjectIStreamAsnBinary(httpRequest.GetResponseStream()));
        CRef<CGC_Assemblies> res(new CGC_Assemblies());
        *obj_strm >> *res;

        if (res && res->CanGetAssemblies())
            assemblies = res->GetAssemblies();
    } NCBI_CATCH(errMsg);
}

void CAssemblyCache::x_Run()
{
    for (;;) {
        std::unique_lock<std::mutex> locker(m_QueueMutex);
        auto dataReady = [this]() { return !m_ToLoad.empty() || m_StopRequested; };
        if (!dataReady())
            m_QueueCheck.wait(locker, dataReady);

        if (m_StopRequested)
            break;

        list<string> toLoadCopy = std::move(m_ToLoad);
        locker.unlock();

        for (const auto& seqAcc : toLoadCopy) {
            {
                std::unique_lock<std::mutex> lock(m_DataMutex);
                if (m_BestAccMap.find(seqAcc) != m_BestAccMap.end())
                    continue;
            }

            list<string> accessions;
            string best_acc;
            vector<string> toLoad;
            s_GetAccessions(seqAcc, accessions, best_acc);

            {
                std::unique_lock<std::mutex> lock(m_DataMutex);
                if (!best_acc.empty()) {
                    m_BestAccMap[seqAcc] = best_acc;
                } else if (!accessions.empty()) {
                    m_BestAccMap[seqAcc] = accessions.front();
                }
                for (const auto& acc : accessions) {
                    if (m_AssMap.find(acc) == m_AssMap.end())
                        toLoad.push_back(acc);
                }
            }

            list<CRef<objects::CGC_Assembly> > assemblies;
            if (!toLoad.empty())
                s_GetAssemblies(toLoad, assemblies);

            {
                std::unique_lock<std::mutex> lock(m_DataMutex);
                for (auto a : assemblies)
                    m_AssMap[a->GetAccession()] = a;

                auto& mol = m_MolMap[seqAcc];
                for (const auto& acc : accessions) {
                    if (m_AssMap.find(acc) != m_AssMap.end())
                        mol.push_back(acc);
                }
            }

            {
                std::unique_lock<std::mutex> locker(m_QueueMutex);
                if (m_StopRequested)
                    break;
            }
        }
    }
}

void CAssemblyCache::x_StartThread()
{
    if (!m_WorkerThread) {
        m_WorkerThread.Reset(new CWorkerThread(*this));
        m_WorkerThread->Run();
    }
}

void CAssemblyCache::x_Load(const string& seqAcc)
{
    std::unique_lock<std::mutex> locker(m_QueueMutex);
    list<string>::const_iterator it2 = find(m_ToLoad.begin(), m_ToLoad.end(), seqAcc);
    if (it2 == m_ToLoad.end()) {
        m_ToLoad.push_back(seqAcc);
        m_QueueCheck.notify_one();
    }
}

END_NCBI_SCOPE
