#ifndef GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_DS__HPP
#define GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_DS__HPP

/*  $Id: hit_matrix_ds.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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
#include <corelib/ncbistl.hpp>

#include <gui/gui_export.h>

#include <gui/widgets/hit_matrix/hit.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqalign/Seq_align.hpp>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// IHitSeqId - abstract identifier of a sequence in an alignment. Depending
/// on the alignment type it can be Seq_id, row index or something else.

class NCBI_GUIWIDGETS_HIT_MATRIX_EXPORT IHitSeqId
{
public:
    virtual ~IHitSeqId()    {};

    virtual bool        Equals(const IHitSeqId& id) const = 0;
    virtual IHitSeqId*  Clone() const = 0;

    /// returns CSeq_id associated with IHitSeqId
    virtual CConstRef<objects::CSeq_id>  GetSeqId()  const = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// IHitMatrixDataSource - abstract Data Source for Hit Matrix Widget
class NCBI_GUIWIDGETS_HIT_MATRIX_EXPORT IHitMatrixDataSource
{
public:
    typedef vector< CConstRef<objects::CSeq_align> >  TAlignVector;
    typedef vector<IHitSeqId*>  TIdVector;
    typedef vector<IHit*>  THitAdapterCont;
    typedef pair<double, double>    TValueRange;
    typedef IHit::TDirection    TDirection;

    struct SParams {
        bool    m_RowBased;
        TDirection  m_Direction;

        bool operator==(const SParams& p) const
        {
            return m_RowBased == p.m_RowBased  &&  m_Direction == p.m_Direction;
        }
    };

    virtual ~IHitMatrixDataSource() {};

    virtual void    Init(objects::CScope& scope, TAlignVector& aligns) = 0;

    virtual bool    CanCreateRowBased() const = 0;
    virtual void    SetParams(const SParams& params, bool create_hits) = 0;
    virtual void    GetParams(SParams& params) const = 0;

    /// selects hits by query and subject IDs
    virtual bool    SelectIds(const IHitSeqId& q_id,
                              const IHitSeqId& s_id) = 0;

    /// selects default query and subject
    virtual bool    SelectDefaultIds() = 0;

    /// returns list of all seq ids in the Data Source
    virtual const TIdVector& GetHitSeqIds() = 0;

    /// returns a list of ids that Data Source will create if the given params
    /// are applied. The caller is responsible for deleting ids.
    virtual void    GetHitSeqIdsForParams(TIdVector& ids,
                                          const SParams& params) const = 0;

    // returns ids for all sequences aligned with the given sequence
    virtual void    GetAlignedIds(const IHitSeqId& id, TIdVector& aligned) = 0;

    /// returns a list of ids that will be aligned with the specified id if
    /// the given params are applied.
    virtual void GetAlignedIdsForParams(const IHitSeqId& id,
                                        const TIdVector& ids,
                                        TIdVector& aligned,
                                        const SParams& params) = 0;

    virtual objects::CScope&     GetScope() = 0;
    virtual IHitSeqId&      GetQueryId()    const = 0;
    virtual IHitSeqId&      GetSubjectId()  const = 0;

    virtual objects::CBioseq_Handle GetQueryHandle() = 0;
    virtual objects::CBioseq_Handle GetSubjectHandle() = 0;
    virtual objects::CBioseq_Handle GetBioseqHandle(const IHitSeqId& id) = 0;

    virtual string  GetLabel(const IHitSeqId& id) = 0;

    virtual void    GetScoreNames(vector<string>& names)const = 0;
    virtual TValueRange    GetScoreRange(const string& name) const = 0;

    virtual const THitAdapterCont&  GetHits()   const = 0;

    virtual TSeqRange   GetQueryHitsRange() = 0;
    virtual TSeqRange   GetSubjectHitsRange() = 0;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_DS__HPP
