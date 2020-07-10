/*  $Id: object_contrib.cpp 18488 2008-12-11 02:10:27Z voronov $
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
* Authors: Yury Voronov
*
* File Description:
*
*/

#include <ncbi_pch.hpp>

#include <gui/widgets/data/object_contrib.hpp>

#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/utils/extension_impl.hpp>

#include <wx/menu.h>
#include <wx/event.h>


BEGIN_NCBI_SCOPE

void AddCommandsForScopedObjects(
    wxMenu& aMenu,
    vector<wxEvtHandler*>& handlers,
    const string& anExtPointId,
    TConstScopedObjects& objects,
    CUICommandRegistry* aCmdReg
){
    if( objects.empty() ){
        return;
    }

    if( aCmdReg == NULL ){
        aCmdReg = &CUICommandRegistry::GetInstance();
    }

    // get contributed menus
    vector< CIRef<IObjectCmdContributor> > contributors;
    GetExtensionAsInterface( anExtPointId, contributors );

    // Merge contributed menus into the main Menu
    for( size_t i = 0; i < contributors.size(); i++ ){
        IObjectCmdContributor& obj = *contributors[i];

        IObjectCmdContributor::TContribution contrib = obj.GetMenu( objects );
        wxMenu* obj_menu = contrib.first;
        if( obj_menu ){
            Merge( aMenu, *obj_menu );
            delete obj_menu;
        }
        wxEvtHandler* handler = contrib.second;
        if( handler ){
            handlers.push_back( handler );
        }
    }
}

WX_DEFINE_MENU(kContextMenuBackbone)
    WX_MENU_SEPARATOR_L("Top Actions")
    WX_MENU_SEPARATOR_L("Zoom")
    WX_MENU_SEPARATOR_L("Actions")
    WX_MENU_SEPARATOR_L("Edit")
    WX_MENU_SEPARATOR_L("Settings")
WX_END_MENU()

wxMenu* CreateContextMenuBackbone()
{
    return CUICommandRegistry::GetInstance().CreateMenu( kContextMenuBackbone );
}


END_NCBI_SCOPE

