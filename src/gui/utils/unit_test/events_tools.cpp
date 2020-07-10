/*  $Id: events_tools.cpp 40279 2018-01-19 17:48:49Z katargir $
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


BEGIN_NCBI_SCOPE

static const char* kTraceSeparator = " | ";

///////////////////////////////////////////////////////////////////////////////
/// CTraceEvent

CTraceEvent::CTraceEvent(TEventClass ecl, TEventID eid)
:   CEvent(ecl, eid)
{
}


CTraceEvent::CTraceEvent(TEventClass ecl, TEventID eid, IEventAttachment* att,
                         EOwnershipPolicy policy, TEventSender* sender)
: CEvent(ecl, eid, att, policy, sender)
{
}


const CTraceEvent::TTrace& CTraceEvent::GetTrace() const
{
    return m_Trace;
}


void CTraceEvent::AddTrace(const TTrace& trace)
{
    if( ! m_Trace.empty())  {
        m_Trace += kTraceSeparator;
    }
    m_Trace += trace;
}


///////////////////////////////////////////////////////////////////////////////
/// CTracingHandler

CTracingHandler::CTracingHandler(int id)
:   m_ID(id),
    m_Trace(NULL)
{
}


CTracingHandler::CTracingHandler(int id, TTrace& trace)
:   m_ID(id),
    m_Trace(&trace)
{
}


BEGIN_EVENT_MAP(CTracingHandler, CEventHandler)
    ON_EVENT(CTraceEvent, CTraceEvent::eType_A, &CTracingHandler::OnEvent_A)
    ON_EVENT(CTraceEvent, CTraceEvent::eType_B, &CTracingHandler::OnEvent_B)

    ON_EVENT(CTraceEvent, CTraceEvent::eType_A, &CTracingHandler::OnMessage_A)
    ON_EVENT(CTraceEvent, CTraceEvent::eType_B, &CTracingHandler::OnMessage_B)

    ON_COMMAND(eCmd_1, &CTracingHandler::OnCommand_1)
    ON_COMMAND(eCmd_2, &CTracingHandler::OnCommand_2)
    ON_COMMAND_RANGE(eCmd_3, eCmd_4, &CTracingHandler::OnCommand_Range)

    ON_UPDATE_COMMAND_UI(eCmd_1, &CTracingHandler::OnUpdateCommand_1)
    ON_UPDATE_COMMAND_UI(eCmd_2, &CTracingHandler::OnUpdateCommand_2)
    ON_UPDATE_COMMAND_UI_RANGE(eCmd_3, eCmd_4, &CTracingHandler::OnUpdateCommand_Range)
END_EVENT_MAP()


void CTracingHandler::ClearTrace()
{
    m_Trace->clear();
}


const CTracingHandler::TTrace* CTracingHandler::GetTrace()
{
    return m_Trace;
}


void CTracingHandler::OnEvent_A(CEvent* evt)
{
    x_AddTrace(evt, "OnEvent_A");
}


void CTracingHandler::OnEvent_B(CEvent* evt)
{
    x_AddTrace(evt, "OnEvent_B");
}


void CTracingHandler::OnMessage_A(CEvent* evt)
{
    x_AddTrace(evt, "OnMessage_A");
}


void CTracingHandler::OnMessage_B(CEvent* evt)
{
    x_AddTrace(evt, "OnMessage_B");
}


void CTracingHandler::OnCommand_1()
{
    x_AddTrace(x_GetMethodTrace("OnCommand_1"));
}


void CTracingHandler::OnCommand_2()
{
    x_AddTrace(x_GetMethodTrace("OnCommand_2"));
}


void CTracingHandler::OnCommand_Range(TCmdID cmd)
{
    string s = x_GetMethodTrace("OnCommand_Range ");
    s += NStr::IntToString(cmd);
    x_AddTrace(s);
}


void CTracingHandler::OnUpdateCommand_1(ICmdUI* pCmdUI)
{
    x_AddTrace(x_GetMethodTrace("OnUpdateCommand_1"));
    pCmdUI->Enable(true);
}


void CTracingHandler::OnUpdateCommand_2(ICmdUI* pCmdUI)
{
    x_AddTrace(x_GetMethodTrace("OnUpdateCommand_2"));
    pCmdUI->Enable(true);
}


void CTracingHandler::OnUpdateCommand_Range(ICmdUI* pCmdUI)
{
    string s = x_GetMethodTrace("OnUpdateCommand_Range ");
    s += NStr::IntToString(pCmdUI->GetCommand());
    x_AddTrace(s);

    pCmdUI->Enable(true);
}


string CTracingHandler::x_GetMethodTrace(const string& method) const
{
    string s = NStr::IntToString(m_ID);
    s += "::";
    s += method;
    return s;
}


void CTracingHandler::x_AddTrace(CEvent* evt, const string& method)
{
    CTraceEvent* tr_evt = dynamic_cast<CTraceEvent*>(evt);
    _ASSERT(tr_evt);

    tr_evt->AddTrace(x_GetMethodTrace(method));
}


void CTracingHandler::x_AddTrace(const string& trace)
{
    _ASSERT(m_Trace);
    *m_Trace += kTraceSeparator;
}


///////////////////////////////////////////////////////////////////////////////
/// CTestCmdUI

CTestCmdUI::CTestCmdUI(TCmdID cmd, const string& label)
:   m_Cmd(cmd), m_Label(label),
    m_Enabled(false), m_Checked(false), m_Radio(false), m_Visited(false)
{
}


CTestCmdUI::CTestCmdUI(TCmdID cmd, const string& label, bool en, bool check, bool radio)
:   m_Cmd(cmd), m_Label(label),
    m_Enabled(en), m_Checked(check), m_Radio(radio), m_Visited(false)
{
}


TCmdID  CTestCmdUI::GetCommand() const
{
    return m_Cmd;
}


void    CTestCmdUI::Enable(bool en)
{
    m_Enabled = en;
    m_Visited = true;
}


void    CTestCmdUI::SetCheck(bool set)
{
    m_Checked = true;
    m_Visited = true;
}


void    CTestCmdUI::SetRadio(bool set)
{
    m_Radio = true;
    m_Visited = true;
}


void    CTestCmdUI::SetLabel(const string& label)
{
    m_Label = label;
    m_Visited = true;
}


END_NCBI_SCOPE
