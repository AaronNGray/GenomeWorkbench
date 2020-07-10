/*  $Id: ui_gcassembly_load_manager.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  Liangshou Wu
 *
 */

#include <ncbi_pch.hpp>

#include <objmgr/object_manager.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seq/Seqdesc.hpp>

#include <gui/core/ui_gcassembly_load_manager.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/object_loading_task.hpp>
#include <gui/framework/service.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/loaders/assembly_list_panel.hpp>
#include <gui/widgets/loaders/assembly_object_loader.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objects/assembly_info.hpp>

#include <wx/msgdlg.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CGCAssemblyLoadManager
CGCAssemblyLoadManager::CGCAssemblyLoadManager()
   : m_Descriptor("Genome Assembly", "")
   , m_SrvLocator(NULL)
   , m_ParentWindow(NULL)
   , m_State(eInvalid)
   , m_AssmPanel()
   , m_PrjPanel(NULL)
{
    m_Descriptor.SetLogEvent("loaders");
    m_ProjectParams.m_EnableDecideLater = false;
}


void CGCAssemblyLoadManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CGCAssemblyLoadManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CGCAssemblyLoadManager::GetDescriptor() const
{
    return m_Descriptor;
}


void CGCAssemblyLoadManager::InitUI()
{
    m_State = eSearchAssembly;
}


void CGCAssemblyLoadManager::CleanUI()
{
    m_State = eInvalid;
    m_AssmPanel = NULL;
    m_PrjPanel = NULL;
    m_AssmAccs.clear();
}

static const string kAssmListTag = ".AssmListPanel";

wxPanel* CGCAssemblyLoadManager::GetCurrentPanel()
{
    if (m_State == eSearchAssembly) {
        if(m_AssmPanel == NULL)   {
            m_AssmPanel = new CAssemblyListPanel(m_ParentWindow);
            if ( !m_RegPath.empty() ) {
                m_AssmPanel->SetRegistryPath(m_RegPath + kAssmListTag);
                m_AssmPanel->LoadSettings();
            }
        }
        return m_AssmPanel;
    } else if (m_State == eSelectProject) {
        if(m_PrjPanel == NULL)   {
            CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
            m_PrjPanel = new CProjectSelectorPanel(m_ParentWindow);
            m_PrjPanel->SetProjectService(srv);
            m_PrjPanel->SetParams(m_ProjectParams);
            m_PrjPanel->TransferDataToWindow();
        }
        return m_PrjPanel;
    }
    return NULL;
}


bool CGCAssemblyLoadManager::CanDo(EAction action)
{
    switch(m_State) {
    case eSearchAssembly:
        if (action == eBack) {
            return false;
        }
        return true;
    case eSelectProject:
        return action == eBack  ||  action == eNext;
    case eCompleted:
        return false; // nothing left to do
    default:
        _ASSERT(false);
        return false;
    }
}


bool CGCAssemblyLoadManager::IsFinalState()
{
    return m_State == eSelectProject;
}


bool CGCAssemblyLoadManager::IsCompletedState()
{
    return m_State == eCompleted;
}


bool CGCAssemblyLoadManager::DoTransition(EAction action)
{

    if (m_State == eSearchAssembly) {
        if (action == eNext) {
            if (m_AssmPanel  &&  m_AssmPanel->IsInputValid()) {
                m_AssmPanel->SaveSettings();
                m_AssmAccs = m_AssmPanel->GetSelectedAssemblies();
            }
            else
                m_AssmAccs.clear();

            if ( !m_AssmAccs.empty() ) {
                m_State = eSelectProject;
                return true;
            }
            return false;
        } else {
            return false;
        }
    } else if (m_State == eSelectProject) {
        if (action == eBack) {
            m_State = eSearchAssembly;
            return true;
        } else if (action == eNext) {
            if(m_PrjPanel->TransferDataFromWindow()) {
                m_State = eCompleted;
                return true;
            }
            return false;
        }
    }

    _ASSERT(false);
    return false;
}


IAppTask* CGCAssemblyLoadManager::GetTask()
{
    CIRef<IObjectLoader> loader(new CAssemblyObjectLoader(m_AssmAccs));

    m_PrjPanel->GetParams(m_ProjectParams);
    string folder_name = m_ProjectParams.m_CreateFolder ? m_ProjectParams.m_FolderName : "";

    CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
    CSelectProjectOptions options;
    m_ProjectParams.ToLoadingOptions(options);
    return new CObjectLoadingTask(srv, *loader, options);
}

static const string kOrgListTag = ".OrgListPanel";
static const string kProjectParamsTag = "ProjectParams";


void CGCAssemblyLoadManager::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}


void CGCAssemblyLoadManager::SaveSettings() const
{
    if ( !m_RegPath.empty() ) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
        m_ProjectParams.SaveSettings(view, kProjectParamsTag);
        if (m_AssmPanel) {
            m_AssmPanel->SaveSettings();
        }
    }
}


void CGCAssemblyLoadManager::LoadSettings()
{
    if ( !m_RegPath.empty() ) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);
        m_ProjectParams.LoadSettings(view, kProjectParamsTag);
        if (m_AssmPanel) {
            m_AssmPanel->LoadSettings();
        }
    }
}


END_NCBI_SCOPE
