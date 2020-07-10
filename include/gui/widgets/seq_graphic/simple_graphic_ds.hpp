#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SIMPLE_GRAPHIC_DS__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SIMPLE_GRAPHIC_DS__HPP

/*  $Id: simple_graphic_ds.hpp 37049 2016-11-30 19:40:50Z shkeda $
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

#include <gui/utils/event_handler.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <gui/widgets/seq_graphic/alngraphic_ds.hpp>

BEGIN_NCBI_SCOPE

class CSparseRowHandle;


///////////////////////////////////////////////////////////////////////////////
/// This class is intended to be used for optimized version of pairwise
/// seq-align to replace CSparseAlnGraphicDataSource to improve performance.
/// There are two scenario that a pairwise seq-align can be simplifed:
///   1. when the alignemnt size (screen pixels) is less than 4, or
///   2. when the aignment is a std-seg and the two aligned segment lengths
///      are not in 1-to-1 or 1-to-3 ratio

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSimpleGraphicDataSource
    : public IAlnGraphicDataSource
    , public CEventHandler
{
public:
    typedef IAlnExplorer::TNumrow TNumrow;
    typedef IAlnExplorer::TResidue   TResidue;

public:
    CSimpleGraphicDataSource(const objects::CSeq_align& align,
        objects::CScope& scope,
        TNumrow anchor);
    
    /// @name IAlnGraphicDataSource implementation
    /// @{
    /// number of rows in alignment
    virtual TNumrow  GetNumRows(void) const;
    virtual TSeqPos  GetAlnStart(void) const;
    virtual TSeqPos  GetAlnStop(void) const;

    virtual TSeqPos GetSeqStart(TNumrow row) const;
    virtual TSeqPos GetSeqStop(TNumrow row) const;

    virtual TSignedRange GetAlnRange(void) const;
    virtual TSignedRange  GetSeqAlnRange(TNumrow row) const;

    virtual TSeqPos GetSeqLength(TNumrow row) const;
    virtual TSeqPos GetBaseWidth(TNumrow row) const;

    virtual TNumrow  GetAnchor(void) const;
    virtual const objects::CSeq_id&  GetSeqId(TNumrow row) const;

    virtual const objects::CBioseq_Handle& GetBioseqHandle(TNumrow row) const;
    virtual bool    IsPositiveStrand(TNumrow row) const;
    virtual bool    IsNegativeStrand(TNumrow row) const;

    virtual TSignedSeqPos  GetSeqPosFromAlnPos(TNumrow for_row,
        TSeqPos aln_pos,
        IAlnExplorer::ESearchDirection dir = IAlnExplorer::eNone,
        bool try_reverse_dir = true) const;

    virtual TSignedSeqPos  GetAlnPosFromSeqPos(TNumrow row,
        TSeqPos seq_pos,
        IAlnExplorer::ESearchDirection dir = IAlnExplorer::eNone,
        bool try_reverse_dir = true) const;

    virtual IAlnSegmentIterator*
        CreateSegmentIterator(TNumrow row, const TSignedRange& range,
                              IAlnSegmentIterator::EFlags flag) const;

    virtual string&  GetAlnSeqString(TNumrow row, string &buffer, 
        const TSignedRange &aln_rng) const;

    virtual string&  GetSeqString(string& buffer, TNumrow row,
        const TSignedRange& seq_rng,
        const TSignedRange& aln_rng,
        bool anchor_direct) const;

    virtual TSignedSeqPos GetSeqPosFromSeqPos(TNumrow for_row, TNumrow row, TSeqPos seq_pos) const;

    virtual EPolyATail HasPolyATail() const;
    
    virtual bool HasCigar() const
    { return  false; }

    virtual const TCigar* GetCigar() const
    { return NULL; }

    virtual bool IsSimplified() const 
    { return true; }

    virtual bool IsRegular() const;
    /// @}

    void SetRegularity(bool f)
    {
        m_IsRegular = f;
    }
    
protected:
    CConstRef<objects::CSeq_align>  m_Alignment;
    mutable CRef<objects::CScope>   m_Scope;
    mutable map<TNumrow, objects::CBioseq_Handle> m_BioseqHandles;
    TNumrow                         m_AnchorRow;
    bool                            m_IsRegular;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___SIMPLE_GRAPHIC_DS__HPP
