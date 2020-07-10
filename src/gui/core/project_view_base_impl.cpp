/*  $Id: project_view_base_impl.cpp 41853 2018-10-25 13:57:22Z katargir $
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
 * Authors:  Mike DiCuccio, Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/core/project_view_base_impl.hpp>

#include <gui/core/project_service.hpp>

#include <gui/core/selection_service_impl.hpp>
#include <gui/core/data_mining_service.hpp>
#include <gui/core/view_extensions.hpp>
#include <gui/core/commands.hpp>
#include <gui/core/open_view_extension.hpp>
#include <gui/utils/extension_impl.hpp>

#include <gui/utils/app_job_impl.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/framework/window_manager_service.hpp>
#include <gui/framework/tool_bar_service.hpp>
#include <gui/framework/app_job_task.hpp>
#include <gui/framework/app_task_service.hpp>

#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/label.hpp>

#include <objects/gbproj/ProjectDescr.hpp>
#include <gui/objects/GBProjectHandle.hpp>
#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objtools/edit/seq_entry_edit.hpp>

#include <wx/msgdlg.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include <wx/aui/auibar.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

/////////////////////////////////////////////////////////////////////////////
/// CProjectViewBase implementation

static CAtomicCounter_WithAutoInit sm_ViewIdCounter;
static vector<CRgbaColor> sm_ColorTable;

static void s_CreateDefaultColorTable()
{
    if (sm_ColorTable.size() == 0) {
        DEFINE_STATIC_MUTEX(s_Mutex);
        CMutexGuard LOCK(s_Mutex);
        if (sm_ColorTable.size() == 0) {
            sm_ColorTable.reserve(24);

            sm_ColorTable.push_back(CRgbaColor::GetColor("red"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("green"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("yellow"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("violet"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("orange"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("pink"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("goldenrod"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("olivedrab"));

            sm_ColorTable.push_back(CRgbaColor::GetColor("darkred"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("darkgreen"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("cornflowerblue"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("yellowgreen"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("darkorange"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("cyan1"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("darkviolet"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("deeppink"));

            sm_ColorTable.push_back(CRgbaColor::GetColor("darkgoldenrod"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("darkolivegreen"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("mediumaquamarine"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("aquamarine"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("blueviolet"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("chartreuse"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("coral"));
            sm_ColorTable.push_back(CRgbaColor::GetColor("cyan4"));
        }
    }
}

// TODO - some of the event handlers need to be ported to wxWidgets
BEGIN_EVENT_MAP( CProjectViewBase, CEventHandler )
    ON_EVENT(
        CGBDocument::TEvent, 
        CGBDocument::TEvent::eProjectChanged,     
        &CProjectViewBase::eh_OnProjectChanged
    )
    ON_EVENT(
    CGBDocument::TEvent, 
    CGBDocument::TEvent::eViewAttached,         
    &CProjectViewBase::eh_OnViewAttached
    )
    ON_EVENT(
        CGBDocument::TEvent, 
        CGBDocument::TEvent::eViewReleased,        
        &CProjectViewBase::eh_OnViewReleased
    )
END_EVENT_MAP()


/*
BEGIN_EVENT_TABLE(CProjectViewBase, wxEvtHandler)
    ON_COMMAND(eCmdUndo, &CProjectViewBase::OnEditUndo)
    ON_COMMAND(eCmdRedo, &CProjectViewBase::OnEditRedo)
    ON_UPDATE_COMMAND_UI(eCmdUndo, &CProjectViewBase::OnUpdateEditUndo)
    ON_UPDATE_COMMAND_UI(eCmdRedo, &CProjectViewBase::OnUpdateEditRedo)
END_EVENT_TABLE()
*/

CProjectViewBase::CProjectViewBase()
:   m_ViewId(sm_ViewIdCounter.Add(1) - 1),
    m_Workbench(NULL),
    m_ProjectId(CGBProjectHandle::sm_NullId),
    m_SelectionService(NULL),
    m_VisibleRangeChangeAllowed(true),
    m_ResetHistAsync(true)
{
    s_CreateDefaultColorTable();

    if (sm_ColorTable.size()) {
        m_Color = sm_ColorTable[m_ViewId % sm_ColorTable.size()];
    }
}


CProjectViewBase::~CProjectViewBase(void)
{
}

void CProjectViewBase::x_AttachToProject(CGBDocument& doc)
{
    m_ProjectId = doc.GetId();
    m_ProjectName = doc.GetDescr().GetTitle();
    m_Scope.Reset(doc.GetScope());
    doc.x_AttachView(this);

    vector<IOpenProjectViewExtension*> clients;
    GetExtensionAsInterface(EXT_POINT__OPEN_PROJECT_VIEW_EXTENSION, clients);
    ITERATE(vector<IOpenProjectViewExtension*>, it, clients) {
        (*it)->OnViewOpen(*this, m_Workbench);
    }
}


/// Job to reset scope history in the background
///
class CResetScopeHistoryJob : public CJobCancelable
{
public:
    CResetScopeHistoryJob(objects::CScope* scope, const string& name)
        : m_Scope(scope),
          m_Name(name)
    {}

    /// @name IAppJob implementation
    /// @{
    virtual EJobState                   Run()
    {
        wxSleep(5);
        m_Scope->ResetHistory();
        return IAppJob::eCompleted;
    }
    virtual CConstIRef<IAppJobProgress> GetProgress() 
    { 
        return CConstIRef<IAppJobProgress>(new CAppJobProgress(0, kEmptyStr));
    }
    virtual CRef<CObject>               GetResult() 
    {
        return CRef<CObject>();
    }
    virtual CConstIRef<IAppJobError>    GetError()
    {
        return CConstIRef<IAppJobError>(m_Error.GetPointer());
    }
    virtual string  GetDescr() const
    {
        return string("Close ") + m_Name; 
    }
    /// @}

private:
    CRef<objects::CScope> m_Scope;
    const string          m_Name;
    CRef<CAppJobError>    m_Error;
};


void CProjectViewBase::x_DetachFromProject()
{
    _ASSERT(x_HasProject());

    if(x_HasProject()  &&  m_Workbench)   {
        {{
            CProjectService* srv = m_Workbench->GetServiceByType<CProjectService>();

            CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
            if (!ws) return;

            CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromId(m_ProjectId));
            _ASSERT(doc);
            doc->x_DetachView(this);
        }}

        // Background job to release history (potentially long and blocking operation)
        if (m_ResetHistAsync)
        {
            CRef<CResetScopeHistoryJob> rjob(new CResetScopeHistoryJob(m_Scope.GetPointer(), GetLabel(IProjectView::eTypeAndContent)));
            CRef<CAppJobTask> task(new CAppJobTask(*rjob, true, kEmptyStr, 3, "ObjManagerEngine"));

            IServiceLocator* srv_loc = m_Workbench->GetServiceLocator();
            CAppTaskService* task_srv = srv_loc->GetServiceByType<CAppTaskService>();

            task_srv->AddTask(*task);
        }
        else
        {
            m_Scope->ResetHistory();
        }

        m_Scope.Reset();
        m_ProjectId = CGBProjectHandle::sm_NullId;
    }
}


void CProjectViewBase::DestroyView()
{
    //_ASSERT(x_HasProject());
    if (x_HasProject()) {
        const IViewSettingsAtProject* setts =
            dynamic_cast<const IViewSettingsAtProject*>(this);

        if (setts) {
            CProjectService* srv = m_Workbench->GetServiceByType<CProjectService>();
            CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
            if (ws) {
                CGBProjectHandle* project = ws->GetProjectFromId(m_ProjectId);
                if (project)
                    setts->SaveSettingsAtProject(*project);
            }
        }

        x_DetachFromProject();
    }
}


void CProjectViewBase::InitialLayout()
{
}


void CProjectViewBase::Destroy()
{
}


CProjectViewBase::TProjectId CProjectViewBase::GetProjectId() const
{
    return m_ProjectId;
}

size_t CProjectViewBase::GetId() const
{
    return m_ViewId;
}


///////////////////////////////////////////////////////////////////////////////
/// IView implementation
void CProjectViewBase::SetWorkbench(IWorkbench* workbench)
{
    IToolBarContext* tb_ctx = dynamic_cast<IToolBarContext*>(this);

    if(workbench)   {
        // connect
        if(tb_ctx)  {
            IToolBarService* tb_srv = workbench->GetToolBarService();
            tb_srv->AddToolBarContext(tb_ctx);
        }

        CIRef<CSelectionService> sel_srv = workbench->GetServiceByType<CSelectionService>();
        sel_srv->AttachClient(this);
    } else {
        // disconnect
        CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
        sel_srv->DetachClient(this);

        if(tb_ctx)  {
            IToolBarService* tb_srv = m_Workbench->GetToolBarService();
            tb_srv->RemoveToolBarContext(tb_ctx);
        }
    }

    m_Workbench = workbench;
}

IServiceLocator* CProjectViewBase::GetServiceLocator() const
{
    return m_Workbench;
}

CIRef<IProjectViewLabel> CProjectViewBase::m_LabelGenerator;

void CProjectViewBase::SetLabelGenerator(IProjectViewLabel* labelGenerator)
{
    m_LabelGenerator.Reset(labelGenerator);
}

void CProjectViewBase::RefreshViewWindow()
{
    IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
    wm_srv->RefreshClient( *this );
}

///////////////////////////////////////////////////////////////////////////////
/// IWMClient implementation

wxEvtHandler* CProjectViewBase::GetCommandHandler()
{
    /// Default implementation dispatches all commands and updates
    /// to focused window

    // Take handler from focused window 
    // if it belongs to client window

    wxWindow* base_win = GetWindow();
    _ASSERT(base_win);

    wxWindow* focused = wxWindow::FindFocus();

    wxWindow* wid = focused;
    while (wid != NULL && wid != base_win) {
        wid = wid->GetParent();
    }

    wid = (wid == NULL) ? base_win : focused;
    if (wid)
        return wid->GetEventHandler();

    return 0;
}

static void sContentCleanup(string& content)
{
    if (content.length() > 70) {
        string::size_type pos = content.find_last_of(" \t\n\r", 70);
        if (pos != string::npos) {
            content.erase(pos);
            content += "...";
        }
    }
}

/// find all capital letters, 
static string sAbbreviate(const string& content)
{
    string s;
    if (content.empty())
        return s;
    s = content[0];
    for (size_t i = 1; i < content.size(); ++i) {
        char c = content[i];
        if (isspace(content[i-1]) && isalnum (c)) {
            s += c;
        }
    }
    NStr::ToUpper(s);
    return s;
}


string CProjectViewBase::GetClientLabel(IWMClient::ELabel ltype) const
{
    string str = GetLabel(eId) + ": ";
    string content;

    switch (ltype) {
    case IWMClient::eContent:
        content = GetLabel(eContent);
        str += content;
        break;
    case IWMClient::eTypeAndContent:
        content = GetLabel(eContent);
        sContentCleanup(content);
        str += content;
        str += " ";
        content = GetLabel(eType);
        content = sAbbreviate(content);
        str += content;
        break;
    default:
        content = GetLabel(eTypeAndContent);
        sContentCleanup(content);
        str += content;
        if(x_HasProject())    {
            str += " [" + GetLabel(eProject) + ']';
        }
        break;
    } // switch
    return str;
}


string CProjectViewBase::GetIconAlias() const
{
    return GetTypeDescriptor().GetIconAlias();
}


const CRgbaColor* CProjectViewBase::GetColor(void) const
{
    return &m_Color;
}


void CProjectViewBase::SetColor(const CRgbaColor& color)
{
    m_Color = color;
}


const wxMenu* CProjectViewBase::GetMenu()
{
    // if m_MenuBarMenu is empty try creating it
    if( ! m_MenuBarMenu.get() ){
        if( GetMenuDef() ){
            m_MenuBarMenu.reset( IMenuContributor::GetMenu() );
        } else {
            x_CreateMenuBarMenu();
        }
    }

    return m_MenuBarMenu.get();
}


IWMClient::CFingerprint CProjectViewBase::GetFingerprint() const
{
    if (!m_Fingerprint.empty())
        return CFingerprint(m_Fingerprint, true);

    const IWMClient* client = this;
    string s = GetClientLabel() + " @" + NStr::PtrToString(client);
    return CFingerprint(s, false);
}

void CProjectViewBase::SetFingerprint(const string& fp)
{
    m_Fingerprint = fp;
}

bool CProjectViewBase::x_HasProject() const
{
    return m_ProjectId != CGBProjectHandle::sm_NullId;
}


CScope* CProjectViewBase::x_GetScope() const
{
    // CProjectViewBase* nc_this = const_cast<CProjectViewBase*>(this);
    // return nc_this->m_Scope.GetPointer();

    return GetScope().GetPointer();
}

const CObject* CProjectViewBase::x_GetOrigObject() const
{
    _ASSERT(false); // override in derived classes
    return NULL;
}

void CProjectViewBase::x_ReportInvalidInputData(TConstScopedObjects& objects)
{
    string obj_s;
    string view_name = GetTypeDescriptor().GetLabel();
    string title = view_name + " - Incompatible Data";

    string s;
    NON_CONST_ITERATE(TConstScopedObjects, it, objects)  {
        const CObject* obj = it->object.GetPointer();
        CScope* scope = it->scope;
        CLabel::GetLabel(*obj, &obj_s, CLabel::eContent, scope);
        s += obj_s + " ";
    }
    s += "\" cannot be shown in " + view_name + ".";
    wxMessageBox(ToWxString(s), ToWxString(title), wxOK | wxICON_EXCLAMATION);
}


void CProjectViewBase::x_CreateMenuBarMenu()
{
    _TRACE("CProjectViewBase::x_CreateMenuBarMenu() - override in derived classes");
}


void CProjectViewBase::eh_OnProjectChanged( CEvent* event )
{
    CGBDocument::TEvent* ev = dynamic_cast<CGBDocument::TEvent*>(event);
    if( ev ){
        {{
            CProjectService* srv = m_Workbench->GetServiceByType<CProjectService>();

            CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
            if (!ws) return;

            CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromId(m_ProjectId));
            _ASSERT(doc);
            m_ProjectName = doc->GetDescr().GetTitle();

            x_UpdateContentLabel();

            RefreshViewWindow();
        }}

        if( ev->GetProjectId() == m_ProjectId ){
            OnProjectChanged( *ev );
        }
    }
}


void CProjectViewBase::eh_OnViewAttached( CEvent* event )
{
    CGBDocument::TEvent* ev = dynamic_cast<CGBDocument::TEvent*>(event);
    if( ev ){
        OnViewAttached( *ev->GetProjectView() );
    }
}

void CProjectViewBase::eh_OnViewReleased( CEvent* event )
{
    CGBDocument::TEvent* ev = dynamic_cast<CGBDocument::TEvent*>(event);
    if( ev ){
        OnViewReleased( *ev->GetProjectView() );
    }
}


void CProjectViewBase::OnProjectChanged( CProjectViewEvent& evt )
{
    _TRACE(typeid(this).name() << " - not yet moved: OnProjectChanged( event )");

    OnProjectChanged();
}


void CProjectViewBase::OnProjectChanged()
{
    //_TRACE(typeid(this).name() << " - unhandled message: OnProjectChanged()");
}


void CProjectViewBase::OnSelectionChanged(const TConstScopedObjects&)
{
    //_TRACE(typeid(this).name() << " - unhandled message: OnSelectionChanged()");
}


void CProjectViewBase::OnViewReleased(IView&)
{
    //_TRACE(typeid(this).name() << " - unhandled message: OnViewReleased()");
}


void CProjectViewBase::OnViewAttached(IView&)
{
    //_TRACE(typeid(this).name() << " - unhandled message: OnViewAttached()");
}

void CProjectViewBase::OnPrint()
{
}


string  CProjectViewBase::GetSelClientLabel()
{
    return GetClientLabel();
}


CRef<CScope> CProjectViewBase::GetScope() const
{
    return m_Scope;
}


void CProjectViewBase::GetCompatibleToolBars(vector<string>& names)
{
    names.push_back(CProjectViewToolBatFactory::sm_NavTBName);
    names.push_back(CProjectViewToolBatFactory::sm_SelTBName);
}


string CProjectViewBase::GetLabel(ELabelType type) const
{
    string str;
    if (m_LabelGenerator && m_LabelGenerator->GetLabel(str, *this, type))
        return str;

    switch (type) {
    case eType:
        str = GetTypeDescriptor().GetLabel();
        break;

    case eContent:
        str = GetContentLabel();
        break;

    case eTypeAndContent:
        str = GetContentLabel();
        str += " (";
        str += GetTypeDescriptor().GetLabel();
        str += ')';
        break;

    case eProject:
        str = m_ProjectName;
        break;

    case eId:   {
        size_t id = GetId();
        do {
            char c = 'A' + id % 26;
            str += c;
            id /= 26;
        } while (id != 0);
        std::reverse(str.begin(), str.end());
        break;
    }
    }
    return str;
}


string CProjectViewBase::GetContentLabel() const
{
    return m_ContentLabel;
}

static bool s_CollidedIds(CObject* obj)
{
    if (CSeq_entry* se = dynamic_cast<CSeq_entry*>(obj)) {
        return edit::HasRepairedIDs(*se);
    }
    else if (CBioseq* bioseq = dynamic_cast<CBioseq*>(obj)) {
        CRef<CSeq_entry> se(new CSeq_entry());
        se->SetSeq(*bioseq);
        return edit::HasRepairedIDs(*se);
    }
    else if (CBioseq_set* bioseqSet = dynamic_cast<CBioseq_set*>(obj)) {
        CRef<CSeq_entry> se(new CSeq_entry());
        se->SetSet(*bioseqSet);
        return edit::HasRepairedIDs(*se);
    }
    else if (CSeq_submit* sub = dynamic_cast<CSeq_submit*>(obj)) {
        if (sub->IsSetData()) {
            const CSeq_submit::TData& data = sub->GetData();
            if (data.IsEntrys()) {
                for (const auto& se : data.GetEntrys()) {
                    if (edit::HasRepairedIDs(*se))
                        return true;
                }
            }
        }
    }

    return false;
}

void CProjectViewBase::x_UpdateContentLabel()
{
    m_ContentLabel.erase();

    if (x_HasProject()) {
        const CObject* obj = x_GetOrigObject();
        CScope* scope = GetScope().GetPointer();

        _ASSERT( obj && scope );

        CLabel::GetLabel( *obj, &m_ContentLabel, CLabel::eDefault, scope );
        if (s_CollidedIds(const_cast<CObject*>(obj))) m_ContentLabel += "[Colliding IDs repaired!]";

        CProjectService* srv = m_Workbench->GetServiceByType<CProjectService>();
        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (!ws) return;

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromId(m_ProjectId));
        _ASSERT(doc);
        doc->ViewLabelChanged(*this);
    }
}



///////////////////////////////////////////////////////////////////////////////
/// CProjectViewToolBatFactory

const string CProjectViewToolBatFactory::sm_NavTBName("Navigation Toolbar");
const string CProjectViewToolBatFactory::sm_SelTBName("Selection Toolbar");


void CProjectViewToolBatFactory::GetToolBarNames(vector<string>& names)
{
    names.push_back(sm_NavTBName);
    names.push_back(sm_SelTBName);
}


wxAuiToolBar* CProjectViewToolBatFactory::CreateToolBar(const string& name, wxWindow* parent)
{
    if(name == sm_NavTBName) {
        return x_CreateNavigationToolbar(parent);
    } else if(name == sm_SelTBName) {
        return x_CreateSelectionToolbar(parent);
    }
    return NULL;
}


wxAuiToolBar* CProjectViewToolBatFactory::x_CreateNavigationToolbar(wxWindow* parent)
{
    long style = wxAUI_TB_HORZ_LAYOUT | wxBORDER_NONE;

    wxAuiToolBar* toolbar = new wxAuiToolBar(parent, wxID_ANY, wxDefaultPosition,
                                             wxDefaultSize, style);
    toolbar->SetName(ToWxString(sm_NavTBName));

    toolbar->SetToolBitmapSize(wxSize(16, 16));

    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();

    cmd_reg.AppendTool(*toolbar, eCmdBack);
    cmd_reg.AppendTool(*toolbar, eCmdForward);
    toolbar->AddSeparator();
    cmd_reg.AppendTool(*toolbar, eCmdZoomIn);
    cmd_reg.AppendTool(*toolbar, eCmdZoomOut);
    cmd_reg.AppendTool(*toolbar, eCmdZoomAll);
    cmd_reg.AppendTool(*toolbar, eCmdZoomSeq);
    cmd_reg.AppendTool(*toolbar, eCmdZoomSel);

    toolbar->Realize();
    return toolbar;
}


wxAuiToolBar* CProjectViewToolBatFactory::x_CreateSelectionToolbar(wxWindow* parent)
{
    long style = wxAUI_TB_HORZ_LAYOUT | wxBORDER_NONE;

    wxAuiToolBar* toolbar = new wxAuiToolBar(parent, wxID_ANY, wxDefaultPosition,
                                       wxDefaultSize, style);
    toolbar->SetName(ToWxString(sm_SelTBName));
    toolbar->SetToolBitmapSize(wxSize(16, 16));

    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();

    cmd_reg.AppendTool(*toolbar, eCmdBroadcastOptions);
    cmd_reg.AppendTool(*toolbar, eCmdBroadcastSel);
    cmd_reg.AppendTool(*toolbar, eCmdBroadcastSelRange);

    toolbar->Realize();
    return toolbar;
}


END_NCBI_SCOPE
