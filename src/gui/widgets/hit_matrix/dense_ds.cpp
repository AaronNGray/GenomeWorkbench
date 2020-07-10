/*  $Id: dense_ds.cpp 24043 2011-07-13 18:20:33Z kuznets $
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

#include <ncbi_pch.hpp>

#include <corelib/ncbistd.hpp>

#include <gui/widgets/hit_matrix/dense_ds.hpp>
#include <gui/widgets/hit_matrix/dense_hit.hpp>
#include <gui/widgets/hit_matrix/diag_hit.hpp>
#include <gui/widgets/hit_matrix/std_seg_hit.hpp>

#include <gui/objutils/label.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqalign/Std_seg.hpp>

#include <algorithm>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


///////////////////////////////////////////////////////////////////////////////
/// CHitSeqId

bool CHitSeqId::Equals(const IHitSeqId& id) const
{
    const CHitSeqId* hit_id = dynamic_cast<const CHitSeqId*>(&id);
    if(hit_id)  {
        CConstRef<CSeq_id> s_id = hit_id->m_SeqId;
        return m_SeqId->Equals(*s_id);
    }
    return false;
}


IHitSeqId* CHitSeqId::Clone() const
{
    return new CHitSeqId(*m_SeqId);
}

///////////////////////////////////////////////////////////////////////////////
/// CHitSeqRowId

CHitSeqRowId::CHitSeqRowId(int row, const objects::CSeq_id& id)
:   m_Row(row),
    m_SeqId(&id)
{
}


CHitSeqRowId::CHitSeqRowId(const CHitSeqRowId& id)
:   m_Row(id.m_Row),
    m_SeqId(id.m_SeqId)
{
}


bool CHitSeqRowId::Equals(const IHitSeqId& id) const
{
    const CHitSeqRowId* r_id = dynamic_cast<const CHitSeqRowId*>(&id);
    if(r_id)    {
        return m_Row == r_id->m_Row  &&  m_SeqId->Equals(*r_id->m_SeqId);
    }
    return false;
}


IHitSeqId* CHitSeqRowId::Clone() const
{
    return new CHitSeqRowId(m_Row, *m_SeqId);
}


CConstRef<objects::CSeq_id> CHitSeqRowId::GetSeqId() const
{
    return m_SeqId;
}


///////////////////////////////////////////////////////////////////////////////
/// CHitMatrixDataSource
CHitMatrixDataSource::CHitMatrixDataSource()
{
    m_Params.m_Direction = CAlnUserOptions::eBothDirections;
}


CHitMatrixDataSource::~CHitMatrixDataSource()
{
    x_Clear();
}


void CHitMatrixDataSource::x_ClearHits()
{
    CBioseq_Handle h_null;
    m_QueryHandle = h_null;
    m_SubjectHandle = h_null;

    m_SubjectHitsRange = TSeqRange::GetEmpty();
    m_QueryHitsRange = TSeqRange::GetEmpty();

    ITERATE(THitAdapterCont, it_a, m_Hits)   {
        delete(*it_a);
    }
    m_Hits.clear();

    m_SubjectID.reset();
    m_QueryID.reset();
}


void CHitMatrixDataSource::x_Clear()
{
    x_ClearHits();

    m_ScoreMap.clear();

    ITERATE(TIdVector, it_s, m_SeqIds)   {
        delete(*it_s);
    }

    m_HIdToHandleMap.clear();
    m_SeqIds.clear();
    m_CanCreateRowBased = false;

    m_AlnIdMap.reset();
    m_AlnStats.Reset();

    m_Scope.Reset();
    m_Aligns.clear();
}


template<typename TIDRefCont>
    typename TIDRefCont::const_iterator
        find_id(const TIDRefCont& cont, const IHitSeqId& id)
{
    ITERATE (typename TIDRefCont, it, cont) {
        if((*it)->Equals(id))
            return it;
    }
    return cont.end();
}


void CHitMatrixDataSource::Init(CScope& scope, TAlignVector& aligns)
{
    x_Clear();

    x_TestAndSelectAligns(aligns);

    m_Scope.Reset(&scope);

    x_UpdateScoreMap();

    m_CanCreateRowBased = x_CanCreateRowBased();
    if(! m_CanCreateRowBased)   {
        m_Params.m_RowBased = false;
    }

    x_CreateIds(m_SeqIds, m_Params);

    /**
    int delta = aligns.size() - m_Aligns.size();
    if(delta > 0) {
        // TODO remove error reporting, Data Source should be clean of GUI
        string msg = "Some of the Seq-aligns are invalid or incompatible with Dot Matrix View.\n\
                     See diagnostic output for details.";
        NcbiErrorBox(msg, "Dot Matrix View - Invalid Data");
    }
    **/
}


bool FIdEqual(const CConstRef<CSeq_id>& id1, const CConstRef<CSeq_id>& id2)
{
    return id1->Equals(*id2);
}


void CHitMatrixDataSource::x_TestAndSelectAlign(const CSeq_align& align)
{
    typedef CSeq_align::C_Segs  TSegs;
    bool check_std = false;

    TSegs::E_Choice seg_type = align.GetSegs().Which();
    switch(seg_type) {
    case TSegs::e_Std:
        check_std = true;

    case TSegs::e_Denseg:
    case TSegs::e_Dendiag:
    case TSegs::e_Sparse:
    case TSegs::e_Spliced:
        try {
            bool good = true;
            if(check_std)   {
                good = x_GoodSeg(align.GetSegs().GetStd());
            }
            if(good)  {
                m_AlnIdMap->push_back(align); // this tests the Seq-align
                m_Aligns.push_back(CConstRef<CSeq_align>(&align));
            }
        } catch (CAlnException e) {
            ERR_POST("CHitMatrixDataSource skipping Seq-align - " << e.what());
        }
        break;

    case TSegs::e_Disc:
        ITERATE (CSeq_align::TSegs::TDisc::Tdata, it, align.GetSegs().GetDisc().Get()) {
            x_TestAndSelectAlign(**it);
        }
        break;
        

    default:
        ERR_POST("CHitMatrixDataSource::x_TestAndSelectAligns() - unsupported type of Seq-align");
        break;
    }
}

void CHitMatrixDataSource::x_TestAndSelectAligns(TAlignVector& aligns)
{

    m_AlnIdMap.reset(new TAlnIdMap(TIdExtract(), aligns.size()));

    ITERATE(TAlignVector, it, aligns)   {
        const CSeq_align& align  = **it;
        x_TestAndSelectAlign(align);
    }

    m_AlnStats.Reset(new TAlnStats(*m_AlnIdMap));
}


bool CHitMatrixDataSource::x_GoodSeg(const TStd& std_list)
{
    ITERATE(TStd, it, std_list) {
        const CStd_seg& stdseg = **it;

        CStd_seg::TLoc locs = stdseg.GetLoc();
        ITERATE(CStd_seg::TLoc, it_loc, locs)  {
            const CSeq_loc& loc = **it_loc;
            ENa_strand strand = loc.GetStrand();
            if(strand != eNa_strand_plus  &&  strand != eNa_strand_minus
                && strand != eNa_strand_unknown)   {
                LOG_POST(Info << "Seq-locs in a Std-seg have invalid strand information");
                return false;
            }
        }
    }
    return true;
}


bool CHitMatrixDataSource::x_CanCreateRowBased()
{
    // TODO this can be optimized with the new API
    const TAlnIdMap::TIdVec* ids = NULL;
    for( size_t i = 0;  i < m_AlnIdMap->size(); i++ )   {
        const TAlnIdMap::TIdVec& ids_2 = (*m_AlnIdMap)[i];
        if(ids == NULL) {
            ids = &ids_2;
        } else {
            if(*ids != ids_2)   {
                return false;
            }
        }
    }
    return true;
}


void CHitMatrixDataSource::SetParams(const SParams& params, bool create_hits)
{
    bool based_ch = (m_Params.m_RowBased != params.m_RowBased);
    bool orient_ch = (m_Params.m_Direction != params.m_Direction);

    if(based_ch  ||  orient_ch)  {
        /// save IDs for later use
        auto_ptr<IHitSeqId> s_id(m_SubjectID.release());
        auto_ptr<IHitSeqId> q_id(m_QueryID.release());

        // delete Hits and clear IDs
        x_ClearHits();

        if(based_ch)    {
            _ASSERT(m_CanCreateRowBased  ||  ! params.m_RowBased);

            s_id.reset();
            q_id.reset();

            m_Params.m_RowBased = params.m_RowBased;
            m_SeqIds.clear();

            x_CreateIds(m_SeqIds, m_Params);
        }

        m_Params.m_Direction = params.m_Direction;

        // recreate Hits if needed (and if possible)
        if(create_hits  &&  s_id.get()  &&  q_id.get())  {
            SelectIds(*s_id, *q_id);
        }
    }
}


bool CHitMatrixDataSource::CanCreateRowBased() const
{
    return m_CanCreateRowBased;
}


void CHitMatrixDataSource::GetParams(SParams& params) const
{
    params = m_Params;
}


/// builds a map of all scores found in the alignments
void CHitMatrixDataSource::x_UpdateScoreMap()
{
    for(size_t i = 0;  i < m_Aligns.size();  i++ ) {
        const CSeq_align& align = *m_Aligns[i];

        if(align.CanGetSegs())  {
            const CSeq_align::TScore& scores = align.GetScore();
            ITERATE(CSeq_align::TScore , itSc, scores)  {
                const CScore&   score = **itSc;
                if(score.CanGetId())    {
                    const CObject_id&   id = score.GetId();

                    // get score value
                    double value = -1;
                    const CScore::C_Value& val = score.GetValue();
                    switch(val.Which()) {
                    case CScore::C_Value::e_Real: value = val.GetReal(); break;
                    case CScore::C_Value::e_Int: value = val.GetInt(); break;
                    default:    _ASSERT(false);
                    }

                    TScoreMap::iterator it_score = m_ScoreMap.find(id.GetStr());
                    if(it_score ==  m_ScoreMap.end())   {
                        // new Score found - insert it
                        m_ScoreMap[id.GetStr()] = make_pair(value, value);
                    } else {
                        // update value range
                        TValueRange& range = it_score->second;
                        range.first = min(range.first, value);
                        range.second = max(range.second, value);
                    }
                }
            }
        }
    }
}


class CSeqIdEquals
{
public:
    CSeqIdEquals(const CSeq_id& id)
        : m_Id(&id)  {}

    CSeqIdEquals(const CRef<CSeq_id>& id)
        : m_Id(id)  {}

    CSeqIdEquals(const CConstRef<CSeq_id>& id)
        : m_Id(id)  {}

    bool operator()(const CSeq_id& id) const
    {
        return m_Id->Equals(id);
    }
    bool operator()(CConstRef<CSeq_id>& id) const
    {
        return m_Id->Equals(*id);
    }
    bool operator()(IHitSeqId* id) const
    {
        return id->GetSeqId()->Equals(*m_Id);
    }
protected:
    CConstRef<CSeq_id> m_Id;
};


void CHitMatrixDataSource::x_GetAlignedSeqIds(const CSeq_align& align,
                                              const IHitSeqId& id,
                                              TSeqIdRefVector& ids) const
{
    _ASSERT(align.CanGetSegs());
    typedef CSeq_align::C_Segs  TSegs;

    switch(align.GetSegs().Which()) {
    case TSegs::e_Denseg:
    case TSegs::e_Dendiag:
    case TSegs::e_Sparse:
    case TSegs::e_Spliced:
    case TSegs::e_Std:  {
        const CSeq_id& seq_id = *id.GetSeqId();
        const TAlnIdMap::TIdVec& id_vec = (*m_AlnIdMap)[align];
        int self_count = 0;

        ITERATE(TAlnIdMap::TIdVec, it, id_vec)  {
            const IAlnSeqId& aln_id = **it;
            const CSeq_id& id = aln_id.GetSeqId();
            CSeqIdEquals Eq(id);
            bool add = true;

            if(Eq(seq_id))  {
                self_count++;
                add = (self_count == 2);
            }
            if(add) {
                if(ids.end() == std::find_if(ids.begin(), ids.end(), Eq))   {
                    ids.push_back(CConstRef<CSeq_id>(&id));
                }
            }
        }

        if(self_count == 0) {
            ids.clear();
        }
        break;
    }
    default:
        break;
    }
}


void CHitMatrixDataSource::x_GetSeqAlignIds(const CSeq_align& align,
                                            TSeqIdRefVector& ids, bool unique) const
{
    _ASSERT(align.CanGetSegs());
    typedef CSeq_align::C_Segs  TSegs;

    switch(align.GetSegs().Which()) {
    case TSegs::e_Denseg:
    case TSegs::e_Dendiag:
    case TSegs::e_Sparse:
    case TSegs::e_Spliced:
    case TSegs::e_Std:  {
        const TAlnIdMap::TIdVec& id_vec = (*m_AlnIdMap)[align];
        ITERATE(TAlnIdMap::TIdVec, it, id_vec)  {
            const IAlnSeqId& aln_id = **it;
            const CSeq_id& id = aln_id.GetSeqId();

            if(unique)  {
                CSeqIdEquals Eq(id);
                if(ids.end() == std::find_if(ids.begin(), ids.end(), Eq))   {
                    ids.push_back(CConstRef<CSeq_id>(&id));
                }
            } else {
                ids.push_back(CConstRef<CSeq_id>(&id));
            }
        }
        break;
    }
    default:
        break;
    }
}


// iterates on Seq-aligns and builds a unique set of Seq-ids
void CHitMatrixDataSource::x_CreateIds(TIdVector& seq_ids,
                                       const SParams& params) const
{
    if(params.m_RowBased)   {
        /// create CHitSeqRowId based on the first Seq-align (all others must be
        /// identical)
        if(m_Aligns.size()) {
            const CSeq_align& align = *m_Aligns[0];
            // alignment must be of type Dense-seg

            TSeqIdRefVector aln_ids;
            x_GetSeqAlignIds(align, aln_ids, false);

            for( size_t row = 0;  row < aln_ids.size();  row++ )   {
                const CSeq_id& seq_id = *aln_ids[row];
                seq_ids.push_back(new CHitSeqRowId((int)row, seq_id));
            }
        }
    } else {
        // ID-based alignment
        for(size_t i = 0;  i < m_Aligns.size();  i++ ) {
            const CSeq_align& align = *m_Aligns[i];

            TSeqIdRefVector aln_ids;
            x_GetSeqAlignIds(align, aln_ids, true);

            ITERATE(TSeqIdRefVector, itID, aln_ids)  {
                CConstRef<CSeq_id> seq_id = *itID;
                // find this ID in the vector
                CSeqIdEquals Eq(seq_id);
                if(seq_ids.end() == std::find_if(seq_ids.begin(), seq_ids.end(), Eq))   {
                    seq_ids.push_back(new CHitSeqId(*seq_id));
                }
            }
        }
    }

    //TODO prefetching bioseq handles
    CScope& scope = const_cast<CScope&>(m_Scope.GetObject());
    ITERATE( TIdVector, it, m_SeqIds ){
        IHitSeqId& id = **it;
        m_HIdToHandleMap.insert(scope.GetBioseqHandle(*id.GetSeqId()));
    }
}


const CHitMatrixDataSource::TIdVector& CHitMatrixDataSource::GetHitSeqIds()
{
    return m_SeqIds;
}


void CHitMatrixDataSource::GetHitSeqIdsForParams(TIdVector& ids,
                                                 const SParams& params) const
{
    if(params == m_Params)  {
        // params are the same as those currently set in the Data Source
        // make a copy of objects in m_SeqIds
        ids = m_SeqIds;
        for( size_t i = 0;  i < ids.size(); i++ )   {
            ids[i] = ids[i]->Clone();
        }
    } else {
        x_CreateIds(ids, params);
    }
}


void CHitMatrixDataSource::GetAlignedIds(const IHitSeqId& id, TIdVector& aligned)
{
    GetAlignedIdsForParams(id, m_SeqIds, aligned, m_Params);
}


// returns ids for all sequences aligned with the given sequence
void CHitMatrixDataSource::GetAlignedIdsForParams(const IHitSeqId& id,
                                                  const TIdVector& ids,
                                                  TIdVector& aligned,
                                                  const SParams& params)
{
    //CStopWatch sw; sw.Start();
    aligned.clear();

    if(params.m_RowBased)  {
        ITERATE(TIdVector, it, ids)    {
            if(! (*it)->Equals(id)) {
                aligned.push_back(*it);
            }
        }
    } else {
        // Seq-id based identification
        TIdVector::const_iterator it_id = find_id(ids, id);

        if(it_id != ids.end())   {
            // TODO get a list of Seq-aligns from m_AlnStats


            CIRef<IAlnSeqId> aln_id(new CAlnSeqId(*id.GetSeqId()));
            const TAlnStats::TIdVec& aligned_ids = m_AlnStats->GetAlignedIds(aln_id);
            ITERATE(TAlnStats::TIdVec, it_a, aligned_ids)   {
                const IAlnSeqId& aa = **it_a;
                aligned.push_back(new CHitSeqId(aa.GetSeqId()));
            }

            /* old code
            // there are alignments with this Id - iterate by all seq_aligns
            ITERATE(TAlignVector, it, m_Aligns) {
                const CSeq_align& align = **it;

                // get the list of Seq-ids in this Seq-align
                TSeqIdRefVector aln_ids;
                x_GetAlignedSeqIds(align, id, aln_ids);

                for( size_t index = 0; index < aln_ids.size();  index++ ) {
                    CHitSeqId s_id(*aln_ids[index]);

                    TIdVector::const_iterator it = find_id(aligned, s_id);
                    if(it == aligned.end()) { // not in our vector yet
                        aligned.push_back(s_id.Clone());
                    }
                }
            }
            */
        } else {
            _ASSERT(false);
        }
    }
    //double t = sw.Elapsed();
    //LOG_POST(Info << "CHitMatrixDataSource::GetAlignedIdsForParams()  - " << (1000.0 * t) << " ms");
}


bool CHitMatrixDataSource::SelectDefaultIds()
{
    if(! m_SeqIds.empty())    {
        size_t index_2 = min<size_t>(1, m_SeqIds.size() - 1);
        return SelectIds(*m_SeqIds[0], *m_SeqIds[index_2]);
    }
    return false;
}


/// Creates Hits for all pairwise alignments between selected by Query and Subject IDs
bool CHitMatrixDataSource::SelectIds(const IHitSeqId& s_id, const IHitSeqId& q_id)
{
    //LOG_POST("CHitMatrixDataSource::SelectIds  subject " << GetLabel(s_id)
    //         << ", query " << GetLabel(q_id));
    x_ClearHits();

    // check that both ids exist in data
    TIdVector::const_iterator itQuery = find_id(m_SeqIds, q_id);
    TIdVector::const_iterator itSubject = find_id(m_SeqIds, s_id);

    bool b_ok = itQuery != m_SeqIds.end() &&  itSubject != m_SeqIds.end();

    if(b_ok)    {
        // iterate though Seq_aligns and create Hit Adapters
        m_SubjectID.reset(s_id.Clone());
        m_QueryID.reset(q_id.Clone());

        m_QueryHandle = m_Scope->GetBioseqHandle(*m_QueryID->GetSeqId());
        m_SubjectHandle = m_Scope->GetBioseqHandle(*m_SubjectID->GetSeqId());

        // iterate by all seq_aligns
        ITERATE(TAlignVector, it, m_Aligns) {
            const CSeq_align& align = **it;
            x_CreateHits(align);
        }

        x_CalculateHitsRange();
    }
    return b_ok;
}


void CHitMatrixDataSource::x_CreateHits(const CSeq_align& align)
{
    _ASSERT(align.IsSetSegs());

    if(m_Params.m_RowBased) {
        /// Create a hit for exactly one pair of rows
        const CHitSeqRowId* q_id =
            dynamic_cast<const CHitSeqRowId*>(m_QueryID.get());
        const CHitSeqRowId* s_id =
            dynamic_cast<const CHitSeqRowId*>(m_SubjectID.get());
        _ASSERT(q_id  &&  s_id);

        int q_index = q_id->GetRow();
        int s_index = s_id->GetRow();

        x_CreateHit(align, q_index, s_index);
    } else {
        /// create hits for all possible unique combinations or rows
        /// corresponding to the given Seq-ids
        TSeqIdRefVector aln_ids;
        x_GetSeqAlignIds(align, aln_ids, false);

        // find all rows in the Seq-align where subject ID occurs
        vector<size_t> s_rows;
        size_t n_rows = aln_ids.size();
        for( size_t s_index = 0;  s_index < n_rows;  s_index++ ) {
            const CSeq_id& id = *aln_ids[s_index];
            if(m_SubjectID->GetSeqId()->Equals(id))  {
                s_rows.push_back(s_index);
            }
        }

        // for every row corresponding to the Query Seq-id
        for( size_t q_index = 0;  q_index < n_rows;  q_index++ ) {
            const CSeq_id& id = *aln_ids[q_index];

            if(m_QueryID->GetSeqId()->Equals(id))   {
                // create Hits for all rows corresponding to the Subject Seq-id
                for( size_t i = 0;  i < s_rows.size();  i++ )   {
                    size_t s_index = s_rows[i];
                    x_CreateHit(align, q_index, s_index);
                }
            }
        }
    }
}


// creates a Hits from the two specified rows of the alignment
void CHitMatrixDataSource::x_CreateHit(const CSeq_align& align,
                                       size_t q_index, size_t s_index)
{
    if(align.IsSetSegs())   {
        typedef CSeq_align::C_Segs  TSegs;

        switch(align.GetSegs().Which()) {
        case TSegs::e_Std:
            x_CreateHit_Std(align, q_index, s_index);
            break;

        case TSegs::e_Denseg:
        case TSegs::e_Dendiag:
        case TSegs::e_Sparse:
        case TSegs::e_Spliced:
            x_CreateHits_Diagonal(align, q_index, s_index);
            break;

        default:
            LOG_POST(Info << "CHitMatrixDataSource() - this type of Seq-align"
                          << " is not supported");
            break;
        }
    }
}


/// creates a single hit from 2 rows of a Seq-align
void CHitMatrixDataSource::x_CreateHit_Denseg(const CSeq_align& align,
                                              size_t q_index, size_t s_index)
{
    _ASSERT(align.GetSegs().IsDenseg());

    const CDense_seg& denseg = align.GetSegs().GetDenseg();
    bool aln = CDenseSegHit::HasAlignment(denseg, q_index, s_index, m_Params.m_Direction);

    if(aln) {
        m_Hits.push_back(new CDenseSegHit(align, (int)q_index, (int)s_index));
    }
}


TSeqRange CHitMatrixDataSource::GetStdSegRange(const TStd& std_list, size_t row)
{
    TSeqRange range(TSeqRange::GetEmpty());

    ITERATE(list< CRef< CStd_seg > >, it, std_list) {
        const CStd_seg& stdseg = **it;
        TSeqRange r(stdseg.GetSeqStart((int)row), stdseg.GetSeqStop((int)row));

        if(range.Empty())   {
            range = r;
        } else {
            range.CombineWith(r);
        }
    }
    return range;
}


void CHitMatrixDataSource::x_CreateHit_Std(const CSeq_align& align,
                                            size_t q_index, size_t s_index)
{
    _ASSERT(align.GetSegs().IsStd());

    typedef list< CRef< CStd_seg > > TStd;
    const TStd& std_list = align.GetSegs().GetStd();

    bool aln = CStdSegHit::HasAlignment(std_list, q_index, s_index, m_Params.m_Direction);

    if(aln)    {
        m_Hits.push_back(new CStdSegHit(align, (int)q_index, (int)s_index));
    }
}


void CHitMatrixDataSource::x_CreateHits_Diagonal(const CSeq_align& align,
                                                size_t q_index, size_t s_index)
{
    TAlnIdMap::TIdVec ids = (*m_AlnIdMap)[align];
    TAlnSeqIdIRef& q_id = ids[q_index];
    TAlnSeqIdIRef& s_id = ids[s_index];

    CRef<CPairwiseAln> pairwise(new CPairwiseAln(q_id, s_id));
    ConvertSeqAlignToPairwiseAln(*pairwise, align, (int)q_index, (int)s_index, m_Params.m_Direction);
    if( ! pairwise->empty()) {
        m_Hits.push_back(new CDiagHit(align, *pairwise));
    }
}


// Determines the smallest rectangle in {query, subject} space that contains
// all the hits
void CHitMatrixDataSource::x_CalculateHitsRange()
{
    for( size_t i = 0;  i < m_Hits.size();  i++ )   {
        IHit& hit = *m_Hits[i];

        size_t n_elem = hit.GetElemsCount();
        for(  size_t j = 0;  j < n_elem;  j++ ) {
            const IHitElement& elem = hit.GetElem(j);

            TSeqPos q_from = elem.GetQueryStart();
            TSeqPos q_to = q_from + (TSeqPos) elem.GetQueryLength() - 1;

            TSeqPos s_from = elem.GetSubjectStart();
            TSeqPos s_to = s_from + (TSeqPos) elem.GetSubjectLength() - 1;

            if(m_QueryHitsRange.Empty()) {
                m_QueryHitsRange.Set(q_from, q_to);
            } else {
                m_QueryHitsRange += TSeqRange(q_from, q_to);
            }
            if(m_SubjectHitsRange.Empty()) {
                m_SubjectHitsRange.Set(s_from, s_to);
            } else {
                m_SubjectHitsRange += TSeqRange(s_from, s_to);
            }
        }
    }
}


CScope& CHitMatrixDataSource::GetScope()
{
    return m_Scope.GetObject();
}


IHitSeqId& CHitMatrixDataSource::GetQueryId() const
{
    return *m_QueryID;
}


IHitSeqId& CHitMatrixDataSource::GetSubjectId() const
{
    return *m_SubjectID;
}


CBioseq_Handle CHitMatrixDataSource::GetQueryHandle()
{
    return m_QueryHandle;
}


CBioseq_Handle CHitMatrixDataSource::GetSubjectHandle()
{
    return m_SubjectHandle;
}


CBioseq_Handle CHitMatrixDataSource::GetBioseqHandle(const IHitSeqId& id)
{
    _ASSERT(m_Scope);

    CConstRef<CSeq_id> seq_id = id.GetSeqId();
    if(seq_id) {
        return m_Scope->GetBioseqHandle(*seq_id);
    }
    return CBioseq_Handle(); // empty
}


string CHitMatrixDataSource::GetLabel(const IHitSeqId& id)
{
    _ASSERT(m_Scope);

    string s;
    const CHitSeqRowId* hit_id = dynamic_cast<const CHitSeqRowId*>(&id);
    if(hit_id)  {
        s = "Row " + NStr::IntToString(hit_id->GetRow()) + ", ";
    }
    CLabel::GetLabel(*id.GetSeqId(), &s, CLabel::eDefault, m_Scope.GetPointer());
    return s;
}


const CHitMatrixDataSource::THitAdapterCont&  CHitMatrixDataSource::GetHits()   const
{
    return m_Hits;
}


void    CHitMatrixDataSource::GetScoreNames(vector<string>& names)    const
{
    ITERATE(TScoreMap, it, m_ScoreMap)  {
        names.push_back(it->first);
    }
}


CHitMatrixDataSource::TValueRange
    CHitMatrixDataSource::GetScoreRange(const string& name) const
{
    TScoreMap::const_iterator it = m_ScoreMap.find(name);
    _ASSERT(it != m_ScoreMap.end());
    return it->second;
}


TSeqRange   CHitMatrixDataSource::GetQueryHitsRange()
{
    return m_QueryHitsRange;
}


TSeqRange   CHitMatrixDataSource::GetSubjectHitsRange()
{
    return m_SubjectHitsRange;
}


END_NCBI_SCOPE
