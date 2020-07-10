/*  $Id: gencoll_svc.cpp 44931 2020-04-21 17:20:26Z rudnev $
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
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>

#include <objects/seq/seq_id_handle.hpp>
#include <objects/genomecoll/GCClient_AssemblyInfo.hpp>
#include <objects/genomecoll/GCClient_AssembliesForSequ.hpp>
#include <objects/genomecoll/GCClient_AssemblySequenceI.hpp>

#include <objmgr/util/sequence.hpp>

#include <gui/objutils/gencoll_svc.hpp>
#include <gui/utils/log_performance.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

DEFINE_CLASS_STATIC_FAST_MUTEX(CGencollSvc::sm_SeqAccToAssmAccsCacheMutex);
CGencollSvc::TSeqAccToAssmAccsCache CGencollSvc::sm_SeqAccToAssmAccsCache;

CRef<CGencollSvc> CGencollSvc::GetInstance(void)
{
    static CSafeStatic<CGencollSvc> s_Instance;
    return Ref(&s_Instance.Get());
}

CGencollSvc::CGencollSvc()
{
}

static string s_MakeCacheKey(const string& assm_acc, const string& mode)
{
    return assm_acc + "#" + mode;
}

CRef<CGC_Assembly> CGencollSvc::GetGCAssembly(const string& assm_acc, bool use_caching, const string& mode, IGencollSvcErrors *errors)
{
    CLogPerformance perflog("CGencollSvc::GetGCAssembly()");
    string sCacheKey{s_MakeCacheKey(assm_acc, mode)};
    if (use_caching) {
        CFastMutexGuard lock(m_Mutex);
        auto it = m_GC_Assembly_cache.find(sCacheKey);
        if (it != m_GC_Assembly_cache.end()) {
            perflog.AddParameter("Assembly source", "cache");
            perflog.Post();
            return it->second;
        }
    }
    CRef<CGenomicCollectionsService> gencoll_service(CGencollSvc::GetGenCollService());
	try {
        CRef<CGC_Assembly> gc_assembly = gencoll_service->GetAssembly(assm_acc, mode);

        // should we cache if gc_assembly is null?
        if (use_caching) {
            CFastMutexGuard lock(m_Mutex);
            auto it = m_GC_Assembly_cache.find(sCacheKey);
            if (it != m_GC_Assembly_cache.end()) {
                perflog.AddParameter("Assembly source", "GenColl");
                perflog.Post();
                return it->second;
            }
            m_GC_Assembly_cache[sCacheKey] = gc_assembly;
            if (gc_assembly && gc_assembly->GetAccession() != assm_acc) {
                m_GC_Assembly_cache[s_MakeCacheKey(gc_assembly->GetAccession(), mode)] = gc_assembly;
            }
        }
        perflog.AddParameter("Assembly source", "GenColl");
        perflog.Post();
        return gc_assembly;
    } catch (const CException& e) {
        ERR_POST(Error << "GenColl request failed: " << e.GetMsg());
        if (nullptr != errors)
            errors->ReportError(assm_acc, e.GetMsg());
    } catch (const exception& e) {
        ERR_POST(Error << "GenColl request failed: " << e.what());
        if (nullptr != errors)
            errors->ReportError(assm_acc, e.what());
    }
    perflog.AddParameter("Assembly source", "GenColl error");
    perflog.Post();
    return CRef<CGC_Assembly>();
}

// get an instance of CGenomicCollectionsService with a given timeout in seconds
// this will never return a null (unless new fails)
// if the timeout is -1, will try to read it from registry (section [gencoll], key timeout, default value 5 seconds)
// always uses only 1 retry
CRef<CGenomicCollectionsService> CGencollSvc::GetGenCollService(int timeout_sec)
{
    string sGuessTimeout(CNcbiApplication::Instance()->GetConfig().GetString("gencoll", "timeout", "5"));
    CRef<CGenomicCollectionsService> gencoll_service(new CGenomicCollectionsService);
    // Set time-out
    unsigned timeout_sec_reg = NStr::StringToUInt(sGuessTimeout, NStr::fConvErr_NoThrow);
    STimeout timeout;
    timeout.sec = timeout_sec >= 0 ? (unsigned)timeout_sec : (timeout_sec_reg ? timeout_sec_reg : 5);
    timeout.usec = 0;
    gencoll_service->SetTimeout(&timeout);
    gencoll_service->SetRetryLimit(1);

    return gencoll_service;
}

// checks whether the bioseq handle is potentially in GenColl
// should be used to avoid unnecessary calls to GenColl (SV-2822, SV-3997)
bool CGencollSvc::isGenCollSequence(const objects::CBioseq_Handle& handle)
{
    try {
        // first: check that this is an NC_ (SV-4678)
        string text_id;
        objects::CSeq_id_Handle best_idh(sequence::GetId(handle, sequence::eGetId_Best));
        if(!best_idh) {
            text_id = handle.GetInitialSeqIdOrNull().IsNull() ? "" : handle.GetInitialSeqIdOrNull()->GetSeqIdString();
        } else {
            best_idh.GetSeqId()->GetLabel(&text_id, CSeq_id::eContent);
        }
        if(NStr::StartsWith(text_id, "NC_", NStr::eNocase)) {
            return true;
        }
        // second: filter out remaining RNAs and all proteins
        CBioseq_Handle::TMol tmol(handle.GetSequenceType());
        if(tmol == CBioseq_Handle::TMol::eMol_rna || tmol == CBioseq_Handle::TMol::eMol_aa) {
            return false;
        }
        // third: filter out local ids
        CConstRef<CSeq_id> seqid(handle.GetSeqId());
        CSeq_id::EAccessionInfo info = seqid->IdentifyAccession();
        if((info & CSeq_id::eAcc_type_mask) == CSeq_id::e_Local) {
            return false;
        }
        // all others are good
        return true;
    } catch(...) {
        // if an accession is so bad that it can't be recognized, then it's definitely not useful for getting an assembly
        return false;
    }
}

// get assemblies for a given sequence
CGencollSvc::EGencollSvcStatus CGencollSvc::GetAssmAccs(const objects::CBioseq_Handle& handle,
                                      EGCClient_GetAssemblyBySequenceFilter filter,
                                      bool isOne,
                                      TAssmAccs& AssmAccs)
{
    CLogPerformance perflog("CGencollSvc::GetAssmAccs()");
    AssmAccs.clear();

    // do not attempt guessing for sequences that are not ever supposed to be in assemblies
    if(!isGenCollSequence(handle)) {
        perflog.AddParameter("AssmAccs source", "no guess attempt");
        perflog.Post();
        return eGSS_no_attempt;
    }
    string seqAcc;
    handle.GetSeqId()->GetLabel(&seqAcc, CSeq_id::eContent);
    string sCacheKey{seqAcc + "#" + NStr::NumericToString((int)filter) + "#" + NStr::NumericToString((int)isOne)};

    // check for presence of a given cache key in cache
    {
        CFastMutexGuard lock(sm_SeqAccToAssmAccsCacheMutex);
        auto iCacheHit(sm_SeqAccToAssmAccsCache.find(sCacheKey));
        if(iCacheHit != sm_SeqAccToAssmAccsCache.end()) {
            AssmAccs = iCacheHit->second;
            perflog.AddParameter("AssmAccs source", "cache");
            perflog.Post();
            return eGSS_from_cache;
        }
    }
    // if not cached, read from GenColl and record in cache
    {
        CRef<CGenomicCollectionsService> gencoll_service(GetGenCollService());
        try {
            if(isOne) {
                CRef<CGCClient_AssemblyInfo> pAssemblyInfo(gencoll_service->FindOneAssemblyBySequences(seqAcc,
                    filter,
                    CGCClient_GetAssemblyBySequenceRequest::eSort_latest));
                if(pAssemblyInfo.NotNull() && pAssemblyInfo->CanGetAccession()) {
                    AssmAccs.push_back(pAssemblyInfo->GetAccession());
                }
            } else {
                CRef<CGCClient_AssembliesForSequences> reply(gencoll_service->FindAssembliesBySequences(seqAcc,
                                                             filter));
                if (reply && reply->CanGetAssemblies()) {
                    for (auto i: reply->GetAssemblies()) {
                        if (i->CanGetAssembly()) {
                            AssmAccs.push_back(i->GetAssembly().GetAccession());
                        }
                    }
                }
            }
            {
                CFastMutexGuard lock(sm_SeqAccToAssmAccsCacheMutex);
                sm_SeqAccToAssmAccsCache[sCacheKey] = AssmAccs;
            }
        } catch(...) {
            LOG_POST(Error << "Call to GenColl timed out when guessing assembly for ID: " << seqAcc);
            perflog.AddParameter("AssmAccs source", "failed");
            perflog.Post();
            return eGSS_failed;
        }
        perflog.AddParameter("AssmAccs source", "GenColl");
        perflog.Post();
        return eGSS_from_gencoll;
    }
}


void CGencollSvc::GetAssmsInfo(const TAssmAccs& assm_accs, const string& mode, TAssemblies& assemblies, IGencollSvcErrors *errors)
{
    string sAssmAcc;
    try {
        for(auto &iAssmAccs: assm_accs) {
            sAssmAcc = iAssmAccs;
            CRef<objects::CGC_Assembly> assembly{ GetInstance()->GetGCAssembly(sAssmAcc, true, mode, errors) };
            if (assembly.NotNull()) {
                assemblies.push_back(assembly);
            }
            else {
                LOG_POST(Warning << "Call to GenColl failed for assembly " << sAssmAcc);
            }
        }
    }
    catch (const CException& e) {
        LOG_POST(Warning << "Call to GenColl failed for assembly " << sAssmAcc << " :" << e.what());
    }
}



END_NCBI_SCOPE
