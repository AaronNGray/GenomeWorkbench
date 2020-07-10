/*  $Id: aln_table_ds.cpp 25481 2012-03-27 14:58:29Z kuznets $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/aln_table/aln_table_ds.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <objmgr/align_ci.hpp>
#include <objtools/alnmgr/alnvec.hpp>
#include <objmgr/seq_vector.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Score.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/general/Object_id.hpp>
#include <gui/objutils/label.hpp>
#include <serial/iterator.hpp>
#include <algo/align/util/score_builder.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CAlnTableDS::CAlnTableDS()
{
}


CAlnTableDS::CAlnTableDS(CScope& scope,
                         const CSeq_loc& loc)
{
    CAlign_CI align_iter(scope, loc);
    for ( ;  align_iter;  ++align_iter) {
        const CSeq_align& align = align_iter.GetOriginalSeq_align();
        x_AddAlign(scope, align);
    }
}


CAlnTableDS::CAlnTableDS(CScope& scope,
                         const CSeq_annot& annot)
{
    if ( !annot.GetData().IsAlign() ) {
        NCBI_THROW(CException, eUnknown,
                   "Annotation is not a seq-align");
    }

    ITERATE (CSeq_annot::TData::TAlign, iter, annot.GetData().GetAlign()) {
        const CSeq_align& align = **iter;
        x_AddAlign(scope, align);
    }
    x_AdjustColumns();
}


CAlnTableDS::CAlnTableDS(CScope& scope,
                         const CSeq_align& align)
{
    x_AddAlign(scope, align);
    x_AdjustColumns();
}


CAlnTableDS::CAlnTableDS(CScope& scope,
                         const CSeq_align_set& aligns)
{
    ITERATE (CSeq_align_set::Tdata, iter, aligns.Get()) {
        const CSeq_align& align = **iter;
        x_AddAlign(scope, align);
    }
    x_AdjustColumns();
}


CAlnTableDS::CAlnTableDS(CScope& scope,
                         const list< CConstRef<CSeq_align> >& aligns)
{
    ITERATE (list< CConstRef<CSeq_align> >, iter, aligns) {
        const CSeq_align& align = **iter;
        x_AddAlign(scope, align);
    }
    x_AdjustColumns();
}


void CAlnTableDS::x_AddAlign(CScope& scope, const CSeq_align& align)
{
    SAlignment proxy;
    proxy.scope.Reset(&scope);
    proxy.align.Reset(&align);
    m_Alignments.push_back(proxy);

    if (align.GetSegs().IsDisc()) {
        ITERATE (CSeq_align::TSegs::TDisc::Tdata, iter, align.GetSegs().GetDisc().Get()) {
            x_AddAlign(scope, **iter);
        }
    }
}


void CAlnTableDS::Update()
{
    x_FireDataChanged();
}


void CAlnTableDS::Add(CScope& scope, const CSeq_align& align)
{
    SAlignment proxy;
    proxy.align.Reset(&align);
    proxy.scope.Reset(&scope);
    m_Alignments.push_back(proxy);

    x_AdjustColumns(m_Alignments.end() - 1, m_Alignments.end());
}


const CAlnTableDS::TAlignments& CAlnTableDS::GetAlignments(void) const
{
    return m_Alignments;
}


void CAlnTableDS::x_AdjustColumns()
{
    x_AdjustColumns(m_Alignments.begin(), m_Alignments.end());
}


void CAlnTableDS::x_AdjustColumns(TAlignments::const_iterator begin,
                                  TAlignments::const_iterator end)
{
    for (TAlignments::const_iterator iter = begin;  iter != end;  ++iter) {
        const SAlignment& proxy = *iter;
        if (proxy.align->IsSetScore()) {
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
                    size_t col_idx = CAlnTableDS::eScoreStart;
                    col_idx += m_ColNames.size();
                    m_ColNames  [str    ] = col_idx;
                    m_ColIndices[col_idx] = str;
                }
            }
        }
    }
}


int CAlnTableDS::GetNumRows() const
{
    return (int)m_Alignments.size();
}


int CAlnTableDS::GetNumColumns() const
{
    return (int)eScoreStart + m_ColNames.size();
}


wxString CAlnTableDS::GetColumnName(int aColIdx) const
{
    switch (aColIdx) {
    case eLabel:
        {{
            static wxString s(wxT("Description"));
            return s;
        }}
        break;

    case eType:
        {{
            static wxString s(wxT("Type"));
            return s;
        }}
        break;

    case eSeq1Start:
        {{
            static wxString s(wxT("Seq1 Start"));
            return s;
        }}
        break;

    case eSeq1Stop:
        {{
            static wxString s(wxT("Seq1 Stop"));
            return s;
        }}
        break;

    case eSeq2Start:
        {{
            static wxString s(wxT("Seq2 Start"));
            return s;
        }}
        break;

    case eSeq2Stop:
        {{
            static wxString s(wxT("Seq2 Stop"));
            return s;
        }}
        break;

    case eNumSeqs:
        {{
            static wxString s(wxT("# Sequences"));
            return s;
        }}
        break;

    case eAlignedLength:
        {{
            static wxString s(wxT("Aligned Length"));
            return s;
        }}
        break;

    case eMismatchCount:
        {{
            static wxString s(wxT("Mismatches"));
            return s;
        }}
        break;

    case eGapCount:
        {{
            static wxString s(wxT("GapCount"));
            return s;
        }}
        break;

    default:
        {{
            TScoreColIndices::const_iterator it = m_ColIndices.find(aColIdx);
            if (it != m_ColIndices.end()) {
                return ToWxString(it->second);
            }
            static wxString s(wxT("Unknown Score"));
            return s;
        }}
        break;
    }
}


wxString CAlnTableDS::GetColumnType(int aColIdx) const
{
    switch (aColIdx) {
    case eLabel:
    case eType:
        return wxT("string");

    case eNumSeqs:
    case eAlignedLength:
    case eMismatchCount:
    case eGapCount:
    case eSeq1Start:
    case eSeq1Stop:
    case eSeq2Start:
    case eSeq2Stop:
        return wxT("int");

    default:
        return wxT("double");
    }
}


wxVariant CAlnTableDS::GetValueAt(int row, int col) const
{
    _ASSERT(row < (int)m_Alignments.size());
    if (row >= (int)m_Alignments.size()) {
        NCBI_THROW(CException, eUnknown,
            "CAlnTableDS::GetValueAt(): Array index out-of-bounds");
    }

    const SAlignment& aln = m_Alignments[row];
    CRef<CScope> scope = aln.scope;
    switch (col) {
    case eLabel:
        if (aln.label.empty()) {
            CLabel::GetLabel(*aln.align, &aln.label, CLabel::eDefault, scope);
        }
        return ToWxString(aln.label);

    case eType:
        if (aln.type.empty()) {
            CLabel::GetLabel(*aln.align, &aln.type, CLabel::eType, scope);
        }
        return ToWxString(aln.type);

    case eSeq1Start:
        if (aln.seq1_start == -1) {
            x_GetStartStop(aln);
        }
        /// return in 1-based coordinates - this is for viewing!
        return aln.seq1_start + 1;

    case eSeq1Stop:
        if (aln.seq1_stop == -1) {
            x_GetStartStop(aln);
        }
        /// return in 1-based coordinates - this is for viewing!
        return aln.seq1_stop + 1;

    case eSeq2Start:
        if (aln.seq2_start == -1) {
            x_GetStartStop(aln);
        }
        /// return in 1-based coordinates - this is for viewing!
        return aln.seq2_start + 1;

    case eSeq2Stop:
        if (aln.seq2_stop == -1) {
            x_GetStartStop(aln);
        }
        /// return in 1-based coordinates - this is for viewing!
        return aln.seq2_stop + 1;

    case eNumSeqs:
        if (aln.num_seqs == 0) {
            aln.num_seqs = x_GetNumSeqs(aln);
        }
        return (int)aln.num_seqs;

    case eAlignedLength:
        if (aln.aligned_length == 0) {
            x_GetSeqLength(aln);
        }
        return (int)aln.aligned_length;

    case eMismatchCount:
        if (aln.mismatch_count == 0) {
            x_GetMismatchCount(aln);
        }
        return (int)aln.mismatch_count;

    case eGapCount:
        if (aln.gap_count == 0) {
            x_GetGapCount(aln);
        }
        return (int)aln.gap_count;

    default:
        {{
            TScoreColIndices::const_iterator it = m_ColIndices.find(col);
            if (it != m_ColIndices.end()) {
                double f_score = 0;
                if (aln.align->GetNamedScore(it->second, f_score)) {
                    return (float)f_score;
                }
                int i_score;
                if (aln.align->GetNamedScore(it->second, i_score)) {
                    return (float)i_score;
                }
            }
            return (float)0;
        }}
    }
}


int CAlnTableDS::x_GetNumSeqs(const SAlignment& proxy) const
{
    set<CSeq_id_Handle> id_set;
    CTypeConstIterator<CSeq_id> id_iter(*proxy.align);
    for ( ;  id_iter;  ++id_iter) {
        id_set.insert( CSeq_id_Handle::GetHandle(*id_iter) );
    }

    return (int)id_set.size();
}

void CAlnTableDS::x_GetSeqLength(const SAlignment& proxy) const
{
    if (proxy.align) {
        CScoreBuilder bld;
        proxy.aligned_length = bld.GetAlignLength(*proxy.align);
    }
}


void CAlnTableDS::x_GetGapCount(const SAlignment& proxy) const
{
    if (proxy.align) {
        CScoreBuilder bld;
        proxy.gap_count = bld.GetGapCount(*proxy.align);
    }
}


void CAlnTableDS::x_GetStartStop(const SAlignment& proxy) const
{
    if (proxy.align) {
        TSeqRange range = proxy.align->GetSeqRange(0);
        proxy.seq1_start = range.GetFrom();
        proxy.seq1_stop  = range.GetTo();

        range = proxy.align->GetSeqRange(1);
        proxy.seq2_start = range.GetFrom();
        proxy.seq2_stop  = range.GetTo();
    }
}


void CAlnTableDS::x_GetMismatchCount(const SAlignment& proxy) const
{
    if (proxy.align) {
        /// first, check to see if there is a num_ident score
        /// if this is present, we can use this; otherwise, we must fetch sequence
        proxy.mismatch_count = 0;
        if (proxy.align->GetNamedScore("num_ident", proxy.mismatch_count)) {
            if ( !proxy.aligned_length ) {
                x_GetSeqLength(proxy);
            }
            proxy.mismatch_count = proxy.aligned_length - proxy.mismatch_count;
            return;
        }

        ///
        /// heavy lifting
        ///
        CScoreBuilder bld;
        proxy.mismatch_count = bld.GetMismatchCount(*proxy.scope, *proxy.align);
    }
}


const CAlnTableDS::SAlignment& CAlnTableDS::GetData(size_t row) const
{
    _ASSERT(row < m_Alignments.size());
    if (row >= m_Alignments.size()) {
        NCBI_THROW(CException, eUnknown,
            "CAlnTableDS::GetData(): Array index out-of-bounds");
    }
    return m_Alignments[row];
}

END_NCBI_SCOPE
