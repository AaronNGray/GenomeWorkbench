/*  $Id: cmd_del_bioseq_set.cpp 27217 2013-01-14 16:07:23Z choi $
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
 * Authors:  Roman Katargin, Igor Filippov
 */


#include <ncbi_pch.hpp>

#include <gui/objutils/cmd_del_bioseq_set.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CCmdDelBioseqSet::CCmdDelBioseqSet(CBioseq_set_Handle& bssh)
    : m_BSSH(bssh)
{
}

void CCmdDelBioseqSet::Execute()  
{
    // Does this set have a parent set?
    m_ParentBSSH_OrNull = m_BSSH.GetParentBioseq_set();

    // Seqentry wrapper around current set
    m_SEH = m_BSSH.GetParentEntry();

    // If parent set exists, note the position of the current set within 
    // the parent set, so we can later UNDO this action by re-inserting
    // the deleted set into its correct position
    if (m_ParentBSSH_OrNull) {
        m_index = m_ParentBSSH_OrNull.GetSeq_entry_Index(m_SEH);   
    }
    else {
        m_index = -1;
    }

    // Now prepare to delete the current set
    m_Set.Reset();
    m_Set = m_BSSH.GetCompleteBioseq_set();
    CBioseq_set_EditHandle eh(m_BSSH);
    eh.Remove();
}

void CCmdDelBioseqSet::Unexecute() 
{
    if (m_ParentBSSH_OrNull) {
        // If parent set existed, reattach the deleted set into its
        // correct position within the parent set.
        CSeq_entry_EditHandle eh = 
            m_ParentBSSH_OrNull.GetEditHandle().AddNewEntry(m_index);
	m_BSSH = eh.SelectSet(const_cast<CBioseq_set&>(*m_Set));
	m_Set.Reset();
    }
    else {
	CSeq_entry_EditHandle eh(m_SEH);
	m_BSSH = eh.SelectSet(const_cast<CBioseq_set&>(*m_Set));
	m_Set.Reset();
    }
}

string CCmdDelBioseqSet::GetLabel()
{
    return "Delete Bioseq_set";
}

END_NCBI_SCOPE
