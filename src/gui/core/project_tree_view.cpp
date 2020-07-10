/*  $Id: project_tree_view.cpp 41875 2018-10-31 17:19:44Z filippov $
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

#include <ncbi_pch.hpp>

#include <gui/core/project_tree_view.hpp>

#include <gui/core/project_service.hpp>

#include <gui/core/project_tree_panel.hpp>
#include <gui/core/pt_project_item.hpp>
#include <gui/core/pt_project.hpp>
#include <gui/core/pt_utils.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/core/data_mining_service.hpp>

#include <gui/framework/view_manager_service.hpp> //TODO
#include <gui/framework/workbench.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/utils/extension_impl.hpp>

#include <serial/iterator.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


static CExtensionPointDeclaration
    point1(EXT_POINT__PROJECT_TREE_VIEW__CONTEXT_MENU__ITEM_CMD_CONTRIBUTOR,
           "Project View - commands applicable to Explorer Items");

/// register the factory in the Extention Point
static CExtensionDeclaration
    decl1("view_manager_service::view_factory", new CProjectTreeViewFactory());


///////////////////////////////////////////////////////////////////////////////
/// CProjectTreeView

CViewTypeDescriptor CProjectTreeView::m_TypeDescr(
    "Project View", // type name
    "project_tree_view", // icon alias TODO
    "Project View hint - TODO",
    "Project View description - TODO",
    "PROJECT_TREE_VIEW", // help Id
    "System",     // category
    true);       // singleton

BEGIN_EVENT_MAP(CProjectTreeView, CEventHandler)
    ON_EVENT(CEvent, CViewEvent::eWidgetSelectionChanged,
                     &CProjectTreeView::OnWidgetSelChanged)
END_EVENT_MAP()


CProjectTreeView::CProjectTreeView()
:   m_Workbench(NULL),
    m_SelectionService(NULL),
    m_Panel(NULL)
{
}


CProjectTreeView::~CProjectTreeView()
{
}


const CViewTypeDescriptor& CProjectTreeView::GetTypeDescriptor() const
{
    return m_TypeDescr;
}


void CProjectTreeView::SetWorkbench(IWorkbench* workbench)
{
    if(workbench)   {
        // connect to services
        CIRef<CSelectionService> sel_srv = workbench->GetServiceByType<CSelectionService>();
        if(sel_srv) {
            sel_srv->AttachClient(this);
        }

        // connect
        CIRef<CDataMiningService> dm_srv = workbench->GetServiceByType<CDataMiningService>();
        if (dm_srv) {
            dm_srv->AttachContext(*this);
        }

        m_Panel->SetWorkbench(workbench);

        // to receive selection change event
        m_Panel->AddListener(this, ePool_Parent);
    } else if(m_Workbench)  {
        m_Panel->RemoveListener(this);

        m_Panel->SetWorkbench(NULL);

        // disconnect from services
        CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
        if(sel_srv) {
            sel_srv->DetachClient(this);
        }

        // disconnect
        CIRef<CDataMiningService> dm_srv = m_Workbench->GetServiceByType<CDataMiningService>();
        if (dm_srv) {
            dm_srv->DetachContext(*this);
        }
    }

    m_Workbench = workbench;
}

void CProjectTreeView::CreateViewWindow(wxWindow* parent)
{
    m_Panel = new CProjectTreePanel();
    m_Panel->Create(parent, wxID_ANY);
}


void CProjectTreeView::DestroyViewWindow()
{
    if(m_Panel) {
        m_Panel->Destroy();
        m_Panel = NULL;
    }
}

wxWindow* CProjectTreeView::GetWindow()
{
    return m_Panel;
}


string CProjectTreeView::GetClientLabel(IWMClient::ELabel) const
{
    return m_TypeDescr.GetLabel();
}


string CProjectTreeView::GetIconAlias() const
{
    return m_TypeDescr.GetIconAlias();
}


const CRgbaColor* CProjectTreeView::GetColor() const
{
    return NULL;
}


void CProjectTreeView::SetColor(const CRgbaColor& color)
{
    // do nothing
}


const wxMenu* CProjectTreeView::GetMenu()
{
    return NULL;
}


void CProjectTreeView::UpdateMenu(wxMenu& root_menu)
{
}


IWMClient::CFingerprint CProjectTreeView::GetFingerprint() const
{
    return CFingerprint(m_TypeDescr.GetLabel(), true);
}


void CProjectTreeView::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}


//static const char* kDisplayModeTag = "DisplayMode";

void CProjectTreeView::SaveSettings() const
{
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    gui_reg.Set("GBENCH.Application.ViewOptions.HideDisabledProjectItems", PT::sm_HideDisabledItems);
}


void CProjectTreeView::LoadSettings()
{
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    PT::sm_HideDisabledItems = gui_reg.GetBool("GBENCH.Application.ViewOptions.HideDisabledProjectItems", false);
}


/// ISelectionClient //////////////////////////////////////////////////////////

void CProjectTreeView::OnWidgetSelChanged(CEvent* evt)
{
    _ASSERT(evt);

    if (evt->GetID() == CViewEvent::eWidgetSelectionChanged  &&  m_SelectionService) {
        m_SelectionService->OnSelectionChanged(this);
    }
}

void CProjectTreeView::GetSelection(TConstScopedObjects& objs) const
{
    CProjectTreeView* nc_this = const_cast<CProjectTreeView*>(this);

    PT::TItems sel_items;
    nc_this->m_Panel->GetSelectedItems(sel_items);
    if (sel_items.empty())
        return;

    CIRef<CProjectService> prj_srv = m_Workbench->GetServiceByType<CProjectService>();

    CRef<CGBWorkspace> ws = prj_srv->GetGBWorkspace();
    if (!ws) return;

    map<int, const CProjectItem*> itemMap;

    NON_CONST_ITERATE (PT::TItems, it, sel_items) {
        PT::CItem& item = **it;

        SConstScopedObject obj;

        switch( item.GetType() ){
        case PT::eProject: {
            /* TODO
            TProjectTreeItem* prj_item = dynamic_cast<TProjectTreeItem*>(&item);

            int id = prj_item->GetData();
            CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromId(id));
            if(doc  &&  doc->IsLoaded()  &&  doc->GetScope()) {
                obj.object.Reset(doc);
                obj.scope.Reset(doc->GetScope());
            }*/
            break;
        }

        case PT::eProjectItem: {
            PT::CProjectItem* prj_item = static_cast<PT::CProjectItem*>(&item);
            if (!prj_item) continue;

            CRef<CProjectItem> projItem = prj_item->GetData();
            if (!projItem->IsEnabled())
                continue;

            PT::CProject* projectItem  = PT::GetProject(*m_Panel->m_Tree, *prj_item);
            if (!projectItem) continue;

            CGBDocument* doc = projectItem->GetData();

            obj.object.Reset(projItem->GetObject());
            obj.scope.Reset(doc->GetScope());
            break;
        }

        default:
            break;
        }

        if (obj.scope.GetPointer()  &&  obj.object.GetPointer()) {
            objs.push_back(obj);
        }
    }
}


void CProjectTreeView::SetSelectionService(ISelectionService* service)
{
    m_SelectionService = service;
}


string CProjectTreeView::GetSelClientLabel()
{
    return GetClientLabel(IWMClient::eDefault);
}


void CProjectTreeView::GetSelection(CSelectionEvent& evt) const
{
    // do nothing, CProjectTreeView does not support selection broadcast
}


void CProjectTreeView::SetSelection(CSelectionEvent& evt)
{
    // do nothing, CProjectTreeView does not support selection broadcast
}


void CProjectTreeView::GetMainObject(TConstScopedObjects& objs) const
{
    // no main object
}

void CProjectTreeView::GetSingleObject(TConstScopedObjects& objs) const
{
    CProjectTreeView* nc_this = const_cast<CProjectTreeView*>(this);

    PT::TItems all_items;
    nc_this->m_Panel->GetAllItems(all_items);
    if (all_items.empty())
        return;

    CIRef<CProjectService> prj_srv = m_Workbench->GetServiceByType<CProjectService>();

    CRef<CGBWorkspace> ws = prj_srv->GetGBWorkspace();
    if (!ws) return;

    map<int, const CProjectItem*> itemMap;

    NON_CONST_ITERATE (PT::TItems, it, all_items) {
        PT::CItem& item = **it;

        SConstScopedObject obj;

        switch( item.GetType() ){
        case PT::eProject: {
            /* TODO
            TProjectTreeItem* prj_item = dynamic_cast<TProjectTreeItem*>(&item);

            int id = prj_item->GetData();
            CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromId(id));
            if(doc  &&  doc->IsLoaded()  &&  doc->GetScope()) {
                obj.object.Reset(doc);
                obj.scope.Reset(doc->GetScope());
            }*/
            break;
        }

        case PT::eProjectItem: {
            PT::CProjectItem* prj_item = static_cast<PT::CProjectItem*>(&item);
            if (!prj_item) continue;

            CRef<CProjectItem> projItem = prj_item->GetData();
            if (!projItem->IsEnabled())
                continue;

            PT::CProject* projectItem  = PT::GetProject(*m_Panel->m_Tree, *prj_item);
            if (!projectItem) continue;

            CGBDocument* doc = projectItem->GetData();

            obj.object.Reset(projItem->GetObject());
            obj.scope.Reset(doc->GetScope());
            break;
        }

        default:
            break;
        }

        if (obj.scope.GetPointer()  &&  obj.object.GetPointer()) {
            objs.push_back(obj);
            break; // only need 1 object;
        }
    }
}

/// END of ISelectionClient ///////////////////////////////////////////////////

void CProjectTreeView::GetSelectedItems(PT::TItems& items)
{
    if(m_Panel) {
        m_Panel->GetSelectedItems(items);
    }
}


string CProjectTreeView::GetDMContextName()
{
    return GetClientLabel(IWMClient::eDefault);
}

CRef<CSeq_loc> CProjectTreeView::GetSearchLoc()
{
    CRef<CSeq_loc> loc;
    CRef<CSeq_id>  id(new CSeq_id());

    TConstScopedObjects objs;
    GetSelection(objs);
    if (objs.size()>0) {
        const CSeq_id * sid  = dynamic_cast<const CSeq_id*>(objs.begin()->object.GetPointer());
        if (sid) {
            id->Assign(*sid);
            loc.Reset(new CSeq_loc());
            loc->SetWhole(*id);
        }
    }

    return loc;
}

void CProjectTreeView::GetMultiple(TConstScopedObjects & pObjects)
{   
    TConstScopedObjects objs;
    GetSelection(objs);
    
    ITERATE (TConstScopedObjects, obj, objs) {
        CRef<CSeq_loc> loc;
        CRef<CSeq_id>  id(new CSeq_id());
       

        const CSeq_id * sid  = dynamic_cast<const CSeq_id*>(obj->object.GetPointer());

        if (sid) {
            CRef<CScope> scope = obj->scope;
            id->Assign(*sid);
            loc.Reset(new CSeq_loc());
            loc->SetWhole(*id);
            pObjects.push_back(SConstScopedObject(loc, scope));
        }
    }
}


CRef<CScope> CProjectTreeView::GetSearchScope()
{
    TConstScopedObjects objs;
    GetSelection(objs);
    if (objs.size()==1) {
        return objs.begin()->scope; 
    }
    return CRef<CScope>();
}

///////////////////////////////////////////////////////////////////////////////
/// CProjectTreeViewFactory
string  CProjectTreeViewFactory::GetExtensionIdentifier() const
{
    static string sid("project_tree_view_factory");
    return sid;
}


string  CProjectTreeViewFactory::GetExtensionLabel() const
{
    static string slabel("Project View Factory");
    return slabel;
}


void CProjectTreeViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    //TODO
}


const CViewTypeDescriptor& CProjectTreeViewFactory::GetViewTypeDescriptor() const
{
    return CProjectTreeView::m_TypeDescr;
}


IView* CProjectTreeViewFactory::CreateInstance() const
{
    return new CProjectTreeView();
}


IView* CProjectTreeViewFactory::CreateInstanceByFingerprint(const TFingerprint& fingerprint) const
{
    TFingerprint print(CProjectTreeView::m_TypeDescr.GetLabel(), true);
    if(print == fingerprint)   {
        return new CProjectTreeView();
    }
    return NULL;
}


END_NCBI_SCOPE
