#ifndef READER_ID2_BASE__HPP_INCLUDED
#define READER_ID2_BASE__HPP_INCLUDED

/*  $Id: reader_id2_base.hpp 568299 2018-08-02 14:49:59Z vasilche $
* ===========================================================================
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
* ===========================================================================
*
*  Author:  Eugene Vasilchenko
*
*  File Description: Data reader from ID2
*
*/

#include <objtools/data_loaders/genbank/reader.hpp>
#include <corelib/ncbitime.hpp>

BEGIN_NCBI_SCOPE

class CByteSourceReader;
class CObjectIStream;
class CObjectInfo;

BEGIN_SCOPE(objects)

class CSeq_id;
class CID2_Blob_Id;

class CID2_Request;
class CID2_Request_Packet;
class CID2_Request_Get_Seq_id;
class CID2_Request_Get_Blob_Id;
class CID2_Request_Get_Blob;
class CID2_Request_Get_Blob_Info;
class CID2_Get_Blob_Details;

class CID2_Error;
class CID2_Reply;
class CID2_Reply_Get_Seq_id;
class CID2_Reply_Get_Blob_Id;
class CID2_Reply_Get_Blob_Seq_ids;
class CID2_Reply_Get_Blob;
class CID2_Reply_Data;
class CID2S_Reply_Get_Split_Info;
class CID2S_Split_Info;
class CID2S_Reply_Get_Chunk;
class CID2S_Chunk_Id;
class CID2S_Chunk;
class CID2Processor;
class CID2ProcessorContext;
class CId2ReaderProcessorResolver;
class CReaderRequestResult;
struct SId2LoadedSet;
struct SId2PacketInfo;
struct SId2PacketReplies;
struct SId2ProcessingState;

class NCBI_XREADER_EXPORT CId2ReaderBase : public CReader
{
public:
    CId2ReaderBase(void);
    ~CId2ReaderBase(void);

    // new interface
    bool LoadSeq_idSeq_ids(CReaderRequestResult& result,
                           const CSeq_id_Handle& seq_id);
    bool LoadSeq_idGi(CReaderRequestResult& result,
                      const CSeq_id_Handle& seq_id);
    bool LoadSeq_idAccVer(CReaderRequestResult& result,
                          const CSeq_id_Handle& seq_id);
    bool LoadSeq_idLabel(CReaderRequestResult& result,
                         const CSeq_id_Handle& seq_id);
    bool LoadSeq_idTaxId(CReaderRequestResult& result,
                         const CSeq_id_Handle& seq_id);
    bool LoadSequenceHash(CReaderRequestResult& result,
                          const CSeq_id_Handle& seq_id);
    bool LoadSequenceLength(CReaderRequestResult& result,
                            const CSeq_id_Handle& seq_id);
    bool LoadSequenceType(CReaderRequestResult& result,
                          const CSeq_id_Handle& seq_id);
    bool LoadSeq_idBlob_ids(CReaderRequestResult& result,
                            const CSeq_id_Handle& seq_id,
                            const SAnnotSelector* sel);

    bool LoadAccVers(CReaderRequestResult& result,
                     const TIds& ids, TLoaded& loaded, TIds& ret);
    bool LoadGis(CReaderRequestResult& result,
                 const TIds& ids, TLoaded& loaded, TGis& ret);
    bool LoadLabels(CReaderRequestResult& result,
                    const TIds& ids, TLoaded& loaded, TLabels& ret);
    bool LoadTaxIds(CReaderRequestResult& result,
                    const TIds& ids, TLoaded& loaded, TTaxIds& ret);
    bool LoadHashes(CReaderRequestResult& result,
                    const TIds& ids, TLoaded& loaded,
                    THashes& ret, TKnown& known);
    bool LoadLengths(CReaderRequestResult& result,
                     const TIds& ids, TLoaded& loaded, TLengths& ret);
    bool LoadTypes(CReaderRequestResult& result,
                   const TIds& ids, TLoaded& loaded, TTypes& ret);
    bool LoadStates(CReaderRequestResult& result,
                    const TIds& ids, TLoaded& loaded, TStates& ret);

    bool LoadBlobState(CReaderRequestResult& result,
                       const TBlobId& blob_id);
    bool LoadBlobVersion(CReaderRequestResult& result,
                         const TBlobId& blob_id);

    bool LoadBlobs(CReaderRequestResult& result,
                   const CSeq_id_Handle& seq_id,
                   TContentsMask mask,
                   const SAnnotSelector* sel);
    bool LoadBlobs(CReaderRequestResult& result,
                   const CLoadLockBlobIds& blobs,
                   TContentsMask mask,
                   const SAnnotSelector* sel);
    bool LoadBlob(CReaderRequestResult& result,
                  const TBlobId& blob_id);
    bool LoadChunk(CReaderRequestResult& result,
                   const TBlobId& blob_id, TChunkId chunk_id);
    bool LoadChunks(CReaderRequestResult& result,
                    const TBlobId& blob_id,
                    const TChunkIds& chunk_ids);
    bool LoadBlobSet(CReaderRequestResult& result,
                     const TSeqIds& seq_ids);

    static TBlobId GetBlobId(const CID2_Blob_Id& blob_id);
    
    enum EDebugLevel
    {
        eTraceError    = 1,
        eTraceOpen     = 2,
        eTraceConn     = 4,
        eTraceASN      = 5,
        eTraceBlob     = 8,
        eTraceBlobData = 9
    };
    static int GetDebugLevel(void);

protected:
    virtual string x_ConnDescription(TConn conn) const = 0;

    virtual void x_SendPacket(TConn conn, const CID2_Request_Packet& packet)=0;
    virtual void x_ReceiveReply(TConn conn, CID2_Reply& reply) = 0;
    void x_ReceiveReply(CObjectIStream& stream, TConn conn, CID2_Reply& reply);

    virtual void x_EndOfPacket(TConn conn);

    void x_SetResolve(CID2_Request_Get_Blob_Id& get_blob_id,
                      const CSeq_id& seq_id);
    void x_SetResolve(CID2_Blob_Id& blob_id, const CBlob_id& src);

    void x_SetDetails(CID2_Get_Blob_Details& details,
                      TContentsMask mask);

    void x_SetExclude_blobs(CID2_Request_Get_Blob_Info& get_blob_info,
                            const CSeq_id_Handle& idh,
                            CReaderRequestResult& result);

    void x_ProcessRequest(CReaderRequestResult& result,
                          CID2_Request& req,
                          const SAnnotSelector* sel);
    void x_ProcessPacket(CReaderRequestResult& result,
                         CID2_Request_Packet& packet,
                         const SAnnotSelector* sel);

    enum EErrorFlags {
        fError_warning              = 1 << 0,
        fError_no_data              = 1 << 1,
        fError_bad_command          = 1 << 2,
        fError_bad_connection       = 1 << 3,
        fError_warning_dead         = 1 << 4,
        fError_restricted           = 1 << 5,
        fError_withdrawn            = 1 << 6,
        fError_suppressed_perm      = 1 << 7,
        fError_suppressed_temp      = 1 << 8,
        fError_inactivity_timeout   = 1 << 9
    };
    typedef int TErrorFlags;
    TErrorFlags x_GetError(CReaderRequestResult& result,
                           const CID2_Error& error);
    TErrorFlags x_GetMessageError(const CID2_Error& error);
    static void sx_CheckErrorFlag(const CID2_Error& error,
                                  TErrorFlags& error_flags,
                                  EErrorFlags test_flag,
                                  const char* marker1,
                                  const char* marker2 = 0);
    TErrorFlags x_GetError(CReaderRequestResult& result,
                           const CID2_Reply& reply);
    TErrorFlags x_GetMessageError(const CID2_Reply& reply);
    TBlobState x_GetBlobStateFromID2(const CBlob_id& blob_id,
                                     SId2LoadedSet& loaded_set,
                                     int id2_state);
    TBlobState x_GetBlobState(const CBlob_id& blob_id,
                              SId2LoadedSet& loaded_set,
                              const CID2_Reply& reply,
                              TErrorFlags* errors_ptr = 0);

    void x_ProcessReply(CReaderRequestResult& result,
                        SId2LoadedSet& loaded_set,
                        const CID2_Reply& main_reply,
                        const CID2_Request& main_request);
    void x_ProcessEmptyReply(CReaderRequestResult& result,
                             SId2LoadedSet& loaded_set,
                             const CID2_Reply& main_reply,
                             const CID2_Request& main_request);
    void x_ProcessGetSeqId(CReaderRequestResult& result,
                           SId2LoadedSet& loaded_set,
                           const CID2_Reply& main_reply,
                           const CID2_Request_Get_Seq_id& request,
                           const CID2_Reply_Get_Seq_id* reply);
    void x_ProcessGetSeqIdSeqId(CReaderRequestResult& result,
                                SId2LoadedSet& loaded_set,
                                const CID2_Reply& main_reply,
                                const CSeq_id_Handle& seq_id,
                                const CID2_Request_Get_Seq_id& request,
                                const CID2_Reply_Get_Seq_id* reply);
    void x_ProcessGetBlobId(CReaderRequestResult& result,
                            SId2LoadedSet& loaded_set,
                            const CID2_Reply& main_reply,
                            const CID2_Reply_Get_Blob_Id& reply);
    void x_ProcessGetBlobSeqIds(CReaderRequestResult& result,
                                SId2LoadedSet& loaded_set,
                                const CID2_Reply& main_reply,
                                const CID2_Reply_Get_Blob_Seq_ids& reply);
    void x_ProcessGetBlob(CReaderRequestResult& result,
                          SId2LoadedSet& loaded_set,
                          const CID2_Reply& main_reply,
                          const CID2_Reply_Get_Blob& reply);
    void x_ProcessGetSplitInfo(CReaderRequestResult& result,
                               SId2LoadedSet& loaded_set,
                               const CID2_Reply& main_reply,
                               const CID2S_Reply_Get_Split_Info& reply);
    void x_ProcessGetChunk(CReaderRequestResult& result,
                           SId2LoadedSet& loaded_set,
                           const CID2_Reply& main_reply,
                           const CID2S_Reply_Get_Chunk& reply);

    void x_UpdateLoadedSet(CReaderRequestResult& result,
                           SId2LoadedSet& data,
                           const SAnnotSelector* sel);

    bool x_LoadSeq_idBlob_idsSet(CReaderRequestResult& result,
                                 const TSeqIds& seq_ids);

    friend class CId2ReaderProcessorResolver;

    void x_DumpPacket(TConn conn, const CID2_Request_Packet& packet, const char* msg = "Sending");
    void x_DumpReply(TConn conn, CID2_Reply& reply, const char* msg = "Received");
    void x_SetContextData(CID2_Request& request);
    void x_SendToConnection(TConn conn, CID2_Request_Packet& packet);
    void x_SendID2Packet(CReaderRequestResult& result,
                         SId2ProcessingState& state,
                         CID2_Request_Packet& packet);
    CRef<CID2_Reply> x_ReceiveFromConnection(TConn conn);
    CRef<CID2_Reply> x_ReceiveID2ReplyStage(SId2ProcessingState& state, size_t pos);
    CRef<CID2_Reply> x_ReceiveID2Reply(SId2ProcessingState& state);
    void x_AssignSerialNumbers(SId2PacketInfo& info,
                               CID2_Request_Packet& packet);
    int x_GetReplyIndex(CReaderRequestResult& result,
                        CConn* conn,
                        SId2PacketInfo& packet,
                        const CID2_Reply& reply);
    bool x_DoneReply(SId2PacketInfo& info,
                     int num,
                     const CID2_Reply& reply);

    void x_DisableProcessors();

private:
    CAtomicCounter_WithAutoInit m_RequestSerialNumber;

    enum {
        fAvoidRequest_nested_get_blob_info = 1<<0,
        fAvoidRequest_for_Seq_id_label     = 1<<1,
        fAvoidRequest_for_Seq_id_taxid     = 1<<2,
        fAvoidRequest_for_Seq_id_hash      = 1<<3,
        fAvoidRequest_for_Seq_id_length    = 1<<4,
        fAvoidRequest_for_Seq_id_type      = 1<<5
    };
    typedef int TAvoidRequests;
    TAvoidRequests m_AvoidRequest;

    struct SProcessorInfo {
        CRef<CID2Processor> processor;
        CRef<CID2ProcessorContext> context;
    };
    typedef vector<SProcessorInfo> TProcessors;
    TProcessors m_Processors;
};


END_SCOPE(objects)
END_NCBI_SCOPE

#endif // READER_ID2_BASE__HPP_INCLUDED
