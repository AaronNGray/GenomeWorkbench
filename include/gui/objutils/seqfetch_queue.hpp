#ifndef GUI_OBJUTILS___SEQFETCH_QUEUE__HPP
#define GUI_OBJUTILS___SEQFETCH_QUEUE__HPP

/*  $Id: seqfetch_queue.hpp 33210 2015-06-17 20:37:46Z katargir $
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

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <util/thread_pool.hpp>
#include <util/range.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CScope;
    class CSeq_id;
    class CSeq_id_Handle;
END_SCOPE(objects)


class NCBI_GUIOBJUTILS_EXPORT CSeqFetchQueue : public CObject,
                                               private CStdPoolOfThreads
{
public:
    enum ERetrievalFlags {

        /// retrieve the sequence associated with the ID
        fRetrieveSequence     = 0x01,

        /// retrieve the "core" features
        /// this includes genes, mRNAs, and CDSs only
        fRetrieveCoreFeatures = 0x02,

        /// retrieve all features not in external annotations
        fRetrieveFeatures     = 0x04,

        fRetrieveAll = fRetrieveSequence | fRetrieveFeatures,

        /// should we enqueue all items recursively?
        /// levels below the requested level are retrieved with a lower
        /// priority to favor interactive tasks that need immediate attention
        fRetrieveRecursive   = 0x08,

        /// should we enqueue product retrievals as well?
        /// this implies retrieval of mRNA and CDS features
        fRetrieveProducts    = 0x10,

        /// default just calls GetBioseqHandle()
        fDefaults = 0x0
    };
    typedef int TRetrievalFlags;

    CSeqFetchQueue(objects::CScope& scope,
                 int threads = 4, int max_queue_size = 100000);

    /// Enqueue a single ID for retrieval
    void Add(const objects::CSeq_id& id,
             TRetrievalFlags flags = fDefaults);
    void Add(const objects::CSeq_id& id,
             const TSeqRange& range,
             TRetrievalFlags flags = fDefaults);

    /// Enqueue a single ID for retrieval
    void Add(objects::CSeq_id_Handle id,
             TRetrievalFlags flags = fDefaults);
    void Add(objects::CSeq_id_Handle id,
             const TSeqRange& range,
             TRetrievalFlags flags = fDefaults);

    /// Enqueue a set of IDs for retrieval
    void Add(const vector<objects::CSeq_id_Handle>& ids,
             TRetrievalFlags flags = fDefaults);

    /// Enqueue a set of gis for retrieval
    void Add(const vector<TGi>& ids,
             TRetrievalFlags flags = fDefaults);

    /// Ask if the queue is full or empty
    bool IsFull() const;
    bool IsEmpty() const;

    /// Clear all items from the queue
    void Clear();

    /// Finish processing of all items in the queue
    void Finish();

    ///
    /// static interface to access singleton
    ///
    static CSeqFetchQueue& GetInstance();

protected:

    friend class CResolveRequest;

    /// internal version of add
    /// all of the other APIs feed into this one
    void Add(objects::CSeq_id_Handle idh,
             const TSeqRange& range,
             TRetrievalFlags flags, int priority);

private:
    CRef<objects::CScope> m_Scope;

    /// flag: set to true when the class is shutting down
    /// this blocks all further acceptances
    bool m_IsShuttingDown;

    CSeqFetchQueue(const CSeqFetchQueue&);
    CSeqFetchQueue& operator=(const CSeqFetchQueue&);
};



END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___SEQFETCH_QUEUE__HPP
