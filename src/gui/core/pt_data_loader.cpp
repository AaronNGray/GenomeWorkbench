/*  $Id: pt_data_loader.cpp 39092 2017-07-27 17:18:17Z katargir $
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

#include <gui/core/pt_data_loader.hpp>
#include <gui/core/pt_project.hpp>
#include <gui/core/pt_utils.hpp>
#include <gui/core/pt_icon_list.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <objects/general/User_object.hpp>

#include <wx/treectrl.h>

BEGIN_NCBI_SCOPE

using namespace PT;

void CDataLoader::Initialize(wxTreeCtrl& treeCtrl)
{
    CPTIcons& icons = CPTIcons::GetInstance();
    treeCtrl.SetItemImage(m_TreeItemId,
                          icons.GetImageIndex(CPTIcons::eProjectItemIcon),
                          wxTreeItemIcon_Normal);

    wxColour cl_text = treeCtrl.GetForegroundColour();
    wxColour cl_back = treeCtrl.GetBackgroundColour();
    wxColour clr = GetData()->GetEnabled() ? cl_text : GetAverage(cl_text, cl_back, 0.5);
    treeCtrl.SetItemTextColour(m_TreeItemId, clr);

    string label;
    const objects::CUser_object& obj = GetData()->GetData();
    CConstRef<objects::CUser_field> fieldLabel = obj.GetFieldRef("UILabel");
    if (fieldLabel)
        label = fieldLabel->GetData().GetStr();

    if (label.empty())
        label = GetData()->GetLabel();

    treeCtrl.SetItemText(m_TreeItemId, wxString::FromUTF8(label.c_str()));
}

bool CDataLoader::DoRemove(wxTreeCtrl& treeCtrl)
{
    CProject* project = GetProject(treeCtrl, *this);
    if (!project) return false;

    CGBDocument* doc = project->GetData();
    _ASSERT(doc);
    if (!doc) return false;

    CItem* parent = GetParent(treeCtrl);
    if (!parent) return false;

    const string& label = GetData()->GetLabel();
    CRef<objects::CLoaderDescriptor> loader = doc->FindDataLoaderByLabel(label);
    if (!loader) return false;

    if (!doc->RemoveDataLoader(*loader)) {
        NcbiMessageBoxW(wxT("Sorry, Data Loaders can't be removed!\n")
                        wxT("Please close the corresponding views and try again!"));
    } else {
        wxTreeItemId toDelete = m_TreeItemId;
        doc->SetDirty (true);
        project->UpdateLabel(treeCtrl);
        treeCtrl.Delete(toDelete);
        return true;
    }
    return false;
}

END_NCBI_SCOPE
