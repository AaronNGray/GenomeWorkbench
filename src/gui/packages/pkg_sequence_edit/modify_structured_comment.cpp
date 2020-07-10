/*  $Id: modify_structured_comment.cpp 42191 2019-01-10 16:36:16Z asztalos $
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
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/descriptor_change.hpp>

#include <wx/textdlg.h>
#include <gui/widgets/wx/wx_utils.hpp>
#include <objects/valid/Field_set.hpp>
#include <objects/valid/Field_rule.hpp>
#include <objtools/validator/validatorp.hpp> // TODO
#include <gui/packages/pkg_sequence_edit/modify_structured_comment.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CModifyStructuredComment::ApplyToCSeq_entry (CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite)  
{

    FOR_EACH_SEQDESC_ON_SEQENTRY (it, se) 
    {
        if ((*it)->IsUser()) 
        {
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->Assign(**it);

            bool modified = ApplyToUserObject(new_desc->SetUser());
            if (modified)
            {
                CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(tse.GetScope().GetSeq_entryHandle(se), **it, *new_desc));
                composite->AddCommand(*cmd);
            }
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET (it, se.GetSet()) {
            ApplyToCSeq_entry (tse, **it, composite);
        }
    }
}

void CModifyStructuredComment::apply_to_seq_and_feat(CSeq_entry_Handle tse, CCmdComposite* composite)
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
                CRef<CSeq_feat> edited_feat(new CSeq_feat);
                edited_feat->Assign(feat->GetOriginalFeature());
                bool modified = ApplyToUserObject(edited_feat->SetData().SetUser());
                if (modified)
                {
                    CIRef<IEditCommand> cmd(new CCmdChangeSeq_feat(fh, *edited_feat));
                    composite->AddCommand(*cmd);
                }
            }
        }
        ++feat;
    }
}

bool CModifyStructuredComment::apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, const string& title)
{
    bool ret = false;
    if (tse) {

        wxTextEntryDialog dlg(NULL,wxT("Enter Structure Comment Field:"), wxT("Field"), m_FindStr, wxOK|wxCANCEL);
        if (dlg.ShowModal() == wxID_OK)
        {
            m_FindStr = dlg.GetValue();
            if (m_FindStr.empty()) return ret;
            m_Field = ToStdString(m_FindStr);
            CRef<CCmdComposite> composite(new CCmdComposite(title));    
            apply_to_seq_and_feat(tse,composite);
            cmdProcessor->Execute(composite.GetPointer());
            ret = true;
        }
    }
    return ret;
}

bool CModifyStructuredComment::ApplyToUserObject(CUser_object& user)
{
    bool found = false;
    if (user.IsSetType() && user.GetType().IsStr() && user.GetType().GetStr() == "StructuredComment" && user.IsSetData()) // User-object.type.str 
    {
        CUser_object::TData::iterator i = user.SetData().begin();
        while (i != user.SetData().end())
        {
            CRef< CUser_field > uf = *i;
            if (uf->CanGetLabel() && uf->GetLabel().IsStr() && uf->GetLabel().GetStr() == m_Field)
            {
                found = true;
                i = user.SetData().erase(i);
            }
            else
                i++;
        }
        if (user.GetData().empty())
            user.ResetData();
    }
    return found;
}


bool CReorderStructuredComment::apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, const string& title)
{
    bool ret = false;
    if (tse) 
    {
        CValidError errors;
        validator::CValidError_imp imp(tse.GetScope().GetObjectManager(),&errors);
        m_Rules = CComment_set::GetCommentRules();
        CRef<CCmdComposite> composite(new CCmdComposite(title));    
        ApplyToCSeq_entry_user (tse, *(tse.GetCompleteSeq_entry()), composite);
        cmdProcessor->Execute(composite.GetPointer());
        ret = true;
    }
    return ret;
}

void CReorderStructuredComment::ApplyToCSeq_entry_user (CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite)  
{
    CSeq_entry_Handle seh = tse.GetScope().GetSeq_entryHandle(se);
    FOR_EACH_SEQDESC_ON_SEQENTRY (it, se) 
    {
        if ((*it)->IsUser()) 
        {
            const CUser_object& user = (*it)->GetUser();
            if (user.IsSetType() && user.GetType().IsStr() && user.GetType().GetStr() == "StructuredComment" && user.IsSetData()) // User-object.type.str 
            {
                const CUser_field& prefix = user.GetField("StructuredCommentPrefix");
                if (prefix.IsSetData() && prefix.GetData().IsStr() && m_Rules) 
                {
                    const string& pfx = prefix.GetData().GetStr();
                    try 
                    {
                        const CComment_rule& rule = m_Rules->FindCommentRule(pfx);
                        reorder_comment_by_rule(**it, seh, &rule, composite);
                        
                    }
                    catch (CException ) 
                    {
                        // no rule for this prefix, no error
                        reorder_comment_by_rule(**it, seh, nullptr, composite);
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

bool s_UserFieldCompare (const CRef<CUser_field>& f1, const CRef<CUser_field>& f2) // this happy little function travels from file to file...
{
    if (!f1->IsSetLabel()) return true;
    if (!f2->IsSetLabel()) return false;
    return f1->GetLabel().Compare(f2->GetLabel()) < 0;
}


void CReorderStructuredComment::reorder_comment_by_rule(const CSeqdesc& user_desc, CSeq_entry_Handle seh, const CComment_rule* rule, CCmdComposite* composite)
{
    CRef<CSeqdesc> edited_desc(new CSeqdesc);
    edited_desc->Assign(user_desc);

    const CUser_object& user = user_desc.GetUser();
    CUser_object& edited = edited_desc->SetUser();
    edited.ResetData();

    if (user.HasField("StructuredCommentPrefix"))
    {
        const CUser_field& prefix = user.GetField("StructuredCommentPrefix");
        if (prefix.IsSetData())
        {
            CRef< CUser_field > prefix_ref(new CUser_field);
            prefix_ref->Assign(prefix);
            edited.SetData().push_back(prefix_ref);
        }
    }

    CUser_object tmp;
    tmp.Assign(user);
    CUser_object::TData& fields = tmp.SetData();
    if (rule)
    {
        CField_set::Tdata::const_iterator field_rule = rule->GetFields().Get().begin();
        while (field_rule != rule->GetFields().Get().end())
        {
            string expected_field = (*field_rule)->GetField_name();
            CUser_object::TData::iterator field = fields.begin();
            while (field != fields.end())
            {
                bool found = false;
                if ((*field)->IsSetLabel()) 
                {
                    string label;
                    if ((*field)->GetLabel().IsStr()) label = (*field)->GetLabel().GetStr();
                    else                              label = NStr::IntToString((*field)->GetLabel().GetId());
                    
                    if (NStr::Equal(expected_field, label))
                    {
                        edited.SetData().push_back(*field);
                        field = fields.erase(field);
                        found = true;
                    }
                }
                if (!found) field++;
            }
            field_rule++;
        }
    }
    //  add fields not in the rules.
    stable_sort (fields.begin(), fields.end(), s_UserFieldCompare);
    CUser_object::TData::iterator field = fields.begin();
    while (field != fields.end())
    {
        if (!((*field)->IsSetLabel() && (*field)->GetLabel().IsStr() && ((*field)->GetLabel().GetStr() == "StructuredCommentPrefix" || (*field)->GetLabel().GetStr() == "StructuredCommentSuffix")))
        edited.SetData().push_back(*field);
        field++;
    }
    
    if (user.HasField("StructuredCommentSuffix"))
    {
        const CUser_field& suffix = user.GetField("StructuredCommentSuffix");
        if (suffix.IsSetData())
        {
            CRef< CUser_field > suffix_ref(new CUser_field);
            suffix_ref->Assign(suffix);
            edited.SetData().push_back(suffix_ref);
        }
    }

    CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(seh, user_desc, *edited_desc));
    composite->AddCommand(*cmd);
}

END_NCBI_SCOPE
