#ifndef CONNECT_SERVICES__NETSTORAGE_FT__HPP
#define CONNECT_SERVICES__NETSTORAGE_FT__HPP

/*  $Id: netstorage_ft.hpp 493146 2016-02-24 17:38:53Z sadyrovr $
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
 * Authors: Rafael Sadyrov
 *
 * File Description:
 *   A NetStorage API for FileTrack objects.
 *
 */


#include "netstorage.hpp"


BEGIN_NCBI_SCOPE


/// Represents file path of underlying FileTrack object.
/// File path is locked in constructor and held for up to 20 minues (FT default).
///
class NCBI_XCONNECT_EXPORT CNetStorageObject_FileTrack_Path
{
public:
    /// Locks file path if object resides in FileTrack.
    ///
    /// @throw CNetStorageException
    ///     Throws an exception if object does not reside in FileTrack,
    //      or there is an error.
    CNetStorageObject_FileTrack_Path(CNetStorageObject object);

    /// Get file path.
    operator string() const { return m_Path; }

private:
    const string m_Path;
};


END_NCBI_SCOPE


#endif
