/*  $Id: gbench_app.hpp 43927 2019-09-20 18:14:56Z katargir $
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


#include <gui/widgets/wx/wx_app.hpp>

#include "gui.hpp"

#include <gui/framework/workbench_impl.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/core/ws_auto_saver.hpp>
#include <gui/utils/job_future.hpp>

#include "wakeup_thread.hpp"
#include "read_pipe_thread.hpp"

#include <wx/timer.h>

BEGIN_NCBI_SCOPE

class CPkgManager;

///////////////////////////////////////////////////////////////////////////////
/// CGBenchApplication
class CGBenchApplication : public CNCBIwxApplication, public IGBenchGUIApp
{
public:
    CGBenchApplication();

    /// @name CNcbiApplication overridables
    /// @{
    virtual void Init();
    /// @}

    // CNcbiApplication tweak
    virtual bool LoadConfig( 
        CNcbiRegistry& reg, const string* conf, CNcbiRegistry::TFlags flags 
    );
    // Our counterpart
    void SaveConfig();

public:
    // IGBenchGUIApp implementation
    virtual CPkgManager* GetPackageManager() { return m_PkgManager.get(); }

protected:
    virtual int x_Run();

    void x_SetupArgDescriptions();
    void x_HandleArgs( const CArgs& args );
    void x_GetArgs( const CArgs& args, vector<string>& fnames );

    void x_ReportVersion();

    void x_StartCrashMonitor();
    void x_StopCrashMonitor();

    void x_StartMonitor();
    void x_StopMonitor();

    bool x_RunningInstance( const CArgs& args );
    void x_InitPackages();
    void x_ConfigUIToolRegistry();
    void x_InitVDBCache();

    static wxString GetGuardFilepath();
    static wxString GetCrashFilepath();

    /// raise application window
    static void Raise();

    void x_TestConfig();

private:
    auto_ptr<CPkgManager> m_PkgManager;
};


///////////////////////////////////////////////////////////////////////////////
/// Derive our application class from CwxNCBIApp and use it with
/// CNCBIwxApplication.

class CGBenchApp: public CwxNCBIApp
{
public:
    friend class CViewOptionsPage;

    CGBenchApp();
    void SetGBenchGUIApp( IGBenchGUIApp* app ) { m_GUI.SetGBenchGUIApp( app ); }

    virtual wxAppTraits *CreateTraits();

    // Load project or workspace file provided by command line or named pipe
    vector<string> m_Args;

    void SetAutoSaveMode(CWorkspaceAutoSaver::ERestoreMode mode) { m_AutoSaveMode = mode; }
    void SetAppExtMode(const string& mode) { m_AppExtMode = mode; }

    string GetStartUpString() const { return m_StartUpStr; }
    void SetStartUpString( const string& aStr ){ m_StartUpStr = aStr; }

    virtual void x_LoadGuiRegistry();
    virtual void x_SaveGuiRegistry();
    virtual wxString x_GetGuiRegistryPath();

    /// Called to open dropped or double-clicked project files on mac
    virtual void MacOpenFile(const wxString &fileName);


public:
	/// Initialises the application
	virtual bool OnInit();

	/// Called on exit
	virtual int OnExit();

protected:
    // we are overriding this function to create our main frame window
    void x_InitExtMode();
    void x_ShutExtMode();

    virtual bool x_CreateGUI();
    virtual void x_DestroyGUI();

protected:
    virtual bool x_TestNcbiConnection();
    virtual void x_InitAppJobDispatcher();
    virtual bool x_OnIdle();

    void x_TestGuiRegistry();

protected:
    CGBenchGUI   m_GUI;
    auto_ptr<CWorkbench>          m_Workbench;

    auto_ptr<CWorkspaceAutoSaver>     m_WsAutoSaver;
    CWorkspaceAutoSaver::ERestoreMode m_AutoSaveMode;

    CRef<CWakeupThread>    m_WakeupThread;
    CRef<CReadPipeThread>  m_readPipeThread;

    void OnSaveRegistryTimer(wxTimerEvent& event);
    void OnMemCheckTimer(wxTimerEvent& event);

private:
    string m_StartUpStr;
    string m_AppExtMode;

    enum {
        kSaveRegistryTimer = 100,
        kMemCheckTimer,
        kPagerMessageTimer
    };

    wxTimer m_SaveRegistryTimer;
    wxTimer m_MemCheckTimer;

    std::unique_ptr<async_job> m_PagerMessageJob;

    DECLARE_EVENT_TABLE()
};


END_NCBI_SCOPE;
