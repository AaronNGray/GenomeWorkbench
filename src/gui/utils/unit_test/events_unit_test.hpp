#ifndef GUI_UTILS_UNIT_TEST___EVENTS_UNIT_TEST__HPP
#define GUI_UTILS_UNIT_TEST___EVENTS_UNIT_TEST__HPP

/*  $Id: events_unit_test.hpp 14565 2007-05-18 12:32:01Z dicuccio $
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


BEGIN_NCBI_SCOPE


////////////////////////////////////////////////////////////////////////////////
/// CTracingHandler_1
class CTracingHandler_1 : public CTracingHandler
{
public:
    CTracingHandler_1(int id = -1);

    // empty map - no handlers
protected:
    DECLARE_EVENT_MAP();
};


////////////////////////////////////////////////////////////////////////////////
/// CTracingHandler_2
class CTracingHandler_2 : public CTracingHandler_1
{
public:
    CTracingHandler_2(int id = -1);

    void    OnEvent_A_Derived(CEvent* evt); // overriding base handler
    void    OnEvent_C_Derived(CEvent* evt); // new handler

protected:
    DECLARE_EVENT_MAP();
};


////////////////////////////////////////////////////////////////////////////////
/// CTestEventAttachment
/// CTestEventAttachment is used for testing attachment management policies
/// (how attachments are deleted)
class CTestEventAttachment : public IEventAttachment
{
public:
    CTestEventAttachment();
    virtual ~CTestEventAttachment();

    static int  Count();
protected:
    static int sm_Count;
};


////////////////////////////////////////////////////////////////////////////////
///
class CTestCommandHandler : public CTracingHandler
{
public:
    CTestCommandHandler(int id, TTrace& trace);

    void    OnCommand_5(); /// handler for eCmd_5 that does not have update handler

    void    OnUpdateDisable(ICmdUI* pCmdUI); /// disables pCmdUI
    void    OnUpdateCheck(ICmdUI* pCmdUI); /// enables and checks pCmdUI

protected:
    DECLARE_EVENT_MAP();
};


END_NCBI_SCOPE

#endif  // GUI_UTILS_UNIT_TEST___EVENTS_UNIT_TEST__HPP
