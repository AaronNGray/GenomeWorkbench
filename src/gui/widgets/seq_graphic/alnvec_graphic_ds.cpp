/*  $Id: alnvec_graphic_ds.cpp 37049 2016-11-30 19:40:50Z shkeda $
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
*/

#include <ncbi_pch.hpp>

#include <gui/widgets/seq_graphic/alnvec_graphic_ds.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/utils/event_translator.hpp>

#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seq/Bioseq.hpp>

#include <objmgr/align_ci.hpp>
#include <objtools/alnmgr/alnvec_iterator.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


CAlnVecGraphicDataSource::CAlnVecGraphicDataSource(const objects::CAlnVec& aln_mgr)
: m_AlnVec(&aln_mgr)
{
}


CAlnVecGraphicDataSource::~CAlnVecGraphicDataSource()
{
}


IAlnExplorer::EAlignType CAlnVecGraphicDataSource::GetAlignType() const
{
    if( ! m_AlnVec) {
        return IAlnExplorer::fInvalid;
    }
    if(m_AlnVec->GetDenseg().IsSetWidths())   {
        return IAlnExplorer::fMixed;
    } else {
        CBioseq_Handle bsh = m_AlnVec->GetBioseqHandle(0);
        if (bsh.IsProtein()) {
            return IAlnExplorer::fProtein;
        } else {
            return IAlnExplorer::fDNA;
        }
    }
}


CAlnVecGraphicDataSource::TNumrow CAlnVecGraphicDataSource::GetNumRows (void) const
{
    return (TNumrow) m_AlnVec->GetNumRows();
}


TSeqPos CAlnVecGraphicDataSource::GetAlnStart (void) const
{
    return m_AlnVec->GetAlnStart();
}


TSeqPos CAlnVecGraphicDataSource::GetAlnStop (void) const
{
    return m_AlnVec->GetAlnStop();
}


TSeqPos CAlnVecGraphicDataSource::GetSeqStart(TNumrow row) const
{
    return m_AlnVec->GetSeqStart(row);
}


TSeqPos CAlnVecGraphicDataSource::GetSeqStop(TNumrow row) const
{
    return m_AlnVec->GetSeqStop(row);
}


IAlnExplorer::TSignedRange
CAlnVecGraphicDataSource::GetAlnRange(void) const
{
    return TSignedRange(m_AlnVec->GetAlnStart(), m_AlnVec->GetAlnStop());
}


IAlnExplorer::TSignedRange
CAlnVecGraphicDataSource::GetSeqAlnRange(TNumrow row) const
{
    return m_AlnVec->GetSeqAlnRange(row);
}


TSeqPos CAlnVecGraphicDataSource::GetSeqLength(TNumrow row) const
{
    return m_AlnVec->GetBioseqHandle(row).GetBioseqLength();
}


TSeqPos CAlnVecGraphicDataSource::GetBaseWidth(TNumrow row) const
{
    // assumming alignment coordinates are in native sequence coordinates.
    return (TSeqPos)1;
}


CAlnVecGraphicDataSource::TNumrow CAlnVecGraphicDataSource::GetAnchor(void) const
{
    return m_AlnVec->GetAnchor();
}



const CSeq_id& CAlnVecGraphicDataSource::GetSeqId(TNumrow row) const
{
    return m_AlnVec->GetSeqId(row);
}


const objects::CBioseq_Handle& CAlnVecGraphicDataSource::GetBioseqHandle(TNumrow row) const
{
    return m_AlnVec->GetBioseqHandle(row);
}

bool CAlnVecGraphicDataSource::IsPositiveStrand(TNumrow row) const
{
    return m_AlnVec->IsPositiveStrand(row);
}


bool CAlnVecGraphicDataSource::IsNegativeStrand(TNumrow row) const
{
    return m_AlnVec->IsNegativeStrand(row);
}


TSignedSeqPos CAlnVecGraphicDataSource::GetSeqPosFromAlnPos(TNumrow row, TSeqPos seq_pos,
                                                             IAlnExplorer::ESearchDirection dir,
                                                             bool try_reverse_dir) const
{
    return m_AlnVec->GetSeqPosFromAlnPos(row, seq_pos, (CAlnVec::ESearchDirection) dir, try_reverse_dir);
}


TSignedSeqPos CAlnVecGraphicDataSource::GetAlnPosFromSeqPos(TNumrow row,
                                                             TSeqPos seq_pos,
                                                             IAlnExplorer::ESearchDirection dir,
                                                             bool try_reverse_dir) const
{
    return m_AlnVec->GetAlnPosFromSeqPos(row, seq_pos, dir, try_reverse_dir);
}


string& CAlnVecGraphicDataSource::GetAlnSeqString(TNumrow row, string &buffer,
                                                  const TSignedRange& aln_range) const
{
    return m_AlnVec->GetAlnSeqString(buffer, row, aln_range);
}


IAlnSegmentIterator*
CAlnVecGraphicDataSource::CreateSegmentIterator(TNumrow row,
                                                const TSignedRange& range,
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

   bool reversed = m_AlnVec->IsNegativeStrand(row);
   CRef<CAlnMap::CAlnChunkVec> aln_chunk = m_AlnVec->GetAlnChunks(row, range, av_flags);
   return new CAlnVecIterator(*aln_chunk, reversed);
}



TSignedSeqPos CAlnVecGraphicDataSource::GetSeqPosFromSeqPos(TNumrow for_row,
                                                            TNumrow row, TSeqPos seq_pos) const
{
    return m_AlnVec->GetSeqPosFromSeqPos(for_row, row, seq_pos);
}


END_NCBI_SCOPE
