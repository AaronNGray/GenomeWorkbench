/*  $Id: app_updater_task.cpp 41347 2018-07-12 18:28:31Z evgeniev $
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
* Authors:  Vladislav Evgeniev
*
* File Description:
*
*/

#include <ncbi_pch.hpp>

#include <wx/wx.h>
#include <wx/platinfo.h>
#include <wx/filename.h>

#include <corelib/ncbireg.hpp>
#include <corelib/ncbi_url.hpp>

#include <gui/objects/GBenchVersionInfo.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/utils/app_job_impl.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/usage_report_job.hpp>
#include <gui/objutils/registry.hpp>
#include <gui/utils/ftp_utils.hpp>
#include <gui/utils/download_job.hpp>

#include "app_updater_task.hpp"
#include "gbench_app.hpp"
#include <gui/objects/gbench_version.hpp>
#include "new_version_dlg.hpp"

BEGIN_NCBI_SCOPE

using objects::CGBenchVersionInfo;

DECLARE_APP(ncbi::CGBenchApp)

const char* kGBenchPath = "ftp://ftp.ncbi.nlm.nih.gov/toolbox/gbench/";
const char* kSilenceUpdateFile = "SILENT";
const char* kVersionPrefix = "ver-";
const char* kGBenchPrefix = "gbench";
const size_t kPrefixPosition = 4;
const char* kIgnoreVersion = "GBENCH.Application.AutoUpdates.IgnoreVersion";

///////////////////////////////////////////////////////////////////////////////
/// CAppUpdateCheckResult - result from the update check job, indicates if there is a new version and its URL
class CAppUpdateCheckResult : public CObject
{
public:
    CAppUpdateCheckResult()
        : m_NewVersionAvailable(false)
    {}

    CAppUpdateCheckResult(CGBenchVersionInfo& new_version, const string &url, const string &package)
        : m_NewVersionAvailable(true)
        , m_NewVersion(&new_version)
        , m_NewVersionUrl(url)
        , m_Package(package)
    {}

    bool IsNewVersion() const { return m_NewVersionAvailable; }
    CRef<CGBenchVersionInfo> GetNewVersion() const { return m_NewVersion; }
    const string& GetNewVersionURL() const { return m_NewVersionUrl; }
    const string& GetPackage() const { return m_Package; }

public:
    bool                        m_NewVersionAvailable;
    CRef<CGBenchVersionInfo>    m_NewVersion;
    string                      m_NewVersionUrl;
    string                      m_Package;
};

bool CompareVersionString(const string &first, const string &second)
{
    vector<string> tokensFirst;
    NStr::Split(first, string("."), tokensFirst, NStr::fSplit_Tokenize);
    vector<string> tokensSecond;
    NStr::Split(second, string("."), tokensSecond, NStr::fSplit_Tokenize);
    for (size_t i = 0; (i < tokensFirst.size()) && (i < tokensSecond.size()); ++i) {
        if (tokensFirst[i] == tokensSecond[i])
            continue;
        if (tokensFirst[i].length() == tokensSecond[i].length())
            return (tokensFirst[i] < tokensSecond[i]);
        return (tokensFirst[i].length() < tokensSecond[i].length());
    }

    return (tokensFirst.size() < tokensSecond.size());
}

///////////////////////////////////////////////////////////////////////////////
/// CAppUpdateCheckJob - a job to check if new version is available and to get its URL
class CAppUpdateCheckJob : public CAppJob
{
public:
    CAppUpdateCheckJob(bool overrideIgnore) :
        CAppJob("Checking for updates"),
        m_OverrideIgnore(overrideIgnore)
    {
        CNcbiApplication* app = CNcbiApplication::Instance();
        _ASSERT(app);
        string url = app->GetEnvironment().Get("GBENCH_UPDATE_URL");
        m_UpdateURL = url.empty() ? kGBenchPath : url;
    }

    /// @name IAppJob implementation
    /// @{
    virtual EJobState                   Run();
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string                      GetDescr() const;
    /// @}

    static string GetVersionToIgnore();
    static void SetVersionToIgnore(const string &version);

protected:
    void x_FilterList(vector<string> &items, const string &prefix);
    bool x_GetLatestVersion(string &latest_version);
    bool x_IsUpdateSilenced(const string &update_folder);
    string x_GetPackage(const string &releaseUrl);

private:
    string                              m_UpdateURL;
    bool                                m_OverrideIgnore;
    CRef<CAppUpdateCheckResult>         m_Result;
    CRef<CAppJobError>                  m_Error;
    typedef map<string, string>         TLinuxDistrosPkgsExt;
    static const TLinuxDistrosPkgsExt   m_DistrosPkgsExt;
};

const CAppUpdateCheckJob::TLinuxDistrosPkgsExt CAppUpdateCheckJob::m_DistrosPkgsExt({
    { "ubuntu", "deb" },
    { "debian", "deb" },
    { "fedora", "rpm" },
    { "opensuse", "rpm" }
});


string CAppUpdateCheckJob::GetVersionToIgnore()
{
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    return gui_reg.GetString(kIgnoreVersion, "");
}

void CAppUpdateCheckJob::SetVersionToIgnore(const string &version)
{
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    gui_reg.Set(kIgnoreVersion, version);
}

CRef<CObject> CAppUpdateCheckJob::GetResult()
{
    CFastMutexGuard guard(m_Mutex);
    return CRef<CObject>(m_Result.GetPointer());
}

CConstIRef<IAppJobError> CAppUpdateCheckJob::GetError()
{
    CFastMutexGuard guard(m_Mutex);
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}

string CAppUpdateCheckJob::GetDescr() const
{
    return "Checking for application updates";
}

IAppJob::EJobState CAppUpdateCheckJob::Run()
{
    CGBenchVersionInfo cur_version;
    GetGBenchVersionInfo(cur_version);

    string latest_version;
    if (!x_GetLatestVersion(latest_version))
        return eFailed;

    LOG_POST(Info << "The latest GBench version is " << latest_version);

    vector<string> tokens;
    NStr::Split(latest_version, string("."), tokens, NStr::fSplit_Tokenize);
    if (tokens.size() == 3){
        size_t ver_major = NStr::StringToNumeric<size_t>(tokens[0]);
        size_t ver_minor = NStr::StringToNumeric<size_t>(tokens[1]);
        size_t ver_patch = NStr::StringToNumeric<size_t>(tokens[2]);

        CRef<CGBenchVersionInfo> new_version(new CGBenchVersionInfo());
        new_version->SetVer_major(ver_major);
        new_version->SetVer_minor(ver_minor);
        new_version->SetVer_patch(ver_patch);

        string ignore_version;
        if (!m_OverrideIgnore) {
            ignore_version = GetVersionToIgnore();
        }
        if (!ignore_version.empty()) {
            LOG_POST(Info << "Version to ignore " << ignore_version);
        }

        if ((cur_version < *new_version) && (latest_version != ignore_version)) {

            LOG_POST(Warning << "New GBench version " << latest_version << " is available");

            string releaseUrl(m_UpdateURL);
            releaseUrl += kVersionPrefix;
            releaseUrl += latest_version;
            releaseUrl += '/';
            string package = x_GetPackage(releaseUrl);

            CFastMutexGuard guard(m_Mutex);
            m_Result.Reset(new CAppUpdateCheckResult(*new_version, releaseUrl, package));
        }
        else {
            CFastMutexGuard guard(m_Mutex);
            m_Result.Reset(new CAppUpdateCheckResult());
        }

    }

    return eCompleted;
}

void CAppUpdateCheckJob::x_FilterList(vector<string> &items, const string &prefix)
{
    vector<string> result;
    result.reserve(items.size());
    // Assumes that the file/directory name is last
    for (size_t i = 0; i < items.size(); ++i) {
        size_t pos = items[i].find(prefix);
        if (pos == std::string::npos)
            continue;
        result.push_back(items[i].substr(pos));
    }
    items = result;
}

bool CAppUpdateCheckJob::x_GetLatestVersion(string &latest_version)
{
    bool result(false);

    do {
        vector<string> folders;
        CFtpUtils::GetDirContent(folders, m_UpdateURL);
        if (folders.empty()) {
            string error("Failed to get FTP directory listing ");
            error += m_UpdateURL;
            m_Error = new CAppJobError(error);
            LOG_POST(Warning << error);
            break;
        }
        x_FilterList(folders, kVersionPrefix);
        if (folders.empty()) {
            string error("No GBench folders found on FTP server ");
            error += m_UpdateURL;
            m_Error = new CAppJobError(error);
            LOG_POST(Warning << error);
            break;
        }

        std::sort(folders.begin(), folders.end(), CompareVersionString);

        for (auto latest = folders.rbegin(); latest != folders.rend(); ++latest) {
            latest_version = *latest;
            if (x_IsUpdateSilenced(latest_version))
                continue;

            latest_version = latest_version.substr(kPrefixPosition);
            result = true;
            break;
        }
    }
    while (false);

    return result;
}

bool CAppUpdateCheckJob::x_IsUpdateSilenced(const string &update_folder)
{
    string update_url(kGBenchPath);
    update_url += update_folder;

    try {
        vector<string> content;
        CFtpUtils::GetDirContent(content, update_url);
        for (auto &file : content)
            if (file == kSilenceUpdateFile)
                return true;
    }
    catch (const CException &) {

    }
    return false;
}

string CAppUpdateCheckJob::x_GetPackage(const string &releaseUrl)
{
    string result;
    string filter;
    const wxPlatformInfo &platformInfo = wxPlatformInfo::Get();
    switch (platformInfo.GetOperatingSystemId())
    {
    case wxOS_MAC_OSX_DARWIN:
        filter = "*.dmg";
        break;
    case wxOS_WINDOWS_NT:
        filter = "*.exe";
        break;
    case wxOS_UNIX_LINUX: {
        wxLinuxDistributionInfo distroInfo = platformInfo.GetLinuxDistributionInfo();
        if (distroInfo.Id.IsEmpty())
            break;
        wxString distroId = distroInfo.Id.Lower();
        TLinuxDistrosPkgsExt::const_iterator itDistro = m_DistrosPkgsExt.find(distroId.ToStdString());
        if (itDistro == m_DistrosPkgsExt.end()) {
            LOG_POST(Warning << "Unsupported Linux distribution: " << distroId);
            break;
        }
        filter = "*" + distroId + "*." + itDistro->second;
        break;
    }
    default:
        LOG_POST(Warning << "Unsupported OS: " << platformInfo.GetOperatingSystemDescription());
        break;
    }

    if (filter.empty() || (wxARCH_64 != platformInfo.GetArchitecture()))
        // Return URL to the folder, containg the latest version
        return result;

    vector<string> files;
    // Find the name of the proper package for the user's OS
    do {
        CFtpUtils::GetDirContent(files, releaseUrl);
        if (files.empty()) {
            string error("Failed to get FTP directory listing ");
            error += releaseUrl;
            m_Error = new CAppJobError(error);
            LOG_POST(Warning << error);
            break;
        }
        x_FilterList(files, kGBenchPrefix);
        if (files.empty()) {
            string error("No GBench packages found on FTP server ");
            error += releaseUrl;
            m_Error = new CAppJobError(error);
            LOG_POST(Warning << error);
            break;
        }

        for (size_t i = 0; i < files.size(); ++i) {
            if (!NStr::MatchesMask(files[i], filter, NStr::eNocase))
                continue;

            result = files[i];
            break;
        }

    } while (false);

    return result;
}

BEGIN_EVENT_MAP(CAppUpdaterTask, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged,
             &CAppUpdaterTask::OnAppJobNotification)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eProgress,
             &CAppUpdaterTask::OnAppJobProgress)
END_EVENT_MAP()

CAppUpdaterTask::CAppUpdaterTask(bool silentMode)
    : CAppTask("Checking for updates"),
    m_JobID(CAppJobDispatcher::eInvalidJobID),
    m_JobState(IAppJob::eInvalid),
    m_Progress(0),
    m_NewVersionAvailable(false),
    m_SilentMode(silentMode)
{
}

CAppUpdaterTask::~CAppUpdaterTask()
{
    x_DeleteJob();
}

IAppTask::ETaskState CAppUpdaterTask::Run()
{
    switch (m_State) {
    case eInitial:
    {{
            _ASSERT(!m_Job);
            _ASSERT(m_JobID == CAppJobDispatcher::eInvalidJobID);
            _ASSERT(m_JobState == IAppJob::eInvalid);

            m_Job.Reset(new CAppUpdateCheckJob(!m_SilentMode));
            m_JobType = eCheckForUpdates;

            m_State = eRunning;
            m_StatusText = "Starting...";

            CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
            m_JobID = disp.StartJob(*m_Job, "ThreadPool", *this, -1, true);

            _ASSERT(m_JobID != CAppJobDispatcher::eInvalidJobID);
            m_JobState = IAppJob::eRunning;
            m_State = eBackgrounded;
            m_StatusText = "Working...";
        }}
    break;

    case eBackgrounded:
        // Job has been launched already - check whether its status has changed
        m_State = x_OnWakeUp();
        break;

    default:
        _ASSERT(false); // unexpected
        m_State = eInvalid;
        break;
    }

    if (m_State == eCompleted) {

        switch (m_JobType)
        {
        case eCheckForUpdates :
            if (m_NewVersionAvailable)
                x_UpdateNotification();
            else
                if (!m_SilentMode)
                    wxMessageBox("Your version is up-to-date.", "Check for Updates");
            break;
        case eDownloadUpdate :
            x_ShellExecuteUpdate();
            break;
        }

    }
    else if (eFailed == m_State && !m_Job.IsNull()) {
        CConstIRef<IAppJobError> error = m_Job->GetError();
        m_StatusText = error->GetText();
    }

    return m_State;
}

void CAppUpdaterTask::x_UpdateNotification()
{
    CGBenchVersionInfo version;
    GetGBenchVersionInfo(version);
    string strCurVersion, strLatestVersion;
    version.GetVersionString(&strCurVersion);

    CAppUpdateCheckResult* updateCheckResult = dynamic_cast<CAppUpdateCheckResult*>(m_Result.GetPointer());
    _ASSERT(updateCheckResult);
    updateCheckResult->GetNewVersion()->GetVersionString(&strLatestVersion);
    string download_url = updateCheckResult->GetNewVersionURL();
    m_Package = updateCheckResult->GetPackage();

    CNewVersionDlg dlg;
    dlg.SetCurrentVersion(ToWxString(strCurVersion));
    dlg.SetLatestVersion(ToWxString(strLatestVersion));
    dlg.SetDownloadURL(ToWxString(download_url) + m_Package);
    dlg.SetInstallationEnabled(!m_Package.empty());
    dlg.Create(NULL);
    switch (dlg.ShowModal()) {
    case wxID_APPLY:
    {
        REPORT_USAGE("other", .Add("task_name", "Automatic update"));
        x_DeleteJob();
        vector<string> files_to_download;
        files_to_download.push_back(m_Package);
        m_Job.Reset(new CDownloadJob("Downloading update", wxFileName::GetTempDir().ToStdString(), download_url, files_to_download, false));
        m_JobType = eDownloadUpdate;
        m_State = eRunning;
        m_StatusText = "Starting...";

        CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
        m_JobID = disp.StartJob(*m_Job, "ThreadPool", *this, 1, true);
        m_JobState = IAppJob::eRunning;
        m_State = eBackgrounded;
        m_StatusText = "Downloading...";
        m_NewVersionAvailable = false;
        break;
    }
    case wxID_IGNORE:
    {
        CAppUpdateCheckJob::SetVersionToIgnore(strLatestVersion);
        m_State = eCompleted;
        m_StatusText = "Finished";
        break;
    }
    }
}

void CAppUpdaterTask::x_ShellExecuteUpdate()
{
    wxString packagePath = wxFileName::GetTempDir();
    packagePath += wxFileName::GetPathSeparator();
    packagePath += m_Package;
#if defined( __WXMAC__ )
    wxExecute(wxString("Open ") + packagePath);
    wxMessageBox("Drag the application into your Applications folder to install the updated version."
        "When the installation is complete, launch the application as usual.",
        "Genome Workbench Update");

#else
    wxExecute(packagePath);
#endif
}

string CAppUpdaterTask::GetStatusText() const
{
    return m_StatusText;
}

void CAppUpdaterTask::x_DeleteJob()
{
    if (m_JobID == CAppJobDispatcher::eInvalidJobID)
        return;

    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    disp.DeleteJob(m_JobID);
}

IAppTask::ETaskState CAppUpdaterTask::x_OnWakeUp()
{
    switch (m_JobState)  {
    case IAppJob::eRunning:
        return eBackgrounded;

    case IAppJob::eCompleted:
        return eCompleted;

    case IAppJob::eFailed:
        return eFailed;

    case IAppJob::eCanceled:
        return eCanceled;

    default:
        return eInvalid;
    }
}

void CAppUpdaterTask::x_Finish(IAppJob::EJobState state)
{
    m_JobState = state;
    m_JobID = CAppJobDispatcher::eInvalidJobID;

    _ASSERT(m_Listener);
    if (m_Listener)  {
        CAppTaskServiceEvent::EType type = CAppTaskServiceEvent::eWakeUpSignal;
        if (m_JobState == IAppJob::eCanceled) {
            type = CAppTaskServiceEvent::eTaskCanceled;
        }

        CRef<CEvent> evt(new CAppTaskServiceEvent(*this, type));
        m_Listener->Post(evt);
    }
}

/// handles "state changed" notification from CAppJobDispatcher
void CAppUpdaterTask::OnAppJobNotification(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);

    if (notn)    {
        int job_id = notn->GetJobID();
        if (m_JobID != job_id) {
            _ASSERT(false);
            ERR_POST("CAppUpdaterTask::OnAppJobNotification() - unknown or invalid Job ID " << job_id);
        }
        else {
            switch (notn->GetState())    {
            case IAppJob::eCompleted:   {
                m_StatusText = "Finished";

                CRef<CObject> result = m_Job->GetResult();

                if (!result.IsNull()){
                    CAppUpdateCheckResult* updateCheckResult = dynamic_cast<CAppUpdateCheckResult*>(result.GetPointer());
                    if (updateCheckResult) {
                        m_NewVersionAvailable = updateCheckResult->IsNewVersion();
                    }

                    m_Result = result;
                }

                x_Finish(IAppJob::eCompleted);
                break;
            }

            case IAppJob::eFailed:  {
                x_Finish(IAppJob::eFailed);
                break;
            }
            case IAppJob::eCanceled:    {
                m_StatusText = "";
                x_Finish(IAppJob::eCanceled);
                break;
            }
            default:
                break;
            }
        }
    }
}

void CAppUpdaterTask::OnAppJobProgress(CEvent* evt)
{
    CAppJobNotification* notn =
        dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);

    if (notn)    {
        int job_id = notn->GetJobID();
        if (m_JobID != job_id) {
            _ASSERT(false);
            ERR_POST("CAppUpdaterTask::OnAppJobNotification() - unknown or invalid Job ID " << job_id);
        }
        else {
            CConstIRef<IAppJobProgress> prg = notn->GetProgress();
            const CAppJobProgress* progress =
                dynamic_cast<const CAppJobProgress*>(prg.GetPointer());
            if (progress)  {
                m_Progress = progress->GetNormDone();
                m_StatusText = progress->GetText();

                if (m_Listener) {
                    CRef<CEvent> evt(new CAppTaskServiceEvent(*this, CAppTaskServiceEvent::eStatusUpdate));
                    m_Listener->Post(evt);
                }
            }
        }
    }
}

END_NCBI_SCOPE
