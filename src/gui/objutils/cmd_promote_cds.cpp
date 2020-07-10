/*  $Id: cmd_promote_cds.cpp 45082 2020-05-26 20:40:33Z asztalos $
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
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <gui/objutils/cmd_promote_cds.hpp>
#include <gui/objutils/utils.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


void CCmdPromoteCDS::Execute()
{
    // only move coding regions to nuc-prot set
    if (!m_OrigFeat.IsSetData() || !m_OrigFeat.GetData().IsCdregion()) {
        return;
    }
    // don't move if pseudo
    if (m_OrigFeat.IsSetPseudo() && m_OrigFeat.GetPseudo()) {
        return;
    }

    CBioseq_Handle nuc_bsh = GetBioseqForSeqFeat(*m_OrigFeat.GetOriginalSeq_feat(), m_OrigFeat.GetScope());

    // This is necessary, to make sure that we are in "editing mode"
    const CSeq_annot_Handle& annot_handle = m_OrigFeat.GetAnnot();
    CSeq_entry_EditHandle eh = annot_handle.GetParentEntry().GetEditHandle();

    CSeq_feat_EditHandle feh(m_OrigFeat);
    CSeq_entry_Handle parent_entry = feh.GetAnnot().GetParentEntry();

    if (parent_entry.IsSet()
        && parent_entry.GetSet().IsSetClass()
        && parent_entry.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot) {
        // already on nuc-prot set, leave it alone
    }
    else {
        CBioseq_set_Handle nuc_parent = parent_entry.GetParentBioseq_set();
        if (nuc_parent && nuc_parent.IsSetClass() && nuc_parent.GetClass() == CBioseq_set::eClass_nuc_prot) {
            CSeq_annot_Handle ftable;
            CSeq_entry_Handle parent_seh = nuc_parent.GetParentEntry();
            CSeq_annot_CI annot_ci(parent_seh, CSeq_annot_CI::eSearch_entry);
            for (; annot_ci; ++annot_ci) {
                if ((*annot_ci).IsFtable()) {
                    ftable = *annot_ci;
                    break;
                }
            }

            if (!ftable) {
                CRef<CSeq_annot> new_annot(new CSeq_annot());
                new_annot->SetData().SetFtable();
                CSeq_entry_EditHandle h = parent_seh.GetEditHandle();
                ftable = h.AttachAnnot(*new_annot);
            }

            CSeq_annot_EditHandle old_annot = annot_handle.GetEditHandle();
            CSeq_annot_EditHandle new_annot = ftable.GetEditHandle();
            m_NewFeh = new_annot.TakeFeat(feh);
            // as a result, feh will be in 'removed' state
            const auto& feat_list = old_annot.GetSeq_annotCore()->GetData().GetFtable();
            if (feat_list.empty()) {
                old_annot.Remove();
            }
        }
    }
}

void CCmdPromoteCDS::Unexecute()
{
    CSeq_entry_Handle parent_entry = m_NewFeh.GetAnnot().GetParentEntry();

    if (!parent_entry.IsSet() || !parent_entry.GetSet().IsSetClass() ||
        parent_entry.GetSet().GetClass() != CBioseq_set::eClass_nuc_prot) {
        // no change, not on nuc-prot set
        return;
    }

    CBioseq_CI bi(parent_entry, CSeq_inst::eMol_na);
    if (!bi) {
        // no nucleotide sequence to move to
        return;
    }


    // This is necessary, to make sure that we are in "editing mode"
    const CSeq_annot_Handle& annot_handle = m_NewFeh.GetAnnot();
    CSeq_entry_EditHandle eh = annot_handle.GetParentEntry().GetEditHandle();

    CSeq_annot_Handle ftable;
    CSeq_entry_Handle nuc_seh = bi->GetSeq_entry_Handle();
    CSeq_annot_CI annot_ci(nuc_seh, CSeq_annot_CI::eSearch_entry);
    for (; annot_ci; ++annot_ci) {
        if ((*annot_ci).IsFtable()) {
            ftable = *annot_ci;
            break;
        }
    }

    if (!ftable) {
        CRef<CSeq_annot> new_annot(new CSeq_annot());
        new_annot->SetData().SetFtable();
        CSeq_entry_EditHandle eh = nuc_seh.GetEditHandle();
        ftable = eh.AttachAnnot(*new_annot);
    }

    CSeq_annot_EditHandle old_annot = annot_handle.GetEditHandle();
    CSeq_annot_EditHandle new_annot = ftable.GetEditHandle();

    CConstRef<CSeq_feat> feat_obj = m_NewFeh.GetSeq_feat();
    m_NewFeh.Remove(); // remove it from new place
    CSeq_feat_EditHandle feh(m_OrigFeat);
    feh.Replace(*feat_obj); // reattach it to the old feature handle

    const auto &feat_list = old_annot.GetSeq_annotCore()->GetData().GetFtable();
    if (feat_list.empty()) {
        old_annot.Remove();
    }
}

string CCmdPromoteCDS::GetLabel()
{
    return "Promote CDS";
}

END_NCBI_SCOPE
