#ifndef GUI_CORE___PROJECT_SERVICE__HPP
#define GUI_CORE___PROJECT_SERVICE__HPP

/*  $Id: project_service.hpp 39528 2017-10-05 15:27:37Z katargir $
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

#include <corelib/ncbimtx.hpp>
#include <corelib/ncbistl.hpp>

#include <gui/core/project_service_types.hpp>
#include <gui/core/project_view.hpp>

#include <gui/framework/service.hpp>

#include <gui/objutils/objects.hpp>
#include <gui/objutils/reg_settings.hpp>

#include <gui/utils/mru_list.hpp>

#include <gui/objects/GBProjectHandle.hpp>
#include <gui/objects/GBWorkspace.hpp>

#include <gui/widgets/wx/gui_widget.hpp>

#include <set>

#include <wx/string.h>

BEGIN_NCBI_SCOPE

class IViewManagerService;
class CGBDocument;
class CProjectTreePanel;

BEGIN_SCOPE(objects)
    class CGBWorkspace;
    class CWorkspaceFolder;
    class CUser_object;
END_SCOPE(objects)

////////////////////////////////////////////////////////////////////////////////
/// CProjectService - a service providing API for operations with Workspaces and
/// Projects.
/// CProjectService
///
/// CProjectService shall not have any dependencies on GUI.

class NCBI_GUICORE_EXPORT CProjectService :
    public CObjectEx,
    public IService,
    public IServiceLocatorConsumer,
    public IRegSettings
{
    friend class CWorkspaceAutoSaver;
public:
    typedef CTimeMRUList<wxString>    TMRUPathList;

public:
    CProjectService();
    virtual ~CProjectService();

    /// @name IService interface implementation
    /// @{
    virtual void    InitService();
    virtual void    ShutDownService();
    /// @}

    /// @name IServiceLocatorConsumer interface implementation
    /// @{
    virtual void    SetServiceLocator(IServiceLocator* locator);
    /// @}

    IServiceLocator* GetServiceLocator() { return m_ServiceLocator; }

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    SaveSettings() const;
    virtual void    LoadSettings();
    /// @}

    /// @name Workspace manipulators
    /// @{
    bool    HasWorkspace();
    void    CreateNewWorkspace();
    /// Loads a workspace from a file and optionally set as current
    bool    LoadWorkspace(const wxString& filename);
    /// release the current workspace and all its contents TODO - review
    void    ResetWorkspace();
    /// Save a workspace.  This will (potentially) ask the user for a
    /// filename to which to save the workspace
    bool    SaveWorkspace();
    /// @}

    /// get MRU Projects and Workspaces
    const TMRUPathList& GetProjectWorkspaceMRUList() const;
    void  AddToProjectWorkspaceMRUList(const wxString& path);
    /// @}

    /// @name Project View Manipulators
    /// @{
    /// initialized view, connects it to a projects and adds to View Manager Service
    CIRef<IProjectView> AddProjectView(const string& view_name, SConstScopedObject& object, const objects::CUser_object* params, bool bFloat = false);
    CIRef<IProjectView> AddProjectView(const string& view_name, TConstScopedObjects& objects, const objects::CUser_object* params, bool bFloat = false);

    IProjectView* FindView(const CObject& mainObject, const string& viewType);
    void          FindViews(vector<CIRef<IProjectView> >& projectViews, const CObject& mainObject);
    void          FindViews(vector<CIRef<IProjectView> >& projectViews);

    CIRef<IProjectView> ShowView(const string& viewName, FWindowFactory widgetFactory, TConstScopedObjects& objects, bool bFloat);

    void          ActivateProjectView(IProjectView* projectView);

    /// removes the view from View manager Service and disconnects it from the project
    void    RemoveProjectView(IProjectView& view);
    void    RemoveAllProjectViews();

    /// called by CDocument when IProjectView is attached/detached to it
    void    OnViewAttached(IProjectView* view);

    void    GetObjProjects(TConstScopedObjects& objects, vector<int>& ids);

    CRef<objects::CGBWorkspace> GetGBWorkspace();

    void    AddProject(CGBDocument& doc);
    void    RemoveProject(CGBDocument& doc);

    CProjectTreePanel* GetProjectTreePanel();

    const objects::CProjectItem* GetProjectItem(const CObject& object, objects::CScope& scope);

protected:
    CRef<objects::CGBWorkspace> x_LoadWorkspace(const wxString& filename);
    void x_CreateDocuments(objects::CWorkspaceFolder& folder);

    void x_RemoveView(IViewManagerService& view_srv, IProjectView& view, bool reset_hist_async = true);

    void x_ReloadProjectTreeView();

    void x_UpdateWorkspaceLabel();

private:
    /// all constructing / copying of project managers is forbidden
    CProjectService(const CProjectService&);
    CProjectService& operator= (const CProjectService&);

protected:
    typedef set<int>    TWorkspaceIds;
    typedef set<int>    TProjectIds;

protected:
    /// path in the Registry to our settings
    string  m_RegPath;

    IServiceLocator* m_ServiceLocator;

    /// the only Workspace
    CRef<objects::CGBWorkspace> m_Workspace;

    string m_Filename;  /// filename for the workspace

    TMRUPathList    m_ProjectWorkspaceMRUList;
};


END_NCBI_SCOPE

#endif  /// GUI_CORE___PROJECT_SERVICE__HPP
