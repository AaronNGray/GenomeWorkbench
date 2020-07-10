/*  $Id: alnvec_row_handle.cpp 44485 2019-12-27 18:49:45Z shkeda $
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

#include <gui/widgets/aln_multiple/alnvec_row_handle.hpp>
#include <gui/objutils/utils.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CAlnVecRowHandle

CAlnVecRowHandle::TNumrow CAlnVecRowHandle::GetRowNum() const
{
    return m_Row;
}


TSignedSeqPos  CAlnVecRowHandle::GetSeqAlnStart() const
{
    return m_AlnVec->GetSeqAlnStart(m_Row);
}


TSignedSeqPos  CAlnVecRowHandle::GetSeqAlnStop() const
{
    return m_AlnVec->GetSeqAlnStop(m_Row);
}


TSignedSeqPos  CAlnVecRowHandle::GetSeqStart() const
{
    return m_AlnVec->GetSeqStart(m_Row);
}


TSignedSeqPos  CAlnVecRowHandle::GetSeqStop() const
{
    return m_AlnVec->GetSeqStop(m_Row);
}


bool CAlnVecRowHandle::IsNegativeStrand() const
{
    return m_AlnVec->IsNegativeStrand(m_Row);
}


IAlnExplorer::TSignedRange  CAlnVecRowHandle::GetSeqAlnRange() const
{
    return m_AlnVec->GetSeqAlnRange(m_Row);
}


TSignedSeqPos  CAlnVecRowHandle::GetAlnPosFromSeqPos(TSeqPos seq_pos,
                        IAlnExplorer::ESearchDirection dir,
                        bool try_reverse_dir) const
{
    CAlnVec::ESearchDirection av_dir = CAlnVec::eNone;
    switch(dir) {
    case IAlnExplorer::eLeft:
        av_dir = CAlnVec::eLeft;
        break;
    case IAlnExplorer::eRight:
        av_dir = CAlnVec::eRight;
        break;
    default:
        break;
    }
    return m_AlnVec->GetAlnPosFromSeqPos(m_Row, seq_pos, av_dir, try_reverse_dir);
}


TSignedSeqPos  CAlnVecRowHandle::GetSeqPosFromAlnPos (TSeqPos aln_pos,
                    IAlnExplorer::ESearchDirection dir,
                    bool try_reverse_dir) const
{
    CAlnVec::ESearchDirection av_dir = CAlnVec::eNone;
    switch(dir) {
    case IAlnExplorer::eLeft:
        av_dir = CAlnVec::eLeft;
        break;
    case IAlnExplorer::eRight:
        av_dir = CAlnVec::eRight;
        break;
    default:
        break;
    }
    return m_AlnVec->GetSeqPosFromAlnPos(m_Row, aln_pos, av_dir, try_reverse_dir);
}


IAlnSegmentIterator*
    CAlnVecRowHandle::CreateSegmentIterator(const IAlnExplorer::TSignedRange& range,
                                     IAlnSegmentIterator::EFlags flags) const
{
   int av_flags = 0;
   switch(flags)    {
   case IAlnSegmentIterator::eAllSegments:
       av_flags = CAlnVec::fAllChunks;
       break;
   case IAlnSegmentIterator::eSkipGaps:
       av_flags = CAlnVec::fSkipAllGaps;
       break;
   case IAlnSegmentIterator::eInsertsOnly:
       av_flags = CAlnVec::fInsertsOnly;
       break;
   case IAlnSegmentIterator::eSkipInserts:
       av_flags = CAlnVec::fSkipInserts;
       break;
   default:
       _ASSERT(false);
   }
   bool reversed = m_AlnVec->IsNegativeStrand(m_Row);
   return new CAlnVecIterator(*m_AlnVec->GetAlnChunks(m_Row, range, av_flags), reversed);
}


bool CAlnVecRowHandle::CanGetSeqString() const
{
    return true;
}


string& CAlnVecRowHandle::GetSeqString (string &buffer,
                                        const IAlnExplorer::TRange &seq_rng) const
{
    return m_AlnVec->GetSeqString(buffer, m_Row, seq_rng);
}


string& CAlnVecRowHandle::GetAlnSeqString(string &buffer,
                                            const IAlnExplorer::TSignedRange &aln_rng) const
{
    if (!CanGetBioseqHandle())
        return buffer;
    // the widths will be set only if the alignment is mix of na/aa rows
    if (m_AlnVec->GetWidth(m_Row) == 3 && m_GenCode == -1) {
        m_GenCode = CSeqUtils::GetGenCode(GetBioseqHandle());
        m_AlnVec->SetGenCode(m_GenCode, m_Row);
    }

    return m_AlnVec->GetAlnSeqString(buffer, m_Row, aln_rng);
}


bool CAlnVecRowHandle::CanGetBioseqHandle() const
{
    if (m_CanGetBioseq == -1) {
        CBioseq_Handle bsh;
        try {
            bsh = m_AlnVec->GetBioseqHandle(m_Row);
            m_CanGetBioseq = bsh ? 1 : 0;
        } catch (CAlnException& e) {
            if (e.GetErrCode() == CAlnException::eInvalidSeqId)
                m_CanGetBioseq = 0;
        }

    }
    return m_CanGetBioseq == 1;
}


const objects::CBioseq_Handle&  CAlnVecRowHandle::GetBioseqHandle() const
{
    return m_AlnVec->GetBioseqHandle(m_Row);
}

const objects::CSeq_id& CAlnVecRowHandle::GetSeq_id() const
{
    return m_AlnVec->GetSeqId(m_Row);
}

objects::CScope& CAlnVecRowHandle::GetScope() const
{
    return m_AlnVec->GetScope();
}



bool CAlnVecRowHandle::UsesAATranslation() const
{
    if (!CanGetBioseqHandle())
        return false;
    if (m_AlnVec->GetBioseqHandle(m_Row).IsAa())
        return true;

    return (m_AlnVec->GetWidth(m_Row) == 3);
}

bool CAlnVecRowHandle::IsAnchor() const
{
    return m_AlnVec->IsSetAnchor() && m_AlnVec->GetAnchor() == m_Row;
}


bool CAlnVecRowHandle::IsNucProtAlignment() const
{
    TNumrow num_rows = m_AlnVec->GetNumRows();
    if (num_rows <= 1)
        return false;
    int base_width = m_AlnVec->GetWidth(0);
    for (TNumrow row = 1; row < num_rows; ++row) {
        if (base_width != m_AlnVec->GetWidth(row)) 
            return true;
    }
    return false;
}


size_t CAlnVecRowHandle::GetBaseWidth() const 
{
    if (IsNucProtAlignment())
        return 1;
    return m_AlnVec->GetWidth(m_Row);
}

END_NCBI_SCOPE
