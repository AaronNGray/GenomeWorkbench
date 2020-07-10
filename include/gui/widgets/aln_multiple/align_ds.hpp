#ifndef GUI_WIDGETS_ALN_MULTIPLE___ALIGN_DS__HPP
#define GUI_WIDGETS_ALN_MULTIPLE___ALIGN_DS__HPP

/*  $Id: align_ds.hpp 25706 2012-04-25 00:21:04Z voronov $
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

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <objtools/alnmgr/alnvec.hpp>

#include <objmgr/bioseq_handle.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CAlign_CI;
    class CAlnMix;
    class CDense_seg;
    class CSeq_annot;
    class CSeq_align;
    class CBioseq_Handle;
END_SCOPE(objects)

// class CAlignDataSource defines the API required of a data source object
// for CAlnVec-based alignment widgets.
//
// This is a bridge interface to the alignment manager and to Genome
// Workbench's data storage mechanism.
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CAlignDataSource : public CObject
{
public:
    CAlignDataSource();

    // create a data source around an existing alignment manager
    virtual void    Init(objects::CAlnVec& vec);

    // create a new data source around an alignment iterator
    virtual void    Init(objects::CAlign_CI& iter, objects::CScope& scope);

    // create a new data source around a seq-align
    virtual void    Init(const objects::CSeq_align& align, objects::CScope& scope);

    // create a new data source around a seq-annot
    virtual void    Init(const objects::CSeq_annot& annot, objects::CScope& scope);

    // create a new data source around a dense-seg
    virtual void    Init(const objects::CDense_seg& seg, objects::CScope& scope);

    virtual void    Init(const objects::CBioseq_Handle& handle, objects::CScope& scope);

    objects::CAlnVec&       SetAlnMgr(void);
    const objects::CAlnVec& GetAlnMgr(void) const;

protected:
    CRef<objects::CAlnVec> m_AlnMgr; // the alignment

    int m_ConsRowIndex;

protected:
    virtual void x_Init(objects::CAlnMix& mix);
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_MULTIPLE___ALIGN_DS__HPP
