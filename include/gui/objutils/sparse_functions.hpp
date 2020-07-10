#ifndef GUI_OBJUTILS___SPARSE_FUNCTIONS__HPP
#define GUI_OBJUTILS___SPARSE_FUNCTIONS__HPP

/*  $Id: sparse_functions.hpp 25706 2012-04-25 00:21:04Z voronov $
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
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/objutils/sparse_alignment.hpp>

#include <objects/seqalign/Dense_seg.hpp> // TODO - remove
#include <objtools/alnmgr/alnvec.hpp>
#include <serial/iterator.hpp>


BEGIN_SCOPE(objects)
    class CSeq_id;
    class CSeq_align;
END_SCOPE(objects)


BEGIN_NCBI_SCOPE

/// This is the place where elements of the new Alignment Library will be collected

///////////////////////////////////////////////////////////////////////////////
/// Extractors

/// Generic Extractor
template<class T>
    void    ExtractSeqAligns(const T& obj,
                         vector< CConstRef<objects::CSeq_align> >& aligns)
{
    CTypeConstIterator<objects::CSeq_align> it(obj);
    while(it)   {
        aligns.push_back(CConstRef<objects::CSeq_align>(&*it));
        ++it;
    }
}


///////////////////////////////////////////////////////////////////////////////
/// Converters

/// Converter CSeq_align -> SAlignedSeq-s
NCBI_GUIOBJUTILS_EXPORT
    bool ConvertToPairwise(const objects::CSeq_align& align,
                           const objects::CSeq_id& master_id,
                           vector<SAlignedSeq*>& aln_seqs);

/// Converter vector of CSeq_align -> SAlignedSeq-s
NCBI_GUIOBJUTILS_EXPORT
    bool ConvertToPairwise(const vector< CConstRef<objects::CSeq_align> >& aligns,
                           const objects::CSeq_id& master_id,
                           vector<SAlignedSeq*>& aln_seqs);

/// Converter CSparse_seg -> vector<SAlignedSeq*>
NCBI_GUIOBJUTILS_EXPORT
    bool ConvertToPairwise(const objects::CSparse_seg& sparse_seg,
                           vector<SAlignedSeq*>& aln_seqs);

/// TODO not Sparse - move to a different file
NCBI_GUIOBJUTILS_EXPORT
    bool ConvertToPairwise(const objects::CDense_seg& dense_seg,
                           objects::CDense_seg::TDim anchor_row,
                           vector<SAlignedSeq*>& aln_seqs);

///////////////////////////////////////////////////////////////////////////////
/// Helper functions used by Converters
NCBI_GUIOBJUTILS_EXPORT
    SAlignedSeq* CreateAlignRow(const objects::CSparse_align& align,
                                bool master_first);

/// Converter CSparse_align ->
/// TODO redesign API
NCBI_GUIOBJUTILS_EXPORT
    void GetAlignColl(const objects::CSparse_align& sparse_align,
                      const objects::CSeq_id& master_id,
                      SAlignTools::TAlignColl& coll);

/// TODO not Sparse - move to a different file
NCBI_GUIOBJUTILS_EXPORT
    SAlignedSeq*  CreateAlignRow(const objects::CDense_seg& dense_seg,
                                 objects::CDense_seg::TDim row_1,
                                 objects::CDense_seg::TDim row_2);

///////////////////////////////////////////////////////////////////////////////
/// Builder functions

/// Build a Sparse Alignment
NCBI_GUIOBJUTILS_EXPORT CSparseAlignment*
    BuildSparseAlignment(const objects::CSeq_id& master_id,
                         vector<SAlignedSeq*>& aln_seqs,
                         objects::CScope& scope);


/// Build a Dense Alignment
NCBI_GUIOBJUTILS_EXPORT objects::CAlnVec*
    BuildDenseAlignment(const objects::CSeq_id& master_id,
                        vector<SAlignedSeq*>& aln_seqs,
                        objects::CScope& scope);

///////////////////////////////////////////////////////////////////////////////
/// Reverse Converters

NCBI_GUIOBJUTILS_EXPORT
    CRef<objects::CSparse_align>
        CreateSparseAlign(const objects::CSeq_id& id_1,
                          const objects::CSeq_id& id_2,
                          const SAlignTools::TAlignColl& coll);

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___SPARSE_FUNCTIONS__HPP
