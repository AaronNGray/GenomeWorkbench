#ifndef GUI_UTILS___CANCELED_IMPL__HPP
#define GUI_UTILS___CANCELED_IMPL__HPP

/*  $Id: canceled_impl.hpp 26667 2012-10-19 17:47:39Z katargir $
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
 */

/** @addtogroup GUI_UTILS
*
* @{
*/

#include <corelib/ncbicntr.hpp>

#include <util/icanceled.hpp>

BEGIN_NCBI_SCOPE

class CCanceledImpl : public ICanceled
{
public:
    void RequestCancel() { m_Cancel.Set(1); }

    // ICanceled implementation
    virtual bool IsCanceled() const { return (m_Cancel.Get() != 0); }

private:        
    CAtomicCounter_WithAutoInit m_Cancel;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_UTILS___CANCELED_IMPL__HPP
