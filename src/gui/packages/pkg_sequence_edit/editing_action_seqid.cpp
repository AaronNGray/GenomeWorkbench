/*  $Id: editing_action_seqid.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
#include <objects/seq/seq_macros.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/object_manager.hpp>
#include <gui/widgets/edit/bioseq_editor.hpp>
#include <objtools/edit/string_constraint.hpp>
#include <objects/seq/Seq_data.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_seqid.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CEditingActionSeqid::CEditingActionSeqid(CSeq_entry_Handle seh, const string &name)
    : IEditingAction(seh, name)
{
}

void CEditingActionSeqid::Find(EActionType action)
{
    if (!m_TopSeqEntry)
        return;
    size_t count = 0;
    for (CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na); b_iter; ++b_iter)
    {
        CBioseq_Handle bsh = *b_iter; 
        m_CurrentSeqEntry = bsh.GetSeq_entry_Handle();
        if (m_constraint->Match(bsh))
        {     
            FOR_EACH_SEQID_ON_BIOSEQ(id_iter, *bsh.GetBioseqCore())
            {
                m_id = *id_iter;
                Modify(action);              
            }                       
        }
        ++count;
        if (count >= m_max_records)
            break;      
    }
}

void CEditingActionSeqid::FindRelated(EActionType action)
{
    Find(action); 
}

void CEditingActionSeqid::Modify(EActionType action)
{
    Action(action); 
    if (m_modified)
        m_ChangedIds = true;
}

bool CEditingActionSeqid::SameObject()
{
    CEditingActionSeqid *other = dynamic_cast<CEditingActionSeqid*>(m_Other);
    if (other)
        return true;
    return false;
}

void CEditingActionSeqid::SwapContext(IEditingAction* source)
{
    CEditingActionSeqid *id_source = dynamic_cast<CEditingActionSeqid*>(source);
    if (id_source)
    {
        swap(m_id, id_source->m_id);
    }
    IEditingAction::SwapContext(source);
}

bool CEditingActionSeqid::IsSetValue()
{
    return m_id;
}

void CEditingActionSeqid::SetValue(const string &value)
{
}

string CEditingActionSeqid::GetValue()
{
    string value;
    m_id->GetLabel(&value, CSeq_id::eContent);
    return value;
}

void CEditingActionSeqid::ResetValue()
{
}

CEditingActionFileId::CEditingActionFileId(CSeq_entry_Handle seh)
    : CEditingActionSeqid(seh, "CEditingActionFileId")
{ 
}

bool CEditingActionFileId::SameObject()
{
    CEditingActionFileId *other = dynamic_cast<CEditingActionFileId*>(m_Other);
    if (other)
        return true;
    return false;
}

bool CEditingActionFileId::IsSetValue()
{
    return m_id && m_id->IsGeneral() && m_id->GetGeneral().IsSetDb()
        && NStr::EqualCase(m_id->GetGeneral().GetDb(), "NCBIFILE");
}

string CEditingActionFileId::GetValue()
{
    string value;
    m_id->GetLabel(&value, CSeq_id::eContent);
    NStr::TrimPrefixInPlace(value, "NCBIFILE:");
    return value;
}

CEditingActionEntireGeneralId::CEditingActionEntireGeneralId(CSeq_entry_Handle seh)
    : CEditingActionSeqid(seh, "CEditingActionEntireGeneralId")
{ 
}

bool CEditingActionEntireGeneralId::SameObject()
{
    CEditingActionEntireGeneralId *other = dynamic_cast<CEditingActionEntireGeneralId*>(m_Other);
    if (other)
        return true;
    return false;
}

bool CEditingActionEntireGeneralId::IsSetValue()
{
    return m_id && m_id->IsGeneral();
}

string CEditingActionEntireGeneralId::GetValue()
{
    string value;
    m_id->GetGeneral().GetLabel(&value);
    return value;
}

CEditingActionGeneralIdDb::CEditingActionGeneralIdDb(CSeq_entry_Handle seh)
    : CEditingActionSeqid(seh, "CEditingActionGeneralIdDb")
{ 
}

bool CEditingActionGeneralIdDb::SameObject()
{
    CEditingActionGeneralIdDb *other = dynamic_cast<CEditingActionGeneralIdDb*>(m_Other);
    if (other)
        return true;
    return false;
}

bool CEditingActionGeneralIdDb::IsSetValue()
{
    return m_id && m_id->IsGeneral() && m_id->GetGeneral().IsSetDb();
}

string CEditingActionGeneralIdDb::GetValue()
{
    return m_id->GetGeneral().GetDb();
}

CEditingActionGeneralIdTag::CEditingActionGeneralIdTag(CSeq_entry_Handle seh, const string &db)
    : CEditingActionSeqid(seh, "CEditingActionGeneralIdTag"), m_db(db)
{ 
}

bool CEditingActionGeneralIdTag::SameObject()
{
    CEditingActionGeneralIdTag *other = dynamic_cast<CEditingActionGeneralIdTag*>(m_Other);
    if (other)
        return true;
    return false;
}

bool CEditingActionGeneralIdTag::IsSetValue()
{
    return m_id && m_id->IsGeneral() && m_id->GetGeneral().IsSetTag() 
        && (NStr::IsBlank(m_db) ||   (m_id->GetGeneral().IsSetDb() && NStr::EqualCase(m_id->GetGeneral().GetDb(), m_db)));
}

string CEditingActionGeneralIdTag::GetValue()
{
    CNcbiOstrstream ostr;
    m_id->GetGeneral().GetTag().AsString(ostr);
    return CNcbiOstrstreamToString(ostr);
}

CEditingActionSeqid* CreateActionSeqid(CSeq_entry_Handle seh, const string &field)
{
    if (field == "SeqId")
        return new CEditingActionSeqid(seh);   
    if (field == "File ID")
        return new CEditingActionFileId(seh);   
    if (field == "Entire General ID")
        return new CEditingActionEntireGeneralId(seh);
    if (field == "General ID DB")
        return new CEditingActionGeneralIdDb(seh);
    if (NStr::StartsWith(field, "General ID Tag"))
    {
        string db = field;
        NStr::TrimPrefixInPlace(db, "General ID Tag");
        NStr::TrimPrefixInPlace(db," ");
        return new CEditingActionGeneralIdTag(seh, db);
    }

    return NULL;
}



END_NCBI_SCOPE

