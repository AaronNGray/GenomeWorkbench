#ifndef GUI_OBJUTILS___DATA_ELEMENTS__HPP
#define GUI_OBJUTILS___DATA_ELEMENTS__HPP

/*  $Id: data_elements.hpp 25706 2012-04-25 00:21:04Z voronov $
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

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <util/align_range.hpp>
#include <util/align_range_coll.hpp>

#include <objmgr/scope.hpp>
#include <objects/seqalign/Sparse_seg.hpp>
#include <objtools/alnmgr/aln_explorer.hpp>


BEGIN_NCBI_SCOPE

/// TODO merge with CAlignUtils ?
struct  SAlignTools {
    typedef TSignedSeqPos  TPos;
    typedef CAlignRange<TPos>   TAlignRange;
    typedef CAlignRangeCollection<TAlignRange>    TAlignColl;
    typedef IAlnExplorer::TSignedRange    TSignedRange;
};


/// This is a building block for a Builder
/// represents a Sequence aligned to an Anchor (pairwise alignment)
struct SAlignedSeq
{
    typedef SAlignTools::TPos  TPos;
    typedef SAlignTools::TAlignRange    TAlignRange;
    typedef SAlignTools::TAlignColl     TAlignColl;
    typedef SAlignTools::TSignedRange    TSignedRange;

    CConstRef<objects::CSeq_id> m_SeqId; //< sequence id
    TAlignColl*     m_AlignColl;    //< sequnce mapping to the alignment
    TSignedRange    m_SecondRange; /// range of the segments on the sequence
    bool    m_NegativeStrand;

    SAlignedSeq()   :   m_AlignColl(NULL), m_NegativeStrand(false)  {}
    ~SAlignedSeq()
    {
        delete m_AlignColl;
    }
};


END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___DATA_ELEMENTS__HPP
