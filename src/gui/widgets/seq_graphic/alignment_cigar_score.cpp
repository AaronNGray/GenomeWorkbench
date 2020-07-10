/*  $Id: alignment_cigar_score.cpp 43001 2019-05-03 19:55:12Z shkeda $
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
#include <gui/widgets/seq_graphic/alignment_cigar_score.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CAlignCigarScore
///
bool CAlignCigarScore::HasScores() const
{
    return (m_AlnMgr  &&  m_AlnMgr->IsCigarUnambiguous());
}


IAlnScoreIterator* CAlignCigarScore::GetScoreIterator(int row) const
{
    if (m_AlnMgr->IsCigarUnambiguous()) {
        const IAlnGraphicDataSource::TCigar* cigar = m_AlnMgr->GetCigar();
        TSeqPos start = m_AlnMgr->GetSeqStart(m_AlnMgr->GetAnchor());
        return new CCigarIterator(cigar, start);
    }

    return NULL;
}


CRgbaColor CAlignCigarScore::GetColorForScore(float score,
                                              IScoringMethod::EColorType type) const
{
    if (score > 0.5) {
        return type == IScoringMethod::fBackground ?
            CRgbaColor(0.8f, 0.8f, 0.8f, 1.0f) :
        CRgbaColor(0.0f, 0.0f, 0.8f, 1.0f);
    }

    return type == IScoringMethod::fBackground ?
        CRgbaColor(1.0f, 0.0f, 0.0f, 1.0f) :
    CRgbaColor(0.0f, 0.0f, 1.0f, 1.0f);

}


bool CAlignCigarScore::IsAverageable() const
{
    return true;
}


int CAlignCigarScore::GetSupportedColorTypes() const
{
    return IScoringMethod::fBackground | IScoringMethod::fForeground;
}


bool CAlignCigarScore::IsCacheable() const
{
    return false;
}


const string& CAlignCigarScore::GetScoringMethod() const
{
    return GetScoreName();
}


const string& CAlignCigarScore::GetScoreName()
{
    static const string kScoreName = "CIGAR";
    return kScoreName;
}


END_NCBI_SCOPE
