#ifndef GUI_WIDGETS_ALN_MULTIPLE___BUILD_ALN_VEC_TASK__HPP
#define GUI_WIDGETS_ALN_MULTIPLE___BUILD_ALN_VEC_TASK__HPP

/*  $Id: build_alnvec_job.hpp 44958 2020-04-28 18:04:11Z shkeda $
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
 */

#include <gui/utils/app_job.hpp>
#include <gui/utils/app_job_impl.hpp>

#include <objects/seq/Seq_annot.hpp>

#include <objmgr/scope.hpp>

#include <objtools/alnmgr/task_progress.hpp>
#include <objtools/alnmgr/alnvec.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///  CBuildAlnVecResult
class CBuildAlnVecResult : public CObject
{
public:
    CRef<objects::CAlnVec>  m_AlnVec;
};


///////////////////////////////////////////////////////////////////////////////
/// CBuildAlnVecJob
class CBuildAlnVecJob
    : public CJobCancelable
    , public ITaskProgressCallback

{
public:
    typedef vector< CConstRef<objects::CSeq_align> >  TAligns;

    CBuildAlnVecJob(const TAligns& aligns, objects::CScope& scope, bool select_anchor = false);
    virtual ~CBuildAlnVecJob();

    /// @name IAppJob implementation
    /// @{
    virtual EJobState   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string    GetDescr() const;
    /// @}

    virtual void    Build();

    /// @name ITaskProgressCallback
    /// @{
    virtual void SetTaskName(const string& name);
    virtual void SetTaskCompleted (int completed);
    virtual void SetTaskTotal(int total);
    virtual bool InterruptTask();
    /// @}

protected:
    CFastMutex m_Mutex;

    string  m_Descr;

    CRef<CBuildAlnVecResult>   m_Result;
    CRef<CAppJobError>      m_Error;

    CRef<objects::CScope>    m_Scope;

    TAligns   m_Aligns;

    // ITaskProgressCallback support
    string  m_TaskName;
    int     m_TaskCompleted;
    int     m_TaskTotal;
    bool    m_SelectAnchor = false;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_MULTIPLE___BUILD_ALN_VEC_TASK__HPP
