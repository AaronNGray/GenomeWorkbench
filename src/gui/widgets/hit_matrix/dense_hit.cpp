/*  $Id: dense_hit.cpp 25706 2012-04-25 00:21:04Z voronov $
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

#include <gui/widgets/hit_matrix/dense_hit.hpp>

#include <gui/objutils/sparse_functions.hpp>
#include <objects/general/Object_id.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


///////////////////////////////////////////////////////////////////////////////
/// CDenseSegHit

bool CDenseSegHit::HasAlignment(const objects::CDense_seg& denseg,
                                size_t q_index, size_t s_index,
                                TDirection dir)
{
    // check whether orientation of the specified rows matches "dir"
    bool reversed = false; // by default
    if(denseg.IsSetStrands())    {
        bool q_minus = denseg.GetSeqStrand((CDense_seg::TDim)q_index) == eNa_strand_minus;
        bool s_minus = denseg.GetSeqStrand((CDense_seg::TDim)s_index) == eNa_strand_minus;
        reversed = (q_minus != s_minus);
    }

    if((reversed  &&  dir == CAlnUserOptions::eDirect)  ||
       (! reversed  &&  dir == CAlnUserOptions::eReverse)) {
        return false; // this alignment does not qualify
    }

    // check whether do q_index and s_index define a pairwise alignment -
    // look for a column that has non-negative starts in both Q and S rows
    CDense_seg::TDim dim = denseg.GetDim();
    CDense_seg::TNumseg n_seg = denseg.GetNumseg();
    const CDense_seg::TStarts& starts = denseg.GetStarts();

    for( CDense_seg::TNumseg seg = 0;  seg < n_seg; seg++ )   {
        CDense_seg::TDim offset = seg * dim;
        TSignedSeqPos q_start = starts[q_index + offset];
        TSignedSeqPos s_start = starts[s_index + offset];
        if(q_start >= 0  && s_start >=0 )   {
            return true;
        }
    }
    return false;
}


CDenseSegHit::CDenseSegHit(const objects::CSeq_align& align, int q_index, int s_index)
:   m_SeqAlign(&align),
    m_QueryIndex(q_index),
    m_SubjectIndex(s_index)
{
    _ASSERT(align.GetSegs().IsDenseg());
    const CDense_seg& denseg = align.GetSegs().GetDenseg();

    /// using Alignment functions
    auto_ptr<SAlignedSeq> aln_seq;
    aln_seq.reset(CreateAlignRow(denseg, (CDense_seg::TDim)m_QueryIndex, (CDense_seg::TDim)m_SubjectIndex));

    _ASSERT(aln_seq.get());
    const SAlignedSeq::TAlignColl& coll = *aln_seq->m_AlignColl;

    ITERATE(SAlignedSeq::TAlignColl, it, coll)  {
        const SAlignedSeq::TAlignRange& range = *it;

        TRange q_r(range.GetFirstFrom(), range.GetFirstTo());
        TRange s_r(range.GetSecondFrom(), range.GetSecondTo());
        ENa_strand q_strand = eNa_strand_plus;
        ENa_strand s_strand = range.IsReversed() ? eNa_strand_minus : eNa_strand_plus;
        CDenseSegHitElement* elem = new CDenseSegHitElement(*this, q_r, s_r, q_strand, s_strand);

        m_Elements.push_back(elem);
    }
}


CDenseSegHit::~CDenseSegHit()
{
    for(size_t i = 0; i < m_Elements.size(); i++)   {
        delete m_Elements[i];
    }
}


CDenseSegHit::TDim CDenseSegHit::GetElemsCount() const
{
    return m_Elements.size();
}


const IHitElement& CDenseSegHit::GetElem(TDim elem_index) const
{
    _ASSERT(size_t(elem_index) < m_Elements.size());
    return *m_Elements[elem_index];
}


double CDenseSegHit::GetScoreValue(const string& score_name) const
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


const objects::CSeq_align* CDenseSegHit::GetSeqAlign() const
{
    return m_SeqAlign;
}


///////////////////////////////////////////////////////////////////////////////
///
CDenseSegHitElement::CDenseSegHitElement()
:   m_Hit(NULL)
{
}

CDenseSegHitElement::~CDenseSegHitElement()
{
}


CDenseSegHitElement::CDenseSegHitElement(const CDenseSegHit& hit,
                                         const TRange& q_r, const TRange& s_r,
                                         ENa_strand q_strand,
                                         ENa_strand s_strand)
:   m_Hit(&hit),
    m_QueryRange(q_r),
    m_SubjectRange(s_r),
    m_QueryStrand(q_strand),
    m_SubjectStrand(s_strand)
{

}


const IHit& CDenseSegHitElement::GetHit() const
{
    return *m_Hit;
}


TSignedSeqPos CDenseSegHitElement::GetQueryStart() const
{
    return m_QueryRange.GetFrom();
}


TSignedSeqPos CDenseSegHitElement::GetSubjectStart() const
{
    return m_SubjectRange.GetFrom();
}


TSeqPos CDenseSegHitElement::GetQueryLength() const
{
    return m_QueryRange.GetLength();
}


TSeqPos CDenseSegHitElement::GetSubjectLength() const
{
    return m_SubjectRange.GetLength();
}


objects::ENa_strand CDenseSegHitElement::GetQueryStrand() const
{
    return m_QueryStrand;
}


objects::ENa_strand CDenseSegHitElement::GetSubjectStrand() const
{
    return m_SubjectStrand;
}



END_NCBI_SCOPE
