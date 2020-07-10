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

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/sparse_graph.hpp>
#include <gui/widgets/seq_graphic/graph_cache.hpp>
#include <gui/utils/url_utils.hpp>
#include <util/bitset/bmserial.h>
#include <util/bitset/bmsparsevec_serial.h>
#include <util/checksum.hpp>
#include <corelib/rwstream.hpp>
#include <connect/ncbi_conn_stream.hpp>
#include <future>
#include <memory>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
///   CSparseGraph


CCompoundID CSparseGraph::CreateCompoundID(CScope& scope, const CSeq_id& seq_id, const string& remote_path)
{
    CCompoundIDPool id_pool;
    auto id = id_pool.NewID(eCIC_GenericID);
    auto sih_in = CSeq_id_Handle::GetHandle(seq_id);
    auto sih = scope.GetAccVer(sih_in);
    string seq_id_str = (sih) ? sih.GetSeqId()->GetSeqIdString(true) : seq_id.GetSeqIdString(true);

    id.AppendSeqID(seq_id_str);
    size_t seq_len = scope.GetSequenceLength(sih_in);
    _ASSERT(seq_len);
    id.AppendInteger(seq_len);
    id.AppendHost(remote_path);
    return id;
}

void CSparseGraph::InitFromCompoundID(CCompoundID id)
{
    m_SeqId = id.GetFirst(eCIT_SeqID).GetSeqID();
    if (m_SeqId.empty()) NCBI_THROW(CException, eUnknown, "Empty seq_id");

    m_RemotePath = id.GetFirst(eCIT_Host).GetHost();
    if (m_RemotePath.empty()) NCBI_THROW(CException, eUnknown, "Empty remote path");
    
    m_DataSize = id.GetFirst(eCIT_Integer).GetInteger();
    if (m_DataSize == 0) NCBI_THROW(CException, eUnknown, "Empty seq_len");
    
    //    m_LastModified = id.GetFirst(eCIT_String).GetString();
    //    if (m_LastModified.empty()) NCBI_THROW(CException, eUnknown, "Empty last modified");

}

static
void s_GenerateSessionTag(string& session_tag)
{
    CCompoundIDPool id_pool;
    auto id = id_pool.NewID(eCIC_GenericID);
    // GUID to establish blob ownership
    // 'our' blobs don't need to be synchronized
    // thus a trip to NetCache can be avoided
    // GUID: pid + curent_time + rand()
    
    id.AppendID(CCurrentProcess::GetPid());
    id.AppendCurrentTime();
    id.AppendRandom();
    session_tag  = id.ToString();
}

CSparseGraph::CSparseGraph()
{
    s_GenerateSessionTag(m_SessionTag);
    m_LastSynced = CTime(CTime::eCurrent).AddHour(-1);
}


CSparseGraph::CSparseGraph(const CSparseGraph& data)
{
    m_SessionTag = data.m_SessionTag;
    m_NetCacheKey = data.m_NetCacheKey;
    _ASSERT(!m_NetCacheKey.empty());
    if (m_NetCacheKey.empty())
        throw runtime_error("Empty data key!");
    m_SeqId = data.m_SeqId;
    m_DataSize = data.m_DataSize;
    m_DataMap = data.m_DataMap; 
    // copy data
    for (auto& old_data : data.m_Data) {
        m_Data.emplace(old_data.first, unique_ptr<TSparseVector>(new TSparseVector(*old_data.second)));
    }
    m_RemotePath = data.m_RemotePath;
    m_LastModified = data.m_LastModified;
    m_LastChecked = data.m_LastChecked;
    m_LastSynced = CTime(CTime::eCurrent).AddHour(-1);
    m_IsCacheGood = true;
}

void CSparseGraph::GetMissingRegions(const TSeqRange& range, vector<TSeqRange>& missing_ranges)
{
    missing_ranges.clear();
    unique_lock<mutex> guard(m_DataMutex);
    size_t num_bits = m_DataMap.count_range(range.GetFrom(), range.GetTo());
    if (num_bits >= range.GetLength()) {
        //ERR_POST(Info << "All cached!");
        // All cached
        return;
    }
    // the region is not cached yet, need to calc a new range
    if (num_bits == 0) {
        // Nothing cached 
        //ERR_POST(Info << "Nothing cached!");
        missing_ranges.emplace_back(range.GetFrom(), range.GetTo());
        return;
    }
    //ERR_POST(Info << "Partially cached!");
    bm::bvector<> mask;
    mask.set_range(range.GetFrom(), range.GetTo());
    mask &= m_DataMap;
    guard.unlock();

    mask.invert();
    if (range.GetFrom() > 0)
        mask.set_range(0, range.GetFrom() - 1, false);
    if (range.GetToOpen() < m_DataSize - 1)
        mask.set_range(range.GetToOpen(), m_DataSize - 1, false);

    auto first = mask.get_first();
    auto stop = range.GetToOpen();
    vector<TSeqRange> mr;
    while (first < stop) {
        mask.invert();
        // searching the end of the missing range
        auto last = mask.get_next(first);
        if (last == 0 || last >= stop - 1) {
            mr.emplace_back(first, stop - 1);
            break;
        }
        mr.emplace_back(first, last - 1);
        mask.invert();
        // searching the start of the next missing range
        first = mask.get_next(last);
        if (first == 0)
            break;
    }
    if (!mr.empty()) {
        missing_ranges.push_back(mr.front());
        // merge neighboring ranges
        for (size_t i = 1; i < mr.size(); ++i) {
            if (mr[i].GetFrom() - missing_ranges.back().GetFrom() < 20000) {
                missing_ranges.back().SetTo(mr[i].GetTo());
            } else {
                missing_ranges.emplace_back(mr[i].GetFrom(), mr[i].GetTo());
            }
        }
    }
}
static
void s_GetMD5Tag(bm::bvector<>& bv, string& tag, bm::word_t* tb = 0)
{
    tag.clear();
    unique_ptr<bm::bvector<>::statistics> st_op(new bm::bvector<>::statistics);
    bv.optimize(tb, bm::bvector<>::opt_compress, &*st_op);
    struct bm::bvector<>::statistics st;
    bv.calc_stat(&st);
    unique_ptr<unsigned char> smem(new unsigned char[st.max_serialize_mem]);
    unsigned slen = bm::serialize(bv, smem.get(), tb);

    CChecksum cs(CChecksum::eMD5);
    cs.AddChars((const char*)smem.get(), slen);
    cs.GetMD5Digest(tag);
}


void CSparseGraph::x_Sync(TGraphCachePool& icache_pool)
{

    CObjPoolGuard<TGraphCachePool> icache1(icache_pool);
    
    // is there any data stored?
    bool has_blob = false;
    try {
        has_blob = icache1->GetSize(m_NetCacheKey, 0, NcbiEmptyString) >0;
    } catch(const exception& ) {
    }
    if (!has_blob) {
        x_CheckRemoteData();
        return;
    }

    // check stored data for consistency 
    // read and merge
    BM_DECLARE_TEMP_BLOCK(tb);
    string my_MD5;
    
    bool synced = false;
    unsigned short attempt = 1;
    while (attempt <= 3) {
        try {
            string tag;
            string remote_path;
            string last_modified;
            int last_checked = 0;
            bm::bvector<> data_map;
            string md5;
            x_ReadMap(*icache1, tag, remote_path, last_modified, last_checked, md5, data_map, tb);
            bool must_sync = tag != m_SessionTag;
            
            if (must_sync) {
                // if session tags are different
                // check if current md5 is different from cached md5
                // if they are the same no need for syncing
                if (my_MD5.empty()) { // it can be not empty if the first attempt to sync failed
                    lock_guard<mutex> guard(m_DataMutex);
                    s_GetMD5Tag(m_DataMap, my_MD5, tb);
                }
                must_sync = (my_MD5 != md5);
            } 
            
            if (must_sync == false) {
                // no syncing required
                // oour memory blob is current
                lock_guard<mutex> guard(m_DataMutex);
                m_LastSynced.SetCurrent();
                if (m_LastModified.empty()) {
                    m_LastModified = last_modified;
                    m_LastChecked  = max(last_checked, m_LastChecked);
                }
                break;
            }
            map<string, unique_ptr<TSparseVector>> temp_vectors;
            bool all_is_good = false;
            if (m_Data.size() > 1) {
                vector<future<bool>> res;
                for (auto& data : m_Data) {
                    temp_vectors.emplace(data.first, unique_ptr<TSparseVector>(new TSparseVector));
                    temp_vectors[data.first]->resize(data.second->size());
                    //CRef<CRequestContext> context{ CDiagContext::GetRequestContext().Clone() };
                    
                    res.emplace_back(async(launch::async, [&]()->bool {
                                //          if (context)
                                //            CDiagContext::SetRequestContext(context);
                                string data_key = m_NetCacheKey + "_" + data.first;
                                CObjPoolGuard<TGraphCachePool> icache(icache_pool);
                                return x_ReadData(*icache, data_key, tag, *temp_vectors[data.first]); }));
                }
                
                all_is_good = all_of(res.begin(), res.end(), [](future<bool>& f) { return f.get(); });
            } else {
                const auto& data = *m_Data.begin();
                temp_vectors.emplace(data.first, unique_ptr<TSparseVector>(new TSparseVector));
                temp_vectors[data.first]->resize(data.second->size());
                string data_key = m_NetCacheKey + "_" + data.first;
                CObjPoolGuard<TGraphCachePool> icache(icache_pool);
                all_is_good = x_ReadData(*icache, data_key, tag, *temp_vectors[data.first]);;
            }
            if (all_is_good) {
                lock_guard<mutex> guard(m_DataMutex);
                if (!last_modified.empty()) {
                    m_LastModified = last_modified;
                    m_LastChecked  = max(last_checked, m_LastChecked);
                } 
                m_DataMap |= data_map; 
                m_LastSynced.SetCurrent();
                for (auto&& data : m_Data) {
                    data.second->join(*temp_vectors[data.first]);
                }
                synced = true;
                break;
            }
        } catch (exception& e) {
            ERR_POST(Error << e.what());
        }
        chrono::milliseconds timespan(attempt * 50 + rand() % 100); 
        this_thread::sleep_for(timespan);
        ++attempt;
    }
    if (attempt > 1) {
        auto diag = GetDiagContext().Extra();
        if (synced) {
            diag.Print("Sync_passed_attempts", attempt - 1);
        } else {
            diag.Print("Sync_failed_attempts", attempt - 1);
        }
    }

    x_CheckRemoteData();
    if (!m_IsCacheGood) {
        CObjPoolGuard<TGraphCachePool> icache(icache_pool);
        icache->Remove(m_NetCacheKey, 0, NcbiEmptyString);
        for (auto&& data : m_Data) {
            string data_key = m_NetCacheKey + "_" + data.first;
            icache->Remove(data_key, 0, NcbiEmptyString);
        }
    }
} 

void CSparseGraph::Init(TGraphCachePool& icache_pool)
{

    double diff_in_millisec = CTime(CTime::eCurrent).DiffNanoSecond(m_LastSynced) * 1e-6;
    if (diff_in_millisec > 250) 
        x_Sync(icache_pool);
}

void CSparseGraph::Update(const TSeqRange& range, const TUpdateMap& update)
{
    lock_guard<mutex> guard(m_DataMutex);
    _ASSERT(update.size() == m_Data.size());
    if (update.size() != m_Data.size()) 
        NCBI_THROW(CException, eInvalid, "AlignmentGraph  update failed. Incorrect data.");
    for (auto& data : update) {
        if (m_Data.count(data.first) == 0) 
            NCBI_THROW(CException, eInvalid, "AlignmentGraph  update failed. Incorrect data tag.");
        m_Data[data.first]->import(&(*data.second)[0], range.GetLength(), range.GetFrom());
    }
    m_DataMap.set_range(range.GetFrom(), range.GetTo());
}

void CSparseGraph::Save(TGraphCachePool& icache_pool)
{

    CStopWatch sw(CStopWatch::eStart);
    x_Sync(icache_pool);
    
    CObjPoolGuard<TGraphCachePool> icache(icache_pool);
    for (auto&& data : m_Data) {
        string data_key = m_NetCacheKey + "_" + data.first;
        x_WriteData(*icache, data_key, *data.second);
    }
    BM_DECLARE_TEMP_BLOCK(tb);
    x_WriteMap(*icache, tb);

    auto elapsed = sw.Elapsed();
    if (elapsed > 2000) {
        auto diag = GetDiagContext().Extra();
        diag.Print("icache_save_delay", elapsed);
    }
} 


void CSparseGraph::x_CheckRemoteData()
{
    static const int kRemoteDataCheckInterval = 600; // (Seconds) How often  to check remote data
    bool is_http = NStr::StartsWith(m_RemotePath, "http:", NStr::eNocase) || NStr::StartsWith(m_RemotePath, "https:", NStr::eNocase);
    if (!is_http)
        return;
    lock_guard<mutex> guard(m_DataMutex);
    m_IsCacheGood = true;
    CTime t(CTime::eCurrent);
    if (m_LastChecked == 0) {
        CUrlUtils::GetLastModified(m_RemotePath, m_LastModified);
        if (!m_LastModified.empty()) 
            m_LastChecked = t.GetTimeT();
        
    } else if ((t.GetTimeT() - m_LastChecked) > kRemoteDataCheckInterval) { 
        if (CUrlUtils::IfModifiedSince(m_RemotePath, m_LastModified)) {
            m_IsCacheGood = false;
            m_DataMap.clear();
            for (auto&& data : m_Data) {
                data.second->set(0, data.second->size());
            }
            auto diag = GetDiagContext().Extra();
            diag.Print("Remote_data_updated", m_RemotePath);
        }
        m_LastChecked = t.GetTimeT();
    }
}

void CSparseGraph::x_WriteMap(ICache& icache, bm::word_t* tb)
{
    unique_ptr<bm::bvector<>::statistics> st_op(new bm::bvector<>::statistics);
    m_DataMap.optimize(tb, bm::bvector<>::opt_compress, &*st_op);
    struct bm::bvector<>::statistics st;
    m_DataMap.calc_stat(&st);
    unique_ptr<unsigned char> smem(new unsigned char[st.max_serialize_mem]);
    unsigned slen = bm::serialize(m_DataMap, smem.get(), tb);
    CWStream w(icache.GetWriteStream(m_NetCacheKey, 0, NcbiEmptyString), 0, 0, CRWStreambuf::fOwnWriter);
    int val = m_SessionTag.size();
    w.write((const char*)&val, sizeof(int));
    if (!w) NCBI_THROW(CException, eUnknown, "Failed to write Session tag size: " + m_NetCacheKey);
    w.write(m_SessionTag.c_str(), m_SessionTag.size());
    if (!w) NCBI_THROW(CException, eUnknown, "Failed to write session tag: " + m_NetCacheKey);
    val = m_RemotePath.size();
    w.write((const char*)&val, sizeof(int));
    if (!w) NCBI_THROW(CException, eUnknown, "Failed to write remote path length: " + m_NetCacheKey);
    w.write((const char*)m_RemotePath.c_str(), m_RemotePath.size());
    if (!w) NCBI_THROW(CException, eUnknown, "Failed to write remote path length: " + m_NetCacheKey);
    val = m_LastModified.size();
    w.write((const char*)&val, sizeof(int));
    if (!w) NCBI_THROW(CException, eUnknown, "Failed to write last_modified: " + m_NetCacheKey);
    w.write((const char*)m_LastModified.c_str(), val);
    if (!w) NCBI_THROW(CException, eUnknown, "Failed to write last_modified: " + m_NetCacheKey);
    w.write((const char*)&m_LastChecked, sizeof(int));
    if (!w) NCBI_THROW(CException, eUnknown, "Failed to write last_checked: " + m_NetCacheKey);
    w.write((const char*)smem.get(), slen);
    if (!w) NCBI_THROW(CException, eUnknown, "Failed to write vector: " + m_NetCacheKey);
    w.flush();

}

void CSparseGraph::x_WriteData(ICache& icache, const string& data_key, TSparseVector& sv)
{
    sv.optimize();
    bm::sparse_vector_serial_layout<TSparseVector> sv_lay;
    bm::sparse_vector_serialize(sv, sv_lay);
    CWStream w(icache.GetWriteStream(data_key, 0, NcbiEmptyString), 0, 0, CRWStreambuf::fOwnWriter);
    int val = m_SessionTag.size();
    w.write((const char*)&val, sizeof(int));
    if (!w) NCBI_THROW(CException, eUnknown, "Failed to write session tag size: " + data_key);
    w.write(m_SessionTag.c_str(), m_SessionTag.size());
    if (!w) NCBI_THROW(CException, eUnknown, "Failed to write session tag: " + data_key);
    w.write((const char*)sv_lay.buf(), sv_lay.size());
    if (!w) NCBI_THROW(CException, eUnknown, "Failed to write data: " + data_key);
    w.flush();
}

void CSparseGraph::x_ReadMap(ICache& icache, string& tag, string& remote_path, string& last_modified, int& last_checked, string& md5, bm::bvector<>& bv, bm::word_t* tb)
{
    tag.clear();

    size_t size = icache.GetSize(m_NetCacheKey, 0, NcbiEmptyString);
    size_t vector_offset = 0;
    CRStream is(icache.GetReadStream(m_NetCacheKey, 0, NcbiEmptyString), 0, 0, CRWStreambuf::fOwnReader);
    int val = 0;
    {
        is.read((char*)&val, sizeof(int));
        if (!is) NCBI_THROW(CException, eUnknown, "Failed to read tag lengh: " + m_NetCacheKey);
        vector_offset += sizeof(int);
        vector<char> buffer(val, 0);
        is.read(&buffer[0], buffer.size());
        if (!is) NCBI_THROW(CException, eUnknown, "Failed to read tag: " + m_NetCacheKey);
        tag.append(&buffer[0], buffer.size());
        vector_offset += buffer.size();
    }
    is.read((char*)&val, sizeof(int));
    if (!is) NCBI_THROW(CException, eUnknown, "Failed to read remote path length: " + m_NetCacheKey);
    vector_offset += sizeof(int);
    remote_path.clear();
    if (val > 0) {
        vector_offset += val;
        vector<char> buffer(val, 0);
        is.read(&buffer[0], buffer.size());
        if (!is) NCBI_THROW(CException, eUnknown, "Failed to read remote path: " + m_NetCacheKey);
        remote_path.append(&buffer[0], val);
        val = 0;
    }
    is.read((char*)&val, sizeof(int));
    if (!is) NCBI_THROW(CException, eUnknown, "Failed to read last_modified length: " + m_NetCacheKey);

    vector_offset += sizeof(int);
    last_modified.clear();
    if (val > 0) {
        vector_offset += val;
        vector<char> buffer(val, 0);
        is.read(&buffer[0], buffer.size());
        if (!is) NCBI_THROW(CException, eUnknown, "Failed to read last modified: " + m_NetCacheKey);
        last_modified.append(&buffer[0], val);
        val = 0;
    }

    is.read((char*)&val, sizeof(int));
    if (!is) NCBI_THROW(CException, eUnknown, "Failed to read last_checked: " + m_NetCacheKey);
    last_checked = val;
    vector_offset += sizeof(int);

    vector<char> buffer(size - vector_offset, 0);
    is.read(&buffer[0], buffer.size());
    if (!is) NCBI_THROW(CException, eUnknown, "Failed to read vector: " + m_NetCacheKey);
    bv.clear();
    {
        CChecksum cs(CChecksum::eMD5);
        cs.AddChars((const char*)&buffer[0], buffer.size());
        cs.GetMD5Digest(md5);
    }
    bm::deserialize(bv, (const unsigned char*)&buffer[0], tb);
}


bool CSparseGraph::x_ReadData(ICache& icache, const string& data_key, const string& tag, TSparseVector& sv, bm::word_t* tb)
{
    size_t size = 0;
    try {
        size = icache.GetSize(data_key, 0, NcbiEmptyString);
    } catch (exception& e) {
        ERR_POST(Error << e.what());
        return false;
    }
    CRStream is(icache.GetReadStream(data_key, 0, NcbiEmptyString), 0, 0, CRWStreambuf::fOwnReader);
    int val = 0;
    int vector_offset = 0;
    is.read((char*)&val, sizeof(int));
    if (!is) NCBI_THROW(CException, eUnknown, "Failed to read tag lengh: " + data_key);
    vector_offset += sizeof(int);
    vector<char> buffer(val, 0);
    is.read(&buffer[0], buffer.size());
    if (!is) NCBI_THROW(CException, eUnknown, "Failed to read tag: " + data_key);
    string my_tag(&buffer[0], buffer.size());
    if (my_tag != tag)
        return false;
    vector_offset += buffer.size();
    buffer.resize(size - vector_offset);
    is.read(&buffer[0], buffer.size());
    if (!is) NCBI_THROW(CException, eUnknown, "Failed to read vector: " + data_key);
    auto res = sparse_vector_deserialize(sv, (const unsigned char*)&buffer[0], tb);
    if (res != 0) NCBI_THROW(CException, eUnknown, "Deserialization problem");
    return true;

}

///////////////////////////////////////////////////////////////////////////////
///   CPileUpGraph


string CPileUpGraph::CreateCacheKey(CScope& scope, const CSeq_id& seq_id, const string& remote_path)
{
    auto id = CSparseGraph::CreateCompoundID(scope, seq_id, remote_path);
    return id.ToString();
}

CPileUpGraph::CPileUpGraph(const string& cache_key)
{
    _ASSERT(!cache_key.empty());
    if (cache_key.empty())
        NCBI_THROW(CException, eUnknown, "Empty cache key");
    CCompoundIDPool id_pool;
    auto cid = id_pool.FromString(cache_key);
    InitFromCompoundID(cid);

    m_NetCacheKey = "GR_";
    m_NetCacheKey += m_SeqId;
    m_NetCacheKey += "_";

    bool is_http = NStr::StartsWith(m_RemotePath, "http:", NStr::eNocase) || NStr::StartsWith(m_RemotePath, "https:", NStr::eNocase);
    if (is_http) {
        CChecksum cs(CChecksum::eMD5);
        cs.AddLine(m_RemotePath);
        m_NetCacheKey += cs.GetHexSum();
    } else {
        m_NetCacheKey += m_RemotePath;
    }
    m_LastSynced = CTime(CTime::eCurrent).AddHour(-1);

    _ASSERT(m_DataSize > 0);
    m_Data.emplace("m", unique_ptr<TSparseVector>(new TSparseVector));
    m_Data["m"]->resize(m_DataSize);
    m_Data.emplace("mm", unique_ptr<TSparseVector>(new TSparseVector));
    m_Data["mm"]->resize(m_DataSize);
    m_Data.emplace("g", unique_ptr<TSparseVector>(new TSparseVector));
    m_Data["g"]->resize(m_DataSize);
    m_Data.emplace("n", unique_ptr<TSparseVector>(new TSparseVector));
    m_Data["n"]->resize(m_DataSize);
}


void CPileUpGraph::UpdateAlignStats(const TSeqRange& range, float window, CAlnStatGlyph::TStatVec& stats)
{
    if (window < 1.0) window = 1.0;
    size_t num_pixels = (size_t)ceil(range.GetLength() / window);
    stats.resize(num_pixels);
    
    auto start = range.GetFrom();
    auto stop = range.GetToOpen();
    
    int chunk_size = min<int>(range.GetLength(), 1048576); // 1MB chunks
    vector<TValType> matches(chunk_size, 0);
    vector<TValType> mismatches(chunk_size, 0);
    vector<TValType> gaps(chunk_size, 0);
    vector<TValType> introns(chunk_size, 0);
    {
        lock_guard<mutex> guard(m_DataMutex);
        m_Data["m"]->decode(&matches[0], start, chunk_size);
        m_Data["mm"]->decode(&mismatches[0], start, chunk_size);
        m_Data["g"]->decode(&gaps[0], start, chunk_size);
        m_Data["n"]->decode(&introns[0], start, chunk_size);
    }
    
    auto offset = start;
    auto pos = start;
    int stat_idx = 0;
    double curr_pos = start;
    while (pos < stop && stat_idx < num_pixels) {
        curr_pos += window;
        size_t end_idx = (size_t)floor(curr_pos + 0.5);
        if (end_idx > stop)
            end_idx = stop;
        auto&& data = stats[stat_idx].m_Data;
        int max_mismatch = -1;
        int max_gap = -1;
        int max_intron = -1;
        int total = -1;
        while (pos < end_idx) {
            int index = pos - offset;
            if (index >= chunk_size) {
                {
                    chunk_size = min<int>(chunk_size, range.GetToOpen() - pos);
                    lock_guard<mutex> guard(m_DataMutex);
                    m_Data["m"]->decode(&matches[0], pos, chunk_size);
                    m_Data["mm"]->decode(&mismatches[0], pos, chunk_size);
                    m_Data["g"]->decode(&gaps[0], pos, chunk_size);
                    m_Data["n"]->decode(&introns[0], pos, chunk_size);
                }
                offset = pos;
                index = 0;
            }
            int mismatch = mismatches[index];
            int match = matches[index];
            int gap = gaps[index];
            int intron = introns[index];
            total = max<int>(total, match + mismatch + gap + intron);
            max_mismatch = max<int>(max_mismatch, mismatch);
            max_gap = max<int>(max_gap, gap);
            max_intron = max<int>(max_intron, intron);
            ++pos;
        }
        // total is total number of rows in the given range
        // we take the number of matches as total - max error
        // where error is mismatch, gap or intron
        int max_error = max(max_gap, max(max_mismatch, max_intron));
        data[CAlnStatConfig::eStat_Match] = total - max_error;
        data[CAlnStatConfig::eStat_Mismatch] = max_mismatch;
        data[CAlnStatConfig::eStat_Gap] = max_gap;
        data[CAlnStatConfig::eStat_Intron] = max_intron;
        data[CAlnStatConfig::eStat_Total] = total;

        ++stat_idx;
    }
}


int CPileUpGraph::GetNumberOfReads(const TSeqRange& range)
{
    {
        lock_guard<mutex> guard(m_DataMutex);
        size_t num_bits = m_DataMap.count_range(range.GetFrom(), range.GetTo());
        if (num_bits < range.GetLength())
            return -1;
    }

    vector<TValType> matches;
    vector<TValType> mismatches;
    vector<TValType> gaps;
    vector<TValType> introns;
    int total = 0;
    auto from = range.GetFrom();
    auto to = range.GetTo();
    static const int kChunkSize = 1048576;
    while (from < to) {
        // read 1MB chunks
        TSeqRange r(from, min<int>(from + kChunkSize, to));
        matches.resize(r.GetLength());
        mismatches.resize(r.GetLength());
        gaps.resize(r.GetLength());
        introns.resize(r.GetLength());
        m_Data["m"]->decode(&matches[0], r.GetFrom(), r.GetLength());
        m_Data["mm"]->decode(&mismatches[0], r.GetFrom(), r.GetLength());
        m_Data["g"]->decode(&gaps[0], r.GetFrom(), r.GetLength());
        m_Data["n"]->decode(&introns[0], r.GetFrom(), r.GetLength());
        
        int sub_total = 0;
        for (size_t i = 0; i < r.GetLength(); ++i) {
            if (i % 150 == 0) {
                total += sub_total;
                sub_total = 0;
            }
            sub_total = max<int>(sub_total, (matches[i] + mismatches[i] + gaps[i] + introns[i]));
        }
        total += sub_total;
        from = r.GetToOpen();
    }
    return total;
}

END_NCBI_SCOPE
