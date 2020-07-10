#ifndef GUI_OBJUTILS___CMD_CREATE_ALIGN__HPP
#define GUI_OBJUTILS___CMD_CREATE_ALIGN__HPP

/*  $Id: cmd_create_align.hpp 32605 2015-03-31 18:50:15Z filippov $
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
 * Authors:  Igor Filippov based on work by Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objmgr/seq_align_handle.hpp>
#include <gui/gui_export.h>

#include <gui/utils/command_processor.hpp>

#include <objmgr/seq_entry_handle.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CCmdCreateAlign : 
    public CObject, public IEditCommand
{
public:
    CCmdCreateAlign(objects::CSeq_entry_Handle& seh, const objects::CSeq_align& align)
        : m_seh(seh), m_Align(&align)
    {
    }

    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel();
    /// @}
    
protected:
    objects::CSeq_entry_Handle m_seh;
    CConstRef<objects::CSeq_align>  m_Align;
    objects::CSeq_align_Handle  m_ah;
    objects::CSeq_annot_EditHandle m_AnnotCreated;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___CMD_CREATE_ALIGN__HPP
