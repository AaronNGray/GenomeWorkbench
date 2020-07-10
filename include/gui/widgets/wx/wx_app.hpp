#ifndef GUI_WIDGETS_WX___WX_APP__HPP
#define GUI_WIDGETS_WX___WX_APP__HPP

/*  $Id: wx_app.hpp 39674 2017-10-25 21:52:16Z katargir $
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
 *      Defines two base classes CNCBIwxApplication  and CwxNCBIApp. The classes
 *      represent base application classes for C++ Tookit and wxWidgets
 *      respectively. They work together to create and execute a wxWidgets
 *      application within a NCBI Application.
 *      In most cases one will neeed to create a new application class derived  
 *      from CwxNCBIApp, override some of its virtual functions and register the
 *      application class using wxWidgets macros.
 */

#include <gui/gui.hpp>

#include <corelib/ncbiapp.hpp>

#include <wx/app.h>

BEGIN_NCBI_SCOPE

class CwxNCBIApp;

///////////////////////////////////////////////////////////////////////////////
/// CNCBIwxApplication - a base class for NCBI GUI applications using wxWidgets.
/// CNCBIwxApplication parses command line, performs basic initialization,
/// creates an instance of CwxNCBIApp (required by wxWidgets) and executes it.
/// CwxNCBIApp performs all initialization related to GUI.

class NCBI_GUIWIDGETS_WX_EXPORT CNCBIwxApplication : public CNcbiApplication
{
public:
    CNCBIwxApplication(const string& app_name);
    virtual ~CNCBIwxApplication();

    /// @name CNcbiApplication overridables
    /// @{
    virtual void Init(void);
    virtual int  Run(void);
    virtual void Exit(void);
    /// @}

protected:
    virtual void    x_SetupArgDescriptions();
    virtual void    x_Init_wxApplication();
    virtual int     x_Run();
};

///////////////////////////////////////////////////////////////////////////////
/// CwxNCBIApp - this class should be used in NCBI applications instead of wxApp
/// it provides polling support for CEventHandler and CAppJobDispatcher.
class NCBI_GUIWIDGETS_WX_EXPORT CwxNCBIApp: public wxApp
{
    DECLARE_CLASS(CwxNCBIApp)
    DECLARE_EVENT_TABLE()

public:
    CwxNCBIApp(bool use_job_disp = true);
    virtual ~CwxNCBIApp();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

    void OnIdle( wxIdleEvent& event );

protected:
    /// override this function to configure / register engines
    virtual void    x_InitAppJobDispatcher();
    /// override this to test/configure NCBI connection
    /// before access to NCBI network services
    virtual bool    x_TestNcbiConnection() { return true; }

    /// create main application window and initialize associated objects
    /// return true if successful
    virtual bool    x_CreateGUI();

    /// destroy application GUI objects
    virtual void    x_DestroyGUI();

    virtual void    x_LoadGuiRegistry();
    virtual void    x_SaveGuiRegistry();
    virtual wxString x_GetGuiRegistryPath();

    /// register wxWidgets image handlers
    virtual void    x_RegisterImageHandlers();

    /// called from OnIdle() and can be overridden in derived classes
    /// returns true if something was done
    virtual bool    x_OnIdle();

protected:
    // Application Job Dispatcher support
    bool    m_UseJobDisp;   // anable / disable
    // Thread Pool Engine parameters
    int     m_MaxThreads;   // max Threads
    int     m_MaxJobs;      // max Jobs

    bool m_RegistryLoaded;
};


END_NCBI_SCOPE


#endif  // GUI_WIDGETS_WX___WX_APP__HPP


