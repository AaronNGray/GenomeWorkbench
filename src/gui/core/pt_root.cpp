/*  $Id: pt_root.cpp 33078 2015-05-21 15:06:52Z katargir $
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

#include <gui/core/pt_view.hpp>
#include <gui/core/pt_data_source.hpp>
#include <gui/core/pt_icon_list.hpp>
#include <gui/core/ui_data_source_service.hpp>
#include <gui/core/document.hpp>
#include <gui/core/pt_workspace.hpp>
#include <gui/core/pt_root.hpp>
#include <gui/core/pt_folder.hpp>
#include <gui/framework/view_manager_service.hpp>

#include <serial/iterator.hpp>

#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/treectrl.h>

BEGIN_NCBI_SCOPE

using namespace PT;

static const wxString s_ALL_VIEWS_FOLDER = wxT("All Views");
static const wxString s_DATA_SOURCES_FOLDER = wxT("Data Sources");

void CRoot::Initialize(wxTreeCtrl& treeCtrl, objects::CGBWorkspace* ws)
{
    CPTIcons& icons = CPTIcons::GetInstance();

    if (ws) {
        CWorkspace* wsItem = new CWorkspace(CRef<objects::CGBWorkspace>(ws));
        AppendChildItem(treeCtrl, *wsItem);
        wsItem->Initialize(treeCtrl);

        CFolder* viewsFolder = new CFolder(s_ALL_VIEWS_FOLDER);
        AppendChildItem(treeCtrl, *viewsFolder);
        viewsFolder->Initialize(treeCtrl);
        treeCtrl.Expand(viewsFolder->GetTreeItemId());
    }

    CFolder* dsFolder = new CFolder(s_DATA_SOURCES_FOLDER);
    AppendChildItem(treeCtrl, *dsFolder);
    treeCtrl.SetItemText(dsFolder->GetTreeItemId(), s_DATA_SOURCES_FOLDER);
    treeCtrl.SetItemImage(dsFolder->GetTreeItemId(),
                            icons.GetImageIndex(CPTIcons::eDataSourceIcon),
                            wxTreeItemIcon_Normal);
}

void CRoot::UpdateDataSources(wxTreeCtrl& treeCtrl, IServiceLocator& serviceLocator)
{
    CFolder* dsFolder = 0;
    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eFolder && (*it).GetLabel(treeCtrl) == s_DATA_SOURCES_FOLDER) {
            dsFolder = static_cast<CFolder*>(&*it);
            break;
        }
    }

    CPTIcons& icons = CPTIcons::GetInstance();

    if (!dsFolder) {
        dsFolder = new CFolder(s_DATA_SOURCES_FOLDER);
        AppendChildItem(treeCtrl, *dsFolder);
        treeCtrl.SetItemText(dsFolder->GetTreeItemId(), s_DATA_SOURCES_FOLDER);
        treeCtrl.SetItemImage(dsFolder->GetTreeItemId(),
                              icons.GetImageIndex(CPTIcons::eDataSourceIcon),
                              wxTreeItemIcon_Normal);
    }

    treeCtrl.DeleteChildren(dsFolder->GetTreeItemId());

    CUIDataSourceService* ds_srv =
        serviceLocator.GetServiceByType<CUIDataSourceService>();

    _ASSERT(ds_srv);

    // obtain a list of Data Sources
    typedef CUIDataSourceService::TUIDataSourceVec TDSVec;
    TDSVec data_sources;
    ds_srv->GetDataSources(data_sources);

    // create new items
    NON_CONST_ITERATE(TDSVec, it_ds, data_sources)    {
        IUIDataSource& ds = **it_ds;
        CDataSource* new_item =  new CDataSource(CIRef<IUIDataSource>(&ds));
        dsFolder->AppendChildItem (treeCtrl, *new_item);
        new_item->Initialize(treeCtrl, ds);
    }

    treeCtrl.Expand(dsFolder->GetTreeItemId());
}

void CRoot::UpdateAllViews(wxTreeCtrl& treeCtrl, objects::CGBWorkspace* ws)
{
    CFolder* viewsFolder = 0;
    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eFolder && (*it).GetLabel(treeCtrl) == s_ALL_VIEWS_FOLDER) {
            viewsFolder = static_cast<CFolder*>(&*it);
            break;
        }
    }

    if (!viewsFolder)
        return;

    treeCtrl.DeleteChildren(viewsFolder->GetTreeItemId());

    vector< CIRef<IProjectView> > views;

    if (ws) {
        for (CTypeIterator<objects::CGBProjectHandle> it(ws->SetWorkspace()); it; ++it) {
            CGBDocument* doc = dynamic_cast<CGBDocument*>(&*it);
            if (doc) {
                vector< CIRef<IProjectView> > dviews = doc->GetViews();
                if (!dviews.empty())
                    views.insert(views.end(), dviews.begin(), dviews.end());
            }
        }
        for (size_t i = 0;  i < views.size();  i++) {
            IProjectView& prjView = *views[i];
            CView* item = new CView(CIRef<IProjectView>(&prjView));
            viewsFolder->AppendChildItem(treeCtrl, *item);
            item->Initialize(treeCtrl, prjView, false);
        }
    }
    treeCtrl.Expand(viewsFolder->GetTreeItemId());
}

CWorkspace* CRoot::GetWorkspaceItem(wxTreeCtrl& treeCtrl)
{
    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eWorkspace) {
            return static_cast<CWorkspace*>(&*it);
        }
    }
    return 0;
}

void CRoot::UpdateViewLabel(wxTreeCtrl& treeCtrl, IProjectView& view)
{
    CFolder* viewsFolder = 0;
    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eFolder && (*it).GetLabel(treeCtrl) == s_ALL_VIEWS_FOLDER) {
            viewsFolder = static_cast<CFolder*>(&*it);
            break;
        }
    }

    if (!viewsFolder)
        return;

    for (CChildIterator it(treeCtrl, *viewsFolder); it; ++it) {
        if ((*it).GetType() == eView) {
            CView& viewItem = static_cast<CView& >(*it);
            if (viewItem.GetData().GetPointerOrNull() == &view) {
                viewItem.Initialize(treeCtrl, view, false);
                break;
            }
        }
    }
}

END_NCBI_SCOPE
