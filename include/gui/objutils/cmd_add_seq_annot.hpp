#ifndef GUI_OBJUTILS___CMD_ADD_SEQ_ANNOT__HPP
#define GUI_OBJUTILS___CMD_ADD_SEQ_ANNOT__HPP

/*  $Id: cmd_add_seq_annot.hpp 32125 2015-01-05 20:16:18Z asztalos $
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
 * Authors:  Andrea Asztalos, based on a file by Igor Filippov
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <gui/utils/command_processor.hpp>

#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/seq_annot_handle.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT  CCmdCreateSeq_annot : public CObject, public IEditCommand
{
public:
    CCmdCreateSeq_annot(objects::CSeq_entry_Handle& seh, objects::CSeq_annot& annot)
      : m_ParentHandle(seh), m_Annot(&annot) {}

    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel();
    /// @}

protected:
    objects::CSeq_entry_Handle m_ParentHandle;
    CRef<objects::CSeq_annot> m_Annot;
    objects::CSeq_annot_EditHandle m_AEH;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___ADD_DEL_SEQ_ANNOT__HPP
