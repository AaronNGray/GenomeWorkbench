/*  $Id: editing_actions.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/object_manager.hpp>
#include <gui/widgets/edit/bioseq_editor.hpp>
#include <objtools/edit/string_constraint.hpp>
#include <objtools/cleanup/cleanup.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objtools/edit/apply_object.hpp>
#include <objects/seq/Seq_data.hpp>
#include <objmgr/seq_vector.hpp>
#include <objtools/edit/source_edit.hpp>

#include <gui/widgets/edit/source_field_name_panel.hpp>
#include <gui/widgets/edit/rna_field_name_panel.hpp>
#include <gui/widgets/edit/cds_gene_prot_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/misc_field_panel.hpp>
#include <gui/widgets/edit/struct_fieldvalue_panel.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/retranslate_cds.hpp>

#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_features.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_biosource.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_desc.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_seqid.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_misc.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


IEditingAction::IEditingAction(CSeq_entry_Handle seh, const string &name, CConstRef<objects::CSeq_submit> submit) 
    : m_Name(name), m_TopSeqEntry(seh), m_SeqSubmit(submit), m_Other(NULL), m_existing_text(edit::eExistingText_replace_old), m_constraint(CRef<CEditingActionConstraint>(new CEditingActionConstraint)), 
      m_ChangedIds(false), m_update_mrna(false), m_retranslate_cds(false), m_max_records(numeric_limits<size_t>::max())
{
}

const string IEditingAction::GetName(void) const 
{
    return m_Name;
}

const map<CSeq_feat_Handle, CRef<CSeq_feat> >& IEditingAction::GetChangedFeatures(void) 
{
    return m_ChangedFeatures;
}

const map<CSeq_feat_Handle, CSeq_entry_Handle>& IEditingAction::GetCreatedFeatures(void)
{
    return m_CreatedFeatures;
}

const map<const CSeqdesc*, CRef<CSeqdesc> >& IEditingAction::GetChangedDescriptors()
{
    return m_ChangedDescriptors;   
}

const map<const CSeqdesc*, CSeq_entry_Handle>& IEditingAction::GetContextForDescriptors(void)
{
    return m_ContextForDescriptors;
}

const map<const CSeqdesc*, CSeq_entry_Handle>& IEditingAction::GetCreatedDescriptors()
{
    return m_CreatedDescriptors;
}

const map<const CSeqdesc*, CSeq_entry_Handle>& IEditingAction::GetDeletedDescriptors()
{
    return m_DeletedDescriptors;
}

const map<CBioseq_Handle, CRef<CSeq_inst> >& IEditingAction::GetChangedInstances()
{
    return m_ChangedInstances;
}

CRef<CSubmit_block> IEditingAction::GetChangedSubmitBlock(void)
{
    return m_ChangedSubmitBlock;
}

bool IEditingAction::GetChangedIds()
{
    return m_ChangedIds;
}

const vector<string>& IEditingAction::GetChangedValues()
{
    return m_ChangedValues;
}

void IEditingAction::ResetChangedFeatures(void)
{
    m_ChangedFeatures.clear();
    m_CreatedFeatures.clear();
    ResetScope();
}

void IEditingAction::ResetChangedDescriptors(void)
{
    m_ChangedDescriptors.clear();
    m_ContextForDescriptors.clear();
    m_CreatedDescriptors.clear();
    m_DeletedDescriptors.clear();
    m_ChangedSubmitBlock.Reset();
}

void IEditingAction::ResetChangedInstances(void)
{
    m_ChangedInstances.clear();
}

void IEditingAction::ResetChangedIds(void)
{
    m_ChangedIds = false;
}

void IEditingAction::ResetChangedValues()
{
    m_ChangedValues.clear();
}

CRef<CCmdComposite> IEditingAction::GetActionCommand() // only do it for "From" part of the two-action commands
{
    CCleanup cleanup;
    CRef<CCmdComposite> cmd(new CCmdComposite("AECR action"));
    const map<CSeq_feat_Handle, CRef<CSeq_feat> > &changed_features = GetChangedFeatures();
    const map<CSeq_feat_Handle, CSeq_entry_Handle> &created_features = GetCreatedFeatures();

    // offset for making new protein IDs (when needed)
    int offset = 1;
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(m_TopSeqEntry.GetTopLevelEntry());
    for (map<CSeq_feat_Handle, CRef<CSeq_feat> >::const_iterator it = changed_features.begin(); it != changed_features.end(); ++it)
    {
        const CSeq_feat_Handle fh = it->first;
        CRef<CSeq_feat> feat = it->second;
        cleanup.BasicCleanup(*feat);
        CIRef<IEditCommand> chgFeat;
        CIRef<IEditCommand> update_mrna_cmd;
        CRef<CCmdComposite> retranslate_cds_cmd;
        map<CSeq_feat_Handle, CSeq_entry_Handle>::const_iterator cf = created_features.find(fh);
        if (cf != created_features.end())
        {
            CSeq_entry_Handle eh = cf->second;
            chgFeat.Reset(new CCmdCreateFeat(eh, *feat));
        }
        else
        {
            chgFeat.Reset(new CCmdChangeSeq_feat(fh, *feat));
            if (m_update_mrna)
                update_mrna_cmd =  GetUpdateMRNAProductNameCmd(fh, feat);
            if (m_retranslate_cds)
            {
                if (fh.GetOriginalSeq_feat()->IsSetData() && fh.GetOriginalSeq_feat()->GetData().IsCdregion() && 
                    fh.GetOriginalSeq_feat()->IsSetExcept_text() && NStr::Find(fh.GetOriginalSeq_feat()->GetExcept_text(), "RNA editing") != string::npos)
                {
                    chgFeat.Reset();
                }
                else
                {
                    retranslate_cds_cmd = GetRetranslateCdsCmd(fh, feat, offset, create_general_only);
                }
            }
        }

        if (chgFeat)
            cmd->AddCommand(*chgFeat);
        if (update_mrna_cmd)
            cmd->AddCommand(*update_mrna_cmd);
        if (retranslate_cds_cmd)
            cmd->AddCommand(*retranslate_cds_cmd);
    }

    const map<const CSeqdesc*, CRef<CSeqdesc> > &changed_descriptors = GetChangedDescriptors();
    const map<const CSeqdesc*, CSeq_entry_Handle> &descr_contexts = GetContextForDescriptors();
    const map<const CSeqdesc*, CSeq_entry_Handle> &created_descriptors = GetCreatedDescriptors();
    const map<const CSeqdesc*, CSeq_entry_Handle> &deleted_descriptors = GetDeletedDescriptors();

    for (map<const CSeqdesc*, CRef<CSeqdesc> >::const_iterator it = changed_descriptors.begin(); it != changed_descriptors.end(); ++it)
    {
        map<const CSeqdesc*, CSeq_entry_Handle>::const_iterator cd = created_descriptors.find(it->first);
        map<const CSeqdesc*, CSeq_entry_Handle>::const_iterator dd = deleted_descriptors.find(it->first);
        if (cd != created_descriptors.end())
        {
            const CSeq_entry_Handle seh = cd->second;
            CRef<CSeqdesc> new_desc = it->second;
            if (new_desc->IsSource())
            {
                cleanup.BasicCleanup(new_desc->SetSource());
            }
            cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(seh, *new_desc)) );
        }
        else if (dd != deleted_descriptors.end())
        {
            const CSeq_entry_Handle seh = dd->second;
            const CSeqdesc *old_desc = it->first;
            cmd->AddCommand( *CRef<CCmdDelDesc>(new CCmdDelDesc(seh, *old_desc)) );
        }
        else
        {
            const CSeqdesc *old_desc = it->first;
            string old_taxname;
            if (old_desc->IsSource() && old_desc->GetSource().IsSetOrg() && old_desc->GetSource().GetOrg().IsSetTaxname()) {
                old_taxname = old_desc->GetSource().GetOrg().GetTaxname();
            }

            CRef<CSeqdesc> new_desc = it->second;

            string new_taxname;
            if (new_desc->IsSource() && new_desc->GetSource().IsSetOrg() && new_desc->GetSource().GetOrg().IsSetTaxname()) {
                new_taxname = new_desc->GetSource().GetOrg().GetTaxname();
            }

            if (!NStr::IsBlank(old_taxname) && !NStr::Equal(old_taxname, new_taxname)) {
                edit::CleanupForTaxnameChange(new_desc->SetSource());
            }

            if (new_desc->IsSource())
            {
                cleanup.BasicCleanup(new_desc->SetSource());
            }
            auto ctxt_it = descr_contexts.find(old_desc);
            if (ctxt_it != descr_contexts.end()) {
                CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(ctxt_it->second, *old_desc, *new_desc));
                cmd->AddCommand(*ecmd);
            }
        }
    }

    CRef<CSubmit_block> new_submit = GetChangedSubmitBlock();
    if (new_submit && m_SeqSubmit)
    {
    const CSubmit_block &submit = m_SeqSubmit->GetSub();
    const CObject* actual = dynamic_cast<const CObject*>(&submit);
    CChangeSubmitBlockCommand* chg_submit = new CChangeSubmitBlockCommand();
    chg_submit->Add(const_cast<CObject*>(actual), CConstRef<CObject>(new_submit));
    cmd->AddCommand(*chg_submit);
    }

    const map<CBioseq_Handle, CRef<CSeq_inst> > &changed_instances = GetChangedInstances();
    for ( map<CBioseq_Handle, CRef<CSeq_inst> >::const_iterator ci = changed_instances.begin(); ci != changed_instances.end(); ++ci)
    {
        const CBioseq_Handle bsh = ci->first;
        CRef<CSeq_inst> new_inst = ci->second;

        cmd->AddCommand( *CRef<CCmdChangeBioseqInst>(new CCmdChangeBioseqInst(bsh, *new_inst)) );
    }
    return cmd;
}

CIRef<IEditCommand> IEditingAction::GetUpdateMRNAProductNameCmd(const CSeq_feat_Handle fh, const CRef<CSeq_feat> feat)
{
    CIRef<IEditCommand> cmd;
    CConstRef<CSeq_feat> orig_mrna;
    CSeqFeatData::ESubtype subtype = feat->GetData().GetSubtype();
    CScope &scope = fh.GetScope();
    string prot_product;

    if (subtype == CSeqFeatData::eSubtype_prot)
    {
        FOR_EACH_NAME_ON_PROTREF(name_it, feat->GetData().GetProt())
        {
            prot_product = *name_it;
            if (!prot_product.empty())
                break;
        }

        // for proteins: find first the CDS and then find the original overlapping mRNA
        CBioseq_Handle prot_bsh = scope.GetBioseqHandle(fh.GetLocation());
        const CSeq_feat* cds = sequence::GetCDSForProduct(prot_bsh);
        if (cds) 
        {
            orig_mrna.Reset(sequence::GetmRNAforCDS(*cds, scope));           
        }
    }
    
    if (orig_mrna) 
    {
        CRef<CSeq_feat> new_mrna(new CSeq_feat());
        new_mrna->Assign(*orig_mrna);
        string remainder;
        new_mrna->SetData().SetRna().SetRnaProductName(prot_product, remainder);
        CSeq_feat_Handle mrna_fh = scope.GetSeq_featHandle(*orig_mrna);
        cmd.Reset(new CCmdChangeSeq_feat(mrna_fh, *new_mrna));
    }
    return cmd;
}

CRef<CCmdComposite> IEditingAction::GetRetranslateCdsCmd(const CSeq_feat_Handle fh, const CRef<CSeq_feat> feat, int& offset, bool create_general_only)
{
    CScope &scope = fh.GetScope();
    bool cds_change(false);
    CRef<CSeq_feat> cds(new CSeq_feat);
    cds->Assign(*feat);
    return CRetranslateCDS::RetranslateCDSCommand(scope, *fh.GetOriginalSeq_feat(), *cds, cds_change, offset, create_general_only);
}

void IEditingAction::SetTopSeqEntry(CSeq_entry_Handle seh)
{
    m_TopSeqEntry = seh;
    if (seh && seh.HasParentEntry() && seh.GetParentEntry().IsSet() && seh.GetParentEntry().GetSet().IsSetClass() && seh.GetParentEntry().GetSet().GetClass() == CBioseq_set::eClass_nuc_prot)
    {
        m_TopSeqEntry = seh.GetParentEntry();
    }
}

CConstRef<objects::CSeq_submit> IEditingAction::GetSeqSubmit()
{
    return m_SeqSubmit;
}

void IEditingAction::SwapContext(IEditingAction* source)
{
    swap(m_ChangedFeatures, source->m_ChangedFeatures);
    swap(m_CreatedFeatures, source->m_CreatedFeatures);
    swap(m_ChangedDescriptors, source->m_ChangedDescriptors);
    swap(m_ContextForDescriptors, source->m_ContextForDescriptors);
    swap(m_CreatedDescriptors, source->m_CreatedDescriptors);
    swap(m_DeletedDescriptors, source->m_DeletedDescriptors);
    swap(m_ChangedInstances, source->m_ChangedInstances);
    swap(m_ChangedSubmitBlock, source->m_ChangedSubmitBlock);
    swap(m_ChangedValues, source->m_ChangedValues);
}

void IEditingAction::SetExistingText(edit::EExistingText existing_text)
{
    m_existing_text = existing_text;
}

void IEditingAction::SetConstraint(CRef<CEditingActionConstraint> constraint)
{
    m_constraint = constraint;
}

void IEditingAction::HandleValue(const string& value)
{
    if (m_existing_text != edit::eExistingText_replace_old && IsSetValue())
    {
        string curr_val = GetValue();
        if (AddValueToString(curr_val, value, m_existing_text)) 
        {
            SetValue(curr_val);
            m_modified = true;
        }
    }
    else
    {
        SetValue(value);
        m_modified = true;
    }
}

const vector<string>& IEditingAction::GetValues()
{
    ResetChangedValues();
    Modify(eActionType_NOOP);
    return GetChangedValues();
}

bool IEditingAction::CheckValues()
{
    ResetChangedValues();
    Modify(eActionType_NOOP);
    return !GetChangedValues().empty();
}

bool IEditingAction::CheckValue()
{
    if (IsSetValue())
    {
        string val = GetValue();
        return m_constraint->Match(val);
    }
    return false;
}

void IEditingAction::Apply(const string& value)
{
    m_value = value;
    Find(eActionType_Apply);
}

void IEditingAction::Edit(const string& find, const string& replace, macro::CMacroFunction_EditStringQual::ESearchLoc edit_loc, bool case_insensitive, bool is_regex)
{
    m_find = find;
    m_replace = replace;
    m_edit_loc = edit_loc;
    m_case_insensitive = case_insensitive;
    m_is_regex = is_regex;
    Find(eActionType_Edit);
}

void IEditingAction::Remove(void)
{
    Find(eActionType_Remove);
}

void IEditingAction::RemoveOutside(CRef<macro::CRemoveTextOptions> options)
{
    m_remove_options = options;
    Find(eActionType_RemoveOutside);
}

void IEditingAction::ConvertFrom(ECapChange cap_change, bool leave_on_original, const string& strip_name, IEditingAction* destination)
{
    m_cap_change = cap_change;
    m_leave_on_original = leave_on_original;
    m_strip_name = strip_name;
    m_Other = destination;
    Find(eActionType_ConvertFrom);
}

void IEditingAction::ConvertTo(const string &value, IEditingAction* source)
{
    m_value = value;
    m_Other = source;
    if (SameObject() && m_Other->SameObject())
    {
        SwapContext(source);
        Modify(eActionType_ConvertTo);
        source->SwapContext(this);
    }
    else
    {
        IEditingAction::SwapContext(source);
        FindRelated(eActionType_ConvertTo);
        source->IEditingAction::SwapContext(this);
    }
}

void IEditingAction::CopyFrom(IEditingAction* destination)
{
    m_Other = destination;
    Find(eActionType_CopyFrom);
}

void IEditingAction::CopyTo(const string &value, IEditingAction* source)
{
    m_value = value;
    m_Other = source;
    if (SameObject()  && m_Other->SameObject())
    {
        SwapContext(source);
        Modify(eActionType_CopyTo);
        source->SwapContext(this);
    }
    else
    {
        IEditingAction::SwapContext(source);
        FindRelated(eActionType_CopyTo); 
        source->IEditingAction::SwapContext(this);
    }
}

void IEditingAction::SwapFrom(IEditingAction* destination)
{
    m_Other = destination;
    Find(eActionType_SwapFrom);
}

void IEditingAction::SwapTo(const string &value, IEditingAction* source)
{
    m_value = value;
    m_Other = source;
    if (SameObject()  && m_Other->SameObject())
    {
        SwapContext(source);
        Modify(eActionType_SwapTo);
        source->SwapContext(this);
    }
    else
    {
        IEditingAction::SwapContext(source);
        FindRelated(eActionType_SwapTo); 
        source->IEditingAction::SwapContext(this);
    }
}

void IEditingAction::ParseFrom(edit::CParseTextOptions parse_options, IEditingAction* destination)
{
    m_parse_options = parse_options;
    m_Other = destination;
    Find(eActionType_ParseFrom);
}
 
void IEditingAction::ParseTo(const string& value, IEditingAction* source)
{
    m_value = value;
    m_Other = source;
    if (SameObject()  && m_Other->SameObject())
    {
        SwapContext(source);
        Modify(eActionType_ParseTo);
        source->SwapContext(this);
    }
    else
    {
        IEditingAction::SwapContext(source);
        FindRelated(eActionType_ParseTo); 
        source->IEditingAction::SwapContext(this);
    }
}

void IEditingAction::NOOP(void)
{
    Find(eActionType_NOOP);
}

void IEditingAction::DoApply(void)
{ 
    if (m_constraint->Match() || CheckValue())
    {
        HandleValue(m_value);
    }
}

void IEditingAction::DoEdit(void)
{
    if (CheckValue())
    {
        string str = GetValue();
        string old(str);
        macro::CMacroFunction_EditStringQual::s_EditText(str, m_find, m_replace, m_edit_loc, !m_case_insensitive, m_is_regex);
        if (str != old)
        {
            SetValue(str);
            m_modified = true;
        }
    }
}

void IEditingAction::DoRemove(void)
{
    if (CheckValue())
    {
        ResetValue();
        m_modified = true;
    }
}

void IEditingAction::DoRemoveOutside(void)
{
    if (CheckValue())
    {
        string str = GetValue();
        bool modified = m_remove_options->EditText(str);
        if (modified)
        {
            SetValue(str);
            m_modified = true;
        }
    }
}

void IEditingAction::DoConvertFrom(void)
{
    if (CheckValue())
    {
        string val = GetValue();
        RemoveFieldNameFromString(m_strip_name, val);
        FixCapitalizationInString(m_TopSeqEntry.GetTopLevelEntry(), val, m_cap_change);
       
        m_Other->SetTopSeqEntry(m_CurrentSeqEntry); 
        m_Other->ConvertTo(val, this);
        bool other_modified = m_Other->IsModified();
        if (!m_leave_on_original && other_modified)
        {
            ResetValue();
            m_modified = true;
        }
    }
}

void IEditingAction::DoConvertTo(void)
{
    if (CheckValue())
    {
        string current = GetValue();
        m_ChangedValues.push_back(current);
    }
    HandleValue(m_value);
}

void IEditingAction::DoCopyFrom(void)
{
    if (CheckValue())
    {
        string val = GetValue();
        m_Other->SetTopSeqEntry(m_CurrentSeqEntry); 
        m_Other->CopyTo(val, this);
    }
}

void IEditingAction::DoCopyTo(void)
{
    if (CheckValue())
    {
        string current = GetValue();
        m_ChangedValues.push_back(current);
    }
    HandleValue(m_value);
}

void IEditingAction::DoSwapFrom(void)
{
    if (CheckValue())
    {
        string val = GetValue();
        m_Other->SetTopSeqEntry(m_CurrentSeqEntry); 
        m_value.clear();
        m_Other->SwapTo(val, this);
        bool other_modified = m_Other->IsModified();
        if (other_modified)
        {
            SetValue(m_value);
            m_modified = true;
        }
    }
}

void IEditingAction::DoSwapTo(void)
{
    if (IsSetValue())
    {
        string current = GetValue();
        SetValue(m_value);
        m_Other->m_value = current;
        m_modified = true;
    }
}

void IEditingAction::DoParseFrom(void)
{
    if (CheckValue())
    {  
        string val = GetValue();
        string val2 = m_parse_options.GetSelectedText(val);

        if (NStr::IsBlank(val2)) 
        {
            return;
        }

        m_Other->SetTopSeqEntry(m_CurrentSeqEntry); 
        m_Other->ParseTo(val2, this);
        bool other_modified = m_Other->IsModified();
        if (m_parse_options.ShouldRemoveFromParsed() && other_modified) 
        {
            m_parse_options.RemoveSelectedText(val);
            if (NStr::IsBlank(val)) 
            {
                ResetValue();
                m_modified = true;
            } 
            else 
            {
                SetValue(val);
                m_modified = true;
            }
        }
    }
}

void IEditingAction::DoParseTo(void)
{
    if (CheckValue())
    {
        string current = GetValue();
        m_ChangedValues.push_back(current);
    }
    HandleValue(m_value);
}

void IEditingAction::DoNOOP(void)
{
    if (CheckValue())
    {
        string current = GetValue();
        m_ChangedValues.push_back(current);
        m_modified = true;
    }
}

void IEditingAction::Action(EActionType action)
{
    m_modified = false;
    switch (action) 
    {
    case eActionType_Apply: DoApply(); break;
    case eActionType_Edit: DoEdit(); break;
    case eActionType_Remove: DoRemove(); break;
    case eActionType_RemoveOutside: DoRemoveOutside(); break;
    case eActionType_ConvertFrom: DoConvertFrom(); break;
    case eActionType_ConvertTo: DoConvertTo(); break;
    case eActionType_CopyFrom: DoCopyFrom(); break;
    case eActionType_CopyTo: DoCopyTo(); break;
    case eActionType_SwapFrom: DoSwapFrom(); break;
    case eActionType_SwapTo: DoSwapTo(); break;
    case eActionType_ParseFrom: DoParseFrom(); break;
    case eActionType_ParseTo: DoParseTo(); break;
    case eActionType_NOOP: DoNOOP(); break;
    default: break;
    }
}

bool IEditingAction::IsNOOP(EActionType action)
{
    return action == eActionType_NOOP;
}

bool IEditingAction::IsFrom(EActionType action)
{
    return action == eActionType_ConvertFrom || action == eActionType_CopyFrom || action == eActionType_SwapFrom || action == eActionType_ParseFrom;
}

bool IEditingAction::IsTo(EActionType action)
{
    return action == eActionType_ConvertTo || action == eActionType_CopyTo || action == eActionType_SwapTo || action == eActionType_ParseTo;
}

bool IEditingAction::IsCreateNew(EActionType action)
{
    return m_existing_text == edit::eExistingText_add_qual && !IsFrom(action) && !IsNOOP(action);
}

int IEditingAction::CheckForExistingText(CIRef<IEditingAction> action2, EActionType action_type, edit::CParseTextOptions parse_options)
{
    ResetChangedValues();

    switch (action_type) 
    {
        case eActionType_Apply :
        {
            NOOP();
        }
        break;
        case eActionType_Edit :
        {
            // nothing here
        }
        break;
        case eActionType_Remove :
        {
            // nothing here
        }
        break;
        case eActionType_RemoveOutside :
        {
            // nothing here
        }
        break;
       case eActionType_ConvertFrom :
       {
           ConvertFrom(eCapChange_none, true, kEmptyStr, action2);
       }
       break;
      case eActionType_CopyFrom :
      {
          CopyFrom(action2);
      }
      break;
      case eActionType_SwapFrom :
      {
          // nothing here
      }
      break;
      case eActionType_ParseFrom :
      {
          parse_options.SetShouldRemove(false);
          ParseFrom(parse_options, action2);
      }
      break;
      default : break;
    }
    int num =  GetChangedValues().size();
    ResetChangedFeatures();
    ResetChangedDescriptors();
    ResetChangedInstances();
    ResetChangedIds();
    ResetChangedValues();
    if (action2)
    {
        action2->ResetChangedFeatures();
        action2->ResetChangedDescriptors();
        action2->ResetChangedInstances();
        action2->ResetChangedIds();
        action2->ResetChangedValues();
    }
    return num;
}

CIRef<IEditingAction> CreateAction(CSeq_entry_Handle seh, const string &field, CFieldNamePanel::EFieldType field_type, int subtype, CConstRef<objects::CSeq_submit> submit)
{    
    if (subtype > 0 && subtype < CSeqFeatData::eSubtype_any && field_type == CFieldNamePanel::eFieldType_Feature)
    {
        return CIRef<IEditingAction>(CreateActionFeat(seh, field, static_cast<CSeqFeatData::ESubtype>(subtype), CSeqFeatData::GetTypeFromSubtype(static_cast<CSeqFeatData::ESubtype>(subtype)))); 
    }
    else if (field_type == CFieldNamePanel::eFieldType_RNA)
    {
        if (subtype == -1)
            subtype = CSeqFeatData::eSubtype_any;
        return CIRef<IEditingAction>(CreateActionRna(seh, field, static_cast<CSeqFeatData::ESubtype>(subtype)));
    }
    else if (subtype > 0 && subtype < CSeqFeatData::eSubtype_any && field_type == CFieldNamePanel::eFieldType_CDSGeneProt)
    {
        return CIRef<IEditingAction>(CreateActionCdsGeneProt(seh, field, static_cast<CSeqFeatData::ESubtype>(subtype)));
    }  
    else if (field_type == CFieldNamePanel::eFieldType_Source || field_type == CFieldNamePanel::eFieldType_Taxname || field_type == CFieldNamePanel::eFieldType_TaxnameAfterBinomial 
             || field_type == CFieldNamePanel::eFieldType_Dbxref)
    {
        return CIRef<IEditingAction>(CreateActionBiosource(seh, field)); 
    }   
    else if (field_type == CFieldNamePanel::eFieldType_Misc || field_type == CFieldNamePanel::eFieldType_StructuredComment || field_type == CFieldNamePanel::eFieldType_DBLink
             || field_type == CFieldNamePanel::eFieldType_MolInfo || field_type == CFieldNamePanel::eFieldType_Pub  || field_type == CFieldNamePanel::eFieldType_LocalId 
             || field_type == CFieldNamePanel::eFieldType_DefLine || field_type == CFieldNamePanel::eFieldType_BankITComment || field_type == CFieldNamePanel::eFieldType_Comment)
    {
        return CIRef<IEditingAction>(CreateActionDesc(seh, field, submit));
    }
    else if (field_type == CFieldNamePanel::eFieldType_SeqId || field_type == CFieldNamePanel::eFieldType_FileId || field_type == CFieldNamePanel::eFieldType_GeneralId)
    {
        return CIRef<IEditingAction>(CreateActionSeqid(seh, field));
    }
    else if (field_type == CFieldNamePanel::eFieldType_FlatFile)
    {
        return CIRef<IEditingAction>(CreateActionMisc(seh, field));
    }
    return CIRef<IEditingAction>(NULL);
}


int GetSubtype(CFieldNamePanel *field_name_panel)
{
    if (!field_name_panel)
        return -1;

    CFeatureFieldNamePanel *feat_panel = dynamic_cast<CFeatureFieldNamePanel*>(field_name_panel);
    if (feat_panel)
    {
        string field_name = feat_panel->GetFeatureType();
        if (field_name.empty())
            return -1;
        CSeqFeatData::ESubtype subtype = CSeqFeatData::SubtypeNameToValue(field_name);
        return subtype;
    }

    CCDSGeneProtFieldNamePanel *cds_gene_prot = dynamic_cast<CCDSGeneProtFieldNamePanel*>(field_name_panel);
    if (cds_gene_prot)
    {
        string field = cds_gene_prot->GetFieldName();
        if (field == "protein name" || field == "protein description" || field == "protein EC number"
            || field == "protein activity" || field == "protein comment")
        {
            return CSeqFeatData::eSubtype_prot;
        }
        else if (field == "CDS comment" || field == "CDS inference" || field == "codon-start")
        {
            return CSeqFeatData::eSubtype_cdregion;
        }
        else if ( field == "gene description" || field == "gene comment" 
                 || field == "gene inference" || field == "gene allele" || field == "gene maploc"
                 || field == "gene locus tag" || field == "gene synonym" || field == "gene old_locus_tag"
                  || field == "gene locus")
        {
            return CSeqFeatData::eSubtype_gene;
        }       
        else if (field == "mRNA product" || field == "mRNA comment")
        {
            return CSeqFeatData::eSubtype_mRNA;
        }
        else if (field == "mat_peptide name" || field == "mat_peptide description" || field == "mat_peptide EC number"
                 || field == "mat_peptide activity" || field == "mat_peptide comment")
        {
            return CSeqFeatData::eSubtype_mat_peptide_aa;
        }
    }

    CRNAFieldNamePanel *rna_panel = dynamic_cast<CRNAFieldNamePanel*>(field_name_panel);
    CDualRNAFieldNamePanel *dual_rna_panel = dynamic_cast<CDualRNAFieldNamePanel*>(field_name_panel);
    string rna_type;
    string ncrna_class;
    if (rna_panel)
    {
        rna_type = rna_panel->GetRnaTypeOnly();
        ncrna_class = rna_panel->GetNcrnaType();
    }
    if (dual_rna_panel)
    {
        rna_type = dual_rna_panel->GetRnaTypeOnly();
        ncrna_class = dual_rna_panel->GetNcrnaType();
    }
    
    if (!rna_type.empty())
    { 
        if (rna_type == "preRNA")
            return CSeqFeatData::eSubtype_preRNA;
        else if (rna_type =="mRNA")
            return CSeqFeatData::eSubtype_mRNA;
        else if (rna_type == "tRNA")
            return CSeqFeatData::eSubtype_tRNA;
        else if (rna_type == "rRNA")
            return CSeqFeatData::eSubtype_rRNA;
        else if (rna_type == "ncRNA")
        {
            if (ncrna_class == "snRNA")
                return CSeqFeatData::eSubtype_snRNA;
            else if (ncrna_class == "scRNA")
                return  CSeqFeatData::eSubtype_scRNA;
            else if (ncrna_class == "snoRNA")
                return CSeqFeatData::eSubtype_snoRNA;
            return CSeqFeatData::eSubtype_ncRNA;
        }
        else if (rna_type == "tmRNA")
            return CSeqFeatData::eSubtype_tmRNA;
        else if (rna_type == "miscRNA")
            return CSeqFeatData::eSubtype_otherRNA;
    }
    
// CSourceFieldNamePanel
// CGeneFieldPanel
// CProteinFieldPanel
// CFieldHandlerNamePanel
// CStructCommentFieldPanel
// CDbxrefNamePanel
// CMiscFieldPanel
// CGeneralIDPanel
    return -1;
}

CRef<CObject> GetNewObject(CConstRef<CObject> object)
{
    const CSeqdesc * old_desc = dynamic_cast<const CSeqdesc *>(object.GetPointer());
    const CSeq_feat* old_feat = dynamic_cast<const CSeq_feat * >(object.GetPointer());
    CRef<CObject> new_obj(NULL);
        
    if (old_desc) {
        // prepare replacement descriptor
        CRef<CSeqdesc> new_desc( new CSeqdesc() );
        new_desc->Assign(*old_desc);      
        new_obj = (CObject *)new_desc;
    } else if (old_feat) {
        // prepare replacement feature
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(*old_feat);
        new_obj = (CObject *)new_feat;
    }
    return new_obj;
}


CRef<CCmdComposite> GetReplacementCommand(CConstRef<CObject> oldobj, CRef<CObject> newobj, CScope& scope, const string& cmd_name)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Convert Value"));
    bool success = false;

    const CSeqdesc * old_desc = dynamic_cast<const CSeqdesc *>(oldobj.GetPointer());
    const CSeq_feat* old_feat = dynamic_cast<const CSeq_feat * >(oldobj.GetPointer());

    if (old_desc) {
        CSeq_entry_Handle seh = edit::GetSeqEntryForSeqdesc(Ref(&scope), *old_desc);
        const CSeqdesc* new_desc = dynamic_cast<const CSeqdesc*>(newobj.GetPointer());
        if (seh && new_desc) {
            CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(seh, *old_desc, *new_desc));
            cmd->AddCommand(*ecmd);
            success = true;
        }
    } else if (old_feat) {
        CSeq_feat* new_feat = dynamic_cast<CSeq_feat * >(newobj.GetPointer());
        cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(scope.GetSeq_featHandle(*old_feat), *new_feat)));
        success = true;
    }

    if (!success) {
        cmd.Reset(NULL);
    }
    return cmd;
}

vector<CConstRef<CObject> > GetObjects(CSeq_entry_Handle seh, const string &field, CFieldNamePanel::EFieldType field_type, int subtype, CConstRef<objects::CSeq_submit> submit,
                       CRef<CEditingActionConstraint> constraint, vector<CSeq_entry_Handle>* descr_context)
    
{
    CIRef<IEditingAction> action = CreateAction(seh, field, field_type, subtype, submit);                                                                            
    action->SetConstraint(constraint);
    action->NOOP();
    
    const map<CSeq_feat_Handle, CRef<CSeq_feat>>& feathdlesmap = action->GetChangedFeatures();
    const map<const CSeqdesc*, CRef<CSeqdesc>>& descrmap = action->GetChangedDescriptors();
    const map<CBioseq_Handle, CRef<CSeq_inst>>& instmap = action->GetChangedInstances();
    CRef<CSubmit_block> submit_block = action->GetChangedSubmitBlock();
    vector<CConstRef<CObject> > objs;
    for (auto &fh : feathdlesmap)
    {
        objs.emplace_back(fh.first.GetSeq_feat());
    } 
    for (auto &dh : descrmap)
    {
        objs.emplace_back(CConstRef<CObject>(dh.first));
    } 
    for (auto &bh : instmap)
    {
        objs.emplace_back(CConstRef<CObject>(&bh.first.GetInst()));
    }
    if (submit_block)
    {
        objs.emplace_back(CConstRef<CObject>(&submit->GetSub()));
    }

    if (!descrmap.empty() && descr_context) {
        const map<const CSeqdesc*, CSeq_entry_Handle>& contextmap = action->GetContextForDescriptors();
        descr_context->clear();
        for (auto& it : contextmap) {
            descr_context->emplace_back(it.second);
        }
    }
    return objs;
}

END_NCBI_SCOPE

