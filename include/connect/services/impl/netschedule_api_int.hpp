#ifndef CONNECT_SERVICES_IMPL__NETSCHEDULE_API_INT__HPP
#define CONNECT_SERVICES_IMPL__NETSCHEDULE_API_INT__HPP

/*  $Id: netschedule_api_int.hpp 554421 2018-01-03 15:14:01Z sadyrovr $
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
 * Authors: Dmitry Kazimirov, Rafael Sadyrov
 *
 * File Description:
 *   Internal declarations for NetSchedule client API.
 *
 */

#include <util/ncbi_url.hpp>

#include <tuple>

BEGIN_NCBI_SCOPE


const unsigned int kNetScheduleMaxDBDataSize = 2048;

const unsigned int kNetScheduleMaxDBErrSize = 4096;


NCBI_DECLARE_INTERFACE_VERSION(SNetScheduleAPIImpl, "xnetschedule_api", 1,0, 0);

extern NCBI_XCONNECT_EXPORT const char* const kNetScheduleAPIDriverName;

/// @deprecated, use SNetScheduleOutputParser instead
extern NCBI_DEPRECATED NCBI_XCONNECT_EXPORT
int g_ParseNSOutput(const string& attr_string, const char* const* attr_names,
        string* attr_values, size_t attr_count);

struct NCBI_XCONNECT_EXPORT SNetScheduleOutputParser : private CUrlArgs
{
    SNetScheduleOutputParser(const string& output);
    const string& operator()(const string& param) const;
};

void NCBI_XCONNECT_EXPORT NCBI_EntryPoint_xnetscheduleapi(
     CPluginManager<SNetScheduleAPIImpl>::TDriverInfoList&   info_list,
     CPluginManager<SNetScheduleAPIImpl>::EEntryPointRequest method);

struct SNetScheduleNotificationReceiver
{
    CDatagramSocket socket;
    unsigned short port;
    string message;

    SNetScheduleNotificationReceiver();
    bool operator()(string* server_host);
};

class NCBI_XCONNECT_EXPORT CNetScheduleNotificationHandler
{
public:
    CNetScheduleNotificationHandler();

    bool ReceiveNotification(string* server_host = NULL);

    bool WaitForNotification(const CDeadline& deadline,
                             string*          server_host = NULL);

    unsigned short GetPort() const { return m_Receiver.port; }

    const string& GetMessage() const { return m_Receiver.message; }

    void PrintPortNumber();

// Submitter methods.
public:
    void SubmitJob(CNetScheduleSubmitter::TInstance submitter,
            CNetScheduleJob& job,
            unsigned wait_time,
            CNetServer* server = NULL);

    bool CheckJobStatusNotification(const string& job_id,
            CNetScheduleAPI::EJobStatus* job_status,
            int* last_event_index = NULL);

    // This method requires calling SubmitJob prior with wait_time set
    CNetScheduleAPI::EJobStatus WaitForJobCompletion(CNetScheduleJob& job,
            CDeadline& deadline, CNetScheduleAPI ns_api, time_t* job_exptime = NULL);

    using TLastEventIndex = int;
    using TProgressMessage = string;
    using TJobInfo = tuple<CNetScheduleAPI::EJobStatus, TLastEventIndex, TProgressMessage>;

    TJobInfo RequestJobWatching(CNetScheduleAPI::TInstance ns_api,
            const string& job_id,
            const CDeadline& deadline);

    /// @deprecated, use tuple returning overload instead
    NCBI_DEPRECATED
    bool RequestJobWatching(CNetScheduleAPI::TInstance ns_api,
            const string& job_id,
            const CDeadline& deadline,
            CNetScheduleAPI::EJobStatus* job_status,
            int* last_event_index);

    enum EJobStatusMask {
        fJSM_Pending        = 1 << CNetScheduleAPI::ePending,
        fJSM_Running        = 1 << CNetScheduleAPI::eRunning,
        fJSM_Canceled       = 1 << CNetScheduleAPI::eCanceled,
        fJSM_Failed         = 1 << CNetScheduleAPI::eFailed,
        fJSM_Done           = 1 << CNetScheduleAPI::eDone,
        fJSM_Reading        = 1 << CNetScheduleAPI::eReading,
        fJSM_Confirmed      = 1 << CNetScheduleAPI::eConfirmed,
        fJSM_ReadFailed     = 1 << CNetScheduleAPI::eReadFailed,
        fJSM_Deleted        = 1 << CNetScheduleAPI::eDeleted
    };
    typedef int TJobStatusMask;

    CNetScheduleAPI::EJobStatus WaitForJobEvent(
            const string& job_key,
            CDeadline& deadline,
            CNetScheduleAPI ns_api,
            TJobStatusMask status_mask,
            int last_event_index = kMax_Int,
            int *new_event_index = NULL);

// Worker node methods.
public:
    static string MkBaseGETCmd(
        CNetScheduleExecutor::EJobAffinityPreference affinity_preference,
        const string& affinity_list);
    void CmdAppendTimeoutGroupAndClientInfo(string& cmd,
            const CDeadline* deadline, const string& job_group);
    bool RequestJob(CNetScheduleExecutor::TInstance executor,
                    CNetScheduleJob& job,
                    const string& cmd);
    bool CheckRequestJobNotification(CNetScheduleExecutor::TInstance executor,
                                     CNetServer* server);

protected:
    bool GetJobDetailsIfCompleted(CNetScheduleAPI ns_api, CNetScheduleJob& job,
            time_t* job_exptime, CNetScheduleAPI::EJobStatus& job_status);

    SNetScheduleNotificationReceiver m_Receiver;
};

struct NCBI_XCONNECT_EXPORT CNetScheduleAPIExt : CNetScheduleAPI
{
    CNetScheduleAPIExt() {}
    CNetScheduleAPIExt(TInstance api) : CNetScheduleAPI(api) {}
    CNetScheduleAPIExt& operator=(TInstance api)
    {
        CNetScheduleAPI::operator=(api);
        return *this;
    }
    void AddToClientNode(const string& data);
    void UpdateAuthString();
    void UseOldStyleAuth();
    CCompoundIDPool GetCompoundIDPool();

    /// Extract one of the servers comprising this service
    /// as a separate NetSchedule API object.
    CNetScheduleAPI GetServer(CNetServer::TInstance server);

    // These allow empty values, too (unlike Set* counterparts)
    void ReSetClientNode(const string&);
    void ReSetClientSession(const string&);

    // Create workernode-compatible API
    static TInstance CreateWnCompat(const string&, const string&);

    // Create API with no auto config loading (from server)
    static TInstance CreateNoCfgLoad(const string&, const string&, const string&);
};

struct SNetScheduleAdminImpl;

class NCBI_XCONNECT_EXPORT CNetScheduleAdmin
{
    NCBI_NET_COMPONENT(NetScheduleAdmin);

    /// Status map, shows number of jobs in each status
    typedef map<string, unsigned> TStatusMap;

    /// Returns statuses for a given affinity token
    /// @param status_map
    ///    Status map (status to job count)
    /// @param affinity_token
    ///    Affinity token (optional)
    /// @param job_group
    ///    Only jobs belonging to the specified group (optional)
    void StatusSnapshot(TStatusMap& status_map,
            const string& affinity_token = kEmptyStr,
            const string& job_group = kEmptyStr);

    /// Create an instance of the given queue class.
    /// @param qname
    ///    Name of the queue to create
    /// @param qclass
    ///    Parameter set described in config file in a qclass_<qname> section.
    /// @param description
    ///    Brief free text description of the queue.
    void CreateQueue(
        const string& qname,
        const string& qclass,
        const string& description = kEmptyStr);

    /// Delete queue
    /// Applicable only to queues, created through CreateQueue method
    /// @param qname
    ///    Name of the queue to delete.
    void DeleteQueue(const string& qname);


    /// Shutdown level
    ///
    enum EShutdownLevel {
        eNoShutdown = 0,    ///< No Shutdown was requested
        eNormalShutdown,    ///< Normal shutdown was requested
        eShutdownImmediate, ///< Urgent shutdown was requested
        eDie,               ///< A serious error occurred, the server shuts down
        eDrain              ///< Wait for all server data to expire.
    };

    /// Enable server drain mode.
    ///
    void SwitchToDrainMode(ESwitch on_off);

    /// Shutdown the server daemon.
    ///
    void ShutdownServer(EShutdownLevel level = eNormalShutdown);

    /// Cancel all jobs in the queue (optionally with particular statuses).
    ///
    /// @param job_statuses
    ///    Optional comma-separated list of job statuses
    void CancelAllJobs(const string& job_statuses = kEmptyStr);

    void DumpJob(CNcbiOstream& out, const string& job_key);
    CNetServerMultilineCmdOutput DumpJob(const string& job_key);

    void ReloadServerConfig();

    //////////////////////////////////////////////////////
    /// Print version string
    void PrintServerVersion(CNcbiOstream& output_stream);

    struct SWorkerNodeInfo {
        string name;
        string prog;
        string session;
        string host;
        unsigned short port;
        CTime last_access;
    };

    void GetWorkerNodes(list<SWorkerNodeInfo>& worker_nodes);

    void PrintConf(CNcbiOstream& output_stream);

    enum EStatisticsOptions
    {
        eStatisticsAll,
        eStatisticsBrief,
        eStatisticsClients
    };

    void PrintServerStatistics(CNcbiOstream& output_stream,
        EStatisticsOptions opt = eStatisticsBrief);

    void PrintHealth(CNcbiOstream& output_stream);

    // Two versions of DumpQueue to keep the code compile compatible
    void DumpQueue(CNcbiOstream& output_stream,
        const string& start_after_job = kEmptyStr,
        size_t job_count = 0,
        const string& job_statuses = kEmptyStr,
        const string& job_group = kEmptyStr);
    void DumpQueue(CNcbiOstream& output_stream,
        const string& start_after_job,
        size_t job_count,
        CNetScheduleAPI::EJobStatus status,
        const string& job_group = kEmptyStr);

    typedef map<string, string> TQueueInfo;
    // Get information on a particular queue of a particular server.
    void GetQueueInfo(CNetServer server, const string& queue_name,
            TQueueInfo& queue_info);
    // The same as above, but for any random server in the service.
    void GetQueueInfo(const string& queue_name, TQueueInfo& queue_info);
    // Return information on the current queue.
    void GetQueueInfo(CNetServer server, TQueueInfo& queue_info);
    // The same as above, but for any random server in the service.
    void GetQueueInfo(TQueueInfo& queue_info);
    void PrintQueueInfo(const string& queue_name, CNcbiOstream& output_stream);

    struct SServerQueueList {
        CNetServer server;
        list<string> queues;

        SServerQueueList(SNetServerImpl* server_impl) : server(server_impl) {}
    };

    typedef list<SServerQueueList> TQueueList;

    void GetQueueList(TQueueList& result);
};

enum ENetScheduleStatTopic {
    eNetScheduleStatJobGroups,
    eNetScheduleStatClients,
    eNetScheduleStatNotifications,
    eNetScheduleStatAffinities,
    eNumberOfNetStheduleStatTopics
};
// Return a JSON object where each key is a worker node
// session and the value is a JSON object containing
// status information reported by the worker node itself.
NCBI_XCONNECT_EXPORT
CJsonNode g_GetWorkerNodeInfo(CNetScheduleAPI api);
NCBI_XCONNECT_EXPORT
string g_UnquoteIfQuoted(const CTempString& str);
NCBI_XCONNECT_EXPORT
string g_GetNetScheduleStatCommand(ENetScheduleStatTopic topic);
NCBI_XCONNECT_EXPORT
CJsonNode g_GenericStatToJson(CNetServer server, ENetScheduleStatTopic topic, bool verbose);
NCBI_XCONNECT_EXPORT
bool g_FixMisplacedPID(CJsonNode& stat_info, CTempString& executable_path, const char* pid_key);
NCBI_XCONNECT_EXPORT
CJsonNode g_ServerInfoToJson(CNetServerInfo server_info, bool server_version_key);
NCBI_XCONNECT_EXPORT
CJsonNode g_WorkerNodeInfoToJson(CNetServer worker_node);

END_NCBI_SCOPE


#endif
