/*  $Id: remove_annot.cpp 27132 2012-12-26 20:23:14Z filippov $
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
 * Authors:  Igor Filippov
 */


#include <ncbi_pch.hpp>
#include <gui/packages/pkg_sequence_edit/remove_annot.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;

bool CRemoveSeqAnnot::ApplyToSeqAnot(const CSeq_annot_Handle& eh,  CSeq_annot::C_Data::E_Choice which)
{
    CConstRef<CSeq_annot> annot = eh.GetCompleteSeq_annot();
    if (annot->IsSetData() && annot->GetData().Which() == which)
        return true;
    else
        return false;
}


void CRemoveSeqAnnot::ApplyToCSeq_entry (objects::CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite, CSeq_annot::C_Data::E_Choice which)  
{
    CSeq_entry_Handle seh = tse.GetScope().GetSeq_entryHandle(se);
    for ( CSeq_annot_CI it(seh, CSeq_annot_CI::eSearch_entry); it ; ++it ) 
    {
        if (ApplyToSeqAnot(*it,which))
        {
            CIRef<IEditCommand> cmdDelAnnot(new CCmdDelSeq_annot(*it));
            composite->AddCommand(*cmdDelAnnot);
        }
    }


    if (se.IsSet()) 
    {
        FOR_EACH_SEQENTRY_ON_SEQSET (it, se.GetSet()) 
        {
            ApplyToCSeq_entry (tse, **it, composite, which);
        }
    }
}


void CRemoveSeqAnnot::apply_to_seq_and_feat(objects::CSeq_entry_Handle tse, CCmdComposite* composite, CSeq_annot::C_Data::E_Choice which)
{
    ApplyToCSeq_entry (tse, *(tse.GetCompleteSeq_entry()), composite, which);
    // Seq-annot does not seem to be present in seqfeat
}


bool CRemoveSeqAnnot::apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title, CSeq_annot::C_Data::E_Choice which)
{
    if (tse) {
        CRef<CCmdComposite> composite(new CCmdComposite(title));    
        apply_to_seq_and_feat(tse,composite, which);
        cmdProcessor->Execute(composite.GetPointer());
        return true;
    }
    else
        return false;
}

END_NCBI_SCOPE
