/*  $Id: cmd_insert_bioseq_set.cpp 31019 2014-08-20 13:08:23Z bollin $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Colleen Bollin and Roman Katargin
 */


#include <ncbi_pch.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/seq_annot_handle.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <gui/objutils/cmd_insert_bioseq_set.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CCmdInsertBioseqSet::CCmdInsertBioseqSet(CSeq_entry_Handle seh, const CBioseq_set::TClass new_class)
    : m_seh(seh), m_Class(new_class)
{ 
}

void CCmdInsertBioseqSet::Execute()
{
    CSeq_entry_EditHandle edit = m_seh.GetEditHandle();

    if (m_seh.IsSet()) {
        size_t num_in_set = 0;
        CConstRef<CBioseq_set> old_set = m_seh.GetSet().GetCompleteBioseq_set();
        if (!m_seh.GetSet().IsEmptySeq_set()) {
            num_in_set = old_set->GetSeq_set().size();
        }
        CRef<CSeq_entry> new_set(new CSeq_entry());
        new_set->SetSet().SetClass(m_Class);
        CSeq_entry_EditHandle new_set_edit = edit.AttachEntry(*new_set);
        new_set_edit.TakeAllAnnots(edit);
        new_set_edit.TakeAllDescr(edit);
        x_TransferSeqSet(new_set_edit, edit, num_in_set);
    } else {
        edit.ConvertSeqToSet(m_Class);
    }
        
}

void CCmdInsertBioseqSet::Unexecute()
{
    CSeq_entry_EditHandle edit = m_seh.GetEditHandle();
    CConstRef<CSeq_entry> entry = m_seh.GetSet().GetCompleteBioseq_set()->GetSeq_set().front();
    CSeq_entry_Handle e = edit.GetScope().GetSeq_entryHandle(*entry);
    CSeq_entry_EditHandle new_set_edit = e.GetEditHandle();
    edit.TakeAllAnnots(new_set_edit);
    edit.TakeAllDescr(new_set_edit);
    if (entry->IsSet()) {
        size_t num_in_set = 0;
        if (entry->GetSet().IsSetSeq_set()) {
            num_in_set = entry->GetSet().GetSeq_set().size();
        }
        x_TransferSeqSet(edit, new_set_edit, num_in_set);
        new_set_edit.Remove();    
    } else {
        edit.ConvertSetToSeq();
    }
}


string CCmdInsertBioseqSet::GetLabel()
{
    return "Insert Set";
}


void CCmdInsertBioseqSet::x_TransferSeqSet(CSeq_entry_EditHandle dst, CSeq_entry_EditHandle src, size_t num_in_set)
{
    CScope& scope = src.GetScope();
    CConstRef<CBioseq_set> old_set = src.GetSet().GetCompleteBioseq_set();
    for (size_t i = 0; i < num_in_set; i++) {
        CConstRef<CSeq_entry> entry = old_set->GetSeq_set().front();
        CSeq_entry_Handle e = scope.GetSeq_entryHandle(*entry);
        CSeq_entry_EditHandle take = e.GetEditHandle();
        dst.TakeEntry(take);
    }
}


END_NCBI_SCOPE
