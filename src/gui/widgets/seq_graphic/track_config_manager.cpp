/*  $Id: track_config_manager.cpp 44576 2020-01-23 23:22:13Z rudnev $
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
 * Authors:  Liangshou Wu, Victor Joukov
 *
 */


#include <ncbi_pch.hpp>

#include <gui/objects/HiddenSetting.hpp>
#include <gui/objects/TextBox.hpp>

#include <gui/widgets/seq_graphic/track_config_manager.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_conf.hpp>

#include <gui/widgets/seq_graphic/config_utils.hpp>
//#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>

#include <gui/widgets/seq_graphic/sequence_track.hpp>
#include <gui/widgets/seq_graphic/gene_model_track.hpp>
#include <gui/widgets/seq_graphic/feature_track.hpp>
#include <gui/widgets/seq_graphic/segment_map_track.hpp>
#include <gui/widgets/seq_graphic/scaffold_track.hpp>
#include <gui/widgets/seq_graphic/component_track.hpp>
#include <gui/widgets/seq_graphic/alignment_track.hpp>
#include <gui/widgets/seq_graphic/all_other_features_track.hpp>
#include <gui/widgets/seq_graphic/six_frames_trans_track.hpp>
#include <gui/widgets/seq_graphic/graph_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_graph_ds.hpp>
#include <gui/widgets/seq_graphic/epigenomics_track.hpp>
#include <gui/widgets/seq_graphic/dbvar_track.hpp>
#include <gui/widgets/seq_graphic/track_container_track.hpp>
#include <gui/widgets/seq_graphic/graph_overlay.hpp>
#include <gui/widgets/seq_graphic/seqtable_graph_track.hpp>
#include <gui/widgets/seq_graphic/trace_graph_ds.hpp>
#include <gui/widgets/seq_graphic/trace_graph_track.hpp>
#include <gui/widgets/seq_graphic/aggregate_feature_track.hpp>

#include <gui/utils/extension_impl.hpp>
#include <gui/utils/track_info.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/na_utils.hpp>

#include <limits.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

#define MAX_LEVEL numeric_limits<int>::max()


CTrackConfigManager::CTrackConfigManager()
{
    x_InitTrackManager();
    m_KnownAnnots[MAX_LEVEL] = TAnnotMetaDataList();
}


void CTrackConfigManager::RegisterSGTracks()
{
    // register layout track factories
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
        new CSequenceTrackFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
        new CGeneModelFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
        new CFeatureTrackFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
        new CSegmentMapTrackFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
        new CScaffoldTrackFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
        new CComponentTrackFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
        new CAlignmentTrackFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
        new CGraphTrackFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
        new CTrackContainerFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
        new CGraphOverlayFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
        new CTraceGraphTrackFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
        new CAllOtherFeaturesTrackFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
        new CSixFramesTransTrackFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
        new CEpigenomicsTrackFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
        new CVarTrackFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
        new CSeqTableGraphTrackFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
        new CAggregateFeatureTrackFactory());

    // register data source extensions
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_DATA_SOURCE_TYPE,
        new CSGSequenceDSType());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_DATA_SOURCE_TYPE,
        new CSGSegmentMapDSType());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_DATA_SOURCE_TYPE,
        new CSGFeatureDSType());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_DATA_SOURCE_TYPE,
        new CSGAlignmentDSType());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_DATA_SOURCE_TYPE,
        new CSGGraphDSType());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_DATA_SOURCE_TYPE,
       new CSGTraceGraphDSType());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_DATA_SOURCE_TYPE,
        new CFeaturePanelDSType());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_DATA_SOURCE_TYPE,
        new CSFTransDSType());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_DATA_SOURCE_TYPE,
        new CEpigenomicsDSType());
}


const ILayoutTrackFactory*
CTrackConfigManager::GetTrackFactory(const string& key) const
{
    // dump all registered factories
    /*
    string sFactoriesList;
    for(const auto& factory: m_Factories) {
        sFactoriesList += ", " + factory.first;
    }
    cerr << sFactoriesList << endl;
    */
//    LOG_POST(Trace << "<<<< looking for factory for key: " << key);
    TFactoryMap::const_iterator f_iter = m_Factories.find(key);
    if (f_iter != m_Factories.end()) {
//        LOG_POST(Trace << ">>>> found at address: " << f_iter->second << ", factory class name: " << typeid(*f_iter->second).name());
        return f_iter->second;
    }
//    LOG_POST(Trace << ">>>> not found");
    return NULL;
}


list<string>
CTrackConfigManager::GetTrackTypes() const
{
    list<string> ttypes;
    ITERATE(TFactoryMap, it, m_Factories) {
        ttypes.push_back(it->first);
    }
    return ttypes;
}


void
CTrackConfigManager::x_AppendKeyDisplayOptions(
    const string& key,
    CDisplayOptionsBundle& bundle) const
{

    CDisplayOptionsBundle::TOptions& bundle_options = bundle.SetOptions();
    CDisplayOptionsBundle::TTracks& bundle_tracks = bundle.SetTracks();

    TFactoryMap::const_iterator f_iter = m_Factories.find(key);
    if (f_iter == m_Factories.end()) return;

    const ITrackConfigurable* factory =
        dynamic_cast<const ITrackConfigurable*>(f_iter->second.GetPointer());
    if (factory) {
        CRef<CTrackConfigSet> configs;
        TKeyValuePairs settings;
        configs = factory->GetSettings("Default", settings, NULL);
        ITERATE(CTrackConfigSet::Tdata, it, configs->Get()) {
            CRef<CTrackConfig> config = *it;

            CRef<CDisplayTrack> track(new CDisplayTrack);
            track->SetKey(key);
            CDisplayTrack::TOptions& options = track->SetOptions();

            if (config->IsSetSubkey()) {
                track->SetSubkey(config->GetSubkey());
            }
            if (config->IsSetHelp()) {
                track->SetHelp(config->GetHelp());
            }
            if (config->IsSetLegend_text()) {
                track->SetLegend_text(config->GetLegend_text());
            }
            if (config->IsSetChoice_list()) {
                ITERATE(CTrackConfig::TChoice_list, it, config->GetChoice_list()) {
                    const CChoice& choice = **it;
                    CRef<CDisplayOption> option(new CDisplayOption);
                    option->SetType("enum");
                    option->SetKey(choice.GetName());
                    // label
                    if (choice.IsSetDisplay_name()) {
                        option->SetLabel(choice.GetDisplay_name());
                    }
                    // help
                    if (choice.IsSetHelp()) {
                        option->SetHelp(choice.GetHelp());
                    }
                    CDisplayOption::TValues& values = option->SetValues();
                    ITERATE(CChoice::TValues, it1, choice.GetValues()) {
                        const CChoiceItem& item = **it1;
                        CRef<CDisplayOptionValue> value(new CDisplayOptionValue);
                        value->SetKey(item.GetName());
                        if (item.IsSetDisplay_name()) {
                            value->SetLabel(item.GetDisplay_name());
                        }
                        if (item.IsSetHelp()) {
                            value->SetHelp(item.GetHelp());
                        }
                        if (item.IsSetLegend_text()
                            && !item.GetLegend_text().empty()) {
                            value->SetLegend(item.GetLegend_text());
                        }
                        values.push_back(value);
                    }
                    option->SetDflt(choice.GetCurr_value());
                    int optnum = bundle_options.size();
                    options.push_back(optnum);
                    bundle_options.push_back(option);
                }
            }
            if (config->IsSetCheck_boxes()) {
                ITERATE(CTrackConfig::TCheck_boxes, it, config->GetCheck_boxes()) {
                    const CCheckBox& check_box = **it;
                    CRef<CDisplayOption> option(new CDisplayOption);
                    option->SetType("bool");
                    option->SetKey(check_box.GetName());
                    // label
                    if (check_box.IsSetDisplay_name()) {
                        option->SetLabel(check_box.GetDisplay_name());
                    }
                    // help
                    if (check_box.IsSetHelp()) {
                        option->SetHelp(check_box.GetHelp());
                    }
                    // legend ?? add to DisplayOption
                    option->SetDflt(check_box.GetValue() ? "true" : "false");
                    int optnum = bundle_options.size();
                    options.push_back(optnum);
                    bundle_options.push_back(option);
                }
            }
            if (config->IsSetText_boxes()) {
                ITERATE(CTrackConfig::TText_boxes, it, config->GetText_boxes()) {
                    const CTextBox& text_box = **it;
                    CRef<CDisplayOption> option(new CDisplayOption);
                    option->SetType("text");
                    option->SetKey(text_box.GetName());
                    // label
                    if (text_box.IsSetDisplay_name()) {
                        option->SetLabel(text_box.GetDisplay_name());
                    }
                    // help
                    if (text_box.IsSetHelp()) {
                        option->SetHelp(text_box.GetHelp());
                    }
                    // legend ?? add to DisplayOption
                    option->SetDflt(text_box.GetValue());
                    int optnum = bundle_options.size();
                    options.push_back(optnum);
                    bundle_options.push_back(option);
                }
            }
            if (config->IsSetHidden_settings()) {
                ITERATE(CTrackConfig::THidden_settings, it, config->GetHidden_settings()) {
                    const CHiddenSetting& hs = **it;
                    CRef<CDisplayOption> option(new CDisplayOption);
                    option->SetType("hidden");
                    option->SetKey(hs.GetName());
                    option->SetDflt(hs.GetValue());
                    int optnum = bundle_options.size();
                    options.push_back(optnum);
                    bundle_options.push_back(option);
                }
            }
            bundle_tracks.push_back(track);
        }
    }
}


CConstRef<CDisplayOptionsBundle>
CTrackConfigManager::GetTrackDisplayOptions(const list<string>& keys) const
{
    CRef<CDisplayOptionsBundle> bundle(new CDisplayOptionsBundle);
    ITERATE(list<string>, it, keys) {
        x_AppendKeyDisplayOptions(*it, *bundle);
    }
    return bundle;
}


const CTrackConfigManager::TAnnotMetaDataList&
CTrackConfigManager::GetKnownAnnots(int level) const
{
    if (level == -1) level = 0;
    TKnownAnnotMap::const_iterator iter = m_KnownAnnots.find(level);
    if (iter == m_KnownAnnots.end()) {
        level = MAX_LEVEL;
        iter = m_KnownAnnots.find(level);
        _ASSERT(iter != m_KnownAnnots.end());
    }

    return iter->second;
}


bool CTrackConfigManager::OnShownList(const CTempTrackProxy* p_proxy,
                                      const CTempTrackProxy* t_proxy,
                                      const string& track_name) const
{
    bool is_naa = CSeqUtils::IsNAA(track_name);

    if ( p_proxy->GetShown() ) {
        if (is_naa) {
            const CTrackProxy* temp_proxy =
                dynamic_cast<const CTrackProxy*>(p_proxy);
            if (temp_proxy) {
                const CTrackProxy::TAnnots& annots = temp_proxy->GetAnnots();
                if (find(annots.begin(), annots.end(), track_name) != annots.end()  ||
                    (!annots.empty()  &&  annots.front() == "NA*")) {
                    return true;
                }
            }
        } else {
            if (nullptr != t_proxy)
                return t_proxy->GetShown();
            return true;
        }
    }

    return false;
}


void CTrackConfigManager::AdjustTrackOrder(CTempTrackProxy* t_proxy)
{
    // sort the tracks according to the order of both
    // annotation list in the track proxy and NAAs.
    // if there is any conflict, the former order is prefered.
    // we only sort the tracks under some special cases,
    // mainly for cgi mode, to avoid frustrating users.
    // annot list contain more than one annot and at least
    // one of them is NAA, or NAAs is not empty.
    CTrackContainer* con_track =
        dynamic_cast<CTrackContainer*>(t_proxy->GetTrack());
    if ( !con_track  || con_track->GetSubtrackProxies().size() < 2 ) {
        return; // no need to sort
    }

    typedef pair<string, bool> TAnnotStatus;
    list<TAnnotStatus> all_tracks;

    // honor the track order in track's annot list over m_NAAs list
    const CTrackProxy* full_track = dynamic_cast<const CTrackProxy*>(t_proxy);
    if (full_track) {
        ITERATE (CTrackProxy::TAnnots, iter, full_track->GetAnnots()) {
            string name = *iter;
            CSeqUtils::TAnnotNameType type =
                CSeqUtils::NameTypeStrToValue(name);
            if (type == CSeqUtils::eAnnot_Other  ||
                type == CSeqUtils::eAnnot_Unnamed) {
                NStr::ReplaceInPlace(name, ".", "_");
                all_tracks.push_back(TAnnotStatus(name, false));
            }
        }
    }

    if ( !all_tracks.empty() ) {
        // If no specific order has been specified for Unnamed annotatioin,
        // we always put it as the first track
        list<TAnnotStatus>::iterator t_iter = all_tracks.begin();
        while (t_iter != all_tracks.end()  &&  !CSeqUtils::IsUnnamed(t_iter->first)) {
            ++t_iter;
        }
        if (t_iter == all_tracks.end()) {
            all_tracks.push_front(TAnnotStatus(CSeqUtils::GetUnnamedAnnot(), false));
        }

        vector<int> old_orders;

        // mark the tracks that are in track container and save there old orders
        ITERATE (TTrackProxies, iter, con_track->GetSubtrackProxies()) {
            const string& name = (*iter)->GetName();
            NON_CONST_ITERATE (list<TAnnotStatus>, tt_iter, all_tracks) {
                if (tt_iter->first == name) {
                    old_orders.push_back((*iter)->GetOrder());
                    tt_iter->second = true;
                    break;
                }
            }
        }

        // remove the tracks that are not in track container from all_tracks
        t_iter = all_tracks.begin();
        while (t_iter != all_tracks.end()) {
            if ( !t_iter->second ) {
                t_iter = all_tracks.erase(t_iter);
            } else {
                ++t_iter;
            }
        }

        if (all_tracks.size() > old_orders.size()) {
            all_tracks.pop_front();
        }

        NON_CONST_ITERATE (TTrackProxies, iter, con_track->GetSubtrackProxies()) {
            int idx = 0;
            t_iter = all_tracks.begin();
            while (t_iter != all_tracks.end()  &&
                t_iter->first != (*iter)->GetName()) {
                    ++idx;
                    ++t_iter;
            }
            if (t_iter != all_tracks.end()) {
                (*iter)->SetOrder(old_orders[idx]);
            }
        }
        con_track->GetSubtrackProxies().sort(CTrackProxy::STPSorterByOrder::s_CompareCRefs);
        con_track->SortChildTracksByOrder();
    }
}


/// help struct for storing track settings
struct STrackSettings
{
    typedef vector<STrackSettings> TTrackConfig;
    typedef CSGConfigUtils::TKeyValuePairs   TProfile;
    typedef vector<string> TSubTracks;

    static void ParseSettings(
        const CTrackConfigManager::TTrackSettingsSet& settings_set,
        TTrackConfig& config)
        {
            ITERATE (CTrackConfigManager::TTrackSettingsSet, iter, settings_set) {
                const CTrackConfigManager::TTrackSettings& track = *iter;
                CTrackConfigManager::TTrackSettings::const_iterator s_iter = track.find("key");
                if (s_iter == iter->end()) continue;

                STrackSettings settings;
                s_iter = track.begin();
                while (s_iter != track.end()) {
                    const string& k_str = s_iter->first;
                    if (NStr::EqualNocase(k_str, "key")) {
                        settings.m_Key = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "original_key")) {
                        settings.m_OriginalKey = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "subkey")) {
                        settings.m_Subkey = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "name")) {
                        settings.m_Name = CTrackUtils::RemoveEscape(s_iter->second);
                    } else if (NStr::EqualNocase(k_str, "id")) {
                        settings.m_Id = CTrackUtils::RemoveEscape(s_iter->second);
                    } else if (NStr::EqualNocase(k_str, "dbname")) {
                        settings.m_DB = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "data_key")) {
                        settings.m_DataKey = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "order")) {
                        try {
                            settings.m_Order = NStr::StringToInt(s_iter->second);
                        } catch (CException&) {
                            // ignore
                        }
                    } else if (NStr::EqualNocase(k_str, "shown")) {
                        try {
                            settings.m_Shown = NStr::StringToBool(s_iter->second);
                        } catch (CException&) {
                            // ignore
                        }
                    } else if (NStr::EqualNocase(k_str, "category")) {
                        settings.m_Category = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "subcategory")) {
                        settings.m_Subcategory = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "setting_group")) {
                        vector<string> tokens;
                        CTrackUtils::TokenizeWithEscape(s_iter->second, "|", tokens, true);
                        if (tokens.size() > 0) {
                            settings.m_SettingGroup = NStr::TruncateSpaces(tokens[0]);
                            if (tokens.size() > 1) {
                                try {
                                    settings.m_UseGroupDefault =
                                        NStr::StringToBool(NStr::TruncateSpaces(tokens[1]));
                                } catch (CException&) {
                                    // ignore it
                                }
                            }
                        }
                    } else if (NStr::EqualNocase(k_str, "filter")) {
                        settings.m_Filter = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "sort_by")) {
                        settings.m_SortBy = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "display_name")) {
                        settings.m_DispName = CTrackUtils::RemoveEscape(s_iter->second);
                    } else if (NStr::EqualNocase(k_str, "annots")) {
                        CTrackUtils::TokenizeWithEscape(s_iter->second, "|", settings.m_Annots, true);
                    } else if (NStr::EqualNocase(k_str, "comments")) {
                        settings.m_Comments = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "highlights")) {
                        settings.m_Highlights = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "highlights_color")) {
                        settings.m_HighlightsColor = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "help")) {
                        settings.m_Help = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "show_title")) {
                        try {
                            settings.m_ShowTitle = NStr::StringToBool(s_iter->second);
                        } catch (CException&) {
                            // ignore
                        }
                    } else if (NStr::EqualNocase(k_str, "is_private")) {
                        try {
                            settings.m_isPrivate = NStr::StringToBool(s_iter->second);
                        } catch (CException&) {
                            // ignore
                        }
                    } else if (NStr::EqualNocase(k_str, "remote_data")) {
                        settings.m_RemoteData = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "seq_data_key")) {
                        settings.m_SeqDataKey = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "stored_scale") || NStr::EqualNocase(k_str, "is_scaled")) {
                        settings.m_StoredScale = s_iter->second;
                        settings.m_Profile["stored_scale"] = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "subtracks")) {
                        CTrackUtils::TokenizeWithEscape(s_iter->second, "|", settings.m_SubTracks, true);
                    } else if (NStr::EqualNocase(k_str, "uid")) {
                        settings.m_UId = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "batch")) {
                        settings.m_Batch = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "is_source_assembly_query")) {
                        settings.m_sExtraInfo += string(settings.m_sExtraInfo.empty() ? "" : ";") + k_str + string("=") + s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "remote_path")) {
                        settings.m_RemotePath = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "rmt_mapped_id")) {
                        settings.m_RemoteId = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "big_data_index")) {
                        settings.m_BigDataIndex = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, "hub_id")) {
                        settings.m_HubId = s_iter->second;
                    }
                    else if (NStr::EqualNocase(k_str, CTrackUtils::cTrackSettingKey_ErrorSeverity)) {
                        try {
                            settings.m_ErrorStatus.m_ErrorSeverity = NStr::StringToInt(s_iter->second);
                        } catch (CException&) { /* ignore*/ }
                    } else if (NStr::EqualNocase(k_str, CTrackUtils::cTrackSettingKey_ErrorCode)) {
                        try {
                            settings.m_ErrorStatus.m_ErrorCode = NStr::StringToInt(s_iter->second);
                        } catch (CException&) { /* ignore*/ }
                    } else if (NStr::EqualNocase(k_str, CTrackUtils::cTrackSettingKey_ErrorShortMsg)) {
                        settings.m_ErrorStatus.m_ShortErrorMsg = s_iter->second;
                    } else if (NStr::EqualNocase(k_str, CTrackUtils::cTrackSettingKey_ErrorMsg)) {
                        settings.m_ErrorStatus.m_ErrorMsg = s_iter->second;
                    } else {
                        settings.m_Profile[k_str] = s_iter->second;
                    }
                    ++s_iter;
                }
                config.push_back(settings);
            }
        }

    static string CreateProfileString(const TProfile& profile)
        {
            string profile_str = kEmptyStr;
            bool first = true;
            ITERATE (TProfile, iter, profile) {
                if(NStr::StartsWith(iter->first, CTrackUtils::cTrackSettingKey_PrivatePrefix)) {
                    // skip temporary/internal profile strings
                    continue;
                }
                if (first) first = false;
                else  profile_str += ",";
                profile_str += iter->first + ":" + iter->second;
            }
            return profile_str;
        }


    string m_Key;
    string m_OriginalKey;
    string m_Subkey;
    string m_Name;
    string m_Id;
    string m_DB;
    string m_DataKey;
    string m_Category;
    string m_Subcategory;
    string m_SettingGroup;
    bool   m_UseGroupDefault = false;
    string m_Filter;
    string m_SortBy;
    string m_DispName;
    int    m_Order = -1;
    bool   m_Shown = true;
    string m_Help;

    TProfile m_Profile;
    CTrackProxy::TAnnots m_Annots;
    string m_Comments;
    string m_Highlights;
    string m_HighlightsColor;
    bool   m_ShowTitle = true;
    TTrackErrorStatus m_ErrorStatus;
    bool   m_isPrivate = false;
    string m_StoredScale;
    TSubTracks m_SubTracks;
    string m_UId;
    string m_Batch;
    string m_RemotePath;
    string m_sExtraInfo;
    string m_RemoteData;
    string m_RemoteId;
    string m_SeqDataKey;
    string m_BigDataIndex;
    string m_HubId;
};

/// The matching logic is 'vague' due to the fact that the
/// user-provided 'tracks' may be ambiguous. For example,
/// only the track type (and subtype) is specified. We need to
/// consider several information to decide if two tracks match.
/// @param exact_annot_match if true, the two tracks must show
/// the exactly same set of annotations in order to match. Otherwise,
/// the two tracks will be considered as 'matched' if src_track
/// has no annotatioin specified.
static bool s_TrackMatch(
    const STrackSettings& src_track,
    const CTempTrackProxy& target_track,
    bool exact_annot_match = true)
{
    const CTrackProxy* t_track = dynamic_cast<const CTrackProxy*>(&target_track);
    if ( !t_track ) return false;

    if ( !src_track.m_Id.empty()  &&
         ( !t_track->GetId().empty()  ||  
            !NStr::StartsWith(src_track.m_Id, CTrackUtils::kStdTrackId) ||  
            !NStr::StartsWith(src_track.m_Id, "R" + CTrackUtils::kStdTrackId) )) {
        // if track id is not "[R]STD", it's a TMS based id and is a unique track identifier
        // Ignore the annotation field.
        if (t_track->GetId() != src_track.m_Id)
            return false;

        if (!src_track.m_Filter.empty()) {
            if (t_track->GetFilter() != src_track.m_Filter)
                return false;
        }

        if (!src_track.m_Key.empty()) {
            if (t_track->GetKey() != src_track.m_Key)
                return false;
        }

        return true;
    }

    if ( !src_track.m_Name.empty() ) {
        // Track name used to be a unique identifier before ID was introduced.
        // Ignore the annotation field.
        return src_track.m_Name == t_track->GetName();
    }

    if (src_track.m_Key == t_track->GetKey()  &&  src_track.m_Subkey == t_track->GetSubkey()) {
        if ( !exact_annot_match  &&  src_track.m_Annots.empty()) {
            return true;
        }

        // otherwise the annotations specified have to be completely matched
        if (src_track.m_Annots.size() == t_track->GetAnnots().size()) {
            CTrackProxy::TAnnots annots1 = src_track.m_Annots;
            CTrackProxy::TAnnots annots2 = t_track->GetAnnots();
            std::sort(annots1.begin(), annots1.end());
            std::sort(annots2.begin(), annots2.end());
            return std::equal(annots1.begin(), annots1.end(), annots2.begin());
        }
    }
    return false;
}


/// Helper function to find a matched track from a list of tracks
/// for a given source track. It returns the first that makes
/// the exact match.
static CTrackConfigManager::TTrackProxies::iterator
s_FindMatchedTrack(const STrackSettings& src_track,
                   CTrackConfigManager::TTrackProxies& track_list)
{
    CTrackConfigManager::TTrackProxies::iterator iter = track_list.begin();
    for(; iter != track_list.end(); ++iter) {
        if (s_TrackMatch(src_track, **iter, true)) break;
    }
    return iter;
}



void s_InitTrackFromSettings(
    CTrackProxy& track, const STrackSettings& s,
    int order, bool update_annots = true)
{
    if (!s.m_Name.empty()) {
        track.SetName(s.m_Name);
    }
    // update other feature list
    if (track.GetOrder() < 0  &&  !s.m_Subkey.empty()) {
        track.SetSubkey(s.m_Subkey);
        // assign a name if in case it is not named.
        if (track.GetName().empty()) {
            track.SetName(s.m_Subkey);
        }
    }

    track.SetShown(s.m_Shown);
    if (order >= 0) {
        track.SetOrder(order);
    }
    if (!s.m_Help.empty()) {
        track.SetHelp(s.m_Help);
    }
    track.SetKey(s.m_Key);
    track.SetOriginalKey(s.m_OriginalKey);
    if (!s.m_Profile.empty()) {
        track.SetTrackProfile(STrackSettings::CreateProfileString(s.m_Profile));
    }
    if (update_annots  &&  !s.m_Annots.empty()) {
        track.SetAnnots(s.m_Annots);
    }
    track.SetErrorStatus(s.m_ErrorStatus);
    if (!s.m_Category.empty()) {
        track.SetCategory(s.m_Category);
    }
    if (!s.m_Subcategory.empty()) {
        track.SetSubcategory(s.m_Subcategory);
    }
    if (!s.m_SettingGroup.empty()) {
        track.SetSettingGroup(s.m_SettingGroup);
    }
    track.SetUseGroupDefault(s.m_UseGroupDefault);

    if (!s.m_Filter.empty()) {
        track.SetFilter(s.m_Filter);
    }
    if (!s.m_SortBy.empty()) {
        track.SetSortBy(s.m_SortBy);
    }
    if (!s.m_DispName.empty()) {
        track.SetDisplayName(s.m_DispName);
    }
    if (!s.m_Id.empty()) {
        track.SetId(s.m_Id);
    }
    if (!s.m_DB.empty()) {
        track.SetDB(s.m_DB);
    }
    if (!s.m_DataKey.empty()) {
        track.SetDataKey(s.m_DataKey);
    }

    track.SetComments(s.m_Comments);
    track.SetHighlights(s.m_Highlights);
    track.SetHighlightsColor(s.m_HighlightsColor);
    track.SetShowTitle(s.m_ShowTitle);
    track.SetIsPrivate(s.m_isPrivate);
    track.SetStoredScale(s.m_StoredScale);
    track.SetUId(s.m_UId);
    track.SetSubTracks() = s.m_SubTracks;
    track.SetBatch(s.m_Batch);
    track.SetExtraInfo(s.m_sExtraInfo);
    track.SetRemotePath(s.m_RemotePath);

    if (!s.m_RemoteData.empty()) {
        track.SetRemoteData(s.m_RemoteData);
    }
    if (!s.m_RemoteId.empty()) {
        track.SetRemoteId(s.m_RemoteId);
    }
    if (!s.m_SeqDataKey.empty()) {
        track.SetSeqDataKey(s.m_SeqDataKey);
    }
    if (!s.m_BigDataIndex.empty()) {
        track.SetBigDataIndex(s.m_BigDataIndex);
    }
    if (!s.m_HubId.empty()) {
        track.SetHubId(s.m_HubId);
    }
}


//
//subtrackid format ID_x
// where x is
// 's'  shown
// 'h'  hidden
// 'm'  marked/highlighted

static void s_ParseSubTrackId(const string& subtrack_id, string& track_id, string& track_attr)
{
    size_t pos = subtrack_id.find_last_of('_');
    if (pos != string::npos) {
        track_id = subtrack_id.substr(0, pos);
        track_attr = subtrack_id.substr(pos + 1);
    } else {
        track_id = subtrack_id;
        track_attr.clear();
    }
}

CTrackConfigManager::CTrackIndex::CTrackIndex(TTrackProxies& t_settings)
{
    // build indexes
    NON_CONST_ITERATE(TTrackProxies, iter, t_settings) {
        CTrackProxy* track = dynamic_cast<CTrackProxy*>(iter->GetPointer());
        if (!track->GetId().empty()) {
            m_IdIndex.emplace(track->GetId(), track);
        }
        if (!track->GetAnnots().empty()) {
            m_AnnotIndex.emplace(track->GetAnnots().front(), track);
        }
        if (!track->GetUId().empty()) {
            m_UidIndex.emplace(track->GetUId(), track);
        }
    }
}

CRef<CTrackProxy> CTrackConfigManager::CTrackIndex::FindTrackById(const string& track_id)
{
    string tr_id = track_id;
    size_t pos = track_id.find_last_of('_');
    if (pos != string::npos) 
        tr_id = track_id.substr(0, pos);
    
    auto it = m_IdIndex.find(tr_id);
    if (it != m_IdIndex.end())
        return Ref(it->second);
    it = m_AnnotIndex.find(tr_id);
    if (it != m_AnnotIndex.end())
        return Ref(it->second);
    it = m_UidIndex.find(tr_id);
    if (it != m_UidIndex.end()) 
        return Ref(it->second);
    return CRef<CTrackProxy>(0);

}

// currently relevant for graph overlays only
void s_ResolveSubTracks(CTrackConfigManager::TTrackProxies& t_settings)
{
    vector<CTrackProxy*> overlays;
    NON_CONST_ITERATE (CTrackConfigManager::TTrackProxies, iter, t_settings) {
        CTrackProxy* track = dynamic_cast<CTrackProxy*>(iter->GetPointer());
        if (track->GetKey() == "graph_overlay") {
            overlays.push_back(track);
        }
    }
    if (overlays.empty())
        return;
    CTrackConfigManager::CTrackIndex track_index(t_settings);
    NON_CONST_ITERATE (vector<CTrackProxy*>, parent_it, overlays) {
        (*parent_it)->GetChildren().clear();
        ITERATE(CTrackProxy::TSubTracks, subtr_it, (*parent_it)->GetSubTracks()) {
            string track_id, track_attr;
            s_ParseSubTrackId(*subtr_it, track_id, track_attr);
            auto t_proxy = track_index.FindTrackById(track_id);
            if (t_proxy) {
                CRef<CTempTrackProxy> subtrack_proxy(new CTrackProxy(*t_proxy));
                subtrack_proxy->SetShown(track_attr != "hidden");
                subtrack_proxy->SetUId(*subtr_it);
                if (subtrack_proxy->GetSource().empty()) {
                    subtrack_proxy->SetSource(NStr::Join(t_proxy->GetAnnots(), "-"));
                }
                (*parent_it)->GetChildren().push_back(subtrack_proxy);
            }
        }
    }
}




void CTrackConfigManager::AlignTrackConfig(TTrackProxies& t_settings,
    const TTrackSettingsSet& in_settings)
{
    // in_settings: the input track list (and settings) that we want
    //              to apply to the existing track list
    // t_settings:  an existing track list which we want to update

    // what we want to achieve:
    //   1. only show the tracks presented in the in_settings
    //   2. keep the original settings not presented in the input track
    //      settings for a given track
    //   3. insert the tracks not present in the original track list
    //   4. show them in the order presented in the input list
    //
    // what we will do to achieve above:
    //   1. Reset the track on/off state
    //   2. Update the track settings in the original track list that
    //      also present in the input track list. The potential updated
    //      settings include track on/off state, annot list, and
    //      track-specific settings.  Track order is not updated at
    //      this point.
    //      Put the tracks that present in both the input track list and
    //      the original track list in a new list acording to the track
    //      order in the input track list. For track that presents only
    //      in the input track list, create a track proxy for them and
    //      add them into the bottom of the oringal track list, and put
    //      it in the newly created track list accordingly.
    //   3. Go through the updated original track list, create a order list
    //      for the tracks that are set to on.
    //   4. Go through the newly created track list, update the track
    //      order ids using the stored order list.
    //   5. Sort the tracks.

    // first, turn all track off
    NON_CONST_ITERATE (TTrackProxies, iter, t_settings) {
        (*iter)->SetShown(false);
    }

    STrackSettings::TTrackConfig config;
    STrackSettings::ParseSettings(in_settings, config);
    t_settings.sort(CTrackProxy::STPSorterByOrder::s_CompareCRefs);
    int last_id = t_settings.back()->GetOrder();

    // second, update the track settings
    TTrackProxies show_list;
    ITERATE (STrackSettings::TTrackConfig, iter, config) {
        TTrackProxies::iterator t_iter = s_FindMatchedTrack(*iter, t_settings);

        CRef<CTrackProxy> track;
        int order = -1;
        if (t_iter != t_settings.end()) {
            track.Reset(dynamic_cast<CTrackProxy*>(t_iter->GetPointer()));
        } else {
            track.Reset(new CTrackProxy());
            t_settings.push_back(CRef<CTempTrackProxy>(track.GetPointer()));
            if (iter->m_Order >= 0) {
                order = iter->m_Order;
            } else {
                order = ++last_id;
            }
        }
        s_InitTrackFromSettings(*track, *iter, order);

        if (iter->m_Shown) {
            show_list.push_back(CRef<CTempTrackProxy>(track.GetPointer()));
        }
    }

    vector<int> order_list;
    NON_CONST_ITERATE (TTrackProxies, iter, t_settings) {
        CTrackProxy* track = dynamic_cast<CTrackProxy*>(iter->GetPointer());
        if (track->GetShown()) {
            order_list.push_back(track->GetOrder());
        }
    }

    // make sure track order in both the input tracks and original track
    // settings are preserved in favor of input tracks order if there is a conflict.
    _ASSERT(order_list.size() == show_list.size());
    size_t idx = 0;
    NON_CONST_ITERATE (TTrackProxies, iter, show_list) {
        (*iter)->SetOrder(order_list[idx++]);
    }

    // sort the tracks
    t_settings.sort(CTrackProxy::STPSorterByOrder::s_CompareCRefs);

    s_ResolveSubTracks(t_settings);

}

void CTrackConfigManager::MergeTrackConfig(TTrackProxies& t_settings,
                                           const TTrackSettingsSet& in_settings,
                                           EAnnotMerge merge)
{
    STrackSettings::TTrackConfig config;
    STrackSettings::ParseSettings(in_settings, config);
    t_settings.sort(CTrackProxy::STPSorterByOrder::s_CompareCRefs);
    int last_id = t_settings.back()->GetOrder();

    ITERATE (STrackSettings::TTrackConfig, iter, config) {
        TTrackProxies::iterator t_iter = s_FindMatchedTrack(*iter, t_settings);

        CRef<CTrackProxy> track;
        int order = -1;
        if (t_iter != t_settings.end()) {
            track.Reset(dynamic_cast<CTrackProxy*>(t_iter->GetPointer()));
        } else {
            track.Reset(new CTrackProxy());
            t_settings.push_back(CRef<CTempTrackProxy>(track.GetPointer()));
            if (iter->m_Order >= 0) {
                order = iter->m_Order;
            } else {
                order = ++last_id;
            }
        }
        bool was_on = track->GetShown();
        s_InitTrackFromSettings(*track, *iter, order, false);

        if ( !iter->m_Annots.empty() ) {
            if (merge == eAnnot_Merge) {
                if ( !was_on ) {
                    track->SetAnnots(iter->m_Annots);
                } else if (!track->GetAnnots().empty() ) {
                    CTrackProxy::TAnnots annots = track->GetAnnots();
                    copy(iter->m_Annots.begin(), iter->m_Annots.end(), back_inserter(annots));
                    track->SetAnnots(annots);
                } else {
                    track->SetAnnots(iter->m_Annots);
                }
            } else if (merge == eAnnot_Replace) {
                track->SetAnnots(iter->m_Annots);
            }
        }
    }
    s_ResolveSubTracks(t_settings);
}


void CTrackConfigManager::SetTrackConfig(TTrackProxies& t_settings,
                                         const TTrackSettingsSet& in_settings,
                                         int order_offset)
{
    // what we want to achieve:
    //   1. only show the tracks presented
    //   2. keep the original settings not presented in the track
    //      settings for a given track
    //   3. show them in the order presented in the input list
    //
    // what we will do to achieve above:
    //   1. Reset the track order ids with a large number (max_id)
    //   2. Replace the track settings for a given track (in order)
    //      from the input in_settings.  The track order will
    //      be the sequence order number in the list
    //   3. Sort the tracks by order
    //   4. Remove all tracks whose track order is equal to the max_order


    const int max_order = INT_MAX;

    // reset track order
    NON_CONST_ITERATE (TTrackProxies, iter, t_settings) {
        (*iter)->SetOrder(max_order);
    }

    // replace the track settings
    int order = order_offset;
    STrackSettings::TTrackConfig config;
    STrackSettings::ParseSettings(in_settings, config);
    ITERATE (STrackSettings::TTrackConfig, iter, config) {
        if (order == max_order)
            break;
        TTrackProxies::iterator t_iter = s_FindMatchedTrack(*iter, t_settings);

        CRef<CTrackProxy> track;
        if (t_iter != t_settings.end()) {
            track.Reset(dynamic_cast<CTrackProxy*>(t_iter->GetPointer()));
        } else {
            track.Reset(new CTrackProxy());
            t_settings.push_back(CRef<CTempTrackProxy>(track.GetPointer()));
        }
        s_InitTrackFromSettings(*track, *iter, order++);
    }

    // sort the tracks
    t_settings.sort(CTrackProxy::STPSorterByOrder::s_CompareCRefs);
    // remove the rest of tracks
    TTrackProxies::iterator t_iter = t_settings.begin();
    while (t_iter != t_settings.end()  &&  (*t_iter)->GetOrder() < max_order) {
        ++t_iter;
    }
    t_settings.erase(t_iter, t_settings.end());

    s_ResolveSubTracks(t_settings);

}

void CTrackConfigManager::CreateDisplayName(const CAnnotMetaData* metadata, const CTempTrackProxy& temp_proxy, string& display_name) const
{

    const string& annot_name = temp_proxy.GetSource();

    // get annotation meta-data if exists
    const CAnnotMetaData* meta_data = metadata;
    if (meta_data == 0) {
        const TAnnotMetaDataList& md_list = GetKnownAnnots(0);
        TAnnotMetaDataList::const_iterator iter = md_list.find(annot_name);
        if (iter != md_list.end()) {
            meta_data = iter->second;
        }
    }

    if (!temp_proxy.GetDisplayName().empty()) {
        display_name = temp_proxy.GetDisplayName();
    } else if (meta_data  &&  !meta_data->m_Title.empty()) {
        display_name = meta_data->m_Title;
    } else if (!CSeqUtils::IsUnnamed(annot_name)) {
        if (annot_name.find(display_name) != string::npos) {
            display_name = annot_name;
        } else if (display_name.find(annot_name) == string::npos) {
            // prevent ugly names starting with "SV-Anon" from appearing
            if(NStr::StartsWith(display_name, CTempTrackProxy::kAnonTrackName)) {
                display_name.clear();
            }
            if (!display_name.empty())
                display_name += " - ";
            display_name += annot_name;
        }
    }
    if (!NStr::StartsWith(display_name, "(H)") && !NStr::StartsWith(display_name, "(R)") && !NStr::StartsWith(display_name, "(U)")) {
        if (NStr::StartsWith(temp_proxy.GetId(), 'R')) {
            display_name = "(R) " + display_name;
        }
        else if (NStr::StartsWith(temp_proxy.GetId(), 'U')) {
            display_name = "(U) " + display_name;
        }
        else {
            for (const auto &annot : temp_proxy.GetAnnots()) {
                if (!NStr::StartsWith(annot, "http", NStr::eNocase) && !NStr::StartsWith(annot, "ftp", NStr::eNocase))
                    continue;

                display_name = "(R) " + display_name;
                break;
            }
        }
    }
}

CRef<CTrackConfig>
CTrackConfigManager::CreateTrackConfig(CTempTrackProxy* temp_proxy,
                                       CSeqGraphicConfig& g_config,
                                       const string& title_base) const
{
    CRef<CCategoryConfig> cat_config = g_config.GetCategoryConfig();

    const CTrackProxy* proxy = dynamic_cast<const CTrackProxy*>(temp_proxy);
    CRef<CLayoutTrack> track(temp_proxy->GetTrack());
    _ASSERT(track);

    // disconnect the track from its track proxy to use all settings
    // from the track proxy instead of the track
    temp_proxy->SetTrack(NULL);

    const string& key = track->GetTypeInfo().GetId();
    const ILayoutTrackFactory* factory = GetTrackFactory(key);
    _ASSERT(factory);

    // extract track profile and settings
    TKeyValuePairs settings;
    string profile = temp_proxy->GetTrackProfile();
    CSGConfigUtils::ParseProfileString(profile, settings);

    TKeyValuePairs::const_iterator p_iter = settings.find("profile");
    if (p_iter != settings.end()) {
        profile = p_iter->second;
    } else if ( !settings.empty()  ||  profile.empty()) {
        profile = "Default";
    }

    const string& annot_name = temp_proxy->GetSource();

    // get annotation meta-data if exists
    const CAnnotMetaData* meta_data = NULL;
    {{
            const TAnnotMetaDataList& md_list = GetKnownAnnots(0);
            TAnnotMetaDataList::const_iterator iter = md_list.find(annot_name);
            if (iter != md_list.end()) {
                meta_data = iter->second;
            }
    }}

    // link the track with proxy again in case it is needed for
    // retrieving track settings
    temp_proxy->SetTrack(track.GetPointer());

    // create track-specific settings
    CRef<CTrackConfig> config;
    const ITrackConfigurable* conf_factory =
        dynamic_cast<const ITrackConfigurable*>(factory);
    if (conf_factory) {
        CRef<CTrackConfigSet> configs =
            conf_factory->GetSettings(profile, settings, temp_proxy);
        if ( !configs->Get().empty() ) {
            config = configs->Set().front();
        }
    } else {
        config.Reset(new CTrackConfig);
        config->SetHelp() = factory->GetThisTypeInfo().GetDescr();
    }

    if ( !config ) {
        return config;
    }

    // add 'comments' settings if any
    if ( !temp_proxy->GetComments().empty() ) {
        typedef vector<string> TCommentPos;
        TCommentPos cmt_pos;
        CTrackUtils::TokenizeWithEscape(temp_proxy->GetComments(), "|", cmt_pos, true);
        TCommentPos::const_iterator iter = cmt_pos.begin();
        while (iter != cmt_pos.end()) {
            const string& label = *iter;
            if (++iter != cmt_pos.end()) {
                config->SetComments().push_back(
                    CTrackConfigUtils::CreateComment(label, *iter));
                ++iter;
            }
        }
    }

    // add 'highlights' settings if any
    if ( !temp_proxy->GetHighlights().empty() ) {
        typedef vector<string> THighlights;
        THighlights hls;
        CTrackUtils::TokenizeWithEscape(temp_proxy->GetHighlights(), "|", hls, true);
        ITERATE(THighlights, h_iter, hls) {
            string hl = NStr::TruncateSpaces(*h_iter);
            if (!hl.empty()) {
                config->SetHighlights().push_back(hl);
            }
        }
    }

    // add 'highlights_color' settings if any
    if (!temp_proxy->GetHighlightsColor().empty()) {
        config->SetHighlights_color(temp_proxy->GetHighlightsColor());
    }

    if ( !temp_proxy->GetShowTitle() ) {
        // By default, we always show title. So we only need to
        // set the flag when it is not shown
        config->SetShow_title(false);
    }
    config->SetIs_private(temp_proxy->GetIsPrivate());
    if(!temp_proxy->GetRemoteData().empty()) {
        config->SetRemote_data(temp_proxy->GetRemoteData());
    }

    if(!temp_proxy->GetRemoteId().empty()) {
        config->SetRmt_mapped_id(temp_proxy->GetRemoteId());
    }

    if(!temp_proxy->GetSeqDataKey().empty()) {
        config->SetSeq_data_key(temp_proxy->GetSeqDataKey());
    }
    if(!temp_proxy->GetBatch().empty()) {
        config->SetBatch(temp_proxy->GetBatch());
    }
    if(!temp_proxy->GetExtraInfo().empty()) {
        config->SetExtra_info(temp_proxy->GetExtraInfo());
    }
    if(!temp_proxy->GetStoredScale().empty()) {
        config->SetStored_scale(temp_proxy->GetStoredScale());
    }
    if (!temp_proxy->GetHelp().empty()) {
        config->SetHelp(temp_proxy->GetHelp());
    }
    // use annotation description for track help if exists
    if (meta_data  &&  !meta_data->m_Descr.empty() && temp_proxy->GetHelp().empty()) {
        config->SetHelp() = NStr::Replace(meta_data->m_Descr, "\n", "<br/>");
    }
    // initialize track name and display name (title)
    config->SetName(temp_proxy->GetName());
    string display_name = title_base;
    CreateDisplayName(meta_data, *temp_proxy, display_name);
    config->SetDisplay_name(display_name);
    /*

    if (!temp_proxy->GetDisplayName().empty()) {
        config->SetDisplay_name() = temp_proxy->GetDisplayName();
    } else if (meta_data  &&  !meta_data->m_Title.empty()) {
        config->SetDisplay_name() = meta_data->m_Title;
    } else if (CSeqUtils::IsUnnamed(annot_name)) {
        config->SetDisplay_name() = title_base;
    } else if (annot_name.find(title_base) != string::npos) {
        config->SetDisplay_name() = annot_name;
    } else if (title_base.find(annot_name) == string::npos) {
        config->SetDisplay_name() = title_base + " - " + annot_name;
    } else {
        config->SetDisplay_name() = title_base;
    }
    */

    // initialize track key (track type) and subkey (subtype)
    config->SetKey() = key;
    config->SetOrder() = temp_proxy->GetOrder();
    string subkey = "";
    if (proxy) {
        if ( !proxy->GetSubkey().empty() ) {
            subkey = config->SetSubkey() = proxy->GetSubkey();
        }
    } else {
        const CFeatureTrack* f_track =
            dynamic_cast<const CFeatureTrack*>(track.GetPointer());
        if (f_track) {
            const CFeatList& feats(*CSeqFeatData::GetFeatList());
            config->SetSubkey() =
                feats.GetStoragekey(f_track->GetFeatSubtype());
        }
    }

    // initialize track category and subcategory
    // We set category and subcat in the following priority order:
    //  - setting from track proxy
    //  - xClass from meta-data
    //  - track type and subtype
    string cat_name = temp_proxy->GetCategory();
    string sub_cat_name = temp_proxy->GetSubcategory();
    if (cat_name.empty()) {
        if (meta_data  &&  !meta_data->m_xClass.empty()) {
            cat_name = meta_data->m_xClass;
            //if ( !meta_data->m_Provider.empty() ) {
            //    sub_cat_name = meta_data->m_Provider;
            //}
        } else {
            if (key == CGraphTrackFactory::GetTypeInfo().GetId()) {
                cat_name = "Graphs";
            } else if (key == CFeatureTrackFactory::GetTypeInfo().GetId()) {
                if (subkey.find("RNA") != string::npos  ||
                    subkey.find("exon") != string::npos) {
                    cat_name = "Genes";
                } else {
                    cat_name  = "Features";
                    sub_cat_name = title_base;
                }
            } else if (key == CGeneModelFactory::GetTypeInfo().GetId()) {
                cat_name = "Genes";
            } else if (key == CAlignmentTrackFactory::GetTypeInfo().GetId()) {
                cat_name = "Alignments";
            } else {
                cat_name = "Others";
            }
        }
    }


    CRef<CCategory> cat;
    CRef<CCategory> sub_cat;
    if (cat_config) {
        cat = cat_config->GetCategory(cat_name);
        sub_cat = cat_config->GetSubcategory(cat_name, sub_cat_name);
    }
    if ( !cat ) {
        cat = CTrackConfigUtils::CreateCategory(cat_name, cat_name, "", 10000);
    }
    config->SetCategory(*cat);

    if ( !sub_cat  &&  !sub_cat_name.empty() ) {
        sub_cat = CTrackConfigUtils::CreateCategory(sub_cat_name, sub_cat_name, "", 100);
    }

    if (sub_cat) {
        config->SetSubcategory(*sub_cat);
    }

    if ( !temp_proxy->GetSettingGroup().empty() ) {
        config->SetSetting_group() = temp_proxy->GetSettingGroup();
    }

    // fill in other track information.
    if ( !temp_proxy->GetFilter().empty() ) {
        config->SetFilter() = temp_proxy->GetFilter();
    }
    if ( !temp_proxy->GetSortBy().empty() ) {
        config->SetSort_by() = temp_proxy->GetSortBy();
    }
    if ( !temp_proxy->GetId().empty() ) {
        config->SetId(temp_proxy->GetId());
    }
    if (proxy  &&  !proxy->GetDB().empty() ) {
        config->SetDbname(proxy->GetDB());
    }
    if (proxy  &&  !proxy->GetDataKey().empty() ) {
        config->SetData_key(proxy->GetDataKey());
    }

    config->SetShown() = temp_proxy->GetShown();
    config->SetAnnots().push_back(annot_name);

    if (!temp_proxy->GetUId().empty())
        config->SetUId(temp_proxy->GetUId());

    if (!temp_proxy->GetSubTracks().empty()) {
        config->SetSubTracks() = temp_proxy->GetSubTracks();
    }

    if (!temp_proxy->GetBigDataIndex().empty()) {
        config->SetBig_data_index() = temp_proxy->GetBigDataIndex();
    }

    if (!temp_proxy->GetHubId().empty()) {
        config->SetHub_id(temp_proxy->GetHubId());
    }

    return config;
}


void CTrackConfigManager::x_InitTrackManager()
{
    typedef vector< CIRef<ILayoutTrackFactory> >  TTrackFactoryVec;
    TTrackFactoryVec factories;
    GetExtensionAsInterface(
        EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY, factories);

    ITERATE (TTrackFactoryVec, iter, factories) {
        m_Factories[(*iter)->GetThisTypeInfo().GetId()] = *iter;
    }
}

/*
CRef<CTrackProxy> CTrackConfigManager::FindTrackByUId(const string& uid)
{
    if (m_UIds.count(uid) != 0)
        return m_UIds[uid];
    return CRef<CTrackProxy>(0);

}
*/
void CTrackConfigManager::GetUsedFeatureKeys(set<string>& subkeys, const TTrackProxies& proxies)
{
    const string& featKey = CFeatureTrackFactory::GetTypeInfo().GetId();
    for (auto& p : proxies) {
        if (p->GetKey() == featKey)
            subkeys.insert(p->GetSubkey());
    }
}

bool CTrackConfigManager::ValidFeatureTrack(const CFeatListItem& item)
{
    int subtype = item.GetSubtype();

    return subtype != CSeqFeatData::eSubtype_any &&
        subtype != CSeqFeatData::eSubtype_variation &&
        subtype != CSeqFeatData::eSubtype_variation_ref &&
        !CDataTrackUtils::IsGeneModelFeature(item.GetType(), subtype);
}

END_NCBI_SCOPE
