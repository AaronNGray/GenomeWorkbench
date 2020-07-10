#ifndef GUI_WIDGETS_SEQ_GRAPHIC___WIG_GRAPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___WIG_GRAPH__HPP

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
#include <gui/widgets/seq_graphic/sparse_graph.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/graph_ci.hpp>
#include <util/bitset/ncbi_bitset.hpp>
#include <util/bitset/bmdef.h>
#include <util/bitset/bmserial.h>
#include <util/bitset/bmsparsevec.h>
#include <mutex>
#include <future>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CSeq_id;
    class CScope;
END_SCOPE(objects)


///////////////////////////////////////////////////////////////////////////////
///   CWigGraph

class CWigGraph : public CSparseGraph
{
 public:

    static string GetStorageName() {
        return "WigGraphCache";
    }

    static constexpr float kFract = 1000.;

    ///< CWigGraph data come in two flavors
    ///< Summary - low-res representation for the whole sequence length
    ///< and raw data - hi-res sparse cache
    enum EDataType {
        eSummary,
        eData
    };

    ///< Creates a cache key for the given remote file and seq_id
    ///< the key value is CCompoundID
    ///< that parsed by constructor when the class is instantiated
    static string GetCacheKey(objects::CScope& scope,
                              const objects::CSeq_id& seq_id,
                              const string& wig_id,
                              const string& remote_path,
                              const string& last_modified,
                              EDataType data_type);

    CWigGraph(const string& cache_key);

    ///< Copy constructor is invoked by CGraphCache
    ///< which clones the data for aynchroneous saving operation
    CWigGraph(const CWigGraph& data);
    
    virtual ~CWigGraph();

    ///<  Init is invoked by CGraphCache
    ///<  when a new instance of CWigGraph is initialized
    virtual void Init(TGraphCachePool& icache_pool) override;

    ///<  Save is invoked by CGraphCache
    ///<  when cached data needs to be permanently saved into NetCache
    virtual void Save(TGraphCachePool& icache_pool) override;

    ///< GetData serializes the data into CDensityMap
    ///< for the range provided in @data
    void GetData(CHistogramGlyph::TMap& data);

    ///< GetDataCoverage returns fraction (0. - 1f) of retrieved data 
    ///< in the given range 
    float GetDataCoverage(const TSeqRange& range);
    
    bool IsSummary() const;

    void Prefetch(const TSeqRange& range);

protected:
    void x_CreateNetCacheKey();
    void x_SaveSummary(ICache& icache, TSparseVector& sv);
    bool x_ReadSummary(ICache& icache, TSparseVector& sv);

    int x_RunBigWigScript(const TSeqRange& range, const string tmp_file);
    
    void x_GetBigWigSummary(TSparseVector& summary_sv);
    void x_GetBigWigData(const TSeqRange& range, vector<TValType>& v_out);

    void x_EncodeDelta(const TSeqRange& range, const vector<float>& v_in, vector<TValType>& v_out);
    void x_DecodeDelta(const TSeqRange& range, vector<float>& v_out);

    void x_AddDataToMap(const vector<float>& data, const TSeqPos from, CHistogramGlyph::TMap& the_map);

    /// Float values are packed with precision loss (3 decimal places retained) 
    /// values are conveeted to postive unsigned long long
    /// and the first bit is used to indicated the sign
    void x_PackFloat(float v_in, TSparseVector::value_type& v_out);
    void x_UnPackFloat(TSparseVector::value_type v_in,  float& v_out);

    /// Encode/Decode negtaive vaues to positive ones
    uint64_t x_EncodeSign(int64_t v);
    int64_t x_DecodeSign(uint64_t v);

    string m_WigId;          ///< WigFile id, e.g. chr1
    EDataType m_DataType = eSummary;
    
    static const int kMinBinSize = 500;
    int m_SummaryBinCount = 100000; /// Number od data points in one Summary point

    atomic<bool>       m_PrefetchRequested{false};

    using TPrefetchRequest = struct {
        TSeqRange range;
        vector<TValType> data;
    } ;
    using TPrefetchData = unique_ptr<TPrefetchRequest>;
    map<TSeqRange, future<TPrefetchData>> m_PendingData;

    using TPrefetchSummary = unique_ptr<TSparseVector>;
    future<TPrefetchSummary> m_PendingSummary;
    void x_FetchSummary();
    void x_FetchData();

};


inline
CWigGraph::~CWigGraph()
{
}


/// Float values are packed with precision loss (3 decimal places retained) 
/// values are conveeted to postive unsigned long long
/// and the first bit is used to indicated the sign
inline
void CWigGraph::x_PackFloat(float v_in, TSparseVector::value_type& v_out)
{
    int64_t v = round(v_in * kFract);
    v_out = x_EncodeSign(v);
}

inline
void CWigGraph::x_UnPackFloat(TSparseVector::value_type v_in, float& v_out)
{
    v_out = x_DecodeSign(v_in);
    v_out /= kFract;
}


inline
uint64_t CWigGraph::x_EncodeSign(int64_t v) {
    //return (v >> 63) ^ (v << 1); //zigzag encodeing

    bool is_negative = v < 0;
    uint64_t res = abs(v) << 1;
    if (is_negative)
        res |= 1;
    return res;
}

inline
int64_t CWigGraph::x_DecodeSign(uint64_t v) {
    //return (v >> 1) ^ -(v & 1); //zigzag decoding
    bool negative = v & 1;
    int64_t val = v >> 1;
    if (negative)
        val = -val;
    return val;
}




END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___WIG_GRAPH__HPP
