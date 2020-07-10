/*  $Id: editing_action_desc.hpp 45101 2020-05-29 20:53:24Z asztalos $
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

#ifndef _EDITING_ACTION_DESC_H_
#define _EDITING_ACTION_DESC_H_

#include <corelib/ncbistd.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/biblio/Title.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/biblio/ArticleId.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/macro_fn_do.hpp>
#include <objtools/cleanup/capitalization_string.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CEditingActionConstraint;

class IEditingActionDesc : public IEditingAction 
{
public:
    IEditingActionDesc(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit, const string &name, const CSeqdesc::E_Choice subtype);
    void SetDesc(const CSeqdesc* desc);
    CBioseq_Handle GetBioseqHandle();

protected:
    virtual void Find(EActionType action);
    virtual void FindRelated(EActionType action);
    virtual void Modify(EActionType action);
    virtual bool SameObject();
    virtual void SwapContext(IEditingAction* source);

    CSeqdesc::E_Choice m_subtype;
    const CSeqdesc* m_Desc;
    CRef<CSeqdesc> m_EditedDesc;
    bool m_delete;
    CBioseq_Handle m_bsh;
    CSeq_entry_Handle m_DescContext;
    bool m_change_submit_block;
};

class CEditingActionDescCommentDescriptor : public IEditingActionDesc
{
public:
    CEditingActionDescCommentDescriptor(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescDefline : public IEditingActionDesc
{
public:
    CEditingActionDescDefline(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescKeyword : public IEditingActionDesc
{
public:
    CEditingActionDescKeyword(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
private:
    string *m_keyword;
    bool m_erase;
};

class CEditingActionDescGenomeProjectId: public IEditingActionDesc
{
public:
    CEditingActionDescGenomeProjectId(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescUserObject: public IEditingActionDesc
{
public:
    CEditingActionDescUserObject(CSeq_entry_Handle seh, const string &obj_type, const string &field, const bool create_multiple = false, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool SameObject();
    virtual void Find(EActionType action);
    virtual void FindRelated(EActionType action);
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);

private:
    string m_type;
    string m_field;
    string *m_str_value;
    int *m_int_value;
    double *m_real_value;
    bool *m_bool_value;
    bool m_create_multiple;
    bool m_erase;
};

class CEditingActionDescStructCommDbName: public IEditingActionDesc
{
public:
    CEditingActionDescStructCommDbName(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescStructCommFieldName: public IEditingActionDesc
{
public:
    CEditingActionDescStructCommFieldName(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
private:
    string *m_label;
    bool m_erase;
};

class CEditingActionMolInfoMolecule: public IEditingActionDesc
{
public:
    CEditingActionMolInfoMolecule(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionMolInfoTechnique: public IEditingActionDesc
{
public:
    CEditingActionMolInfoTechnique(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionMolInfoCompletedness: public IEditingActionDesc
{
public:
    CEditingActionMolInfoCompletedness(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class IEditingActionInst : public IEditingActionDesc
{
public:
    IEditingActionInst(CSeq_entry_Handle seh, const string &name, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual void Find(EActionType action);
    virtual void FindRelated(EActionType action);
    virtual void Modify(EActionType action);
    virtual bool SameObject();
    virtual void SwapContext(IEditingAction* source);

    CRef<CSeq_inst> m_EditedInst;
    CBioseq_Handle m_bsh;
};

class CEditingActionMolInfoClass: public IEditingActionInst
{
public:
    CEditingActionMolInfoClass(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionMolInfoTopology: public IEditingActionInst
{
public:
    CEditingActionMolInfoTopology(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionMolInfoStrand: public IEditingActionInst
{
public:
    CEditingActionMolInfoStrand(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class IEditingActionDescPub : public IEditingActionDesc
{
public:
    IEditingActionDescPub(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL), const string &name = "IEditingActionDescPub");
protected:
    virtual bool SameObject();
    virtual void SwapContext(IEditingAction* source);
    CRef<CPub> m_pub;
};

class CEditingActionDescPubTitle : public IEditingActionDescPub
{
public:
    CEditingActionDescPubTitle(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
    virtual void SwapContext(IEditingAction* source);
private:
    void GetTitles(CRef<CPub> pub, EActionType action, bool &found);
  
    string* m_title;
    bool m_erase;
};

class CEditingActionDescPubPmid : public IEditingActionDescPub
{
public:
    CEditingActionDescPubPmid(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
    virtual void SwapContext(IEditingAction* source);
private:
    bool m_erase;
    CRef<CArticleId> m_id;
};

class IEditingActionDescAffil : public IEditingActionDescPub
{
public:
    IEditingActionDescAffil(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL), const string &name = "IEditingActionDescAffil");
protected:
    virtual void Modify(EActionType action);
    virtual void SwapContext(IEditingAction* source);
    void AddAuthList(CRef<CAuth_list> auth_list);

    CAffil* m_affil;
};

class CEditingActionDescAffil : public IEditingActionDescAffil
{
public:
    CEditingActionDescAffil(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescDepartment : public IEditingActionDescAffil
{
public:
    CEditingActionDescDepartment(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescStreet : public IEditingActionDescAffil
{
public:
    CEditingActionDescStreet(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescCity : public IEditingActionDescAffil
{
public:
    CEditingActionDescCity(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescState : public IEditingActionDescAffil
{
public:
    CEditingActionDescState(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescCountry : public IEditingActionDescAffil
{
public:
    CEditingActionDescCountry(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescEmail : public IEditingActionDescAffil
{
public:
    CEditingActionDescEmail(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class IEditingActionDescAuthors : public IEditingActionDescPub
{
public:
    IEditingActionDescAuthors(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL), const string &name = "IEditingActionDescAuthors");
protected:
    virtual void Modify(EActionType action);  
    virtual void SwapContext(IEditingAction* source);

    CRef<CAuthor> m_author;
};

class CEditingActionDescAuthorFirstName : public IEditingActionDescAuthors
{
public:
    CEditingActionDescAuthorFirstName(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescAuthorLastName : public IEditingActionDescAuthors
{
public:
    CEditingActionDescAuthorLastName(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescAuthorSuffix : public IEditingActionDescAuthors
{
public:
    CEditingActionDescAuthorSuffix(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescAuthorConsortium : public IEditingActionDescAuthors
{
public:
    CEditingActionDescAuthorConsortium(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescAuthorInitials : public IEditingActionDescAuthors
{
public:
    CEditingActionDescAuthorInitials(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class IEditingActionDescJournal : public IEditingActionDescPub
{
public:
    IEditingActionDescJournal(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL), const string &name = "IEditingActionDescJournal");
protected:
    virtual void Modify(EActionType action);  

    CImprint *m_imprint;
    CCit_gen *m_citgen;
private:
    void GetImprintOrCitgen(CPub& pub);
};

class CEditingActionDescJournal : public IEditingActionDescJournal
{
public:
    CEditingActionDescJournal(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescVolume : public IEditingActionDescJournal
{
public:
    CEditingActionDescVolume(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescIssue : public IEditingActionDescJournal
{
public:
    CEditingActionDescIssue(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescPages : public IEditingActionDescJournal
{
public:
    CEditingActionDescPages(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescStatus : public IEditingActionDescJournal
{
public:
    CEditingActionDescStatus(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionDescAuthorNameList : public IEditingActionDescPub
{
public:
    CEditingActionDescAuthorNameList(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual void Modify(EActionType action);
    virtual void SwapContext(IEditingAction* source);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);

    CAuth_list* m_auth_list;
};

class CEditingActionDescLocalId: public IEditingActionDesc
{
public:
    CEditingActionDescLocalId(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual void Find(EActionType action);
    virtual void FindRelated(EActionType action);
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);

private:
    string m_type;
    string m_field;
    string *m_str_value;
    int *m_int_value;
    const CSeq_id* m_bioseq_value;
};

class CEditingActionDescComment : public IEditingActionDesc
{
public:
    CEditingActionDescComment(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
protected:
    virtual void Find(EActionType action);
    virtual void FindRelated(EActionType action);
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);

private:
    string m_type;
    string m_field;
    string *m_str_value;
};

IEditingActionDesc* CreateActionDesc(CSeq_entry_Handle seh, const string &field, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));

END_NCBI_SCOPE

#endif
    // _EDITING_ACTION_DESC_H_
