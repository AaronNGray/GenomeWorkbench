/*  $Id: menu_service_impl.cpp 41646 2018-09-05 19:01:17Z katargir $
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

#include <gui/framework/menu_service_impl.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>

#include <gui/widgets/wx/commands.hpp> //TODO

#include <wx/menu.h>
#include <wx/frame.h>
#include <wx/app.h>



BEGIN_NCBI_SCOPE

/// TODO
/*
current implementation does not have robust protection against NULL pointers.
This is done intentionaly, as it is not clear what policy we should implement –
throw exceptions, ignore incorrect inputs, return boolean values indicating success etc.
*/

///////////////////////////////////////////////////////////////////////////////
/// CMenuService

CMenuService::CMenuService()
:   m_Frame(NULL),
    m_CmdReg(NULL)
{
#ifdef __WXMAC__
    // disable automatic creation of Window menu on Mac
    // to avoid menu merging issues on Mac
    // wxWidgets 2.8.7: no merging code for system menus
    wxMenuBar::SetAutoWindowMenu(false);
    wxApp::s_macHelpMenuTitleName = wxT("&Help");
#endif
}


void CMenuService::SetFrame(wxFrame* frame, CUICommandRegistry& cmd_reg)
{
    _ASSERT(frame);
    m_Frame = frame;
    m_CmdReg = &cmd_reg;
}


void CMenuService::InitService()
{
}


void CMenuService::ShutDownService()
{
    /*for( size_t i = 0;  i < m_Contributors.size();  i++)    {
        IMenuContributor* contributor = m_Contributors[i];
        contributor->SetMenuService(NULL);
    }*/
    // do not delete Contributors, they have their own life management
    m_Contributors.clear();
}


void CMenuService::AddPendingCommand(int cmd)
{
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, cmd);
    m_Frame->GetEventHandler()->AddPendingEvent(event);
}

void CMenuService::ResetMenuBar()
{
    _ASSERT(m_CmdReg);

    // build combined menu
    wxMenu* menu = new wxMenu();
    for( size_t i = 0;  i < m_Contributors.size();  i++ )   {
        IMenuContributor& contributor = *m_Contributors[i];
        auto_ptr<const wxMenu> c_menu( contributor.GetMenu() );
        if( c_menu.get() ){
            Merge(*menu, *c_menu);
        }
    }

    _ASSERT(m_Frame);

    CleanupSeparators(*menu);

    // If this is the first time the menu is being reset, we need to create
    // the menubar and then, only after it has been updated, add it to the 
    // frame with SetMenuBar.  If we call SetMenuBar earlier, we get an 
    // error (an extra help menu) on mac (carbon).
    wxMenuBar* newBar = new wxMenuBar();

    wxMenuItemList& items = menu->GetMenuItems();
    ITERATE( wxMenuItemList, it, items ){
        wxMenuItem* item = *it;
        if( item->GetSubMenu() ){
            wxMenu*  sub_menu = CloneMenu( *item->GetSubMenu() );
            _ASSERT(sub_menu);

            wxString label = wxStripMenuCodes(item->GetItemLabelText(), wxStrip_Mnemonics);
            newBar->Append( sub_menu, label );
        }
    }

    // replace menu bar
    {
        unique_ptr<wxMenuBar> oldBar(m_Frame->GetMenuBar());
        m_Frame->SetMenuBar(newBar);
    }

    // On mac, deleting the menubar sets a default menubar to current.  WxWidgets would prefer, it seems
    // that we update our menubar with new menu items rather than replacing it as we currently do.
    // (at least on mac)
#ifdef __WXMAC__
    newBar->MacInstallMenuBar();
#endif

    delete menu;
}


void CMenuService::AddContributor(IMenuContributor* contributor)
{
    _ASSERT(contributor);

    if(contributor == NULL) {
        ERR_POST("CMenuService::AddContributor()  - NULL contributor.");
    } else {
        TContributors::const_iterator it =
            std::find(m_Contributors.begin(), m_Contributors.end(), contributor);
        if(it == m_Contributors.end())  {
            m_Contributors.push_back(contributor);
            //contributor->SetMenuService(this);
        } else {
            ERR_POST("CMenuService::AddContributor()  - contributor already added");
        }
    }
}


void CMenuService::RemoveContributor(IMenuContributor* contributor)
{
    _ASSERT(contributor);

    if(contributor == NULL) {
        ERR_POST("CMenuService::AddContributor()  - NULL contributor.");
    } else {
        TContributors::iterator it =
            std::find(m_Contributors.begin(), m_Contributors.end(), contributor);
        if(it == m_Contributors.end())  {
            ERR_POST("CMenuService::RemoveContributor()  - not found");
        } else {
            // do not delete Contributors, they have their own life management
            //contributor->SetMenuService(NULL);
            m_Contributors.erase(it);
        }
    }
}


END_NCBI_SCOPE
