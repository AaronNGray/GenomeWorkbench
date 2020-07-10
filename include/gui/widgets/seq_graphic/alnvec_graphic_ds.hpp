#ifndef GUI_WIDGETS_SEQ_GRAPHIC___ALNVEC_GRAPHIC_DS__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___ALNVEC_GRAPHIC_DS__HPP

/*  $Id: alnvec_graphic_ds.hpp 37049 2016-11-30 19:40:50Z shkeda $
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

#include <objtools/alnmgr/alnvec.hpp>
#include <objects/seqalign/Seq_align.hpp>

#include <gui/utils/event_handler.hpp>
#include <gui/widgets/seq_graphic/alngraphic_ds.hpp>


BEGIN_NCBI_SCOPE

class CAppJobNotification;

////////////////////////////////////////////////////////////////////////////////
/// CAlnVecGraphicDataSource - implementation of CAlnGraphicDataSource for
/// CAlnVec-based alignments.
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CAlnVecGraphicDataSource
    : public IAlnGraphicDataSource
    , public CEventHandler
{
public:
    CAlnVecGraphicDataSource(const objects::CAlnVec& aln_mgr);
    ~CAlnVecGraphicDataSource();
    /// @name CAlnGraphicDataSource implementation
    /// @{
    virtual IAlnExplorer::EAlignType  GetAlignType() const;
    virtual TNumrow  GetNumRows(void) const;
    virtual TSeqPos  GetAlnStart(void) const;
    virtual TSeqPos  GetAlnStop(void) const;
    virtual TSeqPos GetSeqStart(TNumrow row) const;
    virtual TSeqPos GetSeqStop(TNumrow row) const;

    virtual TSignedRange GetAlnRange(void) const;
    virtual TSignedRange  GetSeqAlnRange(TNumrow row) const;

    virtual TSeqPos GetSeqLength(TNumrow row) const;
    virtual TSeqPos GetBaseWidth(TNumrow row) const;

    virtual TNumrow  GetAnchor (void) const;
    virtual const objects::CSeq_id&  GetSeqId(TNumrow row) const;
    virtual const objects::CBioseq_Handle& GetBioseqHandle(TNumrow row) const;

    virtual bool    IsPositiveStrand(TNumrow row) const;
    virtual bool    IsNegativeStrand(TNumrow row) const;

    virtual TSignedSeqPos GetSeqPosFromAlnPos(TNumrow for_row, TSeqPos aln_pos,
        TSearchDirection dir = IAlnExplorer::eNone,
        bool try_reverse_dir = true) const;

    virtual TSignedSeqPos  GetAlnPosFromSeqPos(TNumrow row,
        TSeqPos seq_pos,
        IAlnExplorer::ESearchDirection dir = IAlnExplorer::eNone,
        bool try_reverse_dir = true) const;

    virtual string& GetAlnSeqString(TNumrow row, string &buffer,
        const TSignedRange& aln_range) const;

    virtual IAlnSegmentIterator*    CreateSegmentIterator(TNumrow row,
        const TSignedRange& range,
        IAlnSegmentIterator::EFlags flags) const;

    virtual TSignedSeqPos GetSeqPosFromSeqPos(TNumrow for_row,
        TNumrow row,
        TSeqPos seq_pos) const;
    
    virtual EPolyATail HasPolyATail() const
    {
        return ePolyA_Unknown;
    };

    virtual bool HasCigar() const
    {
        return false;
    }

    virtual const TCigar* GetCigar() const
    {
        return NULL;
    }
    /// @}

private:
    // prohibit copying!
    CAlnVecGraphicDataSource(const CAlnVecGraphicDataSource& ds);
    CAlnVecGraphicDataSource& operator=(const CAlnVecGraphicDataSource& ds);

protected:
    CConstRef<objects::CAlnVec>     m_AlnVec;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___ALNVEC_GRAPHIC_DS__HPP
