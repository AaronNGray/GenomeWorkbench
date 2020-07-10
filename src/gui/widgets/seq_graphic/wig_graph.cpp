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
#include <gui/widgets/seq_graphic/wig_graph.hpp>
#include <gui/widgets/seq_graphic/graph_cache.hpp>
#include <gui/utils/rmtfilestatus.hpp>
#include <gui/utils/url_utils.hpp>

#include <objmgr/scope.hpp>
#include <objects/seq/seq_id_handle.hpp>

#include <util/bitset/bmserial.h>
#include <util/bitset/bmsparsevec_serial.h>
#include <util/bitset/bm.h>
#include <util/bitset/bmalgo_similarity.h>

#include <util/bitset/bmdbg.h>

#include <util/checksum.hpp>
#include <corelib/rwstream.hpp>
#include <connect/ncbi_conn_stream.hpp>
#include <corelib/ncbiexec.hpp>

#include <cmath>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const int kDeltaFrameSize = 65535;

static TSeqPos s_AlignToFrame(TSeqPos pos)
{
    return (pos / kDeltaFrameSize) * kDeltaFrameSize;    
}


///////////////////////////////////////////////////////////////////////////////
///   CWigGraph

string CWigGraph::GetCacheKey(CScope& scope, const CSeq_id& seq_id, const string& wig_id, const string& remote_path, const string& last_modified, EDataType data_type)
{
    auto id = CSparseGraph::CreateCompoundID(scope, seq_id, remote_path);
    CCompoundIDPool id_pool;
    auto nested_id = id_pool.NewID(eCIC_GenericID);
    nested_id.AppendSeqID(wig_id);
    nested_id.AppendInteger(data_type);
    string lmd = last_modified;
    if (lmd.empty()) 
        CUrlUtils::GetLastModified(remote_path, lmd);
    nested_id.AppendString(lmd);
    id.AppendNestedCID(nested_id);
    return id.ToString();

}

CWigGraph::CWigGraph(const string& cache_key)
{

    _ASSERT(!cache_key.empty());
    if (cache_key.empty())
        NCBI_THROW(CException, eUnknown, "Empty cache key");

    CCompoundIDPool id_pool;
    auto cid = id_pool.FromString(cache_key);

    InitFromCompoundID(cid);
    auto nested_cid = cid.GetFirst(eCIT_NestedCID).GetNestedCID();

    m_WigId = nested_cid.GetFirst(eCIT_SeqID).GetSeqID();
    if (m_WigId.empty()) NCBI_THROW(CException, eUnknown, "Empty wig_id");
    auto data_type = nested_cid.GetFirst(eCIT_Integer).GetInteger();

    m_LastModified = nested_cid.GetFirst(eCIT_String).GetString();
    
    m_Data.emplace("0", unique_ptr<TSparseVector>(new TSparseVector));
    switch (data_type) {
    case eSummary:
        m_DataType = eSummary;
        m_SummaryBinCount = m_DataSize/kMinBinSize;
        break;
    case eData:
        m_DataType = eData;
        m_Data["0"]->resize(m_DataSize);
        break;
    default:
        NCBI_THROW(CException, eUnknown, "undefined data type");
    }
    x_CreateNetCacheKey();
}

void CWigGraph::x_CreateNetCacheKey()
{
    CChecksum cs(CChecksum::eMD5);
    cs.AddLine(m_SeqId);
    cs.AddLine(NStr::NumericToString(m_DataSize));
    cs.AddLine(m_WigId);
    cs.AddLine(m_RemotePath);
    cs.AddLine(m_LastModified);
    switch (m_DataType) {
    case eSummary:
        m_NetCacheKey = "WS_";
        break;
    case eData:
        m_NetCacheKey = "WG_";
        break;
    }
    m_NetCacheKey += cs.GetHexSum();
}

CWigGraph::CWigGraph(const CWigGraph& data)
    : CSparseGraph(data)
    , m_WigId(data.m_WigId)
    , m_DataType(data.m_DataType)  
    , m_SummaryBinCount(data.m_SummaryBinCount)
{
}


bool CWigGraph::x_ReadSummary(ICache& icache, TSparseVector& summary)
{
    //   BM_DECLARE_TEMP_BLOCK(tb);
    _ASSERT(m_DataType == eSummary);
    if (m_DataType != eSummary)
        NCBI_THROW(CException, eUnknown, "Invalid read request");

    size_t size = 0;
    try {
        size = icache.GetSize(m_NetCacheKey, 0, NcbiEmptyString);
    } catch (exception& e) {
        ERR_POST(Error << e.what());
        return false;
    }
    
    CRStream is(icache.GetReadStream(m_NetCacheKey, 0, NcbiEmptyString), 0, 0, CRWStreambuf::fOwnReader);

//    if (m_DataType == eSummary) {
        is.read((char*)&m_SummaryBinCount, sizeof(int));
        if (!is) NCBI_THROW(CException, eUnknown, "Failed to read bin count: " + m_NetCacheKey);
        size -= sizeof(int);
    
        int summary_size = 0;
        is.read((char*)&summary_size, sizeof(int));
        if (!is) NCBI_THROW(CException, eUnknown, "Failed to read summary size: " + m_NetCacheKey);
        size -= sizeof(int);
  //  }
    vector<char> buffer(size, 0);
    is.read(&buffer[0], buffer.size());
    if (!is) NCBI_THROW(CException, eUnknown, "Failed to read vector: " + m_NetCacheKey);
    
    auto res = sparse_vector_deserialize(summary/**m_Data["0"]*/, (const unsigned char*)&buffer[0]);
    if (res != 0) NCBI_THROW(CException, eUnknown, "Cache deserialization failed");

    //size -= summary_size;
    return true;
}


void CWigGraph::Init(TGraphCachePool& icache_pool)
{
    if (m_DataType == eSummary) {
        lock_guard<mutex> guard(m_DataMutex);
        if (m_Data["0"]->empty() && m_PrefetchRequested == false) {
            m_PrefetchRequested = true;
            m_PendingSummary = async(std::launch::async, [&]{
                    CObjPoolGuard<TGraphCachePool> icache(icache_pool);
                    TPrefetchSummary s(new TSparseVector);
                    // are there any data stored?
                    bool has_blob = false;
                    try {
                        has_blob = icache->GetSize(m_NetCacheKey, 0, NcbiEmptyString) >0;
                    } catch(exception& e) {
                    }
                    if (has_blob) {
                        try {
                            x_ReadSummary(*icache, *s);
                            return s;
                        } catch (exception& e) {
                            ERR_POST(Error << e.what());
                        }
                    }
                    x_GetBigWigSummary(*s);
                    x_SaveSummary(*icache, *s);
                    return s;
                });          
        }          
/*                
        CObjPoolGuard<TGraphCachePool> icache(icache_pool);
        
        // are there any data stored?
        bool has_blob = false;
        try {
            has_blob = icache->GetSize(m_NetCacheKey, 0, NcbiEmptyString) >0;
        } catch(exception& e) {
        }
        if (has_blob) {
            try {
                x_ReadData(*icache);
                return;
            } catch (exception& e) {
                ERR_POST(Error << e.what());
            }
        }
        x_GetBigWigSummary();
        x_SaveData(*icache);
*/        
    } else {
        // since last_modified is a part of wig graph key
        // we can skip remote data check by updating m_LastChecked
        m_LastChecked = CTime(CTime::eCurrent).GetTimeT(); 
        CSparseGraph::Init(icache_pool);
    }
}

void CWigGraph::Save(TGraphCachePool& icache_pool)
{
    if (m_DataType == eSummary) {
        x_FetchSummary();
        CObjPoolGuard<TGraphCachePool> icache(icache_pool);
        x_SaveSummary(*icache, *m_Data["0"]);
    } else {
        // since last_modified is a part of wig graph key
        // we can skip remote data check by updating m_LastChecked
        m_LastChecked = CTime(CTime::eCurrent).GetTimeT(); 
        CSparseGraph::Save(icache_pool);
    }
}

int CWigGraph::x_RunBigWigScript(const TSeqRange& range, const string tmp_file)
{
    auto from = NStr::NumericToString(range.GetFrom());
    auto to = NStr::NumericToString(range.GetToOpen());
    string request_type;
    string bin_count;

    if (m_DataType == eSummary) {
        request_type = "summary";
        bin_count = NStr::NumericToString(m_SummaryBinCount);
    } else if (m_DataType == eData) {
        request_type = "data";
        bin_count = "0";
    }
    
    CExec::CResult ExecResult =
        CExec::SpawnL(CExec::eNoWait,
                      "./rmt_bigwig_summary.sh",
                      m_RemotePath.c_str(),
                      m_WigId.c_str(),
                      from.c_str(), 
                      to.c_str(),
                      bin_count.c_str(),
                      tmp_file.c_str(),
                      request_type.c_str(),
                      NULL);
    
    //!! parameterize wait time later; for now, set it to 60 seconds
    CProcess::CExitInfo ExitInfo;
    CProcess(ExecResult.GetProcessHandle(), CProcess::eHandle).Wait(CRmtScriptTimeout::Get() * 1000, &ExitInfo);
    return ExitInfo.IsSignaled() ? ExitInfo.GetSignal() : ExitInfo.GetExitCode();
}

void CWigGraph::x_GetBigWigSummary(TSparseVector& summary_sv)
{

    // Occcasionally bigWigSummary fails and it depends on number of data points (m_SummaryBinCount)
    // the code does three attempts to run each decreasing the number of points by 10%
    //_ASSERT(!m_Data.empty());
    float bin_count_pct = 0.9f;
    TSeqRange seq_range(0, m_DataSize - 1);
    if(CRmtFileStatus::Check(m_RemotePath) == CRmtFileStatus::ESuggestedAction_Access) {
        for (size_t count = 0; count < 4; ++count) {
            string sTmpSummary(CDirEntry::GetTmpName(CDirEntry::eTmpFileCreate));
            AutoPtr<fstream> temp_fs(CDirEntry::CreateTmpFile(sTmpSummary));
            
            int exit_code = x_RunBigWigScript(seq_range, sTmpSummary);
            if (exit_code != 0) {
                if (count == 3) {
                    CRmtFileStatus::Set(m_RemotePath, CRmtFileStatus::ESuggestedAction_Skip);
                    string err_msg = (exit_code == -1) ?
                        "bigwig statistics timed out" : "bigwig statistics failed (exit code: " + NStr::NumericToString(exit_code);
                    NCBI_THROW(CException, eUnknown, err_msg);
                }
                ERR_POST(Error << "bin count:" << (bin_count_pct * m_SummaryBinCount) << ", pct:" <<  bin_count_pct);
                m_SummaryBinCount = bin_count_pct * m_SummaryBinCount;
                bin_count_pct -= 0.1;
                continue;
            }
            
            CNcbiIfstream summary_is(sTmpSummary.c_str());
            string val_max, val_min;
            int start = 0;
            vector<TValType> data_points;
            size_t buf_len = 64000;
            data_points.resize(buf_len, 0);
            summary_sv.resize(m_SummaryBinCount);
            //m_Data["0"]->resize(m_SummaryBinCount);
            size_t k = 0;
            int convFlags = NStr::fAllowLeadingSpaces | NStr::fAllowTrailingSpaces | NStr::fConvErr_NoThrow;
            string summary;
            while (NcbiGetline(summary_is, summary, "\n")) {
                if (!NStr::SplitInTwo(summary, "\t", val_min, val_max))
                    continue;
                auto v_max = NStr::StringToNumeric<float>(val_max, convFlags);
                auto v_min = NStr::StringToNumeric<float>(val_min, convFlags);
                float v =  (abs(v_min) > abs(v_max)) ? v_min : v_max;
                x_PackFloat(v, data_points[k]);
                ++k;
                if (k == buf_len) {
                    int batch_size = min<int>(buf_len, m_SummaryBinCount - start);
                    summary_sv.import(&data_points[0], batch_size, start);
                    //m_Data["0"]->import(&data_points[0], batch_size, start);
                    fill_n(data_points.begin(), buf_len, 0);
                    start += batch_size;
                    k = 0;
                }
            }
            if (k > 0) {
                int batch_size = min<int>(k, m_SummaryBinCount - start);
                summary_sv.import(&data_points[0], batch_size, start);
                //m_Data["0"]->import(&data_points[0], batch_size, start);
            }
            summary_sv.optimize();
            //m_Data["0"]->optimize();
            break;
        }
        CRmtFileStatus::Set(m_RemotePath, CRmtFileStatus::ESuggestedAction_Access);
    } else {
        NCBI_THROW(CException, eUnknown, "bigwig statistics not performed because the file is blacklisted");
    }
}

// the data set is split into frams of kDeltaFrameSize
// and each block is compressed using delta coding
void CWigGraph::x_EncodeDelta(const TSeqRange& range, const vector<float>& v_in, vector<TValType>& v_out)
{
    auto len = v_in.size();
    _ASSERT(len > 0);
    v_out.resize(len, 0);
    //vector<TValType> v_out(len, 0);

    TValType last_val = 0;
    for (size_t pos = 0; pos < len; ++pos) {
        TValType curr_val = round(v_in[pos] * kFract);
        if (pos % kDeltaFrameSize == 0) 
            last_val = 0;
        v_out[pos] = x_EncodeSign(curr_val - last_val);
        last_val = curr_val;
    }
}

void CWigGraph::x_DecodeDelta(const TSeqRange& range, vector<float>& v_out)
{

    _ASSERT(range.GetLength() > 0);
    auto from = s_AlignToFrame(range.GetFrom());
    _ASSERT(int(range.GetTo() - from) > 0);
    size_t len = (range.GetTo() - from) + 1;
    vector<TValType> v_in(len, 0);
    m_Data["0"]->extract((TValType*)&v_in[0], len, from);
    v_out.resize(range.GetLength(), 0);
    
    int64_t last = 0;
    size_t index = 0;
    for (size_t i = 0; i < len; ++i) {
        if (i % kDeltaFrameSize == 0) 
            last = 0;
        auto val = x_DecodeSign(v_in[i]);
        val += last;
        last = val;
        if (from + i < range.GetFrom())
            continue;
        float f = val;
        f /= kFract;
        v_out[index++] = f;    
    }
}


void CWigGraph::x_GetBigWigData(const TSeqRange& range, vector<TValType>& data)
{
   _ASSERT(range.GetFrom() % kDeltaFrameSize == 0);
    if (range.GetFrom() % kDeltaFrameSize != 0)
        NCBI_THROW(CException, eUnknown, "Internal error: range is not aligned");
    _ASSERT(range.GetLength() > 0);
    vector<float> raw_data(range.GetLength(), 0);
    {
        string sTmpFile(CDirEntry::GetTmpName(CDirEntry::eTmpFileCreate));
        AutoPtr<fstream> tmp_stream(CDirEntry::CreateTmpFile(sTmpFile));
        auto exit_code = x_RunBigWigScript(range, sTmpFile);
        if (exit_code != 0) {
            string err_msg = (exit_code == -1) ?
                "bigwig data retrieval timed out" : "bigwig data retrieval failed (exit code: " + NStr::NumericToString(exit_code);
            NCBI_THROW(CException, eUnknown, err_msg);
        }
        
        CNcbiIfstream is(sTmpFile.c_str());
        string curr_line;
        while (NcbiGetlineEOL(is, curr_line)) {
            NStr::TruncateSpacesInPlace(curr_line);
            if (curr_line.empty() || curr_line[0] == '#') // skip comments
                continue;
            vector<string> columns;
            NStr::Split(curr_line, " \t", columns, NStr::fSplit_Tokenize);
            if (columns.size() < 4)
                continue;
            if (columns[0] == "browser" || columns[0] == "track")
                continue;
            int start = max<int>(NStr::StringToNumeric<int>(columns[1]), range.GetFrom());
            int stop = NStr::StringToNumeric<int>(columns[2]); // open pos
            int span = stop - start;
            if (span <= 0)
                continue;
            float value = NStr::StringToNumeric<float>(columns[3]);
            TSignedSeqPos pos = start - range.GetFrom();
            fill_n(raw_data.begin() + pos, span, value);
        }
    }
    x_EncodeDelta(range, raw_data, data);    
}    


void CWigGraph::x_SaveSummary(ICache& icache, TSparseVector& data)
{
    //    BM_DECLARE_TEMP_BLOCK(tb);
//    auto& data = *m_Data["0"];
    _ASSERT(m_DataType == eSummary);
    if (m_DataType != eSummary)
        NCBI_THROW(CException, eUnknown, "Invalid save request");

    data.optimize();
    bm::sparse_vector_serial_layout<TSparseVector> sv_lay;
    bm::sparse_vector_serialize(data, sv_lay);
    int len = sv_lay.size();

    CWStream w(icache.GetWriteStream(m_NetCacheKey, 0, NcbiEmptyString), 0, 0, CRWStreambuf::fOwnWriter);
    //if (m_DataType == eSummary) {
        w.write((const char*)&m_SummaryBinCount, sizeof(m_SummaryBinCount));
        if (!w) NCBI_THROW(CException, eUnknown, "Failed to write wig summary bins: " + m_NetCacheKey);
        w.write((const char*)&len, sizeof(len));
        if (!w) NCBI_THROW(CException, eUnknown, "Failed to write wig cache size: " + m_NetCacheKey);
    //}
    
    w.write((const char*)sv_lay.buf(), len);
    if (!w) NCBI_THROW(CException, eUnknown, "Failed to write wig cache data: " + m_NetCacheKey);
    w.flush();
}


static void s_SplitRange(const TSeqRange& range, int frames, vector<TSeqRange>& res)
{
    _ASSERT(frames);
    if (frames <= 0)
        throw runtime_error("Invalid frames size");
    auto start = s_AlignToFrame(range.GetFrom());
    int chunk_size = frames * kDeltaFrameSize;
    res.emplace_back(range.GetFrom(), min<int>(range.GetFrom() + (chunk_size - 1), range.GetTo()));
    start += chunk_size;
    _ASSERT(start % kDeltaFrameSize == 0);
    auto stop = range.GetTo();
    while (start <= stop) {
        res.emplace_back(start, min<int>(start + (chunk_size - 1), stop));
        start += chunk_size;
    }
}


void CWigGraph::x_AddDataToMap(const vector<float>& data, const TSeqPos from, CHistogramGlyph::TMap& the_map)
{
    if (data.empty())
        return;
    auto it = data.begin();
    auto last = *it;

    int start = from;
    int to = from + (data.size() - 1);
    int stop = start;
    while (++it != data.end()) {
        auto val = *it;
        if (val != last) {
            auto tr = TSeqRange(start, stop);
            the_map.AddRange(tr, last);
            last = val;
            start = stop + 1;
        }
        stop +=1;
    }
    if (start < to) {
        auto tr = TSeqRange(start, to);
        the_map.AddRange(tr, last);
    }
}

void CWigGraph::GetData(CHistogramGlyph::TMap& data)
{
    const auto& data_r = data.GetRange();
    _ASSERT(data_r.GetLength() > 0);
    if (data_r.GetLength() == 0) 
        return;
    if (m_DataType == eSummary) {
        x_FetchSummary();
        _ASSERT(!m_Data.empty());
        int bin_size = max<int>(1, m_DataSize/m_SummaryBinCount);
        auto start = data_r.GetFrom()/bin_size;
        auto stop = data_r.GetTo()/bin_size;
        int len = (stop - start) + 1;
        auto from = data_r.GetFrom();
        {
            vector<TSparseVector::value_type>  data_points;
            size_t buf_len = 64000;
            data_points.resize(buf_len, 0);
            while (len > 0) {
                int batch_size = min<int>(buf_len, (stop + 1) - start);
                m_Data["0"]->decode(&data_points[0], start, batch_size);
                for (auto i = 0; i < batch_size; ++i) {
                    auto f = from + i * bin_size;
                    TSeqRange r(f, f + bin_size);
                    float v = 0;
                    x_UnPackFloat(data_points[i], v);
                    data.AddRange(r, v);
                }
                start += batch_size;
                from += (batch_size * bin_size);
                len -= batch_size;
            }
        }
    } else {
        if (m_PrefetchRequested == false)
            Prefetch(data_r);
        x_FetchData();    
        /*
        bool update_data = false;
        for (auto& r : m_PendingData) {
            TPrefetchData d = r.second.get();
            TUpdateMap update = {{ "0", &d->data }};
            Update(d->range, update);
            update_data = true;
        }
        m_PrefetchRequested = false;
        m_PendingData.clear();
        */
        vector<TSeqRange> chunks;
        // split range into chunks of size 10 * kDeltaFrameSize (655353)
        // each chunk except the first one is aligned to frame start
        // first chunk start is not aligned
        s_SplitRange(data_r, 10, chunks);
        for (const auto& chunk : chunks) {
            vector<float> data_points;
            x_DecodeDelta(chunk, data_points);
            _ASSERT(chunk.GetLength() == data_points.size());
            x_AddDataToMap(data_points, chunk.GetFrom(), data);
        }

        //if (update_data)  {
        //    CGraphCache<CWigGraph>::GetInstance().SaveData(Ref(this));
        //}
    }
}

float CWigGraph::GetDataCoverage(const TSeqRange& range)
{
    if (range.GetLength() == 0)
        return 0.;
    vector<TSeqRange> missing_ranges;
    GetMissingRegions(range, missing_ranges);
    int len = range.GetLength();
    for (const auto& r : missing_ranges) {
        len -= r.GetLength();
    }
    float coverage = len;
    coverage /=range.GetLength();
    return coverage;
}

void CWigGraph::Prefetch(const TSeqRange& range)
{
    _ASSERT(m_DataType != eSummary);
    if (m_DataType == eSummary) 
        return;
    //if (m_PrefetchRequested || !m_PendingData.empty())
        //NCBI_THROW(CException, eUnknown, "Prefetch conflict: prefetch requested multiple times");

    vector<TSeqRange> missing_ranges;
    GetMissingRegions(range, missing_ranges);
    bool update_data = !missing_ranges.empty();
    if (update_data == false) 
        return;
    vector<TSeqRange> normalized_ranges;
    TSeqPos from = s_AlignToFrame (missing_ranges.front().GetFrom());
    TSeqPos to = s_AlignToFrame(missing_ranges.front().GetTo()) + (kDeltaFrameSize - 1);
    normalized_ranges.emplace_back(from, to);
    for (size_t i = 1; i < missing_ranges.size(); ++i) {
        const auto& r = missing_ranges[i];
        if (r.GetFrom() > normalized_ranges.back().GetTo()) {
            from = s_AlignToFrame(r.GetFrom());
            to = s_AlignToFrame(r.GetTo()) + (kDeltaFrameSize - 1);
            normalized_ranges.emplace_back(from, to);
        } else if (r.GetTo() > normalized_ranges.back().GetTo()) {
            to = s_AlignToFrame(r.GetTo()) + (kDeltaFrameSize - 1);
            normalized_ranges.back().SetTo(to);
        }
    }
    lock_guard<mutex> guard(m_DataMutex);
    for (auto& range : normalized_ranges) {
        auto it = m_PendingData.find(range);
        if (it == m_PendingData.end()) {
            m_PrefetchRequested = true;    
            m_PendingData.emplace(range, async(std::launch::async, [range, this]{
                TPrefetchData d(new TPrefetchRequest());
                d->range = range;
                x_GetBigWigData(range, d->data);
                return d;
            }));                
        }
    }
}    

void CWigGraph::x_FetchSummary()
{    
    lock_guard<mutex> guard(m_DataMutex);
    if (m_PrefetchRequested) {
        auto s = m_PendingSummary.get();
        m_Data["0"].swap(s);
        m_PrefetchRequested = false;
    }
}

void CWigGraph::x_FetchData()
{
    map<TSeqRange, future<TPrefetchData>> pending_data;
    {
        lock_guard<mutex> guard(m_DataMutex);
        if (m_PrefetchRequested) {
            pending_data = std::move(m_PendingData);
            m_PendingData.clear();
            m_PrefetchRequested = false;
        }
    }
    if (!pending_data.empty()) {
        for (auto& r : pending_data) {
            TPrefetchData d = r.second.get();
            TUpdateMap update = {{ "0", &d->data }};
            Update(d->range, update);
        }
        CGraphCache<CWigGraph>::GetInstance().SaveData(Ref(this));
    }
}


END_NCBI_SCOPE
