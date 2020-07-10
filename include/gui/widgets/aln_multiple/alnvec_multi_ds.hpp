#ifndef GUI_WIDGETS_ALN_MULTIPLE___ALNVEC_MULTI_DS__HPP
#define GUI_WIDGETS_ALN_MULTIPLE___ALNVEC_MULTI_DS__HPP

/*  $Id: alnvec_multi_ds.hpp 44958 2020-04-28 18:04:11Z shkeda $
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

#include <gui/utils/event_handler.hpp>

#include <gui/widgets/aln_multiple/alnmulti_ds.hpp>
#include <gui/widgets/aln_multiple/alnvec_row_handle.hpp>
#include <gui/widgets/aln_multiple/build_alnvec_job.hpp>

#include <objects/seqalign/Seq_align.hpp>


BEGIN_NCBI_SCOPE

class CAppJobNotification;

////////////////////////////////////////////////////////////////////////////////
/// CAlnVecMultiDataSource - implementation of IAlnMultiDataSource for
/// CAlnVec-based alignments.
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CAlnVecMultiDataSource
    :   public IAlnMultiDataSource,
        public CEventHandler
{
public:
    CAlnVecMultiDataSource(objects::CScope& scope);
    ~CAlnVecMultiDataSource();

    virtual void    Init(const objects::CSeq_annot& annot, bool sync = false, bool select_anchor = false);
    virtual void    Init(const objects::CBioseq_Handle& handle, bool sync = false, bool select_anchor = false);
    virtual void    Init(const vector< CConstRef<objects::CSeq_align> >&  aligns, 
                         bool sync = false, bool select_anchor = false);

    //virtual void    OnTaskFinished(CEvent* event);

    /// @name IAlnMultiDataSource implementation
    /// @{
    virtual bool    IsEmpty() const;

    virtual void  SetGapChar(TResidue gap_char);

    virtual IAlnExplorer::EAlignType  GetAlignType() const;

    virtual int     GetConsensusRow() const;

    virtual bool    CanCreateConsensus();
    void            CreateConsensus(vector<string>& consens) const;
    virtual void    CreateConsensus();
    virtual void    SetCreateConsensus(bool b) { m_CreateConsensus = b; }

    virtual TNumrow  GetNumRows(void) const;

    virtual TSeqPos  GetAlnStart(void) const;
    virtual TSeqPos  GetAlnStop(void) const;

    virtual TSeqPos GetSeqStart(TNumrow row) const;
    virtual TSeqPos GetSeqStop(TNumrow row) const;

    virtual bool  IsSetAnchor (void) const;
    virtual TNumrow  GetAnchor (void) const;
    virtual bool  CanChangeAnchor(void) const;
    virtual bool  SetAnchor (TNumrow anchor);
    virtual bool  UnsetAnchor (void);

    virtual bool    CanGetId(TNumrow row) const;
    virtual const objects::CSeq_id&  GetSeqId(TNumrow row) const;
    virtual const objects::CBioseq_Handle& GetBioseqHandle(TNumrow row) const;
    
    virtual TSeqPos GetBaseWidth(TNumrow row) const;

    virtual bool    IsPositiveStrand(TNumrow row) const;
    virtual bool    IsNegativeStrand(TNumrow row) const;

    virtual TSignedSeqPos GetSeqPosFromAlnPos(TNumrow for_row, TSeqPos aln_pos,
                                TSearchDirection dir = IAlnExplorer::eNone,
                                bool try_reverse_dir = true) const;
    virtual TSignedSeqPos GetAlnPosFromSeqPos(TNumrow row, TSeqPos seq_pos,
                                TSearchDirection dir = IAlnExplorer::eNone,
                                bool try_reverse_dir = true) const;

    virtual string& GetAlnSeqString(TNumrow row, string &buffer,
                                    const IAlnExplorer::TSignedRange& aln_range) const;

    virtual const IAlignRowHandle*  GetRowHandle(TNumrow row) const;

    virtual IAlnSegmentIterator*    CreateSegmentIterator(TNumrow row,
                                       const IAlnExplorer::TSignedRange& range,
                                       IAlnSegmentIterator::EFlags flags) const;

    virtual void  GetAlnFromSeq(TNumrow row, const TRangeColl& seq_coll,
                                TRangeColl& aln_coll) const;
    virtual void  GetSeqFromAln(TNumrow row, const TRangeColl& aln_coll,
                                TRangeColl& seq_coll) const;
    virtual bool  IsDataReadSync() const { return m_isDataReadSync; }

    /// @}

    /// @name ITimerListener implementation
    /// @{
    //virtual void    OnTimeout(int timer_id);
    /// @}
    void    OnAppJobNotification(CEvent* evt);

protected:

    virtual void    x_ClearHandles();
    virtual void    x_CreateHandles();
    virtual void    x_Assign(objects::CAlnVec& aln_vec);

    virtual bool    x_IsJobRunning();
    virtual void    x_DeleteJob();
    virtual void    x_OnJobProgress(CAppJobNotification& notn);

    DECLARE_EVENT_MAP();

private:
    // prohibit copying!
    CAlnVecMultiDataSource(const CAlnVecMultiDataSource& ds);
    CAlnVecMultiDataSource& operator=(const CAlnVecMultiDataSource& ds);

protected:
    typedef vector<CAlnVecRowHandle*>    THandleVector;

    CRef<objects::CScope>   m_Scope;
    CRef<objects::CAlnVec>  m_AlnVec;
    int m_ConsRowIndex;
    bool m_CreateConsensus;

    CRef<CBuildAlnVecJob>   m_Job;
    int m_JobID;

    THandleVector   m_Handles;

private:
    bool    m_isDataReadSync;

};





END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_MULTIPLE___ALNVEC_MULTI_DS__HPP
