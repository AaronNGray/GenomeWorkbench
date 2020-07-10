#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SPARSE_GRAPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SPARSE_GRAPH__HPP

/*  
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
 * Authors:  Andrei Shkeda
 *
 * File Description:
 *
 */
#include <gui/widgets/seq_graphic/graph_cache.hpp>
#include <gui/widgets/seq_graphic/align_statistics_glyph.hpp>
#include <util/bitset/ncbi_bitset.hpp>
#include <util/bitset/bmdef.h>
#include <util/bitset/bmserial.h>
#include <util/bitset/bmsparsevec.h>
#include <connect/services/compound_id.hpp>
#include <mutex>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CSeq_id;
    class CScope;
END_SCOPE(objects)


///////////////////////////////////////////////////////////////////////////////
///   CSparseGraph

class CSparseGraph : public CObject
{
 public:
    using TValType = uint64_t;
    CSparseGraph();
    // Copy constructor
    CSparseGraph(const CSparseGraph& data);
    virtual ~CSparseGraph();

    static CCompoundID CreateCompoundID(objects::CScope& scope, const objects::CSeq_id& seq_id, const string& remote_path);
    void InitFromCompoundID(CCompoundID id);

    virtual void Init(TGraphCachePool& icache_pool);
    virtual void Save(TGraphCachePool& icache_pool);
    bool IsCacheGood() const;
    
    typedef map<string, vector<TValType>*> TUpdateMap;
    void Update(const TSeqRange& range, const TUpdateMap& update);
    void GetMissingRegions(const TSeqRange& range, vector<TSeqRange>& missing_ranges);
    
protected:
    typedef bm::sparse_vector<TValType, bm::bvector<> >  TSparseVector;
    void x_WriteMap(ICache& icache, bm::word_t* tb = 0);
    void x_WriteData(ICache& icache, const string& data_key, TSparseVector& sv);
    void x_ReadMap(ICache& icache, string& tag, string& remote_path, string& last_modified, int& last_checked, string& md5, bm::bvector<>& bv, bm::word_t* tb = 0);
    bool x_ReadData(ICache& icache, const string& data_key, const string& tag, TSparseVector& sv, bm::word_t* tb = 0);
    void x_CheckRemoteData();
    void x_Sync(TGraphCachePool& icache_pool);

    string m_SessionTag;      ///< A current session tag, used to separate session netcache blobs from the blobs saved by other sessions
    string m_NetCacheKey;     ///< data access key 
    bm::bvector<> m_DataMap;  ///< bvector shows the regions m_Data exist
    string m_SeqId;           ///< NCBI seq_id, e.g. NC_000001 
    size_t m_DataSize = 0;    ///< size of each vector of m_Data
    map<string, unique_ptr<TSparseVector>> m_Data; ///< map of sprasvectors, the key is used as a part of suffix of NetCache key (m_NetCacheKey + "_" + this map key
    mutex m_DataMutex;      ///< data access mutex
    string m_RemotePath;    ///< path to remote data or srz accession
    string m_LastModified;  ///< timestamp of the remote data last modified date as reported by www server
    int m_LastChecked = 0;  ///< timestamp: last time when m_LastModified was checked
    bool m_IsCacheGood = true;  ///< false value indicates that remote data were updated and the cache was reset
    CTime m_LastSynced;
};

///////////////////////////////////////////////////////////////////////////////
///   CPileUpGraph

class CPileUpGraph : public CSparseGraph
{
public:
    static string GetStorageName() {
        return "GraphCache";
    }
     // New data key
    static string CreateCacheKey(objects::CScope& scope,
                                 const objects::CSeq_id& seq_id,
                                 const string& remote_path);
    CPileUpGraph(const string& data_key);
    void UpdateAlignStats(const TSeqRange& range, float window, CAlnStatGlyph::TStatVec& stats);
    // guesstimated number of reads based on average 150bp read size 

    int GetNumberOfReads(const TSeqRange& range);
};

///   inline methods
inline
bool CSparseGraph::IsCacheGood() const
{
    return m_IsCacheGood;
}

inline
CSparseGraph::~CSparseGraph()
{}

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___SPARSE_GRAPH__HPP
