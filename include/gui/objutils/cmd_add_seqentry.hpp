#ifndef GUI_OBJUTILS___CMD_ADD_SEQENTRY__HPP
#define GUI_OBJUTILS___CMD_ADD_SEQENTRY__HPP

/*  $Id: cmd_add_seqentry.hpp 43474 2019-07-11 19:20:45Z filippov $
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
 * Authors:  Colleen Bollin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <gui/utils/command_processor.hpp>

#include <objects/seqset/Seq_entry.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <objects/seq/MolInfo.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

class CSeq_entry;

class NCBI_GUIOBJUTILS_EXPORT CCmdAddSeqEntry :
    public CObject, public IEditCommand
{
public:
    CCmdAddSeqEntry(CSeq_entry* seqentry, CSeq_entry_Handle seh);

    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel();
    /// @}

protected:
    CSeq_entry_Handle m_SEH;
    CRef<CSeq_entry> m_Add;
    int m_index;
};

END_SCOPE(objects)

NCBI_GUIOBJUTILS_EXPORT bool SetMolinfoCompleteness (objects::CMolInfo& mi, bool partial5, bool partial3);
NCBI_GUIOBJUTILS_EXPORT void SetMolinfoForProtein (CRef<objects::CSeq_entry> protein, bool partial5, bool partial3);
NCBI_GUIOBJUTILS_EXPORT CRef<objects::CSeq_feat> AddEmptyProteinFeatureToProtein (CRef<objects::CSeq_entry> protein, bool partial5, bool partial3);
NCBI_GUIOBJUTILS_EXPORT CRef<objects::CSeq_entry> CreateTranslatedProteinSequence (CRef<objects::CSeq_feat> cds, objects::CBioseq_Handle nuc_h, bool create_general_only, int *offset = nullptr);
NCBI_GUIOBJUTILS_EXPORT CRef<objects::CSeq_feat> AddProteinFeatureToProtein (CRef<objects::CSeq_entry> protein, bool partial5, bool partial3);



END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___CMD_ADD_SEQENTRY__HPP
