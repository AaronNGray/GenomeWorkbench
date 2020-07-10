/*  $Id: text_ds.cpp 41123 2018-05-29 16:21:42Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/execute_lock_guard.hpp>

#include <gui/widgets/seq/fasta_retrieve_job.hpp>
#include <gui/widgets/seq/flat_file_retrieve_job.hpp>
#include <gui/widgets/seq/flat_feature_retrieve_job.hpp>
#include <gui/widgets/seq/asn_retrieve_job.hpp>
#include <gui/widgets/seq/xml_retrieve_job.hpp>
#include "flat_file_sequence_list_job.hpp"
#include <gui/widgets/text_widget/text_item_panel.hpp>

#include <gui/widgets/seq/text_ds.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

BEGIN_EVENT_MAP(CTextDataSource, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged,
             &CTextDataSource::OnJobNotification)
END_EVENT_MAP()

////////////////////////////////////////////////////////////////////////////////

CTextDataSource::CTextDataSource(CTextPanel& textPanel) : m_TextPanel(textPanel), m_JobId(CAppJobDispatcher::eInvalidJobID)
{
}

CTextDataSource::~CTextDataSource()
{
    x_CancelJob();
}

bool CTextDataSource::IsLoading() const
{
    return (m_JobId != CAppJobDispatcher::eInvalidJobID);
}

void CTextDataSource::x_CancelJob()
{
    if (m_JobId != CAppJobDispatcher::eInvalidJobID) {
        CAppJobDispatcher::GetInstance().DeleteJob(m_JobId);
        m_JobId = CAppJobDispatcher::eInvalidJobID;
    }
}

void CTextDataSource::x_SetDataLocker(CJobCancelable& job)
{
    ICommandProccessor* cmdProccessor = m_TextPanel.GetCmdProccessor();
    if (cmdProccessor) job.SetDataLocker(new CWeakExecuteGuard(*cmdProccessor));
}


void CTextDataSource::LoadFasta(CTextPanelContext* context,
                                const vector<pair<CBioseq_Handle, string> >& handles)
{
    x_CancelJob();
    _ASSERT(!handles.empty());

    CRef<CJobCancelable> job(new CFastaRetrieveJob(*context, handles));
    x_SetDataLocker(*job);

    try {
        m_JobId = CAppJobDispatcher::GetInstance().StartJob(*job, "ObjManagerEngine", *this, -1, true);
    } catch(CAppJobException& e)  {
        ERR_POST("CTextDataSource::LoadFasta() - Failed to start job");
        e.ReportAll();
    }
}

void CTextDataSource::LoadFasta(CTextPanelContext* context, const CSeq_loc& loc, CScope& scope)
{
    x_CancelJob();

    CRef<CJobCancelable> job(new CFastaRetrieveLocJob(*context, loc, scope));
    x_SetDataLocker(*job);

    try {
        m_JobId = CAppJobDispatcher::GetInstance().StartJob(*job, "ObjManagerEngine", *this, -1, true);
    } catch(CAppJobException& e)  {
        ERR_POST("CTextDataSource::LoadFasta() - Failed to start job");
        e.ReportAll();
    }
}

void CTextDataSource::GetFlatFileSeqs(
    CSeq_entry_Handle& h,
    const CSubmit_block* submitBlock,
    const CSeq_loc* seq_loc,
    CFlatFileConfig::EStyle style)
{
    x_CancelJob();

    CRef<CJobCancelable> job(new CFlatFileSequenceListJob(h, submitBlock, seq_loc, style));
    x_SetDataLocker(*job);

    try {
        m_JobId = CAppJobDispatcher::GetInstance().StartJob(*job, "ObjManagerEngine", *this, -1, true);
    }
    catch (CAppJobException& e) {
        ERR_POST("CTextDataSource::GetFlatFileSeqs() - Failed to start job");
        e.ReportAll();
    }
}

void CTextDataSource::LoadFlatFile(
    CTextPanelContext* context,
    const vector<pair<CBioseq_Handle, string> >& handles,
    const SFlatFileParams& params)
{
    x_CancelJob();

    CRef<CJobCancelable> job(new CFlatFileRetrieveJob(*context, handles, params));
    x_SetDataLocker(*job);

    try {
        m_JobId = CAppJobDispatcher::GetInstance().StartJob(*job, "ObjManagerEngine", *this, -1, true);
    } catch(CAppJobException& e) {
        ERR_POST("CTextDataSource::LoadSequence() - Failed to start job");
        e.ReportAll();
    }
}

void CTextDataSource::LoadFlatFile(
    CTextPanelContext* context,
    CScope& scope,
    const CSeq_feat& seq_feat,
    bool getGeneModel,
    CFlatFileConfig::EMode mode,
    CFlatFileConfig::EStyle style)
{
    x_CancelJob();

    CRef<CJobCancelable> job(new CFlatFeatureRetrieveJob(*context, mode, style, scope, seq_feat, getGeneModel));
    x_SetDataLocker(*job);

    try {
        m_JobId = CAppJobDispatcher::GetInstance().StartJob(*job, "ObjManagerEngine", *this, -1, true);
    } catch(CAppJobException& e) {
        ERR_POST("CTextDataSource::LoadSequence() - Failed to start job");
        e.ReportAll();
    }
}

void CTextDataSource::LoadAsn(CTextPanelContext* context, const CSerialObject& so, CScope& scope)
{
    x_CancelJob();

    CRef<CJobCancelable> job(new CAsnRetrieveJob(*context, so, scope));
    x_SetDataLocker(*job);

    try {
        m_JobId = CAppJobDispatcher::GetInstance().StartJob(*job, "ObjManagerEngine", *this, -1, true);
    } catch(CAppJobException& e)  {
        ERR_POST("CTextDataSource::LoadSequence() - Failed to start job");
        e.ReportAll();
    }
}

void CTextDataSource::LoadXml(CTextPanelContext* context, const CSerialObject& so, CScope& scope)
{
    x_CancelJob();

    CRef<CJobCancelable> job(new CXmlRetrieveJob(*context, so));
    x_SetDataLocker(*job);

    try {
        m_JobId = CAppJobDispatcher::GetInstance().StartJob(*job, "ObjManagerEngine", *this, -1, true);
    } catch(CAppJobException& e)  {
        ERR_POST("CTextDataSource::LoadSequence() - Failed to start job");
        e.ReportAll();
    }
}

void CTextDataSource::OnJobNotification(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);
    if (!notn) return;

    if (m_JobId != notn->GetJobID())
        return;

    switch (notn->GetState())    {
    case IAppJob::eCompleted:
        m_JobId = CAppJobDispatcher::eInvalidJobID;
        {{
            CRef<CObject> result = notn->GetResult();
            CTextViewRetrieveResult* res1 = dynamic_cast<CTextViewRetrieveResult*>(result.GetPointer());
            CTextViewSequenceListResult* res2 = dynamic_cast<CTextViewSequenceListResult*>(result.GetPointer());

            if (res1) {
                auto_ptr<CRootTextItem>     rootItem;
                auto_ptr<CTextPanelContext> context;
                res1->GetData(rootItem, context);
                m_TextPanel.DataLoaded(*rootItem, *context);
                rootItem.release();
                context.release();
            }
            else if (res2) {
                m_TextPanel.SequenceListLoaded(res2->GetData());
            }
        }}
        break;
    case IAppJob::eFailed:
        m_JobId = CAppJobDispatcher::eInvalidJobID;
        {{
            CConstIRef<IAppJobError> error = notn->GetError();

            string err_msg = "Failed: ";
            if (error)
                err_msg += error->GetText();
            else
                err_msg += "Unknown fatal error";

            m_TextPanel.GetWidget()->ReportError(err_msg);
        }}
        break;
    case IAppJob::eCanceled:
        m_JobId = CAppJobDispatcher::eInvalidJobID;
        break;
    default:
        break;
    }
}

END_NCBI_SCOPE

