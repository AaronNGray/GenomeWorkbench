/*  $Id: exec.cpp 16579 2008-04-02 13:20:08Z dicuccio $
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
 * Authors:  Josh Cherry
 *
 * File Description:  execute another program, capturing output in strings
 *
 */


#include <ncbi_pch.hpp>
#include <gui/utils/exec.hpp>

#include <corelib/ncbitime.hpp>
#include <connect/ncbi_pipe.hpp>

BEGIN_NCBI_SCOPE

// run an executable using strings for std*
int CExecute::Exec(const string& cmd,
                   const vector<string>& args,
                   const string& std_in,
                   string& std_out,
                   string& std_err,
                   STimeout *timeout)
{
    // open a pipe, with text mode for std_in, std_out, and std_err
    CPipe pipe;

    // set up timeout, if requested
    CStopWatch sw;
    double max_time = 0;  // initialize to avoid compiler warning
    if (timeout) {
        pipe.SetTimeout(eIO_Open, timeout);
        pipe.SetTimeout(eIO_Close, timeout);
        max_time = timeout->sec + timeout->usec / 1000000.0;
    }
    sw.Start();

    pipe.Open(cmd.c_str(), args, CPipe::fStdErr_Open);

    STimeout short_time = {0, 1};
    pipe.SetTimeout(eIO_Read, &short_time);
    pipe.SetTimeout(eIO_Write, &short_time);

    size_t bytes_written;
    size_t total_bytes_written = 0;
    bool out_done = false;
    bool err_done = false;

    const size_t buf_size = 4096;
    char buf[buf_size];

    while (!out_done || !err_done) {

        // write stdin
        if (total_bytes_written < std_in.size()) {
            pipe.Write(std_in.c_str() + total_bytes_written,
                       std_in.size() - total_bytes_written,
                       &bytes_written);
            total_bytes_written += bytes_written;
            if (total_bytes_written == std_in.size()) {
                pipe.CloseHandle(CPipe::eStdIn);
            }
        }

        EIO_Status rstatus;
        size_t bytes_read;

        // read stdout
        if (!out_done) {
            rstatus = pipe.Read(buf, buf_size, &bytes_read);
            std_out.append(buf, bytes_read);
            if (rstatus != eIO_Success && rstatus != eIO_Timeout) {
                out_done = true;
            }
        }

        // read stderr
        if (!err_done) {
            rstatus = pipe.Read(buf, buf_size, &bytes_read, CPipe::eStdErr);
            std_err.append(buf, bytes_read);
            if (rstatus != eIO_Success && rstatus != eIO_Timeout) {
                err_done = true;
            }
        }

        // check whether total time (timeout paramater) exceeded
        if (timeout) {
            if (sw.Elapsed() > max_time) {
                pipe.SetTimeout(eIO_Close, &short_time);
                break;
            }
        }
    }

    int exit_value;
    pipe.Close(&exit_value);

    // return the value with which the process exited
    return exit_value;
}


END_NCBI_SCOPE
