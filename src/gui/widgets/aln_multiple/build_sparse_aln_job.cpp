/*  $Id: build_sparse_aln_job.cpp 26643 2012-10-18 17:54:05Z katargir $
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

#include <ncbi_pch.hpp>

#include <gui/widgets/aln_multiple/build_sparse_aln_job.hpp>
#include <gui/utils/event_translator.hpp>

#include <objtools/alnmgr/aln_converters.hpp>
#include <objtools/alnmgr/aln_builders.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

///////////////////////////////////////////////////////////////////////////////
/// CBuildSparseAlnJob
CBuildSparseAlnJob::CBuildSparseAlnJob(const TAnchoredAlnVector& aligns,
                                       const TOptions& options,
                                       CScope& scope)
:   m_Descr("Build CAlnVec-based alignment"),
    m_Scope(&scope),
    m_AnchoredAlns(aligns),
    m_Options(options),
    m_TaskCompleted(0),
    m_TaskTotal(0)
{
}


CBuildSparseAlnJob::~CBuildSparseAlnJob()
{
}


IAppJob::EJobState CBuildSparseAlnJob::Run()
{
    //LOG_POST(Info << "CBuildSparseAlnJob::Run()  Started  " << m_Descr);
    m_Result.Reset();
    m_Error.Reset();

    string err_msg, logMsg = "CBuildSparseAlnJob - exception in Run() ";

    try {
        Build();
    }
    catch (CException& e) {
        err_msg = logMsg + GetDescr() + ". " + e.GetMsg();
        LOG_POST(Error << err_msg);
        LOG_POST(Error << e.ReportAll());
    } catch (std::exception& e) {
        err_msg = logMsg + GetDescr() + ". " + e.what();
        LOG_POST(Error << err_msg);
    } 
    if (IsCanceled())
        return eCanceled;

    if (!err_msg.empty()) {
        m_Error.Reset(new CAppJobError(err_msg));
        return eFailed;
    }

    //LOG_POST(Info << "CBuildSparseAlnJob::Run()  Finished  " << m_Descr);
    return eCompleted;
}



CConstIRef<IAppJobProgress> CBuildSparseAlnJob::GetProgress()
{
    CAppJobProgress* prg = new CAppJobProgress();

    CFastMutexGuard lock(m_Mutex);

    string s = m_TaskName + "  ";
    s += NStr::IntToString(m_TaskCompleted) + " of ";
    s += NStr::IntToString(m_TaskTotal);
    prg->SetText(s);
    prg->SetNormDone( static_cast<float>( m_TaskCompleted ) / m_TaskTotal );

    return CConstIRef<IAppJobProgress>(prg);
}


CRef<CObject> CBuildSparseAlnJob::GetResult()
{
    return CRef<CObject>(m_Result.GetPointer());
}


CConstIRef<IAppJobError> CBuildSparseAlnJob::GetError()
{
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}


string CBuildSparseAlnJob::GetDescr() const
{
    return m_Descr;
}


void CBuildSparseAlnJob::SetTaskName(const string& name)
{
    CFastMutexGuard lock(m_Mutex);
    m_TaskName = name;
}


void CBuildSparseAlnJob::SetTaskCompleted (int completed)
{
    CFastMutexGuard lock(m_Mutex);
    m_TaskCompleted = completed;
}


void CBuildSparseAlnJob::SetTaskTotal(int total)
{
    CFastMutexGuard lock(m_Mutex);
    m_TaskTotal = total;
}


bool CBuildSparseAlnJob::InterruptTask()
{
    return IsCanceled();
}


void CBuildSparseAlnJob::Build()
{
    if (!m_AnchoredAlns.size())
        return;

    SetTaskName("Adding Seq-aligns to the alignment");
    SetTaskTotal((int)m_AnchoredAlns.size());

    /// Build a single anchored aln
    CAnchoredAln* built_anchored_aln = new CAnchoredAln(); //TODO leak

    if (m_AnchoredAlns.size() == 1) {
        *built_anchored_aln = *m_AnchoredAlns[0];
    } else {
        BuildAln(m_AnchoredAlns, *built_anchored_aln, m_Options);
    }

    /// Get sequence:
    CSparseAln* alignment = new CSparseAln(*built_anchored_aln, *m_Scope);
    if( !alignment ) {
        NCBI_THROW(CException, eUnknown, "Failed to build a sparse alignment");
    }

    {
        CFastMutexGuard lock(m_Mutex);
        m_Result = new CBuildSparseAlnResult();
        m_Result->m_SparseAln = alignment;
    }

    const size_t row_num = alignment->GetNumRows();
    // preload the sequences

    SetTaskName("Loading sequences... ");
    SetTaskTotal(alignment->GetNumRows());
    for (size_t row = 0; row < row_num; ++row) {
        if(IsCanceled())
            return;
        try {
            alignment->GetBioseqHandle((int)row);
        } catch (std::exception&) {
            // Preloading is just for performance consideration.
            // we simply ignore any errors if a sequence fails to load.
        }

        SetTaskCompleted((int)(row + 1));
    }
}


END_NCBI_SCOPE
