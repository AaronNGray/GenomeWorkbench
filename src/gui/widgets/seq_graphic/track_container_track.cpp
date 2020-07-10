/*  $Id: track_container_track.cpp 44797 2020-03-17 23:36:03Z rudnev $
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
#include <gui/widgets/seq_graphic/track_container_track.hpp>
#include <gui/widgets/seq_graphic/graph_overlay.hpp>
#include <gui/widgets/seq_graphic/layout_track_proxy.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/reg_settings.hpp>
#include <gui/objutils/utils.hpp>
#include <wx/event.h>
#include <wx/menu.h>


BEGIN_NCBI_SCOPE


static const size_t kMaxMenuTracks = 25;
static const int kBaseID = 10000;
static const int kBaseID_Settings = kBaseID + kMaxMenuTracks;

static const size_t kMaxNameLen = 50;

CTrackTypeInfo
CTrackContainer::m_TypeInfo("track_container", "Track Container Track");

class CTrackContainerEvtHandler : public wxEvtHandler
{
public:
    CTrackContainerEvtHandler(CTrackContainer* track)
        : m_Track( track )
    {}

    void OnContextMenu( wxContextMenuEvent& anEvent );

private:
    void x_OnMore(wxCommandEvent& /*event*/)
        { m_Track->OnMore(); m_Track->ResetSearch();}
    void x_OnSelectAll(wxCommandEvent& /*event*/)
    { m_Track->OnSelectAll(); m_Track->ResetSearch();}
    void x_OnDeselectAll(wxCommandEvent& /*event*/)
    { m_Track->OnDeselectAll(); m_Track->ResetSearch();}
    void x_OnRevertSelection(wxCommandEvent& /*event*/)
    { m_Track->OnRevertSelection(); m_Track->ResetSearch();}
    void x_OnToggleTrack(wxCommandEvent& event)
    {
        m_Track->OnMenuToggleTrack(event.GetId() - kBaseID);
        m_Track->ResetSearch();
    }
    void x_OnTrackSettings(wxCommandEvent& event)
    {
        m_Track->OnMenuTrackSettings(event.GetId() - kBaseID_Settings);
        m_Track->ResetSearch();
    }

private:
    CTrackContainer* m_Track;

    DECLARE_EVENT_TABLE()
};


BEGIN_EVENT_TABLE(CTrackContainerEvtHandler, wxEvtHandler)
    EVT_MENU(kBaseID - 4, CTrackContainerEvtHandler::x_OnMore)
    EVT_MENU(kBaseID - 3, CTrackContainerEvtHandler::x_OnSelectAll)
    EVT_MENU(kBaseID - 2, CTrackContainerEvtHandler::x_OnDeselectAll)
    EVT_MENU(kBaseID - 1, CTrackContainerEvtHandler::x_OnRevertSelection)
    EVT_MENU_RANGE(kBaseID, kBaseID + kMaxMenuTracks - 1, CTrackContainerEvtHandler::x_OnToggleTrack)
    EVT_MENU_RANGE(kBaseID_Settings, kBaseID_Settings + kMaxMenuTracks - 1, CTrackContainerEvtHandler::x_OnTrackSettings)
END_EVENT_TABLE()


CTrackContainer::CTrackContainer(CRenderingContext* r_cntx, CFeaturePanelDS* ds)
    : CDataTrack(r_cntx)
    , m_DS(ds)
    , m_Handler(new CTrackContainerEvtHandler(this))
    , m_AnnotLevel(-1)
    , m_Adaptive(true)
    , m_TrackInitFinished(true)
    , m_SkipGenuineCheck(false)
{
    SetConfigMgr(new CTrackConfigManager);
    x_RegisterIcon(SIconInfo(eIcon_Content, "Content", true, "track_content"));
    if (m_DS) {
       m_DS->SetJobListener(this);
    }
}


CTrackContainer::~CTrackContainer()
{
    if (m_DS) {
        DeleteAllJobs();
        m_DS->DeleteMetaDataJob();
    }
    delete m_Handler;
}

void CTrackContainer::OnMore()
{
    string category;
    if (!m_TrackProxies.empty())
        category = m_TrackProxies.front()->GetCategory();
    m_LTHost->LTH_ConfigureTracksDlg(category);
}

// returns true if container should be visible
void CTrackContainer::ToggleTracks(vector<string>& naAccessions)
{
    for (auto& p : m_TrackProxies) {
        CLayoutTrack* track = p->GetTrack();
        if (track) {
            CTrackContainer* cont = dynamic_cast<CTrackContainer*>(track);
            if (cont) {
                if (cont->x_HasToggleTracks()) {
                    if (!p->GetShown()) {
                        if (cont->x_HasToBeShown()) {
                            x_ShowTrack(track, true);
                            p->SetShown(true);
                        }
                    }
                    vector<string> naAccs;
                    cont->ToggleTracks(naAccs);
                    if (!naAccs.empty())
                        LoadNATracks(p, naAccs);
                }
            }
            else {
                switch (p->GetAction()) {
                case CTempTrackProxy::eMakeVisible:
                    if (!p->GetShown())
                        x_ShowTrack(track, true);
                    p->SetShown(true);
                    break;
                case CTempTrackProxy::eHide:
                    if (p->GetShown())
                        x_ShowTrack(track, false);
                    p->SetShown(false);
                    break;
                default:
                    break;
                }
            }
        }
        else if(p->IsNA()) {
            if (p->GetAction() == CTempTrackProxy::eMakeVisible) {
                p->SetShown(true);
                // LOG_POST(Trace << "ToggleTracks() adding shown accession: " << p->GetSource());
                naAccessions.push_back(p->GetSource());
            }
        }
        p->SetAction(CTempTrackProxy::eNone);
    }
    ResetSearch();
}

bool CTrackContainer::x_HasToggleTracks() const
{
    for (const auto& i : m_TrackProxies) {
        const CTrackContainer* cont
            = dynamic_cast<const CTrackContainer*>(i->GetTrack());
        if (cont) {
            if (cont->x_HasToggleTracks())
                return true;
        }
        else if (i->GetAction() != CTempTrackProxy::eNone)
            return true;
    }
    return false;
}

bool CTrackContainer::x_HasToBeShown() const
{
    for (const auto& i : m_TrackProxies) {
        const CTrackContainer* cont
            = dynamic_cast<const CTrackContainer*>(i->GetTrack());
        if (cont) {
            if (cont->x_HasToBeShown())
                return true;
        }
        else if (i->GetAction() == CTempTrackProxy::eMakeVisible)
            return true;
    }
    return false;
}

void CTrackContainer::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
    // Get html active areas for child tracks
    for (const auto& p : m_TrackProxies) {
        const CLayoutTrack* track = p->GetTrack();
        if (track  &&  track->IsOn()) {
            CHTMLActiveArea* area = track->InitHTMLActiveArea(p_areas);
            if (area) {
                string t_name = p->GetName();
                if (area->m_Signature.empty()) {
                    area->m_Signature = NStr::JsonEncode(t_name);
                } else {
                    // support multiple signatures including
                    // underlying object signature and track signature.
                    area->m_Signature = NStr::JsonEncode(t_name) + ";" + area->m_Signature;
                }
            }
        }
    }

    // Collect html active areas for non-track glyphs.
    GetGroup().GetHTMLActiveAreas(p_areas);
}


void CTrackContainer::DeleteAllJobs()
{
    CDataTrack::DeleteAllJobs();
    NON_CONST_ITERATE (TTrackProxies, iter, m_TrackProxies) {
        if ((*iter)->GetTrack()) {
            CDataTrack* data_track =
                dynamic_cast<CDataTrack*>((*iter)->GetTrack());
            if (data_track) {
                data_track->DeleteAllJobs();
            }
        }
    }
    SetTrackInitDone(true);
}


bool CTrackContainer::AllJobsFinished() const
{
    if (!IsTrackInitDone())
        return false;

    if (m_DS && !m_DS->AllJobsFinished())
        return false;

    for (const auto& p : m_TrackProxies) {
        if (!p->GetShown()) continue;

        const CLayoutTrack* t = p->GetTrack();
        if (!t) continue;

        const CDataTrack* data_track = dynamic_cast<const CDataTrack*>(t);
        if (data_track) {
            if (!data_track->AllJobsFinished())
                return false;
            continue;
        }

        const CTrackContainer* track_cont = dynamic_cast<const CTrackContainer*>(t);
        if (track_cont && !track_cont->AllJobsFinished())
            return false;
    }

    return true;
}


bool CTrackContainer::AllTracksCreated() const
{
    return IsTrackInitDone();
}


/// help functor for sorting CSeqGlyphs (known as CLayoutTrack) by order.
static bool s_TrackByOrder(const CRef<CSeqGlyph>& t1,
                        const CRef<CSeqGlyph>& t2)
{
    return dynamic_cast<const CLayoutTrack&>(*t1).GetOrder() <
        dynamic_cast<const CLayoutTrack&>(*t2).GetOrder();
}

void CTrackContainer::AddTrackProxy(CTempTrackProxy* proxy, bool upgateMsg)
{
    _ASSERT(proxy->GetOrder() >= 0);

    // LOG_POST(Trace << "AddTrackProxy(): source: " << proxy->GetSource() << ", display name: " << proxy->GetDisplayName() << ", order: " << proxy->GetOrder());
    // find out where we need to insert into the complete track list
    auto iter = m_TrackProxies.begin();
    if (iter == m_TrackProxies.end() || m_TrackProxies.back()->GetOrder() < proxy->GetOrder())
        iter = m_TrackProxies.end();
    else {
        while (iter != m_TrackProxies.end() && (*iter)->GetOrder() < proxy->GetOrder())
            ++iter;
    } 

    if (iter == m_TrackProxies.end() || (*iter)->GetOrder() > proxy->GetOrder())
        m_TrackProxies.emplace(iter, proxy);

    // add the track into the visible group if shown.
    CLayoutTrack* track = proxy->GetTrack();
    if (track) {
        // initialize the track
        track->SetHost(m_LTHost);
        track->SetConfig(m_gConfig);
        track->SetIndent(m_IndentLevel + 1);
        track->SetParentTrack(this);
        if (track->IsOn()) {
            Add(track);
            SortChildTracksByOrder();
        }
    }

    if (upgateMsg) x_UpdateMsg();
}


CTrackProxy* CTrackContainer::AddNewTrack(const string& track_key,
                                          const CTrackProxy::TAnnots& annots,
                                          const string& disp_name,
                                          const string& name,
                                          const string& filter,
                                          const string& sub_key)
{
    int order = 0;
    if ( !m_TrackProxies.empty() ) {
        order = m_TrackProxies.back()->GetOrder() + 1;
    }
    string track_name = name;
    if (track_name.empty()) {
        track_name = track_key;
        ITERATE (CTrackProxy::TAnnots, iter, annots) {
            track_name += NStr::Replace(*iter, ".", "_");
        }
        track_name = GetUniqueTrackName(track_name);
    }
    // LOG_POST(Trace << "AddNewTrack(): name: " << track_name << ", display name: " << disp_name << ", order: " << order);

    CTrackProxy* track = new CTrackProxy(order, track_name, true, track_key, sub_key);
    CRef<CTempTrackProxy> proxy(track);
    track->SetAnnots(annots);
    track->SetFilter(filter);
    track->SetDisplayName(disp_name);
    m_TrackProxies.push_back(proxy);
    x_ReinitTrack(track);
    return track;
}

void CTrackContainer::AddNewTrack(CTrackProxy* track)
{
    CRef<CTempTrackProxy> proxy(track);
    m_TrackProxies.push_back(proxy);
    x_ReinitTrack(track);
}

static string s_GetBestTrackTitle(const CTempTrackProxy* proxy,
                                  const CLayoutTrack* track)
{
    // The track title set externally has the highest priority
    if (proxy  &&  !proxy->GetDisplayName().empty() ) {
        return proxy->GetDisplayName();
    }

    // The track title generated when a track is created has the
    // second priority. The track title in this case may be the
    // default track title based on track type, or retrieved from
    // data.
    if (track  &&  !track->GetTitle().empty() ) {
        return track->GetTitle();
    }

    // No clue. It will be up to the track to decide later.
    return NcbiEmptyString;
}

CTrackContainer* CTrackContainer::x_ConvertToContainer(CTempTrackProxy* proxy)
{
    CRef<CLayoutTrack> track(proxy->GetTrack());
    CRef<CTempTrackProxy> childProxy;

    if (track) {
        CTrackContainer* cont = dynamic_cast<CTrackContainer*>(track.GetPointerOrNull());
        if (cont) return cont;

        string data_source =
            ILayoutTrackFactory::MergeAnnotAndFilter(
            proxy->GetSource(),
            proxy->GetFilter());

        // first, remove the original track from its parent.
        bool was_on = track->IsOn();
        CTrackProxy* contProxy = dynamic_cast<CTrackProxy*>(proxy);
        if (contProxy && contProxy->GetContShown()) {
            contProxy->SetContShown(false);
            was_on = true;
        }

        RemoveTrack(track->GetOrder(), false);
        if (was_on) proxy->SetShown(true);
        proxy->SetEmpty(false);

        childProxy = proxy->GetChildTempTrack(data_source);

        // copy over the potential modified profile
        childProxy->SetTrackProfile(proxy->GetTrackProfile());

        if(childProxy->IsNA() && !childProxy->GetDisplayName().empty())
            track->SetTitle(childProxy->GetDisplayName());
    }

    CRef<CTrackContainer> cont(new CTrackContainer(m_Context));
    cont->SetConfig(m_gConfig);
    cont->SetAnnotLevel(GetAnnotLevel());
    cont->SetTitle(s_GetBestTrackTitle(proxy, NULL), proxy->GetName());
    if(proxy->GetCategory() == "BLAST") {
        cont->SetIsBlast(true);
    }
    x_AddSubtrack(this, cont, proxy);

    if (childProxy)
        x_AddSubtrack(cont, track, childProxy);

    return cont.GetPointerOrNull();
}

static void s_UpdateTitle(CTempTrackProxy* proxy, CLayoutTrack* track)
{
    string title;
    if (proxy->IsNA())
        title = proxy->GetDisplayName();

    if (title.empty()) title = track->GetTitle();

    proxy->SetDisplayName(title);
    if (proxy->IsNA()) track->SetTitle(title);
}

void CTrackContainer::AddTracks(CTempTrackProxy* t_proxy, TTrackMap& tracks)
{
    _ASSERT(t_proxy);
    bool adjust_order = true;
    CRef<CLayoutTrack> original_track(t_proxy->GetTrack());
    if (original_track) {
        CTrackContainer* con_track =
            dynamic_cast<CTrackContainer*>(original_track.GetPointer());
        if (con_track) { // we already have a container track to hold the subtracks
            NON_CONST_ITERATE (TTrackMap, iter, tracks) {
                CLayoutTrack* track = iter->second;

                CRef<CTempTrackProxy> proxy = t_proxy->GetChildTempTrack(iter->first);
                s_UpdateTitle(proxy, track);

                string annot_name = ILayoutTrackFactory::ExtractAnnotName(iter->first);
                string filter_name = ILayoutTrackFactory::ExtractFilterName(iter->first);
                if ( !filter_name.empty() ) {
                    proxy->SetFilter(filter_name);
                }

                t_proxy->UpdateNA_ChildVisibility(*proxy);

                if ( !proxy->GetTrack() ) {
                    if ( !proxy->GetDisplayName().empty() ) {
                        track->SetTitle(proxy->GetDisplayName());
                    }
                    x_AddSubtrack(con_track, track, proxy);
                    track->Update(false);
                }
            }
        } else { // there was only one track originally
            string data_source =
                ILayoutTrackFactory::MergeAnnotAndFilter(
                    t_proxy->GetSource(),
                    t_proxy->GetFilter());

            _ASSERT( !data_source.empty() );
            TTrackMap::iterator t_iter = tracks.find(data_source);
            if (t_iter != tracks.end()) {
                CRef<CLayoutTrack> curr_track = t_iter->second;
                tracks.erase(t_iter);
                // We might need to use different track titles for this two
                // cases:
                //   1.  A track is the only child track of certain type
                //       of track, such as Alignment Tracks. In this case,
                //       we directly show this track.
                //   2.  A track is one of the 2 or more child tracks of
                //       a given type of track. In this case, we show a
                //       parent container track, and add all child tracks
                //       under the parent container track.
                if ( !tracks.empty() ) {
                    // There are other sibling tracks, we need to restore
                    // the track to whatever title set when it is created
                    // The original track title is t_iter->second->GetTitle()
                    // which may be empty depending on the data and track
                    // type.
                    original_track->SetTitle(curr_track->GetTitle());
                }
                // Otherwise, keep the original track title since there
                // is only one child track.
            }

            if ( !tracks.empty() ) { // now there are at least two subtracks
                CRef<CTrackContainer> cont(x_ConvertToContainer(t_proxy));

                // last add the newly created subtrack.
                NON_CONST_ITERATE (TTrackMap, iter, tracks) {
                    CLayoutTrack* sub_track = iter->second;
                    CRef<CTempTrackProxy> c_proxy = t_proxy->GetChildTempTrack(iter->first);
                    s_UpdateTitle(c_proxy, sub_track);

                    string annot_name = ILayoutTrackFactory::ExtractAnnotName(iter->first);
                    string filter_name = ILayoutTrackFactory::ExtractFilterName(iter->first);

                    if ( !filter_name.empty() ) {
                        c_proxy->SetFilter(filter_name);
                    }

                    t_proxy->UpdateNA_ChildVisibility(*c_proxy);

                    x_AddSubtrack(cont, sub_track, c_proxy);
                    sub_track->Update(false);
                }
            }
        }
    } else {
        CTrackProxy* track_proxy = dynamic_cast<CTrackProxy*>(t_proxy);
        CRef<CTrackContainer> cont(0);
        CGraphOverlay* go = 0;

        bool graph_overlay = track_proxy && track_proxy->GetKey() == "graph_overlay";
        if (graph_overlay) {

            TTrackMap::iterator go_it = tracks.find(track_proxy->GetAnnots().front());
            if (go_it == tracks.end())
                NCBI_THROW(CException, eUnknown, "Incorrect graph overlay initialization");
            go = dynamic_cast<CGraphOverlay*>(go_it->second.GetPointer());
            _ASSERT(go);
            if (!go)
                NCBI_THROW(CException, eUnknown, "Incorrect graph overlay initialization");
            go->SetConfig(m_gConfig);
            go->UpdateSource(t_proxy);
            if (tracks.size() > 1)
                tracks.erase(go_it);

            go->SetAnnotLevel(GetAnnotLevel());
            go->SetTitle(s_GetBestTrackTitle(t_proxy, NULL), t_proxy->GetName());
            if(t_proxy->GetCategory() == "BLAST") {
                go->SetIsBlast(true);
            }
            cont.Reset(go);
            x_AddSubtrack(this, cont, t_proxy);
        }
        else if (tracks.size() > 1) { // we need a container track
            cont.Reset(x_ConvertToContainer(t_proxy));
        }

        if (cont) {
            NON_CONST_ITERATE (TTrackMap, iter, tracks) {
                CLayoutTrack* track = iter->second;
                if (dynamic_cast<CGraphOverlay*>(track))
                    continue;

                string annot_name = ILayoutTrackFactory::ExtractAnnotName(iter->first);
                string filter_name = ILayoutTrackFactory::ExtractFilterName(iter->first);

                CRef<CTempTrackProxy> proxy = t_proxy->GetChildTempTrack(iter->first);
                s_UpdateTitle(proxy, track);

                proxy->SetSource(annot_name);
                if ( !filter_name.empty() ) {
                    proxy->SetFilter(filter_name);
                }

                if (IsOverlay()) {
                    dynamic_cast<CGraphOverlay*>(this)->UpdateTrackParams(iter->first);
                    track->SetShowTitle(false);
                } if (go) {
                    go->UpdateTrackParams(iter->first);
                    proxy->SetShown(true);
                    track->SetShowTitle(false);
                } else {
                    t_proxy->UpdateNA_ChildVisibility(*proxy);
                }
                x_AddSubtrack(cont, track, proxy);

            }
            if (IsOverlay() || go)
                adjust_order = false;
            if (go) {
                tracks.clear();
                tracks[track_proxy->GetAnnots().front()] = cont;
            }

            cont->Update(false);
        } else {
            CLayoutTrack* track = tracks.begin()->second;
            track->SetTitle(s_GetBestTrackTitle(t_proxy, track), t_proxy->GetName());
            if(t_proxy->GetCategory() == "BLAST") {
                track->SetIsBlast(true);
            }
            string annot_name = ILayoutTrackFactory::ExtractAnnotName(tracks.begin()->first);
            t_proxy->SetSource(annot_name);

            string filter_name = ILayoutTrackFactory::ExtractFilterName(tracks.begin()->first);
            if ( !filter_name.empty() ) {
                t_proxy->SetFilter(filter_name);
            }
            // make sure we use the correct profile
            // it is possible we are looking at a range within which
            // there is only one track.  There might be more than one
            // tracks when zoomming out.  So we are using the parent
            // tracks's profile, not the track's own profile.
            CRef<CTempTrackProxy> proxy = t_proxy->GetChildTempTrack(tracks.begin()->first);
            s_UpdateTitle(proxy, track);

            t_proxy->SetTrackProfile(proxy->GetTrackProfile());
            if (!proxy->GetShown()) {
                if (track_proxy)
                    track_proxy->SetContShown(track_proxy->GetShown());
                t_proxy->SetShown(false);
            }

            x_AddSubtrack(this, track, t_proxy);
            if (IsOverlay()) {
                CGraphOverlay* go = dynamic_cast<CGraphOverlay *>(this);
                if (go)
                    go->UpdateTrackParams(annot_name);
                track->SetShowTitle(false);
            }

            track->Update(false);
        }
    }
    if (adjust_order)
        CTrackConfigManager::AdjustTrackOrder(t_proxy);
}


void CTrackContainer::SetTrack(CLayoutTrack* track, int order, bool shown)
{
    // initialize the track
    track->SetHost(m_LTHost);
    track->SetConfig(m_gConfig);
    track->SetIndent(m_IndentLevel + 1);
    track->SetOrder(order);
    track->SetParentTrack(this);

    // find out where we need to insert into the complete track list
    TTrackProxies::iterator iter = m_TrackProxies.begin();
    while (iter != m_TrackProxies.end()  &&  (*iter)->GetOrder() != order) {
        ++iter;
    }

    if (iter != m_TrackProxies.end()) {
        if ((*iter)->GetTrack()) {
            SetGroup().Remove((*iter)->GetTrack());
        }
        (*iter)->SetTrack(track);
    } else if (track->IsMovable()) {
        /// we shouldn't do this
        _ASSERT(false);
        return;
    }

    // add the track into the visible group if shown.
    track->SetShow(shown);
    if (shown) {
        Add(track);
        SortChildTracksByOrder();
    }

    x_UpdateMsg();
}


void CTrackContainer::CloneTrack(int order, const string& track_title)
{
    // find the source track that we want to duplicate
    TTrackProxies::iterator iter = m_TrackProxies.begin();
    while (iter != m_TrackProxies.end()  &&  (*iter)->GetOrder() != order) {
        ++iter;
    }
    if (iter == m_TrackProxies.end()) return;
    CRef<CTempTrackProxy> src_proxy = *iter;

    // depending on if the souce track is explicitly specified in
    // configure file or it is just a derived sub-track of other such
    // track, we will insert the new track at a different place.
    CTrackContainer* p_track = this;
    if (src_proxy->IsTempTrack()) {
        // So we need to find out where it will be added and who its
        // parent/grandparent is.
        while (p_track->GetParentTrack()) {
            order = p_track->GetOrder();
            p_track = dynamic_cast<CTrackContainer*>(p_track->GetParentTrack());
            _ASSERT(p_track);
            iter = m_TrackProxies.begin();
            while (iter != m_TrackProxies.end()  &&  (*iter)->GetOrder() != order) {
                ++iter;
            }
            if (iter == m_TrackProxies.end()) return;

            if ( !(*iter)->IsTempTrack() )  {
                break;
            }
        }
    }

    // generate a unique track name for the new track
    string track_name = GetUniqueTrackName(src_proxy->GetName() + " clone");

    // create proxy for the new track
    CRef<CTempTrackProxy> new_proxy = (*iter)->Clone();
    CTrackProxy* full_proxy = dynamic_cast<CTrackProxy*>(new_proxy.GetPointer());

    if (src_proxy->IsTempTrack()) {
        // For derived track, track key(, subkey), and annotation
        // are not available in src_proxy. We need to extract them
        // from somewhere else.
        const CLayoutTrack* src_track = src_proxy->GetTrack();
        _ASSERT(src_track);
        string track_key = src_track->GetTypeInfo().GetId();
        full_proxy->SetKey(track_key);
        CTrackProxy::TAnnots annots;
        annots.push_back(src_proxy->GetSource());
        full_proxy->SetAnnots(annots);
        const ILayoutTrackFactory* factory =
            p_track->GetConfigMgr()->GetTrackFactory(track_key);
        factory->CloneTrack(src_track, full_proxy);
    }

    // initialize other info
    new_proxy->SetName(track_name);
    new_proxy->SetDisplayName(track_title);
    // expand the cloned track automatically
    new_proxy->SetExpanded(true);

    new_proxy->SetOrder(++order);
    ++iter;

    // update orders to make sure the newly added track appear right below
    // the source track.
    iter = p_track->m_TrackProxies.insert(iter, new_proxy);
    ++order;
    while (iter != p_track->m_TrackProxies.end()  &&  order > (*iter)->GetOrder()) {
        (*iter)->SetOrder(order);
        ++order;
        ++iter;
    }

    p_track->x_ReinitTrack(full_proxy);
}


void CTrackContainer::RenameChildTrack(int order, const string& new_name)
{
    TTrackProxies::iterator iter = m_TrackProxies.begin();
    while (iter != m_TrackProxies.end()  &&  (*iter)->GetOrder() != order) {
        ++iter;
    }
    if (iter != m_TrackProxies.end()) {
        (*iter)->SetDisplayName(new_name);
        (*iter)->GetTrack()->SetTitle(new_name);
    }
}


void CTrackContainer::RemoveTrack(int order, bool immediately)
{
    TTrackProxies::iterator iter = m_TrackProxies.begin();
    while (iter != m_TrackProxies.end()  &&  (*iter)->GetOrder() != order) {
        ++iter;
    }
    if (iter != m_TrackProxies.end()) {
        if ((*iter)->GetTrack()) {
            (*iter)->GetTrack()->SaveProfile();
            SetGroup().Remove((*iter)->GetTrack());
        }
        (*iter)->SetShown(false);
        if (immediately) {
            m_TrackProxies.erase(iter);
        } else {
            // mark it as empty track in order to remove it
            // completely from GUI registry.
            (*iter)->SetEmpty(true);
        }
        x_UpdateLayout();
    }
}


void CTrackContainer::CloseTrack(int order)
{
    TTrackProxies::iterator iter = m_TrackProxies.begin();
    while (iter != m_TrackProxies.end()  &&  (*iter)->GetOrder() != order) {
        ++iter;
    }
    if (iter != m_TrackProxies.end()) {
        CDataTrack* data_track = dynamic_cast<CDataTrack*>((*iter)->GetTrack());
        if (data_track) {
            data_track->DeleteAllJobs();
        }
        SetGroup().Remove((*iter)->GetTrack());
        (*iter)->SetShown(false);
        x_UpdateMsg();
        SetGroup().UpdateLayout();
    }
}


void CTrackContainer::MoveUp(int order)
{
    // get the track order for the track above the moved track
    CLayoutGroup::TObjectList::iterator iter = SetChildren().begin();
    CLayoutGroup::TObjectList::iterator p_iter = iter;
    while (iter != SetChildren().end()  &&
        dynamic_cast<const CLayoutTrack&>(**iter).GetOrder() != order) {
        p_iter = iter;
        ++iter;
    }
    if (iter == SetChildren().end()  ||  iter == p_iter) return;
    int pre_order = dynamic_cast<const CLayoutTrack&>(**p_iter).GetOrder();

    // get track proxy for the moved track (with track_order = order)
    TTrackProxies::iterator proxy_iter = m_TrackProxies.begin();
    while (proxy_iter != m_TrackProxies.end()  &&
        (*proxy_iter)->GetOrder() != order) {
        ++proxy_iter;
    }

    // get track proxy for the previous track  (with track_order = pre_order)
    TTrackProxies::iterator pre_proxy_iter = m_TrackProxies.begin();
    while (pre_proxy_iter != m_TrackProxies.end()  &&
        (*pre_proxy_iter)->GetOrder() != pre_order) {
        ++pre_proxy_iter;
    }

    _ASSERT(proxy_iter != m_TrackProxies.end()  &&
        pre_proxy_iter != m_TrackProxies.end());

    // swap track orders
    (*pre_proxy_iter)->SetOrder(order);
    if ((*pre_proxy_iter)->GetTrack())
        (*pre_proxy_iter)->GetTrack()->SetOrder(order);

    (*proxy_iter)->SetOrder(pre_order);
    if ((*proxy_iter)->GetTrack())
        (*proxy_iter)->GetTrack()->SetOrder(pre_order);

    // sort both track proxies and tracks
    m_TrackProxies.sort(CTrackProxy::STPSorterByOrder::s_CompareCRefs);
    SortChildTracksByOrder();
}


void CTrackContainer::MoveDown(int order)
{
    // get the track order for the track below the moved track
    CLayoutGroup::TObjectList::reverse_iterator iter = SetChildren().rbegin();
    CLayoutGroup::TObjectList::reverse_iterator n_iter = iter;
    while (iter != SetChildren().rend()  &&
        dynamic_cast<const CLayoutTrack&>(**iter).GetOrder() != order) {
        n_iter = iter;
        ++iter;
    }
    if (iter == SetChildren().rend()  ||  iter == n_iter) return;
    int n_order = dynamic_cast<const CLayoutTrack&>(**n_iter).GetOrder();

    // get track proxy for the moved track (with track_order = order)
    TTrackProxies::reverse_iterator proxy_iter = m_TrackProxies.rbegin();
    while (proxy_iter != m_TrackProxies.rend()  &&
        (*proxy_iter)->GetOrder() != order) {
        ++proxy_iter;
    }

    // get track proxy for the next track  (with track_order = pre_order)
    TTrackProxies::reverse_iterator n_proxy_iter = m_TrackProxies.rbegin();
    while (n_proxy_iter != m_TrackProxies.rend()  &&
        (*n_proxy_iter)->GetOrder() != n_order) {
        ++n_proxy_iter;
    }

    _ASSERT(proxy_iter != m_TrackProxies.rend()  &&
        n_proxy_iter != m_TrackProxies.rend());

    // swap track orders
    (*n_proxy_iter)->SetOrder(order);
    if ((*n_proxy_iter)->GetTrack())
        (*n_proxy_iter)->GetTrack()->SetOrder(order);
    (*proxy_iter)->SetOrder(n_order);
    if ((*proxy_iter)->GetTrack())
        (*proxy_iter)->GetTrack()->SetOrder(n_order);

    // sort both track proxies and tracks
    m_TrackProxies.sort(CTrackProxy::STPSorterByOrder::s_CompareCRefs);
    SortChildTracksByOrder();
}


void CTrackContainer::MoveTrackToTop(int order)
{
    // get track proxy for the moved track (with track_order = order)
    TTrackProxies::reverse_iterator proxy_iter = m_TrackProxies.rbegin();
    while (proxy_iter != m_TrackProxies.rend()  &&
        (*proxy_iter)->GetOrder() != order) {
        ++proxy_iter;
    }

    TTrackProxies::reverse_iterator next_track(proxy_iter);
    if (proxy_iter != m_TrackProxies.rend()) ++next_track;
    while (next_track != m_TrackProxies.rend()) {
        (*proxy_iter)->SetOrder((*next_track)->GetOrder());
        if ((*proxy_iter)->GetTrack())
            (*proxy_iter)->GetTrack()->SetOrder((*next_track)->GetOrder());
        (*next_track)->SetOrder(order);
        if ((*next_track)->GetTrack())
            (*next_track)->GetTrack()->SetOrder(order);
        order = (*proxy_iter)->GetOrder();
        ++next_track;
    }
    // sort both track proxies and tracks
    m_TrackProxies.sort(CTrackProxy::STPSorterByOrder::s_CompareCRefs);
    SortChildTracksByOrder();
    x_UpdateLayout();
}


void CTrackContainer::MoveTrackToBottom(int order)
{
    // get track proxy for the moved track (with track_order = order)
    TTrackProxies::iterator proxy_iter = m_TrackProxies.begin();
    while (proxy_iter != m_TrackProxies.end()  &&
        (*proxy_iter)->GetOrder() != order) {
        ++proxy_iter;
    }

    TTrackProxies::iterator next_track(proxy_iter);
    if (proxy_iter != m_TrackProxies.end()) ++next_track;
    while (next_track != m_TrackProxies.end()) {
        (*proxy_iter)->SetOrder((*next_track)->GetOrder());
        if ((*proxy_iter)->GetTrack())
            (*proxy_iter)->GetTrack()->SetOrder((*next_track)->GetOrder());

        (*next_track)->SetOrder(order);
        if ((*next_track)->GetTrack())
            (*next_track)->GetTrack()->SetOrder(order);

        order = (*proxy_iter)->GetOrder();
        ++next_track;
    }
    // sort both track proxies and tracks
    m_TrackProxies.sort(CTrackProxy::STPSorterByOrder::s_CompareCRefs);
    SortChildTracksByOrder();
    x_UpdateLayout();
}


void CTrackContainer::MoveTrack(int from, int to)
{
    // get track proxy for the moved track (with track_order = order)
    TTrackProxies::iterator proxy_from = m_TrackProxies.begin();
    while (proxy_from != m_TrackProxies.end() &&
           (*proxy_from)->GetOrder() != from) {
        ++proxy_from;
    }
    if (proxy_from == m_TrackProxies.end())
        return;
    TTrackProxies::iterator proxy_to = m_TrackProxies.begin();
    while (proxy_to != m_TrackProxies.end() &&
           (*proxy_to)->GetOrder() != to) {
        ++proxy_to;
    }
    while (proxy_to != m_TrackProxies.end()) {
        int new_order = (*proxy_to)->GetOrder() + 1;
        (*proxy_to)->SetOrder(new_order);
        if (!(*proxy_to)->TrackNotSet()) {
            (*proxy_to)->GetTrack()->SetOrder(new_order);
        }
        ++proxy_to;
    }
    (*proxy_from)->SetOrder(to);
    if (!(*proxy_from)->TrackNotSet()) {
        (*proxy_from)->GetTrack()->SetOrder(to);
    }
    // sort both track proxies and tracks
    m_TrackProxies.sort(CTrackProxy::STPSorterByOrder::s_CompareCRefs);
    SortChildTracksByOrder();
    x_UpdateLayout();
}


bool CTrackContainer::HasSubtracks() const
{
    bool has_any = !GetChildren().empty();
    TTrackProxies::const_iterator iter = m_TrackProxies.begin();
    while(!has_any  &&  iter != m_TrackProxies.end()) {
        has_any = ((*iter)->GetTrack() != NULL);
        ++iter;
    }
    return has_any;
}


bool CTrackContainer::NoSubtrackEver() const
{
    bool no_track = GetChildren().empty();
    CRef<CSGSequenceDS> seq_ds = m_Context->GetSeqDS();
    int seq_len = seq_ds->GetSequenceLength();
    int off = (int)(seq_len * 0.0001);
    TSeqRange total_range(off, seq_len - off);
    TTrackProxies::const_iterator iter = m_TrackProxies.begin();
    while(no_track  &&  iter != m_TrackProxies.end()) {
        no_track =
            (*iter)->IsEmpty()  ||
            ((*iter)->GetTrack() == NULL  &&
            dynamic_cast<const CTrackProxy*>(iter->GetPointer())  &&
            (*iter)->GetVisitedRange().IntersectionWith(total_range) == total_range);
        ++iter;
    }
    return no_track;
}


void CTrackContainer::SaveTrackStates(int order)
{
    TTrackProxies::iterator iter = m_TrackProxies.begin();
    while (iter != m_TrackProxies.end()  &&  (*iter)->GetOrder() != order) {
        ++iter;
    }
    if (iter != m_TrackProxies.end()) {
        const CLayoutTrack* track = (*iter)->GetTrack();
        if (track) {
            (*iter)->SetExpanded(track->IsExpanded());
            (*iter)->SetShown(track->IsOn());
        }
    }
}


void CTrackContainer::ClearTracks()
{
    SetHeight(0);
    // canceling all unfinished jobs if any
    DeleteAllJobs();
    SetGroup().Clear();
    m_TrackProxies.clear();
}


string CTrackContainer::GetUniqueTrackName(const string& name) const
{
    size_t length = name.length(), post_fix = 0;
    for (const auto& p : m_TrackProxies) {
        const string& n = p->GetName();
        if (n.length() <= length || n.substr(0, length) != name)
            continue;

        size_t tmp =  NStr::StringToNumeric<size_t>(n.substr(length), NStr::fConvErr_NoThrow);
        if (errno == 0 && tmp >= post_fix)
            post_fix = tmp + 1;
    }
    return name + NStr::NumericToString(post_fix);
}


CTrackContainer*
CTrackContainer::GetChildContainerTrack(const string& name)
{
    NON_CONST_ITERATE (TTrackProxies, iter, m_TrackProxies) {
        CTrackContainer* track =
            dynamic_cast<CTrackContainer*>(iter->GetPointer());
        if (track  &&  track->GetTitle() == name) {
            return track;
        }
    }
    return NULL;
}


const CTrackTypeInfo& CTrackContainer::GetTypeInfo() const
{
    return m_TypeInfo;
}


CRef<CSGGenBankDS>  CTrackContainer::GetDataSource()
{
    return CRef<CSGGenBankDS>(m_DS.GetPointer());
}


CConstRef<CSGGenBankDS>  CTrackContainer::GetDataSource() const
{
    return CConstRef<CSGGenBankDS>(m_DS.GetPointer());
}


void CTrackContainer::ExpandAll(bool expand)
{
    NON_CONST_ITERATE (TTrackProxies, iter, m_TrackProxies) {
        CLayoutTrack* track = (*iter)->GetTrack();
        if (track) {
            CTrackContainer* cont = dynamic_cast<CTrackContainer*>(track);
            if (cont) {
                cont->ExpandAll(expand);
            }
            track->Expand(expand);
        }
    }
}


void CTrackContainer::ShowAll(bool shown, bool recursive)
{
    NON_CONST_ITERATE (TTrackProxies, iter, m_TrackProxies) {
        CTempTrackProxy* proxy = *iter;
        CLayoutTrack* track = proxy->GetTrack();
        if (track) {
            if (shown != proxy->GetShown()) {
                x_ShowTrack(track, shown);
            }
            if (recursive) {
                // If the track is a track container track, do the same
                // for its children.
                CTrackContainer* c_track =
                    dynamic_cast<CTrackContainer*>(iter->GetPointer());
                if (c_track) {
                    c_track->ShowAll(shown, recursive);
                }
            }
        } else {
            CTrackProxy* t_proxy = dynamic_cast<CTrackProxy*>(proxy);
            if (t_proxy  &&  shown != t_proxy->GetShown()) {
                t_proxy->SetShown(shown);
                if (shown) x_ReinitTrack(t_proxy);
            }
            if (recursive) {
                NON_CONST_ITERATE(TTrackProxies, t_iter, proxy->GetChildren()) {
                    t_proxy->SetShown(shown);
                }
            }
        }
    }
}


void CTrackContainer::SortChildTracksByOrder()
{
    SetChildren().sort(s_TrackByOrder);
}


void CTrackContainer::RemoveEmptyTracks()
{
    TTrackProxies::iterator iter = m_TrackProxies.begin();
    CRef<CSGSequenceDS> seq_ds = m_Context->GetSeqDS();
    int seq_len = seq_ds->GetSequenceLength();

    // when users
    int off = min((int)(seq_len * 0.0001), 100);
    TSeqRange total_range(off, seq_len - off);
    while (iter != m_TrackProxies.end()) {
        CTempTrackProxy* proxy = *iter;
        // HACK, HACK
        // we don't want to remove empty track_container
        // if it is not temporary track.
        const CTrackProxy* full_proxy =
            dynamic_cast<const CTrackProxy*>(proxy);
        if (full_proxy  &&  full_proxy->GetKey() ==
            CTrackContainerFactory::GetTypeInfo().GetId()) {
                ++iter;
                continue;
        }

        CLayoutTrack* track = proxy->GetTrack();

        // TODO: need to figure out a way to check if a track is
        // indeed an empty track. Currently, we only remove the
        // empty track containers.
        CTrackContainer* c_track = dynamic_cast<CTrackContainer*>(track);
        if (c_track) {
            if (c_track->NoSubtrackEver()) {
                if (c_track->IsOn()) {
                    SetGroup().Remove(c_track);
                    SetGroup().UpdateBoundingBox();
                }
                iter = m_TrackProxies.erase(iter);
            }
            else {
                ++iter;
            }
        } else if (track == NULL  &&  full_proxy  &&
            proxy->GetVisitedRange().IntersectionWith(total_range) == total_range) {
                iter = m_TrackProxies.erase(iter);
        } else {
            ++iter;
        }
    }
}

static bool s_GetMenuTracks(
    CTempTrackProxy::TTrackProxies& dst_proxies,
    const CTempTrackProxy::TTrackProxies& src_proxies)
{
    CTempTrackProxy::TTrackProxies visible, hidden;

    if (src_proxies.size() <= kMaxMenuTracks) {
        for (const auto& t : src_proxies) {
            // we will hide the tracks which are either empty
            // (no data in the whole renage), or temporary and
            // no data so far within the visited ranges.
            if (t->IsEmpty() ||
                (!dynamic_cast<const CTrackProxy*>(t.GetPointer()) &&
                !t->GetTrack() && !t->IsNA())) {
                continue;
            }
            dst_proxies.push_back(t);
        }
        return false;
    }

    for (const auto& t : src_proxies) {
        // we will hide the tracks which are either empty
        // (no data in the whole range), or temporary and
        // no data so far within the visited ranges.
        if (t->IsEmpty() ||
            (!dynamic_cast<const CTrackProxy*>(t.GetPointer()) &&
            !t->GetTrack() && !t->IsNA())) {
            continue;
        }

        if (t->GetShown())
            visible.push_back(t);
        else
            hidden.push_back(t);
    }

    size_t count = 0;
    for (const auto& t : visible) {
        if (count == kMaxMenuTracks)
            break;
         dst_proxies.push_back(t);
         ++count;
    }

    for (const auto& t : hidden) {
        if (count == kMaxMenuTracks)
            break;
        dst_proxies.push_back(t);
        ++count;
    }

    return (visible.size() + hidden.size() > kMaxMenuTracks);
}

void CTrackContainer::OnSelectAll()
{
    CTempTrackProxy::TTrackProxies menuTracks;
    s_GetMenuTracks(menuTracks, m_TrackProxies);

    vector<string> naAccessions;

    for (auto& t : menuTracks) {
        if (!t->GetTrack()) {
            CTrackProxy* t_proxy = dynamic_cast<CTrackProxy*>(t.GetPointer());
            if (t_proxy) { // track is container
                t->SetShown(true);
                x_ReinitTrack(t_proxy);
            }
            else if (t->IsNA()) {
                t->SetShown(true);
                naAccessions.push_back(t->GetSource());
            }
        }
        else if (!t->GetShown()) {
            x_ShowTrack(t->GetTrack(), true);
        }
    }

    if (!naAccessions.empty()) {
        CTrackContainer* parent = dynamic_cast<CTrackContainer*>(GetParentTrack());
        if (parent)
            parent->LoadNATracks(this, naAccessions);
    }
}

void CTrackContainer::OnDeselectAll()
{
    CTempTrackProxy::TTrackProxies menuTracks;
    s_GetMenuTracks(menuTracks, m_TrackProxies);
    for (auto& t : menuTracks) {
        if (!t->GetTrack()) continue;
        if (t->GetShown())
            x_ShowTrack(t->GetTrack(), false);
    }
}

void CTrackContainer::OnRevertSelection()
{
    CTempTrackProxy::TTrackProxies menuTracks;
    s_GetMenuTracks(menuTracks, m_TrackProxies);

    vector<string> naAccessions;

    for (auto& t : menuTracks) {
        if (!t->GetTrack() && t->IsNA()) {
            t->SetShown(true);
            naAccessions.push_back(t->GetSource());
        }
        else {
            x_ShowTrack(t->GetTrack(), !t->GetShown());
        }
    }

    if (!naAccessions.empty()) {
        CTrackContainer* parent = dynamic_cast<CTrackContainer*>(GetParentTrack());
        if (parent)
            parent->LoadNATracks(this, naAccessions);
    }
}

CRef<CTempTrackProxy> CTrackContainer::x_GetMenuTrack(int index)
{
    if (index < 0)
        return CRef<CTempTrackProxy>();

    CTempTrackProxy::TTrackProxies menuTracks;
    s_GetMenuTracks(menuTracks, m_TrackProxies);

    auto it = menuTracks.begin();
    while (it != menuTracks.end() && index-- != 0)
        ++it;

    if (it == menuTracks.end())
        return CRef<CTempTrackProxy>();

    return *it;
}

void CTrackContainer::OnMenuToggleTrack(int index)
{
    CRef<CTempTrackProxy> proxy = x_GetMenuTrack(index);
    if (proxy)
        OnToggleTrack(proxy->GetOrder());
}

void CTrackContainer::OnMenuTrackSettings(int index)
{
    CRef<CTempTrackProxy> proxy = x_GetMenuTrack(index);
    if (proxy)
        OnTrackSettings(proxy->GetOrder());
}

void CTrackContainer::OnToggleTrack(int order)
{
    TTrackProxies::iterator t_iter = m_TrackProxies.begin();
    while (t_iter != m_TrackProxies.end()) {
        CRef<CTempTrackProxy> proxy = *t_iter;
        if (proxy->GetOrder() == order) {
            if (proxy->GetTrack()) {
                x_ShowTrack(proxy->GetTrack(), !proxy->GetShown());
            } else {
                CTrackProxy* t_proxy = dynamic_cast<CTrackProxy*>(proxy.GetPointer());
                if (t_proxy) {
                    t_proxy->SetShown(!proxy->GetShown());
                    if (proxy->GetShown()) {
                        x_ReinitTrack(t_proxy);
                    }
                } else {
                    if (proxy->IsNA()) {
                        proxy->SetShown(true);
                        CTrackContainer* parent = dynamic_cast<CTrackContainer*>(GetParentTrack());
                        if (parent)
                            parent->LoadNATracks(this, vector<string>(1, proxy->GetSource()));
                    }
                }
            }
            break;
        }
        ++t_iter;
    }
}


void CTrackContainer::OnTrackSettings(int order)
{
}

bool CTrackContainer::x_Empty() const
{
    ITERATE (TTrackProxies, iter, m_TrackProxies) {
        if ((*iter)->GetTrack() != NULL  &&  (*iter)->GetTrack()->IsOn()) {
            return false;
        }
    }
    return true;
}


void CTrackContainer::x_UpdateData()
{
    if (m_DS) {
        if ( !m_DS->AllJobsFinished() ) {
            m_DS->DeleteAllJobs();
            SetTrackInitDone(true);
        }
        x_SetStartStatus();
    }

    // make sure we keep the track info up to date
    // E.g. the track titles get changed to more meaningful ones
    // when actual data get loaded.
    x_UpdateMsg();
    CGlyphContainer::Update(false);

    // check if we need to reinitialize the sub tracks.
    if (GetConfigMgr()) {
        x_ReinitSubtracks();
    }
}


void CTrackContainer::x_OnJobCompleted(CAppJobNotification& notify)
{
    m_DS->ClearJobID(notify.GetJobID());
    CRef<CObject> res_obj = notify.GetResult();
    CTrackInitJobResult* result = dynamic_cast<CTrackInitJobResult*>(&*res_obj);
    if (!result) {
        LOG_POST(Error << "CTrackContainer::x_OnJobCompleted() notification \
                          for job does not contain results.");
        return;
    }

    TTrackMap& tracks = result->m_Tracks;
    //CTrackContainer* parent = result.m_ParentTrack;
    CTempTrackProxy*  t_proxy = result->m_TrackParams;

    if ( !tracks.empty() ) {
        if (CTrackContainer* c_track =
            dynamic_cast<CTrackContainer*>(tracks.begin()->second.GetPointer())) {
            c_track->SetConfigMgr(GetConfigMgr());
            c_track->SetSubtrackProxies(t_proxy->GetChildren());
        }
    }

    // update the visited range by including the previous visited range
    // if both intersect to each other
    TSeqRange vis_range = m_Context->GetVisSeqRange();
    if (vis_range.IntersectingWith(t_proxy->GetVisitedRange())) {
        vis_range.CombineWith(t_proxy->GetVisitedRange());
    }
    t_proxy->SetVisitedRange(vis_range);
    t_proxy->SetRealized(true);

    bool need_update = false;
    if ( !tracks.empty() ) {
        need_update = true;
        AddTracks(t_proxy, tracks);
        SetMsg(", Initializing subtracks..., Finish initializing "
            + t_proxy->GetName());
    }

    if (!m_DS->AllJobsFinished()  &&  need_update) {
        x_UpdateLayout();
    }
}

void CTrackContainer::x_CreateTracksMenu(wxMenu& menu, TIconID id)
{
    CTempTrackProxy::TTrackProxies menuTracks;
    bool needMore = s_GetMenuTracks(menuTracks, m_TrackProxies);

    int index = -1;
    for (const auto& proxy : menuTracks) {
        ++index;
        string item_name = proxy->GetDisplayName();
        if (item_name.empty()) {
            item_name = proxy->GetName();
        }
        string help_text = item_name;
        if (proxy->GetTrack()) {
            if (proxy->GetShown()  &&  !proxy->GetTrack()->GetFullTitle().empty() ) {
                help_text = proxy->GetTrack()->GetFullTitle();
                if (help_text.length() < kMaxNameLen) {
                    item_name = help_text;
                } else {
                    item_name = help_text.substr(0, kMaxNameLen);
                    item_name += "...";
                }
            }
            const CTrackContainer* cont =
                dynamic_cast<const CTrackContainer*>(proxy->GetTrack());
            if (cont  &&  !cont->HasSubtracks()) {
                if (cont->IsOn()) {
                    item_name += " (no data found so far)";
                } else {
                    item_name += " (potential)";
                }
            }
        } else if (proxy->GetShown()) {
            item_name += " (no data found so far)";
        } else {
            item_name += " (potential)";
        }

        wxMenuItem* item = 0;
        switch (id) {
        case eIcon_Content:
            item = menu.AppendCheckItem(
                kBaseID + index, ToWxString(item_name), ToWxString(help_text));
            if (proxy->GetShown()) {
                item->Check();
            }
            break;
        case eIcon_Settings:
            item = menu.Append(kBaseID_Settings + index, ToWxString(item_name), ToWxString(help_text));
            break;
        default:
            _ASSERT(0);
            break;
        }

        if (!item)
            continue;

        if (proxy->GetTrack()  && !proxy->GetTrack()->IsCollapsible() ) {
            item->Enable(false);
        }
    }

    if (needMore)
        menu.Append(kBaseID - 4, wxT("More..."));
}

void CTrackContainer::x_OnIconClicked(TIconID id)
{
    if (id == eIcon_Content) {
        wxMenu menu;
        bool all_on = true;
        bool all_off = true;
        TTrackProxies::const_iterator iter = m_TrackProxies.begin();
        while ((all_on  ||  all_off)  &&  iter != m_TrackProxies.end()) {
            if ((*iter)->GetShown()) {
                all_off = false;
            } else {
                all_on = false;
            }
            ++iter;
        }

        wxMenuItem* item = menu.Append(kBaseID - 3, wxT("Select all"));
        if (all_on) item->Enable(false);
        item = menu.Append(kBaseID - 2, wxT("Deselect all"));
        if (all_off) item->Enable(false);
        menu.Append(kBaseID - 1, wxT("Revert selections"));
        menu.AppendSeparator();
        x_CreateTracksMenu(menu, id);
        m_LTHost->LTH_PushEventHandler(m_Handler);
        m_LTHost->LTH_PopupMenu(&menu);
        m_LTHost->LTH_PopEventHandler();
    } else if (id == eIcon_Settings) {
        wxMenu menu;
        x_CreateTracksMenu(menu, id);
        m_LTHost->LTH_PushEventHandler(m_Handler);
        m_LTHost->LTH_PopupMenu(&menu);
        m_LTHost->LTH_PopEventHandler();

    } else {
        // use default handlers
        CLayoutTrack::x_OnIconClicked(id);
    }
    x_OnLayoutChanged();
}


void CTrackContainer::x_OnAllJobsFinished()
{
    SetTrackInitDone(true);
    ITrackInitObserver* observer =
        dynamic_cast<ITrackInitObserver*>(GetParentTrack());
    if (observer) {
        observer->OnTrackInitFinished(this);
    }
    if (CSGDataSource::IsBackgroundJob()) {
        RemoveEmptyTracks();
        x_UpdateMsg();
    }
    x_UpdateLayout();
}


void CTrackContainer::x_ReinitSubtracks()
{
    NON_CONST_ITERATE (TTrackProxies, t_iter, m_TrackProxies) {
        CTrackProxy* t_proxy = dynamic_cast<CTrackProxy*>(t_iter->GetPointer());
        if (!t_proxy) {
            break;
        }
        if (t_proxy->GetShown()) {
            x_ReinitTrack(t_proxy);
        }
    }
    NON_CONST_ITERATE(TTrackProxies, t_iter, m_TrackProxies) {
        CTrackProxy* t_proxy = dynamic_cast<CTrackProxy*>(t_iter->GetPointer());
        if (!t_proxy) {
            break;
        }
        if (!t_proxy->GetShown()) {
            x_ReinitTrack(t_proxy);
        }
    }
}

void CTrackContainer::LoadNATracks(CTempTrackProxy* t_proxy, const vector<string>& accessions)
{
    const ILayoutTrackFactory* factory =
        GetConfigMgr()->GetTrackFactory(t_proxy->GetKey());

    // those tracks without background initialization have
    // been initialized already somewhere else.
    if (factory->NeedBackgroundInit() || !t_proxy->GetTrack()) {
        ILayoutTrackFactory::SExtraParams params(m_AnnotLevel,
            m_Adaptive, &t_proxy->GetAnnots(),
            t_proxy->GetSubkey(), t_proxy->GetFilter(), t_proxy->GetSortBy());
        params.m_SkipGenuineCheck = true;
        params.m_Annots.assign(accessions.begin(), accessions.end());
        params.m_SubTracks = t_proxy->GetSubTracks();
        params.m_RemotePath = t_proxy->GetRemotePath();

        m_DS->CreateTrack("creating " + t_proxy->GetName() + " track",
            factory, this, t_proxy, params);

        if (CSGDataSource::IsBackgroundJob())
            SetTrackInitDone(false);
    }
}

void CTrackContainer::x_ReinitTrack(CTrackProxy* t_proxy)
{
    const TSeqRange& curr_range = m_Context->GetVisSeqRange();
    // visited range covers the current visible range,
    // no need to reinitialize the track.
    if (t_proxy->GetVisitedRange().IntersectionWith(curr_range) ==  curr_range) {
        return;
    }

    const ILayoutTrackFactory* factory =
        GetConfigMgr()->GetTrackFactory(t_proxy->GetKey());

    // those tracks without background initialization have
    // been initialized already somewhere else.
    if (factory->NeedBackgroundInit()  ||  !t_proxy->GetTrack()) {
        ILayoutTrackFactory::SExtraParams params(m_AnnotLevel,
            m_Adaptive, &t_proxy->GetAnnots(),
            t_proxy->GetSubkey(), t_proxy->GetFilter(), t_proxy->GetSortBy());
        params.m_SkipGenuineCheck = GetSkipGenuineCheck();
        params.m_SubTracks = t_proxy->GetSubTracks();
        params.m_RemotePath = t_proxy->GetRemotePath();

        m_DS->CreateTrack("creating " + t_proxy->GetName() + " track",
            factory, this, t_proxy, params);

        // make sure we only do this for background jobs
        if (CSGDataSource::IsBackgroundJob()) {
            SetTrackInitDone(false);
            SetMsg(", Initializing subtracks...");
            CGlyphContainer::Update(false);
        }
    }
    else {
        CTrackContainer* cont = dynamic_cast<CTrackContainer*>(t_proxy->GetTrack());
        if (cont) cont->x_ReinitSubtracks();
    }
}

void CTrackContainer::InitNATracks(const TAnnotMetaDataList& annots, bool makeContainersVisible)
{
    TAnnotMetaDataList copyAnnots = annots;

    for (auto& i : m_TrackProxies) {
        CTrackProxy* t_proxy = dynamic_cast<CTrackProxy*>(i.GetPointer());
        if (t_proxy) x_ReinitNATrack(t_proxy, copyAnnots, makeContainersVisible);
    }
}

void CTrackContainer::x_ReinitNATrack(CTempTrackProxy* t_proxy, TAnnotMetaDataList& annots, bool makeContainersVisible)
{
    string key = t_proxy->GetKey();
    if (key == "sequence_track")
        return;

    const ILayoutTrackFactory* factory = GetConfigMgr()->GetTrackFactory(key);
    if (!factory)
        return;

    if (GetAnnotLevel() != 0 && !GetAdaptive())
        return;

    if (factory->NeedBackgroundInit()) {
        ILayoutTrackFactory::TAnnotNameTitleMap knownAnnots;
        ILayoutTrackFactory::SExtraParams params(GetAnnotLevel(),
            m_Adaptive, &t_proxy->GetAnnots(),
            t_proxy->GetSubkey(), t_proxy->GetFilter(), t_proxy->GetSortBy());
        params.m_SkipGenuineCheck = true;

        factory->GetMatchedAnnots(annots, params, knownAnnots);
        if (knownAnnots.empty())
            return;

        CTrackConfigManager::TAnnotMetaDataList known;

        for (auto it = annots.begin(); it != annots.end();) {
            auto it2 = knownAnnots.find(it->first);
            if (it2 != knownAnnots.end()) {
                known.insert(*it);
                it = annots.erase(it);
            }
            else
                ++it;
        }

        CTrackContainer* cont = x_ConvertToContainer(t_proxy);
        vector<string> load;

        for (auto& p : t_proxy->GetChildren()) {
            auto it = known.find(p->GetSource());
            if (it != known.end()) {
                if (!p->GetTrack()) {
                    p->SetNAdata(*it->second);
                    cont->AddTrackProxy(p, false);
                    if (p->GetShown())
                        load.push_back(it->first);
                }
                known.erase(it);
            }
        }

        for (auto& i : known) {
            string name_modified(i.first);
            NStr::ReplaceInPlace(name_modified, ".", "_");
            CRef<CTempTrackProxy> proxy = t_proxy->AddTempTrack(name_modified, i.second->m_Title);
            proxy->SetNAdata(*i.second);
            cont->AddTrackProxy(proxy, false);
            proxy->SetShown(i.second->m_Shown);
            if (proxy->GetShown())
                load.push_back(i.first);
        }

        cont->x_UpdateMsg();

        if (!load.empty()) {
            LoadNATracks(t_proxy, load);
            if (makeContainersVisible)
                x_ShowTrack(t_proxy->GetTrack(), true);
        }
    }
    else {
        CTrackContainer* cont = dynamic_cast<CTrackContainer*>(t_proxy->GetTrack());
        if (cont) {
            cont->ReinitNASubtracks(annots, makeContainersVisible);
        }
    }
}


void CTrackContainer::LoadNATracks(CDataTrack* child, const vector<string>& naAccessions)
{
    for (auto& p : m_TrackProxies) {
        if (p->GetTrack() == child) {
            LoadNATracks(p, naAccessions);
            break;
        }
    }
}


void CTrackContainer::x_UpdateMsg()
{
    size_t on_num = GetChildren().size();
    size_t total = 0;

    for (const auto& p : m_TrackProxies) {
        if(p->GetTrack() || p->IsNA()) ++total;
    }

    string msg = ", Subtracks: " + NStr::SizetToString(on_num) + " on, ";
    if (total < on_num)
        msg += " error, track is broken";
    else
        msg += NStr::SizetToString(total - on_num) + " off";
    SetMsg(msg);
}


void CTrackContainer::x_AddSubtrack(CTrackContainer* parent,
                                    CLayoutTrack* track,
                                    CTempTrackProxy* proxy)
{
    // connect the real track to the proxy
    proxy->SetTrack(track);
    parent->AddTrackProxy(proxy);

    // load the profile
    string profile = proxy->GetTrackProfile();
    if (proxy->GetStoredScale() != "")
        profile += ",stored_scale:" + proxy->GetStoredScale();
    track->LoadProfile(profile);
    track->SetComments(proxy->GetComments());
    track->SetHighlights(proxy->GetHighlights());
    track->SetHighlightsColor(proxy->GetHighlightsColor());
    track->SetShowTitle(proxy->GetShowTitle()  &&  track->GetShowTitle());
    track->SetOrder(proxy->GetOrder());
}


void CTrackContainer::x_ShowTrack(CLayoutTrack* track, bool on)
{
    bool was_on = track->IsOn();
    if (on != was_on) {
        if (was_on) {
            track->CloseMe();
        } else {
            // add the track back to visible list
            Add(track);
            SortChildTracksByOrder();
            track->ShowMe();
        }
        x_UpdateMsg();
    }
}

bool CTrackContainer::x_ShowTitle() const
{
    bool compact = m_gConfig->GetSizeLevel() == "Compact";
// cerr << "CTrackContainer::x_ShowTitle(): compact: " << compact << ", !overlay: " << !IsOverlay() << endl;
    if (compact && !IsOverlay())
        return false;
    return CDataTrack::x_ShowTitle();
}

/// synchronize and merge track settings.
/// For many cases that the actually track proxies used for a viewed
/// sewquence may be different from the original track proxied stored.
/// We need to synchronize those two before we save the settings. For
/// multiple level layout, it is also necessary to sync multiple copies
/// of the similar sets of track proxies, one for each level, and merge
/// into one unique set of track profile for saving.

void CTrackContainer::SyncSettings(TTrackProxies& dst_proxies, int level) const
{
    if (m_TrackProxies.empty()) return;

    CTempTrackProxy::TTrackProxies src_copy(m_TrackProxies.begin(), m_TrackProxies.end());

    auto NameComp = [](const CRef<CTempTrackProxy>& t1, const CRef<CTempTrackProxy>& t2) ->
        bool { return t1->GetName() < t2->GetName(); };

    src_copy.sort(NameComp);
    if (!dst_proxies.empty())
        dst_proxies.sort(NameComp);

    // In multiple leve layout, we don't need to save settings on
    // track container for level 0 - 2. So it will be partial
    bool partial = (level == -2);

    time_t timeStamp = time(NULL);

    for (auto sit = src_copy.begin(), dit = dst_proxies.begin(); sit != src_copy.end(); ++sit) {
        if (!(*sit)->IsRealized() || !(*sit)->GetTrack()) continue;

        if (partial) {
            const CTrackContainer* cont =
                dynamic_cast<const CTrackContainer*>((*sit)->GetTrack());
            if (cont  &&  cont->GetAnnotLevel() != -2) {
                continue;
            }
        }

        while (dit != dst_proxies.end() && (*dit)->GetName() < (*sit)->GetName())
            ++dit;

        if (dit != dst_proxies.end() && (*dit)->GetName() == (*sit)->GetName()) {
            if ((*sit)->IsTempTrack() && (*sit)->IsEmpty()) {
                dit = dst_proxies.erase(dit);
                continue;
            }

            (*dit)->SetOrder((*sit)->GetOrder());
            (*dit)->SetShown((*sit)->GetShown());
            (*dit)->SetExpanded((*sit)->GetExpanded());
            (*dit)->SetTimeStamp(timeStamp);
            (*dit)->SetTrackProfile((*sit)->GetTrackProfile());
            (*dit)->SetSource((*sit)->GetSource());
            (*dit)->SetFilter((*sit)->GetFilter());
            (*dit)->SetSortBy((*sit)->GetSortBy());
            (*dit)->SetId((*sit)->GetId());
            (*dit)->SetDisplayName((*sit)->GetDisplayName());

            CTrackProxy* dp = dynamic_cast<CTrackProxy*>(dit->GetPointer());
            CTrackProxy* sp = dynamic_cast<CTrackProxy*>(sit->GetPointer());
            if (dp && sp) dp->SetContShown(sp->GetContShown());

            const CTrackContainer* cont =
                dynamic_cast<const CTrackContainer*>((*sit)->GetTrack());
            if (cont) {
                cont->SyncSettings((*dit)->GetChildren(), level);
            }
            else if (dp && sp) {
                string data_source =
                    ILayoutTrackFactory::MergeAnnotAndFilter(
                        sp->GetSource(),
                        sp->GetFilter());
                CRef<CTempTrackProxy> childProxy = dp->GetChildTempTrack(data_source);
                childProxy->SetShown(sp->GetShown());
            }
        }
        else if (!(*sit)->IsEmpty()) {
            dit = dst_proxies.insert(dit, (*sit)->Clone());
            (*dit)->SetTimeStamp(timeStamp);

            const CTrackContainer* cont =
                dynamic_cast<const CTrackContainer*>((*sit)->GetTrack());
            if (cont) {
                cont->SyncSettings((*dit)->GetChildren(), level);
            }
        }
    }

    dst_proxies.sort([](const CRef<CTempTrackProxy>& t1, const CRef<CTempTrackProxy>& t2) ->
        bool { return t1->GetOrder() < t2->GetOrder(); });
}

void CTrackContainer::x_MakeTopLevelTracks(CGlyphContainer* featurePanel, TTrackProxies& dst_proxies, int& order, const string& base_name, int indent)
{
    for (auto& p : m_TrackProxies) {
        CLayoutTrack* track = p->GetTrack();
        if (!track) continue;
        CTrackContainer* c_track = dynamic_cast<CTrackContainer*>(track);
        if (c_track && !c_track->IsOverlay()) {
            c_track->x_MakeTopLevelTracks(featurePanel, dst_proxies, order,
                (base_name.empty() ? "" : base_name + "---") + p->GetName(), indent);
        }
        else {
            p->SetOrder(order++);
            if (p->IsTempTrack()) {
                p->SetName(base_name + "---" + p->GetName());
            }
            track->SetParentTrack(featurePanel);
            dst_proxies.push_back(p);
            if (p->GetShown()) {
                track->SetIndent(indent);
                featurePanel->Add(track);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CTrackContainerFactory
///////////////////////////////////////////////////////////////////////////////
ILayoutTrackFactory::TTrackMap
CTrackContainerFactory::CreateTracks(SConstScopedObject& object,
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
    CTrackContainer* track = new CTrackContainer(r_cntx, fp_ds);
    tracks["Container track"] = CRef<CLayoutTrack>(track);
    track->SetAnnotLevel(params.m_Level);
    track->SetAdaptive(params.m_Adaptive);
    track->SetSkipGenuineCheck(params.m_SkipGenuineCheck);
    return tracks;
}

string CTrackContainerFactory::GetExtensionIdentifier() const
{
    return CTrackContainer::m_TypeInfo.GetId();
}

string CTrackContainerFactory::GetExtensionLabel() const
{
    return CTrackContainer::m_TypeInfo.GetDescr();
}

void CTrackContainer::SetSubtrackProxies(const CTrackContainer::TTrackProxies& proxies)
{
    m_TrackProxies = proxies;
    m_TrackProxies.sort(CTrackProxy::STPSorterByOrder::s_CompareCRefs);
    
}

END_NCBI_SCOPE
