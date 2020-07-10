#ifndef MISC_NETSTORAGE__NETSTORAGE__HPP
#define MISC_NETSTORAGE__NETSTORAGE__HPP

/*  $Id: netstorage.hpp 482954 2015-10-27 18:42:13Z sadyrovr $
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
 * Author: Rafael Sadyrov
 *
 * File Description:
 *   A generic API for accessing heterogeneous storage services
 *   (including direct serverless access to the backeend storages).
 *
 */

#include <connect/services/netstorage.hpp>

BEGIN_NCBI_SCOPE

class CCombinedNetStorage : public CNetStorage
{
public:
    explicit CCombinedNetStorage(const string& init_string,
            TNetStorageFlags default_flags = 0);
};

class CCombinedNetStorageByKey : public CNetStorageByKey
{
public:
    explicit CCombinedNetStorageByKey(const string& init_string,
            TNetStorageFlags default_flags = 0);
};

END_NCBI_SCOPE

#include "impl/netstorage_int.hpp"

#endif
