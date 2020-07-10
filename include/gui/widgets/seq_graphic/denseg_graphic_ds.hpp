#ifndef GUI_WIDGETS_SEQ_GRAPHIC___DENSEG_GRAPHIC_DS__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___DENSEG_GRAPHIC_DS__HPP

/*  $Id: denseg_graphic_ds.hpp 44368 2019-12-05 19:37:05Z shkeda $
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
/// This class is intended to be used for pairwised dense-seg seq-align to
/// replace CSparseAlnGraphicDataSource to improve performance.

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CDensegGraphicDataSource
    : public IAlnGraphicDataSource
    , public CEventHandler
{
public:
    typedef IAlnExplorer::TNumrow TNumrow;
    typedef IAlnExplorer::TResidue   TResidue;

public:
    CDensegGraphicDataSource(const objects::CSeq_align& align,
        objects::CScope& scope,
        TNumrow anchor);
    
    virtual ~CDensegGraphicDataSource();

    /// @name IAlnGraphicDataSource implementation
    /// @{
    virtual IAlnExplorer::EAlignType  GetAlignType() const;
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
    {
        return  m_Cigar!= NULL;
    }

    // No ambigous Ms in Cigar string
    // so cigar can be used to get read's sequence
    virtual bool IsCigarUnambiguous() const 
    {
        return m_IsCigarUnambiguous;
    }

    virtual const TCigar* GetCigar() const
    {
        return m_Cigar;
    }

    /// @}
private:
    void x_InitCigar();

    string x_GetCigarStr() const;
    string x_GetMismatchStr() const;
    string& x_GetAlnStringFromCigar(TNumrow row, string &buffer, const TSignedRange& aln_range) const;


protected:
    CConstRef<objects::CSeq_align>  m_Alignment;
    mutable CRef<objects::CScope>   m_Scope;
    mutable map<TNumrow, objects::CBioseq_Handle> m_BioseqHandles;
    TNumrow                         m_AnchorRow;
    TCigar*                         m_Cigar;
    bool                            m_IsCigarUnambiguous = false;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___DENSEG_GRAPHIC_DS__HPP
