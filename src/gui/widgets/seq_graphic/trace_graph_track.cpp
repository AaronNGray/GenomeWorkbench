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
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/seq_graphic/trace_graph_track.hpp>
#include <gui/widgets/seq_graphic/trace_graph_ds.hpp>
#include <gui/widgets/seq_graphic/trace_glyph.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_data_source.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>


#include <gui/widgets/seq_graphic/layout_track_proxy.hpp>
#include <gui/widgets/seq_graphic/comment_glyph.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>

#include <gui/objects/LegendItem.hpp>

#include <objects/general/Object_id.hpp>
#include <corelib/rwstream.hpp>
#include <connect/services/neticache_client.hpp>
#include <serial/objistr.hpp>
#include <serial/objostr.hpp>
#include <serial/serial.hpp>


#include <wx/menu.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
///   CTraceGraphTrack

static const string kDefaultAnnotName = "Traces";

CTrackTypeInfo CTraceGraphTrack::m_TypeInfo("trace_track",
                                       "DNA Sequencing Chromatograms");

CTraceGraphTrack::CTraceGraphTrack(CSGTraceGraphDS* ds, CRenderingContext* r_cntx)
    : CDataTrack(r_cntx)
    , m_DS(ds)
    , m_Config(new CTraceGraphConfig)
{
    m_DS->SetJobListener(this);
    SetLayoutPolicy(m_Simple);
    x_RegisterIcon(SIconInfo(
        eIcon_Settings, "Settings", true, "track_settings"));

}


CTraceGraphTrack::~CTraceGraphTrack()
{
}


const CTrackTypeInfo& CTraceGraphTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}

CRef<CSGGenBankDS> CTraceGraphTrack::GetDataSource()
{
    return CRef<CSGGenBankDS>(m_DS.GetPointer());
}


CConstRef<CSGGenBankDS> CTraceGraphTrack::GetDataSource() const
{
    return CConstRef<CSGGenBankDS>(m_DS.GetPointer());
}


void CTraceGraphTrack::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
    // use the standard way ?
    CGlyphContainer::GetHTMLActiveAreas(p_areas);
}


CHTMLActiveArea* CTraceGraphTrack::InitHTMLActiveArea(TAreaVector* p_areas) const
{
    return CLayoutTrack::InitHTMLActiveArea(p_areas);
}


void CTraceGraphTrack::x_LoadSettings(const string& /*preset_style*/,
                                 const TKeyValuePairs& settings)
{
    _ASSERT(m_Config);
    if (!m_Config)
        return;
    // Parse known parameters
    ITERATE (TKeyValuePairs, iter, settings) {
        try {
            if (NStr::EqualNocase(iter->first, "colorA")) {
                m_Config->m_colors[0].FromString(iter->second);
            }  else if (NStr::EqualNocase(iter->first, "colorC")) {
                m_Config->m_colors[1].FromString(iter->second);
            } else if (NStr::EqualNocase(iter->first, "colorG")) {
                m_Config->m_colors[2].FromString(iter->second);
            } else if (NStr::EqualNocase(iter->first, "colorT")) {
                m_Config->m_colors[3].FromString(iter->second);
            } else if (NStr::EqualNocase(iter->first, "colorConfMin")) {
                m_Config->m_colorConfMin.FromString(iter->second);
            } else if (NStr::EqualNocase(iter->first, "colorConfMax")) {
                m_Config->m_colorConfMax.FromString(iter->second);
            } else if (NStr::EqualNocase(iter->first, "height")) {
                m_Config->m_Height = NStr::StringToNonNegativeInt(iter->second);
            } else if (NStr::EqualNocase(iter->first, "style")) {
                if (iter->second == "curve")
                    m_Config->m_SignalStyle = CTraceGraphConfig::eCurve;
                else if (iter->second == "intensity")
                    m_Config->m_SignalStyle = CTraceGraphConfig::eIntensity;

            } else if (NStr::EqualNocase(iter->first, "show_conf_graph")) {
                m_Config->m_ConfGraphState = (iter->second == "false")
                    ? CTraceGraphConfig::eHidden : CTraceGraphConfig::eExpanded;
            }
        } catch (std::exception&) {
            LOG_POST(Error << "CTraceGraphTrack::x_LoadSettings: invalid settings "
                     << iter->second);
        }
    }
}

void CTraceGraphTrack::x_SaveSettings(const string& /*preset_style*/)
{
    CSGConfigUtils::TKeyValuePairs settings;

    settings["colorA"] = m_Config->m_colors[0].ToString();
    settings["colorC"] = m_Config->m_colors[1].ToString();
    settings["colorG"] = m_Config->m_colors[2].ToString();
    settings["colorT"] = m_Config->m_colors[3].ToString();
    settings["colorConfMin"] = m_Config->m_colorConfMin.ToString();
    settings["colorConfMax"] = m_Config->m_colorConfMax.ToString();
    settings["height"] = NStr::NumericToString(m_Config->m_Height);
    settings["style"] = m_Config->m_SignalStyle == CTraceGraphConfig::eCurve
        ? "curve" : "intensity";
    settings["show_conf_graph"] = m_Config->m_ConfGraphState == CTraceGraphConfig::eExpanded
        ? "true" : "false";

    SetProfile(CSGConfigUtils::ComposeProfileString(settings));
}

void CTraceGraphTrack::x_UpdateData()
{
    CDataTrack::x_UpdateData();
    if (SetGroup().GetChildrenNum() == 0) {
        //x_SetStartStatus();
        m_DS->LoadData();
        if(m_DS->HasData()) {
            CRef<CTraceGlyph> glyph(new CTraceGlyph(m_DS->GetData(), m_Config));

            //SetGroup().Clear();
            SetGroup().PushBack(glyph);
            SetTitle(m_DS->GetData()->GetTitle());
            //x_SetFinishStatus();
        }
    }
    x_UpdateLayout();
}

void CTraceGraphTrack::x_OnJobCompleted(CAppJobNotification& notify)
{
}

///////////////////////////////////////////////////////////////////////////////
/// CTraceGraphTrackFactory methods

ILayoutTrackFactory::TTrackMap
CTraceGraphTrackFactory::CreateTracks(SConstScopedObject& object,
                                 ISGDataSourceContext* ds_context,
                                 CRenderingContext* r_cntx,
                                 const SExtraParams& params,
                                 const TAnnotMetaDataList& src_annots) const
{
    ILayoutTrackFactory::TTrackMap tracks;
    CIRef<ISGDataSource> ds = ds_context->GetDS(
        typeid(CSGTraceGraphDSType).name(), object);
    CSGTraceGraphDS* trace_graph_ds = dynamic_cast<CSGTraceGraphDS*>(ds.GetPointer());
    if(!params.m_SkipGenuineCheck && !trace_graph_ds->HasData())
        return tracks;
    CRef<CTraceGraphTrack> track(new CTraceGraphTrack(trace_graph_ds, r_cntx));
    tracks["Traces"] = track.GetPointer();
    return tracks;
}


string CTraceGraphTrackFactory::GetExtensionIdentifier() const
{
    return CTraceGraphTrack::m_TypeInfo.GetId();
}


string CTraceGraphTrackFactory::GetExtensionLabel() const
{
    return CTraceGraphTrack::m_TypeInfo.GetDescr();
}


CRef<CTrackConfigSet>
CTraceGraphTrackFactory::GetSettings(const string& /*profile*/,
                                const TKeyValuePairs& settings,
                                const CTempTrackProxy* track_proxy) const
{
    CRef<CTrackConfigSet> config_set(new CTrackConfigSet);
    CRef<CTrackConfig> config(new CTrackConfig);
    config_set->Set().push_back(config);
    config->SetLegend_text("anchor_13");
    string style = "curve";
    string show_conf_graph = "true";

    ITERATE (TKeyValuePairs, iter, settings) {
        try {
            if (NStr::EqualNocase(iter->first, "colorA")) {
                config->SetHidden_settings().push_back
                    (CTrackConfigUtils::CreateHiddenSetting(iter->first, iter->second));
            }  else if (NStr::EqualNocase(iter->first, "colorC")) {
                config->SetHidden_settings().push_back
                    (CTrackConfigUtils::CreateHiddenSetting(iter->first, iter->second));
            } else if (NStr::EqualNocase(iter->first, "colorG")) {
                config->SetHidden_settings().push_back
                    (CTrackConfigUtils::CreateHiddenSetting(iter->first, iter->second));
            } else if (NStr::EqualNocase(iter->first, "colorT")) {
                config->SetHidden_settings().push_back
                    (CTrackConfigUtils::CreateHiddenSetting(iter->first, iter->second));
            } else if (NStr::EqualNocase(iter->first, "colorConfMin")) {
                config->SetHidden_settings().push_back
                    (CTrackConfigUtils::CreateHiddenSetting(iter->first, iter->second));
            } else if (NStr::EqualNocase(iter->first, "colorConfMax")) {
                config->SetHidden_settings().push_back
                    (CTrackConfigUtils::CreateHiddenSetting(iter->first, iter->second));
            } else if (NStr::EqualNocase(iter->first, "height")) {
                config->SetHidden_settings().push_back
                    (CTrackConfigUtils::CreateHiddenSetting(iter->first, iter->second));
            } else if (NStr::EqualNocase(iter->first, "style")) {
                style = iter->second;
            } else if (NStr::EqualNocase(iter->first, "show_conf_graph")) {
                show_conf_graph = iter->second;
            }
        } catch (std::exception&) {
            LOG_POST(Error << "CTraceGraphTrack::GetSettings: invalid settings "
                     << iter->second);
        }
    }

    CRef<CChoice> choice = CTrackConfigUtils::CreateChoice
        ("style",
         "Trace Chromatogram style",
         style,
         "Trace Chromatogram style");
    choice->SetValues().push_back
        (CTrackConfigUtils::CreateChoiceItem
         ("curve",
          "Signal",
          "Shown as Signal Traces",
          "Trace data shown as graph"));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            "intensity",
            "Intensity bands",
            "Shown as intensity bands",
            "Trace data shown as intensity bands"));
    config->SetChoice_list().push_back(choice);

    config->SetCheck_boxes().push_back(CTrackConfigUtils::CreateCheckBox(
       "show_conf_graph", "Show Confidence Graph", "", "",
       !NStr::EqualNocase(show_conf_graph,"false")));

    config->SetHelp() = GetThisTypeInfo().GetDescr();


    return config_set;
}


void CTraceGraphTrack::x_UpdateBoundingBox()
{
    CGlyphContainer::x_UpdateBoundingBox();
}

void CTraceGraphTrack::x_RenderContent() const
{
    if (GetHeight() == 0 || m_Context->IntersectVisible(this).Empty())
        return;
    CGlyphContainer::x_RenderContent();
}

void CTraceGraphTrack::x_OnIconClicked(TIconID id)
{
    switch (id) {
    case eIcon_Settings:
        x_OnSettingsIconClicked();
        break;
    default:
        // use default handlers
        CLayoutTrack::x_OnIconClicked(id);
    }
}


void CTraceGraphTrack::x_OnSettingsIconClicked()
{
    wxMenu menu;
    UseDefaultMarginWidth(menu);
    int id_base = 10000;
    wxMenuItem* item_style = 0;
    if (m_Config->m_SignalStyle == CTraceGraphConfig::eCurve)
        item_style = menu.AppendCheckItem(id_base, wxT("Show Intensity Graph"));
    else
        item_style = menu.AppendCheckItem(id_base, wxT("Show Signal Graph"));

    wxMenuItem* item_conf_graph = menu.AppendCheckItem(id_base + 1, wxT("Show Confidence Graph"));
    if (m_Config->m_ConfGraphState == CTraceGraphConfig::eExpanded)
        item_conf_graph->Check();
    m_LTHost->LTH_PopupMenu(&menu);

    if (item_style->IsChecked()) {
        if (m_Config->m_SignalStyle == CTraceGraphConfig::eCurve) {
            m_Config->m_SignalStyle = CTraceGraphConfig::eIntensity;
        } else {
            m_Config->m_SignalStyle = CTraceGraphConfig::eCurve;
        }
    }
    m_Config->m_ConfGraphState = item_conf_graph->IsChecked() ?
        CTraceGraphConfig::eExpanded : CTraceGraphConfig::eHidden;

    x_UpdateLayout();

}



END_NCBI_SCOPE
