/*  $Id: sparse_row_handle.cpp 44485 2019-12-27 18:49:45Z shkeda $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Andrey Yazhuk
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbistd.hpp>

#include <objtools/alnmgr/sparse_aln.hpp>

#include <gui/widgets/aln_multiple/sparse_row_handle.hpp>
#include <gui/widgets/aln_multiple/sparse_multi_ds.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CSparseRowHandle

CSparseRowHandle::CSparseRowHandle(CSparseAln& aln, TNumrow row)
:   m_Alignment(aln),
    m_Row(row)
{
}


CSparseRowHandle::~CSparseRowHandle()
{
}


CSparseRowHandle::TNumrow CSparseRowHandle::GetRowNum() const
{
    return m_Row;
}


TSignedSeqPos  CSparseRowHandle::GetSeqAlnStart() const
{
    return m_Alignment.GetSeqAlnStart(m_Row);
}


TSignedSeqPos  CSparseRowHandle::GetSeqAlnStop() const
{
    return m_Alignment.GetSeqAlnStop(m_Row);
}


TSignedSeqPos  CSparseRowHandle::GetSeqStart() const
{
    return m_Alignment.GetSeqStart(m_Row);
}


TSignedSeqPos  CSparseRowHandle::GetSeqStop() const
{
    return m_Alignment.GetSeqStop(m_Row);
}


bool CSparseRowHandle::IsNegativeStrand() const
{
    return m_Alignment.IsNegativeStrand(m_Row);
}


IAlnExplorer::TSignedRange  CSparseRowHandle::GetSeqAlnRange() const
{
    return m_Alignment.GetSeqAlnRange(m_Row);
}


TSignedSeqPos  CSparseRowHandle::GetAlnPosFromSeqPos(TSeqPos seq_pos,
                        IAlnExplorer::ESearchDirection dir,
                        bool try_reverse_dir) const
{
    return m_Alignment.GetAlnPosFromSeqPos(m_Row, seq_pos, dir, try_reverse_dir);
}


TSignedSeqPos  CSparseRowHandle::GetSeqPosFromAlnPos (TSeqPos aln_pos,
                    IAlnExplorer::ESearchDirection dir,
                    bool try_reverse_dir) const
{
    return m_Alignment.GetSeqPosFromAlnPos(m_Row, aln_pos, dir, try_reverse_dir);
}


IAlnSegmentIterator*
    CSparseRowHandle::CreateSegmentIterator(const IAlnExplorer::TSignedRange& range,
                                     IAlnSegmentIterator::EFlags flags) const
{
    return m_Alignment.CreateSegmentIterator(m_Row, range, flags);
}


bool CSparseRowHandle::CanGetSeqString() const
{
    return true;
}


string& CSparseRowHandle::GetSeqString (string &buffer,
                                        const IAlnExplorer::TRange &seq_rng) const
{
    return m_Alignment.GetSeqString(m_Row, buffer, seq_rng);
}


string& CSparseRowHandle::GetAlnSeqString(string &buffer,
                                            const IAlnExplorer::TSignedRange &aln_rng) const
{
    return m_Alignment.GetAlnSeqString(m_Row, buffer, aln_rng);
}


bool CSparseRowHandle::CanGetBioseqHandle() const
{
    return true;
}


const objects::CBioseq_Handle&  CSparseRowHandle::GetBioseqHandle() const
{
    return m_Alignment.GetBioseqHandle(m_Row);
}

const objects::CSeq_id&  CSparseRowHandle::GetSeq_id() const
{
    return m_Alignment.GetSeqId(m_Row);
}


objects::CScope& CSparseRowHandle::GetScope() const 
{
    return *m_Alignment.GetScope();
}

bool CSparseRowHandle::UsesAATranslation() const
{
	return m_Alignment.GetBioseqHandle(m_Row).IsAa();
}

bool CSparseRowHandle::IsNucProtAlignment() const
{
    TNumrow num_rows = m_Alignment.GetNumRows();
    if (num_rows <= 1)
        return false;
    int base_width = m_Alignment.GetBaseWidth(0);
    for (TNumrow row = 1; row < num_rows; ++row) {
        if (base_width != m_Alignment.GetBaseWidth(row))
            return true;
    }
    return false;
}

size_t CSparseRowHandle::GetBaseWidth() const 
{
    return m_Alignment.GetBaseWidth(m_Row);
}



END_NCBI_SCOPE
