/*  $Id: test_app_job.cpp 40216 2018-01-09 21:26:33Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbiapp.hpp>
#include <corelib/ncbienv.hpp>
#include <corelib/ncbiargs.hpp>

#include <corelib/ncbi_system.hpp>

#include <gui/utils/app_job.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/thread_pool_engine.hpp>
//#include <gui/objutils/object_manager_engine.hpp>
#include <gui/utils/scheduler_engine.hpp>

#include "test_client.hpp"

USING_NCBI_SCOPE;

////////////////////////////////////////////////////////////////////////////////
/// CAppJobTestApplication
class CAppJobTestApplication : public CNcbiApplication
{
public:

    virtual void Init(void);
    virtual int  Run(void);
    virtual void Exit(void);

    void    CreateClients();
    void    ClientsPoll();

    CPrimeNumberClient*  m_Client1;
    CAppJobDispatcher    m_Disp;
};



void CAppJobTestApplication::Init(void)
{
    // Create command-line argument descriptions class
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "CLabel label interface test application");

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());
}


int CAppJobTestApplication::Run(void)
{
    // Get arguments
    // const CArgs& args = GetArgs();

    m_Disp.RegisterEngine("ThreadPool", *new CThreadPoolEngine(2, 1000));
    //m_Disp.RegisterEngine("ObjectManager", *new CObjectManagerEngine());
    m_Disp.RegisterEngine("Scheduler", *new CSchedulerEngine());

    CreateClients();

    int delay = 400;
    for(int i = 0; i < 100; i++ )   {
        SleepMilliSec(delay);

        // let Clients to query the dispatcher
        ClientsPoll();

        SleepMilliSec(delay);

        m_Disp.IdleCallback();

        SleepMilliSec(delay);

        if( i == 5 ) {
            m_Client1->CancelJobs();
        }
        /// to pump Application Event Queue
        CEventHandler::HandlePostRequest();
    }

    return 0;
}


void CAppJobTestApplication::CreateClients()
{
    m_Client1 = new CPrimeNumberClient(m_Disp);
    m_Client1->StartJobs(3);
}


void CAppJobTestApplication::ClientsPoll()
{
    m_Client1->CheckStatus();
    m_Client1->CheckProgress();
}


/////////////////////////////////////////////////////////////////////////////
//  Cleanup


void CAppJobTestApplication::Exit(void)
{
    SetDiagStream(0);
}


/////////////////////////////////////////////////////////////////////////////
//  MAIN


int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // Execute main application function
    return CAppJobTestApplication().AppMain(argc, argv);
}
