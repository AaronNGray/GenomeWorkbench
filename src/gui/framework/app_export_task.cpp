/*  $Id: app_export_task.cpp 39359 2017-09-14 16:27:31Z katargir $
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
 *
 */

#include <ncbi_pch.hpp>

#include <gui/framework/app_export_task.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CAppExportTask

CAppExportTask::CAppExportTask(IAppJob& job, const wxString fileName)
: CAppJobTask(job, true), m_FileName(fileName)
{
}

void CAppExportTask::x_Finish(IAppJob::EJobState state)
{
    CAppJobTask::x_Finish(state);

    if (IAppJob::eCompleted != state)
        return;

    if (CGuiRegistry::GetInstance().GetBool("GBENCH.Application.ExportOpenFolder", true))
        OpenFileBrowser(m_FileName);
}


END_NCBI_SCOPE
