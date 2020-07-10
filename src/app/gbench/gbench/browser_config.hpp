#ifndef GUI_GBENCH___BROWSER_CONFIG__HPP
#define GUI_GBENCH___BROWSER_CONFIG__HPP

/*  $Id: browser_config.hpp 16601 2008-04-04 17:45:35Z katargir $
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
 * File Description:  Configure things so that gbench MIME types
 *                    launch this copy of gbench
 *
 */

#include <corelib/ncbistd.hpp>

BEGIN_NCBI_SCOPE


/// Configure things so that gbench MIME types
/// launch this copy of gbench

/// On Windows there are two parameters: whether to configure for
/// entire machine (set registry variables in root key
/// HKEY_LOCAL_MACHINE rather than HKEY_CURRENT_USER),
/// and whether to use the -remote rather than -noremote
/// command line argument for gbench.
void GBenchBrowserConfig(bool whole_machine, bool remote = true);


END_NCBI_SCOPE

#endif  // GUI_GBENCH___BROWSER_CONFIG__HPP
