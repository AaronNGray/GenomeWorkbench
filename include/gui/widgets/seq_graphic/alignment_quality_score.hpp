#ifndef GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_QUALITY_SCORE__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_QUALITY_SCORE__HPP

/*  $Id: alignment_quality_score.hpp 26795 2012-11-07 21:43:01Z wuliangs $
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
#include <gui/widgets/seq_graphic/alngraphic_ds.hpp>
#include <gui/widgets/aln_score/scoring_method.hpp>

BEGIN_NCBI_SCOPE


class CSGAlnQualityScore
    : public CObject
    , public ISGAlnScore
{
public:
    typedef unsigned char   TScore;
    typedef CAttrRangeCollection<TScore, TSeqPos>   TScoreColl;

private:
    class CAlnInfo : public CObject
    {
    public:
        CAlnInfo(IAlnExplorer::TNumrow anchor,
            const objects::CSeq_align& aln,
            objects::CScope& scope)
            : m_Anchor(anchor)
            , m_Align(&aln)
            , m_Scope(&scope)
        {}

        IAlnExplorer::TNumrow             m_Anchor;
        CConstRef<objects::CSeq_align>    m_Align;
        CRef<objects::CScope>             m_Scope;
    };

    class CQualityScoreIterator : public IAlnScoreIterator
    {
    public:
        CQualityScoreIterator(const CSGAlnQualityScore::TScoreColl& score_coll)
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
        { return (float)m_Iter->GetAttr(); }

    private:
        const CSGAlnQualityScore::TScoreColl& m_ScoreColl;
        CSGAlnQualityScore::TScoreColl::const_iterator m_Iter;
        CSGAlnQualityScore::TScoreColl::const_iterator m_IterEnd;
    };

public:
    CSGAlnQualityScore(IAlnExplorer::TNumrow anchor,
        const objects::CSeq_align& aln,
        objects::CScope& scope)
        : m_AlnInfo(new CAlnInfo(anchor, aln, scope))
    {}
    
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

    static const string& GetScoreName();

private:
    void x_RetrieveQualityMap(IAlnExplorer::TNumrow row, bool remap);

private:
    TScoreColl                        m_ScoreColl;
    CRef<CAlnInfo>                    m_AlnInfo;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_QUALITY_SCORE__HPP
