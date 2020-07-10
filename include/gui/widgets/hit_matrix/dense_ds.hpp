#ifndef GUI_WIDGETS_HIT_MATRIX___DENSE_DS__HPP
#define GUI_WIDGETS_HIT_MATRIX___DENSE_DS__HPP

/*  $Id: dense_ds.hpp 18736 2009-02-04 17:14:27Z dicuccio $
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

#include <objtools/alnmgr/aln_converters.hpp>
#include <objtools/alnmgr/seqids_extractor.hpp>
#include <objtools/alnmgr/aln_tests.hpp>
#include <objtools/alnmgr/aln_stats.hpp>

#include <gui/gui_export.h>

#include <objects/seq/Seq_annot.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqalign/Score.hpp>
#include <objects/general/Object_id.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>

#include <serial/iterator.hpp>

#include <gui/widgets/hit_matrix/hit_matrix_ds.hpp>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CHitSeqId
class NCBI_GUIWIDGETS_HIT_MATRIX_EXPORT CHitSeqId : public IHitSeqId
{
public:
    CHitSeqId(const objects::CSeq_id& id)    :   m_SeqId(&id)    {}
    CHitSeqId(const CHitSeqId& id)    :   m_SeqId(id.m_SeqId)    {}

    virtual bool    Equals(const IHitSeqId& id) const;
    virtual IHitSeqId*  Clone() const;

    virtual CConstRef<objects::CSeq_id>  GetSeqId()  const
        {   return m_SeqId; }
protected:
    CConstRef<objects::CSeq_id>   m_SeqId;
};


///////////////////////////////////////////////////////////////////////////////
/// CHitSeqRowId
class NCBI_GUIWIDGETS_HIT_MATRIX_EXPORT CHitSeqRowId : public IHitSeqId
{
public:
    CHitSeqRowId(int row, const objects::CSeq_id& id);
    CHitSeqRowId(const CHitSeqRowId& id);

    virtual bool    Equals(const IHitSeqId& id) const;
    virtual IHitSeqId*  Clone() const;

    virtual CConstRef<objects::CSeq_id>  GetSeqId() const;

    int GetRow() const  {   return m_Row;   }
protected:
    int     m_Row;
    CConstRef<objects::CSeq_id>   m_SeqId;
};


///////////////////////////////////////////////////////////////////////////////
/// CHitMatrixDataSource
class NCBI_GUIWIDGETS_HIT_MATRIX_EXPORT CHitMatrixDataSource
    :   public CObject,
        public IHitMatrixDataSource
{
public:
    CHitMatrixDataSource();
    virtual ~CHitMatrixDataSource();

    /// @name IHitMatrixDataSource implementation
    /// @{
    void    Init(objects::CScope& scope, TAlignVector& aligns);

    virtual bool    CanCreateRowBased() const;
    virtual void    SetParams(const SParams& params, bool create_hits);
    virtual void    GetParams(SParams& params) const;

    virtual bool    SelectIds(const IHitSeqId& q_id,
                              const IHitSeqId& s_id);

    virtual bool    SelectDefaultIds();
    virtual const TIdVector&    GetHitSeqIds();
    virtual void    GetHitSeqIdsForParams(TIdVector& ids,
                                          const SParams& params) const;

    virtual void    GetAlignedIds(const IHitSeqId& id, TIdVector& aligned);

    virtual void    GetAlignedIdsForParams(const IHitSeqId& id,
                                           const TIdVector& ids,
                                           TIdVector& aligned,
                                           const SParams& params);

    virtual objects::CScope&    GetScope();
    virtual IHitSeqId&  GetQueryId()    const;
    virtual IHitSeqId&  GetSubjectId()  const;
    virtual objects::CBioseq_Handle GetQueryHandle();
    virtual objects::CBioseq_Handle GetSubjectHandle();
    virtual objects::CBioseq_Handle GetBioseqHandle(const IHitSeqId& id);

    virtual string  GetLabel(const IHitSeqId& id);

    virtual void    GetScoreNames(vector<string>& names)    const;
    TValueRange     GetScoreRange(const string& name) const;

    const THitAdapterCont&  GetHits()   const;

    TSeqRange   GetQueryHitsRange();
    TSeqRange   GetSubjectHitsRange();
    /// @}

protected:
    typedef vector<CConstRef<objects::CSeq_id> > TSeqIdRefVector;
    typedef objects::CSeq_align::TSegs::TStd        TStd;

    typedef vector<const CSeq_align*> TAlnPtrVector;
    typedef CAlnSeqIdsExtract<CAlnSeqId>    TIdExtract;
    typedef CAlnIdMap<TAlnPtrVector, TIdExtract> TAlnIdMap;

    void    x_TestAndSelectAligns(TAlignVector& aligns);
    void    x_TestAndSelectAlign(const objects::CSeq_align& align);

    bool    x_GoodSeg(const TStd& std_list);

    TSeqRange GetStdSegRange(const TStd& std_list, size_t row);

    void    x_Clear();
    void    x_ClearHits();

    bool    x_CanCreateRowBased();

    void    x_UpdateScoreMap();
    void    x_GetSeqAlignIds(const objects::CSeq_align& align,
                             TSeqIdRefVector& ids, bool unique) const;
    void    x_GetAlignedSeqIds(const CSeq_align& align,
                               const IHitSeqId& id,
                               TSeqIdRefVector& ids) const;

    void    x_CreateIds(TIdVector& seq_ids, const SParams& params) const;

    void    x_CreateHits(const objects::CSeq_align& align);
    void    x_CreateHit(const objects::CSeq_align& align,
                        size_t q_index, size_t s_index);
    void    x_CreateHit_Denseg(const objects::CSeq_align& align,
                               size_t q_index, size_t s_index);
    void    x_CreateHit_Std(const objects::CSeq_align& align,
                            size_t q_index, size_t s_index);
    void    x_CreateHits_Diagonal(const objects::CSeq_align& align,
                                 size_t q_index, size_t s_index);

    void    x_CalculateHitsRange();

protected:
    typedef map<string, TValueRange> TScoreMap;// symbol name -> [min, max]
    typedef CSeq_align::TDim TDim;
    typedef const CSeq_id*  TSeqIdPtr;
    typedef vector<TSeqIdPtr>   TSeqIdVector;
    typedef CAlnStats<TAlnIdMap> TAlnStats;
    typedef set<objects::CBioseq_Handle>  THIdToHandleMap;

    TAlignVector    m_Aligns;
    CRef<objects::CScope>   m_Scope;

    auto_ptr<TAlnIdMap>   m_AlnIdMap;
    CRef<TAlnStats> m_AlnStats;
    //TIdToAlnsMap  m_IdToAlnsMap;

    bool m_CanCreateRowBased;
    SParams m_Params;

    TIdVector    m_SeqIds; /// list of Ids for all sequences
    mutable THIdToHandleMap m_HIdToHandleMap;
    TScoreMap    m_ScoreMap;

    auto_ptr<IHitSeqId>  m_SubjectID;
    auto_ptr<IHitSeqId>  m_QueryID;

    objects::CBioseq_Handle  m_SubjectHandle;
    objects::CBioseq_Handle  m_QueryHandle;

    THitAdapterCont m_Hits;

    TSeqRange   m_SubjectHitsRange;
    TSeqRange   m_QueryHitsRange;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_HIT_MATRIX___DENSE_DS__HPP
