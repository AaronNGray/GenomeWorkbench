/*  $Id: graph_track.cpp 44362 2019-12-05 16:29:20Z shkeda $
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
 * Authors:  Liangshou Wu, Dmitry Rudnev, Andrei Shkeda
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/seq_graphic/graph_track.hpp>
#include <gui/widgets/seq_graphic/graph_overlay.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_graph_ds.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job_result.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
//#include <gui/widgets/seq_graphic/seqgraphic_widget.hpp>
#include <gui/widgets/seq_graphic/layout_track_proxy.hpp>
#include <gui/widgets/seq_graphic/comment_glyph.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
//#include <gui/widgets/wx/ui_command.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>

#include <gui/objects/LegendItem.hpp>
#include <gui/objects/Conditional.hpp>

#include <objects/general/Object_id.hpp>
#include <corelib/rwstream.hpp>
#include <connect/services/neticache_client.hpp>
#include <serial/objistr.hpp>
#include <serial/objostr.hpp>
#include <serial/serial.hpp>
#include <util/checksum.hpp>

#include <wx/menu.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
///   CGraphTrack

static const string kTrackTitle           = "Graphs";
static const string kScale                = "scale";
static const string kStoredScale          = "stored_scale";
static const string kClipOutliers         = "clip";
static const string kOutlierColor         = "ocolor";
static const string kSDeviationThreshold  = "sdthresh";
static const string kValueRange           = "range";
static const string kNumBins              = "num_bins";

static const string kNegColor  = "neg_color";

const string CGraphTrack::kOverlayed = "_overlayed"; // used to suffix annot name for the overl;ayed tracks

CTrackTypeInfo CGraphTrack::m_TypeInfo("graph_track",
                                       "Graph track");

static const CGraphTrack::ELayout kDefaultLayout = CGraphTrack::eLayout_Layered;
static const bool kDefaultFixedScale = false;

static
string s_GetCacheKey(const string& annot_name)
{
    CChecksum cs(CChecksum::eMD5);
    cs.AddLine(annot_name);
    string key = cs.GetHexSum();
    key += "_ylimits";
    return key;
} 

CGraphTrack::CGraphTrack(CSGGraphDS* ds, CRenderingContext* r_cntx)
    : CDataTrack(r_cntx)
    , m_DS(ds)
    , m_FixedScale(kDefaultFixedScale)
    , m_Layout(kDefaultLayout)
{
    m_YLimits.min = numeric_limits<double>::max();
    m_YLimits.max = numeric_limits<double>::min();
    m_DS->SetJobListener(this);
    SetLayoutPolicy(m_Simple);

    x_RegisterIcon(SIconInfo(
        eIcon_Settings, "Settings", true, "track_settings"));

    // initialize annotation selector
    if (m_DS->IsSeqTable()) {
        m_Sel = CSeqUtils::GetAnnotSelector(CSeq_annot::C_Data::e_Seq_table);
    } else {
        m_Sel = CSeqUtils::GetAnnotSelector(CSeq_annot::C_Data::e_Graph);
    }
}


CGraphTrack::~CGraphTrack()
{
    if ( !m_DS->AllJobsFinished() ) {
        m_DS->DeleteAllJobs();
    }
}


const CTrackTypeInfo& CGraphTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}

CRef<CSGGenBankDS> CGraphTrack::GetDataSource()
{
    return CRef<CSGGenBankDS>(m_DS.GetPointer());
}


CConstRef<CSGGenBankDS> CGraphTrack::GetDataSource() const
{
    return CConstRef<CSGGenBankDS>(m_DS.GetPointer());
}


void CGraphTrack::GetHTMLActiveAreas(TAreaVector* p_areas) const
{

    if (m_Layout != eLayout_Overlay && m_Layout != eLayout_Overlayed) 
        CGlyphContainer::GetHTMLActiveAreas(p_areas);
}


CHTMLActiveArea* CGraphTrack::InitHTMLActiveArea(TAreaVector* p_areas) const
{

    // for cases that there is only one graph, we initialize
    // it differently.
    if (GetChildren().size() == 1) {
        TModelUnit tb_height = x_GetTBHeight();

        // get child/graph's signature
        if (GetId().empty()) {
            GetChildren().front()->GetHTMLActiveAreas(p_areas);
        } else {
            TAreaVector areas;
            GetChildren().front()->GetHTMLActiveAreas(&areas);
            for (auto& area : areas) {
                area.m_ParentId = GetId();
                area.m_Descr.clear();
                area.m_Flags &= ~CHTMLActiveArea::fTooltipEmbedded;
                if(CSeqUtils::isRmtAnnotName(m_AnnotName) 
                    && !m_Title.empty() 
                    && area.m_Signature != "otl" /* exlude outlier tooltips*/) {
                    area.m_Descr = m_Title;
                }
            }
            p_areas->insert(p_areas->end(), areas.begin(), areas.end());
        }

        // but use track range and size
        CHTMLActiveArea& area = p_areas->back();
        CSeqGlyph::x_InitHTMLActiveArea(area);
        TVPRect& bound = area.m_Bounds;
        bound.SetBottom(bound.Top() + (TVPUnit)tb_height + (tb_height > 0.0 ? 2 : 0));
        // add is_track flag
        area.m_Flags |= CHTMLActiveArea::fTrack | CHTMLActiveArea::fNoNavigation;
        return &area;
    }    
    return CLayoutTrack::InitHTMLActiveArea(p_areas);
}


void CGraphTrack::SetAnnot(const string& annot)
{
    m_AnnotName = annot;
    m_Sel.ResetAnnotsNames();
    if (CSeqUtils::NameTypeStrToValue(m_AnnotName) ==
        CSeqUtils::eAnnot_Unnamed) {
        m_Sel.AddUnnamedAnnots();
    } else {
        m_Sel.AddNamedAnnots(m_AnnotName);
        if (NStr::StartsWith(m_AnnotName, "NA0")) {
            if (m_AnnotName.find(".") == string::npos) {
                m_AnnotName += ".1";
            }
            m_Sel.IncludeNamedAnnotAccession(m_AnnotName);
        }
    }
}


struct SCustomizedHistSettings
{
    SCustomizedHistSettings()
        : m_Height(-1)
    {}

    int    m_Height;
    string m_Type;
    string m_Color;
};


CNetICacheClient CGraphTrack::s_InitICache()
{
    const CNcbiRegistry& reg = CNcbiApplication::Instance()->GetConfig();
    string cache_svc = reg.GetString("Configuration", "service", "NC_SV_Conf_PROD");
    string cache_name = reg.GetString("Configuration", "cache", "conf");
    return CNetICacheClient(cache_svc, cache_name, "sviewer");
}

CNetICacheClient& CGraphTrack::s_GetICacheInstance()
{
    static CNetICacheClient cache_instance(s_InitICache());
    return cache_instance;
}

CGraphTrack::ELayout CGraphTrack::x_LayoutStrToValue(const string& layout)
{
    if (NStr::EqualNocase(layout, "layered"))
        return eLayout_Layered;
    if (NStr::EqualNocase(layout, "overlay"))
        return eLayout_Overlay;
    return eLayout_Layered;
}

string CGraphTrack::x_LayoutValueToStr(CGraphTrack::ELayout layout)
{
    switch (layout) {
    case eLayout_Layered:
        return "layered";
    case eLayout_Overlay:
        return "overlay";
    default:
        break;
    }
    return "layered";
}


void CGraphTrack::x_LoadSettings(const string& /*preset_style*/,
                                 const TKeyValuePairs& settings)
{
    _ASSERT(m_gConfig);
    if (!m_gConfig)
        return;
    // Load min/max for Y axis
    if (m_gConfig->GetCgiMode()) try {
        CNetICacheClient &icache = s_GetICacheInstance();
        string key = s_GetCacheKey(m_AnnotName);
        if (icache.HasBlobs(key, NcbiEmptyString)) {
            CRef<CUser_object> uo = Ref(new CUser_object);
            auto_ptr<IReader> reader(icache.GetReadStream(key, 0, ""));
            auto_ptr<CNcbiIstream> strm(new CRStream(reader.release(), 0, 0, CRWStreambuf::fOwnReader));
            auto_ptr<CObjectIStream> obj_str(CObjectIStream::Open(eSerial_AsnBinary, *strm));
            *obj_str >> *uo;
            obj_str->Close();
            if (uo->HasField("min"))
                m_YLimits.min = min(m_YLimits.min, uo->GetField("min").GetData().GetReal());
            if (uo->HasField("max"))
                m_YLimits.max = max(m_YLimits.max, uo->GetField("max").GetData().GetReal());
        }
    } catch (exception& e) {
        ERR_POST(Error << e.what());
    }

    SCustomizedHistSettings hist_settings;
    // Parse known parameters
    bool parameter_present = false, smooth_curve = false;
    string scale, clip_outliers, outlier_color, sd_thresh, stored_scale, num_bins, neg_color, value_range;
    int opacity = 100;
    int fit_step = 4;
    ITERATE (TKeyValuePairs, iter, settings) {
        try {
            // cerr << "==== key: " << iter->first << ", value: " << iter->second << endl;
            if (NStr::EqualNocase(iter->first, "fixed_scale")) {
                m_FixedScale = NStr::StringToBool(iter->second);
            } else 
            if (NStr::EqualNocase(iter->first, "layout")) {
                m_Layout = x_LayoutStrToValue(iter->second);
            } else if (NStr::EqualNocase(iter->first, "color")) {
                hist_settings.m_Color = iter->second;
                parameter_present = true;
            } else if (NStr::EqualNocase(iter->first, "opacity")) {
                opacity = NStr::StringToInt(iter->second, NStr::fConvErr_NoThrow);
                opacity = (opacity <= 0) ? 100 : min(opacity, 100);
                parameter_present = true;
            } else if (NStr::EqualNocase(iter->first, "height")) {
                hist_settings.m_Height = NStr::StringToInt(iter->second);
                parameter_present = true;
            } else if (NStr::EqualNocase(iter->first, "style")) {
                hist_settings.m_Type = iter->second;
                parameter_present = true;
            } else if (NStr::EqualNocase(iter->first, kScale)) {
                scale = iter->second;
                parameter_present = true;
            } else if (NStr::EqualNocase(iter->first, kStoredScale) || NStr::EqualNocase(iter->first, "is_scaled")) {
                stored_scale = iter->second;
                parameter_present = true;
            //} else if (NStr::EqualNocase(iter->first, kClipOutliers)) {
            //    clip_outliers = iter->second;
            //    parameter_present = true;
            } else if (NStr::EqualNocase(iter->first, kOutlierColor)) {
                outlier_color = iter->second;
                parameter_present = true;
            } else if (NStr::EqualNocase(iter->first, kSDeviationThreshold)) {
                sd_thresh = iter->second;
                parameter_present = true;
            } else if (NStr::EqualNocase(iter->first, kNegColor)) {
                neg_color = iter->second;
                parameter_present = true;
            } else if (NStr::EqualNocase(iter->first, kNumBins)) {
                num_bins = iter->second;
                parameter_present = true;
            } else if (NStr::EqualNocase(iter->first, kValueRange)) {
                value_range = iter->second;
                parameter_present = true;
            } else if (NStr::EqualNocase(iter->first, "smooth_curve")) {
                try {
                    smooth_curve = NStr::StringToBool(iter->second);
                } catch (exception& e) {
                    ERR_POST(Error << e.what());
                }
                parameter_present = true;
            } else if (NStr::EqualNocase(iter->first, "fit_step")) {
                fit_step = NStr::StringToInt(iter->second);
                fit_step = max<int>(fit_step, 1);
                parameter_present = true;
            }
        } catch (std::exception&) {
            LOG_POST(Error << "CGraphTrack::x_LoadSettings: invalid settings "
                     << iter->second);
        }
    }
    // cerr << "==== scale: " << scale << endl;
    // cerr << "==== stored_scale: " << stored_scale << endl;

    if (!parameter_present)
        return;

    CRef<CHistParamsManager> conf_mgr = m_gConfig->GetHistParamsManager();
    CRef<CHistParams> hist_conf;

    if (conf_mgr->HasSettings(m_AnnotName)) {
        // Get the corresponding setting for this data track if exists
        hist_conf = conf_mgr->GetHistParams(m_AnnotName);
    } else {
        CRef<CHistParams> def_conf = conf_mgr->GetDefHistParams();
        hist_conf.Reset(new CHistParams(*def_conf));
        conf_mgr->AddSettings(m_AnnotName, hist_conf);
    }

    try {
        if ( !hist_settings.m_Color.empty() ) {
            CRgbaColor color(hist_settings.m_Color);
            hist_conf->m_fgColor = color;

            // We also use the only color setting as the main color
            // for smear bar or heat map, but use darken version as
            // the max color and lighten version as min color.
            hist_conf->m_SmearColorMax = color;
            hist_conf->m_SmearColorMin = color;
            hist_conf->m_SmearColorMin.Lighten(0.5f);
            hist_conf->m_SmearColorMin.SetAlpha(0.4f);
        }
        if (!neg_color.empty())
            hist_conf->m_fgNegColor = CRgbaColor(neg_color);
        float alpha = opacity;
        alpha /= 100.;
        hist_conf->m_fgColor.SetAlpha(alpha);
        hist_conf->m_SmearColorMax.SetAlpha(alpha);
        hist_conf->m_SmearColorMin.SetAlpha(alpha);
        hist_conf->m_fgNegColor.SetAlpha(alpha);

        hist_conf->m_SmoothCurve = smooth_curve;
        hist_conf->m_FitStep = fit_step;

        if ( !hist_settings.m_Type.empty() ) {
            CHistParams::EType style = CHistParams::TypeStrToValue(hist_settings.m_Type);
            hist_conf->m_Type = style;

            if (style == CHistParams::eSmearBar) {
                // use a different default height for smearbar from histgoram
                hist_conf->m_Height = 10.0;

                // for smearbar, change the background color to use lighter
                // version of foreground color, and enable background color
                hist_conf->m_DrawBg = true;
                hist_conf->m_bgColor = hist_conf->m_fgColor;
                hist_conf->m_bgColor.Lighten(0.8f);
                hist_conf->m_bgColor.SetAlpha(0.2f);
            }
        }

        if (hist_settings.m_Height > 0) {
            // use user-provided size
            hist_conf->m_Height = hist_settings.m_Height;
        }

        if (!scale.empty()) {
            hist_conf->m_Scale = CHistParams::ScaleStrToValue(scale);
        }

        if (!stored_scale.empty()) {
            hist_conf->m_StoredScale = CHistParams::ScaleStrToValue(stored_scale);
        }

        if (!clip_outliers.empty()) {
            hist_conf->m_ClipOutliers = NStr::StringToBool(clip_outliers);
        }

        if (!outlier_color.empty()) {
            hist_conf->m_OutlierColor.FromString(outlier_color);
        }

        if (!sd_thresh.empty()) {
            hist_conf->m_SDeviationThreshold = NStr::StringToInt(sd_thresh);
        }

        if (!num_bins.empty()) {
            hist_conf->m_NumBins = NStr::StringToNonNegativeInt(num_bins, NStr::fConvErr_NoThrow);
            hist_conf->m_NumBins = min(hist_conf->m_NumBins, 10);
        }

        if (!value_range.empty()) {
            string range_min, range_max;
            CTrackConfigUtils::DecodeValueRange(value_range, range_min, range_max, hist_conf->m_RangeAutoscale);
            if (!range_min.empty() && range_min != "inf") {
                hist_conf->m_ValueRange.SetFrom(NStr::StringToNumeric<float>(range_min, NStr::fConvErr_NoThrow));
            }
            if (!range_max.empty() && range_max != "inf") {
                hist_conf->m_ValueRange.SetTo(NStr::StringToNumeric<float>(range_max, NStr::fConvErr_NoThrow));
            }
            bool not_empty = hist_conf->m_ValueRange.GetFrom() != hist_conf->m_ValueRange.GetEmptyFrom()
                && hist_conf->m_ValueRange.GetTo() != hist_conf->m_ValueRange.GetEmptyTo();
            if (not_empty) {
                // swap if from > to
                if (hist_conf->m_ValueRange.GetFrom() > hist_conf->m_ValueRange.GetTo()) {
                    hist_conf->m_ValueRange.Set(hist_conf->m_ValueRange.GetTo(), hist_conf->m_ValueRange.GetFrom());
                }
            }

        }

    } catch (std::exception&) {
        LOG_POST(Error << "CGraphTrack::x_LoadSettings: invalid histogram settings");
    }
}


void CGraphTrack::x_SaveSettings(const string& /*preset_style*/)
{
    vector<string> settings;
    if (m_FixedScale != kDefaultFixedScale)
        settings.push_back("fixed_scale:" + NStr::BoolToString(m_FixedScale));
    if (m_Layout != kDefaultLayout)
        settings.push_back("layout:" + x_LayoutValueToStr(m_Layout));
    string profile = settings.empty() ? NcbiEmptyString : NStr::Join(settings, ",");
    SetProfile(profile);
}


void CGraphTrack::x_OnIconClicked(TIconID id){
    switch (id) {
    case eIcon_Settings:
        x_OnSettingsIconClicked();
        break;
    default:
        // use default handlers
        CLayoutTrack::x_OnIconClicked(id);
    }
}


void CGraphTrack::x_UpdateData()
{
    CDataTrack::x_UpdateData();
    m_DS->DeleteAllJobs();
    x_SetStartStatus();
    m_DS->LoadSeqGraphs(m_AnnotName, m_Sel, *m_Context, m_FixedScale, m_YLimits, x_GetGlobalConfig());
}


void CGraphTrack::x_OnJobCompleted(CAppJobNotification& notify)
{
    m_DS->ClearJobID(notify.GetJobID());
    CRef<CObject> res_obj = notify.GetResult();
    CSGJobResult* result = dynamic_cast<CSGJobResult*>(&*res_obj);
    if (!result) {
        ERR_POST("CGraphTrack::x_OnJobCompleted() notification for job \
                does not contain results.");
        return;
    }

    SetGroup().Clear();
    SetMsg("");
    size_t count = result->m_ObjectList.size();
    if (count > 0) {
        TObjects& objs = result->m_ObjectList;
        vector<string> titles;
        CSGGraphDS::TAxisLimits y_limits = m_YLimits;

        NON_CONST_ITERATE (CSeqGlyph::TObjects, iter, objs) {
            CHistogramGlyph* hist =
               dynamic_cast<CHistogramGlyph*>(iter->GetPointer());
            hist->SetConfig(*x_GetGlobalConfig());
            string title = hist->GetTitle();
            m_YLimits.max = max((double)hist->GetAxisMax(), m_YLimits.max);
            m_YLimits.min = min((double)hist->GetAxisMin(), m_YLimits.min);

            //title += ", Max:" + NStr::UIntToString((int)hist->GetAxisMax());
            //title += " / Min:" + NStr::UIntToString((int)hist->GetAxisMin());
            titles.push_back(title);
        }
        // Save min/max for Y axis
        if (m_gConfig && m_gConfig->GetCgiMode()
            && (m_YLimits.min != y_limits.min || m_YLimits.max != y_limits.max)) try {
            CNetICacheClient &icache = s_GetICacheInstance();
            CRef<CUser_object> uo = Ref(new CUser_object);
            uo->SetType().SetStr("YLimits");
            uo->AddField("min", m_YLimits.min);
            uo->AddField("max", m_YLimits.max);
            string key = s_GetCacheKey(m_AnnotName);
            auto_ptr<IWriter> writer(icache.GetWriteStream(key, 0, NcbiEmptyString));
            auto_ptr<CNcbiOstream> strm(new CWStream(writer.release(), 0, 0, CRWStreambuf::fOwnWriter));
            auto_ptr<CObjectOStream> obj_str(CObjectOStream::Open(eSerial_AsnBinary, *strm));
            *obj_str << *uo;
            obj_str->Close();
        } catch (exception& e) {
            ERR_POST(Error << e.what());
        }

        if (count == 1) {
            CHistogramGlyph* hg =
                dynamic_cast<CHistogramGlyph*>(objs.front().GetPointer());
            _ASSERT(hg);
            if (m_Layout == eLayout_Overlay || m_Layout == eLayout_Overlayed) {
                hg->SetAnnotName(m_Layout == eLayout_Overlayed ? m_AnnotName + kOverlayed : m_AnnotName);
                hg->SetRenderingMode(m_Layout == eLayout_Overlayed  ? 
                    CHistogramGlyph::eMode_Overlay : CHistogramGlyph::eMode_Single);
                Add(objs.front());
                CRef<COverlayLayout> ol(new COverlayLayout);
                SetLayoutPolicy(ol.GetPointer());
            }
            if (m_Title.empty())
                m_Title = titles[0];
            else if (NStr::StartsWith(titles[0], "NA")) {
                hg->SetDesc(m_Title);
            }

            SetObjects(objs);
        } else {
            string msg = ", " + NStr::SizetToString(count) + " graph";
            msg += (count != 1 ? "s shown" : " shown");
            SetMsg(msg);
            if (m_Title.empty())
                m_Title = m_AnnotName;
            if (m_Layout == eLayout_Overlay || m_Layout == eLayout_Overlayed) {

                NON_CONST_ITERATE(CSeqGlyph::TObjects, iter, objs) {
                    CHistogramGlyph* hg =
                        dynamic_cast<CHistogramGlyph*>(iter->GetPointer());
                    _ASSERT(hg);
                    hg->SetAnnotName(m_Layout == eLayout_Overlayed ? m_AnnotName + kOverlayed : m_AnnotName);
                    hg->SetRenderingMode(CHistogramGlyph::eMode_Overlay);
                    Add(*iter);
                }
                CRef<COverlayLayout> ol(new COverlayLayout);
                SetLayoutPolicy(ol.GetPointer());
            } else {
                int i = 0;
                CRef<CCommentConfig> config(new CCommentConfig());
                config->m_LabelColor = CRgbaColor("blue");
                config->m_ShowConnection = false;
                NON_CONST_ITERATE(CSeqGlyph::TObjects, iter, objs)
                {
                    CRef<CLayoutGroup> group(new CLayoutGroup);
                    Add(group);
                    group->SetLayoutPolicy(m_Simple);
                    CRef<CCommentGlyph> label(
                        new CCommentGlyph(titles[i++], *iter));
                    label->SetConfig(config);
                    group->PushBack(label);
                    CHistogramGlyph* hg =
                        dynamic_cast<CHistogramGlyph*>(iter->GetPointer());
                    _ASSERT(hg);
                    if (hg->GetAnnotName().empty())
                        hg->SetAnnotName(m_AnnotName);
                    hg->SetRenderingMode(CHistogramGlyph::eMode_Single);

                    group->PushBack(*iter);
                }
                SetLayoutPolicy(m_Layered);
            }
        }
    }

    x_UpdateLayout();
}

CTrackProxy* s_FindTrackProxy(CTrackContainer& p_track, int order)
{
    CTrackProxy* track_proxy = 0;
    NON_CONST_ITERATE(CTrackContainer::TTrackProxies, iter, p_track.GetSubtrackProxies())
    {
        track_proxy = dynamic_cast<CTrackProxy*>(&**iter);
        if (!track_proxy)
            continue;
        if ((*iter)->GetOrder() == order)
            return track_proxy;
    }
    return 0;
}


void CGraphTrack::x_OnSettingsIconClicked()
{
    wxMenu menu;
    UseDefaultMarginWidth(menu);
    int id_base = 10000;
    wxMenuItem* item = menu.AppendCheckItem(id_base++, wxT("Fixed scale"));
    if (m_FixedScale) {
        item->Check();
    }
    vector<CRef<CHistogramGlyph>> graphs;
    vector<wxMenuItem*> settings_menu;
    x_CollectAllGraphs(graphs);
    if (!graphs.empty()) {
        menu.AppendSeparator();
        for (auto graph : graphs) {
            wxString s = ToWxString(graph->GetTitle());
            static const int kMaxMenuItemLen = 50;
            if (s.Length() > kMaxMenuItemLen) {
                s.Truncate(kMaxMenuItemLen - 3);
                s.Pad(3, '.');
            }
            settings_menu.push_back(menu.AppendCheckItem(id_base++, s));
        }
    }

    CTrackContainer* p_track = dynamic_cast<CTrackContainer*>(GetParentTrack());
    wxMenuItem* move_item = 0;
    if (p_track && p_track->GetParentTrack() != 0) { // if it's inside a graph container
        menu.AppendSeparator();
        move_item = menu.AppendCheckItem(id_base, wxT("Remove from parent container"));
//        CUICommandRegistry::GetInstance().AppendMenuItem(menu, eCmdRemoveTrackFromOverlay);
    }

    m_LTHost->LTH_PopupMenu(&menu);

    if (m_FixedScale != item->IsChecked()) {
        m_FixedScale = item->IsChecked();
        x_UpdateData();
    }
    for (size_t i = 0; i < settings_menu.size() && i < graphs.size(); ++i) {
        if (!settings_menu[i]->IsChecked())
            continue;
        graphs[i]->RunSettingsDialog();
        break;
    }
    if (move_item && move_item->IsChecked()) {
        CTrackContainer* top_container = dynamic_cast<CTrackContainer*>(p_track->GetParentTrack());
        if (!top_container)
            return;
        CRef<CTempTrackProxy> proxy(0);
        NON_CONST_ITERATE(CTrackContainer::TTrackProxies, iter, p_track->GetSubtrackProxies())
        {
            if ((*iter)->GetOrder() == GetOrder()) {
                proxy.Reset(*iter);
            }
        }
        if (!proxy)
            return;
        CTrackProxy* parent_proxy(s_FindTrackProxy(*top_container, p_track->GetOrder()));
        _ASSERT(parent_proxy);
        if (!parent_proxy)
            return;

        // Find order for the removed track
        int new_order = 1;
        ITERATE(CTrackProxy::TTrackProxies, ch_it, top_container->GetSubtrackProxies()) {
            if ((*ch_it)->GetOrder() >= new_order) {
                new_order = (*ch_it)->GetOrder() + 1;
            }
        }

        ERASE_ITERATE(CTrackProxy::TTrackProxies, it, parent_proxy->GetChildren()) {
            if ((*it)->GetOrder() == GetOrder()) {
                VECTOR_ERASE(it, parent_proxy->GetChildren());
                break;
            }
        }
        p_track->RemoveTrack(GetOrder());

        CRef<CTrackProxy> n_proxy(new CTrackProxy);
        n_proxy->CTempTrackProxy::operator=(*proxy);
        n_proxy->SetKey("graph_track");
        n_proxy->SetOrder(new_order);
        n_proxy->SetTrack(proxy->GetTrack());
        n_proxy->GetTrack()->SetShowTitle(true);
        CGraphTrack* gt = dynamic_cast<CGraphTrack*>(n_proxy->GetTrack());
        _ASSERT(gt);
        if (gt) {
            CTrackProxy::TAnnots annots;
            annots.push_back(gt->GetAnnot());
            n_proxy->SetAnnots(annots);
            gt->SetLayout(CGraphTrack::eLayout_Layered);
        }
        top_container->AddTrackProxy(n_proxy.GetPointer());
        // Move right after the fromer parent track
        top_container->MoveTrack(new_order, GetOrder() + 1);

        if (p_track->GetChildren().empty()) {
            top_container->RemoveTrack(p_track->GetOrder());
        }
        top_container->Update(true);
    }
}


bool CGraphTrack::CreateOverlay(vector<CRef<CGraphTrack>>& tracks)
{
    CTrackContainer* p_track = dynamic_cast<CTrackContainer*>(GetParentTrack());
    if (!p_track)
        return false;
    vector<int> tracks2remove; // tracks that will be removed from the main panel
    const CTrackProxy* current_track_proxy = s_FindTrackProxy(*p_track, GetOrder());
    if (!current_track_proxy)
        return false;
    tracks2remove.push_back(current_track_proxy->GetOrder());

    CRef<CTempTrackProxy> new_track(new CTrackProxy(current_track_proxy->GetOrder(),
        "graph overlay", true, "graph_overlay", NcbiEmptyString));
    if (!current_track_proxy->GetId().empty())
        new_track->SetSubTracks().push_back(current_track_proxy->GetId());
    else {
        copy(current_track_proxy->GetAnnots().begin(), current_track_proxy->GetAnnots().end(),
             back_inserter(new_track->SetSubTracks()));
    }
    int order = 1;
    CRef<CTempTrackProxy> subtrack_proxy = current_track_proxy->Clone();
    subtrack_proxy->SetOrder(order++);
    new_track->GetChildren().push_back(subtrack_proxy);

    NON_CONST_ITERATE(vector<CRef<CGraphTrack>>, it, tracks) {
        CGraphTrack* gt = &**it;
        current_track_proxy = s_FindTrackProxy(*p_track, gt->GetOrder());
        if (!current_track_proxy)
            continue;
        _ASSERT(current_track_proxy->GetKey() == "graph_track");
        CRef<CTempTrackProxy> subtrack_proxy(current_track_proxy->Clone());
        subtrack_proxy->SetOrder(order++);
        new_track->GetChildren().push_back(subtrack_proxy);

        if (!current_track_proxy->GetId().empty())
            new_track->SetSubTracks().push_back(current_track_proxy->GetId());
        else {
            copy(current_track_proxy->GetAnnots().begin(), current_track_proxy->GetAnnots().end(),
                 back_inserter(new_track->SetSubTracks()));
        }
        tracks2remove.push_back(gt->GetOrder());
    }
    string track_source = "graphs:";
    track_source += NStr::Join(new_track->GetSubTracks(), "-");
    
    CTempTrackProxy::TAnnots annots;
    annots.push_back(track_source);
    new_track->SetAnnots(annots);

    CRef<CHistParamsManager> conf_mgr = m_gConfig->GetHistParamsManager();
    CRef<CHistParams> overlay_params = conf_mgr->GetHistParams(track_source);
    CRef<CHistParams> this_params = conf_mgr->GetHistParams(m_AnnotName);
    *overlay_params = *this_params;

    ITERATE(vector<int>, it, tracks2remove) {
        p_track->RemoveTrack(*it);
    }

    p_track->AddNewTrack(dynamic_cast<CTrackProxy*>(&*new_track));
    return true;
}


///////////////////////////////////////////////////////////////////////////////
/// CGraphTrackFactory methods
static const string kSeqTable = "seq-table";
static const string kSeqGraph = "seq-graph";

ILayoutTrackFactory::TTrackMap
CGraphTrackFactory::CreateTracks(SConstScopedObject& object,
                                 ISGDataSourceContext* ds_context,
                                 CRenderingContext* r_cntx,
                                 const SExtraParams& params,
                                 const TAnnotMetaDataList& src_annots) const
{
    // we need to separate tracks stored as seq-graph from seq-table
    // to initialize then differently.
    typedef map<string, TAnnotNameTitleMap> TAnnotGroup;
    TAnnotGroup annot_group;
    annot_group.insert(TAnnotGroup::value_type(kSeqTable, TAnnotNameTitleMap()));
    annot_group.insert(TAnnotGroup::value_type(kSeqGraph, TAnnotNameTitleMap()));

    TKeyValuePairs track_settings;
    CSGConfigUtils::ParseProfileString(params.m_TrackProfile, track_settings);
    string graph_cache_key;
    CTrackUtils::GetKey(track_settings, "graph_cache_key", graph_cache_key);
    bool is_cached_bigwig = false;
    if (!graph_cache_key.empty()) {
        string rmt_type;
        CTrackUtils::GetKey(track_settings, "rmt_type", rmt_type);
        is_cached_bigwig = NStr::EqualNocase(rmt_type, "bigWig");
    }

//    LOG_POST(Trace << "params.m_Annot.size(): " << params.m_Annots.size() <<
//                       ", params.m_SkipGenuineCheck: " << params.m_SkipGenuineCheck <<
//                       ", params.m_CoverageGraphCheck: " << params.m_CoverageGraphCheck);
    if (is_cached_bigwig) {
        // skip all annot discovery for remote tracks
        annot_group[kSeqGraph].insert(TAnnotNameTitleMap::value_type(params.m_Annots.front(), ""));
    } else if (params.m_SkipGenuineCheck  &&  !params.m_Annots.empty()) {
        bool isCoverageCheckFinished{false};
        if (params.m_CoverageGraphCheck) {
            try {
                // filter out the seq-table annots
                TAnnotNameTitleMap seqtable_annots;
                SAnnotSelector sel(CSeqUtils::GetAnnotSelector(params.m_Annots));
                sel.ExcludeNamedAnnots("SNP");
                CIRef<ISGDataSource> ds = ds_context->GetDS(
                    typeid(CSGGraphDSType).name(), object);
                CSGGraphDS* graph_ds = dynamic_cast<CSGGraphDS*>(ds.GetPointer());
                graph_ds->SetDepth(params.m_Level);
                graph_ds->SetAdaptive(params.m_Adaptive);
                graph_ds->GetAnnotNames(sel, TSeqRange::GetWhole(), seqtable_annots, true);

    //            LOG_POST(Trace << "seqtable_annots.size(): " << seqtable_annots.size());
    //            for(auto i_seqtable_annots: seqtable_annots) {
    //                LOG_POST(Trace << i_seqtable_annots.first << ": " << i_seqtable_annots.second);
    //            }

                ITERATE (SExtraParams::TAnnots, iter, params.m_Annots) {
                    if (seqtable_annots.count(*iter) > 0) {
                        annot_group[kSeqTable].insert(TAnnotNameTitleMap::value_type(*iter, ""));
    //                    LOG_POST(Trace << "adding to annot_group[kSeqTable]: " << *iter);
                    } else {
                        annot_group[kSeqGraph].insert(TAnnotNameTitleMap::value_type(*iter, ""));
    //                    LOG_POST(Trace << "adding to annot_group[kSeqGraph]: " << *iter);
                    }
                }
                isCoverageCheckFinished = true;
            } NCBI_CATCH("CGraphTrackFactory::CreateTracks()");
        }
        if(!isCoverageCheckFinished) {
            // For this mode, there is no need to worry about if the given graph track
            // is from seq-table or seq-graph (those info won't be used at all)
            ITERATE (SExtraParams::TAnnots, iter, params.m_Annots) {
                annot_group[kSeqGraph].insert(TAnnotNameTitleMap::value_type(*iter, ""));
//                LOG_POST(Trace << "adding to annot_group[kSeqGraph]: " << *iter);
            }
        }
    } else {
        // collect non-NA tracks
        CIRef<ISGDataSource> ds = ds_context->GetDS(
            typeid(CSGGraphDSType).name(), object);
        CSGGraphDS* graph_ds = dynamic_cast<CSGGraphDS*>(ds.GetPointer());
        graph_ds->SetDepth(params.m_Level);
        graph_ds->SetAdaptive(params.m_Adaptive);

        SAnnotSelector sel(CSeqUtils::GetAnnotSelector(params.m_Annots));
        sel.ExcludeNamedAnnots("SNP");
//        LOG_POST(Trace << "GetAnnotNames() into annot_group[kSeqGraph]");
        graph_ds->GetAnnotNames(sel, r_cntx->GetVisSeqRange(), annot_group[kSeqGraph]);
//        LOG_POST(Trace << "got annot_group[kSeqGraph].size() annots: " << annot_group[kSeqGraph].size());

        // collect NA tracks
        // For graph track we don't collect NA tracks if it is
        // not explicitly specified due to potential large number
        // of NA tracks.
//        LOG_POST(Trace << "Checking src_annots.size() using GetMatchedAnnots: " << src_annots.size());
        if ( !src_annots.empty()  &&  !params.m_Annots.empty() ) {
            GetMatchedAnnots(src_annots, params, annot_group[kSeqGraph]);
//            LOG_POST(Trace << "got annot_group[kSeqGraph].size() annots: " << annot_group[kSeqGraph].size());
        }

        if (params.m_Annots.empty()) {
            sel = CSeqUtils::GetAnnotSelector(CSeq_annot::C_Data::e_Seq_table);
            sel.ExcludeNamedAnnots("SNP");
            graph_ds->GetSeqtableAnnots(sel, r_cntx->GetVisSeqRange(), annot_group[kSeqTable]);

            sel = CSeqUtils::GetAnnotSelector(CSeq_annot::C_Data::e_Graph);
            sel.ExcludeNamedAnnots("SNP");
            graph_ds->GetAnnotNames(sel, r_cntx->GetVisSeqRange(), annot_group[kSeqGraph], false);

        } else {
//            LOG_POST(Trace << "GetAnnotNames() into annot_group[kSeqTable], true");
            sel = CSeqUtils::GetAnnotSelector(params.m_Annots);
            sel.ExcludeNamedAnnots("SNP");
            graph_ds->GetAnnotNames(sel, r_cntx->GetVisSeqRange(), annot_group[kSeqTable], true);
        }
    }
//    LOG_POST(Trace << "got annot_group[kSeqGraph].size() annots: " << annot_group[kSeqGraph].size());
//    LOG_POST(Trace << "got annot_group[kSeqTable].size() annots: " << annot_group[kSeqTable].size());

    // create feature tracks
//    LOG_POST(Trace << "Going through annot_group");
    TTrackMap tracks;
    
    ITERATE (TAnnotGroup, g_iter, annot_group) {
        const TAnnotNameTitleMap& annots = g_iter->second;
//        LOG_POST(Trace << "Checking " << g_iter->first);
        ITERATE (TAnnotNameTitleMap, iter, annots) {
//            LOG_POST(Trace << "Using annot " << iter->first << ": " << iter->second);
            CIRef<ISGDataSource> ds =
                ds_context->GetDS(typeid(CSGGraphDSType).name(), object);
            CSGGraphDS* graph_ds = dynamic_cast<CSGGraphDS*>(ds.GetPointer());
            graph_ds->SetDepth(params.m_Level);
            graph_ds->SetAdaptive(params.m_Adaptive);
            graph_ds->SetSeqTable(g_iter->first == kSeqTable);
            

            /*
            if (params.m_CoverageGraphCheck) {
                graph_ds->SetGraphLevels(iter->first);
            }
             */

            CRef<CGraphTrack> track(new CGraphTrack(graph_ds, r_cntx));
            track->SetAnnot(iter->first);

            if ( !iter->second.empty() ) {
                track->SetTitle(iter->second);
            }
            if (!graph_cache_key.empty()) {
                graph_ds->SetGraphCacheKey(graph_cache_key);
            }

            tracks[iter->first] = track.GetPointer();
        }
    }
//    LOG_POST(Trace << "Finished going through annot_group, created tracks: " << tracks.size());

    return tracks;
}


void CGraphTrackFactory::GetMatchedAnnots(
    const TAnnotMetaDataList& src_annots,
    const ILayoutTrackFactory::SExtraParams& params,
    TAnnotNameTitleMap& out_annots) const
{
    ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "graph", "", out_annots);
}


string CGraphTrackFactory::GetExtensionIdentifier() const
{
    return CGraphTrack::m_TypeInfo.GetId();
}


string CGraphTrackFactory::GetExtensionLabel() const
{
    return CGraphTrack::m_TypeInfo.GetDescr();
}


CRef<CTrackConfigSet>
CGraphTrackFactory::GetSettings(const string& /*profile*/,
                                const TKeyValuePairs& settings,
                                const CTempTrackProxy* track_proxy) const
{
    CRef<CTrackConfigSet> config_set(new CTrackConfigSet);
    CRef<CTrackConfig> config(new CTrackConfig);
    config_set->Set().push_back(config);
    config->SetLegend_text("anchor_12");

    CHistParams::EType style = CHistParams::eHistogram;
    CHistParams::EScale scale = CHistParams::eLinear;
    CHistParams::EScale stored_scale = track_proxy? CHistParams::ScaleStrToValue(track_proxy->GetStoredScale()) : CHistParams::eLinear;
    bool fixed_scale = false;
    //bool clip_outliers = false;
    int sd_thresh = 5;
    string value_range;
    bool smooth_curve = false;
    string fit_step = "4";
    ITERATE (TKeyValuePairs, iter, settings) {
        try {
            if (NStr::EqualNocase(iter->first, "fixed_scale")) {
                fixed_scale = NStr::StringToBool(iter->second);
            } else if (NStr::EqualNocase(iter->first, "layout")) {
                config->SetHidden_settings().push_back(
                    CTrackConfigUtils::CreateHiddenSetting("layout", iter->second));
            } else if (NStr::EqualNocase(iter->first, "color")) {
                config->SetHidden_settings().push_back(
                    CTrackConfigUtils::CreateHiddenSetting("color", iter->second));
            } else if (NStr::EqualNocase(iter->first, "neg_color")) {
                config->SetHidden_settings().push_back(
                    CTrackConfigUtils::CreateHiddenSetting("neg_color", iter->second));
            } else if (NStr::EqualNocase(iter->first, "opacity")) {
                config->SetHidden_settings().push_back(
                    CTrackConfigUtils::CreateHiddenSetting("opacity", iter->second));
            } else if (NStr::EqualNocase(iter->first, "height")) {
                config->SetHidden_settings().push_back(
                    CTrackConfigUtils::CreateHiddenSetting("height", iter->second));
            } else if (NStr::EqualNocase(iter->first, "style")) {
                style = CHistParams::TypeStrToValue(iter->second);
            } else if (NStr::EqualNocase(iter->first, kScale)) {
                scale = CHistParams::ScaleStrToValue(iter->second);
            } else if (NStr::EqualNocase(iter->first, kValueRange)) {
                value_range = iter->second;
            } else if (NStr::EqualNocase(iter->first, kStoredScale) || NStr::EqualNocase(iter->first, "is_scaled")) {
                stored_scale = CHistParams::ScaleStrToValue(iter->second);
            //} else if (NStr::EqualNocase(iter->first, kClipOutliers)) {
            //    clip_outliers = NStr::StringToBool(iter->second);
            } else if (NStr::EqualNocase(iter->first, kOutlierColor)) {
                config->SetHidden_settings().push_back(
                    CTrackConfigUtils::CreateHiddenSetting(kOutlierColor, iter->second));
            } else if (NStr::EqualNocase(iter->first, kSDeviationThreshold)) {
                sd_thresh = NStr::StringToInt(iter->second);
            } else if (NStr::EqualNocase(iter->first, kNumBins)) {
                config->SetHidden_settings().push_back(
                    CTrackConfigUtils::CreateHiddenSetting("num_bins", iter->second));
            } else if (NStr::EqualNocase(iter->first, "smooth_curve")) {
                smooth_curve = NStr::StringToBool(iter->second);
            } else if (NStr::EqualNocase(iter->first, "fit_step")) {
                fit_step = iter->second;
            }
        } catch (std::exception&) {
            // ignore the errors
        }
    }
    if (stored_scale != CHistParams::eLinear && scale == CHistParams::eLinear) {
        scale = stored_scale;
    }
    // Style parameter
    CRef<CChoice> choice = CTrackConfigUtils::CreateChoice(
        "style", "Display Style",
        CHistParams::TypeValueToStr(style),
        "Display style for graph data");
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            CHistParams::TypeValueToStr(CHistParams::eHistogram),
            "Histogram",
            "Shown as a histogram",
            "Graph data are shown as a histogram"));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            CHistParams::TypeValueToStr(CHistParams::eSmearBar),
            "Heat Map",
            "Shown as a heat map",
            "Graph data are shown as a heat map (smear bar)"));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            CHistParams::TypeValueToStr(CHistParams::eLineGraph),
            "Line Graph",
            "Shown as a line graph",
            "Graph data are shown as a line graph"));
    config->SetChoice_list().push_back(choice);

    // Scale parameter
    choice = CTrackConfigUtils::CreateChoice(
        kScale, "Linear/Log Scale",
        CHistParams::ScaleValueToStr(scale),
        "Scale for graph data");
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            CHistParams::ScaleValueToStr(CHistParams::eLinear),
            "Linear",
            "Shown at linear scale",
            "Graph data is shown at linear scale"));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            CHistParams::ScaleValueToStr(CHistParams::eLog10),
            "Log Base 10",
            "Shown at log base 10 scale",
            "Graph data is shown at logarithmic (base 10) scale"));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            CHistParams::ScaleValueToStr(CHistParams::eLoge),
            "Log Base e",
            "Shown at natural logarithm (base e) scale",
            "Graph data is shown at natural logrithm (base e) scale"));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            CHistParams::ScaleValueToStr(CHistParams::eLog2),
            "Log Base 2",
            "Shown at log base 2 scale",
            "Graph data is shown at logarithmic (base 2) scale"));
    config->SetChoice_list().push_back(choice);

    // Overlay  layout
    config->SetCheck_boxes().push_back(
        CTrackConfigUtils::CreateCheckBox(
            "smooth_curve", "Smooth curve", "Smooth curve", "", smooth_curve));

    bool range_autoscale = true;
    string range_min, range_max;
    if (!value_range.empty()) {
        string r_min, r_max;
        CTrackConfigUtils::DecodeValueRange(value_range, r_min, r_max, range_autoscale);
        if (!r_min.empty() && r_min != "inf") {
            range_min = (errno) ? "" : r_min;
        }
        if (!r_max.empty() && r_max != "inf") {
            range_max = (errno) ? "" : r_max;
        }
        config->SetHidden_settings().push_back(
            CTrackConfigUtils::CreateHiddenSetting(kValueRange, value_range));
            //NStr::Replace(value_range, "~", ":")));
    }

    auto range_control = CTrackConfigUtils::CreateRangeControl(kValueRange,
        "Value Range",
        "Value Range",
        range_min,
        range_max,
        range_autoscale,
        true);
    {
        auto condition = Ref(new objects::CConditional);
        condition->SetName(kScale);
        condition->SetValue().push_back(CHistParams::ScaleValueToStr(CHistParams::eLog10));
        condition->SetValue().push_back(CHistParams::ScaleValueToStr(CHistParams::eLog2));
        condition->SetValue().push_back(CHistParams::ScaleValueToStr(CHistParams::eLoge));
        condition->SetAction(CConditional::eAction_disable);
        range_control->SetConditions().push_back(condition);
    }
    {
        auto condition = Ref(new objects::CConditional);
        condition->SetName("style");
        condition->SetValue().push_back(CHistParams::TypeValueToStr(CHistParams::eSmearBar));
        condition->SetAction(CConditional::eAction_disable);
        range_control->SetConditions().push_back(condition);
    }

    config->SetRange_controls().push_back(range_control);

    config->SetHelp() = GetThisTypeInfo().GetDescr();

    return config_set;
}

void CGraphTrack::GetLegend(const CTempTrackProxy* track_proxy, CTrackConfig::TLegend& legend) const
{
    CRef<CLegendItem> legend_item(new CLegendItem);
    legend_item->SetLabel(track_proxy->GetSource());
    legend_item->SetColor("blue");
    legend_item->SetId("");

    int opacity = 100;
    string color;
    CHistParams::EType style = CHistParams::eHistogram;
    if (!track_proxy->GetTrackProfile().empty()) {
        TKeyValuePairs settings;
        CSGConfigUtils::ParseProfileString(track_proxy->GetTrackProfile(), settings);
        ITERATE(TKeyValuePairs, iter, settings) try {
            if (NStr::EqualNocase(iter->first, "color")) {
                color = iter->second;
            } else if (NStr::EqualNocase(iter->first, "opacity")) {
                opacity = NStr::StringToInt(iter->second, NStr::fConvErr_NoThrow);
                opacity = (opacity <= 0) ? 100 : min(opacity, 100);
            } else if (NStr::EqualNocase(iter->first, "style")) {
                style = CHistParams::TypeStrToValue(iter->second);
            }
        } catch (std::exception&) {
            LOG_POST(Error << "CGraphTrack::GetLegend: invalid settings " << iter->second);
        }
    }
    bool color_is_set = false;
    CRgbaColor c;
    if (!color.empty()) {
        try {
            c.FromString(color);
            color_is_set = true;
        } catch (exception& e) {
            ERR_POST(Error << e.what());
        }
    } 
    
    if (color_is_set == false) {
        _ASSERT(m_gConfig);
        if (m_gConfig) {
            CRef<CHistParamsManager> conf_mgr = m_gConfig->GetHistParamsManager();
            CRef<CHistParams> hist_conf;
            if (conf_mgr->HasSettings(m_AnnotName)) {
                // Get the corresponding setting for this data track if exists
                hist_conf = conf_mgr->GetHistParams(m_AnnotName);
            } else {
                CRef<CHistParams> def_conf = conf_mgr->GetDefHistParams();
                hist_conf.Reset(new CHistParams(*def_conf));
                conf_mgr->AddSettings(m_AnnotName, hist_conf);
            }
            c = hist_conf->m_fgColor;
        }
    }
    if (opacity < 100 && opacity > 0) {
        float alpha = opacity;
        alpha /= 100.;
        c.SetAlpha(alpha);
    }
    legend_item->SetColor(c.ToString());
    string label = CHistParams::TypeValueToStr(style);
    label += ":";
    legend_item->SetLabel(label);
    legend.push_back(legend_item);
}


void CGraphTrack::x_OnLayoutChanged()
{
    CDataTrack::x_OnLayoutChanged();
    if (m_Layout == eLayout_Overlay || m_Layout == eLayout_Overlayed) {
        if (GetGroup().GetChildren().size() <= 1)
            return;
        float axis_min = numeric_limits<float>::max();
        float axis_max = numeric_limits<float>::min();

        NON_CONST_ITERATE(CLayoutGroup::TObjectList, hist_it, SetGroup().SetChildren())
        {
            CHistogramGlyph* hg = dynamic_cast<CHistogramGlyph*>(&**hist_it);
            if (!hg)
                continue;
            axis_max = max(hg->GetMax(), axis_max);
            axis_min = min(hg->GetMin(), axis_min);
        }

        NON_CONST_ITERATE(CLayoutGroup::TObjectList, hist_it, SetGroup().SetChildren())
        {
            CHistogramGlyph* hg = dynamic_cast<CHistogramGlyph*>(&**hist_it);
            if (!hg)
                continue;
            hg->SetAnnotName(m_Layout == eLayout_Overlayed ? m_AnnotName + kOverlayed : m_AnnotName);
            hg->SetRenderingMode(CHistogramGlyph::eMode_Overlay);
            hg->SetAxisMax(axis_max);
            hg->SetAxisMin(axis_min);
            hg->SetFixedScale(true);
            hg->SetAxisRange();
        }
        if (m_Layout == eLayout_Overlay) {
            x_InitGrid();
            m_Grid->SetAnnotName(m_AnnotName);
            m_Grid->SetAxisMax(axis_max);
            m_Grid->SetAxisMin(axis_min);
            m_Grid->SetFixedScale(true);
            m_Grid->SetAxisRange();
        }
    }
}


void CGraphTrack::x_UpdateBoundingBox()
{
    CGlyphContainer::x_UpdateBoundingBox();
    if (m_Layout == eLayout_Overlay) {
        if (!(m_On  &&  m_Expanded))
            return;
        TModelRange vis_r = m_Context->IntersectVisible(this);
        if (vis_r.Empty())
            return;
        if (m_Group.GetChildren().empty())
            return;
        _ASSERT(m_gConfig);
        if (!m_gConfig)
            return;
        // update Grid
        x_InitGrid();
        m_Grid->SetConfig(*m_gConfig);
        NON_CONST_ITERATE(CLayoutGroup::TObjectList, hist_it, SetGroup().SetChildren())
        {
            CHistogramGlyph* hg = dynamic_cast<CHistogramGlyph*>(&**hist_it);
            if (!hg)
                continue;
            m_Grid->SetTop(m_Group.GetTop() + hg->GetTop());
            m_Grid->SetLeft(hg->GetLeft());
            m_Grid->SetWidth(hg->GetWidth());
            m_Grid->SetHeight(hg->GetHeight());
            break;
        }
    }
}

void CGraphTrack::x_RenderContent() const
{
    if (GetHeight() == 0 || m_Context->IntersectVisible(this).Empty())
        return;
    if (m_Layout == eLayout_Overlay && m_Grid) {
        m_Grid->DrawGrid(true);
    }
    CGlyphContainer::x_RenderContent();
}


void CGraphTrack::SetLayout(ELayout layout)
{
    if (layout == m_Layout)
        return;

    if (layout == eLayout_Layered) {

        CLayoutGroup::TObjectList glyphs;
        bool create_group = GetGroup().GetChildren().size() > 1;

        NON_CONST_ITERATE(CLayoutGroup::TObjectList, hist_it, SetGroup().SetChildren()) {
            CHistogramGlyph* hg = dynamic_cast<CHistogramGlyph*>(&**hist_it);
            if (!hg)
                continue;
            hg->SetAnnotName(m_AnnotName);
            hg->SetRenderingMode(CHistogramGlyph::eMode_Single);
            hg->Update(true);
            if (create_group) {
                CRef<CCommentConfig> config(new CCommentConfig());
                config->m_LabelColor = CRgbaColor("blue");
                config->m_ShowConnection = false;
                CRef<CCommentGlyph> label(new CCommentGlyph(hg->GetTitle(), CRef<CSeqGlyph>(hg)));
                label->SetConfig(config);
                label->SetRenderingContext(m_Context);

                CRef<CLayoutGroup> group(new CLayoutGroup);
                group->SetLayoutPolicy(m_Simple);
                group->SetRenderingContext(m_Context);

                group->PushBack(label);
                group->PushBack(hg);
                glyphs.push_back(CRef<CSeqGlyph>(group.GetPointer()));
            } else {
                glyphs.push_back(*hist_it);
            }
        }
        if (!glyphs.empty())
            SetGroup().SetChildren() = glyphs;
        SetLayoutPolicy(m_Layered);

    } else if (layout == eLayout_Overlay || layout == eLayout_Overlayed) {

        CLayoutGroup::TObjectList graphs;
        NON_CONST_ITERATE(CLayoutGroup::TObjectList, hist_it, SetGroup().SetChildren()) {
            CHistogramGlyph* hg = 0;
            CLayoutGroup* lg = dynamic_cast<CLayoutGroup*>(&**hist_it);
            if (lg && lg->GetChildrenNum() == 2) {
                CRef<CSeqGlyph> g = lg->GetChild(1);
                hg = dynamic_cast<CHistogramGlyph*>(g.GetPointer());
                if (hg) {
                    hg->SetParent(this);
                    graphs.push_back(g);
                }
            } else {
                hg = dynamic_cast<CHistogramGlyph*>(&**hist_it);
            }
            if (hg) {
                hg->SetAnnotName(layout == eLayout_Overlayed ? m_AnnotName + kOverlayed : m_AnnotName);
                hg->Update(true);
            }
        }
        if (!graphs.empty()) {
            SetGroup().SetChildren() = graphs;
        }
        CRef<COverlayLayout> ol = Ref(new COverlayLayout);
        SetLayoutPolicy(ol.GetPointer());
    }

    m_Layout = layout;
    Update(true);
}

void CGraphTrack::x_InitGrid()
{
    if (!m_Grid) {
        CHistogramGlyph::TMap empty_map;
        m_Grid.Reset(new CHistogramGlyph(empty_map, NcbiEmptyString));
        m_Grid->SetRenderingContext(m_Context);
        m_Grid->SetShowTitle(false);
    }

}

bool CGraphTrack::CanDrop(CRef<CLayoutTrack>& track)
{
    CGraphTrack* gt = dynamic_cast<CGraphTrack*>(&*track);
    return (gt != 0 && gt->GetChildren().size() == 1);
}

bool CGraphTrack::Drop(CRef<CLayoutTrack>& track)
{
    vector<CRef<CGraphTrack>> tracks;
    CGraphTrack* gt = dynamic_cast<CGraphTrack*>(&*track);
    if (gt && gt->GetChildren().size() == 1) {
        tracks.push_back(Ref(gt));
        return CreateOverlay(tracks);
    }
    return false;
}


void CGraphTrack::x_CollectAllGraphs(vector<CRef<CHistogramGlyph>>& graphs) const
{
    for (auto child : GetGroup().GetChildren()) {
        CHistogramGlyph* hg = 0;
        CLayoutGroup* lg = dynamic_cast<CLayoutGroup*>(&*child);
        if (lg && lg->GetChildrenNum() == 2) {
            CRef<CSeqGlyph> g = lg->GetChild(1);
            hg = dynamic_cast<CHistogramGlyph*>(g.GetPointer());
        } else {
            hg = dynamic_cast<CHistogramGlyph*>(&*child);
        }
        if (hg) {
            graphs.push_back(Ref(hg));
        }
    }
}


END_NCBI_SCOPE
