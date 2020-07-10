/*  $Id: cmd_change_seq_entry.cpp 39430 2017-09-22 14:55:36Z asztalos $
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
 * Authors:  Igor Filippov, based on work by Colleen Bollin and Roman Katargin
 */


#include <ncbi_pch.hpp>
#include <gui/objutils/cmd_change_seq_entry.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CCmdChangeSeqEntry::CCmdChangeSeqEntry(const CSeq_entry_Handle& seh, CRef<CSeq_entry> new_entry)
    : m_NewEntry(new_entry)
{
    m_SEH = seh.GetEditHandle();

    _ASSERT(m_SEH.Which() != CSeq_entry::e_not_set);
    _ASSERT(m_NewEntry->Which() != CSeq_entry::e_not_set);

    // save the original bseq/bsset edit handle
    if (m_SEH.IsSeq()) {
        m_SaveBsh = m_SEH.SetSeq();
    }
    else {
        m_SaveBssh = m_SEH.SetSet();
    }
}

void CCmdChangeSeqEntry::Execute()
{
    m_SEH.SelectNone();

    if (m_NewEntry->IsSeq()) {
        m_SEH.SelectSeq(m_NewEntry->SetSeq());
    }
    else {
        m_SEH.SelectSet(m_NewEntry->SetSet());
    }
}

void CCmdChangeSeqEntry::Unexecute()
{
    m_SEH.SelectNone();

    if (m_SaveBsh.IsRemoved()) {
        m_SEH.SelectSeq(m_SaveBsh);
    }
    else if (m_SaveBssh.IsRemoved()) {
        m_SEH.SelectSet(m_SaveBssh);
    }

    _ASSERT(m_SEH.Which() != CSeq_entry::e_not_set);
}

string CCmdChangeSeqEntry::GetLabel()
{
    return "Modify Seq-entry";
}


END_NCBI_SCOPE
