#ifndef GUI_FRAMEWORK___VIEW_MANAGER__HPP
#define GUI_FRAMEWORK___VIEW_MANAGER__HPP

/*  $Id: view_manager_service.hpp 32743 2015-04-20 16:11:13Z katargir $
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
 *      IViewManagerService interface declaration.
 */

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/framework/view.hpp>

#include <gui/widgets/wx/wm_client.hpp>

#include <gui/utils/mru_list.hpp>


BEGIN_NCBI_SCOPE

class IWorkbench;

///////////////////////////////////////////////////////////////////////////////
/// IViewManagerService uses the following extension point to obtain
/// registered view factories (instances of IViewFactory)

#define EXT_POINT__VIEW_MANAGER_SERVICE__VIEW_FACTORY "view_manager_service::view_factory"

////////////////////////////////////////////////////////////////////////////////
/// IViewManagerService
/// IViewManagerService manages views in Workbench.

class IViewManagerService
{
public:
    typedef vector< CIRef<IView> >  TViews;
    typedef CMRUList<string>        TMRUViews;
    typedef vector< CIRef<IViewFactory> >       TFactories;
    typedef vector<const CViewTypeDescriptor*>  TDescrVec;

public:
    /// assumes ownership of the factory
    virtual void    RegisterFactory(IViewFactory& factory) = 0;

    virtual void    GetTypeDescriptors(TDescrVec& descrs) = 0;
    virtual void    GetFactories(TFactories& factories) = 0;

    /// create a view instance of the specified type
    virtual CIRef<IView>    CreateViewInstance(const string& type_ui_name) = 0;

    /// create a view instance of the specified type
    virtual CIRef<IView>
        CreateViewInstanceByFingerprint(const IWMClient::CFingerprint& fingerprint) = 0;

    virtual bool    CanCreateView(const string& type_ui_name) = 0;

    /// adds view to Workbench and connects to the services
    /// the view must be already initialized
    virtual void    AddToWorkbench(IView& view, bool bFloat = false) = 0;

    /// disconnects view from services and removes from the Workbench
    virtual void    RemoveFromWorkbench(IView& view) = 0;

    /// disconnects and removes all view from Workbench
    virtual void    RemoveAllViewsFromWorkbench() = 0;

    virtual bool    HasView(const IView& view) const = 0;

    /// get all registered views
    virtual void    GetViews(TViews& views) = 0;

    /// returns a pointer singleton view if it already exists in workbench or NULL
    virtual CIRef<IView>    GetSingletonView(const string& singleton_ui_name) = 0;

    /// activates the specified view, create a new view if it does not exist
    virtual CIRef<IView>    ShowSingletonView(const string& singleton_ui_name) = 0;

    virtual ~IViewManagerService() {};
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___VIEW_MANAGER__HPP

