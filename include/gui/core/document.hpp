#ifndef GUI_CORE___DOCUMENT__HPP
#define GUI_CORE___DOCUMENT__HPP

/* $Id: document.hpp 41444 2018-07-30 19:37:01Z katargir $
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

#include <gui/core/project_view.hpp>

#include <gui/objects/GBProjectHandle.hpp>
#include <objects/gbproj/LoaderDescriptor.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/core/undo_manager.hpp>
#include <gui/utils/app_job.hpp>
#include <gui/utils/job_adapter.hpp>
#include <gui/utils/view_event.hpp>

#include <gui/widgets/wx/async_call.hpp>

#include <wx/string.h>

BEGIN_NCBI_SCOPE

class CProjectService;
class IDataLoaderProvider;
class CProjectViewBase;

BEGIN_SCOPE(objects)
    class CGBProject;
    class CGBProject_ver2;
    class CProjectItem;
END_SCOPE(objects)

///////////////////////////////////////////////////////////////////////////////
/// CProjectViewEvent
class CProjectViewEvent : public CViewEvent
{
public:
    enum EEventSubtype {
        eNone = 0,
        eProperties,
        eData,
        eBothPropsData,
         // Before we change data we syncronously Send
         // eDataChanging event, so view won't use data
         // Then we Post eData event
         // Used in the Undo manager
        eDataChanging,
        eUnloadProject
    };

public:
    CProjectViewEvent( 
        objects::CGBProjectHandle::TId prj_id, EEventSubtype subtype = eNone 
    ) 
        : CViewEvent( eProjectChanged )
        , m_Subtype( subtype )
        , m_View()  
        , m_ProjectId( prj_id )
    {}

    CProjectViewEvent( 
        IProjectView& view, CViewEvent::EEventType type, EEventSubtype subtype = eNone
    )
        :  CViewEvent( type )
        , m_Subtype( subtype )
        , m_View( &view )  
        , m_ProjectId( objects::CGBProjectHandle::sm_NullId )
    {}

    EEventSubtype GetSubtype() const { return m_Subtype; }
    objects::CGBProjectHandle::TId GetProjectId() { return m_ProjectId; }
    CIRef<IProjectView> GetProjectView() { return m_View; }


protected:
    EEventSubtype m_Subtype;
    CIRef<IProjectView> m_View;
    objects::CGBProjectHandle::TId m_ProjectId;
};

///////////////////////////////////////////////////////////////////////////////
/// CGBDocument
class NCBI_GUICORE_EXPORT CGBDocument 
    : public objects::CGBProjectHandle
    , public CEventHandler
    , public CJobAdapter::IJobCallback
{
    friend class CProjectService;
    friend class CUndoManager;
    friend class CProjectLoadingJob;
    friend class CDocLoadingJob;
    friend class CProjectLoadingTask;
    friend class CSmartProjectTask;
    friend class CDataLoadingAppJob;
    friend class CProjectTreePanel;
    friend class CProjectViewBase;
    friend class CSelectionService;
    friend class CSelectProjectOptions;
    friend class CWorkspaceAutoSaver;
    friend class CAddItemsData;

    typedef objects::CGBProjectHandle Tparent;

public:
    typedef vector< CIRef<IProjectView> > TViews;

    typedef CProjectViewEvent TEvent;

    /// Retrieve a label for this view.  The label has several different
    enum ELabelType {
        eType,              //< usually a description of the class
        eContent,           //< The content (what the view is looking at)
        eTypeAndContent,
        eId,                //< an identifier for the view, usually given as a base-26 letter
        eProject,
        eDefault = eTypeAndContent
    };

public:
    CGBDocument( CProjectService* srv );
    CGBDocument( CProjectService* srv, TId id, objects::CGBProject_ver2& proj_impl );
    ~CGBDocument();

    wxString GetFileName() const;
    void SetFileName (const wxString& filename);

    virtual void CreateProjectScope();


    string GetDefaultAssembly() const;

    CUndoManager& GetUndoManager() { return *m_UndoManager; }

    // Load document on worker thread
    bool    IsLoading() const;
    void    CancelLoading();
    void    ResetLoading();

    bool    RemoveProjectItem(objects::CProjectItem* item);
    bool    RemoveProjectItems(const vector<objects::CProjectItem*>& items);

    void    AttachProjectItem(objects::CProjectItem* item);
    void    DetachProjectItem(objects::CProjectItem* item);

    void    AttachProjectItems(const vector<objects::CProjectItem*>& items);
    void    DetachProjectItems(const vector<objects::CProjectItem*>& items);

    void    Save(const wxString& abs_path);

    void    UnloadProject(bool reset_hist_async = true);

    virtual const TViews& GetViews(void) const;

    bool RemoveDataLoader(objects::CLoaderDescriptor& loader);
    bool AttachDataLoader(objects::CLoaderDescriptor& loader);
    bool DetachDataLoader(objects::CLoaderDescriptor& loader);

    void ProjectItemsChanged();
    void ProjectStateChanged();

    void ViewLabelChanged(IProjectView& view);

    /// @name CJobAdapter::IJobCallback interface implementation
    /// @{
    virtual void OnJobResult(CObject* result, CJobAdapter& adapter);
    virtual void OnJobFailed(const string&, CJobAdapter& adapter);
    /// @}

    wxString GetWorkDir() const;

protected:
    void    x_AttachView(IProjectView* view);
    void    x_DetachView(IProjectView* view);
    TViews& x_GetViews(void);
    void    x_RemoveAllViews(bool reset_hist_async);
    bool    x_DetachProjectItems(const vector<objects::CProjectItem*>& items);

    virtual void x_FireProjectChanged( TEvent::EEventSubtype subtype );
    virtual void x_FireViewEvent( IProjectView& view, CViewEvent::EEventType type );
    // virtual void x_PostViewEvent(IProjectView& view, CViewEvent::EEventType type);

    // generates index unique for this type of view ans assigns it to a view
    void         x_AssignViewIndex(IProjectView* view);

    void x_SaveFile(const wxString& fname, ESerialDataFormat fmt, bool keep_backups);
    bool LoadFile(const wxString& fname, ICanceled* canceledCallback);

    IAppJob* CreateLoadJob(const wxString& fileName);

    typedef CRef<objects::CLoaderDescriptor>   TLoaderRef;
    typedef vector<TLoaderRef> TLoaders;

    void AddItems(const string& folderName,
		          vector<CRef<objects::CProjectItem> >& items,
                  const TLoaders& loaders);

    void ReplaceGiSeqIds(ICanceled* canceled);

    void x_UnloadData();

    void AttachData();

private:
    void x_AttachDataLoaders();
    void x_AttachProjectItems();
    void x_ProjectViewsChanged();
    void x_ProjectUpdateLabel();

    void x_AddItems(const string& folderName,
        vector<CRef<objects::CProjectItem> >& items,
        const TLoaders& loaders);

    void x_DetachDataLoaders();

    void x_DetachProjectItems();

    void x_CloseProjectItemViews(objects::CProjectItem& item);

    // Prohibit copy constructor and assignment operator
    CGBDocument(const CGBDocument& value);
    CGBDocument& operator=(const CGBDocument& value);

	static size_t x_GetUndoSize();

    IDataLoaderProvider* x_GetDataSourceByType(const string& type);

private:
    /// Views attached to this project
    TViews      m_Views;

    /// pointer to the service that manages the document
    CProjectService*    m_Service;

    /// map of views to indices for views
    typedef map<IProjectView*, int> TViewToIndex;
    TViewToIndex    m_ViewToIndex;

    /// undo manager, project data centric
    CRef<CUndoManager> m_UndoManager;
    CIRef<IAppJob> m_LoadJob;

    void x_InitializeAssembly() const;
    void x_ClearAssembly() const;

    mutable string m_AssemblyAccession;
    mutable bool m_AssemblyInitialized;

    list<CRef<CJobAdapter> > m_JobAdapters;
};

END_NCBI_SCOPE

#endif // GUI_CORE___DOCUMENT__HPP
