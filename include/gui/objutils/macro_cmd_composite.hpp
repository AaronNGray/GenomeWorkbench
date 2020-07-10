#ifndef GUI_OBJUTILS___MACRO_CMD_COMPOSITE__HPP
#define GUI_OBJUTILS___MACRO_CMD_COMPOSITE__HPP

/*  $Id: macro_cmd_composite.hpp 34809 2016-02-16 16:09:37Z asztalos $
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
 * Authors: Anatoly Osipov
 *
 * File Description: Command for combining functions execution for topmost iterator.
 *                   Keypoint is that each command executed outside for the first time
 *                   because internal command data may depend on the results of previous
 *                   command execution.
 *
 */

#include <gui/objutils/cmd_composite.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

/// implements special composite command, which does not call to its internal commands when run 
/// the very first time. 
///
/// This command calls to its internal commands when the command is undone and redone. 
/// This allows doing modifications step by step while executing macro and
/// make undo and redo as a single "atomic" action.
class NCBI_GUIOBJUTILS_EXPORT CMacroCmdComposite : 
    public CCmdComposite
{
public:
    CMacroCmdComposite(const string& label) : CCmdComposite(label), m_NeverExecuted(true) {}

    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    /// @}
protected:
    bool m_NeverExecuted;   
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_CMD_COMPOSITE__HPP
