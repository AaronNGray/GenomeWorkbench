#ifndef GUI_UTILS___UTILS_MACOSX__HPP
#define GUI_UTILS___UTILS_MACOSX__HPP

/*  $Id: utils_macosx_.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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
 * Authors:  Vlad Lebedev
 *
 * File Description:
 *    Mac OS X specific parts of GUI.
 *    Do not include this file directly. Use utils_platform.hpp instead.
 */


#include <corelib/ncbistd.hpp>

/** @addtogroup GUI_UTILS
 *
 * @{
 */


BEGIN_NCBI_SCOPE


class NCBI_GUIUTILS_EXPORT CMacOSX
{
public:
    // Compile and execute AppleScript commands using OSA engine
    static bool ExecuteAppleScript(string script, string& result);
};


END_NCBI_SCOPE


/* @} */

#endif  // GUI_UTILS___UTILS_MACOSX__HPP