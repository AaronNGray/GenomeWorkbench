/*  $Id: select_project_options.cpp 39892 2017-11-20 18:31:11Z katargir $
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
 * Authors:  Andrey Yazhuk, Liangshou Wu
 *
 */

#include <ncbi_pch.hpp>

#include <gui/core/select_project_options.hpp>
#include <gui/core/project_selector_dlg.hpp>
#include <gui/core/document.hpp>
#include <gui/core/project_service.hpp>
#include <gui/objects/GBWorkspace.hpp>
#include <objects/gbproj/ProjectDescr.hpp>
#include <objects/gbproj/GBProject_ver2.hpp>
#include <gui/objects/WorkspaceFolder.hpp>
#include <objects/general/Date.hpp>

#include <objmgr/object_manager.hpp>

#include <serial/iterator.hpp>
#include <gui/utils/id_generator.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CSelectProjectOptions
CSelectProjectOptions::CSelectProjectOptions()
:   m_Action(eInvalidAction), m_NewProjectName("New Project")
{
}


void CSelectProjectOptions::Set_DecideLater()
{
    m_Action = eDecideLater;
}


void CSelectProjectOptions::Set_CreateNewProject(const string& folder)
{
    m_Action = eCreateNewProject;
    m_TargetProjectId = CGBDocument::GetNextId();//GenerateNewId();
    m_FolderName = folder;
}


void CSelectProjectOptions::Set_CreateSeparateProjects()
{
    m_Action = eCreateSeparateProjects;
    m_FolderName = "";
}


void CSelectProjectOptions::Set_AddToExistingProject(TProjectId& project_id,
                                                   const string& folder)
{
    m_Action = eAddToExistingProject;
    m_TargetProjectId = project_id;
    m_FolderName = folder;
}


CSelectProjectOptions::EAction CSelectProjectOptions::GetAction()
{
    return m_Action;
}


CSelectProjectOptions::TProjectId CSelectProjectOptions::GetTargetProjectId()
{
    return m_TargetProjectId;
}


string CSelectProjectOptions::GetFolderName()
{
    return m_FolderName;
}

bool CSelectProjectOptions::AddItemsToWorkspace(CProjectService* service, const TData& data)
{
    TItems items;
    TLoaders loaders;
    ITERATE(TData, it, data) {
        if (it->first) loaders.push_back(it->first);
        ITERATE(TItems, it2, it->second)
            items.push_back(*it2);
    }

    if (m_Action == eDecideLater) {
        if (!x_ShowSelectProjectDialog(service, items))
            return false;
    }

    switch(m_Action) {
    case eCreateNewProject:
        x_CreateOneProject(service, items, loaders);
        return true;

    case CSelectProjectOptions::eCreateSeparateProjects:
        x_CreateSeparateProjects(service, data);
        return true;

    case CSelectProjectOptions::eAddToExistingProject:   {
        x_AddToExistingProject(service, items, loaders);
        return true;
    }
    case eDecideLater:
        break;
    default:
        _ASSERT(false);
    }
    return false;
}

bool CSelectProjectOptions::x_ShowSelectProjectDialog(CProjectService* service, TItems& items)
{
    CRef<CScope> default_scope( new CScope(*CObjectManager::GetInstance()));
    default_scope->AddDefaults();

    TConstScopedObjects objects;
    for(  size_t i = 0;  i < items.size();  i++ )   {
        const CProjectItem& item = *items[i];
        objects.push_back(SConstScopedObject(item.GetObject(), default_scope));
    }

    CProjectSelectorDlg dlg(NULL, wxID_ANY, wxT("Select project for items"));

    dlg.SetProjectService(service);
    dlg.SetObjects(objects);
    dlg.SetRegistryPath("Dialogs.DataLoadingTask.ProjectSelector");
    if (dlg.ShowModal() != wxID_OK)
        return false;

    SProjectSelectorParams params;
    dlg.GetParams(params);
    params.ToLoadingOptions(*this);

    return true;
}

void CSelectProjectOptions::x_AddToExistingProject(CProjectService* service, TItems& items, const TLoaders& loaders)
{
    CRef<CGBWorkspace> ws = service->GetGBWorkspace();
    if (!ws) return;

	CGBProjectHandle* project = ws->GetProjectFromId(m_TargetProjectId);
	if (!project) {
		LOG_POST(Error << "AddItemsToProject: project " << m_TargetProjectId << " doesn't exist");
		return;
	}

    CGBDocument& doc = dynamic_cast<CGBDocument&>(*project);
    doc.AddItems(m_FolderName, items, loaders);
}

static void s_InitProjectnameGenerator(CGBWorkspace& ws, CUniqueLabelGenerator& projectNames)
{
    const CWorkspaceFolder& root = ws.GetWorkspace();
    for (CTypeConstIterator<CGBProjectHandle> it(root);  it;  ++it) {
        const string& s = it->GetTitle();
        projectNames.AddExistingLabel(s);
    }
}

CGBDocument* s_CreateProject(CProjectService* service, CGBWorkspace& ws, const string& prjName, const string& prjDesc)
{
	CGBProject_ver2* prj_obj = new CGBProject_ver2();
	CRef<CGBDocument> doc(new CGBDocument(service, CGBDocument::GetNextId(), *prj_obj));

	doc->CreateProjectScope();

	// initialize project data
    CProjectDescr& descr = doc->SetProject().SetDescr();
	doc->SetDescr().SetTitle(prjName);
	descr.SetComment(prjDesc);

    CTime time(CTime::eCurrent);
    descr.SetCreateDate(time);
    descr.SetModifiedDate(time);

    return doc.Release();
}

void CSelectProjectOptions::x_CreateOneProject(CProjectService* service, TItems& items, const TLoaders& loaders)
{
    if (!service->HasWorkspace())
        service->CreateNewWorkspace();
    CRef<CGBWorkspace> ws = service->GetGBWorkspace();

    CUniqueLabelGenerator projectNames;
    s_InitProjectnameGenerator(*ws, projectNames);
	string projectName = projectNames.MakeUniqueLabel(m_NewProjectName);

	CRef<CGBDocument> doc(s_CreateProject(service, *ws, projectName, m_NewProjectName));

    doc->AddItems(m_FolderName, items, loaders);
    service->AddProject(*doc);
}

/// for every Project Item in m_Items creates a new project,
/// adds the item to the project and loads the data
void CSelectProjectOptions::x_CreateSeparateProjects(CProjectService* service, const TData& data)
{
    if (!service->HasWorkspace())
        service->CreateNewWorkspace();
    CRef<CGBWorkspace> ws = service->GetGBWorkspace();

    CUniqueLabelGenerator projectNames;
    s_InitProjectnameGenerator(*ws, projectNames);

    ITERATE(TData, it, data) {
        if (!it->second.empty()) {
            ITERATE(TItems, it2, it->second) {
                string projectName = projectNames.MakeUniqueLabel(m_NewProjectName);
                projectNames.AddExistingLabel(projectName);
                CRef<CGBDocument> doc(s_CreateProject(service, *ws, projectName, m_NewProjectName));

                TItems items;
                items.push_back(*it2);

                TLoaders loaders;
                if (it->first) {
                    CRef<CLoaderDescriptor> loader(new CLoaderDescriptor());
                    loader->Assign(*it->first);
                    loaders.push_back(loader);
                }

                doc->AddItems("", items, loaders);
                service->AddProject(*doc);
            }
        }
        else {
            if (it->first) {
                string projectName = projectNames.MakeUniqueLabel(m_NewProjectName);
                projectNames.AddExistingLabel(projectName);
                CRef<CGBDocument> doc(s_CreateProject(service, *ws, projectName, m_NewProjectName));

                TItems items;
                TLoaders loaders;
                CRef<CLoaderDescriptor> loader(new CLoaderDescriptor());
                loader->Assign(*it->first);
                loaders.push_back(loader);
                doc->AddItems("", items, loaders);
                service->AddProject(*doc);
            }
        }
	}
}

END_NCBI_SCOPE
