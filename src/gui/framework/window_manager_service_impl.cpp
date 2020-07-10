/*  $Id: window_manager_service_impl.cpp 33169 2015-06-05 18:46:12Z katargir $
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

#include <gui/framework/window_manager_service_impl.hpp>

#include <gui/framework/workbench_frame.hpp>
#include <gui/widgets/wx/window_manager.hpp>
#include <gui/widgets/wx/wx_utils.hpp>


BEGIN_NCBI_SCOPE


CWindowManagerService::CWindowManagerService()
:   m_WindowManager(NULL),
    m_Frame(NULL),
    m_MenuService(NULL)
{
}


CWindowManagerService::~CWindowManagerService()
{
    if (m_WindowManager)
        m_WindowManager->SetMenuListener(0);
}


void CWindowManagerService::SetFrame(wxFrame* frame)
{
    m_Frame = frame;
}


void CWindowManagerService::InitService()
{
    _ASSERT(m_Frame);

    // create and setip n instance of CWindowManager
    CWindowManager::RegisterImageAliases(*GetDefaultFileArtProvider());

    m_WindowManager = new CWindowManager(m_Frame);
    m_WindowManager->SetMenuListener(this);

    string path = CGuiRegistryUtil::MakeKey(m_RegPath, "Window Manager");
    m_WindowManager->SetRegistryPath(path);

    m_WindowManager->Create(m_Frame, wxID_ANY);

    CWorkbenchFrame* wf = dynamic_cast<CWorkbenchFrame*>(m_Frame);
    if (wf!=NULL) {
        wf->SetDockManager( m_WindowManager->GetDockManager());
    }
}


void CWindowManagerService::ShutDownService()
{
    if(m_WindowManager) {
        m_WindowManager->SetMenuListener(NULL);

        x_ShutDownToolBars();

        //TODO need to FORCE
        m_WindowManager->CloseAllClients();

        m_WindowManager->Hide();
        m_WindowManager->Destroy();
        m_WindowManager = NULL;
    }
}


void CWindowManagerService::SetAdvisor(IWindowManagerAdvisor* advisor)
{
    _ASSERT(m_WindowManager);
    m_WindowManager->SetAdvisor(advisor);
}


void CWindowManagerService::AddClient(IWMClient& client, bool bFloat)
{
    _ASSERT(m_WindowManager);
    m_WindowManager->AddClient(client, bFloat);
}

wxFrame* CWindowManagerService::MoveToFloatingFrame(IWMClient& client)
{
    _ASSERT(m_WindowManager);
    return m_WindowManager->MoveToFloatingFrame(client);
}

void CWindowManagerService::CloseClient(IWMClient& client)
{
    _ASSERT(m_WindowManager);
    m_WindowManager->CloseClient(client);
}


void CWindowManagerService::CloseClients(const TClients& clients)
{
    _ASSERT(m_WindowManager);
    m_WindowManager->CloseClients(clients);
}


void CWindowManagerService::CloseAllClients()
{
    _ASSERT(m_WindowManager);
    m_WindowManager->CloseAllClients();
}


void CWindowManagerService::GetAllClients(TClients& clients)
{
    _ASSERT(m_WindowManager);
    m_WindowManager->GetAllClients(clients);
}


IWMClient* CWindowManagerService::GetActiveClient()
{
    _ASSERT(m_WindowManager);
    return m_WindowManager->GetActiveClient();
}


void CWindowManagerService::ActivateClient(IWMClient& client)
{
    _ASSERT(m_WindowManager);
    m_WindowManager->ActivateClient(client);
}


void CWindowManagerService::ActivateClients(TClients& clients)
{
    _ASSERT(m_WindowManager);
    m_WindowManager->ActivateClients(clients);
}

void CWindowManagerService::RegisterActiveClient(IWMClient& client)
{
    _ASSERT(m_WindowManager);
    m_WindowManager->RegisterActiveClient(client);
}

void CWindowManagerService::UnRegisterActiveClient(IWMClient& client)
{
    _ASSERT(m_WindowManager);
    m_WindowManager->UnRegisterActiveClient(client);
}

void CWindowManagerService::RaiseFloatingWindowsInZOrder()
{
    _ASSERT(m_WindowManager);
    m_WindowManager->RaiseFloatingWindowsInZOrder();
}

void CWindowManagerService::RaiseFloatingWindowsInZOrder(wxRect r)
{
    _ASSERT(m_WindowManager);
    m_WindowManager->RaiseFloatingWindowsInZOrder(r);
}

bool CWindowManagerService::IsDragging() const
{
    _ASSERT(m_WindowManager);
    return (m_WindowManager->IsDragging());
}

void CWindowManagerService::RefreshClient(IWMClient& client)
{
    _ASSERT(m_WindowManager);
    m_WindowManager->RefreshClient(client);
}

void CWindowManagerService::ApplyLayout(const objects::CUser_object& layout)
{
    return m_WindowManager->ApplyLayout(layout);
}


objects::CUser_object* CWindowManagerService::SaveLayout()
{
    _ASSERT(m_WindowManager);
    return m_WindowManager->SaveLayout();
}


void CWindowManagerService::LoadLayout(const objects::CUser_object& layout,
                                       IWMClientFactory& factory)
{
    _ASSERT(m_WindowManager);
    m_WindowManager->LoadLayout(layout, factory);
}


void CWindowManagerService::x_ShutDownToolBars()
{
    //TODO delete all existing toolbars
    m_ToolBarContexts.clear();
    m_TBNameToCount.clear();

    for( size_t i = 0; i < m_ToolBarFactories.size();  i++ )    {
        IToolBarContributor* factory = m_ToolBarFactories[i];
        delete factory;
    }
    m_ToolBarFactories.clear();
}


void CWindowManagerService::AddToolBarFactory(IToolBarContributor* factory)
{
    _ASSERT(m_WindowManager  &&  factory);

    if(factory == NULL) {
        ERR_POST("CToolBarService::AddToolBarFactory() - NULL factory");
    } else {
        TToolBarFactories::const_iterator it =
            std::find(m_ToolBarFactories.begin(), m_ToolBarFactories.end(),
                      factory);

        if(it == m_ToolBarFactories.end())  {
            m_ToolBarFactories.push_back(factory); // add the factory
        } else {
            LOG_POST(Error << "CToolBarService::AddToolBarFactory() - "
                     "factory already registered");
        }
    }
}


void CWindowManagerService::DeleteToolBarFactory(IToolBarContributor* factory)
{
    _ASSERT(factory);

    TToolBarFactories::iterator it =
        std::find(m_ToolBarFactories.begin(), m_ToolBarFactories.end(), factory);

    if(it == m_ToolBarFactories.end())  {
        ERR_POST("CWorkbench::RemoveToolBarFactory() - "
                 "factory is not registered");
    } else {
        delete factory;
        m_ToolBarFactories.erase(it);
    }
}


void CWindowManagerService::UpdateMenuBar()
{
    _ASSERT(m_MenuService);
    if(m_MenuService)   {
        m_MenuService->ResetMenuBar(); //TODO optimize
    }
}


void CWindowManagerService::AddToolBarContext(IToolBarContext* context)
{
    static const char* kFunc = "CWindowManagerService::AddToolBarContext()";
    _ASSERT(context);

    TToolBarContexts::const_iterator it = m_ToolBarContexts.find(context);
    if(it != m_ToolBarContexts.end())    {
        LOG_POST(Error << kFunc << " context already registered");
    } else {
        m_ToolBarContexts.insert(context);

        vector<string> new_names;
        context->GetCompatibleToolBars(new_names);

        // show new toolbars if needed
        for( size_t i = 0;  i < new_names.size(); i++ ) {
            const string& name = new_names[i];
            TTBNameToCount::iterator n_it = m_TBNameToCount.find(name);
            if(n_it == m_TBNameToCount.end())    {
                // add the new toolbar
                m_TBNameToCount[name] = 1;
                x_ShowToolBar(name, true);
            } else {
                n_it->second++;
            }
        }
    }
}


void CWindowManagerService::RemoveToolBarContext(IToolBarContext* context)
{
    static const char* kFunc = "CWindowManagerService::RemoveToolBarContext()";
    _ASSERT(context);

    TToolBarContexts::iterator it = m_ToolBarContexts.find(context);
    if(it == m_ToolBarContexts.end())    {
        _ASSERT(false);
        LOG_POST(Error << kFunc << " context is not registered");
    } else {
        vector<string> names;
        context->GetCompatibleToolBars(names);

        /// remove names from the map
        for( size_t i = 0;  i < names.size(); i++ ) {
            const string& name = names[i];
            TTBNameToCount::iterator n_it = m_TBNameToCount.find(name);

            if( n_it != m_TBNameToCount.end() ){
                if(n_it->second == 1)   {
                    // the last reference to this toolbar
                    x_ShowToolBar(name, false);
                    m_TBNameToCount.erase(n_it);
                } else {
                    n_it->second--;
                }
            }
        }

        m_ToolBarContexts.erase(it);
    }
}


void CWindowManagerService::x_ShowToolBar(const string& name, bool show)
{
#ifdef _DEBUG
    LOG_POST(Info << "CWindowManagerService::x_ShowToolBar() " << name << " "  << show);
#endif

    bool is_shown = m_WindowManager->HasToolBar(name);

    if(is_shown != show)    {
        // action needed
        if(show)    {
            // create and add a new toolbar
            NON_CONST_ITERATE(TToolBarFactories, it, m_ToolBarFactories)    {
                IToolBarContributor& factory = **it;

                vector<string> names;
                factory.GetToolBarNames(names);
                vector<string>::iterator it_n =
                        std::find(names.begin(), names.end(), name);
                if(it_n != names.end()) {
                    wxFrame* frame = m_WindowManager->GetFrameWindow();
                    _ASSERT(frame);

                    wxAuiToolBar* toolbar = factory.CreateToolBar(name, frame);
                    m_WindowManager->AddToolBar(toolbar);
                    return;
                }
            }
            LOG_POST(Error << "Window Manager Service - cannot create toolbar \""
                           << name << " - factory is not found");
        } else {
            m_WindowManager->DeleteToolBar(name);
        }
    }
}


void CWindowManagerService::SetMenuService(IMenuService* service)
{
    m_MenuService = service;
}


const wxMenu* CWindowManagerService::GetMenu()
{
    _ASSERT(m_WindowManager);

    if(m_WindowManager) {
        const wxMenu* menu = m_WindowManager->GetMenu();
        if(menu)    {
            return CloneMenu(*menu);
        }
    }
    return NULL;
}

void CWindowManagerService::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}


void CWindowManagerService::LoadSettings()
{
}


void CWindowManagerService::SaveSettings() const
{
}


bool CWindowManagerService::OnCommandEvent(wxCommandEvent& event)
{
    if(m_WindowManager) {
        return m_WindowManager->ProcessEvent(event);
    }
    return false;
}


END_NCBI_SCOPE
