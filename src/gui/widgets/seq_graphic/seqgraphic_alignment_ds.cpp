/*  $Id: seqgraphic_alignment_ds.cpp 43001 2019-05-03 19:55:12Z shkeda $
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
#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_alignment_ds.hpp>
#include <gui/widgets/seq_graphic/mate_pair_glyph.hpp>
#include <gui/widgets/seq_graphic/alignment_cigar_score.hpp>
#include <gui/widgets/seq_graphic/alignment_quality_score.hpp>
#include <gui/widgets/seq_graphic/alignment_other_score.hpp>
#include <gui/widgets/seq_graphic/sparse_graph.hpp>
#include <gui/widgets/wx/ui_tool_registry.hpp>
#include <objmgr/data_loader.hpp>
#include <objmgr/graph_ci.hpp>

#include <sra/data_loaders/bam/bamloader.hpp>
#include <sra/data_loaders/csra/csraloader.hpp>

#include <util/checksum.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/widgets/wx/wx_utils.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
/// CSGAlignmentDS
///////////////////////////////////////////////////////////////////////////////

/// number of gradient colors in alignment
static const int kColorGradNumber = 64;

CSGAlignmentDS::CSGAlignmentDS(CScope& scope, const CSeq_id& id)
    : CSGGenBankDS(scope, id)
    , m_AlnType(IAlnExplorer::fInvalid)
    , m_Enabled(false)
    , m_LinkMatePairs(true)
    , m_HasCoverageGraph(false)
    , m_HasQualityMap(false)
    , m_ScoreToken(kInvalidJobToken)
    , m_ScoringJobID(-1)
    , m_HideSra(CAlignmentConfig::eHide_None)
    , m_UnalignedTailsMode(CAlignmentConfig::eTails_ShowGlyph)
    , m_MaxStartTail(0)
    , m_MaxEndTail(0)
{
}

CSGAlignmentDS::~CSGAlignmentDS()
{
    // gbench: always release memory 
    // sviewer: release memory if there is a need
    if (!m_CgiMode || NeedReleaseMemory())
        ReleaseMemory();
}

void CSGAlignmentDS::SetDefaultScoringMethods(string dna_method,
                                              string protein_method)
{
    m_DNAMethod = dna_method;
    m_ProtMethod = protein_method;
}


void CSGAlignmentDS::x_GetGraphCacheKey(string& data_key) const
{
    string remote_path = m_RemotePath.empty() ? m_AnnotName : m_RemotePath;
    data_key = CPileUpGraph::CreateCacheKey(m_Handle.GetScope(), *m_Handle.GetSeqId(), remote_path);
}


double CSGAlignmentDS::GetGraphCost(const TSeqRange& range) const
{

    if (range.GetLength() == 0)
        return 0;

    vector<TSeqRange> missing_ranges;
    missing_ranges.emplace_back(range.GetFrom(), range.GetTo());
    if (m_HasPileUpCache) {
        string cache_key;
        x_GetGraphCacheKey(cache_key);
        auto data = CGraphCache<CPileUpGraph>::GetInstance().GetData(cache_key);
        if (data) {
            missing_ranges.clear();
            data->GetMissingRegions(range, missing_ranges);
        } 
    } 
    
    double cost_in_sec = 0.;
    if (!missing_ranges.empty()) {
        SAnnotSelector graph_sel =
            CSeqUtils::GetAnnotSelector(CSeq_annot::C_Data::e_Graph);
        CSeqUtils::SetAnnot(graph_sel, m_AnnotName + " pileup graphs");
        CSeqUtils::SetResolveDepth(graph_sel, m_Adaptive, m_Depth);
        graph_sel.SetCollectCostOfLoading();
        for (auto& r : missing_ranges) {
            CGraph_CI graph_iter(m_Handle, r, graph_sel);
            auto cost = graph_iter.GetCostOfLoadingInSeconds();
            cost_in_sec = max(cost_in_sec, cost);
        }
    } 
    return cost_in_sec;
}

bool CSGAlignmentDS::IsGraphCached(const TSeqRange& range) const
{
    if (!m_HasPileUpCache)
        return false;
    
    if (range.GetLength() == 0)
        return false;
    
    string cache_key;
    x_GetGraphCacheKey(cache_key);
    auto data = CGraphCache<CPileUpGraph>::GetInstance().GetCachedData(cache_key);
    vector<TSeqRange> missing_ranges;
    if (data) {
        data->GetMissingRegions(range, missing_ranges);
    } else {
        missing_ranges.emplace_back(range.GetFrom(), range.GetTo());
    }
    int len = 0;
    for (auto& r : missing_ranges) {
        len += r.GetLength();
    }
    return len <= 40000;
}

double CSGAlignmentDS::GetAlignmentCost(const TSeqRange& range) const
{
    double cost_in_sec = 0.;
    SAnnotSelector aln_sel =
        CSeqUtils::GetAnnotSelector(CSeq_annot::C_Data::e_Align);
    CSeqUtils::SetAnnot(aln_sel, m_AnnotName);
    CSeqUtils::SetResolveDepth(aln_sel, m_Adaptive, m_Depth);
    aln_sel.SetCollectCostOfLoading();
    CAlign_CI align_iter(m_Handle, range, aln_sel);
    cost_in_sec = align_iter.GetCostOfLoadingInSeconds();
    return cost_in_sec;
}

string s_fromatstr(Uint8 bytes)
{
    static const double log_1024 = log(1024);
    if (bytes < 1024)
        return NStr::NumericToString(bytes) + " B";
    int exp = (int)(log(bytes) / log_1024);
    string suffix = "KMGTPE";
    suffix = suffix.substr(exp - 1, 1);
    return  ToStdString(wxString::Format(_T("%.1f %sB"), bytes / pow(1024, exp), suffix.c_str()));
}

bool CSGAlignmentDS::NeedReleaseMemory() const
{
    if (m_HasCoverageGraph == false || !m_DataHandle)
        return false;
    auto used_memory = m_DataHandle.GetUsedMemory();
    return used_memory > m_MemoryLimit;
}


void CSGAlignmentDS::ReleaseMemory()
{
    if (m_HasCoverageGraph == false || !m_DataHandle)
        return;
    try {
        DeleteAllJobs();
        m_AlnScoreMap.clear();
        auto used_memory = m_DataHandle.GetUsedMemory();
        GetScope().RemoveFromHistory(m_DataHandle, CScope::eThrowIfLocked);// CScope::eKeepIfLocked)
        if (used_memory > m_MemoryLimit)
            ERR_POST(Info << "Alignments high memory usage: " << s_fromatstr(used_memory) << " released");
    } catch (CException& /*e*/) {
        // Ignore errors  
        //ERR_POST(Error << e.GetMsg());
    }

}

void CSGAlignmentDS::LoadCoverageGraph(const TSeqRange& range,
                                       TModelUnit window,
                                       TJobToken token)
{
    _ASSERT(m_HasCoverageGraph);

    CRef<CSGAlignmentJob> job;
    SAnnotSelector graph_sel =
        CSeqUtils::GetAnnotSelector(CSeq_annot::C_Data::e_Graph);
    CSeqUtils::SetResolveDepth(graph_sel, true, m_Depth);
    CSeqUtils::SetAnnot(graph_sel, m_AnnotName);

    job.Reset(new CSGAlignmentJob("Alignment coverage graph",
        m_Handle, graph_sel, range, window, m_MaxStartTail, m_MaxEndTail));

    if (job) {
        job->SetToken(token);
        job->SetHideSra(m_HideSra);
        job->SetUnalignedTailsMode(m_UnalignedTailsMode);
        x_LaunchJob(*job);
    }
}


void CSGAlignmentDS::LoadAlignments(const TSeqRange& range,
                                    TModelUnit window,
                                    int align_limit,
                                    bool smear_if_overlimit,
                                    TJobToken token)
{
    CRef<CSGAlignmentJob> job;

    SAnnotSelector sel =
        CSeqUtils::GetAnnotSelector(CSeq_annot::C_Data::e_Align);
    CSeqUtils::SetAnnot(sel, m_AnnotName);
    CSeqUtils::SetResolveDepth(sel, m_Adaptive, m_Depth);

    CSGAlignmentJob* aln_job =
        new CSGAlignmentJob("Alignment", m_Handle, sel, range, window, align_limit,
        smear_if_overlimit, m_LinkMatePairs, m_MaxStartTail, m_MaxEndTail);
    job.Reset(aln_job);
    aln_job->SetSortBy(m_SortBy);

    if (job) {
        job->SetToken(token);
        job->SetHideSra(m_HideSra);
        job->SetUnalignedTailsMode(m_UnalignedTailsMode);
        job->SetLayoutPolicy(m_LayoutPolicy);
        x_LaunchJob(*job);
    }
}

/*
void CSGAlignmentDS::LoadAlignments(const TSeqRange& range,
                                    TModelUnit window, int smear_cutoff,
                                    TJobToken token)
{
    CRef<CSGAnnotJob> job;

    SAnnotSelector sel =
        CSeqUtils::GetAnnotSelector(CSeq_annot::C_Data::e_Align);
    CSeqUtils::SetAnnot(sel, m_AnnotName);
    CSeqUtils::SetResolveDepth(sel, m_Adaptive, m_Depth);

    if (m_HasCoverageGraph) {
        SAnnotSelector graph_sel =
            CSeqUtils::GetAnnotSelector(CSeq_annot::C_Data::e_Graph);
        CSeqUtils::SetResolveDepth(graph_sel, true, m_Depth);
        CSeqUtils::SetAnnot(graph_sel, m_AnnotName);

        job.Reset(new CSGAlignmentJob("Alignment", m_Handle, sel, graph_sel, range,
            window, smear_cutoff, m_LinkMatePairs, true));
    } else {
        job.Reset(new CSGAlignmentJob("Alignment", m_Handle, sel, sel, range,
            window, smear_cutoff, m_LinkMatePairs, false));
    }

    if (job) {
        job->SetToken(token);
        x_LaunchJob(*job);
    }
}
*/

bool CSGAlignmentDS::CanShowRange(const TSeqRange& range, int align_limit)
{
    if (!m_HasPileUpCache)
        return true;
    bool res = true;
    string cache_key;
    x_GetGraphCacheKey(cache_key);
    auto data = CGraphCache<CPileUpGraph>::GetInstance().GetData(cache_key);
    if (data) {
        auto num_reads = data->GetNumberOfReads(range);
        if (num_reads > 0)
            res = num_reads < align_limit;
    }
    return res;
}


bool CSGAlignmentDS::IsRangeCached(const TSeqRange& range) const
{
    if (!m_HasPileUpCache)
        return false;
    string cache_key;
    x_GetGraphCacheKey(cache_key);
    auto data = CGraphCache<CPileUpGraph>::GetInstance().GetData(cache_key);
    if (!data)
        return false;

    vector<TSeqRange> missing_ranges;
    data->GetMissingRegions(range, missing_ranges);
    if (missing_ranges.empty())
        return true;

    return false;
}


void CSGAlignmentDS::CalcAlnStat(const TAlnMgrVec& aligns,
                                 const TSeqRange& range,
                                 TModelUnit window,
                                 TJobToken token)
{

    SAnnotSelector aln_sel =
        CSeqUtils::GetAnnotSelector(CSeq_annot::C_Data::e_Align);
    CSeqUtils::SetAnnot(aln_sel, m_AnnotName);
    CSeqUtils::SetResolveDepth(aln_sel, m_Adaptive, m_Depth);

    SAnnotSelector graph_sel =
        CSeqUtils::GetAnnotSelector(CSeq_annot::C_Data::e_Graph);
    CSeqUtils::SetAnnot(graph_sel, m_AnnotName + " pileup graphs");
    CSeqUtils::SetResolveDepth(graph_sel, m_Adaptive, m_Depth);

    string cache_key;
    
    if (m_HasPileUpCache)
        x_GetGraphCacheKey(cache_key);

    CRef<CSGAlignStatJob> job(new CSGAlignStatJob(
        "pileup graph", m_Handle, aln_sel, graph_sel, range, aligns, window, cache_key));
    job->SetToken(token);
    x_LaunchJob(*job);
}


void CSGAlignmentDS::LoadAlignFeats(const TSeqRange& range,
                                    TModelUnit window,
                                    vector< CRef<CAlignGlyph> > aligns,
                                    const vector<int>& projected_feats,
                                    TJobToken token)
{
    SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
    ITERATE (vector<int>, iter, projected_feats) {
        if (*iter < CSeqFeatData::eSubtype_max  &&
            *iter > CSeqFeatData::eSubtype_bad) {
            sel.IncludeFeatSubtype(CSeqFeatData::ESubtype(*iter));
        }
    }
    CRef<CSGAlignmentJob> job(new CSGAlignmentJob("AlignmentFeature", m_Handle,
        sel, range, window, aligns, m_MaxStartTail, m_MaxEndTail));
    job->SetToken(token);
    job->SetHideSra(m_HideSra);
    job->SetUnalignedTailsMode(m_UnalignedTailsMode);
    x_LaunchJob(*job);
}


IAlnExplorer::EAlignType CSGAlignmentDS::GetAlignType(
    const CSeqGlyph* obj) const
{
    IAlnExplorer::EAlignType alnType = IAlnExplorer::fInvalid;
    try {
        if (const CAlignGlyph* aln =
            dynamic_cast<const CAlignGlyph*>(obj)) {
                alnType = aln->GetAlignMgr().GetAlignType();
        } else if (const CMatePairGlyph* mp =
            dynamic_cast<const CMatePairGlyph*>(obj)){
                const CAlignGlyph* aln = mp->GetSeqAligns().front();
                alnType = aln->GetAlignMgr().GetAlignType();
        }
    } catch (CException& e) {
        LOG_POST(Warning << "Can't determine alignment type due to an error: "
            << e.GetMsg());    }

    return alnType;
}


IAlnExplorer::EAlignType CSGAlignmentDS::InitAlignType(bool isFastConfig)
{
    if (m_AlnType == IAlnExplorer::fInvalid) {
        vector<string> annots;
        annots.push_back(m_AnnotName);
        SAnnotSelector sel(CSeqUtils::GetAnnotSelector(annots, m_Adaptive, m_Depth));
        m_AlnType = CSGAlignmentJob::GetAlignType(m_Handle, sel, m_HasQualityMap, isFastConfig);
        x_InitScoringMethods(m_AlnType);
    }

    return m_AlnType;
}

const string& CSGAlignmentDS::GetDataLoader() const 
{
    if (m_DataLoader == "none") {
        m_DataLoader.clear();
        vector<string> annots;
        annots.push_back(m_AnnotName);
        SAnnotSelector sel(CSeqUtils::GetAnnotSelector(annots, m_Adaptive, m_Depth));
        sel.SetMaxSize(1);
        sel.SetCollectNames(false);
        CAlign_CI aln_iter(m_Handle, TSeqRange::GetWhole(), sel);
        if (aln_iter.GetSize()) {
            CDataLoader* loader = aln_iter.GetAnnot().GetTSE_Handle().GetDataLoader();
            if (loader) {
                if (dynamic_cast<CBAMDataLoader*>(loader))
                    m_DataLoader = kBamDataLoader;
                else if (dynamic_cast<CCSRADataLoader*>(loader))
                    m_DataLoader = kCSRADataLoader;
                else
                    m_DataLoader = loader->GetName();
            }
        }
    }
    return m_DataLoader;
}

class CAlnScoringMethodSelector : public CUIToolRegistry::ISelector
{
public:
    virtual bool Select(const IUITool& tool)
    {
        return dynamic_cast<const IScoringMethod*>(&tool) != NULL;
    }
};


const CSGAlignmentDS::TMethods&
CSGAlignmentDS::GetScoringMethods(IAlnExplorer::EAlignType aln_type)
{
    x_InitScoringMethods(aln_type);

    if (aln_type == IAlnExplorer::fMixed  ||
        aln_type == IAlnExplorer::fProtein) {
        return m_ProteinMethods;
    }

    return m_DNAMethods;
}


bool CSGAlignmentDS::IsValidProteinScoringMethod(const string& name)
{
    if (m_ProteinMethods.empty()) {
        x_InitScoringMethods(IAlnExplorer::fProtein);
    }

    ITERATE (TMethods, it, m_ProteinMethods)   {
        CIRef<IScoringMethod> tmp_meth = *it;
        if (tmp_meth->GetName() == name)   {
            return true;
        }
    }
    return false;
}


bool CSGAlignmentDS::IsValidDNAScoringMethod(const string& name)
{
    if (m_HasQualityMap  &&  name == CSGAlnQualityScore::GetScoreName()) {
        return true;
    }

    if (m_DNAMethods.empty()) {
        x_InitScoringMethods(IAlnExplorer::fDNA);
    }

    ITERATE (TMethods, it, m_DNAMethods)   {
        CIRef<IScoringMethod> tmp_meth = *it;
        if (tmp_meth->GetName() == name)   {
            return true;
        }
    }
    return false;
}


////////////////////////////////////////////////////////////////
void CSGAlignmentDS::InitAlignScore(CAlignGlyph* aln)
{
    if (aln->GetAlignMgr().GetBaseWidth(0) != (TSeqPos)-1) {
        const TAlnAnchorPair& align_pair = aln->GetAlnAnchorPair();
        IAlnExplorer::EAlignType sc_type = GetAlignType(aln);
        if (sc_type == IAlnExplorer::fDNA  &&
            m_DNAMethod == CSGAlnQualityScore::GetScoreName()  &&
            aln->GetAlignMgr().GetNumRows() == 2) {
            InitAlignQualityScore(aln, align_pair, sc_type);
        } else {
            InitAlignScoreCache(aln, align_pair, sc_type);
        }
    }
}


void CSGAlignmentDS::InitAlignQualityScore(CAlignGlyph* aln,
                                           const TAlnAnchorPair& align_pair,
                                           IAlnExplorer::EAlignType sc_type)
{
    TAlnScoreMap::iterator iter = m_AlnScoreMap.find(align_pair);
    if (iter != m_AlnScoreMap.end()  &&  iter->second->HasScores() ) {
        aln->SetScore(*iter->second);
    } else {
        const IAlnGraphicDataSource& aln_mgr = aln->GetAlignMgr();
        CIRef<ISGAlnScore> cache(
            new CSGAlnQualityScore(aln_mgr.GetAnchor(), aln->GetAlignment(), GetScope()));
        aln->SetScore(*cache);
        m_AlnScoreMap[align_pair] = cache;
    }
}


void CSGAlignmentDS::InitAlignScoreCache(CAlignGlyph* aln,
                                         const TAlnAnchorPair& align_pair,
                                         IAlnExplorer::EAlignType sc_type)
{
    CRef<CScoreCache> cache;
    TAlnScoreMap::iterator iter = m_AlnScoreMap.find(align_pair);
    if (iter != m_AlnScoreMap.end()  &&  iter->second->HasScores() ) {
        aln->SetScore(*iter->second);
        CSGAlnOtherScore* other_score =
            dynamic_cast<CSGAlnOtherScore*>(iter->second.GetPointer());
        _ASSERT(other_score);
        cache.Reset(&other_score->GetScoreCache());
    } else {
        // add to map
        const IAlnGraphicDataSource& aln_mgr = aln->GetAlignMgr();
        cache.Reset(new CScoreCache());
        cache->EnableBackgoundProcessing(false);
        cache->SetListener(NULL);
        cache->SetGradNumber(kColorGradNumber);
        CIRef<ISGAlnScore> score(new CSGAlnOtherScore(*cache));

        //CRef<CScoreAlignmentDS> sa_ds(new CScoreAlignmentDS(aln_mgr) );
        cache->SetAlignment(&aln_mgr);

        m_AlnScoreMap[align_pair] = score;
        aln->SetScore(*score);
    }

    x_InitScoringMethods(sc_type);

    TMethods* methods = NULL;
    string name = kEmptyStr;
    if (sc_type == IAlnExplorer::fDNA  &&  m_DNAMethod != "") {
        methods = &m_DNAMethods;
        name = m_DNAMethod;
    } else if ((sc_type == IAlnExplorer::fProtein  ||
        sc_type == IAlnExplorer::fMixed)  &&  m_ProtMethod != "") {
            methods = &m_ProteinMethods;
            name = m_ProtMethod;
    }

    if (methods) {
        IScoringMethod* method = NULL;
        ITERATE (TMethods, it, *methods)   {
            CIRef<IScoringMethod> tmp_meth = *it;
            if (tmp_meth->GetName() == name)   { // found it
                method = tmp_meth.GetPointer();
                break;
            }
        }

        if (method) {
            if (cache->GetScoringMethod() != method) {
                if (cache->HasScores()) {
                    cache->ResetScores();
                }
                cache->SetScoringMethod(method);
            }
        }
    }
}


void CSGAlignmentDS::CalculateAlignmentScore(CSeqGlyph::TObjects& objs,
                                             TJobToken token)
{
    if (objs.empty()) return;

    if (x_IsScoringJobRunning()) {
        x_DeleteScoringJob();
    } else {
        ResetAlnScoringJob();
    }

    CAlignGlyph* first_aln = dynamic_cast<CAlignGlyph*>(objs.front().GetPointer());
    if ( !first_aln ) {
        CMatePairGlyph* first_mp = dynamic_cast<CMatePairGlyph*>(objs.front().GetPointer());
        first_aln = first_mp->SetSeqAligns().front().GetPointer();
    }

    if (GetDNAScoringMethod() == "Show Differences"  &&  first_aln  &&
        first_aln->GetAlignMgr().IsCigarUnambiguous()) {
        m_AlnScoreMap.clear();
        NON_CONST_ITERATE (CSeqGlyph::TObjects, iter, objs) {
            CAlignGlyph* aln = dynamic_cast<CAlignGlyph*>(iter->GetPointer());
            CMatePairGlyph* mp = dynamic_cast<CMatePairGlyph*>(iter->GetPointer());
            if (aln) {
                CConstIRef<ISGAlnScore> cigar_score(new CAlignCigarScore(aln->GetAlignMgr()));
                aln->SetScore(*cigar_score);
            } else if (mp) {
                CMatePairGlyph::TAlignList& aligns = mp->SetSeqAligns();
                NON_CONST_ITERATE(CMatePairGlyph::TAlignList, aln_iter, aligns) {
                    CConstIRef<ISGAlnScore> cigar_score(new CAlignCigarScore(aln->GetAlignMgr()));
                    aln->SetScore(*cigar_score);
                }
            }
        }
    } else {
        if ( !m_AlnScoreMap.empty() ) {
            const string& used_method = m_AlnScoreMap.begin()->second->GetScoringMethod();
            if ((GetDNAScoringMethod() == CSGAlnQualityScore::GetScoreName()  ||
                 used_method == CSGAlnQualityScore::GetScoreName())  &&
                 GetDNAScoringMethod() != used_method) {
                     m_AlnScoreMap.clear();
            }
        }

        // Remove all the unfinished score caches.
        // Maybe some of unfinished alignments (score) are not visible in the
        // current viewed range. To avoid calculating scores for invisible alignments
        // and long alignment score calculating time, remove the unfinished ones. It
        // is possible some of the removed ones are in current visible range. Those will
        // be added latter
        TAlnScoreMap temp_map;
        NON_CONST_ITERATE (TAlnScoreMap, iter, m_AlnScoreMap) {
            if (iter->second->HasScores()) {
                temp_map.insert(TAlnScoreMap::value_type(iter->first, iter->second));
            }
        }
        m_AlnScoreMap.swap(temp_map);

        NON_CONST_ITERATE (CSeqGlyph::TObjects, iter, objs) {
            CAlignGlyph* aln = dynamic_cast<CAlignGlyph*>(iter->GetPointer());
            CMatePairGlyph* mp = dynamic_cast<CMatePairGlyph*>(iter->GetPointer());
            if (aln) {
                InitAlignScore(aln);
            } else if (mp) {
                CMatePairGlyph::TAlignList& aligns = mp->SetSeqAligns();
                NON_CONST_ITERATE(CMatePairGlyph::TAlignList, aln_iter, aligns) {
                    InitAlignScore(aln_iter->GetPointer());
                }
            }
        }

        //// fire the scoring job
        m_ScoringJob = new CSGAlignScoringJob("Align scoring", m_AlnScoreMap);
        m_ScoringJob->SetToken(token);
        m_ScoringJobID = x_LaunchJob(*m_ScoringJob);

    }
}


void CSGAlignmentDS::ResetAlnScoringJob()
{
    if (m_ScoringJobID > -1 || m_ScoringJob) {
        CSGGenBankDS::ClearJobID(m_ScoringJobID);
        m_ScoringJobID = -1;
        m_ScoringJob.Reset();
    }
}


void CSGAlignmentDS::ClearJobID(TJobID job_id)
{
    if (job_id == m_ScoringJobID) {
        ResetAlnScoringJob();
    }
    CSGGenBankDS::ClearJobID(job_id);
}


void CSGAlignmentDS::DeleteAllJobs()
{
    if (m_ScoringJobID > -1 && m_ScoringJob) {
        x_DeleteScoringJob();
    }
    CSGGenBankDS::DeleteAllJobs();
}


bool CSGAlignmentDS::x_IsScoringJobRunning() const
{
    // make sure it has not been deleted
    return IsJobNeeded(m_ScoringJobID)  &&
        m_ScoringJob.GetPointer() != NULL;
}


void CSGAlignmentDS::x_DeleteScoringJob()
{
    _ASSERT(m_ScoringJob  &&  m_ScoringJobID != -1);

    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    disp.DeleteJob(m_ScoringJobID);
    ResetAlnScoringJob();
}


void CSGAlignmentDS::x_InitScoringMethods(IAlnExplorer::EAlignType aln_type)
{
    if (aln_type == IAlnExplorer::fMixed) {
        // use protein's scoring methods for mixed alignments
        aln_type = IAlnExplorer::fProtein;
    } else if (aln_type == IAlnExplorer::fInvalid) {
        // using DNA scoring methods by default
        aln_type = IAlnExplorer::fDNA;
    }

    TMethods* methods = &m_DNAMethods;
    if (aln_type == IAlnExplorer::fProtein) {
        methods = &m_ProteinMethods;
    }

    if (methods->empty()) {
        CUIToolRegistry* reg = CUIToolRegistry::GetInstance();
        vector<CConstIRef<IUITool> > tools;
        CAlnScoringMethodSelector sel;
        reg->GetTools(tools, sel);

        for (size_t  i = 0; i < tools.size(); i++)  {
            const IScoringMethod* method =
                dynamic_cast<const IScoringMethod*>(tools[i].GetPointer());
            _ASSERT(method);
            if(method  &&  method->GetType() & aln_type) {
                CIRef<IUITool> tool = reg->CreateToolInstance(method->GetName());
                if (!tool) {
                    break;
                }

                IScoringMethod* method_clone =
                    dynamic_cast<IScoringMethod*>(tool.GetPointer());
                if (method_clone) {
                    methods->push_back(CIRef<IScoringMethod>(method_clone));
                }
            }
        }
    }
}

void CSGAlignmentDS::EnablePileUpCache(bool enable, bool enable_icache)
{
    m_HasPileUpCache = enable;
    if (m_HasPileUpCache)
        CGraphCache<CPileUpGraph>::GetInstance().EnableICache(enable_icache);
}

void CSGAlignmentDS::ClearCache()
{
    if (!m_HasPileUpCache)
        return;
    try {
        string cache_key;
        x_GetGraphCacheKey(cache_key);
        CGraphCache<CPileUpGraph>::GetInstance().RemoveData(cache_key);
    } catch (exception& e) {
        ERR_POST(Error << e.what());
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CSGAlignmentDSType
///////////////////////////////////////////////////////////////////////////////
string CSGAlignmentDSType::GetExtensionIdentifier() const
{
    static string sid("seqgraphic_alignment_ds_type");
    return sid;
}


string CSGAlignmentDSType::GetExtensionLabel() const
{
    static string slabel("Graphical View Alignment Data Source Type");
    return slabel;
}


ISGDataSource*
CSGAlignmentDSType::CreateDS(SConstScopedObject& object) const
{
    const CSeq_id& id = dynamic_cast<const CSeq_id&>(object.object.GetObject());
    return new CSGAlignmentDS(object.scope.GetObject(), id);
}


bool CSGAlignmentDSType::IsSharable() const
{
    return false;
}


END_NCBI_SCOPE
