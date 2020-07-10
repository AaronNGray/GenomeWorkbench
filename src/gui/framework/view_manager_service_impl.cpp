/*  $Id: view_manager_service_impl.cpp 32743 2015-04-20 16:11:13Z katargir $
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
 * File Description:
 */

#include <ncbi_pch.hpp>

#include <wx/platform.h>

#include <gui/framework/view_manager_service_impl.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/framework/window_manager_service.hpp>

#include <gui/widgets/wx/wm_client.hpp>

#include <gui/utils/extension_impl.hpp>

#include <wx/frame.h>


BEGIN_NCBI_SCOPE

static CExtensionPointDeclaration decl("view_manager_service::view_factory",
                                       "View Manager Service - view factories");


CViewManagerService::CViewManagerService()
:   m_Workbench(NULL)
{
}


CViewManagerService::~CViewManagerService()
{
}


void CViewManagerService::SetWorkbench(IWorkbench* workbench)
{
    m_Workbench = workbench;
}


void CViewManagerService::InitService()
{
    LOG_POST(Info << "Initializing View Manager Service...");

    x_RegisterExtensionFactories();

    LOG_POST(Info << "Finished initializing View Manager Service");
}


void CViewManagerService::ShutDownService()
{
    LOG_POST(Info << "Shutting down View Manager Service...");

    // close all remaining views
    if( ! m_Views.empty())  {
        LOG_POST(Error << "Some views are still open! Closing them forecefully");
        _ASSERT(false);
        RemoveAllViewsFromWorkbench();
    }

    // first get a list of all IWMClients
    vector<IWMClient*> clients;
    IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
    wm_srv->GetAllClients(clients);

    /// remove IViews
    for(size_t i = 0; i < clients.size();  i++) {
        IView* view = dynamic_cast<IView*>(clients[i]);
        if(view)    {
            RemoveFromWorkbench(*view);
        }
    }

    _ASSERT(m_SingletonMap.empty());

    m_ViewNameToFactory.clear();

    LOG_POST(Info << "Finished shutting down View Manager Service");
}


void CViewManagerService::RegisterFactory(IViewFactory& factory)
{
    _ASSERT(m_Workbench);

    const CViewTypeDescriptor& descr = factory.GetViewTypeDescriptor();
    const string& ui_name = descr.GetLabel();

    TViewNameToFactory::iterator it = m_ViewNameToFactory.find(ui_name);
    if(it == m_ViewNameToFactory.end()) {
        m_ViewNameToFactory[ui_name] = CIRef<IViewFactory>(&factory);

        wxFileArtProvider& provider = m_Workbench->GetFileArtProvider();
        factory.RegisterIconAliases(provider);

        CUICommandRegistry& cmd_reg = m_Workbench->GetUICommandRegistry();
        factory.RegisterCommands(cmd_reg, provider);

        IUICommandContributor* cmd_ctrb = dynamic_cast<IUICommandContributor*>( &factory );
        if( cmd_ctrb ){
            vector<CUICommand*> cmds = cmd_ctrb->GetCommands();
            NON_CONST_ITERATE( vector<CUICommand*>, pix, cmds ){
                cmd_reg.RegisterCommand( *pix );
            }

            //x_PushEvtHandler( cmd_ctrb->GetEventHandler() );
        }

        LOG_POST(Info << "CViewManagerService - added a view factory for " << ui_name);
    } else {
        LOG_POST(Error << "CViewManagerService::RegisterFactory() - a factory for view type "
                 << ui_name << " is already registered.");
    }
}


void CViewManagerService::GetTypeDescriptors(TDescrVec& descrs)
{
    NON_CONST_ITERATE(TViewNameToFactory, it,  m_ViewNameToFactory) {
        IViewFactory& factory = *it->second;
        const CViewTypeDescriptor& descr = factory.GetViewTypeDescriptor();
        descrs.push_back(&descr);
    }
}


void CViewManagerService::GetFactories(TFactories& factories)
{
    NON_CONST_ITERATE(TViewNameToFactory, it,  m_ViewNameToFactory) {
        CIRef<IViewFactory> factory = it->second;
        factories.push_back(factory);
    }
}


CIRef<IView> CViewManagerService::CreateViewInstance(const string& type_ui_name)
{
    CIRef<IView> view;
    TViewNameToFactory::iterator it = m_ViewNameToFactory.find(type_ui_name);

    if(it != m_ViewNameToFactory.end()) {
        IViewFactory& factory = *it->second;
        view.Reset(factory.CreateInstance());
    } else {
        LOG_POST(Error << "CViewManagerService::CreateViewInstance() - cannot create " <<
                 type_ui_name << ", factory is not registred for this type");
        _ASSERT(false);
    }
    return view;
}

bool CViewManagerService::CanCreateView(const string& type_ui_name)
{
    TViewNameToFactory::iterator it = m_ViewNameToFactory.find(type_ui_name);
    return it != m_ViewNameToFactory.end();
}


CIRef<IView>
    CViewManagerService::CreateViewInstanceByFingerprint(const TFingerprint& fingerprint)
{
    NON_CONST_ITERATE(TViewNameToFactory, it, m_ViewNameToFactory)    {
        IViewFactory& factory = *it->second;
        IView* view = factory.CreateInstanceByFingerprint(fingerprint);
        if(view)    {
            return CIRef<IView>(view);
        }
    }
    //_ASSERT(false);
    return CIRef<IView>();
}


//TODO clarify error handling policy
void CViewManagerService::AddToWorkbench(IView& view, bool bFloat)
{
    //LOG_POST( Info << "VMS: adding view to workbench..." );

    const CViewTypeDescriptor& descr = view.GetTypeDescriptor();
    IWMClient* client = dynamic_cast<IWMClient*>(&view);

    if( ! client)   {
        LOG_POST(
            Error << "CViewManagerService::AddToWorkbench() - view "
            << descr.GetLabel() << " does not implement IWMClient"
        );
        _ASSERT(false);
    } else {
        _ASSERT(m_Workbench);

        if(m_Workbench) {
            TViews::iterator it = std::find(m_Views.begin(), m_Views.end(), &view);

            if(it != m_Views.end()) {
                const char* msg = "View Manager Service - view already registered!";
                LOG_POST( Error << msg );
                _ASSERT(false);

                NCBI_THROW(CException, eUnknown, msg);
            }

            if(descr.IsSingleton()) {
                const string& ui_name = descr.GetLabel();
                if(m_SingletonMap.find(ui_name) != m_SingletonMap.end())    {
                    LOG_POST(
                        Error << "CViewManagerService::AddToWorkbench() - singleton view"
                        << ui_name << " already exists."
                    );

                    _ASSERT(false);
                    return;
                }
            }

            wxFrame* parent = m_Workbench->GetMainWindow();

            x_DoAddToWorkbench(view, parent);

            // add to Window Manager
            IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
            wm_srv->AddClient(*client, bFloat);

            x_LoadViewSettings(view);
          
            // On some platforms, (I'm looking at you, Mac..) the initial window may
            // not display until a sizing or docking event, so we send an extra size
            // notification to force display.
            //
            // This code also causes delayed crash on wxwidgets 2.9.3 GTK
            // when opening system (non project) views
#ifdef __WXMAC__
            parent->SendSizeEvent();
            parent->Update();
#endif
        }
    }
}


void CViewManagerService::RemoveFromWorkbench(IView& view)
{
    //LOG_POST("CViewManagerService::RemoveFromWorkbench()");

    TViews::iterator it = std::find(m_Views.begin(), m_Views.end(), &view);
    if(it == m_Views.end()) {
        return;
// In composite view the child views are not registered with the View manager
//      _ASSERT(false);
//      const char* s = "CViewManagerService - cannot remove unregistered view";
//      NCBI_THROW(CException, eUnknown, s);
    }

    _ASSERT(m_Workbench);

    IWMClient* client = dynamic_cast<IWMClient*>(&view);

    _ASSERT(client);  // must implement

    if(m_Workbench) {
        // save settings first (while this view is still in window manager)
        x_SaveViewSettings(view);

        // remove from Window Manager
        IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
        wm_srv->CloseClient(*client);

        // remove from Workbench
        x_DoRemoveViewFromWorkbench(view);
    }

    //LOG_POST("CViewManagerService::RemoveFromWorkbench()  END");
}


void CViewManagerService::RemoveAllViewsFromWorkbench()
{
    // make a safe copy, because m_Views may be modified in the loop below
    TViews views = m_Views;

    for( TViews::reverse_iterator it = views.rbegin();  it != views.rend();  it++)   {
        IView& view = **it;
        RemoveFromWorkbench(view);
    }
}


bool CViewManagerService::HasView(const IView& view) const
{
    TViews::const_iterator it = std::find(m_Views.begin(), m_Views.end(), &view);
    return it != m_Views.end();
}


void CViewManagerService::GetViews(TViews& views)
{
    views = m_Views;
}


void CViewManagerService::x_DoAddToWorkbench(IView& view, wxWindow* parent)
{
    //LOG_POST( Info << "VMS: doing add view to workbench..." );

    // here we assume that all preconditions are validated

    // create wxWindow for this view
    view.CreateViewWindow(parent);

    //LOG_POST( Info << "VMS::x_DATW: pushing back view..." );

    // add to the list of registered views
    m_Views.push_back(CIRef<IView>(&view));

    //LOG_POST( Info << "VMS::x_DATW: register singletons..." );

    // register singletons
    const CViewTypeDescriptor& descr = view.GetTypeDescriptor();
    if(descr.IsSingleton()) {
        const string& ui_name = descr.GetLabel();
        m_SingletonMap[ui_name] = &view;
    }

    //LOG_POST( Info << "VMS::x_DATW: finally setting workbench..." );

    // connect to Workbench and Services
    view.SetWorkbench(m_Workbench);
}


void CViewManagerService::x_DoRemoveViewFromWorkbench(IView& view)
{
    //LOG_POST("CViewManagerService::x_DoRemoveViewFromWorkbench()");

    // here we assume that all preconditions are validated
    // disconnect from Workbench and Services
    view.SetWorkbench(NULL);

    // unregister singletons
    const CViewTypeDescriptor& descr = view.GetTypeDescriptor();
    if(descr.IsSingleton()) {
        const string& ui_name = descr.GetLabel();
        m_SingletonMap.erase(ui_name);
    }

    view.DestroyViewWindow();

    // unregister view
    TViews::iterator it = std::find(m_Views.begin(), m_Views.end(), &view);
    m_Views.erase(it);

    //LOG_POST("CViewManagerService::x_DoRemoveViewFromWorkbench()  END");
}


void CViewManagerService::x_LoadViewSettings(IView& view)
{
    IRegSettings* reg_settings = dynamic_cast<IRegSettings*>(&view);
    if(reg_settings)    {
        string reg_path = x_GetViewRegistryPath(view);
        reg_settings->SetRegistryPath(reg_path);
        reg_settings->LoadSettings();
    }
}


void CViewManagerService::x_SaveViewSettings(IView& view)
{
    IRegSettings* reg_settings = dynamic_cast<IRegSettings*>(&view);
    if(reg_settings)    {
        reg_settings->SaveSettings();
    }
}


CIRef<IView> CViewManagerService::GetSingletonView(const string& singleton_ui_name)
{
    TViewNameToFactory::iterator it = m_ViewNameToFactory.find(singleton_ui_name);

    if(it == m_ViewNameToFactory.end())    {
        LOG_POST(Error << "CViewManagerService::ExistsInWorkbench() view "
                 << singleton_ui_name << " is not registered");
    } else {
        IViewFactory& factory = *it->second;
        const CViewTypeDescriptor& descr = factory.GetViewTypeDescriptor();

        _ASSERT(descr.IsSingleton());

        if(descr.IsSingleton()) {
            TSingletonMap::iterator it = m_SingletonMap.find(singleton_ui_name);
            if(it != m_SingletonMap.end())    {
                return it->second;
            }
        } else {
            LOG_POST(Error << "CViewManagerService::ExistsInWorkbench() view "
                     << singleton_ui_name << " is not a singleton.");
        }
    }
    return CIRef<IView>();
}


CIRef<IView> CViewManagerService::ShowSingletonView(const string& singleton_ui_name)
{
    CIRef<IView> view = GetSingletonView(singleton_ui_name);
    if(view)    {
        // activate existing view
        IWindowManagerService* srv = m_Workbench->GetWindowManagerService();
        IWMClient* client = dynamic_cast<IWMClient*>(view.GetPointer());

        _ASSERT(client);

        srv->ActivateClient(*client);
    } else {
        // create a new view
        view = CreateViewInstance(singleton_ui_name);
        if(view)    {
            AddToWorkbench(*view, false);
        }
    }
    return view;
}


IWMClient* CViewManagerService::CreateClient(const TFingerprint& fingerprint,
                                             wxWindow* parent)
{
    /// create IView object
    CIRef<IView> view = CreateViewInstanceByFingerprint(fingerprint);

    // initialize view and connect to Workbench
    if(view)    {
        x_DoAddToWorkbench(*view, parent);

        x_LoadViewSettings(*view);
    }

    IWMClient* client = dynamic_cast<IWMClient*>(view.GetPointer());
    return client;
}


void CViewManagerService::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}


void CViewManagerService::LoadSettings()
{
}


void CViewManagerService::SaveSettings() const
{
}


void  CViewManagerService::x_RegisterExtensionFactories()
{
    // get View Factoroies from our extension point and register them
    vector< CIRef<IViewFactory> >   factories;
    GetExtensionAsInterface("view_manager_service::view_factory", factories);

    for( size_t i = 0;  i < factories.size();  i++ )   {
        IViewFactory& factory = *factories[i];
        RegisterFactory(factory);
    }

    CUICommandRegistry& cmd_reg = m_Workbench->GetUICommandRegistry();
    wxFrame* frame = m_Workbench->GetMainWindow();
    cmd_reg.ApplyAccelerators(frame);
}


string CViewManagerService::x_GetViewRegistryPath(IView& view)
{
    const CViewTypeDescriptor& descr = view.GetTypeDescriptor();

    _ASSERT( ! m_RegPath.empty());
    _ASSERT( ! descr.GetCategory().empty());
    _ASSERT( ! descr.GetLabel().empty());

    string view_key = descr.GetCategory() + "." + descr.GetLabel();
    string reg_path = m_RegPath + ".Views." + view_key;
    return reg_path;
}


///////////////////////////////////////////////////////////////////////////////
/// CViewTypeDescriptor
CViewTypeDescriptor::CViewTypeDescriptor(
    const string& label,
    const string& icon_alias,
    const string& hint,
    const string& description,
    const string& help_id,
    const string& category,
    bool singleton
): CUIObject( label, icon_alias, hint, description, help_id )
    , m_Category( category )
    , m_Singleton( singleton )
{
}


CViewTypeDescriptor::~CViewTypeDescriptor()
{
}


string CViewTypeDescriptor::GetCategory() const
{
    return m_Category;
}

bool CViewTypeDescriptor::IsSingleton() const
{
    return m_Singleton;
}


void CViewTypeDescriptor::SetCategory(const string& category)
{
    m_Category = category;
}


void CViewTypeDescriptor::SetSingleton(bool singleton)
{
    m_Singleton = singleton;
}

END_NCBI_SCOPE
