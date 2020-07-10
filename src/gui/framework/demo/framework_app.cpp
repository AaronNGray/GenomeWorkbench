/*  $Id: framework_app.cpp 35258 2016-04-18 01:35:40Z whlavina $
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


#include <ncbi_pch.hpp>

#include <gui/widgets/wx/wx_app.hpp>

#include <gui/framework/demo/gui.hpp>
#include <gui/framework/workbench_impl.hpp>
#include <gui/framework/app_task_service.hpp>

#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/thread_pool_engine.hpp>
#include <gui/widgets/wx/sys_path.hpp>

#include <wx/menu.h>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// Derive our application class from CwxNCBIApp and use it together with
/// standard CNCBIwxApplication.

class CFrameworkDemoApp: public CwxNCBIApp
{
protected:
    // we are overriding this function to create our main frame window
    virtual bool    x_CreateGUI();
    virtual void    x_DestroyGUI();

protected:
    virtual void    x_InitAppJobDispatcher();

    virtual wxString x_GetGuiRegistryPath();

    virtual bool    x_OnIdle();

protected:
    CFrameworkDemoGUI   m_GUI;
    auto_ptr<CWorkbench>    m_Workbench;
};


// Declare wxWidgets application class
// this will allow CNCBIwxApplication to instantiate our class
DECLARE_APP(ncbi::CFrameworkDemoApp)
IMPLEMENT_APP_NO_MAIN(ncbi::CFrameworkDemoApp)
IMPLEMENT_WX_THEME_SUPPORT


void CFrameworkDemoApp::x_InitAppJobDispatcher()
{
    CwxNCBIApp::x_InitAppJobDispatcher();

    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();

    // register additional Thread Pool engine for executing tools
    static int kMaxToolThreads = 3;
    static int kMaxToolJobs = 100;
    CThreadPoolEngine* engine = new CThreadPoolEngine(kMaxToolThreads, kMaxToolJobs);
    disp.RegisterEngine("ThreadPool", *engine);
}


bool CFrameworkDemoApp::x_CreateGUI()
{
    //LOG_POST("");
    LOG_POST(Info << "CFrameworkDemoApp::x_CreateGUI() BEGIN");

    // create Workbench instance with m_GUI as Advisor
    m_Workbench.reset(new CWorkbench(&m_GUI));

    /// set path in Registry, we need to this before calling Init()
    m_Workbench->SetRegistryPath("Workbench");

    m_GUI.SetWorkbench(m_Workbench.get());
    m_GUI.SetRegistryPath("ApplicationGUI");

    m_Workbench->Init();

    m_GUI.RestoreWindowLayout();

    LOG_POST(Info << "CFrameworkDemoApp::x_CreateGUI() END");
    return true;
}


void CFrameworkDemoApp::x_DestroyGUI()
{
    //LOG_POST("");
    LOG_POST(Info << "CFrameworkDemoApp::x_DestroyGUI() BEGIN");

    // m_Workbench already has been shut down
    m_GUI.SetWorkbench(NULL);

    m_Workbench.reset();

    LOG_POST(Info << "CFrameworkDemoApp::x_DestroyGUI() END");
}


wxString CFrameworkDemoApp::x_GetGuiRegistryPath()
{
    return CSysPath::ResolvePath(wxT("<home>/demo_gui_framework.asn"));
}


// we override this function to add idle processing for CAppTaskService
bool CFrameworkDemoApp::x_OnIdle()
{
    bool handled = CwxNCBIApp::x_OnIdle();
    if( ! handled  &&  m_Workbench.get() != NULL)  {
        CAppTaskService* task_mgr = m_Workbench->GetAppTaskService();
        if(task_mgr)    {
            handled = task_mgr->IdleCallback();
        }
    }
    return handled;
}


END_NCBI_SCOPE

/////////////////////////////////////////////////////////////////////////////
//  main entry point

int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // create standard NCBI wxWidgets application
    ncbi::CNCBIwxApplication ncbi_app("wxWidgets demo App");

    // Execute main application function
    return ncbi_app.AppMain(argc, argv);
}
