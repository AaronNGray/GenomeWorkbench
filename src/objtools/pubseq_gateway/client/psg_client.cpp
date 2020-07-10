/*  $Id: psg_client.cpp 605160 2020-04-07 18:06:40Z ivanov $
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
 * Author: Rafael Sadyrov
 *
 */

#include <ncbi_pch.hpp>

#include <objtools/pubseq_gateway/client/psg_client.hpp>

#ifdef HAVE_PSG_CLIENT

#include <condition_variable>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>

#include <corelib/ncbitime.hpp>
#include <connect/ncbi_socket.hpp>
#include <connect/ncbi_service.h>
#include <connect/ncbi_connutil.h>

#include "psg_client_impl.hpp"

BEGIN_NCBI_SCOPE


const char* CPSG_Exception::GetErrCodeString(void) const
{
    switch (GetErrCode())
    {
        case eTimeout:          return "eTimeout";
        case eServerError:      return "eServerError";
        case eInternalError:    return "eInternalError";
        case eParameterMissing: return "eParameterMissing";
        default:                return CException::GetErrCodeString();
    }
}


SPSG_BlobReader::SPSG_BlobReader(SPSG_Reply::SItem::TTS* src)
    : m_Src(src)
{
    assert(src);
}

ERW_Result SPSG_BlobReader::x_Read(void* buf, size_t count, size_t* bytes_read)
{
    assert(bytes_read);

    *bytes_read = 0;

    CheckForNewChunks();

    for (; m_Chunk < m_Data.size(); ++m_Chunk) {
        auto& data = m_Data[m_Chunk];

        // Chunk has not been received yet
        if (data.empty()) return eRW_Success;

        auto available = data.size() - m_Index;
        auto to_copy = min(count, available);

        memcpy(buf, data.data() + m_Index, to_copy);
        buf = (char*)buf + to_copy;
        count -= to_copy;
        *bytes_read += to_copy;
        m_Index += to_copy;

        if (!count) return eRW_Success;

        m_Index = 0;
    }

    auto src_locked = m_Src->GetLock();
    return src_locked->expected.Cmp<equal_to>(src_locked->received) ? eRW_Eof : eRW_Success;
}

ERW_Result SPSG_BlobReader::Read(void* buf, size_t count, size_t* bytes_read)
{
    size_t read;
    const auto kSeconds = TPSG_ReaderTimeout::GetDefault();
    CDeadline deadline(kSeconds);

    do {
        auto rv = x_Read(buf, count, &read);

        if ((rv != eRW_Success) || (read != 0)) {
            if (bytes_read) *bytes_read = read;
            return rv;
        }
    }
    while (m_Src->WaitUntil(deadline));

    NCBI_THROW_FMT(CPSG_Exception, eTimeout, "Timeout on reading (after " << kSeconds << " seconds)");
    return eRW_Error;
}

ERW_Result SPSG_BlobReader::PendingCount(size_t* count)
{
    assert(count);

    *count = 0;

    CheckForNewChunks();

    auto k = m_Index;

    for (auto i = m_Chunk; i < m_Data.size(); ++i) {
        auto& data = m_Data[i];

        // Chunk has not been received yet
        if (data.empty()) return eRW_Success;

        *count += data.size() - k;
        k = 0;
    }

    return eRW_Success;
}

void SPSG_BlobReader::CheckForNewChunks()
{
    if (m_Src->GetMTSafe().state.Empty()) return;

    auto src_locked = m_Src->GetLock();
    auto& src = *src_locked;
    auto& chunks = src.chunks;

    if (m_Data.size() < chunks.size()) m_Data.resize(chunks.size());

    for (size_t i = 0; i < chunks.size(); ++i) {
        if (!chunks[i].empty()) {
            m_Data[i].swap(chunks[i]);
        }
    }
}


static_assert(is_nothrow_move_constructible<CPSG_BioId>::value, "CPSG_BioId move constructor must be noexcept");
static_assert(is_nothrow_move_constructible<CPSG_BlobId>::value, "CPSG_BlobId move constructor must be noexcept");


template <class TReplyItem>
TReplyItem* CPSG_Reply::SImpl::CreateImpl(TReplyItem* item, const vector<SPSG_Chunk>& chunks)
{
    if (chunks.empty()) return item;

    unique_ptr<TReplyItem> rv(item);
    rv->m_Data = CJsonNode::ParseJSON(chunks.front());

    return rv.release();
}


shared_ptr<CPSG_ReplyItem> CPSG_Reply::SImpl::Create(SPSG_Reply::SItem::TTS* item_ts)
{
    auto user_reply_locked = user_reply.lock();

    assert(user_reply_locked);
    assert(item_ts);

    auto item_locked = item_ts->GetLock();

    item_locked->state.SetReturned();

    unique_ptr<CPSG_ReplyItem::SImpl> impl(new CPSG_ReplyItem::SImpl);
    impl->item = item_ts;

    shared_ptr<CPSG_ReplyItem> rv;

    auto& chunks = item_locked->chunks;
    auto& args = item_locked->args;
    auto item_type = args.GetValue("item_type");

    if (item_type == "blob") {
        auto blob_id = args.GetValue("blob_id");
        auto reason = args.GetValue("reason");

        if (reason.empty()) {
            unique_ptr<CPSG_BlobData> blob_data(new CPSG_BlobData(blob_id));
            blob_data->m_Stream.reset(new SPSG_RStream(item_ts));
            rv.reset(blob_data.release());
        } else {
            auto r = CPSG_SkippedBlob::eUnknown;

            if (reason == "excluded") {
                r = CPSG_SkippedBlob::eExcluded;
            } else if (reason == "inprogress") {
                r = CPSG_SkippedBlob::eInProgress;
            } else if (reason == "sent") {
                r = CPSG_SkippedBlob::eSent;
            }

            rv.reset(new CPSG_SkippedBlob(blob_id, r));
        }

    } else if (item_type == "bioseq_info") {
        rv.reset(CreateImpl(new CPSG_BioseqInfo, chunks));

    } else if (item_type == "blob_prop") {
        auto blob_id = args.GetValue("blob_id");
        rv.reset(CreateImpl(new CPSG_BlobInfo(blob_id), chunks));

    } else if (item_type == "bioseq_na") {
        auto name = args.GetValue("na");
        rv.reset(CreateImpl(new CPSG_NamedAnnotInfo(name), chunks));

    } else {
        NCBI_THROW_FMT(CPSG_Exception, eServerError, "Received unknown item type: " << item_type);
    }

    rv->m_Impl.reset(impl.release());
    rv->m_Reply = user_reply_locked;
    return rv;
}


pair<mutex, weak_ptr<CPSG_Queue::SImpl::CService::TMap>> CPSG_Queue::SImpl::CService::sm_Instance;

SPSG_IoCoordinator& CPSG_Queue::SImpl::CService::GetIoC(const string& service)
{
    if (service.empty()) {
        NCBI_THROW(CPSG_Exception, eParameterMissing, "Service name is empty");
    }

    unique_lock<mutex> lock(sm_Instance.first);

    auto found = m_Map->find(service);

    if (found != m_Map->end()) {
        return *found->second;
    }

    auto created = m_Map->emplace(service, unique_ptr<SPSG_IoCoordinator>(new SPSG_IoCoordinator(service)));
    return *created.first->second;
}

shared_ptr<CPSG_Queue::SImpl::CService::TMap> CPSG_Queue::SImpl::CService::GetMap()
{
    unique_lock<mutex> lock(sm_Instance.first);

    auto rv = sm_Instance.second.lock();

    if (!rv) {
        rv = make_shared<TMap>();
        sm_Instance.second = rv;
    }

    return rv;
}


CPSG_Queue::SImpl::SImpl(const string& service) :
    m_Service(service)
{
}

const char* s_GetTSE(CPSG_Request_Biodata::EIncludeData include_data)
{
    switch (include_data) {
        case CPSG_Request_Biodata::eDefault:  return nullptr;
        case CPSG_Request_Biodata::eNoTSE:    return "none";
        case CPSG_Request_Biodata::eSlimTSE:  return "slim";
        case CPSG_Request_Biodata::eSmartTSE: return "smart";
        case CPSG_Request_Biodata::eWholeTSE: return "whole";
        case CPSG_Request_Biodata::eOrigTSE:  return "orig";
    }

    return nullptr;
}

string CPSG_Queue::SImpl::x_GetAbsPathRef(shared_ptr<const CPSG_Request> user_request)
{
    auto& ioc = m_Service.ioc;
    ostringstream os;
    user_request->x_GetAbsPathRef(os);

    switch (ioc.params.use_cache) {
        case EPSG_UseCache::eDefault:                         break;
        case EPSG_UseCache::eNo:      os << "&use_cache=no";  break;
        case EPSG_UseCache::eYes:     os << "&use_cache=yes"; break;
    }

    os << ioc.GetClientId();
    return os.str();
}

const char* s_GetAccSubstitution(EPSG_AccSubstitution acc_substitution)
{
    switch (acc_substitution) {
        case EPSG_AccSubstitution::Default: break;
        case EPSG_AccSubstitution::Limited: return "&acc_substitution=limited";
        case EPSG_AccSubstitution::Never:   return "&acc_substitution=never";
    }

    return "";
}


struct CPSG_Request::x_GetBioIdParams
{
    const string& id;
    const CPSG_BioId::TType& type;

    x_GetBioIdParams(const CPSG_BioId& bio_id) : id(bio_id.m_Id), type(bio_id.m_Type) {}

    friend ostream& operator<<(ostream& os, const x_GetBioIdParams& bio_id)
    {
        if (bio_id.type) os << "seq_id_type=" << bio_id.type << '&';
        return os << "seq_id=" << bio_id.id;
    }
};


void CPSG_Request_Biodata::x_GetAbsPathRef(ostream& os) const
{
    os << "/ID/get?" << x_GetBioIdParams(m_BioId);

    if (const auto tse = s_GetTSE(m_IncludeData)) os << "&tse=" << tse;

    if (!m_ExcludeTSEs.empty()) {
        os << "&exclude_blobs";

        char delimiter = '=';
        for (const auto& blob_id : m_ExcludeTSEs) {
            os << delimiter << blob_id.Get();
            delimiter = ',';
        }
    }

    os << s_GetAccSubstitution(m_AccSubstitution);
}

void CPSG_Request_Resolve::x_GetAbsPathRef(ostream& os) const
{
    os << "/ID/resolve?" << x_GetBioIdParams(m_BioId) << "&fmt=json&psg_protocol=yes";

    auto value = "yes";
    auto include_info = m_IncludeInfo;
    const auto max_bit = (numeric_limits<unsigned>::max() >> 1) + 1;

    if (include_info & max_bit) {
        os << "&all_info=yes";
        value = "no";
        include_info = ~include_info;
    }

    if (include_info & CPSG_Request_Resolve::fCanonicalId)  os << "&canon_id=" << value;
    if (include_info & CPSG_Request_Resolve::fName)         os << "&name=" << value;
    if (include_info & CPSG_Request_Resolve::fOtherIds)     os << "&seq_ids=" << value;
    if (include_info & CPSG_Request_Resolve::fMoleculeType) os << "&mol_type=" << value;
    if (include_info & CPSG_Request_Resolve::fLength)       os << "&length=" << value;
    if (include_info & CPSG_Request_Resolve::fChainState)   os << "&seq_state=" << value;
    if (include_info & CPSG_Request_Resolve::fState)        os << "&state=" << value;
    if (include_info & CPSG_Request_Resolve::fBlobId)       os << "&blob_id=" << value;
    if (include_info & CPSG_Request_Resolve::fTaxId)        os << "&tax_id=" << value;
    if (include_info & CPSG_Request_Resolve::fHash)         os << "&hash=" << value;
    if (include_info & CPSG_Request_Resolve::fDateChanged)  os << "&date_changed=" << value;
    if (include_info & CPSG_Request_Resolve::fGi)           os << "&gi=" << value;

    os << s_GetAccSubstitution(m_AccSubstitution);
}

void CPSG_Request_Blob::x_GetAbsPathRef(ostream& os) const
{
    os << "/ID/getblob?blob_id=" << m_BlobId.Get();

    if (!m_LastModified.empty()) os << "&last_modified=" << m_LastModified;

    if (const auto tse = s_GetTSE(m_IncludeData)) os << "&tse=" << tse;
}

void CPSG_Request_NamedAnnotInfo::x_GetAbsPathRef(ostream& os) const
{
    os << "/ID/get_na?" << x_GetBioIdParams(m_BioId) << "&names=";

    for (const auto& name : m_AnnotNames) {
        os << name << ",";
    }

    // Remove last comma (there must be some output after seekp to succeed)
    os.seekp(-1, ios_base::cur);
    os << "&fmt=json&psg_protocol=yes";
    os << s_GetAccSubstitution(m_AccSubstitution);
}

void CPSG_Request_TSE_Chunk::x_GetAbsPathRef(ostream& os) const
{
    os << "/ID/get_tse_chunk?tse_id=" << m_TSE_BlobId.Get() <<
        "&chunk=" << m_ChunkNo << "&split_version=" << m_SplitVersion;
}

bool CPSG_Queue::SImpl::SendRequest(shared_ptr<const CPSG_Request> user_request, const CDeadline& deadline)
{
    auto& ioc = m_Service.ioc;
    auto& params = ioc.params;

    auto user_context = params.client_mode == EPSG_PsgClientMode::eOff ?
        nullptr : user_request->GetUserContext<string>();
    const auto request_id = user_context ? *user_context : ioc.GetNewRequestId();
    auto reply = make_shared<SPSG_Reply>(move(request_id), params);
    auto abs_path_ref = x_GetAbsPathRef(user_request);
    auto request = make_shared<SPSG_Request>(move(abs_path_ref), reply, user_request->m_RequestContext, params);

    if (ioc.AddRequest(request, Stopped(), deadline)) {
        shared_ptr<CPSG_Reply> user_reply(new CPSG_Reply);
        user_reply->m_Impl->reply = move(reply);
        user_reply->m_Impl->user_reply = user_reply;
        user_reply->m_Request = move(user_request);
        Push(move(user_reply));
        return true;
    }

    return false;
}

EPSG_Status s_GetStatus(SPSG_Reply::SItem::TTS* ts, const CDeadline& deadline)
{
    assert(ts);

    auto& state = ts->GetMTSafe().state;

    do {
        switch (state.GetState()) {
            case SPSG_Reply::SState::eNotFound:   return EPSG_Status::eNotFound;
            case SPSG_Reply::SState::eError:      return EPSG_Status::eError;
            case SPSG_Reply::SState::eSuccess:    return EPSG_Status::eSuccess;
            case SPSG_Reply::SState::eInProgress: break;
        }
    }
    while (state.change.WaitUntil(deadline));

    return EPSG_Status::eInProgress;
}

EPSG_Status CPSG_ReplyItem::GetStatus(CDeadline deadline) const
{
    assert(m_Impl);

    return s_GetStatus(m_Impl->item, deadline);
}

string CPSG_ReplyItem::GetNextMessage() const
{
    assert(m_Impl);
    assert(m_Impl->item);

    return m_Impl->item->GetLock()->state.GetError();
}

CPSG_ReplyItem::~CPSG_ReplyItem()
{
}

CPSG_ReplyItem::CPSG_ReplyItem(EType type) :
    m_Type(type)
{
}


CPSG_BlobData::CPSG_BlobData(CPSG_BlobId id) :
    CPSG_ReplyItem(eBlobData),
    m_Id(move(id))
{
}


CPSG_BlobInfo::CPSG_BlobInfo(CPSG_BlobId id) :
    CPSG_ReplyItem(eBlobInfo),
    m_Id(move(id))
{
}

enum EPSG_BlobInfo_Flags
{
    fPSGBI_CheckFailed = 1 << 0,
    fPSGBI_Gzip        = 1 << 1,
    fPSGBI_Not4Gbu     = 1 << 2,
    fPSGBI_Withdrawn   = 1 << 3,
    fPSGBI_Suppress    = 1 << 4,
    fPSGBI_Dead        = 1 << 5,
};

string CPSG_BlobInfo::GetCompression() const
{
    return m_Data.GetInteger("flags") & fPSGBI_Gzip ? "gzip" : "";
}

string CPSG_BlobInfo::GetFormat() const
{
    return "asn.1";
}

Uint8 CPSG_BlobInfo::GetVersion() const
{
    return static_cast<Uint8>(m_Data.GetInteger("last_modified"));
}

Uint8 CPSG_BlobInfo::GetStorageSize() const
{
    return static_cast<Uint8>(m_Data.GetInteger("size"));
}

Uint8 CPSG_BlobInfo::GetSize() const
{
    return static_cast<Uint8>(m_Data.GetInteger("size_unpacked"));
}

bool CPSG_BlobInfo::IsDead() const
{
    return m_Data.GetInteger("flags") & fPSGBI_Dead;
}

bool CPSG_BlobInfo::IsSuppressed() const
{
    return m_Data.GetInteger("flags") & fPSGBI_Suppress;
}

bool CPSG_BlobInfo::IsWithdrawn() const
{
    return m_Data.GetInteger("flags") & fPSGBI_Withdrawn;
}

CTime s_GetTime(Int8 milliseconds)
{
    return milliseconds > 0 ? CTime(static_cast<time_t>(milliseconds / kMilliSecondsPerSecond)) : CTime();
}

CTime CPSG_BlobInfo::GetHupReleaseDate() const
{
    return s_GetTime(m_Data.GetInteger("hup_date"));
}

Uint8 CPSG_BlobInfo::GetOwner() const
{
    return static_cast<Uint8>(m_Data.GetInteger("owner"));
}

CTime CPSG_BlobInfo::GetOriginalLoadDate() const
{
    return s_GetTime(m_Data.GetInteger("date_asn1"));
}

objects::CBioseq_set::EClass CPSG_BlobInfo::GetClass() const
{
    return static_cast<objects::CBioseq_set::EClass>(m_Data.GetInteger("class"));
}

string CPSG_BlobInfo::GetDivision() const
{
    return m_Data.GetString("div");
}

string CPSG_BlobInfo::GetUsername() const
{
    return m_Data.GetString("username");
}

struct SId2Info
{
    enum : size_t { eSat, eInfo, eNChunks, eSplitVer, eMinSize = eNChunks + 1 };

    vector<CTempString> values;
    int sat = 0;

    SId2Info(const CJsonNode& data, const CPSG_BlobId& id);

    explicit operator bool() const { return !values.empty() && (sat != 0); }

private:
    string m_Value;
};

SId2Info::SId2Info(const CJsonNode& data, const CPSG_BlobId& id)
{
    if (!data.HasKey("id2_info")) return;

    m_Value = data.GetString("id2_info");

    if (m_Value.empty()) return;

    NStr::Split(m_Value, ".", values);

    if (values.size() < eMinSize ) {
        NCBI_THROW_FMT(CPSG_Exception, eServerError, "Wrong id2_info format: " << m_Value <<
                " for blob '" << id.Get() << '\'');
    }

    auto sat_str = values[eSat];

    if (!sat_str.empty()) {
        sat = NStr::StringToInt(sat_str);
    }
}

CPSG_BlobId CPSG_BlobInfo::GetSplitInfoBlobId() const
{
    SId2Info id2_info(m_Data, m_Id);

    if (!id2_info) return kEmptyStr;

    auto sat_key = id2_info.values[SId2Info::eInfo];
    return CPSG_BlobId(id2_info.sat, NStr::StringToInt(sat_key));
}

CPSG_BlobId CPSG_BlobInfo::GetChunkBlobId(unsigned split_chunk_no) const
{
    if (split_chunk_no == 0) return kEmptyStr;

    int index = static_cast<int>(split_chunk_no);

    SId2Info id2_info(m_Data, m_Id);

    if (!id2_info) return kEmptyStr;

    auto info = NStr::StringToInt(id2_info.values[SId2Info::eInfo]);

    if (info <= 0) return kEmptyStr;

    auto nchunks = NStr::StringToInt(id2_info.values[SId2Info::eNChunks]);

    if (nchunks <= 0) return kEmptyStr;
    if (nchunks < index) return kEmptyStr;

    return CPSG_BlobId(id2_info.sat, info + index - nchunks - 1);
}

CPSG_BlobInfo::TSplitVersion CPSG_BlobInfo::GetSplitVersion() const
{
    SId2Info id2_info(m_Data, m_Id);

    if (!id2_info) return 0;
    if (id2_info.values.size() <= SId2Info::eSplitVer) return 0;

    auto split_ver = id2_info.values[SId2Info::eSplitVer];
    return split_ver.empty() ? 0 : NStr::StringToInt(split_ver);
}


CPSG_SkippedBlob::CPSG_SkippedBlob(CPSG_BlobId id, EReason reason) :
    CPSG_ReplyItem(eSkippedBlob),
    m_Id(id),
    m_Reason(reason)
{
}


CPSG_BioseqInfo::CPSG_BioseqInfo()
    : CPSG_ReplyItem(eBioseqInfo)
{
}

CPSG_BioId CPSG_BioseqInfo::GetCanonicalId() const
{
    auto type = static_cast<CPSG_BioId::TType>(m_Data.GetInteger("seq_id_type"));
    auto accession = m_Data.GetString("accession");
    auto name_node = m_Data.GetByKeyOrNull("name");
    auto name = name_node && name_node.IsString() ? name_node.AsString() : string();
    auto version = static_cast<int>(m_Data.GetInteger("version"));
    return objects::CSeq_id(type, accession, name, version).AsFastaString();
};

vector<CPSG_BioId> CPSG_BioseqInfo::GetOtherIds() const
{
    auto seq_ids = m_Data.GetByKey("seq_ids");
    vector<CPSG_BioId> rv;
    bool error = !seq_ids.IsArray();

    for (CJsonIterator it = seq_ids.Iterate(); !error && it.IsValid(); it.Next()) {
        auto seq_id = it.GetNode();
        error = !seq_id.IsArray() || (seq_id.GetSize() != 2);

        if (!error) {
            auto tag = objects::CSeq_id::eFasta_AsTypeAndContent;
            auto type = static_cast<CPSG_BioId::TType>(seq_id.GetAt(0).AsInteger());
            auto content = seq_id.GetAt(1).AsString();
            rv.emplace_back(objects::CSeq_id(tag, type, content).AsFastaString());
        }
    }

    if (error) {
        auto reply = GetReply();
        _ASSERT(reply);

        auto request = reply->GetRequest().get();
        _ASSERT(request);

        NCBI_THROW_FMT(CPSG_Exception, eServerError, "Wrong seq_ids format: '" << seq_ids.Repr() <<
                "' for " << request->GetType() << " request '" << request->GetId() << '\'');
    }

    return rv;
}

objects::CSeq_inst::TMol CPSG_BioseqInfo::GetMoleculeType() const
{
    return static_cast<objects::CSeq_inst::TMol>(m_Data.GetInteger("mol"));
}

Uint8 CPSG_BioseqInfo::GetLength() const
{
    return m_Data.GetInteger("length");
}

CPSG_BioseqInfo::TState CPSG_BioseqInfo::GetChainState() const
{
    return static_cast<TState>(m_Data.GetInteger("seq_state"));
}

CPSG_BioseqInfo::TState CPSG_BioseqInfo::GetState() const
{
    return static_cast<TState>(m_Data.GetInteger("state"));
}

CPSG_BlobId CPSG_BioseqInfo::GetBlobId() const
{
    auto sat = static_cast<int>(m_Data.GetInteger("sat"));
    auto sat_key = static_cast<int>(m_Data.GetInteger("sat_key"));
    return CPSG_BlobId(sat, sat_key);
}

TTaxId CPSG_BioseqInfo::GetTaxId() const
{
    return static_cast<TTaxId>(m_Data.GetInteger("tax_id"));
}

int CPSG_BioseqInfo::GetHash() const
{
    return static_cast<int>(m_Data.GetInteger("hash"));
}

CTime CPSG_BioseqInfo::GetDateChanged() const
{
    return s_GetTime(m_Data.GetInteger("date_changed"));
}

TGi CPSG_BioseqInfo::GetGi() const
{
    return static_cast<TGi>(m_Data.GetInteger("gi"));
}

CPSG_Request_Resolve::TIncludeInfo CPSG_BioseqInfo::IncludedInfo() const
{
    CPSG_Request_Resolve::TIncludeInfo rv = {};

    if (m_Data.HasKey("accession") && m_Data.HasKey("seq_id_type"))       rv |= CPSG_Request_Resolve::fCanonicalId;
    if (m_Data.HasKey("name"))                                            rv |= CPSG_Request_Resolve::fName;
    if (m_Data.HasKey("seq_ids") && m_Data.GetByKey("seq_ids").GetSize()) rv |= CPSG_Request_Resolve::fOtherIds;
    if (m_Data.HasKey("mol"))                                             rv |= CPSG_Request_Resolve::fMoleculeType;
    if (m_Data.HasKey("length"))                                          rv |= CPSG_Request_Resolve::fLength;
    if (m_Data.HasKey("seq_state"))                                       rv |= CPSG_Request_Resolve::fChainState;
    if (m_Data.HasKey("state"))                                           rv |= CPSG_Request_Resolve::fState;
    if (m_Data.HasKey("sat") && m_Data.HasKey("sat_key"))                 rv |= CPSG_Request_Resolve::fBlobId;
    if (m_Data.HasKey("tax_id"))                                          rv |= CPSG_Request_Resolve::fTaxId;
    if (m_Data.HasKey("hash"))                                            rv |= CPSG_Request_Resolve::fHash;
    if (m_Data.HasKey("date_changed"))                                    rv |= CPSG_Request_Resolve::fDateChanged;
    if (m_Data.HasKey("gi"))                                              rv |= CPSG_Request_Resolve::fGi;

    return rv;
}


CPSG_NamedAnnotInfo::CPSG_NamedAnnotInfo(string name) :
    CPSG_ReplyItem(eNamedAnnotInfo),
    m_Name(move(name))
{
}

CRange<TSeqPos> CPSG_NamedAnnotInfo::GetRange() const
{
    auto start = static_cast<TSeqPos>(m_Data.GetInteger("start"));
    auto stop = static_cast<TSeqPos>(m_Data.GetInteger("stop"));
    return { start, stop };
}

CPSG_BlobId CPSG_NamedAnnotInfo::GetBlobId() const
{
    auto sat = static_cast<int>(m_Data.GetInteger("sat"));
    auto sat_key = static_cast<int>(m_Data.GetInteger("sat_key"));
    return { sat, sat_key };
}

Uint8 CPSG_NamedAnnotInfo::GetVersion() const
{
    return static_cast<Uint8>(m_Data.GetInteger("last_modified"));
}

template <class TResult>
struct SAnnotInfoProcessor
{
    using TAction = function<bool(const CJsonNode&, TResult& result)>;
    using TActions = map<int, TAction>;

    SAnnotInfoProcessor(TActions actions) : m_Actions(move(actions)) {}

    TResult operator()(const CPSG_ReplyItem* item, const CJsonNode& data) const;

private:
    void ThrowError(const CPSG_ReplyItem* item, const CJsonNode& annot_info) const;

    TActions m_Actions;
};

template <class TResult>
TResult SAnnotInfoProcessor<TResult>::operator()(const CPSG_ReplyItem* item, const CJsonNode& data) const
{
    auto annot_info_str(NStr::Unescape(data.GetString("annot_info")));
    auto annot_info(CJsonNode::ParseJSON(annot_info_str));

    if (!annot_info.IsObject()) ThrowError(item, annot_info);

    TResult result;

    for (CJsonIterator it = annot_info.Iterate(); it.IsValid(); it.Next()) {
        auto key = stoi(it.GetKey());
        auto found = m_Actions.find(key);

        if (found != m_Actions.end()) {
            auto node = it.GetNode();

            if (!found->second(node, result)) {
                ThrowError(item, annot_info);
            }
        }
    }

    return result;
}

template <class TResult>
void SAnnotInfoProcessor<TResult>::ThrowError(const CPSG_ReplyItem* item, const CJsonNode& annot_info) const
{
    _ASSERT(item);

    auto reply = item->GetReply();
    _ASSERT(reply);

    auto request = reply->GetRequest().get();
    _ASSERT(request);

    NCBI_THROW_FMT(CPSG_Exception, eServerError, "Wrong annot_info format: '" << annot_info.Repr() <<
            "' for " << request->GetType() << " request '" << request->GetId() << '\'');
}

bool s_GetZoomLevels(const CJsonNode& annot_data, CPSG_NamedAnnotInfo::TZoomLevels& result)
{
    if (!annot_data.IsArray()) return false;

    for (CJsonIterator it = annot_data.Iterate(); it.IsValid(); it.Next()) {
        auto zoom_level = it.GetNode();

        if (!zoom_level.IsInteger()) return false;

        result.push_back(zoom_level.AsInteger());
    }

    return true;
}

template <objects::CSeq_annot::C_Data::E_Choice kAnnot>
bool s_GetTypeAndSubtype(const CJsonNode& annot_data, CPSG_NamedAnnotInfo::TAnnotInfoList& result)
{
    if (!annot_data.IsObject()) return false;

    for (CJsonIterator it = annot_data.Iterate(); it.IsValid(); it.Next()) {
        auto type = stoi(it.GetKey());
        auto subtypes = it.GetNode();

        if (!subtypes.IsArray()) return false;

        for (CJsonIterator sub_it = subtypes.Iterate(); sub_it.IsValid(); sub_it.Next()) {
            auto subtype_node = sub_it.GetNode();

            if (!subtype_node.IsInteger()) return false;

            auto subtype = static_cast<int>(subtype_node.AsInteger());
            result.push_back({ kAnnot, type, subtype });
        }
    }

    return true;
}

template <objects::CSeq_annot::C_Data::E_Choice kAnnot>
bool s_GetType(const CJsonNode& annot_data, CPSG_NamedAnnotInfo::TAnnotInfoList& result)
{
    if (!annot_data.IsArray()) return false;

    for (CJsonIterator it = annot_data.Iterate(); it.IsValid(); it.Next()) {
        auto type_node = it.GetNode();

        if (!type_node.IsInteger()) return false;

        auto type = static_cast<int>(type_node.AsInteger());
        result.push_back({ kAnnot, type, 0 });
    }

    return true;
}

CPSG_NamedAnnotInfo::TZoomLevels CPSG_NamedAnnotInfo::GetZoomLevels() const
{
    static SAnnotInfoProcessor<TZoomLevels> processor
    ({
        { 2048, &s_GetZoomLevels } // Special value for DensityGraph
    });

    return processor(this, m_Data);
}

CPSG_NamedAnnotInfo::TAnnotInfoList CPSG_NamedAnnotInfo::GetAnnotInfoList() const
{
    using TAnnotType = SAnnotInfo::TAnnotType;

    static SAnnotInfoProcessor<TAnnotInfoList> processor
    ({
        { TAnnotType::e_Ftable,    &s_GetTypeAndSubtype<TAnnotType::e_Ftable>    },
        { TAnnotType::e_Align,     &s_GetType          <TAnnotType::e_Align>     },
        { TAnnotType::e_Graph,     &s_GetType          <TAnnotType::e_Graph>     },
        { TAnnotType::e_Seq_table, &s_GetTypeAndSubtype<TAnnotType::e_Seq_table> },
    });

    return processor(this, m_Data);
}


CPSG_Reply::CPSG_Reply() :
    m_Impl(new SImpl)
{
}

EPSG_Status CPSG_Reply::GetStatus(CDeadline deadline) const
{
    assert(m_Impl);

    return s_GetStatus(&m_Impl->reply->reply_item, deadline);
}

string CPSG_Reply::GetNextMessage() const
{
    assert(m_Impl);
    assert(m_Impl->reply);

    return m_Impl->reply->reply_item.GetLock()->state.GetError();
}

shared_ptr<CPSG_ReplyItem> CPSG_Reply::GetNextItem(CDeadline deadline)
{
    assert(m_Impl);
    assert(m_Impl->reply);

    auto& reply_item = m_Impl->reply->reply_item;
    auto& state = reply_item.GetMTSafe().state;

    do {
        bool was_in_progress = state.InProgress();

        if (auto items_locked = m_Impl->reply->items.GetLock()) {
            auto& items = *items_locked;

            for (auto& item_ts : items) {
                const auto& state = item_ts.GetMTSafe().state;

                if (state.Returned()) continue;

                if (state.Empty()) {
                    auto item_locked = item_ts.GetLock();
                    auto& item = *item_locked;

                    // Wait for more chunks on this item
                    if (!item.expected.Cmp<less_equal>(item.received)) continue;
                }

                // Do not hold lock on item_ts around this call!
                return m_Impl->Create(&item_ts);
            }
        }

        // No more reply items
        if (!was_in_progress) {
            return shared_ptr<CPSG_ReplyItem>(new CPSG_ReplyItem(CPSG_ReplyItem::eEndOfReply));
        }
    }
    while (reply_item.WaitUntil(state.GetState(), deadline, SPSG_Reply::SState::eInProgress, true));

    return {};
}

CPSG_Reply::~CPSG_Reply()
{
}


CPSG_Queue::CPSG_Queue() = default;
CPSG_Queue::CPSG_Queue(CPSG_Queue&&) = default;
CPSG_Queue& CPSG_Queue::operator=(CPSG_Queue&&) = default;
CPSG_Queue::~CPSG_Queue() = default;

CPSG_Queue::CPSG_Queue(const string& service) :
    m_Impl(new SImpl(service))
{
}

bool CPSG_Queue::SendRequest(shared_ptr<CPSG_Request> request, CDeadline deadline)
{
    _ASSERT(m_Impl);
    return m_Impl->SendRequest(const_pointer_cast<const CPSG_Request>(request), deadline);
}

shared_ptr<CPSG_Reply> CPSG_Queue::GetNextReply(CDeadline deadline)
{
    _ASSERT(m_Impl);

    shared_ptr<CPSG_Reply> rv;
    m_Impl->Pop(rv, deadline);
    return rv;
}

void CPSG_Queue::Stop()
{
    _ASSERT(m_Impl);
    m_Impl->Stop(CPSG_Queue::SImpl::eDrain);
}

void CPSG_Queue::Reset()
{
    _ASSERT(m_Impl);
    m_Impl->Stop(CPSG_Queue::SImpl::eClear);
}

bool CPSG_Queue::IsEmpty() const
{
    _ASSERT(m_Impl);
    return m_Impl->Empty();
}


END_NCBI_SCOPE

#endif
