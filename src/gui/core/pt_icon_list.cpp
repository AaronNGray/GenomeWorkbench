/*  $Id: pt_icon_list.cpp 32870 2015-04-30 18:47:53Z katargir $
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

#include <gui/core/pt_icon_list.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/wx/fileartprov.hpp>

BEGIN_NCBI_SCOPE

using namespace PT;

CPTIcons& CPTIcons::GetInstance()
{
    static CPTIcons instance;
    return instance;
}

CPTIcons::CPTIcons()
    : m_IconsImageList(16, 16, true, 0)
    , m_DataSourceIcon(-1)
    , m_FolderOpenIcon(-1)
    , m_FolderClosedIcon(-1)
    , m_WorkspaceClosedIcon(-1)
    , m_WorkspaceOpenIcon(-1)
    , m_ProjectItemIcon(-1)
    , m_DocIconDisabled(-1)
    , m_ProjectClosedIcon(-1)
    , m_ProjectOpenIcon(-1)
    , m_ViewerIcon(-1)
    , m_ViewIcon(-1)
    , m_HiddenItemsIcon(-1)
{
    wxFileArtProvider* provider = GetDefaultFileArtProvider();

    provider->RegisterFileAlias(wxT("tree::data_source"), wxT("data_source.png"));

    provider->RegisterFileAlias(wxT("tree::workspace_open_item"), wxT("workspace_open_item.png"));
    provider->RegisterFileAlias(wxT("tree::workspace_closed_item"), wxT("workspace_closed_item.png"));
    provider->RegisterFileAlias(wxT("tree::project_closed_item"), wxT("project_closed_item.png"));
    provider->RegisterFileAlias(wxT("tree::project_open_item"), wxT("project_open_item.png"));
    provider->RegisterFileAlias(wxT("tree::folder_open"), wxT("folder_open.png"));
    provider->RegisterFileAlias(wxT("tree::folder_closed"), wxT("folder_closed.png"));
    provider->RegisterFileAlias(wxT("tree::doc_item"), wxT("doc_item.png"));

    provider->RegisterFileAlias(wxT("tree::view_item"), wxT("view_item.png"));
    provider->RegisterFileAlias(wxT("tree::doc_item_disabled"),  wxT("doc_item_disabled.png"));
    provider->RegisterFileAlias(wxT("tree::viewer_item"), wxT("viewer_item.png"));

    provider->RegisterFileAlias(wxT("tree::bio_tree_item"), wxT("bio_tree_container.png"));
    provider->RegisterFileAlias(wxT("tree::assembly_item"), wxT("gc_assembly.png"));

    provider->RegisterFileAlias(wxT("tree::hidden_items"), wxT("bw_folder_closed.png"));

    m_DataSourceIcon      = x_AddIconToImageList("tree::data_source");
    m_WorkspaceOpenIcon   = x_AddIconToImageList("tree::workspace_open_item");
    m_WorkspaceClosedIcon = x_AddIconToImageList("tree::workspace_closed_item");
    m_ProjectClosedIcon   = x_AddIconToImageList("tree::project_closed_item");
    m_ProjectOpenIcon     = x_AddIconToImageList("tree::project_open_item");
    m_FolderOpenIcon      = x_AddIconToImageList("tree::folder_open");
    m_FolderClosedIcon    = x_AddIconToImageList("tree::folder_closed");
    m_ProjectItemIcon     = x_AddIconToImageList("tree::doc_item");

    m_ViewerIcon          = x_AddIconToImageList("tree::viewer_item");
    m_DocIconDisabled     = x_AddIconToImageList("tree::doc_item_disabled");
    m_ViewIcon            = x_AddIconToImageList("tree::view_item");

    m_HiddenItemsIcon     = x_AddIconToImageList("tree::hidden_items");
}

int CPTIcons::GetImageIndex(EIcon icon) const
{
    switch(icon) {
    case eDataSourceIcon :      return m_DataSourceIcon;
    case eFolderOpenIcon :      return m_FolderOpenIcon;
    case eFolderClosedIcon :    return m_FolderClosedIcon;
    case eWorkspaceClosedIcon : return m_WorkspaceClosedIcon;
    case eWorkspaceOpenIcon :   return m_WorkspaceOpenIcon;
    case eProjectItemIcon :     return m_ProjectItemIcon;
    case eDocIconDisabled :     return m_DocIconDisabled;
    case eProjectClosedIcon :   return m_ProjectClosedIcon;
    case eProjectOpenIcon :     return m_ProjectOpenIcon;
    case eViewerIcon :          return m_ViewerIcon;
    case eViewIcon :            return m_ViewIcon;
    case eHiddenItemsIcon :     return m_HiddenItemsIcon;
    }
    return -1;
}

int CPTIcons::GetImageIndex(const string& alias)
{
    map<string, int>::iterator it = m_AliasToIndex.find(alias);
    if (it == m_AliasToIndex.end()) {
        int index = x_AddIconToImageList(alias);
        if (index != -1)
            m_AliasToIndex[alias] = index;
        return index;
    } else {
        return it->second;
    }
}

int CPTIcons::x_AddIconToImageList(const string& alias)
{
    wxBitmap bmp = wxArtProvider::GetBitmap(ToWxString(alias));
    if (bmp.IsOk())
        return m_IconsImageList.Add(bmp);

    return -1;
}

END_NCBI_SCOPE
