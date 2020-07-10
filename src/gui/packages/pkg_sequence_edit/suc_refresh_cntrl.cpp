/*  $Id: suc_refresh_cntrl.cpp 40279 2018-01-19 17:48:49Z katargir $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/packages/pkg_sequence_edit/suc_refresh_cntrl.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/table_data_suc_job.hpp>
#include <gui/widgets/edit/suc_data_generator.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>  
#include <gui/objutils/execute_lock_guard.hpp>

#include <gui/objects/GBWorkspace.hpp>
#include <gui/core/project_service.hpp>

#include <wx/display.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

BEGIN_EVENT_MAP( CSUCRefreshCntrl::CSUCRefreshCntrlHelper, CEventHandler )
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged, &CSUCRefreshCntrl::CSUCRefreshCntrlHelper::x_OnJobNotification)
END_EVENT_MAP()

void CSUCRefreshCntrl::CSUCRefreshCntrlHelper::LoadData(IWorkbench* wb)
{
    if (m_JobId != CAppJobDispatcher::eInvalidJobID) {
        CAppJobDispatcher::GetInstance().DeleteJob(m_JobId);
        m_JobId = CAppJobDispatcher::eInvalidJobID;
    }

    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = wb->GetServiceByType<CSelectionService>();
    if (!sel_srv) {
        m_Ctrl->OnError("Error");
        return;
    }

    sel_srv->GetActiveObjects(objects);
    if (objects.empty()) 
    {
        GetViewObjects(wb, objects);
    }
    if (objects.empty()){
        m_Ctrl->OnError("No objects");
        return;
    }

    string statusText;

    try {
        CIRef<CProjectService> srv = wb->GetServiceByType<CProjectService>();
        _ASSERT(srv);
        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        CGBDocument* doc = 0;
        if (ws) 
            doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*objects.front().scope));

        CRef<CTableDataSUCJob> job(new CTableDataSUCJob(objects));
        if (doc) 
        {
            job->SetDataLocker(new CWeakExecuteGuard(doc->GetUndoManager()));
        }
        m_JobId = CAppJobDispatcher::GetInstance().StartJob(*job, "ObjManagerEngine", *this);
    } catch( CAppJobException& e ){
        m_JobId = CAppJobDispatcher::eInvalidJobID;
        statusText = "Sort Unique Count failed!";
        LOG_POST( Error
            << "CTableDataSUC failed to start job: "
            << e.GetMsg()
        );
        LOG_POST(e.ReportAll());
    }

    if (!statusText.empty())
        m_Ctrl->OnError(statusText);
}

void CSUCRefreshCntrl::CSUCRefreshCntrlHelper::x_OnJobNotification(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>( evt );
    _ASSERT(notn);
    if (notn->GetJobID() != m_JobId) return;

    string statusText;
    CRef<CSUCResults> res;

    switch (notn->GetState()) {
        case IAppJob::eCompleted:
        {
            CRef<CObject> result = notn->GetResult();
            res.Reset(dynamic_cast<CSUCResults*>(result.GetPointer()));
            if (!res) {
                statusText = "Sort Unique Count failed!";
                break;
            }
            break;
        }
        case IAppJob::eCanceled:
            statusText = "Sort Unique Count canceled!";
            break;
        case IAppJob::eFailed:
            {{
                CConstIRef<IAppJobError> error = notn->GetError();

                string err_msg = "Failed: ";
                if (error)
                    err_msg += error->GetText();
                else
                    err_msg += "Unknown fatal error";

                statusText = err_msg;
            }}
            break;
        default:
            return;
    }

    m_JobId = CAppJobDispatcher::eInvalidJobID;

    if (res)
        m_Ctrl->OnSUCData(*res);
    else 
        m_Ctrl->OnError(statusText);
}


CSUCRefreshCntrl::CSUCRefreshCntrl(IWorkbench* wb)
    : m_Workbench(wb), m_Text(NULL)
{
    m_Helper.Reset(new CSUCRefreshCntrlHelper(this));
    SetRegistryPath("Dialogs.Edit.SortUniqueCount");
}

CSUCRefreshCntrl::CSUCRefreshCntrl()
    : m_Workbench(NULL), m_Text(NULL)
{
    m_Helper.Reset(new CSUCRefreshCntrlHelper(this));
    SetRegistryPath("Dialogs.Edit.SortUniqueCount");
}

CSUCRefreshCntrl::~CSUCRefreshCntrl()
{
    if (m_Text)
        SaveSettings();
}

string CSUCRefreshCntrl::GetRefreshedText()
{
    string text = "";

    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (!sel_srv) return text;
    sel_srv->GetActiveObjects(objects);
    if (objects.empty()) 
    {
        GetViewObjects(m_Workbench, objects);
    }
    if (objects.empty()) return text;

    CRef<CSUCResults> res;
    try {
        res = CSUCDataGenerator::GetSUCResults(objects);
    } catch(const exception&) {
    }

    if (res) {
        res->ExpandAll(true);
        CGenericReportDlg* report = new CGenericReportDlg(NULL);
        report->SetTitle(wxT("Sort Unique Count"));
        size_t num_rows = res->CountLines();
        for (size_t i = 0; i < num_rows; i++) {
            CConstRef<CSUCLine> line = res->GetLine(i);
            if (line) {
                text += "\t";
                text += NStr::NumericToString(line->GetCount());
                text += "\t";
                text += line->GetLine();
                text += "\n";
            }
        }
    }
    return text;
}


void CSUCRefreshCntrl::RefreshText(CGenericPanel* text)
{
    if (!m_Text && text)
    {
        m_Text = text;
        LoadSettings();
    }
    m_Text = text;
    if (!m_Text) {
        return;
    }

    m_Helper->LoadData(m_Workbench);
}

void CSUCRefreshCntrl::OnError(const string error)
{
    m_Text->SetText(ToWxString(error));
}

void CSUCRefreshCntrl::OnSUCData(CSUCResults& sucData)
{
    if (!m_Text)
        return;

    wxString text;
    sucData.GetText(text);
    m_Text->SetText(text);
}

static const char* kFrameWidth = "Frame Width";
static const char* kFrameHeight = "Frame Height";
static const char* kFramePosX = "Frame Position X";
static const char* kFramePosY = "Frame Position Y";

void CSUCRefreshCntrl::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}

void CSUCRefreshCntrl::SaveSettings() const
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

    wxWindow *dlg = m_Text->GetParent();
    view.Set(kFrameWidth, dlg->GetScreenRect().GetWidth());
    view.Set(kFrameHeight, dlg->GetScreenRect().GetHeight());
    view.Set(kFramePosX, dlg->GetScreenPosition().x);
    view.Set(kFramePosY, dlg->GetScreenPosition().y);
}


void CSUCRefreshCntrl::LoadSettings()
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

    wxWindow *dlg = m_Text->GetParent();
    int width = view.GetInt(kFrameWidth, -1);
    int height = view.GetInt(kFrameHeight, -1);
    if (width >= 0  && height >= 0)
        dlg->SetSize(wxSize(width,height));

    int pos_x = view.GetInt(kFramePosX, -1);
    int pos_y = view.GetInt(kFramePosY, -1);

   if (pos_x >= 0  && pos_y >= 0)
   {
       int max_x = 0;
       for (size_t i = 0; i < wxDisplay::GetCount(); i++) // also see gui/widgets/wx/wx_utils.cpp:CorrectWindowRect() for alternative window position validation
       {
        wxDisplay display(i);
        max_x += display.GetGeometry().GetWidth();
       }
       if (pos_x + width > max_x) pos_x = wxGetDisplaySize().GetWidth()-width-5;
       if (pos_y + height > wxGetDisplaySize().GetHeight()) pos_y = wxGetDisplaySize().GetHeight()-height-5;
       
       dlg->SetPosition(wxPoint(pos_x,pos_y));
   }
}

END_NCBI_SCOPE
