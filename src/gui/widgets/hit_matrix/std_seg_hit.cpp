/*  $Id: std_seg_hit.cpp 25706 2012-04-25 00:21:04Z voronov $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/hit_matrix/std_seg_hit.hpp>

#include <gui/objutils/sparse_functions.hpp>

#include <objects/seqloc/Seq_loc.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


///////////////////////////////////////////////////////////////////////////////
/// CStdSegHit

inline bool IsStrandGood(ENa_strand strand)
{
    return strand == eNa_strand_plus  ||
           strand == eNa_strand_minus ||
           strand == eNa_strand_unknown;
}

    // returns "true" if an alignment can be created from the specified rows of
    // the given alignment and with the required orientation
bool CStdSegHit::HasAlignment(const TStd& std_list,
                              size_t q_index, size_t s_index,
                              TDirection dir)
{
    ITERATE(TStd, it, std_list) {
        const CStd_seg& stdseg = **it;
        const CSeq_loc& q_loc = *stdseg.GetLoc()[q_index];
        const CSeq_loc& s_loc = *stdseg.GetLoc()[s_index];
        if(! q_loc.IsEmpty()  &&  ! s_loc.IsEmpty())    {
            ENa_strand q_strand = q_loc.GetStrand();
            ENa_strand s_strand = s_loc.GetStrand();
            bool good_strand = IsStrandGood(q_strand)  &&  IsStrandGood(q_strand);

            if(good_strand) {
                bool reversed =
                    ((q_strand == eNa_strand_minus) != (s_strand == eNa_strand_minus));
                bool match = (reversed  &&  dir != CAlnUserOptions::eDirect)  ||
                             (! reversed  &&  dir != CAlnUserOptions::eReverse);
                if(match)   {
                    // we have a non empty alignment with correct strands and proper
                    // orientation
                    return true;
                }
            }
        }
    }
    return false;
}


CStdSegHit::CStdSegHit(const objects::CSeq_align& align, int q_index, int s_index)
:   m_SeqAlign(&align),
    m_QueryIndex(q_index),
    m_SubjectIndex(s_index)
{
    _ASSERT(align.GetSegs().IsStd());

    typedef CStd_seg::TDim TDim;
    typedef list< CRef< CStd_seg > > TStd;
    const TStd& std_list = align.GetSegs().GetStd();

    ITERATE(TStd, it, std_list) {
        const CStd_seg& stdseg = **it;

        _ASSERT(q_index < stdseg.GetDim() && q_index >= 0);
        _ASSERT(s_index < stdseg.GetDim() && s_index >= 0);

        const CSeq_loc& q_loc = *stdseg.GetLoc()[q_index];
        const CSeq_loc& s_loc = *stdseg.GetLoc()[s_index];

        if(! q_loc.IsEmpty()  &&  ! s_loc.IsEmpty())    {
            // the alignment between the two is defined

            ENa_strand q_strand = q_loc.GetStrand();
            ENa_strand s_strand = s_loc.GetStrand();
            bool good_strand = IsStrandGood(q_strand)  &&  IsStrandGood(q_strand);

            if(good_strand) {
                CRange<TSignedSeqPos> q_range = stdseg.GetSeqRange(q_index);
                CRange<TSignedSeqPos> s_range = stdseg.GetSeqRange(s_index);

                CStdSegHitElement* elem =
                    new CStdSegHitElement(*this, q_range, s_range, q_strand, s_strand);
                m_Elements.push_back(elem);
            }
        }
    }
}


CStdSegHit::~CStdSegHit()
{
    for(size_t i = 0; i < m_Elements.size(); i++)   {
        delete m_Elements[i];
    }
}


CStdSegHit::TDim CStdSegHit::GetElemsCount() const
{
    return m_Elements.size();
}


const IHitElement& CStdSegHit::GetElem(TDim elem_index) const
{
    _ASSERT(size_t(elem_index) < m_Elements.size());
    return *m_Elements[elem_index];
}


double CStdSegHit::GetScoreValue(const string& score_name) const
{
    const objects::CSeq_align::TScore&   scores = m_SeqAlign->GetScore();
    ITERATE(objects::CSeq_align::TScore, itS, scores)   {
        const objects::CScore&   score = **itS;
        _ASSERT(score.CanGetId());

        if(score.GetId().GetStr() == score_name)  { // Match
            const objects::CScore::TValue& val = score.GetValue();
            switch(val.Which()) {
            case objects::CScore::TValue::e_Real: return val.GetReal(); break;
            case objects::CScore::TValue::e_Int: return val.GetInt(); break;
            default:    _ASSERT(false);
            }
        }
    }
    _ASSERT(false);
    return -1;
}


const objects::CSeq_align* CStdSegHit::GetSeqAlign() const
{
    return m_SeqAlign.GetPointer();
}


///////////////////////////////////////////////////////////////////////////////
///
CStdSegHitElement::CStdSegHitElement()
:   m_Hit(NULL)
{
}


CStdSegHitElement::~CStdSegHitElement()
{
}


CStdSegHitElement::CStdSegHitElement(const CStdSegHit& hit,
                                         const TRange& q_r, const TRange& s_r,
                                         ENa_strand q_strand,
                                         ENa_strand s_strand)
:   m_Hit(&hit),
    m_QueryRange(q_r),
    m_SubjectRange(s_r),
    m_QueryStrand(q_strand),
    m_SubjectStrand(s_strand)
{
    // on proteins strand is reported as eNa_strand_unknown, from our stand
    // point it is equavalent to eNa_strand_plus
    if(m_QueryStrand == eNa_strand_unknown) {
        m_QueryStrand = eNa_strand_plus;
    }
    if(m_SubjectStrand == eNa_strand_unknown) {
        m_SubjectStrand = eNa_strand_plus;
    }
}


const IHit& CStdSegHitElement::GetHit() const
{
    return *m_Hit;
}


TSignedSeqPos CStdSegHitElement::GetQueryStart() const
{
    return m_QueryRange.GetFrom();
}


TSignedSeqPos CStdSegHitElement::GetSubjectStart() const
{
    return m_SubjectRange.GetFrom();
}


TSeqPos CStdSegHitElement::GetQueryLength() const
{
    return m_QueryRange.GetLength();
}


TSeqPos CStdSegHitElement::GetSubjectLength() const
{
    return m_SubjectRange.GetLength();
}


objects::ENa_strand CStdSegHitElement::GetQueryStrand() const
{
    return m_QueryStrand;
}


objects::ENa_strand CStdSegHitElement::GetSubjectStrand() const
{
    return m_SubjectStrand;
}



END_NCBI_SCOPE
