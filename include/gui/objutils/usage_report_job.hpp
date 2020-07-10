#ifndef GUI_UTILS__USAGE_REPORT_JOB__HPP
#define GUI_UTILS__USAGE_REPORT_JOB__HPP

/*  $Id: usage_report_job.hpp 41347 2018-07-12 18:28:31Z evgeniev $
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
 * File Description: Defines a job and a macro to log usage information to applog
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbimtx.hpp>
#include <corelib/ncbicntr.hpp>
#include <connect/ncbi_conn_stream.hpp>

#include <gui/gui_export.h>
#include <gui/utils/app_job_impl.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <mutex>


BEGIN_NCBI_SCOPE

/// Temporary object for holding extra report arguments
class NCBI_GUIOBJUTILS_EXPORT CReportParams
{
public:
    // Name must contain only alphanumeric chars or '_'.
    // Value is URL-encoded before printing.
    CReportParams& Add(const string& name, const string& value);
    CReportParams& Add(const string& name, const char* value);
    CReportParams& Add(const string& name, int value);
    CReportParams& Add(const string& name, unsigned int value);
    CReportParams& Add(const string& name, long value);
    CReportParams& Add(const string& name, unsigned long value);
    CReportParams& Add(const string& name, char value);
    CReportParams& Add(const string& name, double value);
    CReportParams& Add(const string& name, bool value);
    string ToString() const;
private:
    typedef std::map<string, string> TStringMap;
    TStringMap m_Params;
};


/// Job to log usage information to applog through stat
///
class NCBI_GUIOBJUTILS_EXPORT CUsageReportJob : public CObject, public IAppJob
{
public:
    CUsageReportJob(const CReportParams& params);

    /// @name IAppJob implementation
    /// @{
    virtual EJobState                   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress() { return CConstIRef<IAppJobProgress>(); }
    virtual CRef<CObject>               GetResult() { return CRef<CObject>(); }
    virtual CConstIRef<IAppJobError>    GetError() { return CConstIRef<IAppJobError>(); }
    virtual string                      GetDescr() const { return NcbiEmptyString; }
    virtual void                        RequestCancel() { m_Canceled = true; }
    virtual bool                        IsCanceled() const { return m_Canceled; }
    /// @}

    /// Indicates whether usage statistics collection is enabled
    static bool IsEnabled();

    /// Switch usage statistics collection on/off 
    static void SetEnabled(bool enable = true);

    static void SaveSettings();
    static void LoadSettings();

    static void ReportUsage(CReportParams& params);

private:
    static string x_GetOS();
    static string x_GetGbenchVersion();

private:
    bool    m_Canceled;
    string  m_URL;
    static bool                 m_IsEnabled;
    static volatile unsigned    m_FailedAttempts;
    static std::mutex m_VersionStringMutex;      ///< version string access mutex
};

/////////////////////////////////////////////////////////////////////////////
/// Convenience macro to log usage statisitics
///
/// @par Usage example:
/// This example demonstrates how to log usage
/// @code
/// REPORT_USAGE("tools",
///               .Add("tool_name", "XYZ")
///               .Add("tool_version", 2) );
/// @endcode
#define REPORT_USAGE(event, args)                                       \
    {                                                                   \
        do {                                                            \
            if (!CUsageReportJob::IsEnabled())                          \
                break;                                                  \
            CReportParams params;                                       \
            params.Add("jsevent", (event)) args;                        \
            CUsageReportJob::ReportUsage(params);                       \
        }                                                               \
        while (false);                                                  \
    }

END_NCBI_SCOPE

#endif // GUI_UTILS__USAGE_REPORT_JOB__HPP
