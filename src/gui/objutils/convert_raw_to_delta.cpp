/*  $Id: convert_raw_to_delta.cpp 44875 2020-04-02 19:03:08Z coulouri $
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
 * Authors:  Igor Filippov
 *
 * File Description:
 *
 */


#include <ncbi_pch.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/seq/Seq_data.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <objects/seqfeat/Feat_id.hpp>
#include <objtools/cleanup/fix_feature_id.hpp>

#include <objmgr/feat_ci.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <objmgr/util/sequence.hpp>


#include <objtools/edit/cds_fix.hpp>
#include <objtools/edit/seq_entry_edit.hpp>
#include <util/sequtil/sequtil_convert.hpp>


#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_add_cds.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/cmd_del_bioseq.hpp>
#include <gui/objutils/cmd_del_seq_align.hpp>
#include <gui/objutils/convert_raw_to_delta.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(NRawToDeltaSeq)
USING_SCOPE(objects);


void GapLocationsFromNs(const CSeq_inst& inst,
    size_t min_unknown, int max_unknown,
    size_t min_known, int max_known,
    vector < pair<int, int> > &start_length_locs,
    vector<bool> &vec_is_known)

{
    // can only convert if starting as raw
    if (!inst.IsSetRepr() || inst.GetRepr() != CSeq_inst::eRepr_raw
        || !inst.IsSetSeq_data()) {
        return;
    }

    string iupacna;

    switch (inst.GetSeq_data().Which()) {
    case CSeq_data::e_Iupacna:
        iupacna = inst.GetSeq_data().GetIupacna();
        break;
    case CSeq_data::e_Ncbi2na:
        CSeqConvert::Convert(inst.GetSeq_data().GetNcbi2na().Get(), CSeqUtil::e_Ncbi2na,
            0, inst.GetLength(), iupacna, CSeqUtil::e_Iupacna);
        break;
    case CSeq_data::e_Ncbi4na:
        CSeqConvert::Convert(inst.GetSeq_data().GetNcbi4na().Get(), CSeqUtil::e_Ncbi4na,
            0, inst.GetLength(), iupacna, CSeqUtil::e_Iupacna);
        break;
    case CSeq_data::e_Ncbi8na:
        CSeqConvert::Convert(inst.GetSeq_data().GetNcbi8na().Get(), CSeqUtil::e_Ncbi8na,
            0, inst.GetLength(), iupacna, CSeqUtil::e_Iupacna);
        break;
    default:
        return;
        break;
    }

    string element = "";
    size_t n_len = 0;
    size_t pos = 0;
    ITERATE(string, it, iupacna) {
        if ((*it) == 'N') {
            n_len++;
            element += *it;
        }
        else {
            if (n_len > 0) {
                // decide whether to turn this past run of Ns into a gap
                bool is_unknown = false;
                bool is_known = false;

                if (n_len >= min_unknown && (max_unknown < 0 || n_len <= max_unknown)) {
                    is_unknown = true;
                }
                else if (n_len >= min_known && (max_known < 0 || n_len <= max_known)) {
                    is_known = true;
                }
                if (is_unknown || is_known) {
                    // make literal to contain sequence before gap
                    if (element.length() > n_len) {
                        element = element.substr(0, element.length() - n_len);
                        pos += element.length();
                    }
                    start_length_locs.push_back(pair<int, int>(pos, n_len));
                    pos += n_len;
                    vec_is_known.push_back(is_known);
                    element.clear();
                }
                n_len = 0;
            }
            element += *it;
        }
    }

    if (n_len > 0) {
        // decide whether to turn this past run of Ns into a gap
        bool is_unknown = false;
        bool is_known = false;

        if (n_len >= min_unknown && (max_unknown < 0 || n_len <= max_unknown)) {
            is_unknown = true;
        }
        else if (n_len >= min_known && (max_known < 0 || n_len <= max_known)) {
            is_known = true;
        }
        if (is_unknown || is_known) {
            // make literal to contain sequence before gap
            if (element.length() > n_len) {
                element = element.substr(0, element.length() - n_len);
                pos += element.length();
            }
            start_length_locs.push_back(pair<int, int>(pos, n_len));
            pos += n_len;
            vec_is_known.push_back(is_known);
        }
    }
    else {
        pos += element.length();
    }

}

void UpdateDensegAlignment(CSeq_align_Handle ah, CBioseq_Handle bsh, CRef<CCmdComposite> composite,
    const vector < pair<int, int> > &start_length_locs,
    const vector<bool> &vec_is_known,
    set<const CSeq_align*> &deleted_aligns)
{
    const CSeq_align& align = *ah.GetSeq_align();
    int num_rows = align.CheckNumRows();
    bool found_row = false;
    int base_row = 0;
    for (int row = 0; row < num_rows; row++)
    {
        const CSeq_id&  align_id = align.GetSeq_id(row);
        if (bsh.IsSynonym(align_id))
        {
            base_row = row;
            found_row = true;
            break;
        }
    }

    bool found_overlap = false;
    if (found_row)
    {

        for (size_t i = 0; i < start_length_locs.size(); i++)
        {
            if (!vec_is_known[i])
            {
                const vector< TSignedSeqPos > &starts = align.GetSegs().GetDenseg().GetStarts(); // TODO take into account strand
                const vector< TSeqPos > &lens = align.GetSegs().GetDenseg().GetLens();
                size_t num_segs = lens.size();

                size_t start = start_length_locs[i].first;
                size_t stop = start + start_length_locs[i].second - 1;

                for (size_t j = 0; j<num_segs; j++)
                {
                    TSignedSeqPos seg_start = starts[num_rows*j + base_row];
                    if (seg_start < 0)
                        continue;
                    TSignedSeqPos seg_stop = seg_start + lens[j] - 1;
                    if ((start >= seg_start && start <= seg_stop) ||
                        (stop >= seg_start && stop <= seg_stop) ||
                        (seg_start >= start && seg_start <= stop) ||
                        (seg_stop >= start && seg_stop <= stop))
                    {
                        found_overlap = true;
                        break;
                    }
                }
            }
        }

    }
    if (found_overlap && deleted_aligns.find(&align) == deleted_aligns.end())
    {
        CRef<CCmdDelSeq_align> cmd(new CCmdDelSeq_align(ah));
        composite->AddCommand(*cmd);
        deleted_aligns.insert(&align);
    }
}

static void UpdateFeaturesForGaps(const map<CBioseq_Handle, pair<vector < pair<int, int> >, vector<bool> > > &map_gaps,
                                  CRef<CCmdComposite> composite,
                                  bool break_features, bool keep_gap_length, bool create_general_only, CObject_id::TId &max_feat_id, map<CObject_id::TId, CObject_id::TId> &old_to_new, bool create_xref_map)
{
    map<CBioseq_Handle, vector<SGap> > new_map_gaps;
    for (const auto &gap : map_gaps)
    {
        auto bsh = gap.first;
        const auto &start_length_locs = gap.second.first;
        const auto &vec_is_known = gap.second.second;
        vector<SGap> gaps;
        gaps.reserve(start_length_locs.size());
        for (size_t i = 0; i < start_length_locs.size(); ++i)
        {
            SGap gap;
            gap.start = start_length_locs[i].first;
            gap.length = start_length_locs[i].second;
            gap.is_known = vec_is_known[i];
            gap.is_replace = true;
            gaps.push_back(gap);
        }
        new_map_gaps[bsh] = gaps;
    }
    AdjustFeatureLocations(new_map_gaps, composite, false, break_features, keep_gap_length, create_general_only, max_feat_id, old_to_new, create_xref_map);
}

static void s_ReplaceFeatureIdXref(CSeq_feat& f, map<CObject_id::TId, CObject_id::TId> &old_to_new)
{
    if (f.IsSetXref()) 
    {
        NON_CONST_ITERATE(CSeq_feat::TXref, xit, f.SetXref()) 
        {
            if ((*xit)->IsSetId() && (*xit)->GetId().IsLocal() && (*xit)->GetId().GetLocal().IsId())
            {
                auto it = old_to_new.find((*xit)->GetId().GetLocal().GetId());
                if (it != old_to_new.end()) 
                {
                    (*xit)->SetId().SetLocal().SetId(it->second);
                }
            }
        }
    }
}

static void x_AdjustOrigLabel(CSeq_feat& feat, size_t& id_offset, string& id_label, const string& qual)
{
    if (!feat.IsSetQual()) {
        return;
    }
    NON_CONST_ITERATE(CSeq_feat::TQual, it, feat.SetQual()) {
        if ((*it)->IsSetQual() && (*it)->IsSetVal() &&
            !NStr::IsBlank((*it)->GetVal()) &&
            NStr::EqualNocase((*it)->GetQual(), qual) &&
            (id_label.empty() || NStr::Equal((*it)->GetVal(), id_label) || NStr::Equal((*it)->GetVal(), id_label + "_1"))) {
            if (id_label.empty()) {
                id_label = (*it)->GetVal();
            }
            (*it)->SetVal(id_label + "_" + NStr::NumericToString(id_offset));
            id_offset++;
        }
    }
}

static void UpdateCodeBreaksAndAnticodons(objects::CSeq_feat &feat, objects::CScope &scope, const map<CBioseq_Handle, vector<SGap> > &gaps, 
                                   bool break_features, bool keep_gap_length)
{
    if (feat.IsSetData() && feat.GetData().IsCdregion() &&
        feat.GetData().GetCdregion().IsSetCode_break()) {

        CCdregion& cdr = feat.SetData().SetCdregion();
        CCdregion::TCode_break::iterator cit = cdr.SetCode_break().begin();
        while (cit != cdr.SetCode_break().end()) {
            bool do_remove = false;
            if ((*cit)->IsSetLoc()) {
                vector<CRef<objects::CSeq_loc> > locations;
                bool comment = false;
                FindNewLocations((*cit)->GetLoc(), scope, gaps, locations, comment,
                                 break_features,
                                 keep_gap_length);
                if (!locations.empty()) {
                    CRef<CSeq_loc> new_loc = feat.GetLocation().Intersect(*locations.front(), 0, NULL);
                    if (new_loc && !new_loc->IsEmpty() && !new_loc->IsNull()) {
                        (*cit)->SetLoc().Assign(*new_loc);
                    } else {
                        do_remove = true;
                    }
                } else  {
                    do_remove = true;
                }
            }
            if (do_remove) {
                cit = cdr.SetCode_break().erase(cit);
            } else {
                ++cit;
            }
        }
        if (cdr.GetCode_break().empty()) {
            cdr.ResetCode_break();
        }
    }
    if (feat.IsSetData() && feat.GetData().IsRna() &&
        feat.GetData().GetRna().IsSetExt() &&
        feat.GetData().GetRna().GetExt().IsTRNA()) {

        CTrna_ext& trna = feat.SetData().SetRna().SetExt().SetTRNA();
        if (trna.IsSetAnticodon()) {

            vector<CRef<objects::CSeq_loc> > locations;
            bool comment = false;
            FindNewLocations(trna.GetAnticodon(), scope, gaps, locations, comment,
                             break_features,
                             keep_gap_length);
            if (!locations.empty()) {
                CRef<CSeq_loc> new_loc = feat.GetLocation().Intersect(*locations.front(), 0, NULL);
                if (new_loc && !new_loc->IsEmpty() && !new_loc->IsNull()) {
                    trna.SetAnticodon().Assign(*new_loc);
                } else {
                    trna.ResetAnticodon();
                }
            } else {
                trna.ResetAnticodon();
            }
        }
    }
}

static void AdjustSingleFeature(const objects::CMappedFeat& feat, objects::CScope &scope, const map<CBioseq_Handle, vector<SGap> > &gaps, CRef<CCmdComposite> composite,
                         bool split_gene_locations, bool break_features, bool keep_gap_length,
                         bool create_general_only, CObject_id::TId &max_feat_id, map<CObject_id::TId, CObject_id::TId> &old_to_new, bool create_xref_map)
{
    if (feat.GetOriginalFeature().IsSetData())
    {
        if (feat.GetOriginalFeature().GetData().IsCdregion() &&
            feat.GetOriginalFeature().IsSetExcept_text() && NStr::Find(feat.GetOriginalFeature().GetExcept_text(), "RNA editing") != string::npos)
        {
            return;
        }
        const objects::CSeq_loc& feat_loc = feat.GetLocation();
        vector<CRef<objects::CSeq_loc> > locations;
        bool comment = false;
        FindNewLocations(feat_loc, scope, gaps, locations, comment,
                         ((feat.GetSeq_feat_Handle().GetFeatSubtype() == CSeqFeatData::eSubtype_cdregion) && break_features) ||
                         ((feat.GetSeq_feat_Handle().GetFeatSubtype() == CSeqFeatData::eSubtype_mRNA) && break_features) ||
                         ((feat.GetSeq_feat_Handle().GetFeatSubtype() == CSeqFeatData::eSubtype_gene) && split_gene_locations),
                         keep_gap_length);

        CRef<objects::CSeq_id> id(new objects::CSeq_id);
        id->Assign(*feat_loc.GetId());

        size_t transcript_id_offset = 1;
        string transcript_id_label;
        size_t protein_id_offset = 1;
        string protein_id_label;
        
        if (locations.empty() || locations.front()->IsNull())
        {
            composite->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(feat.GetSeq_feat_Handle())));
        }
        else
        {           
            CRef<objects::CSeq_feat> new_feat = UpdateFeat(locations.front(), feat.GetOriginalFeature(), comment, scope);           
            UpdateCodeBreaksAndAnticodons(*new_feat, scope, gaps, break_features, keep_gap_length);
            composite->AddCommand(*CRef< CCmdChangeSeq_feat >(new CCmdChangeSeq_feat(feat.GetSeq_feat_Handle(), *new_feat)));


            if (new_feat->IsSetProduct())
            {
                const CSeq_id *id = new_feat->GetProduct().GetId();
                if (id)
                {
                    CBioseq_Handle prot_bsh = scope.GetBioseqHandle(*id);
                    if (prot_bsh)
                    {
                        string prot;
                        try
                        {
                            CSeqTranslator::Translate(*new_feat, scope, prot);
                        }
                        catch (const CSeqVectorException&) {}

                        if (!prot.empty())
                        {
                            if (NStr::EndsWith(prot, "*"))
                            {
                                prot = prot.substr(0, prot.length() - 1);
                            }
                            CRef<CBioseq> prot_seq(new CBioseq);
                            prot_seq->Assign(*(prot_bsh.GetCompleteBioseq()));
                            prot_seq->SetInst().ResetExt();
                            prot_seq->SetInst().SetRepr(objects::CSeq_inst::eRepr_raw);
                            prot_seq->SetInst().SetSeq_data().SetIupacaa().Set(prot);
                            prot_seq->SetInst().SetLength(TSeqPos(prot.length()));
                            prot_seq->SetInst().SetMol(CSeq_inst::eMol_aa);
                            CRef<CCmdChangeBioseqInst> cmd(new CCmdChangeBioseqInst(prot_bsh, prot_seq->SetInst()));
                            composite->AddCommand(*cmd);
                            objects::CFeat_CI prot_feat_ci(prot_bsh, objects::SAnnotSelector(objects::CSeqFeatData::eSubtype_prot));
                            if (prot_feat_ci)
                            {
                                CRef<objects::CSeq_feat> prot_feat(new objects::CSeq_feat());
                                prot_feat->Assign(*(prot_feat_ci->GetSeq_feat()));
                                prot_feat->ResetLocation();
                                prot_feat->SetLocation().SetInt().SetId().Assign(*(prot_seq->GetId().front()));
                                prot_feat->SetLocation().SetInt().SetFrom(0);
                                prot_feat->SetLocation().SetInt().SetTo(prot_seq->GetLength() - 1);
                                edit::AdjustProteinFeaturePartialsToMatchCDS(*prot_feat, *new_feat);
                                CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(prot_feat_ci->GetSeq_feat_Handle(), *prot_feat));
                                composite->AddCommand(*chgFeat);
                            }
                            bool modified;
                            RemapOtherProtFeats(feat.GetOriginalFeature(), *new_feat, prot_bsh, composite, modified);
                        }
                        else
                        {
                            CRef<CCmdDelBioseqInst> cmd(new CCmdDelBioseqInst(prot_bsh));
                            composite->AddCommand(*cmd);
                        }
                    }
                }
            }

        }


        if (feat.GetSeq_feat_Handle().GetFeatSubtype() == CSeqFeatData::eSubtype_cdregion)
        {
            int id_offset = 0;           
            size_t protein_seqid_offset = 1;
            for (size_t j = 1; j < locations.size(); j++)
                if (!locations[j]->IsNull())
                {

                    CRef<objects::CSeq_feat> add_feat = UpdateFeat(locations[j], feat.GetOriginalFeature(), comment, scope);
                    UpdateCodeBreaksAndAnticodons(*add_feat, scope, gaps, break_features, keep_gap_length);
                    if (add_feat->IsSetId() && add_feat->GetId().IsLocal() && add_feat->GetId().GetLocal().IsId())
                    {
                        ++max_feat_id;
                        if (create_xref_map)
                            old_to_new[add_feat->GetId().GetLocal().GetId()] = max_feat_id; 
                        add_feat->SetId().SetLocal().SetId(max_feat_id);
                    }
                    if (!create_xref_map)
                    {
                        s_ReplaceFeatureIdXref(*add_feat, old_to_new);
                        x_AdjustOrigLabel(*add_feat, transcript_id_offset, transcript_id_label, "orig_transcript_id");
                        x_AdjustOrigLabel(*add_feat, protein_id_offset, protein_id_label, "orig_protein_id");
                    }


                    CRef<objects::CSeq_feat> prot_feat;
                    objects::CBioseq_Handle product;
                    if (add_feat->IsSetProduct())
                        product = scope.GetBioseqHandle(add_feat->GetProduct());
                    vector<CRef<CSeq_feat> > other_prot_feats;
                    vector<CRef<objects::CSeq_id> > new_prot_id;
                    string id_label;
                    id_offset++;
                    if (product)
                    {
                        new_prot_id = objects::edit::GetNewProtIdFromExistingProt(product, id_offset, id_label);
                    }
                    else
                    {
                        objects::CBioseq_Handle bsh = scope.GetBioseqHandle(add_feat->GetLocation());
                        new_prot_id.push_back(objects::edit::GetNewProtId(bsh, id_offset, id_label, create_general_only));
                    }
                    add_feat->SetProduct().SetWhole().Assign(*new_prot_id.front());
                    if (product)
                    {
                        for (objects::CFeat_CI prot_feat_ci(product); prot_feat_ci; ++prot_feat_ci)
                        {
                            if (prot_feat_ci->GetSeq_feat_Handle().GetFeatSubtype() == objects::CSeqFeatData::eSubtype_prot)
                            {
                                prot_feat.Reset(new objects::CSeq_feat());
                                prot_feat->Assign(*(prot_feat_ci->GetSeq_feat()));
                                if (prot_feat->IsSetId() && prot_feat->GetId().IsLocal() && prot_feat->GetId().GetLocal().IsId())
                                {
                                    ++max_feat_id;
                                    if (create_xref_map)
                                        old_to_new[prot_feat->GetId().GetLocal().GetId()] = max_feat_id;
                                    prot_feat->SetId().SetLocal().SetId(max_feat_id);
                                }
                                if (!create_xref_map)
                                {
                                    s_ReplaceFeatureIdXref(*prot_feat, old_to_new);
                                }
                            }
                            else
                            {
                                RemapOtherProtFeats(*feat.GetSeq_feat(), *add_feat, prot_feat_ci->GetSeq_feat_Handle(), other_prot_feats, max_feat_id, old_to_new, create_xref_map);
                            }
                        }
                    }

                    const objects::CSeq_annot_Handle& annot_handle = feat.GetAnnot();
                    objects::CSeq_entry_Handle seh = annot_handle.GetParentEntry();
                    composite->AddCommand(*CRef<CCmdCreateCDS>(new CCmdCreateCDS(seh, *add_feat, prot_feat, new_prot_id, other_prot_feats)));
                }
        }
        else
        {          
            for (size_t j = 1; j < locations.size(); j++)
                if (!locations[j]->IsNull())
                {

                    CRef<objects::CSeq_feat> add_feat = UpdateFeat(locations[j], feat.GetOriginalFeature(), comment, scope);
                    UpdateCodeBreaksAndAnticodons(*add_feat, scope, gaps, break_features, keep_gap_length);
                    if (add_feat->IsSetId() && add_feat->GetId().IsLocal() && add_feat->GetId().GetLocal().IsId())
                    {
                        ++max_feat_id;
                        if (create_xref_map)
                            old_to_new[add_feat->GetId().GetLocal().GetId()] = max_feat_id; 
                        add_feat->SetId().SetLocal().SetId(max_feat_id);
                    }
                    if (!create_xref_map)
                    {
                        s_ReplaceFeatureIdXref(*add_feat, old_to_new);
                        x_AdjustOrigLabel(*add_feat, transcript_id_offset, transcript_id_label, "orig_transcript_id");
                        x_AdjustOrigLabel(*add_feat, protein_id_offset, protein_id_label, "orig_protein_id");
                    }
                    const objects::CSeq_annot_Handle& annot_handle = feat.GetAnnot();
                    objects::CSeq_entry_Handle seh = annot_handle.GetParentEntry();
                    composite->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *add_feat)));
                }
        }
    }
}

void AdjustFeatureLocations(const map<CBioseq_Handle, vector<SGap> > &map_gaps, 
                            CRef<CCmdComposite> composite, bool split_gene_locations, bool break_features, bool keep_gap_length, bool create_general_only, 
                            CObject_id::TId &max_feat_id, map<CObject_id::TId, CObject_id::TId> &old_to_new, bool create_xref_map)
{
    if (map_gaps.empty())
        return;
    auto seh = map_gaps.cbegin()->first.GetTopLevelEntry();
    objects::CScope &scope = seh.GetScope();
    for (objects::CFeat_CI feat_it(seh); feat_it; ++feat_it)
    {
        AdjustSingleFeature(*feat_it, scope, map_gaps, composite, split_gene_locations, break_features, keep_gap_length, create_general_only, max_feat_id, old_to_new, create_xref_map);
    }
}

void FindNewLocations(const objects::CSeq_loc &feat_loc, objects::CScope &scope, const map<CBioseq_Handle, vector<SGap> > &map_gaps, 
                      vector<CRef<objects::CSeq_loc> > &locations, bool &comment, bool break_features, bool keep_gap_length)
{
    CRef<objects::CSeq_loc> new_loc(new objects::CSeq_loc);
    new_loc->Assign(feat_loc);  

    locations.clear();
    vector < vector< CRef<objects::CSeq_loc> > > all_locs;
    vector < vector<pair<bool, bool> > > all_partials;
    vector< CRef<objects::CSeq_loc> > locs;
    vector<pair<bool, bool> > partials;

    for (objects::CSeq_loc_I loc_it(*new_loc); loc_it; ++loc_it)      // CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Positional);
    {
        if (loc_it.IsEmpty())
        {
            continue;
        }
        auto strand = loc_it.GetStrand();
        CRef<CSeq_id> id(new CSeq_id);
        id->Assign(loc_it.GetSeq_id());
        CConstRef<CSeq_loc> loc = loc_it.GetRangeAsSeq_loc();
        CBioseq_Handle bsh = scope.GetBioseqHandle(*loc);
        vector<SGap> gaps;
        auto it = map_gaps.find(bsh);
        if (it != map_gaps.end())
            gaps = it->second;
        CRef<objects::CSeq_loc> bioseq_loc = bsh.GetRangeSeq_loc(0, 0);
        TSeqPos seq_start = bioseq_loc->GetStart(objects::eExtreme_Positional);
        objects::CSeq_loc_CI::TRange feat_range = loc_it.GetRange();
        TSeqPos feat_start = feat_range.GetFrom() - seq_start;
        TSeqPos feat_stop = feat_range.GetTo() - seq_start;

        TSignedSeqPos offset = 0;
        TSignedSeqPos offset_middle = 0;
        bool partial5 = false;
        bool done = false;
        for (size_t i = 0; i < gaps.size(); ++i)
        {
            TSignedSeqPos new_gap_length = (keep_gap_length ? gaps[i].length : 100);        

            if (feat_stop < gaps[i].start) // no inserts or replaces in this interval
            {
                TSeqPos new_start = feat_start + offset;
                TSeqPos new_stop = feat_stop + offset + offset_middle;
                offset += offset_middle;
                offset_middle = 0;

                locs.emplace_back(new CSeq_loc(*id, new_start, new_stop, strand));
                partials.emplace_back(partial5, false);

                partial5 = false;
                done = true;
                break;
            }
            else if (!gaps[i].is_replace && gaps[i].start <= feat_start) // insert before the interval
            {
                if (gaps[i].is_known)
                    offset += gaps[i].length;
                else
                    offset += new_gap_length;
                continue;
            }
            else if (gaps[i].is_replace && gaps[i].is_known && gaps[i].start + gaps[i].length <= feat_start) // known replace before the interval
            {
                continue;
            }
            else if (gaps[i].is_replace && !gaps[i].is_known && gaps[i].start + gaps[i].length <= feat_start) // unknown replace before the interval
            {
                offset += new_gap_length - gaps[i].length;
                continue;
            }
            else  if (!gaps[i].is_replace && gaps[i].is_known && gaps[i].start > feat_start && gaps[i].start <= feat_stop) // known insert
            {
                offset_middle += gaps[i].length;
                continue;
            }
            else  if (!gaps[i].is_replace && !gaps[i].is_known && gaps[i].start > feat_start && gaps[i].start <= feat_stop) // unknown insert
            {
                offset_middle += new_gap_length;

                if (break_features)
                {
                    TSeqPos new_start = feat_start + offset;
                    TSeqPos new_stop = gaps[i].start + offset + offset_middle - 1;
                    offset += offset_middle + new_gap_length;
                    offset_middle = 0;

                    locs.emplace_back(new CSeq_loc(*id, new_start, new_stop, strand));
                    partials.emplace_back(partial5, true);

                    partial5 = true;
                    comment = true;
                    feat_start = gaps[i].start; // no offset here because we are processing the current interval again.

                    all_locs.push_back(locs);
                    all_partials.push_back(partials);
                    locs.clear();
                    partials.clear();
                }
                continue;
            }
            else  if (gaps[i].is_replace && gaps[i].is_known && gaps[i].start >= feat_start && gaps[i].start + gaps[i].length - 1 <= feat_stop) // known replace
            {
                continue;
            }
            else  if (gaps[i].is_replace && !gaps[i].is_known && gaps[i].start <= feat_start && gaps[i].start + gaps[i].length - 1 >= feat_start && gaps[i].start + gaps[i].length - 1 < feat_stop) // unknown replace on the left edge
            {
                offset += offset_middle + new_gap_length - gaps[i].length;
                offset_middle = 0;
                partial5 = true;
                feat_start = gaps[i].start + gaps[i].length;

                if (break_features && !locs.empty())             
                {
                    comment = true;
                    all_locs.push_back(locs);
                    all_partials.push_back(partials);
                    locs.clear();
                    partials.clear();
                }
                continue;
            }
            else  if (gaps[i].is_replace && !gaps[i].is_known && gaps[i].start <= feat_start && gaps[i].start + gaps[i].length - 1 >= feat_stop) // unknown replace of the whole interval
            {
                if (break_features && !locs.empty())
                {
                    comment = true;
                    all_locs.push_back(locs);
                    all_partials.push_back(partials);
                    locs.clear();
                    partials.clear();
                }
                done = true;
                break;
            }
            else  if (gaps[i].is_replace && !gaps[i].is_known && gaps[i].start > feat_start  && gaps[i].start <= feat_stop && gaps[i].start + gaps[i].length - 1 > feat_stop) // unknown replace on the right edge
            {
                TSeqPos new_start = feat_start + offset;
                TSeqPos new_stop = gaps[i].start + offset + offset_middle - 1;

                locs.emplace_back(new CSeq_loc(*id, new_start, new_stop, strand));
                partials.emplace_back(partial5, true);
                offset += offset_middle;
                offset_middle = 0;

                if (break_features)
                {
                    comment = true;
                    all_locs.push_back(locs);
                    all_partials.push_back(partials);
                    locs.clear();
                    partials.clear();
                }

                done = true;
                break;
            }
            else  if (gaps[i].is_replace && !gaps[i].is_known && gaps[i].start > feat_start  && gaps[i].start <= feat_stop && gaps[i].start + gaps[i].length - 1 <= feat_stop) // unknown replace
            {
                if (break_features)
                {
                    TSeqPos new_start = feat_start + offset;
                    TSeqPos new_stop = gaps[i].start + offset + offset_middle - 1;
                    offset += offset_middle + new_gap_length - gaps[i].length;
                    offset_middle = 0;

                    locs.emplace_back(new CSeq_loc(*id, new_start, new_stop, strand));
                    partials.emplace_back(partial5, true);

                    partial5 = true;
                    comment = true;
                    feat_start = gaps[i].start + gaps[i].length; // no offset here because we are processing the current interval again.

                    all_locs.push_back(locs);
                    all_partials.push_back(partials);
                    locs.clear();
                    partials.clear();
                }
                else
                {
                    offset_middle += new_gap_length - gaps[i].length;
                }

                continue;
            }
        }
        if (!done) // no inserts or replaces in this interval
        {
            TSeqPos new_start = feat_start + offset;
            TSeqPos new_stop = feat_stop + offset + offset_middle;
            offset += offset_middle;
            offset_middle = 0;
            locs.emplace_back(new CSeq_loc(*id, new_start, new_stop, strand));
            partials.emplace_back(partial5, false);            
            partial5 = false;
        }
    }

    if (!locs.empty())
    {
        all_locs.push_back(locs);
        all_partials.push_back(partials);
        locs.clear();
        partials.clear();
    }

    if (!all_partials.empty())
    {
        if (!all_partials.front().empty())
            all_partials.front().front().first |= feat_loc.IsPartialStart(objects::eExtreme_Positional);
        if (!all_partials.back().empty())
            all_partials.back().back().second |= feat_loc.IsPartialStop(objects::eExtreme_Positional);
    }

    for (size_t j = 0; j < all_locs.size(); j++)
    {
        objects::CSeq_loc_I loc_i;
        if (all_locs[j].empty())
        {
            loc_i.InsertNull();
        }
        else
        {
            for (size_t i = 0; i < all_locs[j].size(); i++)
            {
                loc_i.InsertInterval(*all_locs[j][i]->GetId(), all_locs[j][i]->GetStart(objects::eExtreme_Positional), all_locs[j][i]->GetStop(objects::eExtreme_Positional), all_locs[j][i]->GetStrand());
            }
        }
        CRef<objects::CSeq_loc> changed_loc = loc_i.MakeSeq_loc();
        if (!all_partials[j].empty())
        {
            changed_loc->SetPartialStart(all_partials[j].front().first, objects::eExtreme_Positional);
            changed_loc->SetPartialStop(all_partials[j].back().second, objects::eExtreme_Positional);
        }

        locations.push_back(changed_loc);
    }
}

static void AddComment(CRef<objects::CSeq_feat> new_feat)
{
    const char *cds_gap_comment = "coding region disrupted by sequencing gap";
    if (!new_feat->IsSetComment())
    {
        new_feat->SetComment(cds_gap_comment);
    }
    else if (new_feat->IsSetComment() && new_feat->GetComment().find(cds_gap_comment) == string::npos)
    {
        string comment = new_feat->GetComment();
        comment = comment + "; " + cds_gap_comment;
        new_feat->SetComment(comment);
    }
}


CRef<objects::CSeq_feat> UpdateFeat(CRef<objects::CSeq_loc> changed_loc, const objects::CSeq_feat &feat,
    bool comment, objects::CScope &scope)
{
    CRef<objects::CSeq_feat> new_feat(new objects::CSeq_feat());
    new_feat->Assign(feat);
    new_feat->SetLocation(*changed_loc);
    if (comment)
        AddComment(new_feat);
    new_feat->SetPartial(new_feat->GetLocation().IsPartialStart(objects::eExtreme_Positional) || new_feat->GetLocation().IsPartialStop(objects::eExtreme_Positional));
    if (new_feat->GetData().IsCdregion())
    {
        try
        {
            new_feat->SetData().SetCdregion().SetFrame(objects::CSeqTranslator::FindBestFrame(*new_feat, scope));
        }
        catch (CSeqVectorException &e) {}
    }    
    return new_feat;
}

void RemapOtherProtFeats(const objects::CSeq_feat &old_cds, objects::CSeq_feat &cds, CBioseq_Handle bh, CRef<CCmdComposite> composite, bool &any_actions)
{
    CScope &scope = bh.GetScope();
    CSeq_loc_Mapper map_to_nuc(old_cds, CSeq_loc_Mapper::eProductToLocation, &scope);
    map_to_nuc.SetMergeAbutting();
    CSeq_loc_Mapper map_to_prot(cds, CSeq_loc_Mapper::eLocationToProduct, &scope);
    map_to_prot.SetMergeAbutting();

    for (CFeat_CI feat_ci(bh); feat_ci; ++feat_ci)
    {
        CSeq_feat_Handle fh = feat_ci->GetSeq_feat_Handle();
        if (!fh.IsPlainFeat() || fh.GetFeatSubtype() == CSeqFeatData::eSubtype_prot)
            continue;

        const CSeq_loc &loc = feat_ci->GetLocation();
        CRef<CSeq_loc> nuc_loc = map_to_nuc.Map(loc);
        CRef<CSeq_loc> prot_loc = map_to_prot.Map(*nuc_loc);
        if (prot_loc->IsNull())
        {
            CIRef<IEditCommand> delFeat(new CCmdDelSeq_feat(fh));
            composite->AddCommand(*delFeat);
            any_actions = true;
        }
        else
        {
            if (nuc_loc->GetStart(eExtreme_Positional) < cds.GetLocation().GetStart(eExtreme_Positional) && cds.GetLocation().IsPartialStart(eExtreme_Positional)
                && old_cds.GetLocation().GetStart(eExtreme_Positional) != cds.GetLocation().GetStart(eExtreme_Positional))
                prot_loc->SetPartialStart(true, eExtreme_Positional);
            else
                prot_loc->SetPartialStart(loc.IsPartialStart(eExtreme_Positional), eExtreme_Positional);

            if (nuc_loc->GetStop(eExtreme_Positional) > cds.GetLocation().GetStop(eExtreme_Positional) && cds.GetLocation().IsPartialStop(eExtreme_Positional)
                && old_cds.GetLocation().GetStop(eExtreme_Positional) != cds.GetLocation().GetStop(eExtreme_Positional))
                prot_loc->SetPartialStop(true, eExtreme_Positional);
            else
                prot_loc->SetPartialStop(loc.IsPartialStop(eExtreme_Positional), eExtreme_Positional);

            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(feat_ci->GetOriginalFeature());
            new_feat->SetLocation().Assign(*prot_loc);
            new_feat->SetPartial(new_feat->GetLocation().IsPartialStart(eExtreme_Positional) || new_feat->GetLocation().IsPartialStop(eExtreme_Positional));

            CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_feat));
            composite->AddCommand(*chgFeat);
            any_actions = true;
        }
    }
}

void RemapOtherProtFeats(const objects::CSeq_feat &old_cds, objects::CSeq_feat &cds, CSeq_feat_Handle fh, vector<CRef<CSeq_feat> > &other_prot_feats, 
                         objects::CObject_id::TId &max_feat_id, map<CObject_id::TId, CObject_id::TId> &old_to_new, bool create_xref_map)
{
    CScope &scope = fh.GetScope();
    CSeq_loc_Mapper map_to_nuc(old_cds, CSeq_loc_Mapper::eProductToLocation, &scope);
    map_to_nuc.SetMergeAbutting();
    CSeq_loc_Mapper map_to_prot(cds, CSeq_loc_Mapper::eLocationToProduct, &scope);
    map_to_prot.SetMergeAbutting();


    const CSeq_loc &loc = fh.GetLocation();
    CRef<CSeq_loc> nuc_loc = map_to_nuc.Map(loc);
    CRef<CSeq_loc> prot_loc = map_to_prot.Map(*nuc_loc);

    if (!prot_loc->IsNull())
    {
        if (nuc_loc->GetStart(eExtreme_Positional) < cds.GetLocation().GetStart(eExtreme_Positional) && cds.GetLocation().IsPartialStart(eExtreme_Positional)
            && old_cds.GetLocation().GetStart(eExtreme_Positional) != cds.GetLocation().GetStart(eExtreme_Positional))
            prot_loc->SetPartialStart(true, eExtreme_Positional);
        else
            prot_loc->SetPartialStart(loc.IsPartialStart(eExtreme_Positional), eExtreme_Positional);

        if (nuc_loc->GetStop(eExtreme_Positional) > cds.GetLocation().GetStop(eExtreme_Positional) && cds.GetLocation().IsPartialStop(eExtreme_Positional)
            && old_cds.GetLocation().GetStop(eExtreme_Positional) != cds.GetLocation().GetStop(eExtreme_Positional))
            prot_loc->SetPartialStop(true, eExtreme_Positional);
        else
            prot_loc->SetPartialStop(loc.IsPartialStop(eExtreme_Positional), eExtreme_Positional);

        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(*fh.GetOriginalSeq_feat());
        if (new_feat->IsSetId() && new_feat->GetId().IsLocal() && new_feat->GetId().GetLocal().IsId())
        {
            ++max_feat_id;
            if (create_xref_map)
                old_to_new[new_feat->GetId().GetLocal().GetId()] = max_feat_id;
            new_feat->SetId().SetLocal().SetId(max_feat_id);
        }
        if (!create_xref_map)
        {
            s_ReplaceFeatureIdXref(*new_feat, old_to_new);
        }
        new_feat->SetLocation().Assign(*prot_loc);
        new_feat->SetPartial(new_feat->GetLocation().IsPartialStart(eExtreme_Positional) || new_feat->GetLocation().IsPartialStop(eExtreme_Positional));

        other_prot_feats.push_back(new_feat);
    }
}

static CRef<CCmdComposite> ConvertRawToDeltaByNsCommand_impl(const CSeq_entry_Handle& seh,
                                                             long min_unknown, int max_unknown, long min_known, int max_known, bool adjust_cds, bool keep_gap_length,
                                                             bool& remove_alignments, int& count,
                                                             bool is_assembly_gap, int gap_type, int linkage, int linkage_evidence,
                                                             map<CObject_id::TId, CObject_id::TId> &old_to_new, bool create_xref_map)
{
    CRef<CCmdComposite> composite(new CCmdComposite("Add assembly gaps by Ns"));
    CRef<CCmdComposite> local_cmd(new CCmdComposite("Change seq-inst to facilitate retranslation"));
    map<CBioseq_Handle, pair<vector < pair<int, int> >, vector<bool> > > map_gaps;
    set<const CSeq_align*> deleted_aligns;
    count = 0;
    for (CBioseq_CI bi(seh, CSeq_inst::eMol_na); bi; ++bi) {
        CBioseq_Handle bsh = *bi;
        vector < pair<int, int> > start_length_locs;
        vector<bool> vec_is_known;
        GapLocationsFromNs(bsh.GetCompleteBioseq()->GetInst(), min_unknown, max_unknown, min_known, max_known, start_length_locs, vec_is_known);
        if (start_length_locs.empty())
            continue;

        CRef<CSeq_inst> new_inst(new CSeq_inst());
        new_inst->Assign(bsh.GetCompleteBioseq()->GetInst());
        edit::ConvertRawToDeltaByNs(*new_inst, min_unknown, max_unknown, min_known, max_known, is_assembly_gap, gap_type, linkage, linkage_evidence); // TODO
        if (!keep_gap_length)
            edit::NormalizeUnknownLengthGaps(*new_inst);
        CRef<CCmdChangeBioseqInst> cmd(new CCmdChangeBioseqInst(bsh, *new_inst));
        local_cmd->AddCommand(*cmd);
        count++;

        CAlign_CI align_ci(seh, CSeq_annot::C_Data::e_Align);
        for (; align_ci; ++align_ci)
        {
            if (!align_ci->IsSetSegs())
                continue;
            CSeq_align_Handle ah = align_ci.GetSeq_align_Handle();

            if (!align_ci->GetSegs().IsDenseg()) // TODO only handling Denseg right now
                continue;

            UpdateDensegAlignment(ah, bsh, composite, start_length_locs, vec_is_known, deleted_aligns);
        }
        map_gaps[bsh] = make_pair(start_length_locs, vec_is_known);
    }
    composite->AddCommand(*local_cmd);
    CObject_id::TId max_feat_id = CFixFeatureId::s_FindHighestFeatureId(seh);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(seh);
    local_cmd->Execute();
    UpdateFeaturesForGaps(map_gaps, composite, adjust_cds, keep_gap_length, create_general_only, max_feat_id, old_to_new, create_xref_map);
    local_cmd->Unexecute();
    remove_alignments = (!deleted_aligns.empty());
    return composite;
}

CRef<CCmdComposite> ConvertRawToDeltaByNsCommand(const CSeq_entry_Handle& seh,
                                                 long min_unknown, int max_unknown, long min_known, int max_known, bool adjust_cds, bool keep_gap_length,
                                                 bool& remove_alignments, int& count,
                                                 bool is_assembly_gap, int gap_type, int linkage, int linkage_evidence)
{
    map<CObject_id::TId, CObject_id::TId> old_to_new;
    bool create_xref_map = true;
    ConvertRawToDeltaByNsCommand_impl(seh, min_unknown, max_unknown, min_known, max_known, adjust_cds, keep_gap_length, remove_alignments, count,
                                      is_assembly_gap, gap_type, linkage, linkage_evidence,
                                      old_to_new, create_xref_map);
    create_xref_map = false;
    return  ConvertRawToDeltaByNsCommand_impl(seh, min_unknown, max_unknown, min_known, max_known, adjust_cds, keep_gap_length, remove_alignments, count,
                                              is_assembly_gap, gap_type, linkage, linkage_evidence,
                                              old_to_new, create_xref_map);
}

END_SCOPE(NRawToDeltaSeq)
END_NCBI_SCOPE

