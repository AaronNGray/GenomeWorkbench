/*  $Id: table_data_suc_job.cpp 32231 2015-01-22 15:57:41Z katargir $
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
 * Authors: Colleen Bollin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/edit/table_data_suc_job.hpp>
#include <gui/widgets/edit/suc_data_generator.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CTableDataSUCJob::CTableDataSUCJob(const TConstScopedObjects& objects)
	: m_InputObjects(objects)
{
}

CConstIRef<IAppJobProgress> CTableDataSUCJob::GetProgress()
{
    return CConstIRef<IAppJobProgress>();
}

CRef<CObject> CTableDataSUCJob::GetResult()
{
    return m_Result;
}


CConstIRef<IAppJobError> CTableDataSUCJob::GetError()
{
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}


string CTableDataSUCJob::GetDescr() const
{
    return "SUC job";
}


IAppJob::EJobState CTableDataSUCJob::Run()
{
    string err_msg, logMsg = GetDescr() + " - exception in Run() ";

    try {
        CLockerGuard guard = x_GetGuard();
        if (IsCanceled()) return eCanceled;
        CRef<CSUCResults> results = CSUCDataGenerator::GetSUCResults(m_InputObjects);
        m_Result.Reset(results.GetPointerOrNull());

    }
    catch (CException& e) {
        err_msg = e.GetMsg();
        LOG_POST(Error << logMsg << GetDescr() << ". " << err_msg);
        //e.ReportAll();
    } catch (std::exception& e) {
        err_msg = GetDescr() + ". " + e.what();
        LOG_POST(Error << logMsg << GetDescr() << ". " << err_msg);
    }

    if (IsCanceled())
        return eCanceled;

    if (!err_msg.empty()) {
        string s = err_msg;
        m_Error.Reset(new CAppJobError(s));
        return eFailed;
    }

    return eCompleted;
}

END_NCBI_SCOPE

