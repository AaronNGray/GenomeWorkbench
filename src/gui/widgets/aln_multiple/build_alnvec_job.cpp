/*  $Id: build_alnvec_job.cpp 44958 2020-04-28 18:04:11Z shkeda $
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

#include <ncbi_pch.hpp>

#include <gui/widgets/aln_multiple/build_alnvec_job.hpp>

#include <gui/utils/event_translator.hpp>

#include <objtools/alnmgr/alnmix.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

/*
CBuildAlnVecTask::CBuildAlnVecTask()
:   m_StopRequested(false)
{
}


CBuildAlnVecTask::~CBuildAlnVecTask()
{
}


void CBuildAlnVecTask::Reset(CScope& scope)
{
    CMutexGuard lock(m_Mutex);

    if(! IsRunning())   {
        m_Scope.Reset(&scope);

        m_Aligns.clear();
        m_AlnVec.Reset();

        m_ShadowAligns.clear();
        m_ShadowAlnVec.Reset();
    } else {
        ERR_POST("CBuildAlnVecTask::Reset() - ignored, the task is running");
    }
}


void CBuildAlnVecTask::SetInput(const TAligns& aligns)
{
    m_Aligns = aligns;
}


bool CBuildAlnVecTask::IsEmpty() const
{
    return ! m_AlnVec;
}


string CBuildAlnVecTask::GetStatusMessage()
{
    return m_StatusMessage;
}


CRef<CAlnVec> CBuildAlnVecTask::GetResults()
{
    return m_AlnVec;
}

// this is a qucik-and-dirty way to support synchronous processing
bool CBuildAlnVecTask::RunSync()
{
    x_InitWorkContext();
    bool ok = (x_Run() == eFinished);
    if(ok)    {
        x_TransferResults();
    }
    x_ClearWorkContext();
    return ok;
}


void CBuildAlnVecTask::SetTaskName(const string& name)
{
    CFastMutexGuard lock(m_Mutex);
    m_TaskName = name;
}


void CBuildAlnVecTask::SetTaskCompleted (int completed)
{
    CFastMutexGuard lock(m_Mutex);
    m_TaskCompleted = completed;
}


void CBuildAlnVecTask::SetTaskTotal(int total)
{
    CFastMutexGuard lock(m_Mutex);
    m_TaskTotal = total;
}


bool CBuildAlnVecTask::InterruptTask()
{
    return m_StopRequested;
}


string CBuildAlnVecTask::GetProgressMessage()
{
    CFastMutexGuard lock(m_Mutex);

    string s = m_TaskName + "  ";
    s += NStr::IntToString(m_TaskCompleted) + " of ";
    s += NStr::IntToString(m_TaskTotal);
    return s;
}


CUIAsyncTask::EStatus CBuildAlnVecTask::x_Run()
{
    bool ok = false;
    if(m_ShadowAligns.size()) {

        CAlnMix mix(*m_Scope);

        size_t n_align = m_ShadowAligns.size();
        {
            CFastMutexGuard lock(m_Mutex);
            m_TaskName = "Adding Seq-aligns to the alignment";
            m_TaskTotal = n_align;
        }
        //LOG_POST(Info << "Adding " << n_align << " alignments to CAlnMix");

        // add all Seq-aligns to the Alignment
        for( size_t i = 0;  i < n_align;  i++ ) {
            if(m_StopRequested) {
                return eCanceled;
            }
            //LOG_POST(Info << "Adding " << m_ShadowAligns[i]);

            mix.Add(*m_ShadowAligns[i]);
            SetTaskCompleted(i + 1);

            //LOG_POST("CBuildAlnVecTask::x_Run() - added Seq-align");
        }

        // Merge alignment
        try {
            //LOG_POST("CBuildAlnVecTask::x_Run() - merging alignments...");
            mix.Merge(CAlnMix::fGapJoin);

            //LOG_POST("CBuildAlnVecTask::x_Run() - Merged");
            m_ShadowAlnVec.Reset(new CAlnVec(mix.GetDenseg(), mix.GetScope()));
            ok = true;
            //LOG_POST("CBuildAlnVecTask::x_Run() - replaced Shadow AlnVec");
        }
        catch (CException& e) {
            m_StatusMessage = e.GetMsg();
            ERR_POST(Error << "CAlignDataSource::x_BuildShadowAlnVec() "
                    << e.what());
        }
    }
    return ok ? eFinished : eFailed;
}


void CBuildAlnVecTask::x_InitWorkContext()
{
    m_ShadowAligns = m_Aligns;
    m_ShadowAlnVec.Reset();
}


void CBuildAlnVecTask::x_TransferResults()
{
    m_AlnVec = m_ShadowAlnVec;
}


void CBuildAlnVecTask::x_ClearWorkContext()
{
    m_ShadowAligns.clear();
    m_ShadowAlnVec.Reset();
}


void CBuildAlnVecTask::x_Cancel()
{
    CMutexGuard lock(m_Mutex);
    m_StopRequested = true;
}
*/

///////////////////////////////////////////////////////////////////////////////
/// CBuildAlnVecJob
CBuildAlnVecJob::CBuildAlnVecJob(const TAligns& aligns, CScope& scope, bool select_anchor)
:   m_Descr("Build CAlnVec-based alignment"),
    m_Scope(&scope),
    m_Aligns(aligns),
    m_TaskCompleted(0),
    m_TaskTotal(0),
    m_SelectAnchor(select_anchor)
{
}


CBuildAlnVecJob::~CBuildAlnVecJob()
{
}


IAppJob::EJobState CBuildAlnVecJob::Run()
{
    //LOG_POST(Info << "CBuildAlnVecJob::Run()  Started  " << m_Descr);

    m_Result.Reset();
    m_Error.Reset();

    string err_msg, logMsg = "CBuildAlnVecJob - exception in Run() ";

    try {
        Build();
    
    } catch( CAlnException& aex ){
        switch( aex.GetErrCode() ){
        case CAlnException::eInvalidRequest:
        case CAlnException::eConsensusNotPresent:
        case CAlnException::eTranslateFailure:
        case CAlnException::eMergeFailure:
        case CAlnException::eUnknownMergeFailure:
            err_msg = "Sorry, merging of multiple alignments has failed. Alignments of this type are not supported.";
            LOG_POST( Warning << logMsg << "\n    " << aex.GetMsg() );
            break;
        case CAlnException::eInvalidSeqId:
            // HACK
            // This error happens often due to some resource is not deallocated
            // properly and persists until the node restart. There are not so
            // many ways to convey this information back to worker node, so we
            // put ShutDown as the first word of the error message to be
            // handled in src/internal/gbench/app/sviewer/graphic_cgi_lib/graphic_cgi.cpp 
            err_msg = "ShutDown " + aex.GetMsg();
            break;
        default:
            err_msg = logMsg + GetDescr() + ". " +aex.GetMsg();
            LOG_POST(Error << err_msg);
            LOG_POST(Error << aex.ReportAll());
            break;
        }

    } catch( CException& e ){
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

    //LOG_POST(Info << "CBuildAlnVecJob::Run()  Finished  " << m_Descr);
    return eCompleted;
}




CConstIRef<IAppJobProgress> CBuildAlnVecJob::GetProgress()
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


CRef<CObject> CBuildAlnVecJob::GetResult()
{
    return CRef<CObject>(m_Result.GetPointer());
}


CConstIRef<IAppJobError> CBuildAlnVecJob::GetError()
{
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}


string CBuildAlnVecJob::GetDescr() const
{
    return m_Descr;
}


void CBuildAlnVecJob::SetTaskName(const string& name)
{
    CFastMutexGuard lock(m_Mutex);
    m_TaskName = name;
}


void CBuildAlnVecJob::SetTaskCompleted (int completed)
{
    CFastMutexGuard lock(m_Mutex);
    m_TaskCompleted = completed;
}


void CBuildAlnVecJob::SetTaskTotal(int total)
{
    CFastMutexGuard lock(m_Mutex);
    m_TaskTotal = total;
}


bool CBuildAlnVecJob::InterruptTask()
{
    return IsCanceled();
}


void CBuildAlnVecJob::Build()
{
    if (!m_Aligns.size())
        return;

    CRef<CAlnVec> aln_vec;
    CConstRef<CSeq_id> anchor_id;

    size_t n_align = m_Aligns.size();

    if (n_align == 1 && m_Aligns[0]->GetSegs().IsDenseg()) {
        aln_vec.Reset(new CAlnVec(m_Aligns[0]->GetSegs().GetDenseg(), *m_Scope));
    }
    else {
        CAlnMix mix(*m_Scope);
        mix.SetTaskProgressCallback(this);

        {
            CFastMutexGuard lock(m_Mutex);
            m_TaskName = "Adding Seq-aligns to the alignment";
            m_TaskTotal = (int)n_align;
        }

        // add all Seq-aligns to the Alignment
        CConstRef<CSeq_id> ids[2]; 
        int anchor_index  = -1;
        bool check_anchor = m_SelectAnchor;
        for (size_t i = 0; i < n_align; i++) {

            if(IsCanceled())
                return;
            const auto& aln = *m_Aligns[i];
            check_anchor = check_anchor && aln.CheckNumRows() == 2;
            if (check_anchor) {
                if (i == 0) {
                    ids[0] = ConstRef(&aln.GetSeq_id(0));
                    ids[1] = ConstRef(&aln.GetSeq_id(1));
                } else if (anchor_index == -1) {
                    if (ids[0]->Match(aln.GetSeq_id(0))) {
                        anchor_index = 0;
                    }
                    if (ids[1]->Match(aln.GetSeq_id(1))) {
                        anchor_index = (anchor_index == 0) ? -1 : 1;
                    }
                } else if (ids[anchor_index]->Match(aln.GetSeq_id(anchor_index)) == false) {
                    check_anchor = false;
                    anchor_index = -1;
                }
            }
            mix.Add(aln);
            SetTaskCompleted((int)(i + 1));
        }

        if (check_anchor && anchor_index != -1)
            swap(anchor_id, ids[anchor_index]);
        // Merge alignments
        mix.Merge(CAlnMix::fGapJoin);
        aln_vec.Reset(new CAlnVec(mix.GetDenseg(), mix.GetScope()));
    }

    {
        CFastMutexGuard lock(m_Mutex);
        m_Result = new CBuildAlnVecResult();
        m_Result->m_AlnVec = aln_vec;
    }

    // preload the sequences
    const int row_num = aln_vec->GetNumRows();
    SetTaskName("Loading sequences... ");
    SetTaskTotal(row_num);
    int anchor_row = -1;
    for (int row = 0; row < row_num; ++row) {
        if(IsCanceled())
            return;
        try {
            if (anchor_id && anchor_id->Match(aln_vec->GetSeqId(row))) {
                if (anchor_row == -1) {
                    anchor_row = row;
                } else {
                    // Multiple rows with the same anchor id
                    // anchor can't be calcualted
                    // reset anchor index
                    anchor_id.Reset();
                    anchor_row = -1;
                }    
            }
            aln_vec->GetBioseqHandle(row);
        } catch (std::exception&) {
            // Preloading is just for performance consideration.
            // we simply ignore any errors if a sequence fails to load.
        }

        SetTaskCompleted(row + 1);
    }

    // preset the segment type to avoid blocking the main thread
    SetTaskName("Initializing alignment segment types ... ");
    SetTaskTotal(row_num);
    for (int row = 0; row < row_num; ++row) {
        if(IsCanceled())
            return;
        try {
            aln_vec->GetSegType(row, 0);
        } catch (std::exception&) {
            // Preloading is just for performance consideration.
            // we simply ignore any errors if any failure
        }

        SetTaskCompleted(row + 1);
    }
    if (anchor_row != -1)
        m_Result->m_AlnVec->SetAnchor(anchor_row);

}


END_NCBI_SCOPE
