#ifndef GUI_OBJUTILS___GENCOLL_SVC_HPP
#define GUI_OBJUTILS___GENCOLL_SVC_HPP

/*  $Id: gencoll_svc.hpp 43185 2019-05-23 21:26:50Z rudnev $
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
 * Author:  Liangshou Wu
 *
 * File Description:
 *   A utility class providing service for assembly information retrieval.
 * The target output is one or a list of related *full* assemblies.
 *
 */

#include <corelib/ncbiobj.hpp>
#include <corelib/ncbi_safe_static.hpp>

#include <gui/gui_export.h>

#include <objmgr/bioseq_handle.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/genomecoll/genomic_collections_cli.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT IGencollSvcErrors
{
public:
    virtual void ReportError(const string &accession, const string &error) = 0;
};

class NCBI_GUIOBJUTILS_EXPORT CGencollSvc : public CObject
{
public:
    typedef list<string> TAssmAccs;
    typedef list<CRef<objects::CGC_Assembly> > TAssemblies;
    enum EGencollSvcStatus {
        eGSS_failed,
        eGSS_from_gencoll,
        eGSS_from_cache,
        eGSS_no_attempt     // for accessions that do not satisfy isGenCollSequence()
    };

    static CRef<CGencollSvc> GetInstance(void);

    // get an instance of CGenomicCollectionsService with a given timeout in seconds
    // this will never return a null (unless new fails)
    // if the timeout is -1, will try to read it from registry (section [gencoll], key timeout, default value 5 seconds)
    // always uses only 1 retry
    static CRef<objects::CGenomicCollectionsService> GetGenCollService(int timeout_sec = -1);

// checks whether the bioseq handle is potentially in GenColl
// should be used to avoid unnecessary calls to GenColl (SV-2822, SV-3997)
    static bool isGenCollSequence(const objects::CBioseq_Handle& handle);

    // use_caching requests the use of internal assembly information cache before attempting to contact GenColl, it should be set to
    // true and used unless there are concrete contraindications to using cache
    // mode is in the meaning of CGenomicCollectionsService::GetAssembly() call, defines the scope of sequences returned with the assembly
    // "Gbench" mode gives the most information with all chromosomes and scaffolds, but can be very slow (i.e. time and space expensive)
    // "AssemblyOnly" gives only info about assembly itself, but is fast and cheap
    // "Gbench_chrs" gives info about assembly and chromosomes, somewhat inbetween the other two
    // the least expensive should be chosen whenever possible
    CRef<objects::CGC_Assembly> GetGCAssembly(const string& assm_acc, bool use_caching, const string& mode, IGencollSvcErrors *errors = nullptr);

    /// get all assembly accessions for a given sequence and filter
    /// if isOne == 1, return only the best one assembly
    EGencollSvcStatus GetAssmAccs(const objects::CBioseq_Handle& handle,
                                      objects::EGCClient_GetAssemblyBySequenceFilter filter,
                                      bool isOne,
                                      TAssmAccs& AssmAccs);

    /// query GenColl for assembly information, return list of assemblies
    /// mode is as described above for GetGCAssembly()
    static void GetAssmsInfo(const TAssmAccs& assm_accs, const string& mode, TAssemblies& assemblies, IGencollSvcErrors *errors = nullptr);

private:
    CGencollSvc(void);
    friend class CSafeStatic_Allocator<CGencollSvc>;

    CFastMutex m_Mutex; /// mutex to sync our internals
    map<string, CRef<objects::CGC_Assembly> > m_GC_Assembly_cache;

    typedef map<string, TAssmAccs> TSeqAccToAssmAccsCache;
    DECLARE_CLASS_STATIC_FAST_MUTEX(sm_SeqAccToAssmAccsCacheMutex);
    static TSeqAccToAssmAccsCache sm_SeqAccToAssmAccsCache;
};


END_NCBI_SCOPE

#endif // GUI_OBJUTILS___GENCOLL_SVC_HPP
