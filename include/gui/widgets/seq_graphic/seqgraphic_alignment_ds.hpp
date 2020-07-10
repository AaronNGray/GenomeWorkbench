#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_ALIGNMENT_DS__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_ALIGNMENT_DS__HPP


/*  $Id: seqgraphic_alignment_ds.hpp 40924 2018-05-01 12:41:37Z evgeniev $
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

#include <gui/widgets/seq_graphic/seqgraphic_genbank_ds.hpp>
#include <gui/widgets/seq_graphic/alignment_job.hpp>
#include <gui/widgets/seq_graphic/alignment_glyph.hpp>
#include <gui/widgets/seq_graphic/alignment_score.hpp>
#include <gui/widgets/seq_graphic/alignment_conf.hpp>
#include <objtools/alnmgr/aln_explorer.hpp>
#include <objmgr/tse_handle.hpp>
#include <gui/utils/extension.hpp>

#include <objmgr/impl/scope_info.hpp>

BEGIN_NCBI_SCOPE

static const string  kBamDataLoader = "CBAMDataLoader";
static const string  kCSRADataLoader = "CCSRADataLoader";

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGAlignmentDS :
    public CSGGenBankDS
{
public:
    typedef CAlignGlyph::TAlnAnchorPair TAlnAnchorPair;
    typedef vector< CConstRef<IAlnGraphicDataSource> > TAlnMgrVec;

    enum EDataLoader {
        eLoader_Unknown,
        eLoader_BAM,
        eLoader_CSRA
    };

    CSGAlignmentDS(objects::CScope& scope, const objects::CSeq_id& id);

    virtual ~CSGAlignmentDS();

    /// keep instances of score methods here.
    typedef list<CIRef<IScoringMethod> >  TMethods;

    /// @param smear_align smear the alignments also depending on
    ///        flag 'adpative'
    /// @param adaptive_smear if true, the alignments will be
    ///        smeared adaptively
    void LoadCoverageGraph(const TSeqRange& range, TModelUnit window,
        TJobToken token);

    void LoadAlignments(const TSeqRange& range, TModelUnit window,
        int align_limit, bool smear_if_overlimit, TJobToken token);

    //void LoadAlignments(const TSeqRange& range, TModelUnit window,
    //    int smear_cutoff, TJobToken token);

    void CalcAlnStat(const TAlnMgrVec& aligns, const TSeqRange& range,
                     TModelUnit window,
                     TJobToken token);

    void LoadAlignFeats(const TSeqRange& range, TModelUnit window,
        vector< CRef<CAlignGlyph> > aligns,
        const vector<int>& projected_feats,
        TJobToken token);

    IAlnExplorer::EAlignType GetAlignType(const CSeqGlyph* obj) const;
    IAlnExplorer::EAlignType GetAlignType() const;

    const string& GetDataLoader() const;
    bool IsBamLoader() const;
    bool IsCSRALoader() const;

    /// This method might need object manager to connect to ID, and
    /// it is a block call. Please make sure it is called in a background
    /// thread to avoid blocking the main application.
    /// when isFastConfig set to true, it indicates that that the call is used by seqconfig
    /// this means that setting m_HasCoverageGraph and getting the actual type that are taking a long time
    /// and not needed by seqconfig will be skipped (IAlnExplorer::fHomogenous will be returned)
    IAlnExplorer::EAlignType InitAlignType(bool isFastConfig = false);

    /// Alignment scoring.
    void InitAlignScore(CAlignGlyph* aln);
    void InitAlignQualityScore(CAlignGlyph* aln,
        const TAlnAnchorPair& align_pair, IAlnExplorer::EAlignType sc_type);
    void InitAlignScoreCache(CAlignGlyph* aln,
        const TAlnAnchorPair& align_pair, IAlnExplorer::EAlignType sc_type);

    void CalculateAlignmentScore(CSeqGlyph::TObjects& objs, TJobToken token);
    void ResetAlnScoringJob();

    void SetDefaultScoringMethods(string dna_method, string protein_method);

    /// get all the existing align scoring methods.
    const TMethods& GetScoringMethods(IAlnExplorer::EAlignType aln_type);

    void RecalculateScore(CConstRef<CSeqGlyph> obj);
    //CRef<CScoreCache> GetScoreCache(const CAlignGlyph* aln) const;

    void GetAnnotNames(objects::SAnnotSelector& sel,
        const TSeqRange& range, TAnnotNameTitleMap& names) const;

    void SetDNAScoringMethod(const string& method);
    const string& GetDNAScoringMethod() const;
    void SetProteinScoringMethod(const string& method);
    const string& GetProteinScoringMethod() const;
    bool IsValidProteinScoringMethod(const string& name);
    bool IsValidDNAScoringMethod(const string& name);

    void SetEnableColoration(bool f);
    bool GetEnableColoration() const;
    void SetLinkMatePairs(bool f);
    bool GetLinkMatePairs() const;

    virtual void ClearJobID(TJobID job_id);
    virtual void DeleteAllJobs();

    void SetAnnotName(const string& name);
    const string& GetAnnotName() const;

    void SetHasCoverageGraph(bool flag);
    bool HasCoverageGraph() const;
    bool HasQualityMap() const;

    void SetSortBy(const string& sortby);
    const string& GetSortBy() const;

    void SetHideSra(CAlignmentConfig::EHideSraAlignments hideSra) { m_HideSra = hideSra; }
    void SetUnalignedTailsMode(CAlignmentConfig::EUnalignedTailsMode tailsMode) { m_UnalignedTailsMode = tailsMode; }

    double GetGraphCost(const TSeqRange& range) const;
    bool IsGraphCached(const TSeqRange& range) const;

    double GetAlignmentCost(const TSeqRange& range) const;

    void SetLayoutPolicy(ILayoutPolicy* layout_policy);

    void SetRemotePath(const string& remote_path);
    void EnablePileUpCache(bool enable, bool enable_icache);
    void ClearCache();

    bool CanShowRange(const TSeqRange& range, int align_limit);
    bool IsRangeCached(const TSeqRange& range) const;

    void SetDataHandle(const objects::CTSE_Handle& tse);
    bool NeedReleaseMemory() const;
    void ReleaseMemory();

    void SetMemoryLimit(Uint8 memory_limit);
    void SetCgiMode(bool cgi_mode);

    void SetAlnDataLoader(EDataLoader data_loader);
private:
    bool x_IsScoringJobRunning() const;
    void x_DeleteScoringJob();
    void x_InitScoringMethods(IAlnExplorer::EAlignType aln_type);
    void x_GetGraphCacheKey(string& data_key) const;
private:

    string      m_AnnotName;

    TMethods    m_DNAMethods;      ///< registered DNA scoring methods
    TMethods    m_ProteinMethods;  ///< registered Protein scoring methods

    typedef map< TAlnAnchorPair, CIRef<ISGAlnScore> > TAlnScoreMap;
    mutable TAlnScoreMap m_AlnScoreMap;

    string  m_DNAMethod;    ///< default scoring method for DNA alignments
    string  m_ProtMethod;   ///< default scoring method for protein alignments
    IAlnExplorer::EAlignType  m_AlnType;

    string  m_SortBy;

    bool    m_Enabled;      ///< is alignment score coloration enabled
    bool    m_LinkMatePairs;///< link mate pair alignments
    bool    m_HasCoverageGraph;  ///< High level coverage graph?
    bool    m_HasQualityMap; ///< Has quality map? (e.g. cSRA alignments)

    TJobToken m_ScoreToken; ///< the scoring job token
    TJobID    m_ScoringJobID; ///< alignment scoring job id
    CRef<CSGAlignScoringJob> m_ScoringJob;
    CAlignmentConfig::EHideSraAlignments    m_HideSra; ///< Flag, indicating whether to hide duplicates and/or bad reads
    CAlignmentConfig::EUnalignedTailsMode   m_UnalignedTailsMode; ///< Flag, indicating whether and how to display unaligned tails
    TSignedSeqPos   m_MaxStartTail; ///< Max length of unaligned tail at the start of the sequence
    TSignedSeqPos   m_MaxEndTail;   ///< Max length of unaligned tail at the end of the sequence

    mutable  string m_DataLoader = "none"; // Data loader typename 

    CIRef<ILayoutPolicy> m_LayoutPolicy;
    string          m_RemotePath; ///< url to the remote data

    objects::CTSE_Handle m_DataHandle;  ///< The last loaded data
    Uint8            m_MemoryLimit = 2 * (Uint8)1073741824; ///<  NeedReleaseMemory's default limit 2GB.

    bool             m_HasPileUpCache = false;
    bool             m_CgiMode = false; ///< sviewer context 
    EDataLoader      m_AlnDataLoader = eLoader_Unknown;
};


class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGAlignmentDSType :
    public CObject,
    public ISGDataSourceType,
    public IExtension
{
public:
    /// @name IExtension interface implementation
    /// @{
    virtual string GetExtensionIdentifier() const;
    virtual string GetExtensionLabel() const;
    /// @}

    virtual ISGDataSource*  CreateDS(SConstScopedObject& object) const;

    virtual bool IsSharable() const;
};


///////////////////////////////////////////////////////////////////////////////
/// CSGAlignmentDS inline methods
///
inline
IAlnExplorer::EAlignType CSGAlignmentDS::GetAlignType() const
{
    return m_AlnType;
}

inline
void CSGAlignmentDS::GetAnnotNames(objects::SAnnotSelector& sel,
                                   const TSeqRange& range,
                                   TAnnotNameTitleMap& names) const
{
    CSeqUtils::SetResolveDepth(sel, m_Adaptive, m_Depth);
    CSGAlignmentJob::GetAnnotNames(m_Handle, range, sel, names);
}

inline
void CSGAlignmentDS::SetDNAScoringMethod(const string& method)
{
    m_DNAMethod = method;
}

inline
const string& CSGAlignmentDS::GetDNAScoringMethod() const
{
    return m_DNAMethod;
}

inline
void CSGAlignmentDS::SetProteinScoringMethod(const string& method)
{
    m_ProtMethod = method;
}

inline
const string& CSGAlignmentDS::GetProteinScoringMethod() const
{
    return m_ProtMethod;
}

inline
void CSGAlignmentDS::SetEnableColoration(bool f)
{
    m_Enabled = f;
}

inline
bool CSGAlignmentDS::GetEnableColoration() const
{
    return m_Enabled;
}

inline
void CSGAlignmentDS::SetLinkMatePairs(bool f)
{
    m_LinkMatePairs = f;
}

inline
bool CSGAlignmentDS::GetLinkMatePairs() const
{
    return m_LinkMatePairs;
}

inline
void CSGAlignmentDS::SetAnnotName(const string& name)
{
    m_AnnotName = name;
}

inline
const string& CSGAlignmentDS::GetAnnotName() const
{
    return m_AnnotName;
}

inline
void CSGAlignmentDS::SetHasCoverageGraph(bool flag)
{
    m_HasCoverageGraph = flag;
}

inline
bool CSGAlignmentDS::HasCoverageGraph() const
{
    return m_HasCoverageGraph;
}

inline
bool CSGAlignmentDS::HasQualityMap() const
{
    return m_HasQualityMap;
}

inline
void CSGAlignmentDS::SetSortBy(const string& sortby)
{
    m_SortBy = sortby;
}

inline
const string& CSGAlignmentDS::GetSortBy() const
{
    return m_SortBy;
}

inline
bool CSGAlignmentDS::IsBamLoader() const
{
    if (eLoader_Unknown == m_AlnDataLoader)
        return GetDataLoader() == kBamDataLoader;

    return (eLoader_BAM == m_AlnDataLoader);
}


inline
bool CSGAlignmentDS::IsCSRALoader() const
{
    if (eLoader_Unknown == m_AlnDataLoader)
        return GetDataLoader() == kCSRADataLoader;

    return (eLoader_CSRA == m_AlnDataLoader);
}

inline
void CSGAlignmentDS::SetLayoutPolicy(ILayoutPolicy* layout_policy)
{
    m_LayoutPolicy = layout_policy;
}

inline
void CSGAlignmentDS::SetRemotePath(const string& remote_path)
{
    m_RemotePath = remote_path;
}

inline
void CSGAlignmentDS::SetDataHandle(const objects::CTSE_Handle& tse)
{
    m_DataHandle = tse;
}

inline
void CSGAlignmentDS::SetMemoryLimit(Uint8 memory_limit)
{
    m_MemoryLimit = memory_limit;
}

inline
void CSGAlignmentDS::SetCgiMode(bool cgi_mode)
{
    m_CgiMode = cgi_mode;
}


inline
void CSGAlignmentDS::SetAlnDataLoader(EDataLoader data_loader)
{
    m_AlnDataLoader = data_loader;
}

END_NCBI_SCOPE

#endif  /* GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_ALIGNMENT_DS__HPP */
