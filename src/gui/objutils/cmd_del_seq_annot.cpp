/*  $Id: cmd_del_seq_annot.cpp 31464 2014-10-10 17:12:03Z filippov $
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

#include <gui/objutils/cmd_del_seq_annot.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CCmdDelSeq_annot::Execute()
{
    if (m_AEH && !m_AEH.IsRemoved())
    {
        m_Parent_handle =  m_AEH.GetParentEntry();
        m_AEH.Remove();
    }
}

void CCmdDelSeq_annot::Unexecute()
{
    if (m_Parent_handle)
    {
        CSeq_entry_EditHandle seh(m_Parent_handle);
        seh.AttachAnnot(m_AEH);
    }
}

string CCmdDelSeq_annot::GetLabel()
{
    return "Delete annotation";
}

END_NCBI_SCOPE
