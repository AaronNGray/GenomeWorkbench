/*  $Id: flat_feature_retrieve_job.cpp 39803 2017-11-07 15:36:58Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <objmgr/util/sequence.hpp>

#include <objtools/format/context.hpp>
#include <objtools/format/items/feature_item.hpp>

#include <gui/widgets/seq/flat_file_feature_collapsed.hpp>

#include <gui/widgets/text_widget/expand_item.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>

#include <gui/widgets/seq/flat_feature_retrieve_job.hpp>

#include <gui/objutils/utils.hpp>
#include <objmgr/util/sequence.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CFlatFeatureRetrieveJob::CFlatFeatureRetrieveJob(
    CTextPanelContext& context,
    CFlatFileConfig::EMode mode,
    CFlatFileConfig::EStyle style,
    CScope& scope,
    const CSeq_feat& seq_feat,
    bool getGeneModel)
:   CTextRetrieveJob(context),
    m_Mode(mode), m_Style(style),
    m_Scope(&scope), m_Feature(&seq_feat), m_GetGeneModel(getGeneModel)
{
}

CFlatFeatureRetrieveJob::~CFlatFeatureRetrieveJob()
{
}

static CMappedFeat s_GetMappedFeat(const CSeq_feat& feat, CScope& scope)
{
    SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
    sel.IncludeFeatSubtype(feat.GetData().GetSubtype());
    CFeat_CI feat_it(scope, feat.GetLocation(), sel);
    for ( ;  feat_it;  ++feat_it) {
        if (&feat_it->GetOriginalFeature() == &feat) {
            return *feat_it;
        }
    }
    return CMappedFeat();
}

class CFeatureCoompositeItem : public CCompositeTextItem
{
public:
    CFeatureCoompositeItem(CFlatFileContext& ctx) : m_Ctx(&ctx) {}
private:
    CRef<objects::CFlatFileContext> m_Ctx;
};


IAppJob::EJobState CFlatFeatureRetrieveJob::x_Run()
{
    CBioseq_Handle seq = sequence::GetBioseqFromSeqLoc(m_Feature->GetLocation(), *m_Scope);
    if (!seq) {
        m_Error.Reset(new CAppJobError("Bioseq not found for the feature"));
        return eFailed;
    }

    CFlatFileConfig config(CFlatFileConfig::eFormat_GenBank, m_Mode, m_Style, 0, CFlatFileConfig::fViewAll);
    CRef<CFlatFileContext> ctx(new CFlatFileContext(config));
    ctx->SetEntry(seq.GetParentEntry());

    CRef<CBioseqContext> bctx(new CBioseqContext(seq, *ctx));
    ctx->AddSection(bctx);

    CFeatureCoompositeItem* composite = new CFeatureCoompositeItem(*ctx);
    x_AddItem(composite);

    CConstRef<IFlatItem> item;
    CFlatFileTextItem* flatFileItem = 0;
    CRef<feature::CFeatTree> full_ft
        (new feature::CFeatTree(seq.GetTopLevelEntry()));
    if (m_Feature->GetData().IsBiosrc()) {
        CMappedFeat f = s_GetMappedFeat(*m_Feature, *m_Scope);
        if (f) {
            item.Reset( new CSourceFeatureItem(f, *bctx, full_ft, &m_Feature->GetLocation()) );

            flatFileItem = new CFlatFileTextItem(item);
            flatFileItem->SetEditFlags(0);
            composite->AddItem(new CExpandItem(new CFlatFileFeatureCollapsed(item), flatFileItem, true));
        }

    } else {
        if (m_GetGeneModel) {
            CSeqFeatData::E_Choice type = m_Feature->GetData().Which();
            CMappedFeat mapped_feat = s_GetMappedFeat(*m_Feature, *m_Scope);
            CMappedFeat gene_feat;

            CBioseq_Handle bsh =
                m_Scope->GetBioseqHandle(m_Feature->GetLocation());
            SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
            sel.SetFeatType(CSeqFeatData::e_Gene)
                .IncludeFeatType(CSeqFeatData::e_Cdregion)
                .IncludeFeatType(CSeqFeatData::e_Rna);

            CFeat_CI feat_it(bsh, sel);
            feature::CFeatTree ft(feat_it);

            switch(type) {
            default:
                gene_feat = ft.GetParent(mapped_feat);
                break;

            case CSeqFeatData::e_Gene:
                gene_feat = mapped_feat;
                break;
            }

            if (gene_feat) {
                list<CMappedFeat> mrna_feats;
                list<CMappedFeat> cds_feats;

                feature::GetMrnasForGene(gene_feat, mrna_feats, &ft);
                if (IsCanceled())
                    return eCanceled;

                feature::GetCdssForGene(gene_feat, cds_feats, &ft);
                if (IsCanceled())
                    return eCanceled;

                item.Reset( new CFeatureItem(gene_feat, *bctx, full_ft, &gene_feat.GetLocation()) );

                flatFileItem = new CFlatFileTextItem(item);
                flatFileItem->SetEditFlags(0);

                composite->AddItem(new CExpandItem(new CFlatFileFeatureCollapsed(item), flatFileItem, true));

                ITERATE(list<CMappedFeat> , it, mrna_feats) {
                    x_CheckCancelled();
                    item.Reset( new CFeatureItem(*it, *bctx, full_ft, &it->GetLocation()) );
                    flatFileItem = new CFlatFileTextItem(item);
                    flatFileItem->SetEditFlags(0);

                    composite->AddItem(new CExpandItem(new CFlatFileFeatureCollapsed(item), flatFileItem, true));
                }

                ITERATE(list<CMappedFeat> , it, cds_feats) {
                    x_CheckCancelled();
                    item.Reset( new CFeatureItem(*it, *bctx, full_ft, &it->GetLocation()) );
                    flatFileItem = new CFlatFileTextItem(item);
                    flatFileItem->SetEditFlags(0);
                    composite->AddItem(new CExpandItem(new CFlatFileFeatureCollapsed(item), flatFileItem, true));
                }
            }
            else {
                if (mapped_feat) {
                    item.Reset( new CFeatureItem(mapped_feat, *bctx, full_ft, &m_Feature->GetLocation()) );

                    flatFileItem = new CFlatFileTextItem(item);
                    flatFileItem->SetEditFlags(0);

                    composite->AddItem(new CExpandItem(new CFlatFileFeatureCollapsed(item), flatFileItem, true));
                }
            }
        }
        else {
            SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
            CFeat_CI iter(*m_Scope, m_Feature->GetLocation(), sel);
            bctx->GetFeatTree().AddFeatures(iter);
            for (;  iter;  ++iter) {
                x_CheckCancelled();
                item.Reset( new CFeatureItem(*iter, *bctx, full_ft, NULL));
                flatFileItem = new CFlatFileTextItem(item);
                flatFileItem->SetEditFlags(0);
                composite->AddItem(new CExpandItem(new CFlatFileFeatureCollapsed(item), flatFileItem, true));
            }
        }
    }

    x_CreateResult();

    return eCompleted;
}

END_NCBI_SCOPE
