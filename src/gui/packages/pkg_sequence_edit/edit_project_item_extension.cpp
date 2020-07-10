/*  $Id: edit_project_item_extension.cpp 41157 2018-06-02 21:27:27Z kachalos $
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
 */

#include <ncbi_pch.hpp>

#include "edit_project_item_extension.hpp"

#include <gui/objutils/project_item_extra.hpp>

#include <objects/gbproj/ProjectItem.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/framework/app_task_impl.hpp>
#include <gui/framework/workbench_impl.hpp>
#include <gui/packages/pkg_sequence_edit/editingbtnspanel.hpp>
#include <gui/packages/pkg_sequence_edit/discrepancy_dlg.hpp>
#include <gui/core/selection_service_impl.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CSmartProjectItemExtension

string CEditProjectItemExtension::GetExtensionIdentifier() const
{
    static string sid("edit_project_item_extension");
    return sid;
}

string CEditProjectItemExtension::GetExtensionLabel() const
{
    static string slabel("Package Sequence Edit Project item extension");
    return slabel;
}

void CEditProjectItemExtension::ProjectItemAttached(CProjectItem& item, CGBDocument& doc, IServiceLocator* service_locator)
{
    if (item.HasTag("open_discrepancy_dlg")) {
        if (m_DiscrepancyOpen) {
            CAppTaskService* taskService = service_locator->GetServiceByType<CAppTaskService>();
            COpenDiscrepancyTask* task = new COpenDiscrepancyTask(CDiscrepancyDlg::eNormal, dynamic_cast<CGBProjectHandle&>(doc), dynamic_cast<CWorkbench&>(*service_locator));
            taskService->AddTask(*task);
        }
        if (m_MegareportOpen) {
            CAppTaskService* taskService = service_locator->GetServiceByType<CAppTaskService>();
            COpenDiscrepancyTask* task = new COpenDiscrepancyTask(CDiscrepancyDlg::eMega, dynamic_cast<CGBProjectHandle&>(doc), dynamic_cast<CWorkbench&>(*service_locator));
            taskService->AddTask(*task);
        }
        if (m_OnCallerOpen) {
            CAppTaskService* taskService = service_locator->GetServiceByType<CAppTaskService>();
            COpenDiscrepancyTask* task = new COpenDiscrepancyTask(CDiscrepancyDlg::eOncaller, dynamic_cast<CGBProjectHandle&>(doc), dynamic_cast<CWorkbench&>(*service_locator));
            taskService->AddTask(*task);
        }
        if (m_SubmitterOpen) {
            CAppTaskService* taskService = service_locator->GetServiceByType<CAppTaskService>();
            COpenDiscrepancyTask* task = new COpenDiscrepancyTask(CDiscrepancyDlg::eSubmitter, dynamic_cast<CGBProjectHandle&>(doc), dynamic_cast<CWorkbench&>(*service_locator));
            taskService->AddTask(*task);
        }
    }
    if (item.HasTag("open_editing_buttons")) {
        wxWindow* main_window = NULL;
        CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(service_locator);
        if (wb_frame)
            main_window = wb_frame->GetMainWindow();
        CEditingBtnsPanel::GetInstance(main_window, wb_frame);
    }
}

void CEditProjectItemExtension::ProjectItemDetached(CProjectItem& item, CGBDocument& doc, IServiceLocator* service_locator)
{
    if (item.HasTag("open_discrepancy_dlg")) {
        m_DiscrepancyOpen = false;
        m_MegareportOpen = false;
        m_OnCallerOpen = false;
        m_SubmitterOpen = false;
        wxWindowList children = wxTheApp->GetTopWindow()->GetChildren();
        for (wxWindowList::compatibility_iterator it = children.GetFirst(); it; it = it->GetNext()) {
            wxWindow *current = (wxWindow*)it->GetData();
            CDiscrepancyDlg* dlg = dynamic_cast<CDiscrepancyDlg*>(current);
            if (dlg) {
                switch (dlg->GetType()) {
                    case CDiscrepancyDlg::eNormal:
                        m_DiscrepancyOpen = dlg->NeedToReopen();
                        break;
                    case CDiscrepancyDlg::eMega:
                        m_MegareportOpen = dlg->NeedToReopen();
                        break;
                    case CDiscrepancyDlg::eOncaller:
                        m_OnCallerOpen = dlg->NeedToReopen();
                        break;
                    case CDiscrepancyDlg::eSubmitter:
                        m_SubmitterOpen = dlg->NeedToReopen();
                        break;
                }
            }
        }
    }
    if (!item.HasTag("open_editing_buttons")) {
        return;
    }
}

END_NCBI_SCOPE

