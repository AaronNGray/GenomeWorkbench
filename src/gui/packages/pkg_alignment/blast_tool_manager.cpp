/*  $Id: blast_tool_manager.cpp 40050 2017-12-13 18:32:01Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_alignment/blast_tool_manager.hpp>

#include <gui/packages/pkg_alignment/blast_search_options_panel.hpp>
#include <gui/packages/pkg_alignment/blast_search_params_panel.hpp>
#include <gui/packages/pkg_alignment/blast_search_task.hpp>
#include <gui/packages/pkg_alignment/blast_seq_tool_job.hpp>

#include <gui/core/ui_data_source_service.hpp>

#include <gui/widgets/wx/message_box.hpp>

#include <gui/objutils/registry.hpp>

#include <objects/seqloc/Seq_loc.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <serial/iterator.hpp>

#include <gui/core/async_obj_convert.hpp>

#include <wx/panel.h>
#include <wx/arrstr.h>
#include <wx/msgdlg.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CBLASTToolManager
CBLASTToolManager::CBLASTToolManager()
    : m_Descriptor("BLAST", "",
                   "Align sequences using BLAST",
                   "Perform sequence similarity comparison "
                   "with sets of sequences, NCBI BLAST database or local BLAST database"),
    m_SrvLocator(NULL),
    m_ParentWindow(NULL),
    m_UseNucForMixInput(true),
    m_State(eInvalid),
    m_OptionsPanel(NULL),
    m_ParamsPanel(NULL)
{
    m_Descriptor.SetLogEvent("tools");
}

string CBLASTToolManager::GetExtensionIdentifier() const
{
    return "blast_tool_manager";
}


string CBLASTToolManager::GetExtensionLabel() const
{
    return "BLAST Tool";
}

void CBLASTToolManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CBLASTToolManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CBLASTToolManager::GetDescriptor() const
{
    return m_Descriptor;
}


void CBLASTToolManager::InitUI()
{
    // initially tool panels are not displayed and so the first
    // step of the Run Tool dialog corresponds to invalid state
    m_State = eInvalid;
    m_InputObjects.clear();

    m_OptionsPanel = NULL;
    m_ParamsPanel = NULL;
}


void CBLASTToolManager::CleanUI()
{
    m_State = eInvalid;
    m_InputObjects.clear();

    //TODO Current panel lifetime management
    m_OptionsPanel = NULL;
    m_ParamsPanel = NULL;
}


wxPanel* CBLASTToolManager::GetMaxPanel()
{
    CBLASTSearchOptionsPanel* optionsPanel = new CBLASTSearchOptionsPanel();
    optionsPanel->Create(m_ParentWindow);
    return optionsPanel;
}

wxPanel* CBLASTToolManager::GetCurrentPanel()
{
    switch(m_State) {
    case eOptions:
        return m_OptionsPanel;
    case eParams:
        return m_ParamsPanel;
    default:
        return NULL;
    }
}


void CBLASTToolManager::x_CreateOptionsPanelIfNeeded()
{
    if(m_OptionsPanel == NULL)   {
        // preapare input data
        x_SelectCompatibleInputObjects();

        m_Params.IsNucInput() = x_IsNucInput();
        CRef<CNetBLASTUIDataSource> blast_ds = GetBLASTDataSource();

        // Create Options panel and initialize it
        m_OptionsPanel = new CBLASTSearchOptionsPanel();
        m_OptionsPanel->Setup(*blast_ds, m_Descriptor.GetLabel());
        m_OptionsPanel->SetParams(&m_Params, &m_NucObjects, &m_ProtObjects);

        m_OptionsPanel->Create(m_ParentWindow);

        m_OptionsPanel->SetRegistryPath(m_RegPath + ".OptionsPanel");
        m_OptionsPanel->LoadSettings();
    }
}

CRef<CNetBLASTUIDataSource> CBLASTToolManager::GetBLASTDataSource()
{
    CIRef<CUIDataSourceService> ds_srv =
            m_SrvLocator->GetServiceByType<CUIDataSourceService>();

    CIRef<IUIDataSource> ds = ds_srv->GetUIDataSourceByLabel("NCBI Net BLAST");
    CNetBLASTUIDataSource* blast_ds = dynamic_cast<CNetBLASTUIDataSource*>(ds.GetPointer());
    return CRef<CNetBLASTUIDataSource>(blast_ds);
}


void CBLASTToolManager::x_CreateParamsPanelIfNeeded()
{
    if( m_ParamsPanel == NULL ){
        m_ParamsPanel = new CBLASTSearchParamsPanel( m_ParentWindow );
        m_ParamsPanel->SetParams( &m_Params);
    }
}


bool CBLASTToolManager::CanDo(EAction action)
{
    switch(m_State) {
    case eInvalid:
        return action == eNext;
    case eOptions:
        return action == eNext;
    case eParams:
        return action == eNext ||  action == eBack;
    case eCompleted:
        return false; // nothing left to do
    default:
        _ASSERT(false);
        return false;
    }
}


bool CBLASTToolManager::IsFinalState()
{
    return m_State == eParams;
}


bool CBLASTToolManager::IsCompletedState()
{
    return m_State == eCompleted;
}


bool CBLASTToolManager::DoTransition(EAction action)
{
    bool next = action == eNext;

    if(m_State == eInvalid  &&  next)    {
        m_State = eOptions;
        x_CreateOptionsPanelIfNeeded();
        m_OptionsPanel->TransferDataToWindow();
        return true;
    } else if(m_State == eOptions) {
        if(next)    {
            if (m_OptionsPanel->TransferDataFromWindow()) {
                m_OptionsPanel->SaveSettings();
                m_State = eParams;
                x_CreateParamsPanelIfNeeded();
                m_ParamsPanel->TransferDataToWindow();
                return true;
            } else return false;
        } else {
            m_OptionsPanel->SaveSettings();
            m_State = eInvalid;
            return true;
        }
    } else if(m_State == eParams) {
        if(next)    {
            if(m_ParamsPanel->TransferDataFromWindow()) {
                try{
                    LOG_POST(Trace << "Validationg BLAST Search params...");
                    x_ValidateParams();
                    x_InitProjectParams();
                    m_State = eCompleted;
                    return true;
                } catch (CException& e) {
                    ERR_POST(e.ReportAll());
                    NcbiErrorBox(e.GetMsg());
                }
            }
        } else {
            m_State = eOptions;
            m_OptionsPanel->TransferDataToWindow();
            return true;
        }
        return false;
    }
    _ASSERT(false);
    return false;
}

IAppTask* CBLASTToolManager::GetTask()
{
    if (CBLASTParams::eNCBIDB == m_Params.GetSubjectType()) {
        CRef<CNetBLASTUIDataSource> blast_ds = GetBLASTDataSource();

        CRef<CBlastSearchTask> task(
            new CBlastSearchTask(m_SrvLocator, *blast_ds, m_Descriptor.GetLabel())
            );
        task->Init_SubmitSearch(m_Params);

        // Log the parameters
        m_Params.PrintParams("CBLASTSearchToolManager::");

        CSelectProjectOptions options;
        m_ProjectParams.ToLoadingOptions(options);
        task->SetLoadingOptions(options);

        return task.Release();
    }
    else {
        CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
        CRef<CBLASTSeqToolJob> job(new CBLASTSeqToolJob(m_Params));

        // Log the parameters
        m_Params.PrintParams("CBLASTSeqToolManager::");

        CSelectProjectOptions options;
        m_ProjectParams.ToLoadingOptions(options);

        CRef<CDataLoadingAppTask> task(new CDataLoadingAppTask(srv, options, *job));
        return task.Release();
    }
}


void CBLASTToolManager::x_InitProjectParams()
{
    CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
    NON_CONST_ITERATE(vector<TConstScopedObjects>, it, m_InputObjects) {
        if (!it->empty()) {
            m_ProjectParams.SelectProjectByObjects(*it, srv);
            m_ProjectParams.m_FolderName = "BLAST Results";
            m_ProjectParams.m_CreateFolder = true;
            break;
        }
    }
}


/// validate parameters after Params page
bool CBLASTToolManager::x_ValidateParams()
{
    CRef<blast::CBlastOptionsHandle> handle = m_Params.ToBlastOptions();
    return handle->SetOptions().Validate();
}


string CBLASTToolManager::GetCategory()
{
    return "Alignment Creation";
}


string CBLASTToolManager::SetInputObjects(const vector<TConstScopedObjects>& objects)
{
    CRef<objects::CScope> scope;
    ITERATE(vector<TConstScopedObjects>, it, objects) {
        ITERATE(TConstScopedObjects, it2, *it) {
            if (!scope) {
                scope = it2->scope;
                continue;
            }
            else {
                if (scope != it2->scope) {
                    return "This tool gives unpredictable results with objects\nfrom different projects.\n\nPlease move all objects to the same project\nand try again.";
                }
            }
        }
    }

    m_InputObjects = objects;

    return "";
}

bool CBLASTToolManager::CanQuickLaunch() const
{
    return CBLASTParams::eNCBIDB == m_Params.GetSubjectType();
}

static const char* kSelectedSequences = "Selected sequences";

IAppTask* CBLASTToolManager::QuickLaunch()
{
    if (CBLASTParams::eNCBIDB != m_Params.GetSubjectType()) return 0;

    x_SelectCompatibleInputObjects();

    m_Params.IsNucInput() = x_IsNucInput();

    CRef<CNetBLASTUIDataSource> blast_ds = GetBLASTDataSource();

    if (m_Params.IsNucInput()) {
        map<string, TConstScopedObjects>::const_iterator it = m_NucObjects.find(kSelectedSequences);
        if (it != m_NucObjects.end()) {
            copy(it->second.begin(), it->second.end(), back_inserter(m_Params.GetSeqLocs()));
        }
        else {
            for (const auto& i : m_NucObjects)
                copy(i.second.begin(), i.second.end(), back_inserter(m_Params.GetSeqLocs()));
        }
    }
    else {
        map<string, TConstScopedObjects>::const_iterator it = m_ProtObjects.find(kSelectedSequences);
        if (it != m_ProtObjects.end()) {
            copy(it->second.begin(), it->second.end(), back_inserter(m_Params.GetSeqLocs()));
        }
        else {
            for (const auto& i : m_ProtObjects)
                copy(i.second.begin(), i.second.end(), back_inserter(m_Params.GetSeqLocs()));
        }

        if (m_UseNucForMixInput){
            for (const auto& i : m_NucObjects)
                copy(i.second.begin(), i.second.end(), back_inserter(m_Params.GetSeqLocs()));
        }
    }

    SProjectSelectorParams project_params;
    project_params.m_CreateFolder = false;
    project_params.m_FolderName = "";

    CProjectService* prj_srv = m_SrvLocator->GetServiceByType<CProjectService>();

    vector<int> projects;
    prj_srv->GetObjProjects(m_Params.GetSeqLocs(), projects);
    if (projects.empty()){
        project_params.m_ProjectMode = SProjectSelectorParams::eCreateOneProject;
    }
    else {
        project_params.m_ProjectMode = SProjectSelectorParams::eAddToExistingProject;
        project_params.m_SelectedProjectId = projects.front();
    }

    return GetTask();
}

void CBLASTToolManager::x_SelectCompatibleInputObjects()
{
    wxBusyCursor wait;

    m_NucObjects.clear();
    m_ProtObjects.clear();

    TConstScopedObjects convert, original;
    set<CBioseq_Handle> seqSet;
    for (auto v : m_InputObjects) {
        for (auto o : v) {
            const CSeq_loc* seqLoc = dynamic_cast<const CSeq_loc*>(o.object.GetPointer());
            const CSeq_id* seqId = dynamic_cast<const CSeq_id*>(o.object.GetPointer());
            const CSeq_entry* seqEntry = dynamic_cast<const CSeq_entry*>(o.object.GetPointer());
            const CBioseq* bioSeq = dynamic_cast<const CBioseq*>(o.object.GetPointer());
            const CBioseq_set* bioSeqSet = dynamic_cast<const CBioseq_set*>(o.object.GetPointer());

            if (seqLoc) {
                original.push_back(o);
            }
            else if (seqId) {
                CBioseq_Handle h = o.scope->GetBioseqHandle(*seqId);
                if (h && seqSet.insert(h).second) {
                    CRef<CSeq_id> id(new CSeq_id());
                    id->Assign(*h.GetSeqId());
                    CRef<CSeq_loc> seq_whole(new CSeq_loc());
                    seq_whole->SetWhole(*id);
                    original.push_back(SConstScopedObject(seq_whole, o.scope));
                }
            }
            else if (seqEntry) {
                CSeq_entry_Handle seh = o.scope->GetSeq_entryHandle(*seqEntry);
                if (!seh) continue;
                for (CSeq_entry_CI ci(seh, CSeq_entry_CI::fRecursive|CSeq_entry_CI::fIncludeGivenEntry); ci; ++ci) {
                    if (ci->IsSeq()) {
                        CBioseq_Handle h = ci->GetSeq();
                        if (h && seqSet.insert(h).second) {
                            CRef<CSeq_id> id(new CSeq_id());
                            id->Assign(*h.GetSeqId());
                            CRef<CSeq_loc> seq_whole(new CSeq_loc());
                            seq_whole->SetWhole(*id);
                            original.push_back(SConstScopedObject(seq_whole, o.scope));
                        }
                    }
                }
            }
            else if (bioSeq) {
                CBioseq_Handle h = o.scope->GetBioseqHandle(*bioSeq);
                if (h && seqSet.insert(h).second) {
                    CRef<CSeq_id> id(new CSeq_id());
                    id->Assign(*h.GetSeqId());
                    CRef<CSeq_loc> seq_whole(new CSeq_loc());
                    seq_whole->SetWhole(*id);
                    original.push_back(SConstScopedObject(seq_whole, o.scope));
                }
            }
            else if (bioSeqSet) {
                CTypeConstIterator<CBioseq> iter(*bioSeqSet);
                for (; iter; ++iter) {
                    CBioseq_Handle h = o.scope->GetBioseqHandle(*iter);
                    if (h && seqSet.insert(h).second) {
                        CRef<CSeq_id> id(new CSeq_id());
                        id->Assign(*h.GetSeqId());
                        CRef<CSeq_loc> seq_whole(new CSeq_loc());
                        seq_whole->SetWhole(*id);
                        original.push_back(SConstScopedObject(seq_whole, o.scope));
                    }
                }
            }
            else
                convert.push_back(o);
        }
    }
    seqSet.clear();

    map<string, TConstScopedObjects> seqLocs;

    if (!convert.empty()) {
        AsyncConvertObjects(CSeq_loc::GetTypeInfo(), convert, seqLocs);
    }

    map<string, TConstScopedObjects>::iterator it = seqLocs.find("");
    if (it != seqLocs.end()) {
        TConstScopedObjects& other = seqLocs["Other"];
        other.insert(other.end(), it->second.begin(), it->second.end());
        seqLocs.erase(it);
    }

    if (!original.empty())
        seqLocs[kSelectedSequences] = original;

    for (auto& i : seqLocs) {
        for (auto& j : i.second) {
            const CSeq_loc* seq_loc = dynamic_cast<const CSeq_loc*>(j.object.GetPointer());
            if (!seq_loc || !seq_loc->GetId())
                continue;

            CBioseq_Handle handle = j.scope->GetBioseqHandle(*seq_loc->GetId());
            if (!handle)
                continue;
            map<string, TConstScopedObjects>& target(handle.IsNa() ? m_NucObjects : m_ProtObjects);
            target[i.first].push_back(j);
        }
    }
}

/// defines whether Nucleotide or Protein options shall be selected by default
bool CBLASTToolManager::x_IsNucInput() const
{
    if(m_ProtObjects.empty())   {
        return true; // no proteins - select Nucleotides
    } else {
        // have proteins - use them if not nucleotides, otherwise
        // use m_UseNucForMixInput setting
        return m_NucObjects.empty() ? false : m_UseNucForMixInput;
    }
}


void CBLASTToolManager::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}

static const char* kParamsTag = "Params";
static const char* kUseNucForMixInput = "UseNucForMixInput";
static const char* kLocalWinMaskDirPath = "Workbench.Services.DefaultUIDS.LocalMaskPath";

void CBLASTToolManager::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        if( ! m_ProtObjects.empty()  &&  ! m_NucObjects.empty())    {
            // update option
            m_UseNucForMixInput = m_Params.IsNucInput();
        }
        view.Set(kUseNucForMixInput, m_UseNucForMixInput);

        /// save BLAST parameters
        CRef<CUser_object> param_obj(m_Params.ToUserObject());
        CRef<CUser_field> field =
            view.SetField(CGuiRegistryUtil::MakeKey(kParamsTag, kParamsTag));
        field->SetData().SetObject(*param_obj);

        if(m_OptionsPanel)  {
            m_OptionsPanel->SaveSettings();
        }
    }
}


void CBLASTToolManager::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_UseNucForMixInput = view.GetBool(kUseNucForMixInput, m_UseNucForMixInput);

        /// load BLAST parameters
        string key = CGuiRegistryUtil::MakeKey(kParamsTag, kParamsTag);
        if(view.HasField(key))   {
            const CUser_object& param_obj = view.GetField(key)->GetData().GetObject();
            m_Params.FromUserObject(param_obj);
        }

        m_LocalWinMaskDirPath = gui_reg.GetString( kLocalWinMaskDirPath );
    }
}

void CBLASTToolManager::RestoreDefaults()
{
    wxPanel* current_panel = GetCurrentPanel();
    if (!current_panel)
        return;
    CAlgoToolManagerParamsPanel* params_panel = dynamic_cast<CAlgoToolManagerParamsPanel*>(current_panel);
    if(!params_panel)
        return;
    if (wxOK != wxMessageBox("The active page settings will be restored to their original defaults.", "Confirm", wxOK | wxCANCEL))
        return;
    params_panel->RestoreDefaults();
}

END_NCBI_SCOPE
