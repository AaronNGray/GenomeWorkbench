/*  $Id: pw_alignment.cpp 15908 2008-02-07 17:58:44Z dicuccio $
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
 * Authors:  Vlad Lebedev, Liangshou Wu
 *
 * File Description:
 *   CLayoutPWAlign -- utility class to layout pair-wise alignments
 */

#include <ncbi_pch.hpp>
#include <gui/objutils/pw_alignment.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CLayoutPWAlign::CLayoutPWAlign(CConstRef<IAlnGraphicDataSource> aln_mgr,
                               const CSeq_align_Handle& orig_align,
                               const CSeq_align& mapped_align)
    : CLayoutAlign(aln_mgr, orig_align, mapped_align)
{
}


CLayoutPWAlign::EType CLayoutPWAlign::GetType() const
{
    return eAlignPairwise;
}


void CLayoutPWAlign::x_CalcIntervals(void) const
{
    m_Intervals.clear();

    const IAlnGraphicDataSource& aln_mgr = GetAlignMgr();
    CAlnMap::TNumrow anchor = aln_mgr.GetAnchor();
    TSignedSeqRange range(aln_mgr.GetAlnStart(), aln_mgr.GetAlnStop());

    for (CAlnMap::TNumrow row = 0 ;  row < aln_mgr.GetNumRows();  ++row) {
        if (row == anchor) {
            continue;  // skip master seq
        }

        auto_ptr<IAlnSegmentIterator> p_it(aln_mgr.CreateSegmentIterator(row,
            range, IAlnSegmentIterator::eSkipGaps));

        for ( IAlnSegmentIterator& it(*p_it);  it;  ++it )  {
            const IAlnSegment& seg = *it;
            const TSignedSeqRange& R = seg.GetRange();
            TSeqPos start = R.GetFrom();
            TSeqPos stop = R.GetTo();
            start = aln_mgr.GetSeqPosFromSeqPos(anchor, row, start);
            stop = aln_mgr.GetSeqPosFromSeqPos(anchor, row, stop);
            if (start > stop) {
                swap(start, stop);
            }
            m_Intervals.push_back( TSeqRange(start, stop) );
        }
    } // for (CAlnMap::TNumrow ...
}


END_NCBI_SCOPE

