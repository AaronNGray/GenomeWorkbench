#ifndef GUI_OBJUTILS___VERSIONED_JOB_FACTORY__HPP
#define GUI_OBJUTILS___VERSIONED_JOB_FACTORY__HPP

/*  $Id: versioned_job_factory.hpp 38703 2017-06-10 01:06:33Z rudnev $
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
 * Author:  Vladislav Evgeniev
 *
 * File Description:
 *   Versioned Worker Node Factory
 *
 */

/// @file version_job_factory.hpp
/// Versioned Worker Node Factory
///

#include <connect/services/grid_worker.hpp>
#include <gui/objutils/cgi_version_info.hpp>

BEGIN_NCBI_SCOPE

template <typename TWorkerNodeJob>
class CVersionedJobFactory : public IWorkerNodeJobFactory
{
public:
    CVersionedJobFactory(const CCgiVersionInfo &versionInfo) :
        m_CgiVersionInfo(versionInfo)
    {
    }
    virtual void Init(const IWorkerNodeInitContext& context)
    {
        m_WorkerNodeInitContext = &context;
    }
    virtual IWorkerNodeJob* CreateInstance(void)
    {
        return new TWorkerNodeJob(*m_WorkerNodeInitContext, m_CgiVersionInfo);
    }

private:
    const IWorkerNodeInitContext* m_WorkerNodeInitContext;
    CCgiVersionInfo               m_CgiVersionInfo;
};

#define NCBI_DECLARE_WORKERNODE_VER_FACTORY(TWorkerNodeJob, Version, Revision, BuildDate)                               \
class TWorkerNodeJob##Factory : public CVersionedJobFactory<TWorkerNodeJob>                                             \
{                                                                                                                       \
public:                                                                                                                 \
    TWorkerNodeJob##Factory() :                                                                                         \
        CVersionedJobFactory<TWorkerNodeJob>(CCgiVersionInfo(CVersionInfo(NCBI_AS_STRING(Version)),Revision,BuildDate)) \
    {                                                                                                                   \
    }                                                                                                                   \
    virtual string GetJobVersion() const                                                                                \
    {                                                                                                                   \
        return #TWorkerNodeJob " version " NCBI_AS_STRING(Version);                                                     \
    }                                                                                                                   \
    virtual string GetAppName() const                                                                                   \
    {                                                                                                                   \
        return #TWorkerNodeJob;                                                                                         \
    }                                                                                                                   \
    virtual string GetAppVersion() const                                                                                \
    {                                                                                                                   \
        return NCBI_AS_STRING(Version);                                                                                 \
    }                                                                                                                   \
}

END_NCBI_SCOPE

#endif //GUI_OBJUTILS___VERSIONED_JOB_FACTORY__HPP