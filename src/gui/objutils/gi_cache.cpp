/*  $Id: gi_cache.cpp 35227 2016-04-12 18:56:35Z katargir $
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

#include <ncbi_pch.hpp>
#include <gui/objutils/gi_cache.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/util/sequence.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CRef<CGiCache> CGiCache::m_CacheInstance;


CGiCache::CGiCache()
    : m_Cache(50000)
{
}

CGiCache::~CGiCache()
{
}


bool CGiCache::RecordKnown(const CBioseq_Handle& handle)
{
    bool is_known = false;

    CSeq_id_Handle shdl = handle.GetSeq_id_Handle();
    shdl = sequence::GetId(shdl, handle.GetScope(), sequence::eGetId_ForceGi);
    CSeq_id::TGi gi = shdl.GetGi();
    if (gi > ZERO_GI) {
        CFastMutexGuard LOCK(m_Mutex);
        TCache::const_iterator iter = std::find(m_Cache.begin(), m_Cache.end(), gi);
        if (iter != m_Cache.end()) {
            is_known = true;
        } else {
            m_Cache.push_back(gi);
        }
    }
    return is_known;
}


CGiCache& CGiCache::GetInstance()
{
    if ( !m_CacheInstance ) {
        DEFINE_STATIC_FAST_MUTEX(s_Mutex);
        CFastMutexGuard LOCK(s_Mutex);
        if ( !m_CacheInstance ) {
            m_CacheInstance.Reset(new CGiCache);
        }
    }
    return *m_CacheInstance;
}


END_NCBI_SCOPE


/*
 * ===========================================================================
 * $Log$
 * ===========================================================================
 */
