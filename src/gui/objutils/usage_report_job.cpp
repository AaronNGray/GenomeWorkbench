/*  $Id: usage_report_job.cpp 41379 2018-07-19 18:44:04Z evgeniev $
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
 * Authors: Vladislav Evgeniev
 *
 */

#include <ncbi_pch.hpp>

#include <sstream>

#include <corelib/ncbidiag.hpp>

#include <connect/ncbi_http_session.hpp>

#include <gui/objutils/usage_report_job.hpp>
#include <gui/objects/gbench_version.hpp>
#include <gui/objutils/registry.hpp>

BEGIN_NCBI_SCOPE

USING_SCOPE(objects);

CReportParams& CReportParams::Add(const string& name, const string& value)
{
    
    m_Params[NStr::URLEncode(name, NStr::eUrlEnc_URIQueryName)] = NStr::URLEncode(value, NStr::eUrlEnc_URIQueryValue);
    return *this;
}

CReportParams& CReportParams::Add(const string& name, const char* value)
{
    return Add(name,string(value));
}

CReportParams& CReportParams::Add(const string& name, int value)
{
    return Add(name, NStr::IntToString(value));
}

CReportParams& CReportParams::Add(const string& name, unsigned int value)
{
    return Add(name, NStr::UIntToString(value));
}

CReportParams& CReportParams::Add(const string& name, long value)
{
    return Add(name, NStr::LongToString(value));
}

CReportParams& CReportParams::Add(const string& name, unsigned long value)
{
    return Add(name, NStr::ULongToString(value));
}

CReportParams& CReportParams::Add(const string& name, char value)
{
    return Add(name, string(1, value));
}

CReportParams& CReportParams::Add(const string& name, double value)

{
    return Add(name, NStr::DoubleToString(value));
}

CReportParams& CReportParams::Add(const string& name, bool value)

{
    return Add(name, NStr::BoolToString(value));
}

string CReportParams::ToString() const
{
    std::stringstream result;
    for (auto const &param : m_Params) {
        result << '&' << param.first << '=' << param.second;
    }
    return result.str();
}

///////////////////////////////////////////////////////////////////////////////
/// CUsageReportJob

static string s_GetUrl()
{
    return "https://www.ncbi.nlm.nih.gov/stat?ncbi_app=GenomeWorkbench2";
}

bool CUsageReportJob::m_IsEnabled = true;
volatile unsigned CUsageReportJob::m_FailedAttempts = 0;
std::mutex CUsageReportJob::m_VersionStringMutex;

CUsageReportJob::CUsageReportJob(const CReportParams& params)
    : m_Canceled(false)
    , m_URL(s_GetUrl())
{
    m_URL += params.ToString();
}

IAppJob::EJobState CUsageReportJob::Run()
{
    CRef<CHttpSession> session(new CHttpSession);
    CHttpResponse response = session->Get(m_URL);

    if (response.GetStatusCode() != 200) {
        m_FailedAttempts++;
        return IAppJob::eFailed;
    }
    m_FailedAttempts = 0;
    return IAppJob::eCompleted;
}

bool CUsageReportJob::IsEnabled()
{
    return (m_IsEnabled && (m_FailedAttempts < 10));
}

void CUsageReportJob::SetEnabled(bool enable)
{
    m_IsEnabled = enable;
}

void CUsageReportJob::ReportUsage(CReportParams& params)
{
    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance(); 
    params.Add("os", x_GetOS());
    params.Add("version", x_GetGbenchVersion());
    CIRef<IAppJob>  job(new CUsageReportJob(params));
    try {
        disp.StartJob(*job, "ThreadPool");
    }
    catch (const CException& e) {
        m_FailedAttempts++;
        LOG_POST(Warning << "CUsageReportJob::ReportUsage failed to start job");
        LOG_POST(Warning << e.ReportAll());
    }
}

static const char* kRegistryKey = "GBENCH.Application.ReportUsage.Enabled";

void CUsageReportJob::SaveSettings()
{
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    gui_reg.Set(kRegistryKey, m_IsEnabled);
}

void CUsageReportJob::LoadSettings()
{
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    m_IsEnabled = gui_reg.GetBool(kRegistryKey, true);
}

string CUsageReportJob::x_GetOS()
{
#if defined __WINDOWS__
    return string("Windows");
#elif defined __LINUX__
    return string("Linux");
#elif defined __WXOSX__
    return string("MacOSX");
#else
    return string("unknown");
#endif
}

string CUsageReportJob::x_GetGbenchVersion()
{
    std::lock_guard<std::mutex> guard(m_VersionStringMutex);
    static string ver_str;
    if (ver_str.empty()) {
        stringstream ver_strm;
        ver_strm << CGBenchVersion::eMajor << '.' << CGBenchVersion::eMinor << '.' << CGBenchVersion::ePatchLevel;
        ver_str = ver_strm.str();
    }
    return ver_str;
}

END_NCBI_SCOPE
