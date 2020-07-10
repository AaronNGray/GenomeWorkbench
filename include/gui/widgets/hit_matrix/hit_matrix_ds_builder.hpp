#ifndef GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_DS_BUILDER__HPP
#define GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_DS_BUILDER__HPP

/*  $Id: hit_matrix_ds_builder.hpp 17963 2008-09-30 22:27:56Z yazhuk $
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

#include <gui/widgets/hit_matrix/hit_matrix_ds.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Std_seg.hpp>


BEGIN_SCOPE(objects)
    class CScope;
    class CSeq_annot;
    class CSeq_align;
    class CBioseq_handle;
END_SCOPE(objects)

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CHitMatrixDSBuilder
class NCBI_GUIWIDGETS_HIT_MATRIX_EXPORT CHitMatrixDSBuilder
{
public:
    typedef vector< CConstRef<objects::CSeq_annot> >  TAnnotVector;
    typedef vector< CConstRef<objects::CSeq_align> >  TAlignVector;

    CHitMatrixDSBuilder();
    virtual ~CHitMatrixDSBuilder();

    /// initial data set from which an alignment will be build
    void    Init(objects::CScope& scope, const objects::CSeq_align& align);
    void    Init(objects::CScope& scope, const objects::CSeq_annot& annot);
    void    Init(objects::CScope& scope, const objects::CBioseq_Handle& handle);
    void    Init(objects::CScope& scope, const objects::CBioseq& bioseq);

    void    Init(objects::CScope& scope, TAlignVector& aligns);
    void    Init(objects::CScope& scope, TAnnotVector& annots);

    CIRef<IHitMatrixDataSource>   CreateDataSource();

protected:
    void    x_Clear();
    void    x_TestAlignments();

    CIRef<IHitMatrixDataSource>   x_CreateDataSource();

protected:
    CRef<objects::CScope>   m_Scope;
    TAlignVector   m_OrigAligns;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_DS_BUILDER__HPP
