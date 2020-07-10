#ifndef GUI_UTILS_UNIT_TEST___EVENTS_TOOLS__HPP
#define GUI_UTILS_UNIT_TEST___EVENTS_TOOLS__HPP

/*  $Id: events_tools.hpp 14565 2007-05-18 12:32:01Z dicuccio $
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

#include <corelib/ncbistd.hpp>

#include <gui/utils/event_handler.hpp>
#include <gui/utils/command.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// ETestCmds  - test commands
enum    ETestCmds   {
    eCmd_1 = CEvent::eEvent_MinClientID,
    eCmd_2,
    eCmd_3,
    eCmd_4,
    eCmd_5,
    eCmd_6
};

///////////////////////////////////////////////////////////////////////////////
/// CTraceEvent
class CTraceEvent : public CEvent
{
public:
    typedef string  TTrace;

    enum EType  {
        eType_A,
        eType_B,
        eType_C,
        eType_E,
        eType_F
    };

    CTraceEvent(CEvent::TEventClass cls, CEvent::TEventID id);
    CTraceEvent(TEventClass ecl, TEventID eid, IEventAttachment* att,
                EOwnershipPolicy policy, TEventSender* sender);

    const TTrace&   GetTrace() const;
    void    AddTrace(const TTrace& trace);

protected:
    TTrace  m_Trace;
};


///////////////////////////////////////////////////////////////////////////////
/// CTracingHandler

class CTracingHandler : public ncbi::CEventHandler
{
public:
    typedef CTraceEvent::TTrace TTrace;

    CTracingHandler(int id = -1);

    // creates a handler bound to the specific trace
    CTracingHandler(int id, TTrace& trace);

    void    ClearTrace();
    const TTrace*   GetTrace();

    void    OnEvent_A(CEvent* evt);
    void    OnEvent_B(CEvent* evt);

    void    OnMessage_A(CEvent* evt);
    void    OnMessage_B(CEvent* evt);

    void    OnCommand_1();
    void    OnCommand_2();
    void    OnCommand_Range(TCmdID cmd);

    void    OnUpdateCommand_1(ICmdUI* pCmdUI);
    void    OnUpdateCommand_2(ICmdUI* pCmdUI);
    void    OnUpdateCommand_Range(ICmdUI* pCmdUI);

protected:
    DECLARE_EVENT_MAP();

    string  x_GetMethodTrace(const string& method)  const;
    void    x_AddTrace(CEvent* evt, const string& method);
    void    x_AddTrace(const string& trace);

protected:
    int m_ID;
    TTrace* m_Trace;
};


typedef     void (CTracingHandler::*FOnEventHandler)(CEvent* evt);
typedef     void (CTracingHandler::*FOnCommandHandler)(void);
typedef     void (CTracingHandler::*FOnCommandRangeHandler)(TCmdID cmd);

///////////////////////////////////////////////////////////////////////////////
/// CTestCmdUI
class CTestCmdUI : public ICmdUI
{
public:
    CTestCmdUI(TCmdID cmd, const string& label);
    CTestCmdUI(TCmdID cmd, const string& label, bool en, bool check, bool radio);

    /// @name ICmdUI implementation
    /// @{
    virtual TCmdID  GetCommand() const;
    virtual void    Enable(bool en);
    virtual void    SetCheck(bool set);
    virtual void    SetRadio(bool set);
    virtual void    SetLabel(const string& label);
    /// @}

    /// "true" if at least one memeber was called
    bool    IsEnabled() const   {   return m_Enabled;   }
    bool    IsChecked() const   {   return m_Checked;   }
    bool    IsRadio() const     {   return m_Radio; }

    bool    IsVisited() const   {   return m_Visited;   }

protected:
    TCmdID  m_Cmd;
    string  m_Label;
    bool    m_Enabled;
    bool    m_Checked;
    bool    m_Radio;
    bool    m_Visited;
};


END_NCBI_SCOPE

#endif  // GUI_UTILS_UNIT_TEST___EVENTS_TOOLS__HPP
