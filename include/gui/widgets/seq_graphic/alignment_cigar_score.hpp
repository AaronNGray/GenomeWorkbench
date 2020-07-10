#ifndef GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_CIGAR_SCORE__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_CIGAR_SCORE__HPP

/*  $Id: alignment_cigar_score.hpp 26795 2012-11-07 21:43:01Z wuliangs $
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

BEGIN_NCBI_SCOPE


class CAlignCigarScore
    : public CObject
    , public ISGAlnScore

{
private:
    class CCigarIterator : public IAlnScoreIterator
    {
    public:
        CCigarIterator(const IAlnGraphicDataSource::TCigar* cigar,
            TSeqPos start)
            : m_Cigar(cigar)
            , m_Start(start)
            , m_Iter(m_Cigar->begin())
            , m_IterEnd(m_Cigar->end())
        {
            x_Validate();
        }

        virtual operator bool(void) const
        { return m_Iter != m_IterEnd; }

        virtual IAlnScoreIterator& operator++(void)
        {
            m_Start += m_Iter->second;
            ++m_Iter;
            x_Validate();
            return *this;
        }

        virtual IAlnScoreIterator& MoveTo(TSeqPos pos)
        { 
            return *this;
        }

        virtual TSeqPos GetFrom() const
        { return m_Start; }

        virtual TSeqPos GetTo() const
        { return m_Start + (TSeqPos)m_Iter->second - 1; }

        virtual float GetAttr() const
        {
            if (m_Iter->first == IAlnGraphicDataSource::eCigar_M  ||
                m_Iter->first == IAlnGraphicDataSource::eCigar_Eq) {
                    return 1.0;
            }

            return 0.0;
        }

    private:
        void x_Validate() {
            while (m_Iter != m_IterEnd  &&
                (m_Iter->first == IAlnGraphicDataSource::eCigar_I ||
                m_Iter->first == IAlnGraphicDataSource::eCigar_S ||
                m_Iter->first == IAlnGraphicDataSource::eCigar_H)) {
                    ++m_Iter;
            }
        }

    private:
        const IAlnGraphicDataSource::TCigar* m_Cigar;
        TSeqPos m_Start;
        IAlnGraphicDataSource::TCigar::const_iterator m_Iter;
        IAlnGraphicDataSource::TCigar::const_iterator m_IterEnd;
    };


public:
    CAlignCigarScore(const IAlnGraphicDataSource& aln_mgr) 
        : m_AlnMgr(&aln_mgr) {}

    /// @name ISGAlnScore interface implementation
    /// @{
    virtual void CalculateScores() {}
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
    const IAlnGraphicDataSource* m_AlnMgr;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_CIGAR_SCORE__HPP
