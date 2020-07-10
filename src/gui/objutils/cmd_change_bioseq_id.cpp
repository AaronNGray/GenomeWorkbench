/*  $Id: cmd_change_bioseq_id.cpp 36304 2016-09-09 20:15:36Z filippov $
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


#include <ncbi_pch.hpp>
#include <gui/objutils/cmd_change_bioseq_id.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CCmdChangeBioseqId::CCmdChangeBioseqId(const CBioseq_Handle& bh, const CSeq_id& old_id, const CSeq_id& new_id)
    : m_BH(bh)
{ 
    m_old_idh = CSeq_id_Handle::GetHandle(old_id);
    m_new_idh = CSeq_id_Handle::GetHandle(new_id);
}

void CCmdChangeBioseqId::Execute()
{
    CBioseq_EditHandle eh = m_BH.GetEditHandle();
    eh.AddId(m_new_idh);
    eh.RemoveId(m_old_idh);
    swap(m_old_idh, m_new_idh);
}

void CCmdChangeBioseqId::Unexecute()
{
    Execute();
}

string CCmdChangeBioseqId::GetLabel()
{
    return "Modify Bioseq Id";
}

END_NCBI_SCOPE
