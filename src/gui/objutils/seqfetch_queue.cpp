/*  $Id: seqfetch_queue.cpp 33210 2015-06-17 20:37:46Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbimtx.hpp>

#include <gui/objutils/seqfetch_queue.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/seq_map_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/util/sequence.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


/////////////////////////////////////////////////////////////////////////////
///
/// Siple fatal request used for terminating processing
///
class CResolveRequestFatal : public CStdRequest
{
public:
    void Process()
    {
        CThread::Exit(0);
    }
};


/////////////////////////////////////////////////////////////////////////////
///
/// Wrapper class for a resolution request
///

class CResolveRequest : public CStdRequest
{
public:
    CResolveRequest(CScope& scope, CSeq_id_Handle idh,
                    const TSeqRange& range,
                    CSeqFetchQueue::TRetrievalFlags flags,
                    CSeqFetchQueue& parent_queue,
                    int priority);

    void Process();

private:
    CRef<CScope> m_Scope;
    CSeq_id_Handle m_Id;
    TSeqRange m_Range;
    CSeqFetchQueue::TRetrievalFlags m_Flags;
    CSeqFetchQueue& m_Queue;
    int m_Priority;
};



CResolveRequest::CResolveRequest(CScope& scope, CSeq_id_Handle idh,
                                 const TSeqRange& range,
                                 CSeqFetchQueue::TRetrievalFlags flags,
                                 CSeqFetchQueue& parent_queue,
                                 int priority)
    : m_Scope(&scope)
    , m_Id(idh)
    , m_Range(range)
    , m_Flags(flags)
    , m_Queue(parent_queue)
    , m_Priority(priority)
{
}


void CResolveRequest::Process()
{
    string id_str = m_Id.GetSeqId()->GetSeqIdString();

    /// we always get the bioseq handle
    CBioseq_Handle bsh = m_Scope->GetBioseqHandle(m_Id);

    /// also, always resolve all IDs
    m_Scope->GetIds(m_Id);


    if ( (m_Flags & CSeqFetchQueue::fRetrieveAll) ==
         CSeqFetchQueue::fRetrieveAll ) {
        ///
        /// retrieve all = retrieve complete bioseq
        /// we also ignore the range here
        ///
        bsh.GetCompleteBioseq();

    } else if (m_Flags & CSeqFetchQueue::fRetrieveFeatures) {
        ///
        /// force retrieving of all features
        /// we exclude external annotations here
        /// we only retrieve features in the selected range
        ///
        SAnnotSelector sel;
        sel.SetResolveAll()
            .SetExcludeExternal()
            .SetAdaptiveDepth();
        if (m_Range.IsWhole()) {
            CFeat_CI feat_iter(bsh, sel);
        } else {
            CFeat_CI feat_iter(bsh, m_Range, sel);
        }

    } else if (m_Flags & CSeqFetchQueue::fRetrieveCoreFeatures) {
        ///
        /// retrieve a "core" set of features
        /// this includes genes, mRNAs, and CDS features
        /// we only retrieve features in the selected range
        ///
        SAnnotSelector sel;
        sel.SetResolveAll()
            .SetExcludeExternal()
            .SetAdaptiveDepth()
            .IncludeFeatSubtype(CSeqFeatData::eSubtype_gene)
            .IncludeFeatSubtype(CSeqFeatData::eSubtype_mRNA)
            .IncludeFeatSubtype(CSeqFeatData::eSubtype_cdregion);
        if (m_Range.IsWhole()) {
            CFeat_CI feat_iter(bsh, sel);
        } else {
            CFeat_CI feat_iter(bsh, m_Range, sel);
        }
    }

    /// we plan to enqueue more items; enqueue them at a lower priority to
    /// make sure we finish the previous level first
    int next_priority = max(0, m_Priority - 1);

    ///
    /// retrieve producs, but only those on this sequence
    ///
    if (m_Flags & CSeqFetchQueue::fRetrieveProducts) {
        SAnnotSelector sel;
        sel
            .SetResolveNone()
            .SetExcludeExternal()
            .IncludeFeatSubtype(CSeqFeatData::eSubtype_mRNA)
            .IncludeFeatSubtype(CSeqFeatData::eSubtype_cdregion);

        if (m_Range.IsWhole()) {
            CFeat_CI feat_iter(bsh, sel);
            for ( ;  feat_iter;  ++feat_iter) {
                if ( !feat_iter->IsSetProduct() ) {
                    continue;
                }
                CSeq_id_Handle idh =
                    sequence::GetIdHandle(feat_iter->GetProduct(), m_Scope);
                m_Queue.Add(idh, m_Range, m_Flags, next_priority);
            }
        } else {
            TSeqRange whole = TSeqRange::GetWhole();
            CFeat_CI feat_iter(bsh, m_Range, sel);
            for ( ;  feat_iter;  ++feat_iter) {
                if ( !feat_iter->IsSetProduct() ) {
                    continue;
                }
                CSeq_id_Handle idh =
                    sequence::GetIdHandle(feat_iter->GetProduct(), m_Scope);

                /// we ignore the range on descent
                m_Queue.Add(idh, whole, m_Flags, next_priority);
            }
        }
    }

    ///
    /// descend recursively through the seq-map at this level to retrieve
    /// the main sequence here
    ///
    if (m_Flags & CSeqFetchQueue::fRetrieveRecursive) {
        SSeqMapSelector sel;
        sel.SetFlags(CSeqMap::fFindRef)
            .SetResolveCount(0);

        if (m_Range.IsWhole()) {
            CSeqMap_CI seqmap_iter(bsh, sel);
            for ( ;  seqmap_iter;  ++seqmap_iter) {
                CSeq_id_Handle idh = seqmap_iter.GetRefSeqid();
                if (idh) {
                    m_Queue.Add(idh, m_Range, m_Flags, next_priority);
                }
            }
        } else {
            TSeqRange whole = TSeqRange::GetWhole();
            CSeqMap_CI seqmap_iter(bsh, sel, m_Range.GetFrom());
            for ( ;  seqmap_iter;  ++seqmap_iter) {
                CSeq_id_Handle idh = seqmap_iter.GetRefSeqid();
                if (idh) {
                    /// we ignore the range on descent
                    m_Queue.Add(idh, whole, m_Flags, next_priority);
                }

                if (seqmap_iter.GetEndPosition() > m_Range.GetTo()) {
                    break;
                }
            }
        }
    }
    /**
    LOG_POST(Error << "retrieved: " << m_Id.GetSeqId()->GetSeqIdString()
             << ": " << m_Flags << " / " << m_Priority);
    **/
}



/////////////////////////////////////////////////////////////////////////////
///
/// CSeqFetchQueue implementation details
///


CSeqFetchQueue::CSeqFetchQueue(objects::CScope& scope,
                               int threads, int max_queue_size)
    : CStdPoolOfThreads(threads, max_queue_size)
    , m_Scope(&scope)
    , m_IsShuttingDown(false)
{
}


CSeqFetchQueue& CSeqFetchQueue::GetInstance()
{
    static CRef<CSeqFetchQueue> s_Queue;
    if ( !s_Queue ) {
        DEFINE_STATIC_MUTEX(s_Mutex);
        CMutexGuard LOCK(s_Mutex);
        if ( !s_Queue ) {
            CRef<CObjectManager> om(CObjectManager::GetInstance());
            CRef<CScope> scope(new CScope(*om));
            scope->AddDefaults();
            s_Queue.Reset(new CSeqFetchQueue(*scope, 2));
        }
    }

    return *s_Queue;
}


bool CSeqFetchQueue::IsFull() const
{
    return m_Queue.IsFull();
}


bool CSeqFetchQueue::IsEmpty() const
{
    return m_Queue.IsEmpty();
}


void CSeqFetchQueue::Clear()
{
    m_IsShuttingDown = true;
    while ( !IsEmpty() ) {
        m_Queue.GetHandle();
    }
    Finish();
}


void CSeqFetchQueue::Finish()
{
    m_IsShuttingDown = true;
    CRef<CStdRequest> req(new CResolveRequestFatal);
    AcceptRequest(req);
    while ( !IsEmpty() ) {
    }
    KillAllThreads(true);
}


void CSeqFetchQueue::Add(CSeq_id_Handle idh,
                         const TSeqRange& range,
                         TRetrievalFlags flags,
                         int priority)
{
    if (m_IsShuttingDown) {
        return;
    }

    CRef<CStdRequest> req
        (new CResolveRequest(*m_Scope, idh, range, flags, *this, priority));
    AcceptRequest(req, priority);
}


/// enqueue an ID for retrieval
void CSeqFetchQueue::Add(const CSeq_id& id,
                         TRetrievalFlags flags)
{
    if (m_IsShuttingDown) {
        return;
    }

    TSeqRange range = TSeqRange::GetWhole();
    CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(id);
    Add(idh, range, flags, 128);
}


void CSeqFetchQueue::Add(const CSeq_id& id,
                         const TSeqRange& range,
                         TRetrievalFlags flags)
{
    if (m_IsShuttingDown) {
        return;
    }

    CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(id);
    Add(idh, range, flags, 128);
}


/// enqueue an ID for retrieval
void CSeqFetchQueue::Add(objects::CSeq_id_Handle id,
                         TRetrievalFlags flags)
{
    if (m_IsShuttingDown) {
        return;
    }

    TSeqRange range = TSeqRange::GetWhole();
    Add(id, range, flags, 128);
}



/// Enqueue a set of IDs for retrieval
void CSeqFetchQueue::Add(const vector<CSeq_id_Handle>& ids,
                       TRetrievalFlags flags)
{
    if (m_IsShuttingDown) {
        return;
    }

    TSeqRange range = TSeqRange::GetWhole();
    ITERATE (vector<CSeq_id_Handle>, iter, ids) {
        Add(*iter, range, flags, 128);
    }
}


/// Enqueue a set of gis for retrieval
void CSeqFetchQueue::Add(const vector<TGi>& ids,
                       TRetrievalFlags flags)
{
    if (m_IsShuttingDown) {
        return;
    }

    TSeqRange range = TSeqRange::GetWhole();
    ITERATE (vector<TGi>, iter, ids) {
        CSeq_id_Handle idh = CSeq_id_Handle::GetGiHandle(*iter);
        Add(idh, range, flags, 128);
    }
}


END_NCBI_SCOPE
