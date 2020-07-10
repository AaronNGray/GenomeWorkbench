/*  $Id: denseg_ci.cpp 26225 2012-08-06 20:40:13Z wuliangs $
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

#include <objects/seqalign/Dense_seg.hpp>
#include <gui/widgets/seq_graphic/denseg_ci.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

////////////////////////////////////////////////////////////////////////////////
/// CDensegSegment - IAlnSegment implementation for CAlnMap::CAlnChunk

CDensegSegment::CDensegSegment(void)
    : m_Type(fInvalid),
      m_AlnRange(TSignedRange::GetEmpty()),
      m_RowRange(TSignedRange::GetEmpty())
{
}


CDensegSegment::operator bool(void) const
{
    return !IsInvalidType();
}


CDensegSegment::TSegTypeFlags CDensegSegment::GetType(void) const
{
    return m_Type;
}


const CDensegSegment::TSignedRange& CDensegSegment::GetAlnRange(void) const
{
    return m_AlnRange;
}


const CDensegSegment::TSignedRange& CDensegSegment::GetRange(void) const
{
    return m_RowRange;
}


////////////////////////////////////////////////////////////////////////////////
/// CDenseg_CI

void CDenseg_CI::x_InitSegment(void)
{
    TSignedRange& aln_rg = m_Segment.m_AlnRange;
    TSignedRange& row_rg = m_Segment.m_RowRange;

    const CSeq_align::TSegs::TDenseg& denseg = m_Aln->GetSegs().GetDenseg();
    const CSeq_align::TSegs::TDenseg::TStarts& starts = denseg.GetStarts();
    if (m_CurrSeg >= m_TotalSegs  ||  m_PreToPos_Anchor > m_TotalRange.GetToOpen() ||
        starts[2 * m_CurrSeg + m_AnchorRow] > m_TotalRange.GetTo()) {
        // End of the iterator
        m_Aln.Reset();
        aln_rg = TSignedRange::GetEmpty();
        row_rg = TSignedRange::GetEmpty();
        m_Segment.m_Type = IAlnSegment::fInvalid;
        return;
    }

    const CSeq_align::TSegs::TDenseg::TLens& lens = denseg.GetLens();
    TSignedSeqPos from = starts[2 * m_CurrSeg + m_AnchorRow];
    TSignedSeqPos f_off = 0;
    TSignedSeqPos t_off = 0;
    if (from != -1) {
        if (m_TotalRange.GetFrom() > from) {
            f_off = m_TotalRange.GetFrom() - from;
        }
        if (m_TotalRange.GetTo() < from + (TSignedSeqPos)(lens[m_CurrSeg] - 1)) {
            t_off = from + lens[m_CurrSeg] - 1 - m_TotalRange.GetTo();
        }
    }    

    if (m_AnchorRow == m_Row) {
        if (from == -1) {
            aln_rg = TSignedRange::GetEmpty();
            row_rg = TSignedRange::GetEmpty();
            m_Segment.m_Type = IAlnSegment::fIndel;
        } else {
            aln_rg.SetOpen(from, from + lens[m_CurrSeg]);
            row_rg = aln_rg;
            m_Segment.m_Type = IAlnSegment::fAligned;
        }
    } else {
        TSignedSeqPos row_from = starts[2 * m_CurrSeg + m_Row];
        if (from == -1) {
            //// figure out the indel position on anchor sequence
            //TSignedSeqPos anchor_pos = -1;
            //size_t pre_seg = m_CurrSeg - m_AnchorDirect;
            //size_t next_seg = m_CurrSeg + m_AnchorDirect;
            //if (pre_seg < m_TotalSegs  &&  starts[2 * pre_seg + m_AnchorRow] != -1) {
            //    anchor_pos = starts[2 * pre_seg + m_AnchorRow] + lens[pre_seg];
            //} else if (next_seg < m_TotalSegs  &&  starts[2 * next_seg + m_AnchorRow] != -1) {
            //    anchor_pos = starts[2 * next_seg + m_AnchorRow];
            //}

            //_ASSERT(anchor_pos != -1);
            //aln_rg.SetOpen(anchor_pos, anchor_pos);
            aln_rg.SetOpen(m_PreToPos_Anchor, m_PreToPos_Anchor);
            row_rg.SetOpen(row_from, row_from + lens[m_CurrSeg]);
            m_Segment.m_Type = IAlnSegment::fIndel;
        } else if (row_from == -1) {
            // figure out the indel position on selected row sequence
            TSignedSeqPos row_pos = -1;
            size_t pre_seg = m_CurrSeg - m_RowDirect;
            size_t next_seg = m_CurrSeg + m_RowDirect;
            if (pre_seg < m_TotalSegs  &&  starts[2 * pre_seg + m_Row] != -1) {
                row_pos = starts[2 * pre_seg + m_Row] + lens[pre_seg];
            } else if (next_seg < m_TotalSegs  &&  starts[2 * next_seg + m_Row] != -1) {
                row_pos = starts[2 * next_seg + m_Row];
            }

            _ASSERT(row_pos != -1);
            row_rg.SetOpen(row_pos + 1, row_pos + 1);
            m_PreToPos_Anchor = from + lens[m_CurrSeg];
            aln_rg.SetOpen(from + f_off, m_PreToPos_Anchor - t_off);
            m_Segment.m_Type = IAlnSegment::fIndel;
        } else {
            m_PreToPos_Anchor = from + lens[m_CurrSeg];
            aln_rg.SetOpen(from + f_off, m_PreToPos_Anchor - t_off);
            if (m_RowDirect != m_AnchorDirect) {
                row_rg.SetOpen(row_from + t_off, row_from + lens[m_CurrSeg] - f_off);
            } else {
                row_rg.SetOpen(row_from + f_off, row_from + lens[m_CurrSeg] - t_off);
            }
            m_Segment.m_Type = IAlnSegment::fAligned;
        }
    }

    // The flag shows relative row direction.
    if (m_RowDirect != m_AnchorDirect) {
        m_Segment.m_Type |= IAlnSegment::fReversed;
    }
}

// assuming clipping range
void CDenseg_CI::x_InitIterator(void)
{
    const CSeq_align::TSegs::TDenseg& denseg = m_Aln->GetSegs().GetDenseg();
    m_TotalSegs = denseg.GetNumseg();

    if (m_Row >= 2  ||  m_AnchorRow >= 2) {
        // Invalid row input - nothing to iterate.
        m_Aln.Reset();
        return;
    }

    // Remember the first one and use for all segments.
    m_AnchorDirect = m_Aln->GetSeqStrand(m_AnchorRow) == eNa_strand_minus ? -1 : 1;
    m_RowDirect = m_Aln->GetSeqStrand(m_Row) == eNa_strand_minus ? -1 : 1;

    m_Segment.m_AlnRange = TSignedRange::GetEmpty();
    m_Segment.m_RowRange = TSignedRange::GetEmpty();

    if (m_AnchorDirect == -1) {
        m_CurrSeg = m_TotalSegs - 1;
    } else {
        m_CurrSeg = 0;
    }

    const CSeq_align::TSegs::TDenseg::TStarts& starts = denseg.GetStarts();
    const CSeq_align::TSegs::TDenseg::TLens& lens = denseg.GetLens();
    m_PreToPos_Anchor = m_Aln->GetSeqStart(m_AnchorRow);
    TSignedSeqPos r_from = m_TotalRange.GetFrom();
    if ( !m_TotalRange.IsWhole()  &&  m_PreToPos_Anchor < r_from ) {
        while (m_CurrSeg < m_TotalSegs  &&
            (starts[2 * m_CurrSeg + m_AnchorRow] == -1  ||
            r_from >= starts[2 * m_CurrSeg + m_AnchorRow] + (TSignedSeqPos)lens[m_CurrSeg]) ) {
                if (starts[2 * m_CurrSeg + m_AnchorRow] != -1) {
                    m_PreToPos_Anchor = starts[2 * m_CurrSeg + m_AnchorRow] + lens[m_CurrSeg];
                }
                m_CurrSeg += m_AnchorDirect;
        }
    }

    x_InitSegment();
    x_CheckSegment();
}


void CDenseg_CI::x_CheckSegment(void)
{
    if (m_Flags == eAllSegments) {
        return;
    }

    while ( *this ) {
        if (m_Segment.m_AlnRange.NotEmpty()  ||  m_Segment.m_RowRange.NotEmpty()) {
            if (m_Flags == eSkipGaps) {
                if ( m_Segment.IsAligned() ) {
                    break;
                }
            } else {
                // Distinguish between insertions and deletions.
                bool ins = (m_Segment.m_Type & IAlnSegment::fIndel) != 0  &&
                    m_Segment.m_AlnRange.Empty();
                if ((m_Flags == eInsertsOnly  &&  ins)  ||
                    (m_Flags == eSkipInserts  &&  !ins)) {
                        break;
                }
            }
        }
        x_NextSegment();
    }
}


void CDenseg_CI::x_NextSegment(void)
{
    if ( !*this ) return;
    m_CurrSeg += m_AnchorDirect;
    x_InitSegment();
}


bool CDenseg_CI::x_Equals(const CDenseg_CI& other) const
{
    return m_Aln == other.m_Aln  &&
        m_Flags == other.m_Flags  &&
        m_Row == other.m_Row  &&
        m_AnchorRow == other.m_AnchorRow  &&
        m_TotalRange == other.m_TotalRange  &&
        m_CurrSeg == other.m_CurrSeg  &&
        m_TotalSegs == other.m_TotalSegs  &&
        m_Segment == other.m_Segment;
}


CDenseg_CI::CDenseg_CI(void)
    : m_Flags(eAllSegments)
    , m_Aln(NULL)
    , m_AnchorRow(0)
    , m_Row(0)
    , m_CurrSeg(0)
    , m_TotalSegs(0)
    , m_AnchorDirect(1)
    , m_RowDirect(1)
    , m_PreToPos_Anchor(-1)
{
    m_Segment.m_AlnRange = TSignedRange::GetEmpty();
    m_Segment.m_RowRange = TSignedRange::GetEmpty();
    m_Segment.m_Type = IAlnSegment::fInvalid;
}


CDenseg_CI::CDenseg_CI(const CSeq_align& aln,
                       TNumrow row,
                       TNumrow anchor_row,
                       EFlags flags)
    : m_Flags(flags)
    , m_Aln(&aln)
    , m_AnchorRow(anchor_row)
    , m_Row(row)
    , m_TotalRange(TSignedRange::GetWhole())
{
    x_InitIterator();
}


CDenseg_CI::CDenseg_CI(const CSeq_align& aln,
                       TNumrow row,
                       TNumrow anchor_row,
                       EFlags flags,
                       const TSignedRange& range)
    : m_Flags(flags)
    , m_Aln(&aln)
    , m_AnchorRow(anchor_row)
    , m_Row(row)
    , m_TotalRange(range)
{
    x_InitIterator();
}


CDenseg_CI::CDenseg_CI(const CDenseg_CI& orig)
{
    *this = orig;
}


CDenseg_CI::~CDenseg_CI(void)
{
}


IAlnSegmentIterator* CDenseg_CI::Clone(void) const
{
    return new CDenseg_CI(*this);
}


CDenseg_CI::operator bool(void) const
{
    return m_Aln  &&  m_Segment;
}


IAlnSegmentIterator& CDenseg_CI::operator++(void)
{
    x_NextSegment();
    x_CheckSegment();
    return *this;
}


bool CDenseg_CI::operator==(const IAlnSegmentIterator& it) const
{
    if(typeid(*this) == typeid(it)) {
        const CDenseg_CI* denseg_it = dynamic_cast<const CDenseg_CI*>(&it);
        return x_Equals(*denseg_it);
    }
    return false;
}


bool CDenseg_CI::operator!=(const IAlnSegmentIterator& it) const
{
    if(typeid(*this) == typeid(it)) {
        const CDenseg_CI* denseg_it = dynamic_cast<const CDenseg_CI*>(&it);
        return !x_Equals(*denseg_it);
    }
    return true;
}


const CDenseg_CI::value_type& CDenseg_CI::operator*(void) const
{
    _ASSERT(*this);
    return m_Segment;
}


const CDenseg_CI::value_type* CDenseg_CI::operator->(void) const
{
    _ASSERT(*this);
    return &m_Segment;
}


END_NCBI_SCOPE
