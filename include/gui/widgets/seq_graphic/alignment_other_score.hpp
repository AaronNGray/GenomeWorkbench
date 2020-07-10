#ifndef GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_OTHER_SCORE__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_OTHER_SCORE__HPP

/*  $Id: alignment_other_score.hpp 26795 2012-11-07 21:43:01Z wuliangs $
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


#include <gui/widgets/seq_graphic/alignment_score.hpp>
#include <gui/widgets/aln_score/aln_scoring.hpp>

BEGIN_NCBI_SCOPE


class CSGAlnOtherScore
    : public CObject
    , public ISGAlnScore
{
private:
    
    class CScoreCacheIterator : public IAlnScoreIterator
    {
    public:
        CScoreCacheIterator(const CScoreCache::TScoreColl& score_coll)
            : m_ScoreColl(score_coll)
            , m_Iter(score_coll.begin())
            , m_IterEnd(score_coll.end())
        {}

        virtual operator bool(void) const
        { return m_Iter != m_IterEnd; }

        virtual IAlnScoreIterator& operator++(void)
        { ++m_Iter; return *this; }

        virtual IAlnScoreIterator& MoveTo(TSeqPos pos)
        { 
            m_Iter = m_ScoreColl.find(m_Iter, m_IterEnd, pos);
            return *this;
        }

        virtual TSeqPos GetFrom() const
        { return m_Iter->GetFrom(); }

        virtual TSeqPos GetTo() const
        { return m_Iter->GetTo(); }

        virtual float GetAttr() const
        { return m_Iter->GetAttr(); }

    private:
        const CScoreCache::TScoreColl& m_ScoreColl;
        CScoreCache::TScoreColl::const_iterator m_Iter;
        CScoreCache::TScoreColl::const_iterator m_IterEnd;
    };


public:
    CSGAlnOtherScore(CScoreCache& cache) 
        : m_ScoreCache(&cache) {}
    
    /// @name ISGAlnScore interface implementation
    /// @{
    virtual void CalculateScores();
    virtual bool HasScores() const;
    virtual IAlnScoreIterator* GetScoreIterator(int row) const;
    virtual CRgbaColor GetColorForScore(float score, IScoringMethod::EColorType type) const;
    virtual int GetSupportedColorTypes() const;
    virtual bool IsAverageable() const;
    virtual bool IsCacheable() const;
    virtual const string& GetScoringMethod() const;
    /// @}

    CScoreCache& GetScoreCache()
    {
        return *m_ScoreCache;
    }

    static const string& GetScoreName();

private:
    CRef<CScoreCache> m_ScoreCache;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_OTHER_SCORE__HPP
