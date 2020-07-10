/*  $Id: sparse_alignment.cpp 25706 2012-04-25 00:21:04Z voronov $
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

#include <gui/objutils/sparse_alignment.hpp>
#include <gui/objutils/sparse_iterator.hpp>

#include <objects/seqalign/Sparse_align.hpp>
#include <objmgr/seq_vector.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


///////////////////////////////////////////////////////////////////////////////
///  CSparseAlignment::SRowRec

CSparseAlignment::SRowRec::SRowRec()
:   m_AlignedSeq(NULL),
    m_BioseqHandle(NULL)
{
}


CSparseAlignment::SRowRec::~SRowRec()
{
    delete m_AlignedSeq;
    delete m_BioseqHandle;
}

///////////////////////////////////////////////////////////////////////////////
///  CSparseAlignment
CSparseAlignment::CSparseAlignment()
:   m_GapChar('-')
{
}


CSparseAlignment::~CSparseAlignment()
{
}


void CSparseAlignment::Init(const CSeq_id& master_id,
                            vector<SAlignedSeq*>& aln_seqs,
                            objects::CScope& scope)
{
    Clear();

    if( ! aln_seqs.empty()) {
        m_MasterId.Reset(&master_id);
        m_Scope.Reset(&scope);

        size_t aln_n = aln_seqs.size();
        m_Rows.reserve(aln_n + 1);
        for( size_t i = 0; i < aln_n; i++ ) {
            SRowRec* rec = new SRowRec;
            rec->m_AlignedSeq = aln_seqs[i];
            m_Rows.push_back(rec);

            // update alignment extent
            CRange<TSignedSeqPos> coll_r =
                rec->m_AlignedSeq->m_AlignColl->GetFirstRange();
            if(m_Rows.size() == 1)  {
                m_AlnRange = coll_r;
            } else {
                m_AlnRange.CombineWith(coll_r);
            }
        }
    }

    // create a fake collection for master sequence
    SRowRec* rec = new SRowRec();
    rec->m_AlignedSeq = new SAlignedSeq();
    rec->m_AlignedSeq->m_SeqId = m_MasterId;
    TAlignRange aln_r(m_AlnRange.GetFrom(), m_AlnRange.GetFrom(), m_AlnRange.GetLength());
    rec->m_AlignedSeq->m_AlignColl = new TAlignColl();
    rec->m_AlignedSeq->m_AlignColl->insert(aln_r);
    rec->m_AlignedSeq->m_SecondRange.SetFrom(m_AlnRange.GetFrom()).SetLength(m_AlnRange.GetLength());

    m_Rows.push_back(rec);
}


void CSparseAlignment::Clear()
{
    NON_CONST_ITERATE(TRows, it, m_Rows)    {
        delete *it;
    }
    m_Rows.clear();

    m_AlnRange.Set(0, 0);
}


void CSparseAlignment::SetGapChar(TResidue gap_char)
{
    m_GapChar = gap_char;
}


CRef<CScope> CSparseAlignment::GetScope()
{
    return m_Scope;
}


CSparseAlignment::TNumrow CSparseAlignment::GetNumRows() const
{
    return (CSparseAlignment::TNumrow)m_Rows.size();
}


CSparseAlignment::TARange CSparseAlignment::GetAlnRange() const
{
    return m_AlnRange;
}


const CSparseAlignment::TAlignColl&
    CSparseAlignment::GetAlignCollection(TNumrow row)
{
    _ASSERT(row >= 0  &&  row < (TNumrow) m_Rows.size());
    return *(m_Rows[row]->m_AlignedSeq->m_AlignColl);
}


bool    CSparseAlignment::IsSetAnchor()   const
{
    return true;
}


CSparseAlignment::TNumrow CSparseAlignment::GetAnchor() const
{
    return (CSparseAlignment::TNumrow)(m_Rows.size() - 1);
}


const CSeq_id& CSparseAlignment::GetSeqId(TNumrow row) const
{
    x_AssertRowValid(row);
    return m_Rows[row]->m_AlignedSeq->m_SeqId.GetObject();
}


TSignedSeqPos   CSparseAlignment::GetSeqAlnStart(TNumrow row) const
{
    x_AssertRowValid(row);
    const TAlignColl& coll = *m_Rows[row]->m_AlignedSeq->m_AlignColl;
    return coll.GetFirstFrom();
}


TSignedSeqPos CSparseAlignment::GetSeqAlnStop(TNumrow row) const
{
    x_AssertRowValid(row);
    const TAlignColl& coll = *m_Rows[row]->m_AlignedSeq->m_AlignColl;
    return coll.GetFirstTo();
}


CSparseAlignment::TSignedRange CSparseAlignment::GetSeqAlnRange(TNumrow row) const
{
    x_AssertRowValid(row);
    return m_Rows[row]->m_AlignedSeq->m_AlignColl->GetFirstRange();
}


TSeqPos CSparseAlignment::GetSeqStart(TNumrow row) const
{
    x_AssertRowValid(row);
    return m_Rows[row]->m_AlignedSeq->m_SecondRange.GetFrom();
}


TSeqPos CSparseAlignment::GetSeqStop(TNumrow row) const
{
    x_AssertRowValid(row);
    return m_Rows[row]->m_AlignedSeq->m_SecondRange.GetTo();
}


IAlnExplorer::TRange CSparseAlignment::GetSeqRange(TNumrow row) const
{
    x_AssertRowValid(row);
    TSignedRange& r = m_Rows[row]->m_AlignedSeq->m_SecondRange;
    return IAlnExplorer::TRange(r.GetFrom(), r.GetTo());
}


bool CSparseAlignment::IsPositiveStrand(TNumrow row) const
{
    x_AssertRowValid(row);
    return ! m_Rows[row]->m_AlignedSeq->m_NegativeStrand;
}


bool CSparseAlignment::IsNegativeStrand(TNumrow row) const
{
    x_AssertRowValid(row);
    return m_Rows[row]->m_AlignedSeq->m_NegativeStrand;
}


inline  CSparseAlignment::TAlignColl::ESearchDirection
    GetCollectionSearchDirection(IAlnExplorer::ESearchDirection dir)
{
    typedef CSparseAlignment::TAlignColl   T;
    switch(dir) {
    case IAlnExplorer::eNone:
        return T::eNone;
    case IAlnExplorer::eLeft:
        return T::eLeft;
    case IAlnExplorer::eRight:
        return T::eRight;
    case IAlnExplorer::eForward:
        return T::eForward;
    case IAlnExplorer::eBackwards:
        return T::eBackwards;
    }
    _ASSERT(false); // invalid
    return T::eNone;
}


TSignedSeqPos CSparseAlignment::GetAlnPosFromSeqPos(TNumrow row, TSeqPos seq_pos,
                                                    IAlnExplorer::ESearchDirection dir,
                                                    bool try_reverse_dir) const
{
    x_AssertRowValid(row);

    const TAlignColl& coll = *m_Rows[row]->m_AlignedSeq->m_AlignColl;
    TAlignColl::ESearchDirection c_dir = GetCollectionSearchDirection(dir);
    return coll.GetFirstPosBySecondPos(seq_pos, c_dir);
}


TSignedSeqPos CSparseAlignment::GetSeqPosFromAlnPos(TNumrow row, TSeqPos aln_pos,
                                                    IAlnExplorer::ESearchDirection dir,
                                                    bool try_reverse_dir) const
{
    x_AssertRowValid(row);

    const TAlignColl& coll = *m_Rows[row]->m_AlignedSeq->m_AlignColl;
    TAlignColl::ESearchDirection c_dir = GetCollectionSearchDirection(dir);
    return coll.GetSecondPosByFirstPos(aln_pos, c_dir);
}


string& CSparseAlignment::GetSeqString(TNumrow row, string &buffer,
                                       TSeqPos seq_from, TSeqPos seq_to) const
{
    x_AssertRowValid(row);

    buffer.erase();
    if(seq_to >= seq_from)   {
        const CBioseq_Handle& handle = GetBioseqHandle(row);
        bool positive = ! m_Rows[row]->m_AlignedSeq->m_NegativeStrand;
        CBioseq_Handle::EVectorStrand strand = positive ?
                CBioseq_Handle::eStrand_Plus : CBioseq_Handle::eStrand_Minus;

        CSeqVector seq_vector =
            handle.GetSeqVector(CBioseq_Handle::eCoding_Iupac, strand);

        size_t size = seq_to - seq_from + 1;
        buffer.resize(size, m_GapChar);

        if(positive)    {
            seq_vector.GetSeqData(seq_from, seq_to + 1, buffer);
        } else {
            TSeqPos vec_size = seq_vector.size();
            seq_vector.GetSeqData(vec_size - seq_to - 1, vec_size - seq_from, buffer);
        }
    }
    return buffer;
}


string& CSparseAlignment::GetSeqString(TNumrow row, string &buffer,
                                       const IAlnExplorer::TRange &seq_range) const
{
    return GetSeqString(row, buffer, seq_range.GetFrom(), seq_range.GetTo());
}


string& CSparseAlignment::GetAlnSeqString(TNumrow row, string &buffer,
                                          const TSignedRange &aln_range) const
{
    x_AssertRowValid(row);

    buffer.erase();

    if(aln_range.GetLength() > 0)   {
        const CBioseq_Handle& handle = GetBioseqHandle(row);
        bool positive = ! m_Rows[row]->m_AlignedSeq->m_NegativeStrand;
        CBioseq_Handle::EVectorStrand strand = positive ?
                CBioseq_Handle::eStrand_Plus : CBioseq_Handle::eStrand_Minus;

        CSeqVector seq_vector =
            handle.GetSeqVector(CBioseq_Handle::eCoding_Iupac, strand);
        TSeqPos vec_size = seq_vector.size();

        // buffer holds sequence for "aln_range", 0 index corresonds to aln_range.GetFrom()
        size_t size = aln_range.GetLength();
        buffer.resize(size, ' ');

        // check whether we have a gap at start position
        TAlignColl& coll = *m_Rows[row]->m_AlignedSeq->m_AlignColl;
        size_t prev_to_open = (coll.GetFirstFrom() > aln_range.GetFrom()) ? string::npos : 0;

        string s;
        CSparseIterator it(*m_Rows[row]->m_AlignedSeq->m_AlignColl, IAlnSegmentIterator::eSkipGaps, aln_range);

        //LOG_POST("GetAlnSeqString(" << row << ") ==========================================" );
        while(it)   {
            const IAlnSegment::TSignedRange& aln_r = it->GetAlnRange(); // in alignment
            const IAlnSegment::TSignedRange& r = it->GetRange(); // on sequence
            //LOG_POST("Aln [" << aln_r.GetFrom() << ", " << aln_r.GetTo() << "], Seq  "
            //                 << r.GetFrom() << ", " << r.GetTo());

            // TODO performance issue - waiting for better API
            if(positive)    {
                seq_vector.GetSeqData(r.GetFrom(), r.GetTo() + 1, s);
            } else {
                seq_vector.GetSeqData(vec_size - r.GetTo() - 1,
                                      vec_size - r.GetFrom(), s);
            }
            /*if(it->IsReversed())    {
                std::reverse(s.begin(), s.end());
            }*/
            size_t off = max((TSignedSeqPos) 0, aln_r.GetFrom() - aln_range.GetFrom());
            size_t len = min(buffer.size() - off, s.size());

            if(prev_to_open != string::npos) {   // this is not the first segement
                int gap_size = (int)(off - prev_to_open);
                buffer.replace(prev_to_open, gap_size, gap_size, m_GapChar);
            }

            _ASSERT(off + len <= buffer.size());

            buffer.replace(off, len, s, 0, len);
            prev_to_open = off + len;
            ++it;
        }
        int fill_len = (int)(size - prev_to_open);
        if(prev_to_open != string::npos  &&  fill_len > 0  &&  coll.GetFirstTo() > aln_range.GetTo()) {
            // there is gap on the right
            buffer.replace(prev_to_open, fill_len, fill_len, m_GapChar);
        }
        //LOG_POST(buffer);
    }
    return buffer;
}


const CBioseq_Handle&  CSparseAlignment::GetBioseqHandle(TNumrow row) const
{
    x_AssertRowValid(row);

    SRowRec& rec = *m_Rows[row];
    if(rec.m_BioseqHandle == NULL)  {
        rec.m_BioseqHandle = new CBioseq_Handle();
        *rec.m_BioseqHandle = m_Scope->GetBioseqHandle(*rec.m_AlignedSeq->m_SeqId);
    }
    return *rec.m_BioseqHandle;
}


IAlnSegmentIterator*
    CSparseAlignment::CreateSegmentIterator(TNumrow row,
                                    const IAlnExplorer::TSignedRange& range,
                                    IAlnSegmentIterator::EFlags flag) const
{
    x_AssertRowValid(row);
    const SRowRec& rec = *m_Rows[row];
    return new CSparseIterator(*rec.m_AlignedSeq->m_AlignColl, flag, range);
}


void CSparseAlignment::x_AssertRowValid(TNumrow row) const
{
    _ASSERT(row >= 0  &&  row < (TNumrow) m_Rows.size());
}


END_NCBI_SCOPE
