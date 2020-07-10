#ifndef OBJTOOLS__PUBSEQ_GATEWAY__PSG_CLIENT_HPP
#define OBJTOOLS__PUBSEQ_GATEWAY__PSG_CLIENT_HPP

/*  $Id: psg_client.hpp 599706 2020-01-06 18:04:04Z sadyrovr $
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
 * Authors: Denis Vakatov (design), Rafael Sadyrov (implementation)
 *
 */

#include <corelib/ncbitime.hpp>
#include <corelib/ncbi_url.hpp>
#include <corelib/request_ctx.hpp>
#include <connect/services/json_over_uttp.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objects/seq/Seq_annot.hpp>


#if defined(NCBI_THREADS) && defined(HAVE_LIBNGHTTP2) && defined(HAVE_LIBUV)
#  define HAVE_PSG_CLIENT 1
#endif


#if defined(HAVE_PSG_CLIENT)
BEGIN_NCBI_SCOPE



class CPSG_Exception : public CException
{
public:
    enum EErrCode {
        eTimeout,
        eServerError,
        eInternalError,
        eParameterMissing
    };

    virtual const char* GetErrCodeString(void) const override;

    NCBI_EXCEPTION_DEFAULT(CPSG_Exception, CException);
};



/// Request to the PSG server (see "CPSG_Request_*" below)
///

class CPSG_Request
{
public:
    /// Get the user-provided context
    template<typename TUserContext>
    shared_ptr<TUserContext> GetUserContext() const
    { return static_pointer_cast<TUserContext>(m_UserContext); }

    /// Get request context
    CRef<CRequestContext> GetRequestContext() const { return m_RequestContext; }

    /// Get request type
    string GetType() const { return x_GetType(); }

    // Get request ID
    string GetId() const { return x_GetId(); }

protected:
    CPSG_Request(shared_ptr<void> user_context = {},
                 CRef<CRequestContext> request_context = {})
        : m_UserContext(user_context),
          m_RequestContext(request_context)
    {}

    virtual ~CPSG_Request() = default;

    struct x_GetBioIdParams;

private:
    virtual string x_GetType() const = 0;
    virtual string x_GetId() const = 0;
    virtual void x_GetAbsPathRef(ostream&) const = 0;

    shared_ptr<void> m_UserContext;
    CRef<CRequestContext> m_RequestContext;

    friend class CPSG_Queue;
};



/// Bio-id (such as accession)
///
class CPSG_BioId
{
public:
    using TType = objects::CSeq_id::E_Choice;

    /// @param id
    ///  Bio ID (like accession)
    CPSG_BioId(string id, TType type = {}) : m_Id(move(id)), m_Type(type) {}

    const string& Get()     const { return m_Id; }

private:
    string m_Id;
    TType  m_Type;

    friend class CPSG_Request;
};



/// Data blob unique ID
///
class CPSG_BlobId
{
public:
    /// Mainstream blob ID ctor - from a string ID
    /// @param id
    ///  Blob ID
    CPSG_BlobId(string id) : m_Id(move(id)) {}

    /// Historical blob ID system -- based on the "satellite" and the "key"
    /// inside it. It'll be translated into "<sat>.<sat_key>" string.
    /// @sa  objects::CID2_Blob_Id::TSat, objects::CID2_Blob_Id::TSat_key
    CPSG_BlobId(int sat, int sat_key) : m_Id(to_string(sat) + "." + to_string(sat_key)) {}

    /// Get the blob ID
    const string& Get() const { return m_Id; }

private:
    string m_Id;
};



/// Whether and how to substitute version-less primary seq-ids with
/// the "more unique" secondary seq-ids
enum class EPSG_AccSubstitution {
    Default,  ///< Substitute always (default)
    Limited,  ///< Substitute only if the resolved record's seq_id_type is GI(12)
    Never     ///< No substitution whatsoever - return exact raw accession info
};



/// Request to the PSG server (by bio-id, for a biodata specific info and data)
///

class CPSG_Request_Biodata : public CPSG_Request
{
public:
    /// 
    CPSG_Request_Biodata(CPSG_BioId       bio_id,
                         shared_ptr<void> user_context = {},
                         CRef<CRequestContext> request_context = {})
        : CPSG_Request(user_context, request_context),
          m_BioId(bio_id)
    {}

    const CPSG_BioId& GetBioId() const { return m_BioId; }

    /// Specify which info and data is needed
    enum EIncludeData {
        /// Server default
        eDefault,

        /// Only the info
        eNoTSE,

        /// If ID2 split is available, return split info blob only.
        /// Otherwise, return no data.
        eSlimTSE,

        /// If ID2 split is available, return split info blob only.
        /// Otherwise, return all Cassandra data chunks of the blob itself.
        eSmartTSE,

        /// If ID2 split is available, return all split blobs.
        /// Otherwise, return all Cassandra data chunks of the blob itself.
        eWholeTSE,

        /// Return all Cassandra data chunks of the blob itself.
        eOrigTSE
    };
    void IncludeData(EIncludeData include) { m_IncludeData = include; }

    EIncludeData GetIncludeData() const { return m_IncludeData; }

    using TExcludeTSEs = vector<CPSG_BlobId>;

    void ExcludeTSE(CPSG_BlobId blob_id) { m_ExcludeTSEs.emplace_back(move(blob_id)); }

    const TExcludeTSEs& GetExcludeTSEs() const { return m_ExcludeTSEs; }

    /// Set substitution policy for version-less primary seq-ids
    void SetAccSubstitution(EPSG_AccSubstitution acc_substitution) { m_AccSubstitution = acc_substitution; }

private:
    string x_GetType() const override { return "biodata"; }
    string x_GetId() const override { return GetBioId().Get(); }
    void x_GetAbsPathRef(ostream&) const override;

    CPSG_BioId    m_BioId;
    EIncludeData  m_IncludeData = EIncludeData::eDefault;
    TExcludeTSEs  m_ExcludeTSEs;
    EPSG_AccSubstitution m_AccSubstitution = EPSG_AccSubstitution::Default;
};



/// Request to the PSG server (by bio-id, for a biodata specific info and data)
///

class CPSG_Request_Resolve : public CPSG_Request
{
public:
    /// 
    CPSG_Request_Resolve(CPSG_BioId       bio_id,
                         shared_ptr<void> user_context = {},
                         CRef<CRequestContext> request_context = {})
        : CPSG_Request(user_context, request_context),
          m_BioId(bio_id)
    {}

    const CPSG_BioId& GetBioId() const { return m_BioId; }

    /// Specify which info and data is needed
    enum EIncludeInfo : unsigned {
        // These flags correspond exactly to the CPSG_BioseqInfo's getters
        fCanonicalId      = (1 << 1),
        fName             = (1 << 2), ///< Requests name to use for canonical bio-id
        fOtherIds         = (1 << 3),
        fMoleculeType     = (1 << 4),
        fLength           = (1 << 5),
        fChainState       = (1 << 6),
        fState            = (1 << 7),
        fBlobId           = (1 << 8),
        fTaxId            = (1 << 9),
        fHash             = (1 << 10),
        fDateChanged      = (1 << 11),
        fGi               = (1 << 12),
        fAllInfo          = numeric_limits<unsigned>::max()
    };
    DECLARE_SAFE_FLAGS_TYPE(EIncludeInfo, TIncludeInfo);
    void IncludeInfo(TIncludeInfo include) { m_IncludeInfo = include; }

    TIncludeInfo      GetIncludeInfo() const { return m_IncludeInfo; }

    /// Set substitution policy for version-less primary seq-ids
    void SetAccSubstitution(EPSG_AccSubstitution acc_substitution) { m_AccSubstitution = acc_substitution; }

private:
    string x_GetType() const override { return "resolve"; }
    string x_GetId() const override { return GetBioId().Get(); }
    void x_GetAbsPathRef(ostream&) const override;

    CPSG_BioId    m_BioId;
    TIncludeInfo  m_IncludeInfo = 0;
    EPSG_AccSubstitution m_AccSubstitution = EPSG_AccSubstitution::Default;
};



/// Request to the PSG server (by blob-id, for a particular blob of data)
///

class CPSG_Request_Blob : public CPSG_Request
{
public:
    /// 
    CPSG_Request_Blob(CPSG_BlobId      blob_id,
                      string           last_modified = {},
                      shared_ptr<void> user_context = {},
                      CRef<CRequestContext> request_context = {})
        : CPSG_Request(user_context, request_context),
          m_BlobId(blob_id),
          m_LastModified(last_modified)
    {}

    const CPSG_BlobId& GetBlobId()       const { return m_BlobId; }
    const string&      GetLastModified() const { return m_LastModified; }

    /// Specify which data is needed (info is always returned)
    using EIncludeData = CPSG_Request_Biodata::EIncludeData;
    void IncludeData(EIncludeData include) { m_IncludeData = include; }

    EIncludeData GetIncludeData() const { return m_IncludeData; }

private:
    string x_GetType() const override { return "blob"; }
    string x_GetId() const override { return GetBlobId().Get(); }
    void x_GetAbsPathRef(ostream&) const override;

    CPSG_BlobId  m_BlobId;
    string       m_LastModified;
    EIncludeData m_IncludeData = EIncludeData::eDefault;
};



/// Request meta-information for the named annotations which are defined on the
/// bioseq
///

class CPSG_Request_NamedAnnotInfo : public CPSG_Request
{
public:
    /// Names of the named annotations
    using TAnnotNames = vector<string>;

    /// @param bio_id
    ///  ID of the bioseq
    /// @param annot_names
    ///  List of NAs for which to request the metainfo
    CPSG_Request_NamedAnnotInfo(CPSG_BioId       bio_id,
                                TAnnotNames      annot_names,
                                shared_ptr<void> user_context = {},
                                CRef<CRequestContext> request_context = {})
        : CPSG_Request(user_context, request_context),
          m_BioId(bio_id),
          m_AnnotNames(annot_names)
    {}

    const CPSG_BioId&  GetBioId()      const { return m_BioId;      }
    const TAnnotNames& GetAnnotNames() const { return m_AnnotNames; }

    /// Set substitution policy for version-less primary seq-ids
    void SetAccSubstitution(EPSG_AccSubstitution acc_substitution) { m_AccSubstitution = acc_substitution; }

private:
    string x_GetType() const override { return "annot"; }
    string x_GetId() const override { return GetBioId().Get(); }
    void x_GetAbsPathRef(ostream&) const override;

    CPSG_BioId  m_BioId;
    TAnnotNames m_AnnotNames;
    EPSG_AccSubstitution m_AccSubstitution = EPSG_AccSubstitution::Default;
};



/// Request to the PSG server
/// (by TSE blob-id and chunk number, for a particular version of split)
///

class CPSG_Request_TSE_Chunk : public CPSG_Request
{
public:
    using TChunkNo = unsigned;
    using TSplitVersion = int;

    CPSG_Request_TSE_Chunk(CPSG_BlobId      tse_blob_id,
                           TChunkNo         chunk_no,
                           TSplitVersion    split_version,
                           shared_ptr<void> user_context = {},
                           CRef<CRequestContext> request_context = {})
        : CPSG_Request(user_context, request_context),
          m_TSE_BlobId(tse_blob_id),
          m_ChunkNo(chunk_no),
          m_SplitVersion(split_version)
    {}

    const CPSG_BlobId&  GetTSE_BlobId()   const { return m_TSE_BlobId;   }
    TChunkNo            GetChunkNo()      const { return m_ChunkNo;      }
    TSplitVersion       GetSplitVersion() const { return m_SplitVersion; }

private:
    string x_GetType() const override { return "tse_chunk"; }
    string x_GetId() const override { return GetTSE_BlobId().Get(); }
    void x_GetAbsPathRef(ostream&) const override;

    CPSG_BlobId    m_TSE_BlobId;
    TChunkNo       m_ChunkNo;
    TSplitVersion  m_SplitVersion;
};



/// Retrieval result
/// @sa GetStatus
enum class EPSG_Status {
    eSuccess,       ///< Successfully retrieved
    eInProgress,    ///< Retrieval is not finalized yet, more info may come
    eNotFound,      ///< Not found
    eCanceled,      ///< Request canceled

    /// An error was encountered while trying to send request or to read
    /// and to process the reply.
    /// If PSG server sends a message with severity:
    /// - Error, Critical or Fatal -- this status will be set, and any data
    ///   data in the reply item must be considered invalid; such messages
    ///   will also be logged by the client API with severity Error.
    /// - Trace, Info or Warning -- are considered to be informational, so
    ///   these do NOT affect the status; such messages however will still
    ///   be logged by the client API with the same (T, I or W) severity. 
    eError
};



class CPSG_Reply;



/// A self-containing part of the reply, e.g. a meta-data or a data blob.

class CPSG_ReplyItem
{
public:
    enum EType {
        eBlobData,
        eBlobInfo,
        eSkippedBlob,
        eBioseqInfo,
        eNamedAnnotInfo,
        eEndOfReply,    ///< No more items expected in the (overall!) reply
    };

    EType GetType() const { return m_Type; }

    /// Get the final result of this blob's retrieval.
    /// If the blob retrieval is not finalized by the deadline, then
    /// "eInProgress" is returned.
    EPSG_Status GetStatus(CDeadline deadline) const;

    /// Unstructured text containing auxiliary info about the result --
    /// such as messages and errors that came from the PSG server or occured
    /// while trying to send request or to read and to process the reply.
    string GetNextMessage() const;

    /// Get the reply that contains this item
    shared_ptr<CPSG_Reply> GetReply() const { return m_Reply; }

    virtual ~CPSG_ReplyItem();

protected:
    CPSG_ReplyItem(EType type);

private:
    struct SImpl;
    unique_ptr<SImpl>      m_Impl;
    shared_ptr<CPSG_Reply> m_Reply;
    const EType            m_Type;

    friend class CPSG_Reply;
};



/// Data blob.

class CPSG_BlobData : public CPSG_ReplyItem
{
public:
    /// Get blob ID
    const CPSG_BlobId& GetId() const { return m_Id; }

    /// Get the stream from which to read the item's content.
    /// @note  If no content, then reading from the stream will result in EOF.
    istream& GetStream() const { return *m_Stream; }

private:
    CPSG_BlobData(CPSG_BlobId id);

    CPSG_BlobId         m_Id;
    unique_ptr<istream> m_Stream;

    friend class CPSG_Reply;
};



/// Data blob meta information
///
/// @note  Most of the data comes from table "BIOSEQ_BLOB" or "ANNOT_BLOB".

class CPSG_BlobInfo : public CPSG_ReplyItem
{
public:
    /// Get blob ID
    const CPSG_BlobId& GetId() const { return m_Id; }

    /// Get data compression algorithm: gzip, bzip2, zip, compress, nlmzip, ...
    /// Return empty string if the blob is not compressed
    string GetCompression() const;

    /// Get data serialization format:  asn.1, asn1-text, json, xml, ...
    string GetFormat() const;

    /// Get blob version (the larger the version the fresher the blob data)
    Uint8 GetVersion() const;

    /// Get size of the blob (as it is stored)
    Uint8 GetStorageSize() const;

    /// Get size of the real (before any compression or encryption) blob data
    Uint8 GetSize() const;

    /// Return TRUE if the blob is "dead"
    bool IsDead() const;

    /// Return TRUE if the blob is "suppressed"
    bool IsSuppressed() const;

    /// Return TRUE if the blob is "withdrawn"
    bool IsWithdrawn() const;

    /// Date when the blob will be released for public use.
    /// If the blob is already released, then return "empty" (IsEmpty()) time
    CTime GetHupReleaseDate() const;

    /// Blob owner's ID
    Uint8 GetOwner() const;

    /// Date when the blob was first loaded into the database
    CTime GetOriginalLoadDate() const;

    /// Class of this blob
    objects::CBioseq_set::EClass GetClass() const;

    /// Internal division value (used by various dumpers)
    string GetDivision() const;

    /// Name of the user who loaded this blob
    string GetUsername() const;

    /// Get coordinates of the blob that contains the specified ID2 split info.
    /// If the blob is not split, then return an empty blob id.
    CPSG_BlobId GetSplitInfoBlobId() const;

    /// Get coordinates of a chunk blob -- from the chunk's serial number.
    /// @throw  If the blob has not been splitted.
    CPSG_BlobId GetChunkBlobId(unsigned split_chunk_no) const;

    /// Return ID2 split version (or zero, if not available)
    using TSplitVersion = int;
    TSplitVersion GetSplitVersion() const;

private:
    CPSG_BlobInfo(CPSG_BlobId id);

    CPSG_BlobId m_Id;
    CJsonNode m_Data;

    friend class CPSG_Reply;
};



/// Skipped blob.

class CPSG_SkippedBlob : public CPSG_ReplyItem
{
public:
    enum EReason {
        eExcluded,   // Explicitly excluded by the client
        eInProgress, // Is being sent to the client
        eSent,       // Already sent to the client
        eUnknown,    // Skipped for unknown reason
    };

    /// Get blob ID
    const CPSG_BlobId& GetId() const { return m_Id; }

    // Get reason for blob skipping
    EReason GetReason() const { return m_Reason; }

private:
    CPSG_SkippedBlob(CPSG_BlobId id, EReason reason);

    CPSG_BlobId m_Id;
    EReason     m_Reason;

    friend class CPSG_Reply;
};



/// Bio-sequence metainfo -- result of the bio-id resolution.
///
/// It can be used to identify which data blobs (related to the requested
/// bio-id retrieval) server is sending right away. It also contains
/// resolution information as well as the information about which
/// other biodata-related blobs are also available on the server and how
/// they can be explicitly requested for later retrieval, if needed.
///
/// @note
///  Most of the data comes from table "BIOSEQ_INFO" and from the named
///  annotation tables.

class CPSG_BioseqInfo : public CPSG_ReplyItem
{
public:
    /// Get canonical bio-id for the bioseq (usually "accession.version")
    CPSG_BioId GetCanonicalId() const;

    /// Get non-canonical bio-ids (aliases) for the bioseq
    vector<CPSG_BioId> GetOtherIds() const;

    /// The bioseq's molecule type (DNA, RNA, protein, etc)
    objects::CSeq_inst::TMol GetMoleculeType() const;

    /// Length of bio-sequence
    Uint8 GetLength() const;

    /// State of the bio-sequence's seq-id
    enum EState {
        eDead     =  0,
        eSought   =  1,
        eReserved =  5,
        eMerged   =  7,
        eLive     = 10
    };
    typedef int TState;  ///< @sa EState

    /// State of the bio-sequence's seq-id chain, i.e. the state of the very
    /// latest seq-id in this bio-sequence's seq-id chain
    TState GetChainState() const;

    /// State of this exact bio-sequence's seq-id.
    /// I.e., for the latest seq-id in a chain it is equal to GetState(), and
    /// for all other seq-ids in a chain it's zero (eDead).
    TState GetState() const;

    /// Get coordinates of the TSE blob that contains the bioseq itself
    CPSG_BlobId GetBlobId() const;

    /// Get the bioseq's taxonomy ID
    TTaxId GetTaxId() const;

    /// Get the bioseq's (pre-calculated) hash
    int GetHash() const;

    /// Date when the bioseq was changed last time
    CTime GetDateChanged() const;

    /// Get GI
    TGi GetGi() const;

    /// What data is immediately available now. Other data will require
    /// a separate hit to the server.
    /// @sa CPSG_Request_Resolve::IncludeInfo()
    CPSG_Request_Resolve::TIncludeInfo IncludedInfo() const;

private:
    CPSG_BioseqInfo();

    CJsonNode m_Data;

    friend class CPSG_Reply;
};



/// Named Annotations (NAs) metainfo -- reply to CPSG_Request_NamedAnnotInfo.
///
/// It can be used to identify where various types of requested NAs are located
/// on the bioseq. It also provides information how to retrieve the
/// corresponding NA data blobs (as needed).

class CPSG_NamedAnnotInfo : public CPSG_ReplyItem
{
public:
    /// Name of the annotation
    const string& GetName() const { return m_Name; }

    /// Range where the feature(s) from this NA appear on the bio-sequence
    CRange<TSeqPos> GetRange() const;

    /// Coordinates of the blob that contains the NA data
    CPSG_BlobId GetBlobId() const;

    /// Get NA version (the larger the version the fresher the NA data)
    Uint8 GetVersion() const;

    /// Available zoom levels
    using TZoomLevel  = unsigned int;
    using TZoomLevels = vector<TZoomLevel>;
    TZoomLevels GetZoomLevels() const;

    /// 
    struct SAnnotInfo
    {
        using TAnnotType = objects::CSeq_annot::C_Data::E_Choice;

        TAnnotType annot_type;
        int        feat_type;
        int        feat_subtype;
    };

    using TAnnotInfoList = list<SAnnotInfo>;
    TAnnotInfoList GetAnnotInfoList() const;

private:
    CPSG_NamedAnnotInfo(string name);

    string     m_Name;
    CJsonNode  m_Data;

    friend class CPSG_Reply;
};



/// PSG reply -- corresponds to a PSG request. It is used to retrieve data 
/// (accession resolution; bio-sequence; annotation blobs) from the storage.
/// 
/// Reply may contain:  
///  - Reply items (CPSG_ReplyItem), each of which in turn may contain
///    item-specific info and/or data blob
///  - Server messages related to the whole reply  
/// 

class CPSG_Reply
{
public:
    /// Get the final result of this whole reply's retrieval.
    /// If the reply retrieval is not finalized by the deadline, then
    /// "eInProgress" is returned.
    EPSG_Status GetStatus(CDeadline deadline) const;

    /// Unstructured text containing auxiliary info about the result --
    /// such as messages and errors that came from the PSG server or occured
    /// while trying to send request or to read and to process the reply.
    string GetNextMessage() const;

    /// Get the request that resulted in this reply
    shared_ptr<const CPSG_Request> GetRequest() const { return m_Request; }

    /// Get the next item which has started arriving from the server.
    /// @note
    ///  Some of the item's data may still be in transit or not even sent
    ///  in by the server yet.
    /// @param deadline
    ///  Until what time to wait for the next item to start coming in.
    /// @return
    ///  - The item objects from which you can start reading data
    ///  - If no more items expected in the reply, the returned item will have
    ///    type eEndOfReply
    ///  - On expired timeout, the returned pointer will be empty (nullptr)
    /// @throw
    ///  If an error has been detected.
    shared_ptr<CPSG_ReplyItem> GetNextItem(CDeadline deadline);

    ~CPSG_Reply();

private:
    CPSG_Reply();

    struct SImpl;
    unique_ptr<SImpl>              m_Impl;
    shared_ptr<const CPSG_Request> m_Request;

    friend class CPSG_Queue;
};



/// A queue to retrieve data (accession resolution info; bio-sequence;
/// annotation blobs) from the storage.
///
/// Call SendRequest() to schedule retrievals (by their bio-ids or
/// blob-ids). Then, call GetNextReply() to get the next reply whose data
/// has started coming in.
///
/// All methods are MT-safe.  Data from different replies can be read in
/// parallel.
///
/// The queue object can be used from more than one thread, either to push
/// requests or to get the incoming ready-to-be-retrieved replies.
///
/// Results for the requests which were pushed into a given instance of
/// the queue will be available for retrieval using this (and only this) queue
/// instance regardless of which threads were used to push the request to the
/// queue.
///
/// If more than one request was pushed into the queue, then the replies to all
/// of the requests may come, in any order.
///

class CPSG_Queue
{
public:
    /// Creates an uninitialized instance.
    /// It allows to postpone queue initialization until later.
    /// The uninitialized instances can then be initialized using
    /// regular constructor and move assignment operator.
    CPSG_Queue();

    /// @param service
    ///  Either a name of service (which can be resolved into a set of PSG
    ///  servers) or a single fixed PSG server (in format "host:port")
    CPSG_Queue(const string& service);
    ~CPSG_Queue();

    /// Push request into the queue.
    /// @param request
    ///  The request (containing either bio- or blob-id to retrieve) to send.
    /// @param deadline
    ///  For how long to try to push the request into the queue.
    /// @return
    ///  - TRUE if it succeeds in pushing the request into the queue
    ///  - FALSE on timeout (ie. if cannot do it before the specified deadline)
    /// @throw  CPSG_Exception
    ///  If any (non-timeout) error condition occures.
    /// @sa Get()
    bool SendRequest(shared_ptr<CPSG_Request> request,
                     CDeadline                deadline);


    /// Get the next reply which has started arriving from the server.
    /// @param deadline
    ///  Until what time to wait for the next reply to start coming in.
    /// @return
    ///  - Reply object from which you can obtain particular items.
    ///  - On expired timeout, the returned pointer will be empty (nullptr).
    /// @throw
    ///  If an error has been detected.
    shared_ptr<CPSG_Reply> GetNextReply(CDeadline deadline);


    /// Stop accepting new requests.
    /// All already accepted requests will be processed as usual.
    /// No requests are accepted after the stop.
    void Stop();


    /// Stop accepting new requests and
    /// cancel all requests whose replies have not been returned yet.
    /// No requests are accepted and no replies are returned after the reset.
    void Reset();


    /// Check whether the queue was stopped/reset and is now empty.
    bool IsEmpty() const;

    CPSG_Queue(CPSG_Queue&&);
    CPSG_Queue& operator=(CPSG_Queue&&);

private:
    struct SImpl;
    unique_ptr<SImpl> m_Impl;
};


DECLARE_SAFE_FLAGS(CPSG_Request_Resolve::EIncludeInfo);

END_NCBI_SCOPE


#endif  /* HAVE_PSG_CLIENT */
#endif  /* OBJTOOLS__PUBSEQ_GATEWAY__PSG_CLIENT_HPP */
