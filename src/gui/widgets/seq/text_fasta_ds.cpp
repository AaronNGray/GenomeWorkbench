/*  $Id: text_fasta_ds.cpp 14565 2007-05-18 12:32:01Z dicuccio $
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
#include <gui/widgets/seq/text_fasta_ds.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CFastaTextDS::CFastaTextDS(const CBioseq_Handle& handle,
                           const CSeq_loc& loc)
    : m_Handle(handle)
    , m_Loc(&loc)
{
}


const CSeq_loc& CFastaTextDS::GetLocation() const
{
    return *m_Loc;
}


const CBioseq_Handle& CFastaTextDS::GetBioseqHandle() const
{
    return m_Handle;
}

END_NCBI_SCOPE
