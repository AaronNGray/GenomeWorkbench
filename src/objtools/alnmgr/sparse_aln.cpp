/*  $Id: sparse_aln.cpp 601341 2020-02-05 20:27:33Z vasilche $
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

#include <objtools/alnmgr/sparse_aln.hpp>
#include <objtools/alnmgr/sparse_ci.hpp>
#include <objtools/alnmgr/alnexception.hpp>
#include <objtools/error_codes.hpp>
#include <objmgr/util/sequence.hpp>

#include <objects/seqalign/Sparse_align.hpp>
#include <objects/seqfeat/Genetic_code_table.hpp>
#include <objects/seqfeat/Org_ref.hpp>


#define NCBI_USE_ERRCODE_X   Objtools_Aln_Sparse


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


CSparseAln::CSparseAln(const CAnchoredAln& anchored_aln,
                       objects::CScope& scope)
    : m_Scope(&scope),
      m_GapChar('-'),
      m_NaCoding(CSeq_data::e_not_set),
      m_AaCoding(CSeq_data::e_not_set),
      m_AnchorDirect(true)
{
    x_Build(anchored_aln);
}

    
CSparseAln::~CSparseAln()
{
}


CSparseAln::TDim CSparseAln::GetDim() const {
    return m_Aln->GetDim();
}


struct SGapRange
{
    TSignedSeqPos from; // original position of the gap on the anchor
    TSignedSeqPos second_from; // position on the sequence
    TSignedSeqPos len;  // length of the gap
    bool          direct;
    int           row;  // Row, containing the gap.
    //size_t        idx;  // Index of the gap in the original vector.
    // This gap's 'from' must be shifted by 'shift'.
    // Segments at or after 'from' position must be offset by 'shift + len'.
    TSignedSeqPos shift;

    bool operator<(const SGapRange& rg) const
    {
        if (from != rg.from) return from < rg.from; // sort by pos
        return row < rg.row; // lower rows go first.
        // Don't check the length. Stable sort will preserve the order
        // of consecutive gaps on the same row.
    }
};


typedef vector<SGapRange> TGapRanges;


void CSparseAln::x_Build(const CAnchoredAln& src_align)
{
    const TDim& dim = src_align.GetDim();

    m_BioseqHandles.clear();
    m_BioseqHandles.resize(dim);

    m_SeqVectors.clear();
    m_SeqVectors.resize(dim);

    // Collect all gaps on all rows. They need to be inserted into the
    // new anchor as normal segments.
    TGapRanges gaps;
    for (TDim row = 0; row < dim; ++row) {
        const CPairwiseAln& pw = *src_align.GetPairwiseAlns()[row];
        const CPairwiseAln::TInsertions& ins_vec = pw.GetInsertions();
        gaps.reserve(gaps.size() + ins_vec.size());
        for (auto i = ins_vec.begin(); i != ins_vec.end(); ++i) {
            SGapRange gap;
            gap.from = i->GetFirstFrom();
            gap.second_from = i->GetSecondFrom();
            gap.len = i->GetLength();
            gap.direct = i->IsDirect();
            gap.row = row;
            gap.shift = 0;
            //gap.idx = i;
            gaps.push_back(gap);
        }
    }

    // We need to preserve the order of consecutive gaps at the same
    // position on the same row. Use stable_sort.
    stable_sort(gaps.begin(), gaps.end());
    // Set shift for all gaps.
    TSignedSeqPos shift = 0;
    NON_CONST_ITERATE(TGapRanges, gap_it, gaps) {
        gap_it->shift = shift;
        shift += gap_it->len;
    }

    m_Aln.Reset(new CAnchoredAln());
    m_Aln->SetDim(dim);
    m_Aln->SetAnchorRow(src_align.GetAnchorRow());
    m_Aln->SetScore(src_align.GetScore());

    // Now for each row convert insertions to normal segments and change
    // the old anchor coordinates to alignment coordinates.
    for (TDim row = 0; row < dim; ++row) {
        const CPairwiseAln& pw = *src_align.GetPairwiseAlns()[row];
        CPairwiseAln::const_iterator seg_it = pw.begin();
        TGapRanges::const_iterator gap = gaps.begin();
        TSignedSeqPos shift = 0;
        CRef<CPairwiseAln> dst(new CPairwiseAln(
            pw.GetFirstId(), pw.GetSecondId(), pw.GetFlags()));
        TSeqPos last_to = 0;
        bool first_direct = true;
        bool second_direct = true;
        while (seg_it != pw.end()) {
            CPairwiseAln::TAlignRange rg = *seg_it;
            first_direct = rg.IsFirstDirect();
            second_direct = rg.IsDirect();
            ++seg_it;
            // Check if there are gaps before the new segment's end.
            while (gap != gaps.end()  &&
                gap->from < rg.GetFirstToOpen()) {
                if (gap->row == row) {
                    // Insertion in this row - align to the anchor.
                    CPairwiseAln::TAlignRange ins(gap->from + shift,
                        gap->second_from, gap->len, gap->direct);
                    // Reset flags to match row orientation.
                    ins.SetFirstDirect(first_direct);
                    ins.SetDirect(second_direct);
                    dst->push_back(ins);
                }
                else if (gap->from > rg.GetFirstFrom()) {
                    // Split the range if there are insertions in other rows.
                    CPairwiseAln::TAlignRange sub = rg;
                    sub.SetLength(gap->from - rg.GetFirstFrom());
                    sub.SetFirstFrom(sub.GetFirstFrom() + shift);
                    if (rg.IsDirect()) {
                        rg.SetSecondFrom(rg.GetSecondFrom() + sub.GetLength());
                    }
                    else {
                        sub.SetSecondFrom(rg.GetSecondToOpen() - sub.GetLength());
                    }
                    rg.SetFirstFrom(rg.GetFirstFrom() + sub.GetLength());
                    rg.SetLength(rg.GetLength() - sub.GetLength());
                    dst->push_back(sub);
                }
                shift = gap->shift + gap->len;
                ++gap;
            }
            rg.SetFirstFrom(rg.GetFirstFrom() + shift);
            dst->push_back(rg);
            last_to = rg.GetSecondToOpen();
        }
        // Still have gaps in some rows?
        while (gap != gaps.end()) {
            if (gap->row == row) {
                CPairwiseAln::TAlignRange ins(gap->from + shift,
                    last_to, gap->len, gap->direct);
                ins.SetFirstDirect(first_direct);
                ins.SetDirect(second_direct);
                dst->push_back(ins);
            }
            shift = gap->shift + gap->len;
            ++gap;
        }
        m_Aln->SetPairwiseAlns()[row] = dst;
    }
    // Now the new anchored alignment should contain all segments and gaps
    // aligned to the new anchor coordinates.

    m_SecondRanges.resize(dim);
    for (TDim row = 0;  row < dim;  ++row) {

        /// Check collections flags
        _ASSERT( !m_Aln->GetPairwiseAlns()[row]->IsSet(TAlnRngColl::fInvalid) );
        _ASSERT( !m_Aln->GetPairwiseAlns()[row]->IsSet(TAlnRngColl::fUnsorted) );
        _ASSERT( !m_Aln->GetPairwiseAlns()[row]->IsSet(TAlnRngColl::fOverlap) );

        /// Determine m_FirstRange
        if (row == 0) {
            m_FirstRange = m_Aln->GetPairwiseAlns()[row]->GetFirstRange();
        } else {
            m_FirstRange.CombineWith(m_Aln->GetPairwiseAlns()[row]->GetFirstRange());
        }

        /// Determine m_SecondRanges
        CAlignRangeCollExtender<TAlnRngColl> ext(*m_Aln->GetPairwiseAlns()[row]);
        ext.UpdateIndex();
        m_SecondRanges[row] = ext.GetSecondRange();
    }

    const CPairwiseAln& anch_pw = *m_Aln->GetPairwiseAlns()[m_Aln->GetAnchorRow()];
    m_AnchorDirect = anch_pw.empty() || anch_pw.begin()->IsFirstDirect();
}


CSparseAln::TRng CSparseAln::GetAlnRange() const
{
    return m_FirstRange;
}


void CSparseAln::SetGapChar(TResidue gap_char)
{
    m_GapChar = gap_char;
}


CRef<CScope> CSparseAln::GetScope() const
{
    return m_Scope;
}


const CSparseAln::TAlnRngColl&
CSparseAln::GetAlignCollection(TNumrow row)
{
    _ASSERT(row >= 0  &&  row < GetDim());
    return *m_Aln->GetPairwiseAlns()[row];
}


const CSeq_id& CSparseAln::GetSeqId(TNumrow row) const
{
    _ASSERT(row >= 0  &&  row < GetDim());
    return m_Aln->GetPairwiseAlns()[row]->GetSecondId()->GetSeqId();
}


TSignedSeqPos   CSparseAln::GetSeqAlnStart(TNumrow row) const
{
    _ASSERT(row >= 0  &&  row < GetDim());
    return m_Aln->GetPairwiseAlns()[row]->GetFirstFrom();
}


TSignedSeqPos CSparseAln::GetSeqAlnStop(TNumrow row) const
{
    _ASSERT(row >= 0  &&  row < GetDim());
    return m_Aln->GetPairwiseAlns()[row]->GetFirstTo();
}


CSparseAln::TSignedRange CSparseAln::GetSeqAlnRange(TNumrow row) const
{
    _ASSERT(row >= 0  &&  row < GetDim());
    return TSignedRange(GetSeqAlnStart(row), GetSeqAlnStop(row));
}


TSeqPos CSparseAln::GetSeqStart(TNumrow row) const
{
    _ASSERT(row >= 0  &&  row < GetDim());
    return m_SecondRanges[row].GetFrom();
}


TSeqPos CSparseAln::GetSeqStop(TNumrow row) const
{
    _ASSERT(row >= 0  &&  row < GetDim());
    return m_SecondRanges[row].GetTo();
}


CSparseAln::TRange CSparseAln::GetSeqRange(TNumrow row) const
{
    _ASSERT(row >= 0  &&  row < GetDim());
    return TRange(GetSeqStart(row), GetSeqStop(row));
}


bool CSparseAln::IsPositiveStrand(TNumrow row) const
{
    _ASSERT(row >= 0  &&  row < GetDim());
    _ASSERT( !m_Aln->GetPairwiseAlns()[row]->IsSet(CPairwiseAln::fMixedDir) );
    return m_Aln->GetPairwiseAlns()[row]->IsSet(CPairwiseAln::fDirect) == m_AnchorDirect;
}


bool CSparseAln::IsNegativeStrand(TNumrow row) const
{
    _ASSERT(row >= 0  &&  row < GetDim());
    _ASSERT( !m_Aln->GetPairwiseAlns()[row]->IsSet(CPairwiseAln::fMixedDir) );
    return m_Aln->GetPairwiseAlns()[row]->IsSet(CPairwiseAln::fReversed) == m_AnchorDirect;
}


bool CSparseAln::IsTranslated() const {
    /// TODO Does BaseWidth of 1 always mean nucleotide?  Should we
    /// have an enum (with an invalid (unasigned) value?
    const int k_unasigned_base_width = 0;
    int base_width = k_unasigned_base_width;
    for (TDim row = 0;  row < GetDim();  ++row) {
        if (base_width == k_unasigned_base_width) {
            base_width = m_Aln->GetPairwiseAlns()[row]->GetFirstBaseWidth();
        }
        if (base_width != m_Aln->GetPairwiseAlns()[row]->GetFirstBaseWidth()  ||
            base_width != m_Aln->GetPairwiseAlns()[row]->GetSecondBaseWidth()) {
            return true; //< there *at least one* base diff base width
        }
        /// TODO or should this check be stronger:
        if (base_width != 1) {
            return true;
        }
    }
    return false;
}


inline CSparseAln::TAlnRngColl::ESearchDirection
GetCollectionSearchDirection(CSparseAln::ESearchDirection dir)
{
    typedef CSparseAln::TAlnRngColl   T;
    switch(dir) {
    case CSparseAln::eNone:
        return T::eNone;
    case CSparseAln::eLeft:
        return T::eLeft;
    case CSparseAln::eRight:
        return T::eRight;
    case CSparseAln::eForward:
        return T::eForward;
    case CSparseAln::eBackwards:
        return T::eBackwards;
    }
    _ASSERT(false); // invalid
    return T::eNone;
}


TSignedSeqPos 
CSparseAln::GetAlnPosFromSeqPos(TNumrow row, 
                                TSeqPos seq_pos,
                                ESearchDirection dir,
                                bool try_reverse_dir) const
{
    _ASSERT(row >= 0  &&  row < GetDim());

    TAlnRngColl::ESearchDirection c_dir = GetCollectionSearchDirection(dir);
    return m_Aln->GetPairwiseAlns()[row]->GetFirstPosBySecondPos(seq_pos, c_dir);
}


TSignedSeqPos CSparseAln::GetSeqPosFromAlnPos(TNumrow row, TSeqPos aln_pos,
                                              ESearchDirection dir,
                                              bool try_reverse_dir) const
{
    _ASSERT(row >= 0  &&  row < GetDim());

    TAlnRngColl::ESearchDirection c_dir = GetCollectionSearchDirection(dir);
    return m_Aln->GetPairwiseAlns()[row]->GetSecondPosByFirstPos(aln_pos, c_dir);
}


const CBioseq_Handle&  CSparseAln::GetBioseqHandle(TNumrow row) const
{
    _ASSERT(row >= 0  &&  row < GetDim());

    if ( !m_BioseqHandles[row] ) {
        if ( !(m_BioseqHandles[row] = m_Scope->GetBioseqHandle(GetSeqId(row))) ) {
            string errstr = "Invalid bioseq handle.  Seq id \"" +
                GetSeqId(row).AsFastaString() + "\" not in scope?";
            NCBI_THROW(CAlnException, eInvalidRequest, errstr);
        }
    }
    return m_BioseqHandles[row];
}


CSeqVector& CSparseAln::x_GetSeqVector(TNumrow row) const
{
    _ASSERT(row >= 0  &&  row < GetDim());

    if ( !m_SeqVectors[row] ) {
        CSeqVector vec = GetBioseqHandle(row).GetSeqVector
            (CBioseq_Handle::eCoding_Iupac,
             IsPositiveStrand(row) ? 
             CBioseq_Handle::eStrand_Plus :
             CBioseq_Handle::eStrand_Minus);
        m_SeqVectors[row].Reset(new CSeqVector(vec));
    }

    CSeqVector& seq_vec = *m_SeqVectors[row];
    if ( seq_vec.IsNucleotide() ) {
        if (m_NaCoding != CSeq_data::e_not_set) {
            seq_vec.SetCoding(m_NaCoding);
        }
        else {
            seq_vec.SetIupacCoding();
        }
    }
    else if ( seq_vec.IsProtein() ) {
        if (m_AaCoding != CSeq_data::e_not_set) {
            seq_vec.SetCoding(m_AaCoding);
        }
        else {
            seq_vec.SetIupacCoding();
        }
    }

    return seq_vec;
}


int CSparseAln::x_GetGenCode(TNumrow row) const
{
    CBioseq_Handle h = GetBioseqHandle(row);
    if ( !h ) return kDefaultGenCode;

    // Try to use bio-source first.
    CConstRef<CBioSource> src(sequence::GetBioSource(h));
    if ( src ) return src->GetGenCode();

    // Fallback to org-ref.
    CConstRef<COrg_ref> ref(sequence::GetOrg_refOrNull(h));
    if ( ref ) return ref->GetGcode();

    return kDefaultGenCode;
}


void CSparseAln::TranslateNAToAA(const string& na,
                                 string& aa,
                                 int gencode)
{
    const CTrans_table& tbl = CGen_code_table::GetTransTable(gencode);

    size_t na_remainder = na.size() % 3;
    size_t na_size = na.size() - na_remainder;

    if (&aa != &na) {
        aa.resize(na_size / 3 + (na_remainder ? 1 : 0));
    }

    if ( na.empty() ) return;

    size_t aa_i = 0;
    int state = 0;
    for (size_t na_i = 0;  na_i < na_size; ) {
        for (size_t i = 0;  i < 3;  ++i, ++na_i) {
            state = tbl.NextCodonState(state, na[na_i]);
        }
        aa[aa_i++] = tbl.GetCodonResidue(state);
    }
    if (na_remainder) {
        aa[aa_i++] = '\\';
    }

    if (&aa == &na) {
        aa.resize(aa_i);
    }
}


string& CSparseAln::GetSeqString(TNumrow row,
                                 string &buffer,
                                 TSeqPos seq_from, TSeqPos seq_to,
                                 bool force_translation) const
{
    return GetSeqString(row, buffer, TRange(seq_from, seq_to), force_translation);
}


string& CSparseAln::GetSeqString(TNumrow row,
                                 string &buffer,
                                 const TRange &rq_seq_range,
                                 bool force_translation) const
{
    _ASSERT(row >= 0  &&  row < GetDim());

    TRange seq_range = rq_seq_range;
    if ( seq_range.IsWhole() ) {
        seq_range = GetSeqRange(row);
    }

    buffer.erase();
    int width = m_Aln->GetPairwiseAlns()[row]->GetSecondBaseWidth();
    TSeqPos tr_from = seq_range.GetFrom();
    TSeqPos tr_to = seq_range.GetToOpen();
    if (width > 1) {
        // It's a protein sequence. Protein coordinates were multiplied by 3,
        // any fractions (incomplete AAs) must be discarded.
        tr_from /= 3;
        if (seq_range.GetFrom() % 3 > 0) {
            tr_from++; // skip incomplete AA
        }
        tr_to /= 3;
        force_translation = false; // do not translate AAs.
    }
    if (tr_to > tr_from) {
        CSeqVector& seq_vector = x_GetSeqVector(row);

        TSeqPos size = tr_to - tr_from;
        buffer.resize(size, m_GapChar);

        if (IsPositiveStrand(row)) {
            seq_vector.GetSeqData(tr_from, tr_to, buffer);
        } else {
            TSeqPos vec_size = seq_vector.size();
            seq_vector.GetSeqData(vec_size - tr_to, vec_size - tr_from, buffer);
        }

        if ( force_translation ) {
            TranslateNAToAA(buffer, buffer, x_GetGenCode(row));
        }
    }
    return buffer;
}


string& CSparseAln::GetAlnSeqString(TNumrow row,
                                    string &buffer,
                                    const TSignedRange &rq_aln_range,
                                    bool force_translation) const
{
    _ASSERT(row >= 0  &&  row < GetDim());

    TSignedRange aln_range(rq_aln_range);
    if ( aln_range.IsWhole() ) {
        aln_range = GetSeqAlnRange(row);
    }

    buffer.erase();
    if (aln_range.GetLength() <= 0) {
        return buffer;
    }

    const CPairwiseAln& pairwise_aln = *m_Aln->GetPairwiseAlns()[row];
    if (pairwise_aln.empty()) {
        string errstr = "Invalid (empty) row (" + NStr::IntToString(row) + ").  Seq id \"" +
            GetSeqId(row).AsFastaString() + "\".";
        NCBI_THROW(CAlnException, eInvalidRequest, errstr);
    }

    CSeqVector& seq_vector = x_GetSeqVector(row);
    TSeqPos vec_size = seq_vector.size();

    const int base_width = pairwise_aln.GetSecondBaseWidth();
    int gencode = 0;
    bool translate = force_translation  ||  pairwise_aln.GetSecondId()->IsProtein();

    // buffer holds sequence for "aln_range", 0 index corresonds to aln_range.GetFrom()
    size_t buf_size = aln_range.GetLength();
    if (translate) {
        gencode = x_GetGenCode(row);
        buf_size /= 3;
    }
    buffer.resize(buf_size, m_GapChar);

    string s; // current segment sequence
    CSparse_CI it(*this, row, IAlnSegmentIterator::eSkipInserts, aln_range);

    // Store last incomplete codon's position and length. If the next
    // segment includes the rest of the same codon, it should be included
    // in the results. Otherwise incomplete codons are removed from protein
    // sequences.
    TSeqPos split_codon_pos = kInvalidSeqPos;
    // When trimming one or more codons, future offsets must be adjusted.
    bool direct = IsPositiveStrand(row);
    bool is_first_seg = true;
    size_t trim_from = 0;
    size_t trim_to = 0;
    while ( it )   {
        trim_to = 0;
        const IAlnSegment::TSignedRange& aln_r = it->GetAlnRange(); // in alignment
        const IAlnSegment::TSignedRange& row_r = it->GetRange(); // on sequence
        if ( row_r.Empty() ) {
            ++it;
            is_first_seg = false;
            continue;
        }

        size_t off = aln_r.GetFrom() - aln_range.GetFrom();
        if (base_width == 1) {
            // TODO performance issue - waiting for better API
            if ( direct ) {
                seq_vector.GetSeqData(row_r.GetFrom(), row_r.GetToOpen(), s);
            } else {
                seq_vector.GetSeqData(vec_size - row_r.GetToOpen(),
                    vec_size - row_r.GetFrom(), s);
            }
            if ( translate ) {
                TranslateNAToAA(s, s, gencode);
                off /= 3;
            }
        }
        else {
            _ASSERT(base_width == 3);
            TSeqPos tr_from = row_r.GetFrom();
            TSeqPos tr_to = row_r.GetToOpen();
            if ( direct ) {
                if (tr_from % 3 > 0) {
                    if (tr_from == split_codon_pos) {
                        // Include incomplete codon from the last range.
                        _ASSERT(off > 0);
                        if ( is_first_seg ) trim_from = tr_from % 3;
                        off -= tr_from % 3;
                        tr_from -= tr_from % 3;
                    }
                    else {
                        // Trim incomplete codon at range start.
                        off += 3 - (tr_from % 3);
                        tr_from += 3 - (tr_from % 3);
                    }
                }
                if (tr_to % 3 > 0) {
                    split_codon_pos = tr_to;
                    trim_to = tr_to % 3;
                    tr_to -= tr_to % 3;
                }
            }
            else /* reverse */ {
                if (tr_to % 3 > 0) {
                    if (tr_to == split_codon_pos) {
                        // Include incomplete codon from the last range.
                        _ASSERT(off > 0);
                        if ( is_first_seg ) trim_from = 3 - (tr_to % 3);
                        off -= 3 - (tr_to % 3);
                        tr_to += 3 - (tr_to % 3);
                    }
                    else {
                        off += tr_to % 3;
                        tr_to -= tr_to % 3;
                    }
                }
                if (tr_from % 3 > 0) {
                    split_codon_pos = tr_from;
                    trim_to = 3 - (tr_from % 3);
                    tr_from += 3 - (tr_from % 3);
                }
            }
            off /= 3;
            _ASSERT(tr_from % 3 == 0);
            _ASSERT(tr_to % 3 == 0);
            IAlnSegment::TSignedRange prot_r;
            prot_r.SetOpen(tr_from / 3, tr_to / 3);
            if ( direct ) {
                seq_vector.GetSeqData(prot_r.GetFrom(),
                                        prot_r.GetToOpen(), s);
            }
            else {
                seq_vector.GetSeqData(vec_size - prot_r.GetToOpen(),
                                        vec_size - prot_r.GetFrom(), s);
            }
        }

        size_t len = min(buf_size - off, s.size());
        _ASSERT(off + len <= buf_size);

        if (len > 0) {
            if ( m_AnchorDirect ) {
                buffer.replace(off, len, s, 0, len);
            }
            else {
                buffer.replace(buf_size - off - len, len, s, 0, len);
            }
        }
        ++it;
        is_first_seg = false;
    }
    if (translate  &&  aln_range.GetLength() >= trim_from + trim_to) {
        buffer.resize((aln_range.GetLength() - trim_from - trim_to) / 3);
    }
    return buffer;
}


IAlnSegmentIterator*
CSparseAln::CreateSegmentIterator(TNumrow row,
                                  const TSignedRange& range,
                                  IAlnSegmentIterator::EFlags flag) const
{
    _ASSERT(row >= 0  &&  row < GetDim());
    _ASSERT( !m_Aln->GetPairwiseAlns()[row]->empty() );
    if (m_Aln->GetPairwiseAlns()[row]->empty()) {
        string errstr = "Invalid (empty) row (" + NStr::IntToString(row) + ").  Seq id \"" +
            GetSeqId(row).AsFastaString() + "\".";
        NCBI_THROW(CAlnException, eInvalidRequest, errstr);
    }
    return new CSparse_CI(*this, row, flag, range);
}


END_NCBI_SCOPE
