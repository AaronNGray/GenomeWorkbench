/*  $Id: adjust_consensus_splicesite.cpp 44877 2020-04-02 20:22:08Z asztalos $
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
#include <objects/seqfeat/Org_ref.hpp>

#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objtools/validator/utilities.hpp>

#include <objtools/edit/loc_edit.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/adjust_consensus_splicesite.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


//////////////////////////////////////////////////////////////////
/// AdjustForConsensusSpliceSite
/// Adjust internal intervals of a CDS (and its associated mRNA feature) to
/// position the interval endpoints at consensus splice sites (GT-AG), as long as the 
/// repositioning does not alter the contents of the translated protein.
/// @param cds        Coding region feature 
/// @param scope      The scope in which adjustments are to be made (if necessary)
///
CRef<CCmdComposite> CAdjustForConsensusSpliceSite::GetCommand(const CSeq_feat& cds)
{
    _ASSERT(m_Scope);
    CRef<CSeq_feat> new_cds(new CSeq_feat);
    new_cds->Assign(cds);
    bool changed = AdjustCDS(*new_cds); // update the location

    if (changed) {
        CRef<CCmdComposite> command(new CCmdComposite("Adjust CDS for consensus splice sites"));

        CSeq_feat_Handle fh = m_Scope->GetSeq_featHandle(cds);
        CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_cds));
        command->AddCommand(*chgFeat);

        CRef<CCmdComposite> update_cmd = AdjustmRNAandExonFeatures(*new_cds, cds);
        if (update_cmd) {
            command->AddCommand(*update_cmd);
        }
        return command;
    }

    return CRef<CCmdComposite>(NULL);
}

namespace {
    string GetProteinSeq(const CSeq_feat& cds, CScope& scope)
    {
        string new_prot_seq;
        CSeqTranslator::Translate(cds, scope, new_prot_seq);
        if (*(new_prot_seq.end() - 1) == '*')
            new_prot_seq.erase(new_prot_seq.end() - 1);
        return new_prot_seq;
    }
}

CRef<CCmdComposite> CAdjustForConsensusSpliceSite::GetCommandToAdjustCDSEnds(const CSeq_feat& cds)
{
    _ASSERT(m_Scope);
    CRef<CSeq_feat> new_cds(new CSeq_feat);
    new_cds->Assign(cds);
    bool changed = AdjustCDSEnds(*new_cds); // update the location

    if (changed) {
        CRef<CCmdComposite> command(new CCmdComposite("Adjust CDS ends for consensus splice sites"));

        CSeq_feat_Handle fh = m_Scope->GetSeq_featHandle(cds);
        CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_cds));
        command->AddCommand(*chgFeat);

        if (m_ProtChanged) {
            CBioseq_Handle prot = m_Scope->GetBioseqHandle(cds.GetProduct());
            CRef<CSeq_inst> new_inst(new CSeq_inst);
            new_inst->Assign(prot.GetInst());
            if (new_inst->GetMol() == CSeq_inst::eMol_aa && new_inst->GetRepr() == CSeq_inst::eRepr_raw) {

                string new_protein = GetProteinSeq(*new_cds, *m_Scope);
                new_inst->SetSeq_data().SetNcbieaa().Set(new_protein);
                new_inst->SetLength(TSeqPos(new_protein.length()));
                CIRef<IEditCommand> chgProt(new CCmdChangeBioseqInst(prot, *new_inst));
                command->AddCommand(*chgProt);

                for (CFeat_CI prot_it(prot, SAnnotSelector(CSeqFeatData::e_Prot)); prot_it; ++prot_it) {
                    if (!prot_it->GetData().GetProt().IsSetProcessed()) {
                        CRef<CSeq_feat> new_prot(new CSeq_feat);
                        new_prot->Assign(prot_it->GetOriginalFeature());
                        new_prot->SetLocation().SetInt().SetTo(TSeqPos(new_protein.length() - 1));
                        CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(prot_it->GetSeq_feat_Handle(), *new_prot));
                        command->AddCommand(*chgFeat);
                    }
                       
                }
            }
        }

        CRef<CCmdComposite> update_cmd = AdjustmRNAandExonEnds(*new_cds, cds);
        if (update_cmd) {
            command->AddCommand(*update_cmd);
        }
        return command;
    }

    return CRef<CCmdComposite>(NULL);
}

bool CAdjustForConsensusSpliceSite::AdjustCDS(CSeq_feat& cds)
{
    _ASSERT(m_Scope);
    if (!cds.GetData().IsCdregion() || !cds.IsSetLocation() || !cds.IsSetProduct()) {
        return false;
    }
    if ((!cds.GetLocation().IsMix() && !cds.GetLocation().IsPacked_int()) ||
        cds.GetLocation().GetId() == nullptr) {
        return false;
    }

    // not going to handle mixed-strand exons
    if (cds.GetLocation().GetStrand() == eNa_strand_other) {
        return false;
    }

    CBioseq_Handle product = m_Scope->GetBioseqHandle(cds.GetProduct());
    if (!product || !product.IsProtein()) {
        return false;
    }

    // obtaining the original protein sequence
    CSeqVector prot_vec = product.GetSeqVector(CBioseq_Handle::eCoding_Ncbi);
    prot_vec.SetCoding(CSeq_data::e_Ncbieaa);
    string orig_prot_seq;
    prot_vec.GetSeqData(0, prot_vec.size(), orig_prot_seq);
    if (NStr::IsBlank(orig_prot_seq)) {
        return false;
    }

    // assumed that the coding region is on one sequence
    CBioseq_Handle bsh = m_Scope->GetBioseqHandle(cds.GetLocation());
    if (!bsh) {
        return false;
    }

    x_InitRanges(cds);
    m_Strand = cds.GetLocation().GetStrand();
    TSeqPos length = bsh.GetBioseqLength() - 1;
    CSeqVector seq_vec = bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, m_Strand);

    // update the location of the new cds and constantly check whether the protein is still the same as the original one
    CSeq_loc_CI iter_prev(cds.GetLocation(), CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Biological);
    CSeq_loc_CI iter = iter_prev;
    ++iter;

    TLocs::iterator range_it_prev = m_New_Ranges.begin();
    TLocs::iterator range_it      = range_it_prev;
    ++range_it;

    bool changed = false;
    size_t index = 1;
    while (iter && range_it != m_New_Ranges.end() && index < m_New_Ranges.size()) {
        
        CConstRef<CSeq_loc> loc_prev = iter_prev.GetRangeAsSeq_loc();
        CConstRef<CSeq_loc> loc      = iter.GetRangeAsSeq_loc();
        TSeqRange range_prev0 = *range_it_prev;
        TSeqRange range0      = *range_it;

        if (x_IntronLength(*range_it_prev, *range_it) > 9 && 
            !loc->IsPartialStart(eExtreme_Biological) && !loc_prev->IsPartialStop(eExtreme_Biological) &&
            (loc->IsInt() || loc->IsPnt()) && (loc_prev->IsInt() || loc_prev->IsPnt())) {    

            TSeqPos start = range_it_prev->GetFrom();
            TSeqPos stop  = range_it->GetTo();
            if (m_Strand == eNa_strand_minus) {
                start = length - range_it_prev->GetTo();
                stop = length - range_it->GetFrom();
            }

            string seqdata = kEmptyStr;
            seq_vec.GetSeqData(start, stop, seqdata);

            TSeqPos intron_start = range_it_prev->GetTo() + 1 - start; // donor splice site
            TSeqPos intron_stop  = range_it->GetFrom() -1 - start; // acceptor splice site
            if (m_Strand == eNa_strand_minus) {
                intron_start = length - range_it_prev->GetFrom() + 1 - start; 
                intron_stop = length - range_it->GetTo() -1 -start;
            }
            bool match = true;
            if (s_IsAdjustedSpliceSitePairOK(seqdata, intron_start, intron_stop)) {
                // do nothing
            } else {
                match = false;

                // search forward:
                if ((loc_prev->IsInt() || loc_prev->IsPnt()) && loc->IsInt()) {
                    TSeqPos offset = 1;
                    TSeqPos exon_length = range_it->GetTo() - range_it->GetFrom() + 1;
                    while (offset < exon_length && !match && offset < 4) {
                        if (s_IsAdjustedSpliceSitePairOK(seqdata, intron_start + offset, intron_stop + offset)) {
                            match = true;
                        } else {
                            offset++;
                        }
                    }
                    if (match) {
                        x_ShiftExonPairForward(*range_it_prev, *range_it, offset);
                    }
                }

                if (match) {
                    if (x_HasProteinChanged(cds, orig_prot_seq)) {
                        // undo the changes to the previous and the current range
                        swap(range_prev0, *range_it_prev);
                        swap(range0, *range_it);
                        match = false;
                    } else {
                        changed = true;
                    }
                }


                // search backward:
                if (!match && loc_prev->IsInt() && (loc->IsPnt() || loc->IsInt())) {
                    TSeqPos offset = 1;
                    TSeqPos prev_exon_length = range_it_prev->GetTo() - range_it_prev->GetFrom() + 1;
                    while (offset < prev_exon_length && !match && offset < 4) {
                        if (s_IsAdjustedSpliceSitePairOK(seqdata, intron_start - offset, intron_stop - offset)) {
                            match = true;
                        } else {
                            offset++;
                        }
                    }
                    if (match) {
                        x_ShiftExonPairBackward(*range_it_prev, *range_it, offset);
                        if (x_HasProteinChanged(cds, orig_prot_seq)) {
                            // undo the changes to the previous and the current range
                            swap(range_prev0, *range_it_prev);
                            swap(range0, *range_it);
                        } else {
                            changed = true;
                        }
                    }
                }
            }
        }

        ++range_it_prev;
        ++range_it;

        ++iter_prev;
        ++iter;
    }

    if (changed) {
        CRef<CSeq_loc> new_loc = x_UpdateLocation(cds);
        cds.ResetLocation();
        cds.SetLocation(new_loc.GetObject());
    }

    return changed;
}
 
bool CAdjustForConsensusSpliceSite::AdjustmRNAToMatchCDS(const CSeq_feat& edit_cds, CSeq_feat& mrna)
{
    return x_AlsoAdjustmRNA(mrna, edit_cds.GetLocation());
}

CRef<CCmdComposite> CAdjustForConsensusSpliceSite::AdjustmRNAandExonFeatures(const CSeq_feat& new_cds, const CSeq_feat& orig_cds)
{
    _ASSERT(m_Scope);
    CRef<CCmdComposite> cmd(new CCmdComposite("Update mRNA and exons for consensus splice sites"));

    CConstRef<CSeq_feat> mrna = sequence::GetmRNAforCDS(orig_cds, *m_Scope);
    if ( mrna ) {
        CRef<CSeq_feat> new_mrna(new CSeq_feat());
        new_mrna->Assign(*mrna);
        if (AdjustmRNAToMatchCDS(new_cds, *new_mrna)) {
            CSeq_feat_Handle mrnah = m_Scope->GetSeq_featHandle(mrna.GetObject());
            CIRef<IEditCommand> chgmRNA(new CCmdChangeSeq_feat(mrnah, *new_mrna));
            cmd->AddCommand(*chgmRNA);
        }
    }

    // update the exon features if there were any
    x_UpdateExonFeatures(cmd.GetPointer(), orig_cds, new_cds);
    return cmd;
}

void CAdjustForConsensusSpliceSite::x_UpdateExonFeatures(CCmdComposite* cmd, const CSeq_feat& orig_cds, const CSeq_feat& new_cds)
{
    if (!cmd)
        return;

    CSeq_loc_CI origloc_ci(orig_cds.GetLocation(), CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Positional);
    CSeq_loc_CI newloc_ci(new_cds.GetLocation(), CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Positional);
    for ( ; origloc_ci && newloc_ci; ++origloc_ci, ++newloc_ci) {
        // has been the range updated?
        if (origloc_ci.GetRange().GetFrom() != newloc_ci.GetRange().GetFrom() ||
            origloc_ci.GetRange().GetTo() != newloc_ci.GetRange().GetTo()) {
            // we are only interested in exons that are right on this interval

            CConstRef<CSeq_loc> orig_subloc = origloc_ci.GetRangeAsSeq_loc();
            CFeat_CI exon_ci(*m_Scope, orig_subloc.GetObject(), SAnnotSelector(CSeqFeatData::eSubtype_exon));
            if (exon_ci) {
                // check whether the exon is right on the original interval
                const CSeq_feat& exon = exon_ci->GetMappedFeature();
                if (exon.IsSetLocation()) {
                    TSeqPos exon_start = exon.GetLocation().GetStart(eExtreme_Positional);
                    TSeqPos exon_stop  = exon.GetLocation().GetStop(eExtreme_Positional);
                    if (exon_start == origloc_ci.GetRange().GetFrom() &&
                        exon_stop  == origloc_ci.GetRange().GetTo()) {
                        // adjust the exon for this interval
                        // make a copy of this exon and update the location of the exon
                        TSeqPos new_start = newloc_ci.GetRange().GetFrom();
                        TSeqPos new_stop  = newloc_ci.GetRange().GetTo();
                        CRef<CSeq_feat> exon(new CSeq_feat);
                        exon->Assign(exon_ci->GetMappedFeature());
                        if (exon->GetLocation().IsInt()) {
                            exon->SetLocation().SetInt().SetFrom(new_start);
                            exon->SetLocation().SetInt().SetTo(new_stop);
                            CSeq_feat_Handle exon_fh = m_Scope->GetSeq_featHandle(exon_ci->GetMappedFeature());
                            CIRef<IEditCommand> chgexon(new CCmdChangeSeq_feat(exon_fh, *exon));
                            cmd->AddCommand(*chgexon);
                        }
                    }
                }
            }
        }
    }
}

bool CAdjustForConsensusSpliceSite::s_IsBioseqGood_Strict(const CBioseq_Handle& bsh)
{
    // selection criteria for sequences where adjusting consensus splice sites can be applied
    bool accepted = s_IsBioseqGood_Relaxed(bsh);
    if (accepted) {
        CSeqdesc_CI desc(bsh, CSeqdesc::e_Source);
        if (!desc)
            return false;
        
        const CBioSource& bsrc = desc->GetSource();
        if (bsrc.IsSetLineage() && NStr::FindNoCase(bsrc.GetLineage(), "viruses") != NPOS) {
            return false;
        } else if (bsrc.IsSetOrg() && bsrc.GetOrg().GetTaxId() == 0) {
            return false;
        }
    }

    return accepted;
}

bool CAdjustForConsensusSpliceSite::s_IsBioseqGood_Relaxed(const CBioseq_Handle& bsh)
{
    // it wont' work on sequences that meet any of the following criteria:
    CSeqdesc_CI desc(bsh, CSeqdesc::e_Source);
    if (!desc)
        return false;
    
    const CBioSource& bsrc = desc->GetSource();
    if (bsrc.GetGenome() != CBioSource::eGenome_genomic &&
        bsrc.GetGenome() != CBioSource::eGenome_unknown) {
        return false;
    }
    return true;
}

bool CAdjustForConsensusSpliceSite::s_IsBioseqGood_AdjustEnds(const objects::CBioseq_Handle& bsh)
{
    // must be genomic DNA
    CConstRef<CSeqdesc> molinfo = bsh.GetCompleteBioseq()->GetClosestDescriptor(CSeqdesc::e_Molinfo);
    bool set_genomic = molinfo && molinfo->GetMolinfo().IsSetBiomol() && molinfo->GetMolinfo().GetBiomol() == CMolInfo::eBiomol_genomic;
    if (!set_genomic)
        return false;

    if (bsh.GetInst().GetMol() != CSeq_inst::eMol_dna)
        return false;

    // no organelle
    if (validator::IsOrganelle(bsh))
        return false;

    CSeqdesc_CI desc(bsh, CSeqdesc::e_Source);
    if (!desc)
        return false;

    const CBioSource& bsrc = desc->GetSource();
    // must have a taxID
    if (!bsrc.IsSetOrg())
        return false;
    if (bsrc.GetOrg().GetTaxId() == 0)
        return false;

    return true;
}

namespace {
    bool s_IsAcceptorSpliceSiteOK(const string& seqdata, TSeqPos intron_stop)
    {
        if (seqdata.empty()) return false;
        return (seqdata.at(intron_stop - 1) == 'A' && seqdata.at(intron_stop) == 'G');
    }

    bool s_IsDonorSpliceSiteOK(const string& seqdata, TSeqPos intron_start)
    {
        if (seqdata.empty()) return false;
        return (seqdata.at(intron_start) == 'G' && (seqdata.at(intron_start + 1) == 'T' || seqdata.at(intron_start + 1) == 'C'));
    }
}

bool CAdjustForConsensusSpliceSite::s_IsAdjustedSpliceSitePairOK(const string& seqdata, TSeqPos intron_start, TSeqPos intron_stop) 
{
    if (seqdata.empty()) return false;
    try {
        return s_IsAcceptorSpliceSiteOK(seqdata, intron_stop) && s_IsDonorSpliceSiteOK(seqdata, intron_start);
    } catch (const CException& e) {
        LOG_POST(Error << "Lookup of splice sites failed: " << e.GetMsg());
    }
    catch (const exception& e) {
        LOG_POST(Error << "Lookup of splice sites failed: " << e.what());
    }
    return false;
}



bool CAdjustForConsensusSpliceSite::x_AlsoAdjustmRNA(CSeq_feat& mrna, const CSeq_loc& loc)
{
    // don't change the end points of mrna, but the internal intervals should be the same
    TSeqPos cds_start = loc.GetTotalRange().GetFrom();
    TSeqPos cds_stop  = loc.GetTotalRange().GetTo();

    TSeqPos mRNA_start = mrna.GetLocation().GetStart(eExtreme_Positional);
    TSeqPos mRNA_stop = mrna.GetLocation().GetStop(eExtreme_Positional);

    // if the endpoints of mrna are the same as the cds endpoints, just update the mrna with the new location
    // also update the partialness of the endpoints
    if (mRNA_start == cds_start && mRNA_stop == cds_stop) {
        CRef<CSeq_loc> new_loc(new CSeq_loc);
        new_loc->Assign(loc);

        if (mrna.GetLocation().IsPartialStart(eExtreme_Positional)) {
            new_loc->SetPartialStart(true, eExtreme_Positional);
        }
        if (mrna.GetLocation().IsPartialStop(eExtreme_Positional)) {
            new_loc->SetPartialStop(true, eExtreme_Positional);
        }
        // not setting the strand, as it should be the same as the CDS's strand
        // not setting the id either, for the same reason
        
        mrna.ResetLocation();
        mrna.SetLocation(new_loc.GetObject());
        return true;
    } else {
        TLocs mrna_orig_ranges;
        for (CSeq_loc_CI loc_iter(mrna.GetLocation(), CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Positional);  loc_iter;  ++loc_iter) {
            mrna_orig_ranges.push_back(loc_iter.GetRange());
        }

        // update the ranges
        TLocs mrna_ranges;
        ITERATE(TLocs, iter, m_New_Ranges) {
            mrna_ranges.push_back(*iter);
        }

        TLocs::iterator range_it = mrna_ranges.begin(); // or update it with the orig_ranges
        if (mRNA_start != range_it->GetFrom())
            range_it->SetFrom(mRNA_start);
        range_it = mrna_ranges.end() - 1 ;
        if (mRNA_stop != range_it->GetTo())
            range_it->SetTo(mRNA_stop);

        CRef<CSeq_loc> new_loc(new CSeq_loc);
        if (mrna.GetLocation().IsMix()) {
            CSeq_loc::TMix& mix_locs = new_loc->SetMix();

            TLocs::iterator range_it = mrna_ranges.begin();
            TLocs::iterator orig_rng_it = mrna_orig_ranges.begin();
            CSeq_loc_CI loc_iter(mrna.GetLocation(), CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Positional);

            while (loc_iter && range_it != mrna_ranges.end() && orig_rng_it != mrna_orig_ranges.end()) {
                CConstRef<CSeq_loc> subloc = loc_iter.GetRangeAsSeq_loc();
                if (range_it->GetFrom() != orig_rng_it->GetFrom() || range_it->GetTo() != orig_rng_it->GetTo()) { 
                  // update the subloc
                    const CSeq_id* seqid = subloc->GetId();
                    if (seqid) 
                        mix_locs.AddInterval(*seqid, range_it->GetFrom(), range_it->GetTo(), subloc->GetStrand());	
                } else {
                    if (subloc->IsPnt() || subloc->IsInt()) {
                        mix_locs.AddSeqLoc(*subloc);
                    } // other types are not allowed 
                }
                ++range_it;
                ++orig_rng_it;
                ++loc_iter;
            }
        } else if (mrna.GetLocation().IsPacked_int()) {
            CRef<CPacked_seqint> packed(new CPacked_seqint(const_cast<CSeq_id&>(*(mrna.GetLocation().GetId())), mrna_ranges, mrna.GetLocation().GetStrand()));
            new_loc->SetPacked_int(packed.GetObject());
        }

        if (mrna.GetLocation().IsPartialStart(eExtreme_Positional)) {
            new_loc->SetPartialStart(true, eExtreme_Positional);
        }
        if (mrna.GetLocation().IsPartialStop(eExtreme_Positional)) {
            new_loc->SetPartialStop(true, eExtreme_Positional);
        }
        // not setting the strand, as it should be the same as the CDS's strand
        // not setting the id either, for the same reason
        
        mrna.ResetLocation();
        mrna.SetLocation(new_loc.GetObject());
        return true;
    }	
    return false;
}


CRef<CSeq_loc> CAdjustForConsensusSpliceSite::x_UpdateLocation(const CSeq_feat& cds)
{
    CRef<CSeq_loc> new_loc(new CSeq_loc);

    if (cds.GetLocation().IsMix()) {

        CSeq_loc::TMix& mix_locs = new_loc->SetMix();

        TLocs::iterator range_it = m_New_Ranges.begin();
        TLocs::iterator orig_rng_it = m_Orig_Ranges.begin();
        CSeq_loc_CI loc_iter(cds.GetLocation(), CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Biological);

        while (loc_iter && range_it != m_New_Ranges.end() && orig_rng_it != m_Orig_Ranges.end()) {
            CConstRef<CSeq_loc> subloc = loc_iter.GetRangeAsSeq_loc();
            if (range_it->GetFrom() != orig_rng_it->GetFrom() || range_it->GetTo() != orig_rng_it->GetTo()) { 
              // update the subloc
                const CSeq_id* seqid = subloc->GetId();
                if (seqid) 
                    mix_locs.AddInterval(*seqid, range_it->GetFrom(), range_it->GetTo(), subloc->GetStrand());	
            } else {
                if (subloc->IsPnt() || subloc->IsInt()) {
                    mix_locs.AddSeqLoc(*subloc);
                } // other types are not allowed 
            }
            ++range_it;
            ++orig_rng_it;
            ++loc_iter;
        }
    } else if (cds.GetLocation().IsPacked_int()) {
        CRef<CPacked_seqint> packed(new CPacked_seqint(const_cast<CSeq_id&>(*(cds.GetLocation().GetId())), m_New_Ranges, cds.GetLocation().GetStrand()));
        new_loc->SetPacked_int(packed.GetObject());
    }
    else if (cds.GetLocation().IsInt()) {
        TLocs::iterator range_it = m_New_Ranges.begin();
        CRef<CSeq_interval> interval(new CSeq_interval(const_cast<CSeq_id&>(*(cds.GetLocation().GetId())), range_it->GetFrom(), range_it->GetTo(), cds.GetLocation().GetStrand()));
        new_loc->SetInt(*interval);
    }

    if (cds.GetLocation().IsPartialStart(eExtreme_Biological)) {
        new_loc->SetPartialStart(true, eExtreme_Biological);
    }
    if (cds.GetLocation().IsPartialStop(eExtreme_Biological)) {
        new_loc->SetPartialStop(true, eExtreme_Biological);
    }

    return new_loc;
}

void CAdjustForConsensusSpliceSite::x_InitRanges(const CSeq_feat& cds)
{
    m_Orig_Ranges.clear();
    m_New_Ranges.clear();
    for (CSeq_loc_CI loc_iter(cds.GetLocation(), CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Biological);  loc_iter;  ++loc_iter) {
        m_Orig_Ranges.push_back(loc_iter.GetRange());
        m_New_Ranges.push_back(loc_iter.GetRange());
    }
}

CRef<objects::CSeq_loc> CAdjustForConsensusSpliceSite::x_CreateNewLocation(const CSeq_feat& cds)
{
    CRef<CSeq_loc> new_loc(new CSeq_loc);

    // make/update new location from the new location ranges:
    if (cds.GetLocation().IsMix()) {
        CSeq_loc::TLocations& mix_locs = new_loc->SetMix().Set();
        x_UpdateMixLocations(cds.GetLocation(), mix_locs);
    }
    else if (cds.GetLocation().IsPacked_int()) {
        new_loc->Assign(cds.GetLocation());
        CSeq_loc::TIntervals& int_locs = new_loc->SetPacked_int().Set();
        x_UpdateIntervals(int_locs);
    }
    else if (cds.GetLocation().IsInt()) {
        new_loc->Assign(cds.GetLocation());
        x_UpdateInterval(new_loc->SetInt());
    }
    return new_loc;
}

bool CAdjustForConsensusSpliceSite::x_HasProteinChanged(const CSeq_feat& cds, const string& orig_prot_seq)
{
    CRef<CSeq_feat> new_cds(new CSeq_feat);
    new_cds->Assign(cds);
    new_cds->ResetLocation();
    CRef<CSeq_loc> new_loc = x_CreateNewLocation(cds);
    new_cds->SetLocation(*new_loc);

    string new_prot_seq = GetProteinSeq(*new_cds, *m_Scope);
    return !NStr::EqualNocase(new_prot_seq, orig_prot_seq);
}

bool CAdjustForConsensusSpliceSite::x_HasProteinChangedAt5End(CSeq_feat& cds, const string& orig_prot_seq)
{
    // A similar function to x_HasProteinChanged() function, 
    // Note that the cds might change, specifically its frame, as the new protein might differ 
    // from the original in one amino acid when adjusting the frame

    CRef<CSeq_feat> new_cds(new CSeq_feat);
    new_cds->Assign(cds);
    new_cds->ResetLocation();
    CRef<CSeq_loc> new_loc = x_CreateNewLocation(cds);
    new_cds->SetLocation(*new_loc);

    string new_prot_seq = GetProteinSeq(*new_cds, *m_Scope);
    bool changed = !NStr::EqualNocase(new_prot_seq, orig_prot_seq);

    if (changed) {
        // the original frame of the CDS
        CCdregion::TFrame orig_frame = CCdregion::eFrame_not_set;
        if (cds.GetData().GetCdregion().IsSetFrame()) {
            orig_frame = cds.GetData().GetCdregion().GetFrame();
        }

        // check whether by changing the frame, the protein remains the same (might be longer by 1 amino acid)
        for (int enumI = CCdregion::eFrame_one; enumI < CCdregion::eFrame_three + 1; ++enumI) {
            CCdregion::EFrame fr = (CCdregion::EFrame) (enumI);
            new_cds->SetData().SetCdregion().SetFrame(fr);

            string prot_seq = GetProteinSeq(*new_cds, *m_Scope);
            if (NStr::EqualNocase(prot_seq, orig_prot_seq)) {
                if (fr != orig_frame) {
                    cds.SetData().SetCdregion().SetFrame(fr);
                }
                changed = false;
            }
            else {
                prot_seq.erase(prot_seq.begin());
                if (NStr::EqualNocase(prot_seq, orig_prot_seq)) {
                    if (fr != orig_frame) {
                        cds.SetData().SetCdregion().SetFrame(fr);
                    }
                    changed = false;
                    m_ProtChanged = true;
                }
            }
        }
    }
    return changed;
}

bool CAdjustForConsensusSpliceSite::x_HasProteinChangedAt3End(CSeq_feat& cds, const string& orig_prot_seq)
{
    // the new protein might be shorter than the original one
    CRef<CSeq_feat> new_cds(new CSeq_feat);
    new_cds->Assign(cds);
    new_cds->ResetLocation();
    CRef<CSeq_loc> new_loc = x_CreateNewLocation(cds);
    new_cds->SetLocation(*new_loc);

    const string new_prot_seq = GetProteinSeq(*new_cds, *m_Scope);
    bool changed = !NStr::EqualNocase(new_prot_seq, orig_prot_seq);

    if (changed) {
        string modified_prot = new_prot_seq.substr(0, new_prot_seq.length() - 1);
        if (NStr::EqualNocase(modified_prot, orig_prot_seq)) {
            changed = false;
            m_ProtChanged = true;
        }
        else {
            string shorter_orig_prot = orig_prot_seq.substr(0, orig_prot_seq.length() - 1);
            if (NStr::EqualNocase(new_prot_seq, shorter_orig_prot)) {
                changed = false;
                m_ProtChanged = true;
            }
        }

    }
    return changed;
}

void CAdjustForConsensusSpliceSite::x_UpdateMixLocations(const CSeq_loc& orig_loc, CSeq_loc::TLocations& locs)
{
    if ( ! orig_loc.IsMix())
        return;

    locs.clear();
    CSeq_loc_CI loc_iter(orig_loc, CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Biological);
    TLocs::iterator range_it = m_New_Ranges.begin();
    TLocs::iterator orig_rng_it = m_Orig_Ranges.begin();

    while ( loc_iter && range_it != m_New_Ranges.end() && orig_rng_it != m_Orig_Ranges.end()) {
        CConstRef<CSeq_loc> subloc = loc_iter.GetRangeAsSeq_loc();
        if (subloc->IsPnt()) {
            const CSeq_point& pnt = subloc->GetPnt();
            CRef<CSeq_loc> new_subloc(new CSeq_loc);
            if (range_it->GetFrom() != orig_rng_it->GetFrom() || range_it->GetTo() != orig_rng_it->GetTo()) {
                // make a new interval
                new_subloc->SetInt().SetFrom(pnt.GetPoint());
                new_subloc->SetInt().SetTo(pnt.GetPoint());
                new_subloc->SetInt().SetStrand(m_Strand);
                CRef<CSeq_id> new_id(new CSeq_id);
                new_id->Assign(pnt.GetId());
                new_subloc->SetInt().SetId(*new_id);

            } else {
                // just copy the old point
                new_subloc->SetPnt().Assign(pnt);
            }
            locs.push_back(new_subloc);

        } else if (subloc->IsInt()) {
            CRef<CSeq_loc> new_subloc(new CSeq_loc);
            new_subloc->Assign(*subloc);

            if (range_it->GetFrom() != orig_rng_it->GetFrom()) {
                new_subloc->SetInt().SetFrom(range_it->GetFrom());
            } 
            if (range_it->GetTo() != orig_rng_it->GetTo()) {
                new_subloc->SetInt().SetTo(range_it->GetTo());
            }
            locs.push_back(new_subloc);

        } else {
            // other cases should not be encountered
        }
        ++loc_iter;
        ++range_it;
        ++orig_rng_it;
    }

}

void CAdjustForConsensusSpliceSite::x_UpdateInterval(CSeq_interval& interval)
{
    TLocs::iterator range_it = m_New_Ranges.begin();
    TLocs::iterator orig_rng_it = m_Orig_Ranges.begin();
    if (range_it != m_New_Ranges.end() && orig_rng_it != m_Orig_Ranges.end()) {
        if (range_it->GetFrom() != orig_rng_it->GetFrom()) {
            interval.SetFrom(range_it->GetFrom());
        }
        if (range_it->GetTo() != orig_rng_it->GetTo()) {
            interval.SetTo(range_it->GetTo());
        }
    }
}

void CAdjustForConsensusSpliceSite::x_UpdateIntervals(CSeq_loc::TIntervals& locs)
{
    CSeq_loc::TIntervals::iterator it = locs.begin();
    TLocs::iterator range_it = m_New_Ranges.begin();
    TLocs::iterator orig_rng_it = m_Orig_Ranges.begin();
    
    while (it != locs.end() && range_it != m_New_Ranges.end() && orig_rng_it != m_Orig_Ranges.end()) {
        CSeq_interval& interval = **it;
        if (range_it->GetFrom() != orig_rng_it->GetFrom()) {
            interval.SetFrom(range_it->GetFrom());
        }
        if (range_it->GetTo() != orig_rng_it->GetTo()) {
            interval.SetTo(range_it->GetTo());
        }
        ++it;
        ++range_it;
        ++orig_rng_it;
    }
}


void CAdjustForConsensusSpliceSite::x_ShiftExonPairForward(TSeqRange& range_prev, TSeqRange& range, TSeqPos offset)
{
    if (range.GetFrom() >= range.GetTo())
        return;
    if (m_Strand == eNa_strand_minus) {
        range_prev.SetFrom(range_prev.GetFrom() - offset);
        range.SetTo(range.GetTo() - offset);
    } else {
        range_prev.SetTo(range_prev.GetTo() + offset);
        range.SetFrom(range.GetFrom() + offset);
    }
}

void CAdjustForConsensusSpliceSite::x_ShiftExonPairBackward(TSeqRange& range_prev, TSeqRange& range, TSeqPos offset)
{
    if (range_prev.GetFrom() >= range_prev.GetTo())
        return;
    if (m_Strand == eNa_strand_minus) {
        range.SetTo(range.GetTo() + offset);
        range_prev.SetFrom(range_prev.GetFrom() + offset);
    } else {
        range.SetFrom(range.GetFrom() - offset);
        range_prev.SetTo(range_prev.GetTo() - offset);
    }
}

TSeqPos CAdjustForConsensusSpliceSite::x_IntronLength(const TSeqRange& rng1, const TSeqRange& rng2)
{
    if (rng1.GetFrom() == rng2.GetFrom() && rng1.GetTo() == rng2.GetTo())
        return 0;
    
    TSeqPos intron_len = 0;
    if (m_Strand == eNa_strand_minus) {
        intron_len = rng1.GetFrom() - rng2.GetTo() - 1;
    } else {
        intron_len = rng2.GetFrom() - rng1.GetTo() - 1;
    }
    return intron_len;
}


bool CAdjustForConsensusSpliceSite::AdjustCDSEnds(CSeq_feat& cds)
{
    _ASSERT(m_Scope);
    if (!cds.GetData().IsCdregion() || !cds.IsSetLocation() || !cds.IsSetProduct()) {
        return false;
    }
    if (cds.GetLocation().GetId() == nullptr) {
        return false;
    }

    // not going to handle mixed-strand exons
    if (cds.GetLocation().GetStrand() == eNa_strand_other) {
        return false;
    }

    // no CDS exceptions set
    if ((cds.IsSetExcept() && cds.GetExcept()) || cds.IsSetExcept_text())
        return false;

    // assumed that the coding region is on one sequence
    CBioseq_Handle bsh = m_Scope->GetBioseqHandle(cds.GetLocation());
    if (!bsh) {
        return false;
    }

    CBioseq_Handle product = m_Scope->GetBioseqHandle(cds.GetProduct());
    if (!product || !product.IsProtein()) {
        return false;
    }

    // obtaining the original protein sequence
    CSeqVector prot_vec = product.GetSeqVector(CBioseq_Handle::eCoding_Ncbi);
    prot_vec.SetCoding(CSeq_data::e_Ncbieaa);
    string orig_prot_seq;
    prot_vec.GetSeqData(0, prot_vec.size(), orig_prot_seq);
    if (orig_prot_seq.empty()) {
        return false;
    }

    x_InitRanges(cds);
    m_Strand = cds.GetLocation().GetStrand();

    bool changed = false;
    if (orig_prot_seq.front() != 'M' && 
        cds.GetLocation().IsPartialStart(eExtreme_Biological) && 
        !edit::CLocationEditPolicy::Is5AtEndOfSeq(cds.GetLocation(), bsh)) {
        
        changed = x_AdjustCDS5End(cds, orig_prot_seq);
    }

    if (orig_prot_seq.back() != '*' && 
        cds.GetLocation().IsPartialStop(eExtreme_Biological) &&
        !edit::CLocationEditPolicy::Is3AtEndOfSeq(cds.GetLocation(), bsh)) {

        changed |= x_AdjustCDS3End(cds, orig_prot_seq);
    }
    if (changed) {
        CRef<CSeq_loc> new_loc = x_UpdateLocation(cds);
        cds.ResetLocation();
        cds.SetLocation(new_loc.GetObject());
    }
    
    return changed;
}

namespace {
    // check whether the location is 1 or two bases away from the end of sequence
    bool IsCloseTo5EndOfSeq(const CSeq_loc& loc, CBioseq_Handle bsh)
    {
        if (!bsh) return false;

        ENa_strand strand = loc.GetStrand();
        const auto  start = loc.GetStart(eExtreme_Biological);

        const auto seq_length = bsh.GetInst_Length() - 1;
        bool rval = false;

        if (strand == eNa_strand_minus) {
            if (seq_length - start == 1 || seq_length - start == 2) {
                rval = true;
            }
        }
        else {
            if (start == 1 || start == 2) {
                rval = true;
            }
        }
        return rval;
    }

    bool IsCloseTo3EndOfSeq(const CSeq_loc& loc, CBioseq_Handle bsh)
    {
        if (!bsh) return false;

        ENa_strand strand = loc.GetStrand();
        const auto stop = loc.GetStop(eExtreme_Biological);

        const auto seq_length = bsh.GetInst_Length() - 1;
        bool rval = false;

        if (strand == eNa_strand_minus) {
            if (stop == 1 || stop == 2) {
                rval = true;
            }
        }
        else {
            if (seq_length - stop == 1 || seq_length - stop == 2) {
                rval = true;
            }
        }
        return rval;
    }
}

bool CAdjustForConsensusSpliceSite::x_AdjustCDS5End(CSeq_feat& cds, const string& orig_prot_seq)
{
    CBioseq_Handle bsh = m_Scope->GetBioseqHandle(cds.GetLocation());
    TSeqPos length = bsh.GetBioseqLength() - 1;
    CSeqVector seq_vec = bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, m_Strand);

    // update the location of the new cds and constantly check whether the protein is still the same as the original one
    CSeq_loc_CI iter(cds.GetLocation(), CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Biological);

    TLocs::iterator range_it = m_New_Ranges.begin();
    TSeqRange range_orig = *range_it;

    bool changed = false;

    CConstRef<CSeq_loc> loc = iter.GetRangeAsSeq_loc();

    if (loc->IsInt() || loc->IsPnt()) {

        TSeqPos start = 0;
        TSeqPos stop = range_it->GetTo();
        if (m_Strand == eNa_strand_minus) {
            stop = length - range_it->GetFrom();
        }

        string seqdata = kEmptyStr;
        seq_vec.GetSeqData(start, stop, seqdata);

        TSeqPos intron_stop = range_it->GetFrom() - 1 - start; // acceptor splice site
        if (m_Strand == eNa_strand_minus) {
            intron_stop = length - range_it->GetTo() - 1 - start;
        }

        if (intron_stop < seqdata.size() &&
            intron_stop - 1 < seqdata.size() && // for plus strand
            s_IsAcceptorSpliceSiteOK(seqdata, intron_stop)) {
            // no adjustment is needed
            return changed;
        }

        TSeqPos offset = 1;
        while (offset < 3 && !changed) {
            if (intron_stop - offset < seqdata.size() &&
                intron_stop - offset - 1 < seqdata.size() &&
                s_IsAcceptorSpliceSiteOK(seqdata, intron_stop - offset)) {

                bool ret = x_ExtendStartOfExon(*range_it, offset, cds.GetLocation(), bsh);
                _ASSERT(ret);
                if (x_HasProteinChangedAt5End(cds, orig_prot_seq)) {
                    // undo the changes to the previous and the current range
                    swap(range_orig, *range_it);
                }
                else {
                    changed = true;
                }
            }

            if (!changed &&
                intron_stop + offset < seqdata.size() &&
                intron_stop + offset + 1 < seqdata.size() &&
                s_IsAcceptorSpliceSiteOK(seqdata, intron_stop + offset)) {

                x_TrimStartOfExon(*range_it, offset);
                if (x_HasProteinChangedAt5End(cds, orig_prot_seq)) {
                    // undo the changes to the previous and the current range
                    swap(range_orig, *range_it);
                }
                else {
                    changed = true;
                }
            }

            if (!changed) ++offset;
        }

        // try extending it to the end of sequence
        if (!changed && IsCloseTo5EndOfSeq(cds.GetLocation(), bsh)) { 
            offset = 2;
            while (offset > 0 && !changed) {
                if (x_ExtendStartOfExon(*range_it, offset, cds.GetLocation(), bsh)) {
                    if (x_HasProteinChangedAt5End(cds, orig_prot_seq)) {
                        // undo the changes to the previous and the current range
                        swap(range_orig, *range_it);
                    }
                    else {
                        changed = true;
                    }
                }
                if (!changed) --offset;
            }
        }
    }
    return changed;
}


bool CAdjustForConsensusSpliceSite::x_ExtendStartOfExon(TSeqRange& range, TSeqPos offset, const CSeq_loc& loc, CBioseq_Handle bsh)
{
    bool extended = false;
    const auto seq_stop = bsh.GetInst_Length() - 1;

    if (m_Strand == eNa_strand_minus) {
        if (range.GetTo() + offset <= seq_stop) {
            range.SetTo(range.GetTo() + offset);
            extended = true;
        }
    }
    else {
        if ((int)range.GetFrom() - (int)offset >= 0) {
            range.SetFrom(range.GetFrom() - offset);
            extended = true;
        }
    }
    return extended;
}

void CAdjustForConsensusSpliceSite::x_TrimStartOfExon(TSeqRange& range, TSeqPos offset)
{
    if (m_Strand == eNa_strand_minus) {
        range.SetTo(range.GetTo() - offset);
    }
    else {
        range.SetFrom(range.GetFrom() + offset);
    }
}

bool CAdjustForConsensusSpliceSite::x_AdjustCDS3End(CSeq_feat& cds, const string& orig_prot_seq)
{
    CBioseq_Handle bsh = m_Scope->GetBioseqHandle(cds.GetLocation());
    TSeqPos length = bsh.GetBioseqLength() - 1;
    CSeqVector seq_vec = bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, m_Strand);

    // update the location of the new cds and constantly check whether the protein is still the same as the original one
    CSeq_loc_CI iter(cds.GetLocation(), CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Biological);
    iter.SetPos(iter.GetSize() - 1);

    TLocs::iterator range_it = m_New_Ranges.end() - 1;
    TSeqRange range_orig = *range_it;

    bool changed = false;

    CConstRef<CSeq_loc> loc = iter.GetRangeAsSeq_loc();

    if (loc->IsInt() || loc->IsPnt()) {
        TSeqPos start = range_it->GetFrom();
        TSeqPos stop = length - 1;
        if (m_Strand == eNa_strand_minus) {
            start = length - range_it->GetTo();
        }

        string seqdata = kEmptyStr;
        seq_vec.GetSeqData(start, stop, seqdata);

        TSeqPos intron_start = range_it->GetTo() + 1 - start; // donor splice site
        if (m_Strand == eNa_strand_minus) {
            intron_start = length - range_it->GetFrom() + 1 - start;
        }

        if (intron_start < seqdata.size() && 
            intron_start + 1 < seqdata.size() && // for plus strand
            s_IsDonorSpliceSiteOK(seqdata, intron_start)) {
            // no adjustment is needed
            return changed; 
        }


        TSeqPos offset = 1;
        while (offset < 3 && !changed) {
            if (intron_start + offset < seqdata.size() &&
                intron_start + offset - 1 < seqdata.size() &&
                s_IsDonorSpliceSiteOK(seqdata, intron_start + offset)) {

                bool ret = x_ExtendStopOfExon(*range_it, offset, cds.GetLocation(), bsh);
                _ASSERT(ret);
                if (x_HasProteinChangedAt3End(cds, orig_prot_seq)) {
                    // undo the changes to the previous and the current range
                    swap(range_orig, *range_it);
                }
                else {
                    changed = true;
                }
            }

            if (!changed &&
                intron_start - offset < seqdata.size() &&
                intron_start - offset - 1 < seqdata.size() &&
                s_IsDonorSpliceSiteOK(seqdata, intron_start - offset)) {

                x_TrimStopOfExon(*range_it, offset);
                if (x_HasProteinChangedAt3End(cds, orig_prot_seq)) {
                    // undo the changes to the previous and the current range
                    swap(range_orig, *range_it);
                }
                else {
                    changed = true;
                }
            }

            if (!changed) ++offset;
        }

        // try extending it to the end of sequence
        if (!changed && IsCloseTo3EndOfSeq(cds.GetLocation(), bsh)) { 
            offset = 2;
            while (offset > 0  && !changed) {
                if (x_ExtendStopOfExon(*range_it, offset, cds.GetLocation(), bsh)) {
                    if (x_HasProteinChangedAt3End(cds, orig_prot_seq)) {
                        // undo the changes to the previous and the current range
                        swap(range_orig, *range_it);
                    }
                    else {
                        changed = true;
                    }
                }
                if (!changed) --offset;
            }
        }
    }
    return changed;
}

bool CAdjustForConsensusSpliceSite::x_ExtendStopOfExon(TSeqRange& range, TSeqPos offset, const CSeq_loc& loc, CBioseq_Handle bsh)
{
    bool extended = false;
    const auto seq_stop = bsh.GetInst_Length() - 1;

    if (m_Strand == eNa_strand_minus) {
        if ((int)range.GetFrom() - (int)offset >= 0) {
            range.SetFrom(range.GetFrom() - offset);
            extended = true;
        }
    }
    else {
        if (range.GetTo() + offset <= seq_stop) {
            range.SetTo(range.GetTo() + offset);
            extended = true;
        }
    }
    return extended;
}

void CAdjustForConsensusSpliceSite::x_TrimStopOfExon(TSeqRange& range, TSeqPos offset)
{
    if (m_Strand == eNa_strand_minus) {
        range.SetFrom(range.GetFrom() + offset);
    }
    else {
        range.SetTo(range.GetTo() - offset);
    }
}

CRef<CCmdComposite> CAdjustForConsensusSpliceSite::AdjustmRNAandExonEnds(const CSeq_feat& new_cds, const CSeq_feat& orig_cds)
{
    _ASSERT(m_Scope);
    CRef<CCmdComposite> cmd(new CCmdComposite("Update mRNA and exons for consensus splice sites at the ends"));

    CConstRef<CSeq_feat> mrna = sequence::GetmRNAforCDS(orig_cds, *m_Scope);
    if (mrna) {
        CRef<CSeq_feat> new_mrna(new CSeq_feat());
        new_mrna->Assign(*mrna);
        if (AdjustmRNAToMatchCDSEnds(new_cds, *new_mrna)) {
            CSeq_feat_Handle mrnah = m_Scope->GetSeq_featHandle(mrna.GetObject());
            CIRef<IEditCommand> chgmRNA(new CCmdChangeSeq_feat(mrnah, *new_mrna));
            cmd->AddCommand(*chgmRNA);
        }
    }

    // update the exon features if there were any
    x_UpdateExonFeatures(cmd.GetPointer(), orig_cds, new_cds);
    return cmd;
}

bool CAdjustForConsensusSpliceSite::AdjustmRNAToMatchCDSEnds(const CSeq_feat& edit_cds, CSeq_feat& mrna)
{
    const auto& cds_loc = edit_cds.GetLocation();
    TSeqPos cds_start = cds_loc.GetStart(eExtreme_Biological);
    TSeqPos cds_stop = cds_loc.GetStop(eExtreme_Biological);

    const auto& orig_mrna_loc = mrna.GetLocation();
    TSeqPos mrna_start = orig_mrna_loc.GetStart(eExtreme_Biological);
    TSeqPos mrna_stop = orig_mrna_loc.GetStop(eExtreme_Biological);

    CRef<CSeq_loc> new_loc(new CSeq_loc);
    new_loc->Assign(mrna.GetLocation());

    CSeq_loc_I loc_it(*new_loc);
    // adjust the first sub-interval
    if (cds_start != mrna_start) {
        if (loc_it.IsSetStrand() && loc_it.GetStrand() == eNa_strand_minus) {
            loc_it.SetTo(cds_start);
        }
        else {
            loc_it.SetFrom(cds_start);
        }
    }

    // adjust the last sub-interval
    if (cds_stop != mrna_stop) {
        auto num_intervals = loc_it.GetSize();
        loc_it.SetPos(num_intervals - 1);
        if (loc_it.IsSetStrand() && loc_it.GetStrand() == eNa_strand_minus) {
            loc_it.SetFrom(cds_stop);
        }
        else {
            loc_it.SetTo(cds_stop);
        }
    }
    
    if (loc_it.HasChanges()) {
        CRef<CSeq_loc> loc = loc_it.MakeSeq_loc();
        mrna.SetLocation(*loc);
        return true;
    }
    return false;
}


END_NCBI_SCOPE
