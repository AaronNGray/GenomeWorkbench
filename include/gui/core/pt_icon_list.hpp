#ifndef GUI_CORE___PT_ICON_LIST__HPP
#define GUI_CORE___PT_ICON_LIST__HPP

/*  $Id: pt_icon_list.hpp 33073 2015-05-20 20:48:29Z katargir $
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
 */

#include <corelib/ncbiobj.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/imaglist.h>
#include <wx/string.h>

BEGIN_NCBI_SCOPE

namespace PT
{

class CPTIcons
{
public:
    static CPTIcons& GetInstance();

    enum EIcon {
        eDataSourceIcon,
        eFolderOpenIcon,
        eFolderClosedIcon,
        eWorkspaceClosedIcon,
        eWorkspaceOpenIcon,
        eProjectItemIcon,
        eDocIconDisabled,
        eProjectClosedIcon,
        eProjectOpenIcon,
        eViewerIcon,
        eViewIcon,
        eHiddenItemsIcon
    };

    int GetImageIndex(EIcon icon) const;
    int GetImageIndex(const string& alias);

    wxImageList& GetImageList() { return m_IconsImageList; }

private:
    CPTIcons();

    int  x_AddIconToImageList(const string& alias);

    wxImageList m_IconsImageList;

    map<string, int> m_AliasToIndex;

    int m_DataSourceIcon;
    int m_FolderOpenIcon;
    int m_FolderClosedIcon;
    int m_WorkspaceClosedIcon;
    int m_WorkspaceOpenIcon;
    int m_ProjectItemIcon;
    int m_DocIconDisabled;
    int m_ProjectClosedIcon;
    int m_ProjectOpenIcon;
    int m_ViewerIcon;
    int m_ViewIcon;
    int m_HiddenItemsIcon;
};

} //namespace PT

END_NCBI_SCOPE

#endif  /// GUI_CORE___PT_ICON_LIST__HPP
