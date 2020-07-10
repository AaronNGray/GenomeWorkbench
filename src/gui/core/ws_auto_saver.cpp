/*  $Id: ws_auto_saver.cpp 43749 2019-08-28 15:51:56Z katargir $
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
 *
 */

#include <ncbi_pch.hpp>

#include <gui/core/ws_auto_saver.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/framework/service.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/project_loading_task.hpp>
#include <gui/core/document.hpp>

#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>
#include <objects/general/Date.hpp>
#include <objects/gbproj/ProjectDescr.hpp>

#include <serial/serial.hpp>
#include <serial/iterator.hpp>
#include <serial/objostr.hpp>

#include <corelib/ncbifile.hpp>

#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/async_call.hpp>

#include <wx/filename.h>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

const wxChar* kTemsWsDir = wxT("<home>/TempWs");
const string kAutoSave = "AutoSave: ";
const char* kProjTemplate = "proj%03d";
const char* kWorkspace = "ws";
const char* kFailedService = "Failed to initialize service";
const char* kCanceled = "Canceled";

BEGIN_EVENT_MAP(CWorkspaceAutoSaver, CEventHandler)
  ON_EVENT(CAutoSaverEvent, CAutoSaverEvent::eSave, &CWorkspaceAutoSaver::x_OnSave)
END_EVENT_MAP()


void CWorkspaceAutoSaver::SetInterval(size_t interval)
{
    Stop();
    m_Interval = interval;
    PostSave();
}

void CWorkspaceAutoSaver::PostSave()
{
    StartOnce((int)m_Interval*1000);
}

bool CWorkspaceAutoSaver::CleanUp()
{
    wxString path = CSysPath::ResolvePath(kTemsWsDir);
    if (path.empty()) return false;

    if (wxFileExists(path)) {
        if (!wxRemoveFile(path)) {
            LOG_POST(Error << kAutoSave << "Failed to delete file \"" << path << "\"");
            return false;
        }
        return true;
    }

    if (wxDirExists(path)) {
        if (!wxFileName::Rmdir(path, wxPATH_RMDIR_RECURSIVE)) {
            LOG_POST(Error << kAutoSave << "Failed to delete directory \"" << path << "\"");
            return false;
        }
        return true;
    }
    return true;
}

bool CWorkspaceAutoSaver::CheckCrash(ERestoreMode mode)
{
    if (mode == eIgnore) {
        CleanUp();
        return false;
    }

    wxString path = CSysPath::ResolvePath(kTemsWsDir);
    if (path.empty() || !wxDirExists(path)) return false;

    wxFileName fname;
    fname.SetPath(path);
    fname.SetFullName(kWorkspace);
    if (!fname.FileExists()) return false;

    if (mode == eAskUser && wxOK !=
        wxMessageBox(wxT("Application crash detected on the previous launch.\n")
            wxT("Do you want Genome Workbench to try to load the last saved data?"),
            wxT("Exception"), wxOK | wxCANCEL | wxICON_EXCLAMATION)) {
        CleanUp();
        return false;
    }

    m_Err.clear();

    try {
        GUI_AsyncExec([this](ICanceled& canceled) { this->x_RestoreWorkspace(canceled); },
                      wxT("Restoring workspace..."));
    } NCBI_CATCH("CWorkspaceAutoSaver::x_RestoreWorkspace");

    CProjectService* prj_srv = m_ServiceLocator.GetServiceByType<CProjectService>();
    if (prj_srv) {
        try {
            CRef<CGBWorkspace> ws = prj_srv->GetGBWorkspace();
            if (ws) {
                CWorkspaceFolder& wsFolder = ws->SetWorkspace();
                NON_CONST_ITERATE(objects::CWorkspaceFolder::TProjects, it, wsFolder.SetProjects()) {
                    CGBDocument* doc = dynamic_cast<CGBDocument*>(it->GetPointer());
                    if (doc && doc->IsLoaded()) {
                        doc->AttachData();
                    }
                }
            }
        }
        catch (const CException& e) {
            m_Err = e.GetMsg();
        }
        catch (const exception& e) {
            m_Err = e.what();
        }
    }
    else  {
        m_Err = kFailedService;
    }

    if (!m_Err.empty()) {
        if (prj_srv) prj_srv->ResetWorkspace();
        wxMessageBox(wxT("Failed to load saved data: ") + ToWxString(m_Err),
                     wxT("Error"), wxOK|wxICON_ERROR);
    }
    if (prj_srv) prj_srv->x_ReloadProjectTreeView();
    return true;
}

void CWorkspaceAutoSaver::x_RestoreWorkspace(ICanceled& canceled)
{
    wxString path = CSysPath::ResolvePath(kTemsWsDir);
    if (path.empty() || !wxDirExists(path)) return;

    wxFileName fname;
    fname.SetPath(path);
    fname.SetFullName(kWorkspace);

    CProjectService* prj_srv = m_ServiceLocator.GetServiceByType<CProjectService>();
    if (!prj_srv) { m_Err = kFailedService; return; }
    CIRef<CAppTaskService> task_srv = m_ServiceLocator.GetServiceByType<CAppTaskService>();
    if (!task_srv) { m_Err = kFailedService; return; }

    try {
        CRef<CGBWorkspace> ws = prj_srv->x_LoadWorkspace(fname.GetFullPath());
        if (!ws) NCBI_THROW(CException, eUnknown, "Failed to read workspace");

        prj_srv->m_Workspace = ws;

        for (CTypeIterator<CGBProjectHandle> it(ws->SetWorkspace()); it; ++it) {
            if (canceled.IsCanceled()) {
                CleanUp();
                m_Err = kCanceled;
                return;
            }

            if (!it->IsSetFilename())
                continue;
            string fileName = it->GetFilename();

            int counter = 0;
            string format = kProjTemplate;
            format += "|";
            if (!fileName.empty() && sscanf(fileName.c_str(), format.c_str(), &counter) == 1)
            {
                size_t pos = fileName.find("|");
                if (pos != string::npos && pos != 0) {
                    string projFile = fileName.substr(0, pos);
                    it->SetFilename(fileName.substr(pos + 1));
                    fname.SetFullName(projFile);
                    auto_ptr<CNcbiIfstream> istr(new CNcbiIfstream(fname.GetFullPath().fn_str(), ios::binary|ios::in));
                    it->Load(*istr, &canceled);
                    it->SetDirty(true);
                    CGBDocument* doc = dynamic_cast<CGBDocument*>(&*it);
                    doc->CreateProjectScope();
                    doc->SetLoaded();
                }
            }
        }

        vector<int> ids = ws->GetUnloadedProjects();
        if (!ids.empty()) {
            ITERATE(vector<int>, it2, ids) {
                if (canceled.IsCanceled()) {
                    CleanUp();
                    m_Err = kCanceled;
                    return;
                }
                CRef<CProjectLoadingTask> task(new CProjectLoadingTask(*it2, prj_srv, false));
                task_srv->AddTask(*task);
            }
        }
    } catch (const CException& e) {
        m_Err = e.GetMsg();
    } catch (const exception& e) {
        m_Err = e.what();
    }

    if (!m_Err.empty()) {
        CleanUp();
    }
}

void CWorkspaceAutoSaver::x_SaveWorkspace(ICanceled& canceled)
{
    wxString path = CSysPath::ResolvePath(kTemsWsDir);
    if (path.empty()) return;

    //if (!CleanUp()) return;

    CProjectService* prj_srv = m_ServiceLocator.GetServiceByType<CProjectService>();
    CRef<CGBWorkspace> ws;
    if (prj_srv) ws = prj_srv->GetGBWorkspace();
    if (!ws) return;

    CTypeConstIterator<CGBProjectHandle> it(ws->GetWorkspace());
    if (!it) return;

    if (!wxFileName::DirExists(path) && !wxFileName::Mkdir(path)) {
        LOG_POST(Error << kAutoSave << "Failed to create directory \"" << path << "\"");
        return;
    }

    try {
        wxFileName fname;
        fname.SetPath(path);

        int counter = 0;
        for (CTypeIterator<CGBProjectHandle> it(ws->SetWorkspace()); it; ++it) {
            if (canceled.IsCanceled())
                NCBI_THROW(CException, eUnknown, "Canceled");

            if (it->IsDirty()) {
                char buf[128];
                sprintf(buf, kProjTemplate, ++counter);
                string projFile(buf);

                string fileName = projFile + "|";
                if (it->IsSetFilename())
                    fileName += it->GetFilename();
                it->SetFilename(fileName);

                fname.SetFullName(projFile);
                CTime time = it->GetDescr().GetModified_date().AsCTime();
                string filePath(fname.GetFullPath().ToUTF8());
                CDirEntry dir(filePath);
                if (dir.IsNewer(time, CDirEntry::eIfAbsent_NotNewer))
                    continue;

                CNcbiOfstream ostr(fname.GetFullPath().fn_str(), ios::binary);

                ESerialDataFormat fmt =
#ifdef _DEBUG
                eSerial_AsnText;
#else
                eSerial_AsnBinary;
#endif
                it->Save(ostr, fmt);
                it->SetDirty(true);
                it->SetProject().SetDescr().SetModifiedDate(time);
            }
        }

        fname.SetFullName(kWorkspace);
        CNcbiOfstream ostr(fname.GetFullPath().fn_str());
        auto_ptr<CObjectOStream> os(CObjectOStream::Open(eSerial_AsnText, ostr));
        *os << *ws;
    } NCBI_CATCH(kAutoSave + "x_SaveWorkspace() failed.");

    for (CTypeIterator<CGBProjectHandle> it(ws->SetWorkspace()); it; ++it) {
        string fileName;
        if (it->IsSetFilename())
            fileName = it->GetFilename();
        int counter = 0;
        string format = kProjTemplate;
        format += "|";
        if (!fileName.empty() && sscanf(fileName.c_str(), format.c_str(), &counter) == 1)
        {
            size_t pos = fileName.find("|");
            if (pos != string::npos)
                it->SetFilename(fileName.substr(pos + 1));
        }
    }

    if (canceled.IsCanceled())
        CleanUp();
}

void CWorkspaceAutoSaver::Notify()
{
    Post(CRef<CEvent>(new CAutoSaverEvent()));
}

static size_t s_counter = 0;
static size_t s_max_time = 0;

void CWorkspaceAutoSaver::x_OnSave(CEvent*)
{
    if (CAsyncCall::InsideAsyncCall()) {
        Post(CRef<CEvent>(new CAutoSaverEvent()));
        return;
    }

    CProjectService* prj_srv = m_ServiceLocator.GetServiceByType<CProjectService>();
    CRef<CGBWorkspace> ws;
    if (prj_srv) ws = prj_srv->GetGBWorkspace();
    if (!ws) {
        PostSave();
        return;
    }

    CStopWatch timer;
    timer.Start();

    try {
        GUI_AsyncExec([this](ICanceled& canceled) { this->x_SaveWorkspace(canceled); },
                      wxT("Saving workspace copy..."));
    } NCBI_CATCH("CWorkspaceAutoSaver::x_SaveWorkspace");

    timer.Stop();
    size_t elapsed = (size_t)timer.Elapsed();
    if (elapsed > 5) {
        ++s_counter;
        s_max_time = max(s_max_time, elapsed);
        if (s_counter <= 10) {
            LOG_POST(Info << CTime(CTime::eCurrent).AsString("h:m:s") << " Autosave " << elapsed << " sec");
        }
        else if ((s_counter%100) == 0) {
            LOG_POST(Info << "100 Autosaves with save time more than 5 sec. Max time: " << s_max_time << " sec.");
            s_max_time = 0;
        }
    }
    PostSave();
}

END_NCBI_SCOPE
