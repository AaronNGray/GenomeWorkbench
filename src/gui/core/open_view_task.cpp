/*  $Id: open_view_task.cpp 31192 2014-09-10 18:54:28Z katargir $
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
 * Authors:  Vyacheslav Chetvernin
 *
 * File Description:
 */

#include <ncbi_pch.hpp>

#include <gui/core/open_view_task.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/project_view.hpp>
#include <gui/framework/window_manager_service.hpp>

BEGIN_NCBI_SCOPE

COpenViewTask::COpenViewTask(
        IServiceLocator* srvLocator,
        const string& view_name,
        SConstScopedObject& object,
        const objects::CUser_object* params,
        bool floatView)
:   CAppTask("Open "+view_name, false)
, m_SrvLocator(srvLocator)
, m_ViewName(view_name)
, m_Params(params)
, m_FloatView(floatView)
{
    m_Objects.push_back(object);
}

COpenViewTask::COpenViewTask(
        IServiceLocator* srvLocator,
        const string& view_name,
        TConstScopedObjects& objects,
        const objects::CUser_object* params,
        bool floatView)
:   CAppTask("Open "+view_name, false)
, m_SrvLocator(srvLocator)
, m_ViewName(view_name)
, m_Objects(objects)
, m_Params(params)
, m_FloatView(floatView)
{
}

COpenViewTask::ETaskState COpenViewTask::x_Run()
{
    CProjectService* prj_srv = m_SrvLocator->GetServiceByType<CProjectService>();
    CIRef<IProjectView> view = prj_srv->AddProjectView(m_ViewName, m_Objects, m_Params, m_FloatView);

    if (view) {
        OnViewCreate(*view);
    }

    return eCompleted;
}

END_NCBI_SCOPE
