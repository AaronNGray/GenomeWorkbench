/*  $Id: gbench_monitor.cpp 38599 2017-06-01 18:57:09Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbienv.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/ncbiargs.hpp>
#include <corelib/ncbiexec.hpp>
#include <corelib/ncbi_process.hpp>
#include <corelib/ncbi_system.hpp>
#include <connect/services/neticache_client.hpp>

BEGIN_NCBI_SCOPE

class CGBenchMonitorApp :  public CNcbiApplication
{
public:
    /// default ctor
    CGBenchMonitorApp(void);

private:
    void    Init(void);
    int     Run (void);
    void    Exit(void);

    string x_GetFeedbackAgentPath();
};


CGBenchMonitorApp::CGBenchMonitorApp()
{
}

void CGBenchMonitorApp::Init()
{
    // Create command - line argument descriptions class
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "NCBI Genome Workbench App Monitor");

    arg_desc->AddOptionalKey("inst", "string",
                     "application instance name",
                     CArgDescriptions::eString);

    arg_desc->AddPositional("pid", "Process to monitor",
                            CArgDescriptions::eInteger);

    arg_desc->AddPositional("file", "PID file to monitor",
                            CArgDescriptions::eString);

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());
}


int CGBenchMonitorApp::Run()
{
    const CArgs& args = GetArgs();

    try {
        size_t pid = args["pid"].AsInteger();
        string path = args["file"].AsString();
        string instVal;

        if (args["inst"].HasValue())
            instVal = args["inst"].AsString();

        ///
        /// step 1: clean up any stale PID files
        ///
        {{
            string directory;
            CDirEntry::SplitPath(path, &directory);
            vector<string> files;
            vector<string> paths;
            vector<string> masks;
            paths.push_back(directory);
            masks.push_back("gbench-pid.*");

            FindFiles(files,
                      paths.begin(), paths.end(), masks.begin(), masks.end(),
                      fFF_File | fFF_Recursive);

            ITERATE (vector<string>, iter, files) {
                try {
                    string::size_type pos = iter->find_last_of(".");
                    if (pos == string::npos) {
                        continue;
                    }

                    size_t pid = NStr::StringToInt(iter->substr(pos + 1));
                    CProcess proc(pid, CProcess::ePid);
                    if ( !proc.IsAlive() ) {
                        CFile(*iter).Remove();
                    }
                }
                catch (std::exception&) {
                }
            }

        }}

        /// we poll and monitor this pid
        CProcess proc(pid, CProcess::ePid);
        while (true) {
            SleepSec(3);
            if ( !proc.IsAlive() ) {
                break;
            }
        }

#ifdef NCBI_OS_DARWIN
    path = NStr::Replace(path, "\\", "");
#endif
        /// our monitored process has exited
        /// check to see if it cleaned up its PID file
        if (CFile(path).Exists()) {
#ifndef _DEBUG
            /// launch feedback agent
            LOG_POST(Error << "trapped error - launching feedback process...");

            string strPid = NStr::SizetToString(pid);
            const char* args_c[5];
            if (!instVal.empty()) {
                args_c[1] = "-inst";
                args_c[2] = instVal.c_str();
                args_c[3] = strPid.c_str();
                args_c[4] = NULL;
            }
            else {
                args_c[1] = strPid.c_str();
                args_c[2] = NULL;
            }

            CExec::SpawnV(CExec::eNoWait, x_GetFeedbackAgentPath().c_str(), args_c);
#endif
        } else {
            LOG_POST(Info << "PID file \"" << path << "\" doesn't exist. Normal Genome Workbench exit.");
        }
    }
    catch (std::exception& e) {
        LOG_POST(Error << "error in monitor: " << e.what());
    }
    catch (...) {
        LOG_POST(Error << "unknown error in monitor");
    }

    return 0;
}


void CGBenchMonitorApp::Exit()
{
    SetDiagStream(0);
}


string CGBenchMonitorApp::x_GetFeedbackAgentPath()
{
    string dir;
    CDirEntry::SplitPath(GetProgramExecutablePath(eFollowLinks), &dir);
    return dir + "gbench_feedback_agent";
}


END_NCBI_SCOPE


/////////////////////////////////////////////////////////////////////////////
//  MAIN

int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // Execute main application function
    return ncbi::CGBenchMonitorApp().AppMain(argc, argv, 0, ncbi::eDS_Default, 0);
}
