/*  $Id: flatfile_demo_app.cpp 34644 2016-02-01 21:00:56Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *    A simple wxWidgets GUI demo application.
 */


#include <ncbi_pch.hpp>

#include <gui/widgets/wx/wx_app.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/objutils/object_manager_engine.hpp>

#include "flat_file_demo_dlg.hpp"

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// Derive our application class from CwxNCBIApp and use it together with
/// standard CNCBIwxApplication.

class CDemoApp: public CwxNCBIApp
{
protected:
    // we are overriding this function to create our main frame window
    virtual bool    x_CreateGUI();
    virtual void    x_InitAppJobDispatcher();

    CFlatFileDemoDlg* m_Dlg;
};


// Declare wxWidgets application class
// this will allow CNCBIwxApplication to instantiate our class
DECLARE_APP(ncbi::CDemoApp)
IMPLEMENT_APP_NO_MAIN(ncbi::CDemoApp)
IMPLEMENT_WX_THEME_SUPPORT


bool CDemoApp::x_CreateGUI()
{
    m_Dlg = new CFlatFileDemoDlg(NULL);
    m_Dlg->Show();
    return true;
}

void CDemoApp::x_InitAppJobDispatcher()
{
    CwxNCBIApp::x_InitAppJobDispatcher();

    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance(); /// create

    CObjectManagerEngine* engine = new CObjectManagerEngine();
    disp.RegisterEngine("ObjManagerEngine", *engine);
}

END_NCBI_SCOPE;

/////////////////////////////////////////////////////////////////////////////
//  main entry point

int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // create standard NCBI wxWidgets application
    ncbi::CNCBIwxApplication ncbi_app("wxWidgets demo App");

    // Execute main application function
    return ncbi_app.AppMain(argc, argv);
}
