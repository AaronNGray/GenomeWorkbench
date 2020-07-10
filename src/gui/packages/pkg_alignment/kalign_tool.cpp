/*  $Id: kalign_tool.cpp 38402 2017-05-04 17:24:36Z evgeniev $
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
* Authors:  Roman Katargin, Vladislav Evgeniev
*
*/

#include <ncbi_pch.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/gui_seq_info.hpp>

#include <gui/packages/pkg_alignment/kalign_tool.hpp>
#include <gui/packages/pkg_alignment/kalign_tool_job.hpp>
#include "kalign_tool_panel.hpp"

#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

///////////////////////////////////////////////////////////////////////////////
/// CKalignTool
CKalignTool::CKalignTool()
:   CAlgoToolManagerBase("Kalign",
                         "",
                         "Create an alignment using Kalign",
                         "Fast multiple sequence alignment tool, concentrating on local regions. Suitable for large alignments.",
                         "MSATools",
                         "Multiple Sequence Aligners"),
                         m_Panel()
{
}

string CKalignTool::GetExtensionIdentifier() const
{
    return "kalign_tool";
}


string CKalignTool::GetExtensionLabel() const
{
    return "Kalign Tool";
}

void CKalignTool::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_Panel = NULL;
}

void CKalignTool::CleanUI()
{
    m_Panel = NULL;
    CAlgoToolManagerBase::CleanUI();
}

void CKalignTool::x_CreateParamsPanelIfNeeded()
{
    if (m_Panel == NULL) {
        x_SelectCompatibleInputObjects();

        m_Panel = new CKalignToolPanel(m_ParentWindow, wxID_ANY,
                                   wxDefaultPosition, wxSize(0, 0), SYMBOL_CKALIGNTOOLPANEL_STYLE, false);
        m_Panel->SetData(m_Params);
        m_Panel->SetObjects(&m_Objects);

        m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_Panel->LoadSettings();
    }
}

bool CKalignTool::x_ValidateParams()
{
    return true;
}

void CKalignTool::x_SelectCompatibleInputObjects()
{
    x_ConvertInputObjects(CSeq_loc::GetTypeInfo(), m_Objects);
}

wxPanel* CKalignTool::GetMaxPanel()
{
    CKalignToolPanel* panel = new CKalignToolPanel();
    panel->Create(m_ParentWindow);
    return panel;
}

CAlgoToolManagerParamsPanel* CKalignTool::x_GetParamsPanel()
{
    return m_Panel;
}

IRegSettings* CKalignTool::x_GetParamsAsRegSetting()
{
    return &m_Params;
}

CDataLoadingAppJob* CKalignTool::x_CreateLoadingJob()
{
    if (m_Panel) m_Params = m_Panel->GetData();
    CKalignToolJob* job = new CKalignToolJob(m_Params);
    return job;
}

IAppTask* CKalignTool::QuickLaunch()
{
    x_SelectCompatibleInputObjects();

    if (m_Objects.empty()) {
        wxMessageBox(wxT("No objects that can be proccessed were selected."),
                     wxT("Kalign Tool Error"), wxOK | wxICON_ERROR);
        return 0;
    }

    TConstScopedObjects objects;

    {{
        TConstScopedObjects genomic, proteins;

        NON_CONST_ITERATE (TObjects, it, m_Objects) {
            NON_CONST_ITERATE(TConstScopedObjects, it2, it->second) {
                CIRef<IGuiSeqInfo> gui_seq_info(CreateObjectInterface<IGuiSeqInfo>(*it2, NULL));
                if( gui_seq_info ) {
                    if (gui_seq_info->IsProtein()) {
                        proteins.push_back(*it2);
                    }
                    else {
                        genomic.push_back(*it2);
                    }
                }
            }
        }
        objects = (proteins.size() > genomic.size()) ? proteins : genomic;
    }}

    if (objects.size() < 2) {
        wxMessageBox(wxT("You have to select at least 2 either genomic or protein locations to run Kalign tool."),
                     wxT("Kalign Tool Error"), wxOK | wxICON_ERROR);
        return 0;
    }

    m_Params.SetObjects() = objects;

    CRef<CDataLoadingAppJob> job(x_CreateLoadingJob());

    CProjectService* prj_srv =
        m_SrvLocator->GetServiceByType<CProjectService>();

    vector<int> projects;
    prj_srv->GetObjProjects(objects, projects);

    CSelectProjectOptions options;
    if (projects.empty())
        options.Set_CreateNewProject();
    else
        options.Set_AddToExistingProject(projects.front());

    CRef<CDataLoadingAppTask> task(new CDataLoadingAppTask(prj_srv, options, *job));
    return task.Release();
}

END_NCBI_SCOPE
