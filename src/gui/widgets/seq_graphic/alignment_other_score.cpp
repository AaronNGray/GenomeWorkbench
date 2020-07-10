/*  $Id: alignment_other_score.cpp 26576 2012-10-10 16:27:30Z wuliangs $
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
#include <gui/widgets/seq_graphic/alignment_other_score.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CAlignOhterScore
///
void CSGAlnOtherScore::CalculateScores()
{
    m_ScoreCache->CalculateScores();
}


bool CSGAlnOtherScore::HasScores() const
{
    return (m_ScoreCache->HasScores()  &&  m_ScoreCache->GetScoringMethod());
}


IAlnScoreIterator* CSGAlnOtherScore::GetScoreIterator(int row) const
{
    return new CScoreCacheIterator(m_ScoreCache->GetScores(row));
}


CRgbaColor CSGAlnOtherScore::GetColorForScore(float score,
                                              IScoringMethod::EColorType type) const
{
    return m_ScoreCache->GetColorForScore(score, type);
}


bool CSGAlnOtherScore::IsAverageable() const
{
    return m_ScoreCache->GetScoringMethod()->IsAverageable();
}


int CSGAlnOtherScore::GetSupportedColorTypes() const
{
    return m_ScoreCache->GetScoringMethod()->GetSupportedColorTypes();
}


bool CSGAlnOtherScore::IsCacheable() const
{
    return true;
}


const string& CSGAlnOtherScore::GetScoringMethod() const
{
    return GetScoreName();
}


const string& CSGAlnOtherScore::GetScoreName()
{
    static const string kScoreName = "Other";
    return kScoreName;
}



END_NCBI_SCOPE
