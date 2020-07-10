/*  $Id: flat_file_sequence_list_job.cpp 39645 2017-10-23 19:42:27Z katargir $
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

#include "flat_file_sequence_list_job.hpp"

#include <objtools/format/item_formatter.hpp>
#include <objtools/format/gather_items.hpp>
#include <objtools/format/items/locus_item.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CFlatFileSequenceListJob::CFlatFileSequenceListJob(
    objects::CSeq_entry_Handle& h,
    const objects::CSubmit_block* submitBlock,
    const objects::CSeq_loc* seq_loc,
    objects::CFlatFileConfig::EStyle style)
    : m_Config(CFlatFileConfig::eFormat_Lite, CFlatFileConfig::eMode_GBench, style, 0, CFlatFileConfig::fViewAll)
{
    m_Context.Reset(new CFlatFileContext(m_Config));

    m_Context->SetEntry(h);
    if (submitBlock) {
        m_Context->SetSubmit(*submitBlock);
    }
    m_Context->SetLocation(seq_loc);
}

CFlatFileSequenceListJob::~CFlatFileSequenceListJob()
{

}

IAppJob::EJobState CFlatFileSequenceListJob::Run()
{
    string jobName = x_GetJobName();
    string err_msg, logMsg = jobName + " - exception in Run() ";

    IAppJob::EJobState jobState = eCompleted;

    try {
        CLockerGuard guard = x_GetGuard();
        if (IsCanceled()) return eCanceled;
        jobState = x_Run();
    }
    catch (CException& e) {
        err_msg = e.GetMsg();
        LOG_POST(Error << logMsg << GetDescr() << ". " << err_msg);
        //e.ReportAll();
    }
    catch (std::exception& e) {
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

CConstIRef<IAppJobProgress> CFlatFileSequenceListJob::GetProgress()
{
    return CConstIRef<IAppJobProgress>();
}

CRef<CObject> CFlatFileSequenceListJob::GetResult()
{
    return CRef<CObject>(m_Result.GetPointer());
}

CConstIRef<IAppJobError> CFlatFileSequenceListJob::GetError()
{
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}

void CFlatFileSequenceListJob::CFlatFileSeqBuilder::AddItem(CConstRef<IFlatItem> item)
{
    if (m_Canceled.IsCanceled())
        throw CCancelException();

    const CLocusItem* locusItem = dynamic_cast<const CLocusItem*>(item.GetPointerOrNull());
    if (locusItem) {
        CBioseqContext* ctx = locusItem->GetContext();
        if (ctx) {
            CBioseq_Handle& handle(ctx->GetHandle());
            if (handle)
                m_Seqs.push_back(CFlatFileSeq(locusItem->GetFullName(), handle));
        }
    }
}

CJobCancelable::EJobState CFlatFileSequenceListJob::x_Run()
{
    CRef<CFlatItemFormatter> formatter(CFlatItemFormatter::New(m_Config.GetFormat()));
    formatter->SetContext(*m_Context);

    CRef<CFlatFileSeqBuilder> builder(new CFlatFileSeqBuilder(m_Seqs, *x_GetICanceled()));

    CRef<CFlatGatherer> gatherer(CFlatGatherer::New(m_Config.GetFormat()));
    gatherer->Gather(*m_Context, *builder);

    m_Result.Reset(new CTextViewSequenceListResult(m_Seqs));

    return eCompleted;
}

END_NCBI_SCOPE
