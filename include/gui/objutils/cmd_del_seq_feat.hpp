#ifndef GUI_OBJUTILS___CMD_DEL_SEQ_FEAT__HPP
#define GUI_OBJUTILS___CMD_DEL_SEQ_FEAT__HPP

/*  $Id: cmd_del_seq_feat.hpp 31553 2014-10-22 16:28:19Z katargir $
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
#include <gui/gui_export.h>

#include <gui/utils/command_processor.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/seq_annot_handle.hpp>
#include <objmgr/seq_feat_handle.hpp>
#include <objmgr/scope.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CCmdDelSeq_feat : 
    public CObject, public IEditCommand
{
public:
    CCmdDelSeq_feat(const objects::CSeq_feat_Handle& fh) : m_Orig_feat(fh) {}

    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel();
    /// @}

protected:
    objects::CSeq_feat_Handle     m_Orig_feat;
    CRef<objects::CSeq_feat> m_Saved_feat;
    objects::CSeq_annot_EditHandle m_aeh;
    objects::CSeq_entry_EditHandle m_Parent_handle;
    objects::CSeq_entry_EditHandle m_GrandParent_handle;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___CMD_DEL_SEQ_FEAT__HPP
