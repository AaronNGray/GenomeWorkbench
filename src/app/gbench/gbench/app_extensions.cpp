/*  $Id: app_extensions.cpp 33078 2015-05-21 15:06:52Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 */

#include <ncbi_pch.hpp>

#include "app_extensions.hpp"
#include "commands.hpp"

#include <gui/core/commands.hpp>
#include <gui/core/pt_utils.hpp>

#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/utils/extension_impl.hpp>

#include <gui/core/quick_launch_list.hpp>
#include <gui/core/recent_tool_list.hpp>

#include <wx/menu.h>


BEGIN_NCBI_SCOPE

/// Register CGBenchProjectTreeContextMenuContributor as an Extension
static CExtensionDeclaration
    decl1("project_tree_view::context_menu::item_cmd_contributor",
         new CGBenchProjectTreeContextMenuContributor());

/// Register CGBenchObjectCmdContributor as an Extension
static CExtensionDeclaration
    decl2("scoped_objects::cmd_contributor", new CGBenchObjectCmdContributor());

///////////////////////////////////////////////////////////////////////////////
/// CGBenchProjectTreeContextMenuContributor

IExplorerItemCmdContributor::TContribution
    CGBenchProjectTreeContextMenuContributor::GetMenu(wxTreeCtrl&, PT::TItems& items)
{
    int types = PT::GetItemTypes(items);

    wxMenu* menu = new wxMenu;

    // add commands applicable to Project Items
    if(types  ==  PT::eProjectItem)    {
        CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
        menu->Append(wxID_SEPARATOR, wxT("Top Actions"));
        cmd_reg.AppendMenuItem(*menu, eCmdRunTool);
        CQuickLaunchList::GetInstance()
            .UpdateMenu(*menu, eCmdQuickLaunchStart, eCmdQuickLaunchEnd, (size_t)-1);
        CRecentToolList::GetInstance()
            .UpdateMenu(*menu, eCmdRecentToolStart, eCmdRecentToolEnd, (size_t)-1);
        cmd_reg.AppendMenuItem(*menu, eCmdExportObject);
    }
    return IExplorerItemCmdContributor::TContribution(menu, (wxEvtHandler*)NULL);
}


string CGBenchProjectTreeContextMenuContributor::GetExtensionIdentifier() const
{
    return "gbench::item_cmd_contributor";
}


string CGBenchProjectTreeContextMenuContributor::GetExtensionLabel() const
{
    return "Contributes commands supported by GBENCH and applicable to Explorer items";
}

///////////////////////////////////////////////////////////////////////////////
/// CGBenchObjectCmdContributor

IObjectCmdContributor::TContribution
    CGBenchObjectCmdContributor::GetMenu(TConstScopedObjects& objects)
{
    wxMenu* menu = NULL;

    if( ! objects.empty()  &&  objects[0].scope) {
        CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();

        menu = new wxMenu;
        menu->Append(wxID_SEPARATOR, wxT("Top Actions"));
        cmd_reg.AppendMenuItem(*menu, eCmdOpenView);
        cmd_reg.AppendMenuItem(*menu, eCmdRunTool);
        CQuickLaunchList::GetInstance()
            .UpdateMenu(*menu, eCmdQuickLaunchStart, eCmdQuickLaunchEnd, (size_t)-1);
        CRecentToolList::GetInstance()
            .UpdateMenu(*menu, eCmdRecentToolStart, eCmdRecentToolEnd, (size_t)-1);
        cmd_reg.AppendMenuItem(*menu, eCmdExportObject);
    }
    return TContribution(menu, (wxEvtHandler*)NULL);
}


string CGBenchObjectCmdContributor::GetExtensionIdentifier() const
{
    return "gbench::scoped_objects::cmd_contributor";
}


string CGBenchObjectCmdContributor::GetExtensionLabel() const
{
    return "Contributes commands supported by GBENCH and applicable to scoped objects";
}


END_NCBI_SCOPE
