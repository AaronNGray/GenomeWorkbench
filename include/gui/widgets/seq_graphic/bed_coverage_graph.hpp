#ifndef GUI_WIDGETS_SEQ_GRAPHIC___BED_COVERAGE_GRAPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___BED_COVERAGE_GRAPH__HPP

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
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <util/bitset/ncbi_bitset.hpp>
#include <util/bitset/bmdef.h>
#include <util/bitset/bmserial.h>
#include <util/bitset/bmsparsevec.h>
#include <mutex>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CSeq_id;
    class CScope;
END_SCOPE(objects)



///////////////////////////////////////////////////////////////////////////////
///   CBedCoverageGraph

class CBedCoverageGraph : public CObject
{
 public:
    static string GetStorageName() {
        return "BedGraphCache";
    }

    static const int kMinBinSize = 2500;
    
    static string GetCacheKey(objects::CScope& scope,
                              const objects::CSeq_id& seq_id,
                              const string& bed_id,
                              const string& remote_path,
                              const string& last_modified = NcbiEmptyString);

    CBedCoverageGraph(const string& cache_key);
    CBedCoverageGraph(const CBedCoverageGraph& data);
    
    virtual ~CBedCoverageGraph();

    void Init(TGraphCachePool& icache_pool);
    void Save(TGraphCachePool& icache_pool);

    void GetData(CHistogramGlyph::TMap& data);
    size_t GetEstimatedFeatureCount(const TSeqRange& range, int& max_count);

    bool IsSummary() const;

protected:
    void x_CreateDataKey(string& cache_key);
    void x_InitData(ICache& icache);
    void x_SaveData(ICache& icache);

    string m_SeqId;
    string m_BedId;
    size_t m_SeqLen = 0;
    string m_RemotePath;    ///< path to remote data or srz accession
    string m_LastModified;  ///< timestamp of the remote data last modified date as reported by www server

    mutex m_DataMutex;      ///< data access mutex
 
    string m_DataKey;
    int m_DataBinCount = 100000;
    using TSparseVector = bm::sparse_vector<unsigned, bm::bvector<> >;
    TSparseVector m_Data; ///< map of sprasvectors, the key is used as a part of suffix of NetCache key (m_DataKey + "_" + this map key
};



inline
CBedCoverageGraph::~CBedCoverageGraph()
{
}



END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___BED_COVERAGE_GRAPH__HPP