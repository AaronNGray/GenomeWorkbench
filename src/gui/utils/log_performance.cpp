#include <ncbi_pch.hpp>
#include <gui/utils/log_performance.hpp>
#include <corelib/ncbitime.hpp>

using namespace std;

BEGIN_NCBI_SCOPE

CLogPerformance::CLogPerformance(const string &resource, CPerfLogger::EStart state)
    : m_Resource(resource),
    m_IsPosted(false)
{
    if (CPerfLogger::IsON())
        m_PerfLog.reset(new CPerfLogger(state));
    else
        m_StopWatch.reset(new CStopWatch((CPerfLogger::eStart == state) ? CStopWatch::eStart : CStopWatch::eStop));
}

void CLogPerformance::Post(CRequestStatus::ECode status)
{
    flush();
    if (m_PerfLog.get()) {
        try {
            m_PerfLog->Post(status, m_Resource, str());
        }
        catch (const CCoreException&) {
        }
    }
    else {
        _ASSERT(m_StopWatch.get());
        m_StopWatch->Stop();

        if (m_StopWatch->Elapsed() > m_Threshold) {
            if (CRequestStatus::e200_Ok == status)
                LOG_POST(Info << m_Resource.c_str() << " took: " << m_StopWatch->AsSmartString(CTimeSpan::fSS_Precision6).c_str() << ' ' << str());
            else
                LOG_POST(Info << "(FAIL) " << m_Resource.c_str() << " took: " << m_StopWatch->AsSmartString(CTimeSpan::fSS_Precision6).c_str() << ' ' << str());
        }
    }
    m_IsPosted = true;
}

void CLogPerformance::Suspend()
{
    if (m_PerfLog.get()) {
        m_PerfLog->Suspend();
    }
    else {
        _ASSERT(m_StopWatch.get());
        m_StopWatch->Stop();
    }
}

void CLogPerformance::Start()
{
    if (m_PerfLog.get()) {
        m_PerfLog->Start();
    }
    else {
        _ASSERT(m_StopWatch.get());
        m_StopWatch->Start();
    }
}

void CLogPerformance::AddParameter(const std::string &name, const std::string &value)
{
    (*this) << ' ' << name << ": " << value << ";";
}

CLogPerformance::~CLogPerformance()
{
    if (!m_IsPosted)
        Post(CRequestStatus::e500_InternalServerError);
}

END_NCBI_SCOPE
