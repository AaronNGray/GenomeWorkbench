#ifndef GUI_UTILS___LOCKER__HPP
#define GUI_UTILS___LOCKER__HPP

/*  $Id: locker.hpp 31536 2014-10-21 16:13:10Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <util/icanceled.hpp>

BEGIN_NCBI_SCOPE

class ILocker
{
public:
    virtual ~ILocker() {}

    virtual void Lock(ICanceled* canceled) = 0;
    virtual void Unlock() = 0;
};

class CLockerGuard
{
public:
    explicit CLockerGuard(ILocker* locker, ICanceled* canceled) : m_Locker(locker)
    {
        if (m_Locker) m_Locker->Lock(canceled);
    }
    ~CLockerGuard() { if (m_Locker) m_Locker->Unlock(); }

    CLockerGuard(const CLockerGuard& g) : m_Locker(g.x_Release()) {}
    void operator=(const CLockerGuard& g) { m_Locker = g.x_Release(); }

private:
    ILocker* x_Release() const { ILocker* locker = m_Locker; m_Locker = 0; return locker; }

    mutable ILocker* m_Locker;
};

END_NCBI_SCOPE

#endif  // GUI_UTILS___LOCKER__HPP
