/*  $Id $
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
 * Authors: Yury Voronov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/winmask_files.hpp>

#include "winmask_files_dlg.hpp"

#include <gui/objutils/registry.hpp>

#include <corelib/ncbiapp.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/sys_path.hpp>

#include <gui/utils/download_job.hpp>
#include <gui/utils/ftp_utils.hpp>

#include <wx/filename.h>
#include <wx/dir.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

BEGIN_EVENT_MAP(CWinMaskerFileStorage, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged, &CWinMaskerFileStorage::x_OnJobNotification)
END_EVENT_MAP()

CWinMaskerFileStorage::CWinMaskerFileStorage()
    : m_RegPath()
    , m_UseEnvPath(true)
    , m_JobId(CAppJobDispatcher::eInvalidJobID)
{
}

CWinMaskerFileStorage::~CWinMaskerFileStorage()
{
    if (m_JobId != CAppJobDispatcher::eInvalidJobID)
        CAppJobDispatcher::GetInstance().DeleteJob(m_JobId);
}

CWinMaskerFileStorage& CWinMaskerFileStorage::GetInstance()
{
    static CRef<CWinMaskerFileStorage> s_Storage;
    DEFINE_STATIC_MUTEX(s_StorageMutex);
    if( !s_Storage ){
        CMutexGuard LOCK(s_StorageMutex);
        if( !s_Storage ){
            s_Storage.Reset( new CWinMaskerFileStorage() );
        }
    }
    return *s_Storage;
}

void CWinMaskerFileStorage::x_OnJobNotification(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);
    if (notn->GetJobID() != m_JobId) return;

    switch (notn->GetState()) {
    case IAppJob::eCompleted:
    case IAppJob::eCanceled:
    case IAppJob::eFailed:
        m_JobId = CAppJobDispatcher::eInvalidJobID;
        break;
    default:
        return;
    }
}

void CWinMaskerFileStorage::SetRegistryPath( const string& path )
{
    m_RegPath = path;
}

static const char* kUseEnvPath = "UseEnvPath";

void CWinMaskerFileStorage::SaveSettings() const
{
    _ASSERT(!m_RegPath.empty());

    if( !m_RegPath.empty() ){
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
        view.Set(kUseEnvPath, m_UseEnvPath);
    }
}

void CWinMaskerFileStorage::LoadSettings()
{
    _ASSERT(!m_RegPath.empty());

    if( !m_RegPath.empty() ){
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);
        m_UseEnvPath = view.GetBool(kUseEnvPath, true);
    }
}

wxString CWinMaskerFileStorage::GetEnvPath() const
{
    wxString path;
    CNcbiEnvironment env;
    const string& env_path = env.Get("WINDOW_MASKER_PATH");
    if (!env_path.empty())
        path = wxString::FromUTF8(env_path.c_str());

    if (path.empty()) {
        CNcbiApplication* app = CNcbiApplication::Instance();
        if (app){
            const CNcbiRegistry& reg = app->GetConfig();
            const string& wm_path = reg.Get("WINDOW_MASKER", "WINDOW_MASKER_PATH");
            if (!wm_path.empty())
                path = wxString::FromUTF8(wm_path.c_str());
        }
    }

    if (path.empty()) {
        wxString internal_path = wxT("\\\\SNOWMAN\\gbench\\data\\window-masker");
        if (wxFileName::DirExists(internal_path)){
            path = internal_path;
        }
    }
    // wxFileName doesn't work with win UNC names wich starts with "//"
#ifdef NCBI_OS_MSWIN
    path.Replace(wxT("/"), wxT("\\"));
#endif
    return path;
}

wxString CWinMaskerFileStorage::GetPath() const
{
    return m_UseEnvPath ? GetEnvPath() : x_GetDownloadPath();
}

void CWinMaskerFileStorage::GetTaxIds(vector<int>& ids)
{
    ids.clear();

    bool succeeded = false;

    try {
        wxString dir_path = GetPath();
        if (dir_path.empty())
            return;

        wxDir dir(dir_path);
        if (!dir.IsOpened())
            return;

        wxString dirName;
        bool cont = dir.GetFirst(&dirName, wxEmptyString, wxDIR_DIRS);
        while (cont) {
            long tax_id;
            if (dirName.ToLong(&tax_id) && tax_id)
                ids.push_back((int)tax_id);

            cont = dir.GetNext(&dirName);
        }
        succeeded = true;
    } NCBI_CATCH("CWinMaskerFileStorage::GetWinMaskerTaxIdsSync()");

    if (!succeeded)
        ids.clear();
}

static const wxChar* kFtpUrl = wxT("ftp://ftp.ncbi.nlm.nih.gov/toolbox/gbench/data/window-masker2/");

wxString CWinMaskerFileStorage::GeFtpUrl() const
{
    return kFtpUrl;
}

wxString CWinMaskerFileStorage::x_GetDownloadPath() const
{
    return CSysPath::ResolvePath(wxT("<home>/window-masker"));
}

static bool s_RmDir(const wxString& path)
{
    wxDir dir(path);
    if (!dir.IsOpened())
        return true;

    bool success = true;

    wxString entry;
    bool cont = dir.GetFirst(&entry);
    while (cont) {
        wxString fullPath = wxFileName(path, entry).GetFullPath();
        if (wxFileName::DirExists(fullPath)) {
            if (!s_RmDir(fullPath))
                success = false;
        }
        else {
            if (!wxRemoveFile(fullPath))
                success = false;
        }
        cont = dir.GetNext(&entry);
    }

    if (!wxFileName::Rmdir(path))
        success = false;

    return success;
}

bool CWinMaskerFileStorage::x_ClearDownloads() const
{
    wxString path = x_GetDownloadPath();
    wxDir dir(path);
    if (!dir.IsOpened())
        return true;

    if (wxFileName::DirExists(path) && !s_RmDir(path)) {
        LOG_POST(Error << "Failed to delte direciory for window masker files:" << path.ToUTF8());
        return false;
    }
    return true;
}

void CWinMaskerFileStorage::x_DownloadFileList(vector<string>& files)
{
    CFtpUtils::GetDirContent(files, string(GeFtpUrl().ToUTF8()));
}

void CWinMaskerFileStorage::x_DownloadFiles(const vector<string>& files)
{
    wxString path = x_GetDownloadPath();
    if (!wxFileName::DirExists(path) && !::wxMkdir(path)) {
        LOG_POST(Error << "Failed to create direciory for window masker files:" << path.ToUTF8());
        return;
    }

    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    if (m_JobId != CAppJobDispatcher::eInvalidJobID) {
        disp.DeleteJob(m_JobId);
        m_JobId = CAppJobDispatcher::eInvalidJobID;
    }

    CRef<CDownloadJob> job(new CDownloadJob("Download", string(path.ToUTF8()), string(GeFtpUrl().ToUTF8()), files));
    m_JobId = disp.StartJob(*job, "ThreadPool", *this, 0, true);
}

bool CWinMaskerFileStorage::ShowOptionsDlg()
{
    return (CWinMaskFilesDlg(NULL).ShowModal() == wxID_OK);
}

END_NCBI_SCOPE
