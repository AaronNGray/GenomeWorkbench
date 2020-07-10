/*  $Id:  macrofloweditorapp.cpp 38070 06/07/2017 15:12:46 17:48:35Z Igor Filippov$
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Igor Filippov
 */
#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbiargs.hpp>
#include <corelib/ncbifile.hpp>

#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/thread_pool_engine.hpp>

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "macrofloweditorapp.hpp"

////@begin XPM images

////@end XPM images

BEGIN_NCBI_SCOPE

class CMacroFlowEditorNcbiApp : public CNcbiApplication
{
private:
    virtual void Init(void);
    virtual int  Run(void);
    virtual void Exit(void);
};

/////////////////////////////////////////////////////////////////////////////
//  Init - setup command-line arguments

void CMacroFlowEditorNcbiApp::Init(void)
{
    CArgDescriptions* argdesc = new CArgDescriptions();
    argdesc->SetUsageContext(GetArguments().GetProgramBasename(), "Macro Flow Editor");
    argdesc->AddDefaultKey("s", "script", "Open Script",CArgDescriptions::eString, "");
    argdesc->AddDefaultKey("d", "directory", "Default Folder for Save Script",CArgDescriptions::eString, "");

    SetupArgDescriptions(argdesc);
}


/////////////////////////////////////////////////////////////////////////////
//  Run - i.e. parse command-line arguments and demo simple operations.


int CMacroFlowEditorNcbiApp::Run(void)
{
    const CArgs& args = GetArgs();
    vector<const char*> vargs{args["s"].AsString().c_str(), args["d"].AsString().c_str()};

    if (!CNcbiApplication::HasLoadedConfig()) {
        string config_path;
        CDirEntry::SplitPath(GetProgramExecutablePath(), &config_path);
        config_path = CDirEntry::NormalizePath(config_path + "../etc/macroflow.ini");

        if (CFile(config_path).Exists()) {
            CNcbiApplication::LoadConfig(GetConfig(), &config_path);
        }
    }

    // now execute wxApp instance
    int argc = vargs.size();
    int retcode = wxEntry(argc, const_cast<char**>(vargs.data())); 
    return retcode;
}


/////////////////////////////////////////////////////////////////////////////
//  Exit - perform any needed cleanup

void CMacroFlowEditorNcbiApp::Exit(void)
{
    SetDiagStream(0);
}


////@begin XPM images

////@end XPM images


/*
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP_NO_MAIN( CMacroFlowEditorApp )
////@end implement app


/*
 * CMacroFlowEditorApp type definition
 */

IMPLEMENT_CLASS( CMacroFlowEditorApp, wxApp )


/*
 * CMacroFlowEditorApp event table definition
 */

BEGIN_EVENT_TABLE( CMacroFlowEditorApp, wxApp )

////@begin CMacroFlowEditorApp event table entries
////@end CMacroFlowEditorApp event table entries

END_EVENT_TABLE()


/*
 * Constructor for CMacroFlowEditorApp
 */

CMacroFlowEditorApp::CMacroFlowEditorApp()
{
    Init();
}


/*
 * Member initialisation
 */

void CMacroFlowEditorApp::Init()
{
////@begin CMacroFlowEditorApp member initialisation
////@end CMacroFlowEditorApp member initialisation
}

/*
 * Initialisation for CMacroFlowEditorApp
 */

bool CMacroFlowEditorApp::OnInit()
{    
////@begin CMacroFlowEditorApp initialisation
    // Remove the comment markers above and below this block
    // to make permanent changes to the code.

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
////@end CMacroFlowEditorApp initialisation

    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    // register the default Thread Pool engine for executing background UI tasks
    CThreadPoolEngine* engine = new CThreadPoolEngine(4, 1000);
    disp.RegisterEngine("ThreadPool", *engine);

    m_frame = new CMacroFlowEditor(NULL);
    m_frame->Show(true);
    SetTopWindow(m_frame);

    return true;
}


/*
 * Cleanup for CMacroFlowEditorApp
 */

int CMacroFlowEditorApp::OnExit()
{    
////@begin CMacroFlowEditorApp cleanup
    return wxApp::OnExit();
////@end CMacroFlowEditorApp cleanup
}


END_NCBI_SCOPE

/////////////////////////////////////////////////////////////////////////////
//  MAIN

int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    return ncbi::CMacroFlowEditorNcbiApp().AppMain(argc, argv);
}

