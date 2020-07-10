/*  $Id: feature_track.cpp 44986 2020-05-01 22:45:19Z rudnev $
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

#include <gui/widgets/seq_graphic/feature_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/seq_graphic/layout_track_proxy.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <gui/widgets/seq_graphic/clone_placement_glyph.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <gui/widgets/seq_graphic/feature_filter.hpp>
#include <gui/widgets/seq_graphic/gene_model_group.hpp>
#include <gui/widgets/seq_graphic/named_group.hpp>
#include <gui/widgets/seq_graphic/clone_feature_sorter.hpp>
#include <gui/widgets/seq_graphic/dbvar_feature_sorter.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>

#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/annot_ci.hpp>

#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/User_object.hpp>

#include <wx/menu.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


static const string kBaseKey = "GBPlugins.SeqGraphicFeatureTrack";


///////////////////////////////////////////////////////////////////////////////
///   CFeatureTrack
///////////////////////////////////////////////////////////////////////////////
typedef SStaticPair<const char*, CFeatureTrack::ELayout> TLayoutStr;
static const TLayoutStr s_LayoutStrs[] = {
    { "Adaptive",              CFeatureTrack::eLayout_AdaptiveLayered },
    { "Adaptive - inline",     CFeatureTrack::eLayout_AdaptiveInline },
    { "Columns",               CFeatureTrack::eLayout_ExpandedColumn },
    { "Expanded by position",  CFeatureTrack::eLayout_ExpandedByPos },
    { "Expanded by size",      CFeatureTrack::eLayout_ExpandedBySize },
    { "Layered",               CFeatureTrack::eLayout_ExpandedLayered },
    { "OneLine",               CFeatureTrack::eLayout_OneLine },
    { "Packed",                CFeatureTrack::eLayout_Packed },
};

typedef CStaticArrayMap<string, CFeatureTrack::ELayout> TLayoutMap;
DEFINE_STATIC_ARRAY_MAP(TLayoutMap, sm_LayoutMap, s_LayoutStrs);

CFeatureTrack::ELayout CFeatureTrack::LayoutStrToValue(const string& layout)
{
    TLayoutMap::const_iterator iter = sm_LayoutMap.find(layout);
    if (iter != sm_LayoutMap.end()) {
        return iter->second;
    }
    NCBI_THROW(CException, eInvalid, "Invalid layout string: " + layout);
}


const string& CFeatureTrack::LayoutValueToStr(CFeatureTrack::ELayout layout)
{
    TLayoutMap::const_iterator iter;
    for (iter = sm_LayoutMap.begin();  iter != sm_LayoutMap.end();  ++iter) {
        if (iter->second == layout) {
            return iter->first;
        }
    }
    return kEmptyStr;
}


typedef SStaticPair<const char*, ELinkedFeatDisplay> TLinkedFeatStr;
static const TLinkedFeatStr s_LinkedFeatStrs[] = {
    { "Default",  ELinkedFeatDisplay::eLFD_Default },
    { "Expandable",   ELinkedFeatDisplay::eLFD_Expandable },
    { "Hidden",   ELinkedFeatDisplay::eLFD_Hidden },
    { "OnParent",   ELinkedFeatDisplay::eLFD_PackedWithParent },
    { "Packed",   ELinkedFeatDisplay::eLFD_Packed },
    { "ParentHidden",   ELinkedFeatDisplay::eLFD_ParentHidden },
};

typedef CStaticArrayMap<string, ELinkedFeatDisplay> TLinkedFeatMap;
DEFINE_STATIC_ARRAY_MAP(TLinkedFeatMap, sm_LinkedFeatMap, s_LinkedFeatStrs);

ELinkedFeatDisplay CFeatureTrack::LinkedFeatDisplayStrToValue(const string& linked_feat)
{
    TLinkedFeatMap::const_iterator iter = sm_LinkedFeatMap.find(linked_feat);
    if (iter != sm_LinkedFeatMap.end()) {
        return iter->second;
    }
    NCBI_THROW(CException, eInvalid, "Invalid feature linking mode string: " + linked_feat);
}


const string& CFeatureTrack::LinkedFeatDisplayValueToStr(ELinkedFeatDisplay linked_feat)
{
    TLinkedFeatMap::const_iterator iter;
    for (iter = sm_LinkedFeatMap.begin();  iter != sm_LinkedFeatMap.end();  ++iter) {
        if (iter->second == linked_feat) {
            return iter->first;
        }
    }
    return kEmptyStr;
}


CTrackTypeInfo CFeatureTrack::m_TypeInfo("feature_track",
                                         "Feature track");


CFeatureTrack::CFeatureTrack(CSGFeatureDS* ds, CRenderingContext* r_cntx)
    : CDataTrack(r_cntx)
    , m_DS(ds)
    , m_Subtype(CSeqFeatData::eSubtype_bad)
    , m_FeatLimits(-1)
    , m_CompactThreshold(200)
    , m_MaxRow(200)
    , m_Layout(eLayout_Default)
    , m_LinkedFeat(ELinkedFeatDisplay::eLFD_Default)
    , m_HistThreshold(0)
    , m_Column(new CColumnLayout)
    , m_GroupSimple(new CSimpleLayout)
    , m_GroupLayered(new CLayeredLayout)
    , m_LinkedGroup(new CLayeredLayout)
    , m_Compact(new CCompactLayout)
    , m_HasLinkedFeats(false)
{
    m_DS->SetJobListener(this);
    m_LinkedGroup->SetLimitRowPerGroup(false);

    // initialize annotation selector
    m_FeatSel = CSeqUtils::GetAnnotSelector();
    x_RegisterIcon(SIconInfo(
        eIcon_Layout, "Layout style", true, "track_layout"));
}


CFeatureTrack::~CFeatureTrack()
{
    if ( !m_DS->AllJobsFinished() ) {
        m_DS->DeleteAllJobs();
    }
}


const CTrackTypeInfo& CFeatureTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}


string CFeatureTrack::GetFullTitle() const
{
    string title = GetTitle();
    if (title.empty()) {
        if (CSeqUtils::IsUnnamed(m_AnnotName)) {
            title = "Other annotations";
        } else {
            title = m_AnnotName;
        }
    }

    return title;
}


void CFeatureTrack::SetAnnot(const string& annot)
{
    m_Annot = annot;
    m_AnnotName = annot;
    m_FeatSel.ResetAnnotsNames();
    if (CSeqUtils::NameTypeStrToValue(m_AnnotName) ==
        CSeqUtils::eAnnot_Unnamed) {
        m_FeatSel.AddUnnamedAnnots();
    } else {
        m_FeatSel.AddNamedAnnots(m_AnnotName);
        if (NStr::StartsWith(m_AnnotName, "NA0")) {
            if (m_AnnotName.find(".") == string::npos) {
                m_AnnotName += ".1";
            }
            m_Annot = m_AnnotName;
            m_FeatSel.IncludeNamedAnnotAccession(m_AnnotName);
        }
    }
}


void CFeatureTrack::ExcludeAnnotName(const string& annot)
{
    m_FeatSel.ExcludedAnnotName(annot);
}


void CFeatureTrack::SetFeatSubtype(int subtype)
{
    m_Subtype = subtype;
    CSeqFeatData::ESubtype feat_st = (CSeqFeatData::ESubtype)subtype;
    CSeqFeatData::E_Choice feat_type =
        CSeqFeatData::GetTypeFromSubtype(feat_st);
    const CFeatList& feats(*CSeqFeatData::GetFeatList());
    m_Subtitle = feats.GetDescription((int)feat_type, subtype);

    m_FeatSel.SetFeatSubtype(feat_st);
}


void CFeatureTrack::x_UpdateData()
{
    CDataTrack::x_UpdateData();
    m_Column->SetMinDist(max(TSeqPos(1),
        TSeqPos(m_Context->ScreenToSeq(20.0))));
    m_Compact->SetMinDist(max(TSeqPos(1),
        TSeqPos(m_Context->ScreenToSeq(3.0))));

    m_GroupLayered->SetMinDist(
        max(TSeqPos(1), TSeqPos(m_Context->ScreenToSeq(3.0))));
    m_LinkedGroup->SetMinDist(
        max(TSeqPos(1), TSeqPos(m_Context->ScreenToSeq(3.0))));

    m_DS->DeleteAllJobs();
    x_SetStartStatus();
    TSeqRange range = x_GetVisRange();
    if (range.Empty()) {
        range = m_Context->GetVisSeqRange();
    }

    int feat_limit = m_FeatLimits;
    if (m_Layout != eLayout_Packed  &&  m_Context->WillSeqLetterFit()) {
        // if the layout is not packed, and we are in sequence level,
        // show the features always.
        feat_limit = -1;
    }

    m_DS->LoadFeatures(m_FeatSel, range, m_Context->GetScale(),
                       -1, feat_limit, m_LinkedFeat);
}

void CFeatureTrack::GenerateAsn1(objects::CSeq_entry& seq_entry, TSeqRange range)
{
    CSeqUtils::SetResolveDepth(m_FeatSel, true, -1);
    CBioseq_Handle& Handle = m_DS->GetBioseqHandle();
    CConstRef<CSeq_loc> loc(
        Handle.GetRangeSeq_loc(range.GetFrom(), range.GetTo()) );

//    cerr << "using loc " << endl << 
//            MSerial_AsnText << *loc << endl;
    CFeat_CI feature_iter(Handle.GetScope(), *loc, m_FeatSel);
//    cerr << "got features: " << feature_iter.GetSize() << endl;
    CRef<CSeq_annot> pAnnot(new CSeq_annot);
    CSeq_annot::TData::TFtable& ftable(pAnnot->SetData().SetFtable());

    // filtering features
    CFeatureFilter filter;
    filter.Init(m_DS->GetFilter());
    for ( ;  feature_iter;  ++feature_iter) {
        const CMappedFeat& feat = *feature_iter;
//        cerr << "Got feature" << endl << 
//          MSerial_AsnText << feat.GetMappedFeature() << endl;
        if(!filter.NeedFiltering() || filter.Pass(&feat)) {
            CRef<CSeq_feat> pFeat(new CSeq_feat);
            pFeat->Assign(feat.GetMappedFeature());
            ftable.push_back(pFeat);
        }
    }
    seq_entry.SetAnnot().push_back(pAnnot);
}


void CFeatureTrack::x_OnJobCompleted(CAppJobNotification& notify)
{
    m_DS->ClearJobID(notify.GetJobID());
    CRef<CObject> res_obj = notify.GetResult();
    CSGJobResult* result = dynamic_cast<CSGJobResult*>(&*res_obj);
    if (!result) {
        LOG_POST(Error << "CFeatureTrack::x_OnJobCompleted() notification for job \
                does not contain results.");
        return;
    }

    x_AddFeaturesLayout(*result);
}


void CFeatureTrack::x_LoadSettings(const string& /*preset_style*/,
                                   const TKeyValuePairs& settings)
{
    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryReadView view;
    view = registry.GetReadView(kBaseKey + "." + m_gConfig->GetCurrTheme());
    m_CompactThreshold = view.GetInt("CompactThreshold", 200);
    m_MaxRow = view.GetInt("MaxRow", 200);
    m_MaxAdaptiveHeight = view.GetInt("MaxAdaptiveHeight", 400);

    m_Layered->SetVertSpace(m_gConfig->GetObjectSpace());
    m_Layered->SetMaxRow(m_MaxRow);
    m_Column->SetVertSpace(m_gConfig->GetObjectSpace());
    m_Compact->SetVertSpace(m_gConfig->GetObjectSpace());
    m_GroupSimple->SetVertSpace(2);
    m_GroupLayered->SetVertSpace(2);
    m_LinkedGroup->SetVertSpace(2);

    // feature track doesn't use profile
    ITERATE (TKeyValuePairs, iter, settings) {
        try {
            if (NStr::EqualNocase(iter->first, "Layout")) {
                m_Layout = LayoutStrToValue(iter->second);
            } else if (NStr::EqualNocase(iter->first, "LinkedFeat")) {
                m_LinkedFeat = LinkedFeatDisplayStrToValue(iter->second);
            } else if (NStr::EqualNocase(iter->first, "HistThreshold")) {
                int val = NStr::StringToInt(iter->second);
                if (val > 0) m_HistThreshold = (size_t)val;
                else m_HistThreshold = 0;
            } else if (NStr::EqualNocase(iter->first, "Rendering")) {
                if (NStr::EqualNocase(iter->second, "Histogram")) {
                    m_Layout = eLayout_Packed;
                } else if (NStr::EqualNocase(iter->second, "SingleLine")) {
                    m_Layout = eLayout_OneLine;
                } else {
                    m_LinkedFeat = LinkedFeatDisplayStrToValue(iter->second);
                    m_Layout = eLayout_ExpandedLayered;
                }
            }
        } catch (CException& e) {
            LOG_POST(Warning << "CFeatureTrack::x_LoadSettings() "
                     << e.GetMsg());
        }
    }

    // we would rather assume there are linked features when we
    // hide the child features since we don't really know if
    // there are children or not
    if (m_LinkedFeat == ELinkedFeatDisplay::eLFD_Hidden  ||  m_LinkedFeat == ELinkedFeatDisplay::eLFD_ParentHidden) {
        m_HasLinkedFeats = true;
    }

    x_OnLayoutPolicyChanged();
}


void CFeatureTrack::x_SaveSettings(const string& /*preset_style*/)
{
    TKeyValuePairs settings;
    settings["Layout"] = LayoutValueToStr(m_Layout);
    settings["LinkedFeat"] = LinkedFeatDisplayValueToStr(m_LinkedFeat);
    SetProfile(CSGConfigUtils::ComposeProfileString(settings));
}


void CFeatureTrack::x_OnIconClicked(TIconID id)
{
    switch (id) {
    case eIcon_Content:
        x_OnContentIconClicked();
        break;
    case eIcon_Layout:
        x_OnLayoutIconClicked();
        break;
    default:
        // use default handlers
        CLayoutTrack::x_OnIconClicked(id);
    }
}


static const string kWeightKey = "Weight";
static const string kDispSettings = "DisplaySettings";

bool s_IsSetWeight(const CSeq_feat& feat)
{
    if (feat.IsSetExts()) {
        const CSeq_feat::TExts& exts = feat.GetExts();
        ITERATE (CSeq_feat::TExts, iter, exts) {
            if ((*iter)->GetType().IsStr()  &&
                (*iter)->GetType().GetStr() == kDispSettings  &&
                (*iter)->GetFieldRef(kWeightKey)) {
                return true;
            }
        }
    }
    return false;
}

int s_GetWeight(const CSeq_feat& feat)
{
    if (feat.IsSetExts()) {
        const CSeq_feat::TExts& exts = feat.GetExts();
        ITERATE (CSeq_feat::TExts, iter, exts) {
            if ((*iter)->GetType().IsStr()  &&
                (*iter)->GetType().GetStr() == kDispSettings  &&
                (*iter)->GetFieldRef(kWeightKey)) {
                return (*iter)->GetFieldRef(kWeightKey)->GetData().GetInt();
            }
        }
    }
    return -1;
}

void CFeatureTrack::x_AddHistogram(CHistogramGlyph* hist, int layout)
{
    if (!hist) {
        ERR_POST(Error << "Empty histogramm");
        return;
    }
    if (layout == -1)
        layout = m_Layout;
    hist->SetLevel(0);
    hist->SetDialogHost(dynamic_cast<IGlyphDialogHost*>(m_LTHost));
    auto subtype = m_Subtype == 0 ? CSeqFeatData::eSubtype_any : (CSeqFeatData::ESubtype)m_Subtype;
    hist->SetSubtype(subtype);
    CRef<CHistParamsManager> hist_mgr = m_gConfig->GetHistParamsManager();
    if (ELayout(layout) == eLayout_OneLine) {
        hist->SetSubtype(CSeqFeatData::eSubtype_any);
        SetMsg("");
        CRef<CHistParams> config(new  CHistParams());
        CRef<CHistParams> src_config = hist_mgr->GetHistParams(m_Subtype);
        *config = *src_config;
        config->m_Type = CHistParams::eSmearBar;
        config->m_Height = 10.0;
        config->m_Colors["0"] =
            m_gConfig->GetFeatParams(m_Subtype)->m_fgColor;
        const CCustomFeatureColor::TColorCode& colors =
            m_gConfig->GetCustomFeatColors()->GetColorCode();
        for (size_t i = 1; i < colors.size(); ++i) {
            config->m_Colors[NStr::SizetToString(i)] = colors[i];
        }
        hist_mgr->AddTempSettings(hist->GetAnnotName(), config);
        if(!CSeqUtils::isRmtAnnotName(m_AnnotName)) {
            if (subtype != objects::CSeqFeatData::eSubtype_any && subtype != objects::CSeqFeatData::eSubtype_bad) {
                auto descr = CSeqFeatData::GetFeatList()->GetDescription(CSeqFeatData::GetTypeFromSubtype(subtype), subtype);
                if (!descr.empty()) {
                    descr[0] = toupper(descr[0]);
                } else {
                    descr = "Feature";
                }
                descr += ", heatmap";
                hist->SetDesc(descr);
            }
        }
    } else {
        CRef<CHistParams> hist_conf;
        if (subtype != objects::CSeqFeatData::eSubtype_any && subtype != objects::CSeqFeatData::eSubtype_bad) {
            hist_conf = hist_mgr->GetHistParams(subtype);
        } else if (!hist_mgr->HasSettings(hist->GetAnnotName())) {
            CRef<CHistParams> def_conf = hist_mgr->GetDefHistParams();
            hist_conf = Ref(new CHistParams(*def_conf));
            hist_mgr->AddSettings(hist->GetAnnotName(), hist_conf);
        } else {
            hist_conf = hist_mgr->GetHistParams(hist->GetAnnotName());
        }
        hist->SetDesc(GetTitle());
        
        SetMsg(", Distribution histogram");
    }
    hist->SetConfig(*m_gConfig);
    SetLayoutPolicy(m_Simple);
    Add(hist);
}

void CFeatureTrack::x_AdjustFeatureLayout()
{
    const CNamedGroup* first_group =
        dynamic_cast<const CNamedGroup*>(GetChildren().front().GetPointer());

    // preprocessing to check if we need to show the features in
    // a more compact form.
    int feat_size = 0;
    if (first_group) {
        ITERATE (CSeqGlyph::TObjects, iter, GetChildren()) {
            const CNamedGroup* group = dynamic_cast<const CNamedGroup*>(iter->GetPointer());
            _ASSERT(group);
            x_CountFeats_recursive(group->GetChildren(), feat_size, m_Context->IsOverviewMode());
        }
    } else {
        x_CountFeats_recursive(GetChildren(), feat_size,m_Context->IsOverviewMode());
    }

    if ( !m_HasLinkedFeats ) {
        // we need to do something differently for features with
        // special display settings.  Currently, this is mainly
        // done for primer features created by cPrimer BLAST.
        const CSeqGlyph* glyph = GetChildren().front();
        if (first_group) {
            glyph = first_group->GetChildren().front();
        }
        bool set_weight = false;
        if (const CClonePlacementGlyph* cp_glyph =
            dynamic_cast<const CClonePlacementGlyph*>(glyph)) {
            set_weight = s_IsSetWeight(cp_glyph->GetFeature());
        } else if (const CFeatGlyph* feat_glyph = dynamic_cast<const CFeatGlyph*>(glyph)) {
            set_weight = s_IsSetWeight(feat_glyph->GetFeature());
        }

        if (set_weight) {
            x_SortFeaturesByWeight(SetChildren());
            m_Layered->SetSorted(true);
        }
    }

    ELinkedFeatDisplay link_mode = m_HasLinkedFeats ? m_LinkedFeat : ELinkedFeatDisplay::eLFD_Invalid;
    bool compact_mode = feat_size > m_CompactThreshold;

    int vert_space = m_gConfig->GetObjectSpace();
    if (compact_mode) {
        vert_space = 1;
        m_Layered->SetMaxRow(m_MaxRow);
    } else {
        m_Layered->SetMaxRow(-1); // no limitation
    }

    m_Layered->SetVertSpace(vert_space);
    m_Column->SetVertSpace(vert_space);
    m_Compact->SetVertSpace(vert_space);
    m_GroupSimple->SetVertSpace(vert_space);
    m_GroupLayered->SetVertSpace(vert_space);
    m_LinkedGroup->SetVertSpace(vert_space);
    
    if (compact_mode  &&  m_HasLinkedFeats) {
        link_mode = ELinkedFeatDisplay::eLFD_Hidden;
    }

    if (first_group) {
        NON_CONST_ITERATE (CSeqGlyph::TObjects, grp_iter, SetChildren()) {
            CNamedGroup* curr_group = dynamic_cast<CNamedGroup*>(grp_iter->GetPointer());
            _ASSERT(curr_group);
            x_RegroupFeats(curr_group->SetChildren(), link_mode);
            x_InitGlyphs_Recursive(curr_group->SetChildren(), 0, compact_mode);
            
            // initialize named group
            curr_group->SetTitleColor(m_gConfig->GetFGCommentColor());
            curr_group->SetBackgroundColor(m_gConfig->GetBGCommentColor());
            curr_group->SetTitleFont(m_gConfig->GetCommentFont().GetPointer());
            curr_group->SetIndent(GetIndent() + 2);
            curr_group->SetRepeatDist(m_gConfig->GetCommentRepeatDist());
            curr_group->SetRepeatTitle(m_gConfig->GetRepeatComment());
            curr_group->SetShowTitle(m_gConfig->GetShowComments());
            x_InitLayoutPolicy(*curr_group, link_mode);
        }
        SetLayoutPolicy(m_GroupSimple);
    } else {
        x_RegroupFeats(SetChildren(), link_mode);
        x_InitGlyphs_Recursive(SetChildren(), 0, compact_mode);
        x_InitLayoutPolicy(SetGroup(), link_mode);
    }
    SetGroup().SetLinkedFeat(link_mode);
}

void CFeatureTrack::x_AddFeaturesLayout(const CSGJobResult& result)
{
    SetGroup().Clear();
    SetMsg("");
    const CSeqGlyph::TObjects& objs = result.m_ObjectList;

    // no features or histogram
    if (objs.empty()  &&  !result.m_ExtraObj) {
        m_Attrs |= fNavigable;
        x_UpdateLayout();
        return;
    }
    
    m_AnnotName = result.m_Desc;

    if (!objs.empty()) { // show individual features
        SetObjects(result.m_ObjectList);
        x_AdjustFeatureLayout();
        // auto feat_size = GetGroup().GetChildren().size();
        // always pack if there are too many of them 
        bool pack = false; // feat_size > kMaxShownFeats;
        // don't pack on sequence level
        bool sequence_level = m_Context->GetScale() <= 1. / 8.;
        if (!sequence_level && m_FeatLimits > 0) { // m_FeatLimits here is a derivative of layout mode
            // update to build layout and get the group's height 
            Update(true);
            pack = GetGroup().GetHeight() > m_MaxAdaptiveHeight;
        }
        if (pack && result.m_ExtraObj) {
            CRef<CSeqGlyph> glyph = result.m_ExtraObj;
            CHistogramGlyph* hist = dynamic_cast<CHistogramGlyph*>(glyph.GetPointer());
            if (hist) {
                SetGroup().Clear();
                x_AddHistogram(hist, m_DS->IsBigBed() ? eLayout_OneLine : -1);
            }

        } else {

            if (x_HasLinkedFeats()) 
                x_RegisterIcon(SIconInfo(eIcon_Content, "Child features", true, "track_content"));
            
            string msg = ", total ";
            const size_t obj_size = objs.size();
            msg += NStr::SizetToString(obj_size, NStr::fWithCommas);
            if (obj_size > 1) {
                msg += m_HasLinkedFeats ? " linked feature groups" : " features";
            } else {
                msg += m_HasLinkedFeats ? " linked feature group" : " feature";
            }
            msg += " shown";
            SetMsg(msg);
            m_Attrs |= fNavigable;
        }

    } else { // show as a histogram
        CRef<CSeqGlyph> hist_glyph = result.m_ExtraObj;
        CHistogramGlyph* hist = dynamic_cast<CHistogramGlyph*>(hist_glyph.GetPointer());
        x_AddHistogram(hist, m_DS->IsBigBed() ? eLayout_OneLine : -1);
    }

    x_UpdateLayout();
}


bool CFeatureTrack::x_NeedHighlight(const CSeqGlyph::TObjects& objs) const
{
    if (GetHighlights().empty()) return false;

    ITERATE (CSeqGlyph::TObjects, iter, objs) {
        const CLayoutGroup* group = dynamic_cast<const CLayoutGroup*>(iter->GetPointer());
        if (group  &&  x_NeedHighlight(group->GetChildren())) {
            return true;
        } else {
            const CFeatGlyph* feat =
                dynamic_cast<const CFeatGlyph*>(iter->GetPointer());
            if (x_NeedHighlight(feat)) {
                return true;
            }
        }
    }
    return false;
}


bool CFeatureTrack::x_NeedHighlight(const CSeqGlyph* glyph) const
{
    string label;
    const CFeatGlyph* feat = dynamic_cast<const CFeatGlyph*>(glyph);
    if (feat) {
        feat->GetLabel(label, CLabel::eContent);
    } else {
        const CClonePlacementGlyph* clone =
            dynamic_cast<const CClonePlacementGlyph*>(glyph);
        if (clone) {
            label = clone->GetLabel();
        }
    }

    NStr::ToLower(label);
    if (GetHighlights().count(label) > 0) {
        return true;
    }

    return false;
}


void CFeatureTrack::x_RegroupFeats(CSeqGlyph::TObjects& objs,
                                   ELinkedFeatDisplay link_mode)
{
    if (link_mode == ELinkedFeatDisplay::eLFD_Packed) {
        // put all the linked child features into one CLinkedFeatsGroup
        NON_CONST_ITERATE (CSeqGlyph::TObjects, iter, objs) {
            CLayoutGroup* group = dynamic_cast<CLayoutGroup*>(iter->GetPointer());
            if (group  &&  group->GetChildrenNum() > 1) {
                CRef<CLinkedFeatsGroup> feat_group;

                CLayoutGroup::TObjectList& child_objs = group->SetChildren();
                size_t num_child = child_objs.size();
                CLayoutGroup::TObjectList::iterator c_iter = child_objs.begin();
                ++c_iter;
                while (c_iter != child_objs.end()) {
                    CRef<CSeqGlyph> child = *c_iter;
                    CLayoutGroup* c_group = dynamic_cast<CLayoutGroup*>(child.GetPointer());
                    if ( !c_group  &&  num_child == 2) {
                        break;
                    }
                    if (!feat_group) {
                        feat_group.Reset(new CLinkedFeatsGroup);
                        CRef<CSeqGlyph> child0 = group->GetChild(0);
                        const CFeatGlyph* p_feat =
                            dynamic_cast<const CFeatGlyph*>(child0.GetPointer());
                        _ASSERT(p_feat);
                        feat_group->SetParentFeat(p_feat);
                        feat_group->SetLabelType(CLinkedFeatsGroup::fLabel_FeatNum);
                    }
                    if (c_group) {
                        x_ExtracGlyphs_Recursive(c_group->SetChildren(), feat_group);
                    } else {
                        feat_group->PushBack(child);
                    }
                    c_iter = child_objs.erase(c_iter);
                }
                if (feat_group) {
                    group->PushBack(feat_group.GetPointer());
                }
            }
        }
    } else if (link_mode == ELinkedFeatDisplay::eLFD_PackedWithParent) {
        // put all the linked child features and the parent feature
        // into one CLinkedFeatsGroup
        NON_CONST_ITERATE (CSeqGlyph::TObjects, iter, objs) {
            CLayoutGroup* group = dynamic_cast<CLayoutGroup*>(iter->GetPointer());
            if (group) {
                CRef<CLinkedFeatsGroup> feat_group(new CLinkedFeatsGroup);
                x_ExtracGlyphs_Recursive(group->SetChildren(), feat_group);
                feat_group->SetParent(&m_Group);
                feat_group->SetLabelType(CLinkedFeatsGroup::fLabel_Both);
                feat_group->SetFirstIsParent();
                iter->Reset(feat_group.GetPointer());
            }
        }
    } else if (link_mode == ELinkedFeatDisplay::eLFD_Hidden) {
        // remove all chidren features
        CSeqGlyph::TObjects::iterator iter = objs.begin();
        while (iter != objs.end()) {
            CLayoutGroup* group = dynamic_cast<CLayoutGroup*>(iter->GetPointer());
            if (group) {
                if (x_NeedHighlight(group->GetChildren())) {
                    if (m_LinkedFeat == ELinkedFeatDisplay::eLFD_Packed  &&  group->GetChildrenNum() == 2) {
                        CRef<CSeqGlyph> child1 = group->GetChild(1);
                        CLayoutGroup* c_group =
                            dynamic_cast<CLayoutGroup*>(child1.GetPointer());
                        if (c_group) {
                            CRef<CLinkedFeatsGroup> feat_group(new CLinkedFeatsGroup);
                            CRef<CSeqGlyph> child0 = group->GetChild(0);
                            const CFeatGlyph* p_feat =
                                dynamic_cast<const CFeatGlyph*>(child0.GetPointer());
                            _ASSERT(p_feat);
                            feat_group->SetParentFeat(p_feat);
                            feat_group->SetLabelType(CLinkedFeatsGroup::fLabel_FeatNum);
                            x_ExtracGlyphs_Recursive(c_group->SetChildren(), feat_group);
                            group->Remove(c_group);
                            group->PushBack(feat_group.GetPointer());
                        }

                    } else if (m_LinkedFeat == ELinkedFeatDisplay::eLFD_PackedWithParent) {
                        CRef<CLinkedFeatsGroup> feat_group(new CLinkedFeatsGroup);
                        x_ExtracGlyphs_Recursive(group->SetChildren(), feat_group);
                        feat_group->SetParent(&m_Group);
                        feat_group->SetLabelType(CLinkedFeatsGroup::fLabel_Both);
                        feat_group->SetFirstIsParent();
                        iter->Reset(feat_group.GetPointer());
                    }
                } else if (group->GetChildrenNum() > 0) {
                    CRef<CSeqGlyph> p_feat = group->SetChildren().front();
                    p_feat->SetParent(group->SetParent());
                    iter->Reset(p_feat.GetPointer());
                }
            }
            ++iter;
        }
    }
}


bool CFeatureTrack::x_InitGlyphs_Recursive(CSeqGlyph::TObjects& objs,
                                           int curr_level,
                                           bool compact_mode)
{
    bool need_highlight = false;
    CConstRef<CFeatureParams> conf;
    if (m_Subtype == CSeqFeatData::eSubtype_variation_ref) {
        conf = m_gConfig->GetFeatParams("dbvar");
    } else {
        conf = m_gConfig->GetFeatParams(m_Subtype);
    }

    NON_CONST_ITERATE (CSeqGlyph::TObjects, iter, objs) {
        CSeqGlyph* glyph = *iter;
        glyph->SetHideLabel(compact_mode);
        glyph->SetRenderingContext(m_Context);
        CLayoutGroup* group = dynamic_cast<CLayoutGroup*>(glyph);
        if (group) {
            if ( !m_GroupConfig ) {
                m_GroupConfig.Reset(new CBoundaryParams(true, false,
                    CRgbaColor(0.6f, 0.6f, 0.6f, 0.6f), CRgbaColor("white"), 1.0));
            }
            if (curr_level % 2 == 0  &&  m_LinkedFeat != ELinkedFeatDisplay::eLFD_ParentHidden) {
                group->SetLayoutPolicy(m_GroupSimple);
                group->SetConfig(m_GroupConfig);
            } else {
                group->SetLayoutPolicy(m_GroupLayered);
            }
            if (x_InitGlyphs_Recursive(group->SetChildren(), curr_level + 1, compact_mode)) {
                need_highlight = true;
                group->SetHighlighted(true);
                if (m_HighlightsColor.get())
                    group->SetHighlightsColor(*m_HighlightsColor.get());
            }
        } else if (CLinkedFeatsGroup* feat_group =
            dynamic_cast<CLinkedFeatsGroup*>(glyph)) {
            feat_group->SetLayoutPolicy(m_LinkedGroup);
            feat_group->SetConfig(m_gConfig->GetFeatParams("LinkedFeats"));
            feat_group->SetGroup().SetRenderingContext(m_Context);

            if (x_InitGlyphs_Recursive(feat_group->SetChildren(), curr_level + 1, compact_mode)) {
                need_highlight = true;
                feat_group->SetHighlighted(true);
                if (m_HighlightsColor.get())
                    feat_group->SetHighlightsColor(*m_HighlightsColor.get());
            }
        } else {
            if (!GetHighlights().empty()  &&  x_NeedHighlight(glyph)) {
                glyph->SetHighlighted(true);
                need_highlight = true;
                glyph->SetHideLabel(false);
                if (m_HighlightsColor.get())
                    glyph->SetHighlightsColor(*m_HighlightsColor.get());
            }

            CFeatGlyph* feat = dynamic_cast<CFeatGlyph*>(glyph);
            if (feat) {
                if (CSeqFeatData::eSubtype_bad != m_Subtype) {
                    feat->SetConfig(conf);
                }
                else {
                    CConstRef<CFeatureParams> feat_conf = m_gConfig->GetFeatParams(feat->GetFeature().GetData().GetSubtype());
                    feat->SetConfig(feat_conf);
                }
            } else {
                CClonePlacementGlyph* cp =
                    dynamic_cast<CClonePlacementGlyph*>(glyph);
                if (cp) {
                    cp->SetConfig(m_gConfig->GetClonePlacementParams());
                }
            }
        }
    }
    return need_highlight;
}


void CFeatureTrack::x_ExtracGlyphs_Recursive(CSeqGlyph::TObjects& objs,
                                             CLinkedFeatsGroup* feat_group)
{
    NON_CONST_ITERATE (CSeqGlyph::TObjects, iter, objs) {
        CLayoutGroup* group = dynamic_cast<CLayoutGroup*>(iter->GetPointer());
        if (group) {
            x_ExtracGlyphs_Recursive(group->SetChildren(), feat_group);
        } else {
            feat_group->PushBack(*iter);
        }
    }
}


void CFeatureTrack::x_CountFeats_recursive(const CSeqGlyph::TObjects& objs,
                                           int& count,
                                           bool is_overview)
{
    ITERATE (CSeqGlyph::TObjects, iter, objs) {
        // If it is in zoomed-in view (not overview), and features are linked,
        // we won't show them in a compact mode, hence no need to count feature
        // number.
        if ( !is_overview  &&  m_HasLinkedFeats) {
            break;
        }
        const CLayoutGroup* group =
            dynamic_cast<const CLayoutGroup*>(iter->GetPointer());
        if (group) {
            if (!m_HasLinkedFeats) {
                m_HasLinkedFeats = true;
            }
            x_CountFeats_recursive(group->GetChildren(), count, is_overview);
        } else {
            ++count;
        }
    }
}


void CFeatureTrack::x_OnContentIconClicked()
{
    wxMenu menu;
    UseDefaultMarginWidth(menu);
    int id_base = 10000;
    ITERATE (TLinkedFeatMap, iter, sm_LinkedFeatMap) {
        string label;
        switch (iter->second) {
            case ELinkedFeatDisplay::eLFD_Expanded:
                label = "Show all";
                break;
            case ELinkedFeatDisplay::eLFD_Hidden:
                label = "Show parent, not children";
                break;
            case ELinkedFeatDisplay::eLFD_Expandable:
                label = "Show parent, expand children upon a click";
                break;
            case ELinkedFeatDisplay::eLFD_Packed:
                label = "Show parent, merge children";
                break;
            case ELinkedFeatDisplay::eLFD_PackedWithParent:
                label = "Merge parent and children";
                break;
            case ELinkedFeatDisplay::eLFD_ParentHidden:
                label = "Show children, not parent";
            default:
                break;
        }

        if ( !label.empty() ) {
            wxMenuItem* item = menu.AppendRadioItem(id_base + iter->second,
                ToWxString(label));

            if (m_LinkedFeat == iter->second) {
                item->Check();
            }
        }
    }

    m_LTHost->LTH_PopupMenu(&menu);
    wxMenuItemList& item_list = menu.GetMenuItems();
    ITERATE (wxMenuItemList, iter, item_list) {
        ELinkedFeatDisplay id = (ELinkedFeatDisplay)((*iter)->GetId() - id_base);
        if ((*iter)->IsChecked()  &&  id != m_LinkedFeat) {
            m_LinkedFeat = id;
            x_SaveSettings("");
            x_UpdateData();
        }
    }
}


void CFeatureTrack::x_OnLayoutIconClicked()
{
    wxMenu menu;
    UseDefaultMarginWidth(menu);
    int id_base = 10000;
    ITERATE (TLayoutMap, iter, sm_LayoutMap) {
        bool l_default = iter->second == eLayout_Default;
        wxMenuItem* item = menu.AppendRadioItem(id_base + iter->second,
            ToWxString(iter->first + (l_default ? " (Default)" : "")));

        if (m_Layout == iter->second) {
            item->Check();
        }
    }

    m_LTHost->LTH_PopupMenu(&menu);
    wxMenuItemList& item_list = menu.GetMenuItems();
    ITERATE (wxMenuItemList, iter, item_list) {
        ELayout id = (ELayout)((*iter)->GetId() - id_base);
        if ((*iter)->IsChecked()  &&  id != m_Layout) {
            m_Layout = id;
            x_SaveSettings("");
            x_OnLayoutPolicyChanged();
            x_UpdateData();
        }
    }
}


void CFeatureTrack::x_InitLayoutPolicy(CLayoutGroup& group,
                                       ELinkedFeatDisplay link_mode)
{
    switch (m_Layout) {
        case eLayout_ExpandedByPos:
        case eLayout_ExpandedBySize:
            group.SetLayoutPolicy(m_Simple);
            break;
        case eLayout_AdaptiveInline:
            group.SetLayoutPolicy(m_Inline);
            break;
        case eLayout_ExpandedColumn:
            group.SetLayoutPolicy(m_Column);
            break;
        default:
            if (link_mode == ELinkedFeatDisplay::eLFD_Expanded  ||
                link_mode == ELinkedFeatDisplay::eLFD_Packed  ||
                link_mode == ELinkedFeatDisplay::eLFD_ParentHidden  ||
                link_mode == ELinkedFeatDisplay::eLFD_PackedWithParent ||
                link_mode == ELinkedFeatDisplay::eLFD_Expandable) {
                    // apply more compact layout, but more expensive
                    group.SetLayoutPolicy(m_Compact);
            } else {
                group.SetLayoutPolicy(m_Layered);
            }
            break;
    }
}


void CFeatureTrack::x_OnLayoutPolicyChanged()
{
    switch (m_Layout) {
        case eLayout_Packed:
            m_FeatLimits = 0;
            m_Simple->SetSortingType(CSimpleLayout::eSort_No);
            break;
        case eLayout_OneLine:
            m_FeatLimits = -2;
            m_Simple->SetSortingType(CSimpleLayout::eSort_No);
            break;
        case eLayout_AdaptiveLayered:
            if (m_HistThreshold > 0) {
                m_FeatLimits = m_HistThreshold;
            } else {
                m_FeatLimits = m_gConfig->GetHistogramCutoff();
            }
            m_Simple->SetSortingType(CSimpleLayout::eSort_No);
            break;
        case eLayout_AdaptiveInline:
            if (m_HistThreshold > 0) {
                m_FeatLimits = m_HistThreshold;
            } else {
                m_FeatLimits = m_gConfig->GetHistogramCutoff();
            }
            m_Simple->SetSortingType(CSimpleLayout::eSort_No);
            break;
        case eLayout_ExpandedLayered:
            m_FeatLimits = m_HistThreshold > 0 ? m_HistThreshold : -1;
            break;
        case eLayout_ExpandedColumn:
            m_FeatLimits = m_HistThreshold > 0 ? m_HistThreshold : -1;
            break;
        case eLayout_ExpandedByPos:
            m_FeatLimits = m_HistThreshold > 0 ? m_HistThreshold : -1;
            m_Simple->SetSortingType(CSimpleLayout::eSort_BySeqPos);
            break;
        case eLayout_ExpandedBySize:
            m_FeatLimits = m_HistThreshold > 0 ? m_HistThreshold : -1;
            m_Simple->SetSortingType(CSimpleLayout::eSort_BySeqSize);
            break;
        default:
            _ASSERT(false);
            break;
    }
}


static bool s_FeatByWeight(const CRef<CSeqGlyph>& g1,
                           const CRef<CSeqGlyph>& g2)
{
    int w1 = -1, w2 = -1;
    if (const CClonePlacementGlyph* cp_glyph =
        dynamic_cast<const CClonePlacementGlyph*>(&*g1)) {
        w1 = s_GetWeight(cp_glyph->GetFeature());
    } else if (const CFeatGlyph* feat_glyph =
        dynamic_cast<const CFeatGlyph*>(&*g1)) {
        w1 = s_GetWeight(feat_glyph->GetFeature());
    }

    if (const CClonePlacementGlyph* cp_glyph =
        dynamic_cast<const CClonePlacementGlyph*>(&*g2)) {
        w2 = s_GetWeight(cp_glyph->GetFeature());
    } else if (const CFeatGlyph* feat_glyph =
        dynamic_cast<const CFeatGlyph*>(&*g2)) {
        w2 = s_GetWeight(feat_glyph->GetFeature());
    }

    return w2 < w1;
}


void CFeatureTrack::x_SortFeaturesByWeight(CSeqGlyph::TObjects& objs) const
{
    // sort feature by weight from large to small
    objs.sort(s_FeatByWeight);
}

///////////////////////////////////////////////////////////////////////////////
/// CFeatureTrackFactory
///////////////////////////////////////////////////////////////////////////////
ILayoutTrackFactory::TTrackMap
CFeatureTrackFactory::CreateTracks(SConstScopedObject& object,
                                   ISGDataSourceContext* ds_context,
                                   CRenderingContext* r_cntx,
                                   const SExtraParams& params,
                                   const TAnnotMetaDataList& src_annots) const
{
    TAnnotNameTitleMap annots;
    TTrackMap tracks;

    TKeyValuePairs track_settings;
    CSGConfigUtils::ParseProfileString(params.m_TrackProfile, track_settings);

    const CFeatList& feats(*CSeqFeatData::GetFeatList());
    CFeatListItem item;
    if ( !feats.GetItemByKey(params.m_Subkey, item) && (params.m_Subkey != "auto") ) {
        return tracks;
    }

    if (params.m_SkipGenuineCheck  &&  !params.m_Annots.empty()) {
        ITERATE (SExtraParams::TAnnots, iter, params.m_Annots) {
            annots.insert(TAnnotNameTitleMap::value_type(*iter, ""));
        }
    } else {

        // collect non-NA tracks
        TSeqRange range = params.m_Range;
        if (range.Empty()) {
            range = r_cntx->GetVisSeqRange();
        }

        CIRef<ISGDataSource> pre_ds = ds_context->GetDS(
            typeid(CSGFeatureDSType).name(), object);
        CSGFeatureDS* seq_ds = dynamic_cast<CSGFeatureDS*>(pre_ds.GetPointer());
        seq_ds->SetDepth(params.m_Level);
        seq_ds->SetAdaptive(params.m_Adaptive);

        // we only create track for feature subtypes that data are available
        SAnnotSelector sel(CSeqUtils::GetAnnotSelector(params.m_Annots));
        sel.SetFeatSubtype((CSeqFeatData::ESubtype)item.GetSubtype());
        seq_ds->GetAnnotNames(sel, range, annots);

        // collect NA tracks
        if ( !src_annots.empty() ) {
            GetMatchedAnnots(src_annots, params, annots);
        }
    }

    // create feature tracks
    ITERATE (TAnnotNameTitleMap, iter, annots) {
        CIRef<ISGDataSource> ds = ds_context->GetDS(
            typeid(CSGFeatureDSType).name(), object);
        CSGFeatureDS* seq_ds = dynamic_cast<CSGFeatureDS*>(ds.GetPointer());
        seq_ds->SetDepth(params.m_Level);
        seq_ds->SetAdaptive(params.m_Adaptive);
        if (track_settings.count("graph_cache_key") > 0) {
            seq_ds->SetGraphCacheKey(track_settings["graph_cache_key"]);
        }
        if (track_settings.count("rmt_type") > 0) {
            seq_ds->SetRemoteDataType(track_settings["rmt_type"]);
        }
        // check the total levels of coverage graphs
        // seq_ds->SetGraphLevels(iter->first);

        CRef<CFeatureTrack> feat_track(new CFeatureTrack(seq_ds, r_cntx));
        string annot(iter->first);
        feat_track->SetAnnot(annot);
        feat_track->SetFeatSubtype(item.GetSubtype());
        if ( !iter->second.empty() ) {
            feat_track->SetTitle(iter->second);
        }
        seq_ds->SetFilter(params.m_Filter);
        seq_ds->SetSortBy(params.m_SortBy);
        tracks[annot] = feat_track.GetPointer();
    }

    return tracks;
}

void CFeatureTrackFactory::GetMatchedAnnots(
    const TAnnotMetaDataList& src_annots,
    const ILayoutTrackFactory::SExtraParams& params,
    TAnnotNameTitleMap& out_annots) const
{
    ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "ftable", params.m_Subkey, out_annots);
}

void CFeatureTrackFactory::CloneTrack(const CLayoutTrack* track,
        CTempTrackProxy* track_proxy) const
{
    const CFeatureTrack* f_track = dynamic_cast<const CFeatureTrack*>(track);
    CTrackProxy* tp = dynamic_cast<CTrackProxy*>(track_proxy);
    if (f_track  &&  tp) {
        tp->SetKey(GetExtensionIdentifier());
        const CFeatList& feats(*CSeqFeatData::GetFeatList());
        tp->SetSubkey(feats.GetStoragekey(f_track->GetFeatSubtype()));
    }
}


string CFeatureTrackFactory::GetExtensionIdentifier() const
{
    return CFeatureTrack::m_TypeInfo.GetId();
}


string CFeatureTrackFactory::GetExtensionLabel() const
{
    return CFeatureTrack::m_TypeInfo.GetDescr();
}


CRef<CTrackConfigSet>
CFeatureTrackFactory::GetSettings(const string& /*profile*/,
                                  const TKeyValuePairs& settings,
                                  const CTempTrackProxy* track_proxy) const
{
    CRef<CTrackConfigSet> config_set(new CTrackConfigSet);
    CRef<CTrackConfig> config(new CTrackConfig);
    config_set->Set().push_back(config);
    if (!config->CanGetHelp()) // Don't overwrite the help, provided in the registry
        config->SetHelp() = GetThisTypeInfo().GetDescr();
    config->SetLegend_text("anchor_1");

    // we only expose one set of settings, either layout settings or
    // rendering options.  By default, we use layout settings, but
    // whenever rendering options appear, we use rendering options.
    bool use_layout_settings = true;

    // JIRA SV-1794. We decided to expose feature linking option
    // for all feature tracks regarless of having linked features
    // or not. After moving track discovery from seqconfig.cgi
    // to SVDL module, seqconfig.cgi has no information if a user
    // data track has linked feature or not.
    bool has_linked_feats = true;

    CFeatureTrack::ELayout layout = m_DefaultLayout;

    // remote tracks need a different default
    const CTrackProxy* proxy = dynamic_cast<const CTrackProxy*>(track_proxy);
    if(proxy && CSeqUtils::isRmtPipelineFileType(proxy->GetDB())) {
        layout = CFeatureTrack::eLayout_ExpandedLayered;
    }
    string rendering_str = CFeatureTrack::LinkedFeatDisplayValueToStr(m_DefaultLinkedFeat);
    ELinkedFeatDisplay link_style = m_DefaultLinkedFeat;

    ITERATE (TKeyValuePairs, iter, settings) {
        try {
            if (NStr::EqualNocase(iter->first, "Layout")) {
                layout = CFeatureTrack::LayoutStrToValue(iter->second);
            } else if (NStr::EqualNocase(iter->first, "Rendering")) {
                rendering_str = iter->second;
                use_layout_settings = false;
            } else if (NStr::EqualNocase(iter->first, "LinkedFeat")) {
                link_style = CFeatureTrack::LinkedFeatDisplayStrToValue(iter->second);
                has_linked_feats = true;
            }

        } catch (CException& e) {
            LOG_POST(Warning << "CFeatureTrack::x_LoadSettings() "
                << e.GetMsg());
        }
    }

    // add layout dropdown list
    if (use_layout_settings) {
        CRef<CChoice> choice = CTrackConfigUtils::CreateChoice(
            "Layout", "Rendering options",
            CFeatureTrack::LayoutValueToStr(layout),
            "Rendering style on how to layout features.");

        choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
                CFeatureTrack::LayoutValueToStr(CFeatureTrack::eLayout_Packed),
                "All features on a single line",
                "Track is displayed, with all features collapsed on a single line.",
                "All the features are packed and shown as a Distribution histogram."));
        choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
                CFeatureTrack::LayoutValueToStr(CFeatureTrack::eLayout_AdaptiveLayered),
                "Pack features if necessary",
                "",
                ""));
        choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
                CFeatureTrack::LayoutValueToStr(CFeatureTrack::eLayout_ExpandedLayered),
                "All features are expanded",
                "Track is displayed with features expanded, and on separate lines if needed.",
                "Always show the individual features and layer the features in a compact form"));
        config->SetChoice_list().push_back(choice);
        if (has_linked_feats) {
            choice = CTrackConfigUtils::CreateChoice(
                "LinkedFeat", "Feature Linking",
                CFeatureTrack::LinkedFeatDisplayValueToStr(link_style),
                "Rendering style on how to render linked features.");
            choice->SetValues().push_back(
                CTrackConfigUtils::CreateChoiceItem(
                    CFeatureTrack::LinkedFeatDisplayValueToStr(ELinkedFeatDisplay::eLFD_Default),
                    "Show all",
                    "All features, both parent and all child fetaures",
                    "The black bar on top represents the parent feature. Each bar blow represents "
                    "a child feature."));
            choice->SetValues().push_back(
                CTrackConfigUtils::CreateChoiceItem(
                    CFeatureTrack::LinkedFeatDisplayValueToStr(ELinkedFeatDisplay::eLFD_Packed),
                    "Show parent, Merge children",
                    "Show parent on one line and all children merged below it.",
                    "The black bar on top represents the parent feature. Each bar blow represents "
                    "a child feature."));

            choice->SetValues().push_back(
                CTrackConfigUtils::CreateChoiceItem(
                    CFeatureTrack::LinkedFeatDisplayValueToStr(ELinkedFeatDisplay::eLFD_ParentHidden),
                    "Show children, not parent",
                    "Show only the chidlren,  not parent",
                    "The bar represent children features"));

            choice->SetValues().push_back(
                CTrackConfigUtils::CreateChoiceItem(
                    CFeatureTrack::LinkedFeatDisplayValueToStr(ELinkedFeatDisplay::eLFD_Hidden),
                    "Show parent, not children ",
                    "Show the parent features, not the children",
                    "The black bar represents parent feature."));

            choice->SetValues().push_back(
                CTrackConfigUtils::CreateChoiceItem(
                    CFeatureTrack::LinkedFeatDisplayValueToStr(ELinkedFeatDisplay::eLFD_PackedWithParent),
                    "Superimpose children on parent",
                    "Render all child features on the parent feature",
                    "The underneath black bar represents parent feature and all the red bars on top represent the children."));

            config->SetChoice_list().push_back(choice);
        }

    } else {
        CRef<CChoice> choice = CTrackConfigUtils::CreateChoice(
            "Rendering", "Rendering options",
            rendering_str,
            "Rendering options for linked features.");

        choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
                CFeatureTrack::LinkedFeatDisplayValueToStr(ELinkedFeatDisplay::eLFD_Default),
                "Show all",
                "All features, both parent and all child fetaures",
                "The black bar on top represents the variant region. Each bar blow represents \
the supporting level variants used to define this region.<br>Red: A Deletion or Loss<br> \
Green: A Gain or Insertion<br>Light Gray: An Inversion"));
        choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
                CFeatureTrack::LinkedFeatDisplayValueToStr(ELinkedFeatDisplay::eLFD_Packed),
                "Show parent, Merge children",
                "Show parent on one line and all children merged below it.",
                "The black bar on top represents the variant region. The bar blow represents \
all of the sample level variants merged onto a single line.<br>Red: A Deletion or Loss<br> \
Green: A Gain or Insertion<br>Light Gray: An Inversion"));

        choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
                CFeatureTrack::LinkedFeatDisplayValueToStr(ELinkedFeatDisplay::eLFD_ParentHidden),
                "Show children, not parent",
                "Show only the supporting variants (chidlren) not the variant region (parent)",
                "The bar represent supporting variants in this study.<br>Red: A Deletion or Loss<br>\
Green: A Gain or Insertion<br>Light Gray: An Inversion"));

        choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
                CFeatureTrack::LinkedFeatDisplayValueToStr(ELinkedFeatDisplay::eLFD_Hidden),
                "Show parent, not children ",
                "Show the variant region (parent) not the supporting variants (children)",
                "The black bar represents the variant region."));

//        choice->SetValues().push_back(
//            CTrackConfigUtils::CreateChoiceItem(
//             CFeatureTrack::LinkedFeatDisplayValueToStr(ELinkedFeatDisplay::eLFD_PackedWithParent),
//             "Show on single line",
//             "Render all child features on the parent feature",
//             "The underneath black bar represents the variant region and all the red bars on top represent the supporting variants."));
        choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
                "SingleLine",
                "Display on a single line",
                "Track is displayed, with all features collapsed on a single line.",
                "All the features are packed into one line."));
        choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
                "Histogram",
                "Histogram",
                "Track is displayed, with all features displayed as a histogram.",
                "All the features are packed and shown as a Distribution histogram."));

        config->SetChoice_list().push_back(choice);
    }

    if (track_proxy  &&  !track_proxy->GetSortBy().empty())  {
        string sort_by = track_proxy->GetSortBy();

        CRef<CChoice> sortby_choice = CTrackConfigUtils::CreateChoice(
            "sort_by", "Sort features by",
            sort_by,
            "Sort features according to a selected criterion.");

        sortby_choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
            "", "No sorting", "Don't sort features", ""));

        IObjectSorter::SSorterDescriptor descr = CCloneConcordancySorter::GetSorterDescr();
        if (descr.m_Name == sort_by) {
            // must be a clone feature track, use different legend help
            config->SetLegend_text("anchor_3");
        } else { // otherwise, it is GRC issues
            descr = CGRCStatusSorter::GetSorterDescr();
        }

        sortby_choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
            descr.m_Name, descr.m_DisplayName, descr.m_Descr, ""));
        config->SetChoice_list().push_back(sortby_choice);
    }

    return config_set;
}


END_NCBI_SCOPE
