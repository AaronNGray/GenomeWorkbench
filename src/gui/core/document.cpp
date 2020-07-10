/* $Id: document.cpp 45001 2020-05-05 20:50:53Z asztalos $
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
 * Author:  Vladimir Tereshkov, Andrey Yazhuk
 *
 */

#include <ncbi_pch.hpp>


#include <gui/core/document.hpp>

#include <gui/framework/view_manager_service.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/framework/job_ui_task.hpp>
#include <gui/core/project_tree_view.hpp>
#include <gui/core/project_tree_panel.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/ui_data_source_service.hpp>
#include <gui/utils/command_processor.hpp>
#include <gui/objutils/registry.hpp>
#include <gui/objutils/project_item_extra.hpp>

#include <gui/objutils/attached_handles.hpp>
#include <gui/utils/id_generator.hpp>
#include <gui/utils/extension.hpp>

#include <gui/core/project_item_extension.hpp>
#include <gui/utils/extension_impl.hpp>

#include <gui/widgets/wx/message_box.hpp>

#include <gui/objutils/label.hpp>
#include <gui/utils/app_job_impl.hpp>

#include <objmgr/object_manager.hpp>

/// specific supported project versions
#include <gui/objects/GBProject.hpp>
#include <objects/gbproj/GBProject_ver2.hpp>
#include <gui/objects/GBProjectHandle.hpp>
#include <objects/gbproj/ProjectDescr.hpp>
#include <objects/gbproj/ProjectFolder.hpp>
#include <objects/gbproj/ProjectItem.hpp>
#include <objects/gbproj/FolderInfo.hpp>
#include <objects/gbproj/ProjectHistoryItem.hpp>
#include <objects/gbproj/ProjectAnnot.hpp>
#include <gui/objects/PluginMessage.hpp>

#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/User_object.hpp>

#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objects/general/Date.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>

#include <serial/objistr.hpp>
#include <serial/serial.hpp>
#include <serial/pack_string.hpp>
#include <serial/objostr.hpp>
#include <serial/iterator.hpp>

#include <corelib/rwstream.hpp>

#include <connect/ncbi_conn_stream.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const char* kLoaderName = "Loader Name";

/////////////////////////////////////////////////////////////////////////////
/// CGBDocument

CGBDocument::CGBDocument(CProjectService* srv)
    : m_Service(srv)
    , m_UndoManager(new CUndoManager(x_GetUndoSize(), this))
    , m_AssemblyInitialized(false)
{
    m_Id = GetNextId();
    LOG_POST(Info << "CGBDocument empty constructor");
}

CGBDocument::CGBDocument(CProjectService* srv, TId id, CGBProject_ver2& proj_impl)
    : CGBProjectHandle(proj_impl)
    , m_Service(srv)
    , m_UndoManager(new CUndoManager(x_GetUndoSize(), this))
    , m_AssemblyInitialized(false)
{
    m_Id = id;
    LOG_POST(Info << "CGBDocument(src, id, CGBProject_ver2) id = " << id << " " << this);
}


CGBDocument::~CGBDocument(void)
{
    for (auto& i : m_JobAdapters)
        i->Cancel();

    LOG_POST(Info << "CGBDocument::~CGBDocument");
}

wxString CGBDocument::GetFileName() const
{
    return FnToWxString(CGBProjectHandle::GetFilename());
}

void CGBDocument::SetFileName (const wxString& filename)
{
    CGBProjectHandle::SetFilename(FnToStdString(filename));
}

string CGBDocument::GetDefaultAssembly() const
{
    if (!m_AssemblyInitialized)
        x_InitializeAssembly();
    return m_AssemblyAccession;
}

void CGBDocument::x_InitializeAssembly() const
{
    m_AssemblyAccession.erase();

    if (!IsLoaded()) {
        m_AssemblyInitialized = false;
        return;
    }

    m_AssemblyInitialized = true;
    const CGC_Assembly* assembly = 0;
    for (CTypeConstIterator<CProjectItem> it(GetData()); it; ++it) {
        const CGC_Assembly* obj = dynamic_cast<const CGC_Assembly*>((*it).GetObject());
        if (obj) {
            if (assembly) return; // Several assemblies in project
            assembly = obj;
        }
    }

    if (assembly)
        m_AssemblyAccession = assembly->GetAccession();
}

void CGBDocument::x_ClearAssembly() const
{
    m_AssemblyAccession.erase();
    m_AssemblyInitialized = false;
}

void CGBDocument::CreateProjectScope()
{
    LOG_POST(Info << "CGBDocument::CreateProjectScope() " << this);

    _ASSERT( ! GetScope());

    CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
    CRef<CScope> scope(new CScope(*obj_mgr));
    scope->AddDefaults();
    SetScope(scope.GetPointer());
}

class CDocLoadingJob : public CAppJob
{
public:
    CDocLoadingJob(CGBDocument& doc, const wxString& path)
        : CAppJob("Load Project Job"), m_Document(&doc), m_Path(path) {}

    /// @name IAppJob implementation
    /// @{
    virtual EJobState   Run();
    /// @}

protected:
    CRef<CGBDocument> m_Document;
    wxString  m_Path;
};

static CProjectTreePanel* s_FindProjectTreePanel(IServiceLocator* serviceLocator)
{
    IViewManagerService* view_srv = serviceLocator->GetServiceByType<IViewManagerService>();
    IViewManagerService::TViews views;
    view_srv->GetViews(views);
    NON_CONST_ITERATE(IViewManagerService::TViews, it, views) {
        CProjectTreeView* projectTree = dynamic_cast<CProjectTreeView*>((*it).GetPointer());
        if (projectTree)
            return dynamic_cast< CProjectTreePanel*>(projectTree->GetWindow());
    }

    return 0;
}

void CGBDocument::ViewLabelChanged(IProjectView& view)
{
    CProjectTreePanel* projectTree = s_FindProjectTreePanel(m_Service->GetServiceLocator());
    if (projectTree) projectTree->UpdateViewLabel(*this, view);
}

void CGBDocument::ProjectStateChanged()
{
    CProjectTreePanel* projectTree = s_FindProjectTreePanel(m_Service->GetServiceLocator());
    if (projectTree) projectTree->ProjectStateChanged(*this);
    x_ClearAssembly();
}

void CGBDocument::x_ProjectViewsChanged()
{
    CProjectTreePanel* projectTree = s_FindProjectTreePanel(m_Service->GetServiceLocator());
    if (projectTree) projectTree->ProjectViewsChanged(*this);
}

void CGBDocument::ProjectItemsChanged()
{
    CProjectTreePanel* projectTree = s_FindProjectTreePanel(m_Service->GetServiceLocator());
    if (projectTree) projectTree->ProjectUpdateItems(*this);
    x_ClearAssembly();
}

void CGBDocument::x_ProjectUpdateLabel()
{
    CProjectTreePanel* projectTree = s_FindProjectTreePanel(m_Service->GetServiceLocator());
    if (projectTree) projectTree->ProjectUpdateLabel(*this);
}

IAppJob* CGBDocument::CreateLoadJob(const wxString& fileName)
{
    if (IsLoaded() || m_LoadJob)
        return 0;

    m_LoadJob.Reset(new CDocLoadingJob(*this, fileName));

    ProjectStateChanged();

    return m_LoadJob.GetPointer();
}

bool CGBDocument::IsLoading() const
{
    return m_LoadJob;
}

void CGBDocument::CancelLoading()
{
    if (m_LoadJob) {
        m_LoadJob->RequestCancel();
        m_LoadJob.Reset();
        ProjectStateChanged();
    }
}

void CGBDocument::ResetLoading()
{
    m_LoadJob.Reset();
}

IAppJob::EJobState CDocLoadingJob::Run()
{
    bool ok = m_Document->LoadFile(m_Path, x_GetICanceled());

    if (!ok) {
        if (IsCanceled())
            return eCanceled;

        string s = "Failed to load project: \"" + string(m_Path.ToUTF8()) + "\".";
        CFastMutexGuard lock(m_Mutex);
        m_Error.Reset(new CAppJobError(s));
        return eFailed;
    }

    _ASSERT(m_Document->GetScope() == NULL);
    m_Document->CreateProjectScope();
    m_Document->ReplaceGiSeqIds(x_GetICanceled());

    if (IsCanceled())
        return eCanceled;

    m_Document->SetLoaded();

    return eCompleted;
}

/// NULL stream writer for test serialization
class CNullWriter : public IWriter
{
public:
    ERW_Result Write(const void* buf, size_t count, size_t* bytes_written)
    {
        if (bytes_written) {
            *bytes_written = count;
        }
        return eRW_Success;
    }

    ERW_Result Flush()
    {
        return eRW_Success;
    }
};

/// Attach a view to the current document
void CGBDocument::x_AttachView(IProjectView* view)
{
    _ASSERT(view);

    m_Views.push_back(CIRef<IProjectView>(view));

    x_AssignViewIndex(view);

    // event routing
    CEventHandler* handler = dynamic_cast<CEventHandler*>(view);
    if(handler) {
        AddListener(handler);
        handler->AddListener(this);
    }

    x_ProjectViewsChanged();
}


/// detach a view from the current document
void CGBDocument::x_DetachView(IProjectView* view)
{
    _ASSERT(view);

    CIRef<IProjectView> ref(view);

    // unregister event handling
    CEventHandler* handler = dynamic_cast<CEventHandler*>(view);
    if (handler) {
        RemoveListener(handler);
        handler->RemoveListener(this);
    }

    m_ViewToIndex.erase(view);
    TViews::iterator it = std::find(m_Views.begin(), m_Views.end(), ref);
    m_Views.erase(it);

    x_ProjectViewsChanged();

    x_FireViewEvent( *view, TEvent::eViewReleased );
}


void CGBDocument::x_AssignViewIndex(IProjectView* view)
{
    _ASSERT(view);
    const string& title = view->GetClientLabel();
    int max_index = -1;

    ITERATE (TViewToIndex, it, m_ViewToIndex) {
        IProjectView* v = it->first;
        if (v->GetClientLabel() == title)   { // view of the same type
            max_index = max(max_index, it->second);
        }
    }
    m_ViewToIndex[view] = max_index + 1;
}


/// Retrieve the existing views for this class
const CGBDocument::TViews& CGBDocument::GetViews(void) const
{
    return m_Views;
}


CGBDocument::TViews& CGBDocument::x_GetViews(void)
{
    return m_Views;
}

void CGBDocument::x_FireProjectChanged( TEvent::EEventSubtype subtype )
{
    TEvent* ev = new TEvent( GetId(), subtype );
    if ((CProjectViewEvent::eDataChanging == subtype) || (CProjectViewEvent::eUnloadProject == subtype))
        Send( CRef<CEvent>( ev ) );
    else
        Post( CRef<CEvent>( ev ) );
}

void CGBDocument::x_FireViewEvent( IProjectView& view, CViewEvent::EEventType type )
{
    TEvent ev( view, type );

    Send( &ev );
}

/*
void CGBDocument::x_PostViewEvent( IProjectView& view, CViewEvent::EEventType type )
{
    TEvent* ev = new TEvent( view, TEvent::eProjectChanged );
    //ev->m_View.Reset( &view );

    CRef<CEvent> evt( ev );
    Post( evt );
}
*/

struct STempFileLocker
{
    string filename;

    STempFileLocker(const string& tmp_name)
        : filename(tmp_name)
    {
    }

    void Lock()
    {
    }

    void Unlock()
    {
        if ( !filename.empty() ) {
            CFile(filename).Remove();
        }
    }
};

bool CGBDocument::LoadFile(const wxString& fname, ICanceled* canceledCallback)
{
    auto_ptr<CNcbiIfstream> istr;
    string tmp_name;
    if (fname.StartsWith(wxT("http://")) || fname.StartsWith(wxT("https://"))
        ||  fname.StartsWith(wxT("ftp://"))) {

        auto_ptr<CNcbiIstream> is;
        if (fname.StartsWith(wxT("http"))) {  // this covers both http:// and https://
            // we always open with the *full* path - this will pass params on
            // the command line and log them in the weblogs.
            auto_ptr<CConn_HttpStream> http_str(new CConn_HttpStream(ToStdString(fname)));
            is.reset(http_str.release());
        } else {
            // For FTP connections, we need to separate host name from the path name
            // separate the host to contact
            string _fname = ToStdString(fname);
            string host = _fname;
            string::size_type pos = host.find_first_of("/", 6);
            if (pos != string::npos) {
                host.erase(pos);
            }
            host.erase(0, 6);

            // separate the path to retrieve
            string path;
            if (pos != string::npos) {
                path = _fname.substr(pos, _fname.length() - pos);
            }

            // separate the file name
            //SOCK_SetDataLoggingAPI(eOn);
            is.reset(new CConn_FTPDownloadStream(host, path));
        }

        /// we will be trying a number of things here, so create a
        /// temporary file stream for this
        tmp_name = CFile::GetTmpName();
        {{
            CNcbiOfstream ostr(tmp_name.c_str(), ios::binary);
            NcbiStreamCopy(ostr, *is);
        }}
        istr.reset(new CNcbiIfstream(tmp_name.c_str(), ios::binary|ios::in));

    } else {
        /// default file connection
        istr.reset(new CNcbiIfstream(fname.fn_str(), ios::binary|ios::in));
    }
    _ASSERT(istr.get());

    STempFileLocker tmp_file(tmp_name);
    CGuard<STempFileLocker> LOCK(tmp_file);

    return CGBProjectHandle::Load(*istr, canceledCallback);
}

void CGBDocument::Save(const wxString& abs_path)
{
    CGuiRegistry& reg = CGuiRegistry::GetInstance();
    /// configuration parameters controlling serialization

    /// should the application keep back-up files?
    bool keep_backups =
        reg.GetBool("GBENCH.Application.IO.KeepBackups", true);

    /// do we serialize in text or binary mode?
    /// we have a compiled-in default and a user-override
    ESerialDataFormat fmt =
#ifdef _DEBUG
        eSerial_AsnText;
#else
        eSerial_AsnBinary;
#endif
    {{
        string fmt_str = reg.GetString("GBENCH.Application.IO.SerialFormat");
        if (NStr::CompareNocase(fmt_str, "text") == 0) {
            fmt = eSerial_AsnText;
        } else if (NStr::CompareNocase(fmt_str, "binary") == 0) {
            fmt = eSerial_AsnBinary;
        }
    }}

    // save view-specific settings
    try {
        ITERATE (TViews, iter, m_Views) {
            const IViewSettingsAtProject* setts =
                dynamic_cast<const IViewSettingsAtProject*>(iter->GetPointer());
            if (setts)
                setts->SaveSettingsAtProject(*this);
        }
    } NCBI_CATCH("Exception while saving view settings to project");

    GUI_AsyncExec(
        [this, &abs_path, fmt, keep_backups](ICanceled&) { this->x_SaveFile(abs_path, fmt, keep_backups); },
        wxT("Saving Project..."));

    x_ProjectUpdateLabel();
}


void CGBDocument::x_SaveFile(const wxString& fname, ESerialDataFormat fmt, bool keep_backups)
{
    string fileName(fname.ToUTF8());

    LOG_POST(Info << "CGBDocument::x_SaveFile(" << fileName << ")");

    ///
    /// step 1: write to a new file
    /// we avoid overwriting an existing file before we are sure that
    /// serialization has succeeded
    ///

    bool serialization_success = true;
    wxString serialized_fname;
    try {
        /// rename the existing file, if any
        serialized_fname = fname;
        for (int i = 1;  ;  ++i) {
            wxString f = wxString::Format(wxT("%s.%d"), fname.c_str(), i);
            if ( !::wxFileExists(f) ) {
                serialized_fname = f;
                break;
            }
        }

        CNcbiOfstream ostr(serialized_fname.fn_str(), ios::binary);
        CGBProjectHandle::Save(ostr, fmt);
    }
    catch (std::exception& e) {
        LOG_POST(Error << "CGBProjectHandle::Save(" << fname
                 << "): error in serialization: " << e.what());
        serialization_success = false;
    }

    /// scan the serialized file to make sure that nothing bad happened
    if (serialization_success) {
        try {
            CNcbiIfstream istr(serialized_fname.fn_str(), ios::binary);
            auto_ptr<CObjectIStream> is(CObjectIStream::Open(fmt, istr));

            const IGBProject& project = GetProject();
            switch (project.GetVersion()) {
            case IGBProject::eVersion1:
                is->Skip(CGBProject::GetTypeInfo());
                break;

            case IGBProject::eVersion2:
                is->Skip(CGBProject_ver2::GetTypeInfo());
                break;

            default:
                _ASSERT(false);
                NCBI_THROW(CException, eUnknown, "Unknown project version");
                break;
            }
            is->Close();
            if ( !istr ) {
                NCBI_THROW(CException, eUnknown, "stream read failure");
            }
        }
        catch (std::exception& e) {
            LOG_POST(Error << "CGBProjectHandle::Save(" << fname
                    << "): error in scanning file: " << e.what());
            serialization_success = false;
        }
    }

    if ( !serialization_success ) {
#ifndef _DEBUG
        /// in debug mode, we preserve our bread-crumbs
        ::wxRemoveFile(serialized_fname);
#endif

        string msg("An error occurred while saving '");
        msg += fname.ToUTF8();
        msg += "'.\n";
        if ( ::wxFileExists(fname) ) {
            msg += "The original file was not overwrtten and should be safe.\n";
        }
        msg += "\nYou can try to save the file again, or optionally\n";
        msg += "export any project items individually to preserve the data.";
        NCBI_THROW(CException, eUnknown, msg);
    }

    ///
    /// success!
    ///

    /// rename the existing file, if any
    if (keep_backups) {
        wxString backup_fname = fname + wxT(".bak");
        if (::wxFileExists(backup_fname))
            ::wxRemoveFile(backup_fname);
        if (::wxFileExists(fname))
            ::wxRenameFile(fname,backup_fname);
    } else  if (::wxFileExists(fname)) {
            ::wxRemoveFile(fname);
    }
    ::wxRenameFile(serialized_fname, fname);
}

size_t CGBDocument::x_GetUndoSize()
{
	static int undoSize = -1;
	if (undoSize == -1) {
		undoSize = 25;
		try {
            CRegistryReadView view = CGuiRegistry::GetInstance().GetReadView("GBENCH.Project.Settings");
			undoSize = view.GetInt("UndoBufferSize", undoSize);
		}
		catch (const exception&) {
		}
	}
	return (size_t)undoSize;
}

static void s_AttachProjectItem(CProjectItem& item, CScope& scope)
{
    CRef<CAttachedHandles> handles(new CAttachedHandles());
	handles->Attach(scope, item);
	item.SetUserObject(handles);
    item.SetDisabled(false);
}

static void s_DetachProjectItem(CProjectItem& item, CScope& scope)
{
	CAttachedHandles* handles = dynamic_cast<CAttachedHandles*>(item.GetUserObject());
	if (handles)
		handles->Detach(scope);
    item.SetDisabled(true);
}

static void s_CallExtensionProjectItemAttached(CGBDocument& doc, vector<IProjectItemExtension*>& clients, CProjectItem& item, IServiceLocator* srvLocator)
{
    ITERATE(vector<IProjectItemExtension*>, it, clients) {
        string extName = "Unknown extension";
        IExtension* ext = dynamic_cast<IExtension*>(*it);
        if (ext) extName = ext->GetExtensionIdentifier();
        try {
            (*it)->ProjectItemAttached(item, doc, srvLocator);
        } NCBI_CATCH("Project item attached \"" + extName + "\" error.");
    }
}

static void s_CallExtensionProjectItemDetached(CGBDocument& doc, vector<IProjectItemExtension*>& clients, CProjectItem& item, IServiceLocator* srvLocator)
{
    ITERATE(vector<IProjectItemExtension*>, it, clients) {
        string extName = "Unknown extension";
        IExtension* ext = dynamic_cast<IExtension*>(*it);
        if (ext) extName = ext->GetExtensionIdentifier();
        try {
            (*it)->ProjectItemDetached(item, doc, srvLocator);
        } NCBI_CATCH("Project item detached \"" + extName + "\" error.");
    }
}

void CGBDocument::x_CloseProjectItemViews(CProjectItem& item)
{
    const CSerialObject* obj = item.GetObject();
    if (obj) {
        set<IProjectView*> viewToDelete;
        map<const CObject*, IProjectView*> objToView;
        for (TViews::iterator it = m_Views.begin(); it != m_Views.end(); ++it) {
            TConstScopedObjects objects;
            (*it)->GetMainObject(objects);
            for (TConstScopedObjects::const_iterator it2 = objects.begin(); it2 != objects.end(); ++it2) {
                const CObject* object = it2->object.GetPointer();
                if (object == obj)
                    viewToDelete.insert(*it);
                else
                    objToView[object] = it->GetPointer();
            }
        }

        if (!objToView.empty()) {
            for (CObjectConstIterator it(*obj); it; ++it) {
                map<const CObject*, IProjectView*>::iterator it2 = objToView.find(&*it);
                if (it2 != objToView.end()) {
                    viewToDelete.insert(it2->second);
                }
            }
        }

        ITERATE (set<IProjectView*>, it, viewToDelete) {
            m_Service->RemoveProjectView(**it);
        }
    }
}

bool CGBDocument::RemoveProjectItem(CProjectItem* item)
{
    if (!item) return false;
    vector<CProjectItem*> items;
    items.push_back(item);
    return RemoveProjectItems(items);
}

bool CGBDocument::RemoveProjectItems(const vector<CProjectItem*>& items)
{
    x_FireProjectChanged(CProjectViewEvent::eDataChanging);
    x_DetachProjectItems(items);
    x_FireProjectChanged(CProjectViewEvent::eData);

    bool removed = false;

    for (size_t i = 0; i < items.size(); ++i) {
        CProjectItem* item = items[i];
        if (!item) continue;

        int id = item->GetId();
        CProjectFolder* folder = SetData().FindProjectItemFolder(id);
        if (!folder || folder->GetProjectItem(id) != item) continue;
        folder->RemoveChildItem(id);
        removed = true;
    }

    if (removed) {
        SetDirty (true);
        ProjectItemsChanged();
        return true;
    }

    return false;
}

void CGBDocument::AttachProjectItem(CProjectItem* item)
{
    if (!item) return;
    vector<CProjectItem*> items;
    items.push_back(item);
    AttachProjectItems(items);
}

void CGBDocument::DetachProjectItem(CProjectItem* item)
{
    if (!item) return;
    vector<CProjectItem*> items;
    items.push_back(item);
    DetachProjectItems(items);
}

void CGBDocument::AttachProjectItems(const vector<CProjectItem*>& items)
{
    x_FireProjectChanged(CProjectViewEvent::eDataChanging);

    CScope* scope = GetScope();
    _ASSERT(scope);

    for (size_t i = 0; i < items.size(); ++i) {
        CProjectItem* item = items[i];
        if (!item) continue;

        s_AttachProjectItem(*item, *scope);

        vector<IProjectItemExtension*> clients;
        GetExtensionAsInterface(EXT_POINT__PROJECT_ITEM_EXTENSION, clients);
        if (!clients.empty()) {
            s_CallExtensionProjectItemAttached(*this, clients, *item, m_Service->GetServiceLocator());
        }
    }

    x_FireProjectChanged(CProjectViewEvent::eData);
}

void CGBDocument::DetachProjectItems(const vector<CProjectItem*>& items)
{
    x_FireProjectChanged(CProjectViewEvent::eDataChanging);
    x_DetachProjectItems(items);
    x_FireProjectChanged(CProjectViewEvent::eData);
}

bool CGBDocument::x_DetachProjectItems(const vector<CProjectItem*>& items)
{
    vector<CProjectItem*> enabled;

    for (size_t i = 0; i < items.size(); ++i) {
        CProjectItem* item = items[i];
        if (item && item->IsEnabled()) {
            vector<IProjectItemExtension*> clients;
            GetExtensionAsInterface(EXT_POINT__PROJECT_ITEM_EXTENSION, clients);
            if (!clients.empty()) {
                s_CallExtensionProjectItemDetached(*this, clients, *item, m_Service->GetServiceLocator());
            }
            x_CloseProjectItemViews(*item);
            enabled.push_back(item);
        }
    }

    CScope* scope = GetScope();
    _ASSERT(scope);

    if (!enabled.empty()) {
        GUI_AsyncExec(
            [&enabled, scope](ICanceled&)
            {
                for (auto pi : enabled)
                    s_DetachProjectItem(*pi, *scope);
            },
            wxT("Unloading project item..."));
        return true;
    }

    return false;
}

class CAddItemsData : public CObject
{
public:
    CAddItemsData(const string& folderName,
        vector<CRef<CProjectItem> >& items,
        const CGBDocument::TLoaders& loaders) :
        m_FolderName(folderName), m_Items(items), m_Loaders(loaders) {}

    const string& GetFolderName() const { return m_FolderName; }
    vector<CRef<CProjectItem> >& GetProjectItems() { return m_Items; }
    const CGBDocument::TLoaders& GetLoaders() const { return m_Loaders; }

private:
    string m_FolderName;
    vector<CRef<CProjectItem> > m_Items;
    CGBDocument::TLoaders m_Loaders;
};

class CAttachProjectItemsJob : public CJobCancelable
{
public:
    CAttachProjectItemsJob(CAddItemsData& data, CScope& scope)
        : m_Data(&data), m_Scope(&scope), m_Descr("Attach Project Items") {}
    virtual ~CAttachProjectItemsJob() {}

    /// @name IAppJob implementation
    /// @{
    virtual string                      GetDescr() const { return m_Descr; }
    virtual EJobState                   Run()
    {
        m_Error.Reset(new CAppJobError("N/A"));
        for (auto& i : m_Data->GetProjectItems()) {
            if (IsCanceled()) return eCanceled;
            s_AttachProjectItem(*i, *m_Scope);
        }
        if (IsCanceled()) return eCanceled;
        return eCompleted;
    }
    virtual CConstIRef<IAppJobProgress> GetProgress()
    {
        return CConstIRef<IAppJobProgress>();
    }
    virtual CRef<CObject>               GetResult()
    {
        return CRef<CObject>(m_Data.GetPointer());
    }
    virtual CConstIRef<IAppJobError>    GetError()
    {
        return CConstIRef<IAppJobError>(m_Error.GetPointer());
    }
    /// @}

protected:
    virtual string x_GetJobName() const { return m_Descr; }

private:
    CRef<CAddItemsData> m_Data;
    CRef<CScope> m_Scope;
    CRef<CAppJobError> m_Error;
    string m_Descr;
};

void CGBDocument::OnJobResult(CObject* result, CJobAdapter& adapter)
{
    CAddItemsData* data = dynamic_cast<CAddItemsData*>(result);
    if (data) {
        x_AddItems(data->GetFolderName(), data->GetProjectItems(), data->GetLoaders());
    }

    for (list<CRef<CJobAdapter> >::iterator
        it = m_JobAdapters.begin(); it != m_JobAdapters.end(); ++it) {
        if ((*it).GetPointer() == &adapter) {
            m_JobAdapters.erase(it);
            break;
        }
    }
}

void CGBDocument::OnJobFailed(const string&, CJobAdapter& adapter)
{
    for (list<CRef<CJobAdapter> >::iterator
        it = m_JobAdapters.begin(); it != m_JobAdapters.end(); ++it) {
        if ((*it).GetPointer() == &adapter) {
            m_JobAdapters.erase(it);
            break;
        }
    }
}

void CGBDocument::AddItems(const string& folderName,
                           vector<CRef<CProjectItem> >& items,
                           const TLoaders& loaders)
{
    // Make unique project item names
    CUniqueLabelGenerator itemNames;
    for (CTypeConstIterator<CProjectItem> it(GetData()); it; ++it)
        itemNames.AddExistingLabel(it->GetLabel());

    CScope& scope = *GetScope();

    for (size_t i = 0; i < items.size(); ++i) {
        CProjectItem& item = *items[i];

        // make sure item has a label, create it if needed
        if (!item.IsSetLabel() || item.GetLabel().empty()) {
            CLabel::SetLabelByData(item, &scope);
        }

        // make sure the label is unique
        string label = item.GetLabel();
        string uniqueLabel = itemNames.MakeUniqueLabel(label);
        itemNames.AddExistingLabel(uniqueLabel);
        if (label != uniqueLabel) {
            item.SetLabel(uniqueLabel);

            if (item.GetItem().IsAnnot()) {
                CSeq_annot& annot = item.SetItem().SetAnnot();
                if (annot.CanGetDesc()) {
                    for (auto& d : annot.SetDesc().Set()) {
                        if (d->IsName()) {
                            if (d->GetName() == label)
                                d->SetName(uniqueLabel);
                            break;
                        }
                    }
                }
            }
        }
    }

    x_FireProjectChanged(CProjectViewEvent::eDataChanging);

    CRef<CJobAdapter> adapter(new CJobAdapter(*this));
    CRef<CAddItemsData> data(new CAddItemsData(folderName, items, loaders));
    CIRef<IAppJob> job(new CAttachProjectItemsJob(*data, *GetScope()));
    adapter->Start(*job);
    m_JobAdapters.push_back(adapter);

    CAppJobDispatcher::TJobID jobId = adapter->GetJobId();
    if (jobId != CAppJobDispatcher::eInvalidJobID) {
        IServiceLocator* srvLocator = m_Service->GetServiceLocator();

        string projName = "N/A";
        if (IsLoaded()) {
            const objects::CProjectDescr& descr = GetDescr();
            projName = descr.GetTitle();
        }
        string msg = "Adding items to \"" + projName + "\"";
        CIRef<IAppTask> task(new CJobUITask(msg, jobId));
        CAppTaskService* task_srv = srvLocator->GetServiceByType<CAppTaskService>();
        if (task_srv) task_srv->AddTask(*task);
    }
}


void CGBDocument::x_AddItems(const string& folderName,
		                     vector<CRef<CProjectItem> >& items,
                             const TLoaders& loaders)
{
	CProjectFolder* folder = 0;
	CProjectFolder& topFolder = SetData();
	if (!folderName.empty()) {
		folder = topFolder.FindChildFolderByTitle(folderName);
		if (folder == 0)  {
			CRef<CProjectFolder> new_folder(new CProjectFolder());
			CFolderInfo& info = new_folder->SetInfo();
			info.SetTitle(folderName);
			info.SetComment(folderName);
			info.SetCreate_date().SetToTime(CTime(CTime::eCurrent));
			topFolder.AddChildFolder(*new_folder);
			folder = new_folder;
		}
	} else {
		folder = &topFolder;
	}

	for (size_t i = 0; i < items.size(); ++i) 
        AddItem(*items[i], *folder);

	CUniqueLabelGenerator loaderNames;
	if (GetProject().IsSetDataLoaders()) {
		ITERATE (IGBProject::TDataLoaders, iter, GetProject().GetDataLoaders()) {
			const string& s = (*iter)->GetLabel();
			loaderNames.AddExistingLabel(s);
		}
	}

    ITERATE (TLoaders, it, loaders) {
		TLoaderRef loader = *it;

		const string& loader_type = loader->GetLoader_type();

		IDataLoaderProvider* data_source = x_GetDataSourceByType(loader_type);
		if (data_source != 0) {
			string loader_name = data_source->GetLoaderName(loader->GetData());

			bool loaded = false;
    
			NON_CONST_ITERATE (IGBProject::TDataLoaders, it2, GetDataLoaders()) {
				CLoaderDescriptor& curr_loader = **it2;
				if (curr_loader.GetLoader_type() != loader_type) continue;

				string curr_name = data_source->GetLoaderName(curr_loader.GetData());
				if (curr_name == loader_name) {
					loaded = true;
					break;
				}
			}
			if (loaded)
				continue;
		}

		string label = loader->GetLabel();
		string uniqueLabel = loaderNames.MakeUniqueLabel(label);
		if (label != uniqueLabel)
			loader->SetLabel(uniqueLabel);
		loaderNames.AddExistingLabel(uniqueLabel);

		AddDataLoader(*loader);
		AttachDataLoader(*loader);
    }

    vector<IProjectItemExtension*> clients;
    GetExtensionAsInterface(EXT_POINT__PROJECT_ITEM_EXTENSION, clients);
    if (!clients.empty()) {
        IServiceLocator* srvLocator = m_Service->GetServiceLocator();
	    for (size_t i = 0; i < items.size(); ++i)
            s_CallExtensionProjectItemAttached(*this, clients, *items[i], srvLocator);
    }

    SetDirty (true);
    ProjectStateChanged();
    x_FireProjectChanged(CProjectViewEvent::eData);
}

bool CGBDocument::RemoveDataLoader(CLoaderDescriptor& loader)
{
    if (!loader.GetEnabled() || DetachDataLoader(loader)) {
        if (m_Project->IsSetDataLoaders()) {
            TDataLoaders& loaders = m_Project->SetDataLoaders();
            NON_CONST_ITERATE (IGBProject::TDataLoaders, iter, loaders) {
                if ((*iter)->GetLabel() == loader.GetLabel()) {
                    loaders.erase(iter);
                    break;
                }
            }
        }
        LOG_POST(Info << "Data loader: " << loader.GetLabel() << " is removed!");
        return true;
    }
    return false;
}


IDataLoaderProvider* CGBDocument::x_GetDataSourceByType(const string& type)
{
    IServiceLocator* srv_locator = m_Service->GetServiceLocator();
    CIRef<CUIDataSourceService> srv =
        srv_locator->GetServiceByType<CUIDataSourceService>();
    CUIDataSourceService::TUIDataSourceVec data_sources;
    srv->GetDataSources(data_sources);
    
    // find the matched one
    IDataLoaderProvider* data_source = NULL;
    for (size_t i = 0;  i < data_sources.size();  ++i) {
        CIRef<IUIDataSource> ds = data_sources[i];
        IExtension* ext = dynamic_cast<IExtension*>(&ds->GetType());
        if (ext  &&  ext->GetExtensionIdentifier() == type) {
            data_source = dynamic_cast<IDataLoaderProvider*>(&*ds);
            break;
        }
    }

    return data_source;
}


bool CGBDocument::AttachDataLoader(CLoaderDescriptor& loader)
{
    _ASSERT(IsLoaded());

    IDataLoaderProvider* data_source = x_GetDataSourceByType(loader.GetLoader_type());
    if (data_source == 0)
        return false;

    string loader_name;
    try {
        loader_name = data_source->AddDataLoader(loader.SetData());

    } NCBI_CATCH("Data loader error: " + loader.GetLabel());

    if (loader_name.empty()) {
        loader.SetEnabled(false);
        return false;
    }

    if (loader.SetData().HasField(kLoaderName)) {
        loader.SetData().SetField(kLoaderName).SetString(loader_name);
    } else {
        loader.SetData().AddField(kLoaderName, loader_name);
     }

    CScope::TPriority priority = CScope::kPriority_Default;
    if (loader.IsSetPriority()) {
        priority = loader.GetPriority();
    }

    CScope* scope = GetScope();
    _ASSERT(scope);
    scope->ResetHistory();
    scope->AddDataLoader(loader_name, priority);

    loader.SetEnabled(true);

    return true;
}


bool CGBDocument::DetachDataLoader(CLoaderDescriptor& loader)
{
    _ASSERT(IsLoaded());

    string loader_name;
    CConstRef<CUser_field> loader_name_field = loader.GetData().GetFieldRef(kLoaderName);
    if (loader_name_field) 
        loader_name = loader_name_field->GetData().GetStr();
    if (loader_name.empty()) {
        IDataLoaderProvider* data_source = x_GetDataSourceByType(loader.GetLoader_type());
        if (data_source == 0)
            return false;
        loader_name = data_source->GetLoaderName(loader.GetData());
    }
    if (loader_name.empty())
        return false;

    CScope* scope = GetScope();
    _ASSERT(scope);

    bool result = false;
    try {
        scope->ResetHistory();
        scope->RemoveDataLoader(loader_name);
        loader.SetEnabled(false);
        result = true;
    } NCBI_CATCH("Failed to detach data loader \"" + loader.GetLabel() + "\" from scope.");
    return result;
}

void CGBDocument::AttachData()
{
    x_AttachProjectItems();
    x_AttachDataLoaders();

    ProjectStateChanged();
}


void CGBDocument::x_AttachDataLoaders()
{
    _ASSERT(IsLoaded());

    TDataLoaders& loaders = m_Project->SetDataLoaders();
    NON_CONST_ITERATE (IGBProject::TDataLoaders, iter, loaders)
        AttachDataLoader(**iter);
}

void CGBDocument::x_DetachDataLoaders()
{
    _ASSERT(IsLoaded());

    TDataLoaders& loaders = m_Project->SetDataLoaders();
    NON_CONST_ITERATE (IGBProject::TDataLoaders, iter, loaders)
        DetachDataLoader(**iter);
}

void CGBDocument::x_AttachProjectItems()
{
    _ASSERT(IsLoaded());

    CScope* scope = GetScope();
    _ASSERT(scope);

    vector<IProjectItemExtension*> clients;
    GetExtensionAsInterface(EXT_POINT__PROJECT_ITEM_EXTENSION, clients);
    IServiceLocator* srvLocator = m_Service->GetServiceLocator();

    for (CTypeIterator<CProjectItem> it(SetData()); it; ++it) {
        if (!it->IsEnabled())
            continue;
        s_AttachProjectItem(*it, *scope);
        if (!clients.empty()) {
            s_CallExtensionProjectItemAttached(*this, clients, *it, srvLocator);
        }
    }
}

typedef vector<pair<CProjectItem*, CSeq_id_Handle> > TGiProjectItems;

static void s_ReplaceGIs(TGiProjectItems& gis, CScope& scope)
{
    CScope::TSeq_id_Handles input, results;
    ITERATE(TGiProjectItems, it, gis)
        input.push_back(it->second);

    scope.GetAccVers(&results, input);

    for (size_t i = 0; i < results.size(); ++i) {
        CSeq_id_Handle& h = results[i];
        if (h) {
            CConstRef<CSeq_id> seqId = h.GetSeqIdOrNull();
            if (seqId)
                gis[i].first->SetItem().SetId().Assign(*seqId);
        }
    }
}

void CGBDocument::ReplaceGiSeqIds(ICanceled* canceled)
{
    CScope* scope = GetScope();
    _ASSERT(scope);

    TGiProjectItems gis;

    for (CTypeIterator<CProjectItem> it(SetData()); it; ++it) {
        if (!it->CanGetItem() || !it->GetItem().IsId()) continue;
        const CSeq_id& seqId = it->GetItem().GetId();
        if (!seqId.IsGi()) continue;
        gis.push_back(make_pair(&*it, CSeq_id_Handle::GetHandle(seqId)));

        if (gis.size() >= 2000) {
            if (canceled && canceled->IsCanceled())
                return;
            s_ReplaceGIs(gis, *scope);
            gis.clear();
        }
    }

    if (gis.size() > 0) {
        if (canceled && canceled->IsCanceled())
            return;
        s_ReplaceGIs(gis, *scope);
    }
}

void CGBDocument::x_DetachProjectItems()
{
    _ASSERT(IsLoaded());

    CScope* scope = GetScope();
    _ASSERT(scope);

    for (CTypeIterator<CProjectItem> it(SetData());  it;  ++it) {
        s_DetachProjectItem(*it, *scope);
    }
}

void CGBDocument::x_UnloadData()
{
    _ASSERT(IsLoaded());
    x_DetachDataLoaders();
    x_DetachProjectItems();
    Clear();
}

void CGBDocument::UnloadProject(bool reset_hist_async)
{
    if (IsLoading()) {
        CancelLoading();
        return;
    }

    if (!IsLoaded())
        return;

    wxBusyCursor wait;

    vector<IProjectItemExtension*> clients;
    GetExtensionAsInterface(EXT_POINT__PROJECT_ITEM_EXTENSION, clients);
    IServiceLocator* srvLocator = m_Service->GetServiceLocator();

    for (CTypeIterator<CProjectItem> it(SetData());  it;  ++it) {
        if (!clients.empty()) {
            s_CallExtensionProjectItemDetached(*this, clients, *it, srvLocator);
        }
    }

    x_RemoveAllViews(reset_hist_async);
    x_UnloadData();

    x_FireProjectChanged(CProjectViewEvent::eUnloadProject);

    ProjectStateChanged();
}

void CGBDocument::x_RemoveAllViews(bool reset_hist_async)
{
    if (m_Views.empty())
        return;

    IServiceLocator* srvLocator = m_Service->GetServiceLocator();
    IViewManagerService* view_srv = srvLocator->GetServiceByType<IViewManagerService>();

    TViews views = m_Views;

    for (size_t i = 0; i < views.size(); i++) {
        IProjectView& view = *views[i];
        view.SetAsyncDestroy(reset_hist_async);
        view.DestroyView();
        view_srv->RemoveFromWorkbench(view);
    }
}

wxString CGBDocument::GetWorkDir() const
{
    for (CTypeConstIterator<CProjectItem> it(GetData()); it; ++it) {
        string smartFile;
        CProjectItemExtra::GetStr(*it, "SmartFile", smartFile);
        if (!smartFile.empty()) {
            return wxFileName(wxString::FromUTF8(smartFile.c_str())).GetPath();
        }
    }
    return wxEmptyString;
}

END_NCBI_SCOPE
