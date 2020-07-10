#ifndef GUI_CORE___SELECT_PROJECT_OPTIONS__HPP
#define GUI_CORE___SELECT_PROJECT_OPTIONS__HPP

/*  $Id: select_project_options.hpp 33751 2015-09-14 15:47:01Z katargir $
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
 * Authors: Andrey Yazhuk, Liangshou Wu
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <objects/gbproj/LoaderDescriptor.hpp>
#include <objects/gbproj/ProjectItem.hpp>

BEGIN_NCBI_SCOPE

class CProjectService;

///////////////////////////////////////////////////////////////////////////////
/// CProjectSelectOptions - describes how new Project Items shall be added
/// to a workspace. If folder name is not empty a new Project Folder will be
/// created for the items.
class NCBI_GUICORE_EXPORT  CSelectProjectOptions
{
public:
    enum EAction {
        eInvalidAction = -1,
        eDecideLater,
        eCreateNewProject,
        eCreateSeparateProjects,
        eAddToExistingProject
    };
    typedef int TProjectId;

public:
    CSelectProjectOptions();

    void Set_DecideLater();
    void Set_CreateNewProject( const string& folder = kEmptyStr );
    void Set_CreateSeparateProjects();
    void Set_AddToExistingProject(
        TProjectId& project_id, const string& folder = kEmptyStr
    );

    void SetNewProjectName(const string& newProjectName) { m_NewProjectName = newProjectName; }

    EAction GetAction();
    TProjectId GetTargetProjectId();
    string GetFolderName();

    typedef CRef<objects::CProjectItem>        TItemRef;
    typedef vector<TItemRef> TItems;

    typedef CRef<objects::CLoaderDescriptor>   TLoaderRef;
    typedef vector<TLoaderRef> TLoaders;

    typedef map<TLoaderRef, TItems> TData;

    bool AddItemsToWorkspace(CProjectService* service, const TData& data);

protected:
    bool x_ShowSelectProjectDialog(CProjectService* service, TItems& items);

    void x_AddToExistingProject(CProjectService* service, TItems& items, const TLoaders& loaders);
    void x_CreateOneProject(CProjectService* service, TItems& items, const TLoaders& loaders);
    void x_CreateSeparateProjects(CProjectService* service, const TData& data);

    EAction m_Action;
    TProjectId  m_TargetProjectId;
    string  m_FolderName;
    string  m_NewProjectName;
};

END_NCBI_SCOPE

#endif // GUI_CORE___SELECT_PROJECT_OPTIONS__HPP
