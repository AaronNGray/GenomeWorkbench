#ifndef GUI_CORE___UNDO_MANAGER__HPP
#define GUI_CORE___UNDO_MANAGER__HPP

/*  $Id: undo_manager.hpp 34312 2015-12-11 18:44:57Z katargir $
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

#include <gui/gui_export.h>
#include <corelib/ncbimtx.hpp>
#include <gui/utils/command_processor.hpp>

BEGIN_NCBI_SCOPE

/////////////////////////////////////////////////////////////////////////////

class CGBDocument;

/////////////////////////////////////////////////////////////////////////////

class NCBI_GUICORE_EXPORT CUndoManager : public CObjectEx, public IUndoManager
{
public:
    CUndoManager(size_t maxBufferSize = 0, CGBDocument* document = 0)
        : m_Document(document), m_MaxBufferSize(maxBufferSize), m_LockCounter(0), m_ExclusiveEdit() {}
    virtual ~CUndoManager() {}

    virtual void Undo(wxWindow* window = 0);
    virtual void Redo(wxWindow* window = 0);
    virtual bool CanUndo();
    virtual bool CanRedo();
    virtual string GetUndoLabel();
    virtual string GetRedoLabel();

    virtual bool RequestExclusiveEdit(wxWindow* window, const string& descr);
    virtual bool ReleaseExclusiveEdit(wxWindow* window);

    // ICommandProccessor
    virtual void Execute(IEditCommand* command, wxWindow* window = 0);
    virtual bool ExecuteLock();
    virtual void ExecuteUnlock();

    void Reset();

protected:
    class CWriteLockGuard
    {
    public:
        CWriteLockGuard(CUndoManager& undoManager) : m_UndoManager(undoManager) {}
        ~CWriteLockGuard();
    private:
        CUndoManager& m_UndoManager;
    };

    typedef list< CIRef<IEditCommand> > TCommands;

    void ShowErrorMsg (IEditCommand* pCommand, const string& errMsg);
    void x_SendChangingEvent();
    void x_SendChangedEvent();

    bool x_WriteLock();
    bool x_LockDocument();

    void x_ShowExclusiveEditDlg();

    TCommands m_UndoBuffer;
    TCommands m_RedoBuffer;
    CGBDocument* m_Document;
	size_t       m_MaxBufferSize;

    CFastMutex m_RWMutex;
    int m_LockCounter;

    wxWindow* m_ExclusiveEdit;
    string    m_ExclusiveDescr;
};

END_NCBI_SCOPE

#endif  // GUI_CORE___UNDO_MANAGER__HPP
