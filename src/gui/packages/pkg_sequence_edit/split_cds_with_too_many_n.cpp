/* $Id: split_cds_with_too_many_n.cpp 42167 2019-01-08 17:17:20Z filippov $
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

#include <objects/seq/Seq_data.hpp>
#include <objects/general/Dbtag.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/seq_vector.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/general/Object_id.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objtools/edit/loc_edit.hpp>
#include <objtools/edit/seq_entry_edit.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_add_cds.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/cmd_del_bioseq.hpp>
#include <gui/objutils/cmd_add_seqentry.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <util/sequtil/sequtil_convert.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/packages/pkg_sequence_edit/raw_seq_to_delta_by_loc.hpp>
#include <gui/packages/pkg_sequence_edit/raw_seq_to_delta_by_n.hpp>
#include <gui/packages/pkg_sequence_edit/split_cds_with_too_many_n.hpp>

BEGIN_NCBI_SCOPE

CRef<CCmdComposite> CSplitCDSwithTooManyXs::apply(objects::CSeq_entry_Handle tse)
{
    CRef<CCmdComposite> composite(new CCmdComposite("Split CDS Features with Too Many Xs")); 
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(tse);
    for (objects::CBioseq_CI bi(tse, objects::CSeq_inst::eMol_na); bi; ++bi) 
    {
        objects::CBioseq_Handle bsh = *bi;
        vector < pair<int,int> > gaps;
        GapLocationsFromNs(bsh, gaps);
        CRef<objects::CSeq_loc> loc = bsh.GetRangeSeq_loc(0,0);
        TSeqPos seq_start = loc->GetStart(objects::eExtreme_Positional);
        TSeqPos seq_length = bsh.GetBioseqLength();
        objects::SAnnotSelector sel(objects::CSeqFeatData::e_Cdregion);
        objects::CScope &scope = bsh.GetScope();
        
        for (objects::CFeat_CI feat_it(scope, *loc, sel);feat_it;  ++feat_it) 
        {
            if (TooManyXs(feat_it->GetSeq_feat_Handle()))
            {
                AdjustSingleCDS(*feat_it, seq_start, seq_length, scope, gaps, composite, create_general_only);
            }
        }
    }
    return composite;
}

bool CSplitCDSwithTooManyXs::TooManyXs(objects::CSeq_feat_Handle fh)
{
    objects::CBioseq_Handle prot_bsh = fh.GetScope().GetBioseqHandle(fh.GetProduct());
    if (prot_bsh && prot_bsh.IsSetInst()) 
    {
        const objects::CSeq_inst& inst = prot_bsh.GetInst();

        if (!inst.IsSetRepr() || inst.GetRepr() != objects::CSeq_inst::eRepr_raw || !inst.IsSetSeq_data()) 
            return false;
    
        string iupacaa;

        switch(inst.GetSeq_data().Which()) 
        {
        case objects::CSeq_data::e_Iupacaa:
            iupacaa = inst.GetSeq_data().GetIupacaa();
            break;
        case objects::CSeq_data::e_Ncbieaa:
            CSeqConvert::Convert(inst.GetSeq_data().GetNcbieaa().Get(), CSeqUtil::e_Ncbieaa,
                                 0, inst.GetLength(), iupacaa, CSeqUtil::e_Iupacaa);
            break;
        case objects::CSeq_data::e_Ncbi8aa:
            CSeqConvert::Convert(inst.GetSeq_data().GetNcbi8aa().Get(), CSeqUtil::e_Ncbi8aa,
                                 0, inst.GetLength(), iupacaa, CSeqUtil::e_Iupacaa);
            break;
        case objects::CSeq_data::e_Ncbistdaa:
            CSeqConvert::Convert(inst.GetSeq_data().GetNcbistdaa().Get(), CSeqUtil::e_Ncbistdaa,
                                 0, inst.GetLength(), iupacaa, CSeqUtil::e_Iupacaa);
            break;
        default:
            break;
        }
  
        size_t num_x = 0, num_non_x = 0;
        ITERATE(string, it, iupacaa) 
        {
            if ((*it) == 'X') 
            {
                num_x++;
            } 
            else 
            {
                num_non_x++;
            }
        }
        return num_x > num_non_x;
    }
    return false;
}

void CSplitCDSwithTooManyXs::AdjustSingleCDS(const objects::CMappedFeat& feat, TSeqPos seq_start, TSeqPos seq_length, objects::CScope &scope, const vector < pair<int,int> > &gaps, 
                                             CRef<CCmdComposite> composite, bool create_general_only)
{
    if (feat.GetOriginalFeature().IsSetData())
        {
            objects::CSeq_feat_Handle mrna = objects::feature::GetBestMrnaForCds(feat).GetSeq_feat_Handle();

            const objects::CSeq_loc& feat_loc = feat.GetLocation();
            vector<CRef<objects::CSeq_loc> > locations;
            bool comment = false;
            FindNewLocations(feat_loc, gaps, seq_start, seq_length, locations, comment);                     

            int offset = 0;
            vector<string> id_labels;
            vector<string> transcript_labels;
            vector<int> offsets;
            for (size_t j = 0; j < locations.size(); j++)
            {
                if (!locations[j]->IsNull())
                {
                    offset++;
                    string id_label;
                    CRef<objects::CSeq_feat> add_feat = NRawToDeltaSeq::UpdateFeat(locations[j], feat.GetOriginalFeature(), comment, scope);
                    const objects::CSeq_annot_Handle& annot_handle = feat.GetAnnot();
                    objects::CSeq_entry_Handle seh = annot_handle.GetParentEntry();
                    CRef<objects::CSeq_feat> prot_feat;
                    objects::CBioseq_Handle product = scope.GetBioseqHandle(add_feat->GetProduct());
                    vector<CRef<objects::CSeq_id> > new_prot_id;
                    if (product)
                    {
                        objects::CFeat_CI prot_feat_ci(product, objects::SAnnotSelector(objects::CSeqFeatData::eSubtype_prot));
                        if (prot_feat_ci) 
                        {
                            prot_feat.Reset(new objects::CSeq_feat());
                            prot_feat->Assign(*(prot_feat_ci->GetSeq_feat()));
                        }
                        new_prot_id = objects::edit::GetNewProtIdFromExistingProt(product, offset, id_label);
                    }        
                    else
                    {
                        objects::CBioseq_Handle bsh = scope.GetBioseqHandle(feat.GetLocation());
                        new_prot_id.push_back(objects::edit::GetNewProtId(bsh, offset, id_label, create_general_only));
                    }
                    
                    string transcript_label = id_label;
                    string old_transcript_id = add_feat->GetNamedQual("orig_transcript_id");
                    if (!old_transcript_id.empty())
                    {
                        transcript_label = old_transcript_id + "_" + NStr::NumericToString(offset);
                    }
                    add_feat->RemoveQualifier("orig_protein_id"); 
                    if (mrna) 
                        add_feat->AddOrReplaceQualifier("orig_transcript_id", transcript_label);
                    else
                        add_feat->RemoveQualifier("orig_transcript_id");
                    
                    composite->AddCommand(*CRef<CCmdCreateCDS>(new CCmdCreateCDS(seh, *add_feat, prot_feat, new_prot_id)));
                    id_labels.push_back(id_label);
                    transcript_labels.push_back(transcript_label);
                    offsets.push_back(offset);
                }               
            }

            if (mrna)
            {
                const objects::CSeq_loc& mrna_loc = mrna.GetLocation();
                vector<CRef<objects::CSeq_loc> > mrna_locations;
                bool mrna_comment = false;
                FindNewLocations(mrna_loc, gaps, seq_start, seq_length, mrna_locations, mrna_comment);     
                composite->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(mrna)));
              
                for (size_t j = 0; j < mrna_locations.size(); j++)
                    if (!mrna_locations[j]->IsNull())
                    {
                        CRef<objects::CSeq_feat> add_mrna = NRawToDeltaSeq::UpdateFeat(mrna_locations[j], *mrna.GetSeq_feat(), mrna_comment, scope);
                        string id_label = add_mrna->GetNamedQual("orig_protein_id");
                        string transcript_label= add_mrna->GetNamedQual("orig_transcript_id");

                        if (j < offsets.size())
                        {
                            if (!id_label.empty())
                            {
                                id_label += "_" + NStr::NumericToString(offsets[j]);
                            }
                            else 
                            {
                                id_label = id_labels[j];
                            }

                            if (!transcript_label.empty())
                            {
                                transcript_label += "_" + NStr::NumericToString(offsets[j]);
                            }
                            else
                            {
                                transcript_label = transcript_labels[j];
                            }
                        }
                                            
                        add_mrna->AddOrReplaceQualifier("orig_protein_id", id_label);  
                        add_mrna->AddOrReplaceQualifier("orig_transcript_id", transcript_label);

                        const objects::CSeq_annot_Handle& mrna_annot_handle = mrna.GetAnnot();
                        objects::CSeq_entry_Handle mrna_seh = mrna_annot_handle.GetParentEntry();
                        composite->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(mrna_seh, *add_mrna)));
                    }
            }

            objects::CSeq_id_Handle prot_id = feat.GetProductId();
            if (prot_id) 
            {
                objects::CBioseq_Handle prot_bsh = scope.GetBioseqHandle(feat.GetProduct());
                if (prot_bsh) 
                {
                    CRef<objects::CCmdDelBioseqInst> cmd(new objects::CCmdDelBioseqInst(prot_bsh));
                    composite->AddCommand(*cmd);  
                }
            }
            composite->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(feat.GetSeq_feat_Handle())));                      
        }
}

void CSplitCDSwithTooManyXs::FindNewLocations(const objects::CSeq_loc &feat_loc, const vector < pair<int,int> > &gaps, TSeqPos seq_start, TSeqPos seq_length, vector<CRef<objects::CSeq_loc> > &locations, bool &comment)
{
    CRef<objects::CSeq_loc> new_loc(new objects::CSeq_loc);
    new_loc->Assign(feat_loc);
    vector < vector<pair<TSeqPos,TSeqPos> > > all_locs;
    vector<pair<TSeqPos,TSeqPos> > locs, old_locs;
    vector< vector<objects::ENa_strand> > all_strands;
    vector<objects::ENa_strand> strands, old_strands;
    size_t i = 0;
    size_t j = 0;

    for (objects::CSeq_loc_CI loc_it(*new_loc, objects::CSeq_loc_CI::eEmpty_Skip, objects::CSeq_loc_CI::eOrder_Positional); loc_it; ++loc_it)
    {                
        objects::CSeq_loc_CI::TRange feat_range = loc_it.GetRange();
        TSeqPos feat_start = feat_range.GetFrom() - seq_start;
        TSeqPos feat_stop = feat_range.GetTo() - seq_start;
        old_locs.push_back(pair<TSeqPos,TSeqPos>(feat_start, feat_stop));
        old_strands.push_back(loc_it.IsSetStrand() ? loc_it.GetStrand() : objects::eNa_strand_plus);
    }

    while (j < old_locs.size())
    {
        TSeqPos feat_start = old_locs[j].first;
        TSeqPos feat_stop = old_locs[j].second;
        objects::ENa_strand strand = old_strands[j];

        if ( i >= gaps.size() || feat_stop < gaps[i].first) // no inserts or replaces in this interval
        {
            locs.push_back(pair<TSeqPos,TSeqPos>(feat_start,feat_stop));
            strands.push_back(strand);
            ++j;
        }
        else  if (gaps[i].first <= feat_start && gaps[i].first + gaps[i].second - 1 >= feat_start && gaps[i].first + gaps[i].second - 1 < feat_stop) // left edge
        {
            old_locs[j].first = gaps[i].first + gaps[i].second;
            if (!locs.empty())
            {
                all_locs.push_back(locs);
                all_strands.push_back(strands);
            }
            locs.clear();
            strands.clear();
            comment = true;
            ++i;
        }
        else  if (gaps[i].first > feat_start && gaps[i].first + gaps[i].second - 1 < feat_stop) // known replace
        {
            locs.push_back(pair<TSeqPos,TSeqPos>(feat_start, gaps[i].first - 1));
            strands.push_back(strand);
            old_locs[j].first = gaps[i].first + gaps[i].second;
            all_locs.push_back(locs);
            all_strands.push_back(strands);
            locs.clear();
            strands.clear();
            comment = true;
            ++i;
        }
        else  if (gaps[i].first > feat_start && gaps[i].first < feat_stop && gaps[i].first + gaps[i].second - 1 > feat_stop) // right edge
        {
            locs.push_back(pair<TSeqPos,TSeqPos>(feat_start, gaps[i].first - 1));
            strands.push_back(strand);
            old_locs[j].second = gaps[i].first - 1;
            all_locs.push_back(locs);
            all_strands.push_back(strands);
            locs.clear();
            strands.clear();
            comment = true;
            ++j;
        }
        else  if (gaps[i].first < feat_start && gaps[i].first + gaps[i].second - 1 > feat_stop) // full replace
        {
            if (!locs.empty())
            {
                all_locs.push_back(locs);
                all_strands.push_back(strands);
            }
            locs.clear();
            strands.clear();
            comment = true;
            ++j;
        }
        else
            ++i;
    }

    if (!locs.empty())
    {
        all_locs.push_back(locs);
        all_strands.push_back(strands);
        locs.clear();
        strands.clear();
    }
    
    locations.clear();
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
                TSeqPos start = all_locs[j][i].first;
                TSeqPos end = all_locs[j][i].second;
                objects::ENa_strand strand = all_strands[j][i];
                loc_i.InsertInterval(*feat_loc.GetId(), start, end, strand);
            }
        }
        CRef<objects::CSeq_loc> changed_loc = loc_i.MakeSeq_loc()->Merge(objects::CSeq_loc::fSort, NULL);
        if (j > 0)
        {
            locations.back()->SetPartialStop(true, objects::eExtreme_Positional);
            changed_loc->SetPartialStart(true, objects::eExtreme_Positional);
        }
        locations.push_back(changed_loc);
    }
    locations.front()->SetPartialStart(feat_loc.IsPartialStart(objects::eExtreme_Positional),objects::eExtreme_Positional);
    locations.back()->SetPartialStop(feat_loc.IsPartialStop(objects::eExtreme_Positional),objects::eExtreme_Positional);
}

void CSplitCDSwithTooManyXs::GapLocationsFromNs(objects::CBioseq_Handle bsh, vector < pair<int,int> > &start_length_locs)
{
    string iupacna;
    const objects::CSeqVector seqvec( bsh, objects::CBioseq_Handle::eCoding_Iupac, objects::eNa_strand_plus);
    seqvec.GetSeqData(0, bsh.GetBioseqLength(), iupacna);
    
    size_t n_len = 0;
    size_t pos = 0;
    ITERATE(string, it, iupacna) 
    {
        if ((*it) == 'N') 
        {
            n_len++;
        } 
        else 
        {
            if (n_len > 0) 
            {
                start_length_locs.push_back(pair<int,int>(pos,n_len));
                pos += n_len;
                n_len = 0;
            }
            pos++;
        }
    }

    if (n_len > 0) 
    {
        start_length_locs.push_back(pair<int,int>(pos,n_len));        
    } 
}

END_NCBI_SCOPE
