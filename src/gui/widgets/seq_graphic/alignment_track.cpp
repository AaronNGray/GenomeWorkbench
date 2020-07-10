/*  $Id: alignment_track.cpp 44262 2019-11-20 19:12:00Z shkeda $
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
 * Authors:  Liangshou Wu, Andrei Shkeda
 *
 * File Description:
 *  There are some fairly complicated logics on determining how to display
 *  the alignments based on several different factors:
 *    - Incoming data source: regular alignments vs cSRA/BAM alignments
 *    - Alignment display (layout): Smear (packed), AdaptiveSeq, and so on,
 *      see the comment on CAlignmentTrack::ELayout for more details.
 *    - Zoom level: high level vs pileup display level vs low level
 *    - Pileup display setting: enabled vs disabled
 *    - Alignment type: DNA vs protein vs mixed
 *
 *  Here are preknowledge:
 *    - cSRA/BAM files have precomputed coverage graphs
 *    - cSRA/Bam loader is able to deliver alignment pileup quickly
 *    - For regular alignments, pileup calculation is expensive and slow.
 *    - There is a (upper) limit determining what zoom level is appropriate
 *      to show alignment pileup (for performance consideration).
 *    - Alignment display setting determines when (what zoom level) to show
 *      individual alignments.
 *
 *  And here are the requirements:
 *    1. At high level, the alignments shall be shown in a compact form
 *       that is either coverage graph (for cSRA data) or smear bar (for
 *       regular alignemnts). The high level is defined when the zoom level
 *       reaches to a zoom level,
 *       a. that is over the predefined pileup display level for cSRA data, or
 *       b. at which there are more alignments than a predefined number
 *          determined by the alignment display option within the visible
 *          rnage for regular alignments.
 *    2. For regular alignments with pileup display enabled and for all cSRA
 *       data regardless of piplup display enabled or not, if the zoom level
 *       is between the predefined pileup zoom level and the zoom level
 *       determined by alignment display setting, the pileup display, instead
 *       of smear bar or coverage graph, shall be shown.
 *    3. At low level, all individual alignments shall be shown except for
 *       one alignment display setting which is 'Smear'. With this setting,
 *       alignments shall be shown in a packed form similar to high level mode.
 *       The low level is defined when zoom level reaches to either:
 *         a. sequence level, or
 *         b. any zoom level at which there are less alignments than the
 *            predefined number determined by the alignment display setting.
 *       At this level, if pileup display is enabled, both individual
 *       alignments and pileup shall be shown.
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/seq_graphic/alignment_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_graph_ds.hpp>
#include <gui/widgets/seq_graphic/gene_model_track.hpp>
#include <gui/widgets/seq_graphic/gene_model_group.hpp>
#include <gui/widgets/seq_graphic/feature_track.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <gui/widgets/seq_graphic/layout_track_proxy.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/alignment_glyph.hpp>
#include <gui/widgets/seq_graphic/mate_pair_glyph.hpp>
#include <gui/widgets/seq_graphic/alignment_smear_glyph.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <gui/widgets/seq_graphic/coord_mapper.hpp>
#include <gui/widgets/seq_graphic/named_group.hpp>
#include <gui/widgets/seq_graphic/alignment_sorter_factory.hpp>
#include <gui/widgets/seq_graphic/alignment_sorter.hpp>
#include <gui/widgets/seq_graphic/alignment_quality_score.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/seq/feature_check_dialog.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/na_utils.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/registry.hpp>

#include <objmgr/util/sequence.hpp>

#include <objmgr/impl/scope_info.hpp>

#include <wx/event.h>
#include <wx/menu.h>

#include "wx_aln_stat_dlg.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

// menu item id for projected features
static const size_t kProjectedFeatID = 10000;
// content menu item base id
static const size_t kContentBaseID = 10001;
// potential maximal menu item id range
// assumming there will be less than 100 menu items
static const int kMaxID = 100;

/// Scale at which we unconditionally switch to coverage graph
static const double kMinCoverageZoomLevel = 250.;
/// Range length for unconditional swith to pieup 
static const int kShowPileUpRange = 10000;


// Config key for projected CDS features
//static const char* const kCDSKey = "GBPlugins.SeqGraphicGeneModelCDS";
static const string kGeneModelBaseKey = "GBPlugins.SeqGraphicGeneModel";

class CAlnContentEvtHandler : public wxEvtHandler
{
public:
    CAlnContentEvtHandler(CAlignmentTrack* track)
        : m_Track( track )
    {}

    void OnContextMenu( wxContextMenuEvent& anEvent );

private:
    /// @name event handlers.
    /// @{
    /// on toggle a conent menu item.
    void x_OnToggleContent(wxCommandEvent& event)
    {
        int track_order = event.GetId() - kContentBaseID;
        m_Track->OnToggleContent(track_order);
    }

    /// on change the projected feature list.
    void x_OnChangeProjectedFeats(wxCommandEvent& /*event*/)
    { m_Track->OnChangeProjectedFeats(); }

    /// @}

private:
    CAlignmentTrack* m_Track;

    DECLARE_EVENT_TABLE()
};


BEGIN_EVENT_TABLE(CAlnContentEvtHandler, wxEvtHandler)
    EVT_MENU(kProjectedFeatID, CAlnContentEvtHandler::x_OnChangeProjectedFeats)
    EVT_MENU_RANGE(kContentBaseID, kContentBaseID + kMaxID, CAlnContentEvtHandler::x_OnToggleContent)
END_EVENT_TABLE()


///////////////////////////////////////////////////////////////////////////////
///   CAlignmentTrack


int CAlignmentTrack::x_LayoutToCutoff(CAlignmentTrack::ELayout layout)
{
    int cut_off = m_MaxAlignShownAdaptive;
    switch (layout) {
        case CAlignmentTrack::eLayout_Packed:
            cut_off = -1;
            break;
        case CAlignmentTrack::eLayout_Adaptive:
            cut_off = m_MaxAlignShownAdaptive;
            break;
        case CAlignmentTrack::eLayout_Full:
        case CAlignmentTrack::eLayout_ExpandedByPos:
            cut_off = m_MaxAlignShownFull;
            break;
    }
    return cut_off;
}

int CAlignmentTrack::x_LayoutToCutoff_SRA(CAlignmentTrack::ELayout layout)
{
    int cut_off = m_MaxAlignShownAdaptive;
    switch (layout) {
        case CAlignmentTrack::eLayout_Packed:
            cut_off = -1;
            break;
        case CAlignmentTrack::eLayout_Adaptive:
            cut_off = m_MaxAlignShownAdaptive / 3;
            break;
        case CAlignmentTrack::eLayout_Full:
        case CAlignmentTrack::eLayout_ExpandedByPos:
            cut_off = m_MaxAlignShownFull;
            break;
    }
    return cut_off;
}



/// layout style to layout display name
static const map<CAlignmentTrack::ELayout, string> sm_LayoutDispMap = {
    { CAlignmentTrack::eLayout_Packed,          "Packed" },
    { CAlignmentTrack::eLayout_Adaptive,    "Adaptive" },
    { CAlignmentTrack::eLayout_ExpandedByPos,  "Ladder (one alignment per row)" },
    { CAlignmentTrack::eLayout_Full,  "Show All" }
};

const string& s_LayoutToDisplayName(CAlignmentTrack::ELayout layout)
{
    const auto iter = sm_LayoutDispMap.find(layout);
    if (iter != sm_LayoutDispMap.end()) {
        return iter->second;
    }
    return kEmptyStr;
}

// layout style to/from layout string/name

static const map<string, CAlignmentTrack::ELayout> sm_LayoutMap = {
    { "Adaptive",           CAlignmentTrack::eLayout_Adaptive },
    { "Full",               CAlignmentTrack::eLayout_Full },
    { "Expanded",           CAlignmentTrack::eLayout_ExpandedByPos },
    { "Smear",              CAlignmentTrack::eLayout_Packed },
    { "Packed",             CAlignmentTrack::eLayout_Packed }
};



CAlignmentTrack::ELayout CAlignmentTrack::LayoutStrToValue(const string& layout)
{
    const auto iter = sm_LayoutMap.find(layout);
    if (iter != sm_LayoutMap.end())
        return iter->second;
    if (NStr::FindNoCase(layout, "Adaptive") != string::npos) // compatibility with old methods
        return CAlignmentTrack::eLayout_Adaptive;
    LOG_POST(Warning << "CAlignmentTrack settings. Invalid layout string: '" << layout << "'");
    return eLayout_Default;
}


const string& CAlignmentTrack::LayoutValueToStr(CAlignmentTrack::ELayout layout)
{
    for (auto& iter : sm_LayoutMap) {
        if (iter.second == layout) {
            return iter.first;
        }
    }
    return kEmptyStr;
}


typedef SStaticPair<const char*, CAlignmentConfig::ELabelPosition> TLabelPosStr;
static const TLabelPosStr s_LabelPosStrs[] = {
    { "above",    CAlignmentConfig::ePos_Above },
    { "no label", CAlignmentConfig::ePos_NoLabel },
    { "side",     CAlignmentConfig::ePos_Side },
};

typedef CStaticArrayMap<string, CAlignmentConfig::ELabelPosition> TLabelPosMap;
DEFINE_STATIC_ARRAY_MAP(TLabelPosMap, sm_LabelPosMap, s_LabelPosStrs);


CAlignmentConfig::ELabelPosition
CAlignmentTrack::LabelPosStrToValue(const string& pos)
{
    TLabelPosMap::const_iterator iter = sm_LabelPosMap.find(pos);
    if (iter != sm_LabelPosMap.end()) {
        return iter->second;
    }
    NCBI_THROW(CException, eInvalid, "Invalid label position string: " + pos);
}


const string&
CAlignmentTrack::LabelPosValueToStr(CAlignmentConfig::ELabelPosition pos)
{
    TLabelPosMap::const_iterator iter;
    for (iter = sm_LabelPosMap.begin();  iter != sm_LabelPosMap.end();  ++iter) {
        if (iter->second == pos) {
            return iter->first;
        }
    }
    return kEmptyStr;
}

typedef SStaticPair<const char*, CAlignmentConfig::EHideSraAlignments> THideSraStr;
static const THideSraStr s_HideSraStrs[] = {
    { "bad",        CAlignmentConfig::eHide_BadReads },
    { "both",       CAlignmentConfig::eHide_Both },
    { "duplicates", CAlignmentConfig::eHide_Duplicates },
    { "none",       CAlignmentConfig::eHide_None },
};

typedef CStaticArrayMap<string, CAlignmentConfig::EHideSraAlignments> THideSraMap;
DEFINE_STATIC_ARRAY_MAP(THideSraMap, sm_HideSraMap, s_HideSraStrs);

CAlignmentConfig::EHideSraAlignments CAlignmentTrack::HideFlagStrToValue(const string& hideSra)
{
    THideSraMap::const_iterator iter = sm_HideSraMap.find(hideSra);
    if (iter != sm_HideSraMap.end()) {
        return iter->second;
    }
    return CAlignmentConfig::eHide_None;
}

const string& CAlignmentTrack::HideFlagValueToStr(CAlignmentConfig::EHideSraAlignments hideSra)
{
    THideSraMap::const_iterator iter;
    for (iter = sm_HideSraMap.begin();  iter != sm_HideSraMap.end();  ++iter) {
        if (iter->second == hideSra) {
            return iter->first;
        }
    }
    return kEmptyStr;
}

// Unaligned tails
typedef SStaticPair<const char*, CAlignmentConfig::EUnalignedTailsMode> TUnalignedTailsModeStr;
static const TUnalignedTailsModeStr s_UnalignedTailsModeStrs[] = {
    { "glyph",      CAlignmentConfig::eTails_ShowGlyph },
    { "hide",       CAlignmentConfig::eTails_Hide },
    { "sequence",   CAlignmentConfig::eTails_ShowSequence },
};

typedef CStaticArrayMap<string, CAlignmentConfig::EUnalignedTailsMode> TUnalignedTailsModeMap;
DEFINE_STATIC_ARRAY_MAP(TUnalignedTailsModeMap, sm_UnalignedTailsModeMap, s_UnalignedTailsModeStrs);

CAlignmentConfig::EUnalignedTailsMode CAlignmentTrack::UnalignedTailsModeStrToValue(const string& tailsMode)
{
    TUnalignedTailsModeMap::const_iterator iter = sm_UnalignedTailsModeMap.find(tailsMode);
    if (iter != sm_UnalignedTailsModeMap.end()) {
        return iter->second;
    }
    return CAlignmentConfig::eTails_ShowGlyph;
}

const string& CAlignmentTrack::UnalignedTailsModeValueToStr(CAlignmentConfig::EUnalignedTailsMode tailsMode)
{
    TUnalignedTailsModeMap::const_iterator iter;
    for (iter = sm_UnalignedTailsModeMap.begin(); iter != sm_UnalignedTailsModeMap.end(); ++iter) {
        if (iter->second == tailsMode) {
            return iter->first;
        }
    }
    return kEmptyStr;
}

static const string kAlignGlyphKey = "GBPlugins.SeqGraphicAlignGlyphs";
static const string kMatePairKey = "GBPlugins.SeqGraphicMatePair";
static const string kBaseKey = "GBPlugins.SeqGraphicAlignment";

static const string kTrackName = "Alignments";
static const string kDefProfile = "Default";
static const string kReadsProfile = "BAM";

CTrackTypeInfo CAlignmentTrack::m_TypeInfo("alignment_track",
                                           "Graphical View Alignment Track");

CAlignmentTrack::CAlignmentTrack(CSGAlignmentDS* ds, CRenderingContext* r_cntx)
    : CDataTrack(r_cntx)
    , m_ContentHandler(new CAlnContentEvtHandler(this))
    , m_DS(ds)
    , m_Layout(eLayout_Default)
    , m_ShowAlignedSeqFeats(false)
    , m_ShowLabel(true)
    , m_ShowUnalignedTailsForTrans2GenomicAln(true)
    , m_ShowAlnStat(false)
    , m_ShowIdenticalBases(false)
    , m_HideSraAlignments(CAlignmentConfig::eHide_None)
    , m_UnalignedTailsMode(CAlignmentConfig::eTails_ShowGlyph)
    , m_CompactEnabled(true)
    , m_CompactThreshold(100)
    , m_ExtremeCompactThreshold(1000)
    , m_AlnType(IAlnExplorer::fInvalid)
    , m_ObjNum(0)
    , m_Column(new CColumnLayout)
    , m_GeneModelLayout(new CSimpleLayout)
{
    m_DS->SetJobListener(this);
    // initialize annotation selector
    SetLayoutPolicy(m_Column);
    x_RegisterIcon(SIconInfo(eIcon_Content, "Content", true, "track_content"));
    x_RegisterIcon(SIconInfo(eIcon_Layout, "Layout style",
        true, "track_layout"));
    x_RegisterIcon(SIconInfo(eIcon_Score, "Alignment score coloration",
        true, "track_align_score"));
    x_RegisterIcon(SIconInfo(eIcon_Stat, "Alignment statistics settings",
        true, "track_stat"));
    x_RegisterIcon(SIconInfo(eIcon_Tails, "Unaligned tails display settings",
        true, "track_tails"));

    m_Column->SetVertSpace(2);
    m_GeneModelLayout->SetVertSpace(3);
    m_Simple->SetVertSpace(2);

    // We want to put all alignments in one group if we want to cut off
    // the number of rows displayed during layout. If we have multiple groups,
    // the row number can not be easily determined
    m_Column->SetLimitRowPerGroup(false);
}


CAlignmentTrack::~CAlignmentTrack()
{
    try {
        m_DS->DeleteAllJobs();
        SetGroup().Clear();
        m_DS->ClearCache();
        bool gbench_mode = !(m_gConfig && m_gConfig->GetCgiMode());
        // gbench: release memory if there is a need 
        // sviewer: release memory occurs in m_DS destructor
        if (gbench_mode && m_DS->NeedReleaseMemory())
            m_DS->ReleaseMemory();
    } catch (exception& e) {
        ERR_POST(Error << e.what());
    }
}



const CTrackTypeInfo& CAlignmentTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}


string CAlignmentTrack::GetFullTitle() const
{
    string title = GetTitle();
    if (title.empty()) {
        if (CSeqUtils::IsUnnamed(m_AnnotName)) {
            title = "Other alignments";
        } else {
            title = m_AnnotName;
        }
    }

    return title;
}


void CAlignmentTrack::SetAnnot(const string& annot)
{
    m_AnnotName = annot;
}


void CAlignmentTrack::OnToggleContent(int id)
{
    // the menu item order:
    // 0 - "Show labels"
    // 1 - "Show unaligned tails for all alignments"
    // 2 - "Show unaligned tails for transcript-to-genomic alignments"
    // 3 - "Project features for aligned sequences"
    // 4 - "Link mate pairs"
    // 5 - Show Identical Bases"
    switch (id) {
    case 0:
        {{
            m_ShowLabel = !m_ShowLabel;
            if (m_MultiAlignConf) {
                m_MultiAlignConf->m_ShowLabel = m_ShowLabel;
            }
            if (m_PWAlignConf) {
                m_PWAlignConf->m_ShowLabel = m_ShowLabel;
            }
            if (m_AlignSmearConf) {
                m_AlignSmearConf->m_ShowLabel = m_ShowLabel;
            }
            if (m_MatePairConf) {
                m_MatePairConf->m_ShowLabel = m_ShowLabel;
            }
            break;
        }}

    case 1:
        {{
            m_ShowAlnStat = !m_ShowAlnStat;
            break;
        }}
    case 2:
        m_ShowAlignedSeqFeats = !m_ShowAlignedSeqFeats;
        break;
    case 3:
        m_DS->SetLinkMatePairs( !m_DS->GetLinkMatePairs() );
        break;
    case 4:
        {
            string sort_by = (m_DS->GetSortBy() != CAlignStrandSorter::GetID()) ? CAlignStrandSorter::GetID() : "";
            m_DS->SetSortBy(sort_by);
        }
        break;
    case 5:
        {
            string sort_by = (m_DS->GetSortBy() != CAlignSorterFactory::GetHaplotypeID()) ? CAlignSorterFactory::GetHaplotypeID() : "";
            m_DS->SetSortBy(sort_by);
        }
        break;
    case 6:
        {
            m_ShowIdenticalBases = !m_ShowIdenticalBases;
            if (m_MultiAlignConf) {
                m_MultiAlignConf->m_ShowIdenticalBases = m_ShowIdenticalBases;
            }
            if (m_PWAlignConf) {
                m_PWAlignConf->m_ShowIdenticalBases = m_ShowIdenticalBases;
            }
        }
        break;
    default:
        break;
    }
    x_UpdateData();
}


void CAlignmentTrack::OnChangeProjectedFeats()
{
    CFeatureCheckDialog dlg(NULL);
    CFeatureCheckDialog::TFeatTypeItemSet items_set;
	const CFeatList* feat_list = CSeqFeatData::GetFeatList();
    CFeatListItem item;
    ITERATE(TSubtypeVec, subtype, m_ProjectedFeats) {
        feat_list->GetItemBySubtype(*subtype, item);
        items_set.insert(item);
    }
    dlg.SetSelected(items_set);
    IGlyphDialogHost* dlg_host = dynamic_cast<IGlyphDialogHost*>(m_LTHost);
    if (dlg_host) {
        dlg_host->PreDialogShow();
    }
    if(dlg.ShowModal() == wxID_OK) {
        dlg.GetSelected(items_set);
        m_ProjectedFeats.clear();
        ITERATE(CFeatureCheckDialog::TFeatTypeItemSet, iter, items_set) {
            m_ProjectedFeats.push_back(iter->GetSubtype());
        }
        x_UpdateData();
    }
    if (dlg_host) {
        dlg_host->PostDialogShow();
    }
}


void CAlignmentTrack::x_LoadSettings(const string& preset_style,
                                     const TKeyValuePairs& settings)
{
    if (preset_style.empty() || preset_style == kDefProfile) {
        SetProfile(m_DefaultProfile);
    } else {
        SetProfile(preset_style);
    }

    if (!m_StatConf) {
        m_StatConf.Reset(new CAlnStatConfig);
    }


    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryReadView view =
        CSGConfigUtils::GetReadView(registry, kBaseKey, GetProfile(), kDefProfile);

    m_DS->SetLinkMatePairs(view.GetBool("LinkMatePairAligns", false));
    m_DS->SetDNAScoringMethod(view.GetString("DNAScoringMethod"));
    m_DS->SetProteinScoringMethod(view.GetString("ProteinScoringMethod"));
    m_DS->SetEnableColoration(view.GetBool("EnableColoration", true));
    {
        string memory_limit = view.GetString("memory_limit", "2GB");
        m_DS->SetMemoryLimit(NStr::StringToUInt8_DataSize(memory_limit));
    }
    m_ShowAlignedSeqFeats = view.GetBool("ShowAlignedSeqFeats", false);
    m_ShowIdenticalBases = view.GetBool("ShowIdenticalBases", false);
    m_ShowLabel = view.GetBool("ShowLabel", false);
    m_ShowUnalignedTailsForTrans2GenomicAln = view.GetBool("ShowUnalignedTailsForTrans2GenomicAln", true);
    m_ShowAlnStat = view.GetBool("ShowAlnStat", false);
    m_CompactEnabled = view.GetBool("SupportCompactDisplay");
    m_CompactThreshold = (size_t)view.GetInt("CompactThreshold", 100);
    m_ExtremeCompactThreshold = (size_t)view.GetInt("ExtremeCompactThreshold", 1000);
    m_StatConf->m_Display = view.GetInt("StatDisplay", 15);
    m_StatConf->m_StatZoomLevel = view.GetInt("StatZoomLevel", 100);
    m_HideSraAlignments = HideFlagStrToValue(view.GetString("HideSraAlignments"));
    m_UnalignedTailsMode = UnalignedTailsModeStrToValue(view.GetString("UnalignedTailsMode"));
    m_Layout = LayoutStrToValue(view.GetString("Layout"));

    m_MinPileUpCost = view.GetReal("MinPileUpCost", 3.);
    m_MinAlignCost = view.GetReal("MinAlignCost", 1.5);
    m_MaxAlignCost = view.GetReal("MaxAlignCost", 10.);
    m_MaxAlignShownFull = view.GetInt("MaxAlignShownFull", 250000);
    m_MaxAlignShownAdaptive = view.GetInt("MaxAlignShownAdaptive", 7500);

    CConstRef<CUser_field> f = view.GetField("ProjectedFeats");
    if (f  &&  f->GetData().Which() == CUser_field::TData::e_Str) {
        string str = f->GetData().GetStr();
        NStr::TruncateSpacesInPlace(str);
        list<string> toks;
        NStr::Split(str, ", |", toks, NStr::fSplit_Tokenize);
        try {
            ITERATE(list<string>, iter, toks)
            {
                m_ProjectedFeats.push_back(NStr::StringToInt(*iter));
            }
        } catch (CException& e) {
            LOG_POST(Warning << "CAlignmentTrack::x_LoadSettings error on "
                     << "parsing projected feature list: " << e.GetMsg());
        }
    }

    m_MultiAlignProfile = view.GetString("MultiAlignProfile", kDefProfile);
    m_PWAlignProfile = view.GetString("PWAlignProfile", kDefProfile);
    m_AlignSmearProfile = view.GetString("AlignSmearProfile", kDefProfile);
    m_MatePairProfile = view.GetString("MatePairProfile", kDefProfile);
    {
        CRegistryReadView color_view = CSGConfigUtils::GetColorReadView(registry, kBaseKey, GetProfile(), m_gConfig->GetColorTheme(), kDefProfile);
        CSGConfigUtils::GetColor(color_view, "A", m_StatConf->m_Colors[CAlnStatConfig::eStat_A]);
        CSGConfigUtils::GetColor(color_view, "T", m_StatConf->m_Colors[CAlnStatConfig::eStat_T]);
        CSGConfigUtils::GetColor(color_view, "G", m_StatConf->m_Colors[CAlnStatConfig::eStat_G]);
        CSGConfigUtils::GetColor(color_view, "C", m_StatConf->m_Colors[CAlnStatConfig::eStat_C]);
        CSGConfigUtils::GetColor(color_view, "Gap", m_StatConf->m_Colors[CAlnStatConfig::eStat_Gap]);
        CSGConfigUtils::GetColor(color_view, "Match", m_StatConf->m_Colors[CAlnStatConfig::eStat_Match]);
        CSGConfigUtils::GetColor(color_view, "Mismatch", m_StatConf->m_Colors[CAlnStatConfig::eStat_Mismatch]);
        CSGConfigUtils::GetColor(color_view, "Total", m_StatConf->m_Colors[CAlnStatConfig::eStat_Total]);
    }
    CRef<CHistParams> graph_params = x_GetGraphParams();
    {
        auto size_view = CSGConfigUtils::GetSizeReadView(registry, kBaseKey, GetProfile(), m_gConfig->GetSizeLevel(), kDefProfile);
        _ASSERT(graph_params);
        if (graph_params) {
            graph_params->m_Height = size_view.GetInt("GraphHeight", 45);
            graph_params->m_Scale = CHistParams::ScaleStrToValue(view.GetString("GraphScale", "log2"));
        }
    }
    
    // override those default settings passed in through profile settings
    ITERATE (TKeyValuePairs, iter, settings) {
        try {
            if (NStr::EqualNocase(iter->first, "Color")) {
                if (NStr::EqualNocase(iter->second, "false")) {
                    m_DS->SetEnableColoration(false);
                } else {
                    m_DS->SetEnableColoration(true);
                    if ( !NStr::EqualNocase(iter->second, "true") ) {
                        if ((m_AlnType == IAlnExplorer::fProtein  ||
                            m_AlnType == IAlnExplorer::fMixed)  &&
                            m_DS->IsValidProteinScoringMethod(iter->second)) {
                            m_DS->SetProteinScoringMethod(iter->second);
                        } else if (m_DS->IsValidDNAScoringMethod(iter->second)) {
                            // treat it as DNA
                            m_DS->SetDNAScoringMethod(iter->second);
                        }
                    }
                }
            } else if (NStr::EqualNocase(iter->first, "DNAScoringMethod")) {
                m_DS->SetDNAScoringMethod(iter->second);
            } else if (NStr::EqualNocase(iter->first, "ProteinScoringMethod")) {
                m_DS->SetProteinScoringMethod(iter->second);
            } else if (NStr::EqualNocase(iter->first, "AlignedSeqFeats"))
                m_ShowAlignedSeqFeats = NStr::StringToBool(iter->second);
            else if (NStr::EqualNocase(iter->first, "IdenticalBases"))
                m_ShowIdenticalBases = NStr::StringToBool(iter->second);
            else if (NStr::EqualNocase(iter->first, "Label"))
                m_ShowLabel = NStr::StringToBool(iter->second);
            else if (NStr::EqualNocase(iter->first, "ShowUnalignedTailsForTrans2GenomicAln"))
                m_ShowUnalignedTailsForTrans2GenomicAln = NStr::StringToBool(iter->second);
            else if (NStr::EqualNocase(iter->first, "LinkMatePairAligns"))
                m_DS->SetLinkMatePairs(NStr::StringToBool(iter->second));
            else if (NStr::EqualNocase(iter->first, "Layout")) {
                m_Layout = LayoutStrToValue(iter->second);
                m_IsDefaultLayout = false;
            }  else if (NStr::EqualNocase(iter->first, "StatDisplay"))
                m_StatConf->m_Display = NStr::StringToInt(iter->second);
            else if (NStr::EqualNocase(iter->first, "StatZoomLevel")) {
                m_StatConf->m_StatZoomLevel = NStr::StringToInt(iter->second);
            } else if (NStr::EqualNocase(iter->first, "ShowAlnStat")) {
                m_ShowAlnStat = NStr::StringToBool(iter->second);
            } else if (NStr::EqualNocase(iter->first, "SupportCompactDisplay")) {
                m_CompactEnabled = NStr::StringToBool(iter->second);
            } else if (NStr::EqualNocase(iter->first, "SortBy")) {
                m_DS->SetSortBy(iter->second);
            } else if (NStr::EqualNocase(iter->first, "GraphHeight")) {
                int height = NStr::StringToInt(iter->second, NStr::fConvErr_NoThrow);
                if (height >= 10 && graph_params) {
                    graph_params->m_Height = height;
                }
            } else if (NStr::EqualNocase(iter->first, "GraphScale")) {
                graph_params->m_Scale = CHistParams::ScaleStrToValue(iter->second);
            } else if (NStr::EqualNocase(iter->first, "GraphColor")) {
                CRgbaColor c(iter->second);
                m_StatConf->m_Colors[CAlnStatConfig::eStat_Match] = c;
                if (graph_params) 
                    graph_params->m_fgColor = c;
            } else if (NStr::EqualNocase(iter->first, "ProjectedFeats")) {
                list<string> toks;
                NStr::Split(iter->second, ", |", toks, NStr::fSplit_Tokenize);
                if ( !toks.empty() ) {
                    m_ProjectedFeats.clear();
                }
                try {
                    ITERATE (list<string>, f_iter, toks) {
                        m_ProjectedFeats.push_back(NStr::StringToInt(*f_iter));
                    }
                } catch (CException& e) {
                    LOG_POST(Warning << "CAlignmentTrack::x_LoadSettings() error on "
                        <<"parsing projected feature list: " << e.GetMsg());
                }
            } else if (NStr::EqualNocase(iter->first, "HideSraAlignments")) {
                m_HideSraAlignments = HideFlagStrToValue(iter->second);
            } else if (NStr::EqualNocase(iter->first, "UnalignedTailsMode")) {
                m_UnalignedTailsMode = UnalignedTailsModeStrToValue(iter->second);

            } else if (NStr::EqualNocase(iter->first, "MinPileUpCost")) {
                auto v = NStr::StringToNumeric<double>(iter->second, NStr::fConvErr_NoThrow);
                if (v > 0)
                    m_MinPileUpCost = v;
            } else if (NStr::EqualNocase(iter->first, "MinAlignCost")) {
                auto v = NStr::StringToNumeric<double>(iter->second, NStr::fConvErr_NoThrow);
                if (v > 0)
                    m_MinAlignCost = v;
            } else if (NStr::EqualNocase(iter->first, "MaxAlignCost")) {
                auto v = NStr::StringToNumeric<double>(iter->second, NStr::fConvErr_NoThrow);
                if (v > 0)
                    m_MaxAlignCost = v;
            } else if (NStr::EqualNocase(iter->first, "MaxAlignShownFull")) {
                auto v = NStr::StringToNumeric<int>(iter->second, NStr::fConvErr_NoThrow);
                if (v > 0)
                    m_MaxAlignShownFull = v;
            } else if (NStr::EqualNocase(iter->first, "MaxAlignShownAdaptive")) {
                auto v = NStr::StringToNumeric<int>(iter->second, NStr::fConvErr_NoThrow);
                if (v > 0)
                    m_MaxAlignShownAdaptive = v;
            }
        } catch (CException&) {
            LOG_POST(Warning << "CAlignmentTrack::x_LoadSettings() invalid settings: "
                     << iter->first << ":" << iter->second);
        }
    }

    x_LoadPWAlignSettings();
    if (m_MultiAlignConf) {
        x_LoadMultiAlignSettings();
    }
    if (m_AlignSmearConf) {
        x_LoadAlignSmearSettings();
    }
    if (m_MPPWAlignConf) {
        x_LoadMPPWAlignSettings();
    }
    if (m_MatePairConf) {
        x_LoadMatePairSettings();
    }
    if (CAlignmentConfig::eHide_None != m_HideSraAlignments) {
        m_DS->SetHideSra(m_HideSraAlignments);
    }
    if (CAlignmentConfig::eTails_Hide != m_UnalignedTailsMode) {
        m_DS->SetUnalignedTailsMode(m_UnalignedTailsMode);
    }

    m_StatConf->SetShowLabel(m_gConfig->GetShowComments());

    // enable pileup only if match/mistamtch graph is requested
    // enable permanent(inetcahe) cache if in cgi mode only
    bool has_pileup_cache = m_DS->HasCoverageGraph() && !m_StatConf->ShowAGTC();
    m_DS->EnablePileUpCache(has_pileup_cache, m_gConfig->GetCgiMode());

    // Load CDS settings using GeneModel track key
    // not going to save it because
    // 1. the settings belong to GeneModel track and this tracks just borrows them
    // 2. no changes expected from alignment track
    if (!m_GeneModelConfig) {
        m_GeneModelConfig.Reset(new CGeneModelConfig);
    }

    m_GeneModelConfig->LoadSettings(x_GetGlobalConfig(), kGeneModelBaseKey, GetProfile());
/*
    view = CSGConfigUtils::GetColorReadView(
        registry, kCDSKey, kDefProfile, m_gConfig->GetColorTheme(), kDefProfile);
    CSGConfigUtils::GetColor(view, "BGProtProduct", m_CdsConfig->m_bgProtProd);
    CSGConfigUtils::GetColor(view, "FGProtProduct", m_CdsConfig->m_fgProtProd);
    CSGConfigUtils::GetColor(view, "LabelProtProduct", m_CdsConfig->m_LabelProtProd);
    CSGConfigUtils::GetColor(view, "SeqProtOriginal", m_CdsConfig->m_SeqProt);
    CSGConfigUtils::GetColor(view, "SeqProtMismatch", m_CdsConfig->m_SeqProtMismatch);
    CSGConfigUtils::GetColor(view, "SeqProtTranslated", m_CdsConfig->m_SeqProtTrans);

    // load size settings
    view = CSGConfigUtils::GetSizeReadView(
        registry, kCDSKey, kDefProfile, m_gConfig->GetSizeLevel(), kDefProfile);

    CSGConfigUtils::GetFont(view, "ProdFontFace", "ProdFontSize", m_CdsConfig->m_ProdFont);
    CSGConfigUtils::GetFont(view, "TransFontFace", "TransFontSize", m_CdsConfig->m_TransFont);
*/
    m_DS->SetCgiMode(m_gConfig->GetCgiMode());

    m_Layered->SetVertSpace(2);
    m_Column->SetVertSpace(2);
    x_OnLayoutPolicyChanged();
}


void CAlignmentTrack::x_SaveSettings(const string& preset_style)
{
    TKeyValuePairs settings;

    if ( !preset_style.empty() ) {
        settings["profile"] = preset_style;
    }

    settings["Color"] = NStr::BoolToString(m_DS->GetEnableColoration());
    settings["DNAScoringMethod"] = m_DS->GetDNAScoringMethod();
    settings["ProteinScoringMethod"] = m_DS->GetProteinScoringMethod();
    settings["AlignedSeqFeats"] = NStr::BoolToString(m_ShowAlignedSeqFeats);
    settings["IdenticalBases"] = NStr::BoolToString(m_ShowIdenticalBases);
    settings["Label"] = NStr::BoolToString(m_ShowLabel);
    settings["LinkMatePairAligns"] = NStr::BoolToString(m_DS->GetLinkMatePairs());
    settings["ShowUnalignedTailsForTrans2GenomicAln"] =
        NStr::BoolToString(m_ShowUnalignedTailsForTrans2GenomicAln);
    settings["Layout"] = LayoutValueToStr(m_Layout);

    string projected_feats = kEmptyStr;
    ITERATE (TSubtypeVec, iter, m_ProjectedFeats) {
        projected_feats += NStr::IntToString(*iter) + " ";
    }
    if ( !projected_feats.empty() ) {
        settings["ProjectedFeats"] = projected_feats;
    }
    settings["StatDisplay"] = NStr::IntToString(m_StatConf->m_Display);
    settings["ShowAlnStat"] = NStr::BoolToString(m_ShowAlnStat);
    settings["SupportCompactDisplay"] = NStr::BoolToString(m_CompactEnabled);
    settings["StatZoomLevel"] = NStr::IntToString(m_StatConf->m_StatZoomLevel);
    settings["HideSraAlignments"] = HideFlagValueToStr(m_HideSraAlignments);
    settings["UnalignedTailsMode"] = UnalignedTailsModeValueToStr(m_UnalignedTailsMode);
    settings["SortBy"] = m_DS->GetSortBy();
    SetProfile(CSGConfigUtils::ComposeProfileString(settings));
}


void CAlignmentTrack::x_OnIconClicked(TIconID id)
{
    switch (id) {
    case eIcon_Content:
        x_OnContentIconClicked();
        break;
    case eIcon_Layout:
        x_OnLayoutIconClicked();
        break;
    case eIcon_Score:
        x_OnScoreIconClicked();
        break;
    case eIcon_Stat:
        x_OnStatIconClicked();
        break;
    case eIcon_Tails:
        x_OnTailsIconClicked();
        break;
    default:
        // use default handlers
        CLayoutTrack::x_OnIconClicked(id);
    }
}


void CAlignmentTrack::x_UpdateData()
{
    CDataTrack::x_UpdateData();
    m_DS->DeleteAllJobs();
    x_SetStartStatus();

    int align_limit = x_LayoutToCutoff(m_Layout);
    const TSeqRange& vis_range = m_Context->GetVisSeqRange();
    const TModelUnit& zoom_level = m_Context->GetScale();
/*
    // CSRA Estimate is not accurate yet
    if (m_DS->IsCSRALoader() && m_DS->HasCoverageGraph()) {
        if (m_Context->GetScale() > m_StatConf->m_StatZoomLevel) {
            m_DS->LoadCoverageGraph(vis_range, zoom_level, eJob_CoverageGraph);
        } else {
            if (m_Layout != eLayout_Packed) {
                align_limit = x_LayoutToCutoff_SRA(m_Layout);
                m_DS->LoadAlignments(vis_range, zoom_level, align_limit,
                                     false, eJob_Align);
            } else {
                if (!m_StatGlyph)
                    SetGroup().Clear();
                CSGAlignmentDS::TAlnMgrVec stat_aligns;
                m_DS->CalcAlnStat(stat_aligns, m_Context->GetVisSeqRange(), m_Context->GetScale(), eJob_AlignStat);
            }

        }
        return;
    }
*/
    if (m_DS->NeedReleaseMemory()) {
        CRef<CSeqGlyph> p_glyph = SetGroup().GetChild(m_StatGlyph.size());
        auto p_group = dynamic_cast<CLayoutGroup*>(p_glyph.GetPointer());
        if (p_group) {
            SetGroup().Remove(p_group);
        } else {
            m_StatGlyph.clear();
            SetGroup().Clear();
        }
        m_DS->ReleaseMemory();
    }

    auto align_cost = -1;
    bool sequence_level = m_Context->GetScale() <= 1./8.;
    bool show_all = m_Layout == eLayout_Full || m_Layout == eLayout_ExpandedByPos;

    if (show_all) {
        if (!sequence_level)
            align_cost = m_DS->GetAlignmentCost(vis_range);
        if (align_cost <= m_MaxAlignCost) {
            m_DS->LoadAlignments(vis_range, zoom_level, m_MaxAlignShownFull, false, eJob_Align);
            return;
        }
    }

    bool show_pileup_cache = vis_range.GetLength() < kShowPileUpRange || m_DS->IsRangeCached(vis_range);
    if (m_DS->HasCoverageGraph()) {
        // bam/csra alignments
        bool show_coverage = !sequence_level;
        if (show_coverage) {
            if (show_pileup_cache) {
                show_coverage = false;
            }
            else if (!m_StatConf->ShowAGTC()) {
                if (m_Context->GetScale() < kMinCoverageZoomLevel * 3)
                    show_coverage = m_DS->GetGraphCost(vis_range) > m_MinPileUpCost;
                //show_coverage = m_DS->GetGraphCost(vis_range) > m_MinPileUpCost;
            } else {
                if (m_Context->GetScale() < kMinCoverageZoomLevel)
                    show_coverage = m_DS->GetGraphCost(vis_range) > m_MinPileUpCost;
            }
        }
        if (show_coverage) {
            m_DS->LoadCoverageGraph(vis_range, zoom_level, eJob_CoverageGraph);
        } else {
            bool show_aligns = false;
            if (m_Layout != eLayout_Packed) {
                if (align_cost < 0)
                    align_cost = m_DS->GetAlignmentCost(vis_range);
                show_aligns = (align_cost <= m_MinAlignCost && m_DS->CanShowRange(vis_range, align_limit * 1.5))
                    || (align_cost <= m_MaxAlignCost && sequence_level);
                
            }
            if (show_aligns) {
                m_DS->LoadAlignments(vis_range, zoom_level, align_limit, false, eJob_Align);
                // After alignment loading job is completed, align pileup graph can be loaded.
            } else {
                CRef<CSeqGlyph> p_glyph = SetGroup().GetChild(m_StatGlyph.size());
                auto p_group = dynamic_cast<CLayoutGroup*>(p_glyph.GetPointer());
                if (p_group) {
                    SetGroup().Remove(p_group);
                } else {
                    m_StatGlyph.clear();
                    SetGroup().Clear();
                }
                CSGAlignmentDS::TAlnMgrVec stat_aligns;
                m_DS->CalcAlnStat(stat_aligns, m_Context->GetVisSeqRange(), m_Context->GetScale(), eJob_AlignStat);
            }
        }
    } else {
        // regular alignment annotation
        bool show_stat = m_ShowAlnStat && m_AlnType == IAlnExplorer::fDNA;
        if (show_stat) {
            if (align_cost < 0)
                align_cost = m_DS->GetAlignmentCost(vis_range);
            show_stat = show_stat && align_cost > m_MinAlignCost;
        }
        // Alignment statistics job, if enabled, will be
        // launched right after alignments get loaded.
        static const int kMaxRegulatAlignmentRows = 1000;
        align_limit = kMaxRegulatAlignmentRows;
        bool smear = m_Layout == eLayout_Adaptive;
        if (m_Layout == eLayout_Packed) {
            align_limit = -1;
            smear = true;
        }
        m_DS->LoadAlignments(vis_range, zoom_level, align_limit, smear, eJob_Align);
    }
}


void CAlignmentTrack::x_OnJobCompleted(CAppJobNotification& notify)
{
    m_DS->ClearJobID(notify.GetJobID());
    CRef<CObject> res_obj = notify.GetResult();
    CJobResultBase* result = dynamic_cast<CJobResultBase*>(&*res_obj);
    if (!result) {
        LOG_POST(Error << "CAignmentTrack::x_OnJobCompleted() notification for job \
                does not contain results.");
        return;
    }
    CSGAlignJobResult* aln_result = dynamic_cast<CSGAlignJobResult*>(result);
    if (aln_result && aln_result->m_DataHandle) 
        m_DS->SetDataHandle(aln_result->m_DataHandle);

    switch (result->m_Token) {
        case eJob_Align:
            x_AddAlignmentLayout(dynamic_cast<CSGJobResult&>(*res_obj));
            break;
        case eJob_AlignFeats:
            x_AddAlignFeatLayout(dynamic_cast<CBatchJobResult&>(*res_obj));
            break;
        case eJob_AlignScore:
            x_SetFinishStatus();
            x_SetMsg();

            // make sure we don't update layout twice for synchronized jobs
            if (m_DS->IsBackgroundJob()) {
                // No need to update layout, just redraw
                // x_UpdateLayout();
                m_LTHost->LTH_OnLayoutChanged();
            }
            m_DS->ResetAlnScoringJob();
            break;
        case eJob_CoverageGraph:
            x_AddGraphLayout(dynamic_cast<CSGJobResult&>(*res_obj));
            break;
        case eJob_AlignStat:
            x_AddAlignStatLayout(dynamic_cast<CSGJobResult&>(*res_obj));
            break;
        default:
            break;
    }
}


void CAlignmentTrack::x_OnContentIconClicked()
{
    wxMenu menu;
    int curr_id = 0;
    wxMenuItem* item;
    item = menu.AppendCheckItem(kContentBaseID + curr_id++, wxT("Show labels"));
    if (m_ShowLabel) {
        item->Check();
    }
    item = menu.AppendCheckItem(kContentBaseID + curr_id++,
        wxT("Show alignment statistics"));
    if (m_ShowAlnStat) {
        item->Check();
    }
    item = menu.AppendCheckItem(kContentBaseID + curr_id++,
        wxT("Project features for aligned sequences"));
    if (m_ShowAlignedSeqFeats) {
        item->Check();
    }
    item = menu.AppendCheckItem(kContentBaseID + curr_id++, wxT("Link mate pairs"));
    if (m_DS->GetLinkMatePairs()) {
        item->Check();
    }
    item = menu.AppendCheckItem(kContentBaseID + curr_id++, wxT("Sort alignments by strand"));
    if ( m_DS->GetSortBy() == CAlignStrandSorter::GetID() ) {
        item->Check();
    }
    item = menu.AppendCheckItem(kContentBaseID + curr_id++, wxT("Sort alignments by haplotype"));
    if ( m_DS->GetSortBy() == CAlignSorterFactory::GetHaplotypeID()) {
        item->Check();
    }
    item = menu.AppendCheckItem(kContentBaseID + curr_id++,
        wxT("Show Identical Bases"));
    if (m_ShowIdenticalBases) {
        item->Check();
    }

    menu.AppendSeparator();
    item = menu.Append(kProjectedFeatID, wxT("Change projected feature list..."));

    m_LTHost->LTH_PushEventHandler(m_ContentHandler);
    m_LTHost->LTH_PopupMenu(&menu);
    m_LTHost->LTH_PopEventHandler();
}


void CAlignmentTrack::x_OnLayoutIconClicked()
{
    wxMenu menu;
    UseDefaultMarginWidth(menu);
    int id_base = 10000;

    for (const auto iter : sm_LayoutDispMap) {
        bool l_default = iter.first == eLayout_Default;
        wxMenuItem* item = menu.AppendRadioItem(id_base + iter.first,
            ToWxString(iter.second + (l_default ? " (Default)" : "")));
        if (m_Layout == iter.first) {
            item->Check();
        }
    }

    int compact_item_idx = id_base + max(100, (int)sm_LayoutDispMap.size());
    {
        menu.AppendSeparator();
        wxMenuItem* item = menu.AppendCheckItem(compact_item_idx,
            wxT("Enable compact display"));
        item->Check(m_CompactEnabled);
    }

    m_LTHost->LTH_PopupMenu(&menu);
    wxMenuItemList& item_list = menu.GetMenuItems();
    ITERATE (wxMenuItemList, iter, item_list) {
        if ((*iter)->GetId() == compact_item_idx) {
            if (m_CompactEnabled != (*iter)->IsChecked()) {
                m_CompactEnabled = (*iter)->IsChecked();
                x_UpdateData();
                break;
            }
        } else {
            ELayout id = (ELayout)((*iter)->GetId() - id_base);
            if ((*iter)->IsChecked()  &&  id != m_Layout) {
                m_Layout = id;
                m_IsDefaultLayout = false;
                x_OnLayoutPolicyChanged();
                x_UpdateData();
                break;
            }
        }
    }
}


void CAlignmentTrack::x_OnScoreIconClicked()
{
    if (m_AlnType ==  IAlnExplorer::fInvalid) {
        return;
    }

    wxMenu menu;
    UseDefaultMarginWidth(menu);
    int id_base = 10000;
    int score_id = 0;
    int curr_id = 0;


    const string curr_method = m_AlnType == IAlnExplorer::fDNA ?
        m_DS->GetDNAScoringMethod() : m_DS->GetProteinScoringMethod();

    const CSGAlignmentDS::TMethods& methods = m_DS->GetScoringMethods(m_AlnType);
    ITERATE (CSGAlignmentDS::TMethods, iter, methods) {
        wxMenuItem* item = menu.AppendRadioItem(id_base + curr_id,
            ToWxString((*iter)->GetName()),
            ToWxString((*iter)->GetDescription()));

        if (curr_method == (*iter)->GetName()) {
            item->Check();
            score_id = curr_id;
        }
        curr_id++;
    }
    if (m_DS->HasQualityMap()) {
        const string& score_name = CSGAlnQualityScore::GetScoreName();
        wxMenuItem* item = menu.AppendRadioItem(id_base + curr_id,
            ToWxString(score_name), ToWxString(score_name));

        if (curr_method == score_name) {
            item->Check();
            score_id = curr_id;
        }
        curr_id++;
    }

    wxMenuItem* item = menu.AppendRadioItem(id_base + curr_id, wxT("Disabled"));
    if ( !m_DS->GetEnableColoration() ) {
       item->Check();
       score_id = curr_id;
    }

    m_LTHost->LTH_PopupMenu(&menu);
    wxMenuItemList& item_list = menu.GetMenuItems();
    ITERATE (wxMenuItemList, iter, item_list) {
        int checked_id = (*iter)->GetId() - id_base;
        if ((*iter)->IsChecked()  &&  score_id != checked_id) {
            if (checked_id == curr_id) {
                m_DS->SetEnableColoration(false);
            } else {
                m_DS->SetEnableColoration(true);
                if (m_AlnType == IAlnExplorer::fDNA) {
                    m_DS->SetDNAScoringMethod(ToStdString((*iter)->GetItemLabel()));
                } else if (m_AlnType == IAlnExplorer::fProtein  ||
                           m_AlnType == IAlnExplorer::fMixed) {
                    m_DS->SetProteinScoringMethod(ToStdString((*iter)->GetItemLabel()));
                }
            }
            x_UpdateData();
            break;
        }
    }
}


void CAlignmentTrack::x_OnStatIconClicked()
{
    if ( !m_StatConf)
        return;
    auto graph_params = x_GetGraphParams();
    if (!graph_params)
        return;

    CwxAlnStatOptionsDlg dlg(NULL);
    dlg.SetContent(m_StatConf->ShowAGTC());
    dlg.SetDisplay(m_StatConf->IsBarGraph());
    dlg.SetValueType(m_StatConf->ShowCount());
    dlg.SetGraphHeight(graph_params->m_Height);
    dlg.SetZoomLevel(m_StatConf->m_StatZoomLevel);
    IGlyphDialogHost* dlg_host = dynamic_cast<IGlyphDialogHost*>(m_LTHost);
    if (dlg_host) {
        dlg_host->PreDialogShow();
    }
    if(dlg.ShowModal() == wxID_OK) {
        m_StatConf->SetDisplayFlag(CAlnStatConfig::fBarGraph, dlg.IsBarGraph());
        m_StatConf->SetDisplayFlag(CAlnStatConfig::fShowMismatch, !dlg.IsAGTC());
        m_StatConf->SetDisplayFlag(CAlnStatConfig::fShowCount, dlg.IsCount());
        m_StatConf->m_StatZoomLevel = dlg.GetZoomLevel();
        graph_params->m_Height = max(dlg.GetGraphHeight(), 10);
        m_ShowAlnStat = true;
        x_UpdateData();
    }
    if (dlg_host) {
        dlg_host->PostDialogShow();
    }
}

void CAlignmentTrack::x_OnTailsIconClicked()
{
    wxMenu menu;
    UseDefaultMarginWidth(menu);

    int id_base = 10000;
    int curr_id = 0;

    wxMenuItem* items[3];
    items[0] = menu.AppendRadioItem(id_base + curr_id++, "Hide tails", "Hide the unaligned tails");
    items[1] = menu.AppendRadioItem(id_base + curr_id++, "Show tail length", "Display the length of the unaligned tails");
    items[2] = menu.AppendRadioItem(id_base + curr_id++, "Show tail sequence", "Display the unaligned tails row sequence");
    items[m_UnalignedTailsMode]->Check();
    menu.AppendSeparator();
    wxMenuItem* itemTrans2Genomic = menu.AppendCheckItem(kContentBaseID + curr_id++,
        wxT("Show unaligned tails for transcript-to-genomic alignments"));
    if (m_ShowUnalignedTailsForTrans2GenomicAln) {
        itemTrans2Genomic->Check();
    }
    m_LTHost->LTH_PopupMenu(&menu);

    CAlignmentConfig::EUnalignedTailsMode newTailsMode(CAlignmentConfig::eTails_Hide);
    if (items[1]->IsChecked())
        newTailsMode = CAlignmentConfig::eTails_ShowGlyph;
    else if (items[2]->IsChecked())
        newTailsMode = CAlignmentConfig::eTails_ShowSequence;

    bool update = false;
    if (newTailsMode != m_UnalignedTailsMode) {
        m_UnalignedTailsMode = newTailsMode;
        update = true;
    }

    if (m_ShowUnalignedTailsForTrans2GenomicAln != itemTrans2Genomic->IsChecked()) {
        m_ShowUnalignedTailsForTrans2GenomicAln = itemTrans2Genomic->IsChecked();
        update = true;
    }

    if (update) {
        if (m_PWAlignConf) {
            m_PWAlignConf->m_UnalignedTailsMode = m_UnalignedTailsMode;
            m_PWAlignConf->m_ShowUnalignedTailsForTrans2GenomicAln = m_ShowUnalignedTailsForTrans2GenomicAln;
        }
        if (m_AlignSmearConf) {
            m_AlignSmearConf->m_UnalignedTailsMode = m_UnalignedTailsMode;
            m_AlignSmearConf->m_ShowUnalignedTailsForTrans2GenomicAln = m_ShowUnalignedTailsForTrans2GenomicAln;
        }

        m_DS->SetUnalignedTailsMode(m_UnalignedTailsMode);

        x_UpdateData();
    }
}

void CAlignmentTrack::x_OnLayoutPolicyChanged()
{
    CLayoutGroup* p_group = NULL;
    if (!m_StatGlyph.empty()) {
//    if (m_StatGlyph) {
        SetLayoutPolicy(m_GeneModelLayout);
//        CRef<CSeqGlyph> p_glyph = SetGroup().GetChild(1);
        CRef<CSeqGlyph> p_glyph = SetGroup().GetChild(m_StatGlyph.size());
        if (p_glyph) {
            p_group = dynamic_cast<CLayoutGroup*>(p_glyph.GetPointer());
            if (m_Layout == eLayout_Packed) {
                SetGroup().Remove(p_group);
                p_group = NULL;
            }
        }
    } else {
        p_group = &SetGroup();
    }
    if (!p_group)
        return;

    const CNamedGroup* first_group = NULL;
    if ( !p_group->GetChildren().empty() ) {
        first_group = dynamic_cast<const CNamedGroup*>(p_group->GetChildren().front().GetPointer());
    }

    p_group->SetTearline(0);
    vector<CLayoutGroup*> groups;
    if (first_group) {
        NON_CONST_ITERATE (CSeqGlyph::TObjects, grp_iter, p_group->SetChildren()) {
            CNamedGroup* curr_group = dynamic_cast<CNamedGroup*>(grp_iter->GetPointer());
            _ASSERT(curr_group);
            groups.push_back(curr_group);
        }
        p_group->SetLayoutPolicy(m_GeneModelLayout);
    } else {
        groups.push_back(p_group);
    }

    NON_CONST_ITERATE (vector<CLayoutGroup*>, g_iter, groups) {
        CLayoutGroup* curr_group = *g_iter;
        if (m_Layout == eLayout_ExpandedByPos) {
            curr_group->SetLayoutPolicy(m_Simple);
            curr_group->SetTearline(eMaxRowLimit);
            m_Simple->SetSortingType(CSimpleLayout::eSort_BySeqPos);
            m_DS->SetLayoutPolicy(m_Simple);
        } else {
            m_DS->SetLayoutPolicy(m_Column);
            curr_group->SetLayoutPolicy(m_Column);
            curr_group->SetTearline(0);

            // Lets us limit the number of alignments displayed.
            // Setting this to 0 disables the restriction
            if (m_Layout == CAlignmentTrack::eLayout_Adaptive) 
                curr_group->SetTearline(eAdaptiveRowLimit);
            else if (m_Layout == CAlignmentTrack::eLayout_Full)
                curr_group->SetTearline(eMaxRowLimit);
        }
    }
}


void CAlignmentTrack::x_AddAlignmentLayout(const CSGJobResult& result)
{
    //TIME_ME("x_AddAlignmentLayout()");
    SetMsg("");
    m_ObjNum = result.m_ObjectList.size();
    const CNamedGroup* first_group = NULL;
    if (m_ObjNum > 0) {
        first_group = dynamic_cast<const CNamedGroup*>(result.m_ObjectList.front().GetPointer());
    } else {
        // an empty track should navigable in case there are out of range object
        //if (m_ObjNum == 0) {
        m_Attrs |= fNavigable;
    }


    if (m_AlnType == IAlnExplorer::fInvalid  &&  m_ObjNum > 0) {
        if (first_group) {
            m_AlnType = m_DS->GetAlignType(first_group->GetChildren().front());
        } else {
            m_AlnType = m_DS->GetAlignType(result.m_ObjectList.front());
        }
    }

    bool show_stat = (m_ShowAlnStat  ||
        (m_DS->HasCoverageGraph()  &&  m_ObjNum == 0))  &&
        //        m_Context->GetScale() <= m_StatConf->m_StatZoomLevel  &&
        // For cSRA data (with coverage graph), no need to check align type.
        (m_AlnType == IAlnExplorer::fDNA || m_DS->HasCoverageGraph());

    /// all alignment we want to calculate statistics
    CSGAlignmentDS::TAlnMgrVec stat_aligns;

    // Trying to find out where to add the newly loaded objects.
    // Depending on what is currently being shown, we might have different
    // object hierarchies:
    // 1. both pileup and alignments
    //     group--
    //           |-- pileup glyph
    //           |-- group--
    //                     |-- alignments
    //
    // 2. only smear bar or coverage graph
    //     group--
    //           |-- smear bar/coverage graph
    //
    // 3. only pileup
    //     group--
    //           |-- pileup glyph
    //
    // 4. only alignments
    //     group--
    //           |-- alignments
    //

    if (show_stat)
        // turn off navigation controls
        m_Attrs &= ~fNavigable;

    CLayoutGroup* p_group = NULL;
    if ( !show_stat ) {
        // If we are not going to show pileup (regardless of it was shown
        // or not), use the top level group as the parent group
        p_group = &SetGroup();
        //m_StatGlyph.Reset();
        m_StatGlyph.clear();
    } else if (!m_StatGlyph.empty()) {
    //} else if (m_StatGlyph) {
        // we are showing the pipleup, and will keep showing it.
        if (SetGroup().GetChildrenNum() > m_StatGlyph.size()) {
            // case #1
        //    _ASSERT(SetGroup().GetChildrenNum() == 2);
          //  CRef<CSeqGlyph> p_glyph = SetGroup().GetChild(1);
            CRef<CSeqGlyph> p_glyph = SetGroup().GetChild(m_StatGlyph.size());
            p_group = dynamic_cast<CLayoutGroup*>(p_glyph.GetPointer());
            _ASSERT(p_group);
            if (m_ObjNum == 0) {
                SetGroup().Remove(p_group);
                p_group = NULL;
            }
        } else if (m_ObjNum > 0) {
            // case #3
            CRef<CLayoutGroup> group(new CLayoutGroup);
            p_group = group.GetPointer();
            SetGroup().PushBack(p_group);
        }
    } else {
        // We are not showing the pipleup, and will show it.
        // This falls to case #2 or #4
        p_group = &SetGroup();
    }

    if (p_group) {
        p_group->Clear();
    } else {
        x_UpdateLayout();
        if (show_stat) {
            m_DS->CalcAlnStat(stat_aligns, m_Context->GetVisSeqRange(),
                              m_Context->GetScale(), eJob_AlignStat);
        }
        return;
    }

    p_group->Set(result.m_ObjectList);

    /// all alignments we want to project features from
    vector< CRef<CAlignGlyph> > aligns;

    bool project_feats = m_ShowAlignedSeqFeats  &&
        !m_Context->IsOverviewMode()  &&
        !m_ProjectedFeats.empty();


    vector<CLayoutGroup*> groups;
    if (first_group) {
        NON_CONST_ITERATE (CSeqGlyph::TObjects, grp_iter, p_group->SetChildren()) {
            CNamedGroup* curr_group = dynamic_cast<CNamedGroup*>(grp_iter->GetPointer());
            _ASSERT(curr_group);

            // initialize named group
            curr_group->SetTitleColor(m_gConfig->GetFGCommentColor());
            curr_group->SetBackgroundColor(m_gConfig->GetBGCommentColor());
            curr_group->SetTitleFont(m_gConfig->GetCommentFont().GetPointer());
            curr_group->SetIndent(GetIndent() + 2);
            curr_group->SetRepeatDist(m_gConfig->GetCommentRepeatDist());
            curr_group->SetRepeatTitle(m_gConfig->GetRepeatComment());
            curr_group->SetShowTitle(m_gConfig->GetShowComments());
            groups.push_back(curr_group);
        }
    } else {
        groups.push_back(p_group);
    }

    bool show_score = !m_Context->IsOverviewMode()  &&
        m_DS->GetEnableColoration() &&
        m_AlnType != IAlnExplorer::fInvalid;

    CSeqGlyph::TObjects scored_alns;
    m_ObjNum = 0;
    NON_CONST_ITERATE (vector<CLayoutGroup*>, g_iter, groups) {
        CSeqGlyph::TObjects& objs = (*g_iter)->SetChildren();
        m_ObjNum += objs.size();
        //if (show_score) {
        //    std::copy(objs.begin(), objs.end(), back_inserter(scored_alns));
        //}

        NON_CONST_ITERATE (CSeqGlyph::TObjects, iter, objs) {
            CSeqGlyph* tmp = *iter;
            tmp->SetRenderingContext(m_Context);
            EAlignGlyphType type = x_GetAlignGlyphType(tmp);

            switch (type)
            {
            case eAlign_Multi:
                {{
                    if ( !m_MultiAlignConf ) {
                        x_LoadMultiAlignSettings();
                    }
                    CAlignGlyph* aln = dynamic_cast<CAlignGlyph*>(tmp);
                    aln->SetConfig(m_MultiAlignConf);
                    if (show_stat) {
                        stat_aligns.push_back(CConstRef<IAlnGraphicDataSource>(&aln->GetAlignMgr()));
                    }
                    if (show_score) {
                        scored_alns.push_back(*iter);
                    }
                    m_Attrs |= fNavigable;

                }}
                break;
            case eAlign_PW:
                {{
                    CAlignGlyph* aln = dynamic_cast<CAlignGlyph*>(tmp);
                    aln->SetConfig(m_PWAlignConf);
                    if (project_feats) {
                        // project the annotated features on the aligned
                        // sequence onto the top sequence
                        x_LoadAlignedSeqFeats(aligns, aln);
                    }
                    if (show_stat) {
                        stat_aligns.push_back(CConstRef<IAlnGraphicDataSource>(&aln->GetAlignMgr()));
                    }
                    if (show_score) {
                        if (aln->IsSimplified()) {
                            aln->SetShowScore();
                        } else {
                            scored_alns.push_back(*iter);
                        }
                    }
                    m_Attrs |= fNavigable;
                }}
                break;
            case eAlign_MatePair:
                {{
                    if ( !m_MatePairConf ) {
                        x_LoadMPPWAlignSettings();
                        x_LoadMatePairSettings();
                    }
                    CMatePairGlyph* mp = dynamic_cast<CMatePairGlyph*>(tmp);
                    CMatePairGlyph::TAlignList& mp_aligns = mp->SetSeqAligns();
                    mp->SetConfig(m_MatePairConf);
                    NON_CONST_ITERATE(CMatePairGlyph::TAlignList, mp_iter, mp_aligns) {
                        (*mp_iter)->SetConfig(mp_aligns.size() == 1 ? m_PWAlignConf : m_MPPWAlignConf);
                        (*mp_iter)->SetRenderingContext(m_Context);
                        if (show_stat) {
                            stat_aligns.push_back(CConstRef<IAlnGraphicDataSource>(&(*mp_iter)->GetAlignMgr()));
                        }
                        if (show_score) {
                            if ((*mp_iter)->IsSimplified()) {
                                (*mp_iter)->SetShowScore();
                            } else {
                                scored_alns.push_back(CRef<CSeqGlyph>(mp_iter->GetPointer()));
                            }
                        }
                    }
                    m_Attrs |= fNavigable;
                }}
                break;
            case eAlign_Smear:
                {{
                    if ( !m_AlignSmearConf ) {
                        x_LoadAlignSmearSettings();
                    }
                    CAlignSmearGlyph* as = dynamic_cast<CAlignSmearGlyph*>(tmp);
                    as->SetConfig(m_AlignSmearConf);
                }}
                break;
            case eAlign_Graph:
                {{
                    CHistogramGlyph* graph = dynamic_cast<CHistogramGlyph*>(tmp);
                    graph->SetDialogHost(dynamic_cast<IGlyphDialogHost*>(m_LTHost));
                    graph->SetConfig(*m_gConfig);
                    string coverage_annot;
                    CSeqUtils::CreateCoverageAnnotName(m_AnnotName, coverage_annot);
                    
                    graph->SetAnnotName(coverage_annot);
                    SetMsg(", Coverage graph");
                }}
                break;
            default:
                break;
            }
        }
    }

    CAlignmentConfig::ECompactMode compact_mode = CAlignmentConfig::eNormal;
    if (m_CompactEnabled  &&  m_ObjNum > m_CompactThreshold  &&
        !m_Context->WillSeqLetterFit()) {
        compact_mode = CAlignmentConfig::eCompact;
    }

    if (compact_mode == CAlignmentConfig::eCompact  &&
        m_ObjNum > m_ExtremeCompactThreshold) {
        compact_mode = CAlignmentConfig::eExtremeCompact;
    }

    if (m_PWAlignConf) {
        m_PWAlignConf->m_CompactMode = compact_mode;
        m_PWAlignConf->m_ShowLabel =
            m_ShowLabel  &&  compact_mode == CAlignmentConfig::eNormal;
        m_PWAlignConf->m_ShowIdenticalBases = m_ShowIdenticalBases;
    }
    if (m_MPPWAlignConf) {
        m_MPPWAlignConf->m_CompactMode = compact_mode;
        m_MPPWAlignConf->m_ShowLabel =
            m_ShowLabel  &&  compact_mode == CAlignmentConfig::eNormal;
        // Do we want this for multiple alignment?
        m_MPPWAlignConf->m_ShowIdenticalBases = m_ShowIdenticalBases;
    }


    int vert_space = 2;
    float min_dist = 4.0;
    if (compact_mode == CAlignmentConfig::eCompact) {
        vert_space = 1;
        min_dist = 3.0;
    } else if (compact_mode == CAlignmentConfig::eExtremeCompact) {
        vert_space = 0;
        min_dist = 2.0;
    }

    m_Layered->SetVertSpace(vert_space);
    m_Column->SetVertSpace(vert_space);
    m_Column->SetMinDist(max(TSeqPos(1),
           TSeqPos(m_Context->ScreenToSeq(min_dist))));

    //m_GeneModelLayout->SetVertSpace(2);

    if (show_stat) {
        // enable pileup only if match/mistamtch graph is requested
        // enable permanent(inetcahe) cache if in cgi mode only
        bool has_pileup_cache = m_DS->HasCoverageGraph() && !m_StatConf->ShowAGTC();
        m_DS->EnablePileUpCache(has_pileup_cache, m_gConfig->GetCgiMode());
        // calculate alignment statistics
        // if stat_aligns is empty, it may be because alignments are packed as
        // smear bar or graph. So we still need to try
        m_DS->CalcAlnStat(stat_aligns, m_Context->GetVisSeqRange(),
                          m_Context->GetScale(), eJob_AlignStat);
    }

    // projected features from the aligned sequences
    if ( !aligns.empty() ) {
        m_DS->LoadAlignFeats(m_Context->GetVisSeqRange(),
        m_Context->GetScale(), aligns, m_ProjectedFeats, eJob_AlignFeats);
    }

    // caclucate alignment score
    if ( !scored_alns.empty() ) {
        m_DS->CalculateAlignmentScore(scored_alns, eJob_AlignScore);
    }

    x_OnLayoutPolicyChanged();
    x_SetMsg();
    x_UpdateLayout();
}


void CAlignmentTrack::x_AddGraphLayout(const CSGJobResult& result)
{
    string coverage_annot;
    CSeqUtils::CreateCoverageAnnotName(m_AnnotName, coverage_annot);
    if (!m_StatGlyph.empty()) 
        m_StatGlyph.clear();

    if (result.m_ObjectList.empty()) {
        SetGroup().Clear();
        SetMsg(", no data");
    } else {
        SetGroup().Set(result.m_ObjectList);
        for (auto& gr : SetGroup().SetChildren()) {
            CHistogramGlyph* graph = dynamic_cast<CHistogramGlyph*>(gr.GetPointer());
            if (graph) {
                graph->SetDialogHost(dynamic_cast<IGlyphDialogHost*>(m_LTHost));
                graph->SetConfig(*m_gConfig);
                graph->SetAnnotName(coverage_annot);
                SetMsg(", Coverage graph");
            }
        }
    }

    x_UpdateLayout();
}

CRef<CHistParams> CAlignmentTrack::x_GetGraphParams()
{
    _ASSERT(m_StatConf);
    CRef<CHistParams> params;
    if (!m_AnnotName.empty()) {
        auto conf_mgr = m_gConfig->GetHistParamsManager();
        string coverage_annot;
        CSeqUtils::CreateCoverageAnnotName(m_AnnotName, coverage_annot);
        if (conf_mgr->HasSettings(coverage_annot)) {
            params = conf_mgr->GetHistParams(coverage_annot);
        } else {
            CRef<CHistParams> def_conf = conf_mgr->GetDefHistParams();
            params.Reset(new CHistParams(*def_conf));
            params->m_fgColor = m_StatConf->m_Colors[CAlnStatConfig::eStat_Match];
            conf_mgr->AddSettings(coverage_annot, params);
        }
        params->m_NeedRulerLabels = false; // turn off labels for coverage graph
    }
    return params;
}

void CAlignmentTrack::x_AddAlignStatLayout(const CSGJobResult& result)
{
    x_SetFinishStatus();
    x_SetMsg();
    CSeqGlyph::TObjects objs = result.m_ObjectList;
    vector<CAlnStatGlyph*> stat_glyphs;
    for (auto&& ch : objs) {
        CAlnStatGlyph* stat_glyph =
            dynamic_cast<CAlnStatGlyph*>(ch.GetPointer());
        _ASSERT(stat_glyph);
        if (!stat_glyph)
            continue;
        stat_glyphs.push_back(stat_glyph);
    }

    if (stat_glyphs.empty()) {
        if (!m_StatGlyph.empty()) {
            // do clean up
            if (SetGroup().GetChildrenNum() > m_StatGlyph.size()) {
                //_ASSERT(SetGroup().GetChildrenNum() == 2);
                CRef<CSeqGlyph> p_glyph = SetGroup().GetChild(m_StatGlyph.size());
                CLayoutGroup* p_group = dynamic_cast<CLayoutGroup*>(p_glyph.GetPointer());
                _ASSERT(p_group);
                SetGroup().Set(p_group->GetChildren());
                x_OnLayoutPolicyChanged();
            } else {
                SetGroup().Clear();
            }
            m_StatGlyph.clear();
            x_UpdateLayout();
        }
        return;
    }

    CRef<CLayoutGroup> align_group;
    bool need_update = m_StatGlyph.empty();
    auto graph_params = x_GetGraphParams();
    CSeqGlyph::TObjects aligns;
    if (SetGroup().GetChildrenNum() > m_StatGlyph.size()) {
        if (m_StatGlyph.empty()) {
            align_group.Reset(new CLayoutGroup);
            align_group->Set(GetGroup().GetChildren());
        }  else {
            CRef<CSeqGlyph> p_glyph = SetGroup().GetChild(m_StatGlyph.size());
            align_group.Reset(dynamic_cast<CLayoutGroup*>(p_glyph.GetPointer()));
        }
    }
    m_StatGlyph.clear();
    CLayoutGroup& p_group = SetGroup();
    p_group.Clear();
    for (auto&& stat_glyph : stat_glyphs) {
        stat_glyph->SetConfig(m_StatConf, graph_params);
        stat_glyph->SetCgiMode(m_gConfig->GetCgiMode());
        stat_glyph->SetDialogHost(dynamic_cast<IGlyphDialogHost*>(m_LTHost));
        m_StatGlyph.emplace_back(stat_glyph);
        p_group.PushBack(stat_glyph);
    }
    if (align_group)
        p_group.PushBack(align_group.GetPointer());

    if (need_update)
        x_OnLayoutPolicyChanged();
    x_UpdateLayout();
}


void CAlignmentTrack::x_AddAlignFeatLayout(CBatchJobResult& results)
{
    NON_CONST_ITERATE (CBatchJobResult::TResults, a_iter, results.m_Results) {
        CLayoutGroup::TObjectList& objs = (*a_iter)->m_ObjectList;
        CAlignGlyph* align =
            dynamic_cast<CAlignGlyph*>((*a_iter)->m_Owner.GetPointer());
        if (objs.empty()  ||  !align) continue;

        x_AddAlignFeat_Recursive(objs);
        CLayoutGroup* parent_group =
            dynamic_cast<CLayoutGroup*>(align->SetParent());
        _ASSERT(parent_group);
        CLayoutGroup* group = new CLayoutGroup;
        parent_group->Insert(align, group);
        parent_group->Remove(align);
        group->PushBack(align);
        group->SetLayoutPolicy(m_GeneModelLayout);
        if (objs.size() > 1) {
            // separate objects according to feature types.
            // All gene model features will be grouped together.
            // Anything else will be grouped separately.
            typedef map<int, CLayoutGroup::TObjectList> TFeatGroups;
            TFeatGroups feat_groups;
            NON_CONST_ITERATE (CLayoutGroup::TObjectList, iter, objs) {
                CSeqGlyph* glyph = iter->GetPointer();
                // if the features are in a group (any CLayoutGroup-derived group)
                // we know it must be a gene group
                CLayoutGroup* c_group = dynamic_cast<CLayoutGroup*>(glyph);
                if (c_group) {
                    feat_groups[CSeqFeatData::eSubtype_gene].push_back(*iter);
                } else { // must be a feature
                    const CFeatGlyph* feat = dynamic_cast<const CFeatGlyph*>(glyph);
                    _ASSERT(feat);
                    CSeqFeatData::E_Choice type =
                        feat->GetFeature().GetData().Which();
                    CSeqFeatData::ESubtype s_type =
                        feat->GetFeature().GetData().GetSubtype();
                    if (CDataTrackUtils::IsGeneModelFeature(type, s_type)) {
                        feat_groups[CSeqFeatData::eSubtype_gene].push_back(*iter);
                    } else {
                        feat_groups[s_type].push_back(*iter);
                    }
                }
            }
            NON_CONST_ITERATE (TFeatGroups, iter, feat_groups) {
                if (iter->second.size() > 1) {
                    CLayoutGroup* sub_group = new CLayoutGroup;
                    group->PushBack(sub_group);
                    if ( !m_FeatGroupConf ) {
                        m_FeatGroupConf.Reset(new CBoundaryParams(
                            true, false, CRgbaColor(0.5, 0.5f, 0.5f, 0.5f),
                            CRgbaColor(1.0f, 1.0f, 1.0f, 1.0f), 1.0));
                    }
                    sub_group->SetConfig(m_FeatGroupConf);
                    sub_group->SetLayoutPolicy(m_Column);
                    sub_group->Set(iter->second);
                } else {
                    group->Append(iter->second);
                }
            }
        } else {
            group->Append(objs);
        }
    }
    x_SetMsg();

    // make sure we don't update layout twice for synchronized jobs
    if (m_DS->IsBackgroundJob()) {
        x_UpdateLayout();
    }
}


void CAlignmentTrack::x_AddAlignFeat_Recursive(CLayoutGroup::TObjectList& objs)
{
    NON_CONST_ITERATE (CLayoutGroup::TObjectList, iter, objs) {
        CSeqGlyph* glyph = iter->GetPointer();
        glyph->SetRenderingContext(m_Context);
        if (CGeneGroup* group = dynamic_cast<CGeneGroup*>(glyph)) {
            if ( !m_GeneGroupConf ) {
                m_GeneGroupConf.Reset(new CBoundaryParams(
                    true, false, CRgbaColor(0.0, 0.6f, 0.0f, 0.1f),
                    CRgbaColor(0.6f, 0.8f, 0.3f, 0.3f), 1.0));
            }
            group->SetConfig(m_GeneGroupConf);
            group->SetLayoutPolicy(m_GeneModelLayout);
            group->SetShowGene(true);
            x_AddAlignFeat_Recursive(group->SetChildren());
        } else if (CLayoutGroup* group = dynamic_cast<CLayoutGroup*>(glyph)) {
            // must be an exon group
            group->SetLayoutPolicy(m_Inline);
            x_AddAlignFeat_Recursive(group->SetChildren());
        } else {
            // must be a feature
            CFeatGlyph* feat = dynamic_cast<CFeatGlyph*>(glyph);
            _ASSERT(feat);
            CSeqFeatData::ESubtype subtype =
                feat->GetFeature().GetData().GetSubtype();
            feat->SetConfig(m_gConfig->GetFeatParams(subtype));
            feat->SetProjectedFeat(true);
            switch (subtype) {
            case CSeqFeatData::eSubtype_mRNA:
                feat->SetRulerType(m_GeneModelConfig->m_ShowNtRuler ? CFeatGlyph::eNtRuler : CFeatGlyph::eNoRuler);
                break;
            case CSeqFeatData::eSubtype_cdregion:
                feat->SetRulerType((m_GeneModelConfig->m_ShowNtRuler ? CFeatGlyph::eNtRuler : CFeatGlyph::eNoRuler)
                                   | (m_GeneModelConfig->m_ShowAaRuler ? CFeatGlyph::eAaRuler : CFeatGlyph::eNoRuler));
                if (CCdsGlyph* cds_glyph = dynamic_cast<CCdsGlyph*>(feat)) {
                    cds_glyph->SetCdsConfig(m_GeneModelConfig->m_CdsConfig);
                }
                break;
            default:
                break;
            }
        }
    }
}

void CAlignmentTrack::x_LoadAlignedSeqFeats(vector< CRef<CAlignGlyph> >& aligns,
                                            CAlignGlyph* aln)
{
    // anchored seq-loc
    const CSeq_loc& loc     = aln->GetLocation();
    TSeqRange range = m_Context->GetVisSeqRange();
    range.IntersectWith(loc.GetTotalRange());

    // don't project features when the size (on screen) is less than 4 pixels
    if (m_Context->SeqToScreen((TModelUnit)range.GetLength()) >= 4.0) {
        aligns.push_back(CRef<CAlignGlyph>(aln));
    }

    /*
    CScope& scope = m_Context->GetScope();
    try {
        const IAlnGraphicDataSource& aln_mgr = aln->GetAlignMgr();
        // aligned seq-id
        const CSeq_id& aligned_seq =
        aln_mgr.GetSeqId(aln_mgr.GetAnchor() == 0 ? 1 : 0);

        SConstScopedObject obj(aligned_seq, scope);
        CGeneModelFactory gene_model;

        // map visible range to product feature
        CSeq_loc_Mapper seq_range_mapper(
            aln->GetAlignmentObj(), aligned_seq, &scope);
        CRef<CSeq_loc> tmp_loc(new CSeq_loc());
        tmp_loc->SetInt().SetFrom(range.GetFrom());
        tmp_loc->SetInt().SetTo  (range.GetTo());
        tmp_loc->SetId(*loc.GetId());
        CSeq_loc::TRange vis_range =
            seq_range_mapper.Map(tmp_loc.GetObject())->GetTotalRange();

        ILayoutTrackFactory::SExtraParams params(-1, true);
        params.m_Range = vis_range;
        ILayoutTrackFactory::TTrackMap tracks =
            gene_model.CreateTracks(obj, m_DSContext, m_Context, params);
        if ( !tracks.empty() ) {
            CLayoutGroup* parent_group =
                dynamic_cast<CLayoutGroup*>(aln->SetParent());
            _ASSERT(parent_group);
            CLayoutGroup* group = new CLayoutGroup;
            parent_group->Insert(aln, group);
            parent_group->Remove(aln);
            group->PushBack(aln);
            group->SetLayoutPolicy(m_Simple);

            NON_CONST_ITERATE (ILayoutTrackFactory::TTrackMap, iter, tracks) {
                CDataTrack* track =
                    dynamic_cast<CDataTrack*>(iter->second.GetPointer());
                if (track) {
                    group->PushBack(track);
                    CRef<CCoordMapper_SeqLocMapper> mapper(
                        new CCoordMapper_SeqLocMapper(
                        aln->GetAlignmentObj(), *loc.GetId(), &scope));
                    track->GetDataSource()->SetCoordMapper(mapper);
                    track->SetHost(m_LTHost);
                    track->SetConfig(m_gConfig);
                    track->SetIndent(0);
                    track->SetTrackAttr(0);
                    track->LoadProfile("");

                    track->SetVisRange(vis_range);

                    if (CFeatureTrack* f_track =
                        dynamic_cast<CFeatureTrack*>(track)) {
                        f_track->SetLayout(CFeatureTrack::eLayout_AdaptiveInline);
                    }
                    track->Update(false);
                }
            }
        }

    } catch (CAnnotMapperException&) {
        /// ignore errors from location mapping
    }
    */
}


CAlignmentTrack::EAlignGlyphType
CAlignmentTrack::x_GetAlignGlyphType(const CSeqGlyph* glyph) const
{
    const CAlignGlyph* align = dynamic_cast<const CAlignGlyph*>(glyph);
    if (align) {
        _ASSERT(align->GetAlignMgr().GetNumRows() >= 2);
        if (align->GetAlignMgr().GetNumRows() == 2)
            return eAlign_PW;
        // no pairwise alignment check if it's more than 100 rows
        // we really don't expect such a discontinuous alignments
        if (align->GetAlignMgr().GetNumRows() > 100)
            return eAlign_Multi;
        // if there are only two seq_ids involved then it can be treated as a Pairwise Alignment
        const CSeq_id* id = 0;
        for (IAlnExplorer::TNumrow row = 0; row < align->GetAlignMgr().GetNumRows(); ++row) {
            // skip the anchor
            if (align->GetAlignMgr().GetAnchor() == row)
                continue;
            const CSeq_id& seq_id = align->GetAlignMgr().GetSeqId (row);
            if (!id) {
                id = &seq_id;
                continue;
            }
            if (seq_id.Match(*id))
                continue;
            return eAlign_Multi;
        }
        return eAlign_PW;
    }

    const CMatePairGlyph* mp = dynamic_cast<const CMatePairGlyph*>(glyph);
    if (mp) {
        return eAlign_MatePair;
    }

    const CAlignSmearGlyph* smear = dynamic_cast<const CAlignSmearGlyph*>(glyph);
    if (smear) {
        return eAlign_Smear;
    }

    const CHistogramGlyph* graph = dynamic_cast<const CHistogramGlyph*>(glyph);
    if (graph) {
        return eAlign_Graph;
    }

    return eAlign_Non;
}


void  CAlignmentTrack::x_SaveConfiguration(const string& preset_style) const
{
    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryWriteView view =
        CSGConfigUtils::GetWriteView(registry, kBaseKey, preset_style, kDefProfile);
    view.Set("LinkMatePairAligns", m_DS->GetLinkMatePairs());
    view.Set("DNAScoringMethod", m_DS->GetDNAScoringMethod());
    view.Set("ProteinScoringMethod", m_DS->GetProteinScoringMethod());
    view.Set("EnableColoration", m_DS->GetEnableColoration());
    view.Set("ShowAlignedSeqFeats", m_ShowAlignedSeqFeats);
    view.Set("ShowIdenticalBases", m_ShowIdenticalBases);
    view.Set("ShowLabel", m_ShowLabel);
    view.Set("ShowUnalignedTailsForTrans2GenomicAln", m_ShowUnalignedTailsForTrans2GenomicAln);
    view.Set("Layout", LayoutValueToStr(m_Layout));
    view.Set("HideSraAlignments", HideFlagValueToStr(m_HideSraAlignments));
    view.Set("UnalignedTailsMode", UnalignedTailsModeValueToStr(m_UnalignedTailsMode));

    view.Set("MultiAlignProfile", m_MultiAlignProfile);
    view.Set("PWAlignProfile", m_PWAlignProfile);
    view.Set("AlignSmearProfile", m_AlignSmearProfile);
    view.Set("MatePairProfile", m_MatePairProfile);

    x_SaveMultiAlignSettings();
    x_SavePWAlignSettings();
    x_SaveAlignSmearSettings();
    x_SaveMatePairSettings();
}


void CAlignmentTrack::x_LoadAlignmentSettings(const string& profile,
                                              CRef<CAlignmentConfig>& conf)
{
    if ( !conf ) {
        conf.Reset(new CAlignmentConfig);
    }

    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryReadView view = CSGConfigUtils::GetReadView(
        registry, kAlignGlyphKey, profile, kDefProfile);
    //conf->m_ShowLabel = view.GetBool("ShowLabel", true);
    conf->m_ShowLabel = m_ShowLabel;
    conf->m_ShowIdenticalBases = m_ShowIdenticalBases;
    conf->m_UnalignedTailsMode = m_UnalignedTailsMode;
    conf->m_ShowUnalignedTailsForTrans2GenomicAln = m_ShowUnalignedTailsForTrans2GenomicAln;

    view = CSGConfigUtils::GetLabelPosReadView(
        registry, kAlignGlyphKey, profile, m_gConfig->GetLabelPos(), kDefProfile);
    try {
        conf->m_LabelPos = LabelPosStrToValue(view.GetString("LabelPos"));
    } catch (CException& e) {
        LOG_POST(Warning << "CAlignmentTrack::x_LoadAlignmentSettings() "
            << e.GetMsg());
    }

    // load color settings
    view = CSGConfigUtils::GetColorReadView(
        registry, kAlignGlyphKey, profile, m_gConfig->GetColorTheme(), kDefProfile);
    CSGConfigUtils::GetColor(view, "BG", conf->m_BG);
    CSGConfigUtils::GetColor(view, "FG", conf->m_FG);
    CSGConfigUtils::GetColor(view, "Label", conf->m_Label);
    CSGConfigUtils::GetColor(view, "Sequence", conf->m_Sequence);
    CSGConfigUtils::GetColor(view, "SeqMismatch", conf->m_SeqMismatch);
    CSGConfigUtils::GetColor(view, "TailColor", conf->m_TailColor);
    CSGConfigUtils::GetColor(view, "Insertion", conf->m_Insertion);
    CSGConfigUtils::GetColor(view, "Gap", conf->m_Gap);
    CSGConfigUtils::GetColor(view, "Intron", conf->m_Intron);
    CSGConfigUtils::GetColor(view, "NonConsensus", conf->m_NonConsensus);
    CSGConfigUtils::GetColor(view, "SmearColorMin", conf->m_SmearColorMin);
    CSGConfigUtils::GetColor(view, "SmearColorMax", conf->m_SmearColorMax);
    CSGConfigUtils::GetColor(view, "UnalignedFG", conf->m_UnalignedFG);
    CSGConfigUtils::GetColor(view, "UnalignedSequence", conf->m_UnalignedSequence);

    // load size settings
    view = CSGConfigUtils::GetSizeReadView(
        registry, kAlignGlyphKey, profile, m_gConfig->GetSizeLevel(), kDefProfile);

    conf->m_BarHeight = view.GetInt("BarHeight");
    CSGConfigUtils::GetFont(view, "LabelFontFace", "LabelFontSize", conf->m_LabelFont);
    CSGConfigUtils::GetFont(view, "SeqFontFace", "SeqFontSize", conf->m_SeqFont);
}


void
CAlignmentTrack::x_SaveAlignmentSettings(const string& profile,
                                         CConstRef<CAlignmentConfig> conf) const
{
    if ( !conf  ||  !conf->m_Dirty) {
        return;
    }

    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryWriteView view = CSGConfigUtils::GetWriteView(
        registry, kAlignGlyphKey, profile, kDefProfile);
    view.Set("ShowLabel", conf->m_ShowLabel);
    view.Set("ShowIdenticalBases", conf->m_ShowIdenticalBases);
    view.Set("m_ShowUnalignedTailsForTrans2GenomicAln", conf->m_ShowUnalignedTailsForTrans2GenomicAln);

    // label position
    view = CSGConfigUtils::GetLabelPosRWView(
        registry, kAlignGlyphKey, profile, m_gConfig->GetLabelPos(), kDefProfile);
    view.Set("LabelPos", LabelPosValueToStr(conf->m_LabelPos));

    // load color settings
    view = CSGConfigUtils::GetColorRWView(
        registry, kAlignGlyphKey, profile, m_gConfig->GetColorTheme(), kDefProfile);
    CSGConfigUtils::SetColor(view, "BG", conf->m_BG);
    CSGConfigUtils::SetColor(view, "FG", conf->m_FG);
    CSGConfigUtils::SetColor(view, "Label", conf->m_Label);
    CSGConfigUtils::SetColor(view, "Sequence", conf->m_Sequence);
    CSGConfigUtils::SetColor(view, "SeqMismatch", conf->m_SeqMismatch);
    CSGConfigUtils::SetColor(view, "TailColor", conf->m_TailColor);
    CSGConfigUtils::SetColor(view, "Insertion", conf->m_Insertion);
    CSGConfigUtils::SetColor(view, "Gap", conf->m_Gap);
    CSGConfigUtils::SetColor(view, "Intron", conf->m_Intron);
    CSGConfigUtils::SetColor(view, "NonConsensus", conf->m_NonConsensus);
    CSGConfigUtils::SetColor(view, "SmearColorMin", conf->m_SmearColorMin);
    CSGConfigUtils::SetColor(view, "SmearColorMax", conf->m_SmearColorMax);
    CSGConfigUtils::SetColor(view, "UnalignedFG", conf->m_UnalignedFG);
    CSGConfigUtils::SetColor(view, "UnalignedSequence", conf->m_UnalignedSequence);

    // load size settings
    view = CSGConfigUtils::GetSizeRWView(
        registry, kAlignGlyphKey, profile, m_gConfig->GetSizeLevel(), kDefProfile);

    view.Set("BarHeight", conf->m_BarHeight);
    CSGConfigUtils::SetFont(view, "LabelFontFace", "LabelFontSize", conf->m_LabelFont);
    CSGConfigUtils::SetFont(view, "SeqFontFace", "SeqFontSize", conf->m_SeqFont);

    conf->m_Dirty = false;
}


void CAlignmentTrack::x_LoadMatePairSettings()
{
    if ( !m_MatePairConf) {
        m_MatePairConf.Reset(new CMatePairConfig);
    }

    const string& profile = m_MatePairProfile;
    CMatePairConfig* conf = m_MatePairConf;

    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryReadView view = CSGConfigUtils::GetReadView(
        registry, kMatePairKey, profile, kDefProfile);
    //conf->m_ShowLabel = view.GetBool("ShowLabel", true);
    conf->m_ShowLabel = m_ShowLabel;

    // load color settings
    view = CSGConfigUtils::GetColorReadView(
        registry, kMatePairKey, profile, m_gConfig->GetColorTheme(), kDefProfile);
    CSGConfigUtils::GetColor(view, "BG", conf->m_BG);
    CSGConfigUtils::GetColor(view, "FGDistance", conf->m_FGDistance);
    CSGConfigUtils::GetColor(view, "FGLink", conf->m_FGLink);
    CSGConfigUtils::GetColor(view, "FGNo", conf->m_FGNo);
    CSGConfigUtils::GetColor(view, "FGNonUnique", conf->m_FGNonUnique);
    CSGConfigUtils::GetColor(view, "FGOrientation", conf->m_FGOrientation);
    CSGConfigUtils::GetColor(view, "FGCoAlign", conf->m_FGCoAlign);
    CSGConfigUtils::GetColor(view, "FGContraAlign", conf->m_FGContraAlign);
    CSGConfigUtils::GetColor(view, "Label", conf->m_Label);
    CSGConfigUtils::GetColor(view, "SeqDistance", conf->m_SeqDistance);
    CSGConfigUtils::GetColor(view, "SeqMismatchDistance", conf->m_SeqMismatchDistance);
    CSGConfigUtils::GetColor(view, "SeqMismatchNo", conf->m_SeqMismatchNo);
    CSGConfigUtils::GetColor(view, "SeqMismatchNonUnique", conf->m_SeqMismatchNonUnique);
    CSGConfigUtils::GetColor(view, "SeqMismatchOrientation", conf->m_SeqMismatchOrientation);
    CSGConfigUtils::GetColor(view, "SeqNo", conf->m_SeqNo);
    CSGConfigUtils::GetColor(view, "SeqNonUnique", conf->m_SeqNonUnique);
    CSGConfigUtils::GetColor(view, "SeqOrientation", conf->m_SeqOrientation);

    // load size settings
    view = CSGConfigUtils::GetSizeReadView(
        registry, kMatePairKey, profile, m_gConfig->GetSizeLevel(), kDefProfile);
    CSGConfigUtils::GetFont(view, "LabelFontFace", "LabelFontSize", conf->m_LabelFont);
}


void CAlignmentTrack::x_SaveMatePairSettings() const
{
    if ( !m_MatePairConf  ||  !m_MatePairConf->m_Dirty) {
        return;
    }

    const string& profile = m_MatePairProfile;
    const CMatePairConfig* conf = m_MatePairConf;

    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryWriteView view = CSGConfigUtils::GetWriteView(
        registry, kMatePairKey, profile, kDefProfile);
    view.Set("ShowLabel", conf->m_ShowLabel);

    // load color settings
    view = CSGConfigUtils::GetColorRWView(
        registry, kMatePairKey, profile, m_gConfig->GetColorTheme(), kDefProfile);
    CSGConfigUtils::SetColor(view, "BG", conf->m_BG);
    CSGConfigUtils::SetColor(view, "FGDistance", conf->m_FGDistance);
    CSGConfigUtils::SetColor(view, "FGLink", conf->m_FGLink);
    CSGConfigUtils::SetColor(view, "FGNo", conf->m_FGNo);
    CSGConfigUtils::SetColor(view, "FGNonUnique", conf->m_FGNonUnique);
    CSGConfigUtils::SetColor(view, "FGOrientation", conf->m_FGOrientation);
    CSGConfigUtils::SetColor(view, "Label", conf->m_Label);
    CSGConfigUtils::SetColor(view, "SeqDistance", conf->m_SeqDistance);
    CSGConfigUtils::SetColor(view, "SeqMismatchDistance", conf->m_SeqMismatchDistance);
    CSGConfigUtils::SetColor(view, "SeqMismatchNo", conf->m_SeqMismatchNo);
    CSGConfigUtils::SetColor(view, "SeqMismatchNonUnique", conf->m_SeqMismatchNonUnique);
    CSGConfigUtils::SetColor(view, "SeqMismatchOrientation", conf->m_SeqMismatchOrientation);
    CSGConfigUtils::SetColor(view, "SeqNo", conf->m_SeqNo);
    CSGConfigUtils::SetColor(view, "SeqNonUnique", conf->m_SeqNonUnique);
    CSGConfigUtils::SetColor(view, "SeqOrientation", conf->m_SeqOrientation);

    // load size settings
    view = CSGConfigUtils::GetSizeRWView(
        registry, kMatePairKey, profile, m_gConfig->GetSizeLevel(), kDefProfile);
    CSGConfigUtils::SetFont(view, "LabelFontFace", "LabelFontSize", conf->m_LabelFont);

    conf->m_Dirty = false;
}


void CAlignmentTrack::x_SetMsg()
{
    if (m_ObjNum > 0) {
        string msg = ", total ";
        msg += NStr::SizetToString(m_ObjNum, NStr::fWithCommas);
        msg += " object";
        msg += m_ObjNum > 1 ? "s shown" : " shown";
        SetMsg(msg);
    } else {
        SetMsg("");
    }
}


IAlnExplorer::EAlignType s_ParseAlignType(const string& align_type)
{
    static map<string, IAlnExplorer::EAlignType> s_ALnTypeMap = {
        {"na",  IAlnExplorer::fDNA},
        {"dna", IAlnExplorer::fDNA},
        {"aa", IAlnExplorer::fProtein},
        {"protein", IAlnExplorer::fProtein},
        {"mixed", IAlnExplorer::fMixed}
    };
    if (s_ALnTypeMap.count(align_type) != 0)
        return s_ALnTypeMap[align_type];
    return IAlnExplorer::fInvalid;
}


CSGAlignmentDS::EDataLoader s_ParseAlignDataLoader(const string& align_loader)
{
    static map<string, CSGAlignmentDS::EDataLoader> s_AlnLoaderMap = {
        {"BAM",  CSGAlignmentDS::eLoader_BAM},
        {"CSRA", CSGAlignmentDS::eLoader_CSRA}
    };
    if (s_AlnLoaderMap.count(align_loader) != 0)
        return s_AlnLoaderMap[align_loader];
    return CSGAlignmentDS::eLoader_Unknown;;
}


///////////////////////////////////////////////////////////////////////////////
/// CAlignmentTrackFactory
///////////////////////////////////////////////////////////////////////////////
ILayoutTrackFactory::TTrackMap
CAlignmentTrackFactory::CreateTracks(SConstScopedObject& object,
                                     ISGDataSourceContext* ds_context,
                                     CRenderingContext* r_cntx,
                                     const SExtraParams& params,
                                     const TAnnotMetaDataList& src_annots) const
{
    TAnnotNameTitleMap annots;

    // LOG_POST("<<<<");
    if (params.m_SkipGenuineCheck  &&  !params.m_Annots.empty()) {
        // LOG_POST("Skip genuine check, iterate through existing annots");
        ITERATE (SExtraParams::TAnnots, iter, params.m_Annots) {
            annots.insert(TAnnotNameTitleMap::value_type(*iter, ""));
        }
    } else {
        // LOG_POST("Do annots discovery");
        // collect non-NA tracks
        CIRef<ISGDataSource> pre_ds =
            ds_context->GetDS(typeid(CSGAlignmentDSType).name(), object);
        CSGAlignmentDS* aln_ds = dynamic_cast<CSGAlignmentDS*>(pre_ds.GetPointer());
        aln_ds->SetDepth(params.m_Level);
        aln_ds->SetAdaptive(params.m_Adaptive);
        SAnnotSelector sel(CSeqUtils::GetAnnotSelector(params.m_Annots));
        sel.SetCollectNames();
        aln_ds->GetAnnotNames(sel, r_cntx->GetVisSeqRange(), annots);

        // collect NA tracks
        if ( !src_annots.empty() ) {
            GetMatchedAnnots(src_annots, params, annots);
        }
    }
    // LOG_POST("Annots done");

    TKeyValuePairs track_settings;
    CSGConfigUtils::ParseProfileString(params.m_TrackProfile, track_settings);

    CSGAlignmentDS::EDataLoader data_loader { CSGAlignmentDS::eLoader_Unknown };
    if (track_settings.count("align_loader") > 0) {
        data_loader = s_ParseAlignDataLoader(track_settings["align_loader"]);
    }

    // check if there are any coverage graphs available
    TAnnotNameTitleMap graph_annots;
    try {
        if (params.m_CoverageGraphCheck && (CSGAlignmentDS::eLoader_Unknown == data_loader)) {
            // LOG_POST("Checking coverage graph");
            vector<string> target_g_annots;
            // LOG_POST("Iterate through existing annots");
            ITERATE (TAnnotNameTitleMap, iter, annots) {
                if (CSeqUtils::NameTypeStrToValue(iter->first) == CSeqUtils::eAnnot_Other) {
                    target_g_annots.push_back(iter->first);
                }
            }
            if ( !target_g_annots.empty() ) {
                // LOG_POST("Use ds based discovery");
                CIRef<ISGDataSource> igraph_ds =
                    ds_context->GetDS(typeid(CSGGraphDSType).name(), object);
                CSGGraphDS* graph_ds = dynamic_cast<CSGGraphDS*>(igraph_ds.GetPointer());
                graph_ds->SetDepth(params.m_Level);
                graph_ds->SetAdaptive(params.m_Adaptive);
                SAnnotSelector graph_sel(CSeqUtils::GetAnnotSelector(target_g_annots));
                graph_ds->GetAnnotNames(graph_sel, r_cntx->GetVisSeqRange(), graph_annots);
            }
            // LOG_POST("Checking coverage done");
        }
    } catch(const exception&) {
        // ignore all errors if no actual track check is requited
        // otherwise throw an error
        if(!params.m_SkipGenuineCheck || params.m_Annots.empty()) {
            throw;
        }
    }

    // create feature tracks
    TTrackMap tracks;
    // LOG_POST("Creating tracks");
    ITERATE(TAnnotNameTitleMap, iter, annots) {
        CIRef<ISGDataSource> ds = ds_context->GetDS(
            typeid(CSGAlignmentDSType).name(), object);
        // LOG_POST("Creating tracks 1");
        CSGAlignmentDS* aln_ds = dynamic_cast<CSGAlignmentDS*>(ds.GetPointer());
        aln_ds->SetDepth(params.m_Level);
        aln_ds->SetAdaptive(params.m_Adaptive);
        aln_ds->SetAnnotName(iter->first);
        aln_ds->SetSortBy(params.m_SortBy);
        aln_ds->SetRemotePath(params.m_RemotePath);
        if (CSGAlignmentDS::eLoader_Unknown != data_loader)
            aln_ds->SetAlnDataLoader(data_loader);

        bool is_reads = false;
        // LOG_POST("Creating tracks 2");
        if (!CSeqUtils::IsUnnamed(iter->first)) {
            is_reads = (graph_annots.count(iter->first) > 0) || (CSGAlignmentDS::eLoader_Unknown != data_loader);
            aln_ds->SetHasCoverageGraph(is_reads);
        }

        // LOG_POST("Creating tracks 3");
        CRef<CAlignmentTrack> aln_track(new CAlignmentTrack(aln_ds, r_cntx));
        if (is_reads)
            aln_track->SetDefaultProfile(kReadsProfile);
        // only do this for non-cSRA/BAM file
        // this takes a lot of time and is not needed for seqconfig, so is skipped when called from it
        // LOG_POST("Creating tracks 4");

        auto aln_type = IAlnExplorer::fInvalid;
        if (track_settings.count("align_type") > 0)
            aln_type = s_ParseAlignType(track_settings["align_type"]);
        if (aln_type != IAlnExplorer::fInvalid) {
            aln_track->SetAlnType(aln_type);
        } else if (is_reads) {
            aln_track->SetAlnType(IAlnExplorer::fDNA);
        } else if (aln_ds->IsBamLoader() || aln_ds->IsCSRALoader()) {
            aln_track->SetAlnType(IAlnExplorer::fDNA);
        } else {
            try {
                aln_track->SetAlnType(aln_ds->InitAlignType(params.m_FastConfig));
            } catch (const exception&) {
                // if this fails, most likely the track loading will fail later as well, but we need the track to be created
                // here with at least reasonable type so it can be displayed even if empty
                aln_track->SetAlnType(IAlnExplorer::fHomogenous);
            }
        }

        // LOG_POST("Creating tracks 5");
        aln_track->SetAnnot(iter->first);
        if (!iter->second.empty()) {
            aln_track->SetTitle(iter->second);
        }
        // LOG_POST("Creating tracks 6");
        aln_track->SetDSContext(ds_context);
        // LOG_POST("Creating tracks 7");
        tracks[iter->first] = aln_track.GetPointer();
    }
    // LOG_POST("Creating tracks done");
    // LOG_POST(">>>>");

    return tracks;
}

void CAlignmentTrackFactory::GetMatchedAnnots(
        const TAnnotMetaDataList& src_annots,
        const ILayoutTrackFactory::SExtraParams& params,
        TAnnotNameTitleMap& out_annots) const
{
    ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "align", "", out_annots);
}

static const char* kDefaultSraFilter = "none";
static const char* kDefaultUnalignedTailsMode = "glyph";
static const char* kDefaultScoringMethod = "Show Differences";
enum {
    CONFIG_SRA  = 1,
    CONFIG_NUC  = 2,
    CONFIG_PROT = 3,
    /// special setting for TMS tracks that did not go through complete alignment type discovery in seqconfig
    /// (IAlnExplorer::fHomogenous)
    /// this will create all possible kinds of scoring methods, in other respects will look like _NUC
    /// this is temporary until TMS will start to serve a real alignment type
    CONFIG_ALL  = 4
};

CRef<CTrackConfigSet>
CAlignmentTrackFactory::GetSettings(const string& profile,
                                    const TKeyValuePairs& settings,
                                    const CTempTrackProxy* track_proxy) const
{
    CRef<CTrackConfigSet> config_set(new CTrackConfigSet);
    bool sraTrack = false;
    // If track_proxy is not set we generate as many configs as possible
    // for given track type
    int config_num = CONFIG_PROT; // 3. protein/mixed 2. nuc 1. nuc SRA
    while (config_num) {
        CRef<CTrackConfig> config(new CTrackConfig);
        config_set->Set().push_back(config);
        config->SetHelp() = GetThisTypeInfo().GetDescr();
        config->SetLegend_text("anchor_7");

        CGuiRegistry& registry = CGuiRegistry::GetInstance();
        CRegistryReadView view =
            CSGConfigUtils::GetReadView(registry, kBaseKey, profile, kDefProfile);

        if (track_proxy) {
            IAlnExplorer::EAlignType aln_type = IAlnExplorer::fInvalid;
            CLayoutTrack* track = const_cast<CLayoutTrack*>(track_proxy->GetTrack());
            CAlignmentTrack* aln_track = dynamic_cast<CAlignmentTrack*>(track);
            if (aln_track) {
                aln_type = aln_track->m_AlnType;
            }

            switch(aln_type) {
            case IAlnExplorer::fProtein:
            case IAlnExplorer::fMixed:
                config_num = CONFIG_PROT;
                break;
            case IAlnExplorer::fHomogenous:
                config_num = CONFIG_ALL;
                break;
            default:
                config_num = CONFIG_NUC;
                if (profile == "BAM") {
                    // HACK, HACK
                    // It is a hack to indicate this is a BAM/cSRA track since
                    // we don't have access to BAM/cSRA path at this stage.
                    config_num = CONFIG_SRA;
                    sraTrack = true;
                }
                break;
            }
        }

        vector<string> scoring_methods;
        if (config_num == CONFIG_PROT || config_num == CONFIG_ALL) {
            scoring_methods.push_back("Column Quality score - Protein");
            scoring_methods.push_back("Frequency-Based Difference");
            scoring_methods.push_back(kDefaultScoringMethod);
            scoring_methods.push_back("Rasmol Amino Acid Colors");
            scoring_methods.push_back("Shapely Amino Acid Colors");
            scoring_methods.push_back("BLOSUM45");
            scoring_methods.push_back("BLOSUM62");
            scoring_methods.push_back("BLOSUM80");
            scoring_methods.push_back("Protein Quality Scoring with Coloring");
            scoring_methods.push_back("Hydropathy Scale");
            scoring_methods.push_back("Membrane preference");
            scoring_methods.push_back("Signal sequence");
            scoring_methods.push_back("Size");
        }

        if(config_num != CONFIG_PROT) {
            scoring_methods.push_back("Column Quality score - DNA");
            if(config_num != CONFIG_ALL) {
                scoring_methods.push_back("Frequency-Based Difference");
                scoring_methods.push_back(kDefaultScoringMethod);
            }
            scoring_methods.push_back("Nucleic Acid Colors");
            if (config_num == CONFIG_SRA) {
                scoring_methods.push_back(CSGAlnQualityScore::GetScoreName());
            }
        }

        if (!track_proxy) {
            switch (config_num) {
            case CONFIG_SRA:
                config->SetSubkey("SRA");
                break;
            case CONFIG_PROT:
                config->SetSubkey("protein");
                break;
            default:
                break;
            }
        }

        if (track_proxy) {
            // Configuration is for a specific track, end generation loop
            config_num = 0;
        } else {
            --config_num;
        }

        bool enable_color = view.GetBool("EnableColoration", true);
        string scoring_method = view.GetString("DNAScoringMethod", kDefaultScoringMethod);
        if (config_num == CONFIG_PROT) {
            scoring_method = view.GetString("ProteinScoringMethod", kDefaultScoringMethod);
        }

        bool aligned_seqfeats = view.GetBool("ShowAlignedSeqFeats", false);
        bool show_identical_bases = view.GetBool("ShowIdenticalBases", false);
        bool label = view.GetBool("ShowLabel", true);
        bool link_mate_pair = view.GetBool("LinkMatePairAligns", true);
        string rendering_style = view.GetString("Layout");
        string hide_sra = view.GetString("HideSraAlignments", kDefaultSraFilter);
        string unaligned_tails_mode = view.GetString("UnalignedTailsMode", kDefaultUnalignedTailsMode);
        int flag = view.GetInt("StatDisplay", 15);
        bool show_aln_stat = view.GetBool("ShowAlnStat", false);

        ITERATE (TKeyValuePairs, iter, settings) {
            try {
                if (NStr::EqualNocase(iter->first, "Color")) {
                    if (NStr::EqualNocase(iter->second, "false")) {
                        enable_color = false;
                    } else {
                        enable_color = true;
                        if ( !NStr::EqualNocase(iter->second, "true") ) {
                            // verify to see if it is a legitimate scoring method
                            ITERATE (vector<string>, sm_iter, scoring_methods) {
                                if (NStr::EqualNocase(iter->second, *sm_iter)) {
                                    scoring_method = iter->second;
                                    break;
                                }
                            }
                        }
                    }
                } else if (NStr::EqualNocase(iter->first, "AlignedSeqFeats"))
                    aligned_seqfeats = NStr::StringToBool(iter->second);
                else if (NStr::EqualNocase(iter->first, "IdenticalBases"))
                    show_identical_bases = NStr::StringToBool(iter->second);
                else if (NStr::EqualNocase(iter->first, "Label"))
                    label = NStr::StringToBool(iter->second);
                 else if (NStr::EqualNocase(iter->first, "UnalignedTailsMode"))
                     unaligned_tails_mode = iter->second;
                else if (NStr::EqualNocase(iter->first, "Layout")) {
                    // check whether the user-provided string is a valid layout string
                    CAlignmentTrack::LayoutStrToValue(iter->second);
                    rendering_style = iter->second;
                } else if (NStr::EqualNocase(iter->first, "StatDisplay")) {
                    flag = NStr::StringToInt(iter->second);
                } else if (NStr::EqualNocase(iter->first, "ShowAlnStat")) {
                    show_aln_stat = NStr::StringToBool(iter->second);
                } else if (NStr::EqualNocase(iter->first, "LinkMatePairAligns")) {
                    link_mate_pair = NStr::StringToBool(iter->second);
                } else if (NStr::EqualNocase(iter->first, "HideSraAlignments")) {
                    hide_sra = iter->second;
                } else if (NStr::EqualNocase(iter->first, "GraphHeight")) {
                    config->SetHidden_settings().push_back
                        (CTrackConfigUtils::CreateHiddenSetting("GraphHeight", iter->second));
                } else if (NStr::EqualNocase(iter->first, "GraphScale")) {
                    config->SetHidden_settings().push_back
                        (CTrackConfigUtils::CreateHiddenSetting("GraphScale", iter->second));
                } else if (NStr::EqualNocase(iter->first, "GraphColor")) {
                    config->SetHidden_settings().push_back
                        (CTrackConfigUtils::CreateHiddenSetting("GraphColor", iter->second));
                }  else if (NStr::EqualNocase(iter->first, "MinPileUpCost")) {
                    config->SetHidden_settings().push_back
                        (CTrackConfigUtils::CreateHiddenSetting("MinPileUpCost", iter->second));
                } else if (NStr::EqualNocase(iter->first, "MinAlignCost")) {
                    config->SetHidden_settings().push_back
                        (CTrackConfigUtils::CreateHiddenSetting("MinAlignCost", iter->second));
                } else if (NStr::EqualNocase(iter->first, "MaxAlignCost")) {
                    config->SetHidden_settings().push_back
                        (CTrackConfigUtils::CreateHiddenSetting("MaxAlignCost", iter->second));
                } else if (NStr::EqualNocase(iter->first, "MaxAlignShownFull")) {
                    config->SetHidden_settings().push_back
                        (CTrackConfigUtils::CreateHiddenSetting("MaxAlignShownFull", iter->second));
                } else if (NStr::EqualNocase(iter->first, "MaxAlignShownAdaptive")) {
                    config->SetHidden_settings().push_back
                        (CTrackConfigUtils::CreateHiddenSetting("MaxAlignShownAdaptive", iter->second));
                }

            } catch (CException&) {
                LOG_POST(Warning << "CAlignmentTrack::x_LoadSettings() invalid settings: "
                         << iter->first << ":" << iter->second);
            }
        }
        if (NStr::FindNoCase(rendering_style, "adaptive") != string::npos)
            rendering_style = "Adaptive";

        // alignment rendering style settings
        CRef<CChoice> choice = CTrackConfigUtils::CreateChoice(
            "Layout", "Alignment Display", rendering_style,
            "Alignment rendering style");
        choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
                CAlignmentTrack::LayoutValueToStr(CAlignmentTrack::eLayout_Adaptive),
                s_LayoutToDisplayName(CAlignmentTrack::eLayout_Adaptive),
                "Pack alignments if necessary",
                ""));
        choice->SetValues().push_back(
             CTrackConfigUtils::CreateChoiceItem(
                CAlignmentTrack::LayoutValueToStr(CAlignmentTrack::eLayout_Packed),
                s_LayoutToDisplayName(CAlignmentTrack::eLayout_Packed),
                "Always pack all alignments into a coverage graph or pileup graph",
                ""));
        choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
                CAlignmentTrack::LayoutValueToStr(CAlignmentTrack::eLayout_ExpandedByPos),
                s_LayoutToDisplayName(CAlignmentTrack::eLayout_ExpandedByPos),
                "Show one alignment per row sorted by alignments' start location",
                ""));
        choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
                CAlignmentTrack::LayoutValueToStr(CAlignmentTrack::eLayout_Full),
                s_LayoutToDisplayName(CAlignmentTrack::eLayout_Full),
                "Show all alignments",
                ""));

        config->SetChoice_list().push_back(choice);

        config->SetChoice_list().push_back(CAlnStatGlyph::CreateDisplayOptions("StatDisplay", flag));

        if ( !enable_color ) {
            scoring_method = "false";
        }
        // setting for alignment scoring method
        choice = CTrackConfigUtils::CreateChoice(
            "Color", "Score method", scoring_method,
            "Alignment score coloration method");

        ITERATE (vector<string>, iter, scoring_methods) {
            choice->SetValues().push_back(
                CTrackConfigUtils::CreateChoiceItem(
                    *iter, *iter, *iter, ""));
        }
        choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
                "false", "Disabled", "No score coloration", ""));
        config->SetChoice_list().push_back(choice);

        // setting for linking mate pair (on/off)
        config->SetCheck_boxes().push_back(
            CTrackConfigUtils::CreateCheckBox(
            "LinkMatePairAligns", "Link Mate Pairs",
            "Enable/disable linking mate reads", "", link_mate_pair));

        // setting for show pileup alongside with alignments (on/off)
        config->SetCheck_boxes().push_back(
            CTrackConfigUtils::CreateCheckBox(
                "ShowAlnStat","Show pileup",
                "Enable/disable pileup display alongside with alignments", "",
                show_aln_stat));

        // setting for projecting features from aligned sequence
        config->SetCheck_boxes().push_back(
            CTrackConfigUtils::CreateCheckBox(
                "AlignedSeqFeats", "Project features",
                "Show features projected from the aligned sequences", "",
                aligned_seqfeats));

        // setting for label (on/off)
        config->SetCheck_boxes().push_back(
            CTrackConfigUtils::CreateCheckBox(
            "Label", "Show Labels", "Show/hide alignment labels", "", label));

        // setting for showing identical bases for alignments, or just showing a glyph to show they match
        config->SetCheck_boxes().push_back(
            CTrackConfigUtils::CreateCheckBox(
            "IdenticalBases", "Show Identical Bases", "Write characters for matching bases", "", show_identical_bases));

        // Unaligned tails
        {
            choice = CTrackConfigUtils::CreateChoice(
                "UnalignedTailsMode", "Unaligned Tails", unaligned_tails_mode,
                "Unaligned tails display mode");
            choice->SetValues().push_back(
                CTrackConfigUtils::CreateChoiceItem(
                CAlignmentTrack::UnalignedTailsModeValueToStr(CAlignmentConfig::eTails_Hide),
                "Hide",
                "Hide the unaligned tails",
                ""));

            choice->SetValues().push_back(
                CTrackConfigUtils::CreateChoiceItem(
                CAlignmentTrack::UnalignedTailsModeValueToStr(CAlignmentConfig::eTails_ShowGlyph),
                "Show Tail Length",
                "Display the length of the unaligned tails",
                ""));

            choice->SetValues().push_back(
                CTrackConfigUtils::CreateChoiceItem(
                CAlignmentTrack::UnalignedTailsModeValueToStr(CAlignmentConfig::eTails_ShowSequence),
                "Show Sequence",
                "Display the unaligned tails row sequence",
                ""));

            config->SetChoice_list().push_back(choice);
        }

        // SRA alignments to hide
        if (sraTrack || (CONFIG_SRA == config_num)) {
            choice = CTrackConfigUtils::CreateChoice(
                "HideSraAlignments", "Hide Alignments", hide_sra,
                "SRA alignments to hide");
            choice->SetValues().push_back(
                 CTrackConfigUtils::CreateChoiceItem(
                    CAlignmentTrack::HideFlagValueToStr(CAlignmentConfig::eHide_None),
                    "None",
                    "Show all alignments",
                    ""));

            choice->SetValues().push_back(
                 CTrackConfigUtils::CreateChoiceItem(
                    CAlignmentTrack::HideFlagValueToStr(CAlignmentConfig::eHide_Duplicates),
                    "Duplicates",
                    "Hide PCR duplicates",
                    ""));

            choice->SetValues().push_back(
                 CTrackConfigUtils::CreateChoiceItem(
                    CAlignmentTrack::HideFlagValueToStr(CAlignmentConfig::eHide_BadReads),
                    "Bad reads",
                    "Hide reads with poor sequence quality",
                    ""));

            choice->SetValues().push_back(
                 CTrackConfigUtils::CreateChoiceItem(
                    CAlignmentTrack::HideFlagValueToStr(CAlignmentConfig::eHide_Both),
                    "Duplicates/Bad reads",
                    "Hide PCR duplicates and reads with poor sequence quality",
                    ""));

            config->SetChoice_list().push_back(choice);
        }

        string sort_by = kEmptyStr;
        if (track_proxy)  {
             sort_by = track_proxy->GetSortBy();
        }

        choice = CTrackConfigUtils::CreateChoice(
            "sort_by", "Sort alignments by", sort_by,
            "Sort alignments according to a selected criterion.");

        choice->SetValues().push_back(
            CTrackConfigUtils::CreateChoiceItem(
            "", "No sorting", "Don't sort alignments", ""));

        CAlignSorterFactory::SetTrackSetting(*choice);

        config->SetChoice_list().push_back(choice);

    } // while (config_num)

    return config_set;
}


void CAlignmentTrackFactory::RegisterIconImages() const
{
    CLayoutTrack::RegisterIconImage("track_align_score", "track_align_score.png");
    CLayoutTrack::RegisterIconImage("track_stat", "track_stat.png");
    CLayoutTrack::RegisterIconImage("track_tails", "track_tails.png");
}


string CAlignmentTrackFactory::GetExtensionIdentifier() const
{
    return CAlignmentTrack::m_TypeInfo.GetId();
}


string CAlignmentTrackFactory::GetExtensionLabel() const
{
    return CAlignmentTrack::m_TypeInfo.GetDescr();
}

 


END_NCBI_SCOPE
