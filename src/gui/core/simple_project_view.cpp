/*  $Id: simple_project_view.cpp 38781 2017-06-16 16:43:17Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include <gui/core/simple_project_view.hpp>
#include <gui/framework/workbench.hpp>
#include <gui/framework/view_manager_service.hpp>
#include <gui/framework/window_manager_service.hpp>
#include <gui/widgets/wx/iwindow_manager.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/objects/GBWorkspace.hpp>

#include <gui/objutils/obj_event.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/window_manager.hpp>

#include <wx/msgdlg.h>
#include <wx/dialog.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CIRef<IProjectView> CSimpleProjectView::CreateView(const string& viewName,
                                        FWindowFactory widgetFactory,
                                        IServiceLocator* serviceLocator,
                                        TConstScopedObjects& objects,
                                        ISimpleProjectViewCmdHandler* cmdHandler,
                                        bool bFloat)
{
    CIRef<IProjectView> view(new CSimpleProjectView(viewName, widgetFactory, cmdHandler));
    if (objects.size() == 1) {
        const CSerialObject* so = dynamic_cast<const CSerialObject*>(objects[0].object.GetPointerOrNull());
        if (so) {
            string fp = viewName + ":" + so->GetThisTypeInfo()->GetName();
            dynamic_cast<IWMClient&>(*view).SetFingerprint(fp);
        }
    }


    IViewManagerService* view_srv = serviceLocator->GetServiceByType<IViewManagerService>();
    _ASSERT(view_srv);
    view_srv->AddToWorkbench(*view, bFloat);

    if (!view->InitView(objects, 0)) {
        view_srv->RemoveFromWorkbench(*view);
        view.Reset();
        return view;
    }

    CIRef<CProjectService> prj_srv = serviceLocator->GetServiceByType<CProjectService>();
    prj_srv->OnViewAttached(view);

    CProjectViewBase* prj_view_base = dynamic_cast<CProjectViewBase*>(view.GetPointer());
    if (prj_view_base)
        prj_view_base->RefreshViewWindow();

    return view;
}

CSimpleProjectView::CSimpleProjectView(const string& viewName,
                                       FWindowFactory widgetFactory,
                                       ISimpleProjectViewCmdHandler* cmdHandler)
    : m_ViewDescr(
        viewName, 
        "simple_project_view",
        NcbiEmptyString, NcbiEmptyString, NcbiEmptyString,
        "Generic",
        false,
        "SerialObject",
        eAnyObjectsAccepted),
     m_WidgetFactory(widgetFactory),
     m_Widget(),
     m_CmdHandler(cmdHandler)
{
}

void CSimpleProjectView::x_ActivateTextView(CProjectService& prjSrv)
{
    // get default top level seq-entry
    CSeq_entry_Handle seh;
    CScope& scope = *x_GetScope();
    CScope::TTSE_Handles handles;
    scope.GetAllTSEs(handles);
    if (handles.size() > 0) {
        seh = handles.front().GetTopLevelEntry();
    }

    if (!seh) return;

    // look for the Text View having this main object
    CConstRef<CSeq_entry> entry = seh.GetCompleteSeq_entry();
    CIRef<IProjectView> pTextView(prjSrv.FindView(entry.GetObject(), "Text View"));
    if (pTextView) {
        prjSrv.ActivateProjectView(pTextView);
    }
    else {
        // for Text View whose main object is not a seq-entry
        // look for one within the scope, and activate it if there is only one
        vector<CIRef<IProjectView>> views;
        prjSrv.FindViews(views);
        auto count = 0;
        ITERATE(vector<CIRef<IProjectView> >, it, views) {
            CIRef<IProjectView> view = *it;
            if (&*view->GetScope() == &scope
                && view->GetLabel(IProjectView::eType) == "Text View") {
                pTextView = view;
                count++;
            }
        }
        if (pTextView && count == 1) {
            prjSrv.ActivateProjectView(pTextView);
        }
    }

}

/// IGuiWidgetHost implementation

void CSimpleProjectView::CloseWidget()
{
    CProjectService* prjSrv = m_Workbench->GetServiceByType<CProjectService>();
    if (prjSrv) {
        x_ActivateTextView(*prjSrv);
        prjSrv->RemoveProjectView(*this);
    }
}

void CSimpleProjectView::WidgetSelectionChanged()
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged);
    Send(&evt, ePool_Parent);
}

void CSimpleProjectView::WidgetActivated()
{
    if (dynamic_cast<wxDialog*>(m_Widget) == 0) return;

    CIRef<IWindowManagerService> srv = m_Workbench->GetServiceByType<IWindowManagerService>();
    CWindowManager* windowManager = srv ? srv->GetWindowManager() : 0;
    if (windowManager) {
        windowManager->OnFocusChanged(m_Widget);
    }
}

void CSimpleProjectView::OnWidgetCommand(const string& command)
{
    if (m_CmdHandler)
        m_CmdHandler->OnCommand(command, m_Workbench, m_Widget);
}

void CSimpleProjectView::ShowView(const string& viewName, FWindowFactory widgetFactory, TConstScopedObjects& objects, bool bFloat)
{
    if (objects.empty()) return;
    CIRef<CProjectService> prjSrv = m_Workbench->GetServiceByType<CProjectService>();
    prjSrv->ShowView(viewName, widgetFactory, objects, bFloat);
}

/// IWMClient implementation

wxWindow* CSimpleProjectView::GetWindow()
{
    _ASSERT(m_Widget);
    return m_Widget;
}

bool CSimpleProjectView::IsADialog() const
{
    return (dynamic_cast<wxDialog*>(m_Widget) != 0);
}

/// IView implementation

const CViewTypeDescriptor& CSimpleProjectView::GetTypeDescriptor() const
{
    return m_ViewDescr;
}

void CSimpleProjectView::CreateViewWindow(wxWindow* parent)
{
    _ASSERT(!m_Widget);
    m_Widget = m_WidgetFactory(parent);
    IGuiWidget* guiWidget = dynamic_cast<IGuiWidget*>(m_Widget);
    if (guiWidget) guiWidget->SetHost(this);
}

void CSimpleProjectView::DestroyViewWindow()
{
    if (m_Widget) {
        IGuiWidget* guiWidget = dynamic_cast<IGuiWidget*>(m_Widget);
        if (guiWidget) {
            guiWidget->SetHost(0);
            guiWidget->SetClosing();
        }
        m_Widget->Destroy();
        m_Widget = 0;
    }
}

/// IProjectView implementation

bool CSimpleProjectView::InitView(TConstScopedObjects& objects, const objects::CUser_object* /*params*/)
{
    IGuiWidget* guiWidget = dynamic_cast<IGuiWidget*>(m_Widget);
    if (!guiWidget) {
        string view_name = m_ViewDescr.GetLabel();
        wxMessageBox(ToWxString(view_name + ": widget not created."), wxT("Simple view error"), wxOK | wxICON_EXCLAMATION);
        return false;
    }

    if (objects.empty() || !guiWidget->InitWidget(objects)) {
        x_ReportInvalidInputData(objects);
        return false;
    }

    CScope* scope = objects.front().scope.GetPointer();

    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    _ASSERT(srv);

    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws) return false;

    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*scope));
    _ASSERT(doc);
    if (!doc) return false;

    guiWidget->SetUndoManager(&doc->GetUndoManager());

    x_AttachToProject(*doc);
    x_UpdateContentLabel();

    guiWidget->SetWidgetTitle(ToWxString(GetClientLabel()));

    return true;
}

void CSimpleProjectView::OnProjectChanged(CProjectViewEvent& evt)
{
    IGuiWidget* guiWidget = dynamic_cast<IGuiWidget*>(m_Widget);
    if (!guiWidget) return;

    if (evt.GetSubtype() == CProjectViewEvent::eDataChanging) {
        guiWidget->DataChanging();
    }
    else if (evt.GetSubtype() == CProjectViewEvent::eData ||
             evt.GetSubtype() == CProjectViewEvent::eBothPropsData) {
        guiWidget->DataChanged();
    }
}

/// ISelectionClient Implementation

void CSimpleProjectView::GetSelection(CSelectionEvent& evt) const
{
    IGuiWidget* guiWidget = dynamic_cast<IGuiWidget*>(m_Widget);
    if (!guiWidget) return;

    if (!x_HasProject()) return;

    TConstObjects objects;
    guiWidget->GetSelectedObjects(objects);
    evt.AddObjectSelection(objects);
}

void CSimpleProjectView::GetSelection(TConstScopedObjects& objs) const
{
    IGuiWidget* guiWidget = dynamic_cast<IGuiWidget*>(m_Widget);
    if (!guiWidget) return;

    if (!x_HasProject()) return;

    CScope* scope = x_GetScope();
    TConstObjects objects;
    guiWidget->GetSelectedObjects(objects);
    ITERATE(TConstObjects, it, objects)
        objs.push_back(SConstScopedObject(*it, scope));
}

void CSimpleProjectView::GetMainObject(TConstScopedObjects& objs) const
{
    IGuiWidget* guiWidget = dynamic_cast<IGuiWidget*>(m_Widget);
    if (!guiWidget) return;

    vector<TConstScopedObjects> widgetObjects;
    guiWidget->GetActiveObjects(widgetObjects);
    if (!widgetObjects.empty() && !widgetObjects.back().empty())
        objs.insert(objs.end(), widgetObjects.back().begin(), widgetObjects.back().end());
}

void CSimpleProjectView::GetActiveObjects(TConstScopedObjects& objects) const
{
    IGuiWidget* guiWidget = dynamic_cast<IGuiWidget*>(m_Widget);
    if (!guiWidget) return;

    vector<TConstScopedObjects> widgetObjects;
    guiWidget->GetActiveObjects(widgetObjects);
    ITERATE(vector<TConstScopedObjects>, it, widgetObjects)
        objects.insert(objects.end(), it->begin(), it->end());
}

void CSimpleProjectView::GetActiveObjects(vector<TConstScopedObjects>& objects)
{
    IGuiWidget* guiWidget = dynamic_cast<IGuiWidget*>(m_Widget);
    if (!guiWidget) return;

    guiWidget->GetActiveObjects(objects);
}

/// CProjectViewBase overridables

const CObject* CSimpleProjectView::x_GetOrigObject() const
{
    IGuiWidget* guiWidget = dynamic_cast<IGuiWidget*>(m_Widget);
    return guiWidget ? guiWidget->GetOrigObject() : 0;
}


void CSimpleProjectView::x_OnSetSelection(CSelectionEvent& evt)
{
    IGuiWidget* guiWidget = dynamic_cast<IGuiWidget*>(m_Widget);
    if (!guiWidget) return;

    if (!x_HasProject()) return;

    TConstObjects objs;
    evt.GetAllObjects(objs);
	evt.GetOther(objs);
    guiWidget->SetSelectedObjects(objs);
}

END_NCBI_SCOPE
