#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SPARSEALN_GRAPHIC_DS__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SPARSEALN_GRAPHIC_DS__HPP

/*  $Id: sparsealn_graphic_ds.hpp 43674 2019-08-14 13:57:05Z katargir $
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
#include <objtools/alnmgr/sparse_aln.hpp>
#include <gui/widgets/seq_graphic/alngraphic_ds.hpp>

BEGIN_NCBI_SCOPE

class CSparseRowHandle;


///////////////////////////////////////////////////////////////////////////////
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSparseAlnGraphicDataSource
    : public IAlnGraphicDataSource
    , public CEventHandler
{
public:
    typedef IAlnExplorer::TNumrow TNumrow;
    typedef IAlnExplorer::TResidue   TResidue;

public:
    CSparseAlnGraphicDataSource(CConstRef<CSparseAln> sparseAlign);
    virtual ~CSparseAlnGraphicDataSource();

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

    virtual TSignedSeqPos GetSeqPosFromSeqPos(TNumrow for_row, TNumrow row, TSeqPos seq_pos) const;

    virtual EPolyATail HasPolyATail() const
    {
        return ePolyA_Unknown;
    }

    virtual bool HasCigar() const
    {
        return false;
    }

    virtual const TCigar* GetCigar() const
    {
        return NULL;
    }

    /// @}

protected:
    CConstRef<CSparseAln>  m_Alignment;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___SPARSEALN_GRAPHIC_DS__HPP
