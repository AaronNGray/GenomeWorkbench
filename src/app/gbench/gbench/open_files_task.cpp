/*  $Id: open_files_task.cpp 39528 2017-10-05 15:27:37Z katargir $
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

#include <wx/msgdlg.h>

#include "open_files_task.hpp"
#include "gbench_app.hpp"

#include <gui/utils/extension_impl.hpp>
#include <gui/core/app_open_file_ext.hpp>
#include <gui/core/project_task.hpp>
#include <gui/core/loading_app_job.hpp>
#include <gui/core/ui_file_load_manager.hpp>
#include <gui/core/object_loading_task.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/framework/app_mode_extension.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE

DECLARE_APP(ncbi::CGBenchApp)

COpenFilesTask::COpenFilesTask(IWorkbench* workbench, const vector<string>& fileNames, bool bringToFront)
:   CAppTask("Process data from named pipe", false),
    m_Workbench(workbench), m_FileNames(fileNames), m_BringToFront(bringToFront)
{
    _ASSERT(m_Workbench);
}

IAppTask::ETaskState COpenFilesTask::x_Run()
{   
    vector<IAppOpenFileExt*> clients;
    GetExtensionAsInterface(EXT_POINT__GBENCH_OPEN_FILE, clients);

    ITERATE(vector<string>, it1, m_FileNames) {
        bool processed = false;
        const string& fileName = *it1;
        ITERATE(vector<IAppOpenFileExt*>, it, clients) {
            if ((*it)->Open(fileName, m_Workbench)) {
                processed = true;
                break;
            }
        }
        if (processed) continue;

        wxString fname = wxString::FromUTF8(fileName.c_str());
        if (!fname.StartsWith(wxT("http://")) && !fname.StartsWith(wxT("https://")) && !fname.StartsWith(wxT("ftp://")) && !wxFileName::FileExists(fname))
            continue;

        wxString l_fname = fname;
        l_fname.MakeLower();

        if (l_fname.EndsWith(wxT(".gbp"))) {
            vector<wxString> filenames;
            filenames.push_back(fname);
            CProjectTask* task = new CProjectTask(m_Workbench->GetServiceLocator(), filenames);
            m_Workbench->GetAppTaskService()->AddTask(*task);
        }
        else {
            CIRef<IFileFormatLoaderManager> formatManager;
            CFormatGuess::EFormat fmt = CFormatGuess::eUnknown;

            try {
                CNcbiIfstream istr(fname.fn_str(), ios::binary);
                fmt = CFormatGuess(istr).GuessFormat();
            }
            catch (CException&) {
            }

            if (fmt != CFormatGuess::eUnknown) {
                vector< CIRef<IFileFormatLoaderManager> > managers;
                GetExtensionAsInterface("file_format_loader_manager", managers);
                for (size_t i = 0; i < managers.size(); ++i) {
                    IFileFormatLoaderManager* mgr = managers[i].GetPointer();
                    if (mgr->RecognizeFormat(fmt)) {
                        formatManager.Reset(mgr);
                        break;
                    }
                }
            }

            if (formatManager) {
                formatManager->SetServiceLocator(m_Workbench);

                vector<wxString> filenames;
                filenames.push_back(fname);
                formatManager->SetFilenames(filenames);

                CSelectProjectOptions options;
                options.Set_CreateNewProject();

                CIRef<IAppTask> task;
                CIRef<IObjectLoader> loader(dynamic_cast<IObjectLoader*>(formatManager->GetExecuteUnit()));
                if (loader) {
                    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
                    task.Reset(new CObjectLoadingTask(srv, *loader, options));
                    vector<IAppModeExtension*> appModes;
                    GetExtensionAsInterface(EXT_POINT__APP_MODE_EXTENSION, appModes);
                    for (auto ext : appModes) {
                        if (ext->GetModeName() == "indexer") {
                            CObjectLoadingTask* newtask = new CObjectLoadingTask(srv, *loader, options);
                            newtask->AddProjectItemExtra("SmartFile", string(fname.ToUTF8()));
                            task.Reset(newtask);
                            break;
                        }
                    }
                }
                else {
                    task.Reset(formatManager->GetTask());
                    CDataLoadingAppTask* dlTask = dynamic_cast<CDataLoadingAppTask*>(task.GetPointerOrNull());
                    if (dlTask)
                        dlTask->SetOptions(options);
                }

                if (task)
                    m_Workbench->GetAppTaskService()->AddTask(*task);
            }
        }
    }

    if (m_BringToFront) {
        CGBenchApp& app = wxGetApp();
        wxTopLevelWindow* win = (wxTopLevelWindow*)app.GetTopWindow();
        if(win) {
            if (win->IsIconized())
                win->Maximize(false);
            win->RequestUserAttention();
        }
    }

    return eCompleted;
}

END_NCBI_SCOPE
