/*  $Id: seqgraphic_conf.cpp 44130 2019-11-02 00:14:51Z rudnev $
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
 * Authors:  Vlad Lebedev, Liangshou Wu
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_conf.hpp>
#include <gui/widgets/seq_graphic/comment_glyph.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>


BEGIN_NCBI_SCOPE

USING_SCOPE(objects);

static const string kBaseKey("GBPlugins.SeqGraphicView");
static const string kFeatParamsKey("GBPlugins.SeqGraphicFeats");
static const string kCPParamsKey("GBPlugins.SeqGraphicClonePlacement");
static const string kCommentConfigKey("GBPlugins.SeqGraphicCommentConfig");
static const string kCustomColorKey("GBPlugins.CustomFeatureColors");
static const string kDefFeatKey("Master");

CSeqGraphicConfig::CSeqGraphicConfig(bool cgi_mode, 
                                     CGuiRegistry* registry,
                                     const string& theme,
                                     const string& color,
                                     const string& size,
                                     const string& deco,
                                     const string& profile,
                                     const string& layout,
                                     const string& label_pos)
    : m_Registry( registry ? registry : &CGuiRegistry::GetInstance())
    , m_CurrTheme(theme)
    , m_CurrColor(color)
    , m_CurrSize(size)
    , m_CurrDeco(deco)
    , m_CurrProfile(profile)
    , m_CurrLayout(layout)
    , m_CurrLabelPos(label_pos)
    , m_CgiMode(cgi_mode)
    , m_AnnotDepth(eDepth_unknown)
    , m_LabelFont(new CGlTextureFont())
    , m_CommentFont(new CGlTextureFont())
    , m_TitleFont(new CGlTextureFont())
    , m_HistParamsManager(new CHistParamsManager)
    , m_Dirty(false)
{
    x_VerifySettings();
    LoadSettings();
}


CSeqGraphicConfig::~CSeqGraphicConfig()
{
}


void CSeqGraphicConfig::LoadSettings()
{
    CRegistryReadView view = m_Registry->GetReadView(kBaseKey);

    // In case of using a preset theme, we need to verify it does exist.
    if ( m_CurrTheme.empty()  ||
        !view.HasField(ThemeKey() + "." + m_CurrTheme) ) {
        m_CurrTheme = view.GetString("CurrentTheme", DefTheme());
    }

    // load theme settings
    view = CSGConfigUtils::GetReadView(*m_Registry, kBaseKey + "." + ThemeKey(),
        m_CurrTheme, DefTheme());
    if (m_CurrColor.empty()) {
        m_CurrColor = view.GetString("ColorTheme", CSGConfigUtils::DefColorTheme());
    }
    if (m_CurrSize.empty()) {
        m_CurrSize = view.GetString("SizeLevel", CSGConfigUtils::DefSizeLevel());
    }
    if (m_CurrDeco.empty()) {
        m_CurrDeco = view.GetString("DecoreateStyle", CSGConfigUtils::DefDecoStyle());
    }
    if (m_CurrProfile.empty()) {
        m_CurrProfile = view.GetString("TrackProfile", CSGConfigUtils::DefTrackProfile());
    }
    if (m_CurrLayout.empty()) {
        m_CurrLayout = view.GetString("Layout", CSGConfigUtils::DefLayout());
    }
    if (m_CurrLabelPos.empty()) {
        m_CurrLabelPos = view.GetString("LabelPosition", CSGConfigUtils::DefLabelPosition());
    }
    if (m_CurrGlobal.empty()) {
        m_CurrGlobal = view.GetString("GlobalSettings", CSGConfigUtils::DefGlobalSettingStyle());
    }

    // loading size settings
    view = CSGConfigUtils::GetSizeReadView(*m_Registry, kBaseKey, "", m_CurrSize);
    m_BarHeight = view.GetReal("BarHeight", 9.0);
    m_CmmtTopMargin = view.GetReal("CommentTopMargin", 5.0);
    m_CmmtBottomMargin = view.GetReal("CommentBottomMargin", 2.0);
    m_ObjSpace = view.GetInt("ObjectSpace", 3);
    m_IconSize = view.GetInt("IconSize", 12);
    m_IconSignSize = view.GetInt("IconSignSize", 8);

    CSGConfigUtils::GetFont(view, "LabelFontFace", "LabelFontSize", *m_LabelFont);
    CSGConfigUtils::GetFont(view, "CommentFontFace", "CommentFontSize", *m_CommentFont);
    CSGConfigUtils::GetFont(view, "TitleFontFace", "TitleFontSize", *m_TitleFont);

    // loading color settings
    view = CSGConfigUtils::GetColorReadView(*m_Registry, kBaseKey, "", m_CurrColor);
    CSGConfigUtils::GetColor(view, "Background", m_Background);
    CSGConfigUtils::GetColor(view, "SelectionHairlineOne", m_SelHairlineOne);
    CSGConfigUtils::GetColor(view, "SelectionHairlineMany", m_SelHairlineMany);
    CSGConfigUtils::GetColor(view, "SelectionHairlineDown", m_SelHairlineDown);
    CSGConfigUtils::GetColor(view, "BGComment", m_BGComment);
    CSGConfigUtils::GetColor(view, "FGComment", m_FGComment);
    CSGConfigUtils::GetColor(view, "TitleBackground", m_TitleBackground);
    CSGConfigUtils::GetColor(view, "NonNCBITitleBackground", m_NonNCBITitleBackground);
    CSGConfigUtils::GetColor(view, "SelLabel", m_SelLabel);
    CSGConfigUtils::GetColor(view, "Selection", m_Selection);
    CSGConfigUtils::GetColor(view, "GridHorizontal", m_GridHorizontal);
    CSGConfigUtils::GetColor(view, "GridVertical", m_GridVertical);

    // loading other global settings
    view = CSGConfigUtils::GetReadView(*m_Registry,
        kBaseKey + "." + CSGConfigUtils::GlobalSettingStyleKey(),
        m_CurrGlobal, CSGConfigUtils::DefGlobalSettingStyle());
    m_AdjustRuler = view.GetBool("AdjustRulerToSelection", false);
    m_ShowComments = view.GetBool("ShowComments", true);
    m_ShowSimpleComments = view.GetBool("ShowSimpleComments", true);
    m_ShowMessage = view.GetBool("ShowMessage", true);
    m_ReserveCmmtSpace = view.GetBool("ShowInvisibleComments", false);
    m_ShowCmmtBg = view.GetBool("ShowCommentBackground", false);
    m_RepeatComment = view.GetBool("RepeatComment", false);
    m_AutoHideTrack = view.GetBool("HideTrackWithNoData", false);
    m_ShowHorzGrid = view.GetBool("ShowHorizontalGrid", true);
    m_ShowVertGrid = view.GetBool("ShowVerticalGrid", false);
    m_ShowTrackMinusPlusSign = view.GetBool("ShowTrackMinusPlusSign", true);
    m_BackgroundLoading = view.GetBool("BackgroundLoading", true);
    m_ShowDownwardHairlines = view.GetBool("ShowDownwardHairlines", true);

    m_HairLineOption = view.GetInt("HairLineOption", 2);
    m_CmmtRepDist = view.GetInt("CommentRepeatDist", 1024);
    m_OverviewCutoff = view.GetInt("OverviewCutoff", 1500);
    m_HistogramCutoff = view.GetInt("HistogramCutoff", 50);
    m_LineWidth = view.GetReal("LineWidth", 1.0);
    m_ZoomFactor = view.GetReal("MouseZoomFactor", 25.0);
    SetAnnotDepth(view.GetInt("AnnotationDepth", -2));

    // cache the feature parameters
    x_LoadFeatSettings();
    x_LoadHistSettings();
}

void CSeqGraphicConfig::SaveSettings(bool clear_themes)
{
    // This must come first since it depends on m_CurrColor which is reset if
    // clear_themes is true
    x_SaveHistSettings();

    if (m_Dirty) {
        // clean out old settings
        CRegistryReadView::TKeys keys;
        m_Registry->GetReadView(kBaseKey).GetTopKeys(keys);
        CRegistryWriteView view = m_Registry->GetWriteView(kBaseKey);
        ITERATE(CRegistryReadView::TKeys, iter, keys) {
            view.DeleteField(iter->key);
        }

        // save theme settings
        view.Set("CurrentTheme", m_CurrTheme);

        view = m_Registry->GetWriteView(kBaseKey + "." + ThemeKey()+ "." + m_CurrTheme);
        view.Set("ColorTheme", m_CurrColor);
        view.Set("SizeLevel", m_CurrSize);
        view.Set("DecoreateStyle", m_CurrDeco);
        view.Set("Layout", m_CurrLayout);
        view.Set("LabelPosition", m_CurrLabelPos);
        view.Set("TrackProfile", m_CurrProfile);
        //view.Set("GlobalSettings", m_CurrGlobal);

        /// no need to save them since we don't expose them for change currently.
        /*

        // saving size settings
        view = CSGConfigUtils::GetSizeRWView(*m_Registry, kBaseKey, "", m_CurrSize);
        view.Set("BarHeight", m_BarHeight);
        view.Set("CommentTopMargin", m_CmmtTopMargin);
        view.Set("CommentBottomMargin", m_CmmtBottomMargin);
        view.Set("ObjectSpace", m_ObjSpace);
        view.Set("IconSize", m_IconSize);
        view.Set("IconSignSize", m_IconSignSize);

        CSGConfigUtils::SetFont(view, "LabelFontFace", "LabelFontSize", *m_LabelFont);
        CSGConfigUtils::SetFont(view, "CommentFontFace", "CommentFontSize", *m_CommentFont);
        CSGConfigUtils::SetFont(view, "TitleFontFace", "TitleFontSize", *m_TitleFont);

        // saving color settings
        view = CSGConfigUtils::GetColorRWView(*m_Registry, kBaseKey, "", m_CurrColor);
        CSGConfigUtils::SetColor(view, "Background", m_Background);
        CSGConfigUtils::SetColor(view, "SelectionHairlineOne", m_SelHairlineOne);
        CSGConfigUtils::SetColor(view, "SelectionHairlineMany", m_SelHairlineMany);
        CSGConfigUtils::SetColor(view, "BGComment", m_BGComment);
        CSGConfigUtils::SetColor(view, "FGComment", m_FGComment);
        CSGConfigUtils::SetColor(view, "SelLabel", m_SelLabel);
        CSGConfigUtils::SetColor(view, "Selection", m_Selection);
        CSGConfigUtils::SetColor(view, "GridHorizontal", m_GridHorizontal);
        CSGConfigUtils::SetColor(view, "GridVertical", m_GridVertical);

        // saving the global settings
        view = CSGConfigUtils::GetWriteView(*m_Registry,
            kBaseKey + "." + CSGConfigUtils::GlobalSettingStyleKey(),
            m_CurrGlobal, CSGConfigUtils::DefGlobalSettingStyle());
        view.Set("AdjustRulerToSelection", m_AdjustRuler);
        view.Set("ShowComments", m_ShowComments);
        view.Set("ShowSimpleComments", m_ShowSimpleComments);
        view.Set("ShowMessage", m_ShowMessage);
        view.Set("ShowInvisibleComments", m_ReserveCmmtSpace);
        view.Set("ShowCommentBackground", m_ShowCmmtBg);
        view.Set("RepeatComment", m_RepeatComment);
        view.Set("HideTrackWithNoData", m_AutoHideTrack);
        view.Set("ShowHorizontalGrid", m_ShowHorzGrid);
        view.Set("ShowVerticalGrid", m_ShowVertGrid);
        view.Set("ShowTrackMinusPlusSign", m_ShowTrackMinusPlusSign);
        view.Set("BackgroundLoading", m_BackgroundLoading);

        view.Set("HairLineOption", m_HairLineOption);
        view.Set("CommentRepeatDist", m_CmmtRepDist);
        view.Set("OverviewCutoff", m_OverviewCutoff);
        view.Set("HistogramCutoff", m_HistogramCutoff);
        view.Set("LineWidth", m_LineWidth);
        view.Set("MouseZoomFactor", m_ZoomFactor);

        */


        // saving color settings
        view = CSGConfigUtils::GetColorRWView(*m_Registry, kBaseKey, "", m_CurrColor);
        CSGConfigUtils::SetColor(view, "SelectionHairlineOne", m_SelHairlineOne);
        CSGConfigUtils::SetColor(view, "SelectionHairlineMany", m_SelHairlineMany);
        CSGConfigUtils::SetColor(view, "SelectionHairlineDown", m_SelHairlineDown);

        // saving the global settings
        view = CSGConfigUtils::GetWriteView(*m_Registry,
            kBaseKey + "." + CSGConfigUtils::GlobalSettingStyleKey(),
            m_CurrGlobal, CSGConfigUtils::DefGlobalSettingStyle());
        view.Set("AdjustRulerToSelection", m_AdjustRuler);
        view.Set("HairLineOption", m_HairLineOption);
        view.Set("ShowDownwardHairlines", m_ShowDownwardHairlines);

        // reset the current theme and styles, otherwise,
        // those current theme and style strings won't be
        // reloaded
        if (clear_themes) {
            m_CurrTheme = kEmptyStr;
            m_CurrColor = kEmptyStr;
            m_CurrSize = kEmptyStr;
            m_CurrDeco = kEmptyStr;
            m_CurrProfile = kEmptyStr;
        }

        m_Dirty = false;
    }

    x_SaveFeatSettings();
}


CConstRef<CFeatureParams>
CSeqGraphicConfig::GetFeatParams(TFeatSubtype subtype) const
{
    const CFeatList& feats(*CSeqFeatData::GetFeatList());
    vector<string> feat_hierarchy = feats.GetStoragekeys(subtype);

    // try them in reverse order so the more specific keys get tried first.
    vector<string>::reverse_iterator riter(feat_hierarchy.end());
    vector<string>::reverse_iterator rend(feat_hierarchy.begin());

    for ( ; riter != rend;  ++riter) {
        TFeatParams::const_iterator iter = m_FeatSettings.find(*riter);
        if ( iter != m_FeatSettings.end()) {
            return iter->second;
        }
    }

    // shouldn't get here.
    // means that there wasn't a Master feature item in the global settings.
    _ASSERT(false);
    return CConstRef<CFeatureParams>(NULL);
}


CConstRef<CFeatureParams>
CSeqGraphicConfig::GetFeatParams(const string& name) const
{
    TFeatParams::const_iterator iter = m_FeatSettings.find(name);
    if (iter == m_FeatSettings.end()) {
        iter = m_FeatSettings.find(kDefFeatKey);
    }
    return iter->second;
}


CConstRef<CCommentConfig> CSeqGraphicConfig::GetCommentConfig() const
{
    return CConstRef<CCommentConfig>(m_CommentConfig.GetPointer());
}


CRef<CCategoryConfig> CSeqGraphicConfig::GetCategoryConfig()
{
    if ( !m_CategoryConfig ) {
        CCategoryConfig::TGroupMap group_map;
        CCategoryConfig::TSubgroupMap sub_group_map;
        InitCategoryConfig(group_map, sub_group_map);
    }
    return m_CategoryConfig;
}

void CSeqGraphicConfig::InitCategoryConfig(const CCategoryConfig::TGroupMap& group_map, 
                                            const CCategoryConfig::TSubgroupMap& sub_group_map)
{
    if ( !m_CategoryConfig ) {
        m_CategoryConfig.Reset(new CCategoryConfig);
        string key = "GBPlugins.SeqGraphicCategory";
        CRegistryReadView top_view = m_Registry->GetReadView(key);
        CRegistryReadView::TKeys cat_keys;
        top_view.GetTopKeys(cat_keys);
        ITERATE(CRegistryReadView::TKeys, iter, cat_keys) {
            CCategoryConfig::SCategorySettings cat;
            cat.m_Category =
                CTrackConfigUtils::CreateCategory(iter->key, iter->key, "", -1);

            CRegistryReadView view =
                m_Registry->GetReadView(key + "." + iter->key);
            CRegistryReadView::TKeys keys;
            view.GetTopKeys(keys);
            ITERATE(CRegistryReadView::TKeys, sub_iter, keys) {
                const string& key_n = sub_iter->key;
                if (key_n == "DisplayName") {
                    cat.m_Category->SetDisplay_name() = view.GetString(key_n);
                } else if (key_n == "Order") {
                    // penalize orders that came from static config so that TMS-defined ones would come first
                    cat.m_Category->SetOrder() = view.GetInt(key_n) + 10000;
                } else if (key_n == "Help") {
                    cat.m_Category->SetHelp() = view.GetString(key_n);
                } else {
                    CRegistryReadView sub_view =
                        m_Registry->GetReadView(key + "." + iter->key + "." + key_n);

                    int sub_order = sub_view.GetInt("Order", -1);
                    if(sub_order != -1) {
                        sub_order += 10000;
                    }
                    CRef<CCategory> sub_cat = CTrackConfigUtils::CreateCategory(
                        key_n,
                        sub_view.GetString("DisplayName", key_n),
                        sub_view.GetString("Help", ""),
                        sub_order);

                    cat.AddSubcategory(sub_cat);
                }
            }
            m_CategoryConfig->AddCategory(cat);
        }
        // after static config has been read, update and augment it with what came from TMS with the same names
        for(const auto& group: group_map) {
            CRef<CCategory> cat;
            cat = m_CategoryConfig->GetCategory(group.first);
            if ( !cat ) {
                cat = CTrackConfigUtils::CreateCategory(group.first, group.first, "", group.second);
                CCategoryConfig::SCategorySettings cat_set;
                cat_set.m_Category = cat;
                m_CategoryConfig->AddCategory(cat_set);
            } else {
                cat->SetOrder(group.second);
            }
        }
        for(const auto& sub_group: sub_group_map) {
            CRef<CCategory> sub_cat;
            sub_cat = m_CategoryConfig->GetSubcategory(sub_group.first.first, sub_group.first.second);
            if ( !sub_cat ) {
                sub_cat = CTrackConfigUtils::CreateCategory(sub_group.first.second, sub_group.first.second, "", sub_group.second);
                m_CategoryConfig->AddSubcategory(sub_group.first.first, sub_cat);
            } else {
                sub_cat->SetOrder(sub_group.second);
            }
        }
    }
}

void CSeqGraphicConfig::LoadSettingStyles(const CGuiRegistry& reg, string& theme,
                                          string& color, string& size, string& deco,
                                          string& profile, string& label)
{
    CRegistryReadView view = reg.GetReadView(kBaseKey);

    // In case of using a preset theme, we need to verify it does exist.
    if ( theme.empty()  ||
         !view.HasField(ThemeKey() + "." + theme) ) {
        theme = view.GetString("CurrentTheme", DefTheme());
    }

    // load theme settings
    view = CSGConfigUtils::GetReadView(reg, kBaseKey + "." + ThemeKey(),
                                       theme, DefTheme());
    if (color.empty()) {
        color = view.GetString("ColorTheme", CSGConfigUtils::DefColorTheme());
    }
    if (size.empty()) {
        size = view.GetString("SizeLevel", CSGConfigUtils::DefSizeLevel());
    }
    if (deco.empty()) {
        deco = view.GetString("DecoreateStyle", CSGConfigUtils::DefDecoStyle());
    }
    if (profile.empty()) {
        profile = view.GetString("TrackProfile", CSGConfigUtils::DefTrackProfile());
    }
    if (label.empty()) {
        label = view.GetString("LabelPosition", CSGConfigUtils::DefLabelPosition());
    }
}


void CSeqGraphicConfig::x_VerifySettings() const
{
    CRegistryReadView view = m_Registry->GetReadView(kBaseKey);
    // make sure we find what we expect in the registry.
    if (view.IsEmpty()) {
        NCBI_THROW(CException, eUnknown, "CSeqGraphicConfig: there are errors in the settings.");
    }
}

void CSeqGraphicConfig::x_LoadFeatSettings()
{
    m_FeatSettings.clear();

    // loading feature's custom colors
    CRegistryReadView view =
        m_Registry->GetReadView(kCustomColorKey + "." + m_CurrColor);
    if ( !m_CustomFeatColors ) {
        m_CustomFeatColors.Reset(new CCustomFeatureColor);
    }
    CRgbaColor color;
    CSGConfigUtils::GetColor(view, "LowIdentity", color);
    m_CustomFeatColors->SetColor(CCustomFeatureColor::eLowIdentity, color);
    CSGConfigUtils::GetColor(view, "MidIdentity", color);
    m_CustomFeatColors->SetColor(CCustomFeatureColor::eMidIdentity, color);
    CSGConfigUtils::GetColor(view, "HighIdentity", color);
    m_CustomFeatColors->SetColor(CCustomFeatureColor::eHighIdentity, color);
    CSGConfigUtils::GetColor(view, "CNV", color);
    m_CustomFeatColors->SetColor(CCustomFeatureColor::eCNV, color);
    CSGConfigUtils::GetColor(view, "Insertion", color);
    m_CustomFeatColors->SetColor(CCustomFeatureColor::eInsertion, color);
    CSGConfigUtils::GetColor(view, "Loss", color);
    m_CustomFeatColors->SetColor(CCustomFeatureColor::eLoss, color);
    CSGConfigUtils::GetColor(view, "Gain", color);
    m_CustomFeatColors->SetColor(CCustomFeatureColor::eGain, color);
    CSGConfigUtils::GetColor(view, "Complex", color);
    m_CustomFeatColors->SetColor(CCustomFeatureColor::eComplex, color);
    CSGConfigUtils::GetColor(view, "Unknown", color);
    m_CustomFeatColors->SetColor(CCustomFeatureColor::eUnknown, color);
    CSGConfigUtils::GetColor(view, "Inversion", color);
    m_CustomFeatColors->SetColor(CCustomFeatureColor::eInversion, color);
    CSGConfigUtils::GetColor(view, "Everted", color);
    m_CustomFeatColors->SetColor(CCustomFeatureColor::eEverted, color);
    CSGConfigUtils::GetColor(view, "Transchr", color);
    m_CustomFeatColors->SetColor(CCustomFeatureColor::eTranschr, color);
    CSGConfigUtils::GetColor(view, "DeletionInsertion", color);
    m_CustomFeatColors->SetColor(CCustomFeatureColor::eDeletionInsertion, color);

    view = m_Registry->GetReadView(kFeatParamsKey);
    CRegistryReadView::TKeys keys;
    view.GetTopKeys(keys);
    ITERATE(CRegistryReadView::TKeys, iter, keys) {
        CRegistryReadView sub_view =
            CSGConfigUtils::GetReadView(*m_Registry, kFeatParamsKey, iter->key, kDefFeatKey);
        CRef<CFeatureParams> params(new CFeatureParams);
        params->m_CustomColors = m_CustomFeatColors;
        params->m_ShowNtRuler = sub_view.GetBool("NtRuler", true);
        params->m_ShowAaRuler = sub_view.GetBool("AaRuler", true);
        params->m_ShowStrandIndicator = sub_view.GetBool("ShowStrandIndicator", true);
        params->m_CgiMode = m_CgiMode;

        sub_view = CSGConfigUtils::GetLabelPosReadView(
            *m_Registry, kFeatParamsKey, iter->key, m_CurrLabelPos, kDefFeatKey);
        params->m_LabelPos = CFeatureParams::ePos_Side;
        string label_pos = sub_view.GetString("LabelPos");
        try {
            params->m_LabelPos =
                (CFeatureParams::ELabelPosition)CFeatureParams::DecoStrToValue(label_pos);
        } catch (CException& e) {
            LOG_POST(Warning << "CSeqGraphicConfig::x_LoadFeatSettings() "
                << e.GetMsg());
        }

        // loading color settings
        sub_view = CSGConfigUtils::GetColorReadView(
            *m_Registry, kFeatParamsKey, iter->key, m_CurrColor, kDefFeatKey);
        CSGConfigUtils::GetColor(sub_view, "FG", params->m_fgColor);
        CSGConfigUtils::GetColor(sub_view, "BG", params->m_bgColor);
        CSGConfigUtils::GetColor(sub_view, "Label", params->m_LabelColor);
        params->m_NonConsensus = params->m_fgColor; // set a default
        CSGConfigUtils::GetColor(sub_view, "NonConsensus", params->m_NonConsensus);

        // loading size settings
        sub_view = CSGConfigUtils::GetSizeReadView(
            *m_Registry, kFeatParamsKey, iter->key, m_CurrSize, kDefFeatKey);
        params->m_BarHeight = sub_view.GetReal("BarHeight", 9.0);
        params->m_OverviewFactor = sub_view.GetReal("OverviewSizeFactor", 0.5);
        params->m_LineWidth = sub_view.GetReal("LineWidth", 1.0);
        CSGConfigUtils::GetFont(sub_view, "LabelFontFace", "LabelFontSize", params->m_LabelFont);

        // loading decoration settings
        string curr_key = CSGConfigUtils::ComposeDecoKey(iter->key, m_CurrDeco);
        string def_key1 = CSGConfigUtils::ComposeDecoKey(kDefFeatKey, m_CurrDeco);
        sub_view = CSGConfigUtils::GetReadView(*m_Registry, kFeatParamsKey, curr_key, def_key1);
        params->m_HeadHeight = sub_view.GetReal("HeadHeight", 1.0);
        params->m_TailHeight = sub_view.GetReal("TailHeight", 1.0);
        params->m_HeadStyle = CFeatureParams::eHead_No;
        params->m_TailStyle = CFeatureParams::eTail_No;
        params->m_LineStyle = CFeatureParams::eLine_Solid;
        params->m_Connections = CFeatureParams::eStraightLine;
        try {
            if (sub_view.HasField("HeadStyle"))
                params->m_HeadStyle =
                    (CFeatureParams::EHeadStyle)CFeatureParams::DecoStrToValue(
                    sub_view.GetString("HeadStyle"));
            if (sub_view.HasField("TailStyle"))
                params->m_TailStyle =
                    (CFeatureParams::ETailStyle)CFeatureParams::DecoStrToValue(
                    sub_view.GetString("TailStyle"));
            if (sub_view.HasField("LineStyle"))
                params->m_LineStyle =
                    (CFeatureParams::ELineStyle)CFeatureParams::DecoStrToValue(
                    sub_view.GetString("LineStyle"));
            if (sub_view.HasField("Connection"))
                params->m_Connections =
                    (CFeatureParams::EConnection)CFeatureParams::DecoStrToValue(
                    sub_view.GetString("Connection"));
            if (sub_view.HasField("BoxStyle"))
                params->m_BoxStyle =
                    (CFeatureParams::EBoxStyle)CFeatureParams::DecoStrToValue(
                    sub_view.GetString("BoxStyle"));
        } catch (CException& e) {
            LOG_POST(Warning << "CSeqGraphicConfig::x_LoadFeatSettings() "
                << e.GetMsg());
        }

        m_FeatSettings[iter->key] = params;
    }

    // load settings for clone placement user type feature
    if ( !m_ClonePlacementSettings ) {
        m_ClonePlacementSettings.Reset(new CClonePlacementParams);
    }

    CClonePlacementParams* cp_conf = m_ClonePlacementSettings;

    view = CSGConfigUtils::GetLabelPosReadView(
        *m_Registry, kCPParamsKey, "Default", m_CurrLabelPos);
    cp_conf->m_LabelPos = CFeatureParams::ePos_Side;
    string label_pos = view.GetString("LabelPos");
    try {
        cp_conf->m_LabelPos =
            (CFeatureParams::ELabelPosition)CFeatureParams::DecoStrToValue(label_pos);
    } catch (CException& e) {
        LOG_POST(Warning << "CSeqGraphicConfig::x_LoadFeatSettings() "
            << e.GetMsg());
    }

    // loading color settings
    view = CSGConfigUtils::GetColorReadView(
        *m_Registry, kCPParamsKey, "Default", m_CurrColor);
    CSGConfigUtils::GetColor(view, "Concordant", cp_conf->m_ConcordantColor);
    CSGConfigUtils::GetColor(view, "Discordant", cp_conf->m_DiscordantColor);
    CSGConfigUtils::GetColor(view, "UnknownConcordancy", cp_conf->m_UnknownConcordancyColor);
    CSGConfigUtils::GetColor(view, "Highlight", cp_conf->m_HighlightColor);
    CSGConfigUtils::GetColor(view, "CombinedPlacement", cp_conf->m_CombinedPlacementColor);
    CSGConfigUtils::GetColor(view, "FakeEnd", cp_conf->m_FakeEndColor);
    CSGConfigUtils::GetColor(view, "Label", cp_conf->m_LabelColor);

    // loading size settings
    view = CSGConfigUtils::GetSizeReadView(
        *m_Registry, kCPParamsKey, "Default", m_CurrSize);
    cp_conf->m_BarHeight = view.GetReal("BarHeight", 6.0);
    cp_conf->m_HeadHeight = view.GetReal("HeadHeight", 8.0);
    cp_conf->m_OverviewFactor = view.GetReal("OverviewSizeFactor", 0.5);
    CSGConfigUtils::GetFont(view, "LabelFontFace", "LabelFontSize", cp_conf->m_LabelFont);


    // loading settings for comment
    if ( !m_CommentConfig ) {
        m_CommentConfig.Reset(new CCommentConfig);
    }
    view = m_Registry->GetReadView(kCommentConfigKey + ".Default");
    m_CommentConfig->m_ShowBoundary = view.GetBool("ShowBoundary", false);
    m_CommentConfig->m_ShowConnection = view.GetBool("ShowConnection", true);
    m_CommentConfig->m_Centered = view.GetBool("Centered", true);

    // loading color settings
    view = CSGConfigUtils::GetColorReadView(
        *m_Registry, kCommentConfigKey, "Default", m_CurrColor);
    CSGConfigUtils::GetColor(view, "LabelColor", m_CommentConfig->m_LabelColor);
    CSGConfigUtils::GetColor(view, "LineColor", m_CommentConfig->m_LineColor);

    // loading size settings
    view = CSGConfigUtils::GetSizeReadView(
        *m_Registry, kCommentConfigKey, "Default", m_CurrSize);
    CSGConfigUtils::GetFont(view, "LabelFontFace", "LabelFontSize", m_CommentConfig->m_Font);
}

void CSeqGraphicConfig::x_SaveFeatSettings()
{
    // we don't need to save the setting for features since we don't
    // expose an interfaces for changing the feature settings.
}


END_NCBI_SCOPE

