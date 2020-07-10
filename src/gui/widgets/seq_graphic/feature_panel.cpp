/*  $Id: feature_panel.cpp 44617 2020-02-06 19:35:49Z filippov $
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
 */
#include <ncbi_pch.hpp>


#include <gui/widgets/seq_graphic/feature_panel.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/feature_track.hpp>
#include <gui/widgets/seq_graphic/gene_model_track.hpp>
#include <gui/widgets/seq_graphic/all_other_features_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_graph_ds.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/utils/extension_impl.hpp>
#include <gui/objutils/na_utils.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/alnmgr/alnmix.hpp>
#include <corelib/ncbitime.hpp>
#include <serial/iterator.hpp>
#include <util/sequtil/sequtil_manip.hpp>

#include "configure_tracks_dlg.hpp"

#include <gui/widgets/loaders/assembly_cache.hpp>

#include <objects/seqfeat/Gene_ref.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/Variation_ref.hpp>
#include <objects/seqfeat/Variation_inst.hpp>
#include <objects/seqfeat/Delta_item.hpp>

#include <objects/seqfeat/Clone_ref.hpp>
#include <objects/seqfeat/Clone_seq_set.hpp>
#include <objects/seqfeat/Clone_seq.hpp>
#include <objects/seqfeat/VariantProperties.hpp>
#include <objects/seqfeat/Phenotype.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objtools/snputil/snp_utils.hpp>
#include <gui/objutils/user_type.hpp>
#include <gui/framework/service.hpp>
#include <gui/framework/status_bar_service.hpp>

#include "search_utils.hpp"

#include <algorithm>
#include <ctype.h>
#include <math.h>
#include <unordered_set>

#include <wx/menu.h>
#include <wx/utils.h>
#include <wx/string.h>

#include <wx/msgdlg.h>
#include <wx/richmsgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/// cell sizes for CDS grid
static const int kMinCellSize = 30;
static const int kMaxCellSize = 72;
// CDS grid to follow codon boundaries
static const int kSteps[] = { 3, 6, 9 };

/// maximal display level in multiple level layout.
static const int kMaxDisplayLevel = 3;

static const string kBaseKey("GBPlugins.SeqGraphicFeatPanel");

static const int kLevelTrackBaseID = 10000;

CTrackTypeInfo
CFeaturePanel::m_TypeInfo("feature_panel_track", "Feature Panel");

CFeaturePanel::CFeaturePanel(CRenderingContext* r_cntx, bool cgi_mode)
    : CTrackContainer(r_cntx)
    , m_SelFeatures(NULL)
    , m_SelCDSFeatures(NULL)
    , m_IsMultiLevel(false)
    , m_IconTexInitialized(false)
    , m_CgiMode(cgi_mode)
    , m_ImageInfo(NULL)
    , m_ConfigKey(kBaseKey)
    , m_AssemblyLoaded(false)
{
    x_InitTrackIcons();

    m_Simple->SetTopMargin(0);
    m_Simple->SetVertSpace(0);
    SetLayoutPolicy(m_Simple);

    SetLeft(0.0);
    SetTop(0.0);

    // we want to make sure the direct children of feature panel have
    // zero indentation. So it must be -1 here.
    m_IndentLevel = -1;

    TTrackAttrFlags attr =
        fShowAlways | fFullTrack | fFrameVisible | fShowIcons;
    SetTrackAttr(attr);

    // register track icons
    // we want to put help icon at the end, so deregister conten, and
    // add it back later.
    x_DeregisterIcon(eIcon_Content);
    x_RegisterIcon(SIconInfo(eIcon_Content,
        "Content", true, "track_content", "", "track_content_lite"));
    x_RegisterIcon(SIconInfo(eIcon_Layout,
        "Layout style", true, "track_layout", "", "track_layout_lite"));
    x_RegisterIcon(SIconInfo(eIcon_Color,
        "Color", true, "track_color_theme", "", "track_color_theme_lite"));
    x_RegisterIcon(SIconInfo(eIcon_Size,
        "Size", true, "track_size", "", "track_size_lite"));
    x_RegisterIcon(SIconInfo(eIcon_Decoration, "Feature decoration",
        true, "track_decoration", "", "track_decoration_lite"));
    x_RegisterIcon(SIconInfo(eIcon_Label,
        "Label position", true, "track_label", "", "track_label_lite"));

    SetTitle(m_TypeInfo.GetDescr());

    // setup grid
    m_Gen.SetIntegerMode(true, true);
    m_Gen.EnableMinusOneBased(true, true);
    // don't update step automatically, the step will be updated
    // externally according to ruler's major ticks.
    m_Gen.SetAutoStepUpdate(false);
    m_Grid.EnableIntegerCentering(true);
}


CFeaturePanel::~CFeaturePanel()
{
    if (m_DS  &&  !m_CgiMode) {
        SaveTrackConfig();
    }
    DeleteAllJobs();
}


void CFeaturePanel::Update(bool layout_only)
{
    if (m_IsMultiLevel) {
/*
        if (!layout_only) {
            if ( !m_DS->AllJobsFinished() ) {
                m_DS->DeleteAllJobs();
                SetTrackInitDone(true);
            }
            x_SetStartStatus();
        }
*/
        // make sure we keep the track info up to date
        // E.g. the track titles get changed to more meaningful ones
        // when actual data get loaded.
        x_UpdateMsg();

        // we need to do something differently from CTrackContainer to
        // make sure that we will see data in layout tracks at level 1 and 2
        // before the tracks at level 3 get initialized which might be time
        // consumming.
        bool update_level_3 = true;
        TTrackProxies proxies = m_TrackProxies;
        proxies.sort(CTrackProxy::STPSorterByOrder::s_CompareCRefs);
        NON_CONST_ITERATE (TTrackProxies, iter, proxies) {
            if ( !(*iter)->GetShown() ) continue;

            CLayoutTrack* track = (*iter)->GetTrack();
            if ( !track) continue;

            const CTrackContainer* cont =
                dynamic_cast<const CTrackContainer*>(track);

            int level = -1;
            if ( cont ) {
                level = cont->GetAnnotLevel();
            }
            if (level < 0) {
                track->Update(layout_only);
            } else if (level < 2) {
                track->Update(layout_only);
                if ( !cont->IsTrackInitDone() ) {
                    update_level_3 = false;
                }
            } else if (update_level_3) {
                const TSeqRange& curr_range = m_Context->GetVisSeqRange();
                if (layout_only) {
                    if (curr_range != (*iter)->GetVisitedRange()) {
                        (*iter)->SetVisitedRange(m_Context->GetVisSeqRange());
                        track->Update(false);
                    } else {
                        track->Update(true);
                    }
                } else {
                    track->Update(false);
                    (*iter)->SetVisitedRange(curr_range);
                }
            } else {
                track->Update(true);
            }
        }
		if (!layout_only) {
			x_UpdateData();
        } else {
			x_UpdateLayout();
			// check if we need to reinitialize the sub tracks.
			x_ReinitSubtracks();
		}
    } else {
        CDataTrack::Update(layout_only);
    }

    if (m_CgiMode) {
        if ( !layout_only ) {
            int order_id = 0;
            TTrackProxies proxies;
            SetChildren().clear();
            x_MakeTopLevelTracks(this, proxies, order_id, "", m_IndentLevel + 1);
            SetSubtrackProxies(proxies);
            x_UpdateLayout();
        }
        SetMsg("");
    }
}


CRef<CSeqGlyph> CFeaturePanel::HitTest(const TModelPoint& p)
{
    CRef<CSeqGlyph> glyph;
     // this is a special case since feature panel is a special track
    if (x_HitTitleBar(p)) {
        glyph.Reset(this);
    } else {
        glyph = CGlyphContainer::HitTest(p);
    }
    return glyph;
}

bool CFeaturePanel::HitTestHor(TSeqPos x, const CObject *obj)
{
    return CGlyphContainer::HitTestHor(x, obj);
}

void CFeaturePanel::SetInputObject(SConstScopedObject& obj)
{
    m_InputObj = obj;
    CIRef<ISGDataSource> ds =
        m_DSContext->GetDS(typeid(CFeaturePanelDSType).name(), obj);
    m_DS.Reset(dynamic_cast<CFeaturePanelDS*>(ds.GetPointer()));
    m_DS->SetJobListener(this);
    m_DS->SetDSContext(m_DSContext);
    m_DS->SetRenderingContext(m_Context);

    const CNcbiRegistry& reg = CNcbiApplication::Instance()->GetConfig();
    string tmsContext = "GBench_3-0";
    bool useEutils = reg.GetBool("NA_TRACKS", "USE_EUTILS", true);
    m_DS->SetTMSContext(tmsContext);
    m_DS->SetUseEUtils(useEutils);
}

void CFeaturePanel::SetAssembly(const string& assembly)
{
    if (!m_DS || m_DS->GetAssembly() == assembly)
        return;

    m_DS->SetAssembly(assembly);
    m_NAData.clear();
    ConfigureTracks();
}

string CFeaturePanel::GetCurrentAssembly() const
{
    string assembly;
    if (m_DS)
        assembly = m_DS->GetAssembly();
    return assembly;
}

namespace
{
    class CTrackCounter
    {
    public:
        CTrackCounter() : m_Count(0) {}

        size_t GetCount() const { return m_Count; }

        bool ContainerBegin(const CTempTrackProxy*, const CTrackContainer*, bool) const { return true; }
        void ContainerEnd(const CTempTrackProxy*, const CTrackContainer*, bool) const { ;; }
        bool Track(const CTempTrackProxy*, const CLayoutTrack*, bool /*visible*/) { ++m_Count; return true; }

    private:
        size_t m_Count;
    };    
} // namespace

void CFeaturePanel::ShowConfigureTracksDlg(const string& category)
{
    CTrackCounter counter;
    Traverse(counter, true);

    if (counter.GetCount() == 0) {
        wxMessageBox(wxT("Track information is not loaded. Please wait."), wxT("Track Initialization"), wxOK | wxICON_EXCLAMATION);
        return;
    }

    if (!IsTrackInitDone()) {
        static bool showDialog = true;
        if (showDialog) {
            wxRichMessageDialog dlg(NULL, wxT("Not all tracks are loaded.\nConfigure Tracks dialog will not show all available tracks."),
                wxT("Warning"), wxOK | wxICON_EXCLAMATION);
            dlg.ShowCheckBox(wxT("Don't show this dialog in the current session"));
            dlg.ShowModal();
            if (dlg.IsCheckBoxChecked())
                showDialog = false;
        }
    }

    CConfigureTracksDlg dlg(NULL, this);
    dlg.SetRegistryPath("Dialogs.ConfigureTracks");
    dlg.SetCategory(category);
    dlg.ShowModal();
}

void CFeaturePanel::ResetSearch()
{
    m_search_results.clear();
    m_next_result = 0;
    m_search_text.clear();
    if (m_ServiceLocator) {
        CIRef<IStatusBarService> sb_srv = m_ServiceLocator->GetServiceByType<IStatusBarService>();
        sb_srv->SetStatusMessage(kEmptyStr);
    }
}


CMappedFeat CFeaturePanel::FindText(const string &text, bool match_case)
{
    _ASSERT(m_ServiceLocator);
    CIRef<IStatusBarService> sb_srv = m_ServiceLocator->GetServiceByType<IStatusBarService>();

    CMappedFeat fm;
    if (text != m_search_text || match_case != m_search_match_case)
    {
        ResetSearch();
        m_search_text = text;
        m_search_match_case = match_case;
        s_GatherFeatures(text, match_case, m_DS->GetBioseqHandle(), m_DS->GetScope(), this, m_search_results);
    }
    if (m_search_results.empty())
    {
        sb_srv->SetStatusMessage(kEmptyStr);
        return fm;
    }
    if ( m_next_result >= m_search_results.size())
    {
        if (wxMessageBox(_("No more matches found, wrapping over"), wxT("Confirm"), wxOK | wxCANCEL) == wxOK)
            m_next_result = 0;
        else
            m_next_result = m_search_results.size() - 1;
    }
    fm = m_search_results[m_next_result];
    ++m_next_result;
    CNcbiOstrstream str;
    str << "Search result " << m_next_result << " of " << m_search_results.size();
    sb_srv->SetStatusMessage(CNcbiOstrstreamToString(str));
    return fm;
}

void CFeaturePanel::ConfigureTracks()
{
    if (m_TrackSettings.empty()) {
        x_LoadSettings();
    }
    if (m_DS.NotNull()) {
        if (m_DiscoverTracks && !m_CgiMode  &&  m_NAData.empty()) {
            // we might need to retrieve meta-data for NAAs
            SetTrackInitDone(false);
            x_ConfigureTracks();
            x_StartAssemblyJob();
        }
        else {
            x_ConfigureTracks();
        }
        x_OnLayoutChanged();
    }
}

void CFeaturePanel::x_StartAssemblyJob()
{
    string seqAcc;
    const CSeq_id* seqId = dynamic_cast<const CSeq_id*>(m_InputObj.object.GetPointerOrNull());
    if (seqId && !seqId->IsLocal()) {
        CSeq_id_Handle h = CSeq_id_Handle::GetHandle(*seqId);
        try {
            if (h.IsGi()) {
                CScope::TSeq_id_Handles input, result;
                input.push_back(h);
                m_InputObj.scope->GetAccVers(&result, input);
                if (result.size() == 1)
                    h = result.front();
            }
        } NCBI_CATCH("CGraphicPanel: converting GI Seq-id.");

        CConstRef<CSeq_id> id = h.GetSeqIdOrNull();
        seqAcc = id ? id->GetSeqIdString(true) : seqId->GetSeqIdString(true);
    }

    list<CRef<objects::CGC_Assembly> > assemblies;
    if (seqAcc.empty() || CAssemblyCache::GetInstance().GetAssemblies(seqAcc, assemblies))
        x_OnAssemblyLoaded(assemblies);
    else
        m_DS->GetAssemblies(seqAcc);
}

void CFeaturePanel::x_OnAssemblyLoaded(const list<CRef<objects::CGC_Assembly> >& assemblies)
{
    m_AssemblyLoaded = true;
    m_Assemblies.assign(assemblies.begin(), assemblies.end());

    string a = m_DS->GetAssembly(), cur, def;
    if (!m_Assemblies.empty()) {
        def = m_Assemblies.front()->GetAccession();
        for (const auto& i : m_Assemblies) {
            string acc = i->GetAccession();
            if (acc == a)
                cur = acc;
            if (acc == m_DefaultAssembly)
                def = acc;
        }
    }

    if (cur.empty())
        m_DS->SetAssembly(def);

    m_NAData.clear();
    m_DS->GetAnnotMetaData("Retrieve NA meta-data");
}

void CFeaturePanel::OnDataChanging()
{
    ClearTracks();
    m_DS->DeleteMetaDataJob();
    Update(true);
}


namespace
{
    class CLoadProfile
    {
    public:

        bool ContainerBegin(CTempTrackProxy* proxy, CTrackContainer* container, bool visible)
        {
            Track(proxy, container, visible);
            return true;
        }

        void ContainerEnd(CTempTrackProxy* proxy, CTrackContainer* container, bool visible)
        {
            ;;
        }

        bool Track(CTempTrackProxy* proxy, CLayoutTrack* track, bool)
        {
            if (track) {
                track->LoadProfile(track->GetProfile());
                if (proxy) {
                    track->SetComments(proxy->GetComments());
                    track->SetHighlights(proxy->GetHighlights());
                    track->SetHighlightsColor(proxy->GetHighlightsColor());
                    track->SetShowTitle(proxy->GetShowTitle() && track->GetShowTitle());
                }
            }
            return false;
        }
    };

    class CSaveProfile
    {
    public:
        bool ContainerBegin(CTempTrackProxy* proxy, CTrackContainer* container, bool visible)
        {
            return true;
        }

        void ContainerEnd(CTempTrackProxy* proxy, CTrackContainer* container, bool visible)
        {
            Track(proxy, container, visible);
        }

        bool Track(CTempTrackProxy*, CLayoutTrack* track, bool)
        {
            if (track)
                track->SaveProfile();
            return false;
        }
    };
}

void CFeaturePanel::OnDataChanged()
{
    Traverse(CSaveProfile(), true);
    Traverse(CLoadProfile(), true);
    x_ConfigureTracks();
}


void CFeaturePanel::SetExternalGlyphs(const CSeqGlyph::TObjects& objs)
{
    if (objs.empty() && !m_ExtLayoutTrack)
        return;
    if ( !m_ExtLayoutTrack ) {
        m_ExtLayoutTrack.Reset(new CExternalLayoutTrack(m_Context));
        m_ExtLayoutTrack->SetTitle("View reflection");
        SetTrack(m_ExtLayoutTrack, -100);
    }
    m_ExtLayoutTrack->SetObjects(objs);
    m_ExtLayoutTrack->Update(true);
    SetGroup().UpdateLayout();
}


void CFeaturePanel::LoadDefaultTracks()
{
    // remove the old track prifile from gui registry
    string path = m_ConfigKey + CGuiRegistry::kDecimalDot +
        CSGConfigUtils::TrackProfileKey() + CGuiRegistry::kDecimalDot;
    string profile = GetProfile();

    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    string regPath = path + profile + ".Tracks";

    CRegistryReadView::TKeys keys;
    registry.GetReadView(regPath).GetTopKeys(keys);

    CRegistryWriteView view = registry.GetWriteView(regPath);
    ITERATE(CRegistryReadView::TKeys, iter, keys)
        view.DeleteField(iter->key);

    // set the track profile name to the global default
    string default_profile = CSGConfigUtils::DefTrackProfile();
    SetProfile(default_profile);
    m_gConfig->SetTrackProfile(default_profile);
    m_gConfig->SetDirty(true);
    x_LoadSettings();
    x_ConfigureTracks(true);
}


void CFeaturePanel::SetTrackConfig(const TTrackSettingsSet& settings_set)
{
    GetConfigMgr()->SetTrackConfig(m_TrackSettings, settings_set);
}


void CFeaturePanel::SaveTrackConfig()
{
    Traverse(CSaveProfile(), true);
}


void CFeaturePanel::ShowTrack(const string& track_key)
{
    for (auto& s : m_TrackSettings) {
        CTrackProxy* track = dynamic_cast<CTrackProxy*>(s.GetPointer());
        if (track  &&  track_key == track->GetKey()) {
            track->SetShown(true);
            break;
        }
    }
}


const CTrackTypeInfo& CFeaturePanel::GetTypeInfo() const
{
    return m_TypeInfo;
}


void CFeaturePanel::OnTrackInitFinished(const CLayoutTrack* sender)
{
    if (!m_IsMultiLevel) return;

    const CTrackContainer* cont = dynamic_cast<const CTrackContainer*>(sender);

    if (!cont  ||  cont->GetAnnotLevel() == 2) return;

    bool update_level_3 = true;
    TTrackProxies proxies = m_TrackProxies;
    proxies.sort(CTrackProxy::STPSorterByOrder::s_CompareCRefs);
    NON_CONST_ITERATE (TTrackProxies, iter, proxies) {
        if ( !(*iter)->GetShown() ) continue;

        CTrackContainer* cont_track =
            dynamic_cast<CTrackContainer*>((*iter)->GetTrack());
        if ( !cont_track) continue;

        int level = cont_track->GetAnnotLevel();
        if (level < 2) {
            if ( !cont->IsTrackInitDone() ) {
               update_level_3 = false;
            }
        } else if (update_level_3) {
            const TSeqRange& curr_range = m_Context->GetVisSeqRange();
            if (curr_range != (*iter)->GetVisitedRange()) {
                (*iter)->SetVisitedRange(m_Context->GetVisSeqRange());
                cont_track->Update(false);
                x_UpdateBoundingBox();
            }
        }
    }
}


void CFeaturePanel::x_Draw() const
{
    IRender& gl = GetGl();

    CGlPane* pane = m_Context->GetGlPane();

    // draw grid
    if (!m_Context->IsSkipGrid() && (m_gConfig->GetShowHorzGrid()  ||  m_gConfig->GetShowVertGrid()) ) {
        CRgbaColor v_color = m_gConfig->GetVerticalGridColor();
        CRgbaColor h_color = m_gConfig->GetHorizontalGridColor();
        if (x_NeedSpecialGrid()) {
            v_color.Lighten(0.2f);
            h_color.Lighten(0.2f);
        }
        {{
            CRegularGridRenderer* grid = const_cast<CRegularGridRenderer*>(&m_Grid);
            grid->SetHorzColor(h_color);
            grid->SetVertColor(v_color);
        }}
        m_Grid.Render(pane, pane, const_cast<CRegularGridGen*>(&m_Gen));
    }

    CGlPaneGuard GUARD(*pane, CGlPane::eOrtho);

    // make sure the icon texture objects have been generated.
    // don't initalize textures in cgi mode sinc we don't need them.
    if ( !m_IconTexInitialized  &&  !m_CgiMode) {
        // this call will check if the texture objects are truely invalid.
        m_IconTexInitialized = CLayoutTrack::InitIconTextures();
    }

    bool horz = m_Context->IsHorizontal();
    TModelUnit off_y = horz ? pane->GetOffsetY() : pane->GetOffsetX();
    gl.PushMatrix();
    gl.Translatef(0.0f, -off_y, 0.0f);

    // render speciality grid for selected CDSs
    x_RenderFeatureGrid();

    // Render all fair-lines for  selected features
    gl.Disable(GL_LINE_SMOOTH);
    x_RenderHairLineSelections();

    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.Enable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    gl.LineWidth(1.0f);
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    GetGroup().Draw();
    if (!m_Context->IsSkipControls()) {
        // Don't render icons/menu on vector output
        if (!gl.IsPrinterFriendly())
            x_RenderTitleBar();

        if (!IsTrackInitDone() && !m_CgiMode) {

            gl.LineWidth(1.0);

            //gl.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
            CGlTextureFont font(CGlTextureFont::eFontFace_Helvetica_Italic, 14);
            TModelRect rcm;
            x_GetTBRect(rcm);
            TModelUnit x = m_Context->GetVisibleFrom() +
                m_Context->ScreenToSeq(10.0);
            m_Context->AdjustToOrientation(x);
            static const string kInitTrackStr = "Discovering tracks...";
            TModelUnit w = m_Context->ScreenToSeq(font.TextWidth(kInitTrackStr.c_str()));
            TModelUnit h = font.TextHeight();
            gl.Color3f(1.f, 1.f, 1.f);
            TModelRect rect(x, rcm.Bottom() - 4.0, x + w, rcm.Bottom() - 4.0 - h);
            m_Context->DrawBackground(rect, 2);
            gl.ColorC(CRgbaColor("gray"));
            m_Context->TextOut(&font, kInitTrackStr.c_str(),
                               x, rcm.Bottom() - 4.0, false, false);
        }
    }
    gl.PopMatrix();

    gl.Disable(GL_BLEND);
    gl.Disable(GL_LINE_SMOOTH);
}


bool CFeaturePanel::x_IsJobNeeded(CAppJobDispatcher::TJobID id) const
{
    if (m_DS->IsMetaDataJob(id)) {
        return true;
    }
    return m_DS->IsJobNeeded(id);
}


void CFeaturePanel::x_OnJobCompleted(CAppJobNotification& notify)
{
    CRef<CObject> res_obj = notify.GetResult();
    CGetAssembliesJobResult* assembliesResult =
        dynamic_cast<CGetAssembliesJobResult*>(&*res_obj);

    if (assembliesResult) {
        m_DS->ClearMetaDataJobId();
        x_OnAssemblyLoaded(assembliesResult->m_Assemblies);
        return;
    }

    CAnnotMetaDataJobResult* result =
        dynamic_cast<CAnnotMetaDataJobResult*>(&*res_obj);
    if (result) {
        m_DS->ClearMetaDataJobId();
        if ( !result->m_Annots.empty() ) {
            m_NAData = result->m_Annots;
            x_InitNATracks(false);
        }
    } else {
        CTrackContainer::x_OnJobCompleted(notify);
    }
}

void CFeaturePanel::x_InitNATracks(bool makeContainersVisible)
{
    if (m_IsMultiLevel) {
        for (auto& i : m_TrackProxies) {
            CTrackContainer* cont = dynamic_cast<CTrackContainer*>(i->GetTrack());
            if (!cont) continue;
            cont->InitNATracks(m_NAData, makeContainersVisible);
        }
    }
    else
        InitNATracks(m_NAData, makeContainersVisible);
}

bool CFeaturePanel::x_Empty() const
{
    return GetChildren().empty();
}


void CFeaturePanel::x_LoadSettings(const string& preset_style,
                                   const TKeyValuePairs& /*settings*/)
{
    if (preset_style.empty()) {
        SetProfile(CSGConfigUtils::DefTrackProfile());
    } else {
        SetProfile(preset_style);
    }
    x_LoadSettings();
}


void CFeaturePanel::x_SaveSettings(const string& /*preset_style*/)
{
    x_SaveSettings();
}


void CFeaturePanel::x_GetTBRect(TModelRect& rect) const
{
    int icon_s = m_gConfig->GetIconSize();
    TModelUnit h = icon_s * 1.5;
    TModelUnit w = (m_Icons.size() * 1.5 + 0.5)* icon_s;
    w = m_Context->ScreenToSeq(w) * 0.5;
    TModelUnit center_x =
        (m_Context->GetVisibleFrom() + m_Context->GetVisibleTo()) * 0.5;
    const CGlPane* pane = m_Context->GetGlPane();
    TModelUnit bottom = pane->GetVisibleRect().Bottom();
    if ( !m_Context->IsHorizontal() ) {
        bottom = pane->GetVisibleRect().Left();
    }
    rect.Init(center_x - w, bottom, center_x + w, bottom - h);
}


void CFeaturePanel::x_RenderTitleBar() const
{
    IRender& gl = GetGl();

    if (m_Icons.empty()  ||  m_CgiMode) {
        return;
    }

    CConstRef<CSeqGraphicConfig> config = x_GetGlobalConfig();

    // render track frame
    TModelRect rcm;
    x_GetTBRect(rcm);
    if (m_Attrs & fFrameVisible) {
        TModelUnit left = rcm.Left();
        TModelUnit right = rcm.Right();
   //     m_Context->AdjustToOrientation(left);
//        m_Context->AdjustToOrientation(right);
        TModelRect rcm_adj(left, rcm.Bottom(), right, rcm.Top());
        TModelUnit w = m_Context->SeqToScreen(rcm_adj.Width());
        TModelUnit diff = 0;
        if (w < 100) {
            diff = m_Context->ScreenToSeq(100 - w);
            w = m_Context->ScreenToSeq(200.0);
        }
        rcm_adj.Inflate(diff, 0.0);

        if (m_ShowFrame) {
            gl.Color4f(0.83f, 0.83f, 0.83f, 0.9f);

            int numSegments = 8;
            float delta = 3.141592653589793238463f / numSegments / 2;

            float l = rcm_adj.Left() - m_Context->GetOffset(),
                  r = rcm_adj.Right() - m_Context->GetOffset();
            float b = rcm_adj.Bottom(), ry = rcm.Bottom() - rcm.Top();
            float rx = ry * m_Context->GetScale();

            gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            gl.Begin(GL_TRIANGLE_STRIP);
            gl.Vertex2f(l - rx, b);
            gl.Vertex2f(r + rx, b);

            float a = delta;
            for (int i = 0; i < numSegments - 1; ++i) {
                float sina = ry*sin(a), cosa = rx*cos(a);
                gl.Vertex2f(l - cosa, b - sina);
                gl.Vertex2f(r + cosa, b - sina);
                a += delta;
            }

            gl.Vertex2f(l, b - ry);
            gl.Vertex2f(r, b - ry);
            gl.End();
        }

        size_t icon_num = m_Icons.size();
        for (size_t idx = 0;  idx < icon_num;  ++idx) {
            x_RenderIcon(m_Icons[idx].m_Id, m_HighlightedIcon == (int)idx, !m_ShowFrame);
        }

        if (!m_ShowFrame) {
            gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }
}


void CFeaturePanel::x_OnIconClicked(TIconID id)
{
    switch (id) {
    case eIcon_Layout:
        x_OnLayoutIconClicked();
        break;
    case eIcon_Color:
        x_OnColorIconClicked();
        break;
    case eIcon_Size:
        x_OnSizeIconClicked();
        break;
    case eIcon_Decoration:
        x_OnDecorationIconClicked();
        break;
    case eIcon_Label:
        x_OnLabelIconClicked();
        break;
    default:
        // use default handlers
        CTrackContainer::x_OnIconClicked(id);
    }
}


void CFeaturePanel::x_OnLayoutIconClicked()
{
    typedef map <int, wxString> TStyleMap;
    TStyleMap styles;
    int id_base = 10000;
    styles[id_base] = wxT("Default");
    styles[id_base + 1] = wxT("Multiple level");

    wxMenu menu;
    UseDefaultMarginWidth(menu);

    int id = 0;
    ITERATE (TStyleMap, iter, styles) {
        wxMenuItem* item = menu.AppendRadioItem(iter->first, iter->second);
        if (m_IsMultiLevel  &&  id == 1) {
            item->Check();
        } else if (id == 0) {
            item->Check();
        }
        ++id;
    }

    m_LTHost->LTH_PopupMenu(&menu);
    wxMenuItemList& item_list = menu.GetMenuItems();
    ITERATE (wxMenuItemList, iter, item_list) {
        if ((*iter)->IsChecked()) {
            id = (*iter)->GetId() - id_base;
            break;
        }
    }

    if ((id == 0 && m_IsMultiLevel) || (id == 1 && !m_IsMultiLevel)) {
        Traverse(CSaveProfile(), true);
        Traverse(CLoadProfile(), true);
        m_IsMultiLevel = !m_IsMultiLevel;
        x_ConfigureTracks();
        LOG_POST(Info << "Graphical view: switch to " <<
            (m_IsMultiLevel ? "multiple level layout" : "default layout mode"));
    }
}


void CFeaturePanel::x_OnColorIconClicked()
{
    // prohibit change settings before initialization gets finished
    if ( !IsTrackInitDone() ) return;

    typedef map <int, wxString> TMenuItems;
    TMenuItems items;
    int id_base = 10000;
    items[id_base] = wxT("Greyscale");
    items[id_base + 1] = wxT("Color");

    wxMenu menu;
    UseDefaultMarginWidth(menu);
    ITERATE (TMenuItems, iter, items) {
        wxMenuItem* item = menu.AppendRadioItem(iter->first, iter->second);
        if (iter->second == ToWxString(m_gConfig->GetColorTheme())) {
            item->Check();
        }
    }

    m_LTHost->LTH_PopupMenu(&menu);
    wxMenuItemList& item_list = menu.GetMenuItems();
    ITERATE (wxMenuItemList, iter, item_list) {
        const string& new_color = ToStdString(items[(*iter)->GetId()]);
        if ((*iter)->IsChecked()  &&  new_color != m_gConfig->GetColorTheme()) {
            LOG_POST(Info << "Graphical view: switch color theme from " <<
                m_gConfig->GetColorTheme() << " to " << new_color);
            Traverse(CSaveProfile(), true);
            m_gConfig->SaveSettings(true);
            m_gConfig->SetColorTheme(new_color);
            m_gConfig->LoadSettings();
            if (m_DS->GetBioseqHandle().IsProtein()) {
                m_gConfig->IgnoreFeatureStrand();
            }
            Traverse(CLoadProfile(), true);
            m_gConfig->SetDirty(true);
            m_Context->SetSelectionColor(m_gConfig->GetSelectionColor());
            m_Context->SetSelLabelColor(m_gConfig->GetSelLabelColor());
            Update(false);
            break;
        }
    }
}


void CFeaturePanel::x_OnSizeIconClicked()
{
    // prohibit change settings before initialization gets finished
    if ( !IsTrackInitDone() ) return;

    typedef map <int, wxString> TMenuItems;
    TMenuItems items;
    int id_base = 10000;
    items[id_base] = wxT("Compact");
    items[id_base + 1] = wxT("Normal");
    items[id_base + 2] = wxT("Oversize");

    wxMenu menu;
    UseDefaultMarginWidth(menu);
    ITERATE (TMenuItems, iter, items) {
        wxMenuItem* item = menu.AppendRadioItem(iter->first, iter->second);
        if (iter->second == ToWxString(m_gConfig->GetSizeLevel())) {
            item->Check();
        }
    }

    m_LTHost->LTH_PopupMenu(&menu);
    wxMenuItemList& item_list = menu.GetMenuItems();
    ITERATE (wxMenuItemList, iter, item_list) {
        const string& new_size = ToStdString(items[(*iter)->GetId()]);
        if ((*iter)->IsChecked()  &&  new_size != m_gConfig->GetSizeLevel()) {
            LOG_POST(Info << "Graphical view: switch size level from " <<
                m_gConfig->GetSizeLevel() << " to " << new_size);
            Traverse(CSaveProfile(), true);
            m_gConfig->SaveSettings(true);
            m_gConfig->SetSizeLevel(new_size);
            m_gConfig->LoadSettings();
            if (m_DS->GetBioseqHandle().IsProtein()) {
                m_gConfig->IgnoreFeatureStrand();
            }
            Traverse(CLoadProfile(), true);
            m_gConfig->SetDirty(true);
            Update(false);
            break;
        }
    }
}


void CFeaturePanel::x_OnDecorationIconClicked()
{
    // prohibit change settings before initialization gets finished
    if ( !IsTrackInitDone() ) return;

    typedef map <int, wxString> TMenuItems;
    TMenuItems items;
    int id_base = 10000;
    items[id_base] = wxT("Default");
    items[id_base + 1] = wxT("Arrows");
    items[id_base + 2] = wxT("SquareAnchor");
    items[id_base + 3] = wxT("CircleAnchor");
    items[id_base + 4] = wxT("Fancy");

    wxMenu menu;
    UseDefaultMarginWidth(menu);
    ITERATE (TMenuItems, iter, items) {
        wxMenuItem* item = menu.AppendRadioItem(iter->first, iter->second);
        if (iter->second == ToWxString(m_gConfig->GetDecorateStyle())) {
            item->Check();
        }
    }

    m_LTHost->LTH_PopupMenu(&menu);
    wxMenuItemList& item_list = menu.GetMenuItems();
    ITERATE (wxMenuItemList, iter, item_list) {
        const string& new_decor = ToStdString(items[(*iter)->GetId()]);
        if ((*iter)->IsChecked()  &&  new_decor != m_gConfig->GetDecorateStyle()) {
            LOG_POST(Info << "Graphical view: switch feature decoration from " <<
                m_gConfig->GetDecorateStyle() << " to " << new_decor);
            x_SaveSettings();
            m_gConfig->SaveSettings(true);
            m_gConfig->SetDecorateStyle(new_decor);
            m_gConfig->LoadSettings();
            if (m_DS->GetBioseqHandle().IsProtein()) {
                m_gConfig->IgnoreFeatureStrand();
            }
            m_gConfig->SetDirty(true);
            Update(false);
            break;
        }
    }
}


void CFeaturePanel::x_OnLabelIconClicked()
{
    // prohibit change settings before initialization gets finished
    if ( !IsTrackInitDone() ) return;

    const string& old_label_pos = m_gConfig->GetLabelPos();
    typedef map <int, wxString> TMenuItems;
    TMenuItems items;
    int id_base = 10000;
    items[id_base] = wxT("Default");
    items[id_base + 1] = wxT("Top label");
    items[id_base + 2] = wxT("Side label");
    items[id_base + 3] = wxT("No label");

    wxMenu menu;
    UseDefaultMarginWidth(menu);
    ITERATE (TMenuItems, iter, items) {
        wxMenuItem* item = menu.AppendRadioItem(iter->first, iter->second);
        if (iter->second == ToWxString(old_label_pos)) {
            item->Check();
        }
    }

    m_LTHost->LTH_PopupMenu(&menu);
    wxMenuItemList& item_list = menu.GetMenuItems();
    ITERATE (wxMenuItemList, iter, item_list) {
        const string& new_label_pos = ToStdString(items[(*iter)->GetId()]);
        if ((*iter)->IsChecked()  &&  new_label_pos != old_label_pos) {
            LOG_POST(Info << "Graphical view: switch feature label position from " <<
                old_label_pos << " to " << new_label_pos);
            Traverse(CSaveProfile(), true);
            m_gConfig->SaveSettings(true);
            m_gConfig->SetLabelPos(new_label_pos);
            m_gConfig->LoadSettings();
            if (m_DS->GetBioseqHandle().IsProtein()) {
                m_gConfig->IgnoreFeatureStrand();
            }
            Traverse(CLoadProfile(), true);
            m_gConfig->SetDirty(true);
            Update(false);
            break;
        }
    }
}


void CFeaturePanel::x_ConfigureTracks(bool makeContainersVisible)
{
    CRef<CSeqGraphicConfig> config = m_gConfig;
    if (config) {
        // we don't need to show strand indicator for features on protein.
        // make sure we turn them off if this is a protein sequnce.
        if (m_DS->GetBioseqHandle().IsProtein()) {
            config->IgnoreFeatureStrand();
        }

        ClearTracks();

        if (m_IsMultiLevel) {
            // feature panel track will hold only globally unique tracks,
            // which have no annot level concept (that is why we use -2),
            // and the track containers with level 0, 1, and 2
            SetAnnotLevel(-2);
            SetAdaptive(false);

            /// configure those tracks with no level concept.
            x_ConfigureSubtracks_Recursive(this, m_TrackSettings, -2);
            // configure other level-dependent tracks
            // to avoid track order conflict with the level-independent
            // tracks, we start the level track order = 100000
            int id = kLevelTrackBaseID;
            ITERATE(TTrackProxies, iter, m_TrackProxies) {
                if ((*iter)->GetOrder() >= id)
                    id = (*iter)->GetOrder() + 1;
            }

            for (int level = 0; level < kMaxDisplayLevel; ++level) {
                string track_name = "Level " + NStr::IntToString(level + 1);
                bool shown = (find(m_DisplayLevels.begin(),
                    m_DisplayLevels.end(), level) != m_DisplayLevels.end());
                CRef<CTrackContainer> cont =
                    x_CreateLevelTrack(level, id++, track_name, shown);
                x_ConfigureSubtracks_Recursive(cont, m_TrackSettings, level);
            }
        } else {
            // feature panel uses adaptive annot selector
            if (config->GetAnnotDepth() == CSeqGraphicConfig::eDepth_unknown) {
                SetAdaptive(true);
                /*
                int depth = 0;
                // first check if any feature fetch depth set explicitly
                if ( !CSGUtils::GetFeatFetchDepth(m_DS->GetBioseqHandle(),
                    m_DS->GetScope(), depth) ) {
                    // sequence-specific
                    if (CSGUtils::IsChromosome(m_DS->GetBioseqHandle(), m_DS->GetScope())) {
                        depth = 1;
                    } else if (CSGUtils::IsSegSet(m_DS->GetBioseqHandle(), m_DS->GetScope())) {
                        depth = -1;
                    }
                }
                SetAnnotLevel(depth);
                */
                SetAnnotLevel(-1);
            } else if (config->GetAnnotDepth() == CSeqGraphicConfig::eDepth_nolimit) {
                SetAdaptive(true);
                SetAnnotLevel(-1);
            } else {
                // use exact depth
                SetAdaptive(false);
                SetAnnotLevel((int)config->GetAnnotDepth());
            }
            x_ConfigureSubtracks_Recursive(this, m_TrackSettings, -1);
        }
    }
    Update(false);
    if (!m_NAData.empty()) {
        x_InitNATracks(makeContainersVisible);
    }
}


void CFeaturePanel::x_ConfigureSubtracks_Recursive(CTrackContainer* parent,
                                                   const TTrackProxies& settings,
                                                   int level)
{
    set<string> usedFeatureSubkeys;
    CTrackConfigManager::GetUsedFeatureKeys(usedFeatureSubkeys, m_TrackSettings);

    ITERATE (TTrackProxies, t_iter, settings) {
        const CTrackProxy* param =
            dynamic_cast<const CTrackProxy*>(t_iter->GetPointer());
        if (!param) {
            /// the track and its siblings are temporary tracks that
            /// will be created by its parent. We skip them here.
            break;
        }
        CTrackContainer* cont = x_ConfigureSubtracks(parent, param, level, usedFeatureSubkeys);
        if (cont) {
            x_ConfigureSubtracks_Recursive(cont, param->GetChildren(), level);
        }
    }
}


CTrackContainer*
CFeaturePanel::x_ConfigureSubtracks(CTrackContainer* parent,
                                    const CTrackProxy* param,
                                    int level,
                                    const set<string>& usedFeatureSubkeys)
{
    // returned track container.
    // set this value if the created track is a container track.
    // the returned container track will serve as the parent
    // for recursive track loading.
    CTrackContainer* cont_track = NULL;
    CRef<CTrackProxy> t_proxy;

    const ILayoutTrackFactory* factory =
        GetConfigMgr()->GetTrackFactory(param->GetKey());
    if ( !factory ) return cont_track;

    // this is a track that doesn't understand annotation level concept,
    // but we only want to show the tracks in a given annotation level (>=0)
    if (level >=0  &&  !factory->UnderstandLevel()) return cont_track;

    // this is a track that does understand annotation level concept,
    // but we only want to show the tracks with no annot concept.
    if (level == -2  &&  factory->UnderstandLevel()) return cont_track;

    int depth = level;
    if (GetAdaptive()) {
        depth = GetAnnotLevel();
    }

    // create track proxy for the track
    t_proxy.Reset(new CTrackProxy(*param));

    // create track immediately if no background initialization is required
    if ( !factory->NeedBackgroundInit()  &&
        (!m_CgiMode  ||  t_proxy->GetShown())) {
        ILayoutTrackFactory::SExtraParams extra_params(
            depth, GetAdaptive(), &t_proxy->GetAnnots(),
            t_proxy->GetSubkey(), t_proxy->GetFilter(), t_proxy->GetSortBy());
        extra_params.m_SkipGenuineCheck = GetSkipGenuineCheck();
        extra_params.m_SubTracks = t_proxy->GetSubTracks();
        extra_params.m_UsedFeatureSubkeys = usedFeatureSubkeys;
        extra_params.m_RemotePath = t_proxy->GetRemotePath();

        TTrackMap tracks = factory->CreateTracks(m_InputObj, m_DSContext,
            m_Context, extra_params, TAnnotMetaDataList());

        if ( !tracks.empty()) {
            parent->AddTracks(t_proxy, tracks);
            CLayoutTrack* track = tracks.begin()->second;
            if ((cont_track = dynamic_cast<CTrackContainer*>(track))) {
                cont_track->SetSubtrackProxies(t_proxy->GetChildren());
                cont_track->SetConfigMgr(GetConfigMgr());
            }

            if (CAllOtherFeaturesTrack* feats_track =
                dynamic_cast<CAllOtherFeaturesTrack*>(track)) {
                feats_track->SetProxy(t_proxy);
            }
        }
    } else {
        // add the track proxy to the parent track.
        parent->AddTrackProxy(t_proxy);
    }

    return cont_track;
}

static const string k_multi_level = "MultiLevel";
static const string k_display_level = "DisplayLevels";

void s_StabilizeBaseContainers(CTempTrackProxy::TTrackProxies& proxies,
                               CTempTrackProxy::TTrackProxies& defaultProxies)
{
    if (defaultProxies.empty())
        return;

    auto NameComp = [](const CRef<CTempTrackProxy>& t1, const CRef<CTempTrackProxy>& t2) ->
        bool { return t1->GetName() < t2->GetName(); };

    defaultProxies.sort(NameComp);
    if (!proxies.empty())
        proxies.sort(NameComp);

    for (auto sit = defaultProxies.begin(), dit = proxies.begin(); sit != defaultProxies.end() || dit != proxies.end();) {
        while (dit != proxies.end() && (sit == defaultProxies.end() || (*dit)->GetName() < (*sit)->GetName()))
            dit = proxies.erase(dit);
        while (dit != proxies.end() && sit != defaultProxies.end() && (*dit)->GetName() == (*sit)->GetName())
            ++sit, ++dit;
        while (sit != defaultProxies.end() && (dit == proxies.end() || (*sit)->GetName() < (*dit)->GetName())) {
            dit = proxies.insert(dit, (*sit)->Clone());
            ++sit, ++dit;
        }
    }
    proxies.sort([](const CRef<CTempTrackProxy>& t1, const CRef<CTempTrackProxy>& t2) ->
        bool { return t1->GetOrder() < t2->GetOrder(); });
}


void CFeaturePanel::x_LoadSettings()
{
    CConstRef<CSeqGraphicConfig> g_conf = x_GetGlobalConfig();
    m_Grid.SetHorzColor(g_conf->GetHorizontalGridColor());
    m_Grid.SetVertColor(g_conf->GetVerticalGridColor());
    m_Grid.SetShowHorzGrid(g_conf->GetShowHorzGrid());
    //m_Grid.SetShowVertGrid(g_conf->GetShowVertGrid());
    // no vertical grids for graphical sequence view
    m_Grid.SetShowVertGrid(false);

    string path = m_ConfigKey + CGuiRegistry::kDecimalDot + CSGConfigUtils::LayoutKey();

    // loading layout level settings
    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryReadView view = CSGConfigUtils::GetReadView(registry,
        path, g_conf->GetLayout(), CSGConfigUtils::DefLayout());
    m_IsMultiLevel = view.GetBool(k_multi_level, false);
    m_DisplayLevels.clear();

    CConstRef<CUser_field> f = view.GetField(k_display_level);
    if (f) {
        if (f->GetData().Which() == CUser_field::TData::e_Ints) {
            m_DisplayLevels = f->GetData().GetInts();
        } else if (f->GetData().Which() == CUser_field::TData::e_Str) {
            string str = f->GetData().GetStr();
            NStr::TruncateSpacesInPlace(str);
            string::size_type sb = str.find_first_not_of("([");
            if (sb == string::npos) {
                sb = 0;
            }
            string::size_type se = str.find_last_not_of (")] ");
            if (se == string::npos) {
                se = str.size() - 1;
            }
            list<string> toks;
            NStr::Split(str.substr(sb, se - sb + 1), ", ", toks, NStr::fSplit_Tokenize);
            try {
                ITERATE (list<string>, iter, toks) {
                    int level = NStr::StringToInt(*iter);
                    x_SetShowDisplayLevel(level, true);
                }
            } catch (CException& e) {
                LOG_POST(Error << "Feature panel: errors when parsing "
                    <<"display level settings, " << e.GetMsg());
            }
        } else {
            x_SetShowDisplayLevel(f->GetData().GetInt(), true);
        }
    }
    vector<string> icons;
    NStr::Split(view.GetString("Icons"), ",", icons, NStr::fSplit_Tokenize);
    if (!icons.empty()) {
        static map<string, int> kIconName2Id =
        {
            { "content", eIcon_Content },
            { "help", eIcon_Help },
            { "layout", eIcon_Layout },
            { "color", eIcon_Color },
            { "size", eIcon_Size },
            { "decor", eIcon_Decoration },
            { "label", eIcon_Label }
        };

        // if "Icons" are defined
        // Deregister icons that are not found in "Icons: list
        set<int> requested_icons;
        for (auto&& it : icons) {
            string icon = NStr::ToLower(it);
            if (kIconName2Id.count(icon) != 0)
                requested_icons.insert(kIconName2Id[icon]);
        }
        if (!requested_icons.empty()) {
            m_Icons.erase(remove_if(m_Icons.begin(), m_Icons.end(),
                [&](const SIconInfo& info) { return requested_icons.count(info.m_Id) == 0; }),
                m_Icons.end());
        }
    }

    m_TrackSettings.clear();

    string profile = GetProfile();
    path = m_ConfigKey + CGuiRegistry::kDecimalDot
        + CSGConfigUtils::TrackProfileKey() + CGuiRegistry::kDecimalDot;

    CTempTrackProxy::LoadFromRegistry(m_TrackSettings, path + profile);

    if (NStr::StartsWith(profile, CSGConfigUtils::UserSettingPrefix())) {
        CTempTrackProxy::TTrackProxies defaultProxies;
        CTempTrackProxy::LoadFromRegistry(defaultProxies, path + CSGConfigUtils::DefTrackProfile());
        s_StabilizeBaseContainers(m_TrackSettings, defaultProxies);
    }

    // remove the track that marked as 'empty' (need to removed)
    TTrackProxies::iterator iter = m_TrackSettings.begin();
    while (iter != m_TrackSettings.end()) {
        if ((*iter)->IsEmpty()) {
            iter = m_TrackSettings.erase(iter);
        } else {
            ++iter;
        }
    }

    for (auto& p : m_TrackSettings) {
        int order = 0;
        for (auto& c : p->GetChildren())
            c->SetOrder(++order);
    }
}


void CFeaturePanel::x_SaveSettings()
{
    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CConstRef<CSeqGraphicConfig> g_conf = x_GetGlobalConfig();
    if ( !g_conf ) return;

    // saving layout level settings
    string path = m_ConfigKey + CGuiRegistry::kDecimalDot + CSGConfigUtils::LayoutKey();
    CRegistryWriteView view =
        CSGConfigUtils::GetWriteView(registry, path, g_conf->GetLayout(),
        CSGConfigUtils::DefLayout());
    view.Set(k_multi_level, m_IsMultiLevel);
    string levels = kEmptyStr;
    ITERATE (TDisplayLevels, iter, m_DisplayLevels) {
        levels += " " + NStr::IntToString(*iter);
    }
    CRef<CUser_field> field = view.SetField(k_display_level);
    field->SetData().SetStr(CUtf8::AsUTF8(levels, eEncoding_Ascii));

    if (m_IsMultiLevel) {
        for (int i = 2; i >= 0; --i) {
            ITERATE (TTrackProxies, iter, m_TrackProxies) {
                const CTrackContainer* cont =
                    dynamic_cast<const CTrackContainer*>((*iter)->GetTrack());
                if (cont  &&  cont->GetAnnotLevel() == i) {
                    cont->SyncSettings(m_TrackSettings, i);
                }
            }
        }
        SyncSettings(m_TrackSettings, -2);
    } else {
        SyncSettings(m_TrackSettings, -1);
    }

    path = m_ConfigKey + CGuiRegistry::kDecimalDot + CSGConfigUtils::TrackProfileKey() +
        CGuiRegistry::kDecimalDot;
    string profile = GetProfile();

    // Use a different track profile name to differentiate it from the one
    // shipped with GBench so that users can restore/update the track
    // profile to the original default one
    if ( !NStr::StartsWith(profile, CSGConfigUtils::UserSettingPrefix())) {
        // this is done only once when GBench creates GenomeWorkbench2 folder
        // and save user settings for the first time.
        profile = CSGConfigUtils::UserSettingPrefix() + profile;
        SetProfile(profile);
        m_gConfig->SetTrackProfile(profile);
        // track profile name changed. It needs to be saved.
        m_gConfig->SetDirty(true);
        m_gConfig->SaveSettings();
    }
    else {
        // clear original tracks
        string regPath = path + profile + ".Tracks";
        CRegistryReadView::TKeys keys;
        registry.GetReadView(regPath).GetTopKeys(keys);
        view = registry.GetWriteView(regPath);
        ITERATE(CRegistryReadView::TKeys, iter, keys) {
            view.DeleteField(iter->key);
        }
    }
    CTempTrackProxy::SaveToRegistry(m_TrackSettings, path + profile);
}


void CFeaturePanel::x_InitTrackIcons()
{
    static bool icon_image_registered = false;
    if ( !icon_image_registered  &&  !m_CgiMode) {
        CLayoutTrack::RegisterCommonIconImages();
        ITERATE (CTrackConfigManager::TFactoryMap, iter,
            GetConfigMgr()->GetTrackFactories()) {
            const IIconProvider* icon_track =
                dynamic_cast<const IIconProvider*>(iter->second.GetPointer());
            if (icon_track) {
                icon_track->RegisterIconImages();
            }
        }

        // register icon images for feature panel
        RegisterIconImage("track_color_theme", "track_color_theme.png");
        RegisterIconImage("track_size", "track_size.png");
        RegisterIconImage("track_decoration", "track_decoration.png");
        RegisterIconImage("track_label", "track_label.png");

        // lite versions
        RegisterIconImage("track_color_theme_lite", "track_color_theme_lite.png");
        RegisterIconImage("track_size_lite", "track_size_lite.png");
        RegisterIconImage("track_decoration_lite", "track_decoration_lite.png");
        RegisterIconImage("track_label_lite", "track_label_lite.png");
        icon_image_registered = true;
    }
}


void CFeaturePanel::x_RenderHairLineSelections() const
{
    if (!m_SelFeatures) {
        return;
    }
    // check hairline_options_dlg for values
    // 0. Show no hairlines with selections
    // 1. Show all hairlines with selections
    // 2. Show only hairlines that are not shared (i.e., places that are different)
    // 3. Show only hairlines that are shared by any two of the selections
    // 4. Show hairlines shared by *all* of the selections.

    int opt = m_gConfig->GetHairlineOption();
    if (opt == 0)
        return;

    CGlPane* pane = m_Context->GetGlPane();
    int vp_min = pane->GetViewport().Left();
    int vp_max = pane->GetViewport().Right() + 1;
    vector<bool>  has_line(vp_max-vp_min, false);

    // first, gather all From and To positions
    multiset <TSeqPos> all_pos;
    size_t total_glyph = 0;
    ITERATE (TSelectedGlyphs, iter, *m_SelFeatures) {
        CWeakRef<CSeqGlyph>::TRefType glyph = iter->Lock();
        if ( !glyph ) continue;

        const IObjectBasedGlyph* obj =
            dynamic_cast<const IObjectBasedGlyph*>(glyph.GetPointer());
        _ASSERT(obj);
        ITERATE (IObjectBasedGlyph::TIntervals, iter_i, obj->GetIntervals()) {
            const TSeqRange& curr = *iter_i;
            all_pos.insert(curr.GetFrom());
            all_pos.insert(curr.GetTo() + 1);
        }
        ++total_glyph;
    }  // done

    TModelUnit flip_sign = (m_Context->IsFlippedStrand()) ? -1.0 : 1.0;

    // finally, render hair-lines
    ITERATE (TSelectedGlyphs, iter, *m_SelFeatures) {
        CWeakRef<CSeqGlyph>::TRefType glyph = iter->Lock();
        if ( !glyph ) continue;

        TModelPoint top_left;
        glyph->GetPosInWorld(top_left);
        TModelUnit line_y = top_left.Y();
        const CFeatGlyph* feat =
            dynamic_cast<const CFeatGlyph*>(glyph.GetPointer());
        if (feat) {
            line_y += feat->GetBarCenter();
        }

        const IObjectBasedGlyph* obj =
            dynamic_cast<const IObjectBasedGlyph*>(glyph.GetPointer());
        ITERATE (IObjectBasedGlyph::TIntervals, iter_i, obj->GetIntervals()) {
            const TSeqRange& curr = *iter_i;

            int x = int(floor(m_Context->SeqToScreenXInModelUnit(curr.GetFrom())) * flip_sign);
            if (x>=vp_min && x<vp_max && !has_line[x-vp_min]) {
                has_line[x-vp_min] = true;
                x_DrawHairLine(opt, total_glyph, all_pos, curr.GetFrom(), line_y);
            }

            x = int(floor(m_Context->SeqToScreenXInModelUnit(curr.GetTo() + 1)) * flip_sign);
            if (x>=vp_min && x<vp_max && !has_line[x-vp_min]) {
                has_line[x-vp_min] = true;
                x_DrawHairLine(opt, total_glyph, all_pos, curr.GetTo()+1, line_y);
            }
        }
    }
}


void CFeaturePanel::x_DrawHairLine(int opt, size_t total, const multiset <TSeqPos>& all_pos,
        TSeqPos pos, TModelUnit line_y) const
{
    IRender& gl = GetGl();

    // use light color when no intersection, dark otherwise
    CRgbaColor c_light = m_gConfig->GetSelHairlineOneColor();
    CRgbaColor c_dark  = m_gConfig->GetSelHairlineManyColor();

    // if only one selected feature, we show all hairlines
    // This is for considering merged feature
    if (total == 1) {
        opt = 1;
    }

    bool need_line = true;
    CRgbaColor color;
    switch (opt) {
    case 1: // 1. Show all hairlines with selections
        color = all_pos.count(pos) > 1 ? c_dark : c_light;
        break;
    case 2: // 2. Show only hairlines that are not shared (i.e., places that are different)
        color = c_light;
        need_line = all_pos.count(pos) == 1;
        break;
    case 3: // 3. Show only hairlines that are shared by any two of the selections
        color = c_dark;
        need_line = all_pos.count(pos) >= 2;
        break;
    case 4: // 4. Show hairlines shared by *all* of the selections.
        color = c_dark;
        need_line = all_pos.count(pos) == total;
        break;
    }

    // render "From"
    if (need_line) {
        gl.ColorC(color);
        m_Context->DrawLine(pos, line_y, pos, GetTop());

        if (m_gConfig->GetShowDownwardHairlines()) {
            gl.ColorC(m_gConfig->GetSelHairlineDownColor());
            m_Context->DrawLine(pos, GetBottom()+1, pos, line_y);
        }

        if (m_ImageInfo != NULL) {
            CSeqGraphicHairline hinfo;
            hinfo.m_TopColor = color;

            TModelUnit flip_sign = (m_Context->IsFlippedStrand()) ? -1.0 : 1.0;

            // Javascript takes floor anyway so do it here - otherwise
            // we get rounding errors.
            TModelUnit x1 =  floor(m_Context->SeqToScreenXInModelUnit(pos) * flip_sign);

            hinfo.SetPos(x1);
            hinfo.ShowDownwardHairline(m_gConfig->GetShowDownwardHairlines());
            hinfo.SetBottomColor(m_gConfig->GetSelHairlineDownColor());
            m_ImageInfo->m_Hairlines.push_back(hinfo);
        }
    }
}


void CFeaturePanel::x_RenderFeatureGrid() const
{
    IRender& gl = GetGl();

    if ( !m_gConfig->GetShowHorzGrid()  ||  !x_NeedSpecialGrid()) return;

    CRgbaColor color = m_gConfig->GetVerticalGridColor();
    color.Darken(0.1f);
    gl.ColorC(color);

    ITERATE (TSelectedGlyphs, obj_iter, *m_SelCDSFeatures) {
        CWeakRef<CSeqGlyph>::TRefType glyph = obj_iter->Lock();
        const CFeatGlyph* feat =
            dynamic_cast<const CFeatGlyph*>(glyph.GetPointer());
        if ( !feat ) continue;

        TSeqRange range = feat->GetRange();

        // calculate grid spes for each CDS
        double length = range.GetTo() - range.GetFrom();
        // less than 3 bases, don't draw anything
        if (length < 2) continue;

        double logRange = log10(length);
        logRange = ceil(logRange) - 1;
        double step = pow(10.0, logRange);

        // selecting base_step as step divided by 1, 2 or 5
        double base_step = step;
        double tick_num = length / base_step;
        int i=0;
        while(tick_num < 8) {
            base_step = step / kSteps[i++];
            tick_num = length / base_step;
        }

        double min_cell = m_Context->ScreenToSeq(kMinCellSize);
        double max_cell = m_Context->ScreenToSeq(kMaxCellSize);
        int grid_step = (int)x_CalcGridStep(base_step,  min_cell, max_cell);

        bool neg_strand = (sequence::GetStrand(feat->GetLocation()) == eNa_strand_minus);
        TModelPoint top_left;
        feat->GetPosInWorld(top_left);

        ITERATE (vector<TSeqRange>, iter, feat->GetIntervals()) {
            const TSeqRange& curr = *iter;
            TModelUnit f = curr.GetFrom();
            TModelUnit t = curr.GetTo();

            if (neg_strand) {
                int end = (int)floor(f - 1);
                for (i = (int)ceil(t); i >= end; i -= grid_step) {
                    m_Context->DrawLine(i + 1, GetBottom(), i + 1, GetTop());
                }
            } else {
                int end = (int)ceil(t + 1);
                for (i = (int)floor(f); i <= end; i += grid_step) {
                    m_Context->DrawLine(i, GetBottom(), i, GetTop());
                }
            }
        } // ITERATE
    } // ITERATE
}


double CFeaturePanel::x_CalcGridStep(double base_step,
                                     double min_step,
                                     double max_step) const
{
    if ((base_step >= min_step && base_step <= max_step) || (min_step == max_step))
        return base_step;
    else {
        // base_step has a form M * pow(10, P), where P is power
        double Power = log10(base_step);
        double P = ceil(Power) -1;
        double pow10P = pow(10.0, P);
        double M = base_step / pow10P;
        if (M >= 10) {
            M /= 10;
            pow10P *= 10;
        }

        int oldK = 1, K = 1, Index = 0;
        if(base_step < min_step) // reasing base_step to make it more then minPix
        {
            double minK = min_step / pow10P;
            while (K < minK)
            {
                if(Index <2)
                    K = oldK * kSteps[Index++];
                else {
                    K = oldK = oldK * 10;
                    Index = 0;
                }
            }
            base_step = pow10P * K;
        } else if (base_step > max_step) { // decreasing base_step to make it less then maxPix
            pow10P *= 10;
            double maxK = pow10P / max_step;
            while (K < maxK)
            {
                if(Index <2)
                    K = oldK * kSteps[Index++];
                else {
                    K = oldK = oldK * 10;
                    Index = 0;
                }
            }
            base_step = pow10P / K;
        }
        return base_step;
    }
}


bool CFeaturePanel::x_GetShowDisplayLevel(TLevel level) const
{
    return find(m_DisplayLevels.begin(), m_DisplayLevels.end(), level)
        != m_DisplayLevels.end();
}


void CFeaturePanel::x_SetShowDisplayLevel(TLevel level, bool show)
{
    TDisplayLevels::iterator iter =
        find(m_DisplayLevels.begin(), m_DisplayLevels.end(), level);
    if (show  &&  iter == m_DisplayLevels.end()) {
        m_DisplayLevels.push_back(level);
    } else if (!show  &&  iter != m_DisplayLevels.end()) {
        m_DisplayLevels.erase(iter);
    }
}


CRef<CTrackContainer> CFeaturePanel::x_CreateLevelTrack(int level,
                                                        int id,
                                                        const string& track_name,
                                                        bool shown)
{
    CRef<CTrackContainer> cont_track;

    const string& cont_key = CTrackContainerFactory::GetTypeInfo().GetId();
    const ILayoutTrackFactory* factory =
        GetConfigMgr()->GetTrackFactory(cont_key);
    if ( !factory ) return cont_track;

    // create track proxy for the track
    CRef<CTempTrackProxy> t_proxy(new CTempTrackProxy(id, track_name, shown));

    ILayoutTrackFactory::SExtraParams e_params(level, false, NULL);
    e_params.m_SkipGenuineCheck = GetSkipGenuineCheck();
    TTrackMap tracks = factory->CreateTracks(m_InputObj, m_DSContext,
        m_Context, e_params);

    CLayoutTrack* track = tracks.begin()->second;
    cont_track = dynamic_cast<CTrackContainer*>(track);
    cont_track->SetConfigMgr(GetConfigMgr());
    AddTracks(t_proxy, tracks);

    return cont_track;
}



END_NCBI_SCOPE
