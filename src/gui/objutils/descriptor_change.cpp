/*  $Id: descriptor_change.cpp 42175 2019-01-08 21:15:49Z asztalos $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors: Mike DiCuccio, Roman Katargin, Colleen Bollin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <objmgr/seqdesc_ci.hpp>

#include <gui/objutils/objects.hpp>
#include <gui/objutils/descriptor_change.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

// CCmdChangeSeqdesc
void CCmdChangeSeqdesc::Execute()
{
    if (!m_Seh)
        return;

    CSeq_entry_EditHandle eseh = m_Seh.GetEditHandle();
    if (CSeqdesc* desc = const_cast<CSeqdesc*>(m_NewDesc.GetPointer())) {
        eseh.ReplaceSeqdesc(*m_OldDesc, *desc);
        swap(m_OldDesc, m_NewDesc);
    }
}

void CCmdChangeSeqdesc::Unexecute()
{
    Execute();
}

string CCmdChangeSeqdesc::GetLabel()
{
    return "Change descriptor";
}


END_NCBI_SCOPE
