/*  $Id: conn_test_thread.cpp 26247 2012-08-10 14:53:46Z katargir $
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

#include <connect/ncbi_conn_test.hpp>
#include <corelib/ncbitime.hpp>

#include <gui/utils/conn_test_thread.hpp>

BEGIN_NCBI_SCOPE

CConnTestThread::CConnTestThread(unsigned int timeout) : m_Timeout(timeout), m_GoodConnection(false)
{
}

void* CConnTestThread::Main(void)
{
    CStopWatch sw(CStopWatch::eStart);

    try {
        STimeout tmo;
        tmo.sec  = (unsigned int)m_Timeout;
        tmo.usec = (unsigned int)0;

        CNcbiOstrstream ostr;
        CConnTest conn_test(&tmo, &ostr);
        conn_test.SetCanceledCallback(this);

        conn_test.SetEmail("gbench-bugs@ncbi.nlm.nih.gov");
        conn_test.SetDebugPrintout(eDebugPrintout_Data);

        SetDiagFilter(eDiagFilter_Trace, "!/corelib !/objmgr");

        CDiagCollectGuard guard(eDiag_Info, eDiag_Trace);
        CConnTest::EStage everything = CConnTest::eStatefulService;
        EIO_Status status = conn_test.Execute(everything);
        if (status != eIO_Success) {
            guard.Release(CDiagCollectGuard::ePrint);
            m_GoodConnection = false;
            m_ConnInfo = CNcbiOstrstreamToString(ostr);
        }
        else {
            m_GoodConnection = true;
            m_ConnInfo = "No problems with connection found";
        }
    }
    catch (const std::exception&) { // Exception should only be thrown if cancelled
        m_GoodConnection = false;
        m_ConnInfo = "Connection test cancelled";
    }

    SetDiagFilter(eDiagFilter_Trace, "!/corelib !/connect !/objmgr");
    sw.Stop();

    m_Finished.Set(1);
    LOG_POST(Info << "ConnTest took: " << sw.AsSmartString(CTimeSpan::eSSP_Millisecond));

    return 0;
}

END_NCBI_SCOPE
