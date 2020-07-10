/*  $Id: micro_introns.cpp 607016 2020-04-29 17:24:46Z ivanov $
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
* Author:
*
* File Description:
*   fasta-file generator application
*
* ===========================================================================
*/
#include <ncbi_pch.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/align_ci.hpp>
#include <algo/sequence/gene_model.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

//  ============================================================================
static void InheritPartialness(
    const CSeq_loc& src,
    CSeq_loc& dest)
//  ============================================================================
{
    if( !src.GetId() || !dest.GetId() || !src.GetId()->Equals(*dest.GetId())
            || src.GetStrand() != dest.GetStrand() ) {
        return;
    }
    
    const objects::ESeqLocExtremes ext = eExtreme_Biological;
    const bool same_start = src.GetStart(ext) == dest.GetStart(ext);
    const bool same_stop  = src.GetStop(ext)  == dest.GetStop(ext); 

    if(same_start && src.IsPartialStart(ext)) {
        dest.SetPartialStart(true, ext); 
    }

    if(same_start && src.IsTruncatedStart(ext)) {
        dest.SetTruncatedStart(true, ext);
    }

    if(same_stop && src.IsPartialStop(ext)) {
        dest.SetPartialStop(true, ext);
    }

    if(same_stop && src.IsTruncatedStop(ext)) {
        dest.SetTruncatedStop(true, ext);
    }
}


//  ============================================================================
CConstRef<CSeq_align> 
GetAlignmentForRna(
    const CMappedFeat& a_feat, 
    CScope& a_scope)
//  ============================================================================
{
    CSeq_id_Handle feat_idh = a_feat.GetProductId();
    CSeq_id_Handle canonical = sequence::GetId(feat_idh, a_scope, sequence::eGetId_Canonical);

    if (canonical) {
        feat_idh = canonical;
    }
    list<CConstRef<CSeq_align> > align_list;
    for(CAlign_CI align_it(a_scope, a_feat.GetLocation()); align_it; ++align_it) {
        const CSeq_align& cur_align = *align_it;
        CSeq_id_Handle align_product_idh = CSeq_id_Handle::GetHandle(cur_align.GetSeq_id(0));
        canonical = sequence::GetId(align_product_idh, a_scope, sequence::eGetId_Canonical);
        if (canonical) {
            align_product_idh = canonical;
        }
        if(feat_idh.MatchesTo(align_product_idh)) {
            align_list.push_back(CConstRef<CSeq_align>(&*align_it));
        }
    }
    if(align_list.size() > 1) {
        NCBI_THROW(CException, eUnknown, "Multiple alignments found.");
    }
    return (align_list.size() > 0 ? align_list.front() : CConstRef<CSeq_align>());
}


//  ============================================================================
void CFeatureGenerator::CreateMicroIntrons(
    CScope& scope,
    CBioseq_Handle bsh) 
//  ============================================================================
{
    feature::CFeatTree feat_tree;
    scope.GetEditHandle(bsh);
    //create feature tree from all features 
    {{
         SAnnotSelector sel;
         sel.IncludeFeatType(CSeqFeatData::e_Cdregion);
         sel.IncludeFeatType(CSeqFeatData::e_Rna);
         sel.SetResolveAll().SetAdaptiveDepth(true);
         CFeat_CI feat_it(bsh, sel);
         feat_tree.AddFeatures(feat_it);
     }}

    SAnnotSelector sel;
    sel.IncludeFeatType(CSeqFeatData::e_Cdregion);
    sel.SetResolveAll().SetAdaptiveDepth(true);
    for(CFeat_CI feat_it(bsh, sel); feat_it; ++feat_it) {
        CSeq_feat_Handle cur_feat = feat_it->GetSeq_feat_Handle();
        CSeq_feat_Handle mrna_feat = feat_tree.GetParent(cur_feat, CSeqFeatData::e_Rna).GetSeq_feat_Handle();
        if(!mrna_feat) {
            continue;
        }
        CConstRef<CSeq_align> align_ref = GetAlignmentForRna(mrna_feat, scope);
        if(!align_ref) {
            continue;
        }
        const CSeq_align& cur_align = *align_ref;
        CBioseq_Handle mrna_bsh = scope.GetBioseqHandle(mrna_feat.GetProductId());
        if(!mrna_bsh) {
            NCBI_THROW(CException, eUnknown, "Unable to get mRNA sequence.");
        }
        CMappedFeat prod_cd_feat;
        for(CFeat_CI prod_feat_it(mrna_bsh, sel); prod_feat_it; ++prod_feat_it) {
            if(prod_cd_feat) {
                NCBI_THROW(CException, eUnknown, "Multiple cdregion features found on mRNA.");
            }
            prod_cd_feat = *prod_feat_it;
        }
        if(!prod_cd_feat) {
            NCBI_THROW(CException, eUnknown, "Unable to find cdregion on mRNA: " + mrna_feat.GetProductId().AsString());
        }
        CRef<CSeq_loc> projected_mrna_loc = 
            CFeatureGenerator::s_ProjectRNA(
                cur_align, 
                CConstRef<CSeq_loc>(&prod_cd_feat.GetLocation()));
 
        CRef<CSeq_loc> projected_cd_loc = 
            CFeatureGenerator::s_ProjectCDS(
                cur_align, 
                prod_cd_feat.GetLocation());
             
        InheritPartialness(
            mrna_feat.GetOriginalSeq_feat()->GetLocation(),
            *projected_mrna_loc);
 
        InheritPartialness(
            cur_feat.GetOriginalSeq_feat()->GetLocation(),
            *projected_cd_loc);

        //this ensures we can edit the features
        scope.GetEditHandle(mrna_feat.GetAnnot());
        scope.GetEditHandle(cur_feat.GetAnnot());

        CRef<CSeq_feat> new_mrna_feat(new CSeq_feat);
        new_mrna_feat->Assign(*mrna_feat.GetOriginalSeq_feat());
        new_mrna_feat->SetLocation(*projected_mrna_loc);
        CSeq_feat_EditHandle mrna_eh(mrna_feat);
        mrna_eh.Replace(*new_mrna_feat);

        CRef<CSeq_feat> new_cds_feat(new CSeq_feat);
        new_cds_feat->Assign(*cur_feat.GetOriginalSeq_feat());
        new_cds_feat->SetLocation(*projected_cd_loc);
        CSeq_feat_EditHandle cds_eh(cur_feat);
        cds_eh.Replace(*new_cds_feat);
    }
}

END_NCBI_SCOPE
