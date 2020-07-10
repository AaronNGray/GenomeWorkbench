#ifndef GUI_OBJUTILS___LOG_PERFORMANCE__HPP
#define GUI_OBJUTILS___LOG_PERFORMANCE__HPP

#include <sstream> 
#include <string>
#include <memory>
#include <corelib/ncbiobj.hpp>
#include <corelib/request_status.hpp>
#include <corelib/perf_log.hpp>
#include <gui/gui_export.h>

BEGIN_NCBI_SCOPE

class CStopWatch;

class NCBI_GUIOBJUTILS_EXPORT CLogPerformance :
    public std::stringstream
{
public:
    CLogPerformance(const std::string &resource, CPerfLogger::EStart state = CPerfLogger::eStart);
    void Post(CRequestStatus::ECode status = CRequestStatus::e200_Ok);
    void Suspend();
    void Start();
    void AddParameter(const std::string &name, const std::string &value);
    ~CLogPerformance();
private:
    double                  m_Threshold = 1.0;
    std::string             m_Resource;
    bool                    m_IsPosted;
    std::unique_ptr<CPerfLogger>   m_PerfLog;
    std::unique_ptr<CStopWatch>    m_StopWatch;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___LOG_PERFORMANCE__HPP
