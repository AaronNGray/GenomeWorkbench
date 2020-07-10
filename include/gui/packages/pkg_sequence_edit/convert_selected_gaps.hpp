/*  $Id: convert_selected_gaps.hpp 42755 2019-04-10 16:32:15Z filippov $
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
 * Authors:  Igor Filippov
 */
#ifndef _CONVERT_SELECTED_GAPS_H_
#define _CONVERT_SELECTED_GAPS_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objects/general/Object_id.hpp>

BEGIN_NCBI_SCOPE


class CConvertSelectedGaps
{
public:
    static CRef<CCmdComposite> apply(const TConstScopedObjects &objects, bool to_known, bool adjust_features = false);
private:
static CRef<CCmdComposite> apply_impl(const TConstScopedObjects &objects, bool to_known, bool adjust_features, map<objects::CObject_id::TId, objects::CObject_id::TId> &old_to_new, bool create_xref_map);
};

class CConvertGapsBySize
{
public:
    static CRef<CCmdComposite> apply(objects::CSeq_entry_Handle seh);
};


class CEditSelectedGaps
{
public:
    static CRef<CCmdComposite> apply(const TConstScopedObjects &objects);
private:
static CRef<CCmdComposite> apply_impl(const TConstScopedObjects &objects, map<objects::CObject_id::TId, objects::CObject_id::TId> &old_to_new, bool create_xref_map);
};

class CCombineAdjacentGaps
{
public:
    static CRef<CCmdComposite> apply(objects::CSeq_entry_Handle seh);
private:
static CRef<CCmdComposite> apply_impl(objects::CSeq_entry_Handle seh, map<objects::CObject_id::TId, objects::CObject_id::TId> &old_to_new, bool create_xref_map);
};


END_NCBI_SCOPE

#endif
    // _CONVERT_SELECTED_GAPS_H_
