/*  $Id: net_blast_ui_load_manager.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/packages/pkg_alignment/net_blast_ui_load_manager.hpp>

#include <gui/packages/pkg_alignment/net_blast_load_option_panel.hpp>
#include <gui/packages/pkg_alignment/blast_search_task.hpp>

#include <gui/core/project_service.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/fileartprov.hpp>

#include <gui/objutils/registry.hpp>
#include <gui/utils/extension_impl.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
/// CNetBLASTUILoadManager
CNetBLASTUILoadManager::CNetBLASTUILoadManager(CNetBLASTUIDataSource& data_source)
:   m_DataSource(&data_source),
    m_SrvLocator(NULL),
    m_Descriptor(NET_BLAST_LOADER_LABEL, ""),
    m_State(eInvalid),
    m_OptionPanel(NULL),
    m_ProjectSelPanel(NULL)
{
    m_Descriptor.SetLogEvent("loaders");
}


void CNetBLASTUILoadManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CNetBLASTUILoadManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CNetBLASTUILoadManager::GetDescriptor() const
{
    return m_Descriptor;
}


void CNetBLASTUILoadManager::InitUI()
{
    m_State = eSelectRID;
}


void CNetBLASTUILoadManager::CleanUI()
{
    m_State = eInvalid;
    if(m_OptionPanel)   {
        m_SavedInput = m_OptionPanel->GetInput();
        m_OptionPanel = NULL; // window is destroyed by the system
    }
    m_ProjectSelPanel = NULL;
}


wxPanel* CNetBLASTUILoadManager::GetCurrentPanel()
{
    if(m_State == eSelectRID)   {
        if(m_OptionPanel == NULL)   {
            m_OptionPanel = new CNetBLASTLoadOptionPanel(m_ParentWindow);
            m_OptionPanel->SetInput(m_SavedInput);
        }
        return m_OptionPanel;
    }  else if(m_State == eSelectProject)   {
        if(m_ProjectSelPanel == NULL)   {
            m_ProjectSelPanel = new CProjectSelectorPanel(m_ParentWindow);

            CIRef<CProjectService> srv =
                m_SrvLocator->GetServiceByType<CProjectService>();
            m_ProjectSelPanel->SetProjectService(srv);

            m_ProjectSelPanel->SetParams(m_ProjectParams);
            m_ProjectSelPanel->TransferDataToWindow();
        }
        return m_ProjectSelPanel;
    }
    return NULL;
}


bool CNetBLASTUILoadManager::CanDo(EAction action)
{
    switch(m_State) {
    case eSelectRID:
        return action == eNext;
    case eSelectProject:
        return action == eBack  ||  action == eNext;
    case eCompleted:
        return false; // nothing left to do
    default:
        _ASSERT(false);
        return false;
    }
}


bool CNetBLASTUILoadManager::IsFinalState()
{
    return m_State == eSelectProject;
}


bool CNetBLASTUILoadManager::IsCompletedState()
{
    return m_State == eCompleted;
}


bool CNetBLASTUILoadManager::DoTransition(EAction action)
{
    if(m_State == eSelectRID  &&  action == eNext)    {
        if (m_OptionPanel->IsInputValid())  {
            m_State = eSelectProject;
            return true;
        }
        return false;
    } else if( m_State == eSelectProject) {
        if(action == eBack)  {
            m_State = eSelectRID;
            return true;
        } else if(action == eNext)  {
            if(m_ProjectSelPanel->TransferDataFromWindow()) {
                m_State = eCompleted;
                return true;
            }
            return false;
        }
    }
    _ASSERT(false);
    return false;
}


IAppTask* CNetBLASTUILoadManager::GetTask()
{
    // extract parameters from the dialog panels
    vector<string> RIDs;
    m_OptionPanel->GetRIDs(RIDs);

    // logging...
    {
        string sr;
        for (size_t i = 0; i < RIDs.size(); ++i) {
            sr.append(RIDs[i]);
            sr.append(", ");
        }
        LOG_POST(Info << "CNetBLASTUILoadManager:: RIDs=" << sr);
    }

    m_ProjectSelPanel->GetParams(m_ProjectParams);
    string folder_name = m_ProjectParams.m_CreateFolder ? m_ProjectParams.m_FolderName : "";

    // create Blast Search Task for loading
    CBlastSearchTask* task = new CBlastSearchTask(m_SrvLocator, *m_DataSource, m_Descriptor.GetLabel());
    task->Init_RetrieveRID(RIDs);

    m_ProjectSelPanel->GetParams(m_ProjectParams);

    CSelectProjectOptions options;
    m_ProjectParams.ToLoadingOptions(options);
    task->SetLoadingOptions(options);

    return task;
}


void CNetBLASTUILoadManager::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}


static const char* kRIDsInputTag = "RIDs Input";
static const char* kProjectParamsTag = "ProjectParams";


void CNetBLASTUILoadManager::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        /// remember the selected Format (only if m_OptionPanel exists)
        if(m_OptionPanel)   {
            m_SavedInput = m_OptionPanel->GetInput();
        }
        // do not save large texts
        if (m_SavedInput.size() < 1000)
            view.Set(kRIDsInputTag, m_SavedInput);

        /// save Project Panel settings
        m_ProjectParams.SaveSettings(view, kProjectParamsTag);
    }
}


void CNetBLASTUILoadManager::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        //TODOm_SavedInput = view.GetString(kRIDsInputTag, m_SavedInput);

        /// load Project Panel settings
        m_ProjectParams.LoadSettings(view, kProjectParamsTag);
    }
}


END_NCBI_SCOPE
