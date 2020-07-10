/*  $Id: data_track.cpp 42833 2019-04-19 15:47:35Z shkeda $
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
 */
#include <ncbi_pch.hpp>

#include <gui/utils/log_performance.hpp>
#include <gui/widgets/seq_graphic/data_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_conf.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>

BEGIN_NCBI_SCOPE

BEGIN_EVENT_MAP(CDataTrack, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged,
             &CDataTrack::OnAJNotification)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eProgress,
             &CDataTrack::OnAJProgress)
END_EVENT_MAP()

CDataTrack::CDataTrack(CRenderingContext* r_cntx) : CGlyphContainer(r_cntx)
{}


CDataTrack::~CDataTrack()
{}


void CDataTrack::Update(bool layout_only)
{
    if (!m_On) {
        return;
    }

    if ( !layout_only  &&  m_Expanded) {
        // loading the data
        x_UpdateData();
    }
    CGlyphContainer::Update(true);
}


void CDataTrack::DeleteAllJobs()
{
    CRef<CSGGenBankDS> ds = GetDataSource();
    if (ds) {
        ds->DeleteAllJobs();
    }
}


bool CDataTrack::AllJobsFinished() const
{
    CConstRef<CSGGenBankDS> ds = GetDataSource();
    if (ds) {
        return ds->AllJobsFinished();
    }
    return true;
}


void CDataTrack::OnAJNotification(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);
    ++m_JNotificationDepth;
    if (notn) {
        int job_id = notn->GetJobID();

        if (job_id != -1  &&  !x_IsJobNeeded(job_id)) {
            --m_JNotificationDepth;
            return;
        }
        switch(notn->GetState())    {
            case IAppJob::eCompleted:
                //LOG_POST(Info << "CDataTrack: completed job:" << job_id);
                x_OnJobCompleted(*notn);
                break;
            case IAppJob::eFailed:
                x_OnJobFailed(*notn);
                break;
            case IAppJob::eCanceled:
                //x_OnJobCanceled(*notn, *session);
                break;
            default:
                break;
        }  // switch
    }  // notn

    CRef<CSGGenBankDS> ds = GetDataSource();
    if (ds  &&  ds->AllJobsFinished() && m_JNotificationDepth == 1) {
        x_OnAllJobsFinished();
    }
    --m_JNotificationDepth;
}


void CDataTrack::OnAJProgress(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);
    if (notn) {
        CConstIRef<IAppJobProgress> progress = notn->GetProgress();
        if (progress) {
            int p = (int)(progress->GetNormDone() * 100);
            string msg = kEmptyStr;
            if ( !progress->GetText().empty() ) {
                msg = ", " + progress->GetText();
            }

            if (p == m_Progress  &&  msg == m_Msg) {
                return;
            }
            x_SetStatus(msg, p);
            x_OnLayoutChanged();
        }
    }
}

void CDataTrack::x_UpdateData()
{
    m_PerfLog.reset(new CLogPerformance("CDataTrack"));
    (*m_PerfLog) << "Description: " << GetFullTitle();
}

void CDataTrack::x_Expand(bool expand)
{
    if ((m_Attrs & fCollapsible)  &&  !expand) {
        DeleteAllJobs();
    }
    CLayoutTrack::x_Expand(expand);
}

/* virtual */
bool CDataTrack::x_IsJobNeeded(CAppJobDispatcher::TJobID id) const
{
    CConstRef<CSGGenBankDS> ds = GetDataSource();
    if (ds) {
        return ds->IsJobNeeded(id);
    }
    return false;
}


/* virtual */
void CDataTrack::x_OnJobFailed(CAppJobNotification& notify)
{
    STrackErrorStatus errorStatus;
    errorStatus.m_ErrorCode = STrackErrorStatus::eErrorCode_Other;
    errorStatus.m_ErrorSeverity = STrackErrorStatus::eErrorSeverity_Fatal;
    errorStatus.m_ShortErrorMsg = "Could not retrieve data";
    CConstIRef<IAppJobError> appError = notify.GetError();
    if (appError.NotNull()) {
        errorStatus.m_ErrorMsg = "Track data loading failed";
        LOG_POST(Warning << "Track load error detected, message: " << appError->GetText());
    }
    SetErrorStatus(errorStatus);

    SetMsg(", could not retrieve data");
    CRef<CSGGenBankDS> ds = GetDataSource();
    if (ds) {
        ds->ClearJobID(notify.GetJobID());
    }
    if (!m_PerfLog.get())
        return;

    CConstIRef<IAppJobError> error = notify.GetError();
    if (!error.IsNull() && !error->GetText().empty()) {
        (*m_PerfLog) << " Error: " << error->GetText();
        m_PerfLog->Post(CRequestStatus::e500_InternalServerError);
        m_PerfLog.reset();
    }
}


/* virtual */
void CDataTrack::x_OnAllJobsFinished()
{
    if (GetChildren().empty()) {
        m_Msg += ", No data";
        if(!isInErrorStatus()) {
            SetErrorStatus(STrackErrorStatus(STrackErrorStatus::eErrorSeverity_Warning,
                                            STrackErrorStatus::eErrorCode_NoDataW,
                                            "No data in this range",
                                            "No track data found in this range"));
        }
}
    x_SetStatus(m_Msg, 100);
    if (m_PerfLog.get())
        m_PerfLog->Post();
}


END_NCBI_SCOPE
