#ifndef GUI_GBENCH_NEW___NAMED_PIPE_PROCESS_TASK__HPP
#define GUI_GBENCH_NEW___NAMED_PIPE_PROCESS_TASK__HPP

/*  $Id: open_files_task.hpp 33333 2015-07-08 17:13:23Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */


#include <corelib/ncbiobj.hpp>

#include <gui/framework/app_task_impl.hpp>

BEGIN_NCBI_SCOPE

class IWorkbench;

///////////////////////////////////////////////////////////////////////////////
/// COpenFilesTask
class COpenFilesTask : public CAppTask
{
public:
    COpenFilesTask(IWorkbench* workbench, const vector<string>& fileNames, bool bringToFront);

protected:
    virtual ETaskState    x_Run();

protected:
    IWorkbench* m_Workbench;
    vector<string> m_FileNames;
    bool m_BringToFront;
};

END_NCBI_SCOPE;

#endif  // GUI_GBENCH_NEW___NAMED_PIPE_PROCESS_TASK__HPP
