#ifndef GUI_WIDGETS_ALN_SCORE___ALN_SCORING__HPP
#define GUI_WIDGETS_ALN_SCORE___ALN_SCORING__HPP

/*  $Id: aln_scoring.hpp 40589 2018-03-15 19:34:35Z shkeda $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Andrey Yazhuk
 */

#include <corelib/ncbistd.hpp>

#include <gui/widgets/aln_score/scoring_method.hpp>

#include <gui/utils/app_job.hpp>
#include <gui/utils/app_job_impl.hpp>
#include <gui/utils/event_handler.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CScoringParams
struct NCBI_GUIWIDGETS_ALNSCORE_EXPORT CScoringParams    {
    CScoringParams();

    CConstIRef<IScoringAlignment>   m_Alignment;
    CIRef<IScoringMethod>   m_Method;
    int     m_GradNumber;
};


///////////////////////////////////////////////////////////////////////////////
/// CDMSearchResult
class NCBI_GUIWIDGETS_ALNSCORE_EXPORT CScoringJobResult : public CObject
{
public:
    typedef IScoringMethod::TScoreColl  TScoreColl;
    typedef vector<TScoreColl>  TScoreCollVector;

    CScoringJobResult(TScoreCollVector* colls)   : m_ScoreColls(colls)   {}
    virtual ~CScoringJobResult()  {   delete m_ScoreColls;    };

    TScoreCollVector*   m_ScoreColls;
};


///////////////////////////////////////////////////////////////////////////////
/// CScoringJob
class NCBI_GUIWIDGETS_ALNSCORE_EXPORT CScoringJob : public CJobCancelable
{
public:
    typedef IScoringMethod::TScoreColl  TScoreColl;
    typedef IAlnExplorer::TNumrow    TNumrow;
    typedef IScoringMethod::TScore  TScore;
    typedef IScoringMethod::TScoreVector  TScoreVector;
    typedef vector<TScoreColl>  TScoreCollVector;

    CScoringJob(CScoringParams& params);
    virtual ~CScoringJob();

    /// @name IAppJob implementation
    /// @{
    virtual EJobState   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string    GetDescr() const;
    /// @}

    bool Calculate();

    /// transfers ownership
    TScoreCollVector*   GetResults();

    const   IScoringAlignment*  GetAlignment() const
    { return m_Params.m_Alignment; };


private:
    // Sequence buffer routins.
    char x_BufferGetSeq(TSeqPos pos, TNumrow row) const;
    void x_BufferGetColumn(TSeqPos pos, string& column) const;

    void x_AllocBuffer(TSeqPos row_len);
    void x_FreeBuffer();

    /// Fetches sequence from CAlnVec.
    void x_UpdateBuffer(TSeqPos start, TSeqPos stop, int& state, bool translate = false);
    void TranslateNAToAA(const string& na, string& aa, int& state, int gencode = 1) const;

    TSeqPos x_GetStart() const;
    TSeqPos x_GetStop() const;
    TSeqPos x_GetRowLength() const;

    void    x_CalculateSequenceScores(IRowScoringMethod& method);
    void    x_CalculateAlignmentScores(IColumnScoringMethod& method);
    void    x_WindowAvgScores(size_t window_size);

protected:
    TScoreCollVector*   m_ScoreColls; /// score storage

    CScoringParams  m_Params;

    /// Sequence buffer data members.
    vector<string> m_vRows;
    TSeqPos m_BufferStart; /// Aln position corresponding to the first column of the buffer
    TSeqPos m_RowLength;   /// buffer row length

    // IAppJob
    string  m_Descr;
    float   m_NormDone; /// normalized [0, 1.0] progress

    CRef<CScoringJobResult>   m_Result;
    CRef<CAppJobError>      m_Error;
};


///////////////////////////////////////////////////////////////////////////////
/// CScoreCache class uses given scoring method to calculate scores for
/// CAlnVec-type alignment. CScoreCache stores scores in compressed form
/// as CAttrRangeCollection. To make fetching of sequence and calculation
/// of scores more effective CScoreCache uses sequence buffer.
///
class CAppJobNotification;

class NCBI_GUIWIDGETS_ALNSCORE_EXPORT CScoreCache : public CObjectEx, public CEventHandler
{
public:
    typedef IAlnExplorer::TNumrow    TNumrow;
    typedef IScoringMethod::TScore  TScore;
    typedef IScoringMethod::TScoreVector  TScoreVector;
    typedef IScoringMethod::TScoreColl  TScoreColl;

    class IListener {
    public:
        virtual ~IListener()    {};
        virtual void    OnScoringProgress(float progress, const string& msg) = 0;
        virtual void    OnScoringFinished() = 0;
    };

public:
    CScoreCache();
    ~CScoreCache();

    void    SetListener(IListener* listener);
    void    EnableBackgoundProcessing(bool en);

    /// Sets the number of score gradations.
    void    SetGradNumber(int grad_n);

    void    SetScoringMethod(IScoringMethod *method);
    IScoringMethod*    GetScoringMethod();
    const IScoringMethod*    GetScoringMethod() const;

    void    SetAlignment(const IScoringAlignment* aln);
    const   IScoringAlignment*  GetAlignment() const;

    void ResetScores();

    /// Recalculate scores using current method; resets scores if no method is
    /// selected.
    void    CalculateScores();

    /// returns "true" if CalculateScores() did produce results
    bool    HasScores() const;

    const TScoreColl&   GetScores(TNumrow row) const;
    const CRgbaColor&   GetColorForScore(TScore score,
        IScoringMethod::EColorType type) const;
    const CRgbaColor&   GetColorForNoScore(IScoringMethod::EColorType type) const;

    void    OnAJNotification(CEvent* evt);

protected:
    DECLARE_EVENT_MAP();

    /// returns true if CScoreCache launched the job and it has not finished yet
    /// (it may have finished, but CScoreCache has not yet been notified)
    bool    x_IsJobRunning();

    /// stop the job and destriy it
    void    x_DeleteJob();

    void    x_OnJobCompleted(CAppJobNotification& notn);
    void    x_OnJobFailed(CAppJobNotification& notn);
    void    x_OnJobCanceled(CAppJobNotification& notn);
    void    x_OnJobProgress(CAppJobNotification& notn);

    bool    x_TransferResults(CObject* result);
protected:
    typedef vector<TScoreColl>  TScoreCollVector;
    TScoreCollVector*   m_ScoreColls; /// score storage

    IListener* m_Listener;

    CScoringParams  m_Params;
    bool    m_EnBgProcessing;
    int m_JobID;
    CRef<CScoringJob>   m_Job;

    bool    m_HasScores;
    CRgbaColor    m_NoScoreColor;
};


inline
const CRgbaColor&
CScoreCache::GetColorForScore(TScore score,
                              IScoringMethod::EColorType type) const
{
    return m_Params.m_Method->GetColorForScore(score, type);
}

inline
IScoringMethod* CScoreCache::GetScoringMethod()
{
    return m_Params.m_Method;
}

inline
const IScoringMethod* CScoreCache::GetScoringMethod() const
{
    return m_Params.m_Method;
}

inline
void CScoreCache::SetAlignment(const IScoringAlignment* aln)
{
    m_Params.m_Alignment = aln;
}

inline
const IScoringAlignment* CScoreCache::GetAlignment() const
{
    return m_Params.m_Alignment;
}

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_SCORE___ALN_SCORING__HPP
