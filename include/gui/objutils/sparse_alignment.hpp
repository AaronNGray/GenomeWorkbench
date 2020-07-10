#ifndef GUI_OBJUTILS___SPARSE_ALIGNMENT__HPP
#define GUI_OBJUTILS___SPARSE_ALIGNMENT__HPP

/*  $Id: sparse_alignment.hpp 25706 2012-04-25 00:21:04Z voronov $
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

#include <gui/objutils/data_elements.hpp>

#include <util/align_range.hpp>
#include <util/align_range_coll.hpp>

#include <objmgr/scope.hpp>
#include <objtools/alnmgr/aln_explorer.hpp>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
///  CSparseAlignment - an alignment based on CSparse_seg and
///  CAlingRangeCollection classes rather than on CDense_seg.
/// Assumptions:
///     master is always specified in CSparse-seg and has a Seq-id
///     we display it always anchored
///     chaning anchor is not supported
class NCBI_GUIOBJUTILS_EXPORT CSparseAlignment : public CObject
{
public:
    typedef SAlignTools::TPos  TPos;
    typedef SAlignTools::TAlignRange   TAlignRange;
    typedef SAlignTools::TAlignColl    TAlignColl;

    typedef CRange<TPos>        TARange;

    typedef IAlnExplorer::TNumrow     TNumrow;
    typedef IAlnExplorer::TResidue    TResidue;
    typedef IAlnExplorer::TSignedRange    TSignedRange;

    CSparseAlignment();
    virtual ~CSparseAlignment();

    void    Init(const objects::CSeq_id& master_id,
                 vector<SAlignedSeq*>& aln_seqs,
                 objects::CScope& scope);
    void    Clear();

    void    SetGapChar(TResidue gap_char);

    CRef<objects::CScope>    GetScope();

    TNumrow GetNumRows() const;  /// returns number of rows in the alignment
    TARange GetAlnRange() const;

    const TAlignColl&  GetAlignCollection(TNumrow row);

    bool    IsSetAnchor()   const;
    TNumrow GetAnchor() const;

    const objects::CSeq_id&  GetSeqId(TNumrow row) const;

    TSignedSeqPos   GetSeqAlnStart(TNumrow row) const;
    TSignedSeqPos   GetSeqAlnStop(TNumrow row) const;
    TSignedRange    GetSeqAlnRange (TNumrow row) const;
    TSeqPos     GetSeqStart(TNumrow row) const;
    TSeqPos     GetSeqStop(TNumrow row) const;
    IAlnExplorer::TRange      GetSeqRange(TNumrow row) const;

    bool    IsPositiveStrand(TNumrow row) const;
    bool    IsNegativeStrand(TNumrow row) const;

    TSignedSeqPos   GetAlnPosFromSeqPos(TNumrow row, TSeqPos seq_pos,
                                        IAlnExplorer::ESearchDirection dir = IAlnExplorer::eNone,
                                        bool try_reverse_dir = true) const;
    TSignedSeqPos   GetSeqPosFromAlnPos(TNumrow for_row, TSeqPos aln_pos,
                                        IAlnExplorer::ESearchDirection dir = IAlnExplorer::eNone,
                                        bool try_reverse_dir = true) const;

    string&     GetSeqString(TNumrow row, string &buffer, TSeqPos seq_from, TSeqPos seq_to) const;
    string&     GetSeqString(TNumrow row, string &buffer, const IAlnExplorer::TRange &seq_rng) const;
    string&     GetAlnSeqString(TNumrow row, string &buffer, const TSignedRange &aln_rng) const;

    const objects::CBioseq_Handle&  GetBioseqHandle(TNumrow row) const;

    virtual IAlnSegmentIterator*
        CreateSegmentIterator(TNumrow row, const IAlnExplorer::TSignedRange& range,
                              IAlnSegmentIterator::EFlags flags) const;

protected:
    /// describes a sequence in the alignment

    struct SRowRec{
        SAlignedSeq*    m_AlignedSeq;
        objects::CBioseq_Handle*     m_BioseqHandle;

        SRowRec();
        ~SRowRec();
    };

    typedef vector<SRowRec*> TRows;

protected:
    void    x_AssertRowValid(TNumrow row) const;

protected:
    mutable CRef<objects::CScope>    m_Scope;
    CConstRef<objects::CSeq_id>       m_MasterId;

    mutable TRows   m_Rows;
    TARange     m_AlnRange;     /// the extent of all segments in alignment coords
    TResidue    m_GapChar;
};


END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___SPARSE_ALIGNMENT__HPP
