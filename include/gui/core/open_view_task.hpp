#ifndef GUI_CORE___OPEN_VIEW_TASK__HPP
#define GUI_CORE___OPEN_VIEW_TASK__HPP

/*  $Id: open_view_task.hpp 30763 2014-07-22 16:00:04Z katargir $
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
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>
#include <gui/objutils/objects.hpp>
#include <gui/framework/app_task_impl.hpp>
#include <objects/general/User_object.hpp>

BEGIN_NCBI_SCOPE

class IServiceLocator;
class IProjectView;

class NCBI_GUICORE_EXPORT COpenViewTask : public CAppTask
{
public:
    COpenViewTask(
        IServiceLocator* srvLocator,
        const string& view_name,
        SConstScopedObject& object,
        const objects::CUser_object* params = 0,
        bool floatView = false);

    COpenViewTask(
        IServiceLocator* srvLocator,
        const string& view_name,
        TConstScopedObjects& objects,
        const objects::CUser_object* params = 0,
        bool floatView = false);

protected:
    virtual void OnViewCreate(IProjectView& /*view*/) {}

    IServiceLocator* m_SrvLocator;
    string m_ViewName;
    TConstScopedObjects m_Objects;
    CConstRef<objects::CUser_object> m_Params;
    bool m_FloatView;

    virtual ETaskState x_Run();
};

END_NCBI_SCOPE

#endif  // GUI_CORE___OPEN_VIEW_TASK__HPP
