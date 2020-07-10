/*  $Id: alnmulti_ds_builder.cpp 44958 2020-04-28 18:04:11Z shkeda $
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

#include <gui/widgets/aln_multiple/alnmulti_ds_builder.hpp>

#include <gui/objutils/sparse_functions.hpp>

#include <gui/widgets/aln_multiple/sparse_multi_ds.hpp>
#include <gui/widgets/aln_multiple/alnvec_multi_ds.hpp>

#include <gui/objutils/utils.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seq/seq_id_handle.hpp>

#include <corelib/ncbitime.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/bioseq_handle.hpp>

#include <objtools/alnmgr/aln_converters.hpp>
#include <objtools/alnmgr/aln_builders.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


CAlnMultiDSBuilder::CAlnMultiDSBuilder()
    : m_CreateSparse( true )
    , m_SyncCreate( false )
    , m_Listener(0)
{
}


CAlnMultiDSBuilder::~CAlnMultiDSBuilder()
{
    x_Clear();
}


void CAlnMultiDSBuilder::Init(objects::CScope& scope, const objects::CSeq_align& align)
{
    x_Clear();

    m_Scope.Reset(&scope);
    m_OrigAligns.push_back(CConstRef<CSeq_align>(&align));
}


void CAlnMultiDSBuilder::Init(objects::CScope& scope, const objects::CSeq_annot& annot)
{
    x_Clear();

    m_Scope.Reset(&scope);
    ExtractSeqAligns(annot, m_OrigAligns);
}


void CAlnMultiDSBuilder::Init(objects::CScope& scope, const objects::CBioseq& bioseq)
{
    x_Clear();

    m_Scope.Reset(&scope);
    ExtractSeqAligns(bioseq, m_OrigAligns);
}

void CAlnMultiDSBuilder::Init(objects::CScope& scope, const objects::CSeq_entry& seq_entry)
{
    x_Clear();

    m_Scope.Reset(&scope);
    ExtractSeqAligns(seq_entry, m_OrigAligns);

    CTypeConstIterator<objects::CBioseq> it(seq_entry);
    while(it)   {
        m_Scope->AddBioseq(*it);
        ++it;
    }
}

void CAlnMultiDSBuilder::Init(objects::CScope& scope, const objects::CBioseq_Handle& handle)
{
    x_Clear();

    m_Scope.Reset(&scope);

    SAnnotSelector sel = CSeqUtils::GetAnnotSelector(CSeq_annot::TData::e_Align);
    CAlign_CI it(handle, sel);
    int i = 0;
    for ( ;  it;  ++it) {
        m_OrigAligns.push_back(CConstRef<CSeq_align>(&*it));
        if(i++ > 20)
            return;
    }
}


void CAlnMultiDSBuilder::Init(objects::CScope& scope, TAlignVector& aligns)
{
    x_Clear();

    m_Scope.Reset(&scope);
    m_OrigAligns = aligns;
}


/// initial data set from which an alignment will be build
void CAlnMultiDSBuilder::Init(CScope& scope, TAnnotVector& annots)
{
    x_Clear();

    m_Scope.Reset(&scope);

    ITERATE(TAnnotVector, it_annot, annots) {
        const CSeq_annot& annot = **it_annot;
        if(annot.GetData().IsAlign() ) {
            ExtractSeqAligns(annot, m_OrigAligns);
        }
    }
}


void CAlnMultiDSBuilder::x_Clear()
{
    m_Scope.Reset();
    m_OrigAligns.clear();
    m_AlnStats.Reset();
    m_AnchoredAlns.clear();
    m_MasterId.Reset();
    m_CreateSparse = true;
}


void CAlnMultiDSBuilder::PreCreateDataSource(bool sparse)
{
    m_CreateSparse = sparse;
    if(m_CreateSparse)  {
        x_PreCreateSparseDataSource();
    }
}


CRef<IAlnMultiDataSource>
    CAlnMultiDSBuilder::CreateDataSource()
{
    _TRACE( (unsigned long)m_OrigAligns.size() << " m_OrigAligns");

    x_TestAlignments();

    if(m_CreateSparse)  {
        return x_CreateSparseDataSource();
    } else {
        return x_CreateAlnVecDataSource();
    }
}


void CAlnMultiDSBuilder::GetBioseqHandles(vector<CBioseq_Handle>& handles)
{
    if(m_AlnStats)    {
        ITERATE (TAlnStats::TIdVec, it, m_AlnStats->GetIdVec()) {
            const CSeq_id& sid = (*it)->GetSeqId();
            const CBioseq_Handle h = m_Scope->GetBioseqHandle(sid);
            handles.push_back(h);
        }
    }
}


void CAlnMultiDSBuilder::x_PreCreateSparseDataSource()
{
    m_AlnStats.Reset();
    m_AnchoredAlns.clear();

    if(! m_OrigAligns.empty())    {
        TIdExtract extractor;
        TAlnIdMap aln_id_map(extractor, m_OrigAligns.size());
        ITERATE (TAlignVector, it, m_OrigAligns) {
            aln_id_map.push_back(**it);
        }

        /// Crete align statistics object
        m_AlnStats.Reset(new TAlnStats(aln_id_map));
        //TAlnStats aln_stats(aln_vector, ,
        //                    m_SeqIdAlnBitmap->GetAnchorRows(),
        //                    m_SeqIdAlnBitmap->GetBaseWidths());

        /// Construct a vector of anchored alignments
        CreateAnchoredAlnVec(*m_AlnStats, m_AnchoredAlns, m_Options);

        //TODO use aln_stats to init m_Options
    }
}


CRef<IAlnMultiDataSource>
    CAlnMultiDSBuilder::x_CreateSparseDataSource()
{
    CStopWatch sw;
    sw.Start();

    CRef<IAlnMultiDataSource> ds;
    if(m_MasterId.GetPointer()  &&  ! m_OrigAligns.empty())    {
        _TRACE("Creating CSparseMultiDataSource");
        CSparseMultiDataSource* sp_ds = new CSparseMultiDataSource(*m_Scope);
        if (m_Listener) sp_ds->SetListener(m_Listener);
        ds.Reset(sp_ds);
        sp_ds->Init(m_AnchoredAlns, m_Options, m_SyncCreate);
    }
    _TRACE( 1000 * sw.Elapsed() << " ms" );
    return ds;
}


CRef<IAlnMultiDataSource>
    CAlnMultiDSBuilder::x_CreateAlnVecDataSource()
{
    CRef<CAlnVecMultiDataSource> ds(new CAlnVecMultiDataSource(*m_Scope));
    if (m_Listener) ds->SetListener(m_Listener);
    ds->Init(m_OrigAligns, m_SyncCreate, m_SelectAnchor);
    return CRef<IAlnMultiDataSource>(ds.GetPointer());
}


/// Analyzes m_OrigAligns and decides how to build an alignment from it
void CAlnMultiDSBuilder::x_TestAlignments()
{
    TAlignVector good_aligns;
    x_GetLinearAlignments(good_aligns);

    // build alignment map
    typedef set<const CSeq_align*> TAlignSet;
    typedef map<CSeq_id_Handle, TAlignSet> TIDToAligns;
    TIDToAligns align_map;

    ITERATE(TAlignVector, it_al, good_aligns) {
        const CSeq_align& al = **it_al;

        for ( CTypeConstIterator<CSeq_id> it_id(al); it_id;  ++it_id) {
            CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*it_id);

            TIDToAligns::const_iterator it = align_map.find(idh);
            if(it == align_map.end())   {
                it = align_map.insert(TIDToAligns::value_type(idh, TAlignSet())).first;
            }
            TAlignSet& aln_set = const_cast<TAlignSet&>(it->second);
            aln_set.insert(&al);
        }
    }

    /// select the ID that exist in max number of alignments
    size_t max_al = 0; /// max number of alignments for a sinegle ID
    CSeq_id_Handle max_h;
    ITERATE(TIDToAligns, it_map, align_map) {
        const TAlignSet& aln_set = it_map->second;
        if(aln_set.size() > max_al) {
            max_al = aln_set.size();
            max_h = it_map->first;
        }
    }

    if(max_h)   {
        m_MasterId = max_h.GetSeqId();
    }
    string s_id = m_MasterId ? m_MasterId->GetSeqIdString() : "NULL";
    _TRACE("Master ID " << s_id);
}


// selects alignments that have the same length on all sequences
void CAlnMultiDSBuilder::x_GetLinearAlignments(TAlignVector& aligns)
{
    typedef CSeq_align::C_Segs  TSegs;
    aligns.reserve(m_OrigAligns.size());

    // test every CSeq-aling
    ITERATE(TAlignVector, it, m_OrigAligns) {
        const CSeq_align& align = **it;
        const TSegs& segs = align.GetSegs();
        bool linear = true;

        switch(segs.Which()) {
        case TSegs::e_Denseg:
        case TSegs::e_Dendiag:
            break;
        case TSegs::e_Std: {
            ITERATE(TSegs::TStd, it_s, segs.GetStd())   {
                const CStd_seg& std_seg = **it_s;
                if(! x_IsLinear(std_seg))   {
                    linear = false;
                    break;
                }
            }
            break;
        }
        case TSegs::e_Sparse:
        case TSegs::e_Spliced:
            break;
        default:
            linear = false; // other types currently not supported
            break;
        }
        if(linear)  {
            aligns.push_back(*it);
        }
    }
}

/// returns true if CStd_seg is linear
bool CAlnMultiDSBuilder::x_IsLinear(const CStd_seg& /*seg*/)
{
    return false;
}


END_NCBI_SCOPE
