/*  $Id: smart_file_text_toolbar.cpp 41112 2018-05-24 18:32:57Z katargir $
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

#include "smart_file_text_toolbar.hpp"

#include <objects/submit/Seq_submit.hpp>

#include <gui/core/project_service.hpp>
#include <gui/core/document.hpp>
#include <gui/core/project_task.hpp>
#include <gui/core/ws_auto_saver.hpp>
#include <gui/core/select_project_options.hpp>
#include <gui/core/object_loading_task.hpp>

#include <objects/gbproj/ProjectDescr.hpp>
#include <gui/objects/WorkspaceFolder.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/file_extensions.hpp>

#include <gui/widgets/seq/text_panel.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/grid_widget/grid_event.hpp>
#include <gui/widgets/loaders/asn_object_loader.hpp>

#include <gui/widgets/edit/table_data_validate_job.hpp>
#include <gui/widgets/edit/table_data_validate.hpp>
#include <gui/objutils/execute_lock_guard.hpp>
#include <gui/objutils/project_item_extra.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/object_loader.hpp>
#include <gui/framework/workbench_impl.hpp>
#include <gui/packages/pkg_sequence_edit/validate_frame.hpp>
#include <gui/packages/pkg_sequence_edit/discrepancy_dlg.hpp>
#include <gui/framework/app_task_service.hpp>

#include <wx/aui/aui.h>
#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h> 
#include <wx/filename.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// class CSmartFileTextToolbar

CSmartFileTextToolbar::CSmartFileTextToolbar(wxWindow* parent, 
                                     IProjectView& view,
                                     IServiceLocator* service_locator,
                                     SConstScopedObject validateObj,
                                     const wxString& fullPathName)
    : wxAuiToolBar(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT|wxBORDER_NONE),
      m_View(view), m_ServiceLocator(service_locator), m_ValidateObj(validateObj),  m_JobId(CAppJobDispatcher::eInvalidJobID), m_ValidationInProgress(false),
      m_EventProxy(new CEventProxy(this))
{

    m_MegaButton = dynamic_cast<wxButton*>(AddControl(new wxButton(this, ID_TB_MegaButton, "Mega Report", wxDefaultPosition, wxDefaultSize, 0))->GetWindow());
    m_ValidateButton = dynamic_cast<wxButton*>(AddControl(new wxButton(this, ID_TB_ValidateButton, "Validate", wxDefaultPosition, wxDefaultSize, 0))->GetWindow());
    m_SaveButton = dynamic_cast<wxButton*>(AddControl(new wxButton(this, ID_TB_SaveButton, "Save", wxDefaultPosition, wxDefaultSize, 0))->GetWindow());
    m_SaveAsButton = dynamic_cast<wxButton*>(AddControl(new wxButton(this, ID_TB_SaveAsButton, "SaveAs", wxDefaultPosition, wxDefaultSize, 0))->GetWindow());
    m_CloseButton = dynamic_cast<wxButton*>(AddControl(new wxButton(this, ID_TB_CloseButton, "Close", wxDefaultPosition, wxDefaultSize, 0))->GetWindow());

    // Initialize path and name from fullPathName parameter
    wxFileName file(fullPathName);
    m_FilePath = file.GetPath();
    m_FileName = file.GetFullName();
}

CSmartFileTextToolbar::~CSmartFileTextToolbar()
{
    m_EventProxy->ResetToolbar();

    if (m_JobId != CAppJobDispatcher::eInvalidJobID)
        CAppJobDispatcher::GetInstance().DeleteJob(m_JobId);
}

BEGIN_EVENT_TABLE( CSmartFileTextToolbar, wxAuiToolBar )
    EVT_UPDATE_UI(ID_TB_MegaButton, CSmartFileTextToolbar::OnUpdateValidateBtn)
    EVT_UPDATE_UI(ID_TB_ValidateButton, CSmartFileTextToolbar::OnUpdateValidateBtn)
    EVT_UPDATE_UI(ID_TB_SaveButton, CSmartFileTextToolbar::OnUpdateSaveBtn)
    EVT_UPDATE_UI(ID_TB_SaveAsButton, CSmartFileTextToolbar::OnUpdateSaveAsBtn)
    EVT_UPDATE_UI(ID_TB_CloseButton, CSmartFileTextToolbar::OnUpdateCloseBtn)

    EVT_BUTTON(ID_TB_MegaButton, CSmartFileTextToolbar::OnMegaBtn)
    EVT_BUTTON(ID_TB_ValidateButton, CSmartFileTextToolbar::OnValidateBtn)
    EVT_BUTTON(ID_TB_SaveButton, CSmartFileTextToolbar::OnSaveBtn)
    EVT_BUTTON(ID_TB_SaveAsButton, CSmartFileTextToolbar::OnSaveAsBtn)
    EVT_BUTTON(ID_TB_CloseButton, CSmartFileTextToolbar::OnCloseBtn)
END_EVENT_TABLE()

BEGIN_EVENT_MAP(CSmartFileTextToolbar::CEventProxy, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged, &CSmartFileTextToolbar::CEventProxy::OnJobNotification)
END_EVENT_MAP()

void CSmartFileTextToolbar::CEventProxy::OnJobNotification(CEvent* evt)
{
    if (m_Toolbar) m_Toolbar->OnJobNotification(evt);
}

void CSmartFileTextToolbar::OnUpdateValidateBtn(wxUpdateUIEvent& event)
{
    event.Enable(m_ValidateObj.object && !m_ValidationInProgress);
}

void CSmartFileTextToolbar::OnUpdateSaveBtn(wxUpdateUIEvent& event)
{
    event.Enable((!m_FilePath.IsEmpty() || !m_FileName.IsEmpty()) 
                 && !m_ValidationInProgress);
}

void CSmartFileTextToolbar::OnUpdateSaveAsBtn(wxUpdateUIEvent& event)
{
    event.Enable(!m_ValidationInProgress);
}

void CSmartFileTextToolbar::OnUpdateCloseBtn(wxUpdateUIEvent& event)
{
    event.Enable(!m_ValidationInProgress);
}

static const string sValidateViewType("Validate Table View");

void CSmartFileTextToolbar::OnValidateBtn(wxCommandEvent&)
{
    RunValidateView();
}

void CSmartFileTextToolbar::OnMegaBtn(wxCommandEvent&)
{
    CDiscrepancyDlg::OpenDiscrepancyDlg(CDiscrepancyDlg::eMega, *m_ValidateObj.scope, dynamic_cast<CWorkbench*>(m_ServiceLocator), m_FilePath);
}

void CSmartFileTextToolbar::RunValidateView()
{
    CRef<CUser_object> params(new CUser_object());
    CTableDataValidateParams::SetDoAlignment(*params, true);
    CTableDataValidateParams::SetDoInference(*params, false);

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_ServiceLocator);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CValidateFrame::GetInstance(main_window, wb_frame, params);
}

void CSmartFileTextToolbar::OnSaveBtn(wxCommandEvent&)
{
    CTextPanel* textPanel = dynamic_cast<CTextPanel*>(m_View.GetWindow());
    if (!textPanel) return;

    const CSerialObject* so = dynamic_cast<const CSerialObject*>(textPanel->GetOrigObject());
    if (!so) return;

    if (!m_FilePath.IsEmpty() && !m_FileName.IsEmpty()) {
        wxFileName fname(m_FilePath, m_FileName);
        x_Save(*so, fname.GetFullPath());

        CProjectService* projectService = m_ServiceLocator->GetServiceByType<CProjectService>();
        if (!projectService) return;
        CRef<CGBWorkspace> ws = projectService->GetGBWorkspace();
        if (!ws) return;
        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromId(m_View.GetProjectId()));
        if (doc) doc->SetDataModified(false);
    }
}

void CSmartFileTextToolbar::OnSaveAsBtn(wxCommandEvent& event)
{
    CTextPanel* textPanel = dynamic_cast<CTextPanel*>(m_View.GetWindow());
    if (!textPanel) return;

    const CSerialObject* so = dynamic_cast<const CSerialObject*>(textPanel->GetOrigObject());
    if (!so) return;

    wxString filePath = x_AskForFileName();
    if (!filePath.IsEmpty())
        x_Save(*so, filePath);
}

string CSmartFileTextToolbar::x_GetProject_Path(int projid)
{
    string path;

    CProjectService* projectService = m_ServiceLocator->GetServiceByType<CProjectService>();
    if (!projectService) return path; 
    if (!projectService->HasWorkspace())
        projectService->CreateNewWorkspace();

    CRef<CGBWorkspace> ws = projectService->GetGBWorkspace();
    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromId(projid));
    if (!doc) return path;

    CProjectFolder& data_folder = doc->SetData();
    if (data_folder.CanGetItems()) {
        ITERATE(CProjectFolder::TItems, bit, data_folder.GetItems()) {
            int pitemId = (*bit)->GetId();
            CProjectItem* item = data_folder.FindProjectItemById(pitemId);
            string filePath;
            if (item) {
                CProjectItemExtra::GetStr(*item, "OriginalPath", filePath);
                wxFileName fullpath(filePath);
                path = fullpath.GetPath();
                break;
            }
        }
    }

    return path;
}

void CSmartFileTextToolbar::OnCloseBtn(wxCommandEvent& event)
{
    int projectId = m_View.GetProjectId();
    if (projectId < 0)
	return;
//    CProjectTask::TProjectIdVector toRemove(1, projectId);    
//    CProjectTask::RemoveProjects(m_ServiceLocator, toRemove, false);
    CProjectService* service = m_ServiceLocator->GetServiceByType<CProjectService>().GetPointer();
    CRef<CGBWorkspace> ws = service->GetGBWorkspace();
    if (!ws) 
	return;
    CRef<CGBDocument> doc(dynamic_cast<CGBDocument*>(ws->GetProjectFromId(projectId)));
    if (!doc) 
	return;
    if (doc->IsLoading())
	doc->CancelLoading();
    service->RemoveProject(*doc);

    for (CTypeIterator<CGBProjectHandle> it(ws->SetWorkspace()); it; ++it) {
	CGBDocument* doc = dynamic_cast<CGBDocument*>(&*it);
	if (!doc) continue;
	return;
    }
    CWorkspaceAutoSaver::CleanUp();
}

wxString CSmartFileTextToolbar::x_AskForFileName()
{
    wxString cdir = wxGetCwd();
    if (!m_FilePath.IsEmpty()) {
        cdir = m_FilePath;
    }

    wxFileDialog dlg(this, wxT("Select a file"), cdir, wxT(""),
        CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
        CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() != wxID_OK)
        return wxEmptyString;

    return dlg.GetPath();
}

void CSmartFileTextToolbar::x_Save(const CSerialObject& so, const wxString& fileName)
{
    string errMsg;
    try {
        CNcbiOfstream os(fileName.fn_str());
        os << MSerial_AsnText << so;
    }
    catch (const CException& e) {
        errMsg = e.GetMsg();
    }
    catch (const exception& e) {
        errMsg = e.what();
    }

    if (!errMsg.empty()) {
        wxMessageBox(wxT("File: ") + fileName + wxT("\nError: ") + ToWxString(errMsg),
            wxT("Failed to save file"), wxOK | wxICON_ERROR);
    }
}

void CSmartFileTextToolbar::ReallyDone()
{
    CProjectService* projectService = m_ServiceLocator->GetServiceByType<CProjectService>();
    if (!projectService) return; 


    CRef<CGBWorkspace> ws = projectService->GetGBWorkspace();
    if (!ws) return;

    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromId(m_View.GetProjectId()));
    if (!doc) return;

    string errMsg;
    try {
        doc->Save(doc->GetFileName());
    } catch (const CException& ex) {
        errMsg = ex.GetMsg();
    } catch (const exception& ex) {
        errMsg = ex.what();
    }
    if (!errMsg.empty()) {
        wxMessageBox(errMsg, wxT("Smart client communication error"), wxOK|wxICON_ERROR);
        return;
    }
}

void CSmartFileTextToolbar::OnJobNotification(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>( evt );
    _ASSERT(notn);
    if (notn->GetJobID() != m_JobId) return;
    m_ValidationInProgress = false;
    m_MegaButton->Enable();
    m_ValidateButton->Enable();
    m_SaveButton->Enable();
    m_SaveAsButton->Enable();
    m_CloseButton->Enable();

    switch (notn->GetState()) 
    {
    case IAppJob::eCompleted:
    {
        CRef<CObject> result = notn->GetResult();
        CRef<CObjectFor<objects::CValidError::TErrs> > errs(dynamic_cast<CObjectFor<CValidError::TErrs>*>(result.GetPointer()));
        
        if (errs && errs->GetData().size() != 0) 
        {
            CRef<CTableDataValidate> table(new CTableDataValidate(errs, m_ValidateObj.scope));
            if (table)
            {
                size_t info = table->CountBySeverity(eDiag_Info);
                size_t warn = table->CountBySeverity(eDiag_Warning);
                size_t error = table->CountBySeverity(eDiag_Error);
                size_t reject = table->CountBySeverity(eDiag_Critical);
                if (info != 0 || warn != 0 || error != 0 || reject != 0)
                {
                    string msg = "INFO: " + NStr::NumericToString(info) + " WARNING: " + NStr::NumericToString(warn) +
                        " ERROR: " + NStr::NumericToString(error) + " REJECT: " + NStr::NumericToString(reject);
                    wxMessageDialog dlg(this,  wxT("Submission failed validation tests with:\n") + ToWxString(msg),  _("Validation failed"), wxYES_NO | wxCANCEL | wxICON_QUESTION);
                    dlg.SetYesNoLabels(_("Continue"), _("Review errors"));
                    switch (dlg.ShowModal()) 
                    {
                    case wxID_YES : break;
                    case wxID_NO  : {m_JobId = CAppJobDispatcher::eInvalidJobID; RunValidateView(); return;} break;
                    case wxID_CANCEL : {m_JobId = CAppJobDispatcher::eInvalidJobID; return;} break;
                    default : break;
                    }
                }
            }
        }
        break;
    }
    case IAppJob::eCanceled:
        break;
    case IAppJob::eFailed:
    {
        CConstIRef<IAppJobError> error = notn->GetError();
        string err_msg = "Failed: ";
        if (error)
            err_msg += error->GetText();
        else
            err_msg += "Unknown fatal error";
        
        wxMessageDialog dlg(this, ToWxString(err_msg),  _("Validation failed"), wxOK| wxCANCEL |wxICON_ERROR);
        dlg.SetOKLabel(_("Continue"));
        if (dlg.ShowModal() != wxID_OK)
        {
            m_JobId = CAppJobDispatcher::eInvalidJobID; 
            return;
        }
    }
    break;
    default:
        break;
    }

    m_JobId = CAppJobDispatcher::eInvalidJobID;
    ReallyDone();
}

END_NCBI_SCOPE

