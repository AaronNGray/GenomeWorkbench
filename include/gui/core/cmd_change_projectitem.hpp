#ifndef GUI_CORE___CMD_CHANGE_PROJECTITEM__HPP
#define GUI_CORE___CMD_CHANGE_PROJECTITEM__HPP

/*  $Id: cmd_change_projectitem.hpp 42631 2019-03-27 16:31:06Z asztalos $
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
 * Authors: Andrea Asztalos, based on a file by Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <objects/gbproj/ProjectItem.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objmgr/seq_entry_handle.hpp>

#include <gui/gui_export.h>
#include <gui/framework/workbench.hpp>
#include <gui/core/document.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUICORE_EXPORT CCmdChangeProjectItem : public CObject, public IEditCommand
{
public:
    
    CCmdChangeProjectItem(
        objects::CProjectItem& old_item, 
        CGBDocument* doc, 
        objects::CProjectItem& new_item, 
        IWorkbench* workbench)
        : m_OldItem(&old_item), m_NewItem(&new_item), m_Doc(doc), m_Workbench(workbench)
    {
    }
    
    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel();
    /// @}

protected:
    CRef<objects::CProjectItem> m_OldItem;
    CRef<objects::CProjectItem> m_NewItem;
    CGBDocument* m_Doc;
    IWorkbench* m_Workbench;
};

END_NCBI_SCOPE

#endif  // GUI_CORE___CMD_CHANGE_PROJECTITEM__HPP
