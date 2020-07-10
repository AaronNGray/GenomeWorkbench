/*  $Id: bins_track.cpp 37640 2017-01-31 18:24:42Z shkeda $
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
 * Authors:  Melvin Quintos, Dmitry Rudnev
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_snp/bins/bins_track.hpp>
#include <gui/packages/pkg_snp/bins/bins_glyph.hpp>
#include <gui/packages/pkg_snp/bins/gmark_glyph.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/seq_graphic/simple_layout_policy.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
#include <gui/widgets/seq_graphic/layered_layout_policy.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>

#include <gui/utils/rgba_color.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>

#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/annot_ci.hpp>

#include <objects/seqfeat/SeqFeatData.hpp>

#include <wx/menu.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
///   CBinsTrack
///////////////////////////////////////////////////////////////////////////////

CTrackTypeInfo CBinsTrack::m_TypeInfo("SNP_Bins_track",
                                     "SNP related data grouped by bins of equal width.");

CBinsTrack::CBinsTrack(CBinsDS* ds, CRenderingContext* r_cntx)
: CDataTrack(r_cntx)
    , m_BinType(NSnpBins::eUnknown)
    , m_DS(ds)
    , m_IconLayout(eIcon_Layout, "Layout", true, "track_layout")
{
    m_DS->SetJobListener(this);
    m_FeatSel = CSeqUtils::GetAnnotSelector();
}


CBinsTrack::~CBinsTrack()
{
    if ( !m_DS->AllJobsFinished() ) {
        m_DS->DeleteAllJobs();
    }
}

/// @name CLayoutTrack reimplemented methods.
/// @{
const CTrackTypeInfo& CBinsTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}

string CBinsTrack::GetFullTitle() const
{
    string title = GetTitle();
    if (title.empty()) {
        title = m_PreferredTitle.empty() ? m_AnnotName : m_PreferredTitle;
    }
    return title;
}

void CBinsTrack::SetAnnot(const string& name)
{
    m_AnnotName = name;
}

void CBinsTrack::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
    if((m_BinType != NSnpBins::eGCAT && m_BinType != NSnpBins::eGAP) || GetChildren().size() > 0) {
        // use the standard way
        CGlyphContainer::GetHTMLActiveAreas(p_areas);
    }
}


CHTMLActiveArea* CBinsTrack::InitHTMLActiveArea(TAreaVector* p_areas) const
{
	if (m_BinType == NSnpBins::eGCAT || m_BinType == NSnpBins::eGAP) {
        TModelUnit tb_height = x_GetTBHeight();

        // for Associated results/GAP types of tracks, generate a separate area for the track caption
        CHTMLActiveArea area;
        CSeqGlyph::x_InitHTMLActiveArea(area);

        // adjust the bounds height to that of the caption only
        TVPRect& bound(area.m_Bounds);
        bound.SetBottom(bound.Top() + (TVPUnit)tb_height + (tb_height > 0.0 ? 2 : 0));

        // find out the gi
        TGi gi(sequence::GetGiForId(*(sequence::GetId(m_DS->GetBioseqHandle(), sequence::eGetId_ForceGi).GetSeqId()),
            m_DS->GetBioseqHandle().GetScope()));

        // create the signature
        string sSignature(CBinsGlyph::GenerateBinSignature("",
            m_AnnotName,
            gi,
            area.m_SeqRange,
            m_BinType));
        area.m_Signature = sSignature;
        area.m_Flags = CHTMLActiveArea::fNoSelection | CHTMLActiveArea::fNoPin | CHTMLActiveArea::fTrack | CHTMLActiveArea::fNoNavigation;
        p_areas->push_back(area);
        return &(p_areas->back());
    }

    return CLayoutTrack::InitHTMLActiveArea(p_areas);
}


void CBinsTrack::x_OnIconClicked(TIconID id)
{
    bool bIsLayoutOnlyChange = true;

    if (id == eIcon_Layout) {
        wxMenu menu;

        menu.AppendRadioItem(eLayout_Labels,    wxT("Labels"));

        menu.Check(m_eLayout, true);

        m_LTHost->LTH_PopupMenu(&menu);

        wxMenuItemList& item_list = menu.GetMenuItems();
        ITERATE (wxMenuItemList, iter, item_list) {
            ELayout new_layout = (ELayout)((*iter)->GetId());
            if ((*iter)->IsChecked()  &&  id != m_eLayout) {

                // Reload data if switching back or forth to Labels
                if ( (m_eLayout ==eLayout_Labels) ||
                     (new_layout==eLayout_Labels) )
                {
                    bIsLayoutOnlyChange = false;
                }

                m_eLayout = new_layout;
                break; // stop processing loop
            }
        }
    } else {
        // use default handlers
        CLayoutTrack::x_OnIconClicked(id);
    }

    Update(bIsLayoutOnlyChange);
    x_OnLayoutChanged();
}
/// @}


void CBinsTrack::x_LoadSettings(const string& /*preset_style*/,
                                const TKeyValuePairs& /*settings*/)
{
    m_Layered->SetVertSpace(m_gConfig->GetObjectSpace());
}

/// @}

bool CBinsTrack::x_IsOverviewMode() const
{
    static double kNPP_Cutoff= 50;  // Nucliotides per pixel cutoff for SNP Overview
    return (m_Context->GetScale() > kNPP_Cutoff);
}

/// @name CDataTrack pure virtual interfaces
/// @{

void CBinsTrack::x_UpdateData()
{
    CDataTrack::x_UpdateData();
    m_DS->DeleteAllJobs();
    x_SetStartStatus();
    m_DS->LoadData(m_Context->GetVisSeqRange(),
                   m_Context->GetScale(),
                   m_Context->WillSeqLetterFit());
}


void CBinsTrack::x_OnJobCompleted(CAppJobNotification& notify)
{
    m_DS->ClearJobID(notify.GetJobID());
    CRef<CObject> res_obj = notify.GetResult();
    SBinsJobResult* result = dynamic_cast<SBinsJobResult*>(&*res_obj);
    if (!result) {
        ERR_POST("CBinsTrack::x_OnJobCompleted() notification for job \
                does not contain results.");
        return;
    }

    // Depending on view, setup icon
    if (x_IsOverviewMode())
        x_DeregisterIcon(m_IconLayout.m_Id);
    else
        x_RegisterIcon(m_IconLayout);

    x_ProcessBinsJobResult(*result);
    SetMsg("");
}


/// @}

void CBinsTrack::x_ProcessBinsJobResult(const SBinsJobResult& result)
{
    const CSeqGlyph::TObjects& objs = result.listObjs;

    m_Group.Clear();

    if (objs.empty()) {
        m_Attrs |= fNavigable;
        Update(true);
        x_OnLayoutChanged();
        return;
    }

    CRef<CSeqGlyph> first_glyph = objs.front();

    //!! consider making GetTitle a virtual method in #include <gui/packages/pkg_snp/bins/bins_glyph.hpp>

    if (typeid(*first_glyph) == typeid(CHistogramGlyph)) {
        CHistogramGlyph *hist = dynamic_cast<CHistogramGlyph*> (first_glyph.GetPointer());
        hist->SetConfig(*x_GetGlobalConfig());
        m_PreferredTitle = hist->GetTitle();
        Add(first_glyph);
    }
    else {
        m_Attrs |= fNavigable;
        CBinsGlyph* bins(NULL);
        CGeneMarkerGlyph* gene_marker(NULL);
        ITERATE(CSeqGlyph::TObjects, it, objs) {
            CSeqGlyph* glyph = const_cast<CSeqGlyph*> (it->GetPointer());
            if(typeid(*glyph) == typeid(CBinsGlyph)) {
                bins = dynamic_cast<CBinsGlyph*>(glyph);
                m_BinType = bins->GetType();
            } else {
                gene_marker = dynamic_cast<CGeneMarkerGlyph*>(glyph);
                m_BinType = NSnpBins::eGAP;
            }
            Add(glyph);
        }

        // If there is exactly 1 glyph, then set the title based on what is written inside the glyph
        //!! there can be two glyphs now, adjust for that!
        if (objs.size()==1) {
            if(bins) {
                m_PreferredTitle = bins->GetTitle();
            } else if(gene_marker) {
                m_PreferredTitle = gene_marker->GetTitle();
            }
        }
    }

    Update(true);
    x_OnLayoutChanged();
}



///////////////////////////////////////////////////////////////////////////////
/// CBinsTrackFactory
///////////////////////////////////////////////////////////////////////////////
ILayoutTrackFactory::TTrackMap
CBinsTrackFactory::CreateTracks(SConstScopedObject& object,
                                  ISGDataSourceContext* ds_context,
                                  CRenderingContext* r_cntx,
                                  const SExtraParams& params,
                                  const TAnnotMetaDataList& src_annots) const
{
    TTrackMap tracks;

    TAnnotNameTitleMap annots;
    if (params.m_SkipGenuineCheck  &&  !params.m_Annots.empty()) {
        ITERATE (SExtraParams::TAnnots, iter, params.m_Annots) {
            annots.insert(TAnnotNameTitleMap::value_type(*iter, ""));
        }
    } else {

        CBinsDS::GetTrackNames(object, annots, r_cntx->GetVisSeqRange(), params);

        if ( !src_annots.empty() ) {
            GetMatchedAnnots(src_annots, params, annots);
        }
    }

    // create feature tracks
    ITERATE (TAnnotNameTitleMap, iter, annots) {
        // Create a new DS object for each track
        CIRef<ISGDataSource> pre_ds =
            ds_context->GetDS(typeid(CBinsDSType).name(), object);
        CBinsDS* seq_ds = dynamic_cast<CBinsDS*>(pre_ds.GetPointer());
        seq_ds->SetDepth(params.m_Level);
        seq_ds->SetAdaptive(params.m_Adaptive);
        seq_ds->SetName(iter->first);

        CRef<CBinsTrack> track(new CBinsTrack(seq_ds, r_cntx));
        if ( !iter->second.empty() ) {
            track->SetTitle(iter->second);
        }
        tracks[iter->first] = track.GetPointer();
        track->SetAnnot(iter->first);
    }

    return tracks;
}

void CBinsTrackFactory::GetMatchedAnnots(
    const TAnnotMetaDataList& src_annots,
    const ILayoutTrackFactory::SExtraParams& params,
    TAnnotNameTitleMap& out_annots) const
{
    ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "seq-table", "CitedVar", out_annots);
    ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "seq-table", "ClinVar", out_annots);
    ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "seq-table", "dbGaPAnalysis", out_annots);
    ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "seq-table", "dbGaPProbeAffy", out_annots);
}

string CBinsTrackFactory::GetExtensionIdentifier() const
{
    return CBinsTrack::m_TypeInfo.GetId();
}


string CBinsTrackFactory::GetExtensionLabel() const
{
    return CBinsTrack::m_TypeInfo.GetDescr();
}


CRef<CTrackConfigSet>
CBinsTrackFactory::GetSettings(const string& /*profile*/,
                               const TKeyValuePairs& /*settings*/,
                               const CTempTrackProxy* /*track_proxy*/) const
{
    CRef<CTrackConfigSet> config_set(new CTrackConfigSet);
    CRef<CTrackConfig> config(new CTrackConfig);
    config_set->Set().push_back(config);
    config->SetHelp() = GetThisTypeInfo().GetDescr();
    config->SetLegend_text("anchor_4.3");
    return config_set;
}



END_NCBI_SCOPE
