/*  $Id: guard.cpp 451479 2014-11-06 19:47:51Z grichenk $
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
 * Authors:  Aleksey Grichenko
 *
 * File Description:
 *   CGuard<> -- implementation of RAII-based locking guard
 *
 */


#include <ncbi_pch.hpp>
#include <corelib/guard.hpp>
#include <corelib/ncbiexpt.hpp>


BEGIN_NCBI_SCOPE


void CGuard_Base::ReportException(std::exception& ex)
{
    CException* ncbi_ex = dynamic_cast<CException*>(&ex);
    if ( ncbi_ex ) {
        ERR_POST(Error << "CGuard::~CGuard(): error on release: " << *ncbi_ex);
    }
    else {
        ERR_POST(Error << "CGuard::~CGuard(): error on release: " << ex.what());
    }
}


END_NCBI_SCOPE
