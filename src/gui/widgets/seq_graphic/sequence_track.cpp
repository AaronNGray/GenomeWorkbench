/*  $Id: sequence_track.cpp 44739 2020-03-04 20:37:02Z evgeniev $
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

#include <gui/widgets/seq_graphic/sequence_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job_result.hpp>
#include <gui/widgets/seq_graphic/segment_smear_glyph.hpp>
#include <gui/widgets/seq_graphic/segment_config.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/seq_graphic/layout_track_proxy.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>

#include <gui/opengl/irender.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/registry.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/obj_fingerprint.hpp>

#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/seq/Seq_ext.hpp>
#include <objects/seq/Map_ext.hpp>
#include <objects/general/Object_id.hpp>

#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <util/sequtil/sequtil_manip.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
///   CSequenceTrack
///////////////////////////////////////////////////////////////////////////////

static const int kSeqTooltipLength = 15;
static const int kBarHeight = 10;
static const TModelUnit kGapBarHeight = 3.0;

static const string kDefProfile = "Default";
static const string kBaseKey = "GBPlugins.SeqGraphicSequenceBar";
static const string kShowLabelKey = "ShowLabel";
static const string kColorGapsKey = "ColorGaps";

CTrackTypeInfo CSequenceTrack::m_TypeInfo("sequence_track",
                                          "A horizontal bar representing the \
currently viewed top sequence in a zoomed-out view, and overlaid with sequence \
letters in a zoomed-in view at sequence level.  For nucleotide sequences, both \
the original sequence and the complementary sequence may be shown with two \
horizontal bars.");

CSequenceTrack::CSequenceTrack(CSGSequenceDS* seq_ds,
                               CRenderingContext* r_cntx,
                               CSGSegmentMapDS* seg_map_ds)
    : CDataTrack(r_cntx)
    , m_SeqDS(seq_ds)
    , m_SegMapDS(seg_map_ds)
    , m_ShowLabel(true)
    , m_ShowSegMap(false)
    , m_ColorGapsKey(true)
    , m_SegMapJobCompleted(false)
{
    if (m_SegMapDS) {
        m_SegMapDS->SetJobListener(this);
    }
    m_Simple->SetVertSpace(0);
    SetLayoutPolicy(m_Simple);

    // initialize the sequence label
    string title = "Sequence ";
    title += m_SeqDS->GetAcc_Best();

    string extra = m_SeqDS->GetTitle();
    if ( !extra.empty() ) {
        title += ": ";
        title += extra;
    }
    SetTitle(title);

    // left is always 0, and right is always the sequence length.
    SetLeft(0.0);
    SetWidth(m_SeqDS->GetSequenceLength());

    m_Location.Reset(new CSeq_loc());
    m_Location->SetInt().SetFrom(0);
    m_Location->SetInt().SetTo(m_SeqDS->GetSequenceLength() - 1);
    m_Location->SetInt().SetId().Assign(*m_SeqDS->GetBestIdHandle().GetSeqId());

    const CBioseq::TInst& inst = m_SeqDS->GetBioseqHandle().GetInst();
    if (inst.CanGetExt()  &&  inst.GetExt().IsMap()) {
        ITERATE (CMap_ext::Tdata, iter, inst.GetExt().GetMap().Get()) {
            const CSeq_feat& feat = **iter;
            if (feat.GetData().IsRsite()  &&  feat.GetLocation().IsPacked_pnt()) {
                CRef<CRsitesGlyph> glyph(new CRsitesGlyph(feat));
                glyph->SetRenderingContext(r_cntx);
                glyph->SetParent(this);
                m_Rsites.push_back(glyph);
            }
        }
    }

    //TTrackAttrFlags attr = fShowAlways | fFullTrack | fShowTitle | fCollapsible;
    //SetTrackAttr(attr);
}


const CSeqVector& CSequenceTrack::GetSeqVector() const
{
    return m_SeqDS->GetSeqVector();
}


CRef<CSeqGlyph> CSequenceTrack::HitTest(const TModelPoint& p)
{
    CRef<CSeqGlyph> glyph;
    if (IsIn(p)) {
        glyph.Reset(this);
    }
    return glyph;
}


bool CSequenceTrack::OnLeftDblClick(const TModelPoint& p)
{
    bool consumed = CLayoutTrack::OnLeftDblClick(p);

    if ( !consumed ) {
        TModelPoint pp(p);
        x_World2Local(pp);
        TModelUnit t_h = x_GetTBHeight();

        if (pp.Y() > t_h) {
            m_LTHost->LTH_ZoomOnRange(TSeqRange(0, m_SeqDS->GetSequenceLength()));
            consumed = true;
        }
    }

    return consumed;
}


bool CSequenceTrack::NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    // shortcut
    if ( !x_HasVisibleRsite() ) return true;

    TModelPoint pp(p);
    x_World2Local(pp);
    TModelUnit t_h = x_GetTBHeight();

    if (pp.Y() < t_h)  return true;

    TModelUnit b_h = x_GetBarHeight();
    if (pp.Y() < t_h + t_h - 2.0) return true;

    bool show_complement =
        GetSeqVector().IsNucleotide()  &&  m_Context->WillSeqLetterFit();
    if (show_complement  &&  pp.Y() < t_h + b_h * 2.0 - 2.0) {
        return true;
    }

    ITERATE (TRsites, iter, m_Rsites) {
        if ((*iter)->NeedTooltip(p, tt, t_title)) return true;
    }

    return false;
}


void CSequenceTrack::GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    CLayoutTrack::GetTooltip(p, tt, t_title);
    if ( !tt.IsEmpty() ) {
        return;
    }

    CBioseq_Handle bsh = m_SeqDS->GetBioseqHandle();
    tt.AddRow(sequence::CDeflineGenerator().GenerateDefline(bsh));
    if (tt.IsEmpty()) {
        tt.AddRow(m_SeqDS->GetAcc_Best());
    }

    string tmp;
    try {
        sequence::GetOrg_ref(bsh).GetLabel(&tmp);
        
        tt.AddRow("Organism:", tmp, 0);
    } catch (CException&) {
        /// ignore
    }

    /// add RefGeneTracking validation status
    if (m_SeqDS->IsRefSeq()) {
        CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_User);
        while (desc_ci) {
            const CUser_object& obj = desc_ci->GetUser();
            if (obj.GetType().IsStr()  &&  obj.GetType().GetStr() == "RefGeneTracking") {
                CConstRef<CUser_field> u_field = obj.GetFieldRef("Status");
                if (u_field  &&  u_field->GetData().IsStr()) {
                    tt.AddRow("RefSeq Status:", u_field->GetData().GetStr(),0);
                    break;
                }
            }
            ++desc_ci;
        }
    }

    // add portion specific to the current mouse position
    TModelPoint pp(p);
    x_World2Local(pp);
    TModelUnit t_h = x_GetTBHeight();

    if (pp.Y() < t_h) {
        return;
    }

    TModelUnit b_h = x_GetBarHeight();
    bool show_complement =
        GetSeqVector().IsNucleotide()  &&  m_Context->WillSeqLetterFit();
    bool on_complement = false;
    if (show_complement  &&  pp.Y() > t_h + b_h) {
        on_complement = true;
    }

    int seq_p = int(pp.X());
  
    tmp = NStr::UIntToString(seq_p + 1, NStr::fWithCommas);  // 1 based
    if (on_complement) {
        tmp += " (Complementary Strand)";
    } else {
        tmp += " (Direct Strand)";
    }
    tt.AddRow("Position:", tmp, 0);

    if (seq_p < 0  ||  seq_p > (int)(m_SeqDS->GetSequenceLength() - 1)) {
        return;
    }

    CSeqVector_CI seq_vec_it(GetSeqVector(), seq_p);
    if (seq_vec_it.IsInGap())
    {
        tmp = "gap";
        string length;
        CConstRef<CSeq_literal> gap = seq_vec_it.GetGapSeq_literal();
        if (gap)
        {
            if (gap->IsSetLength())
                length = " (" + NStr::UIntToString(gap->GetLength(), NStr::fWithCommas) + ")";
            if (gap->IsSetSeq_data() && gap->GetSeq_data().IsGap() && gap->GetSeq_data().GetGap().IsSetType() && gap->GetSeq_data().GetGap().GetType() == CSeq_gap::eType_contamination)
                tmp = "Contamination " + tmp;
            else if (gap->IsSetFuzz() && gap->GetFuzz().IsLim() && gap->GetFuzz().GetLim() == CInt_fuzz::eLim_unk)
                tmp = "Unknown " + tmp;           
            else
                tmp = "Known " + tmp;
        }
        tt.AddRow(tmp, length, 0);
        if (gap && gap->IsSetSeq_data() && gap->GetSeq_data().IsGap() && gap->GetSeq_data().GetGap().IsSetLinkage_evidence())
        {
            for (const auto &ev : gap->GetSeq_data().GetGap().GetLinkage_evidence())
            {
                if (ev->IsSetType())
                {
                    tmp = CLinkage_evidence::ENUM_METHOD_NAME(EType)()->FindName(ev->GetType(), true);
                    tt.AddRow("Linkage evidence: ", tmp, 0);
                }
            }
        }
    }

    try {
        tmp.clear();
        TSeqPos left  = max(seq_p - kSeqTooltipLength, 0);
        TSeqPos right = min(seq_p + kSeqTooltipLength,
            (int)(m_SeqDS->GetSequenceLength() - 1));

        m_SeqDS->GetSequence(left, right, tmp);
        if (on_complement) {
            CSeqManip::Complement(tmp, CSeqUtil::e_Iupacna,
                0, tmp.length(), tmp);
        }
        if ( !tmp.empty() ) {
            tmp.insert(tmp.begin() + (seq_p - left + 1), ']');
            tmp.insert(tmp.begin() + (seq_p - left), '[');
            tt.AddRow(tmp);
        }
    }
    catch (CException&) {
        /// ignore errors
    }
}


CHTMLActiveArea* CSequenceTrack::InitHTMLActiveArea(TAreaVector* p_areas) const
{
    if (p_areas == nullptr)
        return nullptr;
    bool seq_fit = m_Context->WillSeqLetterFit();
    // draw complementary sequence for nucleotide seq when showing sequence
    if (GetSeqVector().IsNucleotide()  &&  seq_fit) {
        CHTMLActiveArea area;
        CSeqGlyph::x_InitHTMLActiveArea(area);
        area.m_Flags =
            CHTMLActiveArea::fNoSelection |
            CHTMLActiveArea::fNoPin |
            CHTMLActiveArea::fNoHighlight |
            CHTMLActiveArea::fNoTooltip |
            CHTMLActiveArea::fSequence;

        TModelUnit top = x_GetTBHeight();
        area.m_Bounds.SetBottom(area.m_Bounds.Top() + GetHeight());
        area.m_Bounds.SetTop(area.m_Bounds.Top() + top);
        area.m_Signature = "Sequence strand";
        area.m_ParentId = GetId();
        p_areas->push_back(area);
    }

    // add HTML active areas for restriction sites
    
    if (!m_Rsites.empty()) {
        TAreaVector rsite_areas;
        rsite_areas.reserve(m_Rsites.size());
        for (const auto& rsite : m_Rsites) {
            rsite->GetHTMLActiveAreas(&rsite_areas);
        }
        // html areas need parent_id to be set otherwise tooltip will not be shown 
        for (auto& a : rsite_areas) {
            a.m_ParentId = GetId();
            p_areas->push_back(a);
        }
    }

    return CLayoutTrack::InitHTMLActiveArea(p_areas);
}


const CTrackTypeInfo&  CSequenceTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}


CRef<CSGGenBankDS> CSequenceTrack::GetDataSource()
{
    return CRef<CSGGenBankDS>(m_SegMapDS.GetPointer());
}


CConstRef<CSGGenBankDS> CSequenceTrack::GetDataSource() const
{
    return CConstRef<CSGGenBankDS>(m_SegMapDS.GetPointer());
}


const objects::CSeq_loc& CSequenceTrack::GetLocation(void) const
{
    return *m_Location;
}


CConstRef<CObject> CSequenceTrack::GetObject(TSeqPos pos) const
{
    return CConstRef<CObject>(m_Location->GetId());
}


void  CSequenceTrack::GetObjects(vector<CConstRef<CObject> >& objs) const
{
    objs.push_back(CConstRef<CObject>(m_Location->GetId()));
}


bool CSequenceTrack::HasObject(CConstRef<CObject> obj) const
{
    return m_Location->GetId() == obj.GetPointer();
}


string CSequenceTrack::GetSignature() const
{
    return CObjFingerprint::GetSeqLocSignature(*m_Location, &m_SeqDS->GetScope());
}


const IObjectBasedGlyph::TIntervals& CSequenceTrack::GetIntervals(void) const
{
    static TIntervals v;
    return v;
}


void CSequenceTrack::x_RenderContent() const
{
    if (!m_Context) {
        return;
    }

    bool seq_fit = m_Context->WillSeqLetterFit();
    TModelUnit top = x_GetTBHeight();
    TModelUnit h = x_GetBarHeight();
    TModelUnit gaps_bar_h = x_GetGapsBarHeight();
    const TModelRange& vr = m_Context->GetVisibleRange();
    TModelRect rcm(vr.GetFrom(), top + h + gaps_bar_h, vr.GetTo(), top);

    bool show_complement = GetSeqVector().IsNucleotide()  &&  seq_fit;
    bool show_strand = show_complement  &&  m_gConfig->GetShowComments();

    // draw direct sequence
    x_RenderSequence(rcm, seq_fit, true, show_strand);

    // draw complementary sequence for nucleotide seq when showing sequence
    if (show_complement) {
        rcm.Offset(0.0, h);
        x_RenderSequence(rcm, seq_fit, false, show_strand);
    }
    if (x_ShowSegMap())
        m_Group.Draw();

    ITERATE (TRsites, iter, m_Rsites) {
        (*iter)->Draw();
    }
}


bool CSequenceTrack::x_Empty() const
{
    return false;
}


void CSequenceTrack::x_RenderGaps(const TModelRect& rcm, TSeqPos from, TSeqPos to, bool show3d) const
{
    if ((x_ShowSegMap() && !m_SegMapJobCompleted) || !m_ColorGapsKey)
        return;

    CLogPerformance perfLog("CSequenceTrack::x_RenderGap");
    perfLog.AddParameter ("description", "Rendering gaps");

    IRender& gl = GetGl();
    // CSeqMap_CI it(m_SeqDS->GetBioseqHandle(), objects::CSeqMap::fFindGap, CRange<TSeqPos>(from, to));
    CSeqVector_CI seq_vec_it(GetSeqVector(), from);
    CSeqMap_CI it = seq_vec_it.GetCurrentSeqMap_CI();
    
    while(it)
    {    
        const TSeqPos start = it.GetPosition();
        const TSeqPos end   = min(to, it.GetEndPosition());
        if (start > to)
            break;  
        if (it.GetType() == objects::CSeqMap::eSeqGap)
        {           
            CRgbaColor c("purple");
            CConstRef<CSeq_literal> gap = it.GetRefGapLiteral();
            if (gap)
            {           
                if (gap->IsSetSeq_data() && gap->GetSeq_data().IsGap() && gap->GetSeq_data().GetGap().IsSetType() && gap->GetSeq_data().GetGap().GetType() == CSeq_gap::eType_contamination)
                {
                    c =  CRgbaColor("yellow");
                }
                else if (gap->IsSetFuzz() && gap->GetFuzz().IsLim() && gap->GetFuzz().GetLim() == CInt_fuzz::eLim_unk)
                {
                    c = CRgbaColor("red");
                    c.Darken(0.5);
                }
            }
            if (m_gConfig->GetColorTheme() == "Greyscale")
                c = c.GetGreyscale();
            if (!m_ColorGapsKey)
                c = CRgbaColor("black");
            if (show3d)
            {
                if (x_ShowSegMap())
                {
                    gl.ColorC(c);
                    m_Context->DrawQuad(start, rcm.Bottom() - 2, end, m_ColorGapsKey ? rcm.Top() : rcm.Top() + 2);
                }
                else
                {
                    m_Context->Draw3DQuad(start, rcm.Bottom() - 2, end, rcm.Top() + 2, c, true);
                }
            }
            else
            {        
                gl.ColorC(c);
                m_Context->DrawQuad(start, rcm.Bottom() - 2, end + 0.25, rcm.Top() + 2);            
            }
        }
        ++it;
    }

    perfLog.Post(CRequestStatus::e200_Ok);
}

void CSequenceTrack::x_RenderSequence(const TModelRect& rcm,
                                      bool seq_fit,
                                      bool direct,
                                      bool show_strand) const
{
    IRender& gl = GetGl();

    if ( !seq_fit ) {
        m_Context->Draw3DQuad(rcm.Left(), rcm.Bottom() - 2, rcm.Right(), rcm.Top() + 2 + x_GetGapsBarHeight(), m_SeqBarColor, true);
        x_RenderGaps(rcm, m_Context->GetVisSeqFrom(), m_Context->GetVisSeqTo(), true);
    } else {
        // Draw actual sequence if resolution permits
        gl.ColorC(m_SeqBarColor);
        m_Context->DrawQuad(rcm.Left(), rcm.Bottom() - 2, rcm.Right(),
            rcm.Top() + 2);

        TSeqPos from = m_Context->GetVisSeqFrom();
        TSeqPos to   = m_Context->GetVisSeqTo();

        try {
            string seq_str;
            const CSeqVector& s_vec = GetSeqVector();
            s_vec.GetSeqData(from, to, seq_str);
            x_RenderGaps(rcm, from, to, false);

            if (!direct) {
                CSeqManip::Complement(seq_str, CSeqUtil::e_Iupacna,
                    0, seq_str.length(), seq_str);
            }

            gl.ColorC(m_SeqColor);
            TModelUnit height = gl.TextHeight(m_SeqFont);
            TModelUnit fs = height + 2;

            char bases[2];
            bases[1] = '\0';

            for (TSeqPos bp = 0;  bp != seq_str.length();  bp++) {
                bases[0] = seq_str[bp];
                TModelUnit xM = from + bp + 0.5;
                TModelUnit yM = rcm.Top() + fs + 1;
                m_Context->TextOut(m_SeqFont.GetPointer(), bases, xM, yM, true);
            }
        }
        catch (CException&) {
            /// ignore exceptions from this - the only code that throws from
            /// above is sequence retrieval
        }
    }

    if (show_strand) {
        TModelUnit x = direct ? rcm.Left(): rcm.Right();
        TModelUnit y = direct ? rcm.Top() + 4.0 : rcm.Bottom() - 4.0;
        gl.ColorC(m_StrandColor);
        m_Context->Draw5Prime(x, y, direct, 12.0, 12.0);
    }
}


void  CSequenceTrack::x_LoadSettings(const string& preset_style,
                                     const TKeyValuePairs& settings)
{
    CConstRef<CSeqGraphicConfig> g_conf = x_GetGlobalConfig();
    if ( !g_conf ) {
        return;
    }
    if ( !m_SeqFont ) {
        m_SeqFont.Reset(new CGlTextureFont());
    }

    if (preset_style.empty()) {
        SetProfile(kDefProfile);
    } else {
        SetProfile(preset_style);
    }

    CGuiRegistry& registry = CGuiRegistry::GetInstance();

    // load settings basic settings for squence track
    CRegistryReadView view =
        CSGConfigUtils::GetReadView(registry, kBaseKey, GetProfile(), kDefProfile);
    m_ShowLabel = view.GetBool(kShowLabelKey, true);
    m_ColorGapsKey = view.GetBool(kColorGapsKey, true);
    m_ShowSegMap = view.GetBool("ShowSegmentMap", false);

    ITERATE (TKeyValuePairs, iter, settings) {
        try {
            if (NStr::EqualNocase(iter->first, kShowLabelKey)) {
                m_ShowLabel = NStr::StringToBool(iter->second);
            }
            if (NStr::EqualNocase(iter->first, kColorGapsKey)) {
                m_ColorGapsKey = NStr::StringToBool(iter->second);
            }
        } catch (CException&) {
            LOG_POST(Warning << "CSequenceTrack::x_LoadSettings() invalid settings - "
                     << iter->first << ":" << iter->second);
        }
    }

    view = CSGConfigUtils::GetSizeReadView(
        registry, kBaseKey, GetProfile(), g_conf->GetSizeLevel(), kDefProfile);
    CSGConfigUtils::GetFont(view, "SeqFontFace", "SeqFontSize", *m_SeqFont);

    // restriction sites
    // LOG_POST(Info << "loading rsite settings pre");
    if ( !m_Rsites.empty() ) {
        int rs_w = view.GetInt("RsiteWidth", 8);
        int rs_h = view.GetInt("RsiteHeight", 6);
        // LOG_POST(Info << "loading rsite settings");
        NON_CONST_ITERATE (TRsites, s_iter, m_Rsites) {
            (*s_iter)->SetSiteWidth(rs_w);
            (*s_iter)->SetSiteHeight(rs_h);
        }
    }

    view = CSGConfigUtils::GetColorReadView(
        registry, kBaseKey, GetProfile(), g_conf->GetColorTheme(), kDefProfile);
    CSGConfigUtils::GetColor(view, "Sequence", m_SeqColor);
    CSGConfigUtils::GetColor(view, "SequenceBar", m_SeqBarColor);
    CSGConfigUtils::GetColor(view, "SequenceStrand", m_StrandColor);

    // restriction sites
    if ( !m_Rsites.empty() ) {
        CRgbaColor rs_color;
        CSGConfigUtils::GetColor(view, "RsiteColor", rs_color);
        NON_CONST_ITERATE (TRsites, s_iter, m_Rsites) {
            (*s_iter)->SetSiteColor(rs_color);
        }
    }

    // load settings for segment map
    view = CSGConfigUtils::GetColorReadView(registry,
        "GBPlugins.SeqGraphicComponentMap", GetProfile(), m_gConfig->GetColorTheme(), kDefProfile);
    for (int i = 0; i < 5; ++i) {
        m_SegMapColors.push_back(CRgbaColor());
    }
    CSGConfigUtils::GetColor(view, "FGInc_F", m_SegMapColors[CSegmentConfig::eFinished]);
    CSGConfigUtils::GetColor(view, "FGInc_D", m_SegMapColors[CSegmentConfig::eDraft]);
    CSGConfigUtils::GetColor(view, "FGInc_W", m_SegMapColors[CSegmentConfig::eWgs]);
    CSGConfigUtils::GetColor(view, "FGInc_U", m_SegMapColors[CSegmentConfig::eOther]);
    CSGConfigUtils::GetColor(view, "FGGap",   m_SegMapColors[CSegmentConfig::eGap]);

    SetShowTitle(m_ShowLabel  &&  GetShowTitle());
}


void  CSequenceTrack::x_SaveSettings(const string& preset_style)
{
    TKeyValuePairs settings;

    if ( !preset_style.empty() ) {
        settings["profile"] = preset_style;
    }
    settings[kShowLabelKey] = NStr::BoolToString(m_ShowLabel);
    settings[kColorGapsKey] = NStr::BoolToString(m_ColorGapsKey);
    SetProfile(CSGConfigUtils::ComposeProfileString(settings));
}


void CSequenceTrack::x_UpdateData()
{
    m_SegMapJobCompleted = false;
    CDataTrack::x_UpdateData();
    if (x_ShowSegMap()) {
        m_SegMapDS->DeleteAllJobs();
        TSeqRange range = m_Context->GetVisSeqRange();
        const CSeqVector& seq_vec = GetSeqVector();
        m_SegMapDS->LoadSegmentSmear(range, m_Context->GetScale(), &seq_vec);
    } else {
        x_UpdateBoundingBox();
    }
}


void CSequenceTrack::x_OnJobCompleted(CAppJobNotification& notify)
{
    m_SegMapDS->ClearJobID(notify.GetJobID());
    m_SegMapJobCompleted = true;
    SetMsg("");
    m_Group.Clear();
    CRef<CObject> res_obj = notify.GetResult();
    CSGJobResult* result = dynamic_cast<CSGJobResult*>(&*res_obj);
    if (result  &&  result->m_ObjectList.size() == 1) {
        CSegmentSmearGlyph* seg_map =
            dynamic_cast<CSegmentSmearGlyph*>(result->m_ObjectList.front().GetPointer());
        seg_map->SetHeight(x_GetBarHeight());
        seg_map->SetColorCode(m_SegMapColors);
        seg_map->SetRenderingContext(m_Context);
        seg_map->SetParent(this);
        m_Group.PushBack(seg_map);
        x_UpdateLayout();
    } else {
        LOG_POST(Error << "CSequenceTrack::x_OnJobCompleted() "
            "failed to load segment map.");
    }
}


void  CSequenceTrack::x_SaveConfiguration(const string& preset_style) const
{
    CConstRef<CSeqGraphicConfig> g_conf = x_GetGlobalConfig();
    if ( !g_conf ) {
        return;
    }

    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryWriteView view =
        CSGConfigUtils::GetWriteView(registry, kBaseKey, preset_style, kDefProfile);
    view.Set(kShowLabelKey, m_ShowLabel);
    view.Set(kColorGapsKey, m_ColorGapsKey);

    view = CSGConfigUtils::GetSizeRWView(
        registry, kBaseKey, preset_style, g_conf->GetSizeLevel(), kDefProfile);
    CSGConfigUtils::SetFont(view, "SeqFontFace", "SeqFontSize", *m_SeqFont);

    view = CSGConfigUtils::GetColorRWView(
        registry, kBaseKey, preset_style, g_conf->GetColorTheme(), kDefProfile);
    CSGConfigUtils::SetColor(view, "Sequence", m_SeqColor);
    CSGConfigUtils::SetColor(view, "SequenceBar", m_SeqBarColor);
    CSGConfigUtils::SetColor(view, "SequenceStrand", m_StrandColor);
}


TModelUnit CSequenceTrack::x_GetBarHeight() const
{
    IRender& gl = GetGl();

    TModelUnit h = x_GetGlobalConfig()->GetObjectSpace();
    if ( m_Context->WillSeqLetterFit() ) {
        h += 4.0 + gl.TextHeight(m_SeqFont);
    } else {
        h += kBarHeight;
    }
    return h;
}

inline 
TModelUnit CSequenceTrack::x_GetGapsBarHeight() const
{
    TModelUnit gaps_bar_h{ 0.0 };
    if (GetSeqVector().IsNucleotide() && m_ColorGapsKey && x_ShowSegMap())
        gaps_bar_h = kGapBarHeight;

    return gaps_bar_h;
}

void CSequenceTrack::x_UpdateBoundingBox()
{
    CLayoutTrack::x_UpdateBoundingBox();
    if (m_Expanded) {

        TModelUnit gaps_bar_h = x_GetGapsBarHeight();

        m_Group.SetTop(gaps_bar_h);
        NON_CONST_ITERATE(CSeqGlyph::TObjects, iter, m_Group.SetChildren()) {
            (*iter)->Update(true);
            (*iter)->SetTop(GetHeight());
        }

        TModelUnit h = x_GetBarHeight();
        if (GetSeqVector().IsNucleotide()  &&  m_Context->WillSeqLetterFit()) {
            SetHeight(GetHeight() + 2 * h);
        } else {
            SetHeight(GetHeight() + h + gaps_bar_h);
        }


        NON_CONST_ITERATE (TRsites, iter, m_Rsites) {
            (*iter)->Update(true);
            (*iter)->SetTop(GetHeight());
        }

        if (x_HasVisibleRsite()) {
            SetHeight(GetHeight() + m_Rsites.front()->GetHeight());
        }
    }
}


bool CSequenceTrack::x_ShowSegMap() const
{
    return (m_SegMapDS  &&  !m_Context->WillSeqLetterFit());
}


bool CSequenceTrack::x_HasVisibleRsite() const
{
    ITERATE (TRsites, iter, m_Rsites) {
        if ((*iter)->HasVisibleRsite()) return true;
    }

    return false;
}


///////////////////////////////////////////////////////////////////////////////
/// CSequenceTrackFactory
///////////////////////////////////////////////////////////////////////////////
ILayoutTrackFactory::TTrackMap
CSequenceTrackFactory::CreateTracks(SConstScopedObject& object,
                                    ISGDataSourceContext* ds_context,
                                    CRenderingContext* r_cntx,
                                    const SExtraParams& params,
                                    const TAnnotMetaDataList& src_annots) const
{
    // LOG_POST("<<<<");
    TTrackMap tracks;
    CIRef<ISGDataSource> ds1 =
        ds_context->GetDS(typeid(CSGSequenceDSType).name(), object);
    CSGSequenceDS* seq_ds = dynamic_cast<CSGSequenceDS*>(ds1.GetPointer());

    // LOG_POST("Getting segment map DS");
    CIRef<ISGDataSource> ds2 = ds_context->GetDS(typeid(CSGSegmentMapDSType).name(), object);
    CSGSegmentMapDS* seg_map_ds = dynamic_cast<CSGSegmentMapDS*>(ds2.GetPointer());
    // LOG_POST("done");

    TAnnotNameTitleMap annots;
    if (CSGSegmentMapDS::GetEnabled()) {
        if (!params.m_Annots.empty()) try {
            if (!src_annots.empty()) {
                GetMatchedAnnots(src_annots, params, annots);
            }

            if (annots.empty()) {
                // LOG_POST("Creating a seq-table annot selector and retrieving annot names");
                SAnnotSelector sel(CSeqUtils::GetAnnotSelector(params.m_Annots));
                sel.SetAnnotType(CSeq_annot::C_Data::e_Seq_table);
                sel.SetResolveDepth(0); // SeqgMap is expected to be found on the top level only, GB-5507
                seg_map_ds->GetAnnotNames(sel, r_cntx->GetVisSeqRange(), annots);
                // LOG_POST("done");
            }
        } catch (CException& e) {
            ERR_POST(Error << e.GetMsg());
        } catch (exception& e) {
            ERR_POST(Error << e.what());
        }
        if (annots.empty()) {
            bool is_chromosome =
                CSGUtils::IsChromosome(seq_ds->GetBioseqHandle(), seq_ds->GetScope());
            TSeqRange range = TSeqRange::GetWhole();
            if ((is_chromosome  &&  !seg_map_ds->HasSegmentMap(1, range)) ||
                !seg_map_ds->HasSegmentMap(0, range)) {
                seg_map_ds = NULL;
            }
        } else {
            // there should be only one if any
            seg_map_ds->SetAnnot(annots.begin()->first);
        }
    } else {
        if ( !params.m_Annots.empty() ) {
            // there should be only one if any
            annots.insert(TAnnotNameTitleMap::value_type(params.m_Annots[0], ""));
            seg_map_ds->SetAnnot(annots.begin()->first);
        }
    }

    // LOG_POST("Creating CSequenceTrack");
    tracks[annots.empty() ? "Sequence" : annots.begin()->first] =
        CRef<CLayoutTrack>(new CSequenceTrack(seq_ds, r_cntx, seg_map_ds));
    // LOG_POST("done");
    // LOG_POST(">>>>");

    return tracks;
}

void CSequenceTrackFactory::GetMatchedAnnots(
    const TAnnotMetaDataList& src_annots,
    const ILayoutTrackFactory::SExtraParams& params,
    TAnnotNameTitleMap& out_annots) const
{
    if (!params.m_Annots.empty())
        ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "seq-table", "", out_annots);
}


string CSequenceTrackFactory::GetExtensionIdentifier() const
{
    return CSequenceTrack::m_TypeInfo.GetId();
}


string CSequenceTrackFactory::GetExtensionLabel() const
{
    return CSequenceTrack::m_TypeInfo.GetDescr();
}


CRef<CTrackConfigSet>
CSequenceTrackFactory::GetSettings(const string& profile,
                                   const TKeyValuePairs& settings,
                                   const CTempTrackProxy* track_proxy) const
{
    CRef<CTrackConfigSet> config_set(new CTrackConfigSet);
    // create a track configure
    CRef<CTrackConfig> config(new CTrackConfig);
    config_set->Set().push_back(config);
    config->SetHelp() = GetThisTypeInfo().GetDescr();
    config->SetLegend_text("anchor_8");

    if (track_proxy) {
        const CSequenceTrack* track =
            dynamic_cast<const CSequenceTrack*>(track_proxy->GetTrack());
        if (track  &&  track->m_SegMapDS  &&
            (track->m_ShowSegMap  ||  !track->m_SegMapDS->GetAnnot().empty())) {
            config->SetHelp() = "The sequence bar is colored by the source \
sequence and to generate that base. <br>Blue=finished sequence<br>\
Orange=draft sequence<br>Green=WGS<br>Gray=Other<br>Black=Gap";
        }
    }

    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryReadView view =
        CSGConfigUtils::GetReadView(registry, kBaseKey, profile, kDefProfile);
    bool show_label = view.GetBool(kShowLabelKey, false);
    bool color_gaps = view.GetBool(kColorGapsKey, false);

    ITERATE (TKeyValuePairs, iter, settings) {
        try {
            if (NStr::EqualNocase(iter->first, kShowLabelKey)) {
                show_label = NStr::StringToBool(iter->second);
            }
            if (NStr::EqualNocase(iter->first, kColorGapsKey)) {
                color_gaps = NStr::StringToBool(iter->second);
            }
        } catch (CException&) {
            LOG_POST(Warning << "CSequenceTrack::x_LoadSettings() invalid settings - "
                     << iter->first << ":" << iter->second);
        }
    }


    // setting for label (on/off)
    config->SetCheck_boxes().push_back(
        CTrackConfigUtils::CreateCheckBox(
        kShowLabelKey, "Show Label", "Show/hide sequence track title", "", show_label));
    config->SetCheck_boxes().push_back(
        CTrackConfigUtils::CreateCheckBox(
            kColorGapsKey, "Color gaps by type", "purple = gap of known length, red = gap of unknown length, yellow = contamination", "", color_gaps));

    return config_set;
}


END_NCBI_SCOPE
