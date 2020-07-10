/*  $Id: prefetch_seq_descr.cpp 43872 2019-09-12 17:44:40Z shkeda $
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
 * Authors:  Anatoliy Kuznetsov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/core/prefetch_seq_descr.hpp>

#include <gui/framework/app_task_service.hpp>
#include <gui/framework/app_job_task.hpp>

#include <objects/gbproj/ProjectItem.hpp>

#include <gui/objutils/obj_convert.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/utils.hpp>

#include <gui/utils/app_job_impl.hpp>

#include <objmgr/object_manager.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CSeqDescrRetrievalJob - 
/// class.
class CSeqDescrRetrievalJob : public CAppJob
{
public:
    CSeqDescrRetrievalJob(const TConstObjects& objs)
        : CAppJob("Retrieving sequence descriptions")
        , m_Objects(objs)
    {}

    /// @name IAppJob implementation
    /// @{
    virtual EJobState   Run();
    /// @}

private:
    TConstObjects m_Objects;
};


IAppJob::EJobState CSeqDescrRetrievalJob::Run()
{
    typedef vector< CConstRef<CSeq_id> > TSeqIds;

    TSeqIds seq_ids;
    const string target_type = "Seq-id";

    size_t total_ids = 0;
    CStopWatch sw;
    sw.Start();

    CScope scope(*CObjectManager::GetInstance());
    scope.AddDefaults();

    map<TGi, string> gi_descr_map;
    ITERATE(TConstObjects, obj_iter, m_Objects) {
        const CObject* obj = obj_iter->GetPointer();
        const CSeq_id* seq_id = dynamic_cast<const CSeq_id*>(obj);
        if (seq_id) {
            if (!CSeqUtils::IsVDBAccession(seq_id->GetSeqIdString())) {
                seq_ids.emplace_back(seq_id);
                 ++total_ids;
            }
        }
        else if (obj) {
            x_SetStatusText("Extracing seq-ids from the project item");
            CObjectConverter::TRelationVector relations;
            CObjectConverter::FindRelations(scope, *obj, target_type, relations);

            NON_CONST_ITERATE(CObjectConverter::TRelationVector, itr, relations) {
                if (IsCanceled()) {
                    m_Objects.clear(); // clear the array while in the background
                    return eCanceled;
                }

                /// If there are more than one relation, the first one contains
                /// all the conversions, and the rest contain categorized 
                /// conversions.
                if (relations.size() > 1 && itr == relations.begin()){
                    continue;
                }

                CRelation& rel = **itr;
                string relName = rel.GetProperty(target_type);

                CRelation::TObjects related;
                rel.GetRelated(scope, *obj, related, CRelation::fConvert_NoExpensive, x_GetICanceled());

                ITERATE(CRelation::TObjects, rlt_iter, related) {
                    if (IsCanceled()) {
                        m_Objects.clear();
                        return eCanceled;
                    }

                    const CSeq_id* seq_id = dynamic_cast<const CSeq_id*>(rlt_iter->GetObjectPtr());
                    if (seq_id) {
                        if (!CSeqUtils::IsVDBAccession(seq_id->GetSeqIdString())) {
                            seq_ids.emplace_back(seq_id);
                            ++total_ids;
                        }
                    }
                }
            }
        }
    }

    _TRACE("Get sequence title, extract seq-ids time: " << sw.Elapsed());
    sw.Restart();

    // clear the array while in the background, save destructor time
    m_Objects.clear();

    if (total_ids > 0) {

        sort(seq_ids.begin(), seq_ids.end(), [](const CConstRef<CSeq_id>& a, const CConstRef<CSeq_id>& b) {
            return a->GetSeqIdString() < b->GetSeqIdString();
        });

        seq_ids.erase(unique(seq_ids.begin(), seq_ids.end(), 
            [](CConstRef<CSeq_id>& a, CConstRef<CSeq_id>& b) {
            return a->GetSeqIdString() == b->GetSeqIdString();
        }), seq_ids.end());

        total_ids = seq_ids.size();
        size_t i = 1;
        ITERATE (TSeqIds, id_iter, seq_ids) {
            if (IsCanceled()) {
                m_Objects.clear();
                return eCanceled;
            }
            CLabelDescriptionCache::CacheSeqId(**id_iter, &scope);
            ++i;
            if ((i % 50) == 0) {
                string msg = "Retrieving descriptions: ";
                msg += NStr::SizetToString(i);
                msg += " of " + NStr::SizetToString(total_ids);
                x_SetStatusText(msg);
            }
            if ((i % 100) == 0) {
                scope.ResetDataAndHistory();
            }
        }
    }

    CLabelDescriptionCache::AddCaches(gi_descr_map);
    _TRACE(Info << "Get sequence title, get description total time: " << sw.Elapsed());

    return eCompleted;
}

void CPrefetchSeqDescr::PrefetchSeqDescr(IServiceLocator* serviceLocator, const vector<CRef<CProjectItem> >& items)
{
    TConstObjects objects;
    ITERATE(vector<CRef<CProjectItem> >, it, items) {
        const CSerialObject* so = dynamic_cast<const CSerialObject*>((*it)->GetObject());
        if (!so) continue;
        CSerialObject* copy = (CSerialObject*)so->GetThisTypeInfo()->Create();
        CConstRef<CObject> clone(copy);
        if (!clone) continue;
        copy->Assign(*so);
        objects.push_back(clone);
    }

    if (objects.empty())
        return;

    // prefetch sequence description for performance consideration
    CAppTaskService* task_srv = serviceLocator->GetServiceByType<CAppTaskService>();
    CRef<CSeqDescrRetrievalJob> job(new CSeqDescrRetrievalJob(objects));
    CRef<CAppJobTask> task(new CAppJobTask(*job, false,
        "Retrieving sequence descriptions", 5, "ObjManagerEngine", 2 /*view display delay*/));
    task_srv->AddTask(*task);
}

END_NCBI_SCOPE
