/*  $Id: remove_structuredcomment_keyword.cpp 39271 2017-09-01 12:33:55Z bollin $
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
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <objtools/validator/validerror_desc.hpp>
#include <gui/packages/pkg_sequence_edit/remove_structuredcomment_keyword.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CValidError_desc;

CChangeStructuredCommentKeyword::CChangeStructuredCommentKeyword()
{
    m_PrefixToKeyword["##MIGS-Data-START##"] = "GSC:MIGS:2.1";
    m_PrefixToKeyword["##MIMS-Data-START##"] = "GSC:MIMS:2.1";
    m_PrefixToKeyword["##MIENS-Data-START##"] = "GSC:MIENS:2.1";
    m_PrefixToKeyword["##MIGS:3.0-Data-START##"] = "GSC:MIxS;MIGS:3.0";
    m_PrefixToKeyword["##MIMS:3.0-Data-START##"] = "GSC:MIxS;MIMS:3.0";
    m_PrefixToKeyword["##MIMARKS:3.0-Data-START##"] = "GSC:MIxS;MIMARKS:3.0";
    for (map<string,string>::iterator f = m_PrefixToKeyword.begin(); f != m_PrefixToKeyword.end(); ++f)
        m_Keywords.insert(f->second);
}

void CChangeStructuredCommentKeyword::apply_to_seq_and_feat(objects::CSeq_entry_Handle tse, CCmdComposite* composite)
{
    ApplyToCSeq_entry_user (tse, *(tse.GetCompleteSeq_entry()), composite);
    ApplyToCSeq_entry_genbank(tse, *(tse.GetCompleteSeq_entry()), composite);
    // doesn't really apply to seq-feat in this case it seems
}

// get keyword for prefix
string CChangeStructuredCommentKeyword::PrefixToKeyword(const string &prefix)
{
    map<string,string>::iterator f = m_PrefixToKeyword.find(prefix);
    if (f != m_PrefixToKeyword.end())
        return f->second;
    else
        return "";
}

bool CChangeStructuredCommentKeyword::apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title)
{
    bool ret = false;
    if (tse) 
    {
        CRef<CCmdComposite> composite(new CCmdComposite(title));    
        apply_to_seq_and_feat(tse,composite);
        cmdProcessor->Execute(composite.GetPointer());
        ret = true;
    }
    return ret;
}



void CChangeStructuredCommentKeyword::ApplyToCSeq_entry_user (objects::CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite)  
{

    FOR_EACH_SEQDESC_ON_SEQENTRY (it, se) 
    {
        if ((*it)->IsUser()) 
        {
            const CUser_object& user = (*it)->GetUser();
            // validate structured comment
            CValidError errors;
            validator::CValidError_imp imp(tse.GetScope().GetObjectManager(),&errors);
            validator::CValidError_desc validator(imp);
            if (validator.ValidateStructuredComment(user, **it))
            {
                if (user.IsSetType() && user.GetType().IsStr() && user.GetType().GetStr() == "StructuredComment" && user.IsSetData()) // User-object.type.str 
                {
                    const CUser_field& prefix = user.GetField("StructuredCommentPrefix");
                    if (prefix.IsSetData() && prefix.GetData().IsStr()) 
                    {
                        const string& pfx = prefix.GetData().GetStr();
                        string keyword = PrefixToKeyword(pfx);
                        if (!keyword.empty())
                            m_ValidKeywords.insert(keyword);
                    }
                }
            }
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET (it, se.GetSet()) {
            ApplyToCSeq_entry_user (tse, **it, composite);
        }
    }
}

// remove keyword - it's actually in seqdesc->genbank->keywords and not in the structured comment itself
void CRemoveStructuredCommentKeyword::ApplyToCSeq_entry_genbank (objects::CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite) 
{
 FOR_EACH_SEQDESC_ON_SEQENTRY (it, se) 
    {
        if ((*it)->IsGenbank() && (*it)->GetGenbank().CanGetKeywords()) 
        {
            CRef<CSerialObject> edited_object;
            edited_object.Reset((CSerialObject*)CGB_block::GetTypeInfo()->Create());
            edited_object->Assign((*it)->GetGenbank());
            CGB_block& edited = dynamic_cast<CGB_block&>(*edited_object);
            CGB_block::TKeywords &words = edited.SetKeywords();
            set<string> to_delete;
            for (CGB_block::TKeywords::iterator w = words.begin(); w != words.end(); ++w)
                if (m_Keywords.find(*w) != m_Keywords.end() && m_ValidKeywords.find(*w) == m_ValidKeywords.end()) 
                    to_delete.insert(*w);
            int size1 = words.size();
            for (set<string>::iterator w = to_delete.begin(); w != to_delete.end(); ++w)
                words.remove(*w);
            int size2 = words.size();
            if (size1 != size2)
            {
                if (size2 > 0)
                {
                    CChangeGBblockCommand* cmd = new CChangeGBblockCommand();
                    CObject* actual = (CObject*) &((*it)->GetGenbank());
                    cmd->Add(actual, CConstRef<CObject>(edited_object));
                    composite->AddCommand(*cmd);
                }
                else // remove the whole seq-desc if empty
                {
                    CSeq_entry_Handle seh = tse.GetScope().GetSeq_entryHandle(se);
                    CIRef<IEditCommand> cmdDelDesc(new CCmdDelDesc(seh, **it));
                    composite->AddCommand(*cmdDelDesc);
                }
            }
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET (it, se.GetSet()) {
            ApplyToCSeq_entry_genbank (tse, **it, composite);
        }
    }

}


void CAddStructuredCommentKeyword::ApplyToCSeq_entry_genbank (objects::CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite)
{
    for (set<string>::const_iterator keyword = m_ValidKeywords.begin(); keyword != m_ValidKeywords.end(); keyword++)
    {
        bool modified = false;
        add_to_existing_genbank(tse, se, composite, *keyword, modified);
        if (!modified)
        {
            // create new GB_block
            CRef<CSeqdesc> descr(new CSeqdesc());
            descr->Select(CSeqdesc::e_Genbank);
            descr->SetGenbank().ResetKeywords();
            descr->SetGenbank().SetKeywords().push_back(*keyword);
            CSeq_entry_Handle seh = tse.GetScope().GetSeq_entryHandle(se);
            CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(seh, *descr));
            composite->AddCommand(*cmdAddDesc);
        }
    }
}

void CAddStructuredCommentKeyword::add_to_existing_genbank (objects::CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite, const string keyword, bool &modified) 
{
    FOR_EACH_SEQDESC_ON_SEQENTRY (it, se) 
    {
        if ((*it)->IsGenbank() && (*it)->GetGenbank().CanGetKeywords()) 
        {
            CRef<CSerialObject> edited_object;
            edited_object.Reset((CSerialObject*)CGB_block::GetTypeInfo()->Create());
            edited_object->Assign((*it)->GetGenbank());
            CGB_block& edited = dynamic_cast<CGB_block&>(*edited_object);
            CGB_block::TKeywords &words = edited.SetKeywords();
            bool found = false;
            for (CGB_block::TKeywords::iterator w = words.begin(); w != words.end(); ++w)
                if (*w == keyword) found = true;
            if (!found) 
            {
                words.push_back(keyword);
                CChangeGBblockCommand* cmd = new CChangeGBblockCommand();
                CObject* actual = (CObject*) &((*it)->GetGenbank());
                cmd->Add(actual, CConstRef<CObject>(edited_object));
                composite->AddCommand(*cmd);
                modified = true;
            }
            if (found) modified = true;
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET (it, se.GetSet()) {
            add_to_existing_genbank (tse, **it, composite, keyword, modified);
        }
    }

}


END_NCBI_SCOPE
