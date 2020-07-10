/*  $Id: cgi_version_info.cpp 43704 2019-08-16 16:58:46Z vasilche $
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
 *   Government have not placed any restriction on its use or reproduction.
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
 * Author:  Vladislav Evgeniev
 *
 * File Description: CCgiVersionInfo implementatoin
 *
 */

#include <ncbi_pch.hpp>
#include <sstream>
#include <corelib/ncbi_system.hpp>
#include <corelib/ncbireg.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbistre.hpp>
#include <connect/ncbi_socket.hpp>

#include <gui/objutils/cgi_version_info.hpp>

using namespace std;

BEGIN_NCBI_SCOPE

CCgiVersionInfo::CCgiVersionInfo(const CVersionInfo &version, unsigned long revision, const char *buildDate) : 
    m_VersionInfo(version),
    m_Revision(revision),
    m_BuildDate(buildDate)
{
}

string CCgiVersionInfo::Print(void) const
{
    if (m_VersionInfo.GetMajor() < 0) {
        return kEmptyStr;
    }
    CNcbiOstrstream os;
    os << m_VersionInfo.GetMajor() << "." << (m_VersionInfo.GetMinor() >= 0 ? m_VersionInfo.GetMinor() : 0) << "." << (m_VersionInfo.GetPatchLevel() >= 0 ? m_VersionInfo.GetPatchLevel() : 0) << "." << m_Revision;

    if ( !m_VersionInfo.GetName().empty() ) {
        os << " (" << m_VersionInfo.GetName() << ")";
    }
	
    os << ", " << m_BuildDate << ", " << CSocketAPI::gethostname();

    return CNcbiOstrstreamToString(os);
}

bool CCgiVersionInfo::ReportVersion(const CCgiRequest& request, CCgiResponse& response)
{
    if (request.GetEntry("version").empty())
        return false;

    stringstream rawOut;

    if (request.GetEntry("version") != "short") {
        rawOut  << "{\n"
                << "\t\"major\":" << ((m_VersionInfo.GetMajor()>=0) ? m_VersionInfo.GetMajor() : 0) << ",\n"
                << "\t\"minor\":" << ((m_VersionInfo.GetMinor()>=0) ? m_VersionInfo.GetMinor() : 0) << ",\n"
                << "\t\"patch\":" << ((m_VersionInfo.GetPatchLevel()>=0) ? m_VersionInfo.GetPatchLevel() : 0) << ",\n"
                << "\t\"revision\":" << m_Revision << ",\n"
                << "\t\"timestamp\":\"" << m_BuildDate << "\",\n"
                << "\t\"host\":\"" << CSocketAPI::gethostname() << "\"\n"
                << "}\n";
    }
    else {
        rawOut  << "{\n"
                << "\t\"versioninfo\":\"" << Print()  << "\"\n"
                << "}\n";
    }

    bool hasCallback = !request.GetEntry("callback").empty();

    CNcbiOstream &out = response.out();

    if (hasCallback) 
        response.SetContentType("text/javascript");
    else 
        response.SetContentType("text/plain");

    response.WriteHeader();
    out << rawOut.str();

    return true;
}

const CVersionInfo& CCgiVersionInfo::GetVersionInfo() const
{
    return m_VersionInfo;
}

void CCgiVersionInfo::SetVersionInfo(const CVersionInfo &versionInfo)
{
    m_VersionInfo = versionInfo;
}

unsigned long CCgiVersionInfo::GetRevision () const
{
    return m_Revision;
}

void CCgiVersionInfo::SetRevision (unsigned long revision)
{
    m_Revision = revision;
}

const string & CCgiVersionInfo::GetBuildDate () const
{
    return m_BuildDate;
}

void CCgiVersionInfo::TestDelay(const CCgiRequest& request)
{
    try {
        if (request.GetEntry("delaytest").empty())
            return;

        const CNcbiRegistry& reg(CNcbiApplication::Instance()->GetConfig());

        // get whether delay is allowed
        int max_delaytest = reg.GetInt("debug", "max_delaytest", 0);

        if (max_delaytest == 0)
            return;
    
        int delay_sec = NStr::StringToInt(request.GetEntry("delaytest"));

        SleepSec(delay_sec < max_delaytest ? delay_sec : max_delaytest);
    }
    catch(...) {
        // Ignore the potential numeric conversion exceptions, just don't do a delay
    }
}

END_NCBI_SCOPE
