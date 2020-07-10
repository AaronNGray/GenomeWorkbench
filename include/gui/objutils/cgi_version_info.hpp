#ifndef GUI_OBJUTILS___CGI_VERSION_INFO__HPP
#define GUI_OBJUTILS___CGI_VERSION_INFO__HPP

/*  $Id: cgi_version_info.hpp 38703 2017-06-10 01:06:33Z rudnev $
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
 * Author:  Vladislav Evgeniev
 *
 *
 */

/// @file cgi_version_info.hpp
/// Define CCgiVersionInfo, a CGI and WN version info storage class, extending CVersionInfo with revision number and build date.

#include <string>
#include <gui/gui_export.h>
#include <corelib/version.hpp>
#include <cgi/ncbicgi.hpp>
#include <cgi/ncbicgir.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CCgiVersionInfo
{
public:
    /// Constructor
    /// @param[in] version
    ///     Reference to a CVersionInfo.
    /// @param[in] revision
    ///     Revision number
    CCgiVersionInfo(const CVersionInfo &version, unsigned long revision = 0, const char *buildDate = __DATE__);

    virtual ~CCgiVersionInfo() {}

    /// Prints version information.
    /// @return
    ///   std::string representation of the version,
    ///   Version information is printed in the following forms:
    ///     - <ver_major>.<ver_minor>.<patch_level>.<revision> <build_date>
    ///     - <ver_major>.<ver_minor>.<patch_level>.<revision> <build_date> (<name>)
    ///   Return empty std::string if major version is undefined (< 0).
    std::string Print() const;

    /// Reports version information in JSON format
    /// @param[in] request
    ///     Reference to the CGI request.
    /// @param[in] response
    ///     Reference to the CGI response.
    /// @param[in] context
    ///     Pointer to the worker node job context (optional).
    /// @return
    ///     True if this is a version information request, false otherwise.
    virtual bool ReportVersion(const CCgiRequest& request, CCgiResponse& response);

    /// Returns the version information
    /// @return
    ///     Const reference to the version information.
    const CVersionInfo& GetVersionInfo() const;

    /// Sets the version information
    /// @param[in] revision
    ///     Const reference to the version information.
    void SetVersionInfo(const CVersionInfo &versionInfo);

    /// Returns the revision number
    /// @return
    ///     The revision number if set, 0 otherwise.
    unsigned long GetRevision () const;

    /// Sets the revision number
    /// @param[in] revision
    ///     Revision numer to set.
    void SetRevision (unsigned long revision);

    /// Returns the build date
    /// @return
    ///     Current build date
    const std::string & GetBuildDate () const;

    /// Delay test, simulating Grid framework delay
    /// @param[in] request
    ///     Reference to the CGI request.
    void TestDelay(const CCgiRequest& request);
protected:
    /// Version information
    CVersionInfo    m_VersionInfo;
    /// Revision number;
    unsigned long   m_Revision;
    /// Build date
    std::string     m_BuildDate;
};

END_NCBI_SCOPE

#endif  /// GUI_OBJUTILS___CGI_VERSION_INFO__HPP