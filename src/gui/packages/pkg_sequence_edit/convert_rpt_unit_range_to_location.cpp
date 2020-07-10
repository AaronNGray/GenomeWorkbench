/*  $Id: convert_rpt_unit_range_to_location.cpp 36583 2016-10-12 15:11:14Z evgeniev $
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
#include <objmgr/feat_ci.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <gui/packages/pkg_sequence_edit/convert_rpt_unit_range_to_location.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;

void CConvertRptUnitRangeToLoc::apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor)
{
    if (!tse) 
        return;

    CRef<CCmdComposite> composite( new CCmdComposite("Convert to Delayed Gen-Prod-Set Qualifiers") );   
    for (CFeat_CI feat_ci(tse); feat_ci; ++feat_ci)
    {        
        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
        if (orig.IsSetData() && orig.GetData().IsImp() && orig.GetData().GetImp().IsSetKey() &&
            orig.GetData().GetImp().GetKey() == "repeat_region")
        {
            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(orig);
            bool changed = false; 
            int start, stop;
            EDIT_EACH_GBQUAL_ON_SEQFEAT(qual, *new_feat)
            {
                if ((*qual)->IsSetQual() && (*qual)->GetQual() == "rpt_unit_range" && (*qual)->IsSetVal())
                    {
                        string loc_str = (*qual)->GetVal();
                        list<string> arr;
                        NStr::Split(loc_str, ".", arr, NStr::fSplit_Tokenize);
                        if (arr.size() == 2)
                        {
                            start = NStr::StringToInt(arr.front());
                            stop = NStr::StringToInt(arr.back());
                            ERASE_GBQUAL_ON_SEQFEAT(qual, *new_feat);
                            changed = true;
                        }
                    }                 
            }           
            if (changed)
            {
                const CSeq_loc &feat_loc = new_feat->GetLocation();
                CRef<CSeq_loc> interval(new CSeq_loc);
                interval->SetInt().SetFrom(start-1);
                interval->SetInt().SetTo(stop-1);
                CRef<CSeq_id> id(new CSeq_id);
                id->Assign(*feat_loc.GetId());
                interval->SetInt().SetId(*id);
                new_feat->SetLocation().Assign(*interval); 
                composite->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
            }    
        }
    }
    cmdProcessor->Execute(composite.GetPointer());
}


END_NCBI_SCOPE
