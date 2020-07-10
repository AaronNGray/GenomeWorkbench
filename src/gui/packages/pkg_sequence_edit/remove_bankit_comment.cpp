/*  $Id: remove_bankit_comment.cpp 27157 2013-01-02 17:59:28Z filippov $
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
#include <objmgr/scope.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/remove_bankit_comment.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;

void CRemoveBankitComment::ApplyToCSeq_entry (objects::CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite)  
{

    FOR_EACH_SEQDESC_ON_SEQENTRY (it, se) 
    {
        if ((*it)->IsUser()) 
        {
            CSeq_entry_Handle seh = tse.GetScope().GetSeq_entryHandle(se);
            bool modified = ApplyToUserObject((*it)->GetUser(),seh);
            if (modified)
            {
                CIRef<IEditCommand> cmdDelDesc(new CCmdDelDesc(seh, **it));
                composite->AddCommand(*cmdDelDesc);
            }
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET (it, se.GetSet()) {
            ApplyToCSeq_entry (tse, **it, composite);
        }
    }
}

void CRemoveBankitComment::apply_to_seq_and_feat(objects::CSeq_entry_Handle tse, CCmdComposite* composite)
{
    ApplyToCSeq_entry (tse, *(tse.GetCompleteSeq_entry()), composite);
    /* CFeat_CI feat (tse, SAnnotSelector(CSeqFeatData::e_User));
    while (feat) 
    {
        if (feat->IsSetData())
        {
            CSeq_feat_Handle fh;
            fh = feat->GetSeq_feat_Handle();
            if (fh)
            {
                bool modified = ApplyToUserObject(feat->GetData().GetUser(),fh);
                if (modified)
                {
                    CIRef<IEditCommand> mainCmd(new CCmdDelSeq_feat(fh));
                    composite->AddCommand(*mainCmd);
                }
            }
        }
        ++feat;
        }*/
}

bool CRemoveBankitComment::apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title)
{
    if (tse) {
        CRef<CCmdComposite> composite(new CCmdComposite(title));    
        apply_to_seq_and_feat(tse,composite);
        cmdProcessor->Execute(composite.GetPointer());
        return true;
    }
    else
        return false;
}



bool CRemoveBankitComment::ApplyToUserObject(const CUser_object& user, CSeq_entry_Handle seh)
{
    bool found = false;
    if (user.IsSetType() && user.GetType().IsStr() && user.GetType().GetStr() == "Submission") // User-object.type.str 
        found = true;
    return found;
}

bool CRemoveBankitComment::ApplyToUserObject(const CUser_object& user, CSeq_feat_Handle fh)
{
    bool found = false;
    if (user.IsSetType() && user.GetType().IsStr() && user.GetType().GetStr() == "Submission") // User-object.type.str 
        found = true;
    return found;
}



END_NCBI_SCOPE
