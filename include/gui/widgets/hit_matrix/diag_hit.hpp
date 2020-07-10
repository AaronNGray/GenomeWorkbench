#ifndef GUI_WIDGETS_HIT_MATRIX___DIAG_HIT__HPP
#define GUI_WIDGETS_HIT_MATRIX___DIAG_HIT__HPP

/*  $Id: diag_hit.hpp 15168 2007-10-15 19:11:34Z dicuccio $
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
#include <objtools/alnmgr/pairwise_aln.hpp>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Score.hpp>

#include <serial/iterator.hpp>

#include <objmgr/scope.hpp>


BEGIN_NCBI_SCOPE

class CDiagHit;

///////////////////////////////////////////////////////////////////////////////
/// CDiagHitElement - IHitElement implementation for CDense_seg-based
/// alignments
class CDiagHitElement : public IHitElement
{
public:
    typedef CRange<TSignedSeqPos>   TRange;
    typedef CPairwiseAln::TAlnRng   TAlnRange;

    CDiagHitElement();
    virtual ~CDiagHitElement();

    void    Init(const CDiagHit* hit, size_t index);

    virtual const IHit&  GetHit() const;
    virtual TSignedSeqPos  GetQueryStart() const;
    virtual TSignedSeqPos  GetSubjectStart() const;
    virtual TSeqPos  GetQueryLength() const;
    virtual TSeqPos  GetSubjectLength() const;
    virtual objects::ENa_strand     GetQueryStrand() const;
    virtual objects::ENa_strand     GetSubjectStrand() const;

protected:
    const TAlnRange& x_GetAlnRange() const;

protected:
    const CDiagHit*  m_Hit;
    size_t  m_Index; // index of the element in the Hit
};



class CDiagHit : public IHit
{
public:
    friend class CDiagHitElement;

    typedef list< CRef< objects::CScore > >  TScore;
    typedef CDiagHitElement::TRange TRange;
    typedef CDiagHitElement::TAlnRange TAlnRange;

    CDiagHit(const objects::CSeq_align& seq_align, const CPairwiseAln& pr_align);
    virtual ~CDiagHit();

    virtual TDim   GetElemsCount() const;
    virtual const IHitElement& GetElem(TDim elem_index) const;

    virtual double   GetScoreValue(const string& score_name) const;
    virtual const objects::CSeq_align*   GetSeqAlign() const;

protected:
    const CPairwiseAln& x_GetAln() const    {   return *m_Aln;  }

protected:
    typedef vector<CDiagHitElement>    TElements;

    const objects::CSeq_align*   m_SeqAlign;
    CConstRef<CPairwiseAln>  m_Aln;
    TElements   m_Elements;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_HIT_MATRIX___DIAG_HIT__HPP
