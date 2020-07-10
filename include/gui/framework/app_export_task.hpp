#ifndef GUI_FRAMEWORK___APP_EXPORT_TASK__HPP
#define GUI_FRAMEWORK___APP_EXPORT_TASK__HPP

/*  $Id: app_export_task.hpp 39343 2017-09-13 19:24:30Z katargir $
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
 *      CAppJobTask makes it possible to execute App Jobs as Tasks in Task manager.
 */

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <wx/string.h>

#include <gui/framework/app_job_task.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIFRAMEWORK_EXPORT CAppExportTask : public CAppJobTask
{
public:
    CAppExportTask(IAppJob& job, const wxString fileName);

protected:
    virtual void x_Finish(IAppJob::EJobState state);

private:
    wxString m_FileName;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___APP_EXPORT_TASK__HPP
