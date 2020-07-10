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
#include <gui/widgets/seq_graphic/bed_coverage_graph.hpp>
#include <gui/widgets/seq_graphic/graph_cache.hpp>
#include <gui/utils/rmtfilestatus.hpp>
#include <gui/utils/url_utils.hpp>
#include <objmgr/scope.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <util/bitset/bmserial.h>
#include <util/bitset/bmsparsevec_serial.h>
#include <util/checksum.hpp>
#include <corelib/rwstream.hpp>
#include <connect/ncbi_conn_stream.hpp>
#include <corelib/ncbiexec.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
///   CBedCoverageGraph

string CBedCoverageGraph::GetCacheKey(CScope& scope, const CSeq_id& seq_id, const string& bed_id, const string& remote_path, const string& last_modified)
{
    CCompoundIDPool id_pool;
    auto id = id_pool.NewID(eCIC_GenericID);
    auto sih_in = CSeq_id_Handle::GetHandle(seq_id);
    auto sih = scope.GetAccVer(sih_in);
    string seq_id_str = (sih) ? sih.GetSeqId()->GetSeqIdString(true) : seq_id.GetSeqIdString(true);

    id.AppendString(seq_id_str);
    id.AppendString(bed_id);
    size_t seq_len = scope.GetSequenceLength(sih_in);
    _ASSERT(seq_len);
    id.AppendInteger(seq_len);
    id.AppendHost(remote_path);
    string lmd = last_modified;
    if (lmd.empty()) {
        CUrlUtils::GetLastModified(remote_path, lmd);
    }
    id.AppendString(lmd);
    //    LOG_POST(Trace << "Cache_key:" << seq_id_str << "," << bed_id
    //       << "," << seq_len << "," << remote_path << "," << last_modified);
    
    return id.ToString();

}

CBedCoverageGraph::CBedCoverageGraph(const string& cache_key)
{
    _ASSERT(!cache_key.empty());
    if (cache_key.empty())
        NCBI_THROW(CException, eUnknown, "Empty cache key");

    CCompoundIDPool id_pool;
    auto id = id_pool.FromString(cache_key);

    auto field_seq_id = id.GetFirst(eCIT_String);
    m_SeqId = field_seq_id.GetString();
    if (m_SeqId.empty()) NCBI_THROW(CException, eUnknown, "Empty seq_id");

    auto field_id = field_seq_id.GetNextNeighbor();
    m_BedId = field_id.GetString();
    if (m_BedId.empty()) NCBI_THROW(CException, eUnknown, "Empty bed_id");

    auto field_len = field_id.GetNextNeighbor();
    m_SeqLen = (size_t)field_len.GetInteger();
    if (m_SeqLen == 0) NCBI_THROW(CException, eUnknown, "Empty seq_len");

    auto field_path = field_len.GetNextNeighbor();
    m_RemotePath = field_path.GetHost();
    if (m_RemotePath.empty()) NCBI_THROW(CException, eUnknown, "Empty remote path");

    m_LastModified = field_path.GetNextNeighbor().GetString();
    if (m_LastModified.empty()) NCBI_THROW(CException, eUnknown, "Empty last modified");

    m_DataBinCount = m_SeqLen/kMinBinSize;

    CChecksum cs(CChecksum::eMD5);
    cs.AddLine(m_SeqId);
    cs.AddLine(NStr::NumericToString(m_SeqLen));
    cs.AddLine(m_BedId);
    cs.AddLine(m_RemotePath);
    cs.AddLine(m_LastModified);
    m_DataKey = cs.GetHexSum();
    m_DataKey += "_rmt_bedgraph";
}


CBedCoverageGraph::CBedCoverageGraph(const CBedCoverageGraph& data)
    : m_SeqId(data.m_SeqId)
    , m_BedId(data.m_BedId)
    , m_SeqLen(data.m_SeqLen)
    , m_RemotePath(data.m_RemotePath)
    , m_LastModified(data.m_LastModified)
    , m_DataKey(data.m_DataKey)
    , m_DataBinCount(data.m_DataBinCount)
    , m_Data(data.m_Data)
{
}

static 
bool s_ReadData(ICache& icache, const string& data_key, int& bin_count, bm::sparse_vector<unsigned, bm::bvector<> >& sv, bm::word_t* tb)
{
    size_t size = 0;
    try {
        size = icache.GetSize(data_key, 0, NcbiEmptyString);
    } catch (exception& e) {
        ERR_POST(Error << e.what());
        return false;
    }
    
    CRStream is(icache.GetReadStream(data_key, 0, NcbiEmptyString), 0, 0, CRWStreambuf::fOwnReader);
    is.read((char*)&bin_count, sizeof(int));
    if (!is) NCBI_THROW(CException, eUnknown, "Failed to read vector: " + data_key);
    size -= sizeof(int);
    vector<char> buffer(size, 0);
    is.read(&buffer[0], buffer.size());
    if (!is) NCBI_THROW(CException, eUnknown, "Failed to read vector: " + data_key);
    auto res = sparse_vector_deserialize(sv, (const unsigned char*)&buffer[0], tb);
    if (res != 0) NCBI_THROW(CException, eUnknown, "Cache deserialization failed");
    return true;
}


void CBedCoverageGraph::Init(TGraphCachePool& icache_pool)
{
    lock_guard<mutex> guard(m_DataMutex);
    if (!m_Data.empty())
        return;

    CObjPoolGuard<TGraphCachePool> icache(icache_pool);
    
    // are there any data stored?
    bool has_blob = false;
    try {
        has_blob = icache->GetSize(m_DataKey, 0, NcbiEmptyString) >0;
    } catch(exception& e) {
    }
    if (has_blob) {
        //        LOG_POST(Trace << "Retrieving cached data");
        try {
            BM_DECLARE_TEMP_BLOCK(tb);
            s_ReadData(*icache, m_DataKey, m_DataBinCount, m_Data, tb);
            return;
        } catch (exception& e) {
            ERR_POST(Error << e.what());
        }
    }
    x_InitData(*icache);
}

void CBedCoverageGraph::Save(TGraphCachePool& icache_pool)
{

    CObjPoolGuard<TGraphCachePool> icache(icache_pool);
    x_SaveData(*icache);
}


void s_GetBigBedSummary(const string& data_url, const string& chrom, const string& from, const string& to, int bin_count, vector<unsigned int>& data_points)
{
    string sTmpFilename(CDirEntry::GetTmpName(CDirEntry::eTmpFileCreate));
    AutoPtr<fstream> fs(CDirEntry::CreateTmpFile(sTmpFilename));
    string bin_count_str = NStr::NumericToString(bin_count);
    if(CRmtFileStatus::Check(data_url) == CRmtFileStatus::ESuggestedAction_Access) {
        CExec::CResult ExecResult(CExec::SpawnL(CExec::eNoWait,
                                                "./rmt_bigbed_summary.sh",
                                                data_url.c_str(),
                                                chrom.c_str(),
                                                from.c_str(), 
                                                to.c_str(),
                                                bin_count_str.c_str(),
                                                sTmpFilename.c_str(),
                                                "summary",
                                                NULL));
        
        //!! parametrize wait time later; for now, set it to 60 seconds
        CProcess::CExitInfo ExitInfo;
        CProcess(ExecResult.GetProcessHandle(), CProcess::eHandle).Wait(CRmtScriptTimeout::Get() * 1000, &ExitInfo);
        int ExitCode(ExitInfo.GetExitCode());

        if(ExitCode == -1) {
            CRmtFileStatus::Set(data_url, CRmtFileStatus::ESuggestedAction_Skip);
            NCBI_THROW(CException, eUnknown, "bigbed statistics timed out");
        }
        if(ExitCode) {
            CRmtFileStatus::Set(data_url, CRmtFileStatus::ESuggestedAction_Skip);
            NCBI_THROW(CException, eUnknown, "bigbed statistics failed (exit code: " + NStr::NumericToString(ExitCode));
        }
    } else {
        NCBI_THROW(CException, eUnknown, "bigbed statistics not performed because the file is blacklisted");
    }
    CRmtFileStatus::Set(data_url, CRmtFileStatus::ESuggestedAction_Access);
    CNcbiIfstream is(sTmpFilename.c_str());
    string val;
    while (NcbiGetline(is, val, "\t")) {;
        int v = round(NStr::StringToDouble(val, NStr::fAllowLeadingSpaces | NStr::fAllowTrailingSpaces | NStr::fConvErr_NoThrow));
        data_points.push_back(v);
    }
}


void s_GetBigBedFeats(const string& data_url, const string& chrom, const string& from, const string& to, vector<TSeqRange>& ranges)
{
    string sTmpFilename(CDirEntry::GetTmpName(CDirEntry::eTmpFileCreate));
    AutoPtr<fstream> fs(CDirEntry::CreateTmpFile(sTmpFilename));
    if(CRmtFileStatus::Check(data_url) == CRmtFileStatus::ESuggestedAction_Access) {
        CExec::CResult ExecResult(CExec::SpawnL(CExec::eNoWait,
                                                "./rmt_bigbed_summary.sh",
                                                data_url.c_str(),
                                                chrom.c_str(),
                                                from.c_str(), 
                                                to.c_str(),
                                                "",
                                                sTmpFilename.c_str(),
                                                "feats",
                                                NULL));
        
        //!! parametrize wait time later; for now, set it to 60 seconds
        CProcess::CExitInfo ExitInfo;
        CProcess(ExecResult.GetProcessHandle(), CProcess::eHandle).Wait(CRmtScriptTimeout::Get() * 1000, &ExitInfo);
        int ExitCode(ExitInfo.GetExitCode());

        if(ExitCode == -1) 
            CRmtFileStatus::Set(data_url, CRmtFileStatus::ESuggestedAction_Skip);
            NCBI_THROW(CException, eUnknown, "bigbed statistics timed out");
        if(ExitCode) 
            CRmtFileStatus::Set(data_url, CRmtFileStatus::ESuggestedAction_Skip);
            NCBI_THROW(CException, eUnknown, "bigbed statistics failed (exit code: " + NStr::NumericToString(ExitCode));
    } else {
        NCBI_THROW(CException, eUnknown, "bigbed statistics not performed because the file is blacklisted");
    }
    CRmtFileStatus::Set(data_url, CRmtFileStatus::ESuggestedAction_Access);
    CNcbiIfstream is(sTmpFilename.c_str());
    string val;
    while (NcbiGetline(is, val, "\n")) {
        string start_s, end_s;
        if (!NStr::SplitInTwo(val, "\t", start_s, end_s))
            continue;
        int from = NStr::StringToInt(start_s, NStr::fAllowLeadingSpaces | NStr::fAllowTrailingSpaces | NStr::fConvErr_NoThrow);
        int to = NStr::StringToInt(end_s, NStr::fAllowLeadingSpaces | NStr::fAllowTrailingSpaces | NStr::fConvErr_NoThrow);
        if (from == to)
            continue;
        ranges.emplace_back(from, to);
    }
}

void CBedCoverageGraph::x_SaveData(ICache& icache)
{
    m_Data.optimize();
    bm::sparse_vector_serial_layout<TSparseVector> sv_lay;
    bm::sparse_vector_serialize(m_Data, sv_lay);
    CWStream w(icache.GetWriteStream(m_DataKey, 0, NcbiEmptyString), 0, 0, CRWStreambuf::fOwnWriter);
    w.write((const char*)&m_DataBinCount, sizeof(m_DataBinCount));
    if (!w) NCBI_THROW(CException, eUnknown, "Failed to write bed data: " + m_DataKey);
    w.write((const char*)sv_lay.buf(), sv_lay.size());
    if (!w) NCBI_THROW(CException, eUnknown, "Failed to write bed data: " + m_DataKey);
    w.flush();
}

void CBedCoverageGraph::x_InitData(ICache& icache)
{
    //    LOG_POST(Trace << "Running bigBedSummary");
    string from = "0";
    string to = NStr::NumericToString(m_SeqLen - 1);
    vector<unsigned int> data_points;
    s_GetBigBedSummary(m_RemotePath, m_BedId, from, to, m_DataBinCount, data_points);
    _ASSERT(data_points.size() == m_DataBinCount);
    /*
    bool shallow_depth = true;
    for (auto i = 0; i < data_points.size(); ++i) {
        if (data_points[i] > 5) {
            shallow_depth = false;
            break;
        }
    }
    if (shallow_depth) {
        auto orig_size = data_points.size();
        data_points.clear();
        vector<TSeqRange> ranges;
        s_GetBigBedFeats(m_RemotePath, m_BedId, "0", NStr::NumericToString(m_SeqLen - 1), ranges);
        if (ranges.empty()) NCBI_THROW(CException, eUnknown, "Failed to get bed data");
        CHistogramGlyph::TMap d_map(0, m_SeqLen - 1, float(kMinBinSize), new plus_func<CHistogramGlyph::TMap::container_type::value_type>());
        for (const auto& r: ranges) {
            d_map.AddRange(r, 1);
        }
        auto num_bins = d_map.GetBins();
        int m = 0;
        for (size_t i = 0; i < num_bins; ++i) {
            auto val = round(d_map[i]);
            data_points.push_back(val);
            m = max<int>(m, val);
        }
        m_IsSummary = false;
        LOG_POST(Error << "\n\n -----------------\norig_size:"
                 << orig_size << ", new size:" << data_points.size() << ", expected:" << (m_SeqLen/kMinBinSize)
                 << ",max" << m << ", bins:" << num_bins);
    }
    */
    m_Data.resize(data_points.size());
    m_Data.import(&data_points[0], data_points.size(), 0);
    data_points.clear();
    x_SaveData(icache);
}

void CBedCoverageGraph::GetData(CHistogramGlyph::TMap& data)
{
    const auto& data_r = data.GetRange();
    _ASSERT(data_r.GetLength() > 0);
    if (data_r.GetLength() == 0) {
        data.Clear();
        return;
    }
    int bin_size = max<int>(1, m_SeqLen/m_DataBinCount);
    //    LOG_POST(Trace << "Bin size:" << bin_size);
    //    LOG_POST(Trace << "Range:" << data_r);
    auto start = data_r.GetFrom()/bin_size;
    auto stop = data_r.GetTo()/bin_size;
    int len = (stop - start) + 1;
    _ASSERT(len > 0);
    if (len <= 0) { 
        data.Clear();
        return;
    }
    vector<unsigned int> temp_data(len, 0);
    {
        lock_guard<mutex> guard(m_DataMutex);
        m_Data.decode(&temp_data[0], start, len);
    }
    int max_value = 0;
    TSeqRange r(data_r.GetFrom(), data_r.GetFrom() + bin_size - 1);
    auto val = temp_data[0];
    for (auto i = 1; i < len; ++i) {
        auto from = data_r.GetFrom() + i * bin_size;
        auto to = from + bin_size - 1;
        if (temp_data[i] == val && from == r.GetToOpen()) {
            r.SetTo(to);
            continue;
        }
        data.AddRange(r, val);
        r.Set(from, to);
        val = temp_data[i];
        max_value = max<int>(max_value, val);
    }
    data.AddRange(r, val);
    max_value = max<int>(max_value, val);
}

/* 
    as the actual feat count is not available via bigBedSummary
    the fuction returns the number of bins with data 
*/
size_t CBedCoverageGraph::GetEstimatedFeatureCount(const TSeqRange& range, int& max_count)
{
    max_count = 0;
    if (range.GetLength() == 0) 
        return 0;

    int bin_size = max<int>(1, m_SeqLen/m_DataBinCount);
    auto start = range.GetFrom()/bin_size;
    auto stop = range.GetTo()/bin_size;
    auto len = (stop - start) + 1;
    if (len <= 0)
        return 0;
    vector<unsigned int> temp_data(len, 0);
    {
        lock_guard<mutex> guard(m_DataMutex);
        m_Data.decode(&temp_data[0], start, len);
    }
    int count = 0;
    max_count = 0;
    int empty_bins = 0;
    for (auto i = 0; i < len; ++i) {
        count += temp_data[i];
        if (temp_data[i] == 0) 
            ++empty_bins;
        max_count = max<int>(max_count, temp_data[i]);
    }
    return len - empty_bins;
    
}
    


END_NCBI_SCOPE
