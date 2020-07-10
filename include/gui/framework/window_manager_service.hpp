#ifndef GUI_FRAMEWORK___WINDOW_MANAGER_SERVICE__HPP
#define GUI_FRAMEWORK___WINDOW_MANAGER_SERVICE__HPP

/*  $Id: window_manager_service.hpp 35258 2016-04-18 01:35:40Z whlavina $
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

#include <wx/gdicmn.h>

class wxFrame;

BEGIN_NCBI_SCOPE

class IWMClient;
class IWMClientFactory;
class IWindowManagerAdvisor;
class CWindowManager;
class CDockLayoutTree;

BEGIN_SCOPE(objects)
    class CUser_object;
END_SCOPE(objects)

///////////////////////////////////////////////////////////////////////////////
/// IWindowManagerService
/// Window Manager Service provides access to Window Manager functionality.

class IWindowManagerService
{
public:
    typedef vector<IWMClient*>  TClients;

public:
    virtual CWindowManager* GetWindowManager() = 0;

    /// the service does not assume ownership of the given object
    virtual void    SetAdvisor(IWindowManagerAdvisor* advisor) = 0;

    /// places the given IWMClient in the Main Tabbed Pane in Window Manager
    virtual void    AddClient(IWMClient& client, bool bFloat) = 0;

    virtual wxFrame* MoveToFloatingFrame(IWMClient& client) = 0;

    /// remove client(s) from Window Manager
    /// these functions do not disconnect clients from other services
    virtual void    CloseClient(IWMClient& client) = 0;
    virtual void    CloseClients(const TClients& clients) = 0;
    virtual void    CloseAllClients() = 0;

    /// returns a list of all registered IWMClients
    virtual void    GetAllClients(TClients& clients) = 0;

    /// returns Active client (i.e. client that has focus)
    virtual IWMClient*  GetActiveClient() = 0;

    /// makes client visible and focused
    virtual void    ActivateClient(IWMClient& client) = 0;

    /// makes clients visible, make the first client in the given container
    /// focused
    virtual void    ActivateClients(TClients& clients) = 0;

    virtual void    RegisterActiveClient(IWMClient& client) = 0;
    virtual void    UnRegisterActiveClient(IWMClient& client) = 0;

    /// Raise all floating windows to preserve/refresh z-order
    virtual void    RaiseFloatingWindowsInZOrder() = 0;

    /// Raise all floating windows if they overlap with the provided wxREct
    virtual void    RaiseFloatingWindowsInZOrder(wxRect r) = 0;

    /// Returns true if a drag operation is currently in progress
    virtual bool    IsDragging() const = 0;

    /// refresh client UI (title)
    virtual void    RefreshClient(IWMClient& client) = 0;

    virtual void    ApplyLayout(const objects::CUser_object& layout) = 0;

    /// creates a new window layout based on description in the given
    /// CUser_object creates clients using IWMClientFactory
    virtual void    LoadLayout(const objects::CUser_object& layout,
                               IWMClientFactory& factory) = 0;

    /// saves current window layout to CUser_object
    virtual objects::CUser_object*  SaveLayout() = 0;

    virtual ~IWindowManagerService()    {};
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___WINDOW_MANAGER_SERVICE__HPP

