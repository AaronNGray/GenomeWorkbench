/*  $Id: six_frames_trans_track.cpp 43682 2019-08-14 16:00:25Z katargir $
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

#include <gui/widgets/seq_graphic/six_frames_trans_track.hpp>
#include <gui/widgets/seq_graphic/translation_glyph.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/utils/rgba_color.hpp>
//#include <gui/objutils/registry.hpp>

#include <objects/seqfeat/Genetic_code_table.hpp>
#include <objects/taxon1/taxon1.hpp>
#include <util/sequtil/sequtil_manip.hpp>

#include <objmgr/util/sequence.hpp>

#include <wx/menu.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
    
static const string kTrackTitle = "Six-frame translation";
/// largest sequence range we will do sequence translations.
static const TSeqPos kTransRangeLimit = 100000;

static const int kMaxOrfLen = 100000000;

// content menu item base id
static const size_t kContentBaseID = 10001;

static const string kBaseKey = "GBPlugins.SixframeTrans";
static const string kDefProfile = "Default";

///////////////////////////////////////////////////////////////////////////////
/// CSFTranslationJob - Graphical view six-frame translation job.
///
class CSFTranslationJob : public CSeqGraphicJob
{
public:
    CSFTranslationJob(const string& desc, objects::CBioseq_Handle handle,
        const TSeqRange& range, ENa_strand strand = eNa_strand_both)
        : CSeqGraphicJob(desc)
        , m_Handle(handle)
        , m_Range(range)
        , m_Strand(strand)
        , m_GeneticCode(-1)
        , m_AltStart(false)
    {
        SetTaskName("Translating in six-frame...");
    }

    void SetGeneticCode(int id);
    void SetAltStart(bool f);

protected:
    virtual EJobState x_Execute();

private:
    /// Do translation for three frame shifts.
    /// This method creates only one translation glyph for each frame
    /// shift.  It returns the translated seuence regardless of the
    /// settings.
    bool x_Translate(CSeqGlyph::TObjects& objs, const string& ori_seq,
            const CTrans_table& tbl, TSeqPos start, bool negative);

    void x_InitGeneticCode();

private:
    objects::CBioseq_Handle m_Handle;   ///< target sequence
    TSeqRange               m_Range;    ///< target range
    ENa_strand              m_Strand;   ///< Translated strand (plus, minus, both)
    int                     m_GeneticCode;
    bool                    m_AltStart; ///< allow alternative start codon
};

inline
void CSFTranslationJob::SetGeneticCode(int id)
{
    m_GeneticCode = id;
}

inline
void CSFTranslationJob::SetAltStart(bool f)
{
    m_AltStart = f;
}

IAppJob::EJobState CSFTranslationJob::x_Execute()
{
    CSGJobResult* result = new CSGJobResult();
    m_Result.Reset(result);
    CSeqGlyph::TObjects& objs = result->m_ObjectList;
    if (m_Range.GetLength() < 3) return eCompleted;

    // positive strand translation
    TSeqPos start = m_Range.GetFrom() / 3 * 3;
    TSeqPos to = m_Range.GetTo() / 3 * 3 + 3;
    auto seq_len = m_Handle.GetBioseqLength();
    int t_off = (seq_len - to) % 3;
    to += t_off;

    CRef<objects::CSeqVector> seq_vector(
        new CSeqVector(m_Handle.GetSeqVector(CBioseq_Handle::eCoding_Iupac)));
    string ori_seq;
    seq_vector->GetSeqData(start, to, ori_seq);

    if (m_GeneticCode < 0) {
        x_InitGeneticCode();
        result->m_Desc = NStr::NumericToString(m_GeneticCode);
    }

    const CTrans_table& tbl = CGen_code_table::GetTransTable(m_GeneticCode);

    SetTaskTotal((to - start) * 2);
    SetTaskCompleted(0);

    if (m_Strand == eNa_strand_both || m_Strand == eNa_strand_plus) {
        SetTaskName("Translating in postive strand...");
        if (!x_Translate(objs, ori_seq, tbl, start, false)) {
            return eCanceled;
        }
    }
    if (m_Strand == eNa_strand_both || m_Strand == eNa_strand_minus) {
        string rev_comp_seq;
        CSeqManip::ReverseComplement(ori_seq, CSeqUtil::e_Iupacna,
            0, ori_seq.size(), rev_comp_seq);
        SetTaskName("Translating in negative strand...");
        if (!x_Translate(objs, rev_comp_seq, tbl, start, true)) {
            return eCanceled;
        }
    }
    SetTaskCompleted((to - start) * 2);

    return eCompleted;
}


bool CSFTranslationJob::x_Translate(CSeqGlyph::TObjects& objs,
                                    const string& ori_seq,
                                    const CTrans_table& tbl,
                                    TSeqPos start, bool negative)
{
    // main loop through bases
    size_t shift = 0;
    size_t state = 0;
    size_t length = ori_seq.size();

    typedef vector< CRef<CTranslationGlyph> > TTRans;
    TTRans prots;
    int frame_base = negative ? 3 : 0;
    for (size_t idx = 0; idx < 3; ++idx) {
        CTranslationGlyph::EFrame frame =
            (CTranslationGlyph::EFrame)(frame_base + idx);
        CRef<CTranslationGlyph> prot(
            new CTranslationGlyph(start, start + length -1, frame));
        prot->GetTranslation().reserve(length/3);
        prots.push_back(prot);
    }

    if (negative) start += length - 1;

    int orf_starts[3] = {-1, -1, -1};
    size_t i = 0;
    while (i < 2) {
        state = tbl.NextCodonState(state, ori_seq[i]);
        ++shift;
        ++i;
    }

    while (i < length) {
        if (IsCanceled()) {
            return false;
        }
        state = tbl.NextCodonState(state, ori_seq[i]);
        char res = tbl.GetCodonResidue(state);

        if (++shift == 3) shift = 0;
        prots[shift]->GetTranslation().push_back(res);

        TSeqPos curr_pos = start + (negative ? -1 : 1) * (i - 1);
        if (res == '*') {
            prots[shift]->GetStopCodons().push_back(curr_pos);
            if (orf_starts[shift] > -1) {
                prots[shift]->GetOrfs().push_back(TSeqRange(orf_starts[shift], curr_pos));
                orf_starts[shift] = -1;
            }
        }
        if (res == 'M'  ||  (m_AltStart  &&  tbl.IsAltStart(state))) {
            prots[shift]->GetStartCodons().push_back(curr_pos);
            if (orf_starts[shift] == -1) {
                orf_starts[shift] = curr_pos;
            }
        }
        AddTaskCompleted(1);
        ++i;
    }

    for (size_t idx = 0; idx < 3; ++idx) {
        objs.push_back(CRef<CSeqGlyph>(prots[idx].GetPointer()));
    }

    return true;
}


void CSFTranslationJob::x_InitGeneticCode()
{
    // get an appropriate translation table. For the full list of tables,
    // please refer to https://www.ncbi.nlm.nih.gov/Taxonomy/Utils/wprintgc.cgi
    short gc_id = -1;

    try {
        const COrg_ref& org = sequence::GetOrg_ref(m_Handle);
        if (CSGUtils::IsMitochondrion(m_Handle)  &&  org.IsSetMgcode()) {
            gc_id = org.GetMgcode();
        } else if (CSGUtils::IsPlastid(m_Handle)  &&  org.IsSetPgcode()) {
            gc_id = org.GetPgcode();
        } else if (org.IsSetGcode()) {
            gc_id = org.GetGcode();
        }
    } catch (CException&) {
        // ignore it, will try other approach
    }

    if (gc_id < 0) {
        gc_id = 1;   // use standard genetic code table by default
        CTaxon1 tax;
        STimeout time_out;
        time_out.sec = 5;
        time_out.usec = 0;

        if (tax.Init(&time_out, 3)) { // 5 second time out, and 3 attempts
            // first, get the tax id
            int tax_id = -1;
            CSeq_id_Handle gi_h = sequence::GetId(m_Handle.GetSeq_id_Handle(),
                m_Handle.GetScope(), sequence::eGetId_ForceGi);
            if (gi_h  &&  gi_h.GetGi() > ZERO_GI) {
                TGi gi = gi_h.GetGi();
                tax.GetTaxId4GI(gi, tax_id);
            } else {
                // try using another way for sequences that don't have a gi
                tax_id = m_Handle.GetScope().GetTaxId(m_Handle.GetSeq_id_Handle()); 
            }

            if (tax_id > -1) {
                ITaxon1Node* node;
                if (tax.LoadNode(tax_id, (const ITaxon1Node**)(&node))) {
                    if (CSGUtils::IsMitochondrion(m_Handle)) {
                        gc_id = node->GetMGC();
                    } else {
                        gc_id = node->GetGC();
                    }
                }
            }
        }
    }

    m_GeneticCode = gc_id;
}


///////////////////////////////////////////////////////////////////////////////
/// CSFTransDS
///
CSFTransDS::CSFTransDS(CScope& scope, const CSeq_id& id)
    : CSGGenBankDS(scope, id)
    , m_GeneticCode(-1)
    , m_AltStart(false)
{}


void CSFTransDS::DoTranslation(const TSeqRange& range, ENa_strand strand)
{
    CRef<CSFTranslationJob>
        job( new CSFTranslationJob("Six-frame translation",
        m_Handle, range, strand) );
    job->SetGeneticCode(m_GeneticCode);
    job->SetAltStart(m_AltStart);
    x_LaunchJob(*job);
}


///////////////////////////////////////////////////////////////////////////////
/// CSFTransDSType
///
ISGDataSource*
CSFTransDSType::CreateDS(SConstScopedObject& object) const
{
    const CSeq_id& id = dynamic_cast<const CSeq_id&>(object.object.GetObject());
    return new CSFTransDS(object.scope.GetObject(), id);
}


string CSFTransDSType::GetExtensionIdentifier() const
{
    static string sid("six_frames_trans_ds_type");
    return sid;
}


string CSFTransDSType::GetExtensionLabel() const
{
    static string slabel("Six-frame Translation Data Source Type");
    return slabel;
}


bool CSFTransDSType::IsSharable() const
{
    return false;
}


///////////////////////////////////////////////////////////////////////////////
///   CSixFramesTransTrack
///
typedef SStaticPair<const char*, CSixFramesTransTrack::ETranslation> TTranslationStr;
static const TTranslationStr s_TransStrs[] = {
    { "Adaptive",  CSixFramesTransTrack::eTrans_Adaptive },
    { "Always",    CSixFramesTransTrack::eTrans_Always },
};

typedef CStaticArrayMap<string, CSixFramesTransTrack::ETranslation> TTransMap;
DEFINE_STATIC_ARRAY_MAP(TTransMap, sm_TransMap, s_TransStrs);


CSixFramesTransTrack::ETranslation CSixFramesTransTrack::TransStrToValue(const string& trans)
{
    TTransMap::const_iterator iter = sm_TransMap.find(trans);
    if (iter != sm_TransMap.end()) {
        return iter->second;
    }
    NCBI_THROW(CException, eInvalid, "Invalid translation option string: " + trans);
}


const string& CSixFramesTransTrack::TransValueToStr(CSixFramesTransTrack::ETranslation trans)
{
    TTransMap::const_iterator iter;
    for (iter = sm_TransMap.begin();  iter != sm_TransMap.end();  ++iter) {
        if (iter->second == trans) {
            return iter->first;
        }
    }
    return kEmptyStr;
}


typedef SStaticPair<const char*, CSixFramesTransTrack::EStrandOption> TOptionStr;
static const TOptionStr s_OptionStrs[] = {
    { "All", CSixFramesTransTrack::eOpt_All },
    { "Forward", CSixFramesTransTrack::eOpt_Forward },
    { "Reverse", CSixFramesTransTrack::eOpt_Reverse },
    { "Sense", CSixFramesTransTrack::eOpt_LeftToRight }
};

typedef CStaticArrayMap<string, CSixFramesTransTrack::EStrandOption> TOptionMap;
DEFINE_STATIC_ARRAY_MAP(TOptionMap, sm_OptionMap, s_OptionStrs);


CSixFramesTransTrack::EStrandOption CSixFramesTransTrack::OptionStrToValue(const string& opt)
{
    TOptionMap::const_iterator iter = sm_OptionMap.find(opt);
    if (iter != sm_OptionMap.end()) {
        return iter->second;
    }
    NCBI_THROW(CException, eInvalid, "Invalid translation option string: " + opt);
}


const string& CSixFramesTransTrack::OptionValueToStr(CSixFramesTransTrack::EStrandOption opt)
{
    TOptionMap::const_iterator iter;
    for (iter = sm_OptionMap.begin();  iter != sm_OptionMap.end();  ++iter) {
        if (iter->second == opt) {
            return iter->first;
        }
    }
    return kEmptyStr;
}


CTrackTypeInfo CSixFramesTransTrack::m_TypeInfo("six_frames_translation",
                          "Graphical View Six-frame Translation Track");


CSixFramesTransTrack::CSixFramesTransTrack(CSFTransDS* ds, CRenderingContext* r_cntx)
    : CDataTrack(r_cntx)
    , m_DS(ds)
    , m_Trans(eTrans_Adaptive)
    , m_Option(eOpt_All)
    , m_TransStrand(eNa_strand_both)
{
    m_DS->SetJobListener(this);
    x_RegisterIcon(SIconInfo(eIcon_Help, "Help on genetic code", true, "track_help"));
    x_RegisterIcon(SIconInfo(eIcon_Content, "Content", true, "track_content"));
    x_RegisterIcon(SIconInfo(eIcon_Genetic, "Genetic Code", true, "track_genetic_code"));
    x_RegisterIcon(SIconInfo(eIcon_Settings, "Settings", true, "track_settings"));
    SetLayoutPolicy(m_Simple);
}


CSixFramesTransTrack::~CSixFramesTransTrack()
{
    if ( !m_DS->AllJobsFinished() ) {
        m_DS->DeleteAllJobs();
    }
}


const CTrackTypeInfo& CSixFramesTransTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}


string CSixFramesTransTrack::GetFullTitle() const
{
    if (GetTitle().empty()) {
        return kTrackTitle;
    }
    return GetTitle();
}

void CSixFramesTransTrack::Update(bool layout_only)
{
    if (layout_only) {
        if (m_Option == eOpt_LeftToRight) {
            layout_only = (m_Context->IsFlippedStrand() && m_TransStrand == eNa_strand_minus)
                || (!m_Context->IsFlippedStrand() && m_TransStrand == eNa_strand_plus);
            if (!layout_only) {
                m_TransRange.Set(0, 0);
                x_UpdateTitle();
            }
        }
    }
    CDataTrack::Update(layout_only);
}

void CSixFramesTransTrack::x_LoadSettings(const string& preset_style,
                                          const TKeyValuePairs& settings)
{
    m_Layered->SetVertSpace(m_gConfig->GetObjectSpace());
    m_Simple->SetVertSpace(0);

    if ( !m_Config) {
        m_Config.Reset(new CTranslationConfig);
    }

    if ( !m_GroupConf ) {
        m_GroupConf.Reset(new CBoundaryParams);
        m_GroupConf->SetShowBackground(true);
        m_GroupConf->SetShowBoundary(false);
    }

    if (preset_style.empty()) {
        SetProfile(kDefProfile);
    } else {
        SetProfile(preset_style);
    }

    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryReadView view;
    view = CSGConfigUtils::GetReadView(registry, kBaseKey, GetProfile(), kDefProfile);
    m_Option = OptionStrToValue(view.GetString("ShowOption"));
    m_Config->SetOrfThreshold(view.GetInt("OrfThreshold", 20));
    m_Config->SetHighlightCodons(view.GetBool("HighlightCodons", true));
    m_Config->SetShowLabel(m_gConfig->GetShowComments());

    view = CSGConfigUtils::GetColorReadView(
        registry, kBaseKey, GetProfile(), m_gConfig->GetColorTheme(), kDefProfile);

    CRgbaColor color;
    CSGConfigUtils::GetColor(view, "StartCodonColor", color);
    m_Config->SetStartCodonColor(color);
    CSGConfigUtils::GetColor(view, "StopCodonColor", color);
    m_Config->SetStopCodonColor(color);
    CSGConfigUtils::GetColor(view, "OrfHighlightColor", color);
    m_Config->SetOrfHighlightColor(color);
    CSGConfigUtils::GetColor(view, "SeqColor", color);
    m_Config->SetSeqColor(color);
    CSGConfigUtils::GetColor(view, "OrfSeqColor", color);
    m_Config->SetOrfSeqColor(color);
    CSGConfigUtils::GetColor(view, "BgColor", color);
    m_GroupConf->SetBgColor(color);
    CSGConfigUtils::GetColor(view, "LabelBgColor", color);
    m_Config->SetLabelBgColor(color);
    CSGConfigUtils::GetColor(view, "CommentColor", color);
    m_Config->SetCommentColor(color);


    view = CSGConfigUtils::GetSizeReadView(
        registry, kBaseKey, GetProfile(), m_gConfig->GetSizeLevel(), kDefProfile);
    m_Config->SetBarHeight(view.GetInt("BarHeight", 12));
    CSGConfigUtils::GetFont(view, "SeqFontFace", "SeqFontSize", m_Config->GetSeqFont());
    CSGConfigUtils::GetFont(view, "StrandFontFace", "StrandFontSize", m_Config->GetStrandFont());


    ITERATE (TKeyValuePairs, iter, settings) {
        try {
            if (NStr::EqualNocase(iter->first, "Translation")) {
                m_Trans = TransStrToValue(iter->second);
            } else if (NStr::EqualNocase(iter->first, "OrfThreshold")) {
                m_Config->SetOrfThreshold(NStr::StringToInt(iter->second));
            } else if (NStr::EqualNocase(iter->first, "ShowOption")) {
                m_Option = OptionStrToValue(iter->second);
            } else if (NStr::EqualNocase(iter->first, "HighlightCodons")) {
                m_Config->SetHighlightCodons(NStr::StringToBool(iter->second));
            } else if (NStr::EqualNocase(iter->first, "AltStart")) {
                m_DS->SetAltStart(NStr::StringToBool(iter->second));
            } 
        } catch (CException&) {
            LOG_POST(Warning << "CSixFramesTransTrack::x_LoadSettings() "
                "invalude translation setting:" << iter->second);
        }
    }

    x_UpdateTitle();
}


void CSixFramesTransTrack::x_SaveSettings(const string& preset_style)
{
    TKeyValuePairs settings;

    if ( !preset_style.empty() ) {
        settings["profile"] = preset_style;
    }

    settings["ShowOption"] = OptionValueToStr(m_Option);
    settings["OrfThreshold"] = NStr::IntToString(m_Config->GetOrfThreshold());
    settings["HighlightCodons"] = NStr::BoolToString(m_Config->GetHighlightCodons());
    settings["AltStart"] = NStr::BoolToString(m_DS->GetAltStart());
    SetProfile("Translation:" + TransValueToStr(m_Trans));

    SetProfile(CSGConfigUtils::ComposeProfileString(settings));
}


void CSixFramesTransTrack::x_OnIconClicked(TIconID id)
{
    switch (id) {
    case eIcon_Help:
        {{
            string link = "https://www.ncbi.nlm.nih.gov/Taxonomy/taxonomyhome.html/index.cgi?chapter=tgencodes#SG";
            link += NStr::IntToString(m_DS->GetGeneticCode());
            ::wxLaunchDefaultBrowser(ToWxString(link));
        }}
        break;
    case eIcon_Content:
        x_OnContentIconClicked();
        break;
    case eIcon_Settings:
        x_OnSettingIconClicked();
        break;
    case eIcon_Genetic:
        x_OnGeneticIconClicked();
        break;
    default:
        // use default handlers
        CLayoutTrack::x_OnIconClicked(id);
    }
}


void CSixFramesTransTrack::x_UpdateData()
{
    CDataTrack::x_UpdateData();
    TSeqRange range = m_Context->GetVisSeqRange();
    if (range.GetLength() < 3) 
        return;


    if (m_Trans == eTrans_Always  ||
        (m_Trans == eTrans_Adaptive  &&
        range.GetLength() < kTransRangeLimit)) {
            TSeqPos buffer_len = kTransRangeLimit/8;
            CRef<CSGSequenceDS> seq_ds = m_Context->GetSeqDS();
            TSeqPos max_r = seq_ds->GetSequenceLength() - 1;
            TSeqRange b_range(
                range.GetFrom() > buffer_len ? range.GetFrom() - buffer_len : 0,
                range.GetTo() + buffer_len > max_r ? max_r : range.GetTo() + buffer_len);
            if (m_TransRange.IntersectionWith(b_range) != b_range) {
                SetGroup().Clear();
                m_TransRange.Set(0, 0);
                TSeqRange trans_range = x_GetCurrentTransRange();
                m_DS->DeleteAllJobs();
                x_SetStatus(m_Option == eOpt_All ?
                            ", Translating in six-frame..." : ", Translating in three-frame..."
                            , 0);
                ENa_strand strand = (ENa_strand)m_Option;
                if (m_Option == eOpt_LeftToRight) 
                    strand = m_Context->IsFlippedStrand() ? eNa_strand_minus : eNa_strand_plus;
                m_DS->DoTranslation(trans_range, strand);
                m_TransStrand = strand;
            }
    } else {
        SetGroup().Clear();
        m_TransRange.Set(0, 0);
        ENa_strand strand = (ENa_strand)m_Option;
        if (m_Option == eOpt_LeftToRight)
            strand = m_Context->IsFlippedStrand() ? eNa_strand_minus : eNa_strand_plus;
        m_TransStrand = strand;

        CRef<CCommentConfig> c_config(new CCommentConfig);
        c_config->m_ShowBoundary = false;
        c_config->m_ShowConnection = false;
        c_config->m_Centered = true;
        c_config->m_LabelColor.Set(1.0f, 0.0f, 0.0f);
        c_config->m_LineColor.Set(1.0f, 1.0f, 1.0f);
        c_config->m_Font.SetFontFace(CGlTextureFont::eFontFace_Helvetica);
        c_config->m_Font.SetFontSize(12);

        string msg = "No translation is available with visible range > " +
            NStr::IntToString(kTransRangeLimit, NStr::fWithCommas) +
            " nucleotide bases";
        TModelUnit x = (range.GetFrom() + range.GetTo()) * 0.5;
        CRef<CCommentGlyph> label(
            new CCommentGlyph(msg, TModelPoint(x, 0.0)));
        label->SetConfig(c_config);
        Add(label.GetPointer());

        SetMsg("");
    }
}


void CSixFramesTransTrack::x_OnJobCompleted(CAppJobNotification& notify)
{
    m_DS->ClearJobID(notify.GetJobID());
    CRef<CObject> res_obj = notify.GetResult();
    CSGJobResult* result = dynamic_cast<CSGJobResult*>(&*res_obj);
    if (result) {
        x_AddTranslations(*result);
    } else {
        x_SetStatus(", failed on doing translatinon", 100);
        LOG_POST(Error << "CSixFramesTransTrack::x_OnJobCompleted() "
            "notification for job does not contain results.");
        return;
    }
}


void CSixFramesTransTrack::x_AddTranslations(const CSGJobResult& result)
{
    //TIME_ME("x_AddTranslationsLayout()");

    // update genetic code and code name
    if (m_DS->GetGeneticCode() < 0  &&  !result.m_Desc.empty()) {
        try {
            int gc_id = NStr::StringToInt(result.m_Desc);
            if (gc_id != m_DS->GetGeneticCode()) {
                m_DS->SetGeneticCode(gc_id);
                x_InitGeneticCodeName(gc_id);
                x_UpdateTitle();
            }
        } catch (CException&) {
            // ignore
        }
    }


    SetGroup().Clear();
    TObjects objs = result.m_ObjectList;

    if ( !objs.empty() ) {
        // update cached translation range
        m_TransRange = x_GetCurrentTransRange();

        NON_CONST_ITERATE (CSeqGlyph::TObjects, iter, objs) {
            CTranslationGlyph* trans = dynamic_cast<CTranslationGlyph*>(iter->GetPointer());
            trans->SetConfig(m_Config);
        }
        if (m_Option == eOpt_All  &&  objs.size() > 3) {
            // separate translation into two groups,
            // and add a shaded background for reverse translations
            CRef<CLayoutGroup> group1(new CLayoutGroup);
            Add(group1);
            group1->SetLayoutPolicy(m_Simple);
            CRef<CLayoutGroup> group2(new CLayoutGroup);
            Add(group2);
            group2->SetLayoutPolicy(m_Simple);
            group2->SetConfig(m_GroupConf.GetPointer());
            int i = 0;
            NON_CONST_ITERATE (CSeqGlyph::TObjects, iter, objs) {
                if (i < 3) {
                    group1->PushBack(*iter);
                } else {
                    group2->PushBack(*iter);
                }
                ++i;
            }
        } else {
            SetObjects(objs);
        }

        x_SetStatus("", 100);
    } else {
        x_SetStatus(", failed on doing translatinon", 100);
    }

    x_UpdateLayout();
}


void CSixFramesTransTrack::x_OnContentIconClicked()
{
    wxMenu menu;
    UseDefaultMarginWidth(menu);

    vector<pair<EStrandOption, wxString> > options =
    {
        { eOpt_Forward, wxT("Forward translations") },
        { eOpt_Reverse, wxT("Reverse translations") },
        { eOpt_LeftToRight, wxT("Left-to-right translations") },
        { eOpt_All, wxT("Show all") }
    };
    for (auto&& opt : options) {
        wxMenuItem* item;
        item = menu.AppendRadioItem(kContentBaseID + opt.first, opt.second);
        item->Check(m_Option == opt.first);
    }

    m_LTHost->LTH_PopupMenu(&menu);
    wxMenuItemList& item_list = menu.GetMenuItems();
    ITERATE (wxMenuItemList, iter, item_list) {
        EStrandOption option = (EStrandOption)((*iter)->GetId() - kContentBaseID);
        if ((*iter)->IsChecked()  &&  option != m_Option) {
            m_Option = option;
            // force to update data
            m_TransRange.Set(0, 0);
            x_UpdateTitle();
            x_UpdateData();
        }
    }
}


void CSixFramesTransTrack::x_OnSettingIconClicked()
{
    wxMenu menu;
    UseDefaultMarginWidth(menu);
    
    wxMenuItem* item = NULL;

    // add menu item for Codon setting
    size_t radio_item_base = 0;
    item = menu.AppendCheckItem(kContentBaseID + radio_item_base++, wxT("Highlight Codons"));
    item->Check(m_Config->GetHighlightCodons());
    item = menu.AppendCheckItem(kContentBaseID + radio_item_base++, wxT("Enable alternative Start"));
    item->Check(m_DS->GetAltStart());

    // add a separator
    menu.AppendSeparator();

    // add menu items for ORF threshold setting
    typedef vector< pair<int, string> > TItems;
    TItems items;
    items.push_back(TItems::value_type(0, string("Highlight all ORFs")));
    items.push_back(TItems::value_type(20, string("Highlight ORFs (>= 20 Codons)")));
    items.push_back(TItems::value_type(100, string("Highlight ORFs (>= 100 Codons)")));
    items.push_back(TItems::value_type(250, string("Highlight ORFs (>= 250 Codons)")));
    items.push_back(TItems::value_type(kMaxOrfLen, string("Don't highlight ORFs")));

    for (size_t i = 0; i < items.size(); ++i) {
        item = menu.AppendRadioItem(kContentBaseID + radio_item_base + i,
            ToWxString(items[i].second));
        item->Check(m_Config->GetOrfThreshold() == items[i].first);
    }

    m_LTHost->LTH_PopupMenu(&menu);
    wxMenuItemList& item_list = menu.GetMenuItems();
    ITERATE (wxMenuItemList, iter, item_list) {
        int id = (*iter)->GetId() - kContentBaseID;
        bool checked = (*iter)->IsChecked();
        if (id == 0) {
            if (checked != m_Config->GetHighlightCodons()) {
                m_Config->SetHighlightCodons(checked);
                break;
            }
        } else if (id == 1) {
            if (checked != m_DS->GetAltStart()) {
                m_DS->SetAltStart(checked);
                // force to update data
                m_TransRange.Set(0, 0);
                x_UpdateData();
                break;
            }
        } else if (checked) {
            id -= radio_item_base;
            if (m_Config->GetOrfThreshold() != items[id].first) {
                m_Config->SetOrfThreshold(items[id].first);
                x_UpdateLayout();
                break;
            }
        }
    }
}


void CSixFramesTransTrack::x_OnGeneticIconClicked()
{
    int curr_gc = m_DS->GetGeneticCode();
    if (curr_gc < 0) {
        return;
    }

    wxMenu menu;
    UseDefaultMarginWidth(menu);
    
    wxMenuItem* item = NULL;
    // add menu items for ORF threshold setting
    typedef vector< pair<int, string> > TItems;
    TItems items;
    const CGenetic_code_table::Tdata& codes = CGen_code_table::GetCodeTable().Get();
    ITERATE (CGenetic_code_table::Tdata, code, codes) {
        items.push_back(TItems::value_type((*code)->GetId(), (*code)->GetName()));
    }

    for (size_t i = 0; i < items.size(); ++i) {
        item = menu.AppendRadioItem(kContentBaseID + i, ToWxString(items[i].second));
        if (items[i].first == curr_gc) {
            item->Check(true);
        }
    }

    m_LTHost->LTH_PopupMenu(&menu);
    wxMenuItemList& item_list = menu.GetMenuItems();
    ITERATE (wxMenuItemList, iter, item_list) {
        int id = (*iter)->GetId() - kContentBaseID;
        bool checked = (*iter)->IsChecked();
        if (checked) {
            if (curr_gc != items[id].first) {
                m_DS->SetGeneticCode(items[id].first);
                m_GCName = items[id].second;

                // force to update data
                m_TransRange.Set(0, 0);
                x_UpdateTitle();
                x_UpdateData();
            }
            break;
        }
    }
}


void CSixFramesTransTrack::x_UpdateTitle()
{
    ENa_strand strand = (ENa_strand)m_Option;
    if (m_Option == eOpt_LeftToRight) {
        strand = m_Context->IsFlippedStrand() ? eNa_strand_minus : eNa_strand_plus;
    }
    string frames_b = "Three";
    string frames_a;
    switch (strand) {
        case eNa_strand_plus:
            frames_a = "+1, +2, +3)";
            break;
        case eNa_strand_both:
            frames_b = "Six";
            frames_a = "+1, +2, +3, -1, -2, -3)";
            break;
        case eNa_strand_minus:
            frames_a = "-1, -2, -3)";
            break;
        default:
            break;
    }

    string title = frames_b;
    title += "-frame translations (top -> bottom: ";
    title += frames_a;

    if (!m_GCName.empty()) {
        title += ", Genetic code: ";
        title += m_GCName;
        title += " (" + NStr::IntToString(m_DS->GetGeneticCode()) + ")";
    }

    SetTitle(title);
}


void CSixFramesTransTrack::x_InitGeneticCodeName(int gc_id)
{
    // get genetic code name
    const CGenetic_code_table& code_table = CGen_code_table::GetCodeTable();
    const CGenetic_code_table::Tdata& codes = code_table.Get();
    ITERATE (CGenetic_code_table::Tdata, code, codes) {
        if ((*code)->GetId() == gc_id) {
            SetGCName((*code)->GetName());;
            break;
        }
    }
}


TSeqRange CSixFramesTransTrack::x_GetCurrentTransRange() const
{
    TSeqRange range;
    CRef<CSGSequenceDS> seq_ds = m_Context->GetSeqDS();
    TSeqPos max_r = seq_ds->GetSequenceLength() - 1;
    TSeqRange vis_range = m_Context->GetVisSeqRange();
    TSeqPos center = (vis_range.GetFrom() + vis_range.GetTo()) / 2;
    range.SetFrom(center > kTransRangeLimit ? center - kTransRangeLimit : 0);
    range.SetTo(center + kTransRangeLimit > max_r ? max_r : center + kTransRangeLimit);
    return range;
}


///////////////////////////////////////////////////////////////////////////////
/// CSixFramesTransTrackFactory
///
ILayoutTrackFactory::TTrackMap
CSixFramesTransTrackFactory::CreateTracks(SConstScopedObject& object,
                                          ISGDataSourceContext* ds_context,
                                          CRenderingContext* r_cntx,
                                          const SExtraParams& params,
                                          const TAnnotMetaDataList& /*src_annots*/) const
{
    // create data source
    CIRef<ISGDataSource> ds = ds_context->GetDS(
        typeid(CSFTransDSType).name(), object);
    CSFTransDS* ds_pointer = dynamic_cast<CSFTransDS*>(ds.GetPointer());
    ds_pointer->SetDepth(params.m_Level);

    TTrackMap tracks;

    // make sure we don't create six-frame translation track for protein sequence.
    CRef<CSGSequenceDS> seq_ds = r_cntx->GetSeqDS();
    CSeq_id_Handle idh = seq_ds->GetBestIdHandle();
    CSeq_id::EAccessionInfo info = idh.GetSeqId()->IdentifyAccession();
    if (info & CSeq_id::fAcc_prot) {
        return tracks;
    }

    // create track
    CRef<CSixFramesTransTrack> track(new CSixFramesTransTrack(ds_pointer, r_cntx));
    tracks[kTrackTitle] = track.GetPointer();

    return tracks;
}


CRef<CTrackConfigSet>
CSixFramesTransTrackFactory::GetSettings(const string& profile,
                                         const TKeyValuePairs& settings,
                                         const CTempTrackProxy* /*track_proxy*/) const
{
    CRef<CTrackConfigSet> config_set(new CTrackConfigSet);
    // create a track configure
    CRef<CTrackConfig> config(new CTrackConfig);
    config_set->Set().push_back(config);
    config->SetLegend_text("anchor_10");

    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryReadView view =
        CSGConfigUtils::GetReadView(registry, kBaseKey, profile, kDefProfile);
    string option = view.GetString("ShowOption");
    int orf_threshold = view.GetInt("OrfThreshold", 20);
    bool hl_codons = view.GetBool("HighlightCodons", true);
    bool alt_start = false;

    ITERATE (TKeyValuePairs, iter, settings) {
        try {
            if (NStr::EqualNocase(iter->first, "OrfThreshold")) {
                orf_threshold = NStr::StringToInt(iter->second);
                if (orf_threshold != 0  &&  orf_threshold != 20  &&
                    orf_threshold != 100  &&  orf_threshold != 250  &&
                    orf_threshold != kMaxOrfLen) {
                        orf_threshold = 20;
                }
            } else if (NStr::EqualNocase(iter->first, "ShowOption")) {
                option = iter->second;
            } else if (NStr::EqualNocase(iter->first, "HighlightCodons")) {
                hl_codons = NStr::StringToBool(iter->second);
            } else if (NStr::EqualNocase(iter->first, "AltStart")) {
                alt_start = NStr::StringToBool(iter->second);
            }
        } catch (CException&) {
            LOG_POST(Warning << "CSixFramesTransTrack::x_LoadSettings() "
                "invalude translation setting:" << iter->second);
        }
    }

    string help = "Six-frame translations<br>";
    switch (CSixFramesTransTrack::OptionStrToValue(option)) {
        case CSixFramesTransTrack::eOpt_Forward:
            help += "From top to bottom: forward translation +1, +2 and +3<br>";
            break;
        case CSixFramesTransTrack::eOpt_Reverse:
            help += "From top to bottom: reverse translation -1, -2 and -3<br>";
            break;
        case CSixFramesTransTrack::eOpt_LeftToRight:
            help += "From top to bottom: forward translation +1, +2 and +3 or reverse translation -1, -2 and -3 based on current orientaion<br>";
            break;
        case CSixFramesTransTrack::eOpt_All:
            help += "From top to bottom: forward translation +1, +2 and +3, and reverse translation -1, -2 and -3<br>";
        default:
            break;
    }

    help += "Regions highlighted with Green are start codons<br>";
    help += "Regions highlighted with Red are stop codons<br>";
    help += "Regions highlighted with Gray are ORFs<br>";
    config->SetHelp() = help;

    CRef<CChoice> choice = CTrackConfigUtils::CreateChoice(
        "ShowOption", "Translation strand", option,
        "Translation direction option");
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            CSixFramesTransTrack::OptionValueToStr(CSixFramesTransTrack::eOpt_Forward),
            "Forward translations",
            "Show forward direction translations (+1, +2, +3)",
            ""));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            CSixFramesTransTrack::OptionValueToStr(CSixFramesTransTrack::eOpt_Reverse),
            "Reverse translations",
            "Show reverse direction translations (-1, -2, -3)",
            ""));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
        CSixFramesTransTrack::OptionValueToStr(CSixFramesTransTrack::eOpt_LeftToRight),
        "Left-to-right translations",
        "Show left-to-right translations",
        ""));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            CSixFramesTransTrack::OptionValueToStr(CSixFramesTransTrack::eOpt_All),
            "Show all",
            "Show all six-frame translations",
            ""));
    config->SetChoice_list().push_back(choice);

    CRef<CChoice> orf_choice = CTrackConfigUtils::CreateChoice(
        "OrfThreshold", "Highlight ORFs", NStr::IntToString(orf_threshold),
        "Setting for highlighting ORFs based on number of codons"
        "");

    orf_choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            "0", "Highlight all ORFs", "Highlight all ORFs", ""));

    orf_choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            "20", "Highlight ORFs (>= 20 codons)",
            "Highlight ORFs equal or longer than 20 codons", ""));

    orf_choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            "100", "Highlight ORFs (>= 100 codons)",
            "Highlight ORFs equal or longer than 100 codons", ""));

    orf_choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            "250", "Highlight ORFs (>= 250 codons)",
            "Highlight ORFs equal or longer than 250 codons", ""));

    orf_choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
        NStr::IntToString(kMaxOrfLen), "Don't highlight ORFs",
            "Do not highlight ORFs", ""));

    config->SetChoice_list().push_back(orf_choice);

    config->SetCheck_boxes().push_back(
        CTrackConfigUtils::CreateCheckBox(
            "HighlightCodons","Highlight codons", 
            "Highlight codon", "",
            hl_codons));

    config->SetCheck_boxes().push_back(
        CTrackConfigUtils::CreateCheckBox(
            "AltStart","Enable alternative start", 
            "Enable alternative initiation condon", "",
            alt_start));

    return config_set;
}


void CSixFramesTransTrackFactory::RegisterIconImages() const
{
    CLayoutTrack::RegisterIconImage("track_genetic_code", "track_genetic_code.png");
}


string CSixFramesTransTrackFactory::GetExtensionIdentifier() const
{
    return CSixFramesTransTrack::m_TypeInfo.GetId();
}


string CSixFramesTransTrackFactory::GetExtensionLabel() const
{
    return CSixFramesTransTrack::m_TypeInfo.GetDescr();
}


END_NCBI_SCOPE
