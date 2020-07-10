/*  $Id: autodef_tool_manager.cpp 42004 2018-12-03 15:18:59Z filippov $
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
 * Authors:  Colleen Bollin, adapted from a file by Andrey Yazhuk
 *
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_sequence_edit/autodef_tool_manager.hpp>

#include <gui/packages/pkg_sequence_edit/autodef_params.hpp>

#include <gui/framework/app_job_task.hpp>

#include <gui/widgets/wx/message_box.hpp>

#include <gui/objutils/obj_convert.hpp>
#include <gui/objutils/label.hpp>
#include <gui/core/undo_manager.hpp>

#include <objects/gbproj/ProjectItem.hpp>

#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objtools/edit/autodef_with_tax.hpp>

#include <wx/panel.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);



///////////////////////////////////////////////////////////////////////////////
/// CAutodefToolManager
CAutodefToolManager::CAutodefToolManager()
:   CAlgoToolManagerBase("Autodef",
                         "",
                         "Automatically generate sequence titles using organism information and feature lists",
                         "Automatically generate sequence titles using organism information and feature lists",
                         "AUTODEF",
                         "Defline Creation"),
    m_ParamsPanel(NULL)
{
    m_Descriptor.SetLogEvent("tools");
}


string CAutodefToolManager::GetExtensionIdentifier() const
{
    return "autodef_tool_manager";
}


string CAutodefToolManager::GetExtensionLabel() const
{
    return "Autodef Tool";
}


void CAutodefToolManager::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_ParamsPanel = NULL;
}


void CAutodefToolManager::CleanUI()
{
    m_ParamsPanel = NULL;

    CAlgoToolManagerBase::CleanUI();
}


void CAutodefToolManager::x_CreateParamsPanelIfNeeded()
{
    if(m_ParamsPanel == NULL)   {
        x_SelectCompatibleInputObjects();

        m_ParamsPanel = new CAutodefParamsPanel();
        m_ParamsPanel->Hide(); // to reduce flicker
        m_ParamsPanel->SetParams(&m_Params, &m_SeqEntries);
        m_ParamsPanel->Create(m_ParentWindow);

        m_ParamsPanel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_ParamsPanel->LoadSettings();
    }
}


bool CAutodefToolManager::x_ValidateParams()
{
    string err;
    if(m_Params.m_SeqEntries.size() < 1)  {
        err = "Please select at least one Seq-entry!";
    }
    if( ! err.empty()) {
        NcbiErrorBox(err, m_Descriptor.GetLabel());
        return false;
    }
    return true;
}


/// select only Seq-entries
void CAutodefToolManager::x_SelectCompatibleInputObjects()
{
    m_Params.m_SeqEntries.clear();
    x_ConvertInputObjects(CSeq_entry::GetTypeInfo(), m_SeqEntries);

    CAutoDef autodef;

    NON_CONST_ITERATE(TConstScopedObjects, it, m_SeqEntries) {
        const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(it->object.GetPointer());
        if (!entry)
            continue;
        CSeq_entry_Handle seh = it->scope->GetSeq_entryHandle (*entry);
        autodef.AddSources (seh);
    }

    CRef<CAutoDefModifierCombo> src_combo = autodef.FindBestModifierCombo();
    src_combo->GetAvailableModifiers (m_Params.m_ModifierList);
    /* set requested */
    for (size_t n = 0; n < m_Params.m_ModifierList.size(); n++) {
        if (m_Params.m_ModifierList[n].AnyPresent()) {
            if (m_Params.m_ModifierList[n].IsOrgMod()) {
                if (src_combo->HasOrgMod (m_Params.m_ModifierList[n].GetOrgModType())) {
                    m_Params.m_ModifierList[n].SetRequested(true);
                }
            } else if (src_combo->HasSubSource (m_Params.m_ModifierList[n].GetSubSourceType())) {
                m_Params.m_ModifierList[n].SetRequested(true);
            }
        }
    }
    m_Params.m_NeedHIVRule = src_combo->HasTrickyHIV();
}


CAlgoToolManagerParamsPanel* CAutodefToolManager::x_GetParamsPanel()
{
    return m_ParamsPanel;
}


IRegSettings* CAutodefToolManager::x_GetParamsAsRegSetting()
{
    return &m_Params;
}


CDataLoadingAppJob* CAutodefToolManager::x_CreateLoadingJob()
{
    return NULL;
}


CEditAppJob* CAutodefToolManager::x_CreateEditJob()
{
    CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
    CAutodefJob* job = new CAutodefJob(srv.GetPointer(), m_Params);
    return job;
}


void    CAutodefToolManager::x_CreateProjectPanelIfNeeded()
{
    // do nothing
}


IAppTask* CAutodefToolManager::GetTask()
{
    CRef<CEditAppJob> job (x_CreateEditJob());

    CRef<CEditAppTask> task(new CEditAppTask(*job));
    return task.Release();

}


///////////////////////////////////////////////////////////////////////////////
/// CAutodefJob
CAutodefJob::CAutodefJob(CProjectService* service,
                                         const SAutodefParams& params)
:   CEditAppJob(service),
    m_Params(params)
{
    CFastMutexGuard lock(m_Mutex);

    m_Descr = "Creating sequence titles"; //TODO
}


void CAutodefJob::x_MakeChanges()
{
    TConstScopedObjects& entries = m_Params.m_SeqEntries;

    CObjectConverter::TObjList seqentries;

    CRef<CScope> scope;

    {{
        ITERATE (TConstScopedObjects, iter, entries) {
            
            const CObject& obj = *iter->object;
            scope.Reset(const_cast<CScope*>(&*iter->scope));

            seqentries.clear();
            CObjectConverter::Convert(*scope, obj, CSeq_entry::GetTypeInfo(),
                                      seqentries, ITypeConverter::fConvert_NoExpensive);

            NON_CONST_ITERATE(ITypeConverter::TObjList, it, seqentries) {
                ITypeConverter::SObject& s_obj = *it;
                const CObject* conv_obj = s_obj.GetObjectPtr();
    
                _ASSERT(typeid(*s_obj) == typeid(CSeq_entry));
    
                const CSeq_entry* entry = reinterpret_cast<const CSeq_entry*>(conv_obj);

                CSeq_entry_Handle seh = scope->GetSeq_entryHandle (*entry);

                CAutoDef autodef;
                autodef.AddSources (seh);
                CAutoDefModifierCombo *mod_combo = autodef.GetEmptyCombo ();

                //TODO - set values from m_Params
                for (size_t n = 0; n < m_Params.m_ModifierList.size(); n++) {
                    if (m_Params.m_ModifierList[n].IsRequested()) {
                        if (m_Params.m_ModifierList[n].IsOrgMod()) {
                            mod_combo->AddOrgMod (m_Params.m_ModifierList[n].GetOrgModType());
                        } else {
                            mod_combo->AddSubsource (m_Params.m_ModifierList[n].GetSubSourceType());
                        }
                    }
                }
                mod_combo->SetExcludeSpOrgs (m_Params.m_DoNotApplyToSp);
                mod_combo->SetExcludeNrOrgs (m_Params.m_DoNotApplyToNr);
                mod_combo->SetExcludeCfOrgs (m_Params.m_DoNotApplyToCf);
                mod_combo->SetExcludeAffOrgs (m_Params.m_DoNotApplyToAff);
                mod_combo->SetMaxModifiers (m_Params.m_MaxMods);
                mod_combo->SetUseModifierLabels (m_Params.m_UseLabels);
                mod_combo->SetKeepParen (m_Params.m_LeaveParenthetical);
                mod_combo->SetKeepCountryText (m_Params.m_IncludeCountryText);
                mod_combo->SetKeepAfterSemicolon (m_Params.m_KeepAfterSemicolon);

                if (m_Params.m_NeedHIVRule) {
                    mod_combo->SetHIVCloneIsolateRule((CAutoDefOptions::EHIVCloneIsolateRule)m_Params.m_HIVRule);
                }

                autodef.SetFeatureListType((CAutoDefOptions::EFeatureListType)m_Params.m_FeatureListType);
                autodef.SetMiscFeatRule ((CAutoDefOptions::EMiscFeatRule)m_Params.m_MiscFeatRule);
                if (m_Params.m_ProductFlag != CBioSource::eGenome_unknown) {
                    autodef.SetProductFlag((CBioSource::EGenome)m_Params.m_ProductFlag);
                } else if (m_Params.m_NuclearCopyFlag != CBioSource::eGenome_unknown) {
                    autodef.SetNuclearCopyFlag((CBioSource::EGenome)m_Params.m_NuclearCopyFlag);
                }
                autodef.SetSpecifyNuclearProduct (m_Params.m_SpecifyNuclearProduct);

                autodef.SetAltSpliceFlag (m_Params.m_AltSpliceFlag);
                autodef.SetUseNcRNAComment (m_Params.m_UseNcRNAComment);
                autodef.SetSuppressLocusTags (m_Params.m_SuppressLocusTags);
                autodef.SetGeneClusterOppStrand (m_Params.m_GeneClusterOppStrand);
                autodef.SetSuppressFeatureAltSplice (m_Params.m_SuppressFeatureAltSplice);
                autodef.SuppressMobileElementAndInsertionSequenceSubfeatures
                   (m_Params.m_SuppressMobileElementSubfeatures);
                autodef.SetKeepExons (m_Params.m_KeepExons);
                autodef.SetKeepIntrons (m_Params.m_KeepIntrons);
                autodef.SetKeepRegulatoryFeatures(m_Params.m_KeepRegulatoryFeatures);
                autodef.SetUseFakePromoters(m_Params.m_UseFakePromoters);
                autodef.SetKeep3UTRs (m_Params.m_Keep3UTRs);
                autodef.SetKeep5UTRs (m_Params.m_Keep5UTRs);


                // suppress features
                ITERATE(set<objects::CFeatListItem>, it, m_Params.m_SuppressedFeatures)  {
                    autodef.SuppressFeature (*it);
                }

                // now create titles and AutodefOptions user objects

                CBioseq_CI b_iter(seh);
                for ( ; b_iter; ++b_iter ) {
                    string defline = autodef.GetOneDefLine(mod_combo, *b_iter);
                    CBioseq_EditHandle bsh = b_iter->GetEditHandle ();
                    CRef<CSeqdesc> title;
                    CRef<CSeqdesc> new_defline;
                    new_defline.Reset(new CSeqdesc);
                    new_defline->SetTitle (defline);
                    for (CSeqdesc_CI it(bsh, CSeqdesc::e_Title);  it;  ++it) {
                        title.Reset(const_cast<CSeqdesc*>(&*it));
                        break;
                    }

                    if (title) {
                        title->Assign(*new_defline);
                    } else {
                        bsh.AddSeqdesc(*new_defline);
                    }

                    if (b_iter->IsAa()) {
                        continue;
                    }

                    CRef<CUser_object> autodef_opts = autodef.GetOptionsObject();
                    if (autodef_opts) {
                        for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_User); desc_it; ++desc_it) {
                            if (desc_it->GetUser().GetObjectType() == CUser_object::eObjectType_AutodefOptions) {
                                bsh.RemoveSeqdesc(*desc_it);
                                break;
                            }
                        }
                        CRef<CSeqdesc> opts_desc(new CSeqdesc);
                        opts_desc->SetUser(*autodef_opts);
                        bsh.AddSeqdesc(*opts_desc);
                    }
                }
                delete mod_combo;
                /* now inform project that data has changed */
            }

        }
    }}
}


END_NCBI_SCOPE
