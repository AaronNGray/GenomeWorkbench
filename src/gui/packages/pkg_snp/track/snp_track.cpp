	/*  $Id: snp_track.cpp 45002 2020-05-06 04:00:32Z rudnev $
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

#include <gui/packages/pkg_snp/track/snp_track.hpp>
#include <gui/widgets/snp/filter/snp_filter.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/seq_graphic/simple_layout_policy.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
#include <gui/widgets/seq_graphic/layered_layout_policy.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <gui/widgets/seq_graphic/comment_glyph.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/registry.hpp>

#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/annot_ci.hpp>

#include <objects/general/User_object.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/seqres/Int_graph.hpp>
#include <objects/seqres/Byte_graph.hpp>

#include <objtools/snputil/snp_utils.hpp>

#include <wx/menu.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
///   CSnpTrack
///////////////////////////////////////////////////////////////////////////////
typedef SStaticPair<const char*, CSnpTrack::ELayout> TLayoutStr;
static const TLayoutStr s_LayoutStrs[] = {
    { "Adaptive",  CSnpTrack::eLayout_Adaptive },
    { "Density",   CSnpTrack::eLayout_Density },
    { "Features",  CSnpTrack::eLayout_Features },
    { "Labels",    CSnpTrack::eLayout_Labels },
};

typedef CStaticArrayMap<string, CSnpTrack::ELayout> TLayoutMap;
DEFINE_STATIC_ARRAY_MAP(TLayoutMap, sm_LayoutMap, s_LayoutStrs);

static const string kBaseKey = "GBPlugins.SeqGraphicSNP";
static const string kDefProfile = "Default";

CTrackTypeInfo CSnpTrack::m_TypeInfo("SNP_track",
                                     "Variation features from dbSNP");

static CSnpTrack::ELayout s_LayoutStrToValue(const string& layout)
{
    TLayoutMap::const_iterator iter = sm_LayoutMap.find(layout);
    if (iter != sm_LayoutMap.end()) {
        return iter->second;
    }
    NCBI_THROW(CException, eInvalid, "Invalid layout string: " + layout);
}


const string& s_LayoutValueToStr(CSnpTrack::ELayout layout)
{
    ITERATE(TLayoutMap, iter, sm_LayoutMap) {
        if (iter->second == layout) {
            return iter->first;
        }
    }
    return kEmptyStr;
}

CSnpTrack::CSnpTrack(CSGSnpDS* ds, CRenderingContext* r_cntx, const string& sFilter, const string& sTitle, const string& sLayout)
    : CDataTrack(r_cntx)
    , m_eLayout(eLayout_Adaptive)
    , m_DS(ds)
    , m_IconLayout(eIcon_Layout, "Layout", true, "track_layout")
    , m_PreferredTitle(sTitle)
{
    m_DS->SetJobListener(this);

    // Set layout
    CSnpTrack::ELayout layout(eLayout_Adaptive);
    if(!sLayout.empty()) {
        try {
            layout = s_LayoutStrToValue(sLayout);
        } catch(...) {}
    }
    if(layout == eLayout_Density) {
        SetLayoutPolicy(m_Inline); 
    } else {
        SetLayoutPolicy(m_Layered);
    }
    m_eLayout = layout;        

    m_Params = CSNPFeatureParams::CreateSNPFeatureParams();

    if(!sFilter.empty())
        m_Filter.Reset(new SSnpFilter(sFilter));

    SetTitle(sTitle, sTitle);
}


CSnpTrack::~CSnpTrack()
{
    if ( !m_DS->AllJobsFinished() ) {
        m_DS->DeleteAllJobs();
    }
}

void CSnpTrack::x_UpdateLayout()
{
    if ( !m_JobResult ) {
        return;
    }

    // Determine if we show labels
    bool bShowLabels = false;

    if (m_eLayout == eLayout_Labels || m_eLayout == eLayout_Adaptive) {
        bShowLabels = true;
    }

    // Setup feature parameters
    CRef<CFeatureParams> curr_param = m_Params->SetParamsTemplate();
    curr_param->m_LabelPos =
        bShowLabels ? CFeatureParams::ePos_Side : CFeatureParams::ePos_NoLabel;
    m_Params->UpdateLabelPos();

    if (bShowLabels) {
        m_Layered->SetVertSpace(5);
    } else {
        m_Layered->SetVertSpace(m_gConfig->GetObjectSpace());
    }

    // add the layout
    x_AddSnpsLayout(*m_JobResult);
    Update(true);
    x_OnLayoutChanged();
}

bool CSnpTrack::x_IsOverviewMode() const
{
    return GetRenderingContext()->GetScale() >= 100 || GetRenderingContext()->IsOverviewMode();
}

/// @name CLayoutTrack reimplemented methods.
/// @{
const CTrackTypeInfo& CSnpTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}


string CSnpTrack::GetFullTitle() const
{
    string title = GetTitle();
    if (title.empty()) {
        title = m_PreferredTitle.empty() ? m_AnnotName : m_PreferredTitle;
    }
    return title;
}

void CSnpTrack::SetAnnot(const string& name)
{
    m_AnnotName = name;
}

void CSnpTrack::x_OnIconClicked(TIconID id)
{
    bool bIsLayoutOnlyChange = true;

    if (id == eIcon_Layout) {
        wxMenu menu;
        UseDefaultMarginWidth(menu);

        menu.AppendRadioItem(eLayout_Adaptive,  wxT("Adaptive"));
        menu.AppendRadioItem(eLayout_Density,   wxT("Density"));
        menu.AppendRadioItem(eLayout_Features,  wxT("Features"));
        menu.AppendRadioItem(eLayout_Labels,    wxT("Labels"));

        menu.Check(m_eLayout, true);

        m_LTHost->LTH_PopupMenu(&menu);

        wxMenuItemList& item_list = menu.GetMenuItems();
        ITERATE (wxMenuItemList, iter, item_list) {
            ELayout new_layout = (ELayout)((*iter)->GetId());
            if ((*iter)->IsChecked()  &&  id != m_eLayout) {
                // Reload data if we are changing from FeatGlyphs or HistogramGlyphs
                if (m_JobResult  &&  m_JobResult->eDataType == SSnpJobResult::eFeats) {
                    if (new_layout==eLayout_Density) {
                        bIsLayoutOnlyChange = false;
                    }
                }
                else { // we currently have CHistogramGlyphs
                    if ( (new_layout==eLayout_Labels) ||
                         (new_layout==eLayout_Features)  )
                    {
                        bIsLayoutOnlyChange = false;
                    }
                }

                if (new_layout==eLayout_Adaptive) {
                    bIsLayoutOnlyChange = false;
                }

                m_eLayout = new_layout;
                break; // stop processing loop
            }
        }
    }
    else {
        // use default handlers
        CLayoutTrack::x_OnIconClicked(id);
    }

    if (bIsLayoutOnlyChange) {
        x_UpdateLayout();
    }
    else {
        Update(false);  // if false, reload data
    }
}
/// @}

void CSnpTrack::x_SaveSettings(const string& preset_style)
{
    TKeyValuePairs settings;

    if ( !preset_style.empty() ) {
        settings["profile"] = preset_style;
    }

    settings["layout"] = s_LayoutValueToStr( (CSnpTrack::ELayout) m_eLayout );

    SetProfile(CSGConfigUtils::ComposeProfileString(settings));
}

void CSnpTrack::x_LoadSettings(const string& preset_style,
                               const TKeyValuePairs& settings)
{
    if (preset_style.empty()) {
        SetProfile(kDefProfile);
    } else {
        SetProfile(preset_style);
    }

	// First load default profile settings
    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryReadView view =
        CSGConfigUtils::GetReadView(registry, kBaseKey, GetProfile(), kDefProfile);

    // Now override default settings passed in through profile settings
    ITERATE (TKeyValuePairs, iter, settings) {
        try {
            if (NStr::EqualNocase(iter->first, "layout")) {
                m_eLayout = s_LayoutStrToValue(iter->second);
			}
            else
                LOG_POST(Warning << "SNP track: the settings are not supported - "
                         << iter->first << ":" << iter->second);
        } catch (std::exception&) {
            LOG_POST(Warning << "SNP track: got invalid profile - "
                     << iter->first << ":" << iter->second);
        }
    }

    m_Layered->SetVertSpace(m_gConfig->GetObjectSpace());

    // initialize specific settings for SNP feature
    CConstRef<CFeatureParams> param(
        x_GetGlobalConfig()->GetFeatParams(CSeqFeatData::eSubtype_variation));
    m_Params->Reset(param);
    CRef<CFeatureParams> curr_param = m_Params->SetParamsTemplate();
    curr_param->m_LabelFont.SetFontSize(10);
}


/// @name CDataTrack pure virtual interfaces
/// @{

void CSnpTrack::x_UpdateData()
{
    CDataTrack::x_UpdateData();
    bool bFetchFeatures = (m_eLayout == eLayout_Features);
    bool bAdaptive      = (m_eLayout == eLayout_Adaptive);
    m_DS->DeleteAllJobs();
    x_SetStartStatus();

    if (m_Filter.NotNull()  &&  x_IsOverviewMode()) {
        m_PreferredTitle = m_AnnotName + ", " + m_Filter->name;
        //x_SetStatus(", Please zoom in", 100);
        string msg = "Zoom to see data!";
        const TSeqRange& range = m_Context->GetVisSeqRange();
        TModelUnit x = (range.GetFrom() + range.GetTo() ) * 0.5;
        CRef<CSeqGlyph> message_glyph(new CCommentGlyph(msg, TModelPoint(x, 0.0)));

        if ( !m_JobResult ) {
            m_JobResult.Reset(new SSnpJobResult());
        }

        m_JobResult->listObjs.clear();
        m_JobResult->eDataType = SSnpJobResult::eComment;
        m_JobResult->listObjs.push_back(message_glyph);
        x_ProcessJobResult();
    } else {
        CSnpJob::SParams p;

        p.sAnnotName  = m_AnnotName;
        p.bPreferFeat = bFetchFeatures ||
                        m_eLayout==eLayout_Labels  ||
                        GetRenderingContext()->WillSeqLetterFit();
        p.bAdaptive   = bAdaptive;
        p.isOverview  = x_IsOverviewMode();
        p.scale       = m_Context->GetScale();
        p.r_cntx      = m_Context;

        m_DS->LoadData(m_Context->GetVisSeqRange(), p, m_Filter.GetPointer());
    }
}



CFeat_CI CSnpTrack::x_GetFeat_CI(const TSeqRange& range)
{
    SAnnotSelector sel;

    sel = CSeqUtils::GetAnnotSelector(CSeqFeatData::eSubtype_variation,
        CSeqUtils::fAnnot_UnsetNamed);

    auto sAnnotName(CSnpJob::s_AdjustAnnotName(m_AnnotName));
    if(CSeqUtils::NameTypeStrToValue(sAnnotName) ==
        CSeqUtils::eAnnot_Unnamed) {
        sel.AddUnnamedAnnots();
    } else {
        sel.IncludeNamedAnnotAccession(sAnnotName);
        sel.AddNamedAnnots(sAnnotName);
    }

    CSeqUtils::SetResolveDepth(sel, true, -1);
    CBioseq_Handle& Handle = m_DS->GetBioseqHandle();
    CConstRef<CSeq_loc> loc(
        Handle.GetRangeSeq_loc(range.GetFrom(), range.GetTo()) );

//    cerr << "using loc " << endl << 
//            MSerial_AsnText << *loc << endl;
    return CFeat_CI(Handle.GetScope(), *loc, sel);
}

CGraph_CI CSnpTrack::x_GetGraph_CI(const TSeqRange& range)
{
    SAnnotSelector sel;

    sel = CSeqUtils::GetAnnotSelector(CSeq_annot::TData::e_Graph,
        CSeqUtils::fAnnot_UnsetNamed);

    auto sAnnotName(CSnpJob::s_AdjustAnnotName(m_AnnotName));
    sel.SetExcludeExternal(false);
    CSeqUtils::SetResolveDepth(sel, true, -1);
    CBioseq_Handle& Handle = m_DS->GetBioseqHandle();
    CConstRef<CSeq_loc> loc(
        Handle.GetRangeSeq_loc(range.GetFrom(), range.GetTo()) );

    string zoomAnnotName = sAnnotName;
    // special handling for SNP2
    if (CSeqUtils::IsExtendedNAA(sAnnotName)) {
        sel.IncludeNamedAnnotAccession(sAnnotName, -1);
        sel.SetCollectNames();
        CGraph_CI graph_iter(Handle.GetScope(), *loc, sel);
        int best_zoom_level = kMax_Int;
        int max_zoom_level = loc->GetTotalRange().GetLength()/2000;
        for (auto& name : graph_iter.GetAnnotNames()) {
            if (!name.IsNamed()) {
                continue;
            }
            string base_name;
            int zoom_level;
            if (!ExtractZoomLevel(name.GetName(), &base_name, &zoom_level) ||
                 base_name != sAnnotName) {
                continue;
            }
            if (zoom_level <= max_zoom_level) {
                if (zoom_level > best_zoom_level || best_zoom_level > max_zoom_level) {
                    best_zoom_level = zoom_level;
                }
            }
            else {
                if (zoom_level < best_zoom_level) {
                    best_zoom_level = zoom_level;
                }
            }
        }
        if (best_zoom_level != kMax_Int) {
            zoomAnnotName = CombineWithZoomLevel(sAnnotName, best_zoom_level);
        }
        sel.SetCollectNames(false);
        sel.ResetAnnotsNames();
    }
    sel.IncludeNamedAnnotAccession(zoomAnnotName);
    sel.AddNamedAnnots(zoomAnnotName);

    return CGraph_CI(Handle.GetScope(), *loc, sel);
}

void CSnpTrack::GenerateAsn1(objects::CSeq_entry& seq_entry, TSeqRange range)
{
#if 1
    CFeat_CI feature_iter(x_GetFeat_CI(range));
//    cerr << "got features: " << feature_iter.GetSize() << endl;
    CRef<CSeq_annot> pAnnot(new CSeq_annot);
    CSeq_annot::TData::TFtable& ftable(pAnnot->SetData().SetFtable());

    for ( ;  feature_iter;  ++feature_iter) {
        CRef<CSeq_feat> pFeat(new CSeq_feat);
        const CMappedFeat& feat = *feature_iter;
//        cerr << "Got feature" << endl << 
//          MSerial_AsnText << feat.GetMappedFeature() << endl;
        pFeat->Assign(feat.GetMappedFeature());
        ftable.push_back(pFeat);
    }
#else
    CRef<CSeq_annot> pAnnot(new CSeq_annot);
    CSeq_annot::TData::TGraph& graph(pAnnot->SetData().SetGraph());
    CGraph_CI graph_iter(x_GetGraph_CI(range));
        for ( ; graph_iter; ++graph_iter) {
            CRef<CSeq_graph> pGraph(new CSeq_graph);
            const CMappedGraph& gr = *graph_iter;
            pGraph->Assign(gr.GetMappedGraph());
            graph.push_back(pGraph);
        }
#endif

    seq_entry.SetAnnot().push_back(pAnnot);
}

bool CSnpTrack::CheckRange(TSeqRange range, string& sMsg)
{
    // this would have been the easiest, but the time to get the size is about the same as the time to iterate
    // so prediction is no better than predicted
    // auto feature_count(x_GetFeat_CI(range).GetSize());
    
    // empirically we can handle about 5M features in reasonable time (100 seconds)
    static size_t s_MaxFeatures(5 * 1000 * 1000);

    // if range is less, instant OK
    if(range.GetLength() < s_MaxFeatures) {
        return true;
    }
    size_t features_estimate(0);
    // if we have an available graph with feature density, we can estimate number of features in our range
    CGraph_CI graph_iter(x_GetGraph_CI(range));

    if(graph_iter) {
        for ( ; graph_iter; ++graph_iter) {
            const CMappedGraph& gr = *graph_iter;
            TSeqRange gr_r  = gr.GetLoc().GetTotalRange();
            if(!gr_r.IntersectingWith(range)) {
                continue;
            }
            TSeqPos comp = gr.GetComp();
            TSeqPos pos  = gr_r.GetFrom();
            double a = gr.IsSetA()? gr.GetA(): 1.;
            double b = gr.IsSetB()? gr.GetB(): 0.;
            if (gr.GetGraph().IsByte()) {
                const CByte_graph::TValues& values = gr.GetGraph().GetByte().GetValues();
                ITERATE(CByte_graph::TValues, iter_gr, values) {
                    TSeqRange pos_r = TSeqRange(pos, pos + comp);
                    if(pos_r.IntersectingWith(range)) {
                        features_estimate += CHistogramGlyph::TDataType(*iter_gr * a + b);
                    }
                    pos += comp;
                }
            } else {
                const CInt_graph::TValues& values = gr.GetGraph().GetInt().GetValues();
                ITERATE(CInt_graph::TValues, iter_gr, values) {
                    TSeqRange pos_r = TSeqRange(pos, pos + comp);
                    if(pos_r.IntersectingWith(range)) {
                        features_estimate += CHistogramGlyph::TDataType(*iter_gr * a + b);
                    }
                    pos += comp;
                }
            }
        }
//        cerr << "features_estimate from graph: " << features_estimate << endl;
    } else {
        // if there's no graph, fall back to sampling a density at start, 1/3, 2/3 and end of range
        // to avoid slowdowns, sampling is done over 100kbp ranges
        static TSeqPos s_SamplingRange(100 * 1000);
        
        auto start_count(x_GetFeat_CI(TSeqRange(range.GetFrom(), range.GetFrom() + s_SamplingRange)).GetSize());
        auto one_third_count(x_GetFeat_CI(TSeqRange(range.GetFrom() + range.GetLength()/3 - s_SamplingRange/2, range.GetFrom() + range.GetLength()/3 + s_SamplingRange/2)).GetSize());
        auto two_third_count(x_GetFeat_CI(TSeqRange(range.GetFrom() + range.GetLength() * 2 / 3 - s_SamplingRange/2, range.GetFrom() + range.GetLength() * 2 / 3 + s_SamplingRange/2)).GetSize());
        auto end_count(x_GetFeat_CI(TSeqRange(range.GetTo() - s_SamplingRange, range.GetTo())).GetSize());
        features_estimate = (double(start_count + one_third_count + two_third_count + end_count) / double(s_SamplingRange * 4)) * range.GetLength();
//        cerr << "features_estimate from sampling: " << features_estimate << endl;
    }
    if(features_estimate > s_MaxFeatures) {
        sMsg = "You have requested a range that contains approximately " + NStr::NumericToString(features_estimate) +
               " SNP records. We do not support downloads of more than " + NStr::NumericToString(s_MaxFeatures) +
               " records. Please reduce the range and try again. If you need the whole SNP dataset, you can go to <a href=\"https://ftp.ncbi.nih.gov/snp/\" target=\"_blank\">dbSNP FTP site</a>.";
        return false;
    }
   return true;
}



void CSnpTrack::x_OnJobCompleted(CAppJobNotification& notify)
{
    CRef<CObject> res_obj;

    res_obj = notify.GetResult();
    m_DS->ClearJobID(notify.GetJobID());

    m_JobResult.Reset(dynamic_cast<SSnpJobResult*>(&*res_obj));
    x_ProcessJobResult();
}


void CSnpTrack::x_ProcessJobResult()
{
    if (!m_JobResult) {
        ERR_POST("CSnpTrack::x_OnJobCompleted() notification for job \
                does not contain results.");
        return;
    }

    // Depending on view, setup icon
    if (x_IsOverviewMode()) {
        x_DeregisterIcon(m_IconLayout.m_Id);
    }
    else {
        x_RegisterIcon(m_IconLayout);
    }

    m_Msg = "";
    if ( !m_JobResult->listObjs.empty() ) {
        switch (m_JobResult->eDataType) {
        default:
        case SSnpJobResult::eHistFromFeats:
            m_Msg += ", density (seq-feats)";
            break;
        case SSnpJobResult::eHistFromGraphs:
            m_Msg += ", density (seq-graphs)";
            break;
        case SSnpJobResult::eComment:
            m_Msg += "";
            break;
        case SSnpJobResult::eFeats:
            {
                unsigned int size = (unsigned int) m_JobResult->listObjs.size();
                m_Msg += ", " + NStr::UIntToString(size) + " features";
            }
            break;
        }
    }

    x_UpdateLayout();
}


/// @}
void CSnpTrack::x_AddSnpsLayout(const SSnpJobResult& result)
{
    const CSeqGlyph::TObjects& objs = result.listObjs;

    m_Group.Clear();
    if (m_Filter.NotNull()) {
        m_PreferredTitle = m_AnnotName + ", " + m_Filter->name;
    }

    if (objs.empty()) {
        m_Attrs |= fNavigable;
        return;
    }

    CRef<CSeqGlyph> first_glyph = objs.front();

    if (typeid(*first_glyph) == typeid(CHistogramGlyph)) {
        CHistogramGlyph *hist = dynamic_cast<CHistogramGlyph*> (first_glyph.GetPointer());
        hist->SetConfig(*x_GetGlobalConfig());
        Add(first_glyph);
    } else if (typeid(*first_glyph) == typeid(CCommentGlyph)) {
        CCommentGlyph* comment =
            dynamic_cast<CCommentGlyph*>(first_glyph.GetPointer());
        CRef<CCommentConfig> c_config(new CCommentConfig);
        c_config->m_ShowBoundary = false;
        c_config->m_ShowConnection = false;
        c_config->m_Centered = true;
        c_config->m_LabelColor.Set(1.0f, 0.0f, 0.0f);
        c_config->m_LineColor.Set(1.0f, 1.0f, 1.0f);
        c_config->m_Font.SetFontFace(CGlTextureFont::eFontFace_Helvetica);
        c_config->m_Font.SetFontSize(12);
        comment->SetConfig(c_config);
        Add(first_glyph);
    } else {
        CGuiRegistry& registry(CGuiRegistry::GetInstance());
        string sColorTheme(x_GetGlobalConfig()->GetColorTheme());
        CRegistryReadView ColorView(CSGConfigUtils::GetColorReadView(registry, "GBPlugins.SnpTrack", "Default", sColorTheme));
        ITERATE(CSeqGlyph::TObjects, it, objs) {
            CSeqGlyph *glyph = const_cast<CSeqGlyph*> (it->GetPointer());
            CFeatGlyph *feat = dynamic_cast<CFeatGlyph*>(glyph);

			// get the SNP feature type and color code the drawn feature accordingly
			const CSeq_feat& or_feat(feat->GetFeature());
            feat->SetConfig(m_Params->GetFeatureParams(or_feat, ColorView));
            Add(glyph);
        }
        m_Attrs |= fNavigable;
    }
}



///////////////////////////////////////////////////////////////////////////////
/// CSnpTrackFactory
///////////////////////////////////////////////////////////////////////////////
ILayoutTrackFactory::TTrackMap
CSnpTrackFactory::CreateTracks(SConstScopedObject& object,
                                  ISGDataSourceContext* ds_context,
                                  CRenderingContext* r_cntx,
                                  const SExtraParams& params,
                                  const TAnnotMetaDataList& src_annots) const
{
    TTrackMap tracks;

    // this is a misnomer
    // in fact, the key is just a unique combination of annotation and filter
    // and the value is the annotation
    TAnnotNameTitleMap annots;

    TKeyValuePairs track_settings;
    CSGConfigUtils::ParseProfileString(params.m_TrackProfile, track_settings);

    if (params.m_SkipGenuineCheck  &&  !params.m_Annots.empty()) {
        ITERATE (SExtraParams::TAnnots, iter, params.m_Annots) {
            // when the annotation is "SNP" then the only way to distinguish between
            // different tracks is the filter which is unique
            // on the other hand, SNP 2.0 tracks are not supposed to have filter at all,
            // so it is always empty, but the extended NAs used for them are always unique
            annots[CSeqUtils::IsExtendedNAA(*iter) ? *iter : params.m_Filter] = *iter;
        }
    } else {
        CSGSnpDS::GetAnnotNames(object, annots, r_cntx->GetVisSeqRange(), params);

        // collect TMS tracks
        if ( !src_annots.empty() ) {
            // Not sure how to distinguish dbSNP from dbVar at this point.
            // In current SADB, there is only dbVar data, no dbSNP data.
            GetMatchedAnnots(src_annots, params, annots);
        }
    }

    // create feature tracks
    ITERATE(TAnnotNameTitleMap, iter, annots) {
        // Create a new DS object for each track
        CIRef<ISGDataSource> pre_ds = ds_context->GetDS(typeid(CSGSnpDSType).name(),
                                                    object);
        CSGSnpDS* seq_ds = dynamic_cast<CSGSnpDS*>(pre_ds.GetPointer());
        seq_ds->SetDepth(params.m_Level);
        seq_ds->SetAdaptive(params.m_Adaptive);

        // again, only those strings that are not extended NAs are considered
        // to be filters
        string sAnnot(iter->second);
        string sTitle;
        if(params.m_AnnotNameTitleMap.count(sAnnot)) {
            sTitle = params.m_AnnotNameTitleMap.at(sAnnot);
        }
        CRef<CSnpTrack> feat_track(new CSnpTrack(seq_ds, 
                                                 r_cntx, 
                                                 CSeqUtils::IsExtendedNAA(iter->first) ? "" : iter->first, 
                                                 sTitle,
                                                 track_settings.count("Layout") ? track_settings["Layout"] : ""));

        feat_track->SetAnnot(sAnnot);

        seq_ds->SetJobListener(feat_track.GetPointer());
        tracks[sAnnot] = feat_track.GetPointer();
    }

    return tracks;
}

void CSnpTrackFactory::GetMatchedAnnots(
    const TAnnotMetaDataList& src_annots,
    const ILayoutTrackFactory::SExtraParams& params,
    TAnnotNameTitleMap& out_annots) const
{
    ILayoutTrackFactory::SExtraParams dummy;
    ILayoutTrackFactory::GetMatchedAnnots(src_annots, dummy.m_Annots, "ftable", "SNP", out_annots);

    // currently SNP2.0 annots are captured by the fact that they are extended NAs
    // this needs to be improved if extended NAs would be used for anything else
    for(auto& i : src_annots) {
        if(CSeqUtils::IsExtendedNAA(i.first, true)) {
            out_annots.insert(TAnnotNameTitleMap::value_type(
                i.first, i.second->m_Title));
        }
    }
}


string CSnpTrackFactory::GetExtensionIdentifier() const
{
    return CSnpTrack::m_TypeInfo.GetId();
}

string CSnpTrackFactory::GetExtensionLabel() const
{
    return CSnpTrack::m_TypeInfo.GetDescr();
}

CRef<CTrackConfigSet>
CSnpTrackFactory::GetSettings(const string& /*profile*/,
                              const TKeyValuePairs& settings,
                              const CTempTrackProxy* /*track_proxy*/) const
{
    CRef<CTrackConfigSet> config_set(new CTrackConfigSet);
    CRef<CTrackConfig> config(new CTrackConfig);
    config_set->Set().push_back(config);
    config->SetHelp() = GetThisTypeInfo().GetDescr();
    config->SetLegend_text("anchor_4");

    CSnpTrack::ELayout layout = CSnpTrack::eLayout_Adaptive;
    ITERATE (TKeyValuePairs, iter, settings) {
        try {
            if (NStr::EqualNocase(iter->first, "layout")) {
                layout = s_LayoutStrToValue(iter->second);
                break;
            }
        } catch (std::exception&) {
        }
    }

    // add layout dropdown list
    CRef<CChoice> choice = CTrackConfigUtils::CreateChoice(
        "Layout", "Rendering options", s_LayoutValueToStr(layout),
        "Controls how variants are displayed");
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            s_LayoutValueToStr(CSnpTrack::eLayout_Adaptive),
            "Show variants for 50 or less",
            "Show labels if less than 15 variants; Show density bar if greater than 50 variants",
            "Lines represent the variant features from dbSNP"));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            s_LayoutValueToStr(CSnpTrack::eLayout_Density),
            "Collapse on single line",
            "Always show the density bar of variants",
            "Lines represent the variant features from dbSNP"));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            s_LayoutValueToStr(CSnpTrack::eLayout_Features),
            "Show all",
            "Show all variants rendered individually",
            "Lines represent the variant features from dbSNP"));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            s_LayoutValueToStr(CSnpTrack::eLayout_Labels),
            "Show variants with labels",
            "Always show the labels of variants",
            "Lines represent the variant features from dbSNP"));
    config->SetChoice_list().push_back(choice);

    return config_set;
}


// gets a SNP class of a feature
// if not a SNP feature or class is unknown, returns CSnpBitfield::eUnknownVariation
static CSnpBitfield::EVariationClass s_GetVariationClass(const CSeq_feat& feat)
{
    return CSnpBitfield(feat).GetVariationClass();
}

// check whether the feature is a SNP insertion
// if not a SNP feature or not an insertion, returns false
static CFeatureParams::EBoxStyle s_GetBoxStyle(const CSeq_feat& feat)
{
    CSnpBitfield bf(feat);
    CFeatureParams::EBoxStyle res(bf.GetWeight() > 1 ? CFeatureParams::eBox_Hollow : CFeatureParams::eBox_Filled);
//    LOG_POST(Trace << "<<<< rs" << NSnp::GetRsid(feat));

	// bitfields pre-SNP 2.0 did not have a specific variation class for insertion, so this information had to be obtained from
	// neighbors info encoded in the "Extra" field as neighbors
	if(bf.GetVersion() < 20) {
        if(feat.IsSetExt()) {
            const CUser_object& user(feat.GetExt());
            // according to SV-1896, a SNP feature in an insertion when rc_ngbr - lc_ngbr == 1
            bool isInsertion(false);
            CConstRef<CUser_field> field(user.GetFieldRef("Extra"));
            if(field) {
                try {
                    // format of the string is "nb=lf_nbgr,rf_ngbr-lf_ngbr,lc_ngbr-asn_from,rc_ngbr-asn_from"
                    // cf. CSnpContigAnnotApp::Run() which writes this value
                    string sExtra(field->GetData().GetStr());
                    vector<string> Ngbrs;
    //				LOG_POST(Trace << "==== Ngbrs: " << sExtra);
                    NStr::Split(sExtra, ",", Ngbrs);
                    if(Ngbrs.size() == 4 &&
                        NStr::StringToNumeric<int>(Ngbrs[3]) - NStr::StringToNumeric<int>(Ngbrs[2]) == 1) {
                        isInsertion = true;
                    }
                }
                catch (std::exception&) {
                }
                if(isInsertion) {
                    res = CFeatureParams::eBox_Insertion;
                } else if(bf.GetVariationClass() == CSnpBitfield::eDips || bf.GetVariationClass() == CSnpBitfield::eDeletion) {
                    res = CFeatureParams::eBox_Deletion;
                }
            }
        }
    } else if(bf.GetVersion() == 20) {
        switch(bf.GetVariationClass()) {
            case CSnpBitfield::eInsertion:
                res = CFeatureParams::eBox_Insertion;
                break;
            case CSnpBitfield::eDeletion:
                res = CFeatureParams::eBox_Deletion;
                break;
            default:
                break;
        }
    } else {
        NCBI_ASSERT(true, "Unexpected SNP bitfield version!");
    }
//    LOG_POST(Trace << ">>>> BoxStyle: " << res);
//    NcbiCerr << MSerial_AsnText << feat << endl;
//    NcbiCerr << "bf version: " << bf.GetVersion() << ", variation class: " << bf.GetVariationClass() << ", BoxStyle: " << res << endl;
    return res;
}


// check that the feature is a known SNP class, return a color that corresponds to it
// if not a SNP feature or class is unknown, returns a default color
CRgbaColor CSNPColorPicker::GetSNPColor(const CSeq_feat& feat, const CRegistryReadView& ColorView)
{
	CSnpBitfield::EVariationClass VariationClass(s_GetVariationClass(feat));
	string sColorKey("Default");

	switch(VariationClass)
	{
	case CSnpBitfield::eSingleBase:
		sColorKey = "SingleBase";
		break;
	case CSnpBitfield::eDips:
		sColorKey = "Dips";
		break;
	case CSnpBitfield::eInsertion:
		sColorKey = "Insertion";
		break;
	case CSnpBitfield::eDeletion:
		sColorKey = "Deletion";
		break;
	case CSnpBitfield::eIdentity:
		sColorKey = "Identity";
		break;
	case CSnpBitfield::eInversion:
		sColorKey = "Inversion";
		break;
	case CSnpBitfield::eMultiBase:
		sColorKey = "MultiBase";
		break;
	case CSnpBitfield::eHeterozygous:
		sColorKey = "Heterozygous";
		break;
	case CSnpBitfield::eMicrosatellite:
		sColorKey = "Microsatellite";
		break;
	case CSnpBitfield::eNamedSNP:
		sColorKey = "NamedSNP";
		break;
	case CSnpBitfield::eNoVariation:
		sColorKey = "NoVariation";
		break;
	case CSnpBitfield::eMixed:
		sColorKey = "Mixed";
		break;
	case CSnpBitfield::eUnknownVariation:
	default:
		break;
	}
	CRgbaColor color;
	CSGConfigUtils::GetColor(ColorView, sColorKey, color);
	return color;
}


// create all future params based on the OrigParams
CRef<CSNPFeatureParams> CSNPFeatureParams::CreateSNPFeatureParams()
{
    return CRef<CSNPFeatureParams> (new CSNPFeatureParams());
}

CRef<CSNPFeatureParams> CSNPFeatureParams::CreateSNPFeatureParams(CConstRef<CFeatureParams> OrigParams)
{
    return CRef<CSNPFeatureParams> (new CSNPFeatureParams(OrigParams));
}

void CSNPFeatureParams::Reset(CConstRef<CFeatureParams> OrigParams)
{
    m_ParamsTemplate.Reset(new CFeatureParams);
    *m_ParamsTemplate = *OrigParams;
    m_ParamsCache.clear();
}

void CSNPFeatureParams::UpdateLabelPos()
{
    NON_CONST_ITERATE (TParamsCache, iter, m_ParamsCache) {
        CFeatureParams& param = const_cast<CFeatureParams&>(*iter->second);
        param.m_LabelPos = m_ParamsTemplate->m_LabelPos;
    }
}

// get parameters for a given SNP feature, they are based on m_ParamsTemplate, but modified according to
// the feature contents
CConstRef<CFeatureParams> CSNPFeatureParams::GetFeatureParams(const objects::CSeq_feat& feat, const CRegistryReadView& ColorView) const
{
	CSnpBitfield::EVariationClass VariationClass(s_GetVariationClass(feat));
    auto BoxStyle(s_GetBoxStyle(feat));
	SSNPFeatParamDefiner ParamDefiner(VariationClass, BoxStyle);

    TParamsCache::iterator iParamsCache(m_ParamsCache.find(ParamDefiner));

    if(iParamsCache == m_ParamsCache.end()) {
		CRef<CFeatureParams> feat_params(new CFeatureParams());
		*feat_params = *m_ParamsTemplate;
		feat_params->m_fgColor = CSNPColorPicker::GetSNPColor(feat, ColorView);
		feat_params->m_BoxStyle = BoxStyle;

        m_ParamsCache[ParamDefiner] = feat_params;
        return feat_params;
    }
    else
        return iParamsCache->second;
}



END_NCBI_SCOPE
