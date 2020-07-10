#ifndef GUI_FRAMEWORK___WORKBENCH_IMPL__HPP
#define GUI_FRAMEWORK___WORKBENCH_IMPL__HPP

/*  $Id: workbench_impl.hpp 34714 2016-02-03 18:35:25Z katargir $
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
 *      CWorkbench - standard implementation of IWorkbench interface.
 */

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/framework/workbench.hpp>
#include <gui/framework/workbench_frame.hpp>
#include <gui/framework/service.hpp>

#include <gui/objutils/reg_settings.hpp>
#include <gui/utils/event_handler.hpp>

#include <wx/event.h>

BEGIN_NCBI_SCOPE

class CEventLogService;
class CStatusBarService;
class CWindowManagerService;
class CMenuService;
class CToolBarService;
class CAppTaskService;
class CViewManagerService;

/// dynamic commands start form this number
static const int kDynamicCmdsStart = 20000;

///////////////////////////////////////////////////////////////////////////////
/// CWorkbench - default implementation of IWorkbench.
///
/// CWorkbench implements IWorkbench::GetExtraServiceLocator() function and
/// provides additional functions that allow application components to register
/// supplementary application-specific services.

class NCBI_GUIFRAMEWORK_EXPORT CWorkbench :
    public wxEvtHandler,
    public CEventHandler,
    public IWorkbench,
    public CWorkbenchFrame::IHintListener,
    public IRegSettings
{
public:
    CWorkbench(IWorkbenchAdvisor* advisor);
    virtual ~CWorkbench();

    /// @name IWorkbench implementation
    /// @{
    virtual void    Init();
    virtual bool    CanShutDown();
    virtual void    ShutDown();

    virtual IEventLogService*      GetEventLogService();
    virtual IStatusBarService*     GetStatusBarService();
    virtual IWindowManagerService*    GetWindowManagerService();
    virtual IMenuService*     GetMenuService();
    virtual IToolBarService*  GetToolBarService();

    virtual CAppTaskService*    GetAppTaskService();
    virtual IViewManagerService*       GetViewManagerService();
    virtual wxFileArtProvider&  GetFileArtProvider();

    virtual IServiceLocator* GetServiceLocator();
    virtual void    RegisterService(const string& class_name, IService* service);
    virtual void    UnRegisterService(const string& class_name);

    virtual wxFrame*    GetMainWindow();
    virtual CUICommandRegistry&     GetUICommandRegistry();

    virtual string  GetRegistrySectionPath(ERegistrySection section);
    /// @}

    /// @name IServiceLocator interface implementation
    /// @{
    virtual bool    HasService(const string& name);
    virtual CIRef<IService> GetService(const string& name);
    /// @}

    /// @name CWorkbenchFrame::IHintListener interface implementation
    /// @{
    virtual void    ShowCommandHint(const string& text);
    virtual void    HideCommandHint();
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

    /// @name Event Handling
    /// @{
    bool ProcessEvent(wxEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
    /// @}

protected:
    virtual void    x_CreateFileArtProvider();

    virtual void    x_InitMainFrame();
    virtual void    x_DestroyMainFrame();

    // factory methods that can be overloaded in derived classes
    virtual CWorkbenchFrame*  x_CreateMainFrame(const string& title);

    virtual void    x_ShutDownServices();

protected:
    typedef map<string, CIRef<IService> >   TNameToService;

    string  m_RegPath; /// path to th Workbench section in the Registry

    IWorkbenchAdvisor*  m_Advisor;

    wxFileArtProvider*  m_FileArtProvider;

    /// pointers to the Standard Services

    CWorkbenchFrame*  m_MainFrame;

    CEventLogService*   m_EventLogService;
    CMenuService*       m_MenuService;
    CStatusBarService*  m_StatusBarService;

    CWindowManagerService*  m_WindowManagerService;
    CAppTaskService*        m_AppTaskService;
    CViewManagerService*    m_ViewManagerService;

    /// a Map of Additional Services :  Name -> Service
    TNameToService  m_NameToExtraService;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___WORKBENCH_IMPL__HPP

