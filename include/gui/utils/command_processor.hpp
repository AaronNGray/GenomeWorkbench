#ifndef GUI_UTILS___COMMAND_PROCESSOR__HPP
#define GUI_UTILS___COMMAND_PROCESSOR__HPP

/*  $Id: command_processor.hpp 34312 2015-12-11 18:44:57Z katargir $
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

#include <corelib/ncbistd.hpp>

class wxWindow;

BEGIN_NCBI_SCOPE


/// Interface (functor) for object editing
///
/// All functions except Execute are engaged from UI elements (main menu)
/// Implementation of an IEditObject works as a class factory for IEditCommand
///
/// @sa IEditObject, ICommandProccessor
class IEditCommand
{
public:
    virtual ~IEditCommand() {}

    /// Do the editing action
    virtual void Execute() = 0;
    /// Undo (opposite to Execute())
    virtual void Unexecute() = 0;
    virtual string GetLabel() = 0;

    virtual bool CanMerge(IEditCommand*) { return false; }
    virtual void Merge(IEditCommand*) {}
};

/// Undo/Redo interface for editing operations
///
/// We associate with every GBench project/document Undo manager object.
/// Implementation of this service holds the list of commands (see IEditCommand).
/// All actual data changing work is supposed to be delegated to IEditCommand::Execute()
///
/// @sa IEditObject, IEditCommand

class ICommandProccessor
{
public:
    virtual ~ICommandProccessor() {}

    // his member should be called on the main thread only
    virtual void Execute(IEditCommand* command, wxWindow* window = 0) = 0;

    //
    // these functions can be called on the main thread or worker threads
    // Successful call to ExecuteLock() will prevent Execute() 
    // to finish or succeed until ExecuteUnlock() is called
    // Successful call to ExecuteLock() must be followed by ExecuteUnlock() in the same code
    //

    virtual bool ExecuteLock() = 0;
    virtual void ExecuteUnlock() = 0;
};

class IUndoManager : public ICommandProccessor
{
public:
    virtual void Undo(wxWindow* window = 0) = 0;
    virtual void Redo(wxWindow* window = 0) = 0;
    virtual bool CanUndo() = 0;
    virtual bool CanRedo() = 0;
    virtual string GetUndoLabel() = 0;
    virtual string GetRedoLabel() = 0;

    virtual bool RequestExclusiveEdit(wxWindow* window, const string& descr) = 0;
    virtual bool ReleaseExclusiveEdit(wxWindow* window) = 0;
};

END_NCBI_SCOPE

#endif  // GUI_UTILS___COMMAND_PROCESSOR__HPP
