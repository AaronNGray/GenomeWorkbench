/*  $Id: app_job_impl.cpp 34478 2016-01-13 16:19:10Z katargir $
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
 *
 */

#include <ncbi_pch.hpp>

#include <gui/utils/app_job_impl.hpp>

#include <corelib/guard.hpp>

BEGIN_NCBI_SCOPE



//static const char* kDefaultErrorMessage = "CAppJob: Tool Execution Failed";

///////////////////////////////////////////////////////////////////////////////
/// CAppJobProgress
CAppJobProgress::CAppJobProgress()
:   m_Done(0.0)
{
}


CAppJobProgress::CAppJobProgress(const CAppJobProgress& progress)
{
    CReadLockGuard lock(progress.m_Lock);
    x_CopyFrom(progress);
}

CAppJobProgress& CAppJobProgress::operator=(const CAppJobProgress& progress)
{
    CReadLockGuard lock(progress.m_Lock);
    x_CopyFrom(progress);
    return *this;
}


CAppJobProgress::CAppJobProgress(float done, const string& text)
:   m_Done(done),
    m_Text(text)
{
}


void CAppJobProgress::SetNormDone(float done)
{
    CWriteLockGuard lock(m_Lock);
    m_Done = done;
}


void CAppJobProgress::SetText(const string& text)
{
    CWriteLockGuard lock(m_Lock);
    m_Text = text;
}


float CAppJobProgress::GetNormDone() const
{
    CReadLockGuard lock(m_Lock);
    return m_Done;
}


string CAppJobProgress::GetText() const
{
    CReadLockGuard lock(m_Lock);
    return m_Text;
}

void CAppJobProgress::x_CopyFrom(const CAppJobProgress& progress)
{
    m_Done = progress.m_Done;
    m_Text = progress.m_Text;
}


///////////////////////////////////////////////////////////////////////////////
/// CAppJobError
CAppJobError::CAppJobError( const string& text, bool user_level )
    : m_Text( text )
    , m_UserLevel( user_level )
{
}


string CAppJobError::GetText() const
{
    return m_Text;
}

bool CAppJobError::IsUserLevel() const
{
    return m_UserLevel;
}


///////////////////////////////////////////////////////////////////////////////
/// CAppJobTextResult

CAppJobTextResult::CAppJobTextResult(const string& text)
:   m_Text(text)
{
}


string CAppJobTextResult::GetText() const
{
    return m_Text;
}



///////////////////////////////////////////////////////////////////////////////
/// CAppJob

CAppJob::CAppJob(const string& descr)
:   m_Descr(descr)
{
}


void CAppJob::x_SetStatusText(const string& text)
{
    CFastMutexGuard lock(m_Mutex);
    m_Status = text;
}


void CAppJob::x_ResetState()
{
    CFastMutexGuard lock(m_Mutex);

    m_Error.Reset();

    m_Status = "Starting...";
}



CConstIRef<IAppJobProgress> CAppJob::GetProgress()
{
    CFastMutexGuard lock(m_Mutex);
    CConstIRef<IAppJobProgress> pr(new CAppJobProgress(-1.0, m_Status));
    return pr;
}


CRef<CObject> CAppJob::GetResult()
{
    return CRef<CObject>(); // we do not support this
}


CConstIRef<IAppJobError> CAppJob::GetError()
{
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}


string CAppJob::GetDescr() const
{
    return m_Descr;
}


END_NCBI_SCOPE
