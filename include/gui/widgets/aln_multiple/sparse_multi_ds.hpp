#ifndef GUI_WIDGETS___SPARSE_MULTI_DS__HPP
#define GUI_WIDGETS___SPARSE_MULTI_DS__HPP

/*  $Id: sparse_multi_ds.hpp 43632 2019-08-09 20:17:37Z katargir $
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

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>
#include <gui/utils/event_handler.hpp>
#include <gui/objutils/sparse_alignment.hpp>
#include <gui/widgets/aln_multiple/alnmulti_ds.hpp>
#include <gui/widgets/aln_multiple/build_sparse_aln_job.hpp>
#include <objtools/alnmgr/sparse_aln.hpp>
#include <objtools/alnmgr/aln_explorer.hpp>
#include <objtools/alnmgr/aln_user_options.hpp>

BEGIN_NCBI_SCOPE

class CSparseRowHandle;
class CAppJobNotification;

///////////////////////////////////////////////////////////////////////////////
///
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CSparseMultiDataSource
    : public IAlnMultiDataSource
    , public CEventHandler
{
public:
    typedef CSparseAlignment::TNumrow   TNumrow;
    typedef IAlnExplorer::TResidue   TResidue;

public:
    CSparseMultiDataSource(objects::CScope& scope);
    virtual ~CSparseMultiDataSource();

    virtual void    Init(CSparseAln& align);
    virtual void    Init(vector<CRef<CAnchoredAln> >& aligns,
        const CAlnUserOptions& options, bool sync = false);

    /// @name IAlnMultiDataSource implementation
    /// @{
    virtual bool    IsEmpty() const;

    virtual void  SetGapChar(TResidue gap_char);

    virtual IAlnExplorer::EAlignType  GetAlignType() const;

    /// number of rows in alignment
    virtual TNumrow  GetNumRows(void) const;

    virtual TSeqPos  GetAlnStart(void) const;
    virtual TSeqPos  GetAlnStop(void) const;

    virtual TSeqPos GetSeqStart(TNumrow row) const;
    virtual TSeqPos GetSeqStop(TNumrow row) const;

    /// returns index of the Consensus row or -1 if it doesn't exist
    virtual int     GetConsensusRow() const;

    virtual bool    CanCreateConsensus();
    virtual void    CreateConsensus();
    virtual void    SetCreateConsensus(bool /*b*/) {/*no can do*/ }

    virtual const IAlignRowHandle*  GetRowHandle(TNumrow row) const;
    virtual TSeqPos GetBaseWidth(TNumrow row) const;

    /// Anchoring methods
    virtual bool  IsSetAnchor(void) const;
    virtual TNumrow  GetAnchor(void) const;
    virtual bool  CanChangeAnchor(void) const;
    virtual bool  SetAnchor(TNumrow anchor);
    virtual bool  UnsetAnchor(void);

    virtual bool    CanGetId(TNumrow row) const;
    virtual const objects::CSeq_id&  GetSeqId(TNumrow row) const;

    virtual bool    IsPositiveStrand(TNumrow row) const;
    virtual bool    IsNegativeStrand(TNumrow row) const;

    virtual TSignedSeqPos  GetSeqPosFromAlnPos(TNumrow for_row, TSeqPos aln_pos,
                                IAlnExplorer::ESearchDirection dir = IAlnExplorer::eNone,
                                bool try_reverse_dir = true) const;

    virtual TSignedSeqPos  GetAlnPosFromSeqPos(TNumrow row, TSeqPos seq_pos,
                                IAlnExplorer::ESearchDirection dir = IAlnExplorer::eNone,
                                bool try_reverse_dir = true) const;
    virtual IAlnSegmentIterator*
        CreateSegmentIterator(TNumrow row, const IAlnExplorer::TSignedRange& range,
                              IAlnSegmentIterator::EFlags flag) const;

    virtual bool     CanGetSeqString(TNumrow row) const;
    virtual string&  GetSeqString (TNumrow row, string &buffer, const IAlnExplorer::TRange &seq_rng) const;
    virtual string&  GetAlnSeqString(TNumrow row, string &buffer, const IAlnExplorer::TSignedRange &aln_rng) const;


    virtual bool    CanGetBioseqHandle(TNumrow row) const;
    virtual const objects::CBioseq_Handle&  GetBioseqHandle(TNumrow row) const;

    virtual void  GetAlnFromSeq(TNumrow row, const TRangeColl& seq_coll,
                                TRangeColl& aln_coll) const;
    virtual void  GetSeqFromAln(TNumrow row, const TRangeColl& aln_coll,
                                TRangeColl& seq_coll) const;
    /// @}
    
    void    OnAppJobNotification(CEvent* evt);

protected:
    virtual void x_ClearHandles();
    virtual void x_CreateHandles();

    virtual bool    x_IsJobRunning();
    virtual void    x_DeleteJob();
    virtual void    x_OnJobProgress(CAppJobNotification& notn);

    DECLARE_EVENT_MAP();

protected:
    CRef<objects::CScope>   m_Scope;
    CRef<CSparseAln>  m_Alignment;
    //CRef<CSparseAlignment>  m_Alignment; // the alignment

    typedef vector<CSparseRowHandle*>    THandleVector;
    THandleVector    m_Handles;

    CRef<CBuildSparseAlnJob>   m_Job;
    int m_JobID;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS___SPARSE_MULTI_DS__HPP
