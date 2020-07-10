/*  $Id: pt_project_folder.cpp 36583 2016-10-12 15:11:14Z evgeniev $
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
#include <objects/seq/Annotdesc.hpp>

#include <gui/core/pt_project_folder.hpp>
#include <gui/core/pt_project.hpp>
#include <gui/core/pt_project_item.hpp>
#include <gui/core/pt_workspace.hpp>
#include <gui/core/pt_utils.hpp>
#include <gui/core/fname_validator_imp.hpp>
#include <gui/core/folder_edit_dlg.hpp>

#include <gui/core/pt_icon_list.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/treectrl.h>

#include <serial/iterator.hpp>

BEGIN_NCBI_SCOPE

using namespace PT;

void CProjectFolder::Initialize(wxTreeCtrl& treeCtrl, CGBDocument& doc)
{
    CPTIcons& icons = CPTIcons::GetInstance();

    objects::CProjectFolder& folder = *GetData();


    treeCtrl.SetItemImage(m_TreeItemId,
                          icons.GetImageIndex(CPTIcons::eFolderOpenIcon),
                          wxTreeItemIcon_Normal);
    treeCtrl.SetItemImage(m_TreeItemId,
                          icons.GetImageIndex(CPTIcons::eFolderClosedIcon),
                          wxTreeItemIcon_Expanded);

    const objects::CFolderInfo& info = folder.GetInfo();
    wxString name = ToWxString(info.GetTitle());
    treeCtrl.SetItemText(m_TreeItemId, name);

    bool is_open = info.IsSetOpen() ? info.GetOpen() : false;

    if (folder.CanGetFolders()) {
        NON_CONST_ITERATE(objects::CProjectFolder::TFolders, it, folder.SetFolders()) {
            objects::CProjectFolder& sub_folder = **it;
            CProjectFolder* new_item = new CProjectFolder(CRef<objects::CProjectFolder>(&sub_folder));
            AppendChildItem(treeCtrl, *new_item);
            new_item->Initialize(treeCtrl, doc);
        }
    }

    if (folder.CanGetItems()) {
        NON_CONST_ITERATE(objects::CProjectFolder::TItems, it, folder.SetItems()) {
            objects::CProjectItem& prj_item = **it;
            if (!prj_item.IsEnabled()) {
                if (sm_HideDisabledItems)
                    continue;
            }
            CProjectItem* new_item = new CProjectItem(CRef<objects::CProjectItem>(&prj_item));
            AppendChildItem(treeCtrl, *new_item);
            new_item->Initialize(treeCtrl, doc);
        }
    }

    if (is_open)
        treeCtrl.Expand(m_TreeItemId);
    else
        treeCtrl.Collapse(m_TreeItemId);
}

static size_t s_CountDisabledItems(objects::CProjectFolder& folder)
{
    size_t count = 0;
    if (folder.CanGetItems()) {
        ITERATE(objects::CProjectFolder::TItems, it, folder.GetItems()) {
            if (!(*it)->IsEnabled())
                ++count;
        }
    }
    return count;
}

size_t CProjectFolder::CountDisabledItems(wxTreeCtrl& treeCtrl)
{
    size_t count = 0;
    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eProjectFolder)
            count += static_cast<CProjectFolder&>(*it).CountDisabledItems(treeCtrl);
    }
    count += s_CountDisabledItems(*GetData());
    return count;
}

void CProjectFolder::OnItemExpandedCollapsed(wxTreeCtrl& treeCtrl)
{
    objects::CProjectFolder& folder = *GetData();
    folder.SetInfo().SetOpen(treeCtrl.IsExpanded(m_TreeItemId));
}

static bool s_EditFolder(objects::CFolderInfo& info, const string& title, string reg_path,
                         bool readonly, IFNameValidator& validator)
{
    CFolderEditDlg dlg(NULL, wxID_ANY, ToWxString(title));
    dlg.SetRegistryPath(reg_path);

    dlg.SetReadOnly(readonly);
    dlg.SetNameValidator(&validator);

    dlg.SetFolderName(ToWxString(info.GetTitle()));

    string descr;
    if (info.CanGetComment()) {
        descr = info.GetComment();
    }

    if (info.CanGetAnnot()) {
        ITERATE (list< CRef<objects::CAnnotdesc> >, it, info.GetAnnot()) {
            if ((**it).IsComment()) {
                if (!descr.empty())
                    descr += "\n";
                descr += (*it)->GetComment();
            }
        }
    }

    dlg.SetDescr(ToWxString(descr));

    if(readonly)    {
        dlg.ShowModal();
        return true;
    } else {
        if(dlg.ShowModal() == wxID_OK) {
            info.SetTitle(ToStdString(dlg.GetFolderName()));
            descr = ToStdString(dlg.GetDescr());

            info.SetComment("");
            info.SetAnnot().clear();

            list<string> comment;
            NStr::Split(descr, "\n\r", comment, NStr::fSplit_Tokenize);

            ITERATE (list<string>, it, comment) {
                if (!it->empty()) {
                    if (it == comment.begin())
                        info.SetComment(*it);
                    else {
                        CRef<objects::CAnnotdesc> annot(new objects::CAnnotdesc());
                        annot->SetComment(*it);
                        info.SetAnnot().push_back(annot);
                    }
                }
            }
            return true;
        }
        return false;
    }
}

static const char* kDlgRegPath = "Dialogs.PT.EditFolder";

void CProjectFolder::DoNewFolder(wxTreeCtrl& treeCtrl)
{
    CProject* project = GetProject(treeCtrl, *this);
    if (!project) return;

    CGBDocument* doc = project->GetData();
    _ASSERT(doc);
    if (!doc) return;

    objects::CProjectFolder& parent_folder = *GetData();

    CFolderNameValidator<objects::CProjectFolder> validator(&parent_folder, "");

    CRef<objects::CProjectFolder> new_folder(new objects::CProjectFolder());
    new_folder->SetInfo().SetTitle("New Folder");
    new_folder->SetInfo().SetComment("");
    new_folder->SetInfo().SetOpen(true);

    if (s_EditFolder(new_folder->SetInfo(), "New Project Folder", kDlgRegPath, false, validator)) {
        new_folder->SetInfo().SetCreate_date().SetToTime(CTime(CTime::eCurrent));
        parent_folder.AddChildFolder(*new_folder);

        const objects::CFolderInfo& info = new_folder->GetInfo();
        wxString name = ToWxString(info.GetTitle());
        CProjectFolder* newFolder = new CProjectFolder(new_folder);
        size_t pos = 0;
        for (CChildIterator it(treeCtrl, *this); it; ++it) {
            if ((*it).GetType() != eProjectFolder)
                break;
            ++pos;
        }
        InsertChildItem(treeCtrl, *newFolder, pos);
        newFolder->Initialize(treeCtrl, *doc);

        parent_folder.SetInfo().SetOpen(true);
        treeCtrl.Expand(m_TreeItemId);

        doc->SetDirty (true);
        project->UpdateLabel(treeCtrl);
    }
}

bool CProjectFolder::DoProperties(wxTreeCtrl& treeCtrl)
{
    CProject* project = GetProject(treeCtrl, *this);
    if (!project) return false;

    CGBDocument* doc = project->GetData();
    _ASSERT(doc);
    if (!doc) return false;

    objects::CProjectFolder& folder = *GetData();

    objects::CProjectFolder* parent_folder = 0;
    CProjectFolder* parent = dynamic_cast<CProjectFolder*>(GetParent(treeCtrl));
    if (parent)
        parent_folder = parent->GetData();

    CFolderNameValidator<objects::CProjectFolder> validator(parent_folder, folder.GetInfo().GetTitle());
    bool readonly = (parent_folder == NULL);
    if (s_EditFolder(folder.SetInfo(), "Edit Folder", kDlgRegPath, readonly, validator) && !readonly) {
        treeCtrl.SetItemText(m_TreeItemId, ToWxString(folder.GetInfo().GetTitle()));
        doc->SetDirty (true);
        project->UpdateLabel(treeCtrl);
        return true;
    }
    return false;
}

void CProjectFolder::BeginLabelEdit(wxTreeCtrl& treeCtrl, wxTreeEvent& event)
{
    const CItem* parent_item = GetParent(treeCtrl);
    if (parent_item == 0 || parent_item->GetType() == eProject)
        event.Veto();
}

bool CProjectFolder::EndLabelEdit(wxTreeCtrl& treeCtrl, wxTreeEvent& event)
{
    if (event.IsEditCancelled())
        return false;

    event.Veto();

    CProject* project = GetProject(treeCtrl, *this);
    if (!project) return false;

    CGBDocument* doc = project->GetData();
    _ASSERT(doc);
    if (!doc) return false;

    objects::CProjectFolder& pr_folder = *GetData();

    string new_name = ToStdString(event.GetLabel());
    string oldName = pr_folder.GetInfo().GetTitle();
    if (oldName == new_name)
        return false;

    objects::CProjectFolder* parent_folder = 0;
    CProjectFolder* parent = dynamic_cast<CProjectFolder*>(GetParent(treeCtrl));
    if (parent)
        parent_folder = parent->GetData();
    if (!parent_folder)
        return false;

    CFolderNameValidator<objects::CProjectFolder> validator(parent_folder, oldName);
    string err;
    if (!validator.IsValid(new_name, err)) {
        NcbiErrorBox(err);
    }
    else {
        pr_folder.SetInfo().SetTitle(new_name);
        treeCtrl.SetItemText(m_TreeItemId, ToWxString(new_name));

        doc->SetDirty (true);
        project->UpdateLabel(treeCtrl);
        return true;
    }
    return false;
}

bool CProjectFolder::CanDoRemove(wxTreeCtrl& treeCtrl) const
{
    const CItem* parent = GetParent(treeCtrl);
    return parent != 0 && parent->GetType() == eProjectFolder;
}

static bool s_DeleteFolder(CGBDocument& doc, objects::CProjectFolder& folder)
{
    vector<objects::CProjectItem*> projItems;
    for (CTypeIterator<objects::CProjectItem> it(folder); it; ++it) {
        projItems.push_back(&*it);
    }
    if (!projItems.empty())
        doc.RemoveProjectItems(projItems);
    doc.SetData().RemoveProjectFolder(folder.GetId());
    return true;
}

bool CProjectFolder::DoRemove(wxTreeCtrl& treeCtrl)
{
    CProject* project = GetProject(treeCtrl, *this);
    if (!project) return false;

    CItem* parent = GetParent(treeCtrl);
    if (!parent) return false;

    CGBDocument* doc = project->GetData();
    _ASSERT(doc);
    if (!doc) return false;

    if (s_DeleteFolder(*doc, *GetData())) {
        wxTreeItemId toDelete = m_TreeItemId;
        doc->SetDirty (true);
        project->UpdateLabel(treeCtrl);
        treeCtrl.Delete(toDelete);
        return true;
    }
    return false;
}

bool CProjectFolder::CanCopyToClipboard(wxTreeCtrl& treeCtrl) const
{
    const CItem* parent = GetParent(treeCtrl);
    return parent != 0 && parent->GetType() == eProjectFolder;
}

bool CProjectFolder::CanCutToClipboard(wxTreeCtrl& treeCtrl) const
{
    return CanCopyToClipboard(treeCtrl);
}

bool CProjectFolder::CanPaste(wxTreeCtrl& treeCtrl) const
{
    CProject* project = GetProject(treeCtrl, *this);
    if (!project) return false;
	return true;
}

static CRef<objects::CProjectItem> s_ProjectItemClone(const objects::CProjectItem& item)
{
    CRef<objects::CProjectItem> new_item(new objects::CProjectItem());
    new_item->SetLabel(item.GetLabel());
    new_item->SetDescr().assign(item.GetDescr().begin(), item.GetDescr().end());

    CSerialObject* obj = const_cast<CSerialObject*>(item.GetObject());
    new_item->SetObject(*obj);

    if (!item.IsEnabled())
        new_item->SetDisabled(true);

    return new_item;
}

static void s_ProjectFolderClone(const objects::CProjectFolder& folder, CGBDocument& doc, objects::CProjectFolder& dstFolder)
{
    CRef<objects::CProjectFolder> new_folder(new objects::CProjectFolder());
    objects::CFolderInfo& info = new_folder->SetInfo();
    info.SetTitle(folder.GetInfo().GetTitle());
    info.SetComment(folder.GetInfo().GetComment());
    info.SetCreate_date().SetToTime(CTime(CTime::eCurrent));

    dstFolder.AddChildFolder(*new_folder);

    vector<objects::CProjectItem*> items;
    ITERATE(objects::CProjectFolder::TItems, it, folder.GetItems()) {
        CRef<objects::CProjectItem> i = s_ProjectItemClone(**it);
        doc.AddItem(*i, *new_folder);
        if (i->IsEnabled())
            items.push_back(i);
    }
    doc.AttachProjectItems(items);

    ITERATE(objects::CProjectFolder::TFolders, it, folder.GetFolders()) {
        s_ProjectFolderClone(**it, doc, *new_folder);
    }
}

static bool s_PasteProjectFolder(CGBDocument& src, objects::CProjectFolder& srcFolder,
                                 CGBDocument& dst, objects::CProjectFolder& dstFolder, bool cut)
{
    if (src.GetId() == dst.GetId()) {
        if (srcFolder.GetId() == dstFolder.GetId()) {
            string err = "Cannot paste folder \"" + srcFolder.GetInfo().GetTitle() + "\" on itself.";
            NcbiErrorBox(err);
            return false;
        }
        if (srcFolder.FindChildFolderById(dstFolder.GetId())) {
            string err = "Cannot paste folder \"" + srcFolder.GetInfo().GetTitle();
            err += "\". Destination folder is a child of the source folder.";
            NcbiErrorBox(err);
            return false;
        }
    }

    if (src.GetId() == dst.GetId() && cut) {
        CRef<objects::CProjectFolder>  f = src.SetData().RemoveProjectFolder(srcFolder.GetId());
        if (!f) return false;
        dstFolder.AddChildFolder(*f);
    } else {
        s_ProjectFolderClone(srcFolder, dst, dstFolder);
        if (cut) {
            s_DeleteFolder(src, srcFolder);
        }
    }
    return true;
}

bool CProjectFolder::Paste(wxTreeCtrl& treeCtrl, PT::TItems& items, bool move)
{
    if (items.empty()) return false;

    CProject* project = GetProject(treeCtrl, *this);
    if (!project) return false;

    CGBDocument* doc = project->GetData();
    _ASSERT(doc);
    if (!doc) return false;

    objects::CProjectFolder& folder = *GetData();

    bool modified = false;
    set<CProject*> modProjects;

    typedef map<CGBDocument*, vector<objects::CProjectItem*> > TDocItems;
    TDocItems toDelete;

    NON_CONST_ITERATE(PT::TItems, it, items) {
        CItem& item = **it;
        CProject* srcProject = GetProject(treeCtrl, item);
        if (!srcProject) continue;

        CGBDocument* srcDoc = srcProject->GetData();
        _ASSERT(srcDoc);
        if (!srcDoc) continue;

        switch(item.GetType()) {
        case eProjectFolder:
            {{
                CProjectFolder& srcFolder = static_cast<CProjectFolder&>(item);
                if (s_PasteProjectFolder(*srcDoc, *srcFolder.GetData(), *doc, folder, move)) {
                    if (move) {
                        srcDoc->SetDirty(true);
                        modProjects.insert(srcProject);
                        treeCtrl.Delete(srcFolder.GetTreeItemId());
                    }
                    modified = true;
                }
            }}
            break;
        case eProjectItem:
            {{
                CProjectItem& srcItem = static_cast<CProjectItem&>(item);
                objects::CProjectItem* projItem = srcItem.GetData();

                if (srcDoc == doc && move) {
                    int itemId = projItem->GetId();
                    objects::CProjectFolder* srcFolder = srcDoc->SetData().FindProjectItemFolder(itemId);
                    if (srcFolder && &folder != srcFolder) {
                        folder.AddChildItem(*projItem);
                        srcFolder->RemoveChildItem(itemId);
                        treeCtrl.Delete(srcItem.GetTreeItemId());
                        modified = true;
                    }
                } else {
                    CRef<objects::CProjectItem> new_item = s_ProjectItemClone(*projItem);
                    doc->AddItem(*new_item, folder);
                    if (new_item->IsEnabled())
                        doc->AttachProjectItem(new_item);

                    if (move) {
                        toDelete[srcDoc].push_back(projItem);
                        modProjects.insert(srcProject);
                    }
                    modified = true;
                }
            }}
            break;
        }
    }

    ITERATE(TDocItems, it, toDelete) {
        it->first->RemoveProjectItems(it->second);
        it->first->SetDirty(true);
    }

    if (modified) {
        treeCtrl.DeleteChildren(m_TreeItemId);
        Initialize(treeCtrl, *doc);

        folder.SetInfo().SetOpen(true);
        treeCtrl.Expand(m_TreeItemId);

        doc->SetDirty (true);
        modProjects.insert(project);
    }

    ITERATE(set<CProject*>, it, modProjects)
        if (*it) (*it)->UpdateLabel(treeCtrl);

    CItem* item = project->GetParent(treeCtrl);
    if (item && item->GetType() == eWorkspace) {
        CWorkspace& ws = dynamic_cast<CWorkspace&>(*item);
        ws.UpdateHiddenItems(treeCtrl);
    }

    return modified;
}

void CProjectFolder::UpdateDisabledItems(wxTreeCtrl& treeCtrl)
{
    CProject* project = GetProject(treeCtrl, *this);
    if (!project) return;

    CGBDocument* doc = project->GetData();
    _ASSERT(doc);
    if (!doc) return;

    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eProjectFolder) {
            static_cast<CProjectFolder&>(*it).UpdateDisabledItems(treeCtrl);
        }
    }
    objects::CProjectFolder& folder = *GetData();

    if (s_CountDisabledItems(folder) == 0)
        return;

    if (sm_HideDisabledItems) {
        vector<wxTreeItemId> toDelete;
        for (CChildIterator it(treeCtrl, *this); it; ++it) {
            if ((*it).GetType() == eProjectItem) {
                CProjectItem& item = static_cast<CProjectItem&>(*it);
                if (!item.GetData()->IsEnabled())
                    toDelete.push_back((*it).GetTreeItemId());
            }
        }
        ITERATE(vector<wxTreeItemId>, it, toDelete)
            treeCtrl.Delete(*it);
    } else {
        if (!folder.CanGetItems())
            return;

        objects::CProjectFolder::TItems& items = folder.SetItems();
        objects::CProjectFolder::TItems::iterator it2 = items.begin();

        for (CChildIterator it(treeCtrl, *this); it && it2 != items.end(); ++it) {
            if ((*it).GetType() == eProjectItem) {
                CProjectItem& item = static_cast<CProjectItem&>(*it);
                item.SetData(*it2);
                item.Initialize(treeCtrl, *doc);
                ++it2;
            }
        }

        for (; it2 != items.end(); ++it2) {
            objects::CProjectItem& prjItem = **it2;
            CProjectItem* new_item = new CProjectItem(CRef<objects::CProjectItem>(&prjItem));
            AppendChildItem(treeCtrl, *new_item);
            new_item->Initialize(treeCtrl, *doc);
        }
    }
}

void CProjectFolder::UpdateProjectItems(wxTreeCtrl& treeCtrl, CGBDocument& doc)
{
    CProject* project = GetProject(treeCtrl, *this);
    if (!project) return;

    objects::CProjectFolder& folder = *GetData();

    for (CChildIterator it(treeCtrl, *this); it; ++it) {
        if ((*it).GetType() == eProjectFolder) {
            static_cast<CProjectFolder&>(*it).UpdateProjectItems(treeCtrl, doc);
        }
    }

    if (!folder.CanGetItems())
        return;
    
    objects::CProjectFolder::TItems& items = folder.SetItems();
    objects::CProjectFolder::TItems::iterator it2 = items.begin();
    if (sm_HideDisabledItems) {
        while (it2 != items.end() && !(*it2)->IsEnabled())
            ++it2;
    }

    CChildIterator it(treeCtrl, *this);
    for (; it && it2 != items.end(); ++it) {
        if ((*it).GetType() == eProjectItem) {
            CProjectItem& item = static_cast<CProjectItem&>(*it);
            item.SetData(*it2);
            item.Initialize(treeCtrl, doc);
            ++it2;
            if (sm_HideDisabledItems) {
                while (it2 != items.end() && !(*it2)->IsEnabled())
                    ++it2;
            }
        }
    }

    vector<wxTreeItemId> toDelete;

    for (; it; ++it) {
        if ((*it).GetType() == eProjectItem)
            toDelete.push_back((*it).GetTreeItemId());
    }

    ITERATE(vector<wxTreeItemId>, it, toDelete)
        treeCtrl.Delete(*it);

    for (; it2 != items.end();) {
        objects::CProjectItem& prjItem = **it2;
        CProjectItem* new_item = new CProjectItem(CRef<objects::CProjectItem>(&prjItem));
        AppendChildItem(treeCtrl, *new_item);
        new_item->Initialize(treeCtrl, doc);

        ++it2;
        if (sm_HideDisabledItems) {
            while (it2 != items.end() && !(*it2)->IsEnabled())
                ++it2;
        }
    }
}

END_NCBI_SCOPE
