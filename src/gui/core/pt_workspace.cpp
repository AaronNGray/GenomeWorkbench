/*  $Id: pt_workspace.cpp 41853 2018-10-25 13:57:22Z katargir $
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
#include <objects/gbproj/ProjectDescr.hpp>
#include <objects/gbproj/FolderInfo.hpp>

#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>

#include <gui/core/project_edit_dlg.hpp>
#include <gui/core/pt_project.hpp>
#include <gui/core/pt_workspace.hpp>
#include <gui/core/pt_utils.hpp>
#include <gui/core/pt_icon_list.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/treectrl.h>

BEGIN_NCBI_SCOPE

using namespace PT;

static wxString s_GetLabel(objects::CGBWorkspace* ws)
{
    return wxT("Projects");
    /*
    if (!ws)  return wxT("N/A");
    string name = ws->GetWorkspace().GetInfo().GetTitle();
    name = "Workspace \'" + name + "\'";
    if (ws->IsDirty())
        name += " (*)";
    return ToWxString(name);
    */
}

void CWorkspace::UpdateLabel (wxTreeCtrl& treeCtrl)
{
    treeCtrl.SetItemText(m_TreeItemId, s_GetLabel(GetData()));
}

void CWorkspace::Initialize(wxTreeCtrl& treeCtrl)
{
    CPTIcons& icons = CPTIcons::GetInstance();

    treeCtrl.SetItemImage(m_TreeItemId,
                          icons.GetImageIndex(CPTIcons::eWorkspaceClosedIcon),
                          wxTreeItemIcon_Normal);
    treeCtrl.SetItemImage(m_TreeItemId,
                          icons.GetImageIndex(CPTIcons::eWorkspaceOpenIcon),
                          wxTreeItemIcon_Expanded);

    UpdateLabel(treeCtrl);

    objects::CWorkspaceFolder& wsFolder = GetData()->SetWorkspace();

    size_t disabledItemsCount = 0;

    if (wsFolder.CanGetProjects()) {
        NON_CONST_ITERATE(objects::CWorkspaceFolder::TProjects, it, wsFolder.SetProjects()) {
            CGBDocument* doc = dynamic_cast<CGBDocument*>(it->GetPointer());
            if (doc) {
                PT::CProject* project = new CProject(CRef<CGBDocument>(doc));
                AppendChildItem(treeCtrl, *project);
                project->Initialize(treeCtrl);
                disabledItemsCount += project->CountDisabledItems(treeCtrl);
            }
        }
    }

    if (disabledItemsCount) {
        wxString label = wxString::Format(wxT("%u disabled item(s)"),(unsigned)disabledItemsCount);
        if (PT::sm_HideDisabledItems)
            label = wxT("Show ") + label;
        else
            label = wxT("Hide ") + label;

        PT::CHiddenItems* item = new PT::CHiddenItems(disabledItemsCount);
        InsertChildItem(treeCtrl, *item, 0);
        PT::CPTIcons& icons = PT::CPTIcons::GetInstance();
        treeCtrl.SetItemImage(item->GetTreeItemId(),
                              icons.GetImageIndex(PT::CPTIcons::eHiddenItemsIcon),
                              wxTreeItemIcon_Normal);
        treeCtrl.SetItemText(item->GetTreeItemId(), label);
    }

    treeCtrl.Expand(m_TreeItemId);
}

void CWorkspace::UpdateHiddenItems(wxTreeCtrl& treeCtrl)
{
    size_t disabledItemsCount = 0;
    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eProject)
            disabledItemsCount += static_cast<CProject&>(*it).CountDisabledItems(treeCtrl);
    }

    wxTreeItemId toDelete;

    if (disabledItemsCount == 0) {
        CChildIterator it(treeCtrl, *this);
        if (it) {
            if ((*it).GetType() == eHiddenItems) {
                toDelete = (*it).GetTreeItemId();
            }
        }
    } else {
        wxString label = wxString::Format(wxT("%u disabled item(s)"),(unsigned)disabledItemsCount);
        if (PT::sm_HideDisabledItems)
            label = wxT("Show ") + label;
        else
            label = wxT("Hide ") + label;

        CChildIterator it(treeCtrl, *this);

        if (!it || (*it).GetType() != eHiddenItems) {
            PT::CHiddenItems* item = new PT::CHiddenItems(disabledItemsCount);
            InsertChildItem(treeCtrl, *item, 0);

            PT::CPTIcons& icons = PT::CPTIcons::GetInstance();
            treeCtrl.SetItemImage(item->GetTreeItemId(),
                                  icons.GetImageIndex(PT::CPTIcons::eHiddenItemsIcon),
                                  wxTreeItemIcon_Normal);
            treeCtrl.SetItemText(item->GetTreeItemId(), label);
        } else {
            treeCtrl.SetItemText((*it).GetTreeItemId(), label);
        }
    }

    if (toDelete.IsOk())
        treeCtrl.Delete(toDelete);
}

void CWorkspace::UpdateViews(wxTreeCtrl& treeCtrl, CGBDocument* doc)
{
    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eProject) {
            CProject& project = static_cast<CProject&>(*it);
            if (doc) {
                if (doc == project.GetData()) {
                    project.UpdateViews(treeCtrl);
                    break;
                }
            } else {
                project.UpdateViews(treeCtrl);
            }
        }
    }
}

void CWorkspace::BeginLabelEdit(wxTreeCtrl& treeCtrl, wxTreeEvent& event)
{
    treeCtrl.SetItemText(m_TreeItemId, ToWxString(GetData()->GetDescr().GetTitle()));
}

bool CWorkspace::EndLabelEdit(wxTreeCtrl& treeCtrl, wxTreeEvent& event)
{
    event.Veto();

    objects::CGBWorkspace& ws = *GetData();

    bool modified = false;
    if (!event.IsEditCancelled()) {
        string new_name = ToStdString(event.GetLabel());
        if (new_name != ws.GetDescr().GetTitle()) {
            ws.SetDescr().SetTitle(new_name);
            ws.SetWorkspace().SetInfo().SetTitle(new_name);
            modified = true;
        }
    }
    UpdateLabel(treeCtrl);
    return modified;
}

void CWorkspace::UpdateDisabledItems(wxTreeCtrl& treeCtrl)
{
    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eProject) {
            static_cast<CProject&>(*it).UpdateDisabledItems(treeCtrl);
        }
    }

    UpdateHiddenItems(treeCtrl);
}

void CWorkspace::ProjectStateChanged(wxTreeCtrl& treeCtrl, CGBDocument& doc)
{
    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eProject) {
            CProject& project = static_cast<CProject&>(*it);
            if (project.GetData() == &doc) {
                project.ProjectStateChanged(treeCtrl);
                UpdateHiddenItems(treeCtrl);
                break;
            }
        }
    }
}

void CWorkspace::ProjectAdded(wxTreeCtrl& treeCtrl, CGBDocument& doc)
{
    PT::CProject* project = new CProject(CRef<CGBDocument>(&doc));
    AppendChildItem(treeCtrl, *project);
    project->Initialize(treeCtrl);

    treeCtrl.Expand(m_TreeItemId);

    UpdateHiddenItems(treeCtrl);
}

void CWorkspace::ProjectRemoved(wxTreeCtrl& treeCtrl, size_t id)
{
    wxTreeItemId toDelete;

    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eProject) {
            CProject& project = static_cast<CProject&>(*it);
            if (project.GetData()->GetId() == id) {
                toDelete = project.GetTreeItemId();
                break;
            }
        }
    }

    if (toDelete.IsOk()) {
        treeCtrl.Delete(toDelete);
        UpdateLabel(treeCtrl);
    }

    UpdateHiddenItems(treeCtrl);
}

void CWorkspace::UpdateProjectItems(wxTreeCtrl& treeCtrl, CGBDocument& doc)
{
    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eProject) {
            CProject& project = static_cast<CProject&>(*it);
            if (project.GetData() == &doc) {
                project.UpdateProjectItems(treeCtrl);
                UpdateHiddenItems(treeCtrl);
                break;
            }
        }
    }
}

void CWorkspace::UpdateProjectLabel(wxTreeCtrl& treeCtrl, CGBDocument& doc)
{
    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eProject) {
            CProject& project = static_cast<CProject&>(*it);
            if (project.GetData() == &doc) {
                project.UpdateLabel(treeCtrl);
                break;
            }
        }
    }
}

void CWorkspace::UpdateViewLabel(wxTreeCtrl& treeCtrl, CGBDocument& doc, IProjectView& view)
{
    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eProject) {
            CProject& project = static_cast<CProject&>(*it);
            if (project.GetData() == &doc) {
                project.UpdateViewLabel(treeCtrl, view);
                break;
            }
        }
    }
}

END_NCBI_SCOPE
