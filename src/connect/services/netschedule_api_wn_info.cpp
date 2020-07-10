/*  $Id: netschedule_api_wn_info.cpp 601821 2020-02-13 19:58:06Z sadyrovr $
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
 * Author: Dmitry Kazimirov
 *
 */

#include <ncbi_pch.hpp>

#include "netschedule_api_impl.hpp"

#include <connect/services/netschedule_api.hpp>

#include <locale>

BEGIN_NCBI_SCOPE

typedef CNetScheduleAdmin::SWorkerNodeInfo SWorkerNodeInfo;

/// Types of administrative information NetSchedule can return.
///
enum ENetScheduleStatTopic;

void g_GetWorkerNodes(CNetScheduleAPI api, list<SWorkerNodeInfo>& worker_nodes)
{
    worker_nodes.clear();

    set<string> unique_sessions;

    for (CNetServiceIterator iter = api->m_Service.Iterate(
            CNetService::eIncludePenalized); iter; ++iter) {

        CJsonNode client_info_array = g_GenericStatToJson(*iter,
                eNetScheduleStatClients, /*verbose*/ false);

        for (CJsonIterator client_info_iter = client_info_array.Iterate();
                client_info_iter; ++client_info_iter) {
            CJsonNode client_info = *client_info_iter;
            if (NStr::Find(client_info.GetString("type"),
                    "worker node") == NPOS)
                continue;

            string session = client_info.GetString("session");

            if (unique_sessions.insert(session).second) {
                worker_nodes.push_back(SWorkerNodeInfo());

                SWorkerNodeInfo& wn_info = worker_nodes.back();
                wn_info.name = client_info.GetString("client_node");
                string not_used;
                NStr::SplitInTwo(wn_info.name, "::", wn_info.prog, not_used);
                wn_info.session = session;
                wn_info.host = client_info.GetString("client_host");
                wn_info.port = (unsigned short)
                        client_info.GetInteger("worker_node_control_port");
                wn_info.last_access = CTime(
                        client_info.GetString("last_access"), "M/D/Y h:m:s.r");
            }
        }
    }
}

CJsonNode g_GetWorkerNodeInfo(CNetScheduleAPI api)
{
    CJsonNode result(CJsonNode::NewObjectNode());

    list<SWorkerNodeInfo> worker_nodes;
    g_GetWorkerNodes(api, worker_nodes);

    ITERATE(list<CNetScheduleAdmin::SWorkerNodeInfo>, wn_info, worker_nodes) {
        string wn_address = wn_info->host + ':' +
                NStr::NumericToString(wn_info->port);
        try {
            CNetScheduleAPI wn_api(wn_address,
                    api->m_Service->GetClientName(),
                    kEmptyStr);

            result.SetByKey(wn_info->session, g_WorkerNodeInfoToJson(
                    wn_api.GetService().Iterate().GetServer()));
        }
        catch (CException& e) {
            ERR_POST(e);
        }
    }

    return result;
}

struct {
    const char* command;
    const char* record_prefix;
    const char* entity_name;
} static const s_StatTopics[eNumberOfNetStheduleStatTopics] = {
    // eNetScheduleStatJobGroups
    {"STAT GROUPS", "GROUP: ", "group"},
    // eNetScheduleStatClients
    {"STAT CLIENTS", "CLIENT: ", "client_node"},
    // eNetScheduleStatNotifications
    {"STAT NOTIFICATIONS", "CLIENT: ", "client_node"},
    // eNetScheduleStatAffinities
    {"STAT AFFINITIES", "AFFINITY: ", "affinity_token"}
};

string g_GetNetScheduleStatCommand(ENetScheduleStatTopic topic)
{
    return s_StatTopics[topic].command;
}

// Return unquoted and unescaped version of 'str' if it
// starts with a quote. Otherwise, return 'str' itself.
//
string g_UnquoteIfQuoted(const CTempString& str)
{
    switch (str[0]) {
    case '"':
    case '\'':
        return NStr::ParseQuoted(str);
    default:
        return str;
    }
}

static void NormalizeStatKeyName(CTempString& key)
{
    locale loc;
    char* begin = const_cast<char*>(key.data());
    char* end = begin + key.length();

    while (begin < end && !isalnum(*begin))
        ++begin;

    while (begin < end && !isalnum(end[-1]))
        --end;

    if (begin == end) {
        key = "_";
        return;
    }

    key.assign(begin, end - begin);

    for (; begin < end; ++begin)
        *begin = isalnum(*begin, loc) ? tolower(*begin, loc) : '_';
}

// Detect older format of the worker node info line that contained
// executable name followed by PID.
// @return false if 'stat_info' already has the new format
//         of the executable name; true if 'stat_info' was fixed.
//
bool g_FixMisplacedPID(CJsonNode& stat_info, CTempString& executable_path,
        const char* pid_key)
{
    SIZE_TYPE misplaced_pid = NStr::Find(executable_path, "; PID: ");
    if (misplaced_pid == NPOS)
        return false;

    SIZE_TYPE pos = misplaced_pid + sizeof("; PID: ") - 1;
    stat_info.SetInteger(pid_key, NStr::StringToInt8(
            CTempString(executable_path.data() + pos,
                    executable_path.length() - pos)));
    executable_path.erase(misplaced_pid);
    return true;
}

// Return a JSON object that contains the requested infromation.
// The structure of the object depends on the topic.
//
CJsonNode g_GenericStatToJson(CNetServer server,
        ENetScheduleStatTopic topic, bool verbose)
{
    string stat_cmd(s_StatTopics[topic].command);
    CTempString prefix(s_StatTopics[topic].record_prefix);
    CTempString entity_name(s_StatTopics[topic].entity_name);

    if (verbose)
        stat_cmd.append(" VERBOSE");

    g_AppendClientIPSessionIDHitID(stat_cmd);

    CNetServerMultilineCmdOutput output(server.ExecWithRetry(stat_cmd, true));

    CJsonNode entities(CJsonNode::NewArrayNode());
    CJsonNode entity_info;
    CJsonNode array_value;

    string line;

    while (output.ReadLine(line)) {
        if (NStr::StartsWith(line, prefix)) {
            if (entity_info)
                entities.Append(entity_info);
            entity_info = CJsonNode::NewObjectNode();
            entity_info.SetString(entity_name, g_UnquoteIfQuoted(
                    CTempString(line.data() + prefix.length(),
                    line.length() - prefix.length())));
        } else if (entity_info && NStr::StartsWith(line, "  ")) {
            if (NStr::StartsWith(line, "    ") && array_value) {
                array_value.AppendString(g_UnquoteIfQuoted(
                        NStr::TruncateSpaces(line, NStr::eTrunc_Begin)));
            } else {
                if (array_value)
                    array_value = NULL;
                CTempString key, value;
                NStr::SplitInTwo(line, ":", key, value);
                NormalizeStatKeyName(key);
                string key_norm(key);
                value = NStr::TruncateSpaces_Unsafe(value, NStr::eTrunc_Begin);
                if (value.empty())
                    entity_info.SetByKey(key_norm, array_value =
                            CJsonNode::NewArrayNode());
                else {
                    const auto string_node = (topic == eNetScheduleStatClients) && (key_norm == "client_host");
                    auto node = string_node ? CJsonNode::NewStringNode(value) : CJsonNode::GuessType(value);
                    entity_info.SetByKey(key_norm, node);
                }
            }
        }
    }
    if (entity_info)
        entities.Append(entity_info);

    return entities;
}

// Convert 'server_info' to a JSON object.
//
CJsonNode g_ServerInfoToJson(CNetServerInfo server_info,
        bool server_version_key)
{
    CJsonNode server_info_node(CJsonNode::NewObjectNode());

    string attr_name, attr_value;

    ESwitch old_format = eDefault;

    while (server_info.GetNextAttribute(attr_name, attr_value)) {
        switch (old_format) {
        case eOn:
            if (attr_name == "Build")
                attr_name = "build_date";
            else
                NStr::ReplaceInPlace(NStr::ToLower(attr_name), " ", "_");
            break;

        case eDefault:
            if (NStr::EndsWith(attr_name, " version"))
            {
                old_format = eOn;
                attr_name = server_version_key ? "server_version" : "version";
                break;
            } else
                old_format = eOff;
            /* FALL THROUGH */

        case eOff:
            if (server_version_key && attr_name == "version")
                attr_name = "server_version";
        }

        server_info_node.SetString(attr_name, attr_value);
    }

    return server_info_node;
}

static bool s_ExtractKey(const CTempString& line,
        string& key, CTempString& value)
{
    locale loc;
    key.erase(0, key.length());
    SIZE_TYPE line_len = line.length();
    key.reserve(line_len);

    for (SIZE_TYPE i = 0; i < line_len; ++i)
    {
        char c = line[i];
        if (isalnum(c, loc))
            key += tolower(c, loc);
        else if (c == ' ' || c == '_' || c == '-')
            key += '_';
        else if (c != ':' || key.empty())
            break;
        else {
            if (++i < line_len && line[i] == ' ')
                ++i;
            value.assign(line, i, line_len - i);
            return true;
        }
    }
    return false;
}

static CJsonNode s_WordsToJsonArray(const CTempString& str)
{
    CJsonNode array_node(CJsonNode::NewArrayNode());
    list<CTempString> words;
    NStr::Split(str, TEMP_STRING_CTOR(" "), words,
            NStr::fSplit_MergeDelimiters | NStr::fSplit_Truncate);
    ITERATE(list<CTempString>, it, words) {
        array_node.AppendString(*it);
    }
    return array_node;
}

// Return a JSON object that contains status information reported
// by the specified worker node.
//
CJsonNode g_WorkerNodeInfoToJson(CNetServer worker_node)
{
    CNetServerMultilineCmdOutput output(
            worker_node.ExecWithRetry("STAT", true));

    CJsonNode wn_info(CJsonNode::NewObjectNode());

    string line;
    string key;
    CTempString value;
    CJsonNode job_counters(CJsonNode::NewObjectNode());
    CJsonNode running_jobs(CJsonNode::NewArrayNode());
    CJsonNode alerts(CJsonNode::NewObjectNode());
    SIZE_TYPE pos;
    int running_job_count = 0;
    int free_worker_threads = 1;
    bool suspended = false;
    bool shutting_down = false;
    bool exclusive_job = false;
    bool working = false;

    while (output.ReadLine(line)) {
        if (line.empty() || isspace(line[0]))
            continue;

        if (running_job_count > 0) {
            if ((pos = NStr::Find(CTempString(line),
                            TEMP_STRING_CTOR("running for "),
                            NStr::eNocase, NStr::eReverseSearch)) != NPOS) {
                --running_job_count;

                pos += sizeof("running for ") - 1;
                Uint8 run_time = NStr::StringToUInt8(
                        CTempString(line.data() + pos, line.length() - pos),
                                NStr::fConvErr_NoThrow |
                                NStr::fAllowLeadingSpaces |
                                NStr::fAllowTrailingSymbols);

                if ((pos = NStr::Find(line, TEMP_STRING_CTOR(" "))) != NPOS) {
                    CJsonNode running_job(CJsonNode::NewObjectNode());

                    running_job.SetString("key", CTempString(line.data(), pos));
                    running_job.SetInteger("run_time", (Int8) run_time);
                    running_jobs.Append(running_job);
                }
            }
        } else if (NStr::StartsWith(line, TEMP_STRING_CTOR("Jobs "))) {
            if (s_ExtractKey(CTempString(line.data() + sizeof("Jobs ") - 1,
                    line.length() - (sizeof("Jobs ") - 1)), key, value)) {
                job_counters.SetInteger(key, NStr::StringToInt8(value));
                if (key == "running") {
                    running_job_count = NStr::StringToInt(value,
                            NStr::fConvErr_NoThrow |
                            NStr::fAllowLeadingSpaces |
                            NStr::fAllowTrailingSymbols);
                    working = running_job_count > 0;
                    free_worker_threads -= running_job_count;
                }
            }
        } else if (NStr::StartsWith(line, TEMP_STRING_CTOR("Alert_")) && s_ExtractKey(line, key, value)) {
            alerts.SetString(key, value);
        } else if (s_ExtractKey(line, key, value)) {
            if (key == "host_name")
                key = "hostname";
            else if (key == "node_started_at")
                key = "started";
            else if (key == "executable_path")
                g_FixMisplacedPID(wn_info, value, "pid");
            else if (key == "maximum_job_threads") {
                int maximum_job_threads = NStr::StringToInt(value,
                            NStr::fConvErr_NoThrow |
                            NStr::fAllowLeadingSpaces |
                            NStr::fAllowTrailingSymbols);
                free_worker_threads += maximum_job_threads - 1;
                wn_info.SetInteger(key, maximum_job_threads);
                continue;
            } else if (key == "netschedule_servers") {
                if (value != "N/A")
                    wn_info.SetByKey(key, s_WordsToJsonArray(value));
                else
                    wn_info.SetByKey(key, CJsonNode::NewArrayNode());
                continue;
            } else if (key == "preferred_affinities") {
                wn_info.SetByKey(key, s_WordsToJsonArray(value));
                continue;
            }
            wn_info.SetByKey(key, CJsonNode::GuessType(value));
        } else if (wn_info.GetSize() == 0)
            wn_info = g_ServerInfoToJson(g_ServerInfoFromString(line), false);
        else if (NStr::Find(line, TEMP_STRING_CTOR("suspended")) != NPOS)
            suspended = true;
        else if (NStr::Find(line, TEMP_STRING_CTOR("shutting down")) != NPOS)
            shutting_down = true;
        else if (NStr::Find(line, TEMP_STRING_CTOR("exclusive job")) != NPOS)
            exclusive_job = true;
    }

    if (!wn_info.HasKey("maximum_job_threads"))
        wn_info.SetInteger("maximum_job_threads", 1);
    if (!wn_info.HasKey("version"))
        wn_info.SetString("version", kEmptyStr);
    if (!wn_info.HasKey("build_date"))
        wn_info.SetString("build_date", kEmptyStr);

    wn_info.SetString("status",
            shutting_down ? "shutting_down" :
            suspended ? "suspended" :
            exclusive_job ? "processing_exclusive_job" :
            free_worker_threads == 0 ? "busy" :
            working ? "working" : "ready");

    wn_info.SetByKey("job_counters", job_counters);
    wn_info.SetByKey("running_jobs", running_jobs);
    if (alerts.GetSize()) wn_info.SetByKey("alerts", alerts);

    return wn_info;
}

END_NCBI_SCOPE
