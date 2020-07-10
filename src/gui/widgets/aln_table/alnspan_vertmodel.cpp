/*  $Id: alnspan_vertmodel.cpp 37049 2016-11-30 19:40:50Z shkeda $
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
 * Authors:  Yury Voronov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistre.hpp>

#include <algo/sequence/consensus_splice.hpp>

#include <gui/widgets/aln_table/alnspan_vertmodel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <objtools/alnmgr/aln_explorer.hpp>
#include <objtools/alnmgr/aln_container.hpp>
#include <objtools/alnmgr/aln_converters.hpp>
#include <objtools/alnmgr/aln_builders.hpp>
#include <objtools/alnmgr/alnvec.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/seq_vector.hpp>

#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Score.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqalign/Spliced_exon.hpp>
#include <objects/seqalign/Spliced_exon_chunk.hpp>
#include <objects/seqalign/Product_pos.hpp>
#include <objects/seqalign/Prot_pos.hpp>
#include <objects/general/Object_id.hpp>
#include <gui/objutils/label.hpp>
#include <serial/iterator.hpp>

#include <math.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

#define INIT_MEMBERS \
    m_Threshold( 0 ) \
    , mf_MarkIndels( fGap ) \
    , m_NumSeqs ( 0 ) \
    , mf_UseSpliced( false ) \
    , mf_UseSparse( false ) \
    , mf_ShowIndels( true ) \
    , mf_RowsInvalidated( false )


CAlnSpanVertModel::CAlnSpanVertModel()
    : INIT_MEMBERS
{
    x_Init();
}

/*
CAlnSpanVertModel::CAlnSpanVertModel( IAlnMultiDataSource& source )
    : INIT_MEMBERS
{
    x_Init( source );

    UpdateRows();
}

CAlnSpanVertModel::CAlnSpanVertModel( CSparseAln& source )
    : INIT_MEMBERS
{
    x_Init( source );

    UpdateRows();
}
*/

CAlnSpanVertModel::CAlnSpanVertModel( IAlnMultiDataSource& source, int threshold )
    : INIT_MEMBERS
{
    x_Init( source, threshold );

    UpdateRows();
}

CAlnSpanVertModel::CAlnSpanVertModel( CSparseAln& sparce_aln, int threshold )
    : INIT_MEMBERS
{
    x_Init( sparce_aln, threshold );

    UpdateRows();
}

CAlnSpanVertModel::CAlnSpanVertModel( const CSpliced_seg& spliced_seg, CScope& scope, int threshold )
: INIT_MEMBERS
{
    x_Init( spliced_seg, scope, threshold );

    UpdateRows();
}

CAlnSpanVertModel::CAlnSpanVertModel( vector< CConstRef<CSeq_align> >& input_aligns, CScope& scope, int threshold )
: INIT_MEMBERS
{
    if( input_aligns.size() == 1 && input_aligns[0]->GetSegs().IsSpliced() ){
        m_SplicedSeg.Reset( 
            const_cast<CSpliced_seg*>( &input_aligns[0]->GetSegs().GetSpliced() )
        );
        mf_UseSpliced = true;
    }

    CAlnContainer container;

    if( input_aligns.size() > 0 ){
        ITERATE( vector< CConstRef<CSeq_align> >, ait, input_aligns ){
            container.insert( **ait );
        }

    } else {
        _ASSERT(false);
    }

    /// Create a vector of seq-ids per seq-align
    TIdExtract id_extract;
    TAlnIdMap aln_id_map( id_extract, container.size() );
    ITERATE( CAlnContainer, aln_it, container ){
        try {
            aln_id_map.push_back( **aln_it );

        } catch( CAlnException e ){
            LOG_POST( Error << "Skipping this alignment: " << e.what() );
        }
    }

    /// Create align statistics object
    TAlnStats aln_stats( aln_id_map );

    /// Can the alignments be anchored?
    if ( !aln_stats.CanBeAnchored() ) {
        LOG_POST( Error << "Input alignments cannot be anchored because they don't share at least one common sequence." );
        NCBI_THROW(
            CException,
            eUnknown, "Input alignments cannot be anchored because they don't share at least one common sequence."
        );
    }

    /// Create user options
    CAlnUserOptions aln_user_options;

    /// Construct a vector of anchored alignments
    TAnchoredAlnVec anchored_aln_vec;
    CreateAnchoredAlnVec( aln_stats, anchored_aln_vec, aln_user_options );

    BuildAln( anchored_aln_vec, m_AnchoredAln, aln_user_options );

    if( m_AnchoredAln.GetDim() == 1 ){
        m_AnchoredAln = CAnchoredAln();

        aln_user_options.m_MergeAlgo = CAlnUserOptions::ePreserveRows;

        BuildAln( anchored_aln_vec, m_AnchoredAln, aln_user_options );
    }

    /// Get sequence:
    CRef<CSparseAln> sparse_aln;
    sparse_aln.Reset( new CSparseAln( m_AnchoredAln, scope ) );

    x_Init( *sparse_aln, threshold );

//    UpdateRows();
}

void CAlnSpanVertModel::x_Init()
{
    //LOG_POST( Info << "ASWM::x_Init: with no source..." );

    for( int i = 0; i < eMaxExtraCols; i++ ){
        switch( i ){
        case eLengthCol:
            m_ColNames.push_back( "Length" );
            break;
        case eIdenticalCol:
            m_ColNames.push_back( "% Identity" );
            break;
        case eTypeCol:
            m_ColNames.push_back( "Type" );
            break;
        case eMismatchCountCol:
            m_ColNames.push_back( "Mismatch" );
            break;
        case eGapLengthCol:
            m_ColNames.push_back( "Gap" );
            break;
        default:
            _ASSERT( false ); // Unknown column
        }
    }
    //m_ColNames.push_back( "Aln Range" );

    //LOG_POST( Info << "ASWM::x_Init: threshhold = " << m_Threshold );
    //LOG_POST( Info << "ASWM::x_Init: num of seqs = " << m_NumSeqs );
    //LOG_POST( Info << "ASWM::x_Init: use sparse = " << mf_UseSparse );
}

void CAlnSpanVertModel::x_Init( IAlnMultiDataSource& aDataSource, int aThresh )
{
    //LOG_POST( Info << "ASWM::x_Init: with a data source..." );

    m_AlnSrc = &aDataSource;
    m_Threshold = aThresh;

    m_NumSeqs = m_AlnSrc->GetNumRows();

    for( int row = 0; row < m_NumSeqs; row++ ){
        const IAlignRowHandle* handle = aDataSource.GetRowHandle( row );
        string label;

        CLabel::GetLabel(
            *handle -> GetBioseqHandle().GetSeqId(),
            &label, CLabel::eDefault,
            &handle->GetBioseqHandle().GetScope()
        );

        m_ColNames.push_back( label );
    }

    x_Init();

    mf_RowsInvalidated = true;
}

void CAlnSpanVertModel::x_Init( CSparseAln& aSparseAln, int aThresh )
{
    //LOG_POST( Info << "ASVM::x_Init: with a sparse aln..." );

    //m_SparseAln = &aSparseAln;
    m_SparseAlns.push_back( CRef<CSparseAln>( &aSparseAln ) );
    m_Threshold = aThresh;

    m_NumSeqs = m_SparseAlns[0]->GetNumRows();
    mf_UseSparse = true;

    for( int row = 0; row < m_NumSeqs; row++ ){
        string label;

        CLabel::GetLabel(
            m_SparseAlns[0]->GetSeqId( row ),
            &label, CLabel::eDefault,
            m_SparseAlns[0]->GetScope()
        );

        m_ColNames.push_back( label );
    }

    x_Init();

    mf_RowsInvalidated = true;
}

void CAlnSpanVertModel::x_Init( const CSpliced_seg& spliced_seg, CScope& scope, int aThresh )
{
    m_SplicedSeg.Reset( const_cast<CSpliced_seg*>(&spliced_seg) );
    m_Scope.Reset( &scope );
    m_Threshold = aThresh;

    m_NumSeqs = m_SplicedSeg->CheckNumRows(); // always 2 
    mf_UseSpliced = true;

    string label;

    CLabel::GetLabel(
        m_SplicedSeg->GetProduct_id(),
        &label, CLabel::eDefault,
        m_Scope
    );
    m_ColNames.push_back( label );

    CLabel::GetLabel(
        m_SplicedSeg->GetGenomic_id(),
        &label, CLabel::eDefault,
        m_Scope
    );
    m_ColNames.push_back( label );

    x_Init();

    mf_RowsInvalidated = true;
}


void CAlnSpanVertModel::x_Clear()
{
    m_AlnSrc.Reset();
    //m_SparseAln.Reset();
    m_SparseAlns.clear();

    m_AlnSpans.clear();
    m_ColNames.clear();

    m_Threshold = 0;
    m_NumSeqs = 0;
}


void CAlnSpanVertModel::x_CreateRows()
{
    TAlnSpans aln_spans;

    if (mf_UseSparse) {
        m_AlnRange.SetFrom(m_SparseAlns[0]->GetAlnRange().GetFrom());
        m_AlnRange.SetTo(m_SparseAlns[0]->GetAlnRange().GetTo());
        if (m_SparseAlns[0]->IsTranslated())
            m_BaseWidth = 3;
    } else {
        m_AlnRange.SetFrom(m_AlnSrc->GetAlnStart());
        m_AlnRange.SetTo(m_AlnSrc->GetAlnStop());
        if (m_AlnSrc->GetAlignType() == IAlnExplorer::fMixed)
            m_BaseWidth = 3;
    }

    if (mf_UseSpliced) {
        x_PopulateRowsSpliced(aln_spans);
    } else {
        x_PopulateRows(aln_spans);
    }
    x_InsertIntrons(aln_spans);
    x_MergeRows(aln_spans);
    ///
    /// finally, convert our spans into rows for display
    ///
    x_ConvertRowsToSpans(aln_spans);
    ///
    /// last stage: final book keeping
    /// we need to make sure that all spans have correct meta-information
    ///
    x_UpdateMetaData();
    ///
    /// after last stage: caching locs in span rows
    ///
    x_UpdateLocs();
}

void CAlnSpanVertModel::x_PopulateRows(TAlnSpans& aln_spans)
{
    vector< IAlnSegmentIterator* > iters;
    iters.reserve(m_NumSeqs);
    IAlnSegmentIterator::EFlags flags = IAlnSegmentIterator::eAllSegments;

    if (mf_UseSparse) {
        for (int row = 0; row < m_NumSeqs; row++) {
            iters.push_back(m_SparseAlns[0]->CreateSegmentIterator(row, m_AlnRange, flags));
        }
    } else {
        for (int row = 0; row < m_NumSeqs; row++) {
            iters.push_back(m_AlnSrc->CreateSegmentIterator(row, m_AlnRange, flags));
        }
    }

    ///
    /// first, scan for mismatches and gaps
    ///

    vector<bool> seq_touched(m_NumSeqs, false);

    TSignedSeqPos pos = m_AlnRange.GetFrom();
    while (pos <= m_AlnRange.GetTo()) {
        int next_pos = CRange<TSignedSeqPos>::GetPositionMax();

        IAlnExplorer::TNumrow row;

        for (row = 0; row < m_NumSeqs; ++row) {
            IAlnSegmentIterator& it = *iters[row];
            if (!it)
                continue;
            const IAlnSegment::TSignedRange& a_range = it->GetAlnRange();

            while (a_range.GetTo() < (int)pos  &&  ++it) {
                if (!seq_touched[row])
                    seq_touched[row] = true;
                continue;
            }
            if (!it)
                continue;

            TSignedSeqPos from = a_range.GetFrom();
            TSignedSeqPos too = a_range.GetToOpen();
            if (from > pos) {
                if (from < next_pos)
                    next_pos = from;
            } else if (too < next_pos) {
                next_pos = too;
            }
        }
        TSeqPos off = 0;
        if (m_BaseWidth == 3) {
            if (pos % 3 > 0) {
                off = pos % 3;
                pos -= pos % 3;
            }
        }

        vector<string> seq_spans;

        ///
        /// scan to see if we have a gap
        /// if no gaps, so we inspect for mismatches
        /// we will break this range into match and mismatch segments
        ///
        bool seg_has_gap = false;
        bool seg_has_unaln = false;
        bool seg_is_translated = false;

        for (int row = 0; row < m_NumSeqs; row++) {
            IAlnSegmentIterator& it = *iters[row];
            if (!it)
                continue;
            if (mf_UseSparse) {
                seg_is_translated = m_SparseAlns[0]->IsTranslated()
                    && m_SparseAlns[0]->GetBaseWidth(row) == 1;
            } else {
                seg_is_translated = m_AlnSrc->GetAlignType() == IAlnExplorer::fMixed
                    && !m_AlnSrc->GetRowHandle(row)->UsesAATranslation();
            }

            if (it->GetAlnRange().GetFrom() > pos + off) {
                seg_has_unaln = seq_touched[row];
                continue;
            } else if (it->GetType() &(IAlnSegment::fGap | IAlnSegment::fIndel)) {
                seg_has_gap = true;
                continue;
            } //else//
            //TSignedSeqPos aln_from = it->GetAlnRange().GetFrom();
            //TSignedSeqPos aln_to = it->GetAlnRange().GetTo();
            bool is_aa = false;

            if (mf_UseSparse || m_AlnSrc->GetRowHandle(row)->CanGetSeqString()) {

                CRange<TSignedSeqPos> seq_range(pos, next_pos - 1);

                string seq_span;
                if (mf_UseSparse) {
                    m_SparseAlns[0]->GetAlnSeqString(row, seq_span, seq_range, seg_is_translated);
                } else {
                    m_AlnSrc->GetRowHandle(row)->GetAlnSeqString(seq_span, seq_range);
                    if (seg_is_translated) {
                        string new_seq;
                        CSparseAln::TranslateNAToAA(seq_span, new_seq, m_AlnSrc->GetGenCode(row));
                        seq_span.swap(new_seq);
                    }
                }
                // we collect non-empty strings only
                if (!seq_span.empty()) {
                    seq_spans.push_back(seq_span);
                }
            }
        }

        if (seq_spans.size() <= 1) {

            if (seq_spans.empty()) {
                LOG_POST(Warning << "All gaps/discontinuity for aln range [ " << pos << ", " << (next_pos - 1) << "]");
            }

            SSpanRow newRow;
            newRow.aln_range = TSeqRange(pos, next_pos - 1);
            newRow.length = newRow.aln_range.GetLength();

            newRow.type = (seg_has_gap ? fGap : 0) | (seg_has_unaln ? fDiscontig : 0);

            if (newRow.type == 0) {
                newRow.type = (m_NumSeqs == 1) ? fAligned : fInvalid;

            } else {
                newRow.gap = newRow.aln_range.GetLength();
                if (!seg_is_translated && m_BaseWidth == 3)
                    newRow.gap /= 3;
            }

            aln_spans.push_back(newRow);
            pos = next_pos;

            continue;
        }
        /*
                if (base_width == 3) {
                // adjust start/stop to trim incomplete codons
                // GetAlnSeqString does the same
                if (pos % 3 > 0) {
                pos -= pos % 3;
                pos += 3; // skip incomplete codon
                }

                if (next_pos > pos && next_pos % 3 > 0)
                next_pos -= next_pos % 3; // skip incomplete codon
                }
        */
        int length = next_pos - pos;
        length /= m_BaseWidth;
        int size = (int)seq_spans.size();

        int f_span_type = fInvalid;
        int span_start = 0;

        int i = -1;
        do {
            int new_span_type = fInvalid;
            if (++i < length) {
                bool f_mismatch = false;
                for (int j = 1; j < size; j++) {
                    _ASSERT(i < (int)seq_spans[j].length());
                    if (seq_spans[j][i] != seq_spans[0][i]) {
                        f_mismatch = true;
                        break;
                    }
                }
                new_span_type = f_mismatch ? fMismatch : fAligned;
            }

            if (new_span_type != f_span_type) {

                if (f_span_type != fInvalid) {
                    SSpanRow newRow;
                    newRow.aln_range = TSeqRange(
                        pos + span_start * m_BaseWidth,
                        max(pos + span_start * m_BaseWidth, pos + (i * m_BaseWidth) - 1));
                    // should be always pos +i -1
                    newRow.length = newRow.aln_range.GetLength() / m_BaseWidth;
                    if (span_start > i - 1)
                        LOG_POST(Error << "Miscalculating span: pos=" << pos << ", start=" << span_start << ", oend=" << i);
                    newRow.type = f_span_type;
                    newRow.mismatch = f_span_type == fMismatch ? (newRow.aln_range.GetLength() / m_BaseWidth) : 0;
                    newRow.type |= (seg_has_gap ? fGap : 0) | (seg_has_unaln ? fDiscontig : 0);
                    aln_spans.push_back(newRow);
                }

                span_start = i;
                f_span_type = new_span_type;
            }
        } while (i < length);

        pos = next_pos;
     }

     for (int row_to_delete = 0; row_to_delete < m_NumSeqs; row_to_delete++) {
         delete iters[row_to_delete];
     }
     ///
     /// Preliminary span set
     ///
     TAlnSpans::iterator iter = aln_spans.begin();
     for (; iter != aln_spans.end();) {

         for (size_t row = iter->ranges.size(); (int)row < m_NumSeqs; row++) {

             TSignedSeqPos from, to;
             bool is_aa = false;
             if (mf_UseSparse) {
                 from = m_SparseAlns[0]->GetSeqPosFromAlnPos((int)row, iter->aln_range.GetFrom());
                 to = m_SparseAlns[0]->GetSeqPosFromAlnPos((int)row, iter->aln_range.GetTo());
                 is_aa = m_SparseAlns[0]->GetBaseWidth(row) == 3;
             } else {
                 from = m_AlnSrc->GetSeqPosFromAlnPos((int)row, iter->aln_range.GetFrom());
                 to = m_AlnSrc->GetSeqPosFromAlnPos((int)row, iter->aln_range.GetTo());
                 is_aa = m_AlnSrc->GetRowHandle(row)->UsesAATranslation();
             }

             if (from == -1 || to == -1) {
                 iter->ranges.push_back(TSignedSeqRange::GetEmpty());
             } else {
                 if (from > to) {
                     swap(from, to);
                 }
                 if (is_aa) {
                     from /= m_BaseWidth;
                     to /= m_BaseWidth;
                 } else if (iter->type & fGap) {
                     iter->length /= m_BaseWidth;
                     iter->gap /= m_BaseWidth;
                 }
                 iter->ranges.push_back(TSignedSeqRange(from, to));
             }
         }

         ++iter;
     }

}


void CAlnSpanVertModel::x_InsertIntrons(TAlnSpans& rows)
{
    ///
    /// insert extra gaps, if needed
    ///
    if (!mf_ShowIndels)
        return;
    vector<bool> seq_strand(m_NumSeqs, true);

    if (mf_UseSparse) {
        for (int seq_ix = 0; seq_ix < m_NumSeqs; seq_ix++) {
            seq_strand[seq_ix] = m_SparseAlns[0]->IsPositiveStrand(seq_ix);
        }
    } else {
        for (int seq_ix = 0; seq_ix < m_NumSeqs; seq_ix++) {
            seq_strand[seq_ix] = m_AlnSrc->IsPositiveStrand(seq_ix);
        }
    }

    vector<TSignedSeqPos> seq_pos(m_NumSeqs, -1);

    NON_CONST_ITERATE(TAlnSpans, iter, rows)
    {

        for (int seq_ix = 0; seq_ix < m_NumSeqs; seq_ix++) {
            SSpanRow& span_row = *iter;

            if (!span_row.ranges[seq_ix].Empty()) {

                TSignedSeqPos cur_seq_pos = seq_pos[seq_ix];
                TSignedSeqPos cur_span_from = span_row.ranges[seq_ix].GetFrom();
                TSignedSeqPos cur_span_to = span_row.ranges[seq_ix].GetTo();

                string splice3, splice5;

                if (seq_strand[seq_ix]) {

                    if (cur_seq_pos > 0 && cur_span_from > cur_seq_pos) {
                        SSpanRow gap_row;

                        gap_row.ranges.assign(m_NumSeqs, TSignedSeqRange());
                        gap_row.ranges[seq_ix] =
                            TSignedSeqRange(seq_pos[seq_ix], cur_span_from - 1)
                            ;

                        gap_row.type = mf_MarkIndels;
                        gap_row.length = gap_row.gap = gap_row.ranges[seq_ix].GetLength();
                        gap_row.mismatch = 0;
                        gap_row.identity = 0.0;

                        if (mf_MarkIndels == fIntron && mf_UseSparse) {
                            string intron;
                            m_SparseAlns[0]->GetSeqString(
                                seq_ix, intron, seq_pos[seq_ix], cur_span_from - 1
                                );
                            if (intron.length() > 1) {
                                string splice5 = intron.substr(0, 2);
                                string splice3 = intron.substr(intron.length() - 2);

                                if (!IsConsensusSplice(splice5, splice3)) {
                                    gap_row.type = fIntronNC;
                                }
                            }
                        }

                        iter = rows.insert(iter, gap_row);
                    }

                    seq_pos[seq_ix] = cur_span_to + 1;

                } else {

                    if (cur_seq_pos > 0 && cur_span_to < cur_seq_pos) {
                        SSpanRow gap_row;

                        gap_row.ranges.assign(m_NumSeqs, TSignedSeqRange());
                        gap_row.ranges[seq_ix] =
                            TSignedSeqRange(cur_span_to + 1, seq_pos[seq_ix])
                            ;

                        gap_row.type = mf_MarkIndels;
                        gap_row.length = gap_row.gap = gap_row.ranges[seq_ix].GetLength();
                        gap_row.mismatch = 0;
                        gap_row.identity = 0.0;

                        if (mf_MarkIndels == fIntron && mf_UseSparse) {
                            string intron;
                            m_SparseAlns[0]->GetSeqString(
                                seq_ix, intron, cur_span_to + 1, seq_pos[seq_ix]
                                );
                            if (intron.length() > 1) {
                                string splice5 = intron.substr(0, 2);
                                string splice3 = intron.substr(intron.length() - 2);

                                if (!IsConsensusSplice(splice5, splice3)) {
                                    gap_row.type = fIntronNC;
                                }
                            }
                        }

                        iter = rows.insert(iter, gap_row);
                    }

                    seq_pos[seq_ix] = cur_span_from - 1;
                }
            }
        }
    }
}


void CAlnSpanVertModel::x_MergeRows(TAlnSpans& rows)
{
    ///
    /// next, coalesce our spans
    ///

    if (rows.empty())
        return;
    TAlnSpans::iterator iter = rows.begin();
    TAlnSpans::iterator prev = rows.end();

    for (; iter != rows.end(); prev = iter, ++iter) {
        if (
            iter->type == fIntron
            || iter->type == fIntronNC
            || iter->type == fTail
            || iter->type == fPolyA
            ) {
            continue;
        }

        if (iter->type != fAligned || (int)iter->aln_range.GetLength() > m_Threshold)
            continue;

        bool merge_prev = false;
        bool merge_next = false;

        TAlnSpans::iterator next = iter + 1;

        if (prev != rows.end() && (prev->type & (fMismatch | fAligned))) {
            merge_prev = true;
        }

        if (next != rows.end() && (next->type & (fMismatch | fAligned))) {
            merge_next = true;
        }

        SSpanRow merged_row = *iter;

        if (merge_prev) {
            merged_row.type |= prev->type;
            merged_row.length += prev->length;
            merged_row.mismatch += prev->mismatch;
            merged_row.gap += prev->gap;

            merged_row.aln_range.SetFrom(prev->aln_range.GetFrom());

            for (int row = 0; row < m_NumSeqs; row++) {
                if (row >= (int)merged_row.ranges.size()) {
                    LOG_POST(Warning << "ASV: Not enough rows!");
                    break;

                } else if (row >= (int)prev->ranges.size()) {
                    LOG_POST(Warning << "ASV: Not enough rows!");
                    break;
                }

                merged_row.ranges[row] += prev->ranges[row];
            }
        }

        if (merge_next) {
            merged_row.type |= next->type;
            merged_row.length += next->length;
            merged_row.mismatch += next->mismatch;
            merged_row.gap += next->gap;

            merged_row.aln_range.SetTo(next->aln_range.GetTo());

            for (int row = 0; row < m_NumSeqs; row++) {
                if (row >= (int)merged_row.ranges.size()) {
                    LOG_POST(Warning << "ASV: Not enough rows!");
                    break;

                } else if (row >= (int)next->ranges.size()) {
                    LOG_POST(Warning << "ASV: Not enough rows!");
                    break;
                }

                merged_row.ranges[row] += next->ranges[row];
            }
        }

        TAlnSpans::iterator from = merge_prev ? iter : next;
        TAlnSpans::iterator to = merge_next ? (next + 1) : next;

        rows.erase(from, to);

        if (merge_prev) {
            iter = prev;
        }
        *iter = merged_row;
    }
}

///
/// finally, convert our spans into rows for display
///
void CAlnSpanVertModel::x_ConvertRowsToSpans(TAlnSpans& rows)
{

    TSignedSeqPos pos = m_AlnRange.GetFrom();
    m_AlnSpans.reserve(rows.size() * 2);
    TAlnSpans::iterator iter = rows.begin();
    for (; iter != rows.end();) {
        if (pos < (int)iter->aln_range.GetFrom()) {
            SSpanRow newRow;
            newRow.aln_range.SetFrom(pos);
            newRow.aln_range.SetTo(iter->aln_range.GetFrom() - 1);
            newRow.length = newRow.aln_range.GetLength() / m_BaseWidth;
            newRow.identity = 0;
            newRow.type = 0;
            newRow.gap = 0;
            pos = newRow.aln_range.GetTo() + 1;

            m_AlnSpans.push_back(newRow);
        } else {
            m_AlnSpans.push_back(*iter);
            pos = iter->aln_range.GetTo() + 1;
            ++iter;
        }
    }
    if (pos < m_AlnRange.GetTo()) {
        SSpanRow newRow;
        newRow.aln_range.SetFrom(pos);
        newRow.aln_range.SetTo(m_AlnRange.GetTo() - 1);
        newRow.length = newRow.aln_range.GetLength() / m_BaseWidth;
        newRow.identity = 0;
        newRow.type = 0;
        newRow.gap = 0;
        pos = newRow.aln_range.GetTo() + 1;

        m_AlnSpans.push_back(newRow);

    }
}


void CAlnSpanVertModel::x_UpdateMetaData()
{
    unsigned warning_cnt = 0; // warning counter to limit excessive reporting
    TAlnSpans::iterator iter = m_AlnSpans.begin();
    for (; iter != m_AlnSpans.end();) {

        if (iter->length == 0) {
            iter->length = iter->aln_range.GetLength() / m_BaseWidth;
        }
        if (iter->length == 0) {
            ++warning_cnt;
            if (warning_cnt < 10) {
                LOG_POST(Warning << "Zero length span!");
            } else
                if (warning_cnt == 10) {
                    LOG_POST(Warning << "Zero length span! (Further warnings supressed)");
                }
            iter = m_AlnSpans.erase(iter);
            continue;
        }

        if (!iter->type) {
            iter->type = fAligned;
        }

        if (iter->type & ~(fGap | fDiscontig)) {
            double identity =
                ((double)(iter->length - iter->mismatch)) / iter->length
                ;
            iter->identity = floor(identity * 10000 + 0.5) / 100.0;

        } else {
            iter->identity = 0.0;
        }

        for (size_t row = iter->ranges.size(); (int)row < m_NumSeqs; row++) {

            TSignedSeqPos from, to;
            bool is_aa = false;

            if (mf_UseSparse) {
                from = m_SparseAlns[0]->GetSeqPosFromAlnPos((int)row, iter->aln_range.GetFrom());
                to = m_SparseAlns[0]->GetSeqPosFromAlnPos((int)row, iter->aln_range.GetTo());
                is_aa = m_SparseAlns[0]->GetBaseWidth(row) == 3;
            } else {
                from = m_AlnSrc->GetSeqPosFromAlnPos((int)row, iter->aln_range.GetFrom());
                to = m_AlnSrc->GetSeqPosFromAlnPos((int)row, iter->aln_range.GetTo());
                is_aa = m_AlnSrc->GetRowHandle(row)->UsesAATranslation();
            }


            /*
                    if( from == -1 && iter->aln_range.GetFrom() > 0 ){
                    from = m_AlnSrc->GetSeqPosFromAlnPos( row, iter->aln_range.GetFrom() -1 );
                    }
                    if( to == -1 ){
                    to = m_AlnSrc->GetSeqPosFromAlnPos( row, iter->aln_range.GetTo() +1 );
                    }
                    */

            if (from == -1 || to == -1) {
                iter->ranges.push_back(TSignedSeqRange::GetEmpty());
            } else {
                if (from > to) {
                    swap(from, to);
                }
                if (is_aa) {
                    from /= 3;
                    to /= 3;
                } else if (iter->type & fGap) {
                    iter->length /= 3;
                    iter->gap /= 3;
                }

                iter->ranges.push_back(TSignedSeqRange(from, to));
            }
        }

        ++iter;
    }
}



void CAlnSpanVertModel::x_UpdateLocs()
{
    NON_CONST_ITERATE(TAlnSpans, iter, m_AlnSpans)
    {
        SSpanRow& span_row = *iter;
        for (int row = 0; row < m_NumSeqs; row++) {
            if (span_row.ranges[row].Empty()) {
                continue;
            }
            if (mf_UseSparse || m_AlnSrc->CanGetId(row)) {
                CRef<CSeq_loc> loc(new CSeq_loc());
                loc->SetInt().SetFrom(span_row.ranges[row].GetFrom());
                loc->SetInt().SetTo(span_row.ranges[row].GetTo());
                if (mf_UseSparse) {
                    loc->SetInt().SetStrand(
                        m_SparseAlns[0]->IsPositiveStrand(row) ? eNa_strand_plus : eNa_strand_minus
                        );
                    loc->SetId(m_SparseAlns[0]->GetSeqId(row));
                } else {
                    loc->SetInt().SetStrand(
                        m_AlnSrc->IsPositiveStrand(row) ? eNa_strand_plus : eNa_strand_minus
                        );
                    loc->SetId(m_AlnSrc->GetSeqId(row));
                }
                span_row.locs.push_back(loc);
            }
        }
    }
}


void CAlnSpanVertModel::x_PopulateRowsSpliced( TAlnSpans& span_rows )
{
    int product_length = 0;
    bool is_protein = 
        m_SplicedSeg->GetProduct_type() == CSpliced_seg::eProduct_type_protein
    ;

    if( m_SplicedSeg->IsSetProduct_length() ){
        product_length = m_SplicedSeg->GetProduct_length();
    }

    if( 
        m_SplicedSeg->GetSeqStrand( 0 ) == eNa_strand_minus
        && m_SplicedSeg->IsSetPoly_a() 
    ){
        int poly_a = m_SplicedSeg->GetPoly_a();

        if( poly_a <= -1 ){
            // inferred poly(A) tail at transcript's start
        } else {
            int start = m_SplicedSeg->GetSeqStart( 0 );

            if( poly_a < start ){
                // range [0, poly_a] - poly(A)
                SSpanRow newRow;
                newRow.ranges.push_back( TSignedSeqRange() );
                newRow.ranges.push_back(TSignedSeqRange(0, poly_a));
                newRow.length = poly_a;
                newRow.type = fPolyA;

                span_rows.push_back( newRow );

                if( poly_a < start -1 ){
                    // range [poly_a+1, start-1] - unaligned head 
                    SSpanRow newRow;
                    newRow.ranges.push_back( TSignedSeqRange() );
                    newRow.ranges.push_back(TSignedSeqRange(poly_a + 1, start - 1));
                    newRow.length = start - 1 - poly_a;
                    newRow.type = fTail;

                    span_rows.push_back( newRow );
                }
            }
        }
    } else {
        int start = m_SplicedSeg->GetSeqStart( 0 );

        if( start > 0 ){
            SSpanRow newRow;
            newRow.aln_range = TSeqRange( 0, start-1 );
            newRow.ranges.push_back(TSignedSeqRange());
            newRow.ranges.push_back(TSignedSeqRange());
            newRow.length = start - 1;
            newRow.type = fTail;

            span_rows.push_back( newRow );
        }
    }

    CSpliced_seg::TExons exons = m_SplicedSeg->GetExons();

    ITERATE( CSpliced_seg::TExons, ex_itr, exons ){

        const CSpliced_exon& exon = **ex_itr;

        int prod_pos = 
            is_protein
            ? exon.GetProduct_start().GetProtpos().GetAmin()
            : exon.GetProduct_start().GetNucpos()
        ;
        int gen_pos = exon.GetGenomic_start();

        if( exon.IsSetParts() ){
            ITERATE( CSpliced_exon::TParts, part_itr, exon.GetParts() ){
                const CSpliced_exon_chunk& chunk = **part_itr;

                SSpanRow newRow;
                int len = 0;
                int prod_len = 0;

                switch (chunk.Which()) {
                case CSpliced_exon_chunk::e_Match:
                    len = prod_len = chunk.GetMatch();
                    if (is_protein)
                        prod_len /= 3;
                    else
                        prod_len -= 1;

                    newRow.ranges.push_back(TSignedSeqRange(gen_pos, gen_pos + len - 1));
                    newRow.ranges.push_back(TSignedSeqRange(prod_pos, prod_pos + prod_len));
                    newRow.type = fAligned;

                    span_rows.push_back(newRow);

                    prod_pos += prod_len;
                    gen_pos += len;

                    break;
                case CSpliced_exon_chunk::e_Mismatch:
                    len = prod_len = chunk.GetMismatch();
                    if (is_protein)
                        prod_len /= 3;
                    else
                        prod_len -= 1;

                    newRow.ranges.push_back(TSignedSeqRange(gen_pos, gen_pos + len - 1));
                    newRow.ranges.push_back(TSignedSeqRange(prod_pos, prod_pos + prod_len));
                    newRow.type = fMismatch;
                    span_rows.push_back(newRow);

                    prod_pos += prod_len;
                    gen_pos += len;

                    break;
                case CSpliced_exon_chunk::e_Diag:
                {
                    len = prod_len = chunk.GetDiag();
                    if (is_protein)
                        prod_len /= 3;
                    else
                        prod_len -= 1;


                    TSeqPos from = gen_pos;
                    TSeqPos to = gen_pos + len - 1;
                    TSeqPos aln_from = m_SparseAlns[0]->GetAlnPosFromSeqPos((int)0, from);
                    TSeqPos aln_to = m_SparseAlns[0]->GetAlnPosFromSeqPos((int)0, to);
                    if (aln_to < aln_from)
                        swap(aln_to, aln_from);
                    newRow.aln_range.Set(aln_from, aln_to + 1);
                    from = aln_from;
                    to = aln_to;
                    if (is_protein) {
                        if (from % 3)
                            from += 3 - from % 3;
                        int off = to % 3;
                        if (off == 1) {
                            --to;
                        } else if (off == 2) {
                            ++to;
                        }
                    }
                    // Diag type includes segments with mismatches
                    // and we have to calculate them
                    CRange<TSignedSeqPos> seq_range(aln_from, aln_to);
                    string g_str;
                    m_SparseAlns[0]->GetAlnSeqString(0, g_str, CRange<TSignedSeqPos>(from, to), is_protein);
                    string p_str;
                    m_SparseAlns[0]->GetAlnSeqString(1, p_str, seq_range, false);
                    
                    _ASSERT(g_str.size() == p_str.size());
                    for (auto i = 0; i < g_str.size(); ++i) {
                        if (i < p_str.size()) {
                            if (g_str[i] != p_str[i])
                                ++newRow.mismatch;
                        } else {
                            newRow.mismatch += (g_str.size() - p_str.size());
                            break;
                        }
                    }

                    newRow.ranges.push_back(TSignedSeqRange(gen_pos, gen_pos + len - 1));
                    newRow.ranges.push_back(TSignedSeqRange(prod_pos, prod_pos + prod_len));

                    newRow.type = fAligned;
                    if (newRow.mismatch > 0)
                        newRow.type |= fMismatch;

                    span_rows.push_back(newRow);

                    prod_pos += prod_len;
                    gen_pos += len;

                    break;
                }
                case CSpliced_exon_chunk::e_Product_ins:
                    prod_len = chunk.GetProduct_ins();
                    if (is_protein)
                        prod_len /= 3;
                    else
                        prod_len -= 1;

                    newRow.ranges.push_back( TSignedSeqRange() );
                    newRow.ranges.push_back(TSignedSeqRange(prod_pos, prod_pos + prod_len));
                    newRow.type = fGap;
                    newRow.length = prod_len;
                    span_rows.push_back( newRow );

                    prod_pos += prod_len;

                    break;
                case CSpliced_exon_chunk::e_Genomic_ins:
                    len = chunk.GetGenomic_ins();

                    newRow.ranges.push_back( TSignedSeqRange( gen_pos, gen_pos+len-1 ) );
                    newRow.ranges.push_back(TSignedSeqRange());
                    newRow.type = fGap;
                    newRow.length = len;
                    span_rows.push_back( newRow );

                    gen_pos += len;

                    break;
                default:
                    ;
                }
            }
        }
    }


    if( 
        m_SplicedSeg->GetSeqStrand( 0 ) == eNa_strand_plus
        && m_SplicedSeg->IsSetPoly_a() 
        ){
            int poly_a = m_SplicedSeg->GetPoly_a();

            if( poly_a >= product_length ){
                // inferred poly(A) tail at transcript's end
            } else {
                int stop = m_SplicedSeg->GetSeqStop( 0 );

                if( stop < poly_a ){
                    if( stop+1 < poly_a ){
                        // range [stop+1, poly_a] - unaligned tail
                        SSpanRow newRow;
                        newRow.ranges.push_back( TSignedSeqRange() );
                        newRow.ranges.push_back(TSignedSeqRange(stop + 1, poly_a - 1));
                        newRow.length = (poly_a - stop) + 1;
                        newRow.type = fTail;
                       span_rows.push_back( newRow );
                    }

                    // range [poly_a, product_length] - poly(A)
                    SSpanRow newRow;
                    newRow.ranges.push_back( TSignedSeqRange() );
                    newRow.ranges.push_back(TSignedSeqRange(poly_a, product_length - 1));
                    newRow.length = (product_length - poly_a);
                    newRow.type = fPolyA;
                    span_rows.push_back( newRow );
                }
            }
    } else {
        int stop = m_SplicedSeg->GetSeqStop( 0 );

        if( stop < product_length - 1){
            SSpanRow newRow;
            newRow.ranges.push_back( TSignedSeqRange() );
            newRow.ranges.push_back(TSignedSeqRange(stop + 1, product_length - 1));
            newRow.length = product_length - stop;
            newRow.type = fTail;

            span_rows.push_back( newRow );
        }
    }

    for (auto&& aln_row : span_rows) {
        if (!aln_row.ranges.empty() && aln_row.ranges[0].NotEmpty()) {
            TSeqPos aln_from = m_SparseAlns[0]->GetAlnPosFromSeqPos((int)0, aln_row.ranges[0].GetFrom());
            TSeqPos aln_to = m_SparseAlns[0]->GetAlnPosFromSeqPos((int)0, aln_row.ranges[0].GetTo());
            if (aln_to < aln_from)
                swap(aln_to, aln_from);
            aln_row.aln_range.Set(aln_from, aln_to + 1);
        }
    }

}

void CAlnSpanVertModel::UpdateRows()
{
    if( mf_RowsInvalidated ){

        m_AlnSpans.clear();
        x_CreateRows();

        x_FireDataChanged();

        mf_RowsInvalidated = false;
    }
}

void CAlnSpanVertModel::SetThreshold( int th )
{
    if( th != m_Threshold && th >= 0 ){

        m_Threshold = th;

        mf_RowsInvalidated = true;
    }
}

void CAlnSpanVertModel::SetShowIndels( bool indel )
{ 
    if( indel != mf_ShowIndels ){

        mf_ShowIndels = indel; 

        mf_RowsInvalidated = true;
    }
} 


void CAlnSpanVertModel::x_AdjustColumns()
{
    //TODO revive or remove this
    /*
    for( TAlnSegments::iterator iter = m_AlnSpans.begin();  iter != NULL;  ++iter ){
        const SSpanRow& proxy = *iter;
        if( proxy.align->IsSetScore() ){
            ITERATE (CSeq_align::TScore, score_iter, proxy.align->GetScore()) {
                const CScore& score = **score_iter;
                string str;
                if (score.GetId().IsStr()) {
                    str = score.GetId().GetStr();
                } else {
                    str = NStr::IntToString(score.GetId().GetId());
                }

                proxy.scores[str] = *score_iter;

                TScoreColNames::iterator col_iter = m_ColNames.find(str);
                if (col_iter == m_ColNames.end()) {
                    size_t col_idx = CAlnSpanVertModel::eScoreStart;
                    col_idx += m_ColNames.size();
                    m_ColNames  [str    ] = col_idx;
                    m_ColIndices[col_idx] = str;
                }
            }
        }
    }
    */
}

int CAlnSpanVertModel::GetNumRows() const
{
    return (int)m_AlnSpans.size();
}


int CAlnSpanVertModel::GetNumColumns() const
{
    return (int)m_ColNames.size();
}


wxString CAlnSpanVertModel::GetColumnName( int aColIdx ) const
{
    return ToWxString(m_ColNames[(size_t)aColIdx]);
}


wxString CAlnSpanVertModel::GetColumnType(int aColIdx) const
{
    aColIdx -= m_NumSeqs;

    if( aColIdx < 0 ){
        return wxT("string");
    }

    switch( aColIdx ){
    case eLengthCol:
    case eMismatchCountCol:
    case eGapLengthCol:
        return wxT("int");

    case eIdenticalCol:
        return wxT("double");

    case eTypeCol:
        return wxT("string");
    default:
        ;//_ASSERT( false ); // Unknown column
    }

    return wxT("string");
}


static void s_Append(string& dst, const string& src)
{
    if ( !dst.empty() ) {
        dst += ", ";
    }
    dst += src;
}


wxVariant CAlnSpanVertModel::GetValueAt( int row, int col ) const
{
    _ASSERT( row < (int)m_AlnSpans.size() );
    if( row >= (int)m_AlnSpans.size() ){
        NCBI_THROW(
            CException,
            eUnknown, "CAlnSpanVertModel::GetValueAt(): Array index out-of-bounds"
        );
    }

    const SSpanRow& spanRow = m_AlnSpans[row];

    int extra_col = col - m_NumSeqs;

    switch( extra_col ){
    case eTypeCol:
        if (spanRow.str_type.empty()) {
            // Type
            if( spanRow.type & fAligned ){
                if( spanRow.type & fMismatch ){
                    s_Append( spanRow.str_type, "Mixed" );
                } else {
                    s_Append( spanRow.str_type, "Aligned" );
                }
            } else if( spanRow.type & fMismatch ){
                s_Append( spanRow.str_type, "Mismatch" );
            }
            if( spanRow.type & fGap ){
                s_Append( spanRow.str_type, "Gap" );
            }
            if( spanRow.type & fInvalid ){
                s_Append( spanRow.str_type, "Invalid" );
            }
            if( spanRow.type & fDiscontig ){
                s_Append( spanRow.str_type, "Discontig" );
            }
            if( spanRow.type & fIntron ){
                s_Append( spanRow.str_type, "Intron" );
            }
            if( spanRow.type & fIntronNC ){
                s_Append( spanRow.str_type, "Intron (non-consensus)" );
            }
            if( spanRow.type & fPolyA ){
                s_Append( spanRow.str_type, "Poly(A)" );
            }
            if( spanRow.type & fTail ){
                s_Append( spanRow.str_type, "Tail" );
            }
        }
        return ToWxString( spanRow.str_type );

    case eLengthCol:
        return wxVariant( spanRow.length );

    case eMismatchCountCol:
        return wxVariant( spanRow.mismatch );

    case eGapLengthCol:
        return wxVariant( spanRow.gap );

    case eIdenticalCol:
        return wxVariant( spanRow.identity );
    }

    spanRow.str_ranges.resize( m_NumSeqs +1 );
    if( col < m_NumSeqs ){
        if(
            col < 0
            || col >= (int)spanRow.ranges.size()
            || spanRow.ranges[col].Empty()
        ){
            return wxString();
        }
        if( spanRow.str_ranges[col].empty() ){
            TSignedSeqRange range = spanRow.ranges[col];

            spanRow.str_ranges[col] =
                NStr::IntToString( range.GetFrom() + 1, NStr::fWithCommas )
            ;
            spanRow.str_ranges[col] += "-";
            spanRow.str_ranges[col] +=
                NStr::IntToString( range.GetTo() + 1, NStr::fWithCommas )
            ;
        }

        return ToWxString( spanRow.str_ranges[col] );

    } else {

        if( spanRow.str_ranges[m_NumSeqs].empty() ){
            TSeqRange range = spanRow.aln_range;

            spanRow.str_ranges[m_NumSeqs] =
                NStr::IntToString( range.GetFrom() + 1, NStr::fWithCommas )
            ;
            spanRow.str_ranges[m_NumSeqs] += "-";
            spanRow.str_ranges[m_NumSeqs] +=
                NStr::IntToString( range.GetTo() + 1, NStr::fWithCommas )
             ;
        }

        return ToWxString( spanRow.str_ranges[m_NumSeqs] );
    }
}


const CAlnSpanVertModel::SSpanRow& CAlnSpanVertModel::GetData( size_t row ) const
{
    _ASSERT( row < m_AlnSpans.size() );
    if( row >= m_AlnSpans.size() ){
        NCBI_THROW(
            CException,
            eUnknown, "CAlnSpanVertModel::GetData(): Array index out-of-bounds"
        );
    }
    return m_AlnSpans[row];
}

END_NCBI_SCOPE


/*
 * ===========================================================================
 * $Log$
 * ===========================================================================
 */
