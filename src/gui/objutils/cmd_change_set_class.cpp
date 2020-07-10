/*  $Id: cmd_change_set_class.cpp 31595 2014-10-27 14:51:04Z bollin $
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
 * Authors:  Colleen Bollin and Roman Katargin
 */


#include <ncbi_pch.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/seq_annot_handle.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <gui/objutils/cmd_change_set_class.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CCmdChangeSetClass::CCmdChangeSetClass(const CBioseq_set_Handle& bh, CBioseq_set::EClass new_class)
    : m_BH(bh), m_NewClass(new_class)
{ 
    m_OrigClass = m_BH.IsSetClass() ? m_BH.GetClass() : CBioseq_set::eClass_not_set;
}

void CCmdChangeSetClass::Execute()
{
    CBioseq_set_EditHandle eh(m_BH);
    eh.SetClass(m_NewClass);  

}

void CCmdChangeSetClass::Unexecute()
{
    CBioseq_set_EditHandle eh(m_BH);
    eh.SetClass(m_OrigClass);  
}

string CCmdChangeSetClass::GetLabel()
{
    return "Modify Bioseq_set class";
}



END_NCBI_SCOPE
