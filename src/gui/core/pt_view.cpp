/*  $Id: pt_view.cpp 33073 2015-05-20 20:48:29Z katargir $
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
#include <gui/core/pt_icon_list.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/treectrl.h>

BEGIN_NCBI_SCOPE

using namespace PT;

void CView::Initialize(wxTreeCtrl& treeCtrl, IProjectView& view, bool local)
{
    CPTIcons& icons = CPTIcons::GetInstance();

    const CViewTypeDescriptor&  descr = view.GetTypeDescriptor();
    string alias = descr.GetIconAlias();

    string label;
    if (local) {
        label = view.GetLabel(IProjectView::eId) 
                           + ": "
                           + view.GetLabel(IProjectView::eTypeAndContent);
    } else {
        label = view.GetClientLabel();
    }

    treeCtrl.SetItemImage(m_TreeItemId, icons.GetImageIndex(alias), wxTreeItemIcon_Normal);
    treeCtrl.SetItemText(m_TreeItemId, ToWxString(label));
}

END_NCBI_SCOPE
