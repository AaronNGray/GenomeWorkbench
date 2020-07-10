#ifndef GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_SCORE__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_SCORE__HPP

/*  $Id: alignment_score.hpp 26795 2012-11-07 21:43:01Z wuliangs $
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


#include <corelib/ncbiobj.hpp>
#include <gui/gui.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/widgets/aln_score/scoring_method.hpp>

BEGIN_NCBI_SCOPE


class IAlnScoreIterator
{
public:
    virtual ~IAlnScoreIterator() {}
    virtual operator bool(void) const = 0;
    virtual IAlnScoreIterator& operator++(void) = 0;
    virtual IAlnScoreIterator& MoveTo(TSeqPos pos) = 0;
    virtual TSeqPos GetFrom() const = 0;
    virtual TSeqPos GetTo() const = 0;
    virtual float GetAttr() const = 0;
};


class ISGAlnScore
{
public:
    virtual ~ISGAlnScore() {}
    virtual void CalculateScores() = 0;
    virtual IAlnScoreIterator* GetScoreIterator(int row) const = 0;
    virtual CRgbaColor GetColorForScore(float score,
        IScoringMethod::EColorType type) const = 0;
    virtual int GetSupportedColorTypes() const = 0;
    virtual bool HasScores() const = 0;
    virtual bool IsAverageable() const = 0;
    virtual bool IsCacheable() const = 0;
    virtual const string& GetScoringMethod() const = 0;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_SCORE__HPP
