/* $Id: seq_grid_table_navigator.hpp 38175 2017-04-06 14:52:27Z filippov $
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

#ifndef _SEQ_GRID_TABLE_NAV_HPP
#define _SEQ_GRID_TABLE_NAV_HPP

#include <corelib/ncbistd.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <gui/framework/workbench.hpp>
#include <gui/objutils/objects.hpp>

BEGIN_NCBI_SCOPE

class CSeqGridTableNav
{
public:
    CSeqGridTableNav() : m_Workbench(NULL) {}
    virtual ~CSeqGridTableNav() {}
    void NavigateToTextViewPosition(TConstScopedObjects &objects, objects::CBioseq_Handle bsh, const CObject *obj, IWorkbench* workbench);
    void JumpToTextView(int row);
    virtual const CObject*  RowToScopedObjects(int row, TConstScopedObjects &objects, objects::CBioseq_Handle &bsh) = 0;
protected:
    IWorkbench* m_Workbench;
    CConstRef<objects::CSeq_submit> m_SeqSubmit;
};

END_NCBI_SCOPE

#endif
