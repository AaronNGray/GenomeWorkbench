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
#include <gui/widgets/seq_graphic/vcf_heatmap.hpp>
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
///   CVcfHeatmap


string CVcfHeatmap::GetCacheKey(CScope& scope, const CSeq_id& seq_id, const string& bed_id, const string& remote_path, const string& last_modified)
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

CVcfHeatmap::CVcfHeatmap(const string& cache_key)
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
    m_RemoteId = field_id.GetString();
    if (m_RemoteId.empty()) NCBI_THROW(CException, eUnknown, "Empty remote id");

    auto field_len = field_id.GetNextNeighbor();
    m_SeqLen = (size_t)field_len.GetInteger();
    if (m_SeqLen == 0) NCBI_THROW(CException, eUnknown, "Empty seq_len");

    auto field_path = field_len.GetNextNeighbor();
    m_RemotePath = field_path.GetHost();
    if (m_RemotePath.empty()) NCBI_THROW(CException, eUnknown, "Empty remote path");

    m_LastModified = field_path.GetNextNeighbor().GetString();
    if (m_LastModified.empty()) NCBI_THROW(CException, eUnknown, "Empty last modified");

    CChecksum cs(CChecksum::eMD5);
    cs.AddLine(m_SeqId);
    cs.AddLine(NStr::NumericToString(m_SeqLen));
    cs.AddLine(m_RemoteId);
    cs.AddLine(m_RemotePath);
    cs.AddLine(m_LastModified);
    m_DataKey = cs.GetHexSum();
    m_DataKey += "_rmt_vcf_heatmap";
}


CVcfHeatmap::CVcfHeatmap(const CVcfHeatmap& data)
    : m_SeqId(data.m_SeqId)
    , m_RemoteId(data.m_RemoteId)
    , m_SeqLen(data.m_SeqLen)
    , m_RemotePath(data.m_RemotePath)
    , m_LastModified(data.m_LastModified)
    , m_DataKey(data.m_DataKey)
    , m_Data(data.m_Data)
{
}


void CVcfHeatmap::Init(TGraphCachePool& icache_pool)
{
    lock_guard<mutex> guard(m_DataMutex);
    if (!m_Data.empty())
        return;

    CObjPoolGuard<TGraphCachePool> icache(icache_pool);
    
    // are there any data stored?
    int blob_size = 0;
    try {
        blob_size = icache->GetSize(m_DataKey, 0, NcbiEmptyString);
    } catch(exception& e) {
    }
    if (blob_size > 0) {
        try {
            CRStream is(icache->GetReadStream(m_DataKey, 0, NcbiEmptyString), 0, 0, CRWStreambuf::fOwnReader);
            while (is) {
                uint32_t from = 0, to = 0;
                is.read(reinterpret_cast<char*>(&from), sizeof(from));
                if (is.gcount() == 0)
                    break;
                is.read(reinterpret_cast<char*>(&to), sizeof(to));
                if (is.gcount() == 0)
                    break;
                m_Data.emplace_back(from, to);
            }
            return;
        } catch (exception& e) {
            ERR_POST(Error << e.what());
        }
    }
    x_InitData(*icache);
}

void CVcfHeatmap::Save(TGraphCachePool& icache_pool)
{

    CObjPoolGuard<TGraphCachePool> icache(icache_pool);
    x_SaveData(*icache);
}


void s_GetData(const string& data_url, const string& chrom, const string& length, vector<TSeqRange>& ranges)
{
    string sTmpFilename(CDirEntry::GetTmpName(CDirEntry::eTmpFileCreate));
    AutoPtr<fstream> fs(CDirEntry::CreateTmpFile(sTmpFilename));
    if(CRmtFileStatus::Check(data_url) == CRmtFileStatus::ESuggestedAction_Access) {
  
        CExec::CResult ExecResult(CExec::SpawnL(CExec::eNoWait,
                                                "./rmt_tabixparse",
                                                data_url.c_str(),
                                                chrom.c_str(),
                                                length.c_str(), 
                                                sTmpFilename.c_str(),
                                                NULL));
        
        //!! parametrize wait time later; for now, set it to 60 seconds
        CProcess::CExitInfo ExitInfo;
        CProcess(ExecResult.GetProcessHandle(), CProcess::eHandle).Wait(CRmtScriptTimeout::Get() * 1000, &ExitInfo);
        int ExitCode(ExitInfo.GetExitCode());
        
        if (ExitCode == -1) {
            CRmtFileStatus::Set(data_url, CRmtFileStatus::ESuggestedAction_Skip);
            NCBI_THROW(CException, eUnknown, "vcf index timed out");
        }
        if (ExitCode) {
            CRmtFileStatus::Set(data_url, CRmtFileStatus::ESuggestedAction_Skip);
            NCBI_THROW(CException, eUnknown, "vcf index failed (exit code: " + NStr::NumericToString(ExitCode));
        }
    } else {
        NCBI_THROW(CException, eUnknown, "vcf index is blacklisted");
    }
    CRmtFileStatus::Set(data_url, CRmtFileStatus::ESuggestedAction_Access);
    CNcbiIfstream is(sTmpFilename.c_str());
    while (is) {
        uint32_t from = 0, to = 0;
        is.read(reinterpret_cast<char*>(&from), sizeof(from));
        if (is.gcount() == 0)
            break;
        //if (!is) NCBI_THROW(CException, eUnknown, "Failed to read converted data: " + sTmpFilename);
        is.read(reinterpret_cast<char*>(&to), sizeof(to));
        //if (!is) NCBI_THROW(CException, eUnknown, "Failed to read converted data: " + sTmpFilename);
        ranges.emplace_back(from, to);
    }
}

void CVcfHeatmap::x_SaveData(ICache& icache)
{
    CWStream w(icache.GetWriteStream(m_DataKey, 0, NcbiEmptyString), 0, 0, CRWStreambuf::fOwnWriter);
    for (const auto& r : m_Data) {
        uint32_t from = r.GetFrom(), to = r.GetTo();
        w.write(reinterpret_cast<char*>(&from), sizeof(from));
        if (!w) NCBI_THROW(CException, eUnknown, "Failed to write vcf heatmap: " + m_DataKey);
        w.write(reinterpret_cast<char*>(&to), sizeof(to));
        if (!w) NCBI_THROW(CException, eUnknown, "Failed to write vcf heatmap: " + m_DataKey);
    }
    w.flush();
}

void CVcfHeatmap::x_InitData(ICache& icache)
{
    string str_length = NStr::NumericToString(m_SeqLen);
    s_GetData(m_RemotePath, m_RemoteId, str_length, m_Data);
    x_SaveData(icache);
}

void CVcfHeatmap::GetData(const TSeqRange& range, vector<TSeqRange>& data)
{
    lock_guard<mutex> guard(m_DataMutex);
    data.clear();
    size_t len = range.GetLength();
    _ASSERT(len > 0);
    if (len == 0) 
        return;
    for (const auto& r : m_Data) {
        if (r.GetTo() <= range.GetFrom())
            continue;
        if (r.GetFrom() >= range.GetTo())
            break;
        data.emplace_back(max<int>(range.GetFrom(), r.GetFrom()), min<int>(range.GetTo(), r.GetTo()));
    }
}

END_NCBI_SCOPE
