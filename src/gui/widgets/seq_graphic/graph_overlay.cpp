/*
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
 * Authors:  Andrei Shkeda
 *
 */

#include <ncbi_pch.hpp>
#include "wx_histogram_config_dlg.hpp"
#include <gui/widgets/seq_graphic/graph_overlay.hpp>
#include <gui/widgets/seq_graphic/layout_track_proxy.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_widget.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/seq_graphic/graph_track.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <gui/objects/TrackConfigSet.hpp>
#include <gui/objects/Conditional.hpp>

#include <gui/objutils/reg_settings.hpp>
#include <gui/objutils/utils.hpp>
#include <wx/event.h>
#include <wx/menu.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const string kStacked              = "stacked";
static const string kValueRange           = "range";

CTrackTypeInfo
CGraphOverlay::m_TypeInfo("graph_overlay", "Composite track");


///////////////////////////////////////////////////////////////////////////////
/// CGraphOverlayFactory
///////////////////////////////////////////////////////////////////////////////
CGraphOverlayFactory::CGraphOverlayFactory()
{
}


ILayoutTrackFactory::TTrackMap
CGraphOverlayFactory::CreateTracks(SConstScopedObject& object,
                                     ISGDataSourceContext* ds_context,
                                     CRenderingContext* r_cntx,
                                     const SExtraParams& params,
                                     const TAnnotMetaDataList& /*src_annots*/) const
{
    TTrackMap tracks;
    CIRef<ISGDataSource> ds =
        ds_context->GetDS(typeid(CFeaturePanelDSType).name(), object);
    CFeaturePanelDS* fp_ds = dynamic_cast<CFeaturePanelDS*>(ds.GetPointer());
    fp_ds->SetDSContext(ds_context);
    fp_ds->SetRenderingContext(r_cntx);
/*
    if (!params.m_Annots.empty()) {
        CRef<CGraphTrackFactory> factory(new CGraphTrackFactory);
        ITERATE(SExtraParams::TAnnots, iter, params.m_Annots)
        {
            SExtraParams new_params = params;
            new_params.m_Annots.clear();
            new_params.m_Annots.push_back(*iter);
            TTrackMap a_tracks = factory->CreateTracks(object, ds_context, r_cntx, new_params);
            tracks.insert(a_tracks.begin(), a_tracks.end());
        }
    }
*/    
    const static string kGraphOverlay = "graph_overlay";
    static int s_GraphOverlayNum = 0;
    CGraphOverlay* track = new CGraphOverlay(r_cntx, fp_ds);
    //tracks["graph_overlay"] = CRef<CLayoutTrack>(track);
    string annot = params.m_Annots.empty() ? kGraphOverlay + NStr::NumericToString(s_GraphOverlayNum++) : params.m_Annots.front();
    tracks[annot] = CRef<CLayoutTrack>(track);
    track->SetAnnotLevel(params.m_Level);
    track->SetAdaptive(params.m_Adaptive);
    track->SetSkipGenuineCheck(params.m_SkipGenuineCheck);
    return tracks;
}

string CGraphOverlayFactory::GetExtensionIdentifier() const
{
    return CGraphOverlay::m_TypeInfo.GetId();
}

string CGraphOverlayFactory::GetExtensionLabel() const
{
    return CGraphOverlay::m_TypeInfo.GetDescr();
}

CRef<CTrackConfigSet>
CGraphOverlayFactory::GetSettings(const string& /*profile*/,
                                  const TKeyValuePairs& settings,
                                  const CTempTrackProxy* track_proxy) const
{
    CRef<CTrackConfigSet> config_set(new CTrackConfigSet);
    CRef<CTrackConfig> config(new CTrackConfig);
    config_set->Set().push_back(config);
    config->SetLegend_text("anchor_12");

    CHistParams::EScale scale = CHistParams::eLinear;
    CHistParams::EScale stored_scale = track_proxy
        ? CHistParams::ScaleStrToValue(track_proxy->GetStoredScale())
        : CHistParams::eLinear;
    bool stacked = false;
    string value_range;
    bool smooth_curve = false;
    string fit_step = "5";


    ITERATE (TKeyValuePairs, iter, settings) try {
        if (NStr::EqualNocase(iter->first, "height")) {
            config->SetHidden_settings().push_back
                (CTrackConfigUtils::CreateHiddenSetting("height", iter->second));
        } else if (NStr::EqualNocase(iter->first, "Label")) {
            config->SetHidden_settings().push_back
                (CTrackConfigUtils::CreateHiddenSetting("Label", iter->second));
        } else if (NStr::EqualNocase(iter->first, "RulerColor")) {
            config->SetHidden_settings().push_back
                (CTrackConfigUtils::CreateHiddenSetting("RulerColor", iter->second));
        } else if (NStr::EqualNocase(iter->first, "scale")) {
            scale = CHistParams::ScaleStrToValue(iter->second);
        } else if (NStr::EqualNocase(iter->first, "stored_scale") || NStr::EqualNocase(iter->first, "is_scaled")) {
            stored_scale = CHistParams::ScaleStrToValue(iter->second);
        } else if (NStr::EqualNocase(iter->first, kValueRange)) {
            value_range =iter->second;
        } else if (NStr::EqualNocase(iter->first, kStacked)) {
            stacked = NStr::StringToBool(iter->second);
        } else if (NStr::EqualNocase(iter->first, "smooth_curve")) {
            smooth_curve = NStr::StringToBool(iter->second);
        } else if (NStr::EqualNocase(iter->first, "fit_step")) {
            fit_step = iter->second;
        }

    } catch (std::exception&) {
        // ignore the errors
    }
    if (stored_scale != CHistParams::eLinear && scale == CHistParams::eLinear) {
        scale = stored_scale;
    }

    // Scale parameter
    CRef<CChoice> choice = CTrackConfigUtils::CreateChoice
        ("scale", "Linear/Log Scale",
         CHistParams::ScaleValueToStr(scale),
         "Scale for graph data");
    choice->SetValues().push_back
        (CTrackConfigUtils::CreateChoiceItem
         (CHistParams::ScaleValueToStr(CHistParams::eLinear),
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
        condition->SetName("scale");
        condition->SetValue().push_back(CHistParams::ScaleValueToStr(CHistParams::eLog10));
        condition->SetValue().push_back(CHistParams::ScaleValueToStr(CHistParams::eLog2));
        condition->SetValue().push_back(CHistParams::ScaleValueToStr(CHistParams::eLoge));
        condition->SetAction(CConditional::eAction_disable);
        range_control->SetConditions().push_back(condition);
    }        

    config->SetRange_controls().push_back(range_control);

    // Overlay  layout
    config->SetCheck_boxes().push_back(
        CTrackConfigUtils::CreateCheckBox(
            kStacked, "Stacked", "Graphs are stacked under each other", "", stacked));

    config->SetHelp() = GetThisTypeInfo().GetDescr();

    return config_set;
}


class CGraphOverlayEvtHandler :
    public CObject,
    public wxEvtHandler
{
public:
    CGraphOverlayEvtHandler(CGraphOverlay* overlay, int track_id)
        : m_Overlay(overlay), m_TrackId(track_id)
    {
    }

    void OnContextMenu(wxContextMenuEvent& anEvent);

private:
    /// @name event handlers.
    /// @{
    void x_OnTrackSettings(wxCommandEvent& event)
    {
        m_Overlay->OnTrackSettings(m_TrackId);
    }
    void x_OnHideTrack(wxCommandEvent& event)
    {
        m_Overlay->OnToggleTrack(m_TrackId);
    }
    void x_OnRemoveTrack(wxCommandEvent& event)
    {
        m_Overlay->MoveTrackOut(m_TrackId);
    }
    /// @}

private:
    CRef<CGraphOverlay> m_Overlay;
    int m_TrackId;

    DECLARE_EVENT_TABLE()
};


BEGIN_EVENT_TABLE(CGraphOverlayEvtHandler, wxEvtHandler)
    EVT_MENU(eCmdTrackSettings, CGraphOverlayEvtHandler::x_OnTrackSettings)
    EVT_MENU(eCmdHideTrack, CGraphOverlayEvtHandler::x_OnHideTrack)
    EVT_MENU(eCmdRemoveTrackFromOverlay, CGraphOverlayEvtHandler::x_OnRemoveTrack)
END_EVENT_TABLE()


///////////////////////////////////////////////////////////////////////////////
/// CGraphOverlay
///////////////////////////////////////////////////////////////////////////////

CGraphOverlay::CGraphOverlay(CRenderingContext* r_cntx, CFeaturePanelDS* ds, const string& source) :
    CTrackContainer(r_cntx, ds)
    , m_Source(source)
{
    //_ASSERT(!m_Source.empty());
    if (m_Source.empty())
        m_Source = "Composite track";
    //x_RegisterIcon(SIconInfo(eIcon_Settings, "Settings", true, "track_settings"));

    //CRef<COverlayLayout> p(new COverlayLayout);
    //CRef<CSimpleLayout> p(new CSimpleLayout);
    //SetLayoutPolicy(p);

    CRef<CLayeredLayout> layered_layout(new CLayeredLayout);
    m_Legend.SetLayoutPolicy(&*layered_layout);
    m_Legend.SetRenderingContext(r_cntx);
    m_Legend.SetParent(this);

    CHistogramGlyph::TMap empty_map;
    m_Grid.Reset(new CHistogramGlyph(empty_map, NcbiEmptyString));
    m_Grid->SetRenderingContext(r_cntx);
    m_Grid->SetShowTitle(false);
}


void CGraphOverlay::UpdateTrackParams(const string& annot_name)
{
    if (annot_name == m_Source)
        return;
    string config_name = annot_name;
    config_name +=  CGraphTrack::kOverlayed;
    CRef<CHistParamsManager> conf_mgr = m_gConfig->GetHistParamsManager();
    if (!conf_mgr->HasSettings(m_Source))
        return;
    CRef<CHistParams> curr_params;
    if (conf_mgr->HasSettings(config_name)) {
        // Get the corresponding setting for this data track if exists
        curr_params = conf_mgr->GetHistParams(config_name);
    } else {
        if (conf_mgr->HasSettings(annot_name))
            curr_params = conf_mgr->GetHistParams(annot_name);
        else
            curr_params = conf_mgr->GetDefHistParams();
    }

    CRef<CHistParams> overlay_params = conf_mgr->GetHistParams(m_Source);
    CRef<CHistParams> merged_params(new CHistParams);
    *merged_params = *overlay_params;
    merged_params->m_fgColor = curr_params->m_fgColor;
    // overlayed graphs should be rendered in one color
    // therefore we need override neg color for them 
    //merged_params->m_fgNegColor = (m_Layout == eOverlay) ? curr_params->m_fgColor : curr_params->m_fgNegColor;
    merged_params->m_fgNegColor = curr_params->m_fgColor;
    merged_params->m_SmearColorMin = curr_params->m_SmearColorMin;
    merged_params->m_SmearColorMax = curr_params->m_SmearColorMax;
    merged_params->m_Type = curr_params->m_Type;
    //merged_params->m_SmoothCurve = curr_params->m_SmoothCurve;
    //merged_params->m_ValueRange = curr_params->m_ValueRange;
    //merged_params->m_RangeMin = curr_params->m_RangeMin;
    //merged_params->m_RangeMax = curr_params->m_RangeMax;
    conf_mgr->AddSettings(config_name, merged_params);
}


void CGraphOverlay::x_UpdateContainerParams(CHistParams& params)
{
    CRef<CHistParamsManager> conf_mgr = m_gConfig->GetHistParamsManager();
    _ASSERT(conf_mgr->HasSettings(m_Source));
    if (!conf_mgr->HasSettings(m_Source))
        return;
    CRef<CHistParams> my_params = conf_mgr->GetHistParams(m_Source);
    *my_params = params;
}


void CGraphOverlay::x_UpdateTrackSettings(const string& source_name)
{
    _ASSERT(m_gConfig);
    CRef<CHistParamsManager> conf_mgr = m_gConfig->GetHistParamsManager();
    CRef<CHistParams> overlay_params = conf_mgr->GetHistParams(m_Source);

    string src_name = source_name + CGraphTrack::kOverlayed;
    CRef<CHistParams> curr_params(new CHistParams);
    *curr_params = *conf_mgr->GetHistParams(src_name);
    curr_params->m_NeedRuler = overlay_params->m_NeedRuler;
    curr_params->m_DrawBg = overlay_params->m_DrawBg;

    CHistConfigDlg dlg;
    dlg.SetConfig(curr_params);
    dlg.SetConfigName(source_name);
    dlg.SetRegistryPath("Dialogs.GraphRenderingOptions");
    dlg.CreateX(NULL);
    if(dlg.ShowModal() == wxID_OK) {
        CRef<CHistParams> params = dlg.GetConfig();
        x_UpdateContainerParams(*params);
        params->m_NeedRuler = m_Layout != eOverlay;
        params->m_DrawBg = m_Layout != eOverlay;
        conf_mgr->AddSettings(src_name, params);
        NON_CONST_ITERATE(CLayoutGroup::TObjectList, gt_it, SetGroup().SetChildren()) {
            CGraphTrack* gt = dynamic_cast<CGraphTrack*>(&**gt_it);
            if (!gt)
                continue;
            NON_CONST_ITERATE(CLayoutGroup::TObjectList, hist_it, gt->SetGroup().SetChildren()) {
                CHistogramGlyph* hg = dynamic_cast<CHistogramGlyph*>(&**hist_it);
                if (!hg)
                    continue;
                UpdateTrackParams(gt->GetAnnot());
                hg->SetAxisRange();
                hg->Update(true);
            }
        }
        Update(true);
        x_OnLayoutChanged();
    }
}

void CGraphOverlay::OnTrackSettings(int order)
{
    for (auto& proxy : m_TrackProxies) {
        if (proxy->GetOrder() == order) {
            x_UpdateTrackSettings(proxy->GetSource());
            break;
        }
    }
}


void CGraphOverlay::x_SaveSettings(const string& preset_style)
{
    CTrackContainer::x_SaveSettings(preset_style);

    TKeyValuePairs settings;
    if ( !preset_style.empty())
        settings["profile"] = preset_style;
    CRef<CHistParamsManager> conf_mgr = m_gConfig->GetHistParamsManager();
    CRef<CHistParams> overlay_params = conf_mgr->GetHistParams(m_Source);

    settings["BG"] = overlay_params->m_bgColor.ToString();
    settings["height"] = NStr::NumericToString(overlay_params->m_Height);
    settings["scale"] = CHistParams::ScaleValueToStr(overlay_params->m_Scale);
    settings["stored_scale"] = CHistParams::ScaleValueToStr(overlay_params->m_StoredScale);
    settings["DrawBg"] = NStr::BoolToString(overlay_params->m_DrawBg);
    settings["NeedRuler"] = NStr::BoolToString(overlay_params->m_NeedRuler);
    settings["Label"] = overlay_params->m_LabelColor.ToString();
    settings["RulerColor"] = overlay_params->m_RulerColor.ToString();
    //settings["smooth_curve"] = NStr::BoolToString(overlay_params->m_SmoothCurve);
    SetProfile(CSGConfigUtils::ComposeProfileString(settings));
}

void CGraphOverlay::x_LoadSettings(const string& /*preset_style*/,
                                  const TKeyValuePairs& settings)
{
    // Parse known parameters
    bool parameter_present = false;
    string scale, stored_scale;
    bool draw_bg = false, need_ruler = true;
    string bg_color, label_color, ruler_color, value_range;
    int height = -1;

    bool smooth_curve = false;
    int fit_step = 4;

    ITERATE (TKeyValuePairs, iter, settings) try {
       if (NStr::EqualNocase(iter->first, "BG")) {
           bg_color = iter->second;
           parameter_present = true;
       } else if (NStr::EqualNocase(iter->first, "height")) {
           height = NStr::StringToInt(iter->second);
           parameter_present = true;
       } else if (NStr::EqualNocase(iter->first, "scale")) {
           scale = iter->second;
           parameter_present = true;
       } else if (NStr::EqualNocase(iter->first, "stored_scale") || NStr::EqualNocase(iter->first, "is_scaled")) {
           stored_scale = iter->second;
           parameter_present = true;
       } else if (NStr::EqualNocase(iter->first, "DrawBg")) {
           draw_bg = NStr::StringToBool(iter->second);
           parameter_present = true;
       } else if (NStr::EqualNocase(iter->first, "NeedRuler")) {
           need_ruler = NStr::StringToBool(iter->second);
           parameter_present = true;
       } else if (NStr::EqualNocase(iter->first, "Label")) {
           label_color = iter->second;
           parameter_present = true;
       } else if (NStr::EqualNocase(iter->first, "RulerColor")) {
           ruler_color = iter->second;
           parameter_present = true;
        } else if (NStr::EqualNocase(iter->first, kValueRange)) {
            value_range = iter->second;
            parameter_present = true;
       } else if (NStr::EqualNocase(iter->first, kStacked)) {
           m_Layout = NStr::StringToBool(iter->second) ? eStacked : eOverlay ;
        } else if (NStr::EqualNocase(iter->first, "smooth_curve")) {
            try {
                smooth_curve = NStr::StringToBool(iter->second);
            } catch (exception& e) {
                ERR_POST(Error << e.what());
            }
            parameter_present = true;
        } else if (NStr::EqualNocase(iter->first, "fit_step")) {
            fit_step = NStr::StringToInt(iter->second);
            parameter_present = true;
        }
    } catch (std::exception&) {
        LOG_POST(Error << "CGraphTrackContainer::x_LoadSettings: invalid settings "
                << iter->second);
    }
    CRef<CHistParamsManager> conf_mgr = m_gConfig->GetHistParamsManager();
    CRef<CHistParams> hist_conf;
    if (conf_mgr->HasSettings(m_Source)) {
         // Get the corresponding setting for this data track if exists
         hist_conf = conf_mgr->GetHistParams(m_Source);
    } else {
        CRef<CHistParams> def_conf = conf_mgr->GetDefHistParams();
        hist_conf.Reset(new CHistParams(*def_conf));
        conf_mgr->AddTempSettings(m_Source, hist_conf);
    }
    if (!parameter_present)
        return;

     try {
        hist_conf->m_DrawBg = draw_bg;
        hist_conf->m_NeedRuler = need_ruler;

        if (!bg_color.empty())
            hist_conf->m_bgColor.FromString(bg_color);

        if (!label_color.empty())
            hist_conf->m_LabelColor.FromString(label_color);

        if (!ruler_color.empty())
            hist_conf->m_RulerColor.FromString(ruler_color);

        // use user-provided size
        if (height > 0)
            hist_conf->m_Height = height;

        if (!scale.empty())
            hist_conf->m_Scale = CHistParams::ScaleStrToValue(scale);

        hist_conf->m_SmoothCurve = smooth_curve;
        hist_conf->m_FitStep = fit_step;

        if (!stored_scale.empty()) {
            hist_conf->m_StoredScale = CHistParams::ScaleStrToValue(stored_scale);
            if (hist_conf->m_StoredScale != CHistParams::eLinear)
                hist_conf->m_Scale = hist_conf->m_StoredScale;
        }
        if (!value_range.empty()) {
            string range_min, range_max;
            CTrackConfigUtils::DecodeValueRange(value_range, range_min, range_max, hist_conf->m_RangeAutoscale);
            if (!range_min.empty() && range_min != "inf")
                hist_conf->m_ValueRange.SetFrom(NStr::StringToNumeric<float>(range_min, NStr::fConvErr_NoThrow));
            if (!range_max.empty() && range_max != "inf")
                hist_conf->m_ValueRange.SetTo(NStr::StringToNumeric<float>(range_max, NStr::fConvErr_NoThrow));
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
         LOG_POST(Error << "CGraphTrackContainer::x_LoadSettings: invalid histogram settings");
     }
}


void CGraphOverlay::LH_OnItemDblClick(const string& source_name)
{
    x_UpdateTrackSettings(source_name);
}

static
WX_DEFINE_MENU(sLegenItemPopupMenu)
WX_MENU_ITEM(eCmdTrackSettings)
WX_MENU_ITEM(eCmdHideTrack)
WX_MENU_ITEM(eCmdRemoveTrackFromOverlay)
WX_END_MENU()


void CGraphOverlay::LH_OnItemRightClick(const string& source_name)
{
    int track_id = -1;
    for (auto& proxy : m_TrackProxies) {
        if (proxy->GetSource() == source_name) {
            track_id = proxy->GetOrder();
            break;
        }
    }
    if (track_id == -1)
        return;
    CRef<CGraphOverlayEvtHandler> handler(new CGraphOverlayEvtHandler(const_cast<CGraphOverlay*>(this), track_id));
    m_LTHost->LTH_PushEventHandler(&*handler);
    m_LTHost->LTH_PopupMenu(CUICommandRegistry::GetInstance().CreateMenu(sLegenItemPopupMenu));
    m_LTHost->LTH_PopEventHandler();
}


void CGraphOverlay::x_RenderContent() const
{
    if (GetHeight() == 0 || m_Context->IntersectVisible(this).Empty())
        return;
    if (m_Layout == eOverlay && !m_Group.GetChildren().empty())
        m_Grid->DrawGrid(true);
    m_Group.Draw();
    if (!m_Group.GetChildren().empty() && (m_gConfig && !m_gConfig->GetCgiMode()))
        m_Legend.Draw();
}


CRef<CSeqGlyph> CGraphOverlay::HitTest(const TModelPoint& p)
{
    CRef<CSeqGlyph> glyph;
    if (IsIn(p)) {
        TModelPoint pp(p);
        x_Parent2Local(pp);
        if (x_HitTitleBar(pp)) {
            glyph.Reset(this);
        } else {
            glyph = m_Group.HitTest(pp);
            if (!glyph && (m_gConfig && !m_gConfig->GetCgiMode()))
                glyph = m_Legend.HitTest(pp);
        }
    }
    return glyph;
}

static const int kLegendBarWidth = 30;
static const int kLegendLabelGap = 5;
static const int kLegenPadding = 5;


void CGraphOverlay::x_UpdateBoundingBox()
{
    switch (m_Layout) {
    case eOverlay:
        if (!m_OverlayLayout)
            m_OverlayLayout.Reset(new COverlayLayout);
        SetLayoutPolicy(m_OverlayLayout.GetPointer());
        break;
    case eStacked:
        if (!m_StackedLayout)
            m_StackedLayout.Reset(new CSimpleLayout);
        SetLayoutPolicy(m_StackedLayout.GetPointer());
        break;
    }
    CTrackContainer::x_UpdateBoundingBox();
    m_Legend.Clear();
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

    // Update common track settings

    //    ITERATE(TTrackProxies, iter, m_TrackProxies) {
    //        if (!(*iter)->GetShown())
    //            continue;
    //        if (!(*iter)->TrackNotSet())
    //            UpdateTrackParams((*iter)->GetSource());
    //    }
    // update Grid
    m_Grid->SetConfig(*m_gConfig);

    NON_CONST_ITERATE(TTrackProxies, iter, m_TrackProxies) {
        if (!(*iter)->GetShown())
            continue;
        CGraphTrack* gt = dynamic_cast<CGraphTrack*>((*iter)->GetTrack());
        if (!gt)
            continue;
        NON_CONST_ITERATE(CLayoutGroup::TObjectList, hist_it, gt->SetGroup().SetChildren()) {
            CHistogramGlyph* hg = dynamic_cast<CHistogramGlyph*>(&**hist_it);
            if (!hg)
                continue;
            m_Grid->SetTop(m_Group.GetTop() + gt->GetTop() + hg->GetTop());
            m_Grid->SetLeft(hg->GetLeft());
            m_Grid->SetWidth(hg->GetWidth());
            m_Grid->SetHeight(hg->GetHeight());
            break;
        }
        break;
    }
    x_UpdateLegend();
}

struct SLegendData {
    string annot;
    string label;
    string track_id; // for sviewer
    CHistParams* params;
};

void CGraphOverlay::x_UpdateLegend()
{
    _ASSERT(m_gConfig);
    // update legend
    //    if (m_gConfig->GetCgiMode())
    //        return;

    CRef<CHistParamsManager> conf_mgr = m_gConfig->GetHistParamsManager();
    CRef<CGlTextureFont> font = m_gConfig->GetLabelFont();
    IRender& gl = GetGl();

    TModelRange vis_r = m_Context->IntersectVisible(this);

    TModelUnit legend_item_span = 0;
    vector<SLegendData> legend_fields;
    ITERATE(TTrackProxies, iter, m_TrackProxies) {
        if (!(*iter)->GetShown())
            continue;
        SLegendData data;
        data.annot = (*iter)->GetSource();
        CRef<CHistParams> params = conf_mgr->GetHistParams((*iter)->GetSource() + CGraphTrack::kOverlayed);
        data.params = params.GetPointer();
        data.track_id = (*iter)->GetId();
        data.label = CHistParams::TypeValueToStr(params->m_Type);
        if (!data.label.empty())
            data.label += ": ";
        data.label += (*iter)->GetDisplayName().empty() ? data.annot : (*iter)->GetDisplayName();
        legend_item_span = max((TModelUnit)gl.TextWidth(&*font, data.label.c_str()), legend_item_span);
        legend_fields.push_back(data);
    }
    if (legend_fields.empty())
        return;
    legend_item_span += kLegendBarWidth + kLegendLabelGap + kLegenPadding;

//    if (m_gConfig->GetCgiMode())
//        legend_item_span *= 1.1; // increase length by 10% percent to fit browser generated text

    int view_width = m_Context->GetViewWidth();
    if (view_width == 0)
        view_width = m_Context->SeqToScreen(vis_r.GetLength());
    //view_width -= 10;

    int num_cols = min((int)(view_width/legend_item_span), (int)legend_fields.size());
    if (num_cols == 0)
        return;
    bool cgi_mode = (m_gConfig && m_gConfig->GetCgiMode());
    // in cgi mode, the position represents HTML area and is expected to be in screen coordinates
    // in non cgi mode, the position is used for rendering and is expected to be in model coordinates 
            
    TModelUnit center = cgi_mode ? (view_width/2) : m_Context->SeqToScreen(vis_r.GetFrom() +  vis_r.GetLength() / 2);    

    int num_rows = ceil((float)legend_fields.size() / num_cols);
    TModelUnit row_height = gl.TextHeight (&*font) + 2;
    for (int row = 0; row < num_rows; ++row) {
        size_t base_index = row * num_cols;
        // last row may have less columns
        if (base_index + num_cols >= legend_fields.size())
            num_cols = legend_fields.size() - base_index;
        int c = num_cols / 2;
        TModelUnit left = center;
        if (num_cols % 2 != 0)
            left -= legend_item_span/2;
        for (int col = 0; col < c; ++col) {
            //left -= 4;
            left -= legend_item_span;
        }

        for (int col = 0; col < num_cols; ++col) {
            CRef<CLegendItemGlyph> g
                (new CLegendItemGlyph(legend_fields[base_index + col].annot,
                                      legend_fields[base_index + col].label,
                                      legend_fields[base_index + col].track_id,
                                      legend_fields[base_index + col].params->m_fgColor,
                                      legend_fields[base_index + col].params->m_LabelColor));
            g->SetTop(row * row_height + row * 4 + 2);
            g->SetHeight(row_height);

            if (cgi_mode) {
                g->SetWidth(legend_item_span);
                g->SetLeft(left);
            } else {
                g->SetWidth(m_Context->ScreenToSeq(legend_item_span));
                g->SetLeft(m_Context->ScreenToSeq(left));
            }
            g->SetConfig (*m_gConfig);
            g->SetHost(this);
            m_Legend.PushBack(&*g);
            //left += 4;
            left += legend_item_span;
        }
    }
    m_Legend.SetTop(GetHeight());
    m_Legend.SetLeft(GetLeft());
    m_Legend.SetWidth(GetWidth());
    m_Legend.SetHeight(row_height * num_rows + num_rows * 4);
    SetHeight(GetHeight() + m_Legend.GetHeight());
}



void CGraphOverlay::x_OnLayoutChanged()
{
    if (GetGroup().GetChildren().empty())
        return;
    float axis_min = numeric_limits<float>::max();
    float axis_max = numeric_limits<float>::min();
    CRef<CHistParamsManager> conf_mgr = m_gConfig->GetHistParamsManager();
    CRef<CHistParams> params = conf_mgr->GetHistParams(m_Source);

    NON_CONST_ITERATE(TTrackProxies, iter, m_TrackProxies) {
        if (!(*iter)->GetShown())
            continue;
        CGraphTrack* gt = dynamic_cast<CGraphTrack*>((*iter)->GetTrack());
        if (!gt)
            continue;
        gt->SetLayout(CGraphTrack::eLayout_Overlayed);
        NON_CONST_ITERATE(CLayoutGroup::TObjectList, hist_it, gt->SetGroup().SetChildren()) {
            CHistogramGlyph* hg = dynamic_cast<CHistogramGlyph*>(&**hist_it);
            if (!hg)
                continue;
            axis_max = max(hg->GetMax(), axis_max);
            axis_min = min(hg->GetMin(), axis_min);
        }
    }
    if (params->m_Scale == CHistParams::eLinear && params->m_RangeAutoscale == false) {
        if (params->m_ValueRange.GetFrom() != params->m_ValueRange.GetEmptyFrom()) {
            axis_min = params->m_ValueRange.GetFrom();
            params->m_ClipOutliers = true;
        }
        if (params->m_ValueRange.GetTo() != params->m_ValueRange.GetEmptyTo()) {
            axis_max = params->m_ValueRange.GetTo();
            params->m_ClipOutliers = true;
        }
    }

    NON_CONST_ITERATE(CLayoutGroup::TObjectList, obj_it, SetGroup().SetChildren()) {
        CGraphTrack* gt = dynamic_cast<CGraphTrack*>(&**obj_it);
        if (!gt)
            continue;
        NON_CONST_ITERATE(CLayoutGroup::TObjectList, hist_it, gt->SetGroup().SetChildren()) {
            CHistogramGlyph* hg = dynamic_cast<CHistogramGlyph*>(&**hist_it);
            if (!hg)
                continue;
            hg->SetRenderingMode(m_Layout == eOverlay ? CHistogramGlyph::eMode_Overlay : CHistogramGlyph::eMode_Single);
            hg->SetAxisMax(axis_max);
            hg->SetAxisMin(axis_min);
            hg->SetFixedScale(true);
            hg->SetAxisRange();
        }
        gt->Update(true);
    }

    m_Grid->SetAnnotName(m_Source);
    m_Grid->SetAxisMax(axis_max);
    m_Grid->SetAxisMin(axis_min);
    m_Grid->SetFixedScale(true);
    m_Grid->SetAxisRange();
    CTrackContainer::x_OnLayoutChanged();
}

const CTrackTypeInfo& CGraphOverlay::GetTypeInfo() const
{
    return m_TypeInfo;
}

void CGraphOverlay::x_OnAllJobsFinished()
{
    CTrackContainer::x_OnAllJobsFinished();
    // Update common track settings
    ITERATE(TTrackProxies, iter, m_TrackProxies) {
        if (!(*iter)->GetShown())
            continue;
        if (!(*iter)->TrackNotSet())
            UpdateTrackParams((*iter)->GetSource());
    }

}
void CGraphOverlay::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
     GetGroup().GetHTMLActiveAreas(p_areas);
     for (const auto& ch : GetGroup().GetChildren()) {
         auto graph_track = dynamic_cast<const CGraphTrack*>(ch.GetPointer());
         if (graph_track) {
             for (auto& glyph : graph_track->GetChildren()) {
                const CHistogramGlyph* hg = dynamic_cast<const CHistogramGlyph*>(glyph.GetPointer());
                if (hg) {
                    const_cast<CHistogramGlyph*>(hg)->SetRenderingMode(CHistogramGlyph::eMode_Overlay);
                    hg->GetHTMLActiveAreas(p_areas);
                    const_cast<CHistogramGlyph*>(hg)->SetRenderingMode(m_Layout == eOverlay ? CHistogramGlyph::eMode_Overlay : CHistogramGlyph::eMode_Single);
                }
             }
         }
     }
     m_Legend.GetHTMLActiveAreas(p_areas);
     if (!GetId().empty()) {
         NON_CONST_ITERATE (TAreaVector, iter, *p_areas) {
             if (iter->m_ParentId.empty()) {
                 iter->m_ParentId = GetId();
             }
         }
     }
}


bool CGraphOverlay::CanDrop(CRef<CLayoutTrack>& track)
{
    CGraphTrack* gt = dynamic_cast<CGraphTrack*>(&*track);
    return (gt != 0);
}

bool CGraphOverlay::Drop(CRef<CLayoutTrack>& track)
{
    CGraphTrack* gt = dynamic_cast<CGraphTrack*>(&*track);
    if (!gt)
        return false;
    CTrackContainer* p_track = dynamic_cast<CTrackContainer*>(GetParentTrack());
    if (!p_track)
        return false;

    CTempTrackProxy* current_track_proxy = 0;
    CTempTrackProxy* new_track_proxy = 0;

    NON_CONST_ITERATE(CTrackContainer::TTrackProxies, iter, p_track->GetSubtrackProxies())
    {
        if ((*iter)->GetOrder() == gt->GetOrder()) {
            new_track_proxy = &**iter;
        } else if ((*iter)->GetOrder() == GetOrder()) {
            current_track_proxy = &**iter;
        }
        if (current_track_proxy && new_track_proxy)
            break;
    }
    _ASSERT(current_track_proxy != 0 && new_track_proxy != 0);
    if (current_track_proxy == 0 || new_track_proxy == 0)
        return false;
    //_ASSERT(new_track_proxy->GetKey() == "graph_track");

    x_SaveSettings(NcbiEmptyString);
    CRef<CTempTrackProxy> new_overlay = current_track_proxy->Clone();

    CRef<CTempTrackProxy> subtrack_proxy = new_track_proxy->Clone();
    int new_order = 1;
    ITERATE(TTrackProxies, ch_it, new_overlay->GetChildren())  {
        if ((*ch_it)->GetOrder() >= new_order) {
            new_order = (*ch_it)->GetOrder() + 1;
        }
    }
    subtrack_proxy->SetOrder(new_order);
    new_overlay->GetChildren().push_back(subtrack_proxy);
    CRef<CHistParamsManager> conf_mgr = m_gConfig->GetHistParamsManager();
    CRef<CHistParams> overlay_params = conf_mgr->GetHistParams(m_Source);
    UpdateSource(new_overlay);
    conf_mgr->AddTempSettings(m_Source, overlay_params);
    p_track->RemoveTrack(new_track_proxy->GetOrder());
    p_track->RemoveTrack(current_track_proxy->GetOrder());

    p_track->AddNewTrack(dynamic_cast<CTrackProxy*>(new_overlay.GetPointer()));

    return true;
}

void CGraphOverlay::UpdateSource(CTempTrackProxy* self_proxy)
{
    if (self_proxy == 0) {
        CTrackContainer* p_track = dynamic_cast<CTrackContainer*>(GetParentTrack());
        if (!p_track)
            return;
        TTrackProxies::iterator t_it = p_track->GetSubtrackProxies().begin();
        while (t_it != p_track->GetSubtrackProxies().end() && (*t_it)->GetOrder() != GetOrder())
            ++t_it;
        if (t_it == p_track->GetSubtrackProxies().end())
            return;
        self_proxy = *t_it;
    }
    _ASSERT(self_proxy);
    vector<string> subtracks;
    NON_CONST_ITERATE(CTrackContainer::TTrackProxies, iter, self_proxy->GetChildren())
    {
        string id = NcbiEmptyString;
        if (!(*iter)->GetId().empty())
            subtracks.push_back((*iter)->GetId());
        else if (!(*iter)->GetUId().empty()) {
            subtracks.push_back((*iter)->GetUId());
        } else {
            CTrackProxy* proxy = dynamic_cast<CTrackProxy*>(&**iter);
            if (proxy && !proxy->GetAnnots().empty()) // subtracks with multiple annots are not expected
                subtracks.push_back(proxy->GetAnnots().front());
            else if (!(*iter)->GetSource().empty())
                subtracks.push_back((*iter)->GetSource());
        }
    }
    string source = "graphs";
    if (!subtracks.empty()) {
        sort(subtracks.begin(), subtracks.end());
        source += ":";
        source += NStr::Join(subtracks, "-");
    }
    m_Source = source;
    self_proxy->SetSource(m_Source);
}


void CGraphOverlay::MoveTrackOut(int track_id)
{
    CTrackContainer* p_track = dynamic_cast<CTrackContainer*>(GetParentTrack());
    if (!p_track)
        return;
    CRef<CTempTrackProxy> proxy(0);
    ERASE_ITERATE(TTrackProxies, t_it, m_TrackProxies) {
        if ((*t_it)->GetOrder() == track_id) {
            proxy = *t_it;
            break;
        }
    }
    if (!proxy)
        return;
    CTrackProxy* overlay_proxy = 0;

    NON_CONST_ITERATE(CTrackContainer::TTrackProxies, iter, p_track->GetSubtrackProxies())
    {
        if ((*iter)->GetOrder() == GetOrder()) {
            overlay_proxy= dynamic_cast<CTrackProxy*>(&**iter);
            break;
        }
    }
    _ASSERT(overlay_proxy);
    if (!overlay_proxy)
        return;

//    CRef<CTempTrackProxy> new_track = proxy->Clone();
//    CTrackProxy* new_track_proxy = dynamic_cast<CTrackProxy*>(new_track.GetPointer());
//    _ASSERT(new_track_proxy);
//    int new_order = p_track->GetSubtrackProxies().back()->GetOrder() + 1;
  //  new_track->SetOrder(new_order);
    int new_order = 1;
    ITERATE(TTrackProxies, ch_it, p_track->GetSubtrackProxies())
    {
        if ((*ch_it)->GetOrder() >= new_order) {
            new_order = (*ch_it)->GetOrder() + 1;
        }
    }

    ERASE_ITERATE(CTrackProxy::TTrackProxies, it, overlay_proxy->GetChildren()) {
        if ((*it)->GetOrder() == track_id) {
            VECTOR_ERASE(it, overlay_proxy->GetChildren());
            break;
        }
    }

    RemoveTrack(track_id);
    CRef<CHistParamsManager> conf_mgr = m_gConfig->GetHistParamsManager();
    CRef<CHistParams> old_params = conf_mgr->GetHistParams(m_Source);
    UpdateSource();
    conf_mgr->AddTempSettings(m_Source, old_params);

    CRef<CTrackProxy> n_proxy(new CTrackProxy());// new_order, proxy->GetName(), true, "graph_track", NcbiEmptyString));
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
    p_track->AddTrackProxy(n_proxy.GetPointer());

    //p_track->AddNewTrack(new_track_proxy);

    // move the new track right after the overlay
    p_track->MoveTrack(new_order, GetOrder() + 1);
    x_UpdateMsg();

    if (GetChildren().empty()) {
        p_track->RemoveTrack(GetOrder());

    }
}

END_NCBI_SCOPE
