/*  $Id: dbvar_track.cpp 44699 2020-02-24 22:14:36Z shkeda $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/seq_graphic/dbvar_track.hpp>

#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/dbvar_feature_sorter.hpp>
#include <gui/widgets/seq_graphic/layout_track_proxy.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/obj_fingerprint.hpp>
#include <gui/objects/TextBox.hpp>
#include <objmgr/util/sequence.hpp>
#include <util/checksum.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
///   CVarTrack
///////////////////////////////////////////////////////////////////////////////

CTrackTypeInfo CVarTrack::m_TypeInfo("dbvar_track",
                                     "Variation features");

CVarTrack::CVarTrack(CSGFeatureDS* ds, CRenderingContext* r_cntx)
    : CFeatureTrack(ds, r_cntx)
{
}


CVarTrack::~CVarTrack()
{}


const CTrackTypeInfo& CVarTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}


void CVarTrack::x_UpdateData()
{
    if (m_ZoomToSee) {
        string msg = "Zoom to see if data exists for this region";
        const TSeqRange& range = m_Context->GetVisSeqRange();
        TModelUnit x = (range.GetFrom() + range.GetTo() ) * 0.5;
        CSeqGlyph::TObjects objs;
        CRef<CCommentGlyph> comment(new CCommentGlyph(msg, TModelPoint(x, 0.0)));
        objs.push_back(comment);
        CRef<CCommentConfig> config(new CCommentConfig(*x_GetGlobalConfig()->GetCommentConfig()));
        config->m_ShowConnection = false;
        SetCommentGlyphs(objs, config);
        return;
    }
    CFeatureTrack::x_UpdateData();
}

void CVarTrack::x_LoadSettings(const string& preset_style, const TKeyValuePairs &settings) 
{
    CFeatureTrack::x_LoadSettings(preset_style, settings);
    auto it = settings.find("zoom_to_see");
    m_ZoomToSee = (it != settings.end() && NStr::StringToBool(it->second));
}

string GenerateSignature(const string& title,
    const string& annot_name,
    TGi gi,
    TSeqRange range)
{
    char buff[128];
    Uint2 type;

    // create type
    type = (CObjFingerprint::eVarTrack) << 8;
//    type |= BinType;

    // create checksum
    CChecksum cs(CChecksum::eCRC32);
    Uint4 checksum(0);
    if (!title.empty()) {
        cs.AddLine(title);
        checksum = cs.GetChecksum();
    }

    CChecksum cs_name(CChecksum::eCRC32);
    Uint4 checksum_name;
    cs_name.AddLine(annot_name);
    checksum_name = cs_name.GetChecksum();

    sprintf(buff, "gi|%d-%08x-%08x-%04x-%08x-%08x",
        GI_TO(int, gi), range.GetFrom(), range.GetTo(),
        type, checksum, checksum_name);

    return string(buff);
}

CHTMLActiveArea* CVarTrack::InitHTMLActiveArea(TAreaVector* p_areas) const
{
    if (!GetChildren().empty()) {
        TModelUnit tb_height = x_GetTBHeight();

        // for Associated results/GAP types of tracks, generate a separate area for the track caption
        CHTMLActiveArea area;
        CSeqGlyph::x_InitHTMLActiveArea(area);

        // adjust the bounds height to that of the caption only
        TVPRect& bound(area.m_Bounds);
        bound.SetBottom(bound.Top() + (TVPUnit)tb_height + (tb_height > 0.0 ? 2 : 0));

        // user uploaded tracks are not based on NA annots
        // we don't need to generate signature for them 
        // otherwise CTrackContainer will create multiple signatures
        // and sviewer will issue seqgraphic call to get track's tooltip
        // we don't want that for user uploaded tracks
        
        if (CSeqUtils::IsNAA(m_Annot)) {

            // find out the gi
            TGi gi(sequence::GetGiForId(*(sequence::GetId(m_DS->GetBioseqHandle(), 
                                                          sequence::eGetId_ForceGi).GetSeqId()),
                                        m_DS->GetBioseqHandle().GetScope()));

            // create the signature
            string sSignature(GenerateSignature(GetTitle(),
                                                m_Annot,
                                                gi,
                                                area.m_SeqRange));
            area.m_Signature = sSignature;
        }
        area.m_Flags = CHTMLActiveArea::fNoSelection | CHTMLActiveArea::fNoPin | CHTMLActiveArea::fTrack;
        if (!(m_Attrs & fNavigable))
            area.m_Flags |= CHTMLActiveArea::fNoNavigation;
        p_areas->push_back(area);
        return &(p_areas->back());
    }
    auto area = CLayoutTrack::InitHTMLActiveArea(p_areas);
    if (area && isRmtBased()) // turn off navigation for VCFTabix
        area->m_Flags |= CHTMLActiveArea::fNoNavigation;
    return area;
}

///////////////////////////////////////////////////////////////////////////////
/// CVarTrackFactory
///////////////////////////////////////////////////////////////////////////////
ILayoutTrackFactory::TTrackMap
CVarTrackFactory::CreateTracks(SConstScopedObject& object,
                               ISGDataSourceContext* ds_context,
                               CRenderingContext* r_cntx,
                               const SExtraParams& params,
                               const TAnnotMetaDataList& src_annots) const
{
    TAnnotNameTitleMap annots;
    TTrackMap tracks;

    TKeyValuePairs track_settings;
    CSGConfigUtils::ParseProfileString(params.m_TrackProfile, track_settings);

    CSeqFeatData::ESubtype subtype{ CSeqFeatData::eSubtype_variation_ref };
        
    if (!params.m_Subkey.empty())
        subtype  = CSeqFeatData::SubtypeNameToValue(params.m_Subkey);

    switch (subtype)
    {
        case CSeqFeatData::eSubtype_variation:
        case CSeqFeatData::eSubtype_variation_ref:
            break;
        default:
            LOG_POST(Error << "CVarTrackFactory::CreateTracks() subtype " << params.m_Subkey << " is not supported!");
            return tracks;
    }

    if (params.m_SkipGenuineCheck  &&  !params.m_Annots.empty()) {
        ITERATE (SExtraParams::TAnnots, iter, params.m_Annots) {
            annots.insert(TAnnotNameTitleMap::value_type(*iter, ""));
        }
    } else {

        TSeqRange range = params.m_Range;
        if (range.Empty()) {
            range = r_cntx->GetVisSeqRange();
        }

        // collect non-NA tracks
        CIRef<ISGDataSource> pre_ds = ds_context->GetDS(
            typeid(CSGFeatureDSType).name(), object);
        CSGFeatureDS* seq_ds = dynamic_cast<CSGFeatureDS*>(pre_ds.GetPointer());
        seq_ds->SetDepth(params.m_Level);
        seq_ds->SetAdaptive(params.m_Adaptive);

        SAnnotSelector sel(CSeqUtils::GetAnnotSelector(params.m_Annots));
        sel.SetFeatSubtype(subtype);
        seq_ds->GetAnnotNames_var(sel, range, annots);

        // collect NA tracks
        if ( !src_annots.empty() ) {
            GetMatchedAnnots(src_annots, params, annots);
        }
    }

    // create feature tracks
    ITERATE (TAnnotNameTitleMap, iter, annots) {
        if (iter->first == "SNP") // Skip data from dbSNP
            continue;
        CIRef<ISGDataSource> ds = ds_context->GetDS(
            typeid(CSGFeatureDSType).name(), object);
        CSGFeatureDS* seq_ds = dynamic_cast<CSGFeatureDS*>(ds.GetPointer());
        seq_ds->SetDepth(params.m_Level);
        seq_ds->SetAdaptive(params.m_Adaptive);
        if (track_settings.count("graph_cache_key") > 0) {
            seq_ds->SetGraphCacheKey(track_settings["graph_cache_key"]);
        }
        if (track_settings.count("rmt_type") > 0) {
            seq_ds->SetRemoteDataType(track_settings["rmt_type"]);
        }

        // check the total levels of coverage graphs
        //seq_ds->SetGraphLevels(iter->first);

        CRef<CVarTrack> feat_track(new CVarTrack(seq_ds, r_cntx));
        feat_track->SetAnnot(iter->first);
        feat_track->ExcludeAnnotName("SNP");
        feat_track->SetFeatSubtype(subtype);
        if ( !iter->second.empty() ) {
            feat_track->SetTitle(iter->second);
        }
        seq_ds->SetFilter(params.m_Filter);
        seq_ds->SetSortBy(params.m_SortBy);
        tracks[iter->first] = feat_track.GetPointer();
    }

    return tracks;
}

void CVarTrackFactory::GetMatchedAnnots(
    const TAnnotMetaDataList& src_annots,
    const ILayoutTrackFactory::SExtraParams& params,
    TAnnotNameTitleMap& out_annots) const
{
    if (params.m_Subkey != "variation")
        ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "ftable", "variation", out_annots);
}

string CVarTrackFactory::GetExtensionIdentifier() const
{
    return CVarTrack::m_TypeInfo.GetId();
}


string CVarTrackFactory::GetExtensionLabel() const
{
    return CVarTrack::m_TypeInfo.GetDescr();
}


CRef<CTrackConfigSet>
CVarTrackFactory::GetSettings(const string& /*profile*/,
                              const TKeyValuePairs& settings,
                              const CTempTrackProxy* track_proxy) const
{
    CRef<CTrackConfigSet> config_set(new CTrackConfigSet);
    CRef<CTrackConfig> config(new CTrackConfig);
    config_set->Set().push_back(config);
    config->SetHelp() = GetThisTypeInfo().GetDescr();
    config->SetLegend_text("anchor_5");

    string rendering_str = CFeatureTrack::LinkedFeatDisplayValueToStr(ELinkedFeatDisplay::eLFD_Expandable);
    string hist_t = "0";

    ITERATE (TKeyValuePairs, iter, settings) {
        try {
            if (NStr::EqualNocase(iter->first, "Rendering")) {
                rendering_str = iter->second;
                if (rendering_str == "Histogram") // support legacy value
                    rendering_str = "Heatmap";
            } else if (NStr::EqualNocase(iter->first, "HistThreshold")) {
                int val = NStr::StringToInt(iter->second);
                if (val > 0) hist_t = iter->second;
            }
        } catch (CException& e) {
            LOG_POST(Warning << "CVarTrackFactory::x_LoadSettings() "
                << e.GetMsg());
        }
    }

    string sort_by;
    if (track_proxy)  {
        sort_by = track_proxy->GetSortBy();
    }

    CRef<CChoice> choice = CTrackConfigUtils::CreateChoice(
        "Rendering", "Rendering options",
        rendering_str,
        "Rendering options for linked features.");

    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
        CFeatureTrack::LinkedFeatDisplayValueToStr(ELinkedFeatDisplay::eLFD_Default),
        "Show all",
        "All features, both parent and all child features",
        "The  bar on top represents the variant region. Each bar below represents \
        the supporting level variants used to define this region.<br>Red: A Deletion or Loss<br> \
        Green: A Gain or Insertion<br>Light Gray: An Inversion"));

    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
        CFeatureTrack::LinkedFeatDisplayValueToStr(ELinkedFeatDisplay::eLFD_Expandable),
        "Show parent, Expand children upon a click",
        "Show parent on one line and expand all children upon a mouse click.",
        "The bar on top represents the variant region. The bars below appear upon a mouse click and each bar represents \
        the supporting level variants used to define this region.<br>Red: A Deletion or Loss<br> \
        Green: A Gain or Insertion<br>Light Gray: An Inversion"));

    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
        CFeatureTrack::LinkedFeatDisplayValueToStr(ELinkedFeatDisplay::eLFD_Packed),
        "Show parent, Merge children",
        "Show parent on one line and all children merged below it.",
        "The  bar on top represents the variant region. The bar below represents \
        all of the sample level variants merged onto a single line.<br>Red: A Deletion or Loss<br> \
        Green: A Gain or Insertion<br>Light Gray: An Inversion"));

    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
        CFeatureTrack::LinkedFeatDisplayValueToStr(ELinkedFeatDisplay::eLFD_ParentHidden),
        "Show children, not parent",
        "Show only the supporting variants (chidlren) not the variant region (parent)",
        "The bar represent supporting variants in this study.<br>Red: A Deletion or Loss<br>\
        Green: A Gain or Insertion<br>Light Gray: An Inversion"));

    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
        CFeatureTrack::LinkedFeatDisplayValueToStr(ELinkedFeatDisplay::eLFD_Hidden),
        "Show parent, not children ",
        "Show the variant region (parent) not the supporting variants (children)",
        "The bar represents the variant region."));

    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
        "SingleLine",
        "Display on a single line",
        "Track is displayed, with all features collapsed on a single line.",
        "All the features are packed into one line."));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
        "Heatmap",
        "Heatmap",
        "Track is displayed, with all features displayed as a heatmap.",
        "All the features are packed and shown as a heatmap."));

    config->SetChoice_list().push_back(choice);

    CRef<CChoice> sortby_choice = CTrackConfigUtils::CreateChoice(
        "sort_by", "Sort variants by",
        sort_by,
        "Sort variants according to a selected criterion.");

    sortby_choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
        "", "No sorting", "Don't sort variants", ""));

    IObjectSorter::SSorterDescriptor descr = CDbvarQualitySorter::GetSorterDescr();

    sortby_choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
        descr.m_Name, descr.m_DisplayName, descr.m_Descr, ""));

    descr = CDbvarClinicalAsstSorter::GetSorterDescr();
    sortby_choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
        descr.m_Name, descr.m_DisplayName, descr.m_Descr, ""));

    descr = CDbvarPilotSorter::GetSorterDescr();
    sortby_choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
        descr.m_Name, descr.m_DisplayName, descr.m_Descr, ""));

    descr = CDbvarSamplesetTypeSorter::GetSorterDescr();
    sortby_choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
        descr.m_Name, descr.m_DisplayName, descr.m_Descr, ""));

    descr = CDbvarValidationSorter::GetSorterDescr();
    sortby_choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
        descr.m_Name, descr.m_DisplayName, descr.m_Descr, ""));

    config->SetChoice_list().push_back(sortby_choice);

    CRef<CTextBox> txt_box(new CTextBox);
    txt_box->SetName("HistThreshold");
    txt_box->SetValue(hist_t);
    txt_box->SetDisplay_name("Heatmap Threshold");
    txt_box->SetHelp("Display a heatmap when there are more features in this track than the value specificed."
                     " 'None' or '0' means threshold is disabled, and the display is up to 'Rendering Options'");
    config->SetText_boxes().push_back(txt_box);

    return config_set;
}


END_NCBI_SCOPE
