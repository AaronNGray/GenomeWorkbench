#ifndef GUI_OBJECTS___GBENCH_VERSION__HPP
#define GUI_OBJECTS___GBENCH_VERSION__HPP

/*  $Id: gbench_version.hpp 45103 2020-06-01 14:57:28Z evgeniev $
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

#include <corelib/ncbistd.hpp>
#include <corelib/ncbitime.hpp>

#include <gui/gui_export.h>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CGBenchVersionInfo;
END_SCOPE(objects)

class NCBI_GUIOBJECTS_EXPORT CGBenchVersion
{
public:
    enum EValues {
        eVersion_undefined = -1,

        eMajor      = 3,   ///< absolute (in)compatibility
        eMinor      = 4,  ///< backward (in)compatibility
        ePatchLevel = 1    ///< patch level
    };

    static bool Init();
};

void NCBI_GUIOBJECTS_EXPORT GetGBenchVersionInfo(
    size_t& ver_major, size_t& ver_minor, size_t& ver_patch_level,
    CTime& build_date
);

void NCBI_GUIOBJECTS_EXPORT GetGBenchVersionInfo(objects::CGBenchVersionInfo& version);

END_NCBI_SCOPE

#endif  // GUI_OBJECTS___GBENCH_VERSION__HPP
