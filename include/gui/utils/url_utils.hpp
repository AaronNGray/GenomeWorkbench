#ifndef GUI_UTILS__URL_UTILS_HPP
#define GUI_UTILS__URL_UTILS_HPP

/*  
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
 * Authors:  Andrei Shkeda
 *
 * File Description:
 *    URL Utilities
 */

/** @addtogroup GUI_UTILS
*
* @{
*/

#include <corelib/ncbistd.hpp>

#include <gui/gui.hpp>


BEGIN_NCBI_SCOPE

class NCBI_GUIUTILS_EXPORT CUrlUtils
{
public:
    /// HTTP: Returns header's Last-Modified in the last_modified parameter
    /// FTP: Returns MD5 of first 512 bytes in the last_modified parameter
    static void GetLastModified(const string& url, string& last_modified);

    /// Works with HTTP(s) protocol 
    /// returns true if url was modified since last_modified date
    /// if true last_modified will be updated to new last modified date
    static bool IfModifiedSince(const string& url, string& last_modified);
};


END_NCBI_SCOPE

/* @} */

#endif /// GUI_UTILS__URL_UTILS_HPP
