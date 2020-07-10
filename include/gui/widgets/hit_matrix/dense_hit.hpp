#ifndef GUI_WIDGETS_HIT_MATRIX___DENSE_HIT__HPP
#define GUI_WIDGETS_HIT_MATRIX___DENSE_HIT__HPP

/*  $Id: dense_hit.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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

#include <corelib/ncbistd.hpp>

#include <gui/widgets/hit_matrix/hit.hpp>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqalign/Score.hpp>


#include <serial/iterator.hpp>

#include <objmgr/scope.hpp>

BEGIN_NCBI_SCOPE

class CDenseSegHit;

///////////////////////////////////////////////////////////////////////////////
/// CDenseSegHitElement - IHitElement implementation for CDense_seg-based
/// alignments
class CDenseSegHitElement : public IHitElement
{
public:
    typedef CRange<TSignedSeqPos>   TRange;

    CDenseSegHitElement();
    CDenseSegHitElement(const CDenseSegHit& hit,
                        const TRange& q_r,
                        const TRange& s_r,
                        objects::ENa_strand q_strand,
                        objects::ENa_strand s_strand);
    virtual ~CDenseSegHitElement();

    virtual const IHit&  GetHit() const;
    virtual TSignedSeqPos  GetQueryStart() const;
    virtual TSignedSeqPos  GetSubjectStart() const;
    virtual TSeqPos  GetQueryLength() const;
    virtual TSeqPos  GetSubjectLength() const;
    virtual objects::ENa_strand     GetQueryStrand() const;
    virtual objects::ENa_strand     GetSubjectStrand() const;

protected:
    const CDenseSegHit*  m_Hit;
    TRange  m_QueryRange;
    TRange  m_SubjectRange;
    objects::ENa_strand  m_QueryStrand;
    objects::ENa_strand  m_SubjectStrand;
};



/// Wraps a CSeq_align containing CDense_seg and provides a simple API for
/// interpreting it as a pairwise alignment of two sequences.
///

class CDenseSegHit : public IHit
{
public:
    typedef list< CRef< objects::CScore > >  TScore;
    typedef CDenseSegHitElement::TRange TRange;

    // returns "true" if an alignment can be created from the specified rows of
    // the given alignment and with the required orientation
    static bool HasAlignment(const objects::CDense_seg& denseg,
                            size_t q_index, size_t s_index,
                            TDirection dir);

    CDenseSegHit(const objects::CSeq_align& align, int q_index, int s_index);
    virtual ~CDenseSegHit();

    virtual TDim   GetElemsCount() const;
    virtual const IHitElement& GetElem(TDim elem_index) const;

    virtual double   GetScoreValue(const string& score_name) const;
    virtual const objects::CSeq_align*   GetSeqAlign() const;

protected:
    typedef vector<CDenseSegHitElement*>    TElements;

    const objects::CSeq_align*   m_SeqAlign;
    TDim    m_QueryIndex;
    TDim    m_SubjectIndex;
    TElements   m_Elements;
};



END_NCBI_SCOPE

#endif  // GUI_WIDGETS_HIT_MATRIX___DENSE_HIT__HPP
