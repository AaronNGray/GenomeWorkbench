#ifndef GUI_CORE_DEMO___OBJECT_LOADING_TASK__HPP
#define GUI_CORE_DEMO___OBJECT_LOADING_TASK__HPP

/*  $Id: object_loading_task.hpp 38429 2017-05-09 14:50:25Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>

#include <gui/core/select_project_options.hpp>
#include <gui/framework/app_job_task.hpp>
#include <gui/framework/service.hpp>

#include <gui/utils/object_loader.hpp>


BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
class CLoaderDescriptor;
END_SCOPE(objects)

class NCBI_GUICORE_EXPORT  CObjectLoadingTask : public CAppJobTask
{
public:
    CObjectLoadingTask(CProjectService* service, IObjectLoader& object_loader, CSelectProjectOptions& options);

    /// @name IAppTask interface implementation
    /// @{
    virtual ETaskState Run();
    /// @}

    void AddProjectItemExtra(const string& tag, const string& value);

    static void SetSaveFilePath(bool save) { m_SaveFilePath = save; }

    static bool AddObjects(
        IServiceLocator* serviceLocator,
        IObjectLoader::TObjects& objects,
        objects::CLoaderDescriptor* loader,
        CSelectProjectOptions&  options,
        const map<string, string>& projectItemExtra = map<string, string>());

private:
    CIRef<CProjectService> m_Service;
    CIRef<IObjectLoader>   m_ObjectLoader;
    CSelectProjectOptions  m_Options;

    map<string, string> m_ProjectItemExtra;

    static bool m_SaveFilePath;
};

END_NCBI_SCOPE


#endif // GUI_CORE_DEMO___OBJECT_LOADING_TASK__HPP
