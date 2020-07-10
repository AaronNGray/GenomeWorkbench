#ifndef GUI_OBJUTILS___CMD_DEL_SEQ_ALIGN__HPP
#define GUI_OBJUTILS___CMD_DEL_SEQ_ALIGN__HPP

/*  $Id: cmd_del_seq_align.hpp 37191 2016-12-13 21:25:31Z asztalos $
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
 * Authors:  Roman Katargin, Igor Filippov
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <gui/utils/command_processor.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/seq_annot_handle.hpp>
#include <objmgr/seq_align_handle.hpp>

BEGIN_NCBI_SCOPE
BEGIN_objects_SCOPE

class NCBI_GUIOBJUTILS_EXPORT  CCmdDelSeq_align : public CObject, public IEditCommand
{
public:
    CCmdDelSeq_align(const CSeq_align_Handle& fh) : m_Orig_align(fh) {}

    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel();
    /// @}

protected:
    CSeq_align_Handle     m_Orig_align;
    CConstRef<CSeq_align> m_Saved_align;
    objects::CSeq_annot_EditHandle m_AnnotHandle;
    objects::CSeq_entry_EditHandle m_ParentHandle;
};

END_objects_SCOPE
END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___CMD_DEL_SEQ_ALIGN__HPP
