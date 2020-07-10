/*  $Id: cmd_create_align.cpp 32605 2015-03-31 18:50:15Z filippov $
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
 * Authors:  Igor Filippov based on work by Roman Katargin
 */


#include <ncbi_pch.hpp>

#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/seq_annot_ci.hpp>

#include <gui/objutils/cmd_create_align.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CCmdCreateAlign::Execute()
{
    CSeq_annot_Handle annot;

    CSeq_annot_CI annot_ci(m_seh, CSeq_annot_CI::eSearch_entry);
    for (; annot_ci; ++annot_ci) {
        if ((*annot_ci).IsAlign()) {
            annot = *annot_ci;
            break;
        }
    }

    CSeq_entry_EditHandle eh = m_seh.GetEditHandle();

    if (!annot) {
        CRef<CSeq_annot> new_annot(new CSeq_annot());
        annot = m_AnnotCreated = eh.AttachAnnot(*new_annot);
    }

    CSeq_annot_EditHandle aeh(annot);
    m_ah = aeh.AddAlign(*m_Align);
}

void CCmdCreateAlign::Unexecute()
{
    m_ah.Remove();

    if (m_AnnotCreated) {
        m_AnnotCreated.Remove();
    }
}

string CCmdCreateAlign::GetLabel()
{
    return "Create Alignment";
}

END_NCBI_SCOPE
