/*  $Id: cmd_propagate_feat.cpp 42666 2019-04-01 14:27:25Z filippov $
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
 * Authors:  Frank Ludwig, adapting code by Roman Katargin
 */


#include <ncbi_pch.hpp>
#include <objtools/edit/feature_propagate.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/objutils/cmd_add_cds.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_add_seqentry.hpp>
#include <gui/objutils/cmd_propagate_feat.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


void PropagateToTarget(CRef<CCmdComposite> composite, const vector<CConstRef<CSeq_feat>>& feat_list,
                       CBioseq_Handle src, CBioseq_Handle target, const CSeq_align& align,
                       bool stop_at_stop, bool cleanup_partials, bool merge_abutting, bool extend_over_gaps, bool create_general_only,
                       CMessageListener_Basic* listener, CObject_id::TId* feat_id)
{
    CSeq_entry_Handle seh = target.GetSeq_entry_Handle();
    CBioseq_set_Handle bssh = target.GetParentBioseq_set();
    if (bssh && bssh.CanGetClass() && bssh.GetClass() ==  CBioseq_set::eClass_nuc_prot)
    {
        seh =  bssh.GetParentEntry();
    }
    edit::CFeaturePropagator propagator(src, target, align, stop_at_stop, cleanup_partials, merge_abutting, extend_over_gaps, listener, feat_id);
    vector<CRef<CSeq_feat>> prop_feats = propagator.PropagateFeatureList(feat_list);
    int offset = 1;

    // The product of the propagated coding region is not set
    // The location of propagated protein points to the 'src' sequence
    auto it = prop_feats.begin();
    while (it != prop_feats.end()) {
        auto& feat = **it;
        //LOG_POST(Info << "To be propagated:\n" << MSerial_AsnText << feat);

        if (feat.IsSetData() && feat.GetData().IsCdregion()) {
            CRef<CSeq_feat> prot_feat;
            auto prot_it = it;
            ++prot_it;
            if (prot_it != prop_feats.end() && 
                (*prot_it)->IsSetData() && (*prot_it)->GetData().IsProt()) {
                prot_feat.Reset(new CSeq_feat);
                prot_feat->Assign(**prot_it);
                ++it;
            }
            vector<CRef<CSeq_id>> new_prot_id;
            bool has_product = !CCmdCreateCDS::s_GetProductSequence(feat, target.GetScope()).empty();
            if (has_product) {
                string id_label;
                new_prot_id.push_back(objects::edit::GetNewProtId(target, offset, id_label, create_general_only));
                offset++;
            }
            CRef<CCmdCreateCDS> add_cds(new CCmdCreateCDS(seh, feat, prot_feat, new_prot_id));
            composite->AddCommand(*add_cds);
        }
        else {
            CRef<CCmdCreateFeatBioseq> add_f(new CCmdCreateFeatBioseq(target, feat));
            composite->AddCommand(*add_f);
        }
        ++it;
    }
}

END_NCBI_SCOPE
