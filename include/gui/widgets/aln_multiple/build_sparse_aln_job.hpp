#ifndef GUI_WIDGETS_ALN_MULTIPLE___BUILD_SPARSE_ALN_JOG__HPP
#define GUI_WIDGETS_ALN_MULTIPLE___BUILD_SPARSE_ALN_JOG__HPP

/*  $Id: build_sparse_aln_job.hpp 26653 2012-10-18 21:10:58Z katargir $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <gui/utils/app_job.hpp>
#include <gui/utils/app_job_impl.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objmgr/scope.hpp>
#include <objtools/alnmgr/aln_explorer.hpp>
#include <objtools/alnmgr/sparse_aln.hpp>
#include <objtools/alnmgr/aln_tests.hpp>
#include <objtools/alnmgr/aln_user_options.hpp>
#include <objtools/alnmgr/aln_stats.hpp>
#include <objtools/alnmgr/seqids_extractor.hpp>
//#include <objtools/alnmgr/task_progress.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///  CBuildAlnVecResult
class CBuildSparseAlnResult : public CObject
{
public:
    CRef<CSparseAln>  m_SparseAln;
};


///////////////////////////////////////////////////////////////////////////////
/// CBuildSparseAlnJob
class CBuildSparseAlnJob : public CJobCancelable
{
public:
    typedef vector<CRef<CAnchoredAln> >  TAnchoredAlnVector;
    typedef CAlnUserOptions TOptions;

    CBuildSparseAlnJob(const TAnchoredAlnVector& aligns,
        const TOptions& options, objects::CScope& scope);
    virtual ~CBuildSparseAlnJob();

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
//    bool    m_StopRequested;

    CRef<CBuildSparseAlnResult>   m_Result;
    CRef<CAppJobError>      m_Error;

    CRef<objects::CScope>   m_Scope;

    TAnchoredAlnVector      m_AnchoredAlns;
    TOptions    m_Options; /// control alignment building

    // ITaskProgressCallback support
    string  m_TaskName;
    int     m_TaskCompleted;
    int     m_TaskTotal;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_MULTIPLE___BUILD_SPARSE_ALN_JOG__HPP
