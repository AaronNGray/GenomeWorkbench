#ifndef GUI_UTILS__BROWSER_UTILS_HPP
#define GUI_UTILS__BROWSER_UTILS_HPP

/*  $Id: browser_utils.hpp 18080 2008-10-14 22:33:27Z yazhuk $
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
 * Authors:  Josh Cherry
 *
 * File Description:
 *    Utilities for interacting with a web browser
 */

/** @addtogroup GUI_UTILS
*
* @{
*/

#include <corelib/ncbistd.hpp>

#include <gui/gui.hpp>


BEGIN_NCBI_SCOPE

class NCBI_GUIUTILS_EXPORT CBrowserUtils
{
public:
    /// Send html to a web browser.
    static void SendToBrowser(const string& html);
    /// Send anything to a web browser
    static void SendToBrowser(const string& body, const string& mime_type);
    /// Add a <Base> tag to html (in place)
    static void AddBaseTag(string& html, const string& url);
};


END_NCBI_SCOPE

/* @} */

#endif /// GUI_UTILS__BROWSER_UTILS_HPP
