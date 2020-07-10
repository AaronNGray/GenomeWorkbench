/*  $Id: align_ds.cpp 43632 2019-08-09 20:17:37Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/aln_multiple/align_ds.hpp>
#include <gui/objutils/utils.hpp>

#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seq/Seq_annot.hpp>

#include <objtools/alnmgr/alnmix.hpp>

#include <objmgr/align_ci.hpp>

#include <serial/serial.hpp>
#include <serial/objostr.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


//
// CAlignDataSourceException is a trivial exception class for trapping
// exceptions during initialization.
//

class CAlignDataSourceException : EXCEPTION_VIRTUAL_BASE public CException
{
public:
    // Enumerated list of document management errors
    enum EErrCode {
        eInvalidAnnot
    };

    // Translate the specific error code into a string representations of
    // that error code.
    virtual const char* GetErrCodeString(void) const override
    {
        switch (GetErrCode()) {
        case eInvalidAnnot:         return "eInvalidAnnot";
        default:                    return CException::GetErrCodeString();
        }
    }

    NCBI_EXCEPTION_DEFAULT(CAlignDataSourceException, CException);
};


CAlignDataSource::CAlignDataSource()
: m_ConsRowIndex(-1)
{
}


void    CAlignDataSource::Init(CAlnVec& mgr)
{
    m_ConsRowIndex = -1;
    //x_ClearHandles();

    m_AlnMgr = &mgr;

    //x_CreateHandles();
}


void CAlignDataSource::Init(CAlign_CI& iter, CScope& scope)
{
    CAlnMix mix(scope);
    for ( ;  iter;  ++iter) {
        mix.Add(*iter);
    }
    x_Init(mix);
}


void CAlignDataSource::Init(const CSeq_align& align, CScope& scope)
{
    CAlnMix mix(scope);
    mix.Add(align);
    x_Init(mix);
}


void CAlignDataSource::Init(const CSeq_annot& annot, CScope& scope)
{
    if ( !annot.GetData().IsAlign() ) {
        NCBI_THROW(CAlignDataSourceException, eInvalidAnnot,
                   "Annotation is not an alignment");
    }

    CAlnMix mix(scope);
    ITERATE (CSeq_annot::TData::TAlign, iter, annot.GetData().GetAlign()) {
        mix.Add(**iter);
    }
    x_Init(mix);
}


void CAlignDataSource::Init(const CDense_seg& seg, CScope& scope)
{
    CAlnMix mix(scope);
    mix.Add(seg);
    x_Init(mix);
}


void CAlignDataSource::Init(const CBioseq_Handle& handle, CScope& scope)
{
    // iterate all alignments on this bioseq handle
    SAnnotSelector sel =
        CSeqUtils::GetAnnotSelector(CSeq_annot::TData::e_Align);
    CAlign_CI iter(handle, sel);

    CAlnMix mix(scope);
    for ( ;  iter;  ++iter) {
        mix.Add(*iter);
    }
    x_Init(mix);
}


void CAlignDataSource::x_Init(CAlnMix& mix)
{
    //m_ConsRowIndex = -1;
    //x_ClearHandles();

    try {
        mix.Merge(CAlnMix::fGapJoin);
        m_AlnMgr.Reset(new CAlnVec(mix.GetDenseg(), mix.GetScope()));
        m_AlnMgr->SetGapChar('-');
    }
    catch (CException& e) {
        LOG_POST(Error << "CAlignDataSource::x_Init(): caught exception: "
                 << e.what());
        throw;
    }
    //x_CreateHandles();
}



CAlnVec& CAlignDataSource::SetAlnMgr(void)
{
    return *m_AlnMgr;
}


const CAlnVec& CAlignDataSource::GetAlnMgr(void) const
{
    return *m_AlnMgr;
}

END_NCBI_SCOPE
