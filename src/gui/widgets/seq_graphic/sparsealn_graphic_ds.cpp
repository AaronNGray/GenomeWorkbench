/*  $Id: sparsealn_graphic_ds.cpp 37049 2016-11-30 19:40:50Z shkeda $
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

#include <gui/widgets/seq_graphic/sparsealn_graphic_ds.hpp>
#include <gui/utils/event_translator.hpp>

#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seq/Bioseq.hpp>

#include <objmgr/align_ci.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

///////////////////////////////////////////////////////////////////////////////
///  CSparseAlnGraphicDataSource

CSparseAlnGraphicDataSource::CSparseAlnGraphicDataSource(CConstRef<CSparseAln> sparseAlign)
: m_Alignment(sparseAlign)
{
}


CSparseAlnGraphicDataSource::~CSparseAlnGraphicDataSource()
{
}


CSparseAlnGraphicDataSource::TNumrow  CSparseAlnGraphicDataSource::GetNumRows(void) const
{
    return m_Alignment->GetNumRows();
}


TSeqPos  CSparseAlnGraphicDataSource::GetAlnStart(void) const
{
    return m_Alignment->GetAlnRange().GetFrom();
}


TSeqPos  CSparseAlnGraphicDataSource::GetAlnStop(void) const
{
    return m_Alignment->GetAlnRange().GetTo();
}


TSeqPos  CSparseAlnGraphicDataSource::GetSeqStart(TNumrow row) const
{
    return m_Alignment->GetSeqStart(row);
}


TSeqPos  CSparseAlnGraphicDataSource::GetSeqStop(TNumrow row) const
{
    return m_Alignment->GetSeqStop(row);
}


IAlnExplorer::TSignedRange
CSparseAlnGraphicDataSource::GetAlnRange(void) const
{
    return m_Alignment->GetAlnRange();
}


IAlnExplorer::TSignedRange
CSparseAlnGraphicDataSource::GetSeqAlnRange(TNumrow row) const
{
    return m_Alignment->GetSeqAlnRange(row);
}


TSeqPos CSparseAlnGraphicDataSource::GetSeqLength(TNumrow row) const
{
    return m_Alignment->GetBioseqHandle(row).GetBioseqLength();
}


TSeqPos CSparseAlnGraphicDataSource::GetBaseWidth(TNumrow row) const
{
    return (TSeqPos)m_Alignment->GetBaseWidth(row);
}


CSparseAlnGraphicDataSource::TNumrow  CSparseAlnGraphicDataSource::GetAnchor(void) const
{
    return m_Alignment->GetAnchor();
}


const CSeq_id&  CSparseAlnGraphicDataSource::GetSeqId(TNumrow row) const
{
    return m_Alignment->GetSeqId(row);
}


bool CSparseAlnGraphicDataSource::IsPositiveStrand(TNumrow row) const
{
    return m_Alignment->IsPositiveStrand(row);
}


bool CSparseAlnGraphicDataSource::IsNegativeStrand(TNumrow row) const
{
    return m_Alignment->IsNegativeStrand(row);
}


TSignedSeqPos CSparseAlnGraphicDataSource::GetSeqPosFromAlnPos(TNumrow row, TSeqPos seq_pos,
                                                               IAlnExplorer::ESearchDirection dir,
                                                               bool try_reverse_dir) const
{
    return m_Alignment->GetSeqPosFromAlnPos(row, seq_pos, dir, try_reverse_dir);
}


TSignedSeqPos  CSparseAlnGraphicDataSource::GetAlnPosFromSeqPos(TNumrow row,
                                                                TSeqPos seq_pos,
                                                                IAlnExplorer::ESearchDirection dir,
                                                                bool try_reverse_dir) const
{
    return m_Alignment->GetAlnPosFromSeqPos(row, seq_pos, dir, try_reverse_dir);
}


IAlnSegmentIterator*
CSparseAlnGraphicDataSource::CreateSegmentIterator(TNumrow row,
                                                   const TSignedRange& range,
                                                   IAlnSegmentIterator::EFlags flag) const
{
    return m_Alignment->CreateSegmentIterator(row, range, flag);
}


string& CSparseAlnGraphicDataSource::GetAlnSeqString(TNumrow row, 
                                                     string &buffer,
                                                     const TSignedRange& aln_range) const
{
    return m_Alignment->GetAlnSeqString(row, buffer, aln_range);
}


const objects::CBioseq_Handle& CSparseAlnGraphicDataSource::GetBioseqHandle(TNumrow row) const
{
    return m_Alignment->GetBioseqHandle(row);
}


TSignedSeqPos CSparseAlnGraphicDataSource::GetSeqPosFromSeqPos(TNumrow for_row,
                                                               TNumrow row, TSeqPos seq_pos) const
{
    TSignedSeqPos aln_pos = m_Alignment->GetAlnPosFromSeqPos(row, seq_pos);
    return GetSeqPosFromAlnPos(for_row, aln_pos);
}


END_NCBI_SCOPE
