/*  $Id: convert_selected_gaps.cpp 43470 2019-07-11 14:54:19Z filippov $
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
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/Feat_id.hpp>
#include <objtools/cleanup/fix_feature_id.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/cmd_change_align.hpp>
#include <gui/objutils/objects.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <objects/seq/Seq_ext.hpp>
#include <objects/seq/Delta_ext.hpp>
#include <objects/seq/Delta_seq.hpp>
#include <objects/seq/Seq_literal.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objmgr/align_ci.hpp>
#include <objects/general/Int_fuzz.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <gui/objutils/gap_edit.hpp>
#include <objmgr/seq_vector.hpp>
#include <gui/objutils/convert_raw_to_delta.hpp>
#include <gui/packages/pkg_sequence_edit/convert_selected_gaps.hpp>

#include <wx/textdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

CRef<CCmdComposite> CConvertSelectedGaps::apply(const TConstScopedObjects &objects, bool to_known, bool adjust_features)                                                     
{
    map<CObject_id::TId, CObject_id::TId> old_to_new;
    bool create_xref_map = true;
    apply_impl(objects, to_known, adjust_features, old_to_new, create_xref_map);
    create_xref_map = false;
    return apply_impl(objects, to_known, adjust_features, old_to_new, create_xref_map);
}

CRef<CCmdComposite> CConvertSelectedGaps::apply_impl(const TConstScopedObjects &objects, bool to_known, bool adjust_features,
                                                     map<CObject_id::TId, CObject_id::TId> &old_to_new, bool create_xref_map)
{
    CRef<CCmdComposite> composite( new CCmdComposite("Convert Selected Gaps") );   
    map<CBioseq_Handle, pair<CRef<CSeq_inst>, bool> > bsh_to_inst;
    bool create_general_only = false;
    CObject_id::TId max_feat_id = 1;
    bool first = true;
    ITERATE (TConstScopedObjects, it, objects) 
    {
        CBioseq_Handle bsh;
        const CGapEdit* gap_edit = dynamic_cast<const CGapEdit*>((*it).object.GetPointer());
        if (gap_edit)
        {
            bsh = gap_edit->GetBioseq_Handle();
        }
        const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>((*it).object.GetPointer());
        if (feat && feat->IsSetData() && feat->GetData().IsImp() && feat->GetData().GetImp().IsSetKey() && feat->GetData().GetImp().GetKey() == "gap" && feat->IsSetLocation())
        {
            CRef<objects::CScope> scope = it->scope;
            bsh = scope->GetBioseqHandle(feat->GetLocation());
        }

        if (!bsh)
            continue;
        if (bsh_to_inst.find(bsh) != bsh_to_inst.end())
            continue;
        if (!bsh.IsSetInst())
            continue;
        if (first)
        {
            CSeq_entry_Handle seh = bsh.GetTopLevelEntry();
            max_feat_id = CFixFeatureId::s_FindHighestFeatureId(seh);
            create_general_only = objects::edit::IsGeneralIdProtPresent(seh);
            first = false;
        }
        const CSeq_inst &inst = bsh.GetInst();
        if (!inst.IsSetExt() || !inst.GetExt().IsDelta())
            continue;

        CRef<CSeq_inst> new_inst(new CSeq_inst);
        new_inst->Assign(inst);
        bsh_to_inst[bsh] = make_pair(new_inst, false);
    }
    map<CBioseq_Handle, map<TSeqPos, int> > diffs;
    map<CBioseq_Handle, vector<NRawToDeltaSeq::SGap> > gaps;

    ITERATE (TConstScopedObjects, it, objects) 
    {
        CRef<CScope> scope = it->scope;
        CBioseq_Handle bsh;
        TSeqPos from;
        const CGapEdit* gap_edit = dynamic_cast<const CGapEdit*>((*it).object.GetPointer());
        if (gap_edit)
        {
            bsh = gap_edit->GetBioseq_Handle();
            from = gap_edit->GetFrom();
        }
        const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>((*it).object.GetPointer());
        if (feat && feat->IsSetData() && feat->GetData().IsImp() && feat->GetData().GetImp().IsSetKey() && feat->GetData().GetImp().GetKey() == "gap" && feat->IsSetLocation())
        {
            bsh = scope->GetBioseqHandle(feat->GetLocation());
            from = feat->GetLocation().GetStart(eExtreme_Positional) + 1;
        }

        if (!bsh)
            continue;

        for (const auto& d : diffs[bsh])
            if (d.first < from)
            {
                from += d.second;
            }
        
        CRef<CSeq_inst> new_inst = bsh_to_inst[bsh].first;        
        TSeqPos start = 1;
        NON_CONST_ITERATE(CDelta_ext::Tdata, delta_i, new_inst->SetExt().SetDelta().Set()) 
        {
            if (delta_i->Empty()) 
                continue; 
            CDelta_seq& seg = **delta_i;
   
            size_t delta_len = 0;
            if (seg.IsLiteral()) 
            {
                delta_len =  seg.GetLiteral().GetLength();
            } 
            else if (seg.IsLoc()) 
            { 
                delta_len = sequence::GetLength (seg.GetLoc(), scope.GetPointer());
            }
            
            if (start == from)
            {
                if (to_known)
                    seg.SetLiteral().ResetFuzz();
                else
                {
                    seg.SetLiteral().SetFuzz().SetLim(CInt_fuzz::eLim_unk);
                    seg.SetLiteral().SetLength(100);
                    int diff = 100;
                    diff -= delta_len;
                    if (new_inst->IsSetLength())
                    {
                        diffs[bsh][start] = diff;
                        TSeqPos old_length = new_inst->GetLength();
                        new_inst->SetLength(old_length + diff);
                    }

                    {
                        NRawToDeltaSeq::SGap gap;
                        gap.start = start;
                        gap.length = delta_len;
                        gap.is_known = false;
                        gap.is_replace = true;
                        gaps[bsh].push_back(gap);
                    }
                }
                bsh_to_inst[bsh].second = true;
                break;
            }
            
            start += delta_len;
        }

        int diff = 0;
        for (CFeat_CI feat_ci(bsh, CSeqFeatData::eSubtype_gap); feat_ci; ++feat_ci)
        {
            const CSeq_feat& orig = feat_ci->GetOriginalFeature();
            start = orig.GetLocation().GetStart(eExtreme_Positional) + 1;
            if (start == from)
            {
                CRef<CSeq_feat> new_feat(new CSeq_feat());
                new_feat->Assign(orig);  
                TSeqPos delta_len = new_feat->GetLocation().GetTotalRange().GetLength();
                if (to_known)
                {
                    new_feat->AddOrReplaceQualifier("estimated_length", NStr::UIntToString(delta_len));
                }
                else
                {
                    new_feat->AddOrReplaceQualifier("estimated_length", "unknown");
                    diff = 100;
                    diff -= delta_len;
                    if (new_feat->GetLocation().IsInt())
                    {
                        new_feat->SetLocation().SetInt().SetTo(new_feat->GetLocation().GetInt().GetFrom() + 100 - 1);
                    }
                    if (!bsh_to_inst[bsh].second && new_inst->IsSetLength())
                    {
                        diffs[bsh][start] = diff;
                        TSeqPos old_length = new_inst->GetLength();
                        new_inst->SetLength(old_length + diff);
                        {
                            NRawToDeltaSeq::SGap gap;
                            gap.start = start;
                            gap.length = delta_len;
                            gap.is_known = false;
                            gap.is_replace = true;
                            gaps[bsh].push_back(gap);
                        }
                        bsh_to_inst[bsh].second = true;
                    }
                }
                composite->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
                break;
            }
        }
        if (diff != 0)
        {
            for (CFeat_CI feat_ci(bsh, CSeqFeatData::eSubtype_gap); feat_ci; ++feat_ci)
            {
                const CSeq_feat& orig = feat_ci->GetOriginalFeature();
                start = orig.GetLocation().GetStart(eExtreme_Positional) + 1;
                if (start > from)
                {
                    CRef<CSeq_feat> new_feat(new CSeq_feat());
                    new_feat->Assign(orig);  
                    if (new_feat->GetLocation().IsInt())
                    {
                        new_feat->SetLocation().SetInt().SetFrom(new_feat->GetLocation().GetInt().GetFrom() + diff);
                        new_feat->SetLocation().SetInt().SetTo(new_feat->GetLocation().GetInt().GetTo() + diff);
                        composite->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
                    }
                }
            }
        }
    }
    CRef<CCmdComposite> local_cmd( new CCmdComposite("Seq-inst change to facilitate retranslation") );  
    for (const auto& it : bsh_to_inst)
    {
        CBioseq_Handle bsh = it.first;
        CRef<CSeq_inst> new_inst = it.second.first;
        bool modified = it.second.second;      
        if (modified)
        {
            CRef<CCmdChangeBioseqInst> cmd(new CCmdChangeBioseqInst(bsh, *new_inst));
            local_cmd->AddCommand(*cmd);           
        }
    }
    composite->AddCommand(*local_cmd);
    if (adjust_features)
    {
        local_cmd->Execute();
        NRawToDeltaSeq::AdjustFeatureLocations(gaps, composite, false, false, false, create_general_only, max_feat_id, old_to_new, create_xref_map);
        local_cmd->Unexecute();
    }
    return composite;
}


CRef<CCmdComposite> CConvertGapsBySize::apply(objects::CSeq_entry_Handle seh)
{
    wxTextEntryDialog dlg(NULL, _("Convert gaps longer or equal to"), _("Convert Known Length Gaps to Unknown"), _("100"));
    dlg.SetTextValidator(wxFILTER_NUMERIC);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString val = dlg.GetValue();
        long min_size;
        if(val.ToLong(&min_size))
        {
            CScope &scope = seh.GetScope();
            TConstScopedObjects objects;
            CRef<CCmdComposite> composite( new CCmdComposite("Convert Gaps By Size") );   
            for (CBioseq_CI bi(seh, CSeq_inst::eMol_na); bi; ++bi)
            {
                CBioseq_Handle bsh = *bi;
                TSeqPos len = bsh.GetBioseqLength();
                if (len <= 1)
                    continue;
                if (!bsh.IsSetInst())
                    continue;

                const CSeq_inst &inst = bsh.GetInst();
                if (!inst.IsSetExt() || !inst.GetExt().IsDelta())
                    continue;
                bool has_loc(false);
                ITERATE(CDelta_ext::Tdata, iter, inst.GetExt().GetDelta().Get()) 
                {
                    if ( (*iter)->IsLoc() ) 
                    {
                        has_loc = true;
                        break;
                    }
                }
                if (has_loc)
                    continue;

                try 
                {
                    TSeqPos pos = 0;
                    CSeqVector sv = bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac);
                    ITERATE (CDelta_ext::Tdata, delta_i, inst.GetExt().GetDelta().Get()) 
                    {
                        if (delta_i->Empty()) 
                            continue; 
                        const CDelta_seq& seg = **delta_i;
                        size_t delta_len = seg.GetLiteral().GetLength();
                        if (sv.IsInGap (pos) && delta_len >= min_size && !(seg.GetLiteral().IsSetFuzz() && seg.GetLiteral().GetFuzz().IsLim() && seg.GetLiteral().GetFuzz().GetLim() == objects::CInt_fuzz::eLim_unk) ) 
                        {
                            {
                                CRef<CGapEdit> gap(new CGapEdit(bsh, pos + 1, delta_len));
                                SConstScopedObject obj(gap, &scope);
                                objects.push_back(obj);
                            }
                           
                        }               
                        pos += delta_len;
                    }
                    
                } catch (CException ) {} catch (std::exception ) {}

            }
            if (!objects.empty())
            {
                return  CConvertSelectedGaps::apply(objects, false, true);
            }
        }
    }
    return CRef<CCmdComposite>(NULL);
}


CRef<CCmdComposite> CEditSelectedGaps::apply(const TConstScopedObjects &objects)
{
    map<CObject_id::TId, CObject_id::TId> old_to_new;
    bool create_xref_map = true;
    apply_impl(objects, old_to_new, create_xref_map);
    create_xref_map = false;
    return apply_impl(objects, old_to_new, create_xref_map);
}

CRef<CCmdComposite> CEditSelectedGaps::apply_impl(const TConstScopedObjects &objects, map<CObject_id::TId, CObject_id::TId> &old_to_new, bool create_xref_map)
{
    CRef<CCmdComposite> composite;   

    wxTextEntryDialog dlg(NULL, _("Change length to"), _("Change length of selected gaps"), _("100"));
    dlg.SetTextValidator(wxFILTER_NUMERIC);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString val = dlg.GetValue();
        long new_size;
        if(val.ToLong(&new_size))
        {
            composite.Reset( new CCmdComposite("Edit Selected Gaps") );

            map<CBioseq_Handle, pair<CRef<CSeq_inst>, bool> > bsh_to_inst;
            bool create_general_only = false;
            bool first = true;
            CObject_id::TId max_feat_id = 1;
            ITERATE (TConstScopedObjects, it, objects) 
            {
                CBioseq_Handle bsh;
                const CGapEdit* gap_edit = dynamic_cast<const CGapEdit*>((*it).object.GetPointer());
                if (gap_edit)
                {
                    bsh = gap_edit->GetBioseq_Handle();
                }
                const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>((*it).object.GetPointer());
                if (feat && feat->IsSetData() && feat->GetData().IsImp() && feat->GetData().GetImp().IsSetKey() && feat->GetData().GetImp().GetKey() == "gap" && feat->IsSetLocation())
                {
                    CRef<objects::CScope> scope = it->scope;
                    bsh = scope->GetBioseqHandle(feat->GetLocation());
                }
                
                if (!bsh)
                    continue;
                if (bsh_to_inst.find(bsh) != bsh_to_inst.end())
                    continue;
                if (!bsh.IsSetInst())
                    continue;
                if (first)
                {
                    CSeq_entry_Handle seh = bsh.GetTopLevelEntry();
                    max_feat_id = CFixFeatureId::s_FindHighestFeatureId(seh);
                    create_general_only = objects::edit::IsGeneralIdProtPresent(seh);
                    first = false;
                }
                const CSeq_inst &inst = bsh.GetInst();
                if (!inst.IsSetExt() || !inst.GetExt().IsDelta())
                    continue;
                bool has_loc(false);
                ITERATE(CDelta_ext::Tdata, iter, inst.GetExt().GetDelta().Get()) 
                {
                    if ( (*iter)->IsLoc() ) 
                    {
                        has_loc = true;
                        break;
                    }
                }
                if (has_loc)
                    continue;
                
                CRef<CSeq_inst> new_inst(new CSeq_inst);
                new_inst->Assign(inst);
                bsh_to_inst[bsh] = make_pair(new_inst, false);
            }
            map<CBioseq_Handle, map<TSeqPos, int> > diffs;
            map<CBioseq_Handle, vector<NRawToDeltaSeq::SGap> > gaps;
            
            ITERATE (TConstScopedObjects, it, objects) 
            {
                CRef<CScope> scope = it->scope;
                CBioseq_Handle bsh;
                TSeqPos from;
                const CGapEdit* gap_edit = dynamic_cast<const CGapEdit*>((*it).object.GetPointer());
                if (gap_edit)
                {
                    bsh = gap_edit->GetBioseq_Handle();
                    from = gap_edit->GetFrom();
                }
                const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>((*it).object.GetPointer());
                if (feat && feat->IsSetData() && feat->GetData().IsImp() && feat->GetData().GetImp().IsSetKey() && feat->GetData().GetImp().GetKey() == "gap" && feat->IsSetLocation())
                {
                    bsh = scope->GetBioseqHandle(feat->GetLocation());
                    from = feat->GetLocation().GetStart(eExtreme_Positional) + 1;
                }
                
                if (!bsh)
                    continue;
                
                for (const auto& d : diffs[bsh])
                    if (d.first < from)
                    {
                        from += d.second;
                    }
        
                CRef<CSeq_inst> new_inst = bsh_to_inst[bsh].first;        
                TSeqPos start = 1;
                NON_CONST_ITERATE(CDelta_ext::Tdata, delta_i, new_inst->SetExt().SetDelta().Set()) 
                {
                    if (delta_i->Empty()) 
                        continue; 
                    CDelta_seq& seg = **delta_i;
   
                    size_t delta_len = seg.GetLiteral().GetLength();

                    if (start == from)
                    {
                        seg.SetLiteral().SetLength(new_size);
                        int diff = new_size;
                        diff -= delta_len;
                        if (new_inst->IsSetLength())
                        {
                            diffs[bsh][start] = diff;
                            TSeqPos old_length = new_inst->GetLength();
                            new_inst->SetLength(old_length + diff);
                        }

                        {
                            NRawToDeltaSeq::SGap gap;
                            gap.start = start;
                            gap.length = new_size - delta_len;
                            gap.is_known = true;
                            gap.is_replace = false;
                            gaps[bsh].push_back(gap);
                        }
                        
                        bsh_to_inst[bsh].second = true;
                        break;
                    }
            
                    start += delta_len;
                }

                int diff = 0;
                for (CFeat_CI feat_ci(bsh, CSeqFeatData::eSubtype_gap); feat_ci; ++feat_ci)
                {
                    const CSeq_feat& orig = feat_ci->GetOriginalFeature();
                    start = orig.GetLocation().GetStart(eExtreme_Positional) + 1;
                    if (start == from)
                    {
                        CRef<CSeq_feat> new_feat(new CSeq_feat());
                        new_feat->Assign(orig);  
                        TSeqPos delta_len = new_feat->GetLocation().GetTotalRange().GetLength();
                        new_feat->AddOrReplaceQualifier("estimated_length", NStr::LongToString(new_size));

                        diff = new_size;
                        diff -= delta_len;
                        if (new_feat->GetLocation().IsInt())
                        {
                            new_feat->SetLocation().SetInt().SetTo(new_feat->GetLocation().GetInt().GetFrom() + new_size - 1);
                        }
                        if (!bsh_to_inst[bsh].second && new_inst->IsSetLength())
                        {
                            diffs[bsh][start] = diff;
                            TSeqPos old_length = new_inst->GetLength();
                            new_inst->SetLength(old_length + diff);
                            {
                                NRawToDeltaSeq::SGap gap;
                                gap.start = start;
                                gap.length = new_size - delta_len;
                                gap.is_known = true;
                                gap.is_replace = false;
                                gaps[bsh].push_back(gap);
                            }
                            bsh_to_inst[bsh].second = true;
                        }
                
                        composite->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
                        break;
                    }
                }
                if (diff != 0)
                {
                    for (CFeat_CI feat_ci(bsh, CSeqFeatData::eSubtype_gap); feat_ci; ++feat_ci)
                    {
                        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
                        start = orig.GetLocation().GetStart(eExtreme_Positional) + 1;
                        if (start > from)
                        {
                            CRef<CSeq_feat> new_feat(new CSeq_feat());
                            new_feat->Assign(orig);  
                            if (new_feat->GetLocation().IsInt())
                            {
                                new_feat->SetLocation().SetInt().SetFrom(new_feat->GetLocation().GetInt().GetFrom() + diff);
                                new_feat->SetLocation().SetInt().SetTo(new_feat->GetLocation().GetInt().GetTo() + diff);
                                composite->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
                            }
                        }
                    }
                }
            }

            CRef<CCmdComposite> local_cmd( new CCmdComposite("Seq-inst change to facilitate retranslation") );  
            for (const auto& it : bsh_to_inst)
            {
                CBioseq_Handle bsh = it.first;
                CRef<CSeq_inst> new_inst = it.second.first;
                bool modified = it.second.second;      
                if (modified)
                {
                    CRef<CCmdChangeBioseqInst> cmd(new CCmdChangeBioseqInst(bsh, *new_inst));
                    local_cmd->AddCommand(*cmd);
                }
            }
            composite->AddCommand(*local_cmd);
            local_cmd->Execute();
            NRawToDeltaSeq::AdjustFeatureLocations(gaps, composite, false, false, false, create_general_only, max_feat_id, old_to_new, create_xref_map);
            local_cmd->Unexecute();

            if (!diffs.empty())
            {
                CSeq_entry_Handle tse = diffs.begin()->first.GetTopLevelEntry();
                for (objects::CAlign_CI align_ci(tse); align_ci; ++ align_ci)
                {
                    const objects::CSeq_align& align = *align_ci;
                    CSeq_align_Handle ah = align_ci.GetSeq_align_Handle();
                    int num_rows = align.CheckNumRows();
                    if (num_rows < 2)
                        break;
                    if (!align.IsSetSegs() || !align.GetSegs().IsDenseg() ||
                        !align.GetSegs().GetDenseg().IsSetStarts() || !align.GetSegs().GetDenseg().IsSetLens())
                        continue;
                
                    vector< TSignedSeqPos > starts(align.GetSegs().GetDenseg().GetStarts());
                    size_t num_segs = align.GetSegs().GetDenseg().GetLens().size();
                    for (size_t s = 0; s < num_segs; s++)
                        for (size_t row = 0; row < num_rows; row++)
                        {
                            TSignedSeqPos from = starts[num_rows * s + row];
                            if (from != -1)
                            {
                                const CSeq_id& id = align.GetSeq_id(row);
                                CBioseq_Handle bsh = ah.GetScope().GetBioseqHandle(id);
                                if (!bsh)
                                    continue;
                                for (const auto& d : diffs[bsh])
                                    if (d.first < from)
                                    {
                                        from += d.second;
                                    }
                                starts[num_rows * s + row] = from;
                            }
                        }
                    CRef<CSeq_align> new_align(new CSeq_align);
                    new_align->Assign(align);
                    new_align->SetSegs().SetDenseg().SetStarts() = starts;
                    new_align->SetSegs().SetDenseg().TrimEndGaps();
                    new_align->SetSegs().SetDenseg().RemovePureGapSegs();
                    new_align->SetSegs().SetDenseg().Compact();
                    CRef<CCmdChangeAlign> cmd (new CCmdChangeAlign(ah, *new_align));        
                    composite->AddCommand(*cmd); 
                }
            }
        }
    }
    return composite;
}

CRef<CCmdComposite> CCombineAdjacentGaps::apply(objects::CSeq_entry_Handle seh)
{
    map<CObject_id::TId, CObject_id::TId> old_to_new;
    bool create_xref_map = true;
    apply_impl(seh, old_to_new, create_xref_map);
    create_xref_map = false;
    return apply_impl(seh, old_to_new, create_xref_map);
}

CRef<CCmdComposite> CCombineAdjacentGaps::apply_impl(objects::CSeq_entry_Handle seh, map<CObject_id::TId, CObject_id::TId> &old_to_new, bool create_xref_map)
{
    CRef<CCmdComposite> composite( new CCmdComposite("Combine Adjacent Gaps") );   
    bool any_modified = false;
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(seh);
    CObject_id::TId max_feat_id = CFixFeatureId::s_FindHighestFeatureId(seh);
    map<CBioseq_Handle, vector<NRawToDeltaSeq::SGap> >  gaps;
    for (CBioseq_CI bi(seh, CSeq_inst::eMol_na); bi; ++bi)
    {
        bool bsh_modified = false;
        CBioseq_Handle bsh = *bi;
        TSeqPos len = bsh.GetBioseqLength();
        if (len <= 1)
            continue;
        if (!bsh.IsSetInst())
            continue;
        
        const CSeq_inst &inst = bsh.GetInst();
        if (!inst.IsSetExt() || !inst.GetExt().IsDelta())
            continue;
        bool has_loc(false);
        ITERATE(CDelta_ext::Tdata, iter, inst.GetExt().GetDelta().Get()) 
        {
            if ( (*iter)->IsLoc() ) 
            {
                has_loc = true;
                break;
            }
        }
        if (has_loc)
            continue;
        
        CRef<CSeq_inst> new_inst(new CSeq_inst);
        new_inst->Assign(inst);
        try 
        {
            vector<pair<int, bool> > gap_len;
            TSeqPos pos = 0;
            CSeqVector sv = bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac);
            ITERATE (CDelta_ext::Tdata, delta_i, inst.GetExt().GetDelta().Get()) 
            {
                if (delta_i->Empty()) 
                    continue; 
                const CDelta_seq& seg = **delta_i;
                size_t delta_len = seg.GetLiteral().GetLength();
                if (sv.IsInGap(pos))
                {
                    bool unknown = (seg.GetLiteral().IsSetFuzz() && seg.GetLiteral().GetFuzz().IsLim() && seg.GetLiteral().GetFuzz().GetLim() == objects::CInt_fuzz::eLim_unk);
                    gap_len.push_back(pair<int, bool> (delta_len, unknown));
                }             
                else
                {
                    gap_len.push_back(pair<int, bool> (0, false));
                }
                pos += delta_len;
            }
          
            TSeqPos start = 1;
            size_t i = 0;
            CDelta_ext::Tdata::iterator delta_i = new_inst->SetExt().SetDelta().Set().begin();
            while(delta_i != new_inst->SetExt().SetDelta().Set().end())
            {
                if (delta_i->Empty()) 
                {
                    ++delta_i;
                    continue; 
                }
                CDelta_seq& seg = **delta_i;
                size_t orig_len = seg.GetLiteral().GetLength();
                int delta_len = gap_len[i].first;
                if (delta_len < 0)
                {
                    delta_i = new_inst->SetExt().SetDelta().Set().erase(delta_i);
                    i++;
                    start += orig_len;
                    continue;
                }

                if (delta_len > 0)
                {                   
                    bool unknown = gap_len[i].second;
                    bool modified = false;
                    for (size_t j = i + 1; j < gap_len.size(); j++)
                    {
                        if (gap_len[j].first <= 0)
                            break;
                        delta_len += gap_len[j].first;
                        unknown |= gap_len[j].second;
                        gap_len[j].first = -1;
                        modified = true;
                    }
                    if (modified)
                    {
                        seg.SetLiteral().SetLength(delta_len);
                        if (unknown)
                        {
                            seg.SetLiteral().SetFuzz().SetLim(objects::CInt_fuzz::eLim_unk); 
                            seg.SetLiteral().SetLength(100);
                            int diff = 100;
                            diff -= delta_len;
                            if (new_inst->IsSetLength())
                            {
                                TSeqPos old_length = new_inst->GetLength();
                                new_inst->SetLength(old_length + diff);
                            }

                            NRawToDeltaSeq::SGap gap;
                            gap.start = start;
                            gap.length = diff;
                            gap.is_known = true;
                            gap.is_replace = false;
                            gaps[bsh].push_back(gap);
                        }
                        bsh_modified = true;
                    }
                }             
                ++delta_i;
                i++;
                start += orig_len;
            }
            
        } catch (CException ) {} catch (std::exception ) {}
        
        if (bsh_modified)
        {
            CRef<CCmdChangeBioseqInst> cmd(new CCmdChangeBioseqInst(bsh, *new_inst));
            composite->AddCommand(*cmd);
            any_modified = true;
        }
    }
    if (any_modified)
    {
        auto local_cmd = composite;
        local_cmd->Execute();
        NRawToDeltaSeq::AdjustFeatureLocations(gaps, composite, false, false, false, create_general_only, max_feat_id, old_to_new, create_xref_map);
        local_cmd->Unexecute();
    }
    else
        composite.Reset();
    return  composite;
}

END_NCBI_SCOPE
