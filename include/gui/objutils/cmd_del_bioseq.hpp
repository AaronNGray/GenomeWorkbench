#ifndef GUI_OBJUTILS___CMD_DEL_BIOSEQ__HPP
#define GUI_OBJUTILS___CMD_DEL_BIOSEQ__HPP

/*  $Id: cmd_del_bioseq.hpp 33186 2015-06-11 12:19:16Z filippov $
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
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/bioseq_set_handle.hpp>

BEGIN_NCBI_SCOPE
BEGIN_objects_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CCmdDelBioseqInst :
    public CObject, public IEditCommand
{
public:
    CCmdDelBioseqInst(objects::CBioseq_Handle& bsh);

    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel();
    /// @}

protected:
    CConstRef<objects::CBioseq> m_Inst;
    objects::CBioseq_Handle m_BSH;
    objects::CSeq_entry_Handle m_Set_SEH;
    int m_index;

    // Keep track of setlevel annots/descrs that will be taken and re-attached
    // to the seq during the renormalization of a nuc-prot set to a seq.
    // Then during possible UNDO action, you can use these to take them back
    // from the seq and re-attach them to the nuc-prot set.
    vector<CSeq_annot_Handle>      m_SetLevelAnnots;
    vector< CConstRef<CSeqdesc> >  m_SetLevelDescrs;
};


// Simply deletes the bioseq, nor renormalization of nuc-prot sets
class NCBI_GUIOBJUTILS_EXPORT CCmdDelBioseq :
    public CObject, public IEditCommand
{
public:
    CCmdDelBioseq(objects::CBioseq_Handle& bsh);

    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel();
    /// @}

protected:
    CConstRef<objects::CBioseq> m_Inst;
    objects::CBioseq_Handle m_BSH;
    objects::CSeq_entry_Handle m_Set_SEH;
    int m_index;  
};

END_objects_SCOPE
END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___CMD_DEL_BIOSEQ__HPP
