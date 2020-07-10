#ifndef GUI_FRAMEWORK___TOOL_BAR_SERVICE__HPP
#define GUI_FRAMEWORK___TOOL_BAR_SERVICE__HPP

/*  $Id: tool_bar_service.hpp 35258 2016-04-18 01:35:40Z whlavina $
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

#include <gui/widgets/wx/ui_command.hpp>

class wxAuiToolBar;
class wxWindow;


BEGIN_NCBI_SCOPE

class IToolBarContributor;
class IToolBarContext;

///////////////////////////////////////////////////////////////////////////////
/// IToolBarService
class IToolBarService
{
public:
    /// the service assumes ownership of the given factory (will delete it)
    virtual void    AddToolBarFactory(IToolBarContributor* factory) = 0;

    /// forces deleting of the factory
    virtual void    DeleteToolBarFactory(IToolBarContributor* factory) = 0;

    /// register toolbar context
    /// the service does NOT assume ownership of the context
    virtual void    AddToolBarContext(IToolBarContext* context) = 0;

    /// unregister toolbar context
    virtual void    RemoveToolBarContext(IToolBarContext* context) = 0;

    virtual ~IToolBarService() {};
};

///////////////////////////////////////////////////////////////////////////////
/// IToolBarContext - represent a UI object that requires toolbars.
/// Context declares a set of toolbars with which it is compatible.
class IToolBarContext
{
public:
    /// returns the names of toolbars compatible with this class
    /// (toolbars that are relevant and useful in the context of this class)
    virtual void    GetCompatibleToolBars(vector<string>& names) = 0;

    virtual ~IToolBarContext()  {}
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___TOOL_BAR_SERVICE__HPP

