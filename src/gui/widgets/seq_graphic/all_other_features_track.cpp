/*  $Id: all_other_features_track.cpp 42431 2019-02-22 16:04:19Z katargir $
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

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/all_other_features_track.hpp>
#include <gui/widgets/seq_graphic/feature_track.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CTrackTypeInfo
CAllOtherFeaturesTrack::m_TypeInfo("all_other_features_track", "All Other Features Track");
static const string kDefTrackTitle = "All other features";

CAllOtherFeaturesTrack::CAllOtherFeaturesTrack(CRenderingContext* r_cntx,
                                              CFeaturePanelDS* ds,
                                              const set<string>& usedFeatureSubkeys)
  : CTrackContainer(r_cntx, ds)
  , m_UsedFeatureSubkeys(usedFeatureSubkeys)
{}


CAllOtherFeaturesTrack::~CAllOtherFeaturesTrack()
{}


bool CAllOtherFeaturesTrack::NoSubtrackEver() const
{
    bool no_track = GetChildren().empty();
    CRef<CSGSequenceDS> seq_ds = m_Context->GetSeqDS();
    int seq_len = seq_ds->GetSequenceLength();
    int off = (int)(seq_len * 0.0001);
    TSeqRange total_range(off, seq_len - off);
    if (no_track) {
        CWeakRef<CTrackProxy>::TRefType this_proxy = m_ThisProxy.Lock();
        if (this_proxy.NotNull()) {
            no_track = 
                this_proxy->GetVisitedRange().IntersectionWith(total_range) == total_range;
        }
    }
    TTrackProxies::const_iterator iter = m_TrackProxies.begin();
    while(no_track  &&  iter != m_TrackProxies.end()) {
        no_track = ((*iter)->GetTrack() == NULL);
        ++iter;
    }
    return no_track;
}


const CTrackTypeInfo& CAllOtherFeaturesTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}


string CAllOtherFeaturesTrack::GetFullTitle() const
{
    if (GetTitle().empty()) {
        return kDefTrackTitle;
    }
    return GetTitle();
}


void CAllOtherFeaturesTrack::x_OnAllJobsFinished()
{
    // update the visited range by including the previous visited range
    // if both intersect to each other
    TSeqRange vis_range = m_Context->GetVisSeqRange();
    CWeakRef<CTrackProxy>::TRefType this_proxy = m_ThisProxy.Lock();
    if (this_proxy.NotNull()) {
        if (vis_range.IntersectingWith(this_proxy->GetVisitedRange())) {
            vis_range.CombineWith(this_proxy->GetVisitedRange());
        }
        this_proxy->SetVisitedRange(vis_range);
    }

    CTrackContainer::x_OnAllJobsFinished();
}


void CAllOtherFeaturesTrack::x_ReinitSubtracks()
{
    const TSeqRange& curr_range = m_Context->GetVisSeqRange();
    // visited range covers the current visible range,
    // no need to reinitialize the track.
    CWeakRef<CTrackProxy>::TRefType this_proxy = m_ThisProxy.Lock();
    if (this_proxy.IsNull()  ||
        this_proxy->GetVisitedRange().IntersectionWith(curr_range) ==  curr_range) {
        return;
    }

    for (const auto& item : *CSeqFeatData::GetFeatList()) {
        string storagekey = item.GetStoragekey();
        if (!CTrackConfigManager::ValidFeatureTrack(item) ||
            m_UsedFeatureSubkeys.find(storagekey) != m_UsedFeatureSubkeys.end())
            continue;

        CRef<CTempTrackProxy> track_proxy =
            this_proxy->GetChildTempTrack(item.GetDescription());

        track_proxy->SetKey(CFeatureTrackFactory::GetTypeInfo().GetId());
        track_proxy->SetSubkey(storagekey);

        x_ReinitFeatureTrack(track_proxy);
    }
}

void CAllOtherFeaturesTrack::ReinitNASubtracks(TAnnotMetaDataList& annots, bool makeContainersVisible)
{
    CWeakRef<CTrackProxy>::TRefType this_proxy = m_ThisProxy.Lock();
    if (this_proxy.IsNull())
        return;

    for (const auto& item : *CSeqFeatData::GetFeatList()) {
        string storagekey = item.GetStoragekey();
        if (!CTrackConfigManager::ValidFeatureTrack(item) ||
            m_UsedFeatureSubkeys.find(storagekey) != m_UsedFeatureSubkeys.end())
            continue;

        CRef<CTempTrackProxy> track_proxy =
            this_proxy->GetChildTempTrack(item.GetDescription());

        track_proxy->SetKey(CFeatureTrackFactory::GetTypeInfo().GetId());
        track_proxy->SetSubkey(storagekey);

        x_ReinitNATrack(track_proxy, annots, makeContainersVisible);
    }
}

void CAllOtherFeaturesTrack::x_ReinitFeatureTrack(CTempTrackProxy* t_proxy)
{
    const string& key = t_proxy->GetKey();
    const string& subkey = t_proxy->GetSubkey();

    const ILayoutTrackFactory* factory = GetConfigMgr()->GetTrackFactory(key);
    if (factory) {
        int depth = GetAnnotLevel();
        ILayoutTrackFactory::SExtraParams params(depth, GetAdaptive(), NULL, subkey);
        params.m_SkipGenuineCheck = GetSkipGenuineCheck();
        m_DS->CreateTrack("creating " + t_proxy->GetName() + " track",
            factory, this, t_proxy, params);
        SetTrackInitDone(false);
        SetMsg(", Initializing subtracks...");
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CAllOtherFeaturesTrackFactory
///////////////////////////////////////////////////////////////////////////////
ILayoutTrackFactory::TTrackMap
CAllOtherFeaturesTrackFactory::CreateTracks(SConstScopedObject& object,
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
    CRef<CAllOtherFeaturesTrack> track(new CAllOtherFeaturesTrack(r_cntx, fp_ds, params.m_UsedFeatureSubkeys));
    tracks[kDefTrackTitle] = track.GetPointer();
    track->SetAnnotLevel(params.m_Level);
    track->SetAdaptive(params.m_Adaptive);
    track->SetSkipGenuineCheck(params.m_SkipGenuineCheck);
    return tracks;
}


string CAllOtherFeaturesTrackFactory::GetExtensionIdentifier() const
{
    return CAllOtherFeaturesTrack::m_TypeInfo.GetId();
}


string CAllOtherFeaturesTrackFactory::GetExtensionLabel() const
{
    return CAllOtherFeaturesTrack::m_TypeInfo.GetDescr();
}


CRef<CTrackConfigSet>
CAllOtherFeaturesTrackFactory::GetSettings(const string& /*profile*/,
                                           const TKeyValuePairs& /*settings*/,
                                           const CTempTrackProxy* track_proxy) const
{
    CRef<CTrackConfigSet> config_set(new CTrackConfigSet);
    if (!track_proxy) {
        return config_set;
    }
    // must be data-independent
    if (!track_proxy->GetTrack()) {
        CRef<CTrackConfig> config(new CTrackConfig);
        config_set->Set().push_back(config);
        return config_set;
    }

    const CFeatList* feats = CSeqFeatData::GetFeatList();
    const string& key = CFeatureTrackFactory::GetTypeInfo().GetId();
    bool shown = track_proxy->GetShown();
    CFeatureTrackFactory feature_factory;

    ITERATE (CTempTrackProxy::TTrackProxies, t_iter, track_proxy->GetChildren()) {
        const CTempTrackProxy* track = *t_iter;
        if ( !track->GetTrack() ) continue;

        CRef<CTrackConfigSet> configs;
        TKeyValuePairs empty_settings;
        string empty_profile = "";
        configs = feature_factory.GetSettings(empty_profile, empty_settings, track);
        CRef<CTrackConfig> config = configs->Set().front();
        config->SetName() = track->GetName();
        config->SetKey() = key;
        config->SetShown() = shown;
        
        if (track->GetChildren().size() < 2) {
            const CFeatureTrack* feat_track =
                dynamic_cast<const CFeatureTrack*>(track->GetTrack());
            _ASSERT(feat_track);
            config->SetSubkey() = feats->GetStoragekey(feat_track->GetFeatSubtype());
            config->SetAnnots().push_back(track->GetSource());
        } else {
            const CFeatureTrack* feat_track =
                dynamic_cast<const CFeatureTrack*>(track->GetChildren().front()->GetTrack());
            _ASSERT(feat_track);
            config->SetSubkey() = feats->GetStoragekey(feat_track->GetFeatSubtype());
            ITERATE (CTempTrackProxy::TTrackProxies, iter, track->GetChildren()) {
                string annot_name = (*iter)->GetName();
                if (CSeqUtils::IsNAA(annot_name)) {
                    NStr::ReplaceInPlace(annot_name, "_", ".");
                }
                config->SetAnnots().push_back(annot_name);
            }
        }
        config_set->Set().insert(config_set->Set().end(),
                                 configs->Get().begin(), configs->Get().end());
    }

    return config_set;
}


END_NCBI_SCOPE
