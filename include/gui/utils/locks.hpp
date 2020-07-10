#ifndef GUI_CORE___LOCKS__HPP
#define GUI_CORE___LOCKS__HPP

/* $Id: locks.hpp 19743 2009-07-30 01:44:43Z dicuccio $
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
* Authors:  Anatoliy Kuznetsov
*
* File Description:
*
*
*/

/// @file locks.hpp
///   GUI Core entry point to compile time overloaded sync.primitives
///   (like Locks, Mutex, etc)
///

#include <corelib/ncbistl.hpp>
#include <corelib/ncbimtx.hpp>

BEGIN_NCBI_SCOPE

/// Default GUI RW lock implementation uses RW-Lock from core library
///
typedef CRWLock  CGuiRWLock;



END_NCBI_SCOPE


#endif // GUI_CORE___LOCKS__HPP
