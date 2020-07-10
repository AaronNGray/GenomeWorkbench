/*  $Id: cmd_add_seq_annot.cpp 32125 2015-01-05 20:16:18Z asztalos $
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
 * Authors:  Andrea Asztalos, based on a file by Igor Filippov
 */

#include <ncbi_pch.hpp>
#include <gui/objutils/cmd_add_seq_annot.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CCmdCreateSeq_annot::Execute()
{
    if (m_ParentHandle) {
        CSeq_entry_EditHandle eh = m_ParentHandle.GetEditHandle();
        m_AEH = eh.AttachAnnot(*m_Annot);
    }
}

void CCmdCreateSeq_annot::Unexecute()
{
    if (m_AEH) {
        m_AEH.Remove();
    }
}

string CCmdCreateSeq_annot::GetLabel()
{
    return "Create annotation";
}

END_NCBI_SCOPE
