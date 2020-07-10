/*  $Id: version.cpp 39865 2017-11-14 21:11:40Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *    Versioning info for GBENCH
 */


#include <ncbi_pch.hpp>
#include <gui/utils/version.hpp>


BEGIN_NCBI_SCOPE

#if 0
bool CPluginVersion::CheckVersion(int ver_major, int ver_minor)
{
    //if (ver_major != eMajor  ||  ver_minor > eMinor) {
    // UNTIL version 1.0 - block all version mis-matches
    if (ver_major != eMajor  /*||  ver_minor != eMinor*/) {
        return false;
    }

    if (ver_minor != CPluginVersion::eMinor) {
        LOG_POST(Warning
                 << "CPluginVersion::CheckVersion(): minor version mismatch ("
                 << ver_major << "." << ver_minor
                 << " != " << int(CPluginVersion::eMajor)
                 << "." << int(CPluginVersion::eMinor));
    }

    return true;
}
#endif

END_NCBI_SCOPE
