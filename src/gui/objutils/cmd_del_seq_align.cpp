/*  $Id: cmd_del_seq_align.cpp 37191 2016-12-13 21:25:31Z asztalos $
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

#include <gui/objutils/cmd_del_seq_align.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CCmdDelSeq_align::Execute()
{
    CSeq_annot_Handle ah = m_Orig_align.GetAnnot();
    m_AnnotHandle = ah.GetEditHandle();
    m_ParentHandle = m_AnnotHandle.GetParentEntry();

    m_Saved_align.Reset();
    m_Saved_align = m_Orig_align.GetSeq_align();
    m_Orig_align.Remove();

    // remove the empty seq-annot
    auto& align_list = m_AnnotHandle.GetSeq_annotCore()->GetData().GetAlign();
    if (align_list.empty()) {
        m_AnnotHandle.Remove();
    }
}

void CCmdDelSeq_align::Unexecute()
{
    if (!m_Orig_align.IsRemoved()) {
        return;
    }

    if (m_Saved_align) {
        if (!m_AnnotHandle.IsRemoved()) {
            m_Orig_align = m_AnnotHandle.AddAlign(*m_Saved_align);
        }
        else if (m_ParentHandle) {
            CSeq_annot_EditHandle aeh = m_ParentHandle.AttachAnnot(m_AnnotHandle);
            m_Orig_align = aeh.AddAlign(*m_Saved_align);
        }

        m_Saved_align.Reset();
    }
}

string CCmdDelSeq_align::GetLabel()
{
    return "Delete alignment";
}

END_NCBI_SCOPE
