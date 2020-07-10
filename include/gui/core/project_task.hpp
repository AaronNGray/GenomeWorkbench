#ifndef GUI_CORE___PROJECT_TASK__HPP
#define GUI_CORE___PROJECT_TASK__HPP

/*  $Id: project_task.hpp 39528 2017-10-05 15:27:37Z katargir $
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

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/framework/app_task_impl.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/string.h>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects);
    class CGBWorkspace;
END_SCOPE(objects);


class IServiceLocator;
class CProjectService;

///////////////////////////////////////////////////////////////////////////////
/// CProjectTask -  a task that supports typical application operations
/// applicable to Workspaces and Projects.
/// The task relies on Project Service functions as low-level API.

class  NCBI_GUICORE_EXPORT  CProjectTask :
    public CAppTask
{
public:
    typedef vector<int>  TProjectIdVector;

public:
    CProjectTask(IServiceLocator* srv_locator, vector<wxString>& filenames);
    virtual ~CProjectTask();

    static void UnLoadProjects(IServiceLocator* serviceLocator, const TProjectIdVector& project_ids);
    static void LoadProjects(IServiceLocator* serviceLocator, const TProjectIdVector& project_ids);
    static bool RemoveProjects(IServiceLocator* serviceLocator, const TProjectIdVector& project_ids, bool confirm = true);
    static bool RemoveAllProjects(IServiceLocator* serviceLocator);
    static void Save(IServiceLocator* serviceLocator, const TProjectIdVector& project_ids, bool save_as);

protected:
    virtual ETaskState    x_Run(); /// overriding CAppTask::x_Run()

    void    x_OpenProjects();
    
    static  bool    x_DoSaveUserSelected(CProjectService* service,
                                         objects::CGBWorkspace& ws,
                                         const string& title,
                                         const string& description,
                                         bool save_as,
                                         bool no_btn,
                                         const vector<int>& project_ids,
                                         const vector<int>& selected);
    /// @}

protected:
    IServiceLocator*  m_SrvLocator;
    vector<wxString>  m_ProjectFilenames;
};


END_NCBI_SCOPE


#endif  // GUI_CORE___PROJECT_TASK__HPP

