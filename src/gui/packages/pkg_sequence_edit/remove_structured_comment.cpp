/*  $Id: remove_structured_comment.cpp 29154 2013-11-01 15:17:27Z filippov $
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
#include <gui/packages/pkg_sequence_edit/remove_structured_comment.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;

void CRemoveStructuredComment::ApplyToCSeq_entry (objects::CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite)  
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

void CRemoveStructuredComment::apply_to_seq_and_feat(objects::CSeq_entry_Handle tse, CCmdComposite* composite)
{
    ApplyToCSeq_entry (tse, *(tse.GetCompleteSeq_entry()), composite);
    CFeat_CI feat (tse, SAnnotSelector(CSeqFeatData::e_User));
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
    }
}

bool CRemoveStructuredComment::apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title)
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

bool CRemoveDuplicateStructuredComment::ApplyToUserObject(const CUser_object& user, CSeq_entry_Handle seh)
{
    bool found = false;
    if (user.IsSetType() && user.GetType().IsStr() && user.GetType().GetStr() == "StructuredComment") // User-object.type.str 
    {
        map<CSeq_entry_Handle, vector<const CUser_object *> >::iterator m = m_SeqEntryComments.find(seh);
        if (m != m_SeqEntryComments.end())
            {
                for (unsigned int j=0; j<m->second.size(); ++j)
                {
                    const CUser_object* u = m->second[j];
                    bool uclass = (u->IsSetClass() && user.IsSetClass() && u->GetClass() == user.GetClass()) || (!u->IsSetClass() && !user.IsSetClass());
                    bool data = (u->IsSetData() && user.IsSetData() && u->GetData().size() == user.GetData().size()) || (!u->IsSetData() && !user.IsSetData());
                    if (u->IsSetData() && user.IsSetData() && u->GetData().size() == user.GetData().size())
                    {
                        for (unsigned int i=0; i<u->GetData().size(); ++i)
                        {
                            CRef< CUser_field > uf1 = u->GetData()[i];
                            CRef< CUser_field > uf2 = user.GetData()[i];
                            data = data && uf1->IsSetData() && uf2->IsSetData() && uf1->GetData().IsStr() && uf2->GetData().IsStr() && uf1->GetData().GetStr() == uf2->GetData().GetStr();
                        }
                    }
                    if (uclass && data) found = true;
                }
            }
        if (!found)
            m_SeqEntryComments[seh].push_back(&user);
    }
    
    return found;
}

bool CRemoveDuplicateStructuredComment::ApplyToUserObject(const CUser_object& user, CSeq_feat_Handle fh)
{
    bool found = false;
    if (user.IsSetType() && user.GetType().IsStr() && user.GetType().GetStr() == "StructuredComment") // User-object.type.str 
    {
        map<CSeq_feat_Handle, vector<const CUser_object *> >::iterator m = m_SeqFeatComments.find(fh);
        if (m != m_SeqFeatComments.end())
            {
                for (unsigned int j=0; j<m->second.size(); ++j)
                {
                    const CUser_object* u = m->second[j];
                    bool uclass = (u->IsSetClass() && user.IsSetClass() && u->GetClass() == user.GetClass()) || (!u->IsSetClass() && !user.IsSetClass());
                    bool data = (u->IsSetData() && user.IsSetData() && u->GetData().size() == user.GetData().size()) || (!u->IsSetData() && !user.IsSetData());
                    if (u->IsSetData() && user.IsSetData() && u->GetData().size() == user.GetData().size())
                    {
                        for (unsigned int i=0; i<u->GetData().size(); ++i)
                        {
                            CRef< CUser_field > uf1 = u->GetData()[i];
                            CRef< CUser_field > uf2 = user.GetData()[i];
                            data = data && uf1->IsSetData() && uf2->IsSetData() && uf1->GetData().IsStr() && uf2->GetData().IsStr() && uf1->GetData().GetStr() == uf2->GetData().GetStr();
                        }
                    }
                    if (uclass && data) found = true;
                }
            }
        if (!found)
            m_SeqFeatComments[fh].push_back(&user);

    }

    return found;
}

bool CRemoveAllStructuredComment::ApplyToUserObject(const CUser_object& user, CSeq_entry_Handle seh)
{
    bool found = false;
    if (user.IsSetType() && user.GetType().IsStr() && user.GetType().GetStr() == "StructuredComment") // User-object.type.str 
        found = true;
    return found;
}

bool CRemoveAllStructuredComment::ApplyToUserObject(const CUser_object& user, CSeq_feat_Handle fh)
{
    bool found = false;
    if (user.IsSetType() && user.GetType().IsStr() && user.GetType().GetStr() == "StructuredComment") // User-object.type.str 
        found = true;
    return found;
}

bool CRemoveEmptyStructuredComment::ApplyToUserObject(const CUser_object& user, CSeq_entry_Handle seh)
{
    bool found = false;
    if (user.IsSetType() && user.GetType().IsStr() && user.GetType().GetStr() == "StructuredComment") // User-object.type.str 
    {
        if (!user.IsSetData() || user.GetData().empty())
            found = true;
        else
        {
            bool nonempty = false;
            for (unsigned int i=0; i<user.GetData().size(); ++i)
            {
                CRef< CUser_field > uf = user.GetData()[i];
                if (uf->IsSetData()) nonempty  = true;
            }
            found = !nonempty;
        }
    }
    return found;
}

bool CRemoveEmptyStructuredComment::ApplyToUserObject(const CUser_object& user, CSeq_feat_Handle fh)
{
    bool found = false;
    if (user.IsSetType() && user.GetType().IsStr() && user.GetType().GetStr() == "StructuredComment") // User-object.type.str 
    {
        if (!user.IsSetData() || user.GetData().empty())
            found = true;
        else
        {
            bool nonempty = false;
            for (unsigned int i=0; i<user.GetData().size(); ++i)
            {
                CRef< CUser_field > uf = user.GetData()[i];
                if (uf->IsSetData()) nonempty  = true;
            }
            found = !nonempty;
        }
    }
    return found;
}

END_NCBI_SCOPE
