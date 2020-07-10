/*  $Id: gene_feats_from_xrefs.cpp 37875 2017-02-27 13:40:33Z filippov $
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
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/util/feature.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/gene_feats_from_xrefs.hpp>

#include <wx/msgdlg.h> 

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);


CRef<CCmdComposite> CGeneFeatFromXrefs::GetCommand(CSeq_entry_Handle seh)
{
    int answer = wxMessageBox(_("Remove original Xref?"), _("Remove xrefs after creating Genes"), wxYES_NO |  wxICON_QUESTION);
    bool remove_xref = (answer == wxYES);

    CRef<CCmdComposite> cmd(new CCmdComposite("Gene Features from Xrefs"));
    for (CFeat_CI feat_ci(seh); feat_ci; ++feat_ci)
    {
        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
        CRef<CSeq_feat> changed_feat(new CSeq_feat);
        changed_feat->Assign(orig);
        bool modified = false;
        EDIT_EACH_SEQFEATXREF_ON_SEQFEAT(xref, *changed_feat)
        {
            if ((*xref)->IsSetData() && (*xref)->GetData().IsGene())
            {
                CRef<CSeq_feat> new_feat(new CSeq_feat());

                CRef<CSeqFeatData> data(new CSeqFeatData);
                data->Assign((*xref)->GetData());
                new_feat->SetData(*data);

                CRef<CSeq_loc> loc(new CSeq_loc());
                loc->Assign(orig.GetLocation());
                new_feat->SetLocation(*loc);
                new_feat->SetPartial(loc->IsPartialStart(eExtreme_Biological) || loc->IsPartialStop(eExtreme_Biological));

                if (orig.IsSetPseudo() && orig.GetPseudo())
                    new_feat->SetPseudo(true);

                CSeq_entry_Handle parent = seh.GetScope().GetBioseqHandle(*loc).GetSeq_entry_Handle();
                cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(parent, *new_feat)));
                if (remove_xref)
                {
                    ERASE_SEQFEATXREF_ON_SEQFEAT(xref, *changed_feat);
                    modified = true;
                }
            }
        }
        if (modified)
        {
            if (changed_feat->IsSetXref() && changed_feat->GetXref().empty())
                changed_feat->ResetXref();
            cmd->AddCommand(*CRef< CCmdChangeSeq_feat >(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(),*changed_feat)));
        }
    }
    return cmd;
}

CRef<CCmdComposite> CGeneXrefsFromFeats::GetCommand(CSeq_entry_Handle seh)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Gene Xrefs From Features"));
    for (CFeat_CI feat_ci(seh); feat_ci; ++feat_ci)
    {
        CSeq_feat_Handle fh = feat_ci->GetSeq_feat_Handle();
        if (fh.GetFeatSubtype() == CSeqFeatData::eSubtype_gene)
            continue;

        CMappedFeat gene = feature::GetBestGeneForFeat(*feat_ci);
        if (!gene)
            continue;
        const CSeq_feat& feat = gene.GetMappedFeature();
        if (!feat.IsSetData() || !feat.GetData().IsGene())
            continue;

        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
        CRef<CSeq_feat> changed_feat(new CSeq_feat);
        changed_feat->Assign(orig);
        CRef<CGene_ref> gene_ref(new CGene_ref);
        gene_ref->Assign(feat.GetData().GetGene());
        changed_feat->SetGeneXref(*gene_ref);

        cmd->AddCommand(*CRef< CCmdChangeSeq_feat >(new CCmdChangeSeq_feat(fh,*changed_feat)));
    }
    return cmd;
}

END_NCBI_SCOPE
