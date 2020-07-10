/*  $Id: demo_wx_empty_app.cpp 34634 2016-02-01 20:42:54Z katargir $
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
 *    demo_wx_app - empty wxWidgets/C++ Toolkit application
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/wx_app.hpp>
#include "hello_world_dlg.hpp"

USING_SCOPE(ncbi);

///////////////////////////////////////////////////////////////////////////////
/// Derive our application class from CwxNCBIApp and use it together with
/// standard CNCBIwxApplication.

class CDemoApp : public CwxNCBIApp
{
public:
    CDemoApp() {}
    virtual ~CDemoApp() {}

protected:
    // we are overriding this function to create our main frame window
    virtual bool    x_CreateGUI();

private:
    CDemoApp(const CDemoApp&);
    CDemoApp& operator=(const CDemoApp&);
};


// Declare wxWidgets application class
// this will allow CNCBIwxApplication to instantiate our class
DECLARE_APP(CDemoApp)
IMPLEMENT_APP_NO_MAIN(CDemoApp)
IMPLEMENT_WX_THEME_SUPPORT


bool CDemoApp::x_CreateGUI()
{
    CHelloWorldDlg dlg(NULL);
    dlg.ShowModal();
    return false;
}


/////////////////////////////////////////////////////////////////////////////
//  main entry point

int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // create standard NCBI wxWidgets application
    ncbi::CNCBIwxApplication ncbi_app("wxWidgets demo App");

    // Execute main application function
    return ncbi_app.AppMain(argc, argv);
}
