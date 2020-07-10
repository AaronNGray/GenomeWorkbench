#ifndef GUI_WIDGETS_SEQ_GRAPHIC___VCF_HEATMAP__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___VCF_HEATMAP__HPP

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
///   CVcfHeatmap

class CVcfHeatmap : public CObject
{
 public:
    static string GetStorageName() {
        return "VcfHeatmapCache";
    }

    ///< Creates a cache key for the given remote file and seq_id
    ///< the key value is CCompoundID
    ///< that parsed by constructor when the class is instantiated
    static string GetCacheKey(objects::CScope& scope,
                              const objects::CSeq_id& seq_id,
                              const string& bed_id,
                              const string& remote_path,
                              const string& last_modified = NcbiEmptyString);

    CVcfHeatmap(const string& cache_key);

    ///< Copy constructor is invoked by CGraphCache
    ///< which clones the data for aynchroneous saving operation
    CVcfHeatmap(const CVcfHeatmap& data);
    
    virtual ~CVcfHeatmap();

    ///<  Init is invoked by CGraphCache
    ///<  when a new instance of CWigGraph is initialized
    void Init(TGraphCachePool& icache_pool);

    ///<  Save is invoked by CGraphCache
    ///<  when cached data needs to be permanently saved into NetCache
    void Save(TGraphCachePool& icache_pool);

    void GetData(const TSeqRange& range, vector<TSeqRange>& data);

protected:
    void x_CreateDataKey(string& cache_key);
    void x_InitData(ICache& icache);
    void x_SaveData(ICache& icache);

    string m_SeqId;
    string m_RemoteId;
    size_t m_SeqLen = 0;
    string m_RemotePath;    ///< path to remote data or srz accession
    string m_LastModified;  ///< timestamp of the remote data last modified date as reported by www server

    mutex m_DataMutex;      ///< data access mutex

    string m_DataKey;
    vector<TSeqRange> m_Data;

};



inline
CVcfHeatmap::~CVcfHeatmap()
{
}



END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___VCF_HEATMAP__HPP
