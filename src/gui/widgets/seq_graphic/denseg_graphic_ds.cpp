/*  $Id: denseg_graphic_ds.cpp 44368 2019-12-05 19:37:05Z shkeda $
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
* Authors:  Lianshou Wu
*
* File Description:
*
*/

#include <ncbi_pch.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/widgets/seq_graphic/denseg_graphic_ds.hpp>
#include <gui/widgets/seq_graphic/denseg_ci.hpp>
#include <gui/utils/event_translator.hpp>


#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seq/Bioseq.hpp>

#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>



BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

static const string kTracebacks = "Tracebacks";
static const string kCigar = "CIGAR";
static const string kMismatch = "MISMATCH";

///////////////////////////////////////////////////////////////////////////////
///  CDensegGraphicDataSource

CDensegGraphicDataSource::CDensegGraphicDataSource(
    const objects::CSeq_align& align, objects::CScope& scope, TNumrow anchor)
    : m_AnchorRow(anchor)
    , m_Cigar(NULL)
{
    m_Alignment.Reset(&align);
    m_Scope.Reset(&scope);
    x_InitCigar();

    // must be a non-empty pairwise dense-seg
    _ASSERT(align.GetSegs().IsDenseg());
    _ASSERT(align.CheckNumRows() == 2);
    _ASSERT(align.GetSegs().GetDenseg().CheckNumSegs() > 0);
    // Make sure it is not a genomic-to-protein alingment.
    _ASSERT(!align.GetSegs().GetDenseg().IsSetWidths());
}


CDensegGraphicDataSource::~CDensegGraphicDataSource()
{
    // do clean up
    if (m_Cigar) {
        delete m_Cigar;
    }
}


IAlnExplorer::EAlignType CDensegGraphicDataSource::GetAlignType(void) const
{
    const objects::CBioseq_Handle& bsh = GetBioseqHandle(m_AnchorRow);
    if (bsh.IsProtein()) {
        return IAlnExplorer::fProtein;
    }
    
    return IAlnExplorer::fDNA;
}


CDensegGraphicDataSource::TNumrow
CDensegGraphicDataSource::GetNumRows(void) const
{
    return 2;
}


TSeqPos CDensegGraphicDataSource::GetAlnStart(void) const
{
    return m_Alignment->GetSeqStart(m_AnchorRow);
}


TSeqPos CDensegGraphicDataSource::GetAlnStop(void) const
{
    return m_Alignment->GetSeqStop(m_AnchorRow);
}


TSeqPos CDensegGraphicDataSource::GetSeqStart(TNumrow row) const
{
    return m_Alignment->GetSeqStart(row);
}


TSeqPos CDensegGraphicDataSource::GetSeqStop(TNumrow row) const
{
    return m_Alignment->GetSeqStop(row);
}


IAlnExplorer::TSignedRange
CDensegGraphicDataSource::GetAlnRange(void) const
{
    return TSignedRange((TSignedSeqPos)GetAlnStart(), (TSignedSeqPos)GetAlnStop());
}


IAlnExplorer::TSignedRange
CDensegGraphicDataSource::GetSeqAlnRange(TNumrow row) const
{
    // use the same alignment range
    return GetAlnRange();
}


TSeqPos CDensegGraphicDataSource::GetSeqLength(TNumrow row) const
{
    if (row != m_AnchorRow) {
        // for anchor row, we will never use CIGAR string

        if (IsCigarUnambiguous()) {
            const TCigar& cigars = *GetCigar();
            TSeqPos len = 0;
            ITERATE (TCigar, iter, cigars) {
                switch (iter->first) {
                    case eCigar_M:
                    case eCigar_Eq:
                    case eCigar_X:
                    case eCigar_I:
                    case eCigar_H:
                    case eCigar_S:
                        len += iter->second;
                        break;
                    default:
                        break;
                }
            }
            return len;
        }
    }

    return GetBioseqHandle(row).GetBioseqLength();
}


TSeqPos CDensegGraphicDataSource::GetBaseWidth(TNumrow row) const
{
    // will be 1 always for either protein-to-protein alignments
    // or genomic-to-genomic alignments
    return 1;
}


CDensegGraphicDataSource::TNumrow
CDensegGraphicDataSource::GetAnchor(void) const
{
    return m_AnchorRow;
}


const CSeq_id& CDensegGraphicDataSource::GetSeqId(TNumrow row) const
{
    return m_Alignment->GetSeq_id(row);
}


const objects::CBioseq_Handle&
CDensegGraphicDataSource::GetBioseqHandle(TNumrow row) const
{
    if (m_BioseqHandles.count(row) == 0) {
        m_BioseqHandles[row] = m_Scope->GetBioseqHandle(m_Alignment->GetSeq_id(row));
    }
    return m_BioseqHandles[row];
}


bool CDensegGraphicDataSource::IsPositiveStrand(TNumrow row) const
{
    return m_Alignment->GetSeqStrand(row) != eNa_strand_minus;
}


bool CDensegGraphicDataSource::IsNegativeStrand(TNumrow row) const
{
    return m_Alignment->GetSeqStrand(row) == eNa_strand_minus;
}


TSignedSeqPos 
CDensegGraphicDataSource::GetSeqPosFromAlnPos(TNumrow row, TSeqPos aln_pos,
                                              IAlnExplorer::ESearchDirection dir,
                                              bool try_reverse_dir) const
{
    TSignedSeqPos seq_pos = (TSignedSeqPos)aln_pos;
    if (row == m_AnchorRow) {
        return seq_pos;
    }

    CDenseg_CI curr_iter(*m_Alignment, row, m_AnchorRow,
        IAlnSegmentIterator::eSkipGaps,
        TSignedRange::GetWhole());
    CDenseg_CI pre_iter;

    while (curr_iter  &&  seq_pos > curr_iter->GetAlnRange().GetTo()) {
        pre_iter = curr_iter;
        ++curr_iter;
    }

    if (curr_iter) {
        bool reversed = curr_iter->GetType() & IAlnSegment::fReversed;
        if (curr_iter->GetAlnRange().GetFrom() > seq_pos) {
            if (pre_iter) {
                if (dir == IAlnExplorer::eRight  ||
                    (!reversed  &&  dir == IAlnExplorer::eForward)  ||
                    (reversed  &&  dir == IAlnExplorer::eBackwards)) {
                        seq_pos = reversed ? curr_iter->GetRange().GetTo() : curr_iter->GetRange().GetFrom();

                } else if (dir == IAlnExplorer::eLeft  ||
                    (reversed  &&  dir == IAlnExplorer::eForward)  ||
                    (!reversed  &&  dir == IAlnExplorer::eBackwards)) {
                        seq_pos = reversed ? pre_iter->GetRange().GetFrom() : pre_iter->GetRange().GetTo();
                }

            } else {
                if (try_reverse_dir  ||
                    dir == IAlnExplorer::eRight  ||
                    (!reversed  &&  dir == IAlnExplorer::eForward)  ||
                    (reversed  &&  dir == IAlnExplorer::eBackwards)) {
                        seq_pos = reversed ? curr_iter->GetRange().GetTo() : curr_iter->GetRange().GetFrom();
                }
            }
        } else {
            TSignedSeqPos off = reversed ?
                curr_iter->GetAlnRange().GetTo() - seq_pos :  seq_pos - curr_iter->GetAlnRange().GetFrom();
            seq_pos = curr_iter->GetRange().GetFrom() + off;
        }
    } else {
        _ASSERT(pre_iter);
        seq_pos = -1;
        if (dir != IAlnExplorer::eNone) {
            bool reversed = pre_iter->GetType() & IAlnSegment::fReversed;
            if (try_reverse_dir  ||
                dir == IAlnExplorer::eLeft  ||
                (reversed  &&  dir == IAlnExplorer::eForward)  ||
                (!reversed  &&  dir == IAlnExplorer::eBackwards)) {
                    seq_pos = reversed ? pre_iter->GetRange().GetFrom() : pre_iter->GetRange().GetTo();
            }
        }
    }

    return seq_pos;
}



TSignedSeqPos
CDensegGraphicDataSource::GetAlnPosFromSeqPos(TNumrow row,
                                              TSeqPos seq_pos,
                                              IAlnExplorer::ESearchDirection dir,
                                              bool try_reverse_dir) const
{
    TSignedSeqPos aln_pos = (TSignedSeqPos)seq_pos;
    if (row == m_AnchorRow) {
        return aln_pos;
    }

    CDenseg_CI curr_iter(*m_Alignment, row, m_AnchorRow,
        IAlnSegmentIterator::eSkipGaps,
        TSignedRange::GetWhole());
    CDenseg_CI pre_iter;

    bool reversed = IsPositiveStrand(row) != IsPositiveStrand(m_AnchorRow);

    while (curr_iter  &&  ((reversed  &&  aln_pos < curr_iter->GetRange().GetFrom())  ||
        (!reversed  &&  aln_pos > curr_iter->GetRange().GetTo()))) {
        pre_iter = curr_iter;
        ++curr_iter;
    }

    if (curr_iter) {
        bool reversed = curr_iter->GetType() & IAlnSegment::fReversed;
        if ((reversed  &&  aln_pos > curr_iter->GetRange().GetTo())  ||
            (!reversed  &&  aln_pos < curr_iter->GetRange().GetFrom())) {
            if (pre_iter) {
                if (dir == IAlnExplorer::eRight  ||
                    (!reversed  &&  dir == IAlnExplorer::eForward)  ||
                    (reversed  &&  dir == IAlnExplorer::eBackwards)) {
                        aln_pos = curr_iter->GetAlnRange().GetFrom();

                } else if (dir == IAlnExplorer::eLeft  ||
                    (reversed  &&  dir == IAlnExplorer::eForward)  ||
                    (!reversed  &&  dir == IAlnExplorer::eBackwards)) {
                        aln_pos = pre_iter->GetAlnRange().GetTo();
                }

            } else {
                if (try_reverse_dir  ||
                    dir == IAlnExplorer::eRight  ||
                    (!reversed  &&  dir == IAlnExplorer::eForward)  ||
                    (reversed  &&  dir == IAlnExplorer::eBackwards)) {
                        aln_pos = curr_iter->GetAlnRange().GetFrom();
                }
            }
        } else {
            TSignedSeqPos off = reversed ?
                curr_iter->GetRange().GetTo() - aln_pos :  aln_pos - curr_iter->GetRange().GetFrom();
            aln_pos = curr_iter->GetAlnRange().GetFrom() + off;
        }
    } else {
        _ASSERT(pre_iter);
        aln_pos = -1;
        if (dir != IAlnExplorer::eNone) {
            if (try_reverse_dir  ||
                dir == IAlnExplorer::eLeft  ||
                (reversed  &&  dir == IAlnExplorer::eForward)  ||
                (!reversed  &&  dir == IAlnExplorer::eBackwards)) {
                    aln_pos = pre_iter->GetAlnRange().GetTo();
            }
        }
    }

    return aln_pos;
}


IAlnSegmentIterator*
CDensegGraphicDataSource::CreateSegmentIterator(TNumrow row,
                                                const TSignedRange& range,
                                                IAlnSegmentIterator::EFlags flag) const
{
    return new CDenseg_CI(*m_Alignment, row, m_AnchorRow, flag, range);
}


string& 
CDensegGraphicDataSource::x_GetAlnStringFromCigar(TNumrow row, string &buffer, const TSignedRange& aln_range) const
{
    buffer.clear();
    const TCigar& cigars = *GetCigar();
    size_t f = (size_t)GetSeqPosFromAlnPos(row, aln_range.GetFrom(), IAlnExplorer::eRight);
    size_t t = (size_t)GetSeqPosFromAlnPos(row, aln_range.GetTo(), IAlnExplorer::eLeft);
    bool reversed = IsPositiveStrand(row) != IsPositiveStrand(m_AnchorRow);
    if (reversed) {
        TSeqPos len_m1 = GetSeqLength(row) - 1;
        t = len_m1 - t;
        f = len_m1 - f;
    }
    t += 1;
    _ASSERT(f < t);

    string mismatch_str = x_GetMismatchStr();
    CSeqVector seq_vec = GetBioseqHandle(m_AnchorRow).GetSeqVector(CBioseq_Handle::eCoding_Iupac);
    seq_vec.GetSeqData(aln_range.GetFrom(), aln_range.GetToOpen(), buffer);
    size_t curr_p = 0;
    size_t mis_str_idx = 0;
    size_t curr_seq_idx = 0;
    TCigar::const_iterator c_iter = cigars.begin();
    while (f < t  &&  c_iter != cigars.end()) {
        size_t span = c_iter->second;
        switch (c_iter->first) {
            case eCigar_H:
            case eCigar_S:
            case eCigar_X:
                if (curr_p + span > f) {
                    size_t offset = f - curr_p;
                    size_t left_over = span - offset;
                    mis_str_idx += offset;
                    /*
                    if ( mis_str_idx >= mismatch_str.size()) {
                        cout << "\n -------------------- " << GetSeqId(row).GetSeqIdString(true) << "\n";
                        cout << "\n -------------- buffer:" << buffer << "\n";
                        cout << "\n -------- mismatch_str:" << mismatch_str << "\n";
                        cout << "\n --------  mis_str_idx:" << mis_str_idx << "\n";
                        throw runtime_error("Bad read!");
                    } 
                    */
                    buffer.replace(curr_seq_idx, left_over, mismatch_str, mis_str_idx, left_over);
                    curr_p += span;
                    mis_str_idx += left_over;
                    curr_seq_idx += left_over;
                    f += left_over;
                } else {
                    curr_p += span;
                    mis_str_idx += span; 
                }
                break;
            case eCigar_M:
            case eCigar_Eq:
                if (curr_p + span > f) {
                    curr_p += span;
                    size_t left_over = curr_p - f;
                    curr_seq_idx += left_over;
                    f += left_over;
                } else {
                    curr_p += span;
                }
                break;
            case eCigar_D:
            case eCigar_N:
                if (curr_p + span > f  &&  curr_seq_idx > 0) {
                    buffer.replace(curr_seq_idx, span, span, '-');
                    curr_seq_idx += span;
                }
                break;
            case eCigar_I:
                mis_str_idx += span;
                curr_p += span;
                if (curr_p > f) {
                    f = curr_p;
                }
                break;
            default:
                break;
        }
        ++c_iter;
    }
    return buffer;
}

string&
CDensegGraphicDataSource::GetAlnSeqString(TNumrow row, 
                                          string &buffer,
                                          const TSignedRange& aln_range) const
{
    // assume the anchor row is the reference sequence row
    if (row == m_AnchorRow) {
        CSeqVector seq_vec = GetBioseqHandle(m_AnchorRow).GetSeqVector(CBioseq_Handle::eCoding_Iupac);
        seq_vec.GetSeqData(aln_range.GetFrom(), aln_range.GetToOpen(), buffer);
        return buffer;
    } 
    
    if (IsCigarUnambiguous()) {
        try {
            return x_GetAlnStringFromCigar(row, buffer, aln_range);
        } catch (exception& e) {
            ERR_POST(Error << e.what());
        }    
    }

    buffer.clear();
    bool reversed = IsPositiveStrand(row) != IsPositiveStrand(m_AnchorRow);
    const CBioseq_Handle& bsh = GetBioseqHandle(row);
    if (!bsh)
        return buffer;
    CSeqVector seq_vec = bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac);
    buffer.resize((size_t)aln_range.GetLength());
    CDenseg_CI curr_iter(*m_Alignment, row, m_AnchorRow,
        IAlnSegmentIterator::eAllSegments,
        aln_range);
    TSignedSeqPos f = aln_range.GetFrom();
    TSignedSeqPos t = aln_range.GetToOpen();
    size_t curr_seq_idx = 0; 
    while (curr_iter  &&  f < t) {
        if (curr_iter->GetAlnRange().NotEmpty()) {
            TSignedSeqPos curr_f = curr_iter->GetAlnRange().GetFrom();
            TSignedSeqPos span = curr_iter->GetAlnRange().GetLength();
            if (f < curr_f) {
                size_t off = curr_f - f;
                buffer.replace(curr_seq_idx, off, off, '-');
                f += off;
                curr_seq_idx += off;
            }
            TSignedSeqPos offset = f - curr_f;
            TSignedSeqPos left_over = span - offset;
            if (t < curr_f + span) {
                left_over = t - f;
            }

            if (curr_iter->GetRange().Empty()) {
                if (curr_seq_idx < buffer.length())
                    buffer.replace(curr_seq_idx, left_over, left_over, '-');
            } else {
                string tmp_str;
                TSignedSeqPos row_f = curr_iter->GetRange().GetFrom();
                TSignedSeqPos row_t = curr_iter->GetRange().GetTo();
                if (reversed) {
                    row_t -= offset;
                    row_f = row_t - left_over + 1;
                } else {
                    row_f += offset;
                    row_t = row_f + left_over - 1;
                }
                seq_vec.GetSeqData(row_f, row_t + 1, tmp_str);
                if (reversed) {
                    string tmp_seq;
                    CSeqManip::ReverseComplement(tmp_str, CSeqUtil::e_Iupacna,
                        0, tmp_str.length(), tmp_seq);
                    swap(tmp_str, tmp_seq);
                }
                if (curr_seq_idx < buffer.length())
                    buffer.replace(curr_seq_idx, left_over, tmp_str, 0, left_over);
            }

            curr_seq_idx += left_over;
            f += left_over;
        }
        ++curr_iter;
    }

    return buffer;
}


string&
CDensegGraphicDataSource::GetSeqString(string &buffer, TNumrow row,
                                       const TSignedRange& seq_rng,
                                       const TSignedRange& aln_rng,
                                       bool anchor_direct) const
{
    if (row != m_AnchorRow  &&  IsCigarUnambiguous()) {
        const TCigar& cigars = *GetCigar();
        bool reversed = IsPositiveStrand(row) != IsPositiveStrand(m_AnchorRow);
        size_t f = (size_t)seq_rng.GetFrom();
        size_t t = (size_t)seq_rng.GetTo();
        if (reversed) {
            TSeqPos len_m1 = GetSeqLength(row) - 1;
            f = len_m1 - f;
            t = len_m1 - t;
        }
        t += 1;
        _ASSERT(f < t);

        buffer.clear();
        string mismatch_str = x_GetMismatchStr();
        string anchor_seq;

        CSeqVector seq_vec = GetBioseqHandle(m_AnchorRow).GetSeqVector(CBioseq_Handle::eCoding_Iupac);
        seq_vec.GetSeqData(aln_rng.GetFrom(), aln_rng.GetToOpen(), anchor_seq);

        size_t curr_p = 0;
        size_t mis_str_idx = 0;
        size_t curr_seq_idx = 0;
        TCigar::const_iterator c_iter = cigars.begin();
        while (f < t  &&  c_iter != cigars.end()) {
            size_t span = c_iter->second;
            switch (c_iter->first) {
                    case eCigar_H:
                    case eCigar_S:
                    case eCigar_X:
                        if (curr_p + span > f) {
                            size_t offset = f - curr_p;
                            size_t left_over = span - offset;
                            mis_str_idx += offset;
                            buffer += mismatch_str.substr(mis_str_idx, left_over);
                            curr_p += span;
                            mis_str_idx += left_over;
                            curr_seq_idx += left_over;
                            f += left_over;
                        } else {
                            curr_p += span;
                            mis_str_idx += span; 
                        }
                        break;
                    case eCigar_M:
                    case eCigar_Eq:
                        if (curr_p + span > f) {
                            curr_p += span;
                            size_t left_over = curr_p - f;
                            buffer += anchor_seq.substr(curr_seq_idx, left_over);
                            curr_seq_idx += left_over;
                            f += left_over;
                        } else {
                            curr_p += span;
                        }
                        break;
                    case eCigar_D:
                    case eCigar_N:
                        if (curr_p + span > f  &&  curr_seq_idx > 0) {
                            curr_seq_idx += span;
                        }
                        break;
                    case eCigar_I:
                        if (curr_p + span > f) {
                            size_t offset = f - curr_p;
                            size_t left_over = span - offset;
                            mis_str_idx += offset;
                            buffer += mismatch_str.substr(mis_str_idx, left_over);
                            mis_str_idx += left_over;
                            f += left_over;
                        } else {
                            mis_str_idx += span; 
                        }
                        curr_p += span;
                        break;
                    default:
                        break;
            }
            ++c_iter;
        }

        if (!anchor_direct  &&  reversed) {
            // nucleotide sequence, get reverse and complement
            string tmp_seq;
            CSeqManip::ReverseComplement(buffer, CSeqUtil::e_Iupacna,
                0, buffer.length(), tmp_seq);
            buffer = tmp_seq;
        }

    } else {
        IAlnGraphicDataSource::GetSeqString(buffer, row, seq_rng, aln_rng, anchor_direct);
    }

    return buffer;
}


TSignedSeqPos
CDensegGraphicDataSource::GetSeqPosFromSeqPos(TNumrow for_row,
                                              TNumrow row, TSeqPos seq_pos) const
{
    if (row == m_AnchorRow) {
        return GetSeqPosFromAlnPos(for_row, seq_pos);
    }
    return GetAlnPosFromSeqPos(row, seq_pos);
}


CDensegGraphicDataSource::EPolyATail
CDensegGraphicDataSource::HasPolyATail() const
{
    return ePolyA_Unknown;
}


void CDensegGraphicDataSource::x_InitCigar()
{
    if (m_AnchorRow != 0  ||  m_Cigar != NULL) {
        // We assume the CIGAR is stored for BAM/cSRA files only,
        // and the anchor row must be 0. Otherwise, the behaviour
        // will be incorrect. 
        return;
    }

    string cigar_str = x_GetCigarStr();
    if ( !cigar_str.empty() ) {
        m_IsCigarUnambiguous = true;
        TCigar cigar;
        // VDB CIGAR representation is different between BAM and SRA alignmnets
        // SRA CIGAR == length/type pair [0-9][MIDNSHPX=]
        // BAM CIGAR == type/length pair [MIDNSHPX=][0-9]
        bool is_bam = !isdigit(cigar_str[0]);

        size_t i = 0;
        const size_t c_len = cigar_str.length();
        while (i < c_len) {
            size_t len_start = is_bam ? i + 1 : i;
            size_t len_end = len_start;
            size_t type_index = i;
            while (len_end < c_len  &&  cigar_str[len_end] < '=') {
                ++len_end;
            }
            if (is_bam) {
                i = len_end;
            } else {
                type_index = len_end;
                i = len_end + 1;
            }
            ECigarOp c_op = (ECigarOp)cigar_str[type_index];
            if (c_op == 0)
                continue;
            if (is_bam && m_IsCigarUnambiguous && c_op == eCigar_M) {
                m_IsCigarUnambiguous = false;
            }
            auto len = NStr::StringToSizet(cigar_str.substr(len_start, len_end - len_start));
            cigar.emplace_back(c_op, len);
        }

        if (!cigar.empty()) {
            _ASSERT(m_Cigar == nullptr);
            m_Cigar = new TCigar(move(cigar));
        } 
    }
}


string CDensegGraphicDataSource::x_GetCigarStr() const
{
    if (m_Alignment->CanGetExt()) {
        ITERATE (CSeq_align::TExt, iter, m_Alignment->GetExt()) {
            if ((*iter)->GetType().IsStr()  &&
                (*iter)->GetType().GetStr() == kTracebacks) {
                CConstRef<CUser_field> cigar_field = (*iter)->GetFieldRef(kCigar);
                if (cigar_field  &&  cigar_field->GetData().IsStr()) {
                    return cigar_field->GetData().GetStr();
                }
            }
        }
    }

    return kEmptyStr;
}


string CDensegGraphicDataSource::x_GetMismatchStr() const
{
    if (m_Alignment->CanGetExt()) {
        ITERATE (CSeq_align::TExt, iter, m_Alignment->GetExt()) {
            if ((*iter)->GetType().IsStr()  &&
                (*iter)->GetType().GetStr() == kTracebacks) {
                CConstRef<CUser_field> mismatch_field = (*iter)->GetFieldRef(kMismatch);
                if (mismatch_field  &&  mismatch_field->GetData().IsStr()) {
                    return mismatch_field->GetData().GetStr();
                }
            }
        }
    }

    return kEmptyStr;
}


END_NCBI_SCOPE
