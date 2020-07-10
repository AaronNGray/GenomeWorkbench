/*  $Id: recompute_intervals.cpp 38984 2017-07-14 19:48:54Z filippov $
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
 * Authors:  Igor Filippov
 */


#include <ncbi_pch.hpp>
#include <gui/packages/pkg_sequence_edit/recompute_intervals.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CRecomputeIntervals::ExtendIntervalToEnd (objects::CSeq_interval& ival, size_t len)
{
    if (ival.IsSetStrand() && ival.GetStrand() == objects::eNa_strand_minus) {
        if (ival.GetFrom() > 3) {
            ival.SetFrom(ival.GetFrom() - 3);
        } else {
            ival.SetFrom(0);
        }
    } else {
        if (ival.GetTo() < len - 4) {
            ival.SetTo(ival.GetTo() + 3);
        } else {
            ival.SetTo(len - 1);
        }
    }
}

CRef<CCmdComposite> CRecomputeIntervals::apply(CSeq_entry_Handle tse, bool update_genes)
{
    CRef<CCmdComposite> cmd;
    if (!tse)
        return cmd;
    cmd.Reset(new CCmdComposite("Recompute Intervals"));
    CScope& scope = tse.GetScope();
    bool modified = false;

    for (CFeat_CI fi(tse, CSeqFeatData::eSubtype_cdregion); fi; ++fi)
    {         
        const CSeq_feat& cds = fi->GetOriginalFeature();
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(cds);
        CBioseq_Handle prot_bsh = scope.GetBioseqHandle(cds.GetProduct());
        if (!prot_bsh)
            continue;

        CProSplign prosplign(CProSplignScoring(), false, true, false, false);
        
        CBioseq_Handle bsh = scope.GetBioseqHandle(cds.GetLocation());
            
        CRef<objects::CSeq_id> seq_id(new objects::CSeq_id());
        seq_id->Assign(*(bsh.GetSeqId()));
        CRef<objects::CSeq_loc> match_loc(new objects::CSeq_loc(*seq_id, 0, bsh.GetBioseqLength() - 1));

        CRef<objects::CSeq_align> alignment;
        try
        {
            alignment = prosplign.FindAlignment(scope, *(prot_bsh.GetSeqId()), *match_loc, CProSplignOutputOptions(CProSplignOutputOptions::ePassThrough));
        }
        catch(exception &e)
        {
            alignment.Reset();
        }

        CRef<objects::CSeq_loc> cds_loc(new objects::CSeq_loc());
        bool found_start_codon = false;
        bool found_stop_codon = false;
        if (alignment && alignment->IsSetSegs() && alignment->GetSegs().IsSpliced()) 
        {
            CRef<objects::CSeq_id> seq_id (new objects::CSeq_id());
            seq_id->Assign(*match_loc->GetId());
            ITERATE (objects::CSpliced_seg::TExons, exon_it, alignment->GetSegs().GetSpliced().GetExons()) 
            {
                CRef<objects::CSeq_loc> exon(new objects::CSeq_loc(*seq_id, (*exon_it)->GetGenomic_start(), (*exon_it)->GetGenomic_end()));                
                if ((*exon_it)->IsSetGenomic_strand()) 
                {
                    exon->SetStrand((*exon_it)->GetGenomic_strand());
                }
                cds_loc->SetMix().Set().push_back(exon);
            }
            ITERATE (objects::CSpliced_seg::TModifiers, mod_it, alignment->GetSegs().GetSpliced().GetModifiers()) 
            {
                if ((*mod_it)->IsStart_codon_found()) 
                {
                    found_start_codon = (*mod_it)->GetStart_codon_found();
                }
                if ((*mod_it)->IsStop_codon_found()) 
                {
                    found_stop_codon = (*mod_it)->GetStop_codon_found();
                }
            }        
        }

        if (!cds_loc->IsMix()) 
        {
            continue;
        } 
        else 
        {
            if (cds_loc->GetMix().Get().size() == 1) 
            {
                CRef<objects::CSeq_loc> exon = cds_loc->SetMix().Set().front();
                cds_loc->Assign(*exon);
            }
        }

        if (!found_start_codon) 
        {
            cds_loc->SetPartialStart(true, objects::eExtreme_Biological);
        }
        if (found_stop_codon) 
        {
            // extend to cover stop codon        
            size_t len = bsh.GetInst_Length();
            if (cds_loc->IsMix()) 
            {
                ExtendIntervalToEnd(cds_loc->SetMix().Set().back()->SetInt(), len);
            } 
            else 
            {
                ExtendIntervalToEnd(cds_loc->SetInt(), len);
            }        
        } 
        else 
        {
            cds_loc->SetPartialStop(true, objects::eExtreme_Biological);
        }

        if (sequence::Compare(cds.GetLocation(), *cds_loc, &scope, 0) == sequence::eSame)
            continue;

        new_feat->SetLocation(*cds_loc);
        new_feat->SetPartial(new_feat->GetLocation().IsPartialStart(eExtreme_Positional) || new_feat->GetLocation().IsPartialStop(eExtreme_Positional));
        cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fi->GetSeq_feat_Handle(), *new_feat)));
        modified = true;

        if (update_genes)
        {
            CConstRef<objects::CSeq_feat> gene = sequence::GetOverlappingGene(cds.GetLocation(), scope);
            if (gene)
            {
                CRef<objects::CSeq_feat> new_gene(new CSeq_feat);
                new_gene->Assign(*gene);
                objects::CSeq_loc::TRange range = cds_loc->GetTotalRange();
                CRef<objects::CSeq_loc> new_gene_loc(new objects::CSeq_loc);
                CRef<objects::CSeq_id> id(new objects::CSeq_id);
                id->Assign(*(cds_loc->GetId()));
                CRef<objects::CSeq_interval> new_int(new objects::CSeq_interval(*id, range.GetFrom(), range.GetTo(), cds_loc->GetStrand()));
                new_gene_loc->SetInt(*new_int);
                new_gene_loc->SetPartialStart(cds_loc->IsPartialStart(eExtreme_Biological), objects::eExtreme_Biological);
                new_gene_loc->SetPartialStop(cds_loc->IsPartialStop(eExtreme_Biological), objects::eExtreme_Biological);
                new_gene->SetLocation().Assign(*new_gene_loc);
                new_gene->SetPartial(new_gene->GetLocation().IsPartialStart(eExtreme_Positional) || new_gene->GetLocation().IsPartialStop(eExtreme_Positional));
                cmd->AddCommand(*CRef< CCmdChangeSeq_feat >(new CCmdChangeSeq_feat(scope.GetSeq_featHandle(*gene),*new_gene))); 
            }
        }
    }
    if (!modified)
        cmd.Reset();
    return cmd;
}


END_NCBI_SCOPE
