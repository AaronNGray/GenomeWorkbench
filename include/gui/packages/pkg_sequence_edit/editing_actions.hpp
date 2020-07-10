/*  $Id: editing_actions.hpp 45101 2020-05-29 20:53:24Z asztalos $
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

#ifndef _EDITING_ACTIONS_H_
#define _EDITING_ACTIONS_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/RNA_qual.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/submit/Submit_block.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/seq_feat_handle.hpp>
#include <gui/objutils/macro_fn_aecr.hpp>
#include <gui/objutils/macro_util.hpp>
#include <objtools/cleanup/capitalization_string.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CEditingActionConstraint;

class IEditingAction : public CObject
{
public:
    IEditingAction(CSeq_entry_Handle seh, const string &name, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
    virtual ~IEditingAction() {}
    const string GetName(void) const;
    const map<CSeq_feat_Handle, CRef<CSeq_feat> >& GetChangedFeatures(void);
    const map<CSeq_feat_Handle, CSeq_entry_Handle> &GetCreatedFeatures(void);
    const map<const CSeqdesc*, CRef<CSeqdesc> >& GetChangedDescriptors(void);
    const map<const CSeqdesc*, CSeq_entry_Handle>& GetContextForDescriptors(void);
    const map<const CSeqdesc*, CSeq_entry_Handle>& GetCreatedDescriptors(void);
    const map<const CSeqdesc*, CSeq_entry_Handle>& GetDeletedDescriptors(void);
    const map<CBioseq_Handle, CRef<CSeq_inst> >& GetChangedInstances(void);
    CRef<CSubmit_block> GetChangedSubmitBlock(void);
    const vector<string>& GetChangedValues(void);

    bool GetChangedIds(void);
    void ResetChangedFeatures(void);
    void ResetChangedDescriptors(void);
    void ResetChangedInstances(void);
    void ResetChangedIds(void);
    void ResetChangedValues(void);

    void SetExistingText( edit::EExistingText existing_text);
    void SetConstraint(CRef<CEditingActionConstraint> constraint);
    void SetTopSeqEntry(CSeq_entry_Handle seh);
    void SetCurrentSeqEntry(CSeq_entry_Handle seh);
    CRef<CCmdComposite> GetActionCommand(void);
    CConstRef<objects::CSeq_submit> GetSeqSubmit(void);

    bool CheckValue(void);
    bool CheckValues(void);
    const vector<string>& GetValues(void);

    void Apply(const string& value);
    void Edit(const string& find, const string& replace, macro::CMacroFunction_EditStringQual::ESearchLoc edit_loc, bool case_insensitive, bool is_regex);
    void Remove(void) ;
    void RemoveOutside(CRef<macro::CRemoveTextOptions> options);
    void ConvertFrom(ECapChange cap_change, bool leave_on_original, const string& strip_name, IEditingAction* destination);
    void ConvertTo(const string &value, IEditingAction* source);
    void CopyFrom(IEditingAction* destination);
    void CopyTo(const string& value, IEditingAction* source);
    void SwapFrom(IEditingAction *destination);
    void SwapTo(const string& value, IEditingAction* source);
    void ParseFrom(edit::CParseTextOptions parse_options, IEditingAction *destination);
    void ParseTo(const string& value, IEditingAction* source);
    void NOOP(void);

    enum EActionType {
        eActionType_NOOP = 0,
        eActionType_Apply,
        eActionType_Edit,       
        eActionType_Remove,
        eActionType_RemoveOutside,
        eActionType_ConvertFrom,
        eActionType_CopyFrom,
        eActionType_ParseFrom,
        eActionType_SwapFrom,
        eActionType_ConvertTo,
        eActionType_CopyTo,
        eActionType_ParseTo,
        eActionType_SwapTo
    };

    int CheckForExistingText(CIRef<IEditingAction> action2, EActionType action_type, edit::CParseTextOptions parse_options = edit::CParseTextOptions());
    void SetUpdateMRNAProduct(bool value) {m_update_mrna = value;}
    void SetRetranslateCds(bool value) {m_retranslate_cds = value;}
    bool IsModified() {return m_modified;}
    void SetMaxRecords(size_t max) {m_max_records = max;}
protected:  

    bool IsNOOP(EActionType action);
    bool IsFrom(EActionType action);
    bool IsTo(EActionType action);
    bool IsCreateNew(EActionType action);

    CIRef<IEditCommand> GetUpdateMRNAProductNameCmd(const CSeq_feat_Handle fh, const CRef<CSeq_feat> feat);
    CRef<CCmdComposite> GetRetranslateCdsCmd(const CSeq_feat_Handle fh, const CRef<CSeq_feat> feat, int& offset, bool create_general_only);

    void Action(EActionType action);
    virtual void Find(EActionType action) = 0;
    virtual void FindRelated(EActionType action) = 0;
    virtual void Modify(EActionType action) = 0;
    virtual void SwapContext(IEditingAction* source); 
    virtual bool SameObject() = 0;

    void DoApply(void);
    void DoEdit(void);
    void DoRemove(void);
    void DoRemoveOutside(void);
    void DoConvertFrom(void);
    void DoConvertTo(void);
    void DoCopyFrom(void);
    void DoCopyTo(void);
    void DoSwapFrom(void);
    void DoSwapTo(void);
    void DoParseFrom(void);
    void DoParseTo(void);
    void DoNOOP(void);

    void HandleValue(const string& value);
    virtual bool IsSetValue(void) = 0;
    virtual void SetValue(const string &value) = 0;
    virtual string GetValue(void) = 0;
    virtual void ResetValue(void) = 0;    
    virtual void ResetScope() {}

    map<CSeq_feat_Handle, CRef<CSeq_feat> > m_ChangedFeatures;
    map<CSeq_feat_Handle, CSeq_entry_Handle> m_CreatedFeatures;
    map<const CSeqdesc*, CRef<CSeqdesc> > m_ChangedDescriptors;
    map<const CSeqdesc*, CSeq_entry_Handle> m_ContextForDescriptors;
    map<const CSeqdesc*, CSeq_entry_Handle> m_CreatedDescriptors;
    map<const CSeqdesc*, CSeq_entry_Handle> m_DeletedDescriptors;
    map<CBioseq_Handle, CRef<CSeq_inst> > m_ChangedInstances;
    CRef<CSubmit_block> m_ChangedSubmitBlock;

    vector<string> m_ChangedValues;
    bool m_ChangedIds;

    string m_Name;
    CSeq_entry_Handle m_TopSeqEntry;
    CConstRef<objects::CSeq_submit> m_SeqSubmit;
    CSeq_entry_Handle m_CurrentSeqEntry;
    bool m_modified;

    IEditingAction* m_Other;

    string m_value, m_find, m_replace, m_strip_name;
    macro::CMacroFunction_EditStringQual::ESearchLoc m_edit_loc;
    bool m_case_insensitive;
    bool m_is_regex;
    CRef<macro::CRemoveTextOptions> m_remove_options;
    ECapChange m_cap_change;
    bool m_leave_on_original;
    edit::CParseTextOptions m_parse_options;

    edit::EExistingText m_existing_text;
    CRef<CEditingActionConstraint> m_constraint;
    bool m_update_mrna;
    bool m_retranslate_cds;
    size_t m_max_records;
};


CIRef<IEditingAction> CreateAction(CSeq_entry_Handle seh, const string &field, CFieldNamePanel::EFieldType field_type, int subtype, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL));
int GetSubtype(CFieldNamePanel *field_name_panel);
CRef<CObject> GetNewObject(CConstRef<CObject> object);
CRef<CCmdComposite> GetReplacementCommand(CConstRef<CObject> oldobj, CRef<CObject> newobj, CScope& scope, const string& cmd_name);
vector<CConstRef<CObject> > GetObjects(CSeq_entry_Handle seh, const string &field, CFieldNamePanel::EFieldType field_type, int subtype, CConstRef<objects::CSeq_submit> submit,
				       CRef<CEditingActionConstraint> constraint, vector<CSeq_entry_Handle>* descr_context = nullptr);

// CCmdDelSeq_feat

END_NCBI_SCOPE

#endif
    // _EDITING_ACTIONS_H_
