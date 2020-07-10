/*  $Id: editing_action_desc.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
#include <objects/pub/Pub_equiv.hpp>
#include <objects/biblio/Cit_gen.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/biblio/Cit_book.hpp>
#include <objects/biblio/Cit_proc.hpp>
#include <objects/biblio/Cit_pat.hpp>
#include <objects/biblio/Cit_let.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/biblio_macros.hpp>
#include <objects/valid/Comment_rule.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/object_manager.hpp>
#include <objtools/edit/string_constraint.hpp>
#include <objtools/edit/publication_edit.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seq/Seq_data.hpp>
#include <objects/seqblock/GB_block.hpp>
#include <objects/seqblock/seqblock_macros.hpp>
#include <objects/general/general_macros.hpp>
#include <gui/widgets/edit/molinfo_fieldtype.hpp>
#include <gui/objutils/macro_fn_pubfields.hpp>
#include <gui/packages/pkg_sequence_edit/pubdesc_editor.hpp>
#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_desc.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


IEditingActionDesc::IEditingActionDesc(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit, const string &name, const CSeqdesc::E_Choice subtype)
    : IEditingAction(seh, name, submit), m_subtype(subtype), m_delete(false), m_change_submit_block(false), m_Desc(NULL)
{
  
}

void IEditingActionDesc::SetDesc(const CSeqdesc* desc)
{
    m_EditedDesc.Reset(new CSeqdesc);
    m_EditedDesc->Assign(*desc);
}
 
CBioseq_Handle IEditingActionDesc::GetBioseqHandle()
{
    return m_bsh;
}

void IEditingActionDesc::Find(EActionType action)
{   
    if (m_subtype == CSeqdesc::e_Pub && m_SeqSubmit && m_SeqSubmit->IsSetSub() && m_SeqSubmit->GetSub().IsSetCit()) 
    {
	CRef<CCit_sub> cit_sub(new CCit_sub);
	cit_sub->Assign(m_SeqSubmit->GetSub().GetCit());
	CRef<CPub> pub(new CPub);
	pub->SetSub(*cit_sub);
	CRef<CSeqdesc> seqdesc(new CSeqdesc);
	seqdesc->SetPub().SetPub().Set().push_back(pub);
	if (m_constraint->Match(seqdesc.GetPointer()))
	{
	    m_EditedDesc.Reset(new CSeqdesc);
	    if (m_ChangedSubmitBlock)
	    {
		cit_sub.Reset(new CCit_sub);
		cit_sub->Assign(m_ChangedSubmitBlock->GetCit());
		pub.Reset(new CPub);
		pub->SetSub(*cit_sub);
	    }
	    m_EditedDesc->SetPub().SetPub().Set().push_back(pub);
	    m_change_submit_block = true;
	    m_delete = false;
	    Modify(action);	   
	    m_change_submit_block = false;
	}
    }

    if (!m_TopSeqEntry)
        return;
    size_t count = 0;
    set<const CSeqdesc*> visited;
    for (CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na); b_iter; ++b_iter)
    {
        m_Desc = NULL;
        m_EditedDesc.Reset();
        CBioseq_Handle bsh = *b_iter; 
        m_CurrentSeqEntry = bsh.GetSeq_entry_Handle();
        m_bsh = bsh;
        if (m_constraint->Match(bsh))
        {
            bool found = false;
            CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
            CSeq_entry_Handle found_seh;
            for (CSeqdesc_CI desc_ci( bsh, m_subtype); desc_ci; ++desc_ci) 
            {
                found = true;
                m_Desc = &*desc_ci;
		if (visited.find(m_Desc) != visited.end())
		    continue;
		visited.insert(m_Desc);
                seh = desc_ci.GetSeq_entry_Handle();
                //m_CurrentSeqEntry = seh;
                if (m_constraint->Match(m_Desc))
                {
                    if (IsFrom(action) && found_seh && seh != found_seh)
                        break;
                    if (m_ChangedDescriptors.find(m_Desc) != m_ChangedDescriptors.end())
                        m_EditedDesc = m_ChangedDescriptors[m_Desc];
                    else
                    {
                        m_EditedDesc.Reset(new CSeqdesc);
                        m_EditedDesc->Assign(*m_Desc);
                    }
                    m_delete = false;
                    m_DescContext = seh;
                    Modify(action);
                    if (m_modified && m_delete)
                    {
                        m_DeletedDescriptors[m_Desc] = seh;
                    }
                    found_seh = seh;
                }
            }
            if (!found && !IsNOOP(action))
            {
                m_EditedDesc.Reset(new CSeqdesc);
                m_Desc = m_EditedDesc.GetPointer();
                if (m_constraint->Match(m_Desc))
                {
                    m_CreatedDescriptors[m_Desc] = seh;
                    Modify(action);
                }
            }
        }
        ++count;
        if (count >= m_max_records)
            break;       
    }
}

void IEditingActionDesc::FindRelated(EActionType action)
{
    IEditingActionDesc *desc_other = dynamic_cast<IEditingActionDesc*>(m_Other);
    if (!desc_other)
    {
        Find(action); 
        return;
    }

    if (m_subtype == CSeqdesc::e_Pub && desc_other->GetSeqSubmit() && desc_other->GetSeqSubmit()->IsSetSub() && desc_other->GetSeqSubmit()->GetSub().IsSetCit()) 
    {
	CRef<CCit_sub> cit_sub(new CCit_sub);
	cit_sub->Assign(desc_other->GetSeqSubmit()->GetSub().GetCit());
	CRef<CPub> pub(new CPub);
	pub->SetSub(*cit_sub);
	CRef<CSeqdesc> seqdesc(new CSeqdesc);
	seqdesc->SetPub().SetPub().Set().push_back(pub);
	if (m_constraint->Match(seqdesc.GetPointer()))
	{
	    m_EditedDesc.Reset(new CSeqdesc);
	    if (m_ChangedSubmitBlock)
	    {
		cit_sub.Reset(new CCit_sub);
		cit_sub->Assign(m_ChangedSubmitBlock->GetCit());
		pub.Reset(new CPub);
		pub->SetSub(*cit_sub);
	    }
	    m_EditedDesc->SetPub().SetPub().Set().push_back(pub);
	    m_change_submit_block = true;
	    m_delete = false;
	    Modify(action);
	    m_change_submit_block = false;	  
	}
    }

    m_Desc = NULL;
    m_EditedDesc.Reset();
    CBioseq_Handle bsh = desc_other->GetBioseqHandle(); 
    if (!bsh)
	return;
    bool found = false;
    CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
    CSeq_entry_Handle found_seh;
    for (CSeqdesc_CI desc_ci( bsh, m_subtype); desc_ci; ++desc_ci) 
    {
        found = true;
        m_Desc = &*desc_ci;
        seh = desc_ci.GetSeq_entry_Handle();
        if (IsFrom(action) && found_seh && seh != found_seh)
            break;
        if (m_constraint->Match(m_Desc))
        {
            if (m_ChangedDescriptors.find(m_Desc) != m_ChangedDescriptors.end())
                m_EditedDesc = m_ChangedDescriptors[m_Desc];
            else
            {
                m_EditedDesc.Reset(new CSeqdesc);
                m_EditedDesc->Assign(*m_Desc);
            }
            m_delete = false;
            m_DescContext = seh;
            Modify(action);
            if (m_modified && m_delete)
            {
                m_DeletedDescriptors[m_Desc] = seh;
            }
            found_seh = seh;
        }
    }
    if (!found && !IsNOOP(action))
    {
        m_EditedDesc.Reset(new CSeqdesc);
        m_Desc = m_EditedDesc.GetPointer();
        if (m_constraint->Match(m_Desc))
        {
            m_CreatedDescriptors[m_Desc] = seh;
            Modify(action);
        }
    }
}

void IEditingActionDesc::Modify(EActionType action)
{
    Action(action);
    if (m_modified && m_Desc && m_EditedDesc)
    {
        m_ChangedDescriptors[m_Desc] = m_EditedDesc;
        m_ContextForDescriptors[m_Desc] = m_DescContext;
    }   
    if (m_modified && m_change_submit_block && m_EditedDesc)
    {
	CRef<CCit_sub> cit_sub2(new CCit_sub);
	cit_sub2->Assign(m_EditedDesc->GetPub().GetPub().Get().front()->GetSub());
	m_ChangedSubmitBlock.Reset(new CSubmit_block);
	m_ChangedSubmitBlock->SetCit(*cit_sub2);
    }
}

bool IEditingActionDesc::SameObject()
{
    return false;
}

void IEditingActionDesc::SwapContext(IEditingAction* source)
{
    IEditingActionDesc *desc_source = dynamic_cast<IEditingActionDesc*>(source);
    if (desc_source)
    {
        swap(m_Desc, desc_source->m_Desc);
        swap(m_EditedDesc, desc_source->m_EditedDesc);
        swap(m_DescContext, desc_source->m_DescContext);
    }
    IEditingAction::SwapContext(source);
}

CEditingActionDescCommentDescriptor::CEditingActionDescCommentDescriptor(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDesc(seh, submit, "CEditingActionDescCommentDescriptor",  CSeqdesc::e_Comment)
{
}

bool CEditingActionDescCommentDescriptor::IsSetValue()
{
    return m_EditedDesc && m_EditedDesc->IsComment();
}

void CEditingActionDescCommentDescriptor::SetValue(const string &value)
{
    m_EditedDesc->SetComment(value);
}

string CEditingActionDescCommentDescriptor::GetValue()
{
    return m_EditedDesc->GetComment();
}

void CEditingActionDescCommentDescriptor::ResetValue()
{
    m_delete = true;
}

CEditingActionDescDefline::CEditingActionDescDefline(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDesc(seh, submit, "CEditingActionDescDefline",  CSeqdesc::e_Title)
{
}

bool CEditingActionDescDefline::IsSetValue()
{
    return m_EditedDesc && m_EditedDesc->IsTitle();
}

void CEditingActionDescDefline::SetValue(const string &value)
{
    m_EditedDesc->SetTitle(value);
}

string CEditingActionDescDefline::GetValue()
{
    return m_EditedDesc->GetTitle();
}

void CEditingActionDescDefline::ResetValue()
{
    m_delete = true;
}

CEditingActionDescKeyword::CEditingActionDescKeyword(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDesc(seh, submit, "CEditingActionDescKeyword",  CSeqdesc::e_Genbank), m_keyword(NULL), m_erase(false)
{
}

void CEditingActionDescKeyword::Modify(EActionType action)
{
    bool found = false;
    if (!IsCreateNew(action))
    {
        EDIT_EACH_KEYWORD_ON_GENBANKBLOCK(qual_it, m_EditedDesc->SetGenbank())
        {
            found = true;
            m_erase = false;
            m_keyword = &*qual_it;
            IEditingActionDesc::Modify(action);
            if (m_erase)
                ERASE_KEYWORD_ON_GENBANKBLOCK(qual_it, m_EditedDesc->SetGenbank());
        }
    }
    if (!found)
    {
        m_keyword = NULL;
        m_erase = false;
        IEditingActionDesc::Modify(action);
    }
}

bool CEditingActionDescKeyword::IsSetValue()
{
    return m_keyword != NULL;
}

void CEditingActionDescKeyword::SetValue(const string &value)
{
    if (m_keyword)
    {
        *m_keyword = value;
    }
    else
    {
        m_EditedDesc->SetGenbank().SetKeywords().push_back(value);
    }
}

string CEditingActionDescKeyword::GetValue()
{
    if (m_keyword)
        return *m_keyword;
    return kEmptyStr;
}

void CEditingActionDescKeyword::ResetValue()
{
    m_erase = true;
}


CEditingActionDescGenomeProjectId::CEditingActionDescGenomeProjectId(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDesc(seh, submit, "CEditingActionDescGenomeProjectId",  CSeqdesc::e_User)
{
}

bool CEditingActionDescGenomeProjectId::IsSetValue()
{
    return m_EditedDesc && m_EditedDesc->IsUser() && m_EditedDesc->GetUser().IsSetType() && m_EditedDesc->GetUser().GetType().IsStr()
        && NStr::EqualNocase(m_EditedDesc->GetUser().GetType().GetStr(), "GenomeProjectsDB") && m_EditedDesc->GetUser().HasField("ProjectID");
}

void CEditingActionDescGenomeProjectId::SetValue(const string &value)
{
    int numval = NStr::StringToInt(value, NStr::fConvErr_NoThrow);
    if (numval == 0)
        return;

    if (IsSetValue())
    {
        m_EditedDesc->SetUser().SetField("ProjectID").SetValue(numval);
    }
    else if (!m_EditedDesc->IsUser())
    {
        m_EditedDesc->SetUser().SetType().SetStr("GenomeProjectsDB");
        m_EditedDesc->SetUser().AddField("ProjectID", numval);
    }
}

string CEditingActionDescGenomeProjectId::GetValue()
{
    const CUser_field& field = m_EditedDesc->GetUser().GetField("ProjectID");
    if (field.IsSetData() && field.GetData().IsInt())
    {
        return NStr::NumericToString(field.GetData().GetInt());
    }
    if (field.IsSetData() && field.GetData().IsStr())
    {
        return field.GetData().GetStr();
    }
    return kEmptyStr;
}

void CEditingActionDescGenomeProjectId::ResetValue()
{
    if ( !m_EditedDesc->GetUser().IsSetType() || !m_EditedDesc->GetUser().GetType().IsStr()
         || !NStr::EqualNocase(m_EditedDesc->GetUser().GetType().GetStr(), "GenomeProjectsDB"))
        return;

    EDIT_EACH_USERFIELD_ON_USEROBJECT(field_iter, m_EditedDesc->SetUser())
    {
        CUser_field& field = **field_iter;
        if (field.GetLabel().IsStr() &&  NStr::Equal(field.GetLabel().GetStr(), "ProjectID") )
        {
            ERASE_USERFIELD_ON_USEROBJECT(field_iter, m_EditedDesc->SetUser());
            
        }
    }
    if (m_EditedDesc->GetUser().GetData().empty())
        m_delete = true;
}

CEditingActionDescUserObject::CEditingActionDescUserObject(CSeq_entry_Handle seh, const string &obj_type, const string &field, const bool create_multiple, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDesc(seh, submit, "CEditingActionDescUserObject",  CSeqdesc::e_User), m_type(obj_type), m_field(field), m_create_multiple(create_multiple), m_erase(false)
{
}

bool CEditingActionDescUserObject::SameObject()
{
    CEditingActionDescUserObject *other = dynamic_cast<CEditingActionDescUserObject*>(m_Other);
    if (other && m_type == other->m_type)
        return true;
    return false;
}

void CEditingActionDescUserObject::Find(EActionType action)
{
    if (!m_TopSeqEntry)
        return;
    size_t count = 0;
    set<const CSeqdesc*> visited;
    for (CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na); b_iter; ++b_iter)
    {
        m_Desc = NULL;
        m_EditedDesc.Reset();
        CBioseq_Handle bsh = *b_iter; 
        m_bsh = bsh;
        m_CurrentSeqEntry = bsh.GetSeq_entry_Handle();
        if (m_constraint->Match(bsh))
        {
            bool found = false;
            CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
            CSeq_entry_Handle found_seh;
            for (CSeqdesc_CI desc_ci( bsh, m_subtype); desc_ci; ++desc_ci) 
            {
                if (desc_ci->GetUser().IsSetType() && desc_ci->GetUser().GetType().IsStr()
                    && NStr::EqualNocase(desc_ci->GetUser().GetType().GetStr(), m_type) )
                {
                    found = true;
                    m_Desc = &*desc_ci;
		    if (visited.find(m_Desc) != visited.end())
			continue;
		    visited.insert(m_Desc);
                    seh = desc_ci.GetSeq_entry_Handle();
                    if (IsFrom(action) && found_seh && seh != found_seh)
                        break;
                    //m_CurrentSeqEntry = seh;
                    if (m_constraint->Match(m_Desc))
                    {
                        if (m_ChangedDescriptors.find(m_Desc) != m_ChangedDescriptors.end())
                            m_EditedDesc = m_ChangedDescriptors[m_Desc];
                        else
                        {
                            m_EditedDesc.Reset(new CSeqdesc);
                            m_EditedDesc->Assign(*m_Desc);
                        }
                        m_delete = false;
                        m_DescContext = seh;
                        Modify(action);
                        if (m_modified && m_delete)
                        {
                            m_DeletedDescriptors[m_Desc] = seh;
                        }
                        found_seh = seh;
                    }
                }

            }

            if (!found && !IsNOOP(action))
            {
                m_EditedDesc.Reset(new CSeqdesc);
                m_EditedDesc->SetUser();
                m_Desc = m_EditedDesc.GetPointer();
                if (m_constraint->Match(m_Desc))
                {
                    m_CreatedDescriptors[m_Desc] = seh;
                    Modify(action);
                }
            }
        }
        ++count;
        if (count >= m_max_records)
            break;       
    }
}

void CEditingActionDescUserObject::FindRelated(EActionType action)
{
    IEditingActionDesc *desc_other = dynamic_cast<IEditingActionDesc*>(m_Other);
    if (!desc_other)
    {
        Find(action); 
        return;
    }

    m_Desc = NULL;
    m_EditedDesc.Reset();
    CBioseq_Handle bsh = desc_other->GetBioseqHandle(); 
    if (!bsh)
	return;
    bool found = false;
    CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
    CSeq_entry_Handle found_seh;
    for (CSeqdesc_CI desc_ci( bsh, m_subtype); desc_ci; ++desc_ci) 
    {
        if (desc_ci->GetUser().IsSetType() && desc_ci->GetUser().GetType().IsStr()
            && NStr::EqualNocase(desc_ci->GetUser().GetType().GetStr(), m_type) )
        {
            found = true;
            m_Desc = &*desc_ci;
            seh = desc_ci.GetSeq_entry_Handle();
            if (IsFrom(action) && found_seh && seh != found_seh)
                break;
            if (m_constraint->Match(m_Desc))
            {
                if (m_ChangedDescriptors.find(m_Desc) != m_ChangedDescriptors.end())
                    m_EditedDesc = m_ChangedDescriptors[m_Desc];
                else
                {
                    m_EditedDesc.Reset(new CSeqdesc);
                    m_EditedDesc->Assign(*m_Desc);
                }
                m_delete = false;
                m_DescContext = seh;
                Modify(action);
                if (m_modified && m_delete)
                {
                    m_DeletedDescriptors[m_Desc] = seh;
                }
                found_seh = seh;
            }
        }
    }
    if (!found && !IsNOOP(action))
    {
        m_EditedDesc.Reset(new CSeqdesc);
        m_EditedDesc->SetUser();
        m_Desc = m_EditedDesc.GetPointer();
        if (m_constraint->Match(m_Desc))
        {
            m_CreatedDescriptors[m_Desc] = seh;
            Modify(action);
        }
    }
}

void CEditingActionDescUserObject::Modify(EActionType action)
{  
     bool found = false;
     if (!IsCreateNew(action))
     {
         if (IsFrom(action) && m_EditedDesc->GetUser().IsSetData() && !m_EditedDesc->GetUser().GetData().empty() && m_EditedDesc->GetUser().GetData().capacity() < 2 * m_EditedDesc->GetUser().GetData().size())
         {
             m_EditedDesc->SetUser().SetData().reserve(2 * m_EditedDesc->GetUser().GetData().size());
         }
         
         EDIT_EACH_USERFIELD_ON_USEROBJECT(field_iter, m_EditedDesc->SetUser())
         {
             CUser_field& field = **field_iter;
             if (field.GetLabel().IsStr() && NStr::Equal(field.GetLabel().GetStr(), m_field) && field.IsSetData())
             {
                 m_erase = false;
                 m_str_value = NULL;
                 m_int_value = NULL;
                 m_real_value = NULL;
                 m_bool_value = NULL;
                 switch (field.GetData().Which())
                 {
                 case CUser_field::TData::e_Str :
                 {
                     found = true;
                     m_str_value = &field.SetData().SetStr();
                     IEditingActionDesc::Modify(action);
                     break;
                 }
                 case  CUser_field::TData::e_Int :
                 {
                     found = true;
                     m_int_value = &field.SetData().SetInt();
                     IEditingActionDesc::Modify(action);
                     break;
                 }
                 case CUser_field::TData::e_Real :
                 {
                     found = true;
                     m_real_value = &field.SetData().SetReal();
                     IEditingActionDesc::Modify(action);
                     break;
                 }
                 case CUser_field::TData::e_Bool :
                 {
                     found = true;
                     m_bool_value = &field.SetData().SetBool();
                     IEditingActionDesc::Modify(action);
                     break;
                 }
                 case CUser_field::TData::e_Strs :
                 {
                     ERASE_ITERATE(CUser_field::TData::TStrs, it, field.SetData().SetStrs())
                     {
                         m_erase = false;
                         found = true;
                         m_str_value = &*it;
                         IEditingActionDesc::Modify(action);
                         if (m_erase)
                         {
                             VECTOR_ERASE(it, field.SetData().SetStrs());                   
                             if (field.IsSetNum())
                             {
                            int num = field.GetNum();
                            num--;
                            field.SetNum(num);
                             }
                         }
                     }
                     if (!field.SetData().SetStrs().empty())
                         m_erase = false;
                     break;
                 }
                 case CUser_field::TData::e_Ints :
                 {
                     ERASE_ITERATE(CUser_field::TData::TInts, it, field.SetData().SetInts())
                     {
                         m_erase = false;
                         found = true;
                         m_int_value = &*it;
                         IEditingActionDesc::Modify(action);
                         if (m_erase)
                         {
                             VECTOR_ERASE(it, field.SetData().SetInts());                   
                             if (field.IsSetNum())
                             {
                                 int num = field.GetNum();
                                 num--;
                                 field.SetNum(num);
                             }
                         }
                     }
                     if (!field.SetData().SetStrs().empty())
                         m_erase = false;
                     break;
                 }
                 case CUser_field::TData::e_Reals :
                 {
                     ERASE_ITERATE(CUser_field::TData::TReals, it, field.SetData().SetReals())
                     {
                         m_erase = false;
                         found = true;
                         m_real_value = &*it;
                         IEditingActionDesc::Modify(action);
                         if (m_erase)
                         {
                             VECTOR_ERASE(it, field.SetData().SetReals());                   
                             if (field.IsSetNum())
                             {
                                 int num = field.GetNum();
                                 num--;
                                 field.SetNum(num);
                             }
                         }
                     }
                     if (!field.SetData().SetReals().empty())
                         m_erase = false;
                     break;
                 }
                 default : break;
                 }
                 
                 if (m_erase)
                     ERASE_USERFIELD_ON_USEROBJECT(field_iter, m_EditedDesc->SetUser());     
             }
         }
         
         if (IsFrom(action) && m_EditedDesc->GetUser().IsSetData())
         {
             m_EditedDesc->SetUser().SetData().shrink_to_fit();
         }
     }

    if (!found)
    {
        m_str_value = NULL;
        m_int_value = NULL;
        m_real_value = NULL;
        m_bool_value = NULL;
        m_erase = false;
        IEditingActionDesc::Modify(action);
    }

    if (m_EditedDesc->GetUser().IsSetData() && m_EditedDesc->GetUser().GetData().empty())
        m_delete = true;   
}

bool CEditingActionDescUserObject::IsSetValue()
{
    return m_str_value != NULL ||
        m_int_value != NULL ||
        m_real_value != NULL ||
        m_bool_value != NULL;
}

void CEditingActionDescUserObject::SetValue(const string &value)
{
    if (m_str_value)
    {
        *m_str_value = value;
    }
    else if (m_int_value)
    {
        *m_int_value = NStr::StringToInt(value, NStr::fConvErr_NoThrow);
    }
    else if (m_real_value)
    {
        *m_real_value = NStr::StringToDouble(value, NStr::fConvErr_NoThrow);
    }
    else if (m_bool_value)
    {
        *m_bool_value = NStr::EqualNocase(value, "true");
    }
    else if (m_create_multiple)
    {
        if (m_EditedDesc->GetUser().HasField(m_field) && m_EditedDesc->GetUser().GetField(m_field).IsSetData() && m_EditedDesc->GetUser().GetField(m_field).GetData().IsStrs())
        {
            m_EditedDesc->SetUser().SetField(m_field).SetData().SetStrs().push_back(value);            
            m_EditedDesc->SetUser().SetField(m_field).SetNum(m_EditedDesc->GetUser().GetField(m_field).GetData().GetStrs().size());
        }
        else
        {
            m_EditedDesc->SetUser().SetType().SetStr(m_type);
            vector<string> values(1, value);
            m_EditedDesc->SetUser().AddField(m_field, values);
        }
    }
    else 
    {
        m_EditedDesc->SetUser().SetType().SetStr(m_type);
        m_EditedDesc->SetUser().AddField(m_field, value);
    }    
}

string CEditingActionDescUserObject::GetValue()
{
    if (m_str_value)
        return *m_str_value;
    if (m_int_value)
        return NStr::IntToString(*m_int_value);
    if (m_real_value)
        return NStr::DoubleToString(*m_real_value);
    if (m_bool_value)
        return *m_bool_value ? "true" : "false";
    return kEmptyStr;
}

void CEditingActionDescUserObject::ResetValue()
{
    m_erase = true;   
}

CEditingActionDescStructCommDbName::CEditingActionDescStructCommDbName(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDesc(seh, submit, "CEditingActionDescStructCommDbName",  CSeqdesc::e_User)
{
}

bool CEditingActionDescStructCommDbName::IsSetValue()
{
    return m_EditedDesc && m_EditedDesc->IsUser() && m_EditedDesc->GetUser().IsSetType() && m_EditedDesc->GetUser().GetType().IsStr()
        && NStr::EqualNocase(m_EditedDesc->GetUser().GetType().GetStr(), "StructuredComment") 
        && (m_EditedDesc->GetUser().HasField("StructuredCommentPrefix") || m_EditedDesc->GetUser().HasField("StructuredCommentSuffix"));
}

void CEditingActionDescStructCommDbName::SetValue(const string &value)
{
    string pre = CComment_rule::MakePrefixFromRoot(value);
    string suf = CComment_rule::MakeSuffixFromRoot(value);

    if (IsSetValue())
    {
        m_EditedDesc->SetUser().SetField("StructuredCommentPrefix").SetValue(pre);
        m_EditedDesc->SetUser().SetField("StructuredCommentSuffix").SetValue(suf);
    }
    else if ( m_EditedDesc->IsUser() && m_EditedDesc->GetUser().IsSetType() && m_EditedDesc->GetUser().GetType().IsStr()
              && NStr::EqualNocase(m_EditedDesc->GetUser().GetType().GetStr(), "StructuredComment"))
    {
        m_EditedDesc->SetUser().AddField("StructuredCommentPrefix", pre);
        m_EditedDesc->SetUser().AddField("StructuredCommentSuffix", suf);
    }    
}

string CEditingActionDescStructCommDbName::GetValue()
{
    if (m_EditedDesc->GetUser().HasField("StructuredCommentPrefix"))
    {
        const CUser_field& field = m_EditedDesc->GetUser().GetField("StructuredCommentPrefix");
        if (field.IsSetData() && field.GetData().IsStr())
        {
            string curr_val = field.GetData().GetStr();
            string root = curr_val;
            CComment_rule::NormalizePrefix(root);
            return root;
        }
    }

    if (m_EditedDesc->GetUser().HasField("StructuredCommentSuffix"))
    {
        const CUser_field& field = m_EditedDesc->GetUser().GetField("StructuredCommentSuffix");
        if (field.IsSetData() && field.GetData().IsStr())
        {
            string curr_val = field.GetData().GetStr();
            string root = curr_val;
            CComment_rule::NormalizePrefix(root);
            return root;
        }
    }

    return kEmptyStr;
}

void CEditingActionDescStructCommDbName::ResetValue()
{
    if ( !m_EditedDesc->GetUser().IsSetType() || !m_EditedDesc->GetUser().GetType().IsStr()
         || !NStr::EqualNocase(m_EditedDesc->GetUser().GetType().GetStr(), "StructuredComment"))
        return;

    EDIT_EACH_USERFIELD_ON_USEROBJECT(field_iter, m_EditedDesc->SetUser())
    {
        CUser_field& field = **field_iter;
        if (field.GetLabel().IsStr() &&  
            ( NStr::Equal(field.GetLabel().GetStr(), "StructuredCommentPrefix") || NStr::Equal(field.GetLabel().GetStr(),"StructuredCommentSuffix")) )
        {
            ERASE_USERFIELD_ON_USEROBJECT(field_iter, m_EditedDesc->SetUser());            
        }
    }
    if (m_EditedDesc->GetUser().GetData().empty())
        m_delete = true;
}

CEditingActionDescStructCommFieldName::CEditingActionDescStructCommFieldName(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDesc(seh, submit, "CEditingActionDescStructCommFieldName",  CSeqdesc::e_User), m_label(NULL), m_erase(false)
{
}

void CEditingActionDescStructCommFieldName::Modify(EActionType action)
{
    if (m_EditedDesc && m_EditedDesc->IsUser() && m_EditedDesc->GetUser().IsSetType() && m_EditedDesc->GetUser().GetType().IsStr()
        && NStr::EqualNocase(m_EditedDesc->GetUser().GetType().GetStr(), "StructuredComment") )
    {
        bool found = false;
        if (!IsCreateNew(action))
        {
            if (IsFrom(action) && m_EditedDesc->GetUser().IsSetData() && !m_EditedDesc->GetUser().GetData().empty() && m_EditedDesc->GetUser().GetData().capacity() < 2 * m_EditedDesc->GetUser().GetData().size())
            {
                m_EditedDesc->SetUser().SetData().reserve(2 * m_EditedDesc->GetUser().GetData().size());
            }
            EDIT_EACH_USERFIELD_ON_USEROBJECT(field_iter, m_EditedDesc->SetUser())
            {
                CUser_field& field = **field_iter;
                if (field.GetLabel().IsStr() &&  
                    !NStr::Equal(field.GetLabel().GetStr(), "StructuredCommentPrefix") && !NStr::Equal(field.GetLabel().GetStr(),"StructuredCommentSuffix"))
                {
                    found = true;
                    m_erase = false;
                    m_label = &field.SetLabel().SetStr();
                    IEditingActionDesc::Modify(action);
                    if (m_erase)
                        ERASE_USERFIELD_ON_USEROBJECT(field_iter, m_EditedDesc->SetUser());     
                }
            }
            if (IsFrom(action))
            {
                m_EditedDesc->SetUser().SetData().shrink_to_fit();
            }
        }
        if (!found)
        {
            m_label = NULL;
            m_erase = false;
            IEditingActionDesc::Modify(action);
        }
    }
}

bool CEditingActionDescStructCommFieldName::IsSetValue()
{
    return m_label != NULL;
}

void CEditingActionDescStructCommFieldName::SetValue(const string &value)
{   
    if (m_label)
    {
        *m_label = value;
    }
    else if (m_EditedDesc->IsUser())
    {
        m_EditedDesc->SetUser().AddField(value, kEmptyStr);
    }    
}

string CEditingActionDescStructCommFieldName::GetValue()
{
    if (m_label)
        return *m_label;
    return kEmptyStr;
}

void CEditingActionDescStructCommFieldName::ResetValue()
{
    m_erase = true;
}


CEditingActionMolInfoMolecule::CEditingActionMolInfoMolecule(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDesc(seh, submit, "CEditingActionMolInfoMolecule",  CSeqdesc::e_Molinfo)
{
}

bool CEditingActionMolInfoMolecule::IsSetValue()
{
    return m_EditedDesc->GetMolinfo().IsSetBiomol();
}

void CEditingActionMolInfoMolecule::SetValue(const string &value)
{
    m_EditedDesc->SetMolinfo().SetBiomol(   CMolInfoFieldType::GetBiomolFromLabel(value) ); //static_cast<CMolInfo::EBiomol>(CMolInfo::ENUM_METHOD_NAME(EBiomol)()->FindValue(value)));
}

string CEditingActionMolInfoMolecule::GetValue()
{
    return CMolInfoFieldType::GetBiomolLabel(m_EditedDesc->GetMolinfo().GetBiomol());
}

void CEditingActionMolInfoMolecule::ResetValue()
{
    m_EditedDesc->SetMolinfo().ResetBiomol();
}

CEditingActionMolInfoTechnique::CEditingActionMolInfoTechnique(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDesc(seh, submit, "CEditingActionMolInfoTechnique",  CSeqdesc::e_Molinfo)
{
}

bool CEditingActionMolInfoTechnique::IsSetValue()
{
    return m_EditedDesc->GetMolinfo().IsSetTech();
}

void CEditingActionMolInfoTechnique::SetValue(const string &value)
{
    m_EditedDesc->SetMolinfo().SetTech(CMolInfoFieldType::GetTechFromLabel(value)); //static_cast<CMolInfo::ETech>(CMolInfo::ENUM_METHOD_NAME(ETech)()->FindValue(value)));
}

string CEditingActionMolInfoTechnique::GetValue()
{
    return CMolInfoFieldType::GetTechLabel(m_EditedDesc->GetMolinfo().GetTech());
}

void CEditingActionMolInfoTechnique::ResetValue()
{
    m_EditedDesc->SetMolinfo().ResetTech();
}

CEditingActionMolInfoCompletedness::CEditingActionMolInfoCompletedness(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDesc(seh, submit, "CEditingActionMolInfoCompletedness",  CSeqdesc::e_Molinfo)
{
}

bool CEditingActionMolInfoCompletedness::IsSetValue()
{
    return m_EditedDesc->GetMolinfo().IsSetCompleteness();
}

void CEditingActionMolInfoCompletedness::SetValue(const string &value)
{
    m_EditedDesc->SetMolinfo().SetCompleteness(CMolInfoFieldType::GetCompletenessFromLabel(value)); //static_cast<CMolInfo::ECompleteness>(CMolInfo::ENUM_METHOD_NAME(ECompleteness)()->FindValue(value)));
}

string CEditingActionMolInfoCompletedness::GetValue()
{
    return CMolInfoFieldType::GetCompletenessLabel(m_EditedDesc->GetMolinfo().GetCompleteness());
}

void CEditingActionMolInfoCompletedness::ResetValue()
{
    m_EditedDesc->SetMolinfo().ResetCompleteness();
}



IEditingActionInst::IEditingActionInst(CSeq_entry_Handle seh, const string &name, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDesc(seh, submit,  name, CSeqdesc::e_Molinfo)
{
  
}

void IEditingActionInst::Find(EActionType action)
{
    if (!m_TopSeqEntry)
        return;
    size_t count = 0;
    for (CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na); b_iter; ++b_iter)
    {
        m_EditedInst.Reset();
        m_bsh = *b_iter; 
        m_CurrentSeqEntry = m_bsh.GetSeq_entry_Handle();
        if (m_constraint->Match(m_bsh))
        {
            if (m_ChangedInstances.find(m_bsh) != m_ChangedInstances.end())
                m_EditedInst = m_ChangedInstances[m_bsh];
            else
            {
                m_EditedInst.Reset(new CSeq_inst);
                m_EditedInst->Assign(m_bsh.GetInst());
            }
            Modify(action);
        }
        ++count;
        if (count >= m_max_records)
            break;
    }
}


void IEditingActionInst::FindRelated(EActionType action)
{
  Find(action); 
}

void IEditingActionInst::Modify(EActionType action)
{
    Action(action);
    if (m_modified && m_bsh && m_EditedInst)
        m_ChangedInstances[m_bsh] = m_EditedInst;
}

bool IEditingActionInst::SameObject()
{
    IEditingActionInst *other = dynamic_cast<IEditingActionInst*>(m_Other);
    if (other)
        return true;
    return false;
}

void IEditingActionInst::SwapContext(IEditingAction* source)
{
    IEditingActionInst *inst_source = dynamic_cast<IEditingActionInst*>(source);
    if (inst_source)
    {
        swap(m_bsh, inst_source->m_bsh);
        swap(m_EditedInst, inst_source->m_EditedInst);
    }
    IEditingAction::SwapContext(source);
}

CEditingActionMolInfoClass::CEditingActionMolInfoClass(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionInst(seh, "CEditingActionMolInfoClass", submit)
{
}

bool CEditingActionMolInfoClass::IsSetValue()
{
    return m_EditedInst->IsSetMol();
}

void CEditingActionMolInfoClass::SetValue(const string &value)
{
    m_EditedInst->SetMol(CMolInfoFieldType::GetMolFromLabel(value));
}

string CEditingActionMolInfoClass::GetValue()
{
    return CMolInfoFieldType::GetMolLabel(m_EditedInst->GetMol());
}

void CEditingActionMolInfoClass::ResetValue()
{
    m_EditedInst->ResetMol();
}

CEditingActionMolInfoTopology::CEditingActionMolInfoTopology(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionInst(seh, "CEditingActionMolInfoTopology", submit)
{
}

bool CEditingActionMolInfoTopology::IsSetValue()
{
    return m_EditedInst->IsSetTopology();
}

void CEditingActionMolInfoTopology::SetValue(const string &value)
{
    m_EditedInst->SetTopology(CMolInfoFieldType::GetTopologyFromLabel(value)); //static_cast<CSeq_inst::ETopology>(CSeq_inst::ENUM_METHOD_NAME(ETopology)()->FindValue(value)));
}

string CEditingActionMolInfoTopology::GetValue()
{
    return CMolInfoFieldType::GetTopologyLabel(m_EditedInst->GetTopology());
}

void CEditingActionMolInfoTopology::ResetValue()
{
    m_EditedInst->ResetTopology();
}

CEditingActionMolInfoStrand::CEditingActionMolInfoStrand(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionInst(seh, "CEditingActionMolInfoStrand", submit)
{
}

bool CEditingActionMolInfoStrand::IsSetValue()
{
    return m_EditedInst->IsSetStrand();
}

void CEditingActionMolInfoStrand::SetValue(const string &value)
{
    m_EditedInst->SetStrand(CMolInfoFieldType::GetStrandFromLabel(value)); //static_cast<CSeq_inst::EStrand>(CSeq_inst::ENUM_METHOD_NAME(EStrand)()->FindValue(value)));
}

string CEditingActionMolInfoStrand::GetValue()
{
    return CMolInfoFieldType::GetStrandLabel(m_EditedInst->GetStrand());
}

void CEditingActionMolInfoStrand::ResetValue()
{
    m_EditedInst->ResetStrand();
}

IEditingActionDescPub::IEditingActionDescPub(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit, const string &name)
    : IEditingActionDesc(seh, submit, name,  CSeqdesc::e_Pub)
{
}


bool IEditingActionDescPub::SameObject()
{
    IEditingActionDescPub *other = dynamic_cast<IEditingActionDescPub*>(m_Other);
    if (other)
        return true;
    return false;
}

void IEditingActionDescPub::SwapContext(IEditingAction* source)
{
    IEditingActionDescPub *pub_source = dynamic_cast<IEditingActionDescPub*>(source);
    if (pub_source)
    {
        swap(m_pub, pub_source->m_pub);
    }
    IEditingActionDesc::SwapContext(source);
}

CEditingActionDescPubTitle::CEditingActionDescPubTitle(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescPub(seh, submit, "CEditingActionDescPubTitle"), m_title(NULL), m_erase(false)
{
}

void CEditingActionDescPubTitle::SwapContext(IEditingAction* source)
{
    CEditingActionDescPubTitle *pub_source = dynamic_cast<CEditingActionDescPubTitle*>(source);
    if (pub_source)
    {
        swap(m_title, pub_source->m_title);
    }
    IEditingActionDescPub::SwapContext(source);
}

void CEditingActionDescPubTitle::Modify(EActionType action)
{
    if (m_EditedDesc->GetPub().IsSetPub() && m_EditedDesc->GetPub().GetPub().IsSet())
    {
        for (CPub_equiv::Tdata::iterator pub = m_EditedDesc->SetPub().SetPub().Set().begin(); pub != m_EditedDesc->SetPub().SetPub().Set().end(); ++pub)
        {
            m_pub = *pub;
            if (!m_pub)
                continue;
            bool found = false;
            if (!IsCreateNew(action))
            {
                GetTitles(m_pub, action, found);
            }
            if (!found)
            {
                m_title = NULL;
                m_erase = false;
                IEditingActionDesc::Modify(action);
            }
        }
    }      
}

void CEditingActionDescPubTitle::GetTitles(CRef<CPub> pub, EActionType action, bool &found)
{
    switch( pub->Which() ) 
    {
    case CPub::e_not_set:
    case CPub::e_Medline:
    case CPub::e_Pmid:
    case CPub::e_Pat_id:
        // these types don't have titles, so nothing to do
        break;
    case CPub::e_Gen:
        if( pub->GetGen().IsSetTitle() ) 
        {
            m_title = &pub->SetGen().SetTitle();
            found = true;
            m_erase = false;
            IEditingActionDesc::Modify(action);
            if (m_erase)
                pub->SetGen().ResetTitle();
        }
        break;
    case CPub::e_Sub:
        if( pub->GetSub().IsSetDescr() ) 
        {
            m_title = &pub->SetSub().SetDescr();
            found = true;
            m_erase = false;
            IEditingActionDesc::Modify(action);
            if (m_erase)
                pub->SetSub().ResetDescr();
        }
        break;
    case CPub::e_Article:
        if( pub->GetArticle().IsSetTitle() && pub->GetArticle().GetTitle().IsSet() ) 
        {
            CTitle::Tdata::iterator it = pub->SetArticle().SetTitle().Set().begin();
            while (it != pub->SetArticle().SetTitle().Set().end())
            {
                m_title = NULL;
                m_erase = false;
                switch ((*it)->Which()) 
                {
                case CTitle::C_E::e_Name:     m_title =  &(*it)->SetName(); break;
                case CTitle::C_E::e_Tsub:     m_title =  &(*it)->SetTsub();  break;
                case CTitle::C_E::e_Trans:    m_title =  &(*it)->SetTrans();  break;
                case CTitle::C_E::e_Jta:      m_title =  &(*it)->SetJta();  break;
                case CTitle::C_E::e_Iso_jta:  m_title =  &(*it)->SetIso_jta();  break;
                case CTitle::C_E::e_Ml_jta:   m_title =  &(*it)->SetMl_jta();  break;
                case CTitle::C_E::e_Coden:    m_title =  &(*it)->SetCoden();  break;
                case CTitle::C_E::e_Issn:     m_title =  &(*it)->SetIssn();  break;
                case CTitle::C_E::e_Abr:      m_title =  &(*it)->SetAbr();  break;
                case CTitle::C_E::e_Isbn:     m_title =  &(*it)->SetIsbn();  break;
                default: break;
                }
                if (m_title)
                {
                    found = true;
                    IEditingActionDesc::Modify(action);
                }
                if (m_erase)
                    it = pub->SetArticle().SetTitle().Set().erase(it);
                else
                    ++it;
            }
            if (pub->SetArticle().SetTitle().Set().empty())
                pub->SetArticle().ResetTitle();
        }
        break;
    case CPub::e_Journal:
        if( pub->GetJournal().IsSetTitle() ) 
        {
            CTitle::Tdata::iterator it = pub->SetJournal().SetTitle().Set().begin();
            while (it != pub->SetJournal().SetTitle().Set().end())
            {
                m_title = NULL;
                m_erase = false;
                switch ((*it)->Which()) 
                {
                case CTitle::C_E::e_Name:     m_title =  &(*it)->SetName(); break;
                case CTitle::C_E::e_Tsub:     m_title =  &(*it)->SetTsub();  break;
                case CTitle::C_E::e_Trans:    m_title =  &(*it)->SetTrans();  break;
                case CTitle::C_E::e_Jta:      m_title =  &(*it)->SetJta();  break;
                case CTitle::C_E::e_Iso_jta:  m_title =  &(*it)->SetIso_jta();  break;
                case CTitle::C_E::e_Ml_jta:   m_title =  &(*it)->SetMl_jta();  break;
                case CTitle::C_E::e_Coden:    m_title =  &(*it)->SetCoden();  break;
                case CTitle::C_E::e_Issn:     m_title =  &(*it)->SetIssn();  break;
                case CTitle::C_E::e_Abr:      m_title =  &(*it)->SetAbr();  break;
                case CTitle::C_E::e_Isbn:     m_title =  &(*it)->SetIsbn();  break;
                default: break;
                }
                if (m_title)
                {
                    found = true;
                    IEditingActionDesc::Modify(action);
                }
                if (m_erase)
                    it = pub->SetJournal().SetTitle().Set().erase(it);
                else
                    ++it;
            }
            if (pub->SetJournal().SetTitle().Set().empty())
                pub->SetJournal().ResetTitle();
        }
        break;
    case CPub::e_Book:
        if( pub->GetBook().IsSetTitle() ) 
        {
            CTitle::Tdata::iterator it = pub->SetBook().SetTitle().Set().begin();
            while (it != pub->SetBook().SetTitle().Set().end())
            {
                m_title = NULL;
                m_erase = false;
                switch ((*it)->Which()) 
                {
                case CTitle::C_E::e_Name:     m_title =  &(*it)->SetName(); break;
                case CTitle::C_E::e_Tsub:     m_title =  &(*it)->SetTsub();  break;
                case CTitle::C_E::e_Trans:    m_title =  &(*it)->SetTrans();  break;
                case CTitle::C_E::e_Jta:      m_title =  &(*it)->SetJta();  break;
                case CTitle::C_E::e_Iso_jta:  m_title =  &(*it)->SetIso_jta();  break;
                case CTitle::C_E::e_Ml_jta:   m_title =  &(*it)->SetMl_jta();  break;
                case CTitle::C_E::e_Coden:    m_title =  &(*it)->SetCoden();  break;
                case CTitle::C_E::e_Issn:     m_title =  &(*it)->SetIssn();  break;
                case CTitle::C_E::e_Abr:      m_title =  &(*it)->SetAbr();  break;
                case CTitle::C_E::e_Isbn:     m_title =  &(*it)->SetIsbn();  break;
                default: break;
                }
                if (m_title)
                {
                    found = true;
                    IEditingActionDesc::Modify(action);
                }
                if (m_erase)
                    it = pub->SetBook().SetTitle().Set().erase(it);
                else
                    ++it;
            }
            if (pub->SetBook().SetTitle().Set().empty())
                pub->SetBook().ResetTitle();
        }
        break;
    case CPub::e_Proc:
        // what to do here?  It has a book and meeting
        // It's not entirely clear if this is the best course of action
        if( FIELD_CHAIN_OF_2_IS_SET(pub->GetProc(), Book, Title) ) 
        {
            CTitle::Tdata::iterator it = pub->SetProc().SetBook().SetTitle().Set().begin();
            while (it != pub->SetProc().SetBook().SetTitle().Set().end())
            {
                m_title = NULL;
                m_erase = false;
                switch ((*it)->Which()) 
                {
                case CTitle::C_E::e_Name:     m_title =  &(*it)->SetName(); break;
                case CTitle::C_E::e_Tsub:     m_title =  &(*it)->SetTsub();  break;
                case CTitle::C_E::e_Trans:    m_title =  &(*it)->SetTrans();  break;
                case CTitle::C_E::e_Jta:      m_title =  &(*it)->SetJta();  break;
                case CTitle::C_E::e_Iso_jta:  m_title =  &(*it)->SetIso_jta();  break;
                case CTitle::C_E::e_Ml_jta:   m_title =  &(*it)->SetMl_jta();  break;
                case CTitle::C_E::e_Coden:    m_title =  &(*it)->SetCoden();  break;
                case CTitle::C_E::e_Issn:     m_title =  &(*it)->SetIssn();  break;
                case CTitle::C_E::e_Abr:      m_title =  &(*it)->SetAbr();  break;
                case CTitle::C_E::e_Isbn:     m_title =  &(*it)->SetIsbn();  break;
                default: break;
                }
                if (m_title)
                {
                    found = true;
                    IEditingActionDesc::Modify(action);
                }
                if (m_erase)
                    it = pub->SetProc().SetBook().SetTitle().Set().erase(it);
                else
                    ++it;
            }
            if (pub->SetProc().SetBook().SetTitle().Set().empty())
                pub->SetProc().SetBook().ResetTitle();
        }
        break;
    case CPub::e_Patent:
        if( pub->GetPatent().IsSetTitle() ) 
        {
            m_title = &pub->SetPatent().SetTitle();
            found = true;
            m_erase = false;
            IEditingActionDesc::Modify(action);
            if (m_erase)
                pub->SetPatent().ResetTitle();
        }
        break;
    case CPub::e_Man:
        if( FIELD_CHAIN_OF_2_IS_SET(pub->GetMan(), Cit, Title) ) 
        {
            CTitle::Tdata::iterator it = pub->SetMan().SetCit().SetTitle().Set().begin();
            while (it != pub->SetMan().SetCit().SetTitle().Set().end())
            {
                m_title = NULL;
                m_erase = false;
                switch ((*it)->Which()) 
                {
                case CTitle::C_E::e_Name:     m_title =  &(*it)->SetName(); break;
                case CTitle::C_E::e_Tsub:     m_title =  &(*it)->SetTsub();  break;
                case CTitle::C_E::e_Trans:    m_title =  &(*it)->SetTrans();  break;
                case CTitle::C_E::e_Jta:      m_title =  &(*it)->SetJta();  break;
                case CTitle::C_E::e_Iso_jta:  m_title =  &(*it)->SetIso_jta();  break;
                case CTitle::C_E::e_Ml_jta:   m_title =  &(*it)->SetMl_jta();  break;
                case CTitle::C_E::e_Coden:    m_title =  &(*it)->SetCoden();  break;
                case CTitle::C_E::e_Issn:     m_title =  &(*it)->SetIssn();  break;
                case CTitle::C_E::e_Abr:      m_title =  &(*it)->SetAbr();  break;
                case CTitle::C_E::e_Isbn:     m_title =  &(*it)->SetIsbn();  break;
                default: break;
                }
                if (m_title)
                {
                    found = true;
                    IEditingActionDesc::Modify(action);
                }
                if (m_erase)
                    it = pub->SetMan().SetCit().SetTitle().Set().erase(it);
                else
                    ++it;
            }
            if (pub->SetMan().SetCit().SetTitle().Set().empty())
                pub->SetMan().SetCit().ResetTitle();
        }
        break;
    case CPub::e_Equiv:
        {
            FOR_EACH_PUB_ON_PUBEQUIV(pub_it, pub->GetEquiv()) 
            {
                // dig down recursively
                GetTitles(*pub_it, action, found);
            }
        }
        break;
    default: break;
    }
}

bool CEditingActionDescPubTitle::IsSetValue()
{
    return m_title != NULL;
}

void CEditingActionDescPubTitle::SetValue(const string &value)
{
    if (m_title)
    {
        *m_title = value;
    }
    else
    {
        CRef<CTitle::C_E> title(new CTitle::C_E());
        title->SetName(value);

        switch (m_pub->Which()) 
        {
        case CPub::e_Gen:
            m_pub->SetGen().SetTitle(value);
            break;
        case CPub::e_Sub:
            m_pub->SetSub().SetDescr(value);
            break;
        case CPub::e_Article:
            m_pub->SetArticle().SetTitle().Set().push_back(title);
            break;
        case CPub::e_Book:
            m_pub->SetBook().SetTitle().Set().push_back(title);
            break;
        case CPub::e_Proc:
            m_pub->SetProc().SetBook().SetTitle().Set().push_back(title);
            break;
        case CPub::e_Patent:
            m_pub->SetPatent().SetTitle(value);
            break;
        case CPub::e_Man:
            m_pub->SetMan().SetCit().SetTitle().Set().push_back(title);
            break;
        case CPub::e_Medline:
        case CPub::e_Journal:
        case CPub::e_Pat_id:
        case CPub::e_Equiv:
        case CPub::e_Pmid:
        default:
            // do nothing
            break;
        }
    }
}

string CEditingActionDescPubTitle::GetValue()
{
    if (m_title)
        return *m_title;
    return kEmptyStr;
}

void CEditingActionDescPubTitle::ResetValue()
{
    m_erase = true;
}

CEditingActionDescPubPmid::CEditingActionDescPubPmid(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescPub(seh, submit, "CEditingActionDescPubPmid"), m_erase(false)
{
}

void CEditingActionDescPubPmid::SwapContext(IEditingAction* source)
{
    CEditingActionDescPubPmid *pub_source = dynamic_cast<CEditingActionDescPubPmid*>(source);
    if (pub_source)
    {
        swap(m_id, pub_source->m_id);
    }
    IEditingActionDescPub::SwapContext(source);
}

void CEditingActionDescPubPmid::Modify(EActionType action)
{
    bool found = false;
    if (!IsCreateNew(action))
    {
        if (m_EditedDesc->GetPub().IsSetPub() && m_EditedDesc->GetPub().GetPub().IsSet())
        {
            CPub_equiv::Tdata::iterator pub = m_EditedDesc->SetPub().SetPub().Set().begin();
            while (pub != m_EditedDesc->SetPub().SetPub().Set().end())
            {
		if ((*pub)->IsPmid())
		{
		    m_pub = *pub;
		    m_id.Reset();
		    m_erase = false;
		    found = true;
		    IEditingActionDesc::Modify(action);  
		    if (m_erase)
			pub = m_EditedDesc->SetPub().SetPub().Set().erase(pub);
		    else
			++pub;
		}
		else if ((*pub)->IsArticle())
		{
		    m_pub = *pub;
		    bool found_id = false;
		    if ((*pub)->GetArticle().IsSetIds() && (*pub)->GetArticle().GetIds().IsSet())
		    {
			CArticleIdSet::Tdata::iterator id = (*pub)->SetArticle().SetIds().Set().begin();
			while (id != (*pub)->SetArticle().SetIds().Set().end())
			{
			    if ((*id)->IsPubmed())
			    {
				m_id = *id;
				m_erase = false;
				found = true;
				found_id = true;
				IEditingActionDesc::Modify(action);  
				if (m_erase)
				    id = (*pub)->SetArticle().SetIds().Set().erase(id);
				else
				    ++id;
			    }
			    else
				++id;
			}
			if ((*pub)->GetArticle().GetIds().Get().empty())
			    (*pub)->SetArticle().ResetIds();
		    }
		    if (!found_id)
		    {
			m_id.Reset();
			m_erase = false;
			IEditingActionDesc::Modify(action);  
		    }
		    ++pub;
		}
		else
		    ++pub;
            }
        }      
    }
    if (!found)
    {
        m_pub.Reset();
	m_id.Reset();
        m_erase = false;
        IEditingActionDesc::Modify(action);
    }
}

bool CEditingActionDescPubPmid::IsSetValue()
{
    return (m_pub && m_pub->IsPmid()) || (m_id && m_id->IsPubmed());
}

void CEditingActionDescPubPmid::SetValue(const string &value)
{
    int numval = NStr::StringToInt(value, NStr::fConvErr_NoThrow);
    if (numval == 0)
        return;

    if (m_pub && m_pub->IsPmid())
    {
        m_pub->SetPmid(CPubMedId(numval));
    }
    else if (m_id && m_id->IsPubmed())
    {
	m_id->SetPubmed(CPubMedId(numval));
    }
    else if (m_pub && m_pub->IsArticle())
    {
	CRef<CArticleId> id(new CArticleId);
	id->SetPubmed(CPubMedId(numval));
	m_pub->SetArticle().SetIds().Set().push_back(id);
    }
    else 
    {
        CRef<CPub> pub(new CPub);
        pub->SetPmid(CPubMedId(numval));
        m_EditedDesc->SetPub().SetPub().Set().push_back(pub);
    }
}

string CEditingActionDescPubPmid::GetValue()
{
    string res;
    if (m_pub && m_pub->IsPmid())
      res = NStr::IntToString(m_pub->GetPmid());
    else if (m_id && m_id->IsPubmed())
	res = NStr::IntToString(m_id->GetPubmed());
    return res;
}

void CEditingActionDescPubPmid::ResetValue()
{
    m_erase = true;
}


IEditingActionDescAffil::IEditingActionDescAffil(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit, const string &name)
    : IEditingActionDescPub(seh, submit, name), m_affil(NULL)
{
}

void IEditingActionDescAffil::SwapContext(IEditingAction* source)
{
    IEditingActionDescAffil *pub_source = dynamic_cast<IEditingActionDescAffil*>(source);
    if (pub_source)
    {
        swap(m_affil, pub_source->m_affil);
    }
    IEditingActionDescPub::SwapContext(source);
}

void IEditingActionDescAffil::Modify(EActionType action)
{
    if (m_EditedDesc->GetPub().IsSetPub() && m_EditedDesc->GetPub().GetPub().IsSet())
    {
        for (CPub_equiv::Tdata::iterator pub = m_EditedDesc->SetPub().SetPub().Set().begin(); pub != m_EditedDesc->SetPub().SetPub().Set().end(); ++pub)
        {
            m_pub = *pub;
            if (m_pub->IsSetAuthors())
            {
                const CAuth_list &auth_list =  m_pub->GetAuthors();
                if (auth_list.IsSetAffil())
                {
                    m_affil = &(const_cast<CAuth_list&>(auth_list).SetAffil());
                    IEditingActionDesc::Modify(action);
                }
                else
                {
                    m_affil = NULL;
                    IEditingActionDesc::Modify(action);
                }
            }
        }
    }  
}

void IEditingActionDescAffil::AddAuthList(CRef<CAuth_list> auth_list)
{
    switch (m_pub->Which()) 
    {
    case CPub::e_Gen:
        m_pub->SetGen().SetAuthors().Assign(*auth_list);
        break;
    case CPub::e_Sub:
        m_pub->SetSub().SetAuthors().Assign(*auth_list);
        break;
    case CPub::e_Article:
        m_pub->SetArticle().SetAuthors().Assign(*auth_list);
        break;
    case CPub::e_Book:
        m_pub->SetBook().SetAuthors().Assign(*auth_list);
        break;
    case CPub::e_Proc:
        m_pub->SetProc().SetBook().SetAuthors().Assign(*auth_list);
        break;
    case CPub::e_Patent:
        m_pub->SetPatent().SetAuthors().Assign(*auth_list);
        break;
    case CPub::e_Man:
        m_pub->SetMan().SetCit().SetAuthors().Assign(*auth_list);
        break;
    case CPub::e_Medline:
    case CPub::e_Journal:
    case CPub::e_Pat_id:
    case CPub::e_Equiv:
    case CPub::e_Pmid:
    default:
        // do nothing
        break;
    }
}

CEditingActionDescAffil::CEditingActionDescAffil(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescAffil(seh, submit, "CEditingActionDescAffil")
{
}

bool CEditingActionDescAffil::IsSetValue()
{
    return m_affil && (m_affil->IsStr() || (m_affil->IsStd() && m_affil->GetStd().IsSetAffil()));
}

void CEditingActionDescAffil::SetValue(const string &value)
{
    if (m_affil && m_affil->IsStr())
    {
        m_affil->SetStr(value);
    }
    else if (m_affil && m_affil->IsStd() && m_affil->GetStd().IsSetAffil())
    {
        m_affil->SetStd().SetAffil(value);
    }
    else 
    {
        CRef<CAuth_list> auth_list(new CAuth_list());
        auth_list->Assign(m_pub->GetAuthors());
        auth_list->SetAffil().SetStd().SetAffil(value);
        AddAuthList(auth_list);
    }
}

string CEditingActionDescAffil::GetValue()
{
    if (m_affil->IsStr())
    {
        return m_affil->GetStr();
    }
    else if (m_affil->IsStd() && m_affil->GetStd().IsSetAffil())
    {
        return m_affil->GetStd().GetAffil();
    }
    return kEmptyStr;
}

void CEditingActionDescAffil::ResetValue()
{
    if (m_affil->IsStr())
    {
        m_affil->Reset();
    }
    else if (m_affil->IsStd() && m_affil->GetStd().IsSetAffil())
    {
        m_affil->SetStd().ResetAffil();
    }
}

CEditingActionDescDepartment::CEditingActionDescDepartment(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescAffil(seh, submit, "CEditingActionDescDepartment")
{
}

bool CEditingActionDescDepartment::IsSetValue()
{
    return m_affil && m_affil->IsStd() && m_affil->GetStd().IsSetDiv();
}

void CEditingActionDescDepartment::SetValue(const string &value)
{
   if (m_affil && m_affil->IsStd() && m_affil->GetStd().IsSetDiv())
   {
       m_affil->SetStd().SetDiv(value);
   }
   else 
   {
       CRef<CAuth_list> auth_list(new CAuth_list());
       auth_list->Assign(m_pub->GetAuthors());
       auth_list->SetAffil().SetStd().SetDiv(value);
       AddAuthList(auth_list);
   }
}

string CEditingActionDescDepartment::GetValue()
{
    return m_affil->GetStd().GetDiv();
}

void CEditingActionDescDepartment::ResetValue()
{
    m_affil->SetStd().ResetDiv();
}

CEditingActionDescStreet::CEditingActionDescStreet(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescAffil(seh, submit, "CEditingActionDescStreet")
{
}

bool CEditingActionDescStreet::IsSetValue()
{
    return m_affil && m_affil->IsStd() && m_affil->GetStd().IsSetStreet();
}

void CEditingActionDescStreet::SetValue(const string &value)
{
   if (m_affil && m_affil->IsStd() && m_affil->GetStd().IsSetStreet())
   {
       m_affil->SetStd().SetStreet(value);
   }
   else 
   {
       CRef<CAuth_list> auth_list(new CAuth_list());
       auth_list->Assign(m_pub->GetAuthors());
       auth_list->SetAffil().SetStd().SetStreet(value);
       AddAuthList(auth_list);
   }
}

string CEditingActionDescStreet::GetValue()
{
    return m_affil->GetStd().GetStreet();
}

void CEditingActionDescStreet::ResetValue()
{
    m_affil->SetStd().ResetStreet();
}

CEditingActionDescCity::CEditingActionDescCity(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescAffil(seh, submit, "CEditingActionDescCity")
{
}

bool CEditingActionDescCity::IsSetValue()
{
    return m_affil && m_affil->IsStd() && m_affil->GetStd().IsSetCity();
}

void CEditingActionDescCity::SetValue(const string &value)
{
   if (m_affil && m_affil->IsStd() && m_affil->GetStd().IsSetCity())
   {
       m_affil->SetStd().SetCity(value);
   }
   else 
   {
       CRef<CAuth_list> auth_list(new CAuth_list());
       auth_list->Assign(m_pub->GetAuthors());
       auth_list->SetAffil().SetStd().SetCity(value);
       AddAuthList(auth_list);
   }
}

string CEditingActionDescCity::GetValue()
{
    return m_affil->GetStd().GetCity();
}

void CEditingActionDescCity::ResetValue()
{
    m_affil->SetStd().ResetCity();
}

CEditingActionDescState::CEditingActionDescState(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescAffil(seh, submit, "CEditingActionDescState")
{
}

bool CEditingActionDescState::IsSetValue()
{
    return m_affil && m_affil->IsStd() && m_affil->GetStd().IsSetSub();
}

void CEditingActionDescState::SetValue(const string &value)
{
   if (m_affil && m_affil->IsStd() && m_affil->GetStd().IsSetSub())
   {
       m_affil->SetStd().SetSub(value);
   }
   else 
   {
       CRef<CAuth_list> auth_list(new CAuth_list());
       auth_list->Assign(m_pub->GetAuthors());
       auth_list->SetAffil().SetStd().SetSub(value);
       AddAuthList(auth_list);
   }
}

string CEditingActionDescState::GetValue()
{
    return m_affil->GetStd().GetSub();
}

void CEditingActionDescState::ResetValue()
{
    m_affil->SetStd().ResetSub();
}

CEditingActionDescCountry::CEditingActionDescCountry(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescAffil(seh, submit, "CEditingActionDescCountry")
{
}

bool CEditingActionDescCountry::IsSetValue()
{
    return m_affil && m_affil->IsStd() && m_affil->GetStd().IsSetCountry();
}

void CEditingActionDescCountry::SetValue(const string &value)
{
   if (m_affil && m_affil->IsStd() && m_affil->GetStd().IsSetCountry())
   {
       m_affil->SetStd().SetCountry(value);
   }
   else 
   {
       CRef<CAuth_list> auth_list(new CAuth_list());
       auth_list->Assign(m_pub->GetAuthors());
       auth_list->SetAffil().SetStd().SetCountry(value);
       AddAuthList(auth_list);
   }
}

string CEditingActionDescCountry::GetValue()
{
    return m_affil->GetStd().GetCountry();
}

void CEditingActionDescCountry::ResetValue()
{
    m_affil->SetStd().ResetCountry();
}

CEditingActionDescEmail::CEditingActionDescEmail(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescAffil(seh, submit, "CEditingActionDescEmail")
{
}

bool CEditingActionDescEmail::IsSetValue()
{
    return m_affil && m_affil->IsStd() && m_affil->GetStd().IsSetEmail();
}

void CEditingActionDescEmail::SetValue(const string &value)
{
   if (m_affil && m_affil->IsStd() && m_affil->GetStd().IsSetEmail())
   {
       m_affil->SetStd().SetEmail(value);
   }
   else 
   {
       CRef<CAuth_list> auth_list(new CAuth_list());
       auth_list->Assign(m_pub->GetAuthors());
       auth_list->SetAffil().SetStd().SetEmail(value);
       AddAuthList(auth_list);
   }
}

string CEditingActionDescEmail::GetValue()
{
    return m_affil->GetStd().GetEmail();
}

void CEditingActionDescEmail::ResetValue()
{
    m_affil->SetStd().ResetEmail();
}

IEditingActionDescAuthors::IEditingActionDescAuthors(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit, const string &name)
    : IEditingActionDescPub(seh, submit, name)
{
}


void IEditingActionDescAuthors::SwapContext(IEditingAction* source)
{
    IEditingActionDescAuthors *pub_source = dynamic_cast<IEditingActionDescAuthors*>(source);
    if (pub_source)
    {
        swap(m_author, pub_source->m_author);
    }
    IEditingActionDescPub::SwapContext(source);
}

void IEditingActionDescAuthors::Modify(EActionType action)
{
    if (IsTo(action) && m_author && dynamic_cast<IEditingActionDescAuthors*>(m_Other) != NULL)
    {
	IEditingActionDesc::Modify(action);
	return;
    }
    if (m_EditedDesc->GetPub().IsSetPub() && m_EditedDesc->GetPub().GetPub().IsSet())
    {
        for (CPub_equiv::Tdata::iterator pub = m_EditedDesc->SetPub().SetPub().Set().begin(); pub != m_EditedDesc->SetPub().SetPub().Set().end(); ++pub)
        {
            m_pub = *pub;
            if (m_pub->IsSetAuthors())
            {
                CAuth_list &auth_list =  const_cast<CAuth_list&>(m_pub->GetAuthors());
                EDIT_EACH_AUTHOR_ON_AUTHLIST(author_it, auth_list)
                {
                    m_author = *author_it;
                    IEditingActionDesc::Modify(action);
                }               
            }
        }
    }
}

CEditingActionDescAuthorFirstName::CEditingActionDescAuthorFirstName(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescAuthors(seh, submit, "CEditingActionDescAuthorFirstName")
{
}

bool CEditingActionDescAuthorFirstName::IsSetValue()
{
    return m_author && m_author->IsSetName() && m_author->GetName().IsName() && m_author->GetName().GetName().IsSetFirst();
}

void CEditingActionDescAuthorFirstName::SetValue(const string &value)
{
    if (m_author)
    {
        // Fix initials
        string orig_first_init;
        if (m_author->IsSetName() && m_author->GetName().IsName() && m_author->GetName().GetName().IsSetFirst()) 
        {
            orig_first_init = macro::CMacroFunction_AuthorFix::s_GetFirstNameInitials(m_author->GetName().GetName().GetFirst());
        }
        string new_first_init = macro::CMacroFunction_AuthorFix::s_GetFirstNameInitials(value);
        string initials;
        if (m_author->IsSetName() && m_author->GetName().IsName() && m_author->GetName().GetName().IsSetInitials()) 
        {
            initials = m_author->GetName().GetName().GetInitials();
            initials = macro::CMacroFunction_AuthorFix::s_InsertInitialPeriods(initials);
        }
        if (NStr::StartsWith(initials, orig_first_init)) 
        {
            initials = initials.substr(orig_first_init.length());
        } 
        initials = new_first_init + initials;
        if (!NStr::EndsWith(initials, ".")) 
        {
            initials = initials + ".";
        }
        m_author->SetName().SetName().SetInitials(initials);


        m_author->SetName().SetName().SetFirst(value);
    }  
}

string CEditingActionDescAuthorFirstName::GetValue()
{
    return m_author->GetName().GetName().GetFirst();
}

void CEditingActionDescAuthorFirstName::ResetValue()
{
    m_author->SetName().SetName().ResetFirst();
}

CEditingActionDescAuthorLastName::CEditingActionDescAuthorLastName(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescAuthors(seh, submit, "CEditingActionDescAuthorLastName")
{
}

bool CEditingActionDescAuthorLastName::IsSetValue()
{
    return m_author && m_author->IsSetName() && m_author->GetName().IsName() && m_author->GetName().GetName().IsSetLast();
}

void CEditingActionDescAuthorLastName::SetValue(const string &value)
{
    if (m_author)
    {
        m_author->SetName().SetName().SetLast(value);
    }  
}

string CEditingActionDescAuthorLastName::GetValue()
{
    return m_author->GetName().GetName().GetLast();
}

void CEditingActionDescAuthorLastName::ResetValue()
{
    m_author->SetName().SetName().ResetLast();
}

CEditingActionDescAuthorSuffix::CEditingActionDescAuthorSuffix(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescAuthors(seh, submit, "CEditingActionDescAuthorSuffix")
{
}

bool CEditingActionDescAuthorSuffix::IsSetValue()
{
    return m_author && m_author->IsSetName() && m_author->GetName().IsName() && m_author->GetName().GetName().IsSetSuffix();
}

void CEditingActionDescAuthorSuffix::SetValue(const string &value)
{
    if (m_author)
    {
        m_author->SetName().SetName().SetSuffix(value);
    }  
}

string CEditingActionDescAuthorSuffix::GetValue()
{
    return m_author->GetName().GetName().GetSuffix();
}

void CEditingActionDescAuthorSuffix::ResetValue()
{
    m_author->SetName().SetName().ResetSuffix();
}

CEditingActionDescAuthorConsortium::CEditingActionDescAuthorConsortium(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescAuthors(seh, submit, "CEditingActionDescAuthorConsortium")
{
}

bool CEditingActionDescAuthorConsortium::IsSetValue()
{
    return m_author && m_author->IsSetName() && m_author->GetName().IsConsortium();
}

void CEditingActionDescAuthorConsortium::SetValue(const string &value)
{
    if (m_author)
    {
        m_author->SetName().SetConsortium(value);
    }  
}

string CEditingActionDescAuthorConsortium::GetValue()
{
    return m_author->GetName().GetConsortium();
}

void CEditingActionDescAuthorConsortium::ResetValue()
{
    m_author->ResetName();
}

CEditingActionDescAuthorInitials::CEditingActionDescAuthorInitials(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescAuthors(seh, submit, "CEditingActionDescAuthorInitials")
{
}

bool CEditingActionDescAuthorInitials::IsSetValue()
{
    return m_author && m_author->IsSetName() && m_author->GetName().IsName() && m_author->GetName().GetName().IsSetInitials();
}

void CEditingActionDescAuthorInitials::SetValue(const string &value)
{
    if (m_author)
    {
        m_author->SetName().SetName().SetInitials(value);
    }  
}

string CEditingActionDescAuthorInitials::GetValue()
{
    string initials = m_author->GetName().GetName().GetInitials();
    initials = macro::CMacroFunction_AuthorFix::s_InsertInitialPeriods(initials);
    if (m_author->GetName().GetName().IsSetFirst()) 
    {
        string first_init = macro::CMacroFunction_AuthorFix::s_GetFirstNameInitials(m_author->GetName().GetName().GetFirst());
        if (NStr::StartsWith(initials, first_init)) 
        {
            initials = initials.substr(first_init.length());
        }
    }
    return initials;
}

void CEditingActionDescAuthorInitials::ResetValue()
{
    m_author->SetName().SetName().ResetInitials();
}

IEditingActionDescJournal::IEditingActionDescJournal(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit, const string &name)
    : IEditingActionDescPub(seh, submit, name), m_imprint(NULL), m_citgen(NULL)
{
}


void IEditingActionDescJournal::Modify(EActionType action)
{
    if (m_EditedDesc->GetPub().IsSetPub() && m_EditedDesc->GetPub().GetPub().IsSet())
    {
        for (CPub_equiv::Tdata::iterator pub = m_EditedDesc->SetPub().SetPub().Set().begin(); pub != m_EditedDesc->SetPub().SetPub().Set().end(); ++pub)
        {
            m_pub = *pub;
            m_imprint = NULL;
            m_citgen = NULL;
            GetImprintOrCitgen(*m_pub);
            IEditingActionDesc::Modify(action);
        }
    }
}

void IEditingActionDescJournal::GetImprintOrCitgen(CPub& pub)
{
    switch (pub.Which()) {
        case CPub::e_Article:
            if (pub.GetArticle().IsSetFrom()) {
                switch (pub.GetArticle().GetFrom().Which()) {
                    case CCit_art::TFrom::e_Book:
                        if (pub.GetArticle().GetFrom().GetBook().IsSetImp()) {
                            m_imprint = &(pub.SetArticle().SetFrom().SetBook().SetImp());
                        }
                        break;
                    case CCit_art::TFrom::e_Journal:
                        if (pub.GetArticle().GetFrom().GetJournal().IsSetImp()) {
                            m_imprint = &(pub.SetArticle().SetFrom().SetJournal().SetImp());
                        }
                        break;
                    case CCit_art::TFrom::e_Proc:
                        if (pub.GetArticle().GetFrom().GetProc().IsSetBook()
                            && pub.GetArticle().GetFrom().GetProc().GetBook().IsSetImp()) {
                            m_imprint = &(pub.SetArticle().SetFrom().SetProc().SetBook().SetImp());
                        }
                        break;
                    default:
                        break;
                }
            }
            break;
        case CPub::e_Book:
            if (pub.GetBook().IsSetImp()) {
                m_imprint = &(pub.SetBook().SetImp());
            }
            break;
        case CPub::e_Gen:
            // Cit-gen has no imprint
            m_citgen = &pub.SetGen();
            break;
        case CPub::e_Journal:
            if (pub.GetJournal().IsSetImp()) {
                m_imprint = &(pub.SetJournal().SetImp());
            }
            break;
        case CPub::e_Man:
            if (pub.GetMan().IsSetCit() && pub.GetMan().GetCit().IsSetImp()) {
                m_imprint = &(pub.SetMan().SetCit().SetImp());
            }
            break;
        case CPub::e_Patent:
            // Cit-pat has no imprint
            break;
        case CPub::e_Proc:
            if (pub.GetProc().IsSetBook() && pub.GetProc().GetBook().IsSetImp()) {
                m_imprint = &(pub.SetProc().SetBook().SetImp());
            }
            break;
        case CPub::e_Sub:
            // Cit-sub has no imprint // ???
            if (pub.GetSub().IsSetImp())
            {
                m_imprint = &(pub.SetSub().SetImp());
            }
            break;
        case CPub::e_Medline:
            // no imprint
            break;
        case CPub::e_Muid:
            // no imprint
            break;
        case CPub::e_Pat_id:
            // no imprint
            break;
        case CPub::e_Pmid:
            // no imprint
            break;
        case CPub::e_Equiv:
            NON_CONST_ITERATE(CPub::TEquiv::Tdata, it, pub.SetEquiv().Set()) 
            {
                GetImprintOrCitgen(**it);
                if (m_imprint || m_citgen) 
                {
                    break;
                }
            }
            break;
        case CPub::e_not_set:
        // no imprint
            break;
    }
}

CEditingActionDescJournal::CEditingActionDescJournal(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescJournal(seh, submit, "CEditingActionDescJournal")
{
}

bool CEditingActionDescJournal::IsSetValue()
{
    return m_pub && ((m_pub->IsGen() && m_pub->GetGen().IsSetJournal()) || 
                     (m_pub->IsArticle() && m_pub->GetArticle().IsSetFrom() && m_pub->GetArticle().GetFrom().IsJournal() && m_pub->GetArticle().GetFrom().GetJournal().IsSetTitle()) ||
                     (m_pub->IsJournal() && m_pub->GetJournal().IsSetTitle()));
}

void CEditingActionDescJournal::SetValue(const string &value)
{
    CTitle *title(NULL);
    if (m_pub->IsGen())
        title = &m_pub->SetGen().SetJournal();
    if (m_pub->IsArticle())
        title = &m_pub->SetArticle().SetFrom().SetJournal().SetTitle();
    if (m_pub->IsJournal())
        title = &m_pub->SetJournal().SetTitle();

    if (title)
    {
        bool found = false;
        if (title->IsSet())
        {
            NON_CONST_ITERATE(CTitle::Tdata, it, title->Set()) 
            {
                if ((*it)->IsName()) 
                {
                    (*it)->SetName(value);
                    found = true;
                }
            }
            if (!found) 
            {
                // if we find a not_set, change it to a Name
                NON_CONST_ITERATE(CTitle::Tdata, it, title->Set()) 
                {
                    if ((*it)->Which() == CTitle::C_E::e_not_set) 
                    {
                        (*it)->SetName(value);
                        found = true;
                    }
                }
            }
        }

        // otherwise add a Name
        if (!found) 
        {
            CRef<CTitle::C_E> this_title(new CTitle::C_E());
            this_title->SetName(value);
            title->Set().push_back(this_title); 
        }
    }
}

string CEditingActionDescJournal::GetValue()
{
    const CTitle *title(NULL);
    if (m_pub->IsGen() && m_pub->GetGen().IsSetJournal())
        title = &m_pub->GetGen().GetJournal();
    if (m_pub->IsArticle() && m_pub->GetArticle().IsSetFrom() && m_pub->GetArticle().GetFrom().IsJournal() && m_pub->GetArticle().GetFrom().GetJournal().IsSetTitle())
        title = &m_pub->GetArticle().GetFrom().GetJournal().GetTitle();
    if (m_pub->IsJournal() && m_pub->GetJournal().IsSetTitle())
        title = &m_pub->GetJournal().GetTitle();

    string rval;
    if (title)
    {
        try 
        {
            rval = title->GetTitle(CTitle::C_E::e_Name);
        } 
        catch (exception &) {rval.clear();}       
    }
    return rval;
}

void CEditingActionDescJournal::ResetValue()
{
   if (m_pub->IsGen() && m_pub->GetGen().IsSetJournal())
        m_pub->SetGen().ResetJournal();
    if (m_pub->IsArticle() && m_pub->GetArticle().IsSetFrom() && m_pub->GetArticle().GetFrom().IsJournal() && m_pub->GetArticle().GetFrom().GetJournal().IsSetTitle())
        m_pub->SetArticle().SetFrom().SetJournal().ResetTitle();
    if (m_pub->IsJournal() && m_pub->GetJournal().IsSetTitle())
        m_pub->SetJournal().ResetTitle();  
}


CEditingActionDescVolume::CEditingActionDescVolume(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescJournal(seh, submit, "CEditingActionDescVolume")
{
}

bool CEditingActionDescVolume::IsSetValue()
{
    return (m_imprint && m_imprint->IsSetVolume()) || (m_citgen && m_citgen->IsSetVolume());
}

void CEditingActionDescVolume::SetValue(const string &value)
{
    if (m_imprint)
    {
        m_imprint->SetVolume(value);
    }  
    if (m_citgen)
    {
        m_citgen->SetVolume(value);
    }  
}

string CEditingActionDescVolume::GetValue()
{
    if (m_imprint)
        return m_imprint->GetVolume();
    if (m_citgen)
        return m_citgen->GetVolume();
    return kEmptyStr;
}

void CEditingActionDescVolume::ResetValue()
{
    if (m_imprint)
        m_imprint->ResetVolume();
    if (m_citgen)
        m_citgen->ResetVolume();
}

CEditingActionDescIssue::CEditingActionDescIssue(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescJournal(seh, submit, "CEditingActionDescIssue")
{
}

bool CEditingActionDescIssue::IsSetValue()
{
    return (m_imprint && m_imprint->IsSetIssue()) || (m_citgen && m_citgen->IsSetIssue());
}

void CEditingActionDescIssue::SetValue(const string &value)
{
    if (m_imprint)
    {
        m_imprint->SetIssue(value);
    }  
    if (m_citgen)
    {
        m_citgen->SetIssue(value);
    }  
}

string CEditingActionDescIssue::GetValue()
{
    if (m_imprint)
        return m_imprint->GetIssue();
    if (m_citgen)
        return m_citgen->GetIssue();
    return kEmptyStr;
}

void CEditingActionDescIssue::ResetValue()
{
    if (m_imprint)
        m_imprint->ResetIssue();
    if (m_citgen)
        m_citgen->ResetIssue();
}

CEditingActionDescPages::CEditingActionDescPages(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescJournal(seh, submit, "CEditingActionDescPages")
{
}

bool CEditingActionDescPages::IsSetValue()
{
    return (m_imprint && m_imprint->IsSetPages()) || (m_citgen && m_citgen->IsSetPages());
}

void CEditingActionDescPages::SetValue(const string &value)
{
    if (m_imprint)
    {
        m_imprint->SetPages(value);
    }  
    if (m_citgen)
    {
        m_citgen->SetPages(value);
    }  
}

string CEditingActionDescPages::GetValue()
{
    if (m_imprint)
        return m_imprint->GetPages();
    if (m_citgen)
        return m_citgen->GetPages();
    return kEmptyStr;
}

void CEditingActionDescPages::ResetValue()
{
    if (m_imprint)
        m_imprint->ResetPages();
    if (m_citgen)
        m_citgen->ResetPages();
}

CEditingActionDescStatus::CEditingActionDescStatus(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescJournal(seh, submit, "CEditingActionDescStatus")
{
}

bool CEditingActionDescStatus::IsSetValue()
{
    return m_imprint || m_citgen || m_pub->IsSub() || m_pub->IsPatent();
}

void CEditingActionDescStatus::SetValue(const string &value)
{
    if (m_imprint)
    {
        if (NStr::EqualNocase(value, "unpublished")) 
            m_imprint->SetPrepub(CImprint::ePrepub_other);
        else if (NStr::EqualNocase(value, "published"))
            m_imprint->ResetPrepub();
        else if (NStr::EqualNocase(value, "In Press"))
            m_imprint->SetPrepub(CImprint::ePrepub_in_press);
    }  
    if (m_citgen)
    {
        if (NStr::EqualNocase(value, "unpublished")) 
        {
            m_citgen->SetCit("unpublished");
        } 
        else if (NStr::EqualNocase(value, "published")) 
        {
            m_citgen->ResetCit();
        }
    }  
}

string CEditingActionDescStatus::GetValue()
{
    string status = "Unpublished";
    if (m_imprint)
    {
        if (m_imprint->IsSetPrepub()) 
        {
            if (m_imprint->GetPrepub() == CImprint::ePrepub_in_press) 
            {
                status = "In Press";
            } 
            else 
            {
                status = "Unpublished";
            }
        } 
        else 
        {
            status = "Published";
        }
    }

    if (m_citgen)
    {
        if (m_citgen->IsSetCit() && NStr::EqualNocase(m_citgen->GetCit(), "unpublished")) 
        {
            status = "Unpublished";
            
        } 
        else 
        {
            status = "Published";
        }
    }

    if (m_pub->IsSub())
    {
        status = "Submitter Block";
    }

    if (m_pub->IsPatent())
    {
        status = "Published";
    }

    return status;
}

void CEditingActionDescStatus::ResetValue()
{
    if (m_imprint)
        m_imprint->ResetPrepub();
    if (m_citgen)
        m_citgen->ResetCit();
}

CEditingActionDescAuthorNameList::CEditingActionDescAuthorNameList(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDescPub(seh, submit, "CEditingActionDescAuthorNameList"), m_auth_list(NULL)
{
}

void CEditingActionDescAuthorNameList::SwapContext(IEditingAction* source)
{
    CEditingActionDescAuthorNameList *pub_source = dynamic_cast<CEditingActionDescAuthorNameList*>(source);
    if (pub_source)
    {
        swap(m_auth_list, pub_source->m_auth_list);
    }
    IEditingActionDescPub::SwapContext(source);
}

void CEditingActionDescAuthorNameList::Modify(EActionType action)
{
    if (m_EditedDesc->GetPub().IsSetPub() && m_EditedDesc->GetPub().GetPub().IsSet())
    {
        for (CPub_equiv::Tdata::iterator pub = m_EditedDesc->SetPub().SetPub().Set().begin(); pub != m_EditedDesc->SetPub().SetPub().Set().end(); ++pub)
        {
            m_pub = *pub;
            if (m_pub->IsSetAuthors())
            {
                CAuth_list &auth_list =  const_cast<CAuth_list&>(m_pub->GetAuthors());
                m_auth_list = &auth_list;
                IEditingActionDesc::Modify(action);
            }
        }
    }
}

bool CEditingActionDescAuthorNameList::IsSetValue()
{
    return m_auth_list && m_auth_list->IsSetNames();
}

void CEditingActionDescAuthorNameList::SetValue(const string &value)
{
    if (!m_auth_list)
        return;
    bool rval(false);
    list<string>  arr;
    NStr::Split(value, ",", arr, NStr::fSplit_Tokenize);
    m_auth_list->ResetNames();
    for (list<string>::iterator name = arr.begin(); name != arr.end(); ++name)
    {
        vector<string> full_name;
        NStr::Split(*name, " ", full_name, NStr::fSplit_Tokenize);            
        string first_name, last_name, middle_init, suffix;
        if (full_name.size() == 4)
        {
            first_name = full_name[0];
            string middle = full_name[1];
            last_name = full_name[2];
            suffix = full_name[3];
            middle_init = edit::GetFirstInitial(first_name, true) + edit::GetFirstInitial(middle, false);
            rval = true;
        }
        else if (full_name.size() == 3)
        {
            first_name = full_name[0];
            string middle = full_name[1];
            last_name = full_name[2];
            middle_init = edit::GetFirstInitial(first_name, true) + edit::GetFirstInitial(middle, false);
            rval = true;       
        }
        else if (full_name.size() == 2)
        {
            first_name = full_name[0];
            last_name = full_name[1];
            middle_init = edit::GetFirstInitial(first_name, true);
            rval = true;
        }
        else if (full_name.size() == 1)
        {
            last_name = full_name[0];
            rval = true;
        }
        if (rval)
        {
            CRef<CAuthor> new_auth( new CAuthor());
            new_auth->SetName().SetName().SetLast(last_name);
            if (!first_name.empty())
                new_auth->SetName().SetName().SetFirst(first_name);
            if (!middle_init.empty())
                    new_auth->SetName().SetName().SetInitials(middle_init);
            if (!suffix.empty())
                new_auth->SetName().SetName().SetSuffix(suffix);
            m_auth_list->SetNames().SetStd().push_back(new_auth);
        }
    }
}

string CEditingActionDescAuthorNameList::GetValue()
{
    if (!m_auth_list)
        return kEmptyStr;
    string name_list;
    EDIT_EACH_AUTHOR_ON_AUTHLIST(author_it, *m_auth_list)
    {
        string first_name, last_name, middle_init, suffix;
        if ((*author_it)->IsSetName() && (*author_it)->GetName().IsName() && (*author_it)->GetName().GetName().IsSetFirst())
            first_name = (*author_it)->GetName().GetName().GetFirst();
        if ((*author_it)->IsSetName() && (*author_it)->GetName().IsName() && (*author_it)->GetName().GetName().IsSetLast())
            last_name = (*author_it)->GetName().GetName().GetLast();
        if ((*author_it)->IsSetName() && (*author_it)->GetName().IsName() && (*author_it)->GetName().GetName().IsSetMiddle())
            middle_init = (*author_it)->GetName().GetName().GetMiddle();
        if ((*author_it)->IsSetName() && (*author_it)->GetName().IsName() && (*author_it)->GetName().GetName().IsSetSuffix())
            suffix = (*author_it)->GetName().GetName().GetSuffix();
        if (middle_init.empty() && (*author_it)->IsSetName() && (*author_it)->GetName().IsName() && (*author_it)->GetName().GetName().IsSetInitials())
        {
            string initials = (*author_it)->GetName().GetName().GetInitials();
            initials = macro::CMacroFunction_AuthorFix::s_InsertInitialPeriods(initials);
            if (!first_name.empty()) 
            {
                string first_init = macro::CMacroFunction_AuthorFix::s_GetFirstNameInitials(first_name);
                if (NStr::StartsWith(initials, first_init)) 
                {
                    initials = initials.substr(first_init.length());
                }
            }
            middle_init = initials;
        }
        if (name_list.empty())
            name_list = first_name + " " + middle_init + " " + last_name + " " + suffix;
        else
            name_list += ", " + first_name + " " + middle_init + " " + last_name + " " + suffix;
    }
    NStr::TruncateSpacesInPlace(name_list);
    NStr::ReplaceInPlace(name_list, "  ", " ");
    NStr::ReplaceInPlace(name_list, " ,", ",");
    return name_list;
}

void CEditingActionDescAuthorNameList::ResetValue()
{
    m_auth_list->ResetNames();
}

CEditingActionDescLocalId::CEditingActionDescLocalId(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDesc(seh, submit, "CEditingActionDescLocalId",  CSeqdesc::e_User), m_type("OriginalID"), m_field("LocalId")
{
}

void CEditingActionDescLocalId::Find(EActionType action)
{
    if (!m_TopSeqEntry)
        return;
    size_t count = 0;
    set<const CSeqdesc*> visited;
    for (CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na); b_iter; ++b_iter)
    {
        m_Desc = NULL;
        m_EditedDesc.Reset();
        CBioseq_Handle bsh = *b_iter; 
        m_bsh = bsh;
        m_CurrentSeqEntry = bsh.GetSeq_entry_Handle();
        if (m_constraint->Match(bsh))
        {
            bool found = false;
            CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
            CSeq_entry_Handle found_seh;
            for (CSeqdesc_CI desc_ci( bsh, m_subtype); desc_ci; ++desc_ci) 
            {
                if (desc_ci->GetUser().IsSetType() && desc_ci->GetUser().GetType().IsStr()
                    && NStr::EqualNocase(desc_ci->GetUser().GetType().GetStr(), m_type) )
                {
                    found = true;
                    m_Desc = &*desc_ci;
		    if (visited.find(m_Desc) != visited.end())
			continue;
		    visited.insert(m_Desc);
                    seh = desc_ci.GetSeq_entry_Handle();
                    if (IsFrom(action) && found_seh && seh != found_seh)
                        break;
                    //m_CurrentSeqEntry = seh;
                    if (m_constraint->Match(m_Desc))
                    {
                        if (m_ChangedDescriptors.find(m_Desc) != m_ChangedDescriptors.end())
                            m_EditedDesc = m_ChangedDescriptors[m_Desc];
                        else
                        {
                            m_EditedDesc.Reset(new CSeqdesc);
                            m_EditedDesc->Assign(*m_Desc);
                        }
                        m_delete = false;
                        m_DescContext = seh;
                        Modify(action);                       
                        found_seh = seh;
                    }
                }                
            }

            if (!found && !IsNOOP(action))
            {
                m_EditedDesc.Reset();
                m_CurrentSeqEntry = bsh.GetSeq_entry_Handle();
                Modify(action);
            }
        }
        ++count;
        if (count >= m_max_records)
            break; 
    }
}

void CEditingActionDescLocalId::FindRelated(EActionType action)
{
    Find(action);   
}

void CEditingActionDescLocalId::Modify(EActionType action)
{  
    m_bioseq_value = NULL;
    m_str_value = NULL;
    m_int_value = NULL;
    if (!m_EditedDesc)
    {
        if (m_CurrentSeqEntry.IsSeq())
        {
            const CBioseq *bioseq = m_CurrentSeqEntry.GetSeq().GetBioseqCore().GetPointer();
            if (bioseq && bioseq->IsSetId()) 
            {
                m_bioseq_value = bioseq->GetLocalId();    
                IEditingActionDesc::Modify(action);          
            }      
        }
        return;
    }
    bool found = false;

    EDIT_EACH_USERFIELD_ON_USEROBJECT(field_iter, m_EditedDesc->SetUser())
    {
        CUser_field& field = **field_iter;
        if (field.GetLabel().IsStr() && NStr::Equal(field.GetLabel().GetStr(), m_field) && field.IsSetData())
        {
            m_bioseq_value = NULL;
            m_str_value = NULL;
            m_int_value = NULL;
            switch (field.GetData().Which())
            {
            case CUser_field::TData::e_Str :
            {
                found = true;
                m_str_value = &field.SetData().SetStr();
                IEditingActionDesc::Modify(action);
                break;
            }
            case  CUser_field::TData::e_Int :
            {
                found = true;
                m_int_value = &field.SetData().SetInt();
                IEditingActionDesc::Modify(action);
                break;
            }           
            case CUser_field::TData::e_Strs :
            {
                NON_CONST_ITERATE(CUser_field::TData::TStrs, it, field.SetData().SetStrs())
                {
                    found = true;
                    m_str_value = &*it;
                    IEditingActionDesc::Modify(action);                   
                }
                break;
            }
            case CUser_field::TData::e_Ints :
            {
                NON_CONST_ITERATE(CUser_field::TData::TInts, it, field.SetData().SetInts())
                {
                    found = true;
                    m_int_value = &*it;
                    IEditingActionDesc::Modify(action);                   
                }
                break;
            }          
            default : break;
            }
        }
    }
}

bool CEditingActionDescLocalId::IsSetValue()
{
    return m_str_value != NULL ||  m_int_value != NULL || m_bioseq_value != NULL;
}

void CEditingActionDescLocalId::SetValue(const string &value)
{
    // nothing here
}

string CEditingActionDescLocalId::GetValue()
{
    if (m_str_value)
        return *m_str_value;
    if (m_int_value)
        return NStr::IntToString(*m_int_value);   
    if (m_bioseq_value)
    {
        string value;
        m_bioseq_value->GetLabel(&value, CSeq_id::eContent);
        return value;
    }
    return kEmptyStr;
}

void CEditingActionDescLocalId::ResetValue()
{
// nothing here
}


CEditingActionDescComment::CEditingActionDescComment(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
    : IEditingActionDesc(seh, submit, "CEditingActionDescComment",  CSeqdesc::e_User), m_type("Submission"), m_field("AdditionalComment")
{
}

void CEditingActionDescComment::Find(EActionType action)
{
    if (!m_TopSeqEntry)
        return;
    size_t count = 0;
    set<const CSeqdesc*> visited;
    for (CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na); b_iter; ++b_iter)
    {
        m_Desc = NULL;
        m_EditedDesc.Reset();
        CBioseq_Handle bsh = *b_iter; 
        m_bsh = bsh;
        m_CurrentSeqEntry = bsh.GetSeq_entry_Handle();
        if (m_constraint->Match(bsh))
        {
            CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
            CSeq_entry_Handle found_seh;
            for (CSeqdesc_CI desc_ci( bsh); desc_ci; ++desc_ci) 
                if (desc_ci->Which() == CSeqdesc::e_Comment || desc_ci->Which() == CSeqdesc::e_User)
            {
                if ((desc_ci->IsUser() && desc_ci->GetUser().IsSetType() && desc_ci->GetUser().GetType().IsStr()
                     && NStr::EqualNocase(desc_ci->GetUser().GetType().GetStr(), m_type) )  ||
                    desc_ci->IsComment())
                {
                    m_Desc = &*desc_ci;
		    if (visited.find(m_Desc) != visited.end())
			continue;
		    visited.insert(m_Desc);
                    seh = desc_ci.GetSeq_entry_Handle();
                    if (IsFrom(action) && found_seh && seh != found_seh)
                        break;
                    //m_CurrentSeqEntry = seh;
                    if (m_constraint->Match(m_Desc))
                    {
                        if (m_ChangedDescriptors.find(m_Desc) != m_ChangedDescriptors.end())
                            m_EditedDesc = m_ChangedDescriptors[m_Desc];
                        else
                        {
                            m_EditedDesc.Reset(new CSeqdesc);
                            m_EditedDesc->Assign(*m_Desc);
                        }
                        m_delete = false;
                        m_DescContext = seh;
                        Modify(action);                       
                        found_seh = seh;
                    }
                }
                
            }           
        }
        ++count;
        if (count >= m_max_records)
            break; 
    }
}

void CEditingActionDescComment::FindRelated(EActionType action)
{
    Find(action);
}

void CEditingActionDescComment::Modify(EActionType action)
{  
    if (m_EditedDesc->IsUser())
    {
        EDIT_EACH_USERFIELD_ON_USEROBJECT(field_iter, m_EditedDesc->SetUser())
        {
            CUser_field& field = **field_iter;
            if (field.GetLabel().IsStr() && NStr::Equal(field.GetLabel().GetStr(), m_field) && field.IsSetData())
            {
                m_str_value = NULL;
                switch (field.GetData().Which())
                {
                case CUser_field::TData::e_Str :
                {
                    m_str_value = &field.SetData().SetStr();
                    IEditingActionDesc::Modify(action);
                    break;
                }
                case CUser_field::TData::e_Strs :
                {
                    NON_CONST_ITERATE(CUser_field::TData::TStrs, it, field.SetData().SetStrs())
                    {
                        m_str_value = &*it;
                        IEditingActionDesc::Modify(action);
                    }
                    break;
                }
                default : break;
                }
            }
        }
    }
    if (m_EditedDesc->IsComment())
    {
        m_str_value = NULL;
        IEditingActionDesc::Modify(action);
    }
}

bool CEditingActionDescComment::IsSetValue()
{
    return m_str_value != NULL || (m_EditedDesc && m_EditedDesc->IsComment());
}

void CEditingActionDescComment::SetValue(const string &value)
{
    // nothing here
}

string CEditingActionDescComment::GetValue()
{
    if (m_str_value)
        return *m_str_value;
    if (m_EditedDesc->IsComment())
        return m_EditedDesc->GetComment();
    return kEmptyStr;
}

void CEditingActionDescComment::ResetValue()
{
    // nothing here
}

IEditingActionDesc* CreateActionDesc(CSeq_entry_Handle seh, const string &field, CConstRef<objects::CSeq_submit> submit)
{
    if (field == "Comment Descriptor")
        return new CEditingActionDescCommentDescriptor(seh, submit);
    if (field == "Definition Line")
        return new CEditingActionDescDefline(seh, submit);
    if (field == "Keyword")
        return new CEditingActionDescKeyword(seh, submit);
    if (field == "Genome Project ID")
        return new CEditingActionDescGenomeProjectId(seh, submit);
    if (NStr::StartsWith(field, "Structured comment Field ") && field != "Structured comment Field Name")
    {
        string str_comm_field = field;
        NStr::ReplaceInPlace(str_comm_field, "Structured comment Field ", kEmptyStr);
        if (!str_comm_field.empty())
        {
            return new CEditingActionDescUserObject(seh, "StructuredComment", str_comm_field, submit);
        }
    }
    if (field == "Structured comment Database Name")
        return new CEditingActionDescStructCommDbName(seh, submit);
    if (field == "Structured comment Field Name")
        return new CEditingActionDescStructCommFieldName(seh, submit);
    if (field == "Trace Assembly Archive")
        return new CEditingActionDescUserObject(seh, "DBLink", "Trace Assembly Archive", true, submit);
    if (field == "BioSample")
        return new CEditingActionDescUserObject(seh, "DBLink", "BioSample", true, submit);
    if (field == "ProbeDB")
        return new CEditingActionDescUserObject(seh, "DBLink", "ProbeDB", true, submit);
    if (field == "Sequence Read Archive" || field == "Read Archive")
        return new CEditingActionDescUserObject(seh, "DBLink", "Sequence Read Archive", true, submit);
    if (field == "BioProject")
        return new CEditingActionDescUserObject(seh, "DBLink", "BioProject", true, submit);
    if (field == "Assembly")
        return new CEditingActionDescUserObject(seh, "DBLink", "Assembly", true, submit);
    if (field == "molecule")
        return new CEditingActionMolInfoMolecule(seh, submit);
    if (field == "technique")
        return new CEditingActionMolInfoTechnique(seh, submit);
    if (field == "completedness")
        return new CEditingActionMolInfoCompletedness(seh, submit);
    if (field == "class")
        return new CEditingActionMolInfoClass(seh, submit);
    if (field == "topology")
        return new CEditingActionMolInfoTopology(seh, submit);
    if (field == "strand")
        return new CEditingActionMolInfoStrand(seh, submit);
    if (field == "title")
        return new CEditingActionDescPubTitle(seh, submit);
    if (field == "pmid")
        return new CEditingActionDescPubPmid(seh, submit);
    if (field == "institution")
        return new CEditingActionDescAffil(seh, submit);
    if (field == "department")
        return new CEditingActionDescDepartment(seh, submit);
    if (field == "street")
        return new CEditingActionDescStreet(seh, submit);
    if (field == "city")
        return new CEditingActionDescCity(seh, submit);
    if (field == "state")
        return new CEditingActionDescState(seh, submit);
    if (field == "country")
        return new CEditingActionDescCountry(seh, submit);
    if (field == "email")
        return new CEditingActionDescEmail(seh, submit);
    if (field == "author first name")
        return new CEditingActionDescAuthorFirstName(seh, submit);
    if (field == "author last name")
        return new CEditingActionDescAuthorLastName(seh, submit);
    if (field == "author suffix")
        return new CEditingActionDescAuthorSuffix(seh, submit);
    if (field == "author consortium")
        return new CEditingActionDescAuthorConsortium(seh, submit);
    if (field == "author middle initials")
        return new CEditingActionDescAuthorInitials(seh, submit);
    if (field == "journal")
        return new CEditingActionDescJournal(seh, submit);
    if (field == "volume")
        return new CEditingActionDescVolume(seh, submit);
    if (field == "issue")
        return new CEditingActionDescIssue(seh, submit);
    if (field == "pages")
        return new CEditingActionDescPages(seh, submit);
    if (field == "status")
        return new CEditingActionDescStatus(seh, submit);
    if (field == "author name list")
        return new CEditingActionDescAuthorNameList(seh, submit);
    if (field == "Local ID")
        return new CEditingActionDescLocalId(seh, submit);        
    if (field == "BankIT Comment")
        return new CEditingActionDescUserObject(seh, "Submission", "AdditionalComment", submit);
    if (field == "Comment")
        return new CEditingActionDescComment(seh, submit);
    return NULL;
}



END_NCBI_SCOPE

