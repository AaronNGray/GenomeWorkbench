 /*  $Id: wx_app.cpp 43927 2019-09-20 18:14:56Z katargir $
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

#include <corelib/ncbi_system.hpp>
#include <connect/ncbi_core_cxx.hpp>

#include <wx/image.h>
#include <wx/sysopt.h>
#include <wx/window.h>

#include <gui/widgets/wx/wx_app.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/log_gbench.hpp>
#include <gui/widgets/wx/async_call.hpp>

#include <gui/utils/event_handler.hpp>
#include <gui/objutils/registry.hpp>

#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/thread_pool_engine.hpp>

#if defined(__WXMAC__) && !defined(__WXUNIVERSAL__)
    #include <ApplicationServices/ApplicationServices.h>
#endif

#include <wx/filename.h>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CNCBIwxApplication

CNCBIwxApplication::CNCBIwxApplication(const string& app_name)
{
    //NB: don't do this; it screws up application name settings in the logging
    //framework
    //SetProgramDisplayName(app_name);

}


CNCBIwxApplication::~CNCBIwxApplication()
{
}


void CNCBIwxApplication::Init()
{
#ifdef _DEBUG
    SetDiagPostLevel(eDiag_Trace);
#else
    SetDiagPostLevel(eDiag_Info);
#endif
    x_SetupArgDescriptions();

    // now we are ready to setup wxApp object
    x_Init_wxApplication();
}


// setup command line argument descriptions
void CNCBIwxApplication::x_SetupArgDescriptions()
{
    // Create command-line argument descriptions class
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              GetProgramDisplayName());

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());
}


// create and setup an instance of wxApp
void CNCBIwxApplication::x_Init_wxApplication()
{
    // initializer is provided by DECLARE_APP macro
    wxAppInitializerFunction fnCreate = wxApp::GetInitializerFunction();
    if (fnCreate)   {
        wxAppConsole* app = (*fnCreate)();
        wxApp::SetInstance(app);

        // wxApp instance should inherit program name from CNCBIwxApplication
        app->SetAppName(ToWxString(GetProgramDisplayName()));
    } else {
        _ASSERT(false);
    }
}


int CNCBIwxApplication::Run(void)
{
    int retcode = -1;

	CONNECT_Init(&GetConfig());

    try {   
        retcode = x_Run();
    } NCBI_CATCH("CNCBIwxApplication::Run()");

    if(retcode != 0)    {
        LOG_POST(Error << "Application terminated");
    }
    // at this point wxApp is already dead
    return retcode;
}


int CNCBIwxApplication::x_Run()
{
    // now execute wxApp instance
    int i = 0;
    int retcode = wxEntry(i, (char**)NULL); // no command line supplied
    return retcode;
}


void CNCBIwxApplication::Exit(void)
{
    SetDiagStream(0);
}


///////////////////////////////////////////////////////////////////////////////
/// CwxNCBIApp

//IMPLEMENT_APP_NO_MAIN( ncbi::CwxNCBIApp )
///IMPLEMENT_WX_THEME_SUPPORT

// CwxNCBIApp type definition
IMPLEMENT_CLASS(CwxNCBIApp, wxApp)  

// CwxNCBIApp event table definition
BEGIN_EVENT_TABLE( CwxNCBIApp, wxApp )
    EVT_IDLE( CwxNCBIApp::OnIdle )
END_EVENT_TABLE()


CwxNCBIApp::CwxNCBIApp( bool use_job_disp )
    : wxApp()
    , m_UseJobDisp( use_job_disp )
    , m_MaxThreads(4)
    , m_MaxJobs(10000)
    , m_RegistryLoaded(false)
{
    SetVendorName(wxT("NCBI")); // TODO move into the base class
}


CwxNCBIApp::~CwxNCBIApp()
{
}

bool CwxNCBIApp::OnInit()
{
#if defined(__WXMAC__) && !defined(__WXUNIVERSAL__)
    wxSystemOptions::SetOption( wxT("mac.listctrl.always_use_generic"), true );
#endif

    x_RegisterImageHandlers();

    x_LoadGuiRegistry();

    if (!x_TestNcbiConnection())
        return false;

    if(m_UseJobDisp)    {
        x_InitAppJobDispatcher();
    }

    bool ok = x_CreateGUI();
    return ok;
}


int CwxNCBIApp::OnExit()
{
    x_DestroyGUI();
    x_SaveGuiRegistry();

    // shut down App Job Dispatcher
    if(m_UseJobDisp)    {
        CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
        disp.ShutDown();
        CAppJobDispatcher::ReleaseInstance();
    }
    
    return wxApp::OnExit();
}


void CwxNCBIApp::x_InitAppJobDispatcher()
{
    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance(); /// create

    // register the default Thread Pool engine for executing background UI tasks
    CThreadPoolEngine* engine = new CThreadPoolEngine(m_MaxThreads, m_MaxJobs);
    disp.RegisterEngine("ThreadPool", *engine);
}


bool CwxNCBIApp::x_CreateGUI()
{
    _ASSERT(false);
    // override this function in derived classes, create a main frame and show it
    return false;
}
    

void CwxNCBIApp::x_DestroyGUI()
{
    // override this function in derived classes
}


void CwxNCBIApp::x_LoadGuiRegistry()
{
    if( m_RegistryLoaded ){
        return;
    }

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();

    wxString path = x_GetGuiRegistryPath();
    string action = "Loading app gui registry - ";

    if( !path.empty() ){
        if( wxFileName::FileExists( path ) ){
            CNcbiIfstream istr( path.fn_str() );
            if( istr ){
                gui_reg.SetLocal( istr );
                m_RegistryLoaded = true;
            } else {
                LOG_POST( Error << action << "registry file is invalid " << path );
            }
        } else {
            LOG_POST( Info << action << "skipping, no file " << path );
        }
    } else {
        LOG_POST( Info << action << "skipping, path is empty." );
    }
}


void CwxNCBIApp::x_SaveGuiRegistry()
{
    wxString path = x_GetGuiRegistryPath();
    if( !path.empty() ){
        CNcbiOfstream ostr( path.fn_str() );
        CGuiRegistry::GetInstance().Write( ostr );
    }
    else {
        LOG_POST( Info << "Saving app gui registry - skipping, path is empty" );
    }
}

wxString CwxNCBIApp::x_GetGuiRegistryPath()
{
    return wxEmptyString; // no registry, override this in derived classes
}

void CwxNCBIApp::x_RegisterImageHandlers()
{
#if wxUSE_XPM
    wxImage::AddHandler(new wxXPMHandler);
#endif
#if wxUSE_LIBPNG
    wxImage::AddHandler(new wxPNGHandler);
#endif
#if wxUSE_LIBJPEG
    wxImage::AddHandler(new wxJPEGHandler);
#endif
#if wxUSE_GIF
    wxImage::AddHandler(new wxGIFHandler);
#endif
}

void CwxNCBIApp::OnIdle( wxIdleEvent& event )
{
    static bool fReentry = false;
    if (fReentry)
        return;
    CBoolGuard _guard(fReentry);

    if (CAsyncCall::InsideAsyncCall())
        return;

    if (wxWindow::GetCapture())
        return;

    if (CJobHandler::Instance().Process())
        return;

    try {
        // if something has been done in this cycle - request more events
        bool handled = x_OnIdle();
        if (handled)  {
            event.RequestMore();
        }
    } NCBI_CATCH("CwxNCBIApp::OnIdle()");

    // let's base class to handle it as well
    event.Skip();
}


bool CwxNCBIApp::x_OnIdle()
{
    // we poll various services to let them process their messages, to keep
    // the application responsive we poll only one service at a time
    bool handled = false;
    
    // CEventHandler has the highest priority - try it first
    //
    // if post event queue is long enough we force multiple events
    // processing
    const int kPostEventCount = 10;
    for (int i = 0; i < kPostEventCount; ++i) {
        bool h = CEventHandler::HandlePostRequest();
        handled |= h;
        if (!h) break;
    }

    if(m_UseJobDisp  &&  !handled)  {
        CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
        handled |= disp.IdleCallback();
    }
    return handled; 
}


END_NCBI_SCOPE
