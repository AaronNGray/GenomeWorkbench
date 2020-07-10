/*  $Id: cmd_place_protein_feats.cpp 44876 2020-04-02 20:05:56Z asztalos $
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
 * Authors:  Andrea Asztalos
 */


#include <ncbi_pch.hpp>

#include "cmd_place_protein_feats.hpp"

#include <objmgr/util/sequence.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <objtools/cleanup/cleanup.hpp>

#include <gui/objutils/cmd_promote_cds.hpp>
#include <gui/objutils/cmd_change_seq_entry.hpp>
#include <gui/objutils/cmd_composite.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static CConstRef<CSeq_feat> s_GetCdsByProduct(CScope& scope, const CSeq_loc& product);

// move protein specific features e.g., mat_peptides from the nucleotide to the protein

static bool s_MoveProteinSpecificFeats(CSeq_entry_Handle seh)
{
    bool changed = false;

    SAnnotSelector sel(CSeqFeatData::e_Prot);
    sel.IncludeFeatType(CSeqFeatData::e_Psec_str);
    for (CFeat_CI prot_it(seh, sel); prot_it; ++prot_it) {
        CBioseq_Handle parent_bsh = seh.GetScope().GetBioseqHandle(prot_it->GetLocation());
        if (parent_bsh.IsAa())
            continue;

        CConstRef<CSeq_feat> cds;
        bool matched_by_product = false;

        auto prot_fh = prot_it->GetSeq_feat_Handle();
        if (prot_fh.IsSetProduct() &&
            prot_fh.GetData().IsProt() &&
            prot_fh.GetData().GetProt().IsSetProcessed() &&
            prot_fh.GetData().GetProt().GetProcessed() == CProt_ref::eProcessed_mature) {
            cds = s_GetCdsByProduct(prot_fh.GetScope(), prot_fh.GetProduct());
            if (cds)
                matched_by_product = true;
        }

        if (!cds) {
            cds = sequence::GetOverlappingCDS(prot_fh.GetLocation(), seh.GetScope());
        }
        if (!cds || !cds->IsSetProduct()) {
            // skip feature as no appropriate protein sequence was found to move to
            continue;
        }

        CRef<CSeq_loc> new_loc = CCleanup::GetProteinLocationFromNucleotideLocation(prot_fh.GetLocation(), *cds, prot_fh.GetScope(), true);
        if (!new_loc) {
            continue;
        }

        changed = true;

        CConstRef<CSeq_feat> orig_feat = prot_it->GetSeq_feat();
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(*orig_feat);
        new_feat->ResetLocation();
        new_feat->SetLocation(*new_loc);
        if (matched_by_product) {
            new_feat->ResetProduct();
        }

        // remove the feature from the nuc bioseq
        CSeq_feat_Handle fh = seh.GetScope().GetSeq_featHandle(*orig_feat);
        CSeq_feat_EditHandle edh(fh);
        edh.Remove();

        CBioseq_Handle target_bsh = seh.GetScope().GetBioseqHandle(new_feat->GetLocation());
        CSeq_annot_Handle ftable;
        CSeq_annot_CI annot_ci(target_bsh);
        for (; annot_ci; ++annot_ci) {
            if ((*annot_ci).IsFtable()) {
                ftable = *annot_ci;
                break;
            }
        }

        CBioseq_EditHandle eh = target_bsh.GetEditHandle();

        if (!ftable) {
            CRef<CSeq_annot> new_annot(new CSeq_annot());
            ftable = eh.AttachAnnot(*new_annot);
        }
        // add feature to the protein bioseq
        CSeq_annot_EditHandle aeh(ftable);
        aeh.AddFeat(*new_feat);
    }

    return changed;
}

CConstRef<CSeq_feat> s_GetCdsByProduct(CScope& scope, const CSeq_loc& product)
{
    const bool feat_by_product = true;
    SAnnotSelector sel(CSeqFeatData::e_Cdregion, feat_by_product);
    CFeat_CI fi(scope, product, sel);
    if (fi) {
        return fi->GetOriginalSeq_feat();
    }
    return CConstRef<CSeq_feat>();
};

CIRef<IEditCommand> CCmdPlaceProteinFeats::x_CreateActionCmd()
{
    CConstRef<CSeq_entry> entry = m_seh.GetCompleteSeq_entry();
    CRef<CSeq_entry> copy(new CSeq_entry());
    copy->Assign(*entry);
    CRef<CObjectManager> objmgr = CObjectManager::GetInstance();
    CScope scope2(*objmgr);
    scope2.AddDefaults(); 
    CSeq_entry_Handle new_seh = scope2.AddTopLevelSeqEntry(*copy);
    
    if (!s_MoveProteinSpecificFeats(new_seh))
        return CIRef<IEditCommand>();

    return CIRef<IEditCommand>(new CCmdChangeSeqEntry(m_seh, copy));
}

string CCmdPlaceProteinFeats::GetLabel()
{
    return "Place protein specific features";
}


CIRef<IEditCommand> CCmdMoveCdsToSetLevel::x_CreateActionCmd()
{
    CRef<CCmdComposite> composite(new CCmdComposite("Move coding regions to the nuc-prot set level"));
    for (CFeat_CI feat_it(m_seh, SAnnotSelector(CSeqFeatData::eSubtype_cdregion)); feat_it; ++feat_it) {
        if ((feat_it->IsSetProduct() || sequence::GetLength(feat_it->GetLocation(), &m_seh.GetScope()) >= 6) &&
            (!feat_it->IsSetPseudo() || !feat_it->GetPseudo())) {
            CSeq_feat_Handle fh = feat_it->GetSeq_feat_Handle();
            composite->AddCommand(*CRef<CCmdPromoteCDS>(new CCmdPromoteCDS(fh)));
        }
    }
    return (composite->IsEmpty()) ? CIRef<IEditCommand>() : CIRef<IEditCommand>(composite);
}

string CCmdMoveCdsToSetLevel::GetLabel()
{
    return "Move coding regions from sequence to set level";
}

END_NCBI_SCOPE
