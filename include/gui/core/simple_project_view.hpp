#ifndef GUI_CORE___SIMPLE_PROJECT_VIEW__HPP
#define GUI_CORE___SIMPLE_PROJECT_VIEW__HPP

/*  $Id: simple_project_view.hpp 36097 2016-08-09 19:16:52Z asztalos $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>

#include <gui/core/project_view_impl.hpp>
#include <gui/core/project_view_factory.hpp>

#include <gui/widgets/wx/gui_widget.hpp>

BEGIN_NCBI_SCOPE

/*
    ISimpleProjectViewCmdHandler interface is intended to extend functionality of CSimpleProjectView via commands handling.
    The commands will come from a widget(dialog).
    Derirve your command handler class from ISimpleProjectViewCmdHandler like this:

    class CMyCommandHandler : public CObject, public ISimpleProjectViewCmdHandler

    and pass it as a parameter to 
        CSimpleProjectView::CreateView(....., new CMyCommandHandler());

    A command handler class should be inmplemented at a level higher than gui_core (foor example at a package)
    as it uses IWorkbench interface and probably the GBench services.
*/

class IServiceLocator;

class ISimpleProjectViewCmdHandler
{
public:
    virtual ~ISimpleProjectViewCmdHandler() {}

    virtual void OnCommand(const string& command, IWorkbench* workbench, wxWindow* widget) = 0;
};

class NCBI_GUICORE_EXPORT CSimpleProjectView
    : public CProjectView, public IGuiWidgetHost
{
public:
    static CIRef<IProjectView> CreateView(const string& viewName,
                                         FWindowFactory widgetFactory,
                                         IServiceLocator* serviceLocator,
                                         TConstScopedObjects& objects,
                                         ISimpleProjectViewCmdHandler* cmdHandler = 0,
                                         bool bFloat = false);

    /// @name IGuiWidgetHost implementation
    /// @{
    virtual void CloseWidget();
    virtual void WidgetSelectionChanged();
    virtual void WidgetActivated();
    virtual void OnWidgetCommand(const string& command);
    virtual void ShowView(const string& viewName, FWindowFactory widgetFactory, TConstScopedObjects& objects, bool bFloat);
    /// @}

    /// @name IWMClient implementation
    /// @{
    virtual wxWindow* GetWindow();
    virtual bool IsADialog() const;
    /// @}

    /// @name IView implementation
    /// @(
    virtual const CViewTypeDescriptor&  GetTypeDescriptor() const;
    virtual void    CreateViewWindow(wxWindow* parent);
    virtual void    DestroyViewWindow();
    /// @}

    /// @name IProjectView implementation
    /// @{
    virtual bool InitView(TConstScopedObjects& objects, const objects::CUser_object* params);
    virtual void OnProjectChanged(CProjectViewEvent& evt);
    /// @}

    /// @name ISelectionClient Implementation
    /// @{
    virtual void GetSelection(CSelectionEvent& evt) const;
    virtual void GetSelection(TConstScopedObjects& objs) const;
    virtual void GetMainObject(TConstScopedObjects& objs) const;
    virtual void GetActiveObjects( TConstScopedObjects& objects ) const;
    virtual void GetActiveObjects(vector<TConstScopedObjects>& objects);
    /// @}

private:
    CSimpleProjectView(const string& viewName, FWindowFactory widgetFactory, ISimpleProjectViewCmdHandler* cmdHandler);

protected:
    /// @addtogroup CProjectViewBase overridables
    /// @{
    virtual const CObject* x_GetOrigObject() const;
    virtual void x_OnSetSelection(CSelectionEvent& evt);
    /// @}

    // upon closing, activate the Text View if found one
    void x_ActivateTextView(CProjectService& prjSrv);

    CProjectViewTypeDescriptor m_ViewDescr;

    FWindowFactory m_WidgetFactory;
    wxWindow*      m_Widget;

    CIRef<ISimpleProjectViewCmdHandler> m_CmdHandler;
};


END_NCBI_SCOPE

#endif // GUI_CORE___SIMPLE_PROJECT_VIEW__HPP
