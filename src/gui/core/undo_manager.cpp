/*  $Id: undo_manager.cpp 39241 2017-08-28 15:44:36Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/core/undo_manager.hpp>
#include <gui/core/document.hpp>

#include <objects/gbproj/ProjectDescr.hpp>
#include <objects/general/Date.hpp>

#include <wx/utils.h>

#include "project_locked_dlg.hpp"
#include "exclusive_edit_dlg.hpp"

BEGIN_NCBI_SCOPE

/////////////////////////////////////////////////////////////////////////////


void CUndoManager::Execute(IEditCommand* pCommand, wxWindow* window)
{
    if (m_ExclusiveEdit && m_ExclusiveEdit != window) {
        x_ShowExclusiveEditDlg();
        return;
    }

    CIRef<IEditCommand> command(pCommand);
    string errMsg;
    bool exception = false;

    try {
        {{
            CWriteLockGuard guard(*this);
            if (x_LockDocument()) {
                wxBusyCursor wait;
                x_SendChangingEvent();
                command->Execute();
            }
            else
                return;
        }}
        x_SendChangedEvent();
        m_RedoBuffer.clear();

        if (m_UndoBuffer.size() > 0 && m_UndoBuffer.front()->CanMerge(command)) {
            m_UndoBuffer.front()->Merge(command);
        } else {
            m_UndoBuffer.push_front(CIRef<IEditCommand>(command));
			if (m_MaxBufferSize != 0 && m_UndoBuffer.size() > m_MaxBufferSize) {
				m_UndoBuffer.pop_back();
			}
        }
    }
    catch(const CException& e) {
        exception = true;
        errMsg = e.GetMsg();
    }

    if (exception) {
        ShowErrorMsg (command, errMsg);
    }
}

void CUndoManager::Undo(wxWindow* window)
{
    if (m_ExclusiveEdit && m_ExclusiveEdit != window) {
        x_ShowExclusiveEditDlg();
        return;
    }

    if (m_UndoBuffer.size() == 0) {
        _ASSERT(false);
        return;
    }

    CIRef<IEditCommand> command = m_UndoBuffer.front();
    string errMsg;
    bool exception = false;

    try {
        {{
            CWriteLockGuard guard(*this);
            if (x_LockDocument()) {
                wxBusyCursor wait;
                x_SendChangingEvent();
                command->Unexecute();
            }
            else
                return;
        }}
        x_SendChangedEvent();
        m_UndoBuffer.pop_front();
        m_RedoBuffer.push_front(command);
    }
    catch (CException& e) {
        exception = true;
        errMsg = e.GetMsg();
        LOG_POST(Error << "failed to Undo: " << e.GetMsg());
    }

    if (exception) {
        ShowErrorMsg (command, errMsg);
    }
}

void CUndoManager::Redo(wxWindow* window)
{
    if (m_ExclusiveEdit && m_ExclusiveEdit != window) {
        x_ShowExclusiveEditDlg();
        return;
    }

    if (m_RedoBuffer.size() == 0) {
        _ASSERT(false);
        return;
    }

    CIRef<IEditCommand> command = m_RedoBuffer.front();
    string errMsg;
    bool exception = false;

    try {
        {{
            CWriteLockGuard guard(*this);
            if (x_LockDocument()) {
                wxBusyCursor wait;
                x_SendChangingEvent();
                command->Execute();
            }
            else
                return;
        }}
        x_SendChangedEvent();
        m_RedoBuffer.pop_front();
        m_UndoBuffer.push_front(command);
    }
    catch (CException& e) {
        exception = true;
        errMsg = e.GetMsg();
        LOG_POST(Error << "failed to Redo: " << e.GetMsg());
    }

    if (exception) {
        ShowErrorMsg (command, errMsg);
    }
}

bool CUndoManager::CanUndo()
{
    return (m_UndoBuffer.size() > 0);
}

bool CUndoManager::CanRedo()
{
    return (m_RedoBuffer.size() > 0);
}

string CUndoManager::GetUndoLabel()
{
    _ASSERT(m_UndoBuffer.size() > 0);
    return (m_UndoBuffer.size() > 0) ? m_UndoBuffer.front()->GetLabel() : "";
}

string CUndoManager::GetRedoLabel()
{
    _ASSERT(m_RedoBuffer.size() > 0);
    return (m_RedoBuffer.size() > 0) ? m_RedoBuffer.front()->GetLabel() : "";
}

void CUndoManager::Reset()
{
    m_RedoBuffer.clear();
    m_UndoBuffer.clear();
}

void CUndoManager::ShowErrorMsg (IEditCommand* pCommand, const string& errMsg)
{
    string cmdName = pCommand->GetLabel();

    if (cmdName.empty())
        cmdName = typeid(*pCommand).name();

    if (cmdName.empty())
        cmdName = "Unknown command";
    cmdName += ": ";

    string msg = errMsg.empty() ? "Unknown error" : errMsg;
    if (msg[msg.length() - 1] != '.')
            msg += ".";

    LOG_POST(Error << cmdName + msg);
}

void CUndoManager::x_SendChangingEvent()
{
    if (m_Document)
        m_Document->x_FireProjectChanged(CProjectViewEvent::eDataChanging);
}

void CUndoManager::x_SendChangedEvent()
{
    if (m_Document) {
        m_Document->SetDataModified(true);
        m_Document->x_FireProjectChanged(CProjectViewEvent::eData);
    }
}

bool CUndoManager::ExecuteLock()
{
    CFastMutexGuard lock(m_RWMutex);
    if (m_LockCounter >= 0) {
        ++m_LockCounter;
        return true;
    }
    return false;
}

void CUndoManager::ExecuteUnlock()
{
    CFastMutexGuard lock(m_RWMutex);
    if (m_LockCounter > 0) {
        --m_LockCounter;
    } else {
        _ASSERT(false);
    }
}

bool CUndoManager::x_WriteLock()
{
    CFastMutexGuard lock(m_RWMutex);
    if (m_LockCounter == 0) {
        --m_LockCounter;
        return true;
    }
    return false;
}

CUndoManager::CWriteLockGuard::~CWriteLockGuard()
{
    CFastMutexGuard lock(m_UndoManager.m_RWMutex);
    if (m_UndoManager.m_LockCounter == -1) {
        ++m_UndoManager.m_LockCounter;
    }
}

bool CUndoManager::x_LockDocument()
{
    if (x_WriteLock()) return true;

    for (;;) {
        ::wxMilliSleep(500);
        if (x_WriteLock()) return true;

        CProjectLockedDlg dlg(NULL);
        if (dlg.ShowModal() == wxID_CANCEL)
            return false;

        if (x_WriteLock()) return true;
    }
}

bool CUndoManager::RequestExclusiveEdit(wxWindow* window, const string& descr)
{
    if (m_ExclusiveEdit) {
        if (window == m_ExclusiveEdit)
            return true;
        x_ShowExclusiveEditDlg();
        return false;
    }
    m_ExclusiveEdit = window;
    m_ExclusiveDescr = descr;
    return true;
}

bool CUndoManager::ReleaseExclusiveEdit(wxWindow* window)
{
    if (m_ExclusiveEdit && m_ExclusiveEdit == window) {
        m_ExclusiveEdit = 0;
        m_ExclusiveDescr.clear();
        return true;
    }

    return false;
}

void CUndoManager::x_ShowExclusiveEditDlg()
{
    CExclusiveEditDlg dlg;
    dlg.InitData(m_ExclusiveEdit, m_ExclusiveDescr);
    dlg.Create(NULL);
    dlg.ShowModal();
}

END_NCBI_SCOPE
