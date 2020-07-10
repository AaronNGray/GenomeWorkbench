/*  $Id: pt_data_source.cpp 33073 2015-05-20 20:48:29Z katargir $
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

#include <gui/core/pt_data_source.hpp>
#include <gui/core/pt_icon_list.hpp>
#include <gui/core/ui_data_source_service.hpp>
#include <gui/utils/ui_object.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/treectrl.h>

BEGIN_NCBI_SCOPE

using namespace PT;

void CDataSource::Initialize(wxTreeCtrl& treeCtrl, IUIDataSource& ds)
{
    CPTIcons& icons = CPTIcons::GetInstance();

    const IUIObject& descr = ds.GetDescr();
    wxString label = ToWxString(descr.GetLabel());
    string alias = descr.GetIconAlias();
    int index = icons.GetImageIndex(alias);

    treeCtrl.SetItemImage(m_TreeItemId, index, wxTreeItemIcon_Normal);
    treeCtrl.SetItemText(m_TreeItemId, label);
}

END_NCBI_SCOPE
