/*  $Id: project_task.cpp 39891 2017-11-20 18:17:24Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include <gui/core/project_task.hpp>

#include <gui/core/project_loading_task.hpp>

#include <gui/core/project_service.hpp>
#include <gui/core/document.hpp>
#include <gui/core/loading_app_job.hpp>
#include <gui/core/save_dlg.hpp>
#include <gui/core/ws_auto_saver.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/framework/app_job_task.hpp>

#include <gui/objects/GBWorkspace.hpp>
#include <objects/gbproj/ProjectDescr.hpp>
#include <gui/objects/WorkspaceFolder.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <serial/iterator.hpp>

#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/filename.h>

#include <objmgr/impl/tse_info_object.hpp>

#include "save_smart_objects_dlg.hpp"


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CProjectTask::CProjectTask(IServiceLocator* srv_locator, vector<wxString>& filenames)
:   CAppTask(kEmptyStr, false),
    m_SrvLocator(srv_locator)
{
    _ASSERT(!filenames.empty());
    m_ProjectFilenames = filenames;
}

static vector<int> s_GetUnsavedProjects(CGBWorkspace& ws, const vector<int>& projects)
{
    vector<int> unsaved;
    ITERATE(vector<int>, it, projects) {
        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws.GetProjectFromId(*it));
        if (!doc) continue;

        if (doc->IsDirty()) {
            unsaved.push_back(*it);
            continue;
        }

        wxString path = doc->GetFileName();
        if (!path.empty() && !wxFileName::FileExists(path))
            unsaved.push_back(*it);
    }
    return unsaved;
}

void CProjectTask::UnLoadProjects(IServiceLocator* serviceLocator, const TProjectIdVector& project_ids)
{
    CProjectService* service = serviceLocator->GetServiceByType<CProjectService>().GetPointer();

    CRef<CGBWorkspace> ws = service->GetGBWorkspace();
    if (!ws) return;

    vector<int> toUnload(project_ids.begin(), project_ids.end());

    // collect selected projects that need to be saved
    vector<int> unsaved = s_GetUnsavedProjects(*ws, toUnload);

    bool ok = true;
    if (!unsaved.empty()) {
        // select all
        vector<int> selected;
        for (size_t i = 0; i < unsaved.size(); i++)
            selected.push_back(i);

        // ask user about unsaved projects and saved selected
        ok = x_DoSaveUserSelected(service, *ws, "Unload Project",
            "There are new or modified projects. "
            "If not saved the new projects will be lost and the modified "
            "projects will loose all changes. Would you like to save the selected projects?",
            false, true, unsaved, selected);
    }
    // now unload (remove) the projects
    if (ok) {
        ITERATE(vector<int>, it, toUnload) {
            CRef<CGBDocument> doc(dynamic_cast<CGBDocument*>(ws->GetProjectFromId(*it)));
            if (!doc) continue;

            if (doc->IsLoading()) {
                doc->CancelLoading();
            }
            else if (doc->IsLoaded()) {
                doc->UnloadProject(true);
                if (doc->GetFilename().empty()) { // the user refused to save - remove it completely
                    service->RemoveProject(*doc);
                }
            }
        }
    }
}

void CProjectTask::LoadProjects(IServiceLocator* serviceLocator, const TProjectIdVector& project_ids)
{
    CProjectService* service = serviceLocator->GetServiceByType<CProjectService>().GetPointer();
    CRef<CGBWorkspace> ws = service->GetGBWorkspace();
    if (!ws) return;

    CIRef<CAppTaskService> task_srv = serviceLocator->GetServiceByType<CAppTaskService>();
    _ASSERT(task_srv);

    ITERATE(TProjectIdVector, it, project_ids) {
        CRef<CProjectLoadingTask>
            task(new CProjectLoadingTask(*it, service, false));
        task_srv->AddTask(*task);
    }
}

bool CProjectTask::RemoveProjects(IServiceLocator* serviceLocator, const TProjectIdVector& project_ids, bool confirm)
{
    CProjectService* service = serviceLocator->GetServiceByType<CProjectService>().GetPointer();

    CRef<CGBWorkspace> ws = service->GetGBWorkspace();
    if (!ws) return true;

    vector<int> toRemove;
    copy(project_ids.begin(), project_ids.end(), back_inserter(toRemove));

    // collect selected projects that need to be saved
    vector<int> unsaved = s_GetUnsavedProjects(*ws, toRemove);

    bool ok = true;
    if (!unsaved.empty()) {
        // select all
        vector<int> selected;
        for (size_t i = 0; i < unsaved.size(); i++)
            selected.push_back(i);

        // ask user about unsaved projects and saved selected
        ok = x_DoSaveUserSelected(service, *ws, "Remove - Save changed projects",
            "These projects are new or modified, "
            "all changes will be lost if these projects are not saved. Would you "
            "like to save the selected projects?",
            false, true, unsaved, selected);
    }
    else if (confirm) {
        // simply warn that projects will be removed
        int res = wxMessageBox(wxT("Remove selected projects?"), wxT("Remove Project"),
            wxYES_NO | wxICON_QUESTION);
        ok = (res == wxYES);
    }

    // now remove all projects
    if (ok && !toRemove.empty()) {
        ITERATE(vector<int>, it, toRemove) {
            CRef<CGBDocument> doc(dynamic_cast<CGBDocument*>(ws->GetProjectFromId(*it)));
            if (!doc) continue;
            if (doc->IsLoading())
                doc->CancelLoading();
            service->RemoveProject(*doc);
        }

        for (CTypeIterator<CGBProjectHandle> it(ws->SetWorkspace()); it; ++it) {
            CGBDocument* doc = dynamic_cast<CGBDocument*>(&*it);
            if (!doc) continue;
            return ok;
        }

        CWorkspaceAutoSaver::CleanUp();
    }
    return ok;
}

bool CProjectTask::RemoveAllProjects(IServiceLocator* serviceLocator)
{
    CProjectService* service = serviceLocator->GetServiceByType<CProjectService>().GetPointer();
    CRef<CGBWorkspace> ws = service->GetGBWorkspace();
    if (!ws) return true;

    vector<int> projects;
    for (CTypeIterator<CGBProjectHandle> it(ws->SetWorkspace()); it; ++it) {
        CGBDocument* doc = dynamic_cast<CGBDocument*>(&*it);
        if (!doc) continue;
        projects.push_back(doc->GetId());
    }

    if (RemoveProjects(serviceLocator, projects, false)) {
        CWorkspaceAutoSaver::CleanUp();
        return true;
    }

    return false;
}


void CProjectTask::Save(IServiceLocator* serviceLocator, const TProjectIdVector& project_ids, bool save_as)
{
    CProjectService* service = serviceLocator->GetServiceByType<CProjectService>().GetPointer();

    CRef<CGBWorkspace> ws = service->GetGBWorkspace();
    if (!ws) return;

    vector<int> loaded, selected;

    for (CTypeConstIterator<CGBProjectHandle> it(ws->GetWorkspace()); it; ++it) {
        const CGBDocument* doc = dynamic_cast<const CGBDocument*>(&*it);
        if (!doc) continue;
        else if (doc->IsLoaded()) {
            int id = doc->GetId();
            loaded.push_back(id);

            for (size_t j = 0; j < project_ids.size(); j++) {
                if (project_ids[j] == id)
                    selected.push_back(loaded.size() - 1);
            }
        }
    }

    string title = save_as ? "Save As" : "Save";

    // ask user about unsaved projects and save them
    x_DoSaveUserSelected(service, *ws, title, "", save_as, false,
        loaded, selected);

}

CProjectTask::~CProjectTask()
{
}

CProjectTask::ETaskState CProjectTask::x_Run()
{
    x_OpenProjects();
    return eCompleted;
}

void CProjectTask::x_OpenProjects()
{
    CProjectService* service = m_SrvLocator->GetServiceByType<CProjectService>().GetPointer();

    if (!service->HasWorkspace())
        service->CreateNewWorkspace();

    CRef<CGBWorkspace> ws = service->GetGBWorkspace();
    if (!ws) return;

    CWorkspaceFolder& root_folder = ws->SetWorkspace();

    TProjectIdVector ids;
    for(  size_t i = 0;  i < m_ProjectFilenames.size();  i++ ) {
        wxString filename = m_ProjectFilenames[i];
        if (root_folder.FindProjectByFilename(FnToStdString(filename))) {
            wxString msg = wxString::Format(
                wxT("Project \"%s\" is already opened."), filename.c_str());
            wxMessageBox(msg, wxT("Open Project"), wxOK | wxICON_EXCLAMATION);
        } else {
            CRef<CGBDocument> doc(new CGBDocument(service));
            doc->SetFileName(filename);
            service->AddProject(*doc);
            ids.push_back(doc->GetId());
        }
    }

    if (!ids.empty()) {
        CIRef<CAppTaskService> task_srv = m_SrvLocator->GetServiceByType<CAppTaskService>();

        _ASSERT(task_srv);

        ITERATE(TProjectIdVector, it, ids) {
            CRef<CProjectLoadingTask>
                task(new CProjectLoadingTask(*it, service, true));
            task_srv->AddTask(*task);
        }
    }
}

static wxString AskProjectSaveFilename(const wxString& docTitle, bool as = false)
{
    // generate the default filename
    wxString title = docTitle;
    if (title.length() > 512) {
        title.erase(509);
        title += wxT("...");
    }
    wxString fname = title + wxT(".gbp");

    SWFileDlgData& file_data = CProjectLoadingTask::sm_ProjectDlgData;
    file_data.m_Style = wxFD_SAVE | wxFD_OVERWRITE_PROMPT;
    file_data.SetFilename(fname);
    file_data.m_Title = wxT("Save Project \"") + title + (as ? wxT("\" As") : wxT("\""));

    if (NcbiFileBrowser(CProjectLoadingTask::sm_ProjectDlgData, NULL) == wxID_OK) {
        return CProjectLoadingTask::sm_ProjectDlgData.GetFilename();
    }
    return wxEmptyString;
}

// shows "Save" dialog and save workspace and projects selected by the user
bool CProjectTask::x_DoSaveUserSelected(CProjectService* service,
                                        CGBWorkspace& ws,
                                        const string& title,
                                        const string& description,
                                        bool save_as,
                                        bool no_btn,
                                        const vector<int>& project_ids,
                                        const vector<int>& selected)
{
    // prepare parameters for the dialog
    SSaveProjectsDlgParams params;
    params.m_Description = description;
    params.m_ShowNoButton = no_btn;

    // prepare projects data
    params.m_ProjectIds = project_ids;
    for(  size_t i = 0;  i < selected.size();  i++ )    {
        params.m_ProjectsToSave.Add(selected[i]);
    }

    // create and initialize the dialog
    CSaveDlg dlg(ws, NULL);
    dlg.SetRegistryPath("ApplicationGUI.SaveDlg"); //TODO
    dlg.SetParams(params);
    dlg.SetTitle(ToWxString(title));

    int res = dlg.ShowModal();

    switch(res) {
    case wxID_SAVE:  {
        // save selected
        params = dlg.GetParams();
        bool ok = true;
        for(  size_t i = 0;  ok  && i < params.m_ProjectsToSave.size();  i++)    {
            int index = params.m_ProjectsToSave[i];

            CGBDocument* doc = dynamic_cast<CGBDocument*>(ws.GetProjectFromId(params.m_ProjectIds[index]));
            if (!doc) {
                ok = false;
                continue;
            }
            _ASSERT(doc->IsLoaded());

            wxString filename;
            if (!save_as) {
                filename = doc->GetFileName();
                if (filename.find(wxT("http://")) == 0 || filename.find(wxT("https://")) == 0 || filename.find(wxT("ftp://")) == 0)  {
                    wxString msg = wxT("The project\n") + filename;
                    msg += wxT("\nmust have a local file name in order to save it.");
                    NcbiMessageBoxW(msg);
                    filename.clear();
                }
            }

            if (filename.empty())
                filename = AskProjectSaveFilename(ToWxString(doc->GetDescr().GetTitle()), true);

            if (filename.empty()) {
                ok = false;
                continue;
            }

            try {
                doc->SetFileName(filename);
                doc->Save(filename);
                service->AddToProjectWorkspaceMRUList(filename);
                ERR_POST(Info << "Project saved:" << filename);
            }
            catch (const CException& e)  {
                NcbiErrorBox(e.GetMsg(), "Save Project - Error");
                ok = false;
            }
            catch (const exception& e)  {
                NcbiErrorBox(e.what(), "Save Project - Error");
                ok = false;
            }
        }
        return ok;
    }
    case wxID_NO:
        return true;
    case wxID_CANCEL:
    default:
        return false;
    }
    return false;
}


END_NCBI_SCOPE
