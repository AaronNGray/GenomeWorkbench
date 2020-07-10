/*  $Id: hit_matrix_ds_builder.cpp 25706 2012-04-25 00:21:04Z voronov $
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

#include <gui/widgets/hit_matrix/hit_matrix_ds_builder.hpp>

#include <gui/objutils/sparse_functions.hpp>

#include <gui/widgets/hit_matrix/dense_ds.hpp>

#include <gui/objutils/utils.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seq/seq_id_handle.hpp>

#include <corelib/ncbitime.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/bioseq_handle.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

// TODO factor out common code and move to the base class
CHitMatrixDSBuilder::CHitMatrixDSBuilder()
{
}


CHitMatrixDSBuilder::~CHitMatrixDSBuilder()
{
    x_Clear();
}


void CHitMatrixDSBuilder::Init(objects::CScope& scope, const objects::CSeq_align& align)
{
    x_Clear();

    m_Scope.Reset(&scope);
    m_OrigAligns.push_back(CConstRef<CSeq_align>(&align));
}


void CHitMatrixDSBuilder::Init(objects::CScope& scope, const objects::CSeq_annot& annot)
{
    x_Clear();

    m_Scope.Reset(&scope);
    ExtractSeqAligns(annot, m_OrigAligns);
}


void CHitMatrixDSBuilder::Init(objects::CScope& scope, const objects::CBioseq& bioseq)
{
    x_Clear();

    m_Scope.Reset(&scope);
    ExtractSeqAligns(bioseq, m_OrigAligns);
}


void CHitMatrixDSBuilder::Init(objects::CScope& scope, const objects::CBioseq_Handle& handle)
{
    x_Clear();

    m_Scope.Reset(&scope);

    SAnnotSelector sel = CSeqUtils::GetAnnotSelector(CSeq_annot::TData::e_Align);
    CAlign_CI it(handle, sel);
    for ( ;  it;  ++it) {
        m_OrigAligns.push_back(CConstRef<CSeq_align>(&*it));
    }
}


void CHitMatrixDSBuilder::Init(objects::CScope& scope, TAlignVector& aligns)
{
    x_Clear();

    m_Scope.Reset(&scope);
    m_OrigAligns = aligns;
}


/// initial data set from which an alignment will be build
void CHitMatrixDSBuilder::Init(CScope& scope, TAnnotVector& annots)
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


void CHitMatrixDSBuilder::x_Clear()
{
    m_Scope.Reset();
    m_OrigAligns.clear();
}


CIRef<IHitMatrixDataSource>
    CHitMatrixDSBuilder::CreateDataSource()
{
    //LOG_POST(Info << "CHitMatrixDSBuilder::CreateDataSource() "
    //         << m_OrigAligns.size() << " m_OrigAligns");

    CIRef<IHitMatrixDataSource> ds = x_CreateDataSource();
    if(ds)  {
        IHitMatrixDataSource::SParams params;
        params.m_RowBased = false;
        params.m_Direction = CAlnUserOptions::eBothDirections;

        ds->SetParams(params, false);
    }
    return ds;
}


CIRef<IHitMatrixDataSource>
    CHitMatrixDSBuilder::x_CreateDataSource()
{
    //LOG_POST(Info << "CHitMatrixDSBuilder::x_CreateSparseDataSource()");
    //CStopWatch sw;
    //sw.Start();

    CIRef<IHitMatrixDataSource> ds(new CHitMatrixDataSource());
    ds->Init(*m_Scope, m_OrigAligns);

    //LOG_POST(Info << "CHitMatrixDSBuilder::x_CreateSparseDataSource() " << 1000 * sw.Elapsed() << "ms");
    return ds;
}


END_NCBI_SCOPE
