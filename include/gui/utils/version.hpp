#ifndef GUI_CORE___VERSION__HPP
#define GUI_CORE___VERSION__HPP

/*  $Id: version.hpp 39865 2017-11-14 21:11:40Z katargir $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software / database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software / database is freely available
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


#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

BEGIN_NCBI_SCOPE

class NCBI_GUIUTILS_EXPORT CPluginVersion
{
public:
    enum EValues {
        eVersion_undefined = -1,

        ///< !!! DEPRECATED, OUT OF USE !!! 
        ///< Please use app/gbench/gbench_version.hpp instead.

        eMajor      = 2,   ///< absolute (in)compatibility
        eMinor      = 1,   ///< backward (in)compatibility
        ePatchLevel = 0    ///< patch level
    };

    //static bool CheckVersion(int major, int minor);
};

END_NCBI_SCOPE

#endif  // GUI_CORE___VERSION__HPP
