/*  $Id: task_view.cpp 43619 2019-08-09 16:32:36Z katargir $
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
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/framework/task_view.hpp>

#include <gui/framework/task_details_dlg.hpp>
#include <gui/framework/view_manager_service.hpp> //TODO
#include <gui/framework/workbench.hpp>
#include <gui/framework/app_task_impl.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/data/object_contrib.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/utils/extension_impl.hpp>


#include <wx/artprov.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include <wx/statline.h>


BEGIN_NCBI_SCOPE

/// register the factory
static CExtensionDeclaration
    decl("view_manager_service::view_factory", new CTaskViewFactory());

///////////////////////////////////////////////////////////////////////////////
/// CTaskView

CTaskViewModel::CTaskViewModel(CAppTaskService* manager)
:   m_TaskService(manager)
{
    m_Names[eDescription] = "Description";
    m_Names[eState] = "State";
    m_Names[eStatus] = "Status";
    m_Names[eTime] = "Time";

    m_TimeFormat.SetFormat("b D, H:m:s p");

    m_Filter.resize(IAppTask::eLastState, true);
}


int CTaskViewModel::GetNumRows() const
{
    return (int)m_Records.size();
}


int CTaskViewModel::GetNumColumns() const
{
    return eMaxCols;
}


string  sToUserFriendlyString(const CTimeSpan& span)
{
    long sec = span.GetCompleteSeconds();
    const char* fmt = "$d days $h hrs $m min $h sec";
    if(sec < 60)    {
        fmt = "$s sec";
    } else if(sec < 3600)   {
        fmt = "$m min $s sec";
    } else if(sec < 3600 * 24)  {
        fmt = "$h hrs $m min $m sec";
    }
    CTimeFormat format(fmt, CTimeFormat::eNcbi);
    string s = span.AsString(format);
    return s;
}


wxVariant CTaskViewModel::GetValueAt(int row, int col) const
{
    TRecordRef rec = m_Records[row];
    IAppTask& task = *rec->m_Task;

    switch (col) {
    case eDescription: {
        string s = task.GetDescr();
        return wxString::FromUTF8(s.c_str());
    }
    case eState: {
        IAppTask::ETaskState state = task.GetState();
        string s = CAppTask::StateToString(state);
        return ToWxString(s);
    }
    case eStatus: {
        string s = task.GetStatusText();
        return wxString::FromUTF8(s.c_str());
    }
    case eTime: {
        if(task.GetState() != IAppTask::eInvalid)   {
            CTimeSpan span;
            CTime t2(CTime::eCurrent, CTime::eGmt);
            CTime t1(rec->m_StartTime);
            span = t2 - t1;
            string str = sToUserFriendlyString(span);
            return ToWxString(str);
        } else {
            return wxT("");
        }
    }
    default:
        _ASSERT(false);
        return wxT("");
    }
    return wxT("Error");
}


wxString CTaskViewModel::GetColumnType( int col ) const
{
    return wxT("string");
}


wxString CTaskViewModel::GetColumnName(int col) const
{
    return ToWxString(m_Names[col]);
}


void CTaskViewModel::ShowTasksByState(IAppTask::ETaskState state, bool show, bool up)
{
    m_Filter[state] = show;
    if(up)  {
        UpdateTable();
    }
}


bool CTaskViewModel::IsStateShown(IAppTask::ETaskState state)
{
    return state == IAppTask::eInvalid ? true : m_Filter[state];
}


CTaskViewModel::TTaskRef CTaskViewModel::GetTask(int row)
{
    return m_Records[row]->m_Task;
}


void CTaskViewModel::UpdateTable()
{
    m_Records.clear();

    if(m_TaskService)   {
        TRecRefVec recs;
        m_TaskService->GetPendingTasks(recs);
        x_AddTasks(recs);

        recs.clear();
        m_TaskService->GetBackgroundedTasks(recs);
        x_AddTasks(recs);
    }
    x_FireDataChanged();
}


void CTaskViewModel::x_AddTasks(TRecRefVec& recs)
{
    time_t curr = CTime(CTime::eCurrent).GetTimeT();

    for( size_t i = 0;  i < recs.size();  i++ )   {
        TRecordRef rec = recs[i];
        IAppTask& task = *rec->m_Task;
        if(task.IsVisible())   {
            // don't show task before it is "mature" enough 
            // to avoid excessive blinking for very quick jobs
            int display_delay = task.GetStatusDisplayDelay();
            if (display_delay > 0) {
                int time_diff = curr - rec->m_StartTime; 
                if (time_diff < display_delay) { 
                    continue;
                }
            }
            int state = task.GetState();

            _ASSERT(state >= 0  &&  state < (int) m_Filter.size());

            if(state == IAppTask::eInvalid  ||  m_Filter[state])  {
                m_Records.push_back(rec);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CTaskTablePanel
#define ID_TABLE    10001
#define ID_TOOLBAR  10002

static int kEventTimerID = 1;
static int kTimerPeriod = 5000;

BEGIN_EVENT_TABLE(CTaskTablePanel, wxPanel)
    EVT_CONTEXT_MENU(CTaskTablePanel::OnContextMenu)

#ifdef __WXMAC__
    EVT_BUTTON(eCmdDetails, CTaskTablePanel::OnDetails)
#endif
    // Needed for context menu AND (if not mac) toolbar
    EVT_MENU(eCmdDetails, CTaskTablePanel::OnDetails)

    EVT_MENU(eCmdCancel, CTaskTablePanel::OnCancel)
    EVT_UPDATE_UI(eCmdDetails, CTaskTablePanel::OnUpdateDetails)
    EVT_UPDATE_UI(eCmdCancel, CTaskTablePanel::OnUpdateCancel)

    EVT_LIST_ITEM_ACTIVATED(ID_TABLE, CTaskTablePanel::OnItemActivated)
    EVT_TIMER(kEventTimerID, CTaskTablePanel::OnEventTimer)
END_EVENT_TABLE()


CTaskTablePanel::CTaskTablePanel()
:   wxPanel(),
    m_TaskService(NULL),
    m_EventTimer(this, kEventTimerID),
    m_Model(NULL),
    m_Table(NULL)
{
    Init();
}


CTaskTablePanel::~CTaskTablePanel()
{
    m_EventTimer.Stop();
}


void CTaskTablePanel::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
#ifdef __WXOSX_COCOA__ // GB-8581
    SetBackgroundStyle(wxBG_STYLE_COLOUR);
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
#endif

    wxPanel::Create(parent, id, pos, size);
    CreateControls();
}


void CTaskTablePanel::Init()
{
    m_Model = NULL;
    m_Table = NULL;
}


void CTaskTablePanel::CreateControls()
{
    wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);

    // Create tool bar
    wxToolBar* tool_bar = new wxToolBar(this, ID_TOOLBAR, wxDefaultPosition, wxDefaultSize,
                                        wxTB_FLAT | wxTB_HORIZONTAL |  wxTB_TEXT |wxTB_NOICONS | wxTB_HORZ_LAYOUT );
#ifdef __WXMAC__
    wxButton* b = new wxButton(tool_bar, eCmdDetails, _("Details"),
        wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    b->SetToolTip(wxT("Show event details"));
    tool_bar->AddControl(b);
#else
    wxBitmap bm_empty;
    tool_bar->AddTool(eCmdDetails, wxT("Details"), bm_empty, wxT("Show event details"));
#endif

    tool_bar->Realize();
    sizer->Add(tool_bar, 0, wxEXPAND);

    // separation line before the table
    wxStaticLine* line = 
        new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);

    sizer->Add(line, 0, wxEXPAND);

    // Create Table
    long style = wxLC_REPORT | wxLC_VIRTUAL | wxBORDER_NONE;
    
    m_Table = 
        new CwxTableListCtrl(this, ID_TABLE, wxDefaultPosition, wxDefaultSize, style);
    WindowSetText(*m_Table, wxT("task_TABLE"));

    m_Table->SetSingleStyle(wxLC_SINGLE_SEL);
    m_Table->SetContextMenuEventPropagation(true);
    sizer->Add(m_Table, 1, wxEXPAND);
}


void CTaskTablePanel::CreateModel(CAppTaskService* task_srv)
{
    _ASSERT(! m_Model);

    if(task_srv) {
        m_TaskService = task_srv;
        m_Model = new CTaskViewModel(m_TaskService);
        m_Table->SetModel(m_Model);

        m_Table->SetColumnWidth(CTaskViewModel::eDescription, 250);
        m_Table->SetColumnWidth(CTaskViewModel::eState, 140);
        m_Table->SetColumnWidth(CTaskViewModel::eStatus, 250);
        m_Table->SetColumnWidth(CTaskViewModel::eTime, 100);

        m_Model->UpdateTable();

        m_EventTimer.Start(kTimerPeriod);
    }
}


void CTaskTablePanel::DestroyModel()
{
    m_EventTimer.Stop();

    m_Table->RemoveModel();

    delete m_Model;
    m_Model = NULL;
}


void CTaskTablePanel::UpdateTable()
{
    // save selection
    vector<TTaskRef> tasks;
    x_GetSelectedTasks(tasks);

    m_Model->UpdateTable();

    // restore selection (to preserve it)
    x_SetSelectedTasks(tasks);

    // Refresh(); //TODO
}


void CTaskTablePanel::x_GetSelectedTasks(vector<TTaskRef>& tasks)
{
    if(m_Model) {
        vector<int> rows;
        m_Table->GetDataRowsSelected(rows);

        for(  size_t i = 0;  i < rows.size();  i++ )    {
            TTaskRef task = m_Model->GetTask(rows[i]);
            tasks.push_back(task);
        }
    }
}


void CTaskTablePanel::x_SetSelectedTasks(vector<TTaskRef>& tasks)
{
    if(m_Model) {
        vector<int> rows;

        int n = m_Model->GetNumRows();
        for(  int row = 0;  row < n;  row++  )   {
            TTaskRef task = m_Model->GetTask(row);

            vector<TTaskRef>::iterator it = std::find(tasks.begin(), tasks.end(), task);
            if(it != tasks.end())   {
                rows.push_back(row);
            }
        }


        m_Table->SetDataRowsSelected(rows);
    }
}

void CTaskTablePanel::AppendMenuItems( wxMenu& aMenu )
{
    aMenu.Append( wxID_SEPARATOR, wxT("Actions") );
    aMenu.Append(eCmdCancel, wxT("Cancel Task"));

    aMenu.Append( wxID_SEPARATOR, wxT("Settings") );
    aMenu.Append( eCmdDetails, wxT("Details") );
}

void CTaskTablePanel::OnContextMenu(wxContextMenuEvent& anEvent)
{
    wxMenu* menu;

    void* data = anEvent.GetClientData();
    menu = data ? (wxMenu*)data : new wxMenu();

    AppendMenuItems( *menu );

    if( /*ShouldPropagate()*/ false ){
        anEvent.SetClientData( menu );
        anEvent.Skip();
        return;
    }

    anEvent.SetClientData( NULL );


    auto_ptr<wxMenu> main_menu( CreateContextMenuBackbone() );
    Merge( *main_menu.get(), *menu );

    CleanupSeparators( *main_menu );
    PopupMenu( main_menu.get() );

    delete menu;
}


void CTaskTablePanel::OnDetails(wxCommandEvent& event)
{
    _ASSERT(m_Table);

    long sel_index = m_Table->GetFirstSelected();
    _ASSERT(sel_index != -1);

    OnShowDetailsDialog(sel_index);
}


void CTaskTablePanel::OnItemActivated(wxListEvent &event)
{
    int index = event.GetIndex();
    OnShowDetailsDialog(index);
}


void CTaskTablePanel::OnShowDetailsDialog(int vis_index)
{
    int data_index = m_Table->RowVisibleToData(vis_index);

    CTaskDetailsDlg dlg(this);

    string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, "Details Dialog");
    dlg.SetRegistryPath(reg_path);

    wxVariant var = m_Model->GetValueAt(data_index, CTaskViewModel::eDescription);
    dlg.SetTaskTitle(var.GetString());

    var = m_Model->GetValueAt(data_index, CTaskViewModel::eState);
    dlg.SetState(var.GetString());

    var = m_Model->GetValueAt(data_index, CTaskViewModel::eStatus);
    dlg.SetStatus(var.GetString());

    var = m_Model->GetValueAt(data_index, CTaskViewModel::eTime);
    dlg.SetTime(var.GetString());

    dlg.ShowModal();
}


void CTaskTablePanel::OnUpdateDetails(wxUpdateUIEvent& event)
{
    event.Enable(m_Table  &&  m_Table->GetSelectedItemCount() == 1);
}


void CTaskTablePanel::OnCancel(wxCommandEvent& event)
{
    _ASSERT(m_Table);

    if (m_Table->GetSelectedItemCount() == 1) {
        long sel_index = m_Table->GetFirstSelected();
        int data_index = m_Table->RowVisibleToData(sel_index);
        CTaskViewModel::TTaskRef task = m_Model->GetTask(data_index);

        m_TaskService->CancelTask(*task);
    }
}


void CTaskTablePanel::OnUpdateCancel(wxUpdateUIEvent& event)
{
    bool en = false;
    if(m_Table  &&  m_Table->GetSelectedItemCount() == 1)   {
        long sel_index = m_Table->GetFirstSelected();
        int data_index = m_Table->RowVisibleToData(sel_index);
        CTaskViewModel::TTaskRef task = m_Model->GetTask(data_index);
        en = (task->GetState() == IAppTask::eBackgrounded);
    }
    event.Enable(en);
}


void CTaskTablePanel::OnEventTimer(wxTimerEvent& event)
{
    UpdateTable();
}


void CTaskTablePanel::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}


void CTaskTablePanel::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        m_Table->SaveTableSettings(view);
    }
}


void CTaskTablePanel::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_Table->LoadTableSettings(view);

        m_Model->UpdateTable();
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CTaskView

BEGIN_EVENT_MAP(CTaskView, CEventHandler)
    ON_EVENT(CAppTaskServiceGuiUpdateEvent, CAppTaskServiceGuiUpdateEvent::eUpdate, &CTaskView::x_OnUpdate)
END_EVENT_MAP()

CViewTypeDescriptor CTaskView::m_TypeDescr(
    "Task View", // type name
    "task_view", // icon alias
    "Task View displays a list of application task such as running commands and tools.",
    "Task View displays a list of application task such as running commands and tools.",
    "TASK_VIEW", // help ID
    "System",     // category
    true);       // singleton


CTaskView::CTaskView()
:   m_TaskService(NULL),
    m_Panel(NULL)
{
}


CTaskView::~CTaskView()
{
}


const CViewTypeDescriptor& CTaskView::GetTypeDescriptor() const
{
    return m_TypeDescr;
}


void CTaskView::SetWorkbench(IWorkbench* workbench)
{
    if(workbench)   {
        m_TaskService = workbench->GetAppTaskService();
        m_TaskService->AddListener(this);
        m_Panel->CreateModel(m_TaskService);
    } else {
        if (m_TaskService) {
            m_TaskService->RemoveListener(this);
            m_Panel->DestroyModel();
            m_TaskService = 0;
        }
    }
}

void CTaskView::CreateViewWindow(wxWindow* parent)
{
    m_Panel = new CTaskTablePanel();
    m_Panel->Create(parent, wxID_ANY);
}

void CTaskView::DestroyViewWindow()
{
    if(m_Panel) {
        m_Panel->Destroy();
        m_Panel = NULL;
    }
}

void CTaskView::x_OnUpdate(CEvent*)
{
    m_Panel->UpdateTable();
}

wxWindow* CTaskView::GetWindow()
{
    return m_Panel;
}


string CTaskView::GetClientLabel(IWMClient::ELabel) const
{
    return m_TypeDescr.GetLabel();
}


string CTaskView::GetIconAlias() const
{
    return m_TypeDescr.GetIconAlias();
}


const CRgbaColor* CTaskView::GetColor() const
{
    return NULL;
}


void CTaskView::SetColor(const CRgbaColor& color)
{
    // do nothing
}


const wxMenu* CTaskView::GetMenu()
{
    return NULL;
}


void CTaskView::UpdateMenu(wxMenu& root_menu)
{
}


IWMClient::CFingerprint CTaskView::GetFingerprint() const
{
    return CFingerprint(m_TypeDescr.GetLabel(), true);
}


static const char* kPanelSection = ".Panel";

void CTaskView::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
    m_Panel->SetRegistryPath(m_RegPath + kPanelSection);
}


void CTaskView::SaveSettings() const
{
     m_Panel->SaveSettings();
}


void CTaskView::LoadSettings()
{
    m_Panel->LoadSettings();
}


///////////////////////////////////////////////////////////////////////////////
/// CTaskViewFactory

void CTaskViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    string alias = GetViewTypeDescriptor().GetIconAlias();
    provider.RegisterFileAlias(ToWxString(alias), wxT("task_view.png"));
}


const CViewTypeDescriptor& CTaskViewFactory::GetViewTypeDescriptor() const
{
    return CTaskView::m_TypeDescr;
}


IView* CTaskViewFactory::CreateInstance() const
{
    return new CTaskView();
}


IView* CTaskViewFactory::CreateInstanceByFingerprint(const TFingerprint& fingerprint) const
{
    TFingerprint print(CTaskView::m_TypeDescr.GetLabel(), true);
    if(print == fingerprint)   {
        return new CTaskView();
    }
    return NULL;
}


string CTaskViewFactory::GetExtensionIdentifier() const
{
    static string sid("task_view_factory");
    return sid;
}


string CTaskViewFactory::GetExtensionLabel() const
{
    static string slabel("Task View Factory");
    return slabel;
}


END_NCBI_SCOPE
