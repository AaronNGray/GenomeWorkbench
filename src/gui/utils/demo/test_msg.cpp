/*  $Id: test_msg.cpp 34696 2016-02-01 22:22:53Z katargir $
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
 * Authors:  Lou Friedman
 *
 * File Description:
 *    Test messgae pool.
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbienv.hpp>
#include <corelib/ncbiargs.hpp>
#include <gui/utils/event.hpp>
#include <gui/utils/event_handler.hpp>

USING_SCOPE(ncbi);

class CTestEvent : public CEvent
{
public:
    enum EEvent {
        eEvent_First = eEvent_MinClientID,
        eEvent_Second
    };

    CTestEvent(EEvent evt)
    {
        m_ID = evt;
    }
};


class Dispatcher : public CEventHandler
{
public:
    virtual void FireEvent(CEvent* evt,
                           EDispatch disp_how = eDispatch_Default,
                           int pool_name = ePool_Default)
    {
        Dispatch(evt, disp_how, pool_name);
    }
};


class CTestReceiver : public CEventHandler
{
public:
    typedef CTestEvent TEvent;

    CTestReceiver(const string& name)
        : m_Name(name) {}

    void OnEventReceipt(CEvent* ev)
    {
        TEvent* evt = dynamic_cast<TEvent*>(ev);
        LOG_POST(Warning <<
            m_Name << "::OnEventReceipt(this=" << this <<
            ", ev=" << ev << "): received event id = " << evt->GetID());
    }

protected:
    DECLARE_EVENT_MAP();

private:
    string m_Name;
};

BEGIN_EVENT_MAP(CTestReceiver, CEventHandler)
    ON_EVENT(CTestEvent, CTestEvent::eEvent_First,  &CTestReceiver::OnEventReceipt)
    ON_EVENT(CTestEvent, CTestEvent::eEvent_Second, &CTestReceiver::OnEventReceipt)
END_EVENT_MAP()



class CTestDispatcher : public Dispatcher
{
public:
    typedef CTestEvent TEvent;

    CTestDispatcher(const string& name)
        : m_Name(name) { }

    void Fire(int pool = ePool_Default)
    {
        TEvent event(TEvent::eEvent_First);
        LOG_POST(Warning <<
            m_Name << "::Fire(this=" << this <<
            "): firing " << &event);
        FireEvent(&event, eDispatch_Default, pool);
    }

    void FireFirst(int pool = ePool_Default)
    {
        TEvent event(TEvent::eEvent_First);
        LOG_POST(Warning <<
            m_Name << "::Fire(this=" << this <<
            "): firing " << &event);
        FireEvent(&event, eDispatch_FirstHandler, pool);
    }

    void OnEventReceipt(CEvent* ev)
    {
        TEvent* evt = dynamic_cast<TEvent*>(ev);
        LOG_POST(Warning <<
            m_Name << "::OnEventReceipt(this=" << this <<
            ", ev=" << ev << "): received event id = " << evt->GetID());
    }

protected:
    DECLARE_EVENT_MAP();

private:
    string m_Name;
};

BEGIN_EVENT_MAP(CTestDispatcher, CEventHandler)
    ON_EVENT(CTestEvent, CTestEvent::eEvent_First,  &CTestDispatcher::OnEventReceipt)
    ON_EVENT(CTestEvent, CTestEvent::eEvent_Second, &CTestDispatcher::OnEventReceipt)
END_EVENT_MAP()


class CTestDispatchApp : public CNcbiApplication
{
public:

    virtual void Init(void);
    virtual int  Run(void);
    virtual void Exit(void);
};


void CTestDispatchApp::Init(void)
{
    // Create command-line argument descriptions class
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "Event dispatching test application");

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());
}


int CTestDispatchApp::Run(void)
{
    // Get arguments
    CArgs args = GetArgs();

    LOG_POST(Warning << "Testing standard event firing...");
    {{
        CTestDispatcher disp("disp");
        CTestReceiver   rec1("rec1");
        CTestReceiver   rec2("rec2");
        disp.AddListener(&rec1);
        disp.AddListener(&rec2);

        LOG_POST(Warning << "firing to all...");
        disp.Fire();
        LOG_POST(Warning << "firing to first...");
        disp.FireFirst();
    }}

    LOG_POST(Warning << "Testing hierarchical event firing...");
    {{
        CTestDispatcher disp1("disp1");
        CTestReceiver   rec1("rec1");
        CTestReceiver   rec2("rec2");

        CTestDispatcher disp2("disp2");
        CTestReceiver   rec3("rec3");
        CTestReceiver   rec4("rec4");

        disp1.AddListener(&rec1, CEventHandler::ePool_Child);
        disp1.AddListener(&rec2, CEventHandler::ePool_Child);
        disp1.AddListener(&disp2, CEventHandler::ePool_Child);
        disp2.AddListener(&disp1, CEventHandler::ePool_Parent);

        disp2.AddListener(&rec3, CEventHandler::ePool_Child);
        disp2.AddListener(&rec4, CEventHandler::ePool_Child);

        LOG_POST(Warning << "firing from top to all: disp1");
        disp1.Fire();
        LOG_POST(Warning << "firing from middle to all: disp2");
        disp2.Fire();

        LOG_POST(Warning << "firing from top to first: disp1");
        disp1.FireFirst();
        LOG_POST(Warning << "firing from middle to first: disp2");
        disp2.FireFirst();

        LOG_POST(Warning << "firing to top to child: disp1");
        disp1.Fire(CEventHandler::ePool_Child);
        LOG_POST(Warning << "firing from middle to parent: disp2");
        disp2.Fire(CEventHandler::ePool_Parent);
    }}

    LOG_POST(Warning << "Testing cyclical event firing...");
    {{
        /// this is intentionally cyclical
        CTestDispatcher disp1("disp1");
        CTestDispatcher disp2("disp2");
        CTestDispatcher disp3("disp3");
        CTestDispatcher disp4("disp4");

        disp1.AddListener(&disp2);
        disp2.AddListener(&disp3);
        disp3.AddListener(&disp4);
        disp4.AddListener(&disp1);

        LOG_POST(Warning << "firing from disp1 to all");
        disp1.Fire();
        LOG_POST(Warning << "firing from disp2 to all");
        disp2.Fire();

        LOG_POST(Warning << "firing from disp1 to first");
        disp1.FireFirst();
        LOG_POST(Warning << "firing from disp2 to first");
        disp2.FireFirst();
    }}

    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//  Cleanup


void CTestDispatchApp::Exit(void)
{
    SetDiagStream(0);
}



/////////////////////////////////////////////////////////////////////////////
//  MAIN


int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // Execute main application function
    return CTestDispatchApp().AppMain(argc, argv);
}
