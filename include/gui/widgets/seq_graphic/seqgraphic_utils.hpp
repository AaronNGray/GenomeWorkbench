#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_UTILS__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_UTILS__HPP

/* $Id: seqgraphic_utils.hpp 42838 2019-04-19 17:24:56Z shkeda $
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
 * Author:  Liangshou Wu, Dmitry Rudnev
 *
 */

 /**
 * File Description:
 */

#include <corelib/ncbistr.hpp>
#include <corelib/ncbitime.hpp>
#include <corelib/ncbiobj.hpp>
#include <corelib/request_status.hpp>
#include <gui/utils/log_performance.hpp>
#include <gui/gui_export.h>


BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
class CSeq_annot_Handle;
class CSeq_id;
class CScope;
class CBioseq_Handle;
END_SCOPE(objects)

///////////////////////////////////////////////////////////////////////////////
/// Utility classes for sequence graphical view rendering.
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGUtils
{
public:
    static bool IsChromosome(const objects::CBioseq_Handle& handle,
        objects::CScope& scope);

    static bool IsSegSet(const objects::CBioseq_Handle& handle,
                         objects::CScope& scope);

    static bool IsmRNA(const objects::CBioseq_Handle& handle,
        objects::CScope& scope);

    static bool IsRefSeq(const objects::CBioseq_Handle& handle);
    static bool IsRefSeq(const string& seqid);
    static bool IsRefSeq(const objects::CSeq_id& seqid);

    static bool IsMitochondrion(const objects::CBioseq_Handle& handle);
    static bool IsPlastid(const objects::CBioseq_Handle& handle);

    static bool IsMainFeature(int type, int subtype);

    static bool GetFeatFetchDepth(const objects::CBioseq_Handle& handle,
        objects::CScope& scope, int& depth);

};


/////////////////////////////////////////////////////////////////////////////
/// Helper classes for reporting times.of various steps executed by the CGI
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CTimeReporterEntry: public CObject
{
public:
    CTimeReporterEntry(const string& sTimerName) 
        : m_sTimerName(sTimerName) 
        , m_TimeDelta(0.0) {}
    const string& GetTimerName() { return m_sTimerName; }
    CStopWatch& GetStopWatch() { return m_StopWatch; }

    // Add/change reported time value directly (instead of using start/stop)
    // Useful, for example, to sum up timers running in multiple threads.
    double GetTimeDelta() const { return m_TimeDelta; }
    void SetTimeDelta(double delta) { m_TimeDelta = delta; }

private:
    string m_sTimerName;
    CStopWatch m_StopWatch;
    double m_TimeDelta;
};

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CTimeReporter
{
public:

    typedef list<CRef<CTimeReporterEntry> > TTimeReporterEntries;
    CTimeReporter()
        {
        //ERR_POST("<<<< ctor, m_Timer: " << this);
        ResetAll();
        //ERR_POST(">>>> ctor, m_Timer: " << this);
        }

    virtual ~CTimeReporter() {
        //ERR_POST("==== destructor, m_Timer: " << this);
    }

    void Start(const string& sTimerName)
        {
            //ERR_POST("<<<< starting timer called " << sTimerName << ", m_Timer: " << this);
            x_FindOrCreate(sTimerName)->GetStopWatch().Start();
            //ERR_POST(">>>> started timer called " << sTimerName << ", m_Timer: " << this);
        }

    void Stop(const string& sTimerName)
        {
            //ERR_POST("<<<< stopping timer called " << sTimerName << ", m_Timer: " << this);
            x_FindOrCreate(sTimerName)->GetStopWatch().Stop();
            //ERR_POST(">>>> stopped timer called " << sTimerName << ", m_Timer: " << this);
        }

    double GetTime(const string& sTimerName) {
        NON_CONST_ITERATE(TTimeReporterEntries, iEntries, m_Entries) {
            if((*iEntries)->GetTimerName() == sTimerName) {
                double elapsed((*iEntries)->GetStopWatch().Elapsed() + 
                               (*iEntries)->GetTimeDelta());

                return elapsed;
            }
        }

        return 0.0;
    }

    // Add a specified value to the current timer
    void AddTime(const string& sTimerName, double add_time) {
        CRef<CTimeReporterEntry> entry = x_FindOrCreate(sTimerName);
        entry->SetTimeDelta(entry->GetTimeDelta() + add_time);
    }

    void ResetAll() {
        //ERR_POST("<<<< resetting all timers, m_Timer: " << this);
        m_Entries.clear();
        m_SW.Restart();
        //ERR_POST(">>>> done resetting all timers, m_Timer: " << this);
    }

    virtual void Write(CNcbiOstream& os)
        {
            os << fixed << setprecision(5);
            os << "\"Time\":{\n";
            // length of the JSON structure so the parsing script can know how many lines to take into account
            os << "  \"Number of lines\":" << m_Entries.size() + 4 << "," << endl;
            NON_CONST_ITERATE(TTimeReporterEntries, iEntries, m_Entries) {
                double Elapsed((*iEntries)->GetStopWatch().Elapsed() + 
                               (*iEntries)->GetTimeDelta());
                os << "  \"" << (*iEntries)->GetTimerName() << "\":" << Elapsed << "," << endl;
            }
            os << "  \"Total time\":" << m_SW.Elapsed() << "" << endl;
            os << "}";
        }
private:
    TTimeReporterEntries m_Entries;
    CStopWatch m_SW;

    CRef<CTimeReporterEntry> x_FindOrCreate(const string& sTimerName)
    {
        //ERR_POST("<<<< looking for timer called " << sTimerName << ", m_Timer: " << this);
        NON_CONST_ITERATE(TTimeReporterEntries, iEntries, m_Entries) {
            if((*iEntries)->GetTimerName() == sTimerName) {
                //ERR_POST(">>>> Timer found" << ", m_Timer: " << this);
                return *iEntries;
            }
        }
        CRef<CTimeReporterEntry> pNewEntry(new CTimeReporterEntry(sTimerName));
        m_Entries.push_back(pNewEntry);
        //ERR_POST(">>>> Timer created" << ", m_Timer: " << this);
        return pNewEntry;
    }
};


/// class combining functionality of CPerfLogGuard and a timer from CPerfLogGuard
/// with both log performance data to perf log and keep track of the timers for JSON reporting
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CTimerPerfLogGuard
{
public:
    CTimerPerfLogGuard(const string& sTimerName, CTimeReporter& timer, CPerfLogger::EStart state = CPerfLogger::eStart)
    : m_TimeReporter(timer)
    , m_TimerName(NStr::JsonEncode(sTimerName))
    , m_PerfLogGuard(m_TimerName, state) 
    { 
        m_TimeReporter.Start(m_TimerName); 
    }

    ~CTimerPerfLogGuard() { m_TimeReporter.Stop(m_TimerName); }

    void AddParameter(CTempString name, CTempString value) { m_PerfLogGuard.AddParameter(name, NStr::JsonEncode(value)); }
    void Stop(CRequestStatus::ECode status) { m_TimeReporter.Stop(m_TimerName); m_PerfLogGuard.Post(status); }
    void Suspend() { m_PerfLogGuard.Suspend(); m_TimeReporter.Stop(m_TimerName); }
    void Start() { m_PerfLogGuard.Start(); m_TimeReporter.Start(m_TimerName); }

private:
    CTimeReporter& m_TimeReporter;
    string m_TimerName;
    CLogPerformance m_PerfLogGuard;
};


#ifdef VERBOSE_PERFLOG
#define START_VERBOSE_PERFLOG(id, msg, reporter) CTimerPerfLogGuard perfLog##id((msg), reporter)
#define START_VERBOSE_PERFLOG_COUNTED(id, msg, reporter) static size_t perf_count##id(0); CTimerPerfLogGuard perfLog##id(string(msg) + " " + NStr::NumericToString(++perf_count##id), reporter)
#define ADD_VERBOSE_PERFLOG_DESC(id, desc) perfLog##id.AddParameter("description", (desc))
#define STOP_VERBOSE_PERFLOG(id) perfLog##id.Stop(CRequestStatus::e200_Ok)
#else
#define START_VERBOSE_PERFLOG(id, msg)
#define START_VERBOSE_PERFLOG_COUNTED(id, msg)
#define ADD_VERBOSE_PERFLOG_DESC(id, desc)
#define STOP_VERBOSE_PERFLOG(id)
#endif



END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_UTILS__HPP
