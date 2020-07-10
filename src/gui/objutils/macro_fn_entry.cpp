/*  $Id: macro_fn_entry.cpp 44325 2019-11-29 20:29:44Z asztalos $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Andrea Asztalos
 *
 */

#include <ncbi_pch.hpp>

#include <util/xregexp/regexp.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <objmgr/bioseq_ci.hpp>

#include <objtools/edit/autodef_with_tax.hpp>

#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/convert_raw_to_delta.hpp>

#include <gui/objutils/macro_lib.hpp>
#include <gui/objutils/macro_fn_entry.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_TaxLookup
/// DoTaxLookup() - performs taxonomy lookup, extended cleanup (synch genetic codes)
///

DEFINE_MACRO_FUNCNAME(CMacroFunction_TaxLookup, "DoTaxLookup")
void CMacroFunction_TaxLookup::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(obj.GetPointer());
    if (!entry) {
        return;
    }

    // performs Tax Lookup and Extended Cleanup - with the option to correct genetic codes in coding regions
    CRef<CCmdComposite> cleanup_tax_cmd = CleanupCommand(m_DataIter->GetSEH(), true, true);
    if (cleanup_tax_cmd) {
        m_DataIter->RunCommand(cleanup_tax_cmd, m_CmdComposite);
        CNcbiOstrstream log;
        log << "Performed TaxLookup and corrected genetic codes";
        x_LogFunction(log);
    }
}

bool CMacroFunction_TaxLookup::x_ValidArguments() const
{
    return (m_Args.empty());
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_FixSpelling
/// Usage: FixSpelling();
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_FixSpelling, "FixSpelling")
void CMacroFunction_FixSpelling::TheFunction()
{
    // the iterator should iterate over TSEntry
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_entry* entry = CTypeConverter<CSeq_entry>::SafeCast(oi.GetObjectPtr());
    if (!entry)
        return;

    CNcbiOstrstream log;
    s_SpellingFixes(*entry, log);

    // make changes in the Seq-submit block as well
    if (m_DataIter->IsSetSeqSubmit() && m_DataIter->GetSeqSubmit().IsSetSub()){
        const CSubmit_block& submit_block = m_DataIter->GetSeqSubmit().GetSub();
        CRef<CSubmit_block> edited_subblock(new CSubmit_block);
        edited_subblock->Assign(submit_block);

        CNcbiOstrstream log_sblock;
        s_SpellingFixes(*edited_subblock, log_sblock);

        if (!IsOssEmpty(log_sblock)) {
            CChangeSubmitBlockCommand* chg_subblock = new CChangeSubmitBlockCommand();
            CObject* actual = (CObject*)&submit_block;
            chg_subblock->Add(actual, CConstRef<CObject>(edited_subblock));
            CRef<CCmdComposite> cmd(new CCmdComposite("Fix spelling in the submission block"));
            cmd->AddCommand(*chg_subblock);
            m_DataIter->RunCommand(cmd, m_CmdComposite);
            log << log_sblock.str();
        }
    }

    if (!IsOssEmpty(log)) {
        m_DataIter->SetModified();
        x_LogFunction(log);
    }
}

bool CMacroFunction_FixSpelling::x_ValidArguments() const
{
    return (m_Args.empty());
}

static const SStaticPair<const char*, const char*> macro_spell_fixes[] = {
    { "\\bAgricultrual\\b", "Agricultural" },
    { "\\bAgricultureal\\b", "Agricultural" },
    { "\\bAgricultrure\\b", "Agriculture" },
    { "\\bbioremidiation\\b", "bioremediation" },
    { "\\bColledge\\b", "College" },
    { "\\bInsitiute\\b", "Institute" },
    { "\\bInstutite\\b", "Institute" },
    { "\\binstute\\b", "Institute" },
    { "\\binstitue\\b", "Institute" },
    { "\\binsitute\\b", "Institute" },
    { "\\binsititute\\b", "Institute" },
    { "\\bInstiute\\b", "Institute" },
    { "\\bhpothetical\\b", "hypothetical" },
    { "\\bhyphotetical\\b", "hypothetical" },
    { "\\bhyphotheical\\b", "hypothetical" },
    { "\\bhypotehtical\\b", "hypothetical" },
    { "\\bhypotethical\\b", "hypothetical" },
    { "\\bhypotetical\\b", "hypothetical" },
    { "\\bhypotheical\\b", "hypothetical" },
    { "\\bhypotheitcal\\b", "hypothetical" },
    { "\\bhypothetcial\\b", "hypothetical" },
    { "\\bhypothetica\\b", "hypothetical" },
    { "\\bhypothteical\\b", "hypothetical" },
    { "\\bhypothtical\\b", "hypothetical" },
    { "\\bhypthetical\\b", "hypothetical" },
    { "\\bhyptothetical\\b", "hypothetical" },
    { "\\bidendification\\b", "identification" },
    { "\\bprotien\\b", "protein" },
    { "\\bpuatative\\b", "putative" },
    { "\\bpuative\\b", "putative" },
    { "\\bpuative\\b", "putative" },
    { "\\bputaitive\\b", "putative" },
    { "\\bputaitve\\b", "putative" },
    { "\\bputaive\\b", "putative" },
    { "\\bputataive\\b", "putative" },
    { "\\bputatitve\\b", "putative" },
    { "\\bputitive\\b", "putative" },
    { "\\breseach\\b", "research" },
    { "\\bsequene\\b", "sequence" },
    { "\\buniveristy\\b", "University" },
    { "\\buniverisity\\b", "University" },
    { "\\bunivercity\\b", "University" },
    { "\\buiniversity\\b", "University" },
    { "\\buinversity\\b", "University" },
    { "\\bunivesity\\b", "University" },
    { "\\buviversity\\b", "University" },
    { "\\buniverstiy\\b", "University" },
    { "\\bunvierstity\\b", "University" },
    { "\\buniviersity\\b", "University" },
    { "\\buniverstity\\b", "University" },
    { "\\bUnversity\\b", "University" },
    { "\\bUnivresity\\b", "University" },
    { "\0", "\0" }
};


void CMacroFunction_FixSpelling::s_SpellingFixes(CSerialObject& object, CNcbiOstrstream& oss)
{
    // case-insensitive and whole word matching
    for (CStdTypeIterator<string> it(object); it; ++it) {
        for (size_t pat = 0; macro_spell_fixes[pat].first[0] != '\0'; ++pat) {
            CRegexpUtil replacer(*it);
            if (replacer.Replace(macro_spell_fixes[pat].first, macro_spell_fixes[pat].second,
                CRegexp::fCompile_ignore_case, CRegexp::fMatch_default, 0) > 0) {
                replacer.GetResult().swap(*it);
                string search(macro_spell_fixes[pat].first);
                oss << "Replaced " << search.substr(2, search.length() - 4) << " with " << macro_spell_fixes[pat].second << endl;
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RemoveSingleItemSet
/// RemoveSingleItemSet();
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveSingleItemSet, "RemoveSingleItemSet")
void CMacroFunction_RemoveSingleItemSet::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_entry* entry = CTypeConverter<CSeq_entry>::SafeCast(oi.GetObjectPtr());
    if (!entry)
        return;

    CRef<CScope> new_scope(new CScope(*CObjectManager::GetInstance()));
    new_scope->AddDefaults();
    CSeq_entry_Handle seh = new_scope->AddTopLevelSeqEntry(*entry);

    m_QualsChangedCount = s_RemoveSingleItemSet(seh);
    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": Removed " << m_QualsChangedCount << " wrapper set";
        x_LogFunction(log);
    }
}

short CMacroFunction_RemoveSingleItemSet::s_RemoveSingleItemSet(CSeq_entry_Handle& seh)
{
    short count(0);
    for (CSeq_entry_CI it(seh, CSeq_entry_CI::fIncludeGivenEntry | CSeq_entry_CI::eNonRecursive, CSeq_entry::e_Set); it; ++it) {
        CBioseq_set_Handle bssh = it->GetSet();
        if (!bssh.IsSetClass())
            continue;

        if (bssh.GetClass() == CBioseq_set::eClass_eco_set ||
            bssh.GetClass() == CBioseq_set::eClass_phy_set ||
            bssh.GetClass() == CBioseq_set::eClass_pop_set ||
            bssh.GetClass() == CBioseq_set::eClass_mut_set) {

            if (s_IsSingletonSet(bssh)) {
                CSeq_entry_EditHandle edit_seh(*it);
                edit_seh.CollapseSet();

                // delete multiple titles
                CSeqdesc_CI desc(edit_seh, CSeqdesc::e_Title, 1);
                if (desc) {
                    ++desc;
                }
                while (desc) {
                    const CSeqdesc& seqdesc = *desc;
                    ++desc;
                    edit_seh.RemoveSeqdesc(seqdesc);
                }
                count++;
            }
        }
    }

    return count;
}

bool CMacroFunction_RemoveSingleItemSet::s_IsSingletonSet(const CBioseq_set_Handle& bioseq_set)
{
    CSeq_entry_CI direct_child_ci(bioseq_set, CSeq_entry_CI::eNonRecursive);
    if (!direct_child_ci) {
        // not singleton: has no children
        return false;
    }
    ++direct_child_ci;
    if (direct_child_ci) {
        // not singleton: has more than one child
        return false;
    }

    // not singleton if has any alignment annots
    CSeq_annot_CI annot_ci(bioseq_set, CSeq_annot_CI::eSearch_entry);
    for (; annot_ci; ++annot_ci) {
        if (annot_ci->IsAlign()) {
            return false;
        }
    }

    // it's a singleton: it passed all tests
    return true;
}

bool CMacroFunction_RemoveSingleItemSet::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RenormalizeNucProtSet
/// RenormalizeNucProtSet();
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RenormalizeNucProtSet, "RenormalizeNucProtSet")

void CMacroFunction_RenormalizeNucProtSet::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_entry* entry = CTypeConverter<CSeq_entry>::SafeCast(oi.GetObjectPtr());
    if (!entry) {
        return;
    }

    CRef<CScope> new_scope(new CScope(*CObjectManager::GetInstance()));
    new_scope->AddDefaults();
    CSeq_entry_Handle seh = new_scope->AddTopLevelSeqEntry(*entry);

    m_QualsChangedCount = 0;
    s_RenormalizeNucProtSets(seh, m_QualsChangedCount);

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << "Renormalized " << m_QualsChangedCount << " sets";
        x_LogFunction(log);
    }
}

void CMacroFunction_RenormalizeNucProtSet::s_RenormalizeNucProtSets(CSeq_entry_Handle seh, Int4& count)
{
    if (seh.IsSet()
        && seh.GetSet().IsSetClass()
        && !seh.GetSet().IsEmptySeq_set()) {

        CBioseq_set::TClass set_class = seh.GetSet().GetClass();
        CConstRef<CSeq_entry> entry = seh.GetCompleteSeq_entry();

        if (set_class == CBioseq_set::eClass_nuc_prot) {
            if (entry->GetSet().GetSeq_set().size() == 1
                && entry->GetSet().GetSeq_set().front()->IsSeq()) {
                CSeq_entry_EditHandle eh = seh.GetEditHandle();
                eh.ConvertSetToSeq();
                count++;
            }
        }
        else if (set_class == CBioseq_set::eClass_genbank ||
            set_class == CBioseq_set::eClass_mut_set ||
            set_class == CBioseq_set::eClass_pop_set ||
            set_class == CBioseq_set::eClass_phy_set ||
            set_class == CBioseq_set::eClass_eco_set ||
            set_class == CBioseq_set::eClass_wgs_set ||
            set_class == CBioseq_set::eClass_gen_prod_set ||
            set_class == CBioseq_set::eClass_small_genome_set) {
            ITERATE(CBioseq_set::TSeq_set, it, entry->GetSet().GetSeq_set()) {
                CSeq_entry_Handle next_seh = seh.GetScope().GetSeq_entryHandle(**it);
                s_RenormalizeNucProtSets(next_seh, count);
            }
        }
    }
}

bool CMacroFunction_RenormalizeNucProtSet::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_DiscrepancyAutofix
/// PerformDiscrAutofix("test_name");
/// 
// Changes in the function and parameter names require changes in the respective
// XRC file used in the macro editor
DEFINE_MACRO_FUNCNAME(CMacroFunction_DiscrepancyAutofix, "PerformDiscrAutofix")

const vector<SArgMetaData>
CMacroFunction_DiscrepancyAutofix::sm_Arguments { SArgMetaData("test_name", CMQueryNodeValue::eString, false) };

void CMacroFunction_DiscrepancyAutofix::TheFunction()
{
    // the iterator should iterate over TSEntry
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(obj.GetPointer());
    if (!entry) {
        return;
    }

    // perform Autofix for the given Discrepancy test
    string output;
    CRef<CCmdComposite> autofix_cmd = AutofixCommand(m_DataIter->GetSEH(), m_Args[0]->GetString(), &output, CMacroLib::GetInstance().GetSuspectRules());
    if (autofix_cmd) {
        m_DataIter->RunCommand(autofix_cmd, m_CmdComposite);
        CNcbiOstrstream log;
        log << output;
        x_LogFunction(log);
    }
}

bool CMacroFunction_DiscrepancyAutofix::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eString);
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_Autodef
/// Autodef(clause_list_type, misc_feat_parse_rule, modifier1, modifier2, ...);
/// 
DEFINE_MACRO_FUNCNAME(CMacroFunction_Autodef, "Autodef")
const vector<SArgMetaData> CMacroFunction_Autodef::sm_Arguments
{ SArgMetaData("source_field", CMQueryNodeValue::eString, false)
, SArgMetaData("list_feat_rule", CMQueryNodeValue::eString, false)
, SArgMetaData("misc_feat_check", CMQueryNodeValue::eBool, false)
, SArgMetaData("misc_feat_rule", CMQueryNodeValue::eString, false)
, SArgMetaData("select_all", CMQueryNodeValue::eBool, false)
};

void CMacroFunction_Autodef::TheFunction()
{
    // the iterator should iterate over TSEntry
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(obj.GetPointer());
    const CBioseq* bseq = dynamic_cast<const CBioseq*>(obj.GetPointer());
    if (!entry && !(bseq && bseq->IsNa())) {
        return;
    }

    CAutoDefOptions options;
    CAutoDefOptions::TFeatureListType feat_list_type = options.GetFeatureListType(m_Args[0]->GetString());
    options.SetFeatureListType((CAutoDefOptions::EFeatureListType)feat_list_type);
    string feat_list = m_Args[0]->GetString();
    NStr::ToLower(feat_list);
    m_Descr.append(feat_list + ", ");

    CAutoDefOptions::TMiscFeatRule misc_feat_rule = options.GetMiscFeatRule(m_Args[1]->GetString());
    options.SetMiscFeatRule((CAutoDefOptions::EMiscFeatRule)misc_feat_rule);
    string misc_feat = m_Args[1]->GetString();
    NStr::ToLower(misc_feat);
    m_Descr.append("use misc_feat with comments " + misc_feat);
    
    x_AddModifiersToOptions(options);

    CRef<CCmdComposite> autodef_cmd;
    if (entry) {
        autodef_cmd = s_AutodefSeqEntry(m_DataIter->GetSEH(), options);
    }
    else if (bseq) {
        autodef_cmd = s_AutodefSequence(m_DataIter->GetSEH(), m_DataIter->GetBioseqHandle(), options);
    }

    if (autodef_cmd) {
        m_DataIter->RunCommand(autodef_cmd, m_CmdComposite);
        CNcbiOstrstream log;
        log << x_GetDescription();
        x_LogFunction(log);
    }
}

CRef<CCmdComposite> CMacroFunction_Autodef::s_AutodefSeqEntry(const CSeq_entry_Handle& seh, const CAutoDefOptions& options)
{
    CAutoDefWithTaxonomy autodef;
    autodef.AddSources(seh);
    CRef<CAutoDefModifierCombo> mod_combo = Ref(autodef.GetEmptyCombo());
    mod_combo->InitFromOptions(options);
    mod_combo->SetUseModifierLabels(true);
    mod_combo->SetKeepParen(true);
    mod_combo->SetMaxModifiers(0);

    autodef.SetFeatureListType((CAutoDefOptions::EFeatureListType)options.GetFeatureListType());
    autodef.SetMiscFeatRule((CAutoDefOptions::EMiscFeatRule)options.GetMiscFeatRule());

    CRef<CCmdComposite> cmd(new CCmdComposite("Autodef"));
    s_AutodefBioseqs(seh, autodef, mod_combo, cmd);
    s_AutodefSets(seh, autodef, cmd);
    return cmd;
}

void CMacroFunction_Autodef::s_AutodefBioseqs(const CSeq_entry_Handle& seh, CAutoDefWithTaxonomy& autodef, CAutoDefModifierCombo* mod_combo, CRef<CCmdComposite> composite_cmd)
{
    for (CBioseq_CI b_iter(seh); b_iter; ++b_iter) {
        string defline = autodef.GetOneDefLine(mod_combo, *b_iter);

        CRef<CSeqdesc> new_desc(new CSeqdesc());
        new_desc->SetTitle(defline);
        CSeqdesc_CI desc(*b_iter, CSeqdesc::e_Title, 1);
        if (desc) {
            CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc.GetSeq_entry_Handle(), *desc, *new_desc));
            composite_cmd->AddCommand(*ecmd);
            while (++desc) {
                CRef<CCmdDelDesc> delcmd(new CCmdDelDesc(desc.GetSeq_entry_Handle(), *desc));
                composite_cmd->AddCommand(*delcmd);
            }
        }
        else {
            composite_cmd->AddCommand(*CRef<CCmdCreateDesc>(new CCmdCreateDesc((*b_iter).GetSeq_entry_Handle(), *new_desc)));
        }

        s_AddAutodefOptions(autodef, *b_iter, composite_cmd);
    }
}

void CMacroFunction_Autodef::s_AutodefSets(const CSeq_entry_Handle& seh, CAutoDefWithTaxonomy& autodef, CRef<CCmdComposite> composite_cmd)
{
    for (CSeq_entry_CI si(seh, CSeq_entry_CI::fRecursive | CSeq_entry_CI::fIncludeGivenEntry, CSeq_entry::e_Set); si; ++si) {
        if (si->IsSet()
            && si->GetSet().GetCompleteBioseq_set()->NeedsDocsumTitle()) {
            string defline = autodef.GetDocsumDefLine(*si);
            CRef<CSeqdesc> new_desc(new CSeqdesc());
            new_desc->SetTitle(defline);
            CSeqdesc_CI desc(*si, CSeqdesc::e_Title, 1);
            if (desc) {
                CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc.GetSeq_entry_Handle(), *desc, *new_desc));
                composite_cmd->AddCommand(*ecmd);
            }
            else {
                composite_cmd->AddCommand(*CRef<CCmdCreateDesc>(new CCmdCreateDesc(*si, *new_desc)));
            }
        }
    }
}

CRef<CCmdComposite> CMacroFunction_Autodef::s_AutodefSequence(const CSeq_entry_Handle& seh, const CBioseq_Handle& target, const CAutoDefOptions& options)
{
    CAutoDefWithTaxonomy autodef;
    autodef.AddSources(seh);
    CRef<CAutoDefModifierCombo> mod_combo = Ref(autodef.GetEmptyCombo());
    mod_combo->InitFromOptions(options);
    mod_combo->SetUseModifierLabels(true);
    mod_combo->SetKeepParen(true);
    mod_combo->SetMaxModifiers(0);

    autodef.SetFeatureListType((CAutoDefOptions::EFeatureListType)options.GetFeatureListType());
    autodef.SetMiscFeatRule((CAutoDefOptions::EMiscFeatRule)options.GetMiscFeatRule());

    CRef<CCmdComposite> cmd(new CCmdComposite("Autodef"));

    string defline = autodef.GetOneDefLine(mod_combo, target);

    CRef<CSeqdesc> new_desc(new CSeqdesc());
    new_desc->SetTitle(defline);
    CSeqdesc_CI desc(target, CSeqdesc::e_Title, 1);
    if (desc) {
        CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc.GetSeq_entry_Handle(), *desc, *new_desc));
        cmd->AddCommand(*ecmd);
    }
    else {
        cmd->AddCommand(*CRef<CCmdCreateDesc>(new CCmdCreateDesc(target.GetSeq_entry_Handle(), *new_desc)));
    }

    s_AddAutodefOptions(autodef, target, cmd);
    return cmd;
}

void CMacroFunction_Autodef::s_AddAutodefOptions(const CAutoDef& autodef, const CBioseq_Handle& bsh, CRef<CCmdComposite> composite_cmd)
{
    CRef<CUser_object> autodef_opts = autodef.GetOptionsObject();
    if (bsh.IsAa() || autodef_opts.IsNull() || composite_cmd.IsNull()) {
        return;
    }

    CRef<CSeqdesc> opts_desc(new CSeqdesc);
    opts_desc->SetUser(*autodef_opts);
    bool has_autodef_opts = false;
    for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_User); desc_it; ++desc_it) {
        if (desc_it->GetUser().GetObjectType() == CUser_object::eObjectType_AutodefOptions) {
            has_autodef_opts = true;
            CRef<CCmdChangeSeqdesc> chg_cmd(new CCmdChangeSeqdesc(desc_it.GetSeq_entry_Handle(), *desc_it, *opts_desc));
            composite_cmd->AddCommand(*chg_cmd);
            break;
        }
    }

    if (!has_autodef_opts) {
        composite_cmd->AddCommand(*CRef<CCmdCreateDesc>(new CCmdCreateDesc(bsh.GetSeq_entry_Handle(), *opts_desc)));
    }
}

bool CMacroFunction_Autodef::x_ValidArguments() const
{
    if (m_Args.empty() || m_Args.size() < 2){
        return false;
    }

    for (auto&& it : m_Args) {
        if (it->GetDataType() != CMQueryNodeValue::eString) {
            return false;
        }
    }
    return true;
}

bool CMacroFunction_Autodef::x_IsRequested(const CAutoDefAvailableModifier& modifier)
{
    for (size_t index = 2; index < m_Args.size(); ++index) {
        const string& mod_name = m_Args[index]->GetString();
        if (modifier.IsOrgMod() && COrgMod::IsValidSubtypeName(mod_name)) {
            COrgMod::TSubtype st = COrgMod::GetSubtypeValue(mod_name);
            if ((COrgMod::ESubtype)(st) == modifier.GetOrgModType()) {
                m_Descr.append(", with " + mod_name);
                return true;
            }
        }
        else if (!modifier.IsOrgMod() && CSubSource::IsValidSubtypeName(mod_name)) {
            CSubSource::TSubtype st = CSubSource::GetSubtypeValue(mod_name);
            if ((CSubSource::ESubtype)(st) == modifier.GetSubSourceType()) {
                m_Descr.append(", with " + mod_name);
                return true;
            }
        }
    }
    return false;
}

void CMacroFunction_Autodef::x_AddModifiersToOptions(CAutoDefOptions& opts)
{
    if (m_Args.size() < 3) {
        return;
    }

    CAutoDef autodef;
    autodef.AddSources(m_DataIter->GetSEH());
    CRef<CAutoDefModifierCombo> src_combo = autodef.FindBestModifierCombo();
    CAutoDefSourceDescription::TAvailableModifierVector modifiers;
    src_combo->GetAvailableModifiers(modifiers);

    for (size_t n = 0; n < modifiers.size(); n++) {
        if (modifiers[n].AnyPresent() && x_IsRequested(modifiers[n])) {
            if (modifiers[n].IsOrgMod()) {
                opts.AddOrgMod(modifiers[n].GetOrgModType());
            }
            else {
                opts.AddSubSource(modifiers[n].GetSubSourceType());
            }
        }
    }
}

string CMacroFunction_Autodef::x_GetDescription()
{
    return "Performed Autodef " + m_Descr;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_AutodefId
/// AutodefId()
DEFINE_MACRO_FUNCNAME(CMacroFunction_AutodefId, "AutodefId")

void CMacroFunction_AutodefId::TheFunction()
{
    // the iterator should iterate over TSEntry
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(obj.GetPointer());
    if (!entry) {
        return;
    }


    CAutoDefSourceDescription::TAvailableModifierVector modifiers;
    s_ConfigureAutodefOptionsForID(modifiers, m_DataIter->GetSEH());

    CAutoDefOptions opts;
    opts.SetUseLabels(true);
    opts.SetMiscFeatRule(CAutoDefOptions::eDelete);

    for (size_t n = 0; n < modifiers.size(); n++) {
        if (modifiers[n].IsRequested()) {
            if (modifiers[n].IsOrgMod()) {
                opts.AddOrgMod(modifiers[n].GetOrgModType());
            }
            else {
                opts.AddSubSource(modifiers[n].GetSubSourceType());
            }
        }
    }

    CRef<CCmdComposite> autodef_cmd = CMacroFunction_Autodef::s_AutodefSeqEntry(m_DataIter->GetSEH(), opts);
    if (autodef_cmd) {
        m_DataIter->RunCommand(autodef_cmd, m_CmdComposite);
        CNcbiOstrstream log;
        log << "Autodef_id sequences";
        x_LogFunction(log);
    }
}

bool CMacroFunction_AutodefId::x_ValidArguments() const
{
    return (m_Args.empty());
}

static bool s_ChooseModInModList(bool is_org_mod, int subtype, bool require_all, CAutoDefSourceDescription::TAvailableModifierVector& modifiers)
{
    bool rval = false;
    for (size_t n = 0; n < modifiers.size(); n++) {
        if (modifiers[n].IsOrgMod() && is_org_mod) {
            if (modifiers[n].GetOrgModType() == subtype) {
                if (modifiers[n].AllPresent()) {
                    rval = true;
                }
                else if (modifiers[n].AnyPresent() && !require_all) {
                    rval = true;
                }
                if (rval) {
                    modifiers[n].SetRequested(true);
                }
                break;
            }
        }
        else if (!modifiers[n].IsOrgMod() && !is_org_mod) {
            if (modifiers[n].GetSubSourceType() == subtype) {
                if (modifiers[n].AllPresent()) {
                    rval = true;
                }
                else if (modifiers[n].AnyPresent() && !require_all) {
                    rval = true;
                }
                if (rval) {
                    modifiers[n].SetRequested(true);
                }
                break;
            }
        }
    }
    return rval;
}

static bool s_AreFeatureClausesUnique(CSeq_entry_Handle seh, CAutoDef& autodef)
{
    vector<string> clause_list;
    for (CBioseq_CI bi(seh, CSeq_inst::eMol_na); bi; ++bi) {
        string clause = autodef.GetOneFeatureClauseList(*bi, CBioSource::eGenome_unknown);
        clause_list.push_back(clause);
    }
    if (clause_list.size() < 2) {
        return true;
    }
    sort(clause_list.begin(), clause_list.end());

    vector<string>::iterator it = clause_list.begin();
    string prev = *it;
    it++;
    while (it != clause_list.end()) {
        if (NStr::Equal(prev, *it)) {
            return false;
        }
        prev = *it;
    }
    return true;
}

void CMacroFunction_AutodefId::s_ConfigureAutodefOptionsForID(CAutoDefSourceDescription::TAvailableModifierVector& modifiers, CSeq_entry_Handle seh)
{
    CAutoDef autodef;
    autodef.AddSources(seh);
    CRef<CAutoDefModifierCombo> src_combo = autodef.FindBestModifierCombo();
    src_combo->GetAvailableModifiers(modifiers);

    static int subtypes[] = { COrgMod::eSubtype_strain,
        CSubSource::eSubtype_clone,
        COrgMod::eSubtype_isolate,
        CSubSource::eSubtype_haplotype,
        COrgMod::eSubtype_cultivar,
        COrgMod::eSubtype_ecotype,
        COrgMod::eSubtype_breed,
        COrgMod::eSubtype_specimen_voucher,
        COrgMod::eSubtype_culture_collection,
        COrgMod::eSubtype_bio_material };
    static bool is_orgmod[] = { true, false, true, false, true, true, true, true, true, true };
    static int num_subtypes = sizeof(subtypes) / sizeof(int);

    bool found = false;
    // first look for best identifier found in all
    for (int i = 0; i < num_subtypes && !found; i++) {
        found = s_ChooseModInModList(is_orgmod[i], subtypes[i], true, modifiers);
    }
    if (!found) {
        // if not found in all, use best identifier found in some
        for (int i = 0; i < num_subtypes && !found; i++) {
            found = s_ChooseModInModList(is_orgmod[i], subtypes[i], false, modifiers);
        }
    }
    if (!s_AreFeatureClausesUnique(seh, autodef)) {
        // use best
        for (size_t n = 0; n < modifiers.size(); n++) {
            if (modifiers[n].AnyPresent()) {
                if (modifiers[n].IsOrgMod()) {
                    if (src_combo->HasOrgMod(modifiers[n].GetOrgModType())) {
                        modifiers[n].SetRequested(true);
                    }
                }
                else if (src_combo->HasSubSource(modifiers[n].GetSubSourceType())) {
                    modifiers[n].SetRequested(true);
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_CreateProteinFeats
/// CreateProteinFeatures()
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_CreateProteinFeats, "CreateProteinFeatures")
void CMacroFunction_CreateProteinFeats::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!scope || !entry) 
        return;

    if (m_DataIter->IsBegin()) {
        m_ProductToCds.clear();
        GetProductToCDSMap(*scope, m_ProductToCds);
    }

    CRef<CCmdComposite> cmd(new CCmdComposite("Create Protein Features and adjust coding regions"));
    for (auto& it : m_ProductToCds) {
        CBioseq_Handle product = it.first;
        if (!CFeat_CI(product, CSeqFeatData::eSubtype_prot)) {
            CRef<CSeq_feat> prot(new CSeq_feat());
            set<CSeq_feat_Handle> cds_set = it.second;
            if (cds_set.size() == 1) {
                CRef<CSeq_feat> new_cds(new CSeq_feat);
                new_cds->Assign(*cds_set.begin()->GetOriginalSeq_feat());
                bool cds_change = false;
                AdjustProteinFeature(*prot, product, *new_cds, cds_change);
                if (cds_change) {
                    cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(*cds_set.begin(), *new_cds)));
                }

                CSeq_entry_Handle psh = product.GetSeq_entry_Handle();
                cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(psh, *prot)));
                m_QualsChangedCount++;
            }
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->RunCommand(cmd, m_CmdComposite);
        CNcbiOstrstream log;
        log << "Created " << m_QualsChangedCount << " protein features";
        x_LogFunction(log);
    }
}

bool CMacroFunction_CreateProteinFeats::x_ValidArguments() const
{
    return (m_Args.empty());
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ConvertRawToDeltabyNs
/// AddAssemblyGapsbyNs(min_unknown, max_unknown, min_known, max_known, adjust_cds (false), keep_gap_length, gap_type, linkage, linkage_evidence)
///

DEFINE_MACRO_FUNCNAME(CMacroFunction_ConvertRawToDeltabyNs, "AddAssemblyGapsbyNs")
void CMacroFunction_ConvertRawToDeltabyNs::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!entry || !scope)
        return;

    size_t index = 0;
    long min_unknown = (long)m_Args[index]->GetInt();
    int max_unknown = (int)m_Args[++index]->GetInt();
    long min_known = (long)m_Args[++index]->GetInt();
    int max_known = (int)m_Args[++index]->GetInt();

    bool adjust_cds = m_Args[++index]->GetBool();
    bool keep_gap_length = m_Args[++index]->GetBool();
    int gap_type = objects::CSeq_gap::eType_unknown;
    int linkage = -1;
    int linkage_evidence = -1;
    gap_type = CSeq_gap::ENUM_METHOD_NAME(EType)()->FindValue(m_Args[++index]->GetString());
    if (!m_Args[++index]->GetString().empty()) {
        linkage = CSeq_gap::ENUM_METHOD_NAME(ELinkage)()->FindValue(m_Args[index]->GetString());
    }
    if (!m_Args[++index]->GetString().empty()) {
        linkage_evidence = CLinkage_evidence::ENUM_METHOD_NAME(EType)()->FindValue(m_Args[index]->GetString());
    }

    bool remove_alignments = false;
    bool is_assembly_gap = true;
    int count = 0;
    CRef<CCmdComposite> convert_cmd = NRawToDeltaSeq::ConvertRawToDeltaByNsCommand(m_DataIter->GetSEH(), min_unknown, max_unknown, min_known, max_known,
                                                                                   adjust_cds, keep_gap_length, remove_alignments, count, is_assembly_gap, gap_type, linkage, linkage_evidence);

    CNcbiOstrstream log;
    if (convert_cmd) {
        m_DataIter->RunCommand(convert_cmd, m_CmdComposite);
        log << "Added assembly gaps by Ns to " << count << " sequences";
        if (remove_alignments) {
            log << "and affected alignments were removed";
        }
    }
    x_LogFunction(log);
}

bool CMacroFunction_ConvertRawToDeltabyNs::x_ValidArguments() const
{
    if (m_Args.size() != 9)
        return false;

    size_t index = 0;
    for (; index < 4; ++index) {
        if (m_Args[index]->GetDataType() != CMQueryNodeValue::eInt)
            return false;
    }
    if (m_Args[index]->GetDataType() != CMQueryNodeValue::eBool)
        return false;
    index++;
    if (m_Args[index]->GetDataType() != CMQueryNodeValue::eBool)
        return false;
    index++;

    for (;index < m_Args.size(); ++index) {
        if (m_Args[index]->GetDataType() != CMQueryNodeValue::eString)
            return false;
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_DeltaSeqToRaw
/// ConvertDeltaSeqToRaw()
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_DeltaSeqToRaw, "ConvertDeltaSeqToRaw")
void CMacroFunction_DeltaSeqToRaw::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!entry || !scope)
        return;

    CRef<CCmdComposite> delta_to_raw = s_ConvertDeltaToRaw(m_DataIter->GetSEH(), m_QualsChangedCount);

    if (m_QualsChangedCount) {
        m_DataIter->RunCommand(delta_to_raw, m_CmdComposite);
        CNcbiOstrstream log;
        log << "Converted " << m_QualsChangedCount << " delta sequences to raw sequences";
        x_LogFunction(log);
    }
}

bool CMacroFunction_DeltaSeqToRaw::x_ValidArguments() const
{
    return (m_Args.empty());
}

CRef<CCmdComposite> CMacroFunction_DeltaSeqToRaw::s_ConvertDeltaToRaw(CSeq_entry_Handle seh, Int4& count)
{
    count = 0;
    CRef<CCmdComposite> composite(new CCmdComposite("Delta Seq To Raw"));

    for (CBioseq_CI b_iter(seh, CSeq_inst::eMol_na); b_iter; ++b_iter) {
        CRef<CSeq_inst> new_inst(new CSeq_inst());
        new_inst->Assign(b_iter->GetInst());
        if (new_inst->ConvertDeltaToRaw()) {
            CRef<CCmdChangeBioseqInst> cmd(new CCmdChangeBioseqInst(*b_iter, *new_inst));
            composite->AddCommand(*cmd);
            count++;
        }
    }
    if (count == 0)
        composite.Reset();
    return composite;
}

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
