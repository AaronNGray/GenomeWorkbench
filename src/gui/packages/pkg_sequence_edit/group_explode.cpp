/*  $Id: group_explode.cpp 42183 2019-01-09 17:19:23Z filippov $
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
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/objutils/cmd_add_cds.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_del_bioseq.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <wx/msgdlg.h>
#include <gui/packages/pkg_sequence_edit/group_explode.hpp>

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

void CGroupExplode::GetSeqFeats( IWorkbench* wb, vector<const objects::CSeq_feat*> &result )
{
    if (!wb) 
    {
        return;
    }

    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = wb->GetServiceByType<CSelectionService>();
    if (!sel_srv) 
    {
        return;
    }
    sel_srv->GetCurrentSelection(objects);
    if (objects.empty()) 
    {
        return;
    }

    NON_CONST_ITERATE (TConstScopedObjects, it, objects) 
    {
        const CObject* ptr = it->object.GetPointer();
        const objects::CSeq_feat* seqfeat = dynamic_cast<const objects::CSeq_feat*>(ptr);
        if (seqfeat)
        {
            result.push_back(seqfeat);
        }
    }
}

void CGroupExplode::apply( IWorkbench* wb,  ICommandProccessor* cmd_processor, CScope &scope)
{
    vector<const objects::CSeq_feat*> orig_vec;
    GetSeqFeats(wb, orig_vec);
    if (orig_vec.empty())
    {
        wxMessageBox(wxT("No feature selected"), wxT("Error"), wxOK | wxICON_ERROR);
        return;
    }
    bool create_general_only = false;
    CScope::TTSE_Handles vec_tse;
    scope.GetAllTSEs(vec_tse, CScope::eAllTSEs);
    if (!vec_tse.empty())
        create_general_only = objects::edit::IsGeneralIdProtPresent(vec_tse.front());

    CRef<CCmdComposite> composite(new CCmdComposite("Group Explode"));
    ITERATE(vector<const objects::CSeq_feat*>, orig_it, orig_vec)
    {
        CRef<CCmdComposite> add_locs(new CCmdComposite("Add location"));
        const objects::CSeq_feat* orig = *orig_it;
        CSeq_feat_Handle fh = scope.GetSeq_featHandle(*orig, CScope::eMissing_Null);
        if (!fh)
        {
            continue;
        }
        bool is_exon = false;
        int exon_number = 0;
        if (orig->IsSetData() && orig->GetData().IsImp() && orig->GetData().GetImp().IsSetKey() &&
            orig->GetData().GetImp().GetKey() == "exon")
        {
            is_exon = true;
            FOR_EACH_GBQUAL_ON_SEQFEAT(qual,*orig)
            {
                if ((*qual)->IsSetQual() && (*qual)->GetQual() == "number" && (*qual)->IsSetVal())
                {
                    try {
                        exon_number = NStr::StringToUInt((*qual)->GetVal());
                    } catch (const CException&) {
                        // not in a numeric format
                    }
                    break;
                }
            }
        }
        bool is_cds = false;
        objects::CBioseq_Handle product;
        if (orig->IsSetData() && orig->GetData().IsCdregion())
        {
            is_cds = true;
            product = scope.GetBioseqHandle(orig->GetProduct());      
        }
        const CSeq_loc& feat_loc = orig->GetLocation();
        size_t num_locs = 0;
        for(CSeq_loc_CI loc_it(feat_loc, CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Positional); loc_it; ++loc_it)     
        {
            CSeq_loc_CI::TRange feat_range = loc_it.GetRange();
            TSeqPos feat_start = feat_range.GetFrom();
            TSeqPos feat_stop = feat_range.GetTo();
            CRef<CSeq_loc> interval(new CSeq_loc);
            interval->SetInt().SetFrom(feat_start);
            interval->SetInt().SetTo(feat_stop);
            CRef<CSeq_id> id(new CSeq_id);
            id->Assign(*feat_loc.GetId());
            interval->SetInt().SetId(*id);
            if (loc_it.IsSetStrand())
                interval->SetInt().SetStrand(loc_it.GetStrand());

            if (loc_it.GetPos() == 0)
                interval->SetPartialStart(feat_loc.IsPartialStart(objects::eExtreme_Positional),objects::eExtreme_Positional);
            if (loc_it.GetPos() + 1 == loc_it.GetSize())
                interval->SetPartialStop(feat_loc.IsPartialStop(objects::eExtreme_Positional),objects::eExtreme_Positional);

            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(*orig);
            new_feat->SetLocation().Assign(*interval);   
            new_feat->SetPartial(new_feat->GetLocation().IsPartialStart(eExtreme_Positional) || new_feat->GetLocation().IsPartialStop(eExtreme_Positional));
    
            if (is_exon && exon_number > 0)
            {
                EDIT_EACH_GBQUAL_ON_SEQFEAT(qual,*new_feat)
                {
                    if ((*qual)->IsSetQual() && (*qual)->GetQual() == "number" && (*qual)->IsSetVal())
                    {                        
                        (*qual)->SetVal(NStr::NumericToString(exon_number+num_locs));
                        break;
                    }
                }
            }
            CSeq_entry_Handle seh = fh.GetAnnot().GetParentEntry();
            if (is_cds)
            {
                CRef<objects::CSeq_feat> prot_feat;
                if (product)
                {
                    objects::CFeat_CI prot_feat_ci(product, objects::SAnnotSelector(objects::CSeqFeatData::eSubtype_prot));
                    if (prot_feat_ci) 
                    {
                        prot_feat.Reset(new objects::CSeq_feat());
                        prot_feat->Assign(*(prot_feat_ci->GetSeq_feat()));
                    }
                }        
                new_feat->SetData().SetCdregion().SetFrame(objects::CSeqTranslator::FindBestFrame(*new_feat,scope));
                CRef<CCmdAddCDS> cmd(new CCmdAddCDS(seh, *new_feat, prot_feat, create_general_only));
                add_locs->AddCommand(*cmd); 
            }
            else
            {
                add_locs->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat))); 
            }
            num_locs++;
        }
        if (num_locs > 1)
        {
            composite->AddCommand(*add_locs);
            composite->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(fh)));
            if (is_cds && product)
                composite->AddCommand(*CRef<CCmdDelBioseqInst>(new CCmdDelBioseqInst(product)));
        }
    }
    cmd_processor->Execute(composite);
}


static const char* RNA_words[] = {
  "ITS1",
  "ITS2",
  "internal transcribed spacer 1",
  "internal transcribed spacer 2",
  "5.8S",
  "16S",
  "18S",  
  "28S",
  "5.8S ribosomal RNA",
  "16S ribosomal RNA",
  "18S ribosomal RNA",
  "28S ribosomal RNA",
  NULL
};

void CExplodeRNAFeats::RNAWordsFromString(string name, vector<string> &names)
{
    NStr::ReplaceInPlace(name, " and ", kEmptyStr);
    while (!name.empty())
    {
        NStr::TruncateSpacesInPlace(name, NStr::eTrunc_Begin);
        NStr::TrimPrefixInPlace(name, ",");
        NStr::TrimPrefixInPlace(name, "-");
        NStr::TrimPrefixInPlace(name, ";");
        NStr::TrimPrefixInPlace(name, ":");
        NStr::TrimPrefixInPlace(name, ".");
        NStr::TruncateSpacesInPlace(name, NStr::eTrunc_Begin);
        string str;
        for (size_t i = 0; RNA_words[i] != NULL; i++)
        {
            if (NStr::StartsWith(name, RNA_words[i]))
            {
                str = RNA_words[i];
                if (str ==  "5.8S" || str == "16S" || str ==  "18S" || str ==  "28S")
                    str += " ribosomal RNA";
                NStr::TrimPrefixInPlace(name, RNA_words[i]);
                string suffix;
                for (size_t j = 0; j < name.size(); j++)
                {
                    char c = name[j];
                    if (ispunct(c))
                        break;
                    suffix += c;
                }
                if (!suffix.empty())
                {
                    NStr::TrimPrefixInPlace(name, suffix);
                    str += suffix;
                }
                break;
            }
        }
        if (!str.empty())
            names.push_back(str);
        else
            break;
    }
}

CRef<CCmdComposite> CExplodeRNAFeats::apply(objects::CSeq_entry_Handle tse)
{
    bool modified = false;
    CRef<CCmdComposite> composite(new CCmdComposite("Explode RNA Features"));
    for (CFeat_CI feat_ci(tse); feat_ci; ++feat_ci)
    {
        CSeq_feat_Handle fh = feat_ci->GetSeq_feat_Handle();

        if (!feat_ci->GetOriginalFeature().IsSetData() || !feat_ci->GetOriginalFeature().GetData().IsRna())
            continue;

        CRef<CCmdComposite> add_locs(new CCmdComposite("Add location"));
        const CSeq_loc& feat_loc = fh.GetLocation();
        size_t num_locs = 0;
        string name; 
        if (fh.IsSetComment())
            name = fh.GetComment();
        if (name.empty() && feat_ci->GetOriginalFeature().IsSetData() && feat_ci->GetOriginalFeature().GetData().IsRna())
            name = feat_ci->GetOriginalFeature().GetData().GetRna().GetRnaProductName();
            
        vector<string> names;
        RNAWordsFromString(name, names);

        for(CSeq_loc_CI loc_it(feat_loc, CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Positional); loc_it; ++loc_it)     
        {
            CSeq_loc_CI::TRange feat_range = loc_it.GetRange();
            TSeqPos feat_start = feat_range.GetFrom();
            TSeqPos feat_stop = feat_range.GetTo();
            CRef<CSeq_loc> interval(new CSeq_loc);
            interval->SetInt().SetFrom(feat_start);
            interval->SetInt().SetTo(feat_stop);
            CRef<CSeq_id> id(new CSeq_id);
            id->Assign(*feat_loc.GetId());
            interval->SetInt().SetId(*id);
            if (loc_it.IsSetStrand())
                interval->SetInt().SetStrand(loc_it.GetStrand());

            if (loc_it.GetPos() == 0)
                interval->SetPartialStart(feat_loc.IsPartialStart(objects::eExtreme_Positional),objects::eExtreme_Positional);
            if (loc_it.GetPos() + 1 == loc_it.GetSize())
                interval->SetPartialStop(feat_loc.IsPartialStop(objects::eExtreme_Positional),objects::eExtreme_Positional);

            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(feat_ci->GetOriginalFeature());
            new_feat->SetLocation().Assign(*interval);   
            new_feat->SetPartial(new_feat->GetLocation().IsPartialStart(eExtreme_Positional) || new_feat->GetLocation().IsPartialStop(eExtreme_Positional));
            string reminder;
            if (num_locs < names.size())
            {
                string str = names[num_locs];
                new_feat->SetData().SetRna().SetRnaProductName(str, reminder);
                if (NStr::StartsWith(str, "internal transcribed spacer"))
                {
                    new_feat->SetData().SetRna().SetType(CRNA_ref::eType_miscRNA);
                }
                else
                {
                    new_feat->SetData().SetRna().SetType(CRNA_ref::eType_rRNA);
                }
            }

            CSeq_entry_Handle seh = fh.GetAnnot().GetParentEntry();
            add_locs->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat))); 
            num_locs++;
        }
        if (num_locs > 1)
        {
            composite->AddCommand(*add_locs);
            composite->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(fh)));
            modified = true;
        }
    }
    if (!modified)
        composite.Reset();
    return composite;
}



END_NCBI_SCOPE
