/*  $Id: project_service.cpp 41347 2018-07-12 18:28:31Z evgeniev $
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

#include <gui/core/project_service.hpp>

#include <gui/core/document.hpp>
#include <gui/core/simple_project_view.hpp>
#include <gui/core/project_tree_view.hpp>
#include <gui/core/project_tree_panel.hpp>

#include <gui/framework/view_manager_service.hpp>
#include <gui/framework/window_manager_service.hpp>

#include <gui/objutils/label.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objects/GBWorkspace.hpp>
#include <objects/gbproj/ProjectDescr.hpp>
#include <gui/objects/WorkspaceFolder.hpp>
#include <gui/objutils/usage_report_job.hpp>

#include <objects/seq/Annotdesc.hpp>
#include <objects/gbproj/FolderInfo.hpp>
#include <objects/general/Object_id.hpp>

#include <serial/objistr.hpp>
#include <serial/objostr.hpp>
#include <serial/serial.hpp>
#include <serial/iterator.hpp>

#include <wx/msgdlg.h>
#include <wx/filename.h>
#include <gui/widgets/wx/message_box.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CProjectService::CProjectService() : m_ServiceLocator()
{
    //LOG_POST("CProjectService constructor");
}


CProjectService::~CProjectService()
{
    //LOG_POST("CProjectService destructor");
}

void CProjectService::InitService()
{
    LOG_POST(Info << "Initializing Project Service...");

    LoadSettings();
    CreateNewWorkspace();

    LOG_POST(Info << "Finished initializing Project Service");
}


void CProjectService::ShutDownService()
{
    LOG_POST(Info << "Shutting down Project Service...");

    SaveSettings();

    /// get rid of our workspace
    ResetWorkspace();
    m_Workspace.Reset();

    LOG_POST(Info << "Finished shutting down Project Service");
}


void CProjectService::SetServiceLocator(IServiceLocator* locator)
{
    m_ServiceLocator = locator;
}


void CProjectService::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}


static const char* kMRUTag = "MRUProjectsWorkspaces";

void CProjectService::SaveSettings() const
{
    _ASSERT(! m_RegPath.empty());

    if( ! m_RegPath.empty())   {
        // lock service to avoid races
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        // save MRU Project and Workspaces
        vector<string> values;
        // we package times and paths into the same vector (even elements - time, odd - path)
        CTimeFormat format =
            CTimeFormat::GetPredefined(CTimeFormat::eISO8601_DateTimeSec);

        const TMRUPathList::TTimeToTMap& map = m_ProjectWorkspaceMRUList.GetMap();
        ITERATE(TMRUPathList::TTimeToTMap, it, map)  {
            CTime time(it->first);
            time.ToLocalTime();
            string s_time = time.AsString(format);
            string filename = FnToStdString(it->second);
            values.push_back(s_time);
            values.push_back(filename);
        }
        view.Set(kMRUTag, values);
    }
}


void CProjectService::LoadSettings()
{
    _ASSERT(! m_RegPath.empty());

    if( ! m_RegPath.empty())   {
        // lock service to avoid races

        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        // load MRU Project and Workspaces
        m_ProjectWorkspaceMRUList.Clear();

        vector<string> values;
        view.GetStringVec(kMRUTag, values);

        CTimeFormat format =
            CTimeFormat::GetPredefined(CTimeFormat::eISO8601_DateTimeSec);

        for(  size_t i = 0;  i + 1 < values.size() ;  )   {
            // process two elements at once
            string s_time = values[i++];
            wxString path = FnToWxString(values[i++]);
            CTime time(s_time, format);

            time_t t = time.GetTimeT();
            m_ProjectWorkspaceMRUList.Add(path, t);
        }
    }
}


void CProjectService::CreateNewWorkspace()
{
    if(m_Workspace)   {
        _ASSERT(false);
        NCBI_THROW(CProjectServiceException, eInvalidOperation,
                   "Cannot create a new Workspace - a workspace already exists");
    } else {
        {{
            m_Workspace.Reset(new CGBWorkspace());

            /// determine a unique(ish) name for the workspace
            /// this is a generic tag followed by a counter
            /// we test this against the file system in the *current*
            /// directory (without bothering to test what current really means)

            static unsigned int counter = 1;
            string workspace_title;
            for (;;) {
                wxString str = wxString::Format(wxT("Workspace%u"), counter++);
                if (!wxFileName::FileExists(str + wxT(".gbw"))) {
                    workspace_title = str.ToAscii();
                    break;
                }
            }
            m_Workspace->SetDescr().SetTitle(workspace_title);

            CTime create_time(CTime::eCurrent);
            m_Workspace->SetDescr().SetCreateDate(create_time);
            m_Workspace->SetDescr().SetModifiedDate(create_time);
            m_Workspace->SetWorkspace().SetInfo().SetTitle(workspace_title);
            m_Workspace->SetWorkspace().SetInfo().SetCreateDate(create_time);
        }}

        x_ReloadProjectTreeView();
    }
}


bool CProjectService::HasWorkspace()
{
    return (m_Workspace.GetPointer() != NULL);
}

CRef<CGBWorkspace> CProjectService::GetGBWorkspace()
{

    return m_Workspace;
}


void CProjectService::ResetWorkspace()
{
    if (!m_Workspace)
        return;

    vector<int> projIds;

    for (CTypeIterator<CGBProjectHandle> it(m_Workspace->SetWorkspace()); it; ++it) {
        CGBDocument* doc = dynamic_cast<CGBDocument*>(&*it);
        if (!doc) continue;

        if (doc->IsLoading()) {
            doc->CancelLoading();
        } else if (doc->IsLoaded()) {
            doc->UnloadProject();
        }

        projIds.push_back(doc->GetId());
    }

    for (auto id : projIds) {
        CRef<CGBDocument> doc(dynamic_cast<CGBDocument*>(m_Workspace->GetProjectFromId(id)));
        if (doc) RemoveProject(*doc);
    }

    x_ReloadProjectTreeView();
}

bool CProjectService::SaveWorkspace()
{
    //LOG_POST(Info << "CProjectService::SaveWorkspace()");

    static const string kErr("Error saving workspace");

    static const string INVALID_WORKSPACE =
        "Invalid workspace! The Workspace contains projects "
        "with empty filenames.";

    string err_msg;

    try {
        if (!m_Workspace) {
            _ASSERT(false);
            NCBI_THROW(CProjectServiceException, eInvalidOperation,
                       "Cannot save workspace - it does not exists.");
        }

        // check that all projects have valid filename
        for (CTypeConstIterator<CGBProjectHandle> it(m_Workspace->GetWorkspace()); it; ++it) {
            if (it->GetFilename().empty()) {
                //_ASSERT(false); // must not be empty at this point
                NCBI_THROW(CProjectServiceException, eInvalidOperation, INVALID_WORKSPACE);
            }
        }

        wxString filename = FnToWxString(m_Workspace->GetFilename());

        CNcbiOfstream ostr(filename.fn_str());
        auto_ptr<CObjectOStream> os(CObjectOStream::Open(eSerial_AsnText, ostr));
        *os << *m_Workspace;

        x_UpdateWorkspaceLabel();

        return true;
    }
    catch (CException& e) {
        err_msg = e.GetMsg();
    }
    catch (std::exception& e) {
        err_msg = e.what();
    }
    wxMessageBox(ToWxString(err_msg), wxT("Error saving workspace"),
                 wxOK|wxICON_ERROR);

    NCBI_THROW(CException, eUnknown, kErr + " : \n" + err_msg);
    return false;
}


CRef<CGBWorkspace> CProjectService::x_LoadWorkspace(const wxString& filename)
{
    /// in order, we try ASN.1 text, ASN.1 binary, and XML
    /// chances are it'll always be ASN.1 text, but we can try the
    /// others for completeness
    typedef pair<ESerialDataFormat, const char*> TSerialTypePair;
    static const TSerialTypePair sc_DataTypes[] = {
        TSerialTypePair(eSerial_AsnText,    "asn-text"),
        TSerialTypePair(eSerial_AsnBinary,  "asn-binary"),
        TSerialTypePair(eSerial_Xml,        "asn-xml")
    };

    CRef<CGBWorkspace> wks;

    size_t n = sizeof(sc_DataTypes) / sizeof(TSerialTypePair);
    for (size_t i = 0;  i < n;  ++i) {
        const char* type_name = sc_DataTypes[i].second;
        try {
            wks.Reset(new CGBWorkspace());

            ESerialDataFormat format = sc_DataTypes[i].first;
            bool binary = (format == eSerial_AsnBinary);
            CNcbiIfstream istr(filename.fn_str(), (binary ? (ios::binary|ios::in) : ios::in));
            auto_ptr<CObjectIStream> obj_istr(CObjectIStream::Open(format, istr));
            *obj_istr >> *wks;

            // HACK: promoting CGBPorjectHandles to CGBDocuments
            CGBWorkspace::TWorkspace& root_folder = wks->SetWorkspace();
            x_CreateDocuments(root_folder);

            LOG_POST(Info << "CProjectService::x_LoadWorkspace(): loaded workspace: "
                << filename << ": " << type_name);
            break;
        }
        catch (CSerialException& e) {
            wks.Reset();
            LOG_POST(Error << "CProjectService::x_LoadWorkspace(): workspace: "
                << filename << ": not " << type_name
                << ": error reading: " << e.GetMsg());
        }
    }

    return wks;
}

// recursively iterates folders and wraps Project Handles in Documents
void CProjectService::x_CreateDocuments(CWorkspaceFolder& folder)
{
    CWorkspaceFolder::TProjects& projects = folder.SetProjects();

    NON_CONST_ITERATE(CWorkspaceFolder::TProjects, it, projects)  {
        CRef<CGBProjectHandle>& handle = *it;
        // create wrapping Document
        CRef<CGBDocument> doc(new CGBDocument(this));
        doc->Assign(*handle);

        // replace handle with the document
        handle.Reset(doc);
    }

    CWorkspaceFolder::TFolders& folders = folder.SetFolders();
    NON_CONST_ITERATE(CWorkspaceFolder::TFolders, it, folders)  {
        CWorkspaceFolder& child_folder = **it;
        x_CreateDocuments(child_folder);
    }
}


const CProjectService::TMRUPathList&
    CProjectService::GetProjectWorkspaceMRUList() const
{
    return m_ProjectWorkspaceMRUList;
}


void CProjectService::AddToProjectWorkspaceMRUList(const wxString& path)
{
    m_ProjectWorkspaceMRUList.Add(path);
}

void CProjectService::x_ReloadProjectTreeView()
{
    CProjectTreePanel* projectTreePanel = GetProjectTreePanel();
    if (projectTreePanel)
        projectTreePanel->ReloadProjectTree();
}

void sAssertMainThread(const char* err_msg)
{
    bool main_thread = (CThread::GetSelf() == 0);
    if( ! main_thread)  {
        _ASSERT(false);
        NCBI_THROW(CProjectServiceException, eThreadAffinityError, err_msg);
    }
}


CIRef<IProjectView> CProjectService::AddProjectView(const string& view_name,
                                                    SConstScopedObject& object,
                                                    const objects::CUser_object* params,
                                                    bool bFloat)
{
    TConstScopedObjects objects;
    objects.push_back(object);

    return AddProjectView (view_name, objects, params, bFloat);
}

CIRef<IProjectView> CProjectService::AddProjectView(const string& view_name,
                                                    TConstScopedObjects& objects,
                                                    const objects::CUser_object* params,
                                                    bool bFloat)
{
    sAssertMainThread("Views can be created only from the main thread");

    IViewManagerService* view_srv =
        m_ServiceLocator->GetServiceByType<IViewManagerService>()
    ;

    _ASSERT(view_srv);

    CIRef<IView> view( view_srv->CreateViewInstance(view_name) );
    if( !view ){
        return null;
    }

    CIRef<IProjectView> prj_view(dynamic_cast<IProjectView*>(view.GetPointer()));
    if( prj_view ){
        if (objects.size() > 0) {
            const CSerialObject* so = dynamic_cast<const CSerialObject*>(objects[0].object.GetPointerOrNull());
            if (so) {
                string fp = view_name + ":" + so->GetThisTypeInfo()->GetName();
                dynamic_cast<IWMClient&>(*prj_view).SetFingerprint(fp);
            }
        }

        view_srv->AddToWorkbench(*view, bFloat);

        bool ok = false;
        try {
            // initialization logging
            //
            {
                const CViewTypeDescriptor& vdescr = prj_view->GetTypeDescriptor();
                LOG_POST(Info << "InitView = " << vdescr.GetLabel());

                int ix = 0;
                int trace_limit = 10;
                string arg_label;

                ITERATE( TConstScopedObjects, it, objects ){
                    #ifdef _DEBUG
                        trace_limit = 100;
                    #endif

                    if( ix++ == trace_limit ){
                        LOG_POST(Info << "   and " << (objects.size() - trace_limit) << " objects more" );

                        break;
                    }
                    const CObject* obj = it->object.GetPointer();
                    CScope* scope = const_cast<CScope*>(it->scope.GetPointer());
                    arg_label.clear();
                    try {
                        CLabel::GetLabel(*obj, &arg_label, CLabel::eDefault, scope);
                    }
                    catch (const exception&) {
                        arg_label = "EXCEPTION";
                    }

                    LOG_POST(Info << "    object=" << arg_label);

                }
            }

            ok = prj_view->InitView(objects, params);

        } catch( CException& e ){
            LOG_POST( Error << e.what() );
            #ifdef _DEBUG
            NcbiErrorBox( e.what(), "Error while opening view" );
            #else
            NcbiErrorBox( e.GetMsg(), "Open View Error" );
            #endif
        }

        if( ok ){

            OnViewAttached(prj_view);

            CProjectViewBase* prj_view_base = 
                dynamic_cast<CProjectViewBase*>(prj_view.GetPointer())
            ;
            if( prj_view_base ){
                prj_view_base->RefreshViewWindow();
            }
        
        } else {
            view_srv->RemoveFromWorkbench(*view);
            prj_view.Reset();
        }
    } else {
        _ASSERT(false);
        NCBI_THROW(CProjectServiceException, eInvalidArguments,
                   "Created view is not a Project View");
    }
    return prj_view;
}

IProjectView* CProjectService::FindView(const CObject& mainObject, const string& viewType)
{
    if (viewType.empty())
        return 0;

    IViewManagerService* view_srv = m_ServiceLocator->GetServiceByType<IViewManagerService>();
    _ASSERT(view_srv);
    IViewManagerService::TViews views;
    view_srv->GetViews(views);
    NON_CONST_ITERATE(IViewManagerService::TViews, it, views) {
        IProjectView* project_view = dynamic_cast<IProjectView*>((*it).GetPointer());
        if (project_view && project_view->GetLabel(IProjectView::eType) == viewType) {
            TConstScopedObjects objects;
            project_view->GetMainObject(objects);
            if (objects.size() == 1 && &mainObject == objects.front().object.GetPointer()) {
                    return project_view;
            }
        }
    }
    return 0;
}

void CProjectService::FindViews(vector<CIRef<IProjectView> >& projectViews, const CObject& mainObject)
{
    IViewManagerService* view_srv = m_ServiceLocator->GetServiceByType<IViewManagerService>();
    _ASSERT(view_srv);
    IViewManagerService::TViews views;
    view_srv->GetViews(views);
    NON_CONST_ITERATE(IViewManagerService::TViews, it, views) {
        IProjectView* project_view = dynamic_cast<IProjectView*>((*it).GetPointer());
        if (project_view) {
            TConstScopedObjects objects;
            project_view->GetMainObject(objects);
            if (objects.size() == 1 && &mainObject == objects.front().object.GetPointer())
                projectViews.push_back(CIRef<IProjectView>(project_view));
        }
    }
}

void CProjectService::FindViews(vector<CIRef<IProjectView> >& projectViews)
{
    IViewManagerService* view_srv = m_ServiceLocator->GetServiceByType<IViewManagerService>();
    _ASSERT(view_srv);
    IViewManagerService::TViews views;
    view_srv->GetViews(views);
    NON_CONST_ITERATE(IViewManagerService::TViews, it, views) {
        IProjectView* project_view = dynamic_cast<IProjectView*>((*it).GetPointer());
        if (project_view) {
            projectViews.push_back(CIRef<IProjectView>(project_view));
        }
    }
}

void CProjectService::ActivateProjectView(IProjectView* projectView)
{
    if (!projectView)
        return;

    IWindowManagerService* wm_srv = m_ServiceLocator->GetServiceByType<IWindowManagerService>();
    IWMClient* client = dynamic_cast<IWMClient*>(projectView);
    _ASSERT(client);
    wm_srv->ActivateClient(*client);
}

CIRef<IProjectView> CProjectService::ShowView(const string& viewName, FWindowFactory widgetFactory, TConstScopedObjects& objects, bool bFloat)
{
    CIRef<IProjectView> view;
    if (objects.empty()) return view;

    view.Reset(FindView(*(objects.front().object), viewName));
    if (view) {
        ActivateProjectView(view);
        return view;
    }

    if (widgetFactory == 0) {
        return AddProjectView(viewName, objects, 0, bFloat);
    } else {
        return CSimpleProjectView::CreateView(viewName, widgetFactory, m_ServiceLocator, objects, 0, bFloat);
    }

    return view;
}


void CProjectService::RemoveProjectView(IProjectView& view)
{
    sAssertMainThread("Views can be removed only from the main thread");

    IViewManagerService* view_srv = m_ServiceLocator->GetServiceByType<IViewManagerService>();

    _ASSERT(view_srv);

    x_RemoveView(*view_srv, view);
}


void CProjectService::RemoveAllProjectViews()
{
    sAssertMainThread("Views can be removed only from the main thread");

    IViewManagerService* view_srv = m_ServiceLocator->GetServiceByType<IViewManagerService>();

    _ASSERT(view_srv);

    IViewManagerService::TViews views;
    view_srv->GetViews(views);

    for(  size_t i = 0;  i < views.size();  i++ )   {
        IView& view = *views[i];
        IProjectView* prj_view = dynamic_cast<IProjectView*>(&view);
        if(prj_view)    {
            x_RemoveView(*view_srv, *prj_view);
        }
    }
}


void CProjectService::x_RemoveView(IViewManagerService& view_srv, 
                                   IProjectView& view, 
                                   bool reset_hist_async)
{
    CIRef<IProjectView> hold(&view);

    // disconnect from the document
    view.SetAsyncDestroy(reset_hist_async);
    view.DestroyView();

    // disconnect for Workbench (removes it from Window Manager)
    view_srv.RemoveFromWorkbench(view);
}

// attach a view to the document manager
void CProjectService::OnViewAttached(IProjectView* view)
{
    if( view ){
        /*
        const CGuiRegistry& reg = CGuiRegistry::GetInstance();
        bool save_views = reg.GetBool("GBENCH.Application.SaveViews", false);
        if (save_views) {
            RecordViewOpen(view);
        }
        */

        REPORT_USAGE("views", .Add("view_name", view->GetLabel(IProjectView::eType)));

        if (m_Workspace) {
            CGBDocument* doc = dynamic_cast<CGBDocument*>(m_Workspace->GetProjectFromId(view->GetProjectId()));
            if (doc) {
                CProjectViewEvent ev( *view, CViewEvent::eViewAttached );
                doc->Send (&ev);
            }
        }
    }
}

CProjectTreePanel* CProjectService::GetProjectTreePanel()
{
    IViewManagerService* view_srv = m_ServiceLocator->GetServiceByType<IViewManagerService>();

    IViewManagerService::TViews views;
    view_srv->GetViews(views);
    NON_CONST_ITERATE(IViewManagerService::TViews, it, views) {
        CProjectTreeView* projectTree = dynamic_cast<CProjectTreeView*>((*it).GetPointer());
        if (projectTree) return dynamic_cast<CProjectTreePanel*>(projectTree->GetWindow());
    }

    return 0;
}

void CProjectService::GetObjProjects(TConstScopedObjects& objects,
                                     vector<int>& ids)
{
    if (objects.empty())
        return;

    CRef<CGBWorkspace> ws = GetGBWorkspace();
    if (!ws) return;

    set<int> idSet;
    NON_CONST_ITERATE(TConstScopedObjects, it, objects) {
        CScope* scope = it->scope;
        if (!scope) continue;
        CGBProjectHandle* project = ws->GetProjectFromScope(*scope);
        if (project) {
            idSet.insert(project->GetId());
        }
    }
    copy(idSet.begin(), idSet.end(), back_inserter(ids));
}

void CProjectService::AddProject(CGBDocument& doc)
{
    if (!m_Workspace) return;

    m_Workspace->SetWorkspace().AddProject(doc);

    CProjectTreePanel* projectTreePanel = GetProjectTreePanel();
    if (projectTreePanel)
        projectTreePanel->ProjectAdded(doc);
}

void CProjectService::RemoveProject(CGBDocument& doc)
{
    if (!m_Workspace) return;

    size_t id = doc.GetId();
    doc.UnloadProject(true);
    if (m_Workspace->RemoveProject(id)) {
        CProjectTreePanel* projectTreePanel = GetProjectTreePanel();
        if (projectTreePanel) projectTreePanel->ProjectRemoved(id);
    }
}

void CProjectService::x_UpdateWorkspaceLabel()
{
    CProjectTreePanel* projectTreePanel = GetProjectTreePanel();
    if (projectTreePanel) projectTreePanel->UpdateWorkspaceLabel();
}

const CProjectItem* CProjectService::GetProjectItem(const CObject& object, objects::CScope& scope)
{
    if (!m_Workspace) return 0;

    CGBDocument* doc = dynamic_cast<CGBDocument*>(m_Workspace->GetProjectFromScope(scope));
    if (!doc) return 0;

    for (CTypeConstIterator<CProjectItem> it(doc->GetData()); it; ++it) {
        if (it->GetObject() == &object) {
            return &*it;
        }
    }

    return 0;
}

END_NCBI_SCOPE
