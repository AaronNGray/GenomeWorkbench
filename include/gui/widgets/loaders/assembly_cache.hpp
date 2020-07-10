#ifndef GUI_OBJUTILS___ASSEMBLY_CACHE__HPP
#define GUI_OBJUTILS___ASSEMBLY_CACHE__HPP

/*  $Id: assembly_cache.hpp 41424 2018-07-25 15:57:48Z katargir $
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

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <mutex>
#include <condition_variable>

#include <corelib/ncbimtx.hpp>
#include <corelib/ncbithr.hpp>

#include <objects/genomecoll/GC_Assembly.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_LOADERS_EXPORT CAssemblyCache
{
public:
    static CAssemblyCache& GetInstance();
    void Stop();

    bool GetAssemblies(const string& seqAcc, list<CRef<objects::CGC_Assembly> >& assAcc);
    bool GetBestAssemblyAcc(const string& seqAcc, string& best_acc);

private:
    CAssemblyCache();
    ~CAssemblyCache();
    CAssemblyCache(const CAssemblyCache&) = delete;
    CAssemblyCache& operator=(const CAssemblyCache&) = delete;

    class CWorkerThread : public CThread
    {
    public:
        CWorkerThread(CAssemblyCache& cache) : m_Cache(cache) {}

    protected:
        virtual void* Main(void) { m_Cache.x_Run(); return 0;  }

    private:
        CAssemblyCache& m_Cache;
    };

    void x_Run();
    void x_StartThread();
    void x_Load(const string& seqAcc);

    std::mutex m_DataMutex;
    map<string, vector<string> > m_MolMap;
    map<string, CRef<objects::CGC_Assembly> > m_AssMap;
    map<string, string > m_BestAccMap;

    std::mutex              m_QueueMutex;
    std::condition_variable m_QueueCheck;

    CRef<CWorkerThread> m_WorkerThread;
    list<string>  m_ToLoad;
    bool          m_StopRequested;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___ASSEMBLY_CACHE__HPP
