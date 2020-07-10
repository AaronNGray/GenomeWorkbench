/*  $Id: gbench_app.cpp 44943 2020-04-22 20:24:55Z asztalos $
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
 * Authors:  Andrey Yazhuk, Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbiexec.hpp>
#include <corelib/ncbi_system.hpp>
#include <corelib/ncbi_process.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/ncbistr.hpp>
#include <corelib/ncbicfg.h>
#include <corelib/request_ctx.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbireg.hpp>

#include "gbench_app.hpp"
#include <gui/objects/gbench_version.hpp>
#include <gui/widgets/feedback/user_registry.hpp>
#include "init.hpp"
#include "open_files_task.hpp"
#include "net_conn_problem_dlg.hpp"
#include "testing_ncbi_conn_dlg.hpp"

#include <connect/ncbi_namedpipe.hpp>

#include <gui/core/project_service.hpp>
#include <gui/core/ui_data_source_service.hpp>
#include <gui/core/commands.hpp>

#include <gui/framework/pkg_manager.hpp>
#include <gui/framework/app_mode_extension.hpp>
#include <gui/framework/app_job_task.hpp>
#include <gui/utils/extension_impl.hpp>

#include <gui/widgets/wx/wx_app.hpp>
#include <gui/widgets/wx/log_gbench.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/init.hpp>
#include <gui/core/init.hpp>
#include <gui/widgets/aln_score/init_w_aln_score.hpp>

#include <gui/widgets/wx/ui_tool_registry.hpp>
#include <gui/objutils/usage_report_job.hpp>
#include <gui/objutils/seqfetch_queue.hpp>
#include <gui/objutils/label.hpp>

#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/thread_pool_engine.hpp>
#include <gui/utils/scheduler_engine.hpp>
#include <gui/objutils/object_manager_engine.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/utils/conn_test_thread.hpp>
#include <connect/ncbi_conn_test.hpp>
#include <connect/ncbi_http_session.hpp>
#include <gui/objutils/taxon_cache.hpp>

#include <gui/objects/gbench_svc_cli.hpp>
#include <gui/objects/GBenchInitRequest.hpp>
#include <gui/objects/GBenchInitReply.hpp>
#include <gui/objects/GBenchVersionRequest.hpp>
#include <gui/objects/GBenchVersionReply.hpp>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqalign/Score.hpp>

#include <util/line_reader.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/utils/event_translator.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/widgets/wx/wnd_layout_registry.hpp>

#include <gui/osx_api.h>

#include <wx/apptrait.h>
#include <wx/menu.h>
#include <wx/filefn.h>
#include <wx/utils.h>
#include <wx/msgdlg.h>
#include <wx/fs_inet.h>
#include <wx/socket.h>

#include <klib/sra-release-version.h>
#include <sra/readers/sra/vdbread.hpp>

#include <dbapi/driver/dbapi_svc_mapper.hpp>

#include "app_updater_task.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

// Declare wxWidgets application class
// this will allow CNCBIwxApplication to instantiate our class
DECLARE_APP(ncbi::CGBenchApp)
IMPLEMENT_APP_NO_MAIN(ncbi::CGBenchApp)
IMPLEMENT_WX_THEME_SUPPORT

class CSendAppStartJob : public CObject, public IAppJob
{
public:
    virtual EJobState   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress() { return CConstIRef<IAppJobProgress>(); }
    virtual CRef<CObject>   GetResult() { return CRef<CObject>(); }
    virtual CConstIRef<IAppJobError>    GetError() { return CConstIRef<IAppJobError>(); }
    virtual string    GetDescr() const { return "Sending start notification"; }
    virtual void RequestCancel() {  }
    virtual bool IsCanceled() const { return false; }
};

IAppJob::EJobState CSendAppStartJob::Run()
{
    CGBenchVersionInfo cur_version;
    GetGBenchVersionInfo(cur_version);

    CGBenchService().SendAppStart(cur_version);

    return eCompleted;
}

///////////////////////////////////////////////////////////////////////////////
/// CGBenchApplication

#ifdef NCBI_OS_MSWIN
#include <Winhttp.h>

void GetWindowsProxyConfiguration(CNcbiRegistry& reg)
{
    if (!reg.Get(DEF_CONN_REG_SECTION, REG_CONN_HTTP_PROXY_HOST).empty())
        return;

    typedef BOOL (WINAPI *PWHGPCCU) (WINHTTP_CURRENT_USER_IE_PROXY_CONFIG*);

    HINSTANCE hWinhttpLib(nullptr);
    do {
        hWinhttpLib = LoadLibrary(TEXT("Winhttp.dll"));

        PWHGPCCU pWHGPCCU = (PWHGPCCU)GetProcAddress(hWinhttpLib, "WinHttpGetIEProxyConfigForCurrentUser");
        if (nullptr == pWHGPCCU)
            break;

        WINHTTP_CURRENT_USER_IE_PROXY_CONFIG proxyInfo;
        if (!pWHGPCCU(&proxyInfo))
            break;
        
        if (nullptr != proxyInfo.lpszProxy) {
            // One or more of the following strings separated by semicolons or whitespace:
            // ([<scheme>=][<scheme>"://"]<server>[":"<port>])
            wxString proxy(proxyInfo.lpszProxy);
            proxy = proxy.Trim(false);
            proxy = proxy.Trim(true);
            int pos = -1;
            const vector<string> schemas{ "https=", "http=" };
            for (const auto& scheme : schemas) {
                pos = proxy.Find(scheme);
                if (-1 == pos)
                    continue;

                proxy = proxy.Mid(pos + scheme.length());
                break;
            }
            const string separators{ "; \t\n" };
            for (const char separator : separators) {
                pos = proxy.Find(separator);
                if (-1 != pos) {
                    proxy = proxy.Left(pos);
                }
            }
            pos = proxy.Find(':');
            if (-1 == pos) {
                reg.Set(DEF_CONN_REG_SECTION, REG_CONN_HTTP_PROXY_HOST, proxy.ToStdString());
            }
            else {
                wxString proxy_port = proxy.Right(proxy.Length() - pos - 1);
                proxy = proxy.Left(pos);
                reg.Set(DEF_CONN_REG_SECTION, REG_CONN_HTTP_PROXY_HOST, proxy.ToStdString());
                reg.Set(DEF_CONN_REG_SECTION, REG_CONN_HTTP_PROXY_PORT, proxy_port.ToStdString());
            }
            
            GlobalFree(proxyInfo.lpszProxy);
        }

        if (nullptr != proxyInfo.lpszAutoConfigUrl)
            GlobalFree(proxyInfo.lpszAutoConfigUrl);

        if (nullptr != proxyInfo.lpszProxyBypass)
            GlobalFree(proxyInfo.lpszProxyBypass);

    } while (false);

    if (hWinhttpLib)
        FreeLibrary(hWinhttpLib);
}
#endif

CGBenchApplication::CGBenchApplication()
:   CNCBIwxApplication("NCBI Genome Workbench")
{
    SetDiagFilter(eDiagFilter_Trace, "!/corelib !/connect !/objmgr");
    SetDiagPostFlag(eDPF_DateTime);

    UnsetDiagTraceFlag(eDPF_LongFilename);
}

void CGBenchApplication::Init()
{
#   ifdef NCBI_OS_DARWIN
    // First, add run path for NCBI application on Mac

    static string s_FrameworkPath("$ORIGIN/../Frameworks");

    NCBI_SetRunpath( s_FrameworkPath.c_str() );

#   endif

    // Set up permanent GBench Session ID before any possible connection to NCBI
    //
    string session_id;

    CNcbiRegistry& reg = GetConfig();
    if( reg.HasEntry( "GBENCH_CONN", "NCBI_SID" ) ){
        session_id = reg.Get( "GBENCH_CONN", "NCBI_SID" );
        GetDiagContext().GetRequestContext().SetSessionID( session_id );

    } else {
        session_id = GetDiagContext().GetRequestContext().SetSessionID();
        reg.Set( "GBENCH_CONN", "NCBI_SID", session_id, IRegistry::fPersistent );
        SaveConfig();
    }
    GetDiagContext().SetDefaultSessionID( session_id );

    if (!reg.HasEntry( "objmgr", "BLOB_CACHE"))
        reg.Set("objmgr", "BLOB_CACHE", "0");

    // Set default BAM loader's align tags
    if (!reg.HasEntry("bam_loader", "include_align_tags")) 
        reg.Set("bam_loader", "include_align_tags", "HP,SB,MD");

    CNCBIwxApplication::Init();

    const CArgs& args = GetArgs();
    if (args["inst"].HasValue())
        CSysPath::SetInst(args["inst"].AsString());

    if (args["packages"].HasValue()) {
        string packages = args["packages"].AsString();
        list<string> toks;
        NStr::Split(packages, ",", toks, NStr::fSplit_Tokenize);
        if (!toks.empty()) {
            if (!m_PkgManager.get())
                m_PkgManager.reset(new CPkgManager());

            ITERATE (list<string>, it, toks)
                m_PkgManager->AddDefaultPkg(*it);
        }
    }

    if (args["autosave"].HasValue()) {
        string mode = args["autosave"].AsString();
        if (mode == "restore")
            wxGetApp().SetAutoSaveMode(CWorkspaceAutoSaver::eRestore);
        else if (mode == "ignore")
            wxGetApp().SetAutoSaveMode(CWorkspaceAutoSaver::eIgnore);
    }

    if (args["app_mode"].HasValue())
        wxGetApp().SetAppExtMode(args["app_mode"].AsString());

    // We need here our GUI registry load
    // for our services initialization

    CGBenchApp* gbench_wx_app = dynamic_cast<CGBenchApp*>(wxTheApp);
    if( gbench_wx_app ){
        /// insure that the user home path exists
        wxString path = CSysPath::ResolvePath(wxT("<home>"));
        if( !wxFileName::DirExists( path ) ){
            wxFileName::Mkdir(path, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
        }

        // create diagnostic handler
        CwxLogDiagHandler::GetInstance();

        x_ReportVersion();

        gbench_wx_app->x_LoadGuiRegistry();
    } else {
        // something is wrong with our application
        _ASSERT(false);
    }
    wxFileSystem::AddHandler(new wxInternetFSHandler);
    CUsageReportJob::LoadSettings();
}


// setup command line argument descriptions
void CGBenchApplication::x_SetupArgDescriptions()
{
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    string basename = GetArguments().GetProgramBasename();
    arg_desc->SetUsageContext(basename, GetProgramDisplayName());

    arg_desc->AddFlag("v", "Version");
// -t parameter is not used anymore
// removing it breaks clicking on file (project, workspace, etc.) functionality.
// because the parameter is used in file associations in gbench installer
// See GB-5768.
    arg_desc->AddDefaultKey("t", "File_Type", "Type of optional data file",
        CArgDescriptions::eString, "auto");
    arg_desc->SetConstraint("t", &(*new CArgAllow_Strings,
        "auto", "project", "message", "asntext",
        "asnbin", "xml", "fasta", "textalign", "newick"));
    arg_desc->AddExtra(0, 100, "Items to open", CArgDescriptions::eString);

    arg_desc->AddOptionalKey("inst", "string",
                             "application instance name",
                             CArgDescriptions::eString);

    arg_desc->AddOptionalKey("packages", "string",
                             "Packages to be loaded",
                             CArgDescriptions::eString);

    arg_desc->AddOptionalKey("autosave", "restore_mode",
                             "Autosave restore mode",
                              CArgDescriptions::eString);

    arg_desc->SetConstraint("autosave", &(*new CArgAllow_Strings, "ask", "restore", "ignore"));

    arg_desc->AddOptionalKey("app_mode", "app_mode",
        "Application mode",
        CArgDescriptions::eString);

    arg_desc->AddFlag("disable-monitor", "bool");
    
#ifdef __WXOSX_COCOA__
    arg_desc->AddOptionalKey("NSDocumentRevisionsDebugMode", "XCode_debug",
                         "XCode debug option to be ignored",
                          CArgDescriptions::eString);
#endif

    SetupArgDescriptions(arg_desc.release());
}

int CGBenchApplication::x_Run()
{
    wxGetApp().SetGBenchGUIApp(this);

    const CArgs& args = GetArgs();

    if (args["v"]) {    // version checking
        // version is always reported in constructor.
        return 0;
    }

    x_StartCrashMonitor();

    /// start up our monitor process
    x_StartMonitor();

    int retcode = -1;



    /// if handed a file on command line, try sending it to another
    /// application instance via the named pipe if the command line
    /// switches and registry say to do so
    if (x_RunningInstance(args)) {
        retcode = 0;
    } else {
        init_gui_app_gbench();
        x_HandleArgs(args);

        x_TestConfig();

        // Report SRA version
        {{
            SraReleaseVersion sraVersion;
            SraReleaseVersionGet(&sraVersion);

            string releaseType;
            switch (sraVersion.type) {
            case SraReleaseVersion::eSraReleaseVersionTypeAlpha:
                releaseType = " (alpha)";
                break;
            case SraReleaseVersion::eSraReleaseVersionTypeBeta:
                releaseType = " (beta)";
                break;
            case SraReleaseVersion::eSraReleaseVersionTypeRC:
                releaseType = " (release candidate)";
                break;
            case SraReleaseVersion::eSraReleaseVersionTypeFinal:
                releaseType = " (final)";
                break;
            default:
                releaseType = " (unknown)";
                break;
            }

            LOG_POST(Info << "SRA version "
                          << VersionGetMajor(sraVersion.version) << "."
                          << VersionGetMinor(sraVersion.version) << "."
                          << VersionGetRelease(sraVersion.version)
                          << " revision " << sraVersion.revision << releaseType);
        }}


        // initialize SRA config access  (VDB_CONFIG)
        //
        {{
        CNcbiEnvironment& app_env = this->SetEnvironment();
        const string k_vdb_config("VDB_CONFIG");
        const string& vdb_config_path = app_env.Get(k_vdb_config);

        if (vdb_config_path.empty())
        {
            wxString etc_path = CSysPath::ResolvePath( wxT("<std>/etc") );
            string etc_path_s(etc_path.ToUTF8());

            // transform file path to VDB friendly (hack)
            //
            #ifdef NCBI_OS_MSWIN
                NStr::ReplaceInPlace(etc_path_s, "\\", "/");
                NStr::ReplaceInPlace(etc_path_s, ":", "");
                etc_path_s.insert(0, "/");
            #endif

            LOG_POST(Info << "Setting VDB_CONFIG to " << etc_path_s);
            app_env.Set(k_vdb_config, etc_path_s);
        } else {
            LOG_POST(Info << "VDB_CONFIG=" << vdb_config_path);
        }

        }} // VDB_CONFIG


        init_gui_objutils();
        init_gui_core();
        init_w_aln_score();

        // Init cache for GI labels
        CLabelDescriptionCache::InitCache();

        wxString taxCachePath = CSysPath::ResolvePath( wxT("<home>/tax_name_cache.txt"));
        CTaxonCache::InitStorage(taxCachePath.ToUTF8());
        x_InitVDBCache();

        // Enable Database load balancer handlers (for use in RADAR and other in-house plugins)
        DBLB_INSTALL_DEFAULT();

        // load new-style plugins
        x_InitPackages();

        x_ConfigUIToolRegistry(); // TODO

        // set global hooks  to save memory for DenSeg alignments
        //
        //CDense_seg::SetGlobalReserveHooks();

        // by Eugene Vasilchenko
        // This one is not safe because it uses shared Object-id to represent equal
        // score.id objects in different Seq-align.
        // The benefit is the smaller memory used, but because the Object-id is shared
        // changing score.id field in one Seq-align will affect others. You can use the
        // hook if you are sure to not to change the score.id, or at least do it knowingly.
        CReadSharedScoreIdHook::SetGlobalHook();


        retcode = CNCBIwxApplication::x_Run();

        // we intentionally do this twice - once while the GUI is still
        // visible to allow the user to see that the app is still running
        LOG_POST(Info << "Clearing sequence prefetch queue");
        CSeqFetchQueue::GetInstance().Clear();

        // TODO - we may want to make a service out of CUIToolRegistry
        CUIToolRegistry::ShutDown();
    }

    SaveConfig();

    /// shut down the monitoring app
    /// this is explicitly *NOT GUARDED* - an exception thrown by the app will
    /// cause an unnatural shut-down. we do not do this if there was a fatal
    /// error in the message loop that was caught as an exception
    if (retcode == 0) {
        x_StopMonitor();
    }

    x_StopCrashMonitor();

    CLabelDescriptionCache::StopCache();

    return retcode;
}


void CGBenchApplication::x_ReportVersion()
{
    size_t ver_major = 0;
    size_t ver_minor = 0;
    size_t ver_patch_level = 0;
    CTime build_date;
    GetGBenchVersionInfo( ver_major, ver_minor, ver_patch_level, build_date );

    size_t ver_bits = sizeof(void*)*8;

#ifdef _DEBUG
    static const char* reldeb = "  built in debug mode on ";
#else
    static const char* reldeb = "  released on ";
#endif

    string osxApi
#ifdef __WXOSX_COCOA__
#ifdef USE_METAL
    = "-Metal"
#else
    = "-OpenGL"
#endif
#endif
    ;

    LOG_POST(Info << "NCBI Genome Workbench " << ver_bits << "bit v"
        << ver_major << "." << ver_minor << "." << ver_patch_level
        << osxApi << reldeb << build_date.AsString()
    );
}


// TODO
void CGBenchApplication::x_TestConfig()
{
    // TODO
    // CGuiRegistry& registry = CGuiRegistry::GetInstance();
    // CRegistryReadView app_reg = registry.GetReadView("GBENCH.Application");

    if( HasLoadedConfig() ){
        LOG_POST( Info << "Config loaded from " << GetConfigPath() );
        // dump some diagnostic information about the contents of the registry

        list<string> sections;
        CNcbiRegistry& reg = GetConfig();
        reg.EnumerateSections(&sections);

        if( sections.size() ){
            LOG_POST( Info << "Config file contains:" );
            ITERATE( list<string>, iter, sections ){
                list<string> entries;
                reg.EnumerateEntries( *iter, &entries );
                ITERATE( list<string>, it, entries ){
                    if(
                        (*it).find( "_PROXY_USER" ) == string::npos
                        && (*it).find( "_PROXY_PASS" ) == string::npos
                    ){
                        const string& s = reg.Get(*iter, *it);
                        LOG_POST(Info << "  [" << *iter << "] " << *it << " = " << s );
                    } else { // hide proxy details including password
                        //const string& s = reg.Get(*iter, *it);
                        LOG_POST(Info << "  [" << *iter << "] " << *it << " = xxxx" );
                    }
                }
            }
        } else {
            LOG_POST(Error << "Config file is empty");
        }
    } else {
        LOG_POST(Error << "No config file found");
    }
}


static CNamedPipeServer s_NamedPipeServer;

static string s_GetPipeName()
{
    char pipename[256];
    strcpy(pipename, "gbench_pipe-");
    int len = strlen(pipename);
    CORE_GetUsername(pipename + len, sizeof(pipename) - len - 1);

    string inst = CSysPath::GetInst();
    if (!inst.empty()) {
        strcat(pipename, "-");
        strcat(pipename, inst.c_str());
    }
    return pipename;
}


bool CGBenchApplication::x_RunningInstance(const CArgs& args)
{
    string pipeName = s_GetPipeName();

    CNamedPipeClient cli;
    STimeout timeout = { 5, 0 };
    cli.SetTimeout(eIO_Open, &timeout);

    if (cli.Open(pipeName) != eIO_Success) {
        s_NamedPipeServer.Create(pipeName, &timeout, 10*1024);
        return false;
    }

    vector<string> fnames;
    x_GetArgs(args, fnames);
    string msg = "gbench:open";

    ITERATE(vector<string>, it, fnames)
        msg += '\0' + *it;

    size_t n_written;
    size_t total_written = 0;
    EIO_Status status;

    do {
        status = cli.Write(msg.data() + total_written, msg.size() - total_written, &n_written);
        total_written += n_written;
    } while (status == eIO_Success && total_written < msg.size());

    return true;
}

void CGBenchApplication::x_HandleArgs(const CArgs& args)
{
    x_GetArgs(args, wxGetApp().m_Args);
}

void CGBenchApplication::x_GetArgs(const CArgs& args, vector<string>& fnames)
{
    size_t items = args.GetNExtra();

    for (size_t i = 0;  i < items;  ++i) {
        string arg_name = "#" + NStr::SizetToString(i + 1);
        string arg = args[arg_name].AsString();
#if defined(WIN32)
        wxString filename = wxString::FromUTF8(arg.c_str());
#else
        wxString filename(arg.c_str(), wxConvLibc);
#endif
        wxFileName fname(filename);
        if (!fname.IsAbsolute())
            fname.MakeAbsolute();
        fnames.push_back(string(fname.GetFullPath().ToUTF8()));
    }
}

static const wxChar*  kDefUserScoreDir = wxT("<home>/align_scores");
static const wxChar*  kDefScoreDir = wxT("<std>/etc/align_scores");

static const char*  kUIToolsRegKey = "GBENCH.UITools";

void CGBenchApplication::x_ConfigUIToolRegistry()
{
    list<wxString> dirs;
    dirs.push_back(kDefUserScoreDir);
    dirs.push_back(kDefScoreDir);

    CUIToolRegistry* reg = CUIToolRegistry::GetInstance();
    reg->LoadTemplateToolsInfo(dirs);

    reg->SetRegistryPath(kUIToolsRegKey);
    reg->LoadSettings();
}


static const char* kVDBRegistryPath = "GBENCH.Services.VDB";
static const char* kVDBCachePathTag = "CacheDir";
static const char* kVDBCacheTTLTag = "CacheTTL";

void CGBenchApplication::x_InitVDBCache()
{
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(kVDBRegistryPath);
    string cache_path = view.GetString(kVDBCachePathTag, "<home>/cache64/vdb");
    int cache_days = view.GetInt(kVDBCacheTTLTag, 5);
    wxFileName path = wxFileName::DirName(CSysPath::ResolvePath(cache_path));
    if (!path.DirExists()) { // Create the cache path, if it doesn't exist
        path.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }

    {
        CVDBMgr mgr;
        string vdb_cache_root;
        try {
            // Throws an excpetion if the cache root is not set
            vdb_cache_root = mgr.GetCacheRoot();
        }
        catch (const CException&) {

        }
        if (vdb_cache_root != string(path.GetPath().ToUTF8())) {
            try {
                mgr.SetCacheRoot(string(path.GetPath().ToUTF8()));
                mgr.CommitConfig();
            }
            catch (const CException& err) {
                LOG_POST(Error << "VDB failed to set cache root path: " << err);
            }
        }
    }
    try {
        CVDBMgr mgr;
        CStopWatch sw;
        sw.Start();
        mgr.DeleteCacheOlderThan(cache_days);
        LOG_POST(Info << "VDB cache deletion took " << sw.Elapsed() << " seconds");
    }
    catch (const CException& err) {
        LOG_POST(Error << "VDB cache deletion failed: " << err);
    }
}


wxString CGBenchApplication::GetGuardFilepath()
{
    static wxString guard_path;
    if( guard_path.IsEmpty() ){
        guard_path.Printf(wxT("<home>/gbench-pid.%u"), (int)CCurrentProcess::GetPid() );
        guard_path = CSysPath::ResolvePath( guard_path );
    }

    return guard_path;
}


void CGBenchApplication::x_StartMonitor()
{
    // enable monitor only in release builds
#ifndef NODEF //_DEBUG
    /// establish our monitoring process; this is only done for release builds!
    try {
        /// write the PID to a guard file
        wxString guard_path = GetGuardFilepath();
        {{
            CNcbiOfstream ostr( guard_path.fn_str() );
            ostr << CTime( CTime::eCurrent ).AsString();
        }}

        if (GetArgs()["disable-monitor"])
            return;

        string dir;
        CDirEntry::SplitPath(GetProgramExecutablePath(eFollowLinks), &dir);
        string monitor_path(dir + "gbench_monitor");

        string pid_str = NStr::NumericToString( CCurrentProcess::GetPid() );

        string instVal = CSysPath::GetInst();
        string logMsg = monitor_path;
        if (!instVal.empty())
            logMsg += " -inst " + instVal;

        logMsg += " " + pid_str + " " + guard_path;

        LOG_POST(Info << "Starting process monitor: " << logMsg);

        // Use eDetach mode on Unix to let monitor continue to work on gbench crash
        // Use eNoWait mode on Windows to hide console window
#       ifdef NCBI_OS_MSWIN
        CExec::EMode mode = CExec::eNoWait;
#       else
        CExec::EMode mode = CExec::eDetach;
#       endif

        //wxWritableCharBuffer p = guard_path.char_str();

        CExec::CResult res = instVal.empty() ?
            CExec::RunSilent(mode, monitor_path.c_str(),
                             pid_str.c_str(), guard_path.ToUTF8().data(), NULL) :
            CExec::RunSilent(mode, monitor_path.c_str(),
                             "-inst", instVal.c_str(),
                             pid_str.c_str(), guard_path.ToUTF8().data(), NULL);

        if( ! res.GetProcessHandle() ){
            ERR_POST("Failed to start process monitor - cannot get process handle");
        }
    } catch( CException& e ){
        ERR_POST( "Failed to start process monitor - " << e.GetMsg() );
    }
#endif
}


/// stops monitoring process
void CGBenchApplication::x_StopMonitor()
{
    // enable monitor only in release builds
#ifndef _DEBUG
    LOG_POST( Info << "Shutting down process monitor..." );

    /// delete our PID file - this is a sign of successful completion
    ::wxRemoveFile( GetGuardFilepath() );
    LOG_POST( Info << "Finished shutting down process monitor" );
#endif
}

void CGBenchApplication::x_StartCrashMonitor()
{
    CGuiRegistry& registry = CGuiRegistry::GetInstance();
#ifndef _DEBUG
    try {
        wxString path = GetCrashFilepath();
        if( wxFileName::FileExists( path ) ){
            registry.Set( "GBENCH.Application.CrashDetected", true );
            ::wxRemoveFile( path );

        } else {
            registry.Set( "GBENCH.Application.CrashDetected", false );
        }

        CNcbiOfstream ostr( path.fn_str() );
        ostr << CTime( CTime::eCurrent ).AsString();
    }
    catch( CException& e ){
        ERR_POST( "Failed to create crash monitor - " << e.GetMsg() );
    }
#else
    registry.Set( "GBENCH.Application.CrashDetected", false );
#endif

}

void CGBenchApplication::x_StopCrashMonitor()
{
#ifndef _DEBUG
    wxLogNull no_logger;
    ::wxRemoveFile( GetCrashFilepath() );
#endif
}

wxString CGBenchApplication::GetCrashFilepath()
{
    static wxString s_crash_path;
    if( s_crash_path.IsEmpty() ){
        s_crash_path = CSysPath::ResolvePath( wxT("<home>/gbench-crash") );
    }

    return s_crash_path;
}

bool CGBenchApplication::LoadConfig(
    CNcbiRegistry& reg, const string* conf, CNcbiRegistry::TFlags flags
){
    /// load the standard NCBI .ini config file
    
    bool config_loaded{ false };

    string config_path;
    if( !conf || conf->empty() ){
        // User-specific settings
        wxString wx_config_path = CSysPath::ResolvePathExisting( wxT("<home>/gbench_user.ini") );
        if ( !wx_config_path.empty() ){
            config_path = wx_config_path.ToUTF8().data();
            conf = &config_path;
        }

        // GBench default settings
        wxString wx_sys_config_path = CSysPath::ResolvePathExisting(wxT("<std>/etc/gbench.ini"));
        if (!wx_sys_config_path.empty()) {
            string sys_config = wx_sys_config_path.ToUTF8().data();
            LOG_POST(Info << "Loading default config file from " << sys_config);
            if (CNcbiApplication::LoadConfig(reg, &sys_config, CNcbiRegistry::fTransient | CNcbiRegistry::fNoOverride)) {
                config_loaded = true;
            }
            else
            {
                LOG_POST(Info << "Unable to load default Gbench config file");
            }
        }
    }

    if (nullptr != conf) {
        LOG_POST(Info << "Loading config file from " << *conf);
        // Let the base class load the config file.
        // If we couldn't find it above, it will try and find it, and warn otherwise.
        if (CNcbiApplication::LoadConfig(reg, conf, flags)) {
            config_loaded = true;
        }
    }


    if (config_loaded) {
#ifdef NCBI_OS_MSWIN
        GetWindowsProxyConfiguration(reg);
#endif
        // Guard for nonnumeric value of proxy port 
        // which causes uncaught exception

        bool nonNumericProxyPort = true;
        try {
            reg.GetInt(DEF_CONN_REG_SECTION, REG_CONN_HTTP_PROXY_PORT, 0);
            nonNumericProxyPort = false;
        }
        catch (const CStringException&) {
        }

        if (nonNumericProxyPort) {
            reg.Set(DEF_CONN_REG_SECTION, REG_CONN_HTTP_PROXY_PORT, NcbiEmptyString, IRegistry::fPersistent);
        }

        return true;
    }
    else 
    {
        LOG_POST(Info << "No NCBI config file found" );

        return false;
    }
}


void CGBenchApplication::SaveConfig()
{
    /// save the basic INI file registry
    if( GetConfig().Modified() ){
        wxString home_path = CSysPath::ResolvePath( wxT("<home>") );
        if (!wxFileName::DirExists( home_path ) &&
            !wxFileName::Mkdir(home_path, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
            return;

        wxFileName fname( home_path, wxT("gbench_user.ini") );
        CNcbiOfstream ofstr( fname.GetFullPath().fn_str(), ios::out|ios::binary );
        GetConfig().Write( ofstr );
    }
}


static const char*  kUIPkgManager = "GBENCH.UIPkgManager";

void CGBenchApplication::x_InitPackages()
{
    if (!m_PkgManager.get())
        m_PkgManager.reset(new CPkgManager());

    size_t ver_major = 0;
    size_t ver_minor = 0;
    size_t ver_patch_level = 0;
    CTime build_date;
    GetGBenchVersionInfo(ver_major, ver_minor, ver_patch_level, build_date);
    m_PkgManager->SetAppPkg("GBench", ver_major, ver_minor, ver_patch_level);

    m_PkgManager->AddDefaultPkg("sequence");
    m_PkgManager->AddDefaultPkg("alignment");
    m_PkgManager->AddDefaultPkg("ncbi internal");
    m_PkgManager->AddDefaultPkg("snp");

    m_PkgManager->SetRegistryPath(kUIPkgManager);

    wxString packages_path =
        CSysPath::ResolvePath(wxT("<std>/packages"));
    if ( !packages_path.empty() ) {
        m_PkgManager->ScanPackages(packages_path);
    }

    m_PkgManager->ValidatePackages();
    m_PkgManager->LoadPackages();
}

///////////////////////////////////////////////////////////////////////////////
/// CGBenchApp

///////////////////////////////////////////////////////////////////////////////
/// CNCBI_GUIAppTraits

// create Traits class in order to install our own wxLog implementation
class CGBenchAppTraits : public wxGUIAppTraits
{
    virtual wxLog *CreateLogTarget()
    {
        return CwxLogDiagHandler::GetInstance();
    }
#if wxUSE_INTL
    // called during wxApp initialization to set the locale to correspond to
    // the user default (i.e. system locale under Windows, LC_ALL under Unix)
    virtual void SetLocale() {}
#endif // wxUSE_INTL
};


BEGIN_EVENT_TABLE(CGBenchApp, CwxNCBIApp)
    EVT_TIMER(kSaveRegistryTimer, CGBenchApp::OnSaveRegistryTimer)
    EVT_TIMER(kMemCheckTimer, CGBenchApp::OnMemCheckTimer)
END_EVENT_TABLE()


CGBenchApp::CGBenchApp()
    : m_AutoSaveMode(CWorkspaceAutoSaver::eAskUser)
    , m_SaveRegistryTimer(this, kSaveRegistryTimer)
    , m_MemCheckTimer(this, kMemCheckTimer)
{
}

void CGBenchApp::OnSaveRegistryTimer(wxTimerEvent& WXUNUSED(event))
{
    if (m_Workbench.get()) {
        m_GUI.SaveDefaultWindowLayout();
        x_SaveGuiRegistry();
    }
}

void CGBenchApp::OnMemCheckTimer(wxTimerEvent& WXUNUSED(event))
{
    ReportMemoryUsage();
}

wxAppTraits *CGBenchApp::CreateTraits()
{
    return new CGBenchAppTraits();
}


void CGBenchApplication::Raise()
{
    CGBenchApp& app = wxGetApp();
    wxWindow* win = app.GetTopWindow();
    if(win) {
        win->Raise();
    }
}

bool CGBenchApp::x_TestNcbiConnection()
{
    // Performing short connection test.
    CRef<CConnTestThread> thread(new CConnTestThread(5));
    thread->Run();
    thread->Join();
    if (thread->GoodConnection()) {
        LOG_POST(Info << "Short connection test SUCCEEDED");
        return true;
    }

    LOG_POST(Error << "Short connection test FAILED");

    string descr = "Testing NCBI connection";
    thread.Reset(new CConnTestThread(30));
    thread->Run();

    if (CConnTest::IsNcbiInhouseClient())
        return true;

    int res = wxID_OK;

    if (!thread->Finished()) {
        CTestingNcbiConnDlg dlg;
        dlg.SetConnThread(thread.GetPointer());
        dlg.Create(NULL);
        res = dlg.ShowModal();
    }


    if (res != wxID_OK) {
        LOG_POST(Error << descr << ": CANCELLED");
        return true;
    }

    thread->Join();

    string connInfo = thread->GetConnInfo();
    if (thread->GoodConnection()) {
        LOG_POST(Info << descr << ": SUCCEEDED");
    }
    else {
        LOG_POST(Error << descr << ": FAILED");
        LOG_POST(Error << connInfo);

        CNetConnProblemDlg dlg;
        dlg.SetDescr(ToWxString(connInfo));
        dlg.Create(NULL);
        dlg.ShowModal();
    }

    return true;
}

void CGBenchApp::x_InitAppJobDispatcher()
{
    m_MaxThreads = 3;
    m_MaxJobs = 1000;

    /// this will register Thread Pool engine
    CwxNCBIApp::x_InitAppJobDispatcher();

    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    disp.RegisterEngine("ObjManagerEngine", *new CObjectManagerEngine(6));
    disp.RegisterEngine("ObjManagerEngineAsyncCall", *new CObjectManagerEngine(2));
    disp.RegisterEngine("Scheduler", *new CSchedulerEngine());
}

bool CGBenchApp::OnInit()
{
    CwxLogDiagHandler::GetInstance();

    bool ok = CwxNCBIApp::OnInit();

    wxSocketBase::Initialize();

    CAppTaskService* task_service = m_Workbench->GetAppTaskService();

    // Send app-start message
    CRef<CSendAppStartJob> job(new CSendAppStartJob());
    CRef<CAppJobTask> task(new CAppJobTask(*job, false, "", 0));
    task_service->AddTask(*task);

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    if (!gui_reg.GetBool("GBENCH.Application.AutoUpdates.Disabled", false)) {
        CAppUpdaterTask* vcheck_task = new CAppUpdaterTask();
        task_service->AddTask(*vcheck_task);
    }
    else {
        LOG_POST(Warning << "Automatic updates are disabled!");
    }

    if (m_WsAutoSaver.get()) m_WsAutoSaver->PostSave();

    // interval in minutes, default - 30 min
    int saveRegUInterval = 60 * 1000 * gui_reg.GetInt("GBENCH.Application.SaveRegistryInterval", 30);
    if (saveRegUInterval >= 0)
        m_SaveRegistryTimer.Start(saveRegUInterval);

    m_MemCheckTimer.Start(15 * 60 * 1000);

    return ok;
}

int CGBenchApp::OnExit()
{
    // Send app-end message
    try {
        CGBenchVersionInfo cur_version;
        GetGBenchVersionInfo(cur_version);
        CGBenchService().SendAppFinish(cur_version);
    }
    NCBI_CATCH("CGBenchService().SendAppFinish()");

    return CwxNCBIApp::OnExit();
}

void CGBenchApp::x_InitExtMode()
{
    if (m_AppExtMode.empty())
        return;

    vector<IAppModeExtension*> appModes;
    GetExtensionAsInterface(EXT_POINT__APP_MODE_EXTENSION, appModes);
    for (auto ext : appModes) {
        if (ext->GetModeName() == m_AppExtMode) {
            ext->SetWorkbench(m_Workbench.get());
            return;
        }
    }
    LOG_POST(Error << "Application mode extension: \"" << m_AppExtMode << "\" not found.");
}

void CGBenchApp::x_ShutExtMode()
{
    if (m_AppExtMode.empty())
        return;

    vector<IAppModeExtension*> appModes;
    GetExtensionAsInterface(EXT_POINT__APP_MODE_EXTENSION, appModes);
    for (auto ext : appModes) {
        if (ext->GetModeName() == m_AppExtMode) {
            ext->SetWorkbench(0);
            return;
        }
    }
    LOG_POST(Error << "Application mode extension: \"" << m_AppExtMode << "\" not found.");
}

bool CGBenchApp::x_CreateGUI()
{
    LOG_POST(Info << "Creating application GUI...");

    // create Workbench instance with m_GUI as adviser
    m_Workbench.reset(new CWorkbench(&m_GUI));

    x_InitExtMode();

    int autoSaveInterval = CGuiRegistry::GetInstance().GetInt("GBENCH.Application.ViewOptions.AutoSaveInterval", 60);
    if (autoSaveInterval > 0)
        m_WsAutoSaver.reset(new CWorkspaceAutoSaver(*m_Workbench, (size_t)autoSaveInterval));

    /// set path in Registry, we need to this before calling Init()
    m_Workbench->SetRegistryPath("Workbench");

    m_GUI.SetWorkbench(m_Workbench.get());
    m_GUI.SetRegistryPath("ApplicationGUI");

    m_Workbench->Init();

    m_GUI.RestoreWindowLayout();

    CAppTaskService* task_service = m_Workbench->GetAppTaskService();

    bool loadFiles = true;
    if (m_WsAutoSaver.get()) {
        if (m_WsAutoSaver->CheckCrash(m_AutoSaveMode))
            loadFiles = false;
    }

    // Load files provided as arguments on command line
    if (loadFiles && !m_Args.empty()) {
        COpenFilesTask* task = new COpenFilesTask(m_Workbench.get(), m_Args, false);
        task_service->AddTask(*task);
    }
    m_Args.clear();

    m_readPipeThread.Reset(new CReadPipeThread(s_NamedPipeServer, m_Workbench.get()));
    m_readPipeThread->Run();

    m_WakeupThread.Reset(new CWakeupThread());
    m_WakeupThread->Run();

    m_PagerMessageJob.reset(job_async(
        [](ICanceled&)
    {
        string url =
#ifdef _DEBUG
            "https://ftp.ncbi.nlm.nih.gov/toolbox/gbench/message/text_debug";
#else
            "https://ftp.ncbi.nlm.nih.gov/toolbox/gbench/message/text";
#endif
        CRef<CHttpSession> session(new CHttpSession());
        CHttpRequest request = session->NewRequest(url);
        request.SetDeadline(CTimeout(40)).SetRetryProcessing(eOn);
        CRef<CHttpResponse> response(new CHttpResponse(request.Execute()));
        CNcbiIstream& in = response->ContentStream();
        string message;
        char buffer[4096];
        while (in.read(buffer, sizeof(buffer)))
            message.append(buffer, sizeof(buffer));
        message.append(buffer, in.gcount());
        return message;
    },
        [workbench = m_Workbench.get()](job_future<string>& fn)
    {
        string message;
        try {
            message = fn();
        }
        catch (const CHttpSessionException&) {}
        NCBI_CATCH("Message Pager:");

        CWorkbenchFrame* mainWnd = workbench ?
            dynamic_cast<CWorkbenchFrame*>(workbench->GetMainWindow()) : (CWorkbenchFrame*)nullptr;
        if (mainWnd)
            mainWnd->SetPagerMessage(message);
    }
    , "Message Pager"));

    string msg = wxGetApp().GetStartUpString();
    if( !msg.empty() ){
        //NcbiInfoBox(msg, "New Version of Genome Workbench Is Available!");
        NcbiInfoBox( msg, "Welcome to Genome Workbench!" );
    }

    LOG_POST(Info << "Finished creating application GUI");
    return true;
}


void CGBenchApp::x_DestroyGUI()
{
    if (m_WsAutoSaver.get()) {
        m_WsAutoSaver->Stop();
        m_WsAutoSaver.reset();
    }

    if (m_WakeupThread) {
        m_WakeupThread->RequestStop();
        m_WakeupThread->Join();
    }
    if (m_readPipeThread) {
        m_readPipeThread->RequestStop();
        m_readPipeThread->Join();
    }

    LOG_POST(Info << "Destroying application GUI...");

    // m_Workbench already has been shut down
    x_ShutExtMode();
    m_GUI.SetWorkbench(NULL);

    m_Workbench.reset();

    LOG_POST(Info << "Finished destroying application GUI");
}

// we override this function to add idle processing for CAppTaskService
bool CGBenchApp::x_OnIdle()
{
    // we handle more urgent requests first
    bool handled = CwxNCBIApp::x_OnIdle();

    if( ! handled  &&  m_Workbench.get() != NULL)  {
        // if Task Service is available - give it a chance to execute
        CAppTaskService* task_mgr = m_Workbench->GetAppTaskService();
        if(task_mgr) {
            handled = task_mgr->IdleCallback();
        }
    }
    return handled;
}

wxString CGBenchApp::x_GetGuiRegistryPath()
{
    static bool _first = true;
    if (_first) {
        _first = false;
        wxString toDelete = CSysPath::ResolvePathExisting(wxT("<home>/gbench.asn"));
        if (!toDelete.empty()) ::wxRemoveFile(toDelete);
        toDelete = CSysPath::ResolvePathExisting(wxT("<home>/gbench_user.asn"));
        if (!toDelete.empty()) ::wxRemoveFile(toDelete);
    }

    wxString path = CSysPath::ResolvePath(USER_REGISTRY_FILE);
    return path;
}

void CGBenchApp::MacOpenFile(const wxString &fileName)
{
    LOG_POST(Info<< "Opening file: " << fileName);
    
    bool loadFiles = true;
    if (m_WsAutoSaver.get()) {
        if (m_WsAutoSaver->CheckCrash(m_AutoSaveMode))
            loadFiles = false;
    }
    
    if (loadFiles) {
        CAppTaskService* task_service = m_Workbench->GetAppTaskService();

        vector<string> files;
        files.push_back(std::string(fileName));

        COpenFilesTask* task = new COpenFilesTask(m_Workbench.get(), files, false);
        task_service->AddTask(*task);
    }
}

void CGBenchApp::x_LoadGuiRegistry()
{
    if( m_RegistryLoaded ){
        return;
    }

    /// load the standard NCBI .ini file registry

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();

    /// Set the global repository. We have two repository files to look for
    wxString path = CSysPath::ResolvePath( wxT("<std>/etc/gbench.asn") );
    if( wxFileName::FileExists( path ) ){
        CNcbiIfstream istr( path.fn_str() );
        if( istr ){
            LOG_POST( Info << "Loading base GUI registry from " << path );
            gui_reg.AddSite( istr, CGuiRegistry::ePriority_Global );
        }
    }

    CWndLayoutReg::GetInstance().Load();

    path = CSysPath::ResolvePath( wxT("<std>/etc/plugin_config.asn") );
    if( wxFileName::FileExists( path ) ){
        CNcbiIfstream istr( path.fn_str() );
        if( istr ){
            LOG_POST( Info << "Loading plugin config settings from " << path );
            gui_reg.AddSite( istr, CGuiRegistry::ePriority_Global -1 );
        }
    }

    /// set the local repository
    CwxNCBIApp::x_LoadGuiRegistry();

    /// look for any site-specific files and load these
    {{
        CRegistryReadView view = gui_reg.GetReadView("GBENCH.Application.Sites");

        vector<string> site_names;
        view.GetStringVec( "ActiveSites", site_names );
        int site_prioty = CGuiRegistry::ePriority_Local;

        set<wxString> visited_sites;
        ITERATE( vector<string>, iter, site_names ){
            // TODO FromASNString
            path = wxString::FromAscii(
                view.GetString( CGuiRegistryUtil::MakeKey("Sites", *iter) ).c_str()
            );
            path = CSysPath::ResolvePathExisting( path );
            if( path.empty() ){
                continue;
            }

            /// don't add a site more than once
            if( visited_sites.insert( path ).second == false ){
                continue;
            }

            CNcbiIfstream istr( path.fn_str() );
            if( istr ){
                LOG_POST( Info << "Loading site registry file from " << path );
                gui_reg.AddSite( istr, ++site_prioty );
            }
        }
    }}

    {{ // x_SyncRegistryAndEnvironment()
        CNcbiApplication* app = CNcbiApplication::Instance();
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();

        /// propagate to environment where appropriate
#ifdef NCBI_OS_MSWIN
        const char* path_sep = ";";
#else
        const char* path_sep = ":";
#endif
        CRegistryReadView env_view =
            gui_reg.GetReadView( "GBENCH.Application.Environment" )
        ;

        CRegistryReadView::TKeys keys;
        env_view.GetTopKeys( keys );

        ITERATE(CRegistryReadView::TKeys, keyIter, keys){
            string key = keyIter -> key;
            string metaSuffix = "-meta";

            if (NStr::EndsWith(key, metaSuffix)){
                continue;
            }
            string policy_key = key + metaSuffix;
            string policy =
                env_view.GetString(CGuiRegistryUtil::MakeKey(policy_key, "MergePolicy"))
            ;

            vector<string> val;
            env_view.GetStringVec(key, val);
    #if defined(_DEBUG)  &&  defined(NCBI_COMPILER_MSVC)
            {{
                ///
                /// HACK: on Windows, for debug builds, we require a different
                /// path for some variables
                ///
                NON_CONST_ITERATE (vector<string>, iter, val) {
                    if (iter->find("ReleaseDLL") != string::npos) {
                        *iter = NStr::Replace(*iter, "ReleaseDLL", "DebugDLL");
                    }
                }
            }}
    #endif
            if (0 == NStr::CompareNocase(policy, "append")) {
                if (val.size() > 0) {
                    string s = app->GetEnvironment().Get(key);
                    s += path_sep;
                    s += NStr::Join(val, path_sep);
                    app->SetEnvironment(keyIter -> key, s);
                }
            } else if (0 == NStr::CompareNocase(policy, "prepend")) {
                if( val.size() > 0 )    {
                    string s = NStr::Join(val, path_sep);
                    s += path_sep;
                    s += app->GetEnvironment().Get(key);
                    app->SetEnvironment(key, s);
                }
            } else {
                string value = env_view.GetString(key);
                if( ! value.empty() )   {
                    app->SetEnvironment(key, value);
                }
            }
        }

        /// propagate to some internal variables where appropriate
        // Same thing for NcbiEnvironment*
        CRegistryReadView reg_view = gui_reg.GetReadView("GBENCH.Application.Registry");

        keys.clear();
        reg_view.GetTopKeys(keys);

        ITERATE(CRegistryReadView::TKeys, keyIter, keys){
            string key = keyIter->key;
            gui_reg.Set( key, reg_view.GetString(key) );
        }
    }}

    /// seq-id accession mapping guide
    {{
        wxString fname = CSysPath::ResolvePathExisting(wxT("<std>/etc/accguide.txt"));
        if( !fname.empty() ){
            try {
                CNcbiIfstream istr(fname.fn_str());
                CStreamLineReader lreader(istr);
                objects::CSeq_id::LoadAccessionGuide( lreader );
                LOG_POST( Info << "loaded sequence accession map from " << fname );
            }
            STD_CATCH_ALL( "CGBenchApplication::x_LoadGuiRegistry(): failed to load accession guide" );
        }
    }}
}


void CGBenchApp::x_SaveGuiRegistry()
{
    CwxNCBIApp::x_SaveGuiRegistry();
    CWndLayoutReg::GetInstance().Save();
}

END_NCBI_SCOPE;

/////////////////////////////////////////////////////////////////////////////
//  main entry point

int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    USING_NCBI_SCOPE;
    ///
    /// 5/23/2008: HACK
    /// we set a few environment variables manually here to make sure that
    /// we get a safety blanket initialization for gbench
    /// CObject's initialization changed recently.
    ///
    {{
#if defined(_DEBUG)
#ifdef NCBI_COMPILER_MSVC
        /// disable fill for MSVC - Microsoft's debug heap already does this
        static char env[] = "NCBI_MEMORY_FILL=NONE";
        ::putenv(env);
#endif
#elif defined(NDEBUG)
        /// re-enable zero fill
        static char env[] = "NCBI_MEMORY_FILL=ZERO";
        ::putenv(env);
#endif
    }}
    ///
    /// end HACK
    ///


    wxDISABLE_DEBUG_SUPPORT();

    ncbi::CGBenchApplication ncbi_app;

    // Execute main application function
    //
    // Use "Genome-Workbench" (NOT "Genome Workbench") to produce standards compliant HTTP request
    // "User-Agent: Genome-Workbench/2.7.15"
    //

    size_t ver_major, ver_minor, ver_patch_level;
    ncbi::CTime build_date;

    GetGBenchVersionInfo(ver_major, ver_minor, ver_patch_level, build_date);

    string pname = "Genome-Workbench/";
    pname.append(ncbi::NStr::NumericToString(ver_major));
    pname.append(".");
    pname.append(ncbi::NStr::NumericToString(ver_minor));
    pname.append(".");
    pname.append(ncbi::NStr::NumericToString(ver_patch_level));

    return ncbi_app.AppMain(argc, argv, 0, ncbi::eDS_Default, 0, _T_XSTRING(pname));
}
