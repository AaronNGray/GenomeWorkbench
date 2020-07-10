/*  $Id: segment_map_track.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/widgets/seq_graphic/segment_map_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job_result.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <gui/widgets/seq_graphic/layout_track_proxy.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/seq_graphic/comment_glyph.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/utils/rgba_color.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/registry.hpp>


#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/annot_ci.hpp>

#include <objects/seqfeat/SeqFeatData.hpp>

#include <wx/menu.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
///   CSegmentMapTrack
///////////////////////////////////////////////////////////////////////////////

static const size_t kMaxSegmentMapRows = 6;
//static const string kDefTrackTitle = "Segment map";
static const string kDefProfile = "Default";
static string kDefTrackTitle = "Segment Map";
static const string kBaseKey = "GBPlugins.SeqGraphicComponentMap";
static const int kSegmentCutoff = 500;
static const size_t kCompactThreshold = 80;

typedef SStaticPair<const char*, CSegmentMapTrack::TSegmentLevel> TLevelStr;
static const TLevelStr s_LevelStrs[] = {
    { "Adaptive",       CSGSegmentMapDS::eAdaptive },
    { "Component Map",  CSGSegmentMapDS::eComponent },
    { "Contig Map",     CSGSegmentMapDS::eContig },
};


typedef CStaticArrayMap<string, CSegmentMapTrack::TSegmentLevel> TLevelMap;
DEFINE_STATIC_ARRAY_MAP(TLevelMap, sm_LevelMap, s_LevelStrs);


CSegmentMapTrack::TSegmentLevel
CSegmentMapTrack::LevelStrToValue(const string& level)
{
    TLevelMap::const_iterator iter = sm_LevelMap.find(level);
    if (iter != sm_LevelMap.end()) {
        return iter->second;
    }
    NCBI_THROW(CException, eInvalid, "Invalid level string: " + level);

}


const string& CSegmentMapTrack::LevelValueToStr(TSegmentLevel level)
{
    TLevelMap::const_iterator iter;
    for (iter = sm_LevelMap.begin();  iter != sm_LevelMap.end();  ++iter) {
        if (iter->second == level) {
            return iter->first;
        }
    }
    return kEmptyStr;
}


typedef SStaticPair<const char*, CSegmentConfig::ELabelPosition> TLabelPosStr;
static const TLabelPosStr s_LabelPosStrs[] = {
    { "above",    CSegmentConfig::ePos_Above },
    { "inside",   CSegmentConfig::ePos_Inside },
    { "no label", CSegmentConfig::ePos_NoLabel },
    { "side",     CSegmentConfig::ePos_Side },
};

typedef CStaticArrayMap<string, CSegmentConfig::ELabelPosition> TLabelPosMap;
DEFINE_STATIC_ARRAY_MAP(TLabelPosMap, sm_LabelPosMap, s_LabelPosStrs);


CSegmentConfig::ELabelPosition
CSegmentMapTrack::LabelPosStrToValue(const string& pos)
{
    TLabelPosMap::const_iterator iter = sm_LabelPosMap.find(pos);
    if (iter != sm_LabelPosMap.end()) {
        return iter->second;
    }
    NCBI_THROW(CException, eInvalid, "Invalid label position string: " + pos);
}


const string&
CSegmentMapTrack::LabelPosValueToStr(CSegmentConfig::ELabelPosition pos)
{
    TLabelPosMap::const_iterator iter;
    for (iter = sm_LabelPosMap.begin();  iter != sm_LabelPosMap.end();  ++iter) {
        if (iter->second == pos) {
            return iter->first;
        }
    }
    return kEmptyStr;
}


CTrackTypeInfo CSegmentMapTrack::m_TypeInfo("segment_map_track",
                                            "Graphical View Segment Map Track");

CSegmentMapTrack::CSegmentMapTrack(CSGSegmentMapDS* ds,
                                   CRenderingContext* r_cntx)
    : CDataTrack(r_cntx)
    , m_DS(ds)
    , m_ResolveSequences(true)
    , m_UseCutoff(false)
{
    m_DS->SetJobListener(this);
    SetLayoutPolicy(m_Layered);
}


CSegmentMapTrack::~CSegmentMapTrack()
{
    if ( !m_DS->AllJobsFinished() ) {
        m_DS->DeleteAllJobs();
    }
}


const CTrackTypeInfo& CSegmentMapTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}


string CSegmentMapTrack::GetFullTitle() const
{
    if (GetTitle().empty()) {
        return kDefTrackTitle;
    }
    return GetTitle();
}


void  CSegmentMapTrack::x_LoadSettings(const string& preset_style,
                                       const TKeyValuePairs& settings)
{
    if ( !m_Config ) {
        m_Config.Reset(new CSegmentConfig);
    }

    if (preset_style.empty()) {
        SetProfile(kDefProfile);
    } else {
        SetProfile(preset_style);
    }

    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryReadView view;
    view = CSGConfigUtils::GetReadView(
        registry, kBaseKey, GetProfile(), kDefProfile);
    m_ResolveSequences = view.GetBool("ResolveSequences", true);
    m_UseCutoff = view.GetBool("UseCutoff", false);

    // backward compatibility settings
    string level_str = view.GetString("Depth");

    ITERATE (CSGConfigUtils::TKeyValuePairs, iter, settings) {
        try {
            if (NStr::EqualNocase(iter->first, "Depth")) {
                level_str = iter->second;
                /// HACK: for backward compatibility, used to be -1, 0 and 1
                /// now use the expressive strings
                if (level_str.length() == 1) { // old settings
                    level_str = LevelValueToStr((TSegmentLevel)NStr::StringToInt(level_str));
                }
            } else if (NStr::EqualNocase(iter->first, "ResolveSequences")) {
                m_ResolveSequences = NStr::StringToBool(iter->second);
            } else if (NStr::EqualNocase(iter->first, "UseCutoff")) {
                m_UseCutoff = NStr::StringToBool(iter->second);
            }
        } catch (CException&) {
            LOG_POST(Warning << "CSegmentMap::LoadSettings() - invalid setting: "
                << iter->first << ":" << iter->second);
        }
    }

    if (m_DS->GetDepth() == -1  &&  !level_str.empty()) {
        // adaptive mode
        TSegmentLevel level = CSGSegmentMapDS::eAdaptive;
        try {
            level = LevelStrToValue(level_str);
        } catch (CException& e) {
            LOG_POST(Warning << e.GetMsg());
        }
        m_DS->SetSegmentLevel(level);
    }
    // Otherwise
    // else {
        // The segment level is set explicitly, no need to overwrite it
        // with the setting from the setting file. There are two cases
        // this may happen:
        //  1. In multiple layout mode, different level of segments can
        //     only be shown at the corresponding level
        //  2. In case, segment map track are separated into Compoment
        //     track and scaffold track, the level is fixed.
    // }

    // label position
    view = CSGConfigUtils::GetLabelPosReadView(
        registry, kBaseKey, GetProfile(), m_gConfig->GetLabelPos(), kDefProfile);
    try {
        m_Config->m_LabelPos = LabelPosStrToValue(view.GetString("LabelPos"));
    } catch (CException& e) {
        LOG_POST(Warning << e.GetMsg());
    }

    // sizes
    view = CSGConfigUtils::GetSizeReadView(
        registry, kBaseKey, GetProfile(), m_gConfig->GetSizeLevel(), kDefProfile);
    m_Config->m_BarHeight = view.GetInt("BarHeight", 10);
    CSGConfigUtils::GetFont(
        view, "LabelFontFace", "LabelFontSize", m_Config->m_LabelFont);

    // colors
    view = CSGConfigUtils::GetColorReadView(
        registry, kBaseKey, GetProfile(), m_gConfig->GetColorTheme(), kDefProfile);
    CSGConfigUtils::GetColor(view, "BG",        m_Config->m_BG);
    CSGConfigUtils::GetColor(view, "FGExc",     m_Config->m_FGExc);
    CSGConfigUtils::GetColor(view, "FGInc_F",   m_Config->m_FGInc_F);
    CSGConfigUtils::GetColor(view, "FGInc_D",   m_Config->m_FGInc_D);
    CSGConfigUtils::GetColor(view, "FGInc_W",   m_Config->m_FGInc_W);
    CSGConfigUtils::GetColor(view, "FGInc_U",   m_Config->m_FGInc_U);
    CSGConfigUtils::GetColor(view, "FGGap",     m_Config->m_FGGap);
    CSGConfigUtils::GetColor(view, "Label",     m_Config->m_Label);
    CSGConfigUtils::GetColor(view, "SelLabel",     m_Config->m_SelLabel);

    m_Layered->SetVertSpace(m_gConfig->GetObjectSpace());
}


void  CSegmentMapTrack::x_SaveSettings(const string& preset_style)
{
    TKeyValuePairs settings;

    if ( !preset_style.empty() ) {
        settings["profile"] = preset_style;
    }
    SetProfile(CSGConfigUtils::ComposeProfileString(settings));
}


void  CSegmentMapTrack::x_SaveConfiguration(const string& /*preset_style*/) const
{
    const string& profile = GetProfile();
    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryWriteView view =
        CSGConfigUtils::GetWriteView(registry, kBaseKey, profile, kDefProfile);

    // it is very unlikely we will change these
    //if (m_Config) {
    if (false) {
        // label position
        view = CSGConfigUtils::GetLabelPosRWView(
            registry, kBaseKey, profile, m_gConfig->GetLabelPos(), kDefProfile);
        view.Set("LabelPos", LabelPosValueToStr(m_Config->m_LabelPos));

        // save size settings
        view = CSGConfigUtils::GetSizeRWView(
            registry, kBaseKey, profile, m_gConfig->GetSizeLevel(), kDefProfile);
        view.Set("BarHeight", m_Config->m_BarHeight);
        CSGConfigUtils::SetFont(
            view, "LabelFontFace", "LabelFontSize", m_Config->m_LabelFont);

        // save color settings
        view = CSGConfigUtils::GetColorRWView(
            registry, kBaseKey, profile, m_gConfig->GetColorTheme(), kDefProfile);
        CSGConfigUtils::SetColor(view, "BG",        m_Config->m_BG);
        CSGConfigUtils::SetColor(view, "FGExc",     m_Config->m_FGExc);
        CSGConfigUtils::SetColor(view, "FGInc_F",   m_Config->m_FGInc_F);
        CSGConfigUtils::SetColor(view, "FGInc_D",   m_Config->m_FGInc_D);
        CSGConfigUtils::SetColor(view, "FGInc_W",   m_Config->m_FGInc_W);
        CSGConfigUtils::SetColor(view, "FGInc_U",   m_Config->m_FGInc_U);
        CSGConfigUtils::SetColor(view, "FGGap",     m_Config->m_FGGap);
        CSGConfigUtils::SetColor(view, "Label",     m_Config->m_Label);
        CSGConfigUtils::SetColor(view, "SelLabel",  m_Config->m_SelLabel);
    }
}


void CSegmentMapTrack::x_UpdateData()
{
    m_DS->DeleteAllJobs();
    x_SetStartStatus();
    int cutoff = m_UseCutoff ? kSegmentCutoff : -1;
    m_DS->LoadSegmentMap(m_Context->GetVisSeqRange(), cutoff, eSegmentMap);
}


void CSegmentMapTrack::x_OnJobCompleted(CAppJobNotification& notify)
{
    m_DS->ClearJobID(notify.GetJobID());
    CRef<CObject> res_obj = notify.GetResult();
    CJobResultBase* result = dynamic_cast<CJobResultBase*>(&*res_obj);
    if ( !result ) {
        LOG_POST(Error << "CSegmentMapTrack::x_OnJobCompleted() notification for job \
                does not contain results.");
        return;
    }

    EJobType type = (EJobType)result->m_Token;
    switch (type) {
    case eSegmentMap:
        x_AddSegmentMapLayout(dynamic_cast<CSGJobResult&>(*res_obj));
        break;
    case eSegmentMapSeqID:
        {{
            int num_partial = 0;
            int num_finished = 0;
            int num_wgs = 0;
            int num_unknown = 0;
            ITERATE (CSeqGlyph::TObjects, iter, GetChildren()) {
                CSeqGlyph* obj = const_cast<CSeqGlyph*>(iter->GetPointer());
                CSegmentGlyph* seq_map = dynamic_cast<CSegmentGlyph*>(obj);
                CSegmentConfig::ESequenceQuality qual = seq_map->GetSeqQuality();
                switch (qual) {
                    case CSegmentConfig::eDraft:
                        num_partial++;
                        break;
                    case CSegmentConfig::eFinished:
                        num_finished++;
                        break;
                    case CSegmentConfig::eWgs:
                        num_wgs++;
                        break;
                    default:
                        num_unknown++;
                        break;
                }
            }
            string msg = "";
            if (num_partial > 0 ) msg += ", partial: " + NStr::IntToString(num_partial);
            if (num_finished > 0 ) msg += ", finished: " + NStr::IntToString(num_finished);
            if (num_wgs > 0 ) msg += ", WGS: " + NStr::IntToString(num_wgs);
            if (num_unknown > 0 ) msg += ", other segments: " + NStr::IntToString(num_unknown);
            SetMsg(msg);
        }}
        break;
    default:
        break;
    }

    x_OnLayoutChanged();
}


void CSegmentMapTrack::x_AddSegmentMapLayout(const CSGJobResult& result)
{
    SetGroup().Clear();
    size_t count = result.m_ObjectList.size();
    if (count == 0) {
        // an empty track should be navigable as there might be out of range objects
        m_Attrs |= fNavigable;
        SetMsg("");
        x_UpdateLayout();
        return;
    }

    SetObjects(result.m_ObjectList);
    CCommentGlyph* comment =
        dynamic_cast<CCommentGlyph*>(SetChildren().front().GetPointer());
    if (comment) {
        CRef<CCommentConfig> c_config(new CCommentConfig);
        c_config->m_ShowBoundary = false;
        c_config->m_ShowConnection = false;
        c_config->m_Centered = true;
        c_config->m_LabelColor.Set(1.0f, 0.0f, 0.0f);
        c_config->m_LineColor.Set(1.0f, 1.0f, 1.0f);
        c_config->m_Font.SetFontFace(CGlTextureFont::eFontFace_Helvetica);
        c_config->m_Font.SetFontSize(12);
        comment->SetConfig(c_config);
        SetMsg("");
    } else {
        bool drop_label = false;
        if (result.m_ObjectList.size() > kCompactThreshold  &&
            m_Config->m_LabelPos == CSegmentConfig::ePos_Side) {
            drop_label = true;
        }
        NON_CONST_ITERATE (CSeqGlyph::TObjects, iter, SetChildren()) {
            CSegmentGlyph* seg = dynamic_cast<CSegmentGlyph*>(iter->GetPointer());
            seg->SetConfig(m_Config);
            if (!GetHighlights().empty()  &&  x_NeedHighlight(seg)) {
                seg->SetHighlighted(true);
                if (m_HighlightsColor.get())
                    seg->SetHighlightsColor(*m_HighlightsColor.get());
            } else {
                seg->SetHideLabel(drop_label);
            }
        }
        string msg = ", " + NStr::SizetToString(count) + " component";
        msg += (count != 1 ? "s shown" : " shown");
        SetMsg(msg);
        m_Attrs |= fNavigable;

    }

    // Compress Segment Map if there are more than
    // kMaxSegmentMapRows rows in the layout
    m_Layered->SetMaxRow(kMaxSegmentMapRows);

    // launch a new job to resolve segment map seq_ids
    if (!comment  &&  (count < kSegmentLoadLimit  ||  m_ResolveSequences)) {
        x_SetStatus(", Resolving component seq-ids...", 0);
        m_DS->LoadSegmentMapSeqIDs(SetChildren(), eSegmentMapSeqID);
    }

    x_UpdateLayout();
}


bool CSegmentMapTrack::x_NeedHighlight(const CSegmentGlyph* seg) const
{
    if (seg) {
        string label;
        seg->GetLabel(label, CLabel::eContent);
        NStr::ToLower(label);
        if (GetHighlights().count(label) > 0) {
            return true;
        }
    }
    return false;
}


///////////////////////////////////////////////////////////////////////////////
/// CSegmentMapTrackFactory methods
///////////////////////////////////////////////////////////////////////////////
ILayoutTrackFactory::TTrackMap
CSegmentMapTrackFactory::CreateTracks(SConstScopedObject& object,
                                      ISGDataSourceContext* ds_context,
                                      CRenderingContext* r_cntx,
                                      const SExtraParams& params,
                                      const TAnnotMetaDataList& /*src_annots*/) const
{
    TTrackMap tracks;

    // params.m_Level stores the target level at which the data will
    // be displayed. In multi-level layout mode, level = 0 means top
    // sequence level, level = 1 means the second level, and so forth.
    // 'level' variable here represent the segment level which is
    // slightly different the target level. For segment level, 0 means
    // scaffold, and 1 means components. -1 means show all levels.
    int level = params.m_Level;
    if (params.m_Adaptive) {
        // adpative means Default Layout
        level = -1;
    } else {
        // Otherwise, it means Multi-level layout where annoations/data
        // at different levels are shown at the corresponding level.

        // For multi-level layout, we move segment map one level down.
        // So we will show level 0 segments (scaffolds) at level 1, and
        // show leve 1 segments (components) at level 2. Therefore, there
        // will be no segment map shown at level 0.
        level -= 1;
        if (level < 0) {
            return tracks;
        }
    }

    CIRef<ISGDataSource> ds = ds_context->GetDS(
        typeid(CSGSegmentMapDSType).name(), object);
    CSGSegmentMapDS* segment_ds = dynamic_cast<CSGSegmentMapDS*>(ds.GetPointer());

    segment_ds->SetDepth(level);
    TSeqRange range = params.m_Range;
    if (range.Empty()) {
        range = r_cntx->GetVisSeqRange();
    }

    if (range.IsWhole()) {
        CRef<CSGSequenceDS> seq_ds = r_cntx->GetSeqDS();
        range.Set(0, seq_ds->GetSequenceLength() - 1);
    }
    bool is_chromosome = x_IsChromosome(*segment_ds);
    segment_ds->SetChromosome(is_chromosome);

    if (params.m_SkipGenuineCheck  ||  segment_ds->HasSegmentMap(level, range)) {
        CRef<CSegmentMapTrack> track(new CSegmentMapTrack(segment_ds, r_cntx));
        tracks[kDefTrackTitle] = track.GetPointer();
    }
    return tracks;
}


string CSegmentMapTrackFactory::GetExtensionIdentifier() const
{
    return CSegmentMapTrack::m_TypeInfo.GetId();
}


string CSegmentMapTrackFactory::GetExtensionLabel() const
{
    return CSegmentMapTrack::m_TypeInfo.GetDescr();
}


CRef<CTrackConfigSet>
CSegmentMapTrackFactory::GetSettings(const string& /*profile*/,
                                     const TKeyValuePairs& settings,
                                     const CTempTrackProxy* track_proxy) const
{
    CRef<CTrackConfigSet> config_set(new CTrackConfigSet);
    CRef<CTrackConfig> config(new CTrackConfig);
    config_set->Set().push_back(config);
    config->SetHelp() = "GenBank sequences used to construct chromosome";
    config->SetLegend_text("anchor_9");

    // bool use_cutoff = true;
    // bool resolve_ids = false;
    CSegmentMapTrack::TSegmentLevel level = CSGSegmentMapDS::eAdaptive;
    ITERATE (CSGConfigUtils::TKeyValuePairs, iter, settings) {
        try {
            if (NStr::EqualNocase(iter->first, "Depth")) {
                level = CSegmentMapTrack::LevelStrToValue(iter->second);
            // } else if (NStr::EqualNocase(iter->first, "ResolveSequences")) {
            //     resolve_ids = NStr::StringToBool(iter->second);
            // } else if (NStr::EqualNocase(iter->first, "UseCutoff")) {
            //     use_cutoff = NStr::StringToBool(iter->second);
            }
        } catch (CException&) {
            LOG_POST(Warning << "CSegmentMap::GetSettings() - invalid setting: "
                << iter->first << ":" << iter->second);
        }
    }
    config->SetHidden_settings().push_back(
        CTrackConfigUtils::CreateHiddenSetting(
        "Depth",
        CSegmentMapTrack::LevelValueToStr(level)));



    /*
    if (level == CSGSegmentMapDS::eComponent) {
//         config->AddCheckBox("Auto hide", "UseCutoff", "If checked, component "
// "map will be shown only when there are no more than 500 components in the given "
// "range.", use_cutoff);
//         config->AddCheckBox("Color segments", "ResolveSequences", "Color the "
// "segments based on the color code. By default, the color coding is disabled (for "
// "performance consideration).", resolve_ids);

        config->SetHidden_settings().push_back(
            CTrackConfigUtils::CreateHiddenSetting(
                "Depth",
                CSegmentMapTrack::LevelValueToStr(CSGSegmentMapDS::eComponent)));

        config->SetHidden_settings().push_back(
            CTrackConfigUtils::CreateHiddenSetting("ResolveSequences", "false"));
        config->SetHelp() = "The GenBank sequences are shown in the order that \
they contribute to the assembly. Colors are as follows:<br> Blue:finished sequence<br>\
Orange:Draft sequence<br>Green:WGS<br>Gray:Other<br>Black:Gap<br>\
Beige:Sequence overlap not used to generate chromosome sequence.";

    } else if (level == CSGSegmentMapDS::eContig) {
        config->SetHidden_settings().push_back(
            CTrackConfigUtils::CreateHiddenSetting(
                "Depth",
                CSegmentMapTrack::LevelValueToStr(CSGSegmentMapDS::eContig)));
        config->SetHelp() = "Coordinated, mapped set of overlapping DNA segments \
derived from the genetic source and representing a complete molecule segment.";

    } else { // adaptive
        CRef<CChoice> choice = CTrackConfigUtils::CreateChoice(
            "Depth", "Segment Level",
            CSegmentMapTrack::LevelValueToStr((CSegmentMapTrack::TSegmentLevel)-1),
            "Choose which segment level to show");

        choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
                CSegmentMapTrack::LevelValueToStr(CSGSegmentMapDSAdaptive),
                CSegmentMapTrack::LevelValueToStr(CSGSegmentMapDSAdaptive),
                "Adaptively show different level of segment based on zoom level",
                ""));

        choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
                CSegmentMapTrack::LevelValueToStr(CSGSegmentMapDS::eContig),
                CSegmentMapTrack::LevelValueToStr(CSGSegmentMapDS::eContig),
                "Show contig map",
                ""));
        choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
                CSegmentMapTrack::LevelValueToStr(CSGSegmentMapDS::eComponent),
                CSegmentMapTrack::LevelValueToStr(CSGSegmentMapDS::eComponent),
                "Show component map",
                ""));

        config->SetChoice_list().push_back(choice);
        config->SetHelp() = "Segment map (either contig or components map)";
    }
    */

    /*
    CRef<CChoice> choice = CTrackConfigUtils::CreateChoice(
        "Depth", "Segment Level",
        CSegmentMapTrack::LevelValueToStr(level),
        "Choose which segment level to show");

    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
        CSegmentMapTrack::LevelValueToStr(CSGSegmentMapDS::eContig),
        CSegmentMapTrack::LevelValueToStr(CSGSegmentMapDS::eContig),
        "Show contig map",
        ""));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
        CSegmentMapTrack::LevelValueToStr(CSGSegmentMapDS::eComponent),
        CSegmentMapTrack::LevelValueToStr(CSGSegmentMapDS::eComponent),
        "Show component map",
        ""));

    config->SetChoice_list().push_back(choice);
    config->SetHelp() = "Segment map (either contig or components map)";
    */

    return config_set;
}


bool CSegmentMapTrackFactory::x_IsChromosome(CSGSegmentMapDS& ds) const
{
    bool is_chromosome = CSGUtils::IsChromosome(ds.GetBioseqHandle(), ds.GetScope());

    // The logic for checking whether a given sequence is a chromosome or not
    // is not reliable, especially for the local sequences that don't contain
    // enough molecule information for determination. We treat any sequences
    // with two levels of segment map as chromosomes.
    if ( !is_chromosome ) {
        TSeqRange range(0, ds.GetBioseqHandle().GetBioseqLength() - 1);
        int seg_level = ds.GetSegmentMapLevels(range);
        is_chromosome = seg_level > 1;
    }

    return is_chromosome;
}


END_NCBI_SCOPE
