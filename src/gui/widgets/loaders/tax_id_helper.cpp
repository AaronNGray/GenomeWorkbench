/*  $Id: tax_id_helper.cpp 40280 2018-01-19 17:54:35Z katargir $
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

#include <corelib/ncbi_system.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/loaders/tax_id_helper.hpp>
#include <gui/utils/app_job_impl.hpp>

#include <gui/objutils/taxon_cache.hpp>

#include <gui/widgets/loaders/winmask_files.hpp>
#include <objtools/blast/services/blast_services.hpp>

#include <wx/ctrlsub.h>

BEGIN_NCBI_SCOPE

BEGIN_EVENT_MAP(CTaxIdHelper, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged, &CTaxIdHelper::x_OnJobNotification)
END_EVENT_MAP()

CTaxIdHelper::CTaxIdHelper()
    : m_LocalTaxonsLoaded(false)
    , m_LocalJobId(CAppJobDispatcher::eInvalidJobID)
    , m_NetTaxonsLoaded(false)
    , m_NetJobId(CAppJobDispatcher::eInvalidJobID)
{
}

CTaxIdHelper::~CTaxIdHelper()
{
    x_CancelJobs();
}

CTaxIdHelper& CTaxIdHelper::GetInstance()
{
    static CRef<CTaxIdHelper> s_Storage;
    if (!s_Storage)
        s_Storage.Reset(new CTaxIdHelper());
    return *s_Storage;
}

void CTaxIdHelper::x_FillControl(wxControlWithItems& ctrl, const TTaxonVector& data, int selectTaxId)
{
    if (data.empty()) {
        ctrl.Append(wxT("none"), (void*)0);
        ctrl.SetSelection(0);
        return;
    }

    wxArrayString items;
    vector<void*> itemData;

    items.push_back(wxT("none"));
    itemData.push_back((void*)0);

    int selectIndex = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        if (data[i].m_Id == selectTaxId)
            selectIndex = i + 1;
        items.push_back(data[i].m_Label);
        itemData.push_back((void*)(intptr_t)data[i].m_Id);
    }
    ctrl.Append(items, &itemData[0]);
    ctrl.SetSelection(selectIndex);
}

void CTaxIdHelper::x_AddCallback(ICallback* callback)
{
    if (!callback) return;

    auto it = m_Callbacks.begin();
    for (; it != m_Callbacks.end(); ++it) {
        if (*it == callback)
            break;
    }
    if (it == m_Callbacks.end())
        m_Callbacks.push_back(callback);
}

void CTaxIdHelper::x_DeleteCallback(ICallback* callback)
{
    if (!callback) return;

    for (auto it = m_Callbacks.begin(); it != m_Callbacks.end();) {
        it = (*it == callback) ? m_Callbacks.erase(it) : ++it;
    }
}

void CTaxIdHelper::x_DoCallbacks(bool local)
{
    for (auto cb : m_Callbacks)
        cb->TaxonsLoaded(local);
}

namespace
{
    class CTaxonDataResult : public CObject
    {
    public:
        CTaxonDataResult() {}
        virtual ~CTaxonDataResult() {}
        CTaxIdHelper::TTaxonVector& SetData() { return m_Data; }
        const CTaxIdHelper::TTaxonVector& GetData() { return m_Data; }
    private:
        CTaxIdHelper::TTaxonVector m_Data;
    };

    class CLocalTaxonDataJob : public CJobCancelable
    {
    public:
        CLocalTaxonDataJob(bool local) : m_Local(local) {}
        virtual ~CLocalTaxonDataJob() {}

        /// @name IAppJob implementation
        /// @{
        virtual string                      GetDescr() const { return "Loading taxon data"; }
        virtual EJobState                   Run();
        virtual CConstIRef<IAppJobProgress> GetProgress() { return CConstIRef<IAppJobProgress>(); }
        virtual CRef<CObject>               GetResult() { return CRef<CObject>(m_Result.GetPointer()); }
        virtual CConstIRef<IAppJobError>    GetError() { return CConstIRef<IAppJobError>(m_Error.GetPointer()); }
        /// @}

    protected:
        virtual string x_GetJobName() const { return "TaxonDataJob"; }

    private:
        bool m_Local;
        CRef<CObject> m_Result;
        CRef<CAppJobError> m_Error;
    };
} // namespace


void CTaxIdHelper::x_OnJobNotification(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);
    if (!notn) return;

    CAppJobDispatcher::TJobID jobId = notn->GetJobID();
    CRef<CTaxonDataResult> result;

    switch (notn->GetState()) {
    case IAppJob::eCompleted:
        result.Reset(dynamic_cast<CTaxonDataResult*>(notn->GetResult().GetPointerOrNull()));
        if (result)
            x_JobCompleted(jobId, result->GetData());
        else
            x_JobFailed(jobId);
        break;
    case IAppJob::eFailed:
        x_JobFailed(jobId);
        break;
    case IAppJob::eCanceled:
        x_JobCanceled(jobId);
        break;
    default:
        break;
    }
}

void CTaxIdHelper::x_JobCompleted(CAppJobDispatcher::TJobID jobId, const TTaxonVector& result)
{
    bool local = true;
    if (jobId == m_LocalJobId) {
        m_LocalJobId = CAppJobDispatcher::eInvalidJobID;
    }
    else if (jobId == m_NetJobId) {
        m_NetJobId = CAppJobDispatcher::eInvalidJobID;
        local = false;
    }
    else
        return;

    bool& loaded = local ? m_LocalTaxonsLoaded : m_NetTaxonsLoaded;
    TTaxonVector& data = local ? m_LocalTaxons : m_NetTaxons;

    loaded = true;
    data = result;
    x_DoCallbacks(local);
}

void CTaxIdHelper::x_JobFailed(CAppJobDispatcher::TJobID jobId)
{
    if (jobId == m_LocalJobId) {
        m_LocalJobId = CAppJobDispatcher::eInvalidJobID;
        x_DoCallbacks(true);
    }
    else if (jobId == m_NetJobId) {
        m_NetJobId = CAppJobDispatcher::eInvalidJobID;
        x_DoCallbacks(false);
    }
}

void CTaxIdHelper::x_JobCanceled(CAppJobDispatcher::TJobID jobId)
{
    if (jobId == m_LocalJobId)
        m_LocalJobId = CAppJobDispatcher::eInvalidJobID;
    else if (jobId == m_NetJobId)
        m_NetJobId = CAppJobDispatcher::eInvalidJobID;
}

void CTaxIdHelper::x_CancelJobs()
{
    if (m_LocalJobId != CAppJobDispatcher::eInvalidJobID) {
        CAppJobDispatcher::GetInstance().DeleteJob(m_LocalJobId);
        m_LocalJobId = CAppJobDispatcher::eInvalidJobID;
    }

    if (m_NetJobId != CAppJobDispatcher::eInvalidJobID) {
        CAppJobDispatcher::GetInstance().DeleteJob(m_NetJobId);
        m_NetJobId = CAppJobDispatcher::eInvalidJobID;
    }
}

bool CTaxIdHelper::FillControl(ICallback* callback, wxControlWithItems& ctrl, bool local, int selectTaxId)
{
    bool& loaded = local ? m_LocalTaxonsLoaded : m_NetTaxonsLoaded;
    TTaxonVector& data = local ? m_LocalTaxons : m_NetTaxons;
    CAppJobDispatcher::TJobID& jobId = local ? m_LocalJobId : m_NetJobId;

    ctrl.Clear();
    x_AddCallback(callback);

    if (loaded) {
        x_FillControl(ctrl, data, selectTaxId);
        return true;
    }
    else {
        if (jobId == CAppJobDispatcher::eInvalidJobID) {
            jobId = CAppJobDispatcher::GetInstance().StartJob(*new CLocalTaxonDataJob(local), "ObjManagerEngine", *this, -1, true);
            if (jobId == CAppJobDispatcher::eInvalidJobID) {
                x_FillControl(ctrl, data, selectTaxId);
                loaded = true;
                return true;
            }
        }
        return false;
    }
}

void CTaxIdHelper::Reset(bool local)
{
    bool& loaded = local ? m_LocalTaxonsLoaded : m_NetTaxonsLoaded;
    CAppJobDispatcher::TJobID& jobId = local ? m_LocalJobId : m_NetJobId;

    bool reload = loaded;
    if (jobId != CAppJobDispatcher::eInvalidJobID) {
        CAppJobDispatcher::GetInstance().DeleteJob(jobId);
        jobId = CAppJobDispatcher::eInvalidJobID;
        reload = true;
    }

    loaded = false;

    if (reload) {
        jobId = CAppJobDispatcher::GetInstance().StartJob(*new CLocalTaxonDataJob(local), "ObjManagerEngine", *this, -1, true);
        if (jobId == CAppJobDispatcher::eInvalidJobID) {
            loaded = true;
            x_DoCallbacks(local);
        }
    }
}

CLocalTaxonDataJob::EJobState CLocalTaxonDataJob::Run()
{
    string errMsg;
    CRef<CTaxonDataResult> result(new CTaxonDataResult());
    m_Result.Reset(result.GetPointer());

    vector<int> ids;
    if (m_Local) {
        CWinMaskerFileStorage& storage = CWinMaskerFileStorage::GetInstance();
        while (storage.IsDownloading()) {
            if (IsCanceled())
                return eCanceled;
            SleepMilliSec(2000);
        }
        storage.GetTaxIds(ids);
    }
    else {
        try {
            CBlast4_get_windowmasked_taxids_reply::Tdata data =
                CBlastServices().GetTaxIdWithWindowMaskerSupport();
            ids.reserve(data.size());
            for_each(data.begin(), data.end(), [&ids](int id) {
                if (id != 0) ids.push_back(id);
            });
        } NCBI_CATCH("CBlastServices::GetTaxIdWithWindowMaskerSupport()");
    }
    if (IsCanceled()) return eCanceled;
    if (ids.empty()) return eCompleted;

    sort(ids.begin(), ids.end(), [](int i1, int i2) -> bool
    {
        if (i2 == 9606)  return false;
        if (i1 == 9606)  return true;
        if (i2 == 10090) return false;
        if (i1 == 10090) return true;
        return i1 < i2;
    });

    CTaxonCache& cache = CTaxonCache::GetInstance();
    cache.Initialize(ids);

    CTaxIdHelper::TTaxonVector& data = result->SetData();

    for (auto i : ids) {
        if (IsCanceled())
            return eCanceled;

        wxString label = wxString::Format(wxT("%6d"), i);
        string name = cache.GetLabel(i);

        if (!name.empty()) {
            label += wxT(" ") + ToWxString(name);
        }
        data.push_back(CTaxIdHelper::STaxonData(i, label));
    }

    // Close connection to taxon service. Action optional, but useful for better
    // resource utilization
    //
    cache.ResetConnection();

    return eCompleted;
}

END_NCBI_SCOPE
