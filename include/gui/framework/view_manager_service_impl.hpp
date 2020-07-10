#ifndef GUI_FRAMEWORK___VIEW_MANAGER_IMPL__HPP
#define GUI_FRAMEWORK___VIEW_MANAGER_IMPL__HPP

/*  $Id: view_manager_service_impl.hpp 32743 2015-04-20 16:11:13Z katargir $
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

#include <gui/framework/view_manager_service.hpp>
#include <gui/framework/service.hpp>

#include <gui/objutils/user_type.hpp>
#include <gui/objutils/reg_settings.hpp>

#include <gui/widgets/wx/wm_client.hpp>

#include <set>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CViewManagerService - the standard implementation of IViewManagerService
/// interface.
///
/// This class implements IWMClientFactory so that it can be used with
/// Window Manager to instantiate view for loaded window layouts.

class   NCBI_GUIFRAMEWORK_EXPORT  CViewManagerService :
    public CObject,
    public IViewManagerService,
    public IService,
    public IWMClientFactory,
    public IRegSettings
{
public:
    CViewManagerService();
    virtual ~CViewManagerService();

    /// connect / disconnect to / from Workbench
    virtual void    SetWorkbench(IWorkbench* workbench);

    /// @name IService interface implementation
    /// @{
    virtual void    InitService();
    virtual void    ShutDownService();
    /// @}

    /// @name IViewManagerService implementation
    /// @{
    virtual void    RegisterFactory(IViewFactory& factory);

    virtual void    GetTypeDescriptors(TDescrVec& descrs);
    virtual void    GetFactories(TFactories& factories);

    virtual CIRef<IView>    CreateViewInstance(const string& type_ui_name);
    virtual CIRef<IView>
           CreateViewInstanceByFingerprint(const TFingerprint& fingerprint);

    virtual bool    CanCreateView(const string& type_ui_name);

    virtual void    AddToWorkbench(IView& view, bool bFloat);
    virtual void    RemoveFromWorkbench(IView& view);
    virtual void    RemoveAllViewsFromWorkbench();

    virtual bool    HasView(const IView& view) const;
    virtual void    GetViews(TViews& views);

    virtual CIRef<IView>    GetSingletonView(const string& singleton_ui_name);

    virtual CIRef<IView>    ShowSingletonView(const string& singleton_ui_name);
    /// @}

    /// @name IWMClientFactory interface implementation
    /// @{
    virtual IWMClient*  CreateClient(const TFingerprint& fingerprint, wxWindow* parent);
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

protected:
    void    x_RegisterExtensionFactories();
    string  x_GetViewRegistryPath(IView& view);

    void    x_DoAddToWorkbench(IView& view, wxWindow* parent);
    void    x_DoRemoveViewFromWorkbench(IView& view);

    void    x_LoadViewSettings(IView& view);
    void    x_SaveViewSettings(IView& view);

protected:
    typedef map<string, CIRef<IViewFactory> > TViewNameToFactory;
    typedef map<string, CIRef<IView> > TSingletonMap;

    IWorkbench* m_Workbench;

    TViewNameToFactory  m_ViewNameToFactory;
    TViews              m_Views; // a list of all registered views
    TSingletonMap       m_SingletonMap; // map of existing Singleton Views

    string  m_RegPath;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___VIEW_MANAGER_IMPL__HPP

