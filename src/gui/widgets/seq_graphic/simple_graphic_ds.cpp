/*  $Id: simple_graphic_ds.cpp 37049 2016-11-30 19:40:50Z shkeda $
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
#include <gui/widgets/seq_graphic/simple_graphic_ds.hpp>
#include <objects/seq/Bioseq.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


/// Fake segment iterator that always returns no segment.
class CSimpleSeg_CI : public IAlnSegmentIterator
{
public:
    class CFakeSegment : public IAlnSegment
    {
    public:
        CFakeSegment(void) : m_EmptyRange(TSignedRange::GetEmpty()) {}
        virtual operator bool(void) const { return false; }
        virtual TSegTypeFlags GetType(void) const {return fInvalid; }
        virtual const TSignedRange& GetAlnRange(void) const
        { return m_EmptyRange; }
        virtual const TSignedRange& GetRange(void) const
        { return m_EmptyRange; }

    private:
        TSignedRange m_EmptyRange;
    };

    /// Create 'empty' iterator.
    CSimpleSeg_CI(void) {};
    virtual ~CSimpleSeg_CI(void) {};

    /// Create a copy of the iterator.
    virtual IAlnSegmentIterator* Clone(void) const
    {return new CSimpleSeg_CI(*this); }

    /// Return true if iterator points to a valid segment
    virtual operator bool(void) const
    { return false; }

    // Postfix operators are not defined to avoid performance overhead.
    virtual IAlnSegmentIterator& operator++(void)
    { return *this; }

    virtual bool operator==(const IAlnSegmentIterator& it) const
    { return false; }

    virtual bool operator!=(const IAlnSegmentIterator& it) const
    { return true; }

    virtual const value_type& operator*(void) const
    { return m_EmptySeg; }

    virtual const value_type* operator->(void) const
    { return &m_EmptySeg; }

private:
    CFakeSegment m_EmptySeg;
};

///////////////////////////////////////////////////////////////////////////////
///  CSimpleGraphicDataSource

CSimpleGraphicDataSource::CSimpleGraphicDataSource(
    const objects::CSeq_align& align, objects::CScope& scope, TNumrow anchor)
    : m_AnchorRow(anchor)
    , m_IsRegular(true)
{
    m_Alignment.Reset(&align);
    m_Scope.Reset(&scope);

    // must be a pairwise alignment
    _ASSERT(align.CheckNumRows() == 2);
}


CSimpleGraphicDataSource::TNumrow
CSimpleGraphicDataSource::GetNumRows(void) const
{
    return 2;
}


TSeqPos CSimpleGraphicDataSource::GetAlnStart(void) const
{
    return m_Alignment->GetSeqStart(m_AnchorRow);
}


TSeqPos CSimpleGraphicDataSource::GetAlnStop(void) const
{
    return m_Alignment->GetSeqStop(m_AnchorRow);
}


TSeqPos CSimpleGraphicDataSource::GetSeqStart(TNumrow row) const
{
    return m_Alignment->GetSeqStart(row);
}


TSeqPos CSimpleGraphicDataSource::GetSeqStop(TNumrow row) const
{
    return m_Alignment->GetSeqStop(row);
}


IAlnExplorer::TSignedRange CSimpleGraphicDataSource::GetAlnRange(void) const
{
    return TSignedRange((TSignedSeqPos)GetAlnStart(), (TSignedSeqPos)GetAlnStop());
}


IAlnExplorer::TSignedRange CSimpleGraphicDataSource::GetSeqAlnRange(TNumrow row) const
{
    // use the same alignment range
    return GetAlnRange();
}


TSeqPos CSimpleGraphicDataSource::GetSeqLength(TNumrow row) const
{
    return GetBioseqHandle(row).GetBioseqLength();
}


TSeqPos CSimpleGraphicDataSource::GetBaseWidth(TNumrow row) const
{
    // will be 1 always since the native coordinate is used
    // for both row regardless of the molecule type
    return 1;
}


CSimpleGraphicDataSource::TNumrow
CSimpleGraphicDataSource::GetAnchor(void) const
{
    return m_AnchorRow;
}


const CSeq_id& CSimpleGraphicDataSource::GetSeqId(TNumrow row) const
{
    return m_Alignment->GetSeq_id(row);
}


const objects::CBioseq_Handle&
CSimpleGraphicDataSource::GetBioseqHandle(TNumrow row) const
{
    if (m_BioseqHandles.count(row) == 0) {
        m_BioseqHandles[row] = m_Scope->GetBioseqHandle(m_Alignment->GetSeq_id(row));
    }
    return m_BioseqHandles[row];
}


bool CSimpleGraphicDataSource::IsPositiveStrand(TNumrow row) const
{
    return m_Alignment->GetSeqStrand(row) != eNa_strand_minus;
}


bool CSimpleGraphicDataSource::IsNegativeStrand(TNumrow row) const
{
    return m_Alignment->GetSeqStrand(row) == eNa_strand_minus;
}


TSignedSeqPos 
CSimpleGraphicDataSource::GetSeqPosFromAlnPos(TNumrow row, TSeqPos aln_pos,
                                              IAlnExplorer::ESearchDirection dir,
                                              bool try_reverse_dir) const
{
    TSignedSeqPos seq_pos = (TSignedSeqPos)aln_pos;
    if (row == m_AnchorRow) {
        return seq_pos;
    }

    // this call is not expected.
    _ASSERT(false);

    return seq_pos;
}



TSignedSeqPos
CSimpleGraphicDataSource::GetAlnPosFromSeqPos(TNumrow row,
                                              TSeqPos seq_pos,
                                              IAlnExplorer::ESearchDirection dir,
                                              bool try_reverse_dir) const
{
    TSignedSeqPos aln_pos = (TSignedSeqPos)seq_pos;
    if (row == m_AnchorRow) {
        return aln_pos;
    }

    // this call is not expected.
    _ASSERT(false);

    return aln_pos;
}


IAlnSegmentIterator*
CSimpleGraphicDataSource::CreateSegmentIterator(TNumrow row,
                                                const IAlnExplorer::TSignedRange& range,
                                                IAlnSegmentIterator::EFlags flag) const
{
    // this call is not expected.
    //_ASSERT(false);
    return new CSimpleSeg_CI();
}


string& CSimpleGraphicDataSource::GetAlnSeqString(TNumrow row, 
                                                  string &buffer,
                                                  const IAlnExplorer::TSignedRange& aln_range) const
{
    // this call is not expected.
    _ASSERT(false);
    return buffer;
}


string& CSimpleGraphicDataSource::GetSeqString(string &buffer, TNumrow row,
                                               const IAlnExplorer::TSignedRange& seq_rng,
                                               const IAlnExplorer::TSignedRange& aln_rng,
                                               bool anchor_direct) const
{
    // this call is not expected.
    _ASSERT(false);

    IAlnGraphicDataSource::GetSeqString(buffer, row, seq_rng, aln_rng, anchor_direct);
    return buffer;
}


TSignedSeqPos
CSimpleGraphicDataSource::GetSeqPosFromSeqPos(TNumrow for_row,
                                              TNumrow row, TSeqPos seq_pos) const
{
    if (row == m_AnchorRow) {
        return GetSeqPosFromAlnPos(for_row, seq_pos);
    }

    // this call is not expected.
    _ASSERT(false);
    return GetAlnPosFromSeqPos(row, seq_pos);
}


CSimpleGraphicDataSource::EPolyATail
CSimpleGraphicDataSource::HasPolyATail() const
{
    return ePolyA_No;
}

bool CSimpleGraphicDataSource::IsRegular() const
{
    return m_IsRegular;
}

END_NCBI_SCOPE
