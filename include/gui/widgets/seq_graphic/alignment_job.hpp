#ifndef GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_JOB__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_JOB__HPP

/*  $Id: alignment_job.hpp 44887 2020-04-08 17:09:43Z shkeda $
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
#include <gui/widgets/seq_graphic/graph_cache.hpp>
#include <gui/widgets/seq_graphic/sparse_graph.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job.hpp>
#include <gui/widgets/seq_graphic/alignment_glyph.hpp>
#include <gui/widgets/seq_graphic/feature_sorter.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job_result.hpp>
#include <gui/widgets/seq_graphic/alignment_score.hpp>
#include <gui/widgets/seq_graphic/alignment_conf.hpp>
#include <gui/gui.hpp>
#include <gui/widgets/seq_graphic/layout_policy.hpp>
#include <gui/widgets/seq_graphic/align_statistics_glyph.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CAlign_CI;
    class CSeq_loc_Mapper;
END_SCOPE(objects)


///////////////////////////////////////////////////////////////////////////////
/// CSGAlignmentJob
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGAlignmentJob : public CSGAnnotJob
{
public:
    /// constructor for loading coverage graph.
    CSGAlignmentJob(const string& desc, objects::CBioseq_Handle handle,
        const objects::SAnnotSelector& sel,
        const TSeqRange& range, TModelUnit window,
        TSignedSeqPos &maxStartTail, TSignedSeqPos &maxEndTail);

    /// constructor for loading alignments.
    CSGAlignmentJob(const string& desc, objects::CBioseq_Handle handle,
        const objects::SAnnotSelector& sel,
        const TSeqRange& range, TModelUnit window,
        int align_limit, bool smear_if_overlimit,
        bool link_pair, TSignedSeqPos &maxStartTail, TSignedSeqPos &maxEndTail);

    /// constructor for loading features on the aligned sequences.
    CSGAlignmentJob(const string& desc, objects::CBioseq_Handle handle,
        const objects::SAnnotSelector& sel, const TSeqRange& range,
        TModelUnit window, vector< CRef<CAlignGlyph> > aligns,
        TSignedSeqPos &maxStartTail, TSignedSeqPos &maxEndTail);

    void SetAnnotName(const string& name);

    void SetSortBy(const string& sort_by);

    void SetHideSra(CAlignmentConfig::EHideSraAlignments hideSra) { m_HideSra = hideSra; }

    void SetUnalignedTailsMode(CAlignmentConfig::EUnalignedTailsMode tailsMode) { m_UnalignedTailsMode = tailsMode; }

    static void GetAnnotNames(const objects::CBioseq_Handle& handle,
        const TSeqRange& range, objects::SAnnotSelector& sel,
        TAnnotNameTitleMap& names);

    /// when isFastConfig set to true, it indicates that that the call is used by seqconfig
    /// this means that setting m_HasCoverageGraph and getting the actual type that are taking a long time
    /// and not needed by seqconfig will be skipped (IAlnExplorer::fNotDefined will be returned)
    static IAlnExplorer::EAlignType GetAlignType(
        const objects::CBioseq_Handle& handle,
        objects::SAnnotSelector& sel,
        bool& has_quality_map,
        bool isFastConfig = false);

    bool FilterDuplicatesAndBadReads(const objects::CSeq_align& mapped_align);

    /// Request a list of alignments from a bioseq.
    enum EAlignFlags {
        fAlign_LinkMatePairs        = 0x1,
        fAlign_MatedAlignsAsPwAlign = 0x2,
        fAlign_Default = 0x0
    };
    typedef int TAlignFlags;

    void SetLayoutPolicy(ILayoutPolicy* layout_policy);

protected:
    virtual EJobState x_Execute();

private:
    EJobState x_LoadAlignments();
    CRef<CSGJobResult> x_LoadAlignmentFeats(CAlignGlyph& align);

    CRef<CSeqGlyph> x_CreateFeatGlyph(objects::CSeq_loc_Mapper& mapper,
                                      const objects::CMappedFeat& mapped_feat, CAlignGlyph& align) const;

    bool x_CreateGeneModels(objects::CSeq_loc_Mapper& mapper,
                            CLinkedFeature::TLinkedFeats& feats, CSeqGlyph::TObjects& objs, CAlignGlyph& align) const;

    IAppJob::EJobState x_GetAlignSmear(CSeqGlyph::TObjects& objs,
        objects::CAlign_CI& align_iter);

    IAppJob::EJobState x_GetAlignments(CSeqGlyph::TObjects& objs,
        objects::CAlign_CI& align_iter, bool sparseAln, bool filterResults = false);

    IAppJob::EJobState x_GetCoverageGraph();

    void x_UpdateMaxTails(const IAlnGraphicDataSource &aln_datasource);

private:
    vector< CRef<CAlignGlyph> > m_Aligns;   ///< alignments feature retrieval
    TModelUnit             m_Window;        ///< current window for smear bars
    CIRef<IAlignSorter>    m_Sorter;

    /// max number of alignments to load.
    /// m_AlignLimit < 0, means smear all alignments
    /// m_AlignLimit = 0, show alignments only when at sequence level
    int                    m_AlignLimit;
    bool                   m_SmearOverLimit = false;///< flag indicating if smear is needed
    bool                   m_LinkPair;      ///< link mate pair aligns
    bool                   m_LoadCoverageGraph;
    CAlignmentConfig::EHideSraAlignments    m_HideSra; ///< Flag, indicating whether to hide duplicates and/or bad reads
    CAlignmentConfig::EUnalignedTailsMode   m_UnalignedTailsMode; ///< Flag, indicating whether and how to display unaligned tails
    TSignedSeqPos   &m_MaxStartTail; ///< Max length of unaligned tail at the start of the sequence
    TSignedSeqPos   &m_MaxEndTail;   ///< Max length of unaligned tail at the end of the sequence
    CIRef<ILayoutPolicy>  m_LayoutPolicy;

};


///////////////////////////////////////////////////////////////////////////////
/// CSGAlignScoreJob
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGAlignScoringJob : public CSeqGraphicJob
{
public:
    typedef pair<CConstRef<CObject>, int> TAlnAnchorPair;
    typedef map<TAlnAnchorPair, CIRef<ISGAlnScore> > TAlnScoreMap;

    CSGAlignScoringJob(const string& desc, const TAlnScoreMap& scores);

private:
    CSGAlignScoringJob(const CSGAlignScoringJob&);
    const CSGAlignScoringJob& operator=(const CSGAlignScoringJob&);

    virtual EJobState x_Execute();

private:
    TAlnScoreMap m_AlnScoreMap;
};

///////////////////////////////////////////////////////////////////////////////
/// CGraphStatCollector
/// Helper class to use with CSGAlignStatJob::x_CollectAlignStats
/// x_CollectAlignStats reads alignments pileups and stores them into 
/// either CAlignmentGraph or CAlnStatGlyph::TStatVec using the corresponding
/// descendant of CGraphStatCollector

class CGraphStatCollector
{
public:
    CGraphStatCollector(double window)
        : m_Window(window)
    {}
    virtual void NewGroup(int batch_size) {}
    virtual void AddStat(size_t index, CAlnStatGlyph::SStatStruct& stat) = 0;
    virtual void UpdateGroup(size_t offset) {}
    virtual void ClearRange(const TSeqRange& range) {}
    virtual bool ComputePileUp() { return true;}
    double GetWindow() const
    { return m_Window;}
    virtual ~CGraphStatCollector() = default;
private:    
    double m_Window = 1;
};

///////////////////////////////////////////////////////////////////////////////
/// CSGAlignStatJob
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGAlignStatJob : public CSGAnnotJob
{
public:
    typedef vector< CConstRef<IAlnGraphicDataSource> > TAlnMgrVec;

    CSGAlignStatJob(const string& desc,
                    objects::CBioseq_Handle handle,
                    const objects::SAnnotSelector& aln_sel,
                    const objects::SAnnotSelector& graph_sel,
                    const TSeqRange& range,
                    const TAlnMgrVec& aligns,
                    TModelUnit scale,
                    const string& cache_key);

private:
    CSGAlignStatJob(const CSGAlignStatJob&);
    const CSGAlignStatJob& operator=(const CSGAlignStatJob&);

    virtual EJobState x_Execute();
    EJobState x_LoadAlignments(objects::CTSE_Handle& tse_handle);

    typedef vector<CConstRef<objects::CSeq_graph>> TGraphs;
    typedef IAlnGraphicDataSource::TNumrow TNumrow;

    void x_CreatePileUpGraphs(TSeqRange&range, TGraphs& graphs);
    EJobState x_CollectAlignStats(const TSeqRange& main_range, objects::CTSE_Handle& tse_handle, CGraphStatCollector& stat_collector);
private:
    objects::SAnnotSelector m_GraphSel;   ///< pileup graph selector
    TAlnMgrVec  m_Aligns;
    TModelUnit  m_Window;                 ///< current zoom scale
    string      m_CacheKey;                ///< Graph cache key calculated by alignment ds
};

inline 
void CSGAlignmentJob::SetLayoutPolicy(ILayoutPolicy* layout_policy)
{
    m_LayoutPolicy = layout_policy;
}


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_JOB__HPP
