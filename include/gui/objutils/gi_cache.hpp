#ifndef GUI_OBJUTILS___GI_CACHE__HPP
#define GUI_OBJUTILS___GI_CACHE__HPP

/*  $Id: gi_cache.hpp 35494 2016-05-13 14:16:41Z evgeniev $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <corelib/ncbimtx.hpp>
#include <objects/seqloc/Seq_id.hpp>

#include <gui/gui_export.h>


BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
class CBioseq_Handle;
END_SCOPE(objects)

///
/// class CGiCache provides a simple, thread-safe interface for storing
/// the older or outdated versions of sequence records viewed in GBench.
/// This is useful for avoiding showing the same warning message multiple
/// times for the same record in one session.
///
class NCBI_GUIOBJUTILS_EXPORT CGiCache : public CObject
{
public:
    typedef vector<objects::CSeq_id::TGi> TCache;

    CGiCache();
    ~CGiCache();

    /// Check if the record is in the cache.
    /// if not, store it.
    bool RecordKnown(const objects::CBioseq_Handle& handle);

    /// retrieve a singleton.
    static CGiCache& GetInstance();

private:
    CFastMutex m_Mutex;
    TCache m_Cache;

    static CRef<CGiCache> m_CacheInstance;

    /// forbidden
    CGiCache(const CGiCache&);
    CGiCache& operator=(const CGiCache&);
};


END_NCBI_SCOPE


#endif  // GUI_OBJUTILS___ENTREZ_CACHE__HPP
