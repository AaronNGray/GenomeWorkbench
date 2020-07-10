/*  $Id: sparse_functions.cpp 25706 2012-04-25 00:21:04Z voronov $
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

#include <gui/objutils/sparse_functions.hpp>

#include <corelib/ncbitime.hpp>
#include <objects/seqalign/Sparse_align.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqloc/Seq_id.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


// Conversion function CSparse_align -> SAlignedSeq
SAlignedSeq* CreateAlignRow(const CSparse_align& align, bool master_first)
{
    auto_ptr<SAlignedSeq> aln_seq(new SAlignedSeq());
    aln_seq->m_SeqId.Reset(master_first ? &align.GetSecond_id()
                                        : &align.GetFirst_id());
    SAlignedSeq::TSignedRange& range = aln_seq->m_SecondRange;

    // get references to the containers inside CSparse_align
    const CSparse_align::TFirst_starts& starts_1 = align.GetFirst_starts();
    const CSparse_align::TSecond_starts& starts_2 = align.GetSecond_starts();
    const CSparse_align::TLens& lens = align.GetLens();
    const CSparse_align::TSecond_strands* strands =
        align.IsSetSecond_strands() ? &align.GetSecond_strands() : 0;

    // create a new Align Collection
    SAlignedSeq::TAlignColl* coll = new SAlignedSeq::TAlignColl();
    range.SetFrom(0).SetLength(0);
    SAlignedSeq::TPos aln_from = -1, from = -1;

    // iterate on Sparse-seg elements
    typedef CSparse_align::TNumseg TNumseg;
    for( TNumseg i = 0;  i < align.GetNumseg(); i++  )  {
        aln_from = master_first ? starts_1[i] : starts_2[i];
        from = master_first ? starts_2[i] : starts_1[i];
        SAlignedSeq::TPos len = lens[i];
        bool dir = strands ? ((*strands)[i] == eNa_strand_plus) : true;

        // update range
        if(coll->empty())    {
            range.SetFrom(aln_from);
            range.SetLength(len);
        } else {
            range.SetFrom(min(range.GetFrom(), aln_from));
            range.SetToOpen(max(range.GetToOpen(), aln_from + len));
        }

        coll->insert(SAlignedSeq::TAlignRange(aln_from, from, len, dir));
    }
    aln_seq->m_AlignColl = coll;

    int dir = (coll->GetFlags() & SAlignedSeq::TAlignColl::fMixedDir);
    if(dir == SAlignedSeq::TAlignColl::fMixedDir)    {
        // incorrect - do not return anything
        return NULL;
    } else if(dir == SAlignedSeq::TAlignColl::fReversed) {
        aln_seq->m_NegativeStrand = true;
    }
    return aln_seq.release();
}

/// Converter
bool ConvertToPairwise(const CSeq_align& align,
                       const CSeq_id& master_id,
                       vector<SAlignedSeq*>& aln_seqs)
{
    typedef CSeq_align::TSegs TSegs;
    const TSegs& segs = align.GetSegs();

    switch(segs.Which())    {
    case TSegs::e_Denseg: {
        const CDense_seg& dense_seg = segs.GetDenseg();
        // find the row corresponding to master_id
        const CDense_seg::TIds& ids = dense_seg.GetIds();
        for( CDense_seg::TDim row = 0;  row < dense_seg.GetDim();  row++ )  {
            if(ids[row]->Equals(master_id)) {
                return ConvertToPairwise(dense_seg, row, aln_seqs);
            }
        }
        return false;
    }
    /// add code to support other types of Segs
    default:
        return false;
    }
}


/// Converter
bool ConvertToPairwise(const vector< CConstRef<objects::CSeq_align> >& aligns,
                       const CSeq_id& master_id,
                       vector<SAlignedSeq*>& aln_seqs)
{
    CStopWatch timer;
    timer.Start();

    bool ok = false;
    for( size_t i = 0;  i < aligns.size();  i++ )   {
        const CSeq_align& align = *aligns[i];
        bool res = ConvertToPairwise(align, master_id, aln_seqs);
        ok |= res;
    }
    LOG_POST("ConvertToPairwise( vector of CSeq_align) " << 1000 * timer.Elapsed() << " ms");
    return ok;
}


/// Converter CSparse_seg -> SAlignedSeq-s
bool ConvertToPairwise(const CSparse_seg& sparse_seg, vector<SAlignedSeq*>& aln_seqs)
{
    CConstRef<objects::CSeq_id> master_id(&sparse_seg.GetMaster_id());

    typedef CSparse_seg::TRows  TRows;
    const TRows& rows = sparse_seg.GetRows();
    TRows::const_iterator it = rows.begin();

    // convert pairwise alignment to TAlignColl objects
    for(  ;  it != rows.end();  ++it    )   {
        const CSparse_align& align = **it;

        int master_index = -1;
        if(master_id->Compare(align.GetFirst_id()) == CSeq_id::e_YES) {
            master_index = 0;
        } else if(master_id->Compare(align.GetSecond_id()) == CSeq_id::e_YES) {
            master_index = 1;
        }

        if(master_index != -1)  {   // create an alignment row from this CSparse_align
            SAlignedSeq* aln_seq = CreateAlignRow(align, master_index == 0);
            if(aln_seq) {
                aln_seqs.push_back(aln_seq);
            }
        } else {
            LOG_POST(Error << "CreateAlignRow() - a CSparse_align is"
                     << "invalid, neither of its CSeq_ids match master id");
        }
    }
    return true; // handle errors
}


bool ConvertToPairwise(const CDense_seg& dense_seg,
                       CDense_seg::TDim anchor_row,
                       vector<SAlignedSeq*>& aln_seqs)
{
    typedef CDense_seg::TDim    TDim;

    for(TDim row = 0;  row < dense_seg.GetDim();  row++ )   {
        if(row != anchor_row)   {
            SAlignedSeq* aln_seq = CreateAlignRow(dense_seg, anchor_row, row);
            if(aln_seq) {
                aln_seqs.push_back(aln_seq);
            }
        }
    }
    return true;
}


/// Builder function
CSparseAlignment* BuildSparseAlignment(const CSeq_id& master_id,
                                       vector<SAlignedSeq*>& aln_seqs,
                                       objects::CScope& scope)
{
    if(! aln_seqs.empty()) {
        CSparseAlignment* aln = new CSparseAlignment();
        aln->Init(master_id, aln_seqs, scope);
        return aln;
    }
    return NULL;
}


/// Builder function
CAlnVec* BuildDenseAlignment(const CSeq_id& master_id,
                             vector<SAlignedSeq*>& aln_seqs,
                             objects::CScope& scope)
{
    if(! aln_seqs.empty()) {
    }
    return NULL;
}


/// Converter Helper function
/// Creates an Align Collection from the two rows of a CDense_seg
SAlignedSeq*  CreateAlignRow(const CDense_seg& dense_seg,
                                CDense_seg::TDim row_1,
                                CDense_seg::TDim row_2)
{
    _ASSERT(row_1 >=0  &&  row_1 < dense_seg.GetDim());
    _ASSERT(row_2 >=0  &&  row_2 < dense_seg.GetDim());

    auto_ptr<SAlignedSeq> aln_seq(new SAlignedSeq());
    aln_seq->m_SeqId.Reset(dense_seg.GetIds()[row_2]);
    SAlignedSeq::TSignedRange& range = aln_seq->m_SecondRange;

    aln_seq->m_AlignColl = new SAlignedSeq::TAlignColl();
    SAlignedSeq::TAlignColl& coll = *aln_seq->m_AlignColl;

    typedef CDense_seg::TDim TDim;
    typedef CDense_seg::TNumseg TNum;

    const CDense_seg::TStarts& starts = dense_seg.GetStarts();
    const CDense_seg::TLens& lens = dense_seg.GetLens();
    const CDense_seg::TStrands* strands =
        dense_seg.IsSetStrands() ? &dense_seg.GetStrands() : NULL;

    // iterate by segements and add aligned segments to the collection
    TDim n_rows = dense_seg.GetDim();
    TNum n_seg = dense_seg.GetNumseg();
    for( TNum i = 0; i < n_seg;  i++ )  {
        int offset = i * n_rows;
        int from_1 = starts[row_1 + offset];
        int from_2 = starts[row_2 + offset];

        if(from_1 != -1  &&  from_2 != -1)  { // not a gap
            int len = lens[i];
            bool direct = true;
            if(strands) {
                bool minus_1 = (*strands)[row_1 + offset] == eNa_strand_minus;
                bool minus_2 = (*strands)[row_2 + offset] == eNa_strand_minus;
                direct = (! minus_1  &&  ! minus_2)  ||  (minus_1 == minus_2);
            }
            coll.insert(SAlignTools::TAlignRange(from_1, from_2, len, direct));

            // update range
            if(coll.empty())    {
                range.SetFrom(from_1);
                range.SetLength(len);
            } else {
                range.SetFrom(min(range.GetFrom(), from_1));
                range.SetToOpen(max(range.GetToOpen(), from_1 + len));
            }
        }
    }
    //LOG_POST("GetAlignColl() rows [" << row_1 << ", " << row_2 << "]" << ",  segments " << coll.size());

    _ASSERT((coll.GetFlags() & SAlignTools::TAlignColl::fInvalid) == 0);
    return aln_seq.release();
}


/// Creates Align Collection from a CSparse_seg
void GetAlignColl(const CSparse_align& sparse_align,
                  const CSeq_id& master_id,
                  SAlignTools::TAlignColl& coll)
{
    coll.clear();

    int index = -1;
    if(master_id.Compare(sparse_align.GetFirst_id()) == CSeq_id::e_YES) {
        index = 0;
    } else if(master_id.Compare(sparse_align.GetSecond_id()) == CSeq_id::e_YES) {
        index = 1;
    }
    if(index != -1) {
        bool first = (index == 0);
        const CSparse_align::TFirst_starts& starts_1 = sparse_align.GetFirst_starts();
        const CSparse_align::TFirst_starts& starts_2 = sparse_align.GetSecond_starts();
        const CSparse_align::TLens& lens = sparse_align.GetLens();
        const CSparse_align::TSecond_strands* strands =
            sparse_align.IsSetSecond_strands() ? &sparse_align.GetSecond_strands() : 0;

        typedef CSparse_align::TNumseg TNumseg;
        TNumseg n_seg = sparse_align.GetNumseg();
        for( TNumseg i = 0;  i < n_seg;  i++ )  {
            int from_1 = first ? starts_1[i] : starts_2[i];
            int from_2 = first ? starts_2[i] : starts_1[i];
            int len = lens[i];
            bool direct = strands  &&  ((*strands)[i] == eNa_strand_minus);

            coll.insert(SAlignTools::TAlignRange(from_1, from_2, len, direct));
        }
    }
}


/// Reverse Converter
/// Converts Align Collection into a CSparse_align
CRef<CSparse_align> CreateSparseAlign(const CSeq_id& id_1,
                                      const CSeq_id& id_2,
                                      const SAlignTools::TAlignColl& coll)
{
    CRef<CSparse_align> align(new CSparse_align());

    CRef<CSeq_id> rid_1(new CSeq_id());
    rid_1->Assign(id_1);
    align->SetFirst_id(*rid_1);

    CRef<CSeq_id> rid_2(new CSeq_id());
    rid_2->Assign(id_2);
    align->SetSecond_id(*rid_2);

    // initilize containers
    typedef CSparse_align::TNumseg TNumseg;
    TNumseg n_seg = (TNumseg)coll.size();
    align->SetNumseg(n_seg);

    CSparse_align::TFirst_starts& starts_1 = align->SetFirst_starts();
    starts_1.resize(n_seg);
    CSparse_align::TFirst_starts& starts_2 = align->SetSecond_starts();
    starts_2.resize(n_seg);
    CSparse_align::TLens& lens = align->SetLens();
    lens.resize(n_seg);

    CSparse_align::TSecond_strands* strands = NULL;
    if(coll.GetFlags()  &  SAlignTools::TAlignColl::fReversed) {
        // there are reversed segments in the collection - need to fill "Strands"
        strands = &align->SetSecond_strands();
        strands->resize(n_seg);
    }

    // move data to the containers
    TNumseg i = 0;
    ITERATE(SAlignTools::TAlignColl, it, coll)   {
        const SAlignTools::TAlignRange& r = *it;

        starts_1[i] = r.GetFirstFrom();
        starts_2[i] = r.GetSecondFrom();
        lens[i] = r.GetLength();
        if(strands)  {
            (*strands)[i] = r.IsDirect() ? eNa_strand_plus : eNa_strand_minus;
        }
        i++;
    }

    return align;
}


END_NCBI_SCOPE
