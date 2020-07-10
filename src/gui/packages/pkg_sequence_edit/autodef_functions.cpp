/*  $Id: autodef_functions.cpp 42444 2019-02-27 19:00:08Z asztalos $
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
 * Authors: Colleen Bollin, Andrea Asztalos
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/util/create_defline.hpp>

#include <gui/objutils/macro_fn_entry.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/packages/pkg_sequence_edit/autodef_functions.hpp>
#include <objtools/edit/autodef_with_tax.hpp>

#include <objects/general/User_object.hpp>
#include <objmgr/seq_entry_ci.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void ConfigureAutodefParamsForBestCombo(SAutodefParams& params, CSeq_entry_Handle seh)
{
    CAutoDef autodef;
    autodef.AddSources(seh);
    CRef<CAutoDefModifierCombo> src_combo = autodef.FindBestModifierCombo();
    src_combo->GetAvailableModifiers(params.m_ModifierList);
    /* set requested */
    for (size_t n = 0; n < params.m_ModifierList.size(); n++) {
        if (params.m_ModifierList[n].AnyPresent()) {
            if (params.m_ModifierList[n].IsOrgMod()) {
                if (src_combo->HasOrgMod(params.m_ModifierList[n].GetOrgModType())) {
                    params.m_ModifierList[n].SetRequested(true);
                }
            }
            else if (src_combo->HasSubSource(params.m_ModifierList[n].GetSubSourceType())) {
                params.m_ModifierList[n].SetRequested(true);
            }
        }
    }
    params.m_NeedHIVRule = src_combo->HasTrickyHIV();
    params.m_UseLabels = true;
}

void ConfigureAutodefParamsForID(SAutodefParams& params, CSeq_entry_Handle seh)
{
    macro::CMacroFunction_AutodefId::s_ConfigureAutodefOptionsForID(params.m_ModifierList, seh);
    params.m_UseLabels = true;
}

unsigned int ConfigureParamsForPrefixCombo(SAutodefParams& params, CSeq_entry_Handle seh)
{
    unsigned int found_modifier = 0;
    CAutoDef autodef;
    autodef.AddSources(seh);
    CRef<CAutoDefModifierCombo> src_combo = autodef.FindBestModifierCombo();
    src_combo->GetAvailableModifiers(params.m_ModifierList);

    for (size_t n = 0; n < params.m_ModifierList.size(); n++)
        if (params.m_ModifierList[n].AnyPresent() && !NStr::IsBlank(params.m_ModifierList[n].Label())){
            params.m_ModifierList[n].SetRequested(true);
            found_modifier++;
        }
    return found_modifier;
}

CAutoDefModifierCombo * ConfigureAutodefForSeqEntry(CAutoDef& autodef, CSeq_entry_Handle entry, SAutodefParams* params)
{
    autodef.AddSources(entry);
    CAutoDefModifierCombo *mod_combo = autodef.GetEmptyCombo();

    if (params) {
        for (size_t n = 0; n < params->m_ModifierList.size(); n++) {
            if (params->m_ModifierList[n].IsRequested()) {
                if (params->m_ModifierList[n].IsOrgMod()) {
                    mod_combo->AddOrgMod(params->m_ModifierList[n].GetOrgModType(), true);
                }
                else {
                    mod_combo->AddSubsource(params->m_ModifierList[n].GetSubSourceType(), true);
                }
            }
        }
        mod_combo->SetExcludeSpOrgs(params->m_DoNotApplyToSp);
        mod_combo->SetExcludeNrOrgs(params->m_DoNotApplyToNr);
        mod_combo->SetExcludeCfOrgs(params->m_DoNotApplyToCf);
        mod_combo->SetExcludeAffOrgs(params->m_DoNotApplyToAff);
        mod_combo->SetMaxModifiers(params->m_MaxMods);
        mod_combo->SetUseModifierLabels(params->m_UseLabels);
        mod_combo->SetKeepParen(params->m_LeaveParenthetical);
        mod_combo->SetKeepCountryText(params->m_IncludeCountryText);
        mod_combo->SetKeepAfterSemicolon(params->m_KeepAfterSemicolon);

        if (params->m_NeedHIVRule) {
            mod_combo->SetHIVCloneIsolateRule((CAutoDefOptions::EHIVCloneIsolateRule)params->m_HIVRule);
        }

        autodef.SetFeatureListType((CAutoDefOptions::EFeatureListType)params->m_FeatureListType);
        autodef.SetMiscFeatRule((CAutoDefOptions::EMiscFeatRule)params->m_MiscFeatRule);
        if (params->m_ProductFlag != CBioSource::eGenome_unknown) {
            autodef.SetProductFlag((CBioSource::EGenome)params->m_ProductFlag);
        }
        else if (params->m_NuclearCopyFlag != CBioSource::eGenome_unknown) {
            autodef.SetNuclearCopyFlag((CBioSource::EGenome)params->m_NuclearCopyFlag);
        }

        autodef.SetSpecifyNuclearProduct(params->m_SpecifyNuclearProduct);
        autodef.SetAltSpliceFlag(params->m_AltSpliceFlag);
        autodef.SetUseNcRNAComment(params->m_UseNcRNAComment);
        autodef.SetSuppressLocusTags(params->m_SuppressLocusTags);
        autodef.SetGeneClusterOppStrand(params->m_GeneClusterOppStrand);
        autodef.SetSuppressFeatureAltSplice(params->m_SuppressFeatureAltSplice);
        autodef.SuppressMobileElementAndInsertionSequenceSubfeatures
            (params->m_SuppressMobileElementSubfeatures);
        autodef.SuppressMiscFeatSubfeatures(params->m_SuppressMiscFeatSubfeatures);
        autodef.SetKeepExons(params->m_KeepExons);
        autodef.SetKeepIntrons(params->m_KeepIntrons);
        autodef.SetKeepRegulatoryFeatures(params->m_KeepRegulatoryFeatures);
        autodef.SetUseFakePromoters(params->m_UseFakePromoters);
        autodef.SetKeepLTRs(params->m_KeepLTRs);
        autodef.SetKeep3UTRs(params->m_Keep3UTRs);
        autodef.SetKeep5UTRs(params->m_Keep5UTRs);
        autodef.SetKeepMiscRecomb(params->m_KeepMiscRecomb);
        autodef.SetKeepRepeatRegion(params->m_KeepRepeatRegion);

        // suppress features
        ITERATE(set<objects::CFeatListItem>, it, params->m_SuppressedFeatures)  {
            autodef.SuppressFeature(*it);
        }

        // custom phrase
        autodef.SetCustomFeatureClause(params->m_CustomFeatureClause);
    }
    return mod_combo;
}

CRef<CCmdComposite> AutodefSeqEntry(CSeq_entry_Handle entry, SAutodefParams* params, bool docsum_only)
{
    CAutoDefWithTaxonomy autodef;
    CAutoDefModifierCombo *mod_combo = ConfigureAutodefForSeqEntry(autodef, entry, params);

    CRef<CCmdComposite> cmd(new CCmdComposite("Autodef"));
    if (!docsum_only) {
        macro::CMacroFunction_Autodef::s_AutodefBioseqs(entry, autodef, mod_combo, cmd);
    }
    delete mod_combo;

    macro::CMacroFunction_Autodef::s_AutodefSets(entry, autodef, cmd);
    return cmd;
}

CRef<CCmdComposite> AutodefSeqEntry(CSeq_entry_Handle entry, CBioseq_Handle target, SAutodefParams* params)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Autodef"));

    CAutoDefWithTaxonomy autodef;
    CAutoDefModifierCombo *mod_combo = ConfigureAutodefForSeqEntry(autodef, entry, params);

    // now create titles

    string defline = autodef.GetOneDefLine(mod_combo, target);

    CRef<CSeqdesc> new_desc(new CSeqdesc());
    new_desc->SetTitle(defline);
    CSeqdesc_CI desc(target, CSeqdesc::e_Title);
    if (desc) {
        CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc.GetSeq_entry_Handle(), *desc, *new_desc));
        cmd->AddCommand(*ecmd);
    }
    else {
        cmd->AddCommand(*CRef<CCmdCreateDesc>(new CCmdCreateDesc(target.GetSeq_entry_Handle(), *new_desc)));
    }

    macro::CMacroFunction_Autodef::s_AddAutodefOptions(autodef, target, cmd);

    delete mod_combo;

    return cmd;
}

CRef<CCmdComposite> InstantiateProteinTitles(CSeq_entry_Handle entry)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("InstantiateProteinTitles"));
    bool any = false;

    sequence::CDeflineGenerator gen;
    for (CBioseq_CI b_iter(entry, CSeq_inst::eMol_aa); b_iter; ++b_iter) {
        string defline = gen.GenerateDefline(*b_iter,
            sequence::CDeflineGenerator::fIgnoreExisting |
            sequence::CDeflineGenerator::fAllProteinNames);

        CRef<CSeqdesc> new_desc(new CSeqdesc());
        new_desc->SetTitle(defline);
        CSeqdesc_CI desc(*b_iter, CSeqdesc::e_Title);
        if (desc) {
            CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc.GetSeq_entry_Handle(), *desc, *new_desc));
            cmd->AddCommand(*ecmd);
        }
        else {
            cmd->AddCommand(*CRef<CCmdCreateDesc>(new CCmdCreateDesc((*b_iter).GetSeq_entry_Handle(), *new_desc)));
        }
        any = true;
    }

    if (!any) {
        cmd.Reset(NULL);
    }
    return cmd;
}

CRef<CCmdComposite> RefreshDeflineCommand(CSeq_entry_Handle& se)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Regenerate Deflines"));
    for (CBioseq_CI b_iter(se, objects::CSeq_inst::eMol_na); b_iter; ++b_iter) 
    {
        CSeqdesc_CI desc(*b_iter, CSeqdesc::e_User);
        while (desc && desc->GetUser().GetObjectType() != CUser_object::eObjectType_AutodefOptions) 
        {
            ++desc;
        }
        if (desc) 
        {
            CAutoDef autodef;
            autodef.SetOptionsObject(desc->GetUser());
            CAutoDefModifierCombo mod_combo;
            CAutoDefOptions options;
            options.InitFromUserObject(desc->GetUser());
            mod_combo.SetOptions(options);
            string defline = autodef.GetOneDefLine(&mod_combo, *b_iter);

            bool found_existing = false;
            ITERATE(CBioseq_EditHandle::TDescr::Tdata, it, b_iter->GetDescr().Get()) 
            {
                if ((*it)->IsTitle()) 
                {
                    if (!NStr::Equal((*it)->GetTitle(), defline)) 
                    {
                        CRef<CSeqdesc> new_desc(new CSeqdesc);
                        new_desc->Assign(**it);
                        new_desc->SetTitle(defline);
                        CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(b_iter->GetSeq_entry_Handle(), **it, *new_desc));
                        cmd->AddCommand (*ecmd);
                    }
                    found_existing = true;
                    break;
                }
            }
            if (!found_existing) 
            {
                CRef<CSeqdesc> new_desc(new CSeqdesc());
                new_desc->SetTitle(defline);
                CSeq_entry_Handle seh = b_iter->GetSeq_entry_Handle();
                cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(seh, *new_desc)));
            }                   
        }
    }

    // update the title of the set 
    for (CSeq_entry_CI si(se, CSeq_entry_CI::fRecursive | CSeq_entry_CI::fIncludeGivenEntry, CSeq_entry::e_Set); si; ++si) 
    {
        if (si->IsSet() && si->GetSet().IsSetClass() && CBioseq_set::NeedsDocsumTitle(si->GetSet().GetClass()))
        {
            CAutoDefWithTaxonomy autodef;

            CConstRef<CUser_object> options(NULL);
            CBioseq_CI b(si->GetSet(), CSeq_inst::eMol_na);
            while (b && !options) 
            {
                CSeqdesc_CI desc(*b, CSeqdesc::e_User);
                while (desc && desc->GetUser().GetObjectType() != CUser_object::eObjectType_AutodefOptions) 
                {
                    ++desc;
                }
                if (desc) 
                {
                    options.Reset(&(desc->GetUser()));
                }
                ++b;
            }

            if (options) 
            {
                autodef.SetOptionsObject(*options);
            }
            autodef.AddSources(se);
            string defline = autodef.GetDocsumDefLine(*si);

            bool found_existing = false;
            CBioseq_set_Handle bssh = si->GetSet();
            ITERATE(CBioseq_set_Handle::TDescr::Tdata, it, bssh.GetDescr().Get()) 
            {
                if ((*it)->IsTitle()) 
                {
                    if (!NStr::Equal((*it)->GetTitle(), defline)) 
                    {
                        CRef<CSeqdesc> new_desc(new CSeqdesc);
                        new_desc->Assign(**it);
                        new_desc->SetTitle(defline);
                        CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(bssh.GetParentEntry(), **it, *new_desc));
                        cmd->AddCommand (*ecmd);
                    }
                    found_existing = true;
                    break;
                }
            }
            if (!found_existing) 
            {
                CRef<CSeqdesc> new_desc(new CSeqdesc());
                new_desc->SetTitle(defline);
                CSeq_entry_Handle seh = bssh.GetParentEntry();
                cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(seh, *new_desc)));
            }
        }
    }
    return cmd;
}

END_NCBI_SCOPE
