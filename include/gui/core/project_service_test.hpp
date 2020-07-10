#ifndef GUI_CORE___PROJECT_SERVICE_TEST__HPP
#define GUI_CORE___PROJECT_SERVICE_TEST__HPP

/*  $Id: project_service_test.hpp 30165 2014-04-11 17:25:10Z katargir $
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
 *  Concurrency test for Project Service.
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/framework/app_task_impl.hpp>
#include <gui/framework/app_job_task.hpp>

#include <gui/utils/app_job_impl.hpp>


BEGIN_NCBI_SCOPE

class IServiceLocator;
class CProjectService;

///////////////////////////////////////////////////////////////////////////////
/// CProjectServiceTestTask - concurrency test for Project Service.
class  NCBI_GUICORE_EXPORT  CProjectServiceTestTask :
    public CAppTask
{
public:
    CProjectServiceTestTask(IServiceLocator* srv_locator);
    virtual ~CProjectServiceTestTask();

protected:
    virtual ETaskState    x_Run(); /// overriding CAppTask::x_Run()

protected:
    IServiceLocator*  m_SrvLocator;
    CProjectService*  m_Service;
};


///////////////////////////////////////////////////////////////////////////////
/// CProjectServiceTestJob
class NCBI_GUICORE_EXPORT  CProjectServiceTestJob : public CAppJob
{
public:
    /// Job types
    enum EType  {
        eCreateItems,
        eModifyItems,
        eDeleteItems,
        ePrintItems
    };

    CProjectServiceTestJob(CProjectService* service, EType type, int sleep_ms);

    /// @name IAppJob implementation
    /// @{
    virtual EJobState   Run();
    /// @}

protected:
    void    x_CreateItems();
    void    x_ModifyItems();
    void    x_DeleteItems();
    void    x_PrintItems();

    string  x_GetJobTypeString();

protected:
    CProjectService* m_ProjectService;
    EType   m_Type;
    int     m_SleepMs;
};


END_NCBI_SCOPE


#endif  // GUI_CORE___PROJECT_SERVICE_TEST__HPP

