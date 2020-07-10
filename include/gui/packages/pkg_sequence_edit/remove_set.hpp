/*  $Id: remove_set.hpp 32893 2015-05-04 16:30:38Z filippov $
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
#ifndef _REMOVE_SET_H_
#define _REMOVE_SET_H_

#include <corelib/ncbistd.hpp>
#include <gui/framework/workbench_impl.hpp>
#include <gui/objutils/cmd_change_bioseq_set.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

class CRemoveSet
{    
public:
    static void remove_set(IWorkbench* wb);
    static void remove_sets_from_set(IWorkbench* wb);
private:
    static void MoveDescrToEntries(CBioseq_set& se);
    static TConstScopedObjects GetSelectedObjects(IWorkbench* wb);
    static ICommandProccessor* GetCmdProcessor(IWorkbench* wb, CScope& scope);
    static void RemoveOneSet(const CBioseq_set* bioseqset, CScope& scope, CRef<CBioseq_set> &new_set);
};

END_NCBI_SCOPE

#endif
    // _REMOVE_SET_H_
