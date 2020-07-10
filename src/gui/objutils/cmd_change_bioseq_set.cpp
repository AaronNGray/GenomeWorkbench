/*  $Id: cmd_change_bioseq_set.cpp 32002 2014-12-10 14:19:49Z asztalos $
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
#include <gui/objutils/cmd_change_bioseq_set.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CCmdChangeBioseqSet::CCmdChangeBioseqSet(const CBioseq_set_Handle& bh, const CBioseq_set& new_set)
    : m_BH(bh)
{ 
    m_Set.Reset(new CBioseq_set());
    m_Set->Assign(new_set); 
}

void CCmdChangeBioseqSet::Execute()
{
    CRef<CBioseq_set> saveSet(new CBioseq_set());
    saveSet->Assign(*m_BH.GetCompleteBioseq_set());

    CBioseq_set_EditHandle eh(m_BH);

    if (m_Set->CanGetId())   eh.SetId(m_Set->SetId());
    else eh.ResetId();
    if (m_Set->CanGetColl()) eh.SetColl(m_Set->SetColl());
    else eh.ResetColl();
    if (m_Set->CanGetLevel()) eh.SetLevel(m_Set->GetLevel());
    else eh.ResetLevel();
    if (m_Set->CanGetClass()) eh.SetClass(m_Set->GetClass());  // Level and Class are passed by value. Everything else is passed by reference.
    else eh.ResetClass();
    if (m_Set->CanGetRelease()) eh.SetRelease(m_Set->SetRelease());
    else eh.ResetRelease();
    if (m_Set->CanGetDate())    eh.SetDate(m_Set->SetDate());
    else eh.ResetDate();
    if (m_Set->CanGetDescr())   eh.SetDescr(m_Set->SetDescr());
    else eh.ResetDescr();


    // The following has been copied from objtools/cleanup/newcleanupp.cpp
    while( ! RAW_FIELD_IS_EMPTY_OR_UNSET( *m_BH.GetCompleteBioseq_set(), Seq_set ) )  {
        CSeq_entry_CI entry_ci( m_BH );
        CSeq_entry_EditHandle( *entry_ci ).Remove();
    }
    EDIT_EACH_SEQENTRY_ON_SEQSET( entry_iter, *m_Set ) {
        eh.AttachEntry( **entry_iter );
    }

    while( ! RAW_FIELD_IS_EMPTY_OR_UNSET( *m_BH.GetCompleteBioseq_set(), Annot ) )  {
        CSeq_annot_CI annot_ci( m_BH );
        CSeq_annot_EditHandle( *annot_ci ).Remove();
    }
    EDIT_EACH_SEQANNOT_ON_SEQSET( annot_iter, *m_Set ) {
        eh.AttachAnnot( **annot_iter );
    }

     m_Set = saveSet;
}

void CCmdChangeBioseqSet::Unexecute()
{
    Execute();
}

string CCmdChangeBioseqSet::GetLabel()
{
    return "Modify Bioseq_set";
}



END_NCBI_SCOPE
