/*  $Id: text_object_description.hpp 575222 2018-11-26 17:50:01Z kachalos $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Sema Kachalo
 */

#ifndef _EDIT_RNA_EDIT__HPP_
#define _EDIT_RNA_EDIT__HPP_

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <objmgr/scope.hpp>
#include <objects/seqfeat/Seq_feat.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
BEGIN_SCOPE(edit)

NCBI_XOBJEDIT_EXPORT string GetTextObjectDescription(const CSeq_feat& sf, CScope& scope);
NCBI_XOBJEDIT_EXPORT string GetTextObjectDescription(const CSeqdesc& sd, CScope& scope);
NCBI_XOBJEDIT_EXPORT string GetTextObjectDescription(const CBioseq& bs, CScope& scope);
NCBI_XOBJEDIT_EXPORT string GetTextObjectDescription(const CBioseq_set& bs, CScope& scope);
// Giving the same name to all functions was a creative idea!
NCBI_XOBJEDIT_EXPORT void GetTextObjectDescription(const CSeq_feat& seq_feat, CScope& scope, string &type, string &context, string &location, string &locus_tag);

END_SCOPE(edit)
END_SCOPE(objects)
END_NCBI_SCOPE

#endif
        // _EDIT_RNA_EDIT__HPP_
