#ifndef OBJTOOLS_DATA_LOADERS_CDD___CDD_LOADER_IMPL__HPP
#define OBJTOOLS_DATA_LOADERS_CDD___CDD_LOADER_IMPL__HPP

/*  $Id: cdd_loader_impl.hpp 602734 2020-03-02 15:36:01Z grichenk $
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
 * Author: Aleksey Grichenko
 *
 * File Description: CDD data loader
 *
 * ===========================================================================
 */


#include <corelib/ncbistd.hpp>
#include <corelib/ncbimtx.hpp>
#include <objects/id2/ID2_Blob_Id.hpp>
#include <objmgr/data_loader.hpp>
#include <objtools/data_loaders/cdd/cdd_access/cdd_access__.hpp>
#include <objtools/data_loaders/cdd/cdd_access/cdd_client.hpp>
#include <map>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

class CCDDBlobId : public CBlobId
{
public:
    using TSat = CID2_Blob_Id::TSat;
    using TSubSat = CID2_Blob_Id::TSub_sat;
    using TSatKey = CID2_Blob_Id::TSat_key;
    using TVersion = CID2_Blob_Id::TVersion;

    CCDDBlobId(void) {}

    explicit CCDDBlobId(CTempString str_id);
    CCDDBlobId(const CID2_Blob_Id& blob_id);

    const CID2_Blob_Id& Get(void) const;
    CID2_Blob_Id& Get(void);

    TSat GetSat(void) const { return (m_Id2BlobId && m_Id2BlobId->CanGetSat()) ? m_Id2BlobId->GetSat() : -1; }
    TSubSat GetSubSat(void) const { return (m_Id2BlobId && m_Id2BlobId->CanGetSub_sat()) ? m_Id2BlobId->GetSub_sat() : 0; }
    TSatKey GetSatKey(void) const { return (m_Id2BlobId && m_Id2BlobId->CanGetSat_key()) ? m_Id2BlobId->GetSat_key() : 0; }
    TVersion GetVersion(void) const { return (m_Id2BlobId && m_Id2BlobId->CanGetVersion()) ? m_Id2BlobId->GetVersion() : 0; }

    string ToString(void) const;

    bool operator<(const CBlobId& blob_id) const;
    bool operator==(const CBlobId& blob_id) const;

    bool operator<(const CCDDBlobId& blob_id) const;
    bool operator==(const CCDDBlobId& blob_id) const;
    bool operator!=(const CCDDBlobId& blob_id) const { return !(*this == blob_id); }

    bool IsEmpty(void) const { return GetSat() < 0; }

private:
    mutable CRef<CID2_Blob_Id> m_Id2BlobId;
};


class CCDDBlobCache;

class CCDDDataLoader_Impl : public CObject
{
public:
    CCDDDataLoader_Impl(const CCDDDataLoader::SLoaderParams& params);
    ~CCDDDataLoader_Impl(void);

    typedef CDataLoader::TBlobId TBlobId;
    typedef CDataLoader::TSeq_idSet TSeq_idSet;

    CDataLoader::TTSE_LockSet GetBlobBySeq_ids(const TSeq_idSet& ids, CDataSource& ds);

private:
    typedef multimap<time_t, CRef<CCDDClient> > TClientPool;
    typedef TClientPool::iterator TClient;
    friend class CCDDClientGuard;

    bool x_IsValidId(const CSeq_id& id);
    TClient x_GetClient();
    void x_ReleaseClient(TClient& client);
    void x_DiscardClient(TClient& client);
    bool x_CheckReply(CRef<CCDD_Reply>& reply, int serial, CCDD_Reply::TReply::E_Choice choice);

    static int x_NextSerialNumber(void);

    string              m_ServiceName;
    size_t              m_PoolSoftLimit;
    time_t              m_PoolAgeLimit;
    bool                m_ExcludeNucleotides;
    mutable CFastMutex  m_PoolLock;
    TClientPool         m_InUse;
    TClientPool         m_NotInUse;
    unique_ptr<CCDDBlobCache> m_Cache;
};

END_SCOPE(objects)
END_NCBI_SCOPE

#endif  // OBJTOOLS_DATA_LOADERS_CDD___CDD_LOADER_IMPL__HPP
