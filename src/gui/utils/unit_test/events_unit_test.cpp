/*  $Id: events_unit_test.cpp 17781 2008-09-15 21:02:42Z yazhuk $
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
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>

#include "events_tools.hpp"
#include "events_unit_test.hpp"

// Keep Boost's inclusion of <limits> from breaking under old WorkShop versions.
#if defined(numeric_limits)  &&  defined(NCBI_NUMERIC_LIMITS)
#  undef numeric_limits
#endif

#include <boost/version.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_log.hpp>
// #include <boost/test/unit_test_result.hpp>
#include <common/test_assert.h>  /* This header must go last */

using boost::unit_test_framework::test_suite;

BEGIN_NCBI_SCOPE


////////////////////////////////////////////////////////////////////////////////
/// CTracingHandler_1

CTracingHandler_1::CTracingHandler_1(int id)
: CTracingHandler(id)
{
}

// deriving event map
BEGIN_EVENT_MAP(CTracingHandler_1, CTracingHandler)
END_EVENT_MAP()


////////////////////////////////////////////////////////////////////////////////
/// CTracingHandler_2

CTracingHandler_2::CTracingHandler_2(int id)
: CTracingHandler_1(id)
{
}

// the map is derived from CTracingHandler_1
BEGIN_EVENT_MAP(CTracingHandler_2, CTracingHandler_1)
    ON_EVENT(CTraceEvent, CTraceEvent::eType_C, &CTracingHandler_2::OnEvent_C_Derived)
    ON_EVENT(CTraceEvent, CTraceEvent::eType_A, &CTracingHandler_2::OnEvent_A_Derived)
END_EVENT_MAP()


void CTracingHandler_2::OnEvent_A_Derived(CEvent* evt)
{
    x_AddTrace(evt, "OnEvent_A_Derived");
}


void CTracingHandler_2::OnEvent_C_Derived(CEvent* evt)
{
    x_AddTrace(evt, "OnEvent_A_Derived");
}


////////////////////////////////////////////////////////////////////////////////
/// CTestEventAttachment

int CTestEventAttachment::sm_Count = 0;


CTestEventAttachment::CTestEventAttachment()
{
    sm_Count++;
}


CTestEventAttachment::~CTestEventAttachment()
{
    sm_Count--;
}


int CTestEventAttachment::Count()
{
    return sm_Count;
}


////////////////////////////////////////////////////////////////////////////////
/// CTestEventAttachment

CTestCommandHandler::CTestCommandHandler(int id, TTrace& trace)
: CTracingHandler(id, trace)
{
}

BEGIN_EVENT_MAP(CTestCommandHandler, CTracingHandler)
    ON_COMMAND(eCmd_5, &CTestCommandHandler::OnCommand_5)
    ON_UPDATE_COMMAND_UI(eCmd_5, &CTestCommandHandler::OnUpdateDisable)
    ON_UPDATE_COMMAND_UI(eCmd_1, &CTestCommandHandler::OnUpdateCheck)
END_EVENT_MAP()


void CTestCommandHandler::OnCommand_5()
{
    *m_Trace += x_GetMethodTrace("OnCommand_1");
}


void CTestCommandHandler::OnUpdateDisable(ICmdUI* pCmdUI)
{
    *m_Trace += x_GetMethodTrace("OnUpdateDisable");
    pCmdUI->Enable(false);
}


void CTestCommandHandler::OnUpdateCheck(ICmdUI* pCmdUI)
{
    *m_Trace += x_GetMethodTrace("OnUpdateCheck");
    pCmdUI->Enable(true);
    pCmdUI->SetCheck(true);
}


////////////////////////////////////////////////////////////////////////////////
/// CBasicEventsTest

void TestEventDispatch(CTracingHandler& src, int id,
                       CEventHandler::EDispatch disp_how, CTracingHandler& dst)
{
    CTraceEvent evt(CEvent::eEvent_Message, id);
    src.Send(&evt, disp_how);

    CTraceEvent test_evt(CEvent::eEvent_Message, id);
    dst.OnEvent(&test_evt);

    BOOST_CHECK(evt.GetTrace() == test_evt.GetTrace());
}


void TestOnEvent(CEvent::TEventClass cls, CEvent::TEventID id,
                                     CTracingHandler& handler, FOnEventHandler func)
{
    CTraceEvent evt(cls, id);
    handler.OnEvent(&evt);

    CTraceEvent test_evt(cls, id);
    (handler.*func)(&test_evt); // call the right function directly

    BOOST_CHECK(evt.GetTrace() == test_evt.GetTrace());
}


void TestCommand(TCmdID cmd, bool custom,
                                     CTracingHandler& handler, FOnCommandHandler func)
{
    CEvent* evt = NULL;
    if(custom)  {
        evt = new CTraceEvent(CEvent::eEvent_Command, cmd);
    } else {
        evt = new CEvent(CEvent::eEvent_Command, cmd);
    }

    handler.ClearTrace();
    handler.OnEvent(evt);
    string trace = *handler.GetTrace();

    handler.ClearTrace();
    (handler.*func)();
    BOOST_CHECK(trace == *handler.GetTrace());
}


void TestCommand(TCmdID cmd, CTracingHandler& handler,
                                     FOnCommandRangeHandler func)
{
    CTraceEvent evt(CEvent::eEvent_Command, cmd);

    handler.ClearTrace();
    handler.OnEvent(&evt);
    string trace = *handler.GetTrace();

    handler.ClearTrace();
    (handler.*func)(cmd);
    BOOST_CHECK(trace == *handler.GetTrace());
}


// tests that command update can reach its handler function
void TestCommandUpdateReached(CTracingHandler& handler, TCmdID cmd)
{
    CTestCmdUI ui(eCmd_1, "", false, false, false);
    CTraceEvent evt(CEvent::eEvent_CommandUpdate, eCmd_1, &ui, CEvent::eRelease, NULL);
    handler.OnEvent(&evt);

    BOOST_CHECK(ui.IsVisited()  &&  ui.IsEnabled());
}


// helper function to check pool size
void AssertPoolSize(CTracingHandler& handler, int pool, int size)
{
    const CEventHandler::TListeners* ls = handler.GetListeners(pool);
    if(size == 0)   {
        BOOST_CHECK(ls == NULL);
    } else {
        BOOST_CHECK(ls != NULL  &&  (int) ls->size() == size);
    }
}



void PostEvent(CEventHandler& post_dst, CEventHandler& send_dst, int cmd)
{
    CRef<CEvent> evt(new CTraceEvent(CEvent::eEvent_Command, cmd));
    post_dst.Post(evt);
    send_dst.Send(&*evt);
}

void TestPostVersusSend(CEventHandler& target, CEventHandler::EDispatch disp_how,
                          int pool_name)
{
    CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
    target.Post(CRef<CEvent>(&evt));

    CTraceEvent test_evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
    target.Send(&test_evt);

    bool ok = CEventHandler::HandlePostRequest();

    BOOST_CHECK(ok);
    BOOST_CHECK(evt.GetTrace() == test_evt.GetTrace());
}


void TestEventMap()
{
    string test_trace;
    CTracingHandler handler(0, test_trace);

    TestOnEvent(CEvent::eEvent_Message, CTraceEvent::eType_A, handler, &CTracingHandler::OnEvent_A);
    TestOnEvent(CEvent::eEvent_Message, CTraceEvent::eType_B, handler, &CTracingHandler::OnEvent_B);

    TestOnEvent(CEvent::eEvent_Message, CTraceEvent::eType_A, handler, &CTracingHandler::OnMessage_A);
    TestOnEvent(CEvent::eEvent_Message, CTraceEvent::eType_B, handler, &CTracingHandler::OnMessage_B);

    // event eType_C - not handled
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_C);
        handler.OnEvent(&evt);

        BOOST_CHECK(evt.GetTrace() == ""); // not handled - trace empty
    }

    // test commands
    TestCommand(eCmd_1, false, handler, &CTracingHandler::OnCommand_1);
    TestCommand(eCmd_2, false, handler, &CTracingHandler::OnCommand_2);
    TestCommand(eCmd_1, true, handler, &CTracingHandler::OnCommand_1);
    TestCommand(eCmd_2, true, handler, &CTracingHandler::OnCommand_2);

    // test commands using range handlers
    TestCommand(eCmd_3, handler, &CTracingHandler::OnCommand_Range);
    TestCommand(eCmd_4, handler, &CTracingHandler::OnCommand_Range);

    // test command not handled
    {
        CTraceEvent evt(CEvent::eEvent_Command, eCmd_5);
        handler.ClearTrace();
        handler.OnEvent(&evt);

        BOOST_CHECK(*handler.GetTrace() == "");
    }

    // test command updates
    TestCommandUpdateReached(handler, eCmd_1);
    TestCommandUpdateReached(handler, eCmd_2);

    // test command updates using range handlers
    TestCommandUpdateReached(handler, eCmd_4);
    TestCommandUpdateReached(handler, eCmd_3);
}


/// Create an instance of CTracingHandler_2 class extending the event map of
/// its base - CTracingHandler. Test how both event maps work together.
void TestMapInheritance()
{
    CTracingHandler_2 handler_2; // derived handler (grand child)

    // test CTracingHandler_2 overriding eType_A event handling
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_2.OnEvent(&evt);

        CTraceEvent test_evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_2.OnEvent_A_Derived(&test_evt); // derived function must be called

        BOOST_CHECK(evt.GetTrace() == test_evt.GetTrace());
    }
    // test CTracingHandler_2 adding handling for new event eType_C
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_C);
        handler_2.OnEvent(&evt);

        CTraceEvent test_evt(CEvent::eEvent_Message, CTraceEvent::eType_C);
        handler_2.OnEvent_C_Derived(&test_evt);

        BOOST_CHECK(evt.GetTrace() == test_evt.GetTrace());
    }
    // check that the base class does not handle eType_C
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_C);
        CTracingHandler handler;
        handler.OnEvent(&evt);

        BOOST_CHECK(evt.GetTrace() == ""); // not handled
    }
}


// test operation with Pools and Listeners
void TestListenersMgmt()
{
    CTracingHandler handler(0);
    CTracingHandler handler_1(1);
    CTracingHandler handler_2(2);
    CTracingHandler handler_3(3);

    // setup listeners
    handler.AddListener(&handler_1);
    handler.AddListener(&handler_2, CEventHandler::ePool_Parent);
    handler.AddListener(&handler_3, CEventHandler::ePool_Child);

    // check that listeners attached OK
    BOOST_CHECK(handler.HasListener(&handler_1));

    BOOST_CHECK(handler.HasListener(&handler_2));
    BOOST_CHECK(handler.HasListener(&handler_2, CEventHandler::ePool_Parent));

    BOOST_CHECK(handler.HasListener(&handler_3));
    BOOST_CHECK(handler.HasListener(&handler_3, CEventHandler::ePool_Child));

    // adding listeners is not symmetric - check it
    BOOST_CHECK( ! handler_1.HasListener(&handler));
    BOOST_CHECK( ! handler_2.HasListener(&handler));
    BOOST_CHECK( ! handler_3.HasListener(&handler));

    // test pool sizes
    AssertPoolSize(handler, CEventHandler::ePool_Default, 3); // all listeners are here
    AssertPoolSize(handler, CEventHandler::ePool_Parent, 1);
    AssertPoolSize(handler, CEventHandler::ePool_Child, 1);

    // check non-existing pool
    AssertPoolSize(handler, 13, 0); // empty

    // remove handler_2 - must be removed from both pools
    handler.RemoveListener(&handler_2);

    BOOST_CHECK(handler.HasListener(&handler_2, CEventHandler::ePool_Default) == false);
    BOOST_CHECK(handler.HasListener(&handler_2, CEventHandler::ePool_Parent) == false);
    AssertPoolSize(handler, CEventHandler::ePool_Default, 2);

    // test Remove All - all polls must be empty
    handler.RemoveAllListeners();

    const CEventHandler::TListeners* ls = handler.GetListeners();
    BOOST_CHECK(ls == NULL);

    ls = handler.GetListeners(CEventHandler::ePool_Parent);
    BOOST_CHECK(ls == NULL);

    // prepare new setup - all listeers in the Child pool
    handler.AddListener(&handler_1, CEventHandler::ePool_Child);
    handler.AddListener(&handler_2, CEventHandler::ePool_Child);
    handler.AddListener(&handler_3, CEventHandler::ePool_Child);

    AssertPoolSize(handler, CEventHandler::ePool_Child, 3);

    // add the same listener twice
    handler.AddListener(&handler_2, CEventHandler::ePool_Child);
    AssertPoolSize(handler, CEventHandler::ePool_Child, 3);

    // add NULL - must be ignored
    BOOST_CHECK_NO_THROW(handler.AddListener(NULL, CEventHandler::ePool_Child));
    AssertPoolSize(handler, CEventHandler::ePool_Child, 3);

    // remove NULL - must be ignored
    BOOST_CHECK_NO_THROW(handler.RemoveListener(NULL));

    // remove handler_1 twice
    handler.RemoveListener(&handler_1);
    BOOST_CHECK_NO_THROW(handler.RemoveListener(&handler_1));
    AssertPoolSize(handler, CEventHandler::ePool_Child, 2);
}


/// CEventHandler has several entry points - OnEvent(), Dispatch(), Send() and Post().
/// Test logic of every entry point
void TestEntryPoints()
{
    CTracingHandler handler_1(1);
    CTracingHandler handler_2(2);

    handler_1.AddListener(&handler_2);

    // OnEvent - direct handling by handler_1
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.OnEvent(&evt);

        CTraceEvent test_evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.OnEvent_A(&test_evt);

        BOOST_CHECK(evt.GetTrace() == test_evt.GetTrace());
    }
    // Dispatch - handled only by the handler_2
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.Dispatch(&evt, CEventHandler::eDispatch_AllHandlers,
                           CEventHandler::ePool_Default);

        CTraceEvent test_evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_2.OnEvent_A(&test_evt);

        BOOST_CHECK(evt.GetTrace() == test_evt.GetTrace());
    }
    // Send() - handled by both handler_1 and handler_2
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.Send(&evt, CEventHandler::eDispatch_AllHandlers,
                       CEventHandler::ePool_Default);

        CTraceEvent test_evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.OnEvent_A(&test_evt);
        handler_2.OnEvent_A(&test_evt);

        BOOST_CHECK(evt.GetTrace() == test_evt.GetTrace());
    }
    // Post() - handled by both handler_1 and handler_2 asynchronously
    {
        CRef<CTraceEvent> evt(new CTraceEvent(CEvent::eEvent_Message, CTraceEvent::eType_A));
        handler_1.Post(CRef<CEvent>(evt),
                       CEventHandler::eDispatch_AllHandlers,
                       CEventHandler::ePool_Default);

        CTraceEvent test_evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.OnEvent_A(&test_evt);
        handler_2.OnEvent_A(&test_evt);

        bool ok = CEventHandler::HandlePostRequest();

        BOOST_CHECK(ok);
        BOOST_CHECK(evt->GetTrace() == test_evt.GetTrace());
    }
}


// specifically tests async event delivery
void TestPost()
{
    CTracingHandler handler_1(1);

    // check that Post() works as Send() except for timing
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.Post(CRef<CEvent>(&evt));

        CTraceEvent test_evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.Send(&test_evt);

        BOOST_CHECK(evt.GetTrace().empty()); // nothing happened yet

        bool ok = CEventHandler::HandlePostRequest();

        BOOST_CHECK(ok);
        BOOST_CHECK(evt.GetTrace() == test_evt.GetTrace());
    }

    CTracingHandler_2 handler_2(2);
    CTracingHandler_2 handler_3(3);

    handler_1.AddListener(&handler_2);
    handler_1.AddListener(&handler_3, CEventHandler::ePool_Child);

    // test dispatching (startegies and pools)
    TestPostVersusSend(handler_1, CEventHandler::eDispatch_SelfOnly, CEventHandler::ePool_Default);
    TestPostVersusSend(handler_1, CEventHandler::eDispatch_FirstHandler, CEventHandler::ePool_Default);
    TestPostVersusSend(handler_1, CEventHandler::eDispatch_AllHandlers, CEventHandler::ePool_Default);
    TestPostVersusSend(handler_1, CEventHandler::eDispatch_SelfOnly, CEventHandler::ePool_Child);
    TestPostVersusSend(handler_1, CEventHandler::eDispatch_FirstHandler, CEventHandler::ePool_Child);
    TestPostVersusSend(handler_1, CEventHandler::eDispatch_AllHandlers, CEventHandler::ePool_Child);

    // test delivering to the dead handler
    {
        CTracingHandler* dead = new CTracingHandler(4);
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        dead->Post(CRef<CEvent>(&evt));
        delete dead;

        bool ok = CEventHandler::HandlePostRequest();
        BOOST_CHECK(ok == false  &&  evt.GetTrace() == "");
    }

    // test sequence of posts to different targets
    {
        string trace_5, trace_5s, trace_6, trace_6s;
        CTracingHandler handler_5(5, trace_5);
        CTracingHandler handler_6(6, trace_6);

        CTracingHandler handler_5s(5, trace_5s);
        CTracingHandler handler_6s(6, trace_6s);

        PostEvent(handler_5, handler_5s, eCmd_1);
        PostEvent(handler_6, handler_6s, eCmd_2);
        PostEvent(handler_6, handler_6s, eCmd_3);
        PostEvent(handler_5, handler_5s, eCmd_4);

        int n = 0;
        while(CEventHandler::HandlePostRequest()) {
            n++;
        }
        BOOST_CHECK(n == 4); // 4 events processed
        BOOST_CHECK(trace_5 == trace_5s);
        BOOST_CHECK(trace_6 == trace_6s);
    }
}


void TestEventAttachment()
{
    CTestEventAttachment* att = new CTestEventAttachment();
    _ASSERT(CTestEventAttachment::Count() == 1);

    CTracingHandler handler;

    // test eRelease policy
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_A, att, CEvent::eRelease, NULL);
        handler.OnEvent(&evt);
    }
    BOOST_CHECK(CTestEventAttachment::Count() == 1); // not deleted

    // test eDelete policy
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_A, att, CEvent::eDelete, NULL);
        handler.OnEvent(&evt);
    }
    BOOST_CHECK(CTestEventAttachment::Count() == 0); // not deleted
    att = NULL;


    //handler.Post(evt);
}


void TestDispatchingOnChain()
{
    CTracingHandler     handler_1(1);
    CTracingHandler_2   handler_2(2); // handles eType_c not handled by CTracingHandler
    CTracingHandler     handler_3(3);

    // make a chain  handler_1 -> handler_2 -> handler_3
    handler_1.AddListener(&handler_2);
    handler_2.AddListener(&handler_3);

    // send eType_A from handler_1 with different disp strategies
    TestEventDispatch(handler_1, CTraceEvent::eType_A,
                        CEventHandler::eDispatch_SelfOnly, handler_1);
    TestEventDispatch(handler_1, CTraceEvent::eType_A,
                        CEventHandler::eDispatch_FirstHandler, handler_1);
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.Send(&evt, CEventHandler::eDispatch_AllHandlers);

        CTraceEvent test_evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.OnEvent(&test_evt);
        handler_2.OnEvent(&test_evt);
        handler_3.OnEvent(&test_evt);

        BOOST_CHECK(evt.GetTrace() == test_evt.GetTrace());
    }

    // eType_C is handled by different handlers depending on strategy
    TestEventDispatch(handler_1, CTraceEvent::eType_C,
                        CEventHandler::eDispatch_SelfOnly, handler_1);
    TestEventDispatch(handler_1, CTraceEvent::eType_C,
                        CEventHandler::eDispatch_FirstHandler, handler_2);
}


void TestDispatchingOnTree()
{
    CTracingHandler     handler_1(1);

    CTracingHandler     handler_2_1(21);
    CTracingHandler     handler_2_2(22);
    CTracingHandler     handler_2_3(23);

    CTracingHandler_2   handler_2_2_1(221);
    CTracingHandler     handler_2_2_2(222);

    // make a tree
    handler_1.AddListener(&handler_2_1);
    handler_1.AddListener(&handler_2_2);
    handler_1.AddListener(&handler_2_3);

    handler_2_2.AddListener(&handler_2_2_1);
    handler_2_2.AddListener(&handler_2_2_2);

    // dispatch to all handlers
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.Send(&evt, CEventHandler::eDispatch_AllHandlers);

        CTraceEvent test_evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.OnEvent(&test_evt);
        handler_2_1.OnEvent(&test_evt);
        handler_2_2.OnEvent(&test_evt);
        handler_2_2_1.OnEvent(&test_evt);
        handler_2_2_2.OnEvent(&test_evt);
        handler_2_3.OnEvent(&test_evt);

        BOOST_CHECK(evt.GetTrace() == test_evt.GetTrace());
    }

    // dispatch to the handler_2_2_1
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_C);
        handler_1.Send(&evt, CEventHandler::eDispatch_FirstHandler);

        CTraceEvent test_evt(CEvent::eEvent_Message, CTraceEvent::eType_C);
        handler_2_2_1.OnEvent(&test_evt);

        BOOST_CHECK(evt.GetTrace() == test_evt.GetTrace());
    }
}


void TestDispatchingOnLoops()
{
    CTracingHandler     handler_1(1);
    CTracingHandler     handler_2(2);
    CTracingHandler     handler_3(3);

    // conncet all handlers
    handler_1.AddListener(&handler_2);
    handler_1.AddListener(&handler_3);

    handler_2.AddListener(&handler_1);
    handler_2.AddListener(&handler_3);

    handler_3.AddListener(&handler_1);
    handler_3.AddListener(&handler_2);

    // dispatch from handler_2
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_2.Send(&evt, CEventHandler::eDispatch_AllHandlers);

        CTraceEvent test_evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_2.OnEvent(&test_evt);
        handler_1.OnEvent(&test_evt);
        handler_3.OnEvent(&test_evt);

        BOOST_CHECK(evt.GetTrace() == test_evt.GetTrace());
    }
    // dispatch from handler_3
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_3.Send(&evt, CEventHandler::eDispatch_AllHandlers);

        CTraceEvent test_evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_3.OnEvent(&test_evt);
        handler_1.OnEvent(&test_evt);
        handler_2.OnEvent(&test_evt);

        BOOST_CHECK(evt.GetTrace() == test_evt.GetTrace());
    }
}


/// test pool isolation by firing events in different pools
void TestDispatchingInPools()
{
    CTracingHandler     handler_1(1);
    CTracingHandler     handler_2(2);
    CTracingHandler     handler_3(3);
    CTracingHandler     handler_4(4);

    // conncet all handlers
    handler_1.AddListener(&handler_2);
    handler_1.AddListener(&handler_3, 7);
    handler_1.AddListener(&handler_4, 7);
    handler_1.AddListener(&handler_4, 13);


    // dispatch to general pool
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.Send(&evt, CEventHandler::eDispatch_AllHandlers);

        CTraceEvent test_evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.OnEvent(&test_evt);
        handler_2.OnEvent(&test_evt);
        handler_3.OnEvent(&test_evt);
        handler_4.OnEvent(&test_evt);

        BOOST_CHECK(evt.GetTrace() == test_evt.GetTrace());
    }
    // dispatch to pool 7
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.Send(&evt, CEventHandler::eDispatch_AllHandlers, 7);

        CTraceEvent test_evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.OnEvent(&test_evt);
        handler_3.OnEvent(&test_evt);
        handler_4.OnEvent(&test_evt);

        BOOST_CHECK(evt.GetTrace() == test_evt.GetTrace());
    }
    // dispatch to pool 13
    {
        CTraceEvent evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.Send(&evt, CEventHandler::eDispatch_AllHandlers, 13);

        CTraceEvent test_evt(CEvent::eEvent_Message, CTraceEvent::eType_A);
        handler_1.OnEvent(&test_evt);
        handler_4.OnEvent(&test_evt);

        BOOST_CHECK(evt.GetTrace() == test_evt.GetTrace());
    }
}


void TestCommandUpdates()
{
    string trace;
    CTestCommandHandler handler(0, trace);

    // command that has handler and so is enebled
    {
        CTestCmdUI ui(eCmd_2, "", false, false, false);
        CTraceEvent evt(CEvent::eEvent_CommandUpdate, eCmd_2, &ui, CEvent::eRelease, NULL);
        handler.OnEvent(&evt);

        BOOST_CHECK(ui.IsVisited()  &&  ui.IsEnabled());
    }
    // command that has handler but is disabled
    {
        CTestCmdUI ui(eCmd_5, "", true, false, false);
        CTraceEvent evt(CEvent::eEvent_CommandUpdate, eCmd_5, &ui, CEvent::eRelease, NULL);
        handler.OnEvent(&evt);

        BOOST_CHECK(ui.IsVisited()  &&  ui.IsEnabled() == false);
    }
    // command that does not have a handler and so is disabled
    {
        CTestCmdUI ui(eCmd_6, "", true, false, false);
        CTraceEvent evt(CEvent::eEvent_CommandUpdate, eCmd_6, &ui, CEvent::eRelease, NULL);
        bool handled = handler.OnEvent(&evt);

        BOOST_CHECK(ui.IsVisited() == false  &&  handled == false);
    }
    // command is checked by update handler
    {
        CTestCmdUI ui(eCmd_1, "", false, false, false);
        CTraceEvent evt(CEvent::eEvent_CommandUpdate, eCmd_1, &ui, CEvent::eRelease, NULL);
        handler.OnEvent(&evt);

        BOOST_CHECK(ui.IsVisited()  &&  ui.IsEnabled()  &&  ui.IsChecked());
    }
}


END_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// main entry point for tests

std::ofstream out("events_result.xml"); //TODO


test_suite* init_unit_test_suite(int argc, char * argv[])
{
#if BOOST_VERSION >= 103300
    typedef boost::unit_test_framework::unit_test_log_t TLog;
    TLog& log = boost::unit_test_framework::unit_test_log;
    log.set_stream(out);
    log.set_format(boost::unit_test_framework::XML);
    log.set_threshold_level(boost::unit_test_framework::log_test_suites);
#else
    typedef boost::unit_test_framework::unit_test_log TLog;
    TLog& log = TLog::instance();
    log.set_log_stream(out);
    log.set_log_format("XML");
    log.set_log_threshold_level(boost::unit_test_framework::log_test_suites);
#endif

    //boost::unit_test_framework::unit_test_result::set_report_format("XML");
    test_suite* test = BOOST_TEST_SUITE("gui/utils Event Handling Test.");

    test_suite* suite = BOOST_TEST_SUITE("Event Handling Unit Test");
    suite->add(BOOST_TEST_CASE(ncbi::TestEventMap));
    suite->add(BOOST_TEST_CASE(ncbi::TestMapInheritance));
    suite->add(BOOST_TEST_CASE(ncbi::TestListenersMgmt));
    suite->add(BOOST_TEST_CASE(ncbi::TestEntryPoints));
    suite->add(BOOST_TEST_CASE(ncbi::TestPost));
    suite->add(BOOST_TEST_CASE(ncbi::TestEventAttachment));
    suite->add(BOOST_TEST_CASE(ncbi::TestDispatchingOnChain));
    suite->add(BOOST_TEST_CASE(ncbi::TestDispatchingOnTree));
    suite->add(BOOST_TEST_CASE(ncbi::TestDispatchingOnLoops));
    suite->add(BOOST_TEST_CASE(ncbi::TestDispatchingInPools));
    suite->add(BOOST_TEST_CASE(ncbi::TestCommandUpdates));

    test->add(suite);

    return test;
}
