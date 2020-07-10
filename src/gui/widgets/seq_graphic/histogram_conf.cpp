/*  $Id: histogram_conf.cpp 41963 2018-11-21 19:31:19Z shkeda $
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
 */

/// @file
///

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/histogram_conf.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <gui/widgets/seq_graphic/layout_conf.hpp>
#include <util/static_map.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const string kHistParamsKey("GBPlugins.SeqGraphicHistogram");
static const string kDefHistKey("Default");

///////////////////////////////////////////////////////////////////////////////
/// CHistParams
///////////////////////////////////////////////////////////////////////////////

typedef SStaticPair<const char*, CHistParams::EType> TTypeStr;
static const TTypeStr s_TypeStrs[] = {
    { "histogram",   CHistParams::eHistogram },
    { "line graph",  CHistParams::eLineGraph },
    { "merged bar",  CHistParams::eMergedBar },
    { "smear bar",   CHistParams::eSmearBar },
};

typedef CStaticArrayMap<string, CHistParams::EType> TTypeMap;
DEFINE_STATIC_ARRAY_MAP(TTypeMap, sm_TypeMap, s_TypeStrs);

CHistParams::EType CHistParams::TypeStrToValue(const string& type)
{
    TTypeMap::const_iterator iter = sm_TypeMap.find(type);
    if (iter != sm_TypeMap.end()) {
        return iter->second;
    }
    NCBI_THROW(CException, eInvalid, "Invalid type string: " + type);
}

const string& CHistParams::TypeValueToStr(CHistParams::EType type)
{
    TTypeMap::const_iterator iter;
    for (iter = sm_TypeMap.begin();  iter != sm_TypeMap.end();  ++iter) {
        if (iter->second == type) {
            return iter->first;
        }
    }
    return kEmptyStr;
}

typedef SStaticPair<const char*, CHistParams::EScale> TScaleStr;
static const TScaleStr s_ScaleStrs[] = {
    { "linear",   CHistParams::eLinear },
    { "log10",  CHistParams::eLog10 },
    { "log2",  CHistParams::eLog2 },
    { "loge",  CHistParams::eLoge }
};

static const map<CHistParams::EScale, string> sm_ScaleNames = {
    { CHistParams::eLinear, "linear" },
    { CHistParams::eLog10, "log 10"},
    { CHistParams::eLoge, "ln"},
    { CHistParams::eLog2, "log 2" }
};

typedef CStaticArrayMap<string, CHistParams::EScale> TScaleMap;
DEFINE_STATIC_ARRAY_MAP(TScaleMap, sm_ScaleMap, s_ScaleStrs);

CHistParams::EScale CHistParams::ScaleStrToValue(const string& scale)
{
    TScaleMap::const_iterator iter = sm_ScaleMap.find(scale);
    if (iter != sm_ScaleMap.end()) {
        return iter->second;
    }
    // in case it was blank
    else {
        return CHistParams::eLinear;
    }
}

const string& CHistParams::ScaleValueToStr(CHistParams::EScale scale)
{
    TScaleMap::const_iterator iter;
    for (iter = sm_ScaleMap.begin();  iter != sm_ScaleMap.end();  ++iter) {
        if (iter->second == scale) {
            return iter->first;
        }
    }
    return kEmptyStr;
}
const string& CHistParams::ScaleValueToName(CHistParams::EScale scale)
{
    for (auto iter = sm_ScaleNames.begin(); iter != sm_ScaleNames.end(); ++iter) {
        if (iter->first == scale) {
            return iter->second;
        }
    }
    return kEmptyStr;
}

string CHistParams::ScaleTMSToStr(const string& sScaleTMS)
{
    // TMS does not currently support anything but log2 scaling, and the absence of setting is
    // considered to be "linear" (SV-1516)
    // other cases added (SV-2703)
    if(sScaleTMS == "log2 scaled") {
        return "log2";
    } else if(sScaleTMS == "linear scaled") {
        return "linear";
    } else if(sScaleTMS == "loge scaled") {
        return "loge";
    } else if(sScaleTMS == "log10 scaled") {
        return "log10";
    }
    return "linear";
}



///////////////////////////////////////////////////////////////////////////////
/// CHistParamsManager implementation
///////////////////////////////////////////////////////////////////////////////

void CHistParamsManager::LoadSettings(const string& curr_color,
                                       const string& curr_size)
{
    m_HistSettings.clear();
    const CGuiRegistry& registry = CGuiRegistry::GetInstance();

    CRegistryReadView view = registry.GetReadView(kHistParamsKey);
    CRegistryReadView::TKeys keys;
    view.GetTopKeys(keys);
    ITERATE(CRegistryReadView::TKeys, iter, keys) {
        string base_key = kHistParamsKey + "." + iter->key;
        CRegistryReadView sub_view =
            CSGConfigUtils::GetReadView(registry, kHistParamsKey, iter->key, kDefHistKey);

        CRef<CHistParams> params(new CHistParams);
        params->m_Type = (CHistParams::EType)CHistParams::TypeStrToValue(
            sub_view.GetString("Type"));
        params->m_Scale = (CHistParams::EScale)CHistParams::ScaleStrToValue(
            sub_view.GetString("Scale"));
        params->m_DrawBg = sub_view.GetBool("DrawBg", false);
        params->m_NeedRuler = sub_view.GetBool("NeedRuler", true);
        params->m_ClipOutliers = sub_view.GetBool("clip", false);
        params->m_SDeviationThreshold = sub_view.GetInt("SDeviationThreshold", 5);

        // loading color settings
        sub_view = CSGConfigUtils::GetColorReadView(
            registry, kHistParamsKey, iter->key, curr_color, kDefHistKey);
        CSGConfigUtils::GetColor(sub_view, "FG", params->m_fgColor);
        params->m_fgNegColor = params->m_fgColor.RotateColor(params->m_fgColor, -90); // Set default
        CSGConfigUtils::GetColor(sub_view, "FG_neg", params->m_fgNegColor);
        CSGConfigUtils::GetColor(sub_view, "BG", params->m_bgColor);
        CSGConfigUtils::GetColor(sub_view, "SmearColorMin", params->m_SmearColorMin);
        CSGConfigUtils::GetColor(sub_view, "SmearColorMax", params->m_SmearColorMax);
        CSGConfigUtils::GetColor(sub_view, "Label", params->m_LabelColor);
        CSGConfigUtils::GetColor(sub_view, "RulerColor", params->m_RulerColor);
        CSGConfigUtils::GetColor(sub_view, "ocolor", params->m_OutlierColor);

        // loading color set
        string sub_key = base_key + "." + CSGConfigUtils::ColorKey() +
            "." + curr_color + ".ColorSet";
        sub_view = registry.GetReadView(sub_key);
        CRegistryReadView::TKeys color_keys;
        sub_view.GetKeys(color_keys);
        ITERATE(CRegistryReadView::TKeys, citer, color_keys) {
            CRgbaColor color;
            CSGConfigUtils::GetColor(sub_view, citer->key, color);
            params->m_Colors[citer->key] = color;
        }

        // loading size settings
        sub_view = CSGConfigUtils::GetSizeReadView(
            registry, kHistParamsKey, iter->key, curr_size, kDefHistKey);
        params->m_Height = sub_view.GetReal("Height", 10.0);

        m_HistSettings[iter->key] = params;
    }
}


void CHistParamsManager::SaveSettings(const string& curr_color,
                                      const string& curr_size) const
{
    CGuiRegistry& registry = CGuiRegistry::GetInstance();

    ITERATE (THistParams, iter, m_HistSettings) {
        CRef<CHistParams> params = iter->second;
        if ( !params->GetDirty() ) continue;

        string base_key = kHistParamsKey + "." + iter->first;
        CRegistryWriteView view =
            CSGConfigUtils::GetWriteView(registry, kHistParamsKey, iter->first, kDefHistKey);
        view.Set("Type", CHistParams::TypeValueToStr(params->m_Type));
        view.Set("Scale", CHistParams::ScaleValueToStr(params->m_Scale));
        view.Set("NeedRuler", params->m_NeedRuler);
        view.Set("DrawBg", params->m_DrawBg);
        view.Set("clip", params->m_ClipOutliers);
        view.Set("SDeviationThreshold", params->m_SDeviationThreshold);

        // save color settings
        view = CSGConfigUtils::GetColorRWView(
            registry, kHistParamsKey, iter->first, curr_color, kDefHistKey);
        CSGConfigUtils::SetColor(view, "BG", params->m_bgColor);
        CSGConfigUtils::SetColor(view, "FG", params->m_fgColor);
        CSGConfigUtils::SetColor(view, "FG_neg", params->m_fgNegColor);
        CSGConfigUtils::SetColor(view, "SmearColorMin", params->m_SmearColorMin);
        CSGConfigUtils::SetColor(view, "SmearColorMax", params->m_SmearColorMax);
        CSGConfigUtils::SetColor(view, "Label", params->m_LabelColor);
        CSGConfigUtils::SetColor(view, "RulerColor", params->m_RulerColor);
        CSGConfigUtils::SetColor(view, "ocolor", params->m_OutlierColor);

        string key = base_key + "." + CSGConfigUtils::ColorKey() + "." +
            curr_color + "." + "ColorSet";
        view = registry.GetWriteView(key);
        ITERATE (CHistParams::TColorSet, citer, params->m_Colors) {
            CSGConfigUtils::SetColor(view, citer->first, citer->second);
        }

        // save size settings
        view = CSGConfigUtils::GetSizeRWView(
            registry, kHistParamsKey, iter->first, curr_size, kDefHistKey);
        view.Set("Height", params->m_Height);
    }
}


CRef<CHistParams>
CHistParamsManager::GetDefHistParams() const
{
    return m_HistSettings.find(kDefHistKey)->second;
}


CRef<CHistParams>
CHistParamsManager::GetHistParams(TFeatSubtype subtype) const
{
    if (subtype != CSeqFeatData::eSubtype_any) {
        const CFeatList& feats(*CSeqFeatData::GetFeatList());
        vector<string> feat_hierarchy = feats.GetStoragekeys(subtype);

        // try them in reverse order so the more specific keys get tried first.
        vector<string>::reverse_iterator riter(feat_hierarchy.end());
        vector<string>::reverse_iterator rend(feat_hierarchy.begin());

        for ( ; riter != rend;  ++riter) {
            THistParams::const_iterator iter = m_HistSettings.find(*riter);
            if ( iter != m_HistSettings.end()) {
                return iter->second;
            }
        }
        // shouldn't get here.
        // means that there wasn't a Master feature item in the global settings.
        _ASSERT(false);

    }
    return CRef<CHistParams>(NULL);
}


CRef<CHistParams>
CHistParamsManager::GetHistParams(const string& name) const
{
    string new_name =  NStr::Replace(name, ".", "_");
    THistParams::const_iterator iter = m_HistSettings.find(new_name);
    if (iter != m_HistSettings.end()) {
        return iter->second;
    } else {
        iter = m_TempHistSettings.find(new_name);
        if (iter != m_TempHistSettings.end()) {
            return iter->second;
        }
    }

    iter = m_HistSettings.find(kDefHistKey);
    return iter->second;
}


bool CHistParamsManager::IsTempSettings(const string& name) const
{
    string new_name =  NStr::Replace(name, ".", "_");
    THistParams::const_iterator iter = m_TempHistSettings.find(new_name);
    if (iter != m_TempHistSettings.end()) {
        return true;
    }

    return false;
}


bool CHistParamsManager::HasSettings(const string& name) const
{
    string new_name =  NStr::Replace(name, ".", "_");
    THistParams::const_iterator iter = m_HistSettings.find(new_name);
    if (iter != m_HistSettings.end()) {
        return true;
    } else {
        iter = m_TempHistSettings.find(new_name);
        if (iter != m_TempHistSettings.end()) {
            return true;
        }
    }
    return false;
}


bool CHistParamsManager::HasSettings(TFeatSubtype subtype) const
{
    const CFeatList& feats(*CSeqFeatData::GetFeatList());
    vector<string> feat_hierarchy = feats.GetStoragekeys(subtype);
    if ( !feat_hierarchy.empty()  &&
        m_HistSettings.find(feat_hierarchy.back()) != m_HistSettings.end()) {
            return true;
    }

    return false;
}


void CHistParamsManager::x_AddSettings(const string& name,
                                       CRef<CHistParams> hist_params,
                                       THistParams& settings)
{
    // The reason we need to replace '.' with '_' is because when saving
    // the setting to a .ini file, '.' conflicts with delimeter '.'
    // Hence any methods access the histogram setting will also need to do
    // the similar replacement before retrieving the settings when using a name.
    string new_name =  NStr::Replace(name, ".", "_");
    THistParams::iterator iter = settings.find(new_name);
    if (iter != settings.end()) {
        *iter->second = *hist_params;
    } else {
        settings[new_name] = hist_params;
    }
}

CRef<CChoice> CHistParams::CreateScaleOptions(const string& option_name, CHistParams::EScale option_value)
{
    auto choice = CTrackConfigUtils::CreateChoice
        (option_name, "Linear/Log Scale",
         CHistParams::ScaleValueToStr(option_value),
         "Scale for graph data");
    choice->SetValues()
        .push_back(CTrackConfigUtils::CreateChoiceItem
                   (CHistParams::ScaleValueToStr(CHistParams::eLinear),
                   CHistParams::ScaleValueToName(CHistParams::eLinear),
                   "Shown at linear scale",
                   "Graph data is shown at linear scale"));
    choice->SetValues()
        .push_back(CTrackConfigUtils::CreateChoiceItem
                   (CHistParams::ScaleValueToStr(CHistParams::eLog10),
                   CHistParams::ScaleValueToName(CHistParams::eLog10),
                   "Shown at log base 10 scale",
                   "Graph data is shown at logarithmic (base 10) scale"));
    choice->SetValues()
        .push_back(CTrackConfigUtils::CreateChoiceItem
                   (CHistParams::ScaleValueToStr(CHistParams::eLoge),
                   CHistParams::ScaleValueToName(CHistParams::eLoge),
                   "Shown at natural logarithm (base e) scale",
                   "Graph data is shown at natural logrithm (base e) scale"));
    choice->SetValues()
        .push_back(CTrackConfigUtils::CreateChoiceItem
                   (CHistParams::ScaleValueToStr(CHistParams::eLog2),
                   CHistParams::ScaleValueToName(CHistParams::eLog2),
                   "Shown at log base 2 scale",
                   "Graph data is shown at logarithmic (base 2) scale"));
    return choice;
}

END_NCBI_SCOPE
