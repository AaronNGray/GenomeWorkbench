/*  $Id: gene_model_config.cpp 43606 2019-08-07 22:40:09Z evgeniev $
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

#include <gui/widgets/seq_graphic/gene_model_config.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_conf.hpp>
#include <gui/widgets/seq_graphic/layout_group.hpp>
#include <gui/objutils/registry.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
///   CGeneModelConfig implementation
///

static const char* const kDefProfile = "Default";
static const char* const kCDSKey = "GBPlugins.SeqGraphicGeneModelCDS";

typedef SStaticPair<const char*, CGeneModelConfig::EMergeStyle> TStyleStr;
static const TStyleStr s_StyleStrs[] = {
    { "Merge all",   CGeneModelConfig::eMerge_All },
    { "Merge pairs",  CGeneModelConfig::eMerge_Pairs },
    { "No merge",    CGeneModelConfig::eMerge_No },
    { "Single line",    CGeneModelConfig::eMerge_OneLine },
};


typedef CStaticArrayMap<string, CGeneModelConfig::EMergeStyle> TStyleMap;
DEFINE_STATIC_ARRAY_MAP(TStyleMap, sm_StyleMap, s_StyleStrs);


CGeneModelConfig::EMergeStyle
CGeneModelConfig::StyleStrToValue(const string& style)
{
    TStyleMap::const_iterator iter = sm_StyleMap.find(style);
    if (iter != sm_StyleMap.end()) {
        return iter->second;
    }
    NCBI_THROW(CException, eInvalid, "Invalid merge style string: " + style);
}


const string&
CGeneModelConfig::StyleValueToStr(CGeneModelConfig::EMergeStyle style)
{
    TStyleMap::const_iterator iter;
    for (iter = sm_StyleMap.begin();  iter != sm_StyleMap.end();  ++iter) {
        if (iter->second == style) {
            return iter->first;
        }
    }
    return kEmptyStr;
}


typedef SStaticPair<const char*, CGeneModelConfig::ERenderingOption> TOptionStr;
static const TOptionStr s_OptionStrs[] = {
    { "GeneOnly",    CGeneModelConfig::eOption_GeneOnly },
    { "MergeAll",    CGeneModelConfig::eOption_MergeAll },
    { "MergePairs",  CGeneModelConfig::eOption_MergePairs },
    { "ShowAll",     CGeneModelConfig::eOption_ShowAll },
    { "ShowAllButGenes",   CGeneModelConfig::eOption_ShowAllButGenes },
    { "SingleLine",  CGeneModelConfig::eOption_SingleLine },
};


typedef CStaticArrayMap<string, CGeneModelConfig::ERenderingOption> TOptionMap;
DEFINE_STATIC_ARRAY_MAP(TOptionMap, sm_OptionMap, s_OptionStrs);


const string& CGeneModelConfig::OptionValueToStr(CGeneModelConfig::ERenderingOption option)
{
    TOptionMap::const_iterator iter;
    for (iter = sm_OptionMap.begin();  iter != sm_OptionMap.end();  ++iter) {
        if (iter->second == option) {
            return iter->first;
        }
    }
    return kEmptyStr;
}


CGeneModelConfig::ERenderingOption CGeneModelConfig::OptionStrToValue(const string& option)
{
    TOptionMap::const_iterator iter = sm_OptionMap.find(option);
    if (iter != sm_OptionMap.end()) {
        return iter->second;
    }
    NCBI_THROW(CException, eInvalid, "Invalid rendeirng option string: " + option);
}


void CGeneModelConfig::LoadSettings(CConstRef<CSeqGraphicConfig> g_conf,
                                    const string& reg_path,
                                    const string& profile)
{
    if ( !m_CdsConfig ) {
        m_CdsConfig.Reset(new CCdsConfig);
    }

    if ( !m_GeneBoxConfig ) {
        m_GeneBoxConfig.Reset(new CBoundaryParams(
            true, false, CRgbaColor(0.0, 0.6f, 0.0f, 0.1f),
            CRgbaColor(0.6f, 0.8f, 0.3f, 0.3f), 1.0));
    }

    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryReadView view =
        CSGConfigUtils::GetReadView(registry, reg_path, profile, kDefProfile);
    m_MergeStyle =
        StyleStrToValue(view.GetString("CdsRnaMergeStyle", "No merge"));
    m_ShowLMGeneLabel = view.GetBool("LandmarkGeneLabel", true);
    m_LandmarkFeatLimit = view.GetInt("LandmarkFeatLimit", 199);
    m_OverviewFeatCutoff = view.GetInt("OverviewFeatCutoff", 30);

    // load content settings
    m_ShowGenes = view.GetBool("Genes", true);
    m_ShowRNAs = view.GetBool("RNAs", true);
    m_ShowCDSs = view.GetBool("CDSs", true);
    m_ShowExons = view.GetBool("Exons", true);
    m_ShowVDJCs = view.GetBool("VDJCs", true);   
    m_ShowGeneProducts = view.GetBool("GeneProdcuts", true);
    m_ShowCDSProductFeats = view.GetBool("CDSProductFeats", false);
    m_ShowNtRuler = view.GetBool("NtRuler", true);
    m_ShowAaRuler = view.GetBool("AaRuler", true);
    m_ShowHistogram = view.GetBool("Histogram", true);
    m_CDSProfile = view.GetString("CDSProfile", kDefProfile);
    m_HighlightMode = view.GetInt("HighlightMode", 0);
    m_ShowLabel = view.GetBool("ShowLabel", true);

    // load color settings
    view = CSGConfigUtils::GetColorReadView(
        registry, kCDSKey, m_CDSProfile, g_conf->GetColorTheme(), kDefProfile);
    CSGConfigUtils::GetColor(view, "BGProtProduct", m_CdsConfig->m_bgProtProd);
    CSGConfigUtils::GetColor(view, "FGProtProduct", m_CdsConfig->m_fgProtProd);
    CSGConfigUtils::GetColor(view, "LabelProtProduct", m_CdsConfig->m_LabelProtProd);
    CSGConfigUtils::GetColor(view, "SeqProtOriginal", m_CdsConfig->m_SeqProt);
    CSGConfigUtils::GetColor(view, "SeqProtMismatch", m_CdsConfig->m_SeqProtMismatch);
    CSGConfigUtils::GetColor(view, "SeqProtTranslated", m_CdsConfig->m_SeqProtTrans);

    // load size settings
    view = CSGConfigUtils::GetSizeReadView(
        registry, kCDSKey, m_CDSProfile, g_conf->GetSizeLevel(), kDefProfile);

    CSGConfigUtils::GetFont(view, "ProdFontFace", "ProdFontSize", m_CdsConfig->m_ProdFont);
    CSGConfigUtils::GetFont(view, "TransFontFace", "TransFontSize", m_CdsConfig->m_TransFont);
}


void CGeneModelConfig::LoadSettingsPartial(
    const string& reg_path,
    const string& profile)
{
//    if ( !m_CdsConfig ) {
//        m_CdsConfig.Reset(new CCdsConfig);
//    }

    if ( !m_GeneBoxConfig ) {
        m_GeneBoxConfig.Reset(new CBoundaryParams(
            true, false, CRgbaColor(0.0, 0.6f, 0.0f, 0.1f),
            CRgbaColor(0.6f, 0.8f, 0.3f, 0.3f), 1.0));
    }

    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryReadView view =
        CSGConfigUtils::GetReadView(registry, reg_path, profile, kDefProfile);
    m_MergeStyle =
        StyleStrToValue(view.GetString("CdsRnaMergeStyle", "No merge"));
    m_ShowLMGeneLabel = view.GetBool("LandmarkGeneLabel", true);
    m_LandmarkFeatLimit = view.GetInt("LandmarkFeatLimit", 199);
    m_OverviewFeatCutoff = view.GetInt("OverviewFeatCutoff", 30);

    // load content settings
    m_ShowGenes = view.GetBool("Genes", true);
    m_ShowRNAs = view.GetBool("RNAs", true);
    m_ShowCDSs = view.GetBool("CDSs", true);
    m_ShowExons = view.GetBool("Exons", true);
    m_ShowVDJCs = view.GetBool("VDJCs", true);
    m_ShowGeneProducts = view.GetBool("GeneProdcuts", true);
    m_ShowCDSProductFeats = view.GetBool("CDSProductFeats", false);
//    m_ShowNtRuler = view.GetBool("NtRuler", true);
//    m_ShowAaRuler = view.GetBool("AaRuler", true);
    m_ShowNtRuler = true;
    m_ShowAaRuler = true;
    m_ShowHistogram = view.GetBool("Histogram", true);
    m_CDSProfile = view.GetString("CDSProfile", kDefProfile);
    m_HighlightMode = view.GetInt("HighlightMode", 0);
}


void CGeneModelConfig::SaveSettings(CConstRef<CSeqGraphicConfig> g_conf,
                                    const string& reg_path,
                                    const string& profile) const
{
    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryWriteView view =
        CSGConfigUtils::GetWriteView(registry, reg_path, profile, kDefProfile);
    view.Set("CdsRnaMergeStyle", StyleValueToStr(m_MergeStyle));
    view.Set("LandmarkGeneLabel", m_ShowLMGeneLabel);
    view.Set("LandmarkFeatLimit", m_LandmarkFeatLimit);
    view.Set("OverviewFeatCutoff", m_OverviewFeatCutoff);

    // save content settings
    view.Set("Genes", m_ShowGenes);
    view.Set("RNAs", m_ShowRNAs);
    view.Set("CDSs", m_ShowCDSs);
    view.Set("Exons", m_ShowExons);
    view.Set("VDJCs", m_ShowVDJCs);
    view.Set("GeneProdcuts", m_ShowGeneProducts);
    view.Set("CDSProductFeats", m_ShowCDSProductFeats);
    view.Set("NtRuler", m_ShowNtRuler);
    view.Set("AaRuler", m_ShowAaRuler);
    view.Set("Histogram", m_ShowHistogram);
    view.Set("CDSProfile", m_CDSProfile);
    view.Set("HighlightMode", m_HighlightMode);

    if (m_CdsConfig->m_Dirty) {
        // save color settings
        view = CSGConfigUtils::GetColorRWView(
            registry, kCDSKey, m_CDSProfile, g_conf->GetColorTheme(), kDefProfile);
        CSGConfigUtils::SetColor(view, "BGProtProduct", m_CdsConfig->m_bgProtProd);
        CSGConfigUtils::SetColor(view, "FGProtProduct", m_CdsConfig->m_fgProtProd);
        CSGConfigUtils::SetColor(view, "LabelProtProduct", m_CdsConfig->m_LabelProtProd);
        CSGConfigUtils::SetColor(view, "SeqProtOriginal", m_CdsConfig->m_SeqProt);
        CSGConfigUtils::SetColor(view, "SeqProtMismatch", m_CdsConfig->m_SeqProtMismatch);
        CSGConfigUtils::SetColor(view, "SeqProtTrans", m_CdsConfig->m_SeqProtTrans);

        // save size settings
        view = CSGConfigUtils::GetSizeRWView(
            registry, kCDSKey, m_CDSProfile, g_conf->GetSizeLevel(), kDefProfile);
        CSGConfigUtils::SetFont(view, "ProdFontFace", "ProdFontSize", m_CdsConfig->m_ProdFont);
        CSGConfigUtils::SetFont(view, "TransFontFace", "TransFontSize", m_CdsConfig->m_TransFont);
    }
}

void  CGeneModelConfig::UpdateSettings(const CSGConfigUtils::TKeyValuePairs& settings)
{
    // override those default settings passed in through profile settings
    ITERATE (CSGConfigUtils::TKeyValuePairs, iter, settings) {
        try {
            if (NStr::EqualNocase(iter->first, "Merge"))
                m_MergeStyle = StyleStrToValue(iter->second);
            else if (NStr::EqualNocase(iter->first, "LandmarkGeneLabel"))
                m_ShowLMGeneLabel = NStr::StringToBool(iter->second);
            else if (NStr::EqualNocase(iter->first, "Genes"))
                m_ShowGenes = NStr::StringToBool(iter->second);
            else if (NStr::EqualNocase(iter->first, "RNAs"))
                m_ShowRNAs = NStr::StringToBool(iter->second);
            else if (NStr::EqualNocase(iter->first, "CDSs"))
                m_ShowCDSs = NStr::StringToBool(iter->second);
            else if (NStr::EqualNocase(iter->first, "Exons"))
                m_ShowExons = NStr::StringToBool(iter->second);
            else if (NStr::EqualNocase(iter->first, "VDJCs"))
                m_ShowVDJCs = NStr::StringToBool(iter->second);
            else if (NStr::EqualNocase(iter->first, "CDSProductFeats"))
                m_ShowCDSProductFeats = NStr::StringToBool(iter->second);
            else if (NStr::EqualNocase(iter->first, "NtRuler"))
                //    m_ShowNtRuler = NStr::StringToBool(iter->second);
                ;  // suppress the warning
            else if (NStr::EqualNocase(iter->first, "AaRuler"))
                ///    m_ShowAaRuler = NStr::StringToBool(iter->second);
                ;  // suppress the warning
            else if (NStr::EqualNocase(iter->first, "Histogram"))
                m_ShowHistogram = NStr::StringToBool(iter->second);
            else if (NStr::EqualNocase(iter->first, "HighlightMode"))
                m_HighlightMode = NStr::StringToInt(iter->second);
            else if (NStr::EqualNocase(iter->first, "ShowLabel"))
                m_ShowLabel = NStr::StringToBool(iter->second);
            else if (NStr::EqualNocase(iter->first, "Options")) {
                ERenderingOption option = OptionStrToValue(iter->second);
                switch (option) {
                    case eOption_GeneOnly:
                        m_ShowGenes = true;
                        m_ShowRNAs = false;
                        m_ShowCDSs = false;
                        m_ShowExons = false;
                        m_ShowVDJCs = false;
                        break;
                    case eOption_MergeAll:
                        m_ShowGenes = false;
                        m_ShowRNAs = true;
                        m_ShowCDSs = true;
                        m_ShowExons = true;
                        m_ShowVDJCs = true;
                        m_MergeStyle = eMerge_All;
                        break;
                    case eOption_MergePairs:
                        m_ShowGenes = false;
                        m_ShowRNAs = true;
                        m_ShowCDSs = true;
                        m_ShowExons = true;
                        m_ShowVDJCs = true;
                        m_MergeStyle = eMerge_Pairs;
                        break;
                    case eOption_ShowAllButGenes:
                        m_ShowGenes = false;
                        m_ShowRNAs = true;
                        m_ShowCDSs = true;
                        m_ShowExons = true;
                        m_ShowVDJCs = true;
                        m_MergeStyle = eMerge_No;
                        break;
                    case eOption_ShowAll:
                        m_ShowGenes = true;
                        m_ShowRNAs = true;
                        m_ShowCDSs = true;
                        m_ShowExons = true;
                        m_ShowVDJCs = true;
                        m_MergeStyle = eMerge_No;
                        break;
                    case eOption_SingleLine:
                        m_ShowGenes = false;
                        m_ShowRNAs = true;
                        m_ShowCDSs = true;
                        m_ShowExons = true;
                        m_ShowVDJCs = true;
                        m_MergeStyle = eMerge_OneLine;
                        break;
                    default:
                        break;
                }
            } else
                LOG_POST(Warning << "CGeneModelConfig::UpdateSettings() "
                         << "the setting is not supported: "
                         << iter->first << ":" << iter->second);
        } catch (CException&) {
            LOG_POST(Warning << "CGeneModelConfig::UpdateSettings() invalid setting - " 
                     << iter->first << ":" << iter->second);
        }
    }
}


string CGeneModelConfig::SaveSettingsAsString(const string& preset_style) const
{
    CSGConfigUtils::TKeyValuePairs settings;
    
    if ( !preset_style.empty() ) {
        settings["profile"] = preset_style;
    }
    
    settings["Merge"] = StyleValueToStr(m_MergeStyle);
    settings["LandmarkGeneLabel"] = NStr::BoolToString(m_ShowLMGeneLabel);
    settings["Genes"] = NStr::BoolToString(m_ShowGenes);
    settings["RNAs"] = NStr::BoolToString(m_ShowRNAs);
    settings["CDSs"] = NStr::BoolToString(m_ShowCDSs);
    settings["Exons"] = NStr::BoolToString(m_ShowExons);
    settings["VDJCs"] = NStr::BoolToString(m_ShowVDJCs);
    settings["CDSProductFeats"] = NStr::BoolToString(m_ShowCDSProductFeats);
    settings["NtRuler"] = NStr::BoolToString(m_ShowNtRuler);
    settings["AaRuler"] = NStr::BoolToString(m_ShowAaRuler);

    settings["Histogram"] = NStr::BoolToString(m_ShowHistogram);
    
    return CSGConfigUtils::ComposeProfileString(settings);
}


END_NCBI_SCOPE
