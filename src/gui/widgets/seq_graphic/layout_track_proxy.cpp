/*  $Id: layout_track_proxy.cpp 44576 2020-01-23 23:22:13Z rudnev $
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
 * Authors:  Liangshou Wu, Dmitry Rudnev
 *
 */

/// @file
///

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/layout_track_proxy.hpp>
#include <util/static_map.hpp>
#include <gui/objutils/utils.hpp>

#include <unordered_set>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CTempTrackProxy class implementation
///////////////////////////////////////////////////////////////////////////////

// default track name base for user-provided tracks spec without 'name' set
const string CTempTrackProxy::kAnonTrackName = "SV-Anon";

CTempTrackProxy::CTempTrackProxy(const CTempTrackProxy& param)
    : m_Order(param.GetOrder())
    , m_Name(param.GetName())
    , m_Id(param.GetId())
    , m_DispName(param.GetDisplayName())
    , m_Profile(param.GetTrackProfile())
    , m_Source(param.GetSource())
    , m_Category(param.GetCategory())
    , m_Subcategory(param.GetSubcategory())
    , m_SettingGroup(param.GetSettingGroup())
    , m_UseGroupDefault(param.GetUseGroupDefault())
    , m_Filter(param.m_Filter)
    , m_SortBy(param.m_SortBy)
    , m_Track(NULL)
    , m_Shown(param.GetShown())
    , m_Help(param.m_Help)
    , m_Expanded(param.GetExpanded())
    , m_Realized(param.IsRealized())
    , m_Empty(false)
    , m_Comments(param.m_Comments)
    , m_Highlights(param.m_Highlights)
    , m_HighlightsColor(param.m_HighlightsColor)
    , m_ShowTitle(param.m_ShowTitle)
    , m_ErrorStatus(param.m_ErrorStatus)
    , m_isPrivate(param.m_isPrivate)
    , m_StoredScale(param.m_StoredScale)
    , m_UId(param.m_UId)
    , m_SubTracks(param.m_SubTracks)
    , m_Key(param.m_Key)
    , m_Subkey(param.m_Subkey)
    , m_Annots(param.m_Annots)
    , m_Action(param.m_Action)
    , m_TimeStamp(param.m_TimeStamp)
    , m_Batch(param.m_Batch)
    , m_RemotePath(param.m_RemotePath)
    , m_RemoteData(param.m_RemoteData)
    , m_SeqDataKey(param.m_SeqDataKey)
    , m_BigDataIndex(param.m_BigDataIndex)
{
    ITERATE (TTrackProxies, iter, param.GetChildren()){
        m_Children.push_back((*iter)->Clone());
    }
}


CTempTrackProxy& CTempTrackProxy::operator=(const CTempTrackProxy& param)
{
    if (this == &param) return *this;

    m_Order = param.GetOrder();
    m_Name = param.GetName();
    m_Id = param.GetId();
    m_DispName = param.GetDisplayName();
    m_Profile = param.GetTrackProfile();
    m_Source = param.GetSource();
    m_Category = param.GetCategory();
    m_Subcategory = param.GetSubcategory();
    m_SettingGroup = param.GetSettingGroup();
    m_UseGroupDefault = param.GetUseGroupDefault();
    m_Filter = param.m_Filter;
    m_SortBy = param.m_SortBy;
    m_Track = NULL;
    m_Shown = param.GetShown();
    m_Help = param.m_Help;
    m_Expanded = param.GetExpanded();
    m_Realized = param.IsRealized();
    m_Empty = false;
    m_Comments = param.m_Comments;
    m_Highlights = param.m_Highlights;
    m_HighlightsColor = param.m_HighlightsColor;
    m_ShowTitle = param.m_ShowTitle;
    m_ErrorStatus = param.m_ErrorStatus;
    m_isPrivate = param.m_isPrivate;
    m_StoredScale = param.m_StoredScale;
    m_UId = param.m_UId;
    m_SubTracks = param.m_SubTracks;
    m_Key = param.m_Key;
    m_Subkey = param.m_Subkey;
    m_Annots = param.m_Annots;
    m_Action = param.m_Action;
    m_TimeStamp = param.m_TimeStamp;
    m_Batch = param.m_Batch;
    m_RemotePath = param.m_RemotePath;
    m_RemoteData = param.m_RemoteData;
    m_SeqDataKey = param.m_SeqDataKey;
    m_BigDataIndex = param.m_BigDataIndex;

    ITERATE (TTrackProxies, iter, param.GetChildren()){
        m_Children.push_back((*iter)->Clone());
    }

    return *this;
}


CRef<CTempTrackProxy> CTempTrackProxy::Clone() const
{
    CRef<CTempTrackProxy> proxy(new CTempTrackProxy(*this));
    return proxy;
}


CRef<CTempTrackProxy> CTempTrackProxy::AddTempTrack(const string& name, const string& title)
{
    CRef<CTempTrackProxy> track(new CTempTrackProxy(-1, name, true));
    int next_order = m_Children.empty() ? 0 : m_Children.back()->GetOrder() + 1;
    track->SetOrder(next_order);
    track->SetRealized(false);
    track->SetTrackProfile(GetTrackProfile());
    track->SetCategory(GetCategory());
    track->SetSubcategory(GetSubcategory());
    track->SetFilter(GetFilter());
    track->SetSortBy(GetSortBy());
    track->SetErrorStatus(GetErrorStatus());
    track->SetDisplayName(title);

    m_Children.push_back(track);
//    LOG_POST(Trace << "AddTempTrack() name: " << name << ", title: " << title);
    return track;
}

CRef<CTempTrackProxy> CTempTrackProxy::AddTempTrack(
    const string& name, const string& filter, const string& title)
{
    CRef<CTempTrackProxy> track;
    if (filter.empty()) {
        track = AddTempTrack(name, title);
    } else {
        track = AddTempTrack(name + "-" + filter, title);
        track->SetFilter(filter);
    }
    return track;
}


CConstRef<CTempTrackProxy>
CTempTrackProxy::GetChildTempTrack(const string& name) const
{
    string name_modified(name);
    NStr::ReplaceInPlace(name_modified, ".", "_");
    ITERATE (CTrackProxy::TTrackProxies, iter, m_Children) {
        if ((*iter)->GetName() == name_modified) {
            return CConstRef<CTempTrackProxy>(iter->GetPointer());
        }
    }
    return CConstRef<CTempTrackProxy>();
}


CRef<CTempTrackProxy>
CTempTrackProxy::GetChildTempTrack(const string& name)
{
    string name_modified(name);
    NStr::ReplaceInPlace(name_modified, ".", "_");
    NON_CONST_ITERATE (CTrackProxy::TTrackProxies, iter, m_Children) {
        if ((*iter)->GetName() == name_modified) {
            // if the child doesn't have a profile, we
            // want to use its parent's profile.
            if ((*iter)->GetTrackProfile().empty()) {
                (*iter)->SetTrackProfile(GetTrackProfile());
            }
            if ((*iter)->GetFilter().empty()) {
                (*iter)->SetFilter(m_Filter);
            }
            if ((*iter)->GetSortBy().empty()) {
                (*iter)->SetSortBy(m_SortBy);
            }
            if ((*iter)->GetSource().empty()) {
                (*iter)->SetSource(name);
            }
            return *iter;
        }
    }
    //!! think of where to get the track title here
    CRef<CTempTrackProxy> track = AddTempTrack(name_modified, "");
    track->SetSource(name);
    return track;
}


bool CTempTrackProxy::IsNA() const
{
//    if(IsExtendedNA()) {
//        LOG_POST(Trace << "IsNA() when in fact IsExtendedNA() " << m_Source);
//    }
    return CSeqUtils::IsNAA(m_Source);
}

bool CTempTrackProxy::IsExtendedNA() const
{
    return CSeqUtils::IsExtendedNAA(m_Source);
}

void CTempTrackProxy::SetNAdata(CAnnotMetaData& na)
{
    m_NAdata.Reset(&na);

    SetSource(na.m_Name);
    if (!na.m_SubCategory.empty())
        SetSubcategory(na.m_SubCategory);
    if (!na.m_Title.empty())
        SetDisplayName(na.m_Title);
}

void CTempTrackProxy::UpdateNA_ChildVisibility(CTempTrackProxy& child) const
{
    if (m_Annots.empty())
        return;

    const CAnnotMetaData* naData = child.GetNAdata();
    if (!naData)
        return;

    if (m_Annots.front() == "NA*") {
        child.SetShown(true);
        return;
    }

    if (m_Annots.front() == "NA*" ||
        find(m_Annots.begin(), m_Annots.end(), naData->m_Name) != m_Annots.end()) {
        child.SetShown(true);
    }
}


static const string k_order = "Order";
static const string k_category = "Category";
static const string k_subcategory = "Subcategory";
static const string k_setting_group = "SettingGroup";
static const string k_shown = "Shown";
static const string k_expanded = "Expanded";
static const string k_track_profile = "TrackProfile";
static const string k_key = "Key";
static const string k_subkey = "Subkey";
static const string k_filter = "Filter";
static const string k_sortby = "SortBy";
static const string k_display_name = "DisplayName";
static const string k_help = "Help";
static const string k_annots = "Annots";
static const string k_annot_separation = "AnnotSeparation";

static const string k_id = "Id";
static const string k_db = "dbname";
static const string k_subtracks = "SubTracks";
static const string k_trid = "trid";

static const string k_time_stamp = "TimeStamp";

static void s_LoadTempTrackRecursive(CTempTrackProxy::TTrackProxies& proxies, const string& path)
{
    time_t defTimeStamp = time(NULL);
    if (defTimeStamp != -1) defTimeStamp -= 30 * 24 * 60 * 60;

    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    string key = path + CGuiRegistry::kDecimalDot + "TempTracks";
    CRegistryReadView view = registry.GetReadView(key);
    CRegistryReadView::TKeys keys;
    view.GetTopKeys(keys);
    ITERATE(CRegistryReadView::TKeys, iter, keys) {
        string name = iter->key;
        string sub_key = key + CGuiRegistry::kDecimalDot + name;
        CRegistryReadView sub_view = registry.GetReadView(sub_key);
        CRef<CTempTrackProxy> track(new CTempTrackProxy);
        track->SetOrder(sub_view.GetInt(k_order, -1));
        track->SetCategory(sub_view.GetString(k_category, kEmptyStr));
        track->SetSubcategory(sub_view.GetString(k_subcategory, kEmptyStr));
        track->SetTimeStamp(sub_view.GetInt(k_time_stamp, defTimeStamp));

        {
            string setting_g_str = sub_view.GetString(k_setting_group, kEmptyStr);
            vector<string> tokens;
            CTrackUtils::TokenizeWithEscape(setting_g_str, "|", tokens, true);
            if (tokens.size() > 0) {
                track->SetSettingGroup(NStr::TruncateSpaces(tokens[0]));
                if (tokens.size() > 1) {
                    try {
                        track->SetUseGroupDefault(NStr::StringToBool(NStr::TruncateSpaces(tokens[1])));
                    }
                    catch (CException&) {
                        // ignore it
                    }
                }
            }
        }

        track->SetShown(sub_view.GetBool(k_shown, true));
        track->SetExpanded(sub_view.GetBool(k_expanded, true));
        track->SetName(name);
        if (CSeqUtils::IsNAA_Name(name))
            track->SetSource(NStr::Replace(name, "_", "."));

        track->SetDisplayName(sub_view.GetString(k_display_name, kEmptyStr));
        track->SetHelp(sub_view.GetString(k_help, kEmptyStr));
        track->SetTrackProfile(sub_view.GetString(k_track_profile, kEmptyStr));
        proxies.push_back(track);
        s_LoadTempTrackRecursive(track->GetChildren(), sub_key);
    }
}

void CTempTrackProxy::LoadFromRegistry(TTrackProxies& proxies, const string& path, bool isMergeWithExisting)
{
    unordered_set<string> ExistingKeys;
    if(isMergeWithExisting) {
        for(auto proxy: proxies) {
            ExistingKeys.insert(proxy->GetKey());
        }
    }
    time_t defTimeStamp = time(NULL);

    string key = path + ".Tracks";
    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryReadView view = registry.GetReadView(key);
    CRegistryReadView::TKeys keys;
    view.GetTopKeys(keys);
    ITERATE(CRegistryReadView::TKeys, iter, keys) {
        string base_key = key + CGuiRegistry::kDecimalDot + iter->key;
        // load settings for this track
        CRegistryReadView sub_view = registry.GetReadView(base_key);
        string sKey = sub_view.GetString(k_key);
        
        // if merge is requested, skip tracks with existing keys
        if(isMergeWithExisting && ExistingKeys.count(sKey)) {
            continue;
        }
        CRef<CTrackProxy> track(new CTrackProxy);
        track->SetKey(sKey);
        track->SetSubkey(sub_view.GetString(k_subkey, kEmptyStr));
        track->SetFilter(sub_view.GetString(k_filter, kEmptyStr));
        track->SetSortBy(sub_view.GetString(k_sortby, kEmptyStr));
        track->SetDisplayName(sub_view.GetString(k_display_name, kEmptyStr));
        track->SetHelp(sub_view.GetString(k_help, kEmptyStr));
        track->SetOrder(sub_view.GetInt(k_order, -1));
        track->SetId(sub_view.GetString(k_id, ""));
        track->SetDB(sub_view.GetString(k_db, ""));
        track->SetCategory(sub_view.GetString(k_category, kEmptyStr));
        track->SetSubcategory(sub_view.GetString(k_subcategory, kEmptyStr));
        track->SetTimeStamp(sub_view.GetInt(k_time_stamp, defTimeStamp));

        {
            string setting_g_str = sub_view.GetString(k_setting_group, kEmptyStr);
            vector<string> tokens;
            CTrackUtils::TokenizeWithEscape(setting_g_str, "|", tokens, true);
            if (tokens.size() > 0) {
                track->SetSettingGroup(NStr::TruncateSpaces(tokens[0]));
                if (tokens.size() > 1) {
                    try {
                        track->SetUseGroupDefault(NStr::StringToBool(NStr::TruncateSpaces(tokens[1])));
                    }
                    catch (CException&) {
                        // ignore it
                    }
                }
            }
        }

        track->SetShown(sub_view.GetBool(k_shown));
        track->SetExpanded(sub_view.GetBool(k_expanded, true));
        track->SetTrackProfile(sub_view.GetString(k_track_profile));
        string annot_str = sub_view.GetString(k_annots, kEmptyStr);
        if (annot_str.empty()) {
            annot_str = sub_view.GetString(k_annot_separation, kEmptyStr);
        }
        CTrackProxy::TAnnots annots;
        NStr::Split(annot_str, ",|", annots);
        track->SetAnnots(annots);
        string subtracks_str = sub_view.GetString(k_subtracks, kEmptyStr);
        CTrackProxy::TSubTracks subtracks;
        ParseSubtracks(subtracks_str, subtracks);
        if (!subtracks.empty())
            track->SetSubTracks() = subtracks;
        track->SetName(iter->key);
        proxies.push_back(CRef<CTempTrackProxy>(track.GetPointer()));

        // load children temporary tracks
        s_LoadTempTrackRecursive(track->GetChildren(), base_key);
        // load children tracks
        LoadFromRegistry(track->GetChildren(), base_key);
    }

    proxies.sort(CTrackProxy::STPSorterByOrder::s_CompareCRefs);
}

void CTempTrackProxy::SaveToRegistry(const TTrackProxies& proxies, const string& path)
{
    static const size_t kMaxTrackSaved = 100;

    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    TTrackProxies settings = proxies;

    size_t delCount = settings.size() > kMaxTrackSaved ?
        settings.size() - kMaxTrackSaved : 0;

    if (delCount > 0) {
        settings.sort([](const CRef<CTempTrackProxy>& t1, const CRef<CTempTrackProxy>& t2) -> bool
        { return t1->GetTimeStamp() < t2->GetTimeStamp(); });

        auto it = settings.begin();
        while (it != settings.end() && delCount > 0) {
            it = settings.erase(it);
            --delCount;
        }
    }

    settings.sort([](const CRef<CTempTrackProxy>& t1, const CRef<CTempTrackProxy>& t2) -> bool
    { return t1->GetOrder() < t2->GetOrder(); });

    int off = 0;
    int pre_id = -1;

    ITERATE(TTrackProxies, iter, settings) {
        const CTempTrackProxy* params = *iter;
        const CTrackProxy* full_params =
            dynamic_cast<const CTrackProxy*>(iter->GetPointer());

        // for those unrealized trackd, we ignore them
        if (!params->IsRealized()) {
            --off;
            continue;
        }

        string key = path + ".TempTracks";
        if (full_params) {
            key = path + ".Tracks";
        }

        string sub_key = key + CGuiRegistry::kDecimalDot + params->GetName();
        CRegistryWriteView view = registry.GetWriteView(sub_key);
        if (pre_id == params->GetOrder()) {
            ++off;
        }

        view.Set(k_order, params->GetOrder() + off);
        view.Set(k_category, params->GetCategory());
        view.Set(k_subcategory, params->GetSubcategory());
        time_t timeStamp = params->GetTimeStamp();
        if (timeStamp != -1) view.Set(k_time_stamp, (int)timeStamp);

        if (!params->GetSettingGroup().empty()) {
            view.Set(k_setting_group, params->GetSettingGroup());
        }

        bool shown = params->GetShown();
        if (full_params && full_params->GetContShown())
            shown = true;
        view.Set(k_shown, shown);

        // save collapse state only, by default, a track is expanded.
        if (!params->GetExpanded()) {
            view.Set(k_expanded, params->GetExpanded());
        }
        if (!params->GetDisplayName().empty()) {
            view.Set(k_display_name, params->GetDisplayName());
        }
        if (!params->GetHelp().empty()) {
            view.Set(k_help, params->GetHelp());
        }

//        cerr << "Checking profile for registry sub_key: " << sub_key << " temp proxy name: " << params->GetName() << " params(CTempTrackProxy) at: " << params << " full_params(CTrackProxy) at: " << full_params << endl;
//        cerr << "Profile contents: " << params->GetTrackProfile() << endl;

        if (!params->GetTrackProfile().empty()) {
//            cerr << "Writing profile" << endl;
            view.Set(k_track_profile, params->GetTrackProfile());

            // consider the case where there is only one subtrack so
            // that the current track profile should actually be saved
            // to its child track
            if (!params->GetSource().empty() &&
                !params->GetTrackProfile().empty()) {
//                cerr << "Looking for child track with Source: " << params->GetSource() << endl;
                CConstRef<CTempTrackProxy> c_params =
                    params->GetChildTempTrack(params->GetSource());
                if (c_params) {
                    CTempTrackProxy* t_params =
                        const_cast<CTempTrackProxy*>(c_params.GetPointer());
//                    cerr << "Child track found at address: " << t_params << endl;
//                    cerr << "Current child track profile       : " << t_params->GetTrackProfile() << endl;
                    if(t_params->GetTrackProfile() == "Default") {
//                        cerr << "Overwriting child track profile to: " << params->GetTrackProfile() << endl;
                        t_params->SetTrackProfile(params->GetTrackProfile());
//                   } else {
//                       cerr << "Not overwriting not default child profile" << endl;
                    }
//                } else {
//                    cerr << "Child track not found" << endl;
                }
            }
        }

        if (full_params) {
            view.Set(k_key, full_params->GetKey());
            if (!full_params->GetSubkey().empty()) {
                view.Set(k_subkey, full_params->GetSubkey());
            }
            if (!full_params->GetFilter().empty()) {
                view.Set(k_filter, full_params->GetFilter());
            }
            if (!full_params->GetSortBy().empty()) {
                view.Set(k_sortby, full_params->GetSortBy());
            }

            CTrackProxy::TAnnots annots = full_params->GetAnnots();
            CTrackProxy::TAnnots::iterator a_iter =
                find(annots.begin(), annots.end(),
                CSeqUtils::NameTypeValueToStr(CSeqUtils::eAnnot_All));
            if (a_iter != annots.end()) {
                annots.erase(a_iter);
            }
            if (!annots.empty()) {
                string annots_str = kEmptyStr;
                bool first_one = true;
                ITERATE(CTrackProxy::TAnnots, aa_iter, annots) {
                    if (first_one) {
                        annots_str = *aa_iter;
                        first_one = false;
                    }
                    else {
                        annots_str += "|" + *aa_iter;
                    }
                }
                view.Set(k_annots, annots_str);
            }
        }

        if (!params->GetChildren().empty()) {
            SaveToRegistry(params->GetChildren(), sub_key);
        }

        pre_id = params->GetOrder();
    }
}

/// convert a string with a list of subtracks to a list of subtracks
void CTempTrackProxy::ParseSubtracks(const string& subtracks_str, TSubTracks& subtracks)
{
    NStr::Split(subtracks_str, ",|", subtracks);
}


///////////////////////////////////////////////////////////////////////////////
/// CTrackProxy class implementation
///////////////////////////////////////////////////////////////////////////////

CTrackProxy::CTrackProxy(const CTrackProxy& param)
    : CTempTrackProxy(param)
    , m_DB(param.m_DB)
    , m_DataKey(param.m_DataKey)
    , m_ContShown(param.m_ContShown)
{}


CTrackProxy& CTrackProxy::operator=(const CTrackProxy& param)
{
    if (this == &param) return *this;

    CTempTrackProxy::operator=(param);
    m_DB = param.m_DB;
    m_DataKey = param.m_DataKey;
    m_ContShown = param.m_ContShown;

    return *this;
}


CRef<CTempTrackProxy> CTrackProxy::Clone() const
{
    CRef<CTempTrackProxy> proxy(new CTrackProxy(*this));
    return proxy;
}


END_NCBI_SCOPE
