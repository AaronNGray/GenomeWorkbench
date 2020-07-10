/*  $Id: adjust_feats_for_introns.cpp 37208 2016-12-15 15:58:06Z filippov $
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
#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/adjust_feats_for_introns.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

CRef<CCmdComposite> CAdjustFeatsForIntrons::apply(CSeq_entry_Handle tse, CSeqFeatData::ESubtype subtype)
{
    if (!tse) 
        return CRef<CCmdComposite>(NULL);
    bool modified = false;
    CScope &scope = tse.GetScope();
    CRef<CCmdComposite> composite( new CCmdComposite("Adjust Features for Introns") );   
    for (CFeat_CI feat_ci(tse, subtype); feat_ci; ++feat_ci)
    {
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(feat_ci->GetOriginalFeature());
        CBioseq_Handle bsh = scope.GetBioseqHandle(feat_ci->GetLocation());
        if (!bsh)
            continue;
        CRef<CSeq_loc> new_loc(new CSeq_loc);
        new_loc->Assign(feat_ci->GetLocation());
        for (CFeat_CI intron_ci(bsh, CSeqFeatData::eSubtype_intron); intron_ci; ++intron_ci)
        {
            new_loc = sequence::Seq_loc_Subtract(*new_loc, intron_ci->GetLocation(), CSeq_loc::fMerge_All|CSeq_loc::fSort, &scope);
        }
        if (!new_loc->Equals(feat_ci->GetLocation()))
        {
            if ( sequence::GetCoverage(*new_loc, &scope) == 0)
            {
                composite->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(feat_ci->GetSeq_feat_Handle())));           
            }
            else
            {
                new_feat->SetLocation(*new_loc);
                new_feat->SetPartial(new_feat->GetLocation().IsPartialStart(eExtreme_Positional) || new_feat->GetLocation().IsPartialStop(eExtreme_Positional));
                composite->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
            }
            modified = true;
        }
    }
   
    if (!modified)
        composite.Reset();
    return composite;
}


END_NCBI_SCOPE
