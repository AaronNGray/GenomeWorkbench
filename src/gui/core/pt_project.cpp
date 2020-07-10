/*  $Id: pt_project.cpp 38286 2017-04-21 20:43:46Z katargir $
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

#include <objects/general/Date.hpp>

#include <objects/gbproj/FolderInfo.hpp>
#include <objects/gbproj/ProjectDescr.hpp>

#include <gui/core/pt_project.hpp>
#include <gui/core/pt_project_folder.hpp>
#include <gui/core/pt_view.hpp>
#include <gui/core/pt_folder.hpp>
#include <gui/core/pt_data_loader.hpp>
#include <gui/core/project_edit_dlg.hpp>
#include <gui/core/pt_icon_list.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/treectrl.h>

BEGIN_NCBI_SCOPE

using namespace PT;

static const wxString s_DATA_FOLDER    = wxT("Data");
static const wxString s_LOADERS_FOLDER = wxT("Data Loaders");
static const wxString s_VIEWS_FOLDER   = wxT("Views");

static wxString s_GetLabel(CGBDocument* doc)
{
    wxString label;
    if (doc  &&  doc->IsLoaded()) {    // project is loaded
        const objects::CProjectDescr& descr = doc->GetDescr();
        label = ToWxString(descr.GetTitle());
        if(doc->IsDirty())    {
            label += wxT(" (*)");
        }
    } else if (doc  &&  doc->IsLoading()) {    // project is loading
        label = doc->GetFileName() + wxT(" (Loading ...)");
    } else if (doc) {
        // Project is not loaded
        label = doc->GetFileName() + wxT(" (not loaded)");
    } else {
        label = wxT("N/A");
    }
    return label;
}

static void s_InitializeViews(wxTreeCtrl& treeCtrl, CFolder& folder, vector< CIRef<IProjectView> >& views)
{
    NON_CONST_ITERATE(vector< CIRef<IProjectView> >, it, views) {
        IProjectView& view = **it;
        CView* view_item = new CView(CIRef<IProjectView>(&view));
        folder.AppendChildItem(treeCtrl, *view_item);
        view_item->Initialize(treeCtrl, view, true);
    }
}

void CProject::UpdateLabel(wxTreeCtrl& treeCtrl)
{
    treeCtrl.SetItemText(m_TreeItemId, s_GetLabel(GetData()));
}

void CProject::UpdateViews(wxTreeCtrl& treeCtrl)
{
    CGBDocument* doc = GetData();
    _ASSERT(doc);

    CFolder* views_folder = 0;

    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetLabel(treeCtrl) == s_VIEWS_FOLDER && (*it).GetType() == eFolder) {
            views_folder = static_cast<CFolder*>(&*it);
            break;
        }
    }

    vector< CIRef<IProjectView> > views = doc->GetViews();

    if (views.size()) {
        if (views_folder == NULL) {
            views_folder = new CFolder(s_VIEWS_FOLDER);
            AppendChildItem(treeCtrl, *views_folder);
            views_folder->Initialize(treeCtrl);
        } else {
            treeCtrl.DeleteChildren(views_folder->GetTreeItemId());
        }
        s_InitializeViews(treeCtrl, *views_folder, views);
    } else if (views_folder) {
        treeCtrl.Delete(views_folder->GetTreeItemId());
    }
}

void CProject::UpdateViewLabel(wxTreeCtrl& treeCtrl, IProjectView& view)
{
    CFolder* viewsFolder = 0;

    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetLabel(treeCtrl) == s_VIEWS_FOLDER && (*it).GetType() == eFolder) {
            viewsFolder = static_cast<CFolder*>(&*it);
            break;
        }
    }

    if (!viewsFolder) return;

    for (CChildIterator it(treeCtrl, *viewsFolder); it; ++it) {
        if ((*it).GetType() == eView) {
            CView& viewItem = static_cast<CView& >(*it);
            if (viewItem.GetData().GetPointerOrNull() == &view) {
                viewItem.Initialize(treeCtrl, view, true);
                break;
            }
        }
    }
}

void CProject::Initialize(wxTreeCtrl& treeCtrl)
{
    PT::CPTIcons& icons = PT::CPTIcons::GetInstance();

    treeCtrl.SetItemImage(m_TreeItemId, 
                          icons.GetImageIndex(PT::CPTIcons::eProjectClosedIcon),
                          wxTreeItemIcon_Normal);
    treeCtrl.SetItemImage(m_TreeItemId,
                          icons.GetImageIndex(PT::CPTIcons::eProjectOpenIcon),
                          wxTreeItemIcon_Expanded);

    CGBDocument& doc = *GetData();

    UpdateLabel(treeCtrl);

    if (!doc.IsLoaded()) return;

    objects::CProjectFolder& folder = doc.SetData();

    CProjectFolder* data_item = new CProjectFolder(CRef<objects::CProjectFolder>(&folder));
    AppendChildItem(treeCtrl, *data_item);
    data_item->Initialize(treeCtrl, doc);

    if (doc.GetDataLoaders().size() > 0) {
        CFolder* loaders_folder = new CFolder(s_LOADERS_FOLDER);
        AppendChildItem(treeCtrl, *loaders_folder);
        loaders_folder->Initialize(treeCtrl);

        ITERATE (objects::CGBProjectHandle::TDataLoaders, iter, doc.GetDataLoaders()) {
            CRef<objects::CLoaderDescriptor> loader = *iter;
            CDataLoader* loader_item = new CDataLoader(loader);
            loaders_folder->AppendChildItem(treeCtrl, *loader_item);
            loader_item->Initialize(treeCtrl);
        }
        treeCtrl.Expand(loaders_folder->GetTreeItemId());
    }
    if (doc.GetViews().size()) {
        CFolder* views_folder = new CFolder(s_VIEWS_FOLDER);
        AppendChildItem(treeCtrl, *views_folder);
        views_folder->Initialize(treeCtrl);
        vector< CIRef<IProjectView> > views = doc.GetViews();
        s_InitializeViews(treeCtrl, *views_folder, views);
    }

    treeCtrl.Expand(m_TreeItemId);
}

size_t CProject::CountDisabledItems(wxTreeCtrl& treeCtrl)
{
    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eProjectFolder)
            return static_cast<CProjectFolder&>(*it).CountDisabledItems(treeCtrl);
    }
    return 0;
}

static const char* kDlgRegPath = "Dialogs.PT.EditProject";

bool CProject::CanDoProperties()
{
    return GetData()->IsLoaded();
}

bool CProject::DoProperties(wxTreeCtrl& treeCtrl)
{
    CGBDocument* doc = GetData();

    CProjectEditDlg dlg(NULL, wxID_ANY, wxT("Project Properties"));
    dlg.SetRegistryPath(kDlgRegPath);
	bool readOnly = false;
    if (readOnly)
        dlg.SetReadOnly();

    objects::CProjectDescr& descr = doc->SetDescr();

	dlg.SetProjectName(ToWxString(descr.GetTitle()));
    if (descr.CanGetComment())
	    dlg.SetDescr(ToWxString(descr.GetComment()));
	dlg.SetCreated(ToWxString(descr.GetCreate_date().AsCTime().AsString()));
	dlg.SetModified(ToWxString(descr.GetModified_date().AsCTime().AsString()));
	dlg.SetFilename(doc->GetFileName());

    if (dlg.ShowModal() == wxID_OK && !readOnly) {
        descr.SetTitle(ToStdString(dlg.GetProjectName()));
        descr.SetComment(ToStdString(dlg.GetDescr()));
        CTime time(CTime::eCurrent);
        objects::CDate date(time);
        descr.SetModified_date().Assign(date);
        doc->SetDirty (true);
        UpdateLabel(treeCtrl);
        return true;
    }
    return false;
}

void CProject::BeginLabelEdit(wxTreeCtrl& treeCtrl, wxTreeEvent& event)
{
    CGBDocument* doc = GetData();
    if (!doc->IsLoaded()) {
        event.Veto();
    }
    else {
        treeCtrl.SetItemText(m_TreeItemId, ToWxString(doc->GetDescr().GetTitle()));
    }
}

bool CProject::EndLabelEdit(wxTreeCtrl& treeCtrl, wxTreeEvent& event)
{
    event.Veto();

    CGBDocument* doc = GetData();

    bool modified = false;
    if (!event.IsEditCancelled() && doc->IsLoaded()) {
        string new_name = ToStdString(event.GetLabel());
        if (new_name != doc->GetDescr().GetTitle()) {
            doc->SetDescr().SetTitle(new_name);
            doc->SetDirty (true);
            modified = true;
        }
    }
    UpdateLabel(treeCtrl);
    return modified;
}

void CProject::UpdateDisabledItems(wxTreeCtrl& treeCtrl)
{
    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eProjectFolder) {
            static_cast<CProjectFolder&>(*it).UpdateDisabledItems(treeCtrl);
        }
    }
}

void CProject::ProjectStateChanged(wxTreeCtrl& treeCtrl)
{
    treeCtrl.DeleteChildren(m_TreeItemId);

    UpdateLabel(treeCtrl);

    if (!GetData()->IsLoaded())
        return;

    Initialize(treeCtrl);
}

void CProject::UpdateProjectItems(wxTreeCtrl& treeCtrl)
{
    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eProjectFolder) {
            static_cast<CProjectFolder&>(*it).UpdateProjectItems(treeCtrl, *GetData());
            break;
        }
    }
    UpdateLabel(treeCtrl);
}

END_NCBI_SCOPE
