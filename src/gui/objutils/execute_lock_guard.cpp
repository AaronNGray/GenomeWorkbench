/*  $Id: execute_lock_guard.cpp 31534 2014-10-21 15:31:35Z katargir $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>

#include <corelib/ncbi_system.hpp>

#include <gui/objutils/execute_lock_guard.hpp>

BEGIN_NCBI_SCOPE

CWeakExecuteGuard::~CWeakExecuteGuard()
{
    Unlock();
}

bool CWeakExecuteGuard::TryLock()
{
    if (m_Locked) return true;
    CIRef<ICommandProccessor> proccessor(m_Proccessor.Lock());
    m_Locked = proccessor ? proccessor->ExecuteLock() : true;
    return m_Locked;
}

void CWeakExecuteGuard::Lock(ICanceled* canceled)
{
    while (!TryLock()) {
        SleepMilliSec(500);
        if (canceled && canceled->IsCanceled())
            return;
    }
}

void CWeakExecuteGuard::Unlock()
{
    if (m_Locked) {
        CIRef<ICommandProccessor> proccessor(m_Proccessor.Lock());
        if (proccessor) proccessor->ExecuteUnlock();
        m_Locked = false;
    }
}

END_NCBI_SCOPE
