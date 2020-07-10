#ifndef GUI_WIDGETS_SEQ_GRAPHIC___DENSEG_CI__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___DENSEG_CI__HPP

/*  $Id: denseg_ci.hpp 26189 2012-07-31 15:26:27Z wuliangs $
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
 *
 */

#include <corelib/ncbimisc.hpp>
#include <gui/gui_export.h>
#include <objtools/alnmgr/aln_explorer.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CSeq_align;
END_SCOPE(objects)

/// Implementation of IAlnSegment for CDenseg_aln.
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CDensegSegment : public  IAlnSegment
{
public:
    CDensegSegment(void);
    virtual operator bool(void) const;
    virtual TSegTypeFlags GetType(void) const;
    virtual const TSignedRange& GetAlnRange(void) const;
    virtual const TSignedRange& GetRange(void) const;

    void Init(TSignedSeqPos aln_from,
              TSignedSeqPos aln_to,
              TSignedSeqPos from,
              TSignedSeqPos to,
              TSegTypeFlags type)
    {
        m_AlnRange.Set(aln_from, aln_to);
        m_RowRange.Set(from, to);
        m_Type = type;
    }

private:
    friend class CDenseg_CI;

    TSegTypeFlags m_Type;
    TSignedRange  m_AlnRange;
    TSignedRange  m_RowRange;
};


/// Implementation of IAlnSegmentIterator for CDense_seg.
/// CDenseg_CI is intended to be used for pairwise dense-seg
/// seq-align.  For performance consideration, there is no
/// any validation. It assumes that seq-align is well-constructed,
/// and no mixed strand is allowed.
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CDenseg_CI : public IAlnSegmentIterator
{
public:
    typedef IAlnExplorer::TSignedRange TSignedRange;
    typedef IAlnExplorer::TNumrow      TNumrow;

    /// Create 'empty' iterator.
    CDenseg_CI(void);

    /// Iterate the specified row of the alignment.
    CDenseg_CI(const objects::CSeq_align&   aln,
               TNumrow             row,
               TNumrow             anchor_row,
               EFlags              flags);

    /// Iterate the selected range on the alignment row.
    CDenseg_CI(const objects::CSeq_align&   aln,
               TNumrow             row,
               TNumrow             anchor_row,
               EFlags              flags,
               const TSignedRange& range);

    CDenseg_CI(const CDenseg_CI& orig);

    virtual ~CDenseg_CI(void);

    /// Create a copy of the iterator.
    virtual IAlnSegmentIterator* Clone(void) const;

    /// Return true if iterator points to a valid segment
    virtual operator bool(void) const;

    // Postfix operators are not defined to avoid performance overhead.
    virtual IAlnSegmentIterator& operator++(void);

    virtual bool operator==(const IAlnSegmentIterator& it) const;
    virtual bool operator!=(const IAlnSegmentIterator& it) const;

    virtual const value_type& operator*(void) const;
    virtual const value_type* operator->(void) const;

    /// Check if the anchor row coordinates are on plus strand.
    bool IsAnchorDirect(void) const { return m_AnchorDirect; }

private:
    void x_InitIterator(void);
    void x_InitSegment(void);
    void x_CheckSegment(void);
    void x_NextSegment(void);
    bool x_Equals(const CDenseg_CI& other) const;

    EFlags         m_Flags;        // iterating mode
    CDensegSegment m_Segment;
    CConstRef<objects::CSeq_align> m_Aln;
    TNumrow        m_AnchorRow;
    TNumrow        m_Row;          // Selected row
    TSignedRange   m_TotalRange;   // Total requested (alignment) range
    size_t         m_CurrSeg;      // Current segment index
    size_t         m_TotalSegs;    // Total number of segments
    int            m_AnchorDirect; // Anchor row direction.
    int            m_RowDirect;    // Row direction
    TSignedSeqPos  m_PreToPos_Anchor;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___DENSEG_CI__HPP
