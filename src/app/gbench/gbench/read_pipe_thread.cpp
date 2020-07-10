/*  $Id: read_pipe_thread.cpp 38745 2017-06-14 22:07:34Z rudnev $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/utils/extension_impl.hpp>
#include <connect/ncbi_namedpipe.hpp>
#include <gui/core/app_named_pipe_client.hpp>
#include <gui/framework/workbench.hpp>
#include <gui/framework/app_task_service.hpp>

#include "open_files_task.hpp"
#include "read_pipe_thread.hpp"

BEGIN_NCBI_SCOPE

CReadPipeThread::CReadPipeThread(CNamedPipeServer& pipe, IWorkbench* workbench)
    : m_Pipe(pipe), m_Workbench(workbench), m_StopSignal(0, 10000000)
{
}

void CReadPipeThread::RequestStop()
{
    m_StopSignal.Post();
}

static string s_GetIOStatusDescr(EIO_Status status)
{
    string descr;
    switch (status) {
    case eIO_Success:
        descr = "everything is fine, no error occurred";
        break;
    case eIO_Timeout:
        descr = "timeout expired before any I/O succeeded";
        break;
    case eIO_Closed:
        descr = "peer has closed the connection";
        break;
    case eIO_Interrupt:
        descr = "signal received while I/O was in progress";
        break;
    case eIO_InvalidArg:
        descr = "bad argument value(s)";
        break;
    case eIO_NotSupported:
        descr = "the requested operation is not supported";
        break;
    default:
        descr = "unknown (most probably -- fatal) error";
        break;
    }
    return descr;
}

void* CReadPipeThread::Main(void)
{
    string err_msg;

    try {
        STimeout timeout = { 1, 0 };
        m_Pipe.SetTimeout(eIO_Read, &timeout);
        timeout.sec = 5;
        m_Pipe.SetTimeout(eIO_Open, &timeout);
        m_Pipe.SetTimeout(eIO_Write, &timeout);

        for (;;) {
            EIO_Status status = m_Pipe.Listen();

            if (m_StopSignal.TryWait())
                break;

            if (status == eIO_Timeout) continue;

            if (status != eIO_Success) {
                LOG_POST(Error << "CNamedPipe::Listen() returned \"" << s_GetIOStatusDescr(status) << "\"");
                break;
            }

            string data;
            const size_t buf_size = 1024;
            char buf[buf_size];
            size_t n_read = 0;
            size_t n_read_total = 0;
            do {
                status = m_Pipe.Read(buf, buf_size, &n_read);
                if (n_read > 0) data.append(buf, n_read);
                n_read_total += n_read;
            } while (status == eIO_Success && n_read > 0);

            if (m_StopSignal.TryWait())
                break;

            if (data.empty()) {
                m_Pipe.Disconnect();
                continue;
            }

            vector<IAppNamedPipeClient*> clients;
            GetExtensionAsInterface(EXT_POINT__GBENCH_NAMED_PIPE_CLIENTS, clients);

            bool processed = false;
            string reply;

            ITERATE(vector<IAppNamedPipeClient*>, it, clients) {
                reply.erase();
                try {
                    if ((*it)->Process(data, reply, m_Workbench)) {
                        processed = true;
                        break;
                    }
                }
                catch (const exception&) {}
            }

            if (processed && !reply.empty()) {
                size_t total_written = 0;
                EIO_Status status;
                do {
                    size_t n_written;
                    status = m_Pipe.Write(reply.data() + total_written,
                        reply.size() - total_written, &n_written);
                    total_written += n_written;
                } while (status == eIO_Success && total_written < reply.size());

                if (status != eIO_Success)
                    LOG_POST(Error << "Failed to write to named pipe, status: " << s_GetIOStatusDescr(status));
            }

            m_Pipe.Disconnect();

            if (!processed) {
                vector<string> tokens;
                string delim;
                delim = '\0';  // not an empty string
                NStr::Split(data, delim, tokens, NStr::fSplit_Tokenize);
                if (tokens.size() > 0 && tokens[0] == "gbench:open") {
                    vector<string> files(tokens.begin() + 1, tokens.end());

                    CAppTaskService* taskService = m_Workbench->GetAppTaskService();
                    if (taskService) {
                        COpenFilesTask* task = new COpenFilesTask(m_Workbench, files, true);
                        taskService->AddTask(*task);
                    }
                    else {
                        LOG_POST(Error << "Application Task Service not found: cannot start CNamedPipeProcessTask");
                    }
                    processed = true;
                }

                if (!processed) {
                    if (tokens.size() != 0) {
                        LOG_POST(Error << "Failed to proccess pipe data: " << tokens[0]);
                    }
                    else {
                        LOG_POST(Error << "Empty message received from the named pipe.");
                    }
                }
            }

            if (m_StopSignal.TryWait())
                break;
        }
    }
    catch (std::exception& e) {
        err_msg = "Failed to read via named pipe: ";
        err_msg += e.what();
    }
    catch (...) {
        err_msg = "Failed to read via named pipe: Unknown error.";
    }

    m_Pipe.Close();

    if (!err_msg.empty())
        LOG_POST(Error << err_msg);

    return 0;
}

END_NCBI_SCOPE
