#ifndef GUI_WIDGETS___DATA___OBJECT_CONTRIB__HPP
#define GUI_WIDGETS___DATA___OBJECT_CONTRIB__HPP

/*  $Id: object_contrib.hpp 18488 2008-12-11 02:10:27Z voronov $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software / database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software / database is freely available
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
#include <corelib/ncbistd.hpp>

#include <gui/utils/extension.hpp>

#include <gui/objutils/objects.hpp>

class wxMenu;
class wxEvtHandler;

BEGIN_NCBI_SCOPE

class CUICommandRegistry;

///////////////////////////////////////////////////////////////////////////////
/// This Extension Point allows external components to add commands that
/// can be applied to scoped CObjects.
/// The Extensions shall implement IObjectCmdContributor interface.
/// This extension point can be used by various UI components as a source of
/// commands for context menus.

#define EXT_POINT__SCOPED_OBJECTS__CMD_CONTRIBUTOR "scoped_objects::cmd_contributor"


///////////////////////////////////////////////////////////////////////////////
/// IObjectCmdContributor - contributes commands applicable to scoped objects.

class IObjectCmdContributor
{
public:
    typedef pair<wxMenu*, wxEvtHandler*> TContribution;
    /// returns a menu with commands applicable to the objects and optional
    /// command handler for the commands
    /// caller assumes ownership of the returned objects
    virtual TContribution GetMenu( TConstScopedObjects& objects ) = 0;

    virtual ~IObjectCmdContributor() {};
};

///////////////////////////////////////////////////////////////////////////////
/// Helper function for adding commands provided by contributors to the menu.
NCBI_GUIWIDGETS_DATA_EXPORT void AddCommandsForScopedObjects(
    wxMenu& aMenu,
    vector<wxEvtHandler*>& handlers,
    const string& anExtPointId,
    TConstScopedObjects& objects,
    CUICommandRegistry* aCmdReg = NULL
);

NCBI_GUIWIDGETS_DATA_EXPORT wxMenu* CreateContextMenuBackbone();


END_NCBI_SCOPE

#endif /// GUI_WIDGETS___DATA___OBJECT_CONTRIB__HPP
