#ifndef GUI_FRAMEWORK___MENU_SERVICE_HPP
#define GUI_FRAMEWORK___MENU_SERVICE_HPP

/*  $Id: menu_service.hpp 26901 2012-11-23 11:38:19Z voronov $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbistd.hpp>

class wxMenu;

BEGIN_NCBI_SCOPE

class IMenuContributor;

///////////////////////////////////////////////////////////////////////////////
/// IMenuService - Menu Service
class IMenuService
{
public:
    virtual void ResetMenuBar() = 0;

    /// the service does NOT assume ownership of the contributor
    virtual void AddContributor( IMenuContributor* contributor ) = 0;
    virtual void RemoveContributor( IMenuContributor* contributor ) = 0;

    /// add commands for processing (simulates choosing the command from a
    /// menu)
    virtual void AddPendingCommand( int cmd ) = 0;

    virtual ~IMenuService() {}
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___MENU_SERVICE_HPP

