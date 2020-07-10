/*  $Id: table_view.cpp 40262 2018-01-18 20:27:56Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/core/table_view.hpp>

#include <gui/objutils/table_selection.hpp>

#include <gui/core/project_service.hpp>
#include <gui/core/document.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/framework/status_bar_service.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/widgets/data/query_panel_event.hpp>
#include <gui/widgets/grid_widget/grid_event.hpp>
#include <gui/widgets/grid_widget/grid.hpp>
#include <gui/widgets/grid_widget/grid_widget.hpp>

#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <objects/gbproj/GBProject_ver2.hpp>

#include <wx/app.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CProjectViewTypeDescriptor s_TableViewTypeDescr(
    "Generic Table View", // type name
    "table_view", // icon alias
    "Create an interactive table representation of data.",
    "The Generic Table View shows Seq-table object",
    "TABLE_VIEW", // help ID
    "Generic",   // category
    false,      // not a singleton
    "Seq-table",
    eOneObjectAccepted
);

CTableView::CTableView()
 : CProjectView()
 , m_Window(0)
{
}

BEGIN_EVENT_MAP(CTableView, CProjectView)
  ON_EVENT(CGridWidgetEvent, CGridWidgetEvent::eUrlHover,
           &CTableView::x_OnUrlHover)
  ON_EVENT(CQueryPanelEvent, CQueryPanelEvent::eStatusChange,
           &CTableView::x_QueryStatusChange)
  ON_EVENT(CGridWidgetEvent, CGridWidgetEvent::eRowDClick, 
           &CTableView::x_OnRowDClick)
END_EVENT_MAP()

wxWindow* CTableView::GetWindow()
{
    _ASSERT(m_Window);
    return m_Window;
}

wxEvtHandler* CTableView::GetCommandHandler()
{
    CGridWidget* grid = (CGridWidget*)m_Window;
    _ASSERT(grid);
    return grid->GetEventHandler();
}

// override function to suppress Navigation toolbar
void CTableView::GetCompatibleToolBars(vector<string>& names)
{
    names.push_back(CProjectViewToolBatFactory::sm_SelTBName);
}


bool CTableView::InitView(TConstScopedObjects& objects, const objects::CUser_object* params)
{
    m_OrigObjects = objects;

    CIRef<ITableData> tableData = x_CreateTableData(objects);

    if (tableData) {
        CIRef<ITableSelection> tableSelection;
        if (objects.size() == 1)
            tableSelection.Reset(CreateObjectInterface<ITableSelection>(objects[0], NULL));

        CGridWidget* widget = (CGridWidget*)m_Window;

        if (tableSelection) widget->SetTableSelection(tableSelection);

        CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
        _ASSERT(srv);

        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (!ws) return false;

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*objects[0].scope));
        _ASSERT(doc);
        if (doc) {
            widget->Init(*tableData, &doc->GetUndoManager());

            x_AttachToProject(*doc);
            x_UpdateContentLabel();
            return true;
        }
        return false;
    }

    // cannot represent the data
    x_ReportInvalidInputData(objects);
    return false;
}

void CTableView::OnProjectChanged(CProjectViewEvent& evt)
{
    if (evt.GetSubtype() == CProjectViewEvent::eDataChanging) {
        //CGridWidget* grid = (CGridWidget*)m_Window;
    }
    else if (evt.GetSubtype() == CProjectViewEvent::eData ||
                evt.GetSubtype() == CProjectViewEvent::eBothPropsData) {
        CGridWidget* grid = (CGridWidget*)m_Window;
        grid->Refresh();
    }
}

void CTableView::CreateViewWindow(wxWindow* parent)
{
    _ASSERT(!m_Window);

    CGridWidget* widget = new CGridWidget(parent, wxID_ANY, wxDefaultPosition, wxSize(0, 0), wxBORDER_NONE);
    m_Window = widget;
    widget->AddListener(this, ePool_Parent);
}


void CTableView::DestroyViewWindow()
{
    if (m_Window) {
        m_Window->Reparent(wxTheApp->GetTopWindow()); // prevents double destruction of CQueryParsePanel in AsyncCall
        m_Window->Destroy();
        m_Window = 0;
    }
}


const CViewTypeDescriptor& CTableView::GetTypeDescriptor() const
{
    return s_TableViewTypeDescr;
}

void CTableView::GetSelection(CSelectionEvent& evt) const
{
    if (m_Window  &&  x_HasProject()) {
        CGridWidget* grid = (CGridWidget*)m_Window;
        grid->GetSelection(evt);
    }
}

void CTableView::x_OnSetSelection(CSelectionEvent& evt)
{
    if (m_Window  &&  x_HasProject()) {
        CGridWidget* grid = (CGridWidget*)m_Window;
        grid->SetSelection(evt);
    }
}

void CTableView::GetSelection(TConstScopedObjects& objs) const
{
    if (m_Window  &&  x_HasProject()) {
        CGridWidget* grid = (CGridWidget*)m_Window;
        grid->GetSelectedObjects (objs);
    }
}

void CTableView::GetMainObject(TConstScopedObjects& objs) const
{
    if (m_Window  &&  x_HasProject()) {
        /*CGridWidget* grid = (CGridWidget*)m_Window;*/

        if(m_OrigObjects.size() > 0) {
            const SConstScopedObject& ScopedObject(m_OrigObjects[0]);
            const CObject* object = ScopedObject.object.GetPointer();
            if (object) {
                CScope* scope(const_cast<CScope*>(ScopedObject.scope.GetPointer()));
                objs.push_back(SConstScopedObject(object, scope));
            }
        }
    }
}


void CTableView::x_OnUrlHover(CEvent* evt)
{
    CGridWidgetEvent* gridEvt = dynamic_cast<CGridWidgetEvent*>(evt);
    if (!gridEvt) return;

    CIRef<IStatusBarService> sb_srv = m_Workbench->GetServiceByType<IStatusBarService>();
    sb_srv->SetStatusMessage(ToStdString(gridEvt->GetURL()));
}

void CTableView::x_QueryStatusChange(CEvent* evt)
{
    CQueryPanelEvent* queryEvt = dynamic_cast<CQueryPanelEvent*>(evt);
    if (!queryEvt) return;

    CIRef<IStatusBarService> sb_srv = m_Workbench->GetServiceByType<IStatusBarService>();
    sb_srv->SetStatusMessage(queryEvt->GetStatus());
}

void CTableView::x_OnRowDClick(CEvent* /*evt*/)
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetItemActivated);
    Send(&evt, ePool_Parent);
}

static const string sTextViewType("Text View");

const CObject* CTableView::x_GetOrigObject() const
{
    if(m_OrigObjects.size() > 0) {
        return m_OrigObjects[0].object.GetPointer();
    }

    return NULL;
}

CIRef<ITableData> CTableView::x_CreateTableData(TConstScopedObjects& objects)
{
    CIRef<ITableData> table_data;

    if (objects.size() == 1) {
        table_data.Reset(CreateObjectInterface<ITableData>(objects[0], NULL));
    }

    return table_data;
}

void CTableView::SetRegistryPath(const string& reg_path)
{
    IRegSettings* reg_set = dynamic_cast<IRegSettings*>(m_Window);
    if (reg_set)
        reg_set->SetRegistryPath(reg_path);
}

void CTableView::LoadSettings()
{
    IRegSettings* reg_set = dynamic_cast<IRegSettings*>(m_Window);
    if (reg_set)
        reg_set->LoadSettings();
}


void CTableView::SaveSettings() const
{
    IRegSettings* reg_set = dynamic_cast<IRegSettings*>(m_Window);
    if (reg_set)
        reg_set->SaveSettings();
}



///////////////////////////////////////////////////////////////////////////////
/// CTableViewFactory
string  CTableViewFactory::GetExtensionIdentifier() const
{
    static string sid("table_view_factory");
    return sid;
}


string CTableViewFactory::GetExtensionLabel() const
{
    static string slabel("Table View Factory");
    return slabel;
}


void CTableViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    string alias = GetViewTypeDescriptor().GetIconAlias();
    provider.RegisterFileAlias(ToWxString(alias), wxT("text_view.png"));
}


const CProjectViewTypeDescriptor& CTableViewFactory::GetProjectViewTypeDescriptor() const
{
    return s_TableViewTypeDescr;
}


IView* CTableViewFactory::CreateInstance() const
{
    return new CTableView();
}


IView* CTableViewFactory::CreateInstanceByFingerprint(const TFingerprint&) const
{
    return NULL;
}


int CTableViewFactory::TestInputObjects(TConstScopedObjects& objects)
{
    bool found_good = false;
    bool found_bad = false;

    for(  size_t i = 0;  i < objects.size();  i++)  {
        if (ObjectHasInterface<ITableData>(objects[i]))
            found_good = true;
        else
            found_bad = true;
    }

    if (found_good)
        return fCanShowSeparated | (found_bad ? fCanShowSome : fCanShowAll);

    return 0; // can show nothing
}

CProjectViewTypeDescriptor s_ProjectTableViewTypeDescr(
    "Project Table View", // type name
    "project_table_view", // icon alias
    "Create an interactive table representation of data.",
    "The Project Table View shows Genome Workbench project",
    "PROJECT_TABLE_VIEW", // help ID
    "Generic",   // category
    false,      // not a singleton
    "GBProject-ver2",
    eOneObjectAccepted
);

///////////////////////////////////////////////////////////////////////////////
/// CProjectTableView
class CProjectTableView : public CTableView
{
public:
    CProjectTableView() {}

    /// @name IView implementation
    /// @(
    virtual const CViewTypeDescriptor&  GetTypeDescriptor() const
    {
        return s_ProjectTableViewTypeDescr;
    }
    /// @}

    /// @name IProjectView implementation
    /// @{
    virtual void OnProjectChanged(CProjectViewEvent& evt);
    /// @}
};

void CProjectTableView::OnProjectChanged(CProjectViewEvent& evt)
{
    CGridWidget* widget = (CGridWidget*)GetWindow();

    if (evt.GetSubtype() ==  CProjectViewEvent::eNone ||
        evt.GetSubtype() ==  CProjectViewEvent::eProperties) {
        widget->Refresh();
    }
    else if (evt.GetSubtype() == CProjectViewEvent::eData ||
                evt.GetSubtype() == CProjectViewEvent::eBothPropsData) {
        TConstScopedObjects objects;
        GetMainObject(objects);

        CIRef<ITableData> table_data;
        if (objects.size() > 0) {
            table_data.Reset(CreateObjectInterface<ITableData>(objects[0], NULL));
        }
        if (!table_data)
            return;

        widget->Init(*table_data, x_GetUndoManager());
    }
}

/// CProjectTableViewFactory
string  CProjectTableViewFactory::GetExtensionIdentifier() const
{
    static string sid("project_table_view_factory");
    return sid;
}


string CProjectTableViewFactory::GetExtensionLabel() const
{
    static string slabel("Project Table View Factory");
    return slabel;
}


void CProjectTableViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
}


const CProjectViewTypeDescriptor& CProjectTableViewFactory::GetProjectViewTypeDescriptor() const
{
    return s_ProjectTableViewTypeDescr;
}


IView* CProjectTableViewFactory::CreateInstance() const
{
    return new CProjectTableView();
}


bool CProjectTableViewFactory::IsCompatibleWith(const CObject& object, objects::CScope& scope)
{
    const type_info& type = typeid(object);
    if( typeid(CGBProject_ver2) == type) {
        return true;
    }
    return false;
}

IView* CProjectTableViewFactory::CreateInstanceByFingerprint(const TFingerprint&) const
{
    return NULL;
}


int CProjectTableViewFactory::TestInputObjects(TConstScopedObjects& objects)
{
    bool found_good = false;
    bool found_bad = false;

    for(  size_t i = 0;  i < objects.size();  i++)  {
        if (IsCompatibleWith(*objects[i].object, *objects[i].scope)) {
            found_good = true;
        }

        found_bad = true;
    }

    if (found_good)
        return fCanShowSeparated | (found_bad ? fCanShowSome : fCanShowAll);

    return 0; // can show nothing
}

END_NCBI_SCOPE
