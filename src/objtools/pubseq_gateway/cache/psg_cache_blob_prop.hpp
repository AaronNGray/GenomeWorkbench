#ifndef PSG_CACHE_BLOB_PROP__HPP
#define PSG_CACHE_BLOB_PROP__HPP

/*  $Id: psg_cache_blob_prop.hpp 597490 2019-11-21 21:21:59Z saprykin $
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
 * Authors: Dmitri Dmitrienko
 *
 * File Description: blob_prop table cache
 *
 */

#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <corelib/ncbistl.hpp>

#include <objtools/pubseq_gateway/impl/cassandra/request.hpp>
#include <objtools/pubseq_gateway/impl/cassandra/blob_record.hpp>

#include "psg_cache_base.hpp"

BEGIN_IDBLOB_SCOPE

class CPubseqGatewayCacheBlobProp
    : public CPubseqGatewayCacheBase
{
 public:
    explicit CPubseqGatewayCacheBlobProp(const string& file_name);
    virtual ~CPubseqGatewayCacheBlobProp() override;
    void Open(const set<int>& sat_ids);

    vector<CBlobRecord> Fetch(CBlobFetchRequest const& request);

    static string PackKey(int32_t sat_key);
    static string PackKey(int32_t sat_key, int64_t last_modified);
    static bool UnpackKey(const char* key, size_t key_sz, int64_t& last_modified);
    static bool UnpackKey(const char* key, size_t key_sz, int64_t& last_modified, int32_t& sat_key);

 private:
    bool x_ExtractRecord(CBlobRecord& record, lmdb::val const& value) const;
    vector<unique_ptr<lmdb::dbi, function<void(lmdb::dbi*)>>> m_Dbis;
};

END_IDBLOB_SCOPE

#endif  // PSG_CACHE_BLOB_PROP__HPP
