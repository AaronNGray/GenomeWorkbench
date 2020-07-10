/*  $Id: diag_hit.cpp 17179 2008-06-23 17:34:14Z yazhuk $
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

#include <gui/widgets/hit_matrix/diag_hit.hpp>
#include <objects/general/Object_id.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


///////////////////////////////////////////////////////////////////////////////
/// CDiagHit
/*
bool CDiagHit::HasAlignment(const objects::CDense_seg& denseg,
                                size_t q_index, size_t s_index,
                                EOrientation orient)
{
    // check whether orientation of the specified rows matches "orient"
    bool reversed = false; // by default
    if(denseg.IsSetStrands())    {
        bool q_minus = denseg.GetSeqStrand(q_index) == eNa_strand_minus;
        bool s_minus = denseg.GetSeqStrand(s_index) == eNa_strand_minus;
        reversed = (q_minus != s_minus);
    }

    if((reversed  &&  orient == IHit::eDirect)  ||
       (! reversed  &&  orient == IHit::eReversed)) {
        return false; // this alignment does not qualify
    }

    // check whether do q_index and s_index define a pairwise alignment -
    // look for a column that has non-negative starts in both Q and S rows
    CDense_seg::TDim dim = denseg.GetDim();
    CDense_seg::TNumseg n_seg = denseg.GetNumseg();
    const CDense_seg::TStarts& starts = denseg.GetStarts();

    bool empty = true;
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
*/

CDiagHit::CDiagHit(const objects::CSeq_align& seq_align, const CPairwiseAln& pr_align)
:   m_SeqAlign(&seq_align),
    m_Aln(&pr_align)
{
    size_t n = pr_align.size();
    m_Elements.resize(n);
    for( size_t i = 0;  i < n; i++ )    {
        m_Elements[i].Init(this, i);
    }
}


CDiagHit::~CDiagHit()
{
}


CDiagHit::TDim CDiagHit::GetElemsCount() const
{
    return m_Elements.size();
}


const IHitElement& CDiagHit::GetElem(TDim elem_index) const
{
    _ASSERT(size_t(elem_index) < m_Elements.size());
    return m_Elements[elem_index];
}


double CDiagHit::GetScoreValue(const string& score_name) const
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


const objects::CSeq_align* CDiagHit::GetSeqAlign() const
{
    return m_SeqAlign;
}


///////////////////////////////////////////////////////////////////////////////
///
CDiagHitElement::CDiagHitElement()
:   m_Hit(NULL)
{
}

CDiagHitElement::~CDiagHitElement()
{
}


void CDiagHitElement::Init(const CDiagHit* hit, size_t index)
{
    m_Hit = hit;
    m_Index = index;
    _ASSERT(index < m_Hit->m_Aln->size());
}


const IHit& CDiagHitElement::GetHit() const
{
    return *m_Hit;
}

const CDiagHitElement::TAlnRange& CDiagHitElement::x_GetAlnRange() const
{
    return m_Hit->x_GetAln()[m_Index];
}


TSignedSeqPos CDiagHitElement::GetQueryStart() const
{
    return x_GetAlnRange().GetFirstFrom();
}

TSignedSeqPos CDiagHitElement::GetSubjectStart() const
{
    return x_GetAlnRange().GetSecondFrom();
}


TSeqPos CDiagHitElement::GetQueryLength() const
{
    return x_GetAlnRange().GetLength();
}


TSeqPos CDiagHitElement::GetSubjectLength() const
{
    return x_GetAlnRange().GetLength();
}


objects::ENa_strand CDiagHitElement::GetQueryStrand() const
{
    return eNa_strand_plus;
}


objects::ENa_strand CDiagHitElement::GetSubjectStrand() const
{
    return x_GetAlnRange().IsDirect() ? eNa_strand_plus : eNa_strand_minus;
}


END_NCBI_SCOPE
