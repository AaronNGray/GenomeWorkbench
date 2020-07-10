#ifndef CONNECT_SERVICES_IMPL__NETICACHE_CLIENT_INT__HPP
#define CONNECT_SERVICES_IMPL__NETICACHE_CLIENT_INT__HPP

/*  $Id: neticache_client_int.hpp 554421 2018-01-03 15:14:01Z sadyrovr $
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
 *   Internal declarations for NetCache ICache client API.
 *
 */


BEGIN_NCBI_SCOPE


struct NCBI_NET_CACHE_EXPORT CNetICacheClientExt : CNetICacheClient
{
    CNetICacheClientExt() : CNetICacheClient(eVoid) {}
    CNetICacheClientExt(TInstance api)
        : CNetICacheClient(api)
    {
        if (api) SetFlags(fBestReliability);
    }

    CNetICacheClientExt& operator=(TInstance api)
    {
        CNetICacheClient::operator=(api);
        if (api) SetFlags(fBestReliability);
        return *this;
    }

    void ProlongBlobLifetime(const string&, const CTimeout&,
            const CNamedParameterList* optional = NULL);

    SNetCacheAPIImpl* GetNetCacheAPI();
    const SNetCacheAPIImpl* GetNetCacheAPI() const;

    CNetICacheClientExt GetServer(CNetServer::TInstance server);
};


END_NCBI_SCOPE


#endif
