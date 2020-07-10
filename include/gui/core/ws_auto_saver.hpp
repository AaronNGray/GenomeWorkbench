#ifndef GUI_CORE___WS_AUTO_SAVER__HPP
#define GUI_CORE___WS_AUTO_SAVER__HPP

/*  $Id: ws_auto_saver.hpp 40006 2017-12-08 19:00:22Z katargir $
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


#include <corelib/ncbiobj.hpp>
#include <corelib/ncbistd.hpp>
#include <corelib/ncbitime.hpp>

#include <gui/gui_export.h>

#include <gui/utils/event_handler.hpp>
#include <gui/widgets/wx/async_call.hpp>

#include <wx/timer.h>

BEGIN_NCBI_SCOPE

class IServiceLocator;

///////////////////////////////////////////////////////////////////////////////
/// CWorkspaceAutoSaver
///////////////////////////////////////////////////////////////////////////////

class CAutoSaverEvent : public CEvent
{
public:
    enum EEventId {
        eSave
    };

    CAutoSaverEvent() : CEvent(eSave) {}
};

class NCBI_GUICORE_EXPORT CWorkspaceAutoSaver : public wxTimer, public CEventHandler
{
public:
    CWorkspaceAutoSaver(IServiceLocator& serviceLocator, size_t interval) : m_ServiceLocator(serviceLocator), m_Interval(interval) {}

    enum ERestoreMode {
        eAskUser,
        eRestore,
        eIgnore
    };

    size_t GetInterval() const { return m_Interval; }
    void SetInterval(size_t interval);

    void PostSave();

    // returns true if crash state is restored
    bool CheckCrash(ERestoreMode mode);
    static bool CleanUp();

private:
    typedef void (CWorkspaceAutoSaver::*TExecFunc)(ICanceled&);

    virtual void Notify();
    void    x_OnSave(CEvent*);

    void x_SaveWorkspace(ICanceled& canceled);
    void x_RestoreWorkspace(ICanceled& canceled);

    IServiceLocator& m_ServiceLocator;
    size_t m_Interval;

    string m_Err;

    DECLARE_EVENT_MAP();
};

END_NCBI_SCOPE;

#endif  // GUI_CORE___WS_AUTO_SAVER__HPP
