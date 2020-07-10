#ifndef GUI_FRAMEWORK___WORKBENCH__HPP
#define GUI_FRAMEWORK___WORKBENCH__HPP

/*  $Id: workbench.hpp 34721 2016-02-04 14:05:23Z katargir $
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
 *
 */

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbistd.hpp>

#include <gui/framework/service.hpp>

class wxFileArtProvider;
class wxFrame;
class wxCommandEvent;

BEGIN_NCBI_SCOPE

class IServiceLocator;
class IEventLogService;
class IStatusBarService;
class IWindowManagerService;
class IMenuService;
class IToolBarService;

class CUICommandRegistry;
class CAppTaskService;
class IViewManagerService;

class IWMClient;
class IWorkbench;

///////////////////////////////////////////////////////////////////////////////
/// IWorkbenchAdvisor
/// This is an interface that assists IWorkbench in carrying out its functions.
/// IWorkbench calls IWorkbenchAdvisor's methods at particular moments during
/// its lifetime. Overrding IWorkbenchAdvisor is the recommended way for adding
/// application-specific funtionality.
class IWorkbenchAdvisor
{
public:
    /// get application title
    virtual string  GetAppTitle() = 0;

    /// called by Workbench after creating main app window
    virtual void    PostCreateWindow() = 0;

    /// called after Workbench initialization has been completed
    virtual void    PostInit() = 0;

    /// called by Workbench before starting ShutDown
    virtual void    PreShutDown() = 0;

    /// called by Workbench before destroying main app window
    virtual void    PreDestroyWindow() = 0;

    virtual bool    CanShutDown() = 0;

    virtual ~IWorkbenchAdvisor()    {};
};


///////////////////////////////////////////////////////////////////////////////
/// IWorkbench is the central interface in the application framework.
/// Workbench provides critical application services and UI components and, so
/// forms the environment in which various application components exist and
/// function. An application creates an instance of Workbench at startup and
/// registers additional services and components if needed. Workbench
/// constructs the basic UI, then uses extension registry to retrieve
/// user-defined components that add functionality to Workbench.  A typical
/// application does not need to implement IWorkbench interface, a standard
/// implementation of the interface can be used, customization can be achieved
/// by implementing IWorkbenchAdvisor interface and be registering custom
/// extensions in Extension Registry.
///
/// IWorkbench is derived from IServiceLocator, components that implement
/// IService interface can be obtained using IServiceLocator methods.


class IWorkbench :
    public IServiceLocator
{
public:
    /// enumerates major Registry sections
    enum ERegistrySection   {
        eServices,
        eDialogs,
        eTools
    };

public:
    virtual void    Init() = 0;
    virtual bool    CanShutDown() = 0;
    virtual void    ShutDown() = 0;

    virtual IEventLogService*      GetEventLogService() = 0;
    virtual IStatusBarService*     GetStatusBarService() = 0;
    virtual IWindowManagerService*    GetWindowManagerService() = 0;
    virtual IMenuService*       GetMenuService() = 0;
    virtual IToolBarService*    GetToolBarService() = 0;

    virtual CAppTaskService*    GetAppTaskService() = 0;
    virtual wxFileArtProvider&  GetFileArtProvider() = 0;
    virtual IViewManagerService*       GetViewManagerService() = 0;

    //TODO implement virtual CIRef<IHelpService>         GetHelpService();

    /// get a Service Locator associated with the Workspace
    virtual IServiceLocator* GetServiceLocator() = 0;

    /// register an application-specific service;
    /// Workbench assumes ownership of the service
    virtual void    RegisterService(const string& class_name,
                                    IService* service) = 0;

    /// remove application-specific service
    virtual void    UnRegisterService(const string& class_name) = 0;

    /// returns a pointer to the main application frame window
    virtual wxFrame*    GetMainWindow() = 0;

    /// returns an instance of Command Registry associated with the Workbench
    virtual CUICommandRegistry&     GetUICommandRegistry() = 0;

    /// returns a path to CGuiRegistry section where resources of the
    /// specified type shall be located
    virtual string  GetRegistrySectionPath(ERegistrySection section) = 0;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___WORKBENCH__HPP

