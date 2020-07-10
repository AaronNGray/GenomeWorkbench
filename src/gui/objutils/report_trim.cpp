/*  $Id: report_trim.cpp 41710 2018-09-12 14:35:34Z filippov $
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
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <objmgr/seq_vector.hpp>
#include <gui/objutils/cmd_change_align.hpp>
#include <gui/objutils/cmd_del_seq_align.hpp>
#include <gui/objutils/cmd_change_graph.hpp>
#include <gui/objutils/cmd_del_seq_graph.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/graph_ci.hpp>

#include <gui/objutils/report_trim.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;


void CReportTrim::AdjustAndTranslate(CBioseq_Handle bsh, int orig_length, int orig_from, int orig_to, CRef<CCmdComposite> command, CScope &scope, 
				     map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle> > &product_to_cds)
{
    CSeq_entry_Handle seh = bsh.GetParentEntry();

    CBioseq_set_Handle bssh = bsh.GetParentBioseq_set();
    if (bssh && bssh.IsSetClass() && bssh.GetClass() == CBioseq_set::eClass_nuc_prot)
        seh = bssh.GetParentEntry();
    
    CFeat_CI feat_ci(seh, CSeq_annot::C_Data::e_Ftable);
    for (; feat_ci; ++feat_ci)
    {    
        int from = orig_from;
        int to = orig_to;
        int length = orig_length;

        if (x_IsProtLocation(feat_ci->GetLocation(), bsh, scope))
        {
            continue;
        }
        if (feat_ci->IsSetProduct() && feat_ci->GetData().IsCdregion() &&
            feat_ci->GetOriginalFeature().IsSetExcept_text() && NStr::Find(feat_ci->GetOriginalFeature().GetExcept_text(), "RNA editing") != string::npos)
        {
            continue;
        }
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(feat_ci->GetOriginalFeature());

        int diff = 0;
        if ( x_AdjustLocation(new_feat->SetLocation(),length,from,to,diff) )
        {
            CRef<CCmdComposite> cmd = GetDeleteFeatureCommand(*feat_ci, true, product_to_cds);
            command->AddCommand(*cmd);
            continue;
        }

        if (feat_ci->IsSetData() && feat_ci->GetData().IsCdregion())
        {
            x_AdjustCdregionFrame(new_feat, diff);
            //new_feat->SetData().SetCdregion().SetFrame(objects::CSeqTranslator::FindBestFrame(*new_feat,scope));

            EDIT_EACH_CODEBREAK_ON_CDREGION(codebreak, new_feat->SetData().SetCdregion())
            {
                if ((*codebreak)->IsSetLoc())
                {
                    if ( x_AdjustLocation((*codebreak)->SetLoc(),length,from,to))
                    {
                        ERASE_CODEBREAK_ON_CDREGION(codebreak, new_feat->SetData().SetCdregion());
                    }
                }
            }
            if (new_feat->GetData().GetCdregion().IsSetCode_break() && new_feat->GetData().GetCdregion().GetCode_break().empty())
                new_feat->SetData().SetCdregion().ResetCode_break();     
        }

        if (new_feat->IsSetData() && new_feat->GetData().IsRna() && new_feat->GetData().GetRna().IsSetExt() && 
            new_feat->GetData().GetRna().GetExt().IsTRNA() && new_feat->GetData().GetRna().GetExt().GetTRNA().IsSetAnticodon())
        {
            if ( x_AdjustLocation(new_feat->SetData().SetRna().SetExt().SetTRNA().SetAnticodon(), length,from,to))
            {
                new_feat->SetData().SetRna().SetExt().SetTRNA().ResetAnticodon();
            }
        }

        if (new_feat->IsSetData() && new_feat->GetData().IsClone() && new_feat->GetData().GetClone().IsSetClone_seq())
        {
            CClone_seq_set &clone_set = new_feat->SetData().SetClone().SetClone_seq();
            if (clone_set.IsSet())
            {
                for (CClone_seq_set::Tdata::iterator clone_it = clone_set.Set().begin(); clone_it != clone_set.Set().end(); ++clone_it)
                {
                        if ((*clone_it)->IsSetLocation())
                        {
                            if ( x_AdjustLocation((*clone_it)->SetLocation(), length,from,to))
                            {
                                (*clone_it)->ResetLocation();
                            }
                        }
                        if ((*clone_it)->IsSetSeq())
                        {
                            if ( x_AdjustLocation((*clone_it)->SetSeq(),length,from,to))
                            {
                                (*clone_it)->ResetSeq();
                            }
                        }
                }
            }
        }

        if (new_feat->IsSetData() && new_feat->GetData().IsVariation() )
        {
            CVariation_ref &var = new_feat->SetData().SetVariation();
            x_UpdateVariation(var, length,from,to);
        }
       

        command->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(*feat_ci, *new_feat)));
        
        if (feat_ci->IsSetProduct() && feat_ci->GetData().IsCdregion()) 
        {
            x_RetranslateCDS(scope,  command, new_feat, feat_ci->GetOriginalFeature());
        }
          
    }
}

void  CReportTrim::x_UpdateVariation(CVariation_ref &var, int length,  int from, int to)
{
    if (var.IsSetData() && var.GetData().IsInstance() && var.GetData().GetInstance().IsSetDelta())
    {
        CVariation_inst::TDelta::iterator delta_it = var.SetData().SetInstance().SetDelta().begin();
        while ( delta_it != var.SetData().SetInstance().SetDelta().end())
        {
            bool to_delete = false;
            if ((*delta_it)->IsSetSeq() && (*delta_it)->GetSeq().IsLoc() && x_AdjustLocation((*delta_it)->SetSeq().SetLoc(), length,from,to))
            {
                delta_it = var.SetData().SetInstance().SetDelta().erase(delta_it);
                continue;
            }
            ++delta_it;
        }
        if (var.GetData().GetInstance().GetDelta().empty())
            var.SetData().SetInstance().ResetDelta();
    }
    if (var.IsSetData() && var.GetData().IsSet() && var.GetData().GetSet().IsSetVariations())
    {
        for ( CVariation_ref::C_Data::C_Set::TVariations::iterator var_it = var.SetData().SetSet().SetVariations().begin(); var_it != var.SetData().SetSet().SetVariations().end(); ++var_it)
        {
            x_UpdateVariation(**var_it, length,from,to);
        }
    }
    if (var.IsSetConsequence())
    {
        for (CVariation_ref::TConsequence::iterator con_it = var.SetConsequence().begin(); con_it != var.SetConsequence().end(); ++con_it)
        {
            if ((*con_it)->IsVariation())
            {
                x_UpdateVariation((*con_it)->SetVariation(), length,from,to);
            }
        }
    }
}

bool CReportTrim::x_IsProtLocation(const CSeq_loc& loc, CBioseq_Handle bsh, CScope &scope)
{
    CBioseq_Handle prot_bsh = scope.GetBioseqHandle(loc);
    if (prot_bsh == bsh)
        return false;
    return prot_bsh.IsAa();
}


void CReportTrim::x_AdjustInt(int &begin, int &end, int from, int to, int length, int &diff, ENa_strand strand)
{
    if (begin > from)
    {
        begin -= from;
    }
    else 
    {
        if ( strand != eNa_strand_minus )
            diff += from - begin;
        begin = 0;
    }
    
    if (end > to)
        end = to;
    
    end -= from;
    
    if (end > length - 1)
    {
        if ( strand == eNa_strand_minus )
            diff += end - (length - 1);
        end = length - 1;
    }
}

bool CReportTrim::x_AdjustLocation(CSeq_loc & loc, int length,  int from, int to)
{
    int diff = 0;
    return x_AdjustLocation(loc, length, from, to, diff);
}

bool CReportTrim::x_AdjustLocation(CSeq_loc & loc, int length,  int from, int to, int &diff)
{
    ENa_strand strand = loc.GetStrand(); 

    switch (loc.Which())
    {

    case CSeq_loc::e_Int :
        {
            CSeq_interval &interval = loc.SetInt();
            int begin = interval.GetFrom();
            int end = interval.GetTo();
            if (end < from || begin > to)
                return true;

            x_AdjustInt(begin, end, from, to, length, diff, strand);
            interval.SetFrom(begin);
            interval.SetTo(end);
        }
        break;
    case CSeq_loc::e_Pnt :
        {
            CSeq_point &pnt = loc.SetPnt();
            int point = pnt.GetPoint();
            if (point < from || point > to)
                return true;

            pnt.SetPoint(point - from);
        }
        break;
    case CSeq_loc::e_Packed_int :
       {
            if (loc.SetPacked_int().IsSet())
            {
                vector<CPacked_seqint::Tdata::iterator> del;
                for (CPacked_seqint::Tdata::iterator p = loc.SetPacked_int().Set().begin(); p != loc.SetPacked_int().Set().end(); ++p)
                {
                    CSeq_interval &interval = **p;
                    int begin = interval.GetFrom();
                    int end = interval.GetTo();
                    if (end < from || begin > to)
                    {
                        if ( strand == eNa_strand_minus && begin > to)
                            diff += end-begin;
                        else if (strand != eNa_strand_minus && end < from)
                            diff += end-begin;
                        del.push_back(p);
                        continue;
                    }
                    x_AdjustInt(begin, end, from, to, length, diff, strand);
                    interval.SetFrom(begin);
                    interval.SetTo(end);                    
                }
                for (unsigned int i=0; i<del.size(); i++)
                {
                    CPacked_seqint::Tdata::iterator p = del[i];
                    loc.SetPacked_int().Set().erase(p);
                }   
                if (loc.SetPacked_int().Set().empty()) 
                    return true;
            }
       }
       break;
    case CSeq_loc::e_Packed_pnt :
        {
            if (loc.SetPacked_pnt().IsSetPoints())
            {
                vector<CPacked_seqpnt::TPoints::iterator> del;
                for (CPacked_seqpnt::TPoints::iterator p = loc.SetPacked_pnt().SetPoints().begin(); p != loc.SetPacked_pnt().SetPoints().end(); ++p)
                {
                    int point = *p;
                    if (point < from || point > to)
                    {
                        if ( strand == eNa_strand_minus && point > to)
                            diff++;
                        else if (strand != eNa_strand_minus && point < from)
                            diff++;
                        del.push_back(p);
                        continue;        
                    }            
                    *p  = point - from;
                }
                for (unsigned int i=0; i<del.size(); i++)
                {
                    CPacked_seqpnt::TPoints::iterator p = del[i];
                    loc.SetPacked_pnt().SetPoints().erase(p);
                }   
                if (loc.SetPacked_pnt().SetPoints().empty()) 
                    return true;
            }
        }
        break;
    case CSeq_loc::e_Mix :
        {
            if (loc.SetMix().IsSet())
            {
                vector<CSeq_loc_mix::Tdata::iterator> del;
                for (CSeq_loc_mix::Tdata::iterator p = loc.SetMix().Set().begin(); p != loc.SetMix().Set().end(); ++p)
                {
                    CSeq_loc &loc2 = **p;
                    if (x_AdjustLocation(loc2, length, from,to,diff))
                        del.push_back(p);
                }
                for (unsigned int i=0; i<del.size(); i++)
                {
                    CSeq_loc_mix::Tdata::iterator p = del[i];
                    loc.SetMix().Set().erase(p);
                }   
                if (loc.SetMix().Set().empty()) 
                    return true;
            }
       }
       break;
    case CSeq_loc::e_Equiv :
        {
            if (loc.SetEquiv().IsSet())
            {
                vector<CSeq_loc_equiv::Tdata::iterator> del;
                for (CSeq_loc_equiv::Tdata::iterator p = loc.SetEquiv().Set().begin(); p != loc.SetEquiv().Set().end(); ++p)
                {
                    CSeq_loc &loc2 = **p;
                    int orig_diff = diff;
                    if (x_AdjustLocation(loc2, length, from,to,orig_diff))
                        del.push_back(p);
                    if (p == loc.SetEquiv().Set().begin())
                        diff = orig_diff;
                }
                for (unsigned int i=0; i<del.size(); i++)
                {
                    CSeq_loc_equiv::Tdata::iterator p = del[i];
                    loc.SetEquiv().Set().erase(p);
                }   
                if (loc.SetEquiv().Set().empty()) 
                    return true;
            }
        }
        break;
    default : break;
    }

    return false;
}

void CReportTrim::x_AdjustCdregionFrame(CRef<CSeq_feat> feat, int diff )
{
      // adjust frame to maintain consistency
    if (feat->GetData().IsCdregion()) {
        int orig_frame = 0;
        if (feat->GetData().GetCdregion().IsSetFrame()) {
            if (feat->GetData().GetCdregion().GetFrame() == CCdregion::eFrame_two) {
                orig_frame = 1;
            } else if (feat->GetData().GetCdregion().GetFrame() == CCdregion::eFrame_three) {
                orig_frame = 2;
            }
        }
        int new_offset = (orig_frame - diff) % 3;
        if (orig_frame < diff && new_offset != 0)
        {
            new_offset = 3 - ((diff - orig_frame) % 3);
        }
            
        CCdregion::EFrame new_frame = CCdregion::eFrame_not_set;
        switch (new_offset) {
        case 0:
            new_frame = CCdregion::eFrame_one;
            break;
        case 1:
            new_frame = CCdregion::eFrame_two;
            break;
        case 2:
            new_frame = CCdregion::eFrame_three;
            break;
        default:
            new_frame = CCdregion::eFrame_not_set;
            break;
        }
        feat->SetData().SetCdregion().SetFrame(new_frame);
    }
}

void CReportTrim::x_RetranslateCDS(CScope& scope, CRef<CCmdComposite> command, CRef<CSeq_feat> cds, const CSeq_feat &old_cds)
{
    // Assumption:  cds has been verified to be Cdregion with Product

    // Use Cdregion.Product to get handle to protein bioseq 
    CBioseq_Handle prot_bsh = scope.GetBioseqHandle(cds->GetProduct());

    // Should be a protein!
    if (!prot_bsh.IsProtein())
    {
        return;
    }

    string prot;
    try
    {
        CSeqTranslator::Translate(*cds, scope, prot);
    }
    catch(CSeqMapException&) {}
    if (!prot.empty())
    {
        if (NStr::EndsWith(prot, "*")) 
        {
            prot = prot.substr(0, prot.length() - 1);
        }
        // Make a copy of existing CSeq_inst
        CRef<objects::CSeq_inst> new_inst(new objects::CSeq_inst());
        new_inst->Assign(prot_bsh.GetInst());
        new_inst->ResetExt();
        new_inst->SetRepr(objects::CSeq_inst::eRepr_raw); 
        new_inst->SetSeq_data().SetNcbieaa().Set(prot);
        new_inst->SetLength(TSeqPos(prot.length()));
        new_inst->SetMol(CSeq_inst::eMol_aa);
        // Update protein sequence data and length
        CRef<CCmdChangeBioseqInst> chgInst (new CCmdChangeBioseqInst(prot_bsh, *new_inst));
        command->AddCommand(*chgInst);

        CSeq_loc_Mapper map_to_nuc(old_cds, CSeq_loc_Mapper::eProductToLocation, &scope);
        map_to_nuc.SetMergeAbutting();
        CSeq_loc_Mapper map_to_prot(*cds, CSeq_loc_Mapper::eLocationToProduct, &scope);
        map_to_prot.SetMergeAbutting();

        CFeat_CI prot_feat_ci(prot_bsh);
        for ( ; prot_feat_ci; ++prot_feat_ci ) 
        {
            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(prot_feat_ci->GetOriginalFeature());
            if (prot_feat_ci->GetFeatSubtype() == CSeqFeatData::eSubtype_prot)
            {
                new_feat->SetLocation().SetInt().SetFrom(0);
                new_feat->SetLocation().SetInt().SetTo(new_inst->GetLength() - 1);
                CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(*prot_feat_ci, *new_feat));
                command->AddCommand(*chgFeat);
            }
            else
            {
                CSeq_feat_Handle fh = prot_feat_ci->GetSeq_feat_Handle();
                const CSeq_loc &loc = prot_feat_ci->GetLocation();
                CRef<CSeq_loc> nuc_loc = map_to_nuc.Map(loc);
                CRef<CSeq_loc> prot_loc = map_to_prot.Map(*nuc_loc);
               
                if (prot_loc->IsNull())
                {
                    CIRef<IEditCommand> delFeat(new CCmdDelSeq_feat(fh));
                    command->AddCommand(*delFeat);
                }
                else
                {
                    prot_loc->SetPartialStart(loc.IsPartialStart(eExtreme_Biological), eExtreme_Biological);
                    prot_loc->SetPartialStop(loc.IsPartialStop(eExtreme_Biological), eExtreme_Biological);
                    new_feat->SetLocation().Assign(*prot_loc);
                    new_feat->SetPartial(new_feat->GetLocation().IsPartialStart(eExtreme_Positional) || new_feat->GetLocation().IsPartialStop(eExtreme_Positional));
            
                    CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_feat));
                    command->AddCommand(*chgFeat);
                }
            }
        }
    }
}

void CReportTrim::TrimAlignments(CRef<CCmdComposite> command, CBioseq_Handle bsh, const CBioseq::TId &seqids,  TSeqPos from, TSeqPos to)
{
    // Adjust alignments
    SAnnotSelector align_sel(CSeq_annot::C_Data::e_Align);
    CAlign_CI align_ci(bsh, align_sel);
    for (; align_ci; ++align_ci)
    {
        // So far, handle DENSEG type only
        const CSeq_align& align = *align_ci;
        if ( align.CanGetSegs() && 
             align.GetSegs().Which() == CSeq_align::C_Segs::e_Denseg )
        {
            // Make sure mandatory fields are present in the denseg
            const CDense_seg& denseg = align.GetSegs().GetDenseg();
            if (! (denseg.CanGetDim() && denseg.CanGetNumseg() && 
                   denseg.CanGetIds() && denseg.CanGetStarts() &&
                   denseg.CanGetLens()) )
            {
                continue;
            }
            
            // Make a copy of the alignment
            CRef<CSeq_align> new_align(new CSeq_align());
            new_align->Assign(align_ci.GetOriginalSeq_align());

            // Make edits to the copy

            // On which "row" does the seqid lie?
            const CDense_seg::TIds& ids = denseg.GetIds();
            CDense_seg::TDim row = -1;
            for (CBioseq::TId::const_iterator seqid = seqids.begin(); seqid != seqids.end(); ++seqid)
                for (unsigned int ii = 0; ii < ids.size(); ++ii) {
                    if ( ids[ii]->Match(**seqid) ) {
                        row = ii;
                        break;
                    }
                }
            if ( row < 0 || !denseg.CanGetDim() || row >= denseg.GetDim() ) {
                continue;
            }
            
            x_AdjustDensegAlignment(new_align, row, from, to);
            
            
            // Swap edited copy with the original alignment
            CIRef<IEditCommand> chgAlign(  new CCmdChangeAlign( align_ci.GetSeq_align_Handle(), *new_align ));
            command->AddCommand(*chgAlign);
            
        }
        else {
            // For other alignment types, delete them.  This is what
            // C Toolkit does (see AdjustAlignmentsInAnnot)
            CIRef<IEditCommand> delAlign(
                new CCmdDelSeq_align( align_ci.GetSeq_align_Handle() ));
            command->AddCommand(*delAlign);
            }
    }
}

void CReportTrim::x_AdjustDensegAlignment(CRef<CSeq_align> align, 
                                     CDense_seg::TDim row, 
                                     TSeqPos cut_from,
                                     TSeqPos cut_to)
{
    TSeqPos cut_len = cut_to - cut_from + 1;
    if (cut_to < cut_from) {
        cut_len = cut_from - cut_to + 1;
        cut_from = cut_to;
    } 

    // Note: row is 0-based
    
    // May need to cut the segment at both start and stop positions
    // if they do not fall on segment boundaries
    x_CutDensegSegment(align, row, cut_from);
    x_CutDensegSegment(align, row, cut_from + cut_len);

    // Update segment start values for the trimmed sequence row
    const CDense_seg& denseg = align->GetSegs().GetDenseg();
    for (CDense_seg::TNumseg curseg = 0; curseg < denseg.GetNumseg(); ++curseg) {
        TSeqPos index = curseg * denseg.GetDim() + row;
        TSeqPos seg_start = denseg.GetStarts()[index];
        if (seg_start < 0) {
            // This indicates a gap, no change needed
        }
        else if (seg_start < cut_from) {
            // This is before the cut, no change needed
        }
        else if (seg_start >= cut_from && 
                 seg_start + denseg.GetLens()[curseg] <= cut_from + cut_len) {
            // This is in the gap, indicate it with a -1
            align->SetSegs().SetDenseg().SetStarts()[index] = -1;
        }
        else {
            // This is after the cut - subtract the cut_len
            align->SetSegs().SetDenseg().SetStarts()[index] -= cut_len;
        }
    }
    
}

void CReportTrim::x_CutDensegSegment(CRef<CSeq_align> align, 
                                CDense_seg::TDim row,
                                TSeqPos pos)
{
    // Find the segment where pos occurs for the sequence (identified by 
    // row).
    // If pos is not the start of the segment, cut the segment in two, with 
    // one of the segments using pos as the new start.


    // Find the segment where pos lies
    const CDense_seg& denseg = align->GetSegs().GetDenseg();
    CDense_seg::TNumseg foundseg; 
    TSeqPos seg_start;
    if ( !x_FindSegment(denseg, row, pos, foundseg, seg_start) ) {
        return;
    }

    // Found our segment seg
    // If pos falls on segment boundary, do nothing
    if (pos == seg_start) {
        return;
    }


    // Cut the segment :
    // 1) Allocate a new denseg with numseg size = original size + 1
    // 2) Copy elements before the cut
    // 3) Split segment at pos
    // 4) Copy elements after the cut
    // 5) Replace old denseg with new denseg

    // Allocate a new denseg with numseg size = original size + 1
    CRef<CDense_seg> new_denseg(new CDense_seg);    
    new_denseg->SetDim( denseg.GetDim() );
    new_denseg->SetNumseg( denseg.GetNumseg() + 1 );
    ITERATE( CDense_seg::TIds, idI, denseg.GetIds() ) {
        CSeq_id *si = new CSeq_id;
        si->Assign(**idI);
        new_denseg->SetIds().push_back( CRef<CSeq_id>(si) );
    }

    // Copy elements (starts, lens, strands) before the cut (up to and including
    // foundseg-1 in original denseg)
    for (CDense_seg::TNumseg curseg = 0; curseg < foundseg; ++curseg) {
        // Copy starts
        for (CDense_seg::TDim curdim = 0; curdim < denseg.GetDim(); ++curdim) {
            TSeqPos index = curseg * denseg.GetDim() + curdim;
            new_denseg->SetStarts().push_back( denseg.GetStarts()[index] );
        }

        // Copy lens
        new_denseg->SetLens().push_back( denseg.GetLens()[curseg] );

        // Copy strands
        if ( denseg.IsSetStrands() ) {
            for (CDense_seg::TDim curdim = 0; curdim < denseg.GetDim(); 
                 ++curdim) 
            {
                TSeqPos index = curseg * denseg.GetDim() + curdim;
                new_denseg->SetStrands().push_back(denseg.GetStrands()[index]);
            }
        }
    }

    // Split segment at pos
    // First find the lengths of the split segments, first_len and second_len
    TSeqPos first_len, second_len;
    TSeqPos index = foundseg * denseg.GetDim() + row;
    if ( !denseg.IsSetStrands() || denseg.GetStrands()[index] != eNa_strand_minus )
    {
        first_len  = pos - seg_start;
        second_len = denseg.GetLens()[foundseg] - first_len;
    } 
    else {
        second_len = pos - seg_start;
        first_len  = denseg.GetLens()[foundseg] - second_len;
    }   

    // Set starts, strands, and lens for the split segments (foundseg and foundseg+1)
    // Populate foundseg in new denseg
    for (CDense_seg::TDim curdim = 0; curdim < denseg.GetDim(); ++curdim) {
        TSeqPos index = foundseg * denseg.GetDim() + curdim;
        if (denseg.GetStarts()[index] == -1) {
            new_denseg->SetStarts().push_back(-1);
        }
        else if (!denseg.IsSetStrands() || denseg.GetStrands()[index] != eNa_strand_minus) {
            new_denseg->SetStarts().push_back(denseg.GetStarts()[index]);
        }
        else {
            new_denseg->SetStarts().push_back(denseg.GetStarts()[index] + second_len);
        }

        if (denseg.IsSetStrands()) {
            new_denseg->SetStrands().push_back(denseg.GetStrands()[index]);
        }
    }    
    new_denseg->SetLens().push_back(first_len);
    // Populate foundseg+1 in new denseg
    for (CDense_seg::TDim curdim = 0; curdim < denseg.GetDim(); ++curdim) {
        TSeqPos index = foundseg * denseg.GetDim() + curdim;
        if (denseg.GetStarts()[index] == -1) {
            new_denseg->SetStarts().push_back(-1);
        }
        else if (!denseg.IsSetStrands() || denseg.GetStrands()[index] != eNa_strand_minus) {
            new_denseg->SetStarts().push_back(denseg.GetStarts()[index] + first_len);
        }
        else {
            new_denseg->SetStarts().push_back(denseg.GetStarts()[index]);
        }

        if (denseg.IsSetStrands()) {
            new_denseg->SetStrands().push_back(denseg.GetStrands()[index]);
        }
    }    
    new_denseg->SetLens().push_back(second_len);

    // Copy elements (starts, lens, strands) after the cut (starting from foundseg+1 in 
    // original denseg)
    for (CDense_seg::TNumseg curseg = foundseg+1; curseg < denseg.GetNumseg(); ++curseg) {
        // Copy starts
        for (CDense_seg::TDim curdim = 0; curdim < denseg.GetDim(); ++curdim) {
            TSeqPos index = curseg * denseg.GetDim() + curdim;
            new_denseg->SetStarts().push_back( denseg.GetStarts()[index] );
        }

        // Copy lens
        new_denseg->SetLens().push_back( denseg.GetLens()[curseg] );

        // Copy strands
        if ( denseg.IsSetStrands() ) {
            for (CDense_seg::TDim curdim = 0; curdim < denseg.GetDim(); 
                 ++curdim) 
            {
                TSeqPos index = curseg * denseg.GetDim() + curdim;
                new_denseg->SetStrands().push_back(denseg.GetStrands()[index]);
            }
        }
    }

    // Update 
    align->SetSegs().SetDenseg(*new_denseg);
}

bool CReportTrim::x_FindSegment(const CDense_seg& denseg,
                           CDense_seg::TDim row,
                           TSeqPos pos, 
                           CDense_seg::TNumseg& seg,
                           TSeqPos& seg_start) const
{
    for (seg = 0; seg < denseg.GetNumseg(); ++seg) {
        TSignedSeqPos start = denseg.GetStarts()[seg * denseg.GetDim() + row];
        TSignedSeqPos len   = denseg.GetLens()[seg];
        if (start != -1) {
            if (pos >= start  &&  pos < start + len) {
                seg_start = start;
                return true;
            }
        }
    }
    return false;
}


bool CReportTrim::x_UpdateSeqGraphLoc(CRef<CSeq_graph> new_graph, 
                                 TSeqPos from, TSeqPos to,
                                 int length,
                                 const CRef<CSeq_id> seqid)
{
    bool result = false;
    if (new_graph->CanGetLoc())
    {
        CRef<CSeq_graph::TLoc> new_loc(new CSeq_graph::TLoc);
        new_loc->Assign(new_graph->GetLoc());
        result =  x_AdjustLocation(*new_loc,length,from,to);      
        new_graph->SetLoc(*new_loc);
    }   
    return result;
}

bool CReportTrim::x_TrimSeqGraphData(CRef<CSeq_graph> new_graph, 
                                const CMappedGraph& orig_graph,
                                TSeqPos trim_start, TSeqPos trim_stop,
                                int length,
                                const CRef<CSeq_id> seqid)
{
    // Get range that original seqgraph data covers
    TSeqPos graph_start = new_graph->GetLoc().GetStart(eExtreme_Positional);
    TSeqPos graph_stop = new_graph->GetLoc().GetStop(eExtreme_Positional);

    // Determine range over which to copy seqgraph data from old to new
    TSeqPos copy_start = graph_start;
    if (trim_start > graph_start) {
        copy_start = trim_start;
    }
    TSeqPos copy_stop = graph_stop;
    if (trim_stop < graph_stop) {
        copy_stop = trim_stop;
    }

    // Copy over seqgraph data values.  Handle BYTE type only (see 
    // C Toolkit's GetGraphsProc function in api/sqnutil2.c)
    CSeq_graph::TGraph& dst_data = new_graph->SetGraph();
    dst_data.Reset();
    const CSeq_graph::TGraph& src_data = orig_graph.GetGraph();
    switch ( src_data.Which() ) {
    case CSeq_graph::TGraph::e_Byte:
        // Keep original min, max, axis
        dst_data.SetByte().SetMin(src_data.GetByte().GetMin());
        dst_data.SetByte().SetMax(src_data.GetByte().GetMax());
        dst_data.SetByte().SetAxis(src_data.GetByte().GetAxis());

        // Copy start/stop values are relative to bioseq coordinate system.
        // Change them so that they are relative to graph location.
        copy_start -= graph_start;
        copy_stop -= graph_start;

        // Update data values
        dst_data.SetByte().SetValues();
        dst_data.SetByte().SetValues().insert(
            dst_data.SetByte().SetValues().end(), 
            src_data.GetByte().GetValues().begin() + copy_start,
            src_data.GetByte().GetValues().begin() + copy_stop + 1);

        // Update numvals
        new_graph->SetNumval(copy_stop - copy_start + 1);

        // Update seqloc
        return x_UpdateSeqGraphLoc(new_graph, trim_start, trim_stop, length, seqid);
        break;
    default: break;
    }
    return false;
}


void CReportTrim::TrimSeqGraphs(CRef<CCmdComposite> command, CBioseq_Handle bsh, const CBioseq::TId &seqids,  TSeqPos from, TSeqPos to, int length)
{
    SAnnotSelector graph_sel(CSeq_annot::C_Data::e_Graph);
    CGraph_CI graph_ci(bsh, graph_sel);
    for (; graph_ci; ++graph_ci)
    {
        // Only certain types of graphs are supported.
        // See C Toolkit function GetGraphsProc in api/sqnutil2.c
        const CMappedGraph& graph = *graph_ci;
        if ( graph.IsSetTitle() && 
             (NStr::CompareNocase( graph.GetTitle(), "Phrap Quality" ) == 0 ||
              NStr::CompareNocase( graph.GetTitle(), "Phred Quality" ) == 0 ||
              NStr::CompareNocase( graph.GetTitle(), "Gap4" ) == 0) )
        {
            // Make a copy of the graph
            CRef<CSeq_graph> new_graph(new CSeq_graph());
            new_graph->Assign(graph.GetOriginalGraph());

            // Make edits to the copy
            for (CBioseq::TId::const_iterator seqid = seqids.begin(); seqid != seqids.end(); ++seqid)
            {
                // Find matching seqid referred to by the graph
                if ( graph.GetLoc().GetId()->Match(**seqid) ) {
                    if (x_TrimSeqGraphData(new_graph, graph, from, to, length, *seqid))
                    {
                        CIRef<IEditCommand> delGraph(new CCmdDelSeq_graph( graph.GetSeq_graph_Handle() )); 
                        command->AddCommand(*delGraph);
                    }                
                    else
                    {
                        // Swap edited copy with the original graph
                        CIRef<IEditCommand> chgGraph( new CCmdChangeGraph( graph.GetSeq_graph_Handle(), *new_graph ));
                        command->AddCommand(*chgGraph);
                    }
                    break;
                }
            }
        }
    }
}

CSequenceAmbigTrimmer::EResult CReportTrim::Trim( CBioseq_Handle &bioseq_handle, TSignedSeqPos &left, TSignedSeqPos &right)
{
    _ASSERT( bioseq_handle );

    const CSeqVector seqvec( bioseq_handle, CBioseq_Handle::eCoding_Iupac );

    // there's already no sequence, so nothing to trim
    const TSignedSeqPos bioseq_len = bioseq_handle.GetBioseqLength();
    if( bioseq_len < 1 ) {
        return eResult_NoTrimNeeded;
    }

    TSignedSeqPos leftmost_good_base = 0;
    TSignedSeqPos rightmost_good_base = bioseq_len - 1;
    if( ! x_TestFlag(fFlags_DoNotTrimBeginning) ) {
        leftmost_good_base = x_FindWhereToTrim( 
            seqvec, leftmost_good_base, rightmost_good_base, 
            1 ); // 1 means "towards the right"
    }
 
    if( leftmost_good_base > rightmost_good_base) {
        if (x_IsThereGapNotAtTheEnd(bioseq_handle, 0, bioseq_len -1, bioseq_len))
            return eResult_NoTrimNeeded;
        // trimming leaves nothing left
        return x_TrimToNothing( bioseq_handle );
    }
    
    if (x_IsThereGapNotAtTheEnd(bioseq_handle, leftmost_good_base, bioseq_len -1, bioseq_len))
    {
        leftmost_good_base = 0;
    }

    if( ! x_TestFlag(fFlags_DoNotTrimEnd) ) {
        rightmost_good_base = 
            x_FindWhereToTrim( 
            seqvec, rightmost_good_base, leftmost_good_base, 
            -1 ); // -1 means "towards the left"
    }

    if( leftmost_good_base > rightmost_good_base) {
        if (x_IsThereGapNotAtTheEnd(bioseq_handle, 0, bioseq_len -1, bioseq_len))
            return eResult_NoTrimNeeded;
        // trimming leaves nothing left
        return x_TrimToNothing( bioseq_handle );
    }

    if (x_IsThereGapNotAtTheEnd(bioseq_handle, 0, rightmost_good_base, bioseq_len))
    {
        rightmost_good_base = bioseq_len -1;
    }

    // check if nothing to do
    if( (leftmost_good_base == 0) &&
        (rightmost_good_base == (bioseq_len - 1)) )
    {
        return eResult_NoTrimNeeded;
    }


    // do the actually slicing of the bioseq
    x_SliceBioseq( 
        leftmost_good_base, rightmost_good_base,
        bioseq_handle );

    left = leftmost_good_base;
    right = rightmost_good_base;
    return eResult_SuccessfullyTrimmed;
}

bool CReportTrim::x_IsThereGapNotAtTheEnd(CBioseq_Handle bsh, TSignedSeqPos from, TSignedSeqPos to, int length)
{
    CSeqVector sv = bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac);
    if (from == 0 && to == length - 1)
    {
        bool gap_inside = false;
        if (!sv.IsInGap(0) && !sv.IsInGap(length - 1))
            for (TSeqPos pos = 1; pos < length - 1; pos++) 
                if (sv.IsInGap (pos)) 
                    gap_inside = true;
        return gap_inside;
    }
    
    bool gap_left = false;
    if (!sv.IsInGap(0))
    {
        for (TSeqPos pos = 1; pos < from; pos++) 
            if (sv.IsInGap (pos)) 
                gap_left = true;
    }
    bool gap_right = false;
    if (!sv.IsInGap(length - 1))
    {
        for (TSeqPos pos = to + 1; pos < length - 1; pos++)
            if (sv.IsInGap (pos)) 
                gap_right = true;
    }
    return gap_left || gap_right;
}

END_NCBI_SCOPE
