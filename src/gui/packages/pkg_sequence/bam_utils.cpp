/* 
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
 * Authors:  Andrei Shkeda
 *
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_sequence/bam_utils.hpp>

#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/general/Object_id.hpp>

#include <sra/readers/bam/bamgraph.hpp>
#include <sra/readers/bam/bamread.hpp>
#include <sra/readers/bam/bamindex.hpp>
#include <gui/objutils/gencoll_svc.hpp>
#include <objtools/readers/idmapper.hpp>
#include <objects/seqres/Seq_graph.hpp>

#include <objmgr/object_manager.hpp>
#include <gui/objects/assembly_info.hpp>
#include <util/thread_pool.hpp>

#include <mutex>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const string kBAM_coverage = "BAM coverage";
static const string kSourceFile = "SourceFile";
static const string kIndexFile = "IndexFile";
static const string kAssembly = "Assembly";

static
CRef<CSeq_id> s_GetRefSeq_id(const string& id_str)
{
    CRef<CSeq_id> seq_id;
    try {
        seq_id = new CSeq_id(id_str);
    } catch ( CException&) {
        // ignored
    }

    if ( !seq_id && id_str.find('|') != NPOS ) {
        try {
            CBioseq::TId ids;
            CSeq_id::ParseIDs(ids, id_str);
            if ( !ids.empty() ) {
                seq_id = *ids.begin();
            }
        } catch ( CException& /*ignored*/ ) {
        }
    }

    if ( !seq_id || (seq_id->IsGi() && seq_id->GetGi() < GI_CONST(1000)) ) {
        seq_id = new CSeq_id(CSeq_id::e_Local, id_str);
    }
    return seq_id;
}


typedef struct SCoverageData
{
    const string AnnotName;
    const int BinSize = 0;
    const string bam_file;
    const string index_file;
    bool isEstimated = false;
    CRef<CGC_Assembly> assm;
    mutex data_mutex;
    CRef<CSeq_annot> annot;
    CRef<CScope> scope;
    SCoverageData(const string& assembly,
                  const string& annot_name,
                  int bin_size,
                  const string& bam_file,
                  const string& index_file,
                  bool is_estimated = false) :
         AnnotName(annot_name),
         BinSize(bin_size),
         bam_file(bam_file),
         index_file(index_file),
         isEstimated(is_estimated)
    {
        if (!assembly.empty()) {
            // assembly used for remapping, so a full assembly is needed
            assm = CGencollSvc::GetInstance()->GetGCAssembly(assembly, true, "Gbench");
            CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
            scope.Reset(new CScope(*obj_mgr));
            scope->AddDefaults();
        }
    }
    unique_ptr<IIdMapper> GetMapper()
    {
        unique_ptr<IIdMapper> mapper;
        if (assm) 
            mapper.reset(CGencollIdMapperAdapter::GetIdMapper(assm));
        return mapper;
    }
} SCoverageData;

class CCreateCoverageTask : public CThreadPool_Task
{
public:
    CCreateCoverageTask(const string& id, shared_ptr<SCoverageData> data) :
        m_Id(id),
        m_Data(data)
    {
    }
    EStatus Execute(void)
    {
        bool need_id_mapping = false;
        CBam2Seq_graph cvt;
        cvt.SetRefLabel(m_Id);
        CRef<CSeq_id> seq_id = s_GetRefSeq_id(m_Id);
        if (seq_id->IsLocal()) {
            need_id_mapping = true;
        }
        cvt.SetRefId(*seq_id);
        cvt.SetAnnotName(m_Data->AnnotName);
        cvt.SetGraphTitle(m_Data->AnnotName + " coverage graph");
        cvt.SetGraphValueType(cvt.eGraphValueTyps_int);
        cvt.SetGraphBinSize(m_Data->BinSize);
        cvt.SetEstimated(m_Data->isEstimated);
        //cvt.SetOutlierMax(args["max"].AsDouble());
        CBamMgr mgr;
        CBamDb db(mgr, m_Data->bam_file, m_Data->index_file);
        auto mapper = m_Data->GetMapper();
        if (mapper)
            db.SetIdMapper(mapper.get(), eNoOwnership);

        CRef<CSeq_annot> annot_t = cvt.MakeSeq_annot(db, m_Data->bam_file);
        if (IsCancelRequested())
            return eCanceled;

        if (annot_t && need_id_mapping  && m_Data->assm) {
            auto mapper = m_Data->GetMapper();
            mapper->MapObject(*annot_t);
        }
        if (IsCancelRequested())
            return eCanceled;

        if (annot_t) {
            lock_guard<mutex> guard(m_Data->data_mutex);
            if (m_Data->annot) {
                // move all graphs to one annotation since
                // they all share the same name and title
                CSeq_annot::TData::TGraph& target = m_Data->annot->SetData().SetGraph();
                CSeq_annot::TData::TGraph& src = annot_t->SetData().SetGraph();
                move(src.begin(), src.end(), back_inserter(target));
            } else {
                m_Data->annot = annot_t;
                m_Data->annot->SetTitleDesc(m_Data->AnnotName + " coverage graph");
            }
        }
        return eCompleted;
    }
private:
    string m_Id;
    shared_ptr<SCoverageData> m_Data;
};


CRef<CSeq_annot>
CBamUtils::CreateCoverageGraphs(const string& bam_file,
                                const string& index_file,
                                const string& annot_name,
                                const string& target_assembly,
                                int bin_size,
                                TSeqIdsVector* ids,
                                bool is_estimated, 
                                ICanceled* canceled)
{
    shared_ptr<SCoverageData> data = make_shared<SCoverageData>(target_assembly,
                                                                annot_name,
                                                                bin_size,
                                                                bam_file,
                                                                index_file,
                                                                is_estimated);

    vector<string> ref_ids;
    // Get a list reference sequence ids
    {
        CBamMgr mgr;
        CBamDb db(mgr, bam_file, index_file);
        auto mapper = data->GetMapper();
        if (mapper)
            db.SetIdMapper(mapper.get(), eNoOwnership);

        for (CBamRefSeqIterator sit(db); sit; ++sit) {
            if (canceled && canceled->IsCanceled())
                return CRef<CSeq_annot>(0);
            CRef<CSeq_id> seq_id = sit.GetRefSeq_id();
            if (ids) {
                auto it = find_if(ids->begin(), ids->end(), [&](TSeqIdsVector::value_type& ref_seq_id) {
                    return seq_id->Match(*ref_seq_id);
                });
                if (it == ids->end())
                    continue;
            }
            ref_ids.push_back(sit.GetRefSeqId());
        }
    }

    if (ref_ids.empty())
        return CRef<CSeq_annot>(0);
    if (ref_ids.size() == 1) {
        CCreateCoverageTask task(ref_ids.front(), data);
        task.Execute();
    } else {
        static const int kNumConvertorThreads = 6;
        int num_threads = max<int>(2, ref_ids.size() >= kNumConvertorThreads ? kNumConvertorThreads : ref_ids.size());
        CThreadPool thread_pool(ref_ids.size(), num_threads);
        for (auto ref_id : ref_ids) {
            if (canceled && canceled->IsCanceled()) {
                thread_pool.CancelTasks(CThreadPool::fCancelExecutingTasks | CThreadPool::fCancelQueuedTasks);
                break;
            }
            thread_pool.AddTask(new CCreateCoverageTask(ref_id, data));
        }

        while ((thread_pool.GetQueuedTasksCount() + thread_pool.GetExecutingTasksCount()) > 0) {
            if (canceled && canceled->IsCanceled())
                thread_pool.CancelTasks(CThreadPool::fCancelExecutingTasks | CThreadPool::fCancelQueuedTasks);
            SleepMilliSec(250);
        }
        if (canceled && canceled->IsCanceled())
            data->annot.Reset(NULL);
    }
    CBamUtils::UpdateCoverageGraphInfo(*data->annot, data->bam_file, data->index_file, target_assembly);
    return data->annot;
}


bool CBamUtils::GetCoverageGraphInfo(const CSeq_annot& seq_annot, string& bam_data, string& bam_index, string& assembly)
{
    bam_data.clear();
    bam_index.clear();
    assembly.clear();
    if (!seq_annot.IsGraph() || !seq_annot.IsSetDesc())
        return false;
    for (auto& desc : seq_annot.GetDesc().Get()) {
        if (!desc->IsUser())
            continue;
        auto& user = desc->GetUser();
        if (!(user.CanGetType() &&
            user.GetType().IsStr() &&
            user.GetType().GetStr() == kBAM_coverage &&
            user.CanGetData()))
            continue;
        if (!user.HasField(kSourceFile))
            continue;
        try {
            bam_data = user.GetField(kSourceFile).GetString();
            if (user.HasField(kIndexFile))
                bam_index = user.GetField(kIndexFile).GetString();
            if (user.HasField(kAssembly))
                assembly = user.GetField(kAssembly).GetString();
        } catch (exception& e) {
            ERR_POST(Error << e.what());
        }
    }
    if (bam_data.empty())
        return false;
    return true;
}

static void s_UpdateField(CUser_object& uo, const string& name, const string& value)
{
    if (uo.HasField(name))
        uo.SetField(name).SetValue(value);
    else if (!value.empty())
        uo.AddField(name, value);
}

void CBamUtils::UpdateCoverageGraphInfo(CSeq_annot& seq_annot, const string& bam_data, const string& bam_index, const string& assembly)
{
    _ASSERT(seq_annot.IsGraph());
    if (!seq_annot.IsGraph()) 
        return;
    CRef<CUser_object> uo;
    if (seq_annot.IsSetDesc()) {
        for (auto& desc : seq_annot.SetDesc().Set()) {
            if (!desc->IsUser())
                continue;
            auto& user = desc->SetUser();
            if (!(user.CanGetType() &&
                user.GetType().IsStr() &&
                user.GetType().GetStr() == kBAM_coverage &&
                user.CanGetData()))
                continue;
            uo.Reset(&user);
            break;
        }
    }
    if (!uo) {
        uo = Ref(new CUser_object);
        uo->SetType().SetStr(kBAM_coverage);
        seq_annot.AddUserObject(*uo);
    } 
    s_UpdateField(*uo, kSourceFile, bam_data);
    s_UpdateField(*uo, kIndexFile, bam_index);
    s_UpdateField(*uo, kAssembly, assembly);
}



END_NCBI_SCOPE
