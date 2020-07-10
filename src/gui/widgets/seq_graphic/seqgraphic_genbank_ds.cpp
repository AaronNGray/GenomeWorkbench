/*  $Id: seqgraphic_genbank_ds.cpp 44757 2020-03-05 18:58:50Z evgeniev $
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
 * Authors:  Vlad Lebedev, Liangshou Wu
 *
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_genbank_ds.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/utils/app_job_dispatcher.hpp>

#include <objmgr/util/sequence.hpp>
#include <objmgr/seq_loc_mapper.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/graph_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/data_loader.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/impl/tse_info.hpp>
#include <objmgr/impl/data_source.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/taxon1/taxon1.hpp>

#include <objects/seqblock/GB_block.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/seq/Seq_descr.hpp>

#include <gui/widgets/seq_graphic/graph_utils.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CSGGenBankDS
///
CSGGenBankDS::CSGGenBankDS(CScope& scope, const CSeq_id& id)
    : m_JobListener(NULL)
    , m_Depth(-1)
    , m_Adaptive(true)
{
    m_Handle = scope.GetBioseqHandle(id);
    if ( !m_Handle ) {
        NCBI_THROW(CException, eUnknown,
            string("Can't retrieve sequence for id: ") + id.AsFastaString());
    }

    m_ActiveJobs.clear();
}


CSGGenBankDS::~CSGGenBankDS()
{
    DeleteAllJobs();
}


void CSGGenBankDS::ClearJobID(TJobID job_id)
{
    vector<TJobID>::iterator it =
        find(m_ActiveJobs.begin(), m_ActiveJobs.end(), job_id);
    if (it != m_ActiveJobs.end()) {
        m_ActiveJobs.erase(it);
    }
}


void CSGGenBankDS::DeleteAllJobs()
{
    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();

    ITERATE(vector<TJobID>, it, m_ActiveJobs) {
        TJobID job_id = *it;
        try {
            disp.DeleteJob(job_id);
        } catch(CAppJobException& e)  {
            switch(e.GetErrCode())  {
            case CAppJobException::eUnknownJob:
            case CAppJobException::eEngine_UnknownJob:
                /// this is fine - job probably already finished
                break;
            default:
                // something wrong
                LOG_POST(Error << "CSGGenBankDS::DeleteAllJobs() "
                    << "Error canceling job");
                LOG_POST(e.ReportAll());
            }
        }
    }

    m_ActiveJobs.clear();
}


bool CSGGenBankDS::IsRefSeq() const
{
    return CSGUtils::IsRefSeq(m_Handle);
}


void CSGGenBankDS::x_ForegroundJob(IAppJob& job)
{
    IAppJob::EJobState state = job.Run();

    switch (state) {
        case IAppJob::eCompleted:
        {
            CRef<CObject> res = job.GetResult();
            _ASSERT(res);
            CRef<CEvent> evt(new CAppJobNotification(-1, res));
            m_JobListener->Send(evt);
            break;
        }
        case IAppJob::eFailed:
        {
            CConstIRef<IAppJobError> err(job.GetError().GetPointer());
            if (err) {
                CRef<CEvent> evt(new CAppJobNotification(-1, *err));
                m_JobListener->Send(evt);
            }
            else {
                CRef<CEvent> evt(new CAppJobNotification(-1, state));
                m_JobListener->Send(evt);
            }
            break;
        }
        default:
            break;
    }
}


CSGGenBankDS::TJobID CSGGenBankDS::x_BackgroundJob(IAppJob& job,
                                                   int report_period,
                                                   const string& pool)
{
    TJobID job_id = -1;
    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    try {
        job_id = disp.StartJob(job, pool, *m_JobListener, report_period, true);
    } catch(CAppJobException& e)  {
        LOG_POST(Error << "CSGGenBankDS::x_BackgroundJob() - Failed to start job");
        LOG_POST(e.ReportAll());
    }
    return job_id;
}


CSGGenBankDS::TJobID CSGGenBankDS::x_LaunchJob(IAppJob& job,
                                               int report_period,
                                               const string& pool)
{
    TJobID job_id = -1;
    if (m_Background) {
        job_id = x_BackgroundJob(job, report_period, pool);
        if (job_id != -1) {
            m_ActiveJobs.push_back(job_id);
        }
    } else {
        x_ForegroundJob(job);
    }

    return job_id;
}

void CSGGenBankDS::SetGraphLevels(const string& annot)
{
    CRef<CSeq_loc> seq_loc =
        m_Handle.GetRangeSeq_loc(0,
        m_Handle.GetSeqVector(CBioseq_Handle::eCoding_Iupac).size());

    CGraphUtils::CalcGraphLevels(annot, GetScope(), *seq_loc, m_GraphLevels);
}


///////////////////////////////////////////////////////////////////////////////
/// CSGSequenceDS
CSGSequenceDS::CSGSequenceDS(CScope& scope, const CSeq_id& id)
{
    m_Handle = scope.GetBioseqHandle(id);
    if ( !m_Handle ) {
        NCBI_THROW(CException, eUnknown,
            string("Can't retrieve sequence for id: ") + id.AsFastaString());
    }

    CSeq_id_Handle idh = m_Handle.GetSeq_id_Handle();
    m_Best_idh = sequence::GetId(idh, scope, sequence::eGetId_Best);

    if ( !m_Best_idh ) {
        m_Best_idh = idh;
    }
    m_SeqVector.Reset(
        new CSeqVector(m_Handle.GetSeqVector(CBioseq_Handle::eCoding_Iupac)));
}


CSGSequenceDS::~CSGSequenceDS()
{
}


void CSGSequenceDS::GetSequence(TSeqPos from, TSeqPos to, string& buffer) const
{
    buffer.erase();
    if (m_SeqVector) {
        // CSeqVector::GetSeqData returns sequence for [from stop)
        // so we need to add one more base to get sequence at 'to' position
        m_SeqVector->GetSeqData(from, to + 1, buffer);
    }
}


string CSGSequenceDS::GetTitle() const
{
    return sequence::CDeflineGenerator().GenerateDefline(m_Handle);
}


string CSGSequenceDS::GetAcc_Best() const
{
    string acc_best;
    m_Best_idh.GetSeqId()->GetLabel(&acc_best, CSeq_id::eContent);
    return acc_best;
}


string CSGSequenceDS::GetAcc_All() const
{
    return CSeq_id::GetStringDescr(*m_Handle.GetBioseqCore(),
                                   CSeq_id::eFormat_FastA);
}


bool CSGSequenceDS::IsAccGenomic(CSeq_id::EAccessionInfo acc_info) const
{
    if (acc_info == CSeq_id::eAcc_refseq_chromosome) //NC_
        return true;
    if (acc_info == CSeq_id::eAcc_refseq_contig) //NT_
        return true;
    if (acc_info == CSeq_id::eAcc_refseq_genomic) //NG_
        return true;
    if (acc_info == CSeq_id::eAcc_refseq_genome) //NS_
        return true;
    if (acc_info == CSeq_id::eAcc_refseq_wgs_intermed) // NW_
        return true;
    return false;
}


bool CSGSequenceDS::IsTraceAssembly() const
{
    CConstRef<CSeq_id> id = m_Best_idh.GetSeqId();
    if (id  &&  id->IsGeneral()  &&
        NStr::EqualNocase(id->GetGeneral().GetDb(), "TRACE_ASSM")) {
        return true;
    }

    return false;
}


bool CSGSequenceDS::IsPopSet() const
{
    CSeq_entry_Handle tse = m_Handle.GetTopLevelEntry();
    if (tse.IsSet()  &&  tse.GetSet().IsSetClass()) {
        CBioseq_set::TClass bset_class = tse.GetSet().GetClass();
        switch (bset_class) {
            case CBioseq_set::eClass_pop_set:
            case CBioseq_set::eClass_phy_set:
            case CBioseq_set::eClass_eco_set:
            case CBioseq_set::eClass_mut_set:
                {{
                    CAlign_CI align_iter(m_Handle);
                    if (align_iter.GetSize()) {
                        return true;
                    }
                }}
                break;
            default:
                break;
            }
    }

    return false;
}


bool CSGSequenceDS::IsRefSeq() const
{
    return CSGUtils::IsRefSeq(m_Handle);
}


bool CSGSequenceDS::IsRefSeqGene() const
{
    CSeqdesc_CI desc_ci(m_Handle, CSeqdesc::e_Genbank);
    while (desc_ci) {
        const CGB_block& block = desc_ci->GetGenbank();
        if (block.CanGetKeywords()) {
            ITERATE (CGB_block::TKeywords, k_iter, block.GetKeywords()) {
                if (*k_iter == "RefSeqGene") {
                    return true;
                }
            }
        }
        ++desc_ci;
    }

    return false;
}


///////////////////////////////////////////////////////////////////////////////
/// CSGSequenceDSType
///

ISGDataSource*
CSGSequenceDSType::CreateDS(SConstScopedObject& object) const
{
    const CSeq_id& id = dynamic_cast<const CSeq_id&>(object.object.GetObject());
    return new CSGSequenceDS(object.scope.GetObject(), id);
}

string CSGSequenceDSType::GetExtensionIdentifier() const
{
    static string sid("seqgraphic_genbank_ds_type");
    return sid;
}

string CSGSequenceDSType::GetExtensionLabel() const
{
    static string slabel("Graphical View Genbank Data Source Type");
    return slabel;
}

/// check if the data source can be shared.
bool CSGSequenceDSType::IsSharable() const
{
    return true;
}

END_NCBI_SCOPE
