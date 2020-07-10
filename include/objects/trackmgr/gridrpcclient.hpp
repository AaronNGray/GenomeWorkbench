#ifndef OBJECTS_MISC_SERIAL___GRID_RPC_CLIENT__HPP
#define OBJECTS_MISC_SERIAL___GRID_RPC_CLIENT__HPP

/* $Id: gridrpcclient.hpp 593123 2019-09-12 15:00:12Z grichenk $
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
 * Authors: Peter Meric, Dmitry Kazimirov
 *
 */

/// @file gridrpcclient.hpp
/// Classes pertaining to GRID-based ASN.1 RPC clients

#include <corelib/ncbiobj.hpp>
#include <corelib/stream_utils.hpp>
#include <corelib/ncbiapp_api.hpp>
#include <serial/serial.hpp>
#include <serial/objistr.hpp>
#include <serial/objostr.hpp>
#include <serial/rpcbase.hpp>
#include <connect/services/grid_worker_app.hpp>
#include <connect/services/grid_rw_impl.hpp>
#include <connect/services/grid_client.hpp>
#include <connect/ncbi_conn_stream.hpp>
#include <corelib/perf_log.hpp>
#include <corelib/rwstream.hpp>
#include <util/compress/lzo.hpp>
#include <util/compress/stream_util.hpp>
#include <util/compress/zlib.hpp>


BEGIN_NCBI_SCOPE


///
/// Traits class for binary ASN.1
///
class CAsnBin
{
public:
    /// Return the serialization type
    ///
    /// @return
    ///   data format enumeration
    static ESerialDataFormat GetDataFormat(void)
    {
        return eSerial_AsnBinary;
    }
};

///
/// Traits class for compressed binary ASN.1 streams
///
class CAsnBinCompressed : public CAsnBin
{
public:
    using TOwnership = int;

    struct SStreamProp
    {
        SStreamProp(const CCompressStream::EMethod comp_mthd = CCompressStream::eZip)
            : compress_method(comp_mthd)
        {
        }

        CCompressStream::EMethod compress_method;
    };


    /// Return an object output stream (CObjectOStream)
    ///
    /// @param ostr
    ///   underlying output stream
    /// @return
    ///   object stream
    static unique_ptr<CObjectOStream>
    GetOStream(CNcbiOstream& ostr, SStreamProp stream_prop = SStreamProp(CCompressStream::eZip))
    {
        unique_ptr<CCompressionOStream> outstr_zip(
            new CCompressionOStream(
                ostr,
                CreateStreamCompressor(stream_prop),
                CCompressionStream::fOwnProcessor
            )
        );
        return unique_ptr<CObjectOStream>(CObjectOStream::Open(GetDataFormat(), *outstr_zip.release(), eTakeOwnership));
    }

    /// Return an object input stream (CObjectIStream)
    ///
    /// @param istr
    ///   underlying input stream
    /// @return
    ///   object stream
    static unique_ptr<CObjectIStream>
    GetIStream(CNcbiIstream& istr)
    {
        return GetIStream(istr, GetIStreamProperties(istr));
    }

    static unique_ptr<CObjectIStream>
    GetIStream(const string& job_content, CNetCacheAPI& nc_api)
    {
        SStreamProp sp;
        return GetIStream(job_content, nc_api, sp);
    }

    static unique_ptr<CObjectIStream>
    GetIStream(const string& job_content, CNetCacheAPI& nc_api, SStreamProp& streamprop)
    {
        auto rstr = GetRawIStream(job_content, nc_api, streamprop);
        return GetIStream(*rstr.release(), streamprop, CCompressionStream::fOwnAll);
    }

    static unique_ptr<CNcbiIstream>
    GetRawIStream(const string& job_content, CNetCacheAPI& nc_api)
    {
        SStreamProp sp;
        return GetRawIStream(job_content, nc_api, sp);
    }

    static unique_ptr<CNcbiIstream>
    GetRawIStream(const string& job_content, CNetCacheAPI& nc_api, SStreamProp& streamprop)
    {
        streamprop = GetJobStreamProperties(job_content, nc_api);
        unique_ptr<CStringOrBlobStorageReader> reader(new CStringOrBlobStorageReader(job_content, nc_api));
        unique_ptr<CNcbiIstream> rstr(new CRStream(reader.release(), 0, nullptr, CRWStreambuf::fOwnReader));
        return rstr;
    }

protected:
    static unique_ptr<CObjectIStream>
    GetIStream(CNcbiIstream& istr,
               const SStreamProp& stream_prop,
               TOwnership ownership = CCompressionStream::fOwnProcessor
              )
    {
        unique_ptr<CCompressionIStream> instr_zip(
            new CCompressionIStream(
                istr,
                CreateStreamDecompressor(stream_prop),
                ownership
            )
        );
        return unique_ptr<CObjectIStream>(CObjectIStream::Open(GetDataFormat(), *instr_zip.release(), eTakeOwnership));
    }

    static SStreamProp GetJobStreamProperties(const string& job_content, CNetCacheAPI& nc_api)
    {
        if (job_content.empty()) {
            NCBI_THROW(CException, eUnknown, "job content is empty");
        }
        CStringOrBlobStorageReader reader(job_content, nc_api);
        char buf[5];
        size_t count = ArraySize(buf);
        size_t bytes_read = 0UL;
        reader.Read(buf, count, &bytes_read);
        const bool is_lzo = IsLZOStream(CTempString(buf, bytes_read));
        return SStreamProp(is_lzo ? CCompressStream::eLZO : CCompressStream::eZip);
    };

    static SStreamProp
    GetIStreamProperties(CNcbiIstream& istr)
    {
        return SStreamProp(IsLZOStream(istr) ? CCompressStream::eLZO : CCompressStream::eZip);
    }

    static bool IsLZOStream(CNcbiIstream& istr)
    {
	char buf[5];
        const size_t buflen = ArraySize(buf);
        const streamsize readlen = CStreamUtils::Readsome(istr, buf, buflen);
        CStreamUtils::Stepback(istr, buf, readlen);
        return IsLZOStream(CTempString(buf, readlen));
    }

    static bool IsLZOStream(const CTempString& str)
    {
        /// LZO magic header (see fStreamFormat flag).
        static const char kMagic[] = { 'L', 'Z', 'O', '\0' };
        static const size_t kMagicSize = 4UL;
        return (str.size() < kMagicSize)
            ? false
            : NStr::Equal(CTempString(kMagic, kMagicSize), CTempString(str, 0, kMagicSize));
    }

    static CCompressionStreamProcessor* CreateStreamCompressor(const SStreamProp& stream_prop)
    {
        unique_ptr<CCompressionStreamProcessor> sp;
        if (stream_prop.compress_method == CCompressStream::eLZO) {
            sp.reset(new CLZOStreamCompressor());
        }
        else {
            sp.reset(new CZipStreamCompressor());
        }
        return sp.release();
    }

    static CCompressionStreamProcessor* CreateStreamDecompressor(const SStreamProp& stream_prop)
    {
        unique_ptr<CCompressionStreamProcessor> sp;
        if (stream_prop.compress_method == CCompressStream::eLZO) {
            sp.reset(new CLZOStreamDecompressor());
        }
        else {
            sp.reset(new CZipStreamDecompressor());
        }
        return sp.release();
    }

    static string CompMethodToString(const CCompressStream::EMethod method)
    {
        switch (method) {
        case CCompressStream::eNone:
            return "none";
        case CCompressStream::eBZip2:
            return "BZip2";
        case CCompressStream::eLZO:
            return "LZO";
        case CCompressStream::eZip:
            return "Zip";
        case CCompressStream::eGZipFile:
            return "GZipFile";
        case CCompressStream::eConcatenatedGZipFile:
            return "GZipFile";
        };
        NCBI_THROW(CException, eUnknown, "unexpected compression method");
    }
};


class CGridRPCBaseClientException : public CException
{
public:
    enum EErrCode
    {
        eWaitTimeout,    ///< timeout while waiting for job completion
        eUnexpectedFailure
    };
    virtual const char* GetErrCodeString(void) const override;

    NCBI_EXCEPTION_DEFAULT(CGridRPCBaseClientException, CException);
};


///
/// CGridRPCBaseClient<typename TConnectTraits>
///
/// Base class for GRID-based ASN.1 RPC clients
/// 
/// TConnectTraits template classes: CAsnBinCompressed
///
template <typename TConnectTraits = CAsnBinCompressed, int DefaultTimeout = 20>
class CGridRPCBaseClient : protected TConnectTraits
{
public:
    CGridRPCBaseClient(const string& NS_service,
                       const string& NS_queue,
                       const string& client_name,
                       const string& NC_registry_section
                      )
        : m_NS_api(NS_service, client_name, NS_queue),
          m_Timeout(DefaultTimeout)
    {
        x_Init(NC_registry_section);
    }

    CGridRPCBaseClient(const string& NS_registry_section = "netschedule_api",
                       const string& NC_registry_section = kEmptyStr
                      )
        : m_NS_api(CNetScheduleAPI::eAppRegistry, NS_registry_section),
          m_Timeout(DefaultTimeout)
    {
        CNcbiApplicationGuard guard = CNcbiApplication::InstanceGuard();
        static const CNcbiRegistry& cfg = guard->GetConfig();
        const string nc_reg(
            NStr::IsBlank(NC_registry_section)
                ? cfg.GetString(NS_registry_section, "netcache_api", "netcache_api")
                : NC_registry_section
        );
        x_Init(nc_reg);
    }

    /// Sets the request timeout (default is 20s)
    ///
    /// @param timeout
    ///   Timeout in seconds
    void SetTimeout(const size_t timeout)
    {
        m_Timeout = timeout;
    }

    void x_Init(const string& NC_registry_section)
    {
        m_NC_api = CNetCacheAPI(CNetCacheAPI::eAppRegistry, NC_registry_section);
    }

    virtual ~CGridRPCBaseClient() = default;

    /// Sends an ASN.1 request, waiting specified time
    ///
    /// @param request
    ///   ASN.1 request, serialized from string
    /// @param reply
    ///   ASN.1 reply, serialized to stream
    /// @return
    ///   pair of CNetScheduleJob and boolean, indicating if the request timed out (always false)
    /// @throws
    ///   CGridRPCBaseClientException::eUnexpectedFailure on error
    ///   CGridRPCBaseClientException::eWaitTimeout if the job didn't finish in the specified time
    /// @note
    ///   The timeout can be set by using \link SetTimeout
    pair<CNetScheduleJob, bool> AskStream(CNcbiIstream& request, CNcbiOstream& reply) const
    {
        CPerfLogGuard pl("CGridRPCBaseClient::AskStream");
        CGridClient grid_cli(m_NS_api.GetSubmitter(),
                             m_NC_api,
                             CGridClient::eManualCleanup,
                             CGridClient::eProgressMsgOn
                            );
        auto& job_in = grid_cli.GetOStream(); // job input stream
        NcbiStreamCopy(job_in, request);
        if (job_in.bad()) {
            NCBI_THROW(CIOException, eWrite, "Error while writing request");
        }
        grid_cli.CloseStream();

        CNetScheduleJob& job = grid_cli.GetJob();
        x_PrepareJob(job);

        const CNetScheduleAPI::EJobStatus evt = grid_cli.SubmitAndWait(m_Timeout);
        switch (evt) {
        case CNetScheduleAPI::eDone:
        {
            m_NS_api.GetJobDetails(job);
            auto instr = TConnectTraits::GetRawIStream(job.output, m_NC_api);
            NcbiStreamCopy(reply, *instr);
            break;
        }
        case CNetScheduleAPI::eFailed:
            NCBI_THROW(CGridRPCBaseClientException, eUnexpectedFailure, "Job failed");

        case CNetScheduleAPI::eCanceled:
            NCBI_THROW(CGridRPCBaseClientException, eUnexpectedFailure, "Job canceled");

        case CNetScheduleAPI::ePending:
        case CNetScheduleAPI::eRunning:
        case CNetScheduleAPI::eReading:
            NCBI_THROW(CGridRPCBaseClientException, eWaitTimeout, "The job timed out");

        default:
            NCBI_THROW(CGridRPCBaseClientException,
                       eUnexpectedFailure,
                       "Unexpected status: " + CNetScheduleAPI::StatusToString(evt)
                       );
        }

        pl.Post(CRequestStatus::e200_Ok);
        return make_pair(job, false);
    }

    /// Sends an ASN.1 request, waiting specified time
    ///
    /// @param request
    ///   ASN.1 request
    /// @param reply
    ///   ASN.1 reply
    /// @return
    ///   pair of CNetScheduleJob and boolean, indicating if the request timed out (always false)
    /// @throws
    ///   CGridRPCBaseClientException::eUnexpectedFailure on error
    ///   CGridRPCBaseClientException::eWaitTimeout if the job didn't finish in the specified time
    /// @note
    ///   The timeout can be set by using \link SetTimeout
    template <class TRequest, class TReply>
    pair<CNetScheduleJob, bool> Ask(const TRequest& request, TReply& reply) const
    {
        CPerfLogGuard pl("CGridRPCBaseClient::Ask");
        CGridClient grid_cli(m_NS_api.GetSubmitter(),
                             m_NC_api,
                             CGridClient::eManualCleanup,
                             CGridClient::eProgressMsgOn
                            );
        auto& job_in = grid_cli.GetOStream(); // job input stream
        auto outstr = TConnectTraits::GetOStream(job_in);
        *outstr << request;
        if (job_in.bad()) {
            NCBI_THROW(CIOException, eWrite, "Error while writing request");
        }
        outstr.reset();
        grid_cli.CloseStream();

        CNetScheduleJob& job = grid_cli.GetJob();
        x_PrepareJob(job);

        const CNetScheduleAPI::EJobStatus evt = grid_cli.SubmitAndWait(m_Timeout);
        switch (evt) {
        case CNetScheduleAPI::eDone:
        {
            m_NS_api.GetJobDetails(job);
            auto instr = TConnectTraits::GetIStream(job.output, m_NC_api);
            *instr >> reply;
            break;
        }
        case CNetScheduleAPI::eFailed:
            NCBI_THROW(CGridRPCBaseClientException, eUnexpectedFailure, "Job failed");

        case CNetScheduleAPI::eCanceled:
            NCBI_THROW(CGridRPCBaseClientException, eUnexpectedFailure, "Job canceled");

        case CNetScheduleAPI::ePending:
        case CNetScheduleAPI::eRunning:
        case CNetScheduleAPI::eReading:
            NCBI_THROW(CGridRPCBaseClientException, eWaitTimeout, "The job timed out");

        default:
            NCBI_THROW(CGridRPCBaseClientException,
                       eUnexpectedFailure,
                       "Unexpected status: " + CNetScheduleAPI::StatusToString(evt)
                       );
        }

        pl.Post(CRequestStatus::e200_Ok);
        return make_pair(job, false);
    }

protected:
    CGridRPCBaseClient(const CGridRPCBaseClient&) = delete;
    CGridRPCBaseClient(CGridRPCBaseClient&& c)
        : m_NS_api(move(c.m_NS_api)), m_NC_api(move(c.m_NC_api)),
          m_Timeout(c.m_Timeout)
    {
    }

    virtual void x_PrepareJob(CNetScheduleJob& /*job*/) const
    {
    }

    template <class TReply>
    CNetScheduleJob x_GetJobById(const string job_id, TReply& reply) const
    {
        CPerfLogGuard pl("CGridRPCBaseClient::x_GetJobById");
        CNetScheduleJob job;
        job.job_id = job_id;

        CNetScheduleSubmitter job_submitter = m_NS_api.GetSubmitter();
        const CNetScheduleAPI::EJobStatus evt = job_submitter.WaitForJob(job.job_id, m_Timeout);
        switch (evt) {
        case CNetScheduleAPI::eDone:
        {
            m_NS_api.GetJobDetails(job);
            auto instr = TConnectTraits::GetIStream(job.output, m_NC_api);
            *instr >> reply;
            break;
        }
        case CNetScheduleAPI::eFailed:
            NCBI_THROW(CGridRPCBaseClientException, eUnexpectedFailure, "Job failed");

        case CNetScheduleAPI::eCanceled:
            NCBI_THROW(CGridRPCBaseClientException, eUnexpectedFailure, "Job canceled");

        case CNetScheduleAPI::eRunning:
            NCBI_THROW(CGridRPCBaseClientException, eUnexpectedFailure, "Job running");

        default:
            NCBI_THROW(CGridRPCBaseClientException,
                       eWaitTimeout,
                       "Unexpected status: " + CNetScheduleAPI::StatusToString(evt)
                      );
        }
        pl.Post(CRequestStatus::e200_Ok);
        return job;
    }

private:
    mutable CNetScheduleAPI m_NS_api;
    mutable CNetCacheAPI m_NC_api;
    Uint4 m_Timeout;
};


template <typename TRequest, typename TReply>
class CGridRPCHttpClient : public CRPCClient<TRequest, TReply>
{
protected:
    using TParent = CRPCClient<TRequest, TReply>;

public:
    CGridRPCHttpClient(const string& http_service)
        : TParent(http_service)
    {
    }

    virtual ~CGridRPCHttpClient() = default;

protected:
    virtual void x_Connect() override
    {
        TParent::x_Connect();
        TParent::m_In.reset(CAsnBinCompressed::GetIStream(*TParent::m_Stream).release());
        TParent::m_Out.reset(CAsnBinCompressed::GetOStream(*TParent::m_Stream).release());
    }
};


END_NCBI_SCOPE

#endif // OBJECTS_MISC_SERIAL___GRID_RPC_CLIENT__HPP

