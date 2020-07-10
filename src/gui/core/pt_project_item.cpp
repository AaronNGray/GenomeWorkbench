/*  $Id: pt_project_item.cpp 37251 2016-12-20 18:29:21Z katargir $
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

#include <objects/seq/Annotdesc.hpp>

#include <gui/core/pt_project_item.hpp>
#include <gui/core/pt_project.hpp>
#include <gui/core/pt_project_folder.hpp>
#include <gui/core/pt_utils.hpp>
#include <gui/core/fname_validator_imp.hpp>
#include <gui/core/folder_edit_dlg.hpp>
#include <gui/core/pt_icon_list.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/gui_object_info.hpp>

#include <wx/treectrl.h>

BEGIN_NCBI_SCOPE

using namespace PT;

static const char* kDlgRegPath = "Dialogs.PT.EditProjectItem";

static string s_ProjectItemGetDescr(const objects::CProjectItem& item)
{
    string descr;
    ITERATE(list< CRef<objects::CAnnotdesc> >, it, item.GetDescr()) {
        if ((**it).IsComment()){
            if (!descr.empty())
                descr += "\n";
            descr += (*it)->GetComment();
        }
    }
    return descr;
}

static void s_ProjectItemSetDescr(objects::CProjectItem& item, const string& decsr)
{
    list< CRef<objects::CAnnotdesc> >& annots = item.SetDescr();
    list<CRef<objects::CAnnotdesc> >::iterator it = annots.begin();

    for (; it != annots.end();) {
        if ((**it).IsComment())
            it = annots.erase(it);
        else
            ++it;
    }

    list<string> comment;
    NStr::Split(decsr, "\n\r", comment, NStr::fSplit_Tokenize);
    ITERATE (list<string>, iter, comment) {
        if (!iter->empty()) {
            CRef<objects::CAnnotdesc> annot(new objects::CAnnotdesc());
            annot->SetComment(*iter);
            item.SetDescr().push_back(annot);
        }
    }
}

void CProjectItem::Initialize(wxTreeCtrl& treeCtrl, CGBDocument& doc)
{
    wxColour cl_text = treeCtrl.GetForegroundColour();
    wxColour cl_back = treeCtrl.GetBackgroundColour();
    wxColour clr = GetData()->IsEnabled() ? cl_text : GetAverage(cl_text, cl_back, 0.5);
    treeCtrl.SetItemTextColour(m_TreeItemId, clr);

    PT::CPTIcons& icons = PT::CPTIcons::GetInstance();

    int index = -1;
    const CObject* obj = GetData()->GetObject();
    _ASSERT(obj);
    if (obj) {
        SConstScopedObject sco(obj, doc.GetScope());
        CIRef<IGuiObjectInfo> gui_info(CreateObjectInterface<IGuiObjectInfo>(sco, NULL));
        if( gui_info.IsNull() ) return;

        string alias = gui_info->GetIcon();
        if (!alias.empty())
            index = icons.GetImageIndex(alias);
    }

    if (index == -1)
        index = icons.GetImageIndex(CPTIcons::eProjectItemIcon);

    treeCtrl.SetItemImage(m_TreeItemId, index, wxTreeItemIcon_Normal);
    treeCtrl.SetItemText(m_TreeItemId, ToWxString(GetData()->GetLabel()));
}

bool CProjectItem::DoProperties(wxTreeCtrl& treeCtrl)
{
    CProject* project = GetProject(treeCtrl, *this);
    if (!project) return false;

    CGBDocument* doc = project->GetData();
    _ASSERT(doc);
    if (!doc) return false;

    objects::CProjectItem* pr_item = GetData();
    if (!pr_item) return false;

    CProjectFolder* pt_folder = dynamic_cast<CProjectFolder*>(GetParent(treeCtrl));
    _ASSERT(pt_folder);
    if (!pt_folder) return false;

    objects::CProjectFolder& parent_folder = *pt_folder->GetData();

    CFolderEditDlg dlg(NULL, wxID_ANY, wxT("Project Item Properties"));
    dlg.SetRegistryPath(kDlgRegPath);

    string label = pr_item->GetLabel(), descr = s_ProjectItemGetDescr(*pr_item);
    dlg.SetFolderName(ToWxString(label));

    dlg.SetDescr(wxString::FromUTF8(descr.c_str()));
    dlg.SetDataObject(pr_item->GetObject(), doc->GetScope());

    CPrjItemNameValidator validator(parent_folder, label);
    dlg.SetNameValidator(&validator);

    if (dlg.ShowModal() == wxID_OK)  {
        string new_label = ToStdString(dlg.GetFolderName());
        string new_descr = ToStdString(dlg.GetDescr());
        if (new_label != label || new_descr != descr) {
            new_label = doc->MakeUniqueItemLabel(new_label);
            pr_item->SetLabel(new_label);
            s_ProjectItemSetDescr(*pr_item, new_descr);
            treeCtrl.SetItemText(m_TreeItemId, ToWxString(new_label));

            if (new_label != label && pr_item->IsSetItem() && pr_item->GetItem().IsAnnot()) {
                pr_item->SetItem().SetAnnot().SetNameDesc(new_label);
                doc->Post(CRef<CEvent>(new CProjectViewEvent(doc->GetId(), CProjectViewEvent::eData)));
            }

            doc->SetDirty(true);
            project->UpdateLabel(treeCtrl);
            return true;
        }
    }
    return false;
}

void CProjectItem::BeginLabelEdit(wxTreeCtrl& treeCtrl, wxTreeEvent& event)
{
}

bool CProjectItem::EndLabelEdit(wxTreeCtrl& treeCtrl, wxTreeEvent& event)
{
    if (event.IsEditCancelled())
        return false;

    event.Veto();

    CProject* project = GetProject(treeCtrl, *this);
    if (!project) return false;

    CGBDocument* doc = project->GetData();
    _ASSERT(doc);
    if (!doc) return false;

    objects::CProjectItem* pr_item = GetData();
    if (!pr_item) return false;

    string new_name = ToStdString(event.GetLabel());
    string oldName = pr_item->GetLabel();
    if (oldName == new_name)
        return false;

    objects::CProjectFolder* parent_folder = 0;
    CProjectFolder* parent = dynamic_cast<CProjectFolder*>(GetParent(treeCtrl));
    if (parent)
        parent_folder = parent->GetData();
    if (!parent_folder)
        return false;

    CPrjItemNameValidator validator(*parent_folder, oldName);
    string err;
    if (!validator.IsValid(new_name, err)) {
        NcbiErrorBox(err);
    }
    else {
        pr_item->SetLabel(new_name);
        treeCtrl.SetItemText(m_TreeItemId, ToWxString(pr_item->GetLabel()));

        doc->SetDirty (true);
        project->UpdateLabel(treeCtrl);
        return true;
    }
    return false;
}

bool CProjectItem::DoRemove(wxTreeCtrl& treeCtrl)
{
    CProject* project = GetProject(treeCtrl, *this);
    if (!project) return false;

    CGBDocument* doc = project->GetData();
    _ASSERT(doc);
    if (!doc) return false;

    CItem* parent = GetParent(treeCtrl);
    if (!parent) return false;

    if (doc->RemoveProjectItem(GetData())) {
        wxTreeItemId toDelete = m_TreeItemId;
        project->UpdateLabel(treeCtrl);
        treeCtrl.Delete(toDelete);
        return true;
    }
    return false;
}

bool CProjectItem::CanPaste(wxTreeCtrl& treeCtrl) const
{
    const CItem* parent = GetParent(treeCtrl);
    if (parent)
        return parent->CanPaste(treeCtrl);
    return false;
}

bool CProjectItem::Paste(wxTreeCtrl& treeCtrl, PT::TItems& items, bool move)
{
    CItem* parent = GetParent(treeCtrl);
    if (parent)
        return parent->Paste(treeCtrl, items, move);
    return false;
}

END_NCBI_SCOPE
