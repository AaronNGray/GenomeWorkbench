/*  $Id: cmd_create_desc.cpp 44316 2019-11-27 16:08:06Z filippov $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>

#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>

#include <gui/objutils/cmd_create_desc.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CCmdCreateDesc::Execute()
{
    if (m_Failure || !m_seh)
        return;

    m_TopSeqEntry = m_seh.GetTopLevelEntry();
    if (m_Desc->IsSource()) {
        CSeqdesc_CI desc_it(m_seh, CSeqdesc::e_Source);
        if (desc_it) {
            LOG_POST(Error << "CCmdCreateDesc: BioSource descriptor already set");
            m_Failure = true;
            return;
        }
    }

    CSeq_entry_EditHandle eseh = m_seh.GetEditHandle();
    eseh.AddSeqdesc(const_cast<CSeqdesc&>(*m_Desc));
}

void CCmdCreateDesc::Unexecute()
{
    if (m_Failure || !m_TopSeqEntry)
        return;
    if (!m_seh)
    {
        for ( CSeq_entry_CI entry_it(m_TopSeqEntry, CSeq_entry_CI::fRecursive|CSeq_entry_CI::fIncludeGivenEntry); entry_it; ++entry_it )
            for ( CSeqdesc_CI desc_it(*entry_it, m_Desc->Which(), 1); desc_it; ++desc_it)
                if (&*desc_it == m_Desc.GetPointer())
                    m_seh = desc_it.GetSeq_entry_Handle();
        
    }
    if (!m_seh)
        return;
    CSeq_entry_EditHandle eseh = m_seh.GetEditHandle();
    eseh.RemoveSeqdesc(*m_Desc);
}

string CCmdCreateDesc::GetLabel()
{
    return "Create Descriptor";
}

END_NCBI_SCOPE
