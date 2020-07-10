/*  $Id: text_retrieve_job.cpp 31535 2014-10-21 16:00:48Z katargir $
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

#include <gui/widgets/seq/text_retrieve_job.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>

#include <wx/dcmemory.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CTextRetrieveJob::~CTextRetrieveJob()
{
#ifdef _DEBUG
    m_RootItem.reset();
    m_Context.reset();
    LOG_POST(Info << "CTextRetrieveJob:: Text item count: " << gITextItemCount.Get());
#endif
}

IAppJob::EJobState CTextRetrieveJob::Run()
{
    string jobName = x_GetJobName();
    string err_msg, logMsg = jobName + " - exception in Run() ";

    IAppJob::EJobState jobState = eCompleted;

    try {
        CLockerGuard guard = x_GetGuard();
        if (IsCanceled()) return eCanceled;
        m_RootItem.reset(new CRootTextItem());
        jobState = x_Run();
    }
    catch (CException& e) {
        err_msg = e.GetMsg();
        LOG_POST(Error << logMsg << GetDescr() << ". " << err_msg);
        //e.ReportAll();
    } catch (std::exception& e) {
        err_msg = GetDescr() + ". " + e.what();
        LOG_POST(Error << logMsg << GetDescr() << ". " << err_msg);
    }
    catch (const CCancelException&) {
    }

    if (IsCanceled())
        return eCanceled;

    if (!err_msg.empty()) {
        string s = err_msg;
        m_Error.Reset(new CAppJobError(s));
        return eFailed;
    }

    return jobState;
}


void CTextRetrieveJob::x_CheckCancelled() const
{
    if (IsCanceled()) {
        throw CCancelException();
    }
}

void CTextRetrieveJob::x_AddItem(ITextItem* item)
{
    m_RootItem->AddItem(item, false);
}

void CTextRetrieveJob::x_AddItems(vector<ITextItem*>& items)
{
    m_RootItem->InsertItems(m_RootItem->GetItemCount(), &items[0], items.size(), false);
}

class CCancelGuard
{
public:
    CCancelGuard(ICanceled* canceled, CTextPanelContext& context) : m_Context(context)
    {
        m_SaveCanceled = m_Context.SetCanceled(canceled);
    }
    ~CCancelGuard()
    {
        m_Context.SetCanceled(m_SaveCanceled);
    }
private:
    CTextPanelContext& m_Context;
    ICanceled* m_SaveCanceled;
};

void CTextRetrieveJob::x_CreateResult()
{
    CFastMutexGuard guard(m_Mutex);
    m_Result.Reset(new CTextViewRetrieveResult());

    if (!IsCanceled()) {
        CCancelGuard guard(x_GetICanceled(), *m_Context);
        wxMemoryDC dc;
        wxFont font(m_Context->GetFontDesc());
        dc.SetFont(font);
        m_Context->CalcWSize(dc);
        m_RootItem->CalcSize(dc, m_Context.get());
        m_Result->SetData(m_RootItem, m_Context);
    }
}

CConstIRef<IAppJobProgress> CTextRetrieveJob::GetProgress()
{
    return CConstIRef<IAppJobProgress>();
}

CRef<CObject> CTextRetrieveJob::GetResult()
{
    return CRef<CObject>(m_Result.GetPointer());
}

CConstIRef<IAppJobError> CTextRetrieveJob::GetError()
{
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}

END_NCBI_SCOPE
