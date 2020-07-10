#ifndef GUI_WIDGETS_ALN_MULTIPLE___SPARSE_ROW_HANDLE__HPP
#define GUI_WIDGETS_ALN_MULTIPLE___SPARSE_ROW_HANDLE__HPP

/*  $Id: sparse_row_handle.hpp 44485 2019-12-27 18:49:45Z shkeda $
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

#include <gui/widgets/aln_multiple/alnmulti_ds.hpp>


BEGIN_NCBI_SCOPE

class CSparseAln;

///////////////////////////////////////////////////////////////////////////////
/// CSparseRowHandle
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CSparseRowHandle : public IAlignRowHandle
{
public:
    CSparseRowHandle(CSparseAln& aln, TNumrow row);
    virtual ~CSparseRowHandle();

    virtual TNumrow GetRowNum() const;

    virtual TSignedSeqPos   GetSeqAlnStart() const;
    virtual TSignedSeqPos   GetSeqAlnStop() const;
    virtual IAlnExplorer::TSignedRange    GetSeqAlnRange() const;
    virtual TSignedSeqPos   GetSeqStart() const;
    virtual TSignedSeqPos   GetSeqStop() const;
    virtual bool            IsNegativeStrand() const;

    virtual TSignedSeqPos  GetAlnPosFromSeqPos(TSeqPos seq_pos,
                                IAlnExplorer::ESearchDirection dir = IAlnExplorer::eNone,
                                bool try_reverse_dir = true) const;
    virtual TSignedSeqPos  GetSeqPosFromAlnPos(TSeqPos aln_pos,
                                IAlnExplorer::ESearchDirection dir = IAlnExplorer::eNone,
                                bool try_reverse_dir = true) const;

    virtual IAlnSegmentIterator*
        CreateSegmentIterator(const IAlnExplorer::TSignedRange& range,
                              IAlnSegmentIterator::EFlags flag) const;

    virtual bool     CanGetSeqString() const;
    virtual string&  GetSeqString (string &buffer, const IAlnExplorer::TRange &seq_rng) const;
    virtual string&  GetAlnSeqString(string &buffer, const IAlnExplorer::TSignedRange &aln_rng) const;

    virtual bool    CanGetBioseqHandle() const;
    virtual const objects::CBioseq_Handle&  GetBioseqHandle() const;
    virtual const objects::CSeq_id& GetSeq_id() const;
    virtual objects::CScope& GetScope() const;

	bool UsesAATranslation() const;
    virtual bool IsNucProtAlignment() const;
    virtual size_t GetBaseWidth() const;
protected:
    CSparseAln& m_Alignment;
    TNumrow m_Row;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_MULTIPLE___SPARSE_ROW_HANDLE__HPP
