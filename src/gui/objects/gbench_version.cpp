
/*  $Id: gbench_version.cpp 38225 2017-04-12 14:24:56Z evgeniev $
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
 *
 */

#include <ncbi_pch.hpp>
#include <objects/general/Date.hpp>
#include <gui/objects/GBenchVersionInfo.hpp>

#include <gui/objects/gbench_version.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static bool s_GBenchVersionInit = CGBenchVersion::Init();

bool CGBenchVersion::Init()
{
    CGBenchVersionInfo::Initialize(
        CGBenchVersion::eMajor,
        CGBenchVersion::eMinor,
        CGBenchVersion::ePatchLevel
    );

    return true;
}

void GetGBenchVersionInfo(
    size_t& ver_major, size_t& ver_minor, size_t& ver_patch_level,
    CTime& build_date
){
    ver_major = CGBenchVersion::eMajor;
    ver_minor = CGBenchVersion::eMinor;
    ver_patch_level = CGBenchVersion::ePatchLevel;

    string date_str;
    date_str += __DATE__;
    date_str += " ";
    date_str += __TIME__;
    build_date = CTime( date_str, "b D Y h:m:s" );
}


void GetGBenchVersionInfo( CGBenchVersionInfo& version )
{
    size_t major = 0;
    size_t minor = 0;
    size_t patch = 0;
    CTime build_date;
    GetGBenchVersionInfo( major, minor, patch, build_date );

    version.SetVer_major(major);
    version.SetVer_minor(minor);
    version.SetVer_patch(patch);
    version.SetBuild_date().SetToTime(build_date);
}

END_NCBI_SCOPE
