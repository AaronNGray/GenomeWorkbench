/*  $Id: feature_panel_ds.cpp 43691 2019-08-14 17:56:41Z katargir $
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
#include <gui/widgets/seq_graphic/feature_panel_ds.hpp>
#include <gui/widgets/seq_graphic/track_config_manager.hpp>

#include <gui/utils/app_job.hpp>
#include <gui/utils/app_job_impl.hpp>
#include <gui/objutils/na_utils.hpp>
#include <gui/objects/tms_utils.hpp>

#include <objtools/data_loaders/genbank/gbloader.hpp>
#include <corelib/ncbitime.hpp>
#include <corelib/perf_log.hpp>
#include <gui/utils/log_performance.hpp>

#include <gui/widgets/loaders/assembly_cache.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CTrackInitializationJob
///////////////////////////////////////////////////////////////////////////////
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CTrackInitializationJob :
        public CJobCancelable
{
public:
    typedef ILayoutTrackFactory::TAnnotMetaDataList TAnnotMetaDataList;

    CTrackInitializationJob(const string& desc, const ILayoutTrackFactory* factory,
        CTrackContainer* parent, CTempTrackProxy* params,
        const CObject& obj, objects::CScope& scope,
        ISGDataSourceContext* ds_context, CRenderingContext* r_cntx,
        const ILayoutTrackFactory::SExtraParams& extra_param,
        const TAnnotMetaDataList& src_annots);

    virtual ~CTrackInitializationJob(){};

    /// @name IAppJob implementation
    /// @{
    virtual EJobState                   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string                      GetDescr() const;
    /// @}

private:
    /// @name Job results and status.
    /// @{
    CRef<CAppJobError>          m_Error;
    CRef<CTrackInitJobResult>   m_Result;
    string                      m_Desc;
    /// @}

    CConstIRef<ILayoutTrackFactory>  m_Factory;
    SConstScopedObject          m_ScopedObj;         
    CIRef<ISGDataSourceContext> m_DSContext;
    CRenderingContext*          m_RenderingCntx;
    ILayoutTrackFactory::SExtraParams m_ExtraParam;
    TAnnotMetaDataList          m_SrcAnnots;
};


///////////////////////////////////////////////////////////////////////////////
/// CAnnotMetaDataJob
///////////////////////////////////////////////////////////////////////////////
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CAnnotMetaDataJob : public CJobCancelable
{
public:
    CAnnotMetaDataJob(const string& desc,
		              CBioseq_Handle handle,
		              const string& assembly,
		              const string& tms_context);

    virtual ~CAnnotMetaDataJob() {} 

    /// @name IAppJob implementation
    /// @{
    virtual EJobState                   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string                      GetDescr() const;
    /// @}

private:
    /// @name Job results and status.
    /// @{
    CRef<CAppJobError>              m_Error;
    CRef<CAnnotMetaDataJobResult>   m_Result;
    string                          m_Desc;
    /// @}

    objects::CBioseq_Handle m_Handle;           ///< target sequence
    string m_Assembly;
    string m_TMSContext;
};


///////////////////////////////////////////////////////////////////////////////
/// CGetAssembliesJob
///////////////////////////////////////////////////////////////////////////////
class CGetAssembliesJob : public CJobCancelable
{
public:
    CGetAssembliesJob(const string& seqAcc) : m_SeqAcc(seqAcc)
    {
        m_Result.Reset(new CGetAssembliesJobResult());
    }

    /// @name IAppJob implementation
    /// @{
    virtual EJobState Run()
    {
        while (!CAssemblyCache::GetInstance().GetAssemblies(m_SeqAcc, m_Result->m_Assemblies)) {
            if (IsCanceled())
                return eCanceled;
            SleepMilliSec(500);
        }
        return eCompleted;
    }
    virtual CConstIRef<IAppJobProgress> GetProgress() { return CConstIRef<IAppJobProgress>(NULL); }
    virtual CRef<CObject>               GetResult() { return CRef<CObject>(m_Result.GetPointer()); }
    virtual CConstIRef<IAppJobError>    GetError() { return CConstIRef<IAppJobError>(m_Error.GetPointer()); }
    virtual string                      GetDescr() const { return m_Desc; }
    /// @}

private:
    /// @name Job results and status.
    /// @{
    CRef<CAppJobError>              m_Error;
    CRef<CGetAssembliesJobResult>   m_Result;
    string                          m_Desc;
    /// @}

    string m_SeqAcc;
};


///////////////////////////////////////////////////////////////////////////////
/// CSeqGraphicJob implementation
///////////////////////////////////////////////////////////////////////////////
CTrackInitializationJob::CTrackInitializationJob(const string& desc,
    const ILayoutTrackFactory* factory, CTrackContainer* parent,
    CTempTrackProxy* params, const CObject& obj, objects::CScope& scope,
    ISGDataSourceContext* ds_context, CRenderingContext* r_cntx,
    const ILayoutTrackFactory::SExtraParams& extra_param,
    const TAnnotMetaDataList& src_annots)
    : m_Desc(desc)
    , m_Factory(factory)
    , m_ScopedObj(&obj, &scope)
    , m_DSContext(ds_context)
    , m_RenderingCntx(r_cntx)
    , m_ExtraParam(extra_param)
{
    m_SrcAnnots = src_annots;
    m_Result.Reset(new CTrackInitJobResult);
    m_Result->m_ParentTrack = parent;
    m_Result->m_TrackParams.Reset(params);
    m_Result->m_Desc = desc;
}

IAppJob::EJobState CTrackInitializationJob::Run()
{
    CLogPerformance perfLog("CTrackInitializationJob");
    perfLog << "Name: " << m_Result->m_TrackParams->GetName();
    perfLog << ", Source: " << m_Result->m_TrackParams->GetSource();
    perfLog << ", Category: " << m_Result->m_TrackParams->GetCategory();
    if (!m_Result->m_TrackParams->GetSubcategory().empty())
        perfLog << ", Subcategory: " << m_Result->m_TrackParams->GetSubcategory();

    m_Error.Reset();

    EJobState result = eFailed;
    try {
        // get a table of annot/track title correspondences
        for(auto& i : m_Result->m_TrackParams->GetChildren()) {
            if(CSeqUtils::IsNAA(i->GetSource()) && !i->GetDisplayName().empty()) {
                m_ExtraParam.m_AnnotNameTitleMap[i->GetSource()] = i->GetDisplayName();
            }
        }
        m_ExtraParam.m_TrackProfile = m_Result->m_TrackParams->GetTrackProfile();

        m_Result->m_Tracks =
            m_Factory->CreateTracks(m_ScopedObj, m_DSContext,
            m_RenderingCntx, m_ExtraParam, m_SrcAnnots);

        result = eCompleted;
    } NCBI_CATCH("CTrackInitializationJob::Run()");

    if (result == eFailed) {
        m_Error.Reset(new CAppJobError("failed on initializing " +
            m_Factory->GetThisTypeInfo().GetDescr()));
    }

    perfLog.Post(CRequestStatus::e200_Ok);
    //LOG_POST( Info << "CTrackInitializationJob::Run() completed job" );

    return result;
}

CConstIRef<IAppJobProgress> CTrackInitializationJob::GetProgress()
{
    return CConstIRef<IAppJobProgress>(NULL);
}

CRef<CObject> CTrackInitializationJob::GetResult()
{
    return CRef<CObject>(m_Result.GetPointer());
}

CConstIRef<IAppJobError> CTrackInitializationJob::GetError()
{
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}

string CTrackInitializationJob::GetDescr() const
{
    return m_Desc;
}



///////////////////////////////////////////////////////////////////////////////
/// CAnnotMetaDataJob implementation
///////////////////////////////////////////////////////////////////////////////
CAnnotMetaDataJob::CAnnotMetaDataJob(const string& desc,
                                     CBioseq_Handle handle,
                                     const string& assembly,
                                     const string& tms_context)
    : m_Desc(desc)
    , m_Handle(handle)
    , m_Assembly(assembly)
    , m_TMSContext(tms_context)
{
    m_Result.Reset(new CAnnotMetaDataJobResult);
}


IAppJob::EJobState CAnnotMetaDataJob::Run()
{
    m_Error.Reset();
    try {
        CConstRef<CSeq_id> seq_id = m_Handle.GetSeqId();
        string accession = seq_id->GetSeqIdString(true);

        vector<CRef<CAnnotMetaData> > tmsTracks;
        CStopWatch sw;
        sw.Start();
        CTMSUtils::GetTrackMetaData(tmsTracks, accession, m_Assembly, m_TMSContext);
        sw.Stop();
        string info = "Timing: " + sw.AsSmartString(CTimeSpan::eSSP_Millisecond)
            + ". TMS tracks via seqconfig.cgi for " + accession + ". Assembly " + m_Assembly + ".";
        LOG_POST(Info << info);

        LOG_POST(Info << "TMS tracks: " << tmsTracks.size() << ".");
        for (auto i : tmsTracks) {
            m_Result->m_Annots[i->m_Name] = i;
        }

        if (m_Result->m_Annots.empty()) {
            map<string, CRef<CAnnotMetaData> >  annots;
            if (IsCanceled()) return eCanceled;
            sw.Start();
            CNAUtils na_util(*seq_id, m_Handle.GetScope());
            na_util.GetAllNAMetaData(annots, "", x_GetICanceled());
            sw.Stop();
            if (IsCanceled()) return eCanceled;
            string info = "Timing: " + sw.AsSmartString(CTimeSpan::eSSP_Millisecond)
                + ". Eutils NA tracks for " + accession + ".";
            LOG_POST(Info << info);
            LOG_POST(Info << "EUtils tracks: " << annots.size() << ".");
            int count = 0;
            if (!m_Result->m_Annots.empty()) {
                for (auto& i : m_Result->m_Annots) {
                    if (annots.find(i.first) == annots.end()) {
                        ++count;
                        if (count <= 10) {
                            LOG_POST(Info << "TMS ann not found in EUtils: " << i.first);
                        }
                    }
                }
                LOG_POST(Info << "Total TMS annots not found in Eutils: " << count);
            }

            count = 0;
            for (auto& i : annots) {
                if (m_Result->m_Annots.find(i.first) == m_Result->m_Annots.end()) {
                    ++count;
                    m_Result->m_Annots[i.first] = i.second;

                    if (count <= 10) {
                        LOG_POST(Info << "EUtils ann not found in TMS: " << i.first);
                    }
                }
            }
            LOG_POST(Info << "Total Eutils annots not found in TMS: " << count);
        }

        LOG_POST(Info << "Time for retrieving NA meta-data: "
            << sw.Elapsed() << " seconds");

    } catch (std::exception& e) {
        string id_str;
        m_Handle.GetSeqId()->GetLabel(&id_str);
        LOG_POST( Info << "CAnnotMetaDataJob::Run() failed to get meta data for "
            <<  id_str << ", Details: "
            << e.what());

        // maybe there is no such thing as named annotation for
        // the given sequence type, such as a local sequence.
        // So we won't consider it as failed
    }

    return eCompleted;
}


CConstIRef<IAppJobProgress> CAnnotMetaDataJob::GetProgress()
{
    return CConstIRef<IAppJobProgress>(NULL);
}


CRef<CObject> CAnnotMetaDataJob::GetResult()
{
    return CRef<CObject>(m_Result.GetPointer());
}


CConstIRef<IAppJobError> CAnnotMetaDataJob::GetError()
{
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}


string CAnnotMetaDataJob::GetDescr() const
{
    return m_Desc;
}


///////////////////////////////////////////////////////////////////////////////
/// CFeaturePanelDS
///////////////////////////////////////////////////////////////////////////////

CFeaturePanelDS::CFeaturePanelDS(CScope& scope, const CSeq_id& id)
    : CSGGenBankDS(scope, id)
    , m_RenderingCntx(NULL)
	, m_UseEUtils(false)
    , m_MetaDataJob(-1)
{}


CFeaturePanelDS::~CFeaturePanelDS()
{
    DeleteMetaDataJob();
}

void CFeaturePanelDS::SetAssembly(const string& assembly)
{ 
    m_Assembly = assembly;
    DeleteMetaDataJob();
}

void CFeaturePanelDS::SetTMSContext(const string& tms_context)
{
    m_TMSContext = tms_context;
    DeleteMetaDataJob();
}

void CFeaturePanelDS::SetUseEUtils(bool use)
{
	m_UseEUtils = use;
	DeleteMetaDataJob();
}

bool CFeaturePanelDS::AllJobsFinished() const
{
    if (!CSGGenBankDS::AllJobsFinished())
        return false;
    return (m_MetaDataJob == -1);
}

void CFeaturePanelDS::DeleteMetaDataJob()
{
    if (m_MetaDataJob > -1) {
        CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
        try {
            disp.DeleteJob(m_MetaDataJob);
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
        m_MetaDataJob = -1;
    }
}


void CFeaturePanelDS::ClearMetaDataJobId()
{
    if (m_MetaDataJob > -1) {
        m_MetaDataJob = -1;
    }
}


void CFeaturePanelDS::CreateTrack(const string& desc,
                                  const ILayoutTrackFactory* factory,
                                  CTrackContainer* parent,
                                  CTempTrackProxy* params,
                                  const ILayoutTrackFactory::SExtraParams& extra_param)
{
    CConstRef<CSeq_id> seq_id = m_Handle.GetSeqId();

    CRef<CTrackInitializationJob> job(
        new CTrackInitializationJob(desc, factory, parent, params,
        *seq_id, GetScope(), m_DSContext, m_RenderingCntx,
        extra_param, TAnnotMetaDataList()));
    x_LaunchJob(*job, -1);
}


void CFeaturePanelDS::GetAnnotMetaData(const string& desc)
{
    CRef<CAnnotMetaDataJob> job(new CAnnotMetaDataJob(desc, m_Handle, m_Assembly, m_TMSContext));
    if (m_Background) {
        TJobID job_id = x_BackgroundJob(*job, -1, "ObjManagerEngine");
        if (job_id > -1) {
            m_MetaDataJob = job_id;
        }
    } else {
        x_ForegroundJob(*job);
    }
}


void CFeaturePanelDS::GetAssemblies(const string& seqAcc)
{
    CRef<CGetAssembliesJob> job(new CGetAssembliesJob(seqAcc));
    if (m_Background) {
        TJobID job_id = x_BackgroundJob(*job, -1, "ObjManagerEngine");
        if (job_id > -1) {
            m_MetaDataJob = job_id;
        }
    }
    else {
        x_ForegroundJob(*job);
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CSGFeatureDSType
///////////////////////////////////////////////////////////////////////////////

ISGDataSource*
CFeaturePanelDSType::CreateDS(SConstScopedObject& object) const
{
    const CSeq_id& id = dynamic_cast<const CSeq_id&>(object.object.GetObject());
    return new CFeaturePanelDS(object.scope.GetObject(), id);
}

string CFeaturePanelDSType::GetExtensionIdentifier() const
{
    static string sid("seqgraphic_feature_panel_ds_type");
    return sid;
}

string CFeaturePanelDSType::GetExtensionLabel() const
{
    static string slabel("Graphical View Feature Panel Data Source Type");
    return slabel;
}

bool CFeaturePanelDSType::IsSharable() const
{
    return false;
}


END_NCBI_SCOPE
