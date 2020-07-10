#ifndef GUI_WIDGETS_HIT_MATRIX___STD_SEG_HIT__HPP
#define GUI_WIDGETS_HIT_MATRIX___STD_SEG_HIT__HPP

/*  $Id: std_seg_hit.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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

//TODO clean this
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Std_seg.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqalign/Score.hpp>
#include <objects/general/Object_id.hpp>

#include <serial/iterator.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>

BEGIN_NCBI_SCOPE

class CStdSegHit;

///////////////////////////////////////////////////////////////////////////////
/// CStdSegHitElement - IHitElement implementation for CStd_seg-based
/// alignments
class CStdSegHitElement : public IHitElement
{
public:
    typedef CRange<TSignedSeqPos>   TRange;

    CStdSegHitElement();
    CStdSegHitElement(const CStdSegHit& hit,
                        const TRange& q_r,
                        const TRange& s_r,
                        objects::ENa_strand q_strand,
                        objects::ENa_strand s_strand);
    virtual ~CStdSegHitElement();

    virtual const IHit&  GetHit() const;
    virtual TSignedSeqPos  GetQueryStart() const;
    virtual TSignedSeqPos  GetSubjectStart() const;
    virtual TSeqPos  GetQueryLength() const;
    virtual TSeqPos  GetSubjectLength() const;
    virtual objects::ENa_strand     GetQueryStrand() const;
    virtual objects::ENa_strand     GetSubjectStrand() const;

protected:
    const CStdSegHit*  m_Hit;
    TRange  m_QueryRange;
    TRange  m_SubjectRange;
    objects::ENa_strand  m_QueryStrand;
    objects::ENa_strand  m_SubjectStrand;
};



/// Wraps a CSeq_align containing CStd_seg and provides a simple API for
/// interpreting it as a pairwise alignment of two sequences.
///

class CStdSegHit : public IHit
{
public:
    typedef list< CRef< objects::CScore > >  TScore;
    typedef CStdSegHitElement::TRange TRange;
    typedef objects::CSeq_align::TSegs::TStd TStd;

    // returns "true" if an alignment can be created from the specified rows of
    // the given alignment and with the required orientation
    static bool HasAlignment(const TStd& std_list,
                            size_t q_index, size_t s_index,
                            TDirection dir);

    CStdSegHit(const objects::CSeq_align& align, int q_index, int s_index);
    virtual ~CStdSegHit();

    virtual TDim   GetElemsCount() const;
    virtual const IHitElement& GetElem(TDim elem_index) const;

    virtual double   GetScoreValue(const string& score_name) const;
    virtual const objects::CSeq_align*   GetSeqAlign() const;

protected:
    typedef vector<CStdSegHitElement*>    TElements;

    CConstRef<objects::CSeq_align>  m_SeqAlign;
    TDim    m_QueryIndex;
    TDim    m_SubjectIndex;
    TElements   m_Elements;
};



END_NCBI_SCOPE

#endif  // GUI_WIDGETS_HIT_MATRIX___STD_SEG_HIT__HPP
