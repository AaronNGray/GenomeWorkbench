/*  $Id: ui_command.cpp 42251 2019-01-22 13:56:07Z filippov $
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

#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/artprov.h>
#include <wx/accel.h>
#include <wx/menu.h>
#include <wx/frame.h>
#include <wx/toolbar.h>
#include <wx/aui/auibar.h>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CUICommand
CUICommand::CUICommand(TCmdID cmd_id,
                       string menu_label,
                       string name,
                       string icon_alias,
                       string hint,
                       string description,
                       string help_id,
                       wxItemKind kind)
:   CUIObject(name, icon_alias, hint, description, help_id),
    m_CmdID(cmd_id),
    m_MenuLabel(menu_label),
    m_Kind(kind)
{
    _ASSERT( ! m_MenuLabel.empty()  ||  m_Kind == wxITEM_SEPARATOR );
}


CUICommand::CUICommand(const CUICommand& cmd)
:   CUIObject(cmd),
    m_CmdID(cmd.m_CmdID),
    m_MenuLabel(cmd.m_MenuLabel),
    m_Kind(cmd.m_Kind)
{
    _ASSERT( ! m_MenuLabel.empty());

    if(cmd.m_Accelerators.get())    {
        // make a copy of the container
        m_Accelerators.reset(new TAccelerators(*cmd.m_Accelerators.get()));
    }
}


CUICommand::~CUICommand()
{
}


void CUICommand::SetCmdID( TCmdID cmd_id )
{
    m_CmdID = cmd_id;

    TAccelerators* accels = m_Accelerators.get();
    if(accels )    {
        // update command id in accelerators
        for(  size_t  i = 0;  i < accels->size();  i++  )   {
            wxAcceleratorEntry& entry = (*accels)[i];
            entry.Set(entry.GetFlags(), entry.GetKeyCode(), cmd_id);
        }
    }
}


void CUICommand::AddAccelerator(int flags, int key_code)
{
    if( ! m_Accelerators.get()) {
        m_Accelerators.reset(new TAccelerators);
    }
    m_Accelerators->push_back(wxAcceleratorEntry(flags, key_code, m_CmdID));
}


int CUICommand::GetCmdID() const
{
    return m_CmdID;
}


const string& CUICommand::GetMenuLabel() const
{
    return m_MenuLabel;
}


const CUICommand::TAccelerators* CUICommand::GetAccelerators() const
{
    return m_Accelerators.get();
}


wxMenuItem* CUICommand::AppendMenuItem( wxMenu& menu ) const
{
    return AppendMenuItem( menu, m_Kind );
}


wxMenuItem* CUICommand::AppendMenuItem( wxMenu& menu, wxItemKind kind ) const
{
    _ASSERT( ! m_MenuLabel.empty() );

    if( kind == wxITEM_RADIO ){
        if( menu.GetMenuItemCount() == 0 ){
            UseDefaultMarginWidth( menu );
        }
    }

    wxMenuItem* item = 
        wxMenuItem::New(&menu, m_CmdID, ToWxString(m_MenuLabel), ToWxString(m_Hint), kind);

    // If we do not set the first item as owner draw
    // We get draw problems with mixed owner/system draw on MSWIN
    // What happens on other platforms not checked
    // plus
    // Attempt to avoid bug in wxWidgets 2.9
    // causing improper error log posts
    // 'SetMenuItemInfo' failed with error 0x000005b0 (a menu item was not found.)

#if wxUSE_OWNER_DRAWN
    item->SetOwnerDrawn();
#endif

    if( ! m_IconAlias.empty() ){
        wxBitmap image = wxArtProvider::GetBitmap( ToWxString(m_IconAlias) );
        item->SetBitmap( image );
        SetMenuItemMarginWidth( item, &image );
    }

    menu.Append(item);

    return item;
}


///////////////////////////////////////////////////////////////////////////////
/// CUICommandRegistry

CUICommandRegistry CUICommandRegistry::sm_TheRegistry(10000);

static const int kDefHashSize = 111;

CUICommandRegistry& CUICommandRegistry::GetInstance()
{
    return sm_TheRegistry;
}


CUICommandRegistry::CUICommandRegistry(TCmdID start_id)
:   m_NextID(start_id),
    m_IDToCommand(kDefHashSize)
{
}


CUICommandRegistry::~CUICommandRegistry()
{
    NON_CONST_ITERATE(TIDToCommand, it, m_IDToCommand)  {
        delete it->second;
    }
    m_IDToCommand.clear();
}


int CUICommandRegistry::RegisterCommand(CUICommand* cmd)
{
    _ASSERT(cmd);
    if(cmd) {
        TCmdID cmd_id = cmd->GetCmdID();

        if(cmd_id == eCmdInvalid)   {
            // need to assign id
            cmd->SetCmdID(m_NextID);
            ++m_NextID;
        } else {
            _ASSERT(cmd_id > 0  &&  cmd_id < 0xFFFF); // wxWidgets requirement

            TIDToCommand::const_iterator it = m_IDToCommand.find(cmd_id);
            bool exists = (it != m_IDToCommand.end());
            if(exists)  {
                ERR_POST("CUICommandRegistry::RegisterCommand() - attempty to register command " <<
                         cmd->GetLabel() << " with id" << cmd_id << " that is already in use - ");
                ERR_POST("\n\tpreviously registered as " << it->second->GetMenuLabel());
                _ASSERT(false);
                delete cmd;
                return eCmdInvalid;
            }
        }
        m_IDToCommand[cmd_id] = cmd;

        // register accelerators
        const CUICommand::TAccelerators* accels = cmd->GetAccelerators();
        if(accels)  {
            ITERATE(CUICommand::TAccelerators, it, *accels)   {
                AddAccelerator(*it);
            }
        }
        return cmd_id;
    }

    return eCmdInvalid;
}


int CUICommandRegistry::RegisterCommand(TCmdID cmd_id,
                                        string menu_label,
                                        string label,
                                        string icon_alias,
                                        string hint,
                                        string description,
                                        string help_id,
                                        wxItemKind kind)
{
    CUICommand* cmd = new CUICommand(cmd_id, menu_label, label, icon_alias,
                                     hint, description, help_id, kind);
    int id = RegisterCommand(cmd);
    if(id == eCmdInvalid)   {
        delete cmd; // not registered - delete command
    }
    return id;
}


const CUICommand* CUICommandRegistry::FindCommandByID( TCmdID cmd_id ) const
{
    TIDToCommand::const_iterator it = m_IDToCommand.find( cmd_id );
    return (it == m_IDToCommand.end()) ? NULL : it->second;
}

const CUICommand* CUICommandRegistry::FindCommandByName( string name ) const
{
    ITERATE( TIDToCommand, it, m_IDToCommand ){
        if( it->second->GetName() == name ){
            return it->second;
        }
    }

    return NULL;
}

void CUICommandRegistry::AddAccelerator(const wxAcceleratorEntry& entry)
{
    // accumulate entries in m_AccelEntries
    m_AccelEntries.push_back(entry);
}


void CUICommandRegistry::ApplyAccelerators(wxWindow* frame)
{
    if(frame) {
        // disconnect Frame from m_AccelEntries
        frame->SetAcceleratorTable(wxNullAcceleratorTable);

        int n = (int)m_AccelEntries.size();
        wxAcceleratorEntry* entries = &*m_AccelEntries.begin();

        wxAcceleratorTable accel_table( n, entries );

        frame->SetAcceleratorTable( accel_table ); // apply the new table
    } else {
        _ASSERT(false);
        ERR_POST("CUICommandRegistry::ApplyAccelerators() - frame = NULL, cannot apply accelerators");
    }
}


wxMenuItem* CUICommandRegistry::AppendMenuItem( wxMenu& menu, TCmdID cmd_id ) const
{
    // -1 == wxITEM_SEPARATOR is used here as default value
    // (see below).
    return AppendMenuItem( menu, cmd_id, (wxItemKind)-1 );
}


wxMenuItem* CUICommandRegistry::AppendMenuItem( wxMenu& menu, TCmdID cmd_id, wxItemKind kind ) const
{
    if( cmd_id == wxID_SEPARATOR ){
        return menu.AppendSeparator();

    } else {
        const CUICommand* cmd = FindCommandByID( cmd_id );
        if( cmd ){
            if( kind == -1 ){ // wxITEM_SEPARATOR
                return cmd->AppendMenuItem( menu );
            } else {
                return cmd->AppendMenuItem( menu, kind );
            }
        } else {
            ERR_POST("CUICommandRegistry::AppendMenuItem() cannot find command " << cmd_id);
            _ASSERT(false);
            return NULL;
        }
    }
}


void CUICommandRegistry::AppendMenuItems(wxMenu& menu, const TCmdID* cmd_ids, int count) const
{
    for(  int i = 0;  i < count;  i++  )   {
        TCmdID cmd_id = cmd_ids[i];
        AppendMenuItem(menu, cmd_id);
    }
}


void CUICommandRegistry::AppendMenuItems(wxMenu& menu, const vector<TCmdID> cmd_ids) const
{
    for(  size_t i = 0;  i < cmd_ids.size();  i++   )   {
        TCmdID cmd_id = cmd_ids[i];
        AppendMenuItem(menu, cmd_id);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// creates hierarchy of wxMenuItem objects from the array of SwxMenuItemRec descriptors
wxMenu* CUICommandRegistry::CreateMenu( const SwxMenuItemRec* items )
{
    if( !items ) return NULL;

    bool insideNcbi = RunningInsideNCBI();

    auto visF = [insideNcbi](const SwxMenuItemRec* item)->bool
        { return ((insideNcbi && item->IsInternal()) || (!insideNcbi && item->IsExternal()) || item->IsBoth()); };
    
    typedef pair<wxMenu*, wxString> TPair;

    vector<TPair>  path; //
    const SwxMenuItemRec* p_rec = items;

    while( p_rec ){
        if( p_rec->IsSubMenu() ){
            // create submenu and add to the path
            wxString s = ToWxString(p_rec->m_Label);
            if (path.empty()) {
                path.push_back(TPair(new wxMenu, s));
            }
            else {
                wxMenu* submenu = path.back().first;
                if (submenu && visF(p_rec))
                    path.push_back(TPair(new wxMenu, s));
                else
                    path.push_back(TPair(nullptr, s));
            }

        } else if( p_rec->IsSubMenuEnd() ){
            // end current Submenu and eject it from path
            TPair pair = path.back();
            path.pop_back(); // eject

            if (pair.first) {
                wxMenu* submenu = path.back().first;
                submenu->Append(wxID_ANY, pair.second, pair.first);
            }

            _ASSERT( !path.empty() );

        } else if( p_rec->IsMenuEnd() ){
            // the last menu - return
            TPair pair = path.back();
            path.pop_back();

            _ASSERT( path.empty() ); // all submenus were closed

            return pair.first;

        } else if( p_rec->m_Type == SwxMenuItemRec::eSeparator ){
            // add separator
            wxMenu* submenu = path.back().first;
            if (submenu && visF(p_rec)) {
                wxMenuItem* item =
                    wxMenuItem::New(submenu, wxID_SEPARATOR, ToWxString(p_rec->m_Label));
                // If we do not set the fisrt item as owner draw
                // We get draw problems with mixed owner/system draw on MSWIN
                // What happens on other platforms not checked
#if wxUSE_OWNER_DRAWN
                item->SetOwnerDrawn();
#endif
                submenu->Append(item);
            }
        } else if( p_rec->m_Type == SwxMenuItemRec::eDefaultItem ){
            // add command item
            wxMenu* submenu = path.back().first;
            if (submenu && visF(p_rec))
                AppendMenuItem( *submenu, p_rec->m_CommandID );

        } else if( p_rec->m_Type == SwxMenuItemRec::eNormalItem ){
            // add command item
            wxMenu* submenu = path.back().first;
            if (submenu && visF(p_rec))
                AppendMenuItem(*submenu, p_rec->m_CommandID, wxITEM_NORMAL);

        } else if( p_rec->m_Type == SwxMenuItemRec::eCheckItem ){
            // add command item
            wxMenu* submenu = path.back().first;
            if (submenu && visF(p_rec))
                AppendMenuItem(*submenu, p_rec->m_CommandID, wxITEM_CHECK);

        } else if( p_rec->m_Type == SwxMenuItemRec::eRadioItem ){
            // add command item
            wxMenu* submenu = path.back().first;
            if (submenu && visF(p_rec))
                AppendMenuItem(*submenu, p_rec->m_CommandID, wxITEM_RADIO);

        } else {
            // unexpected
            _ASSERT(false);
        }
        p_rec++;
    }

    return  NULL;
}


void CUICommandRegistry::AppendTool(wxToolBar& tool_bar, TCmdID cmd_id)
{
    TIDToCommand::iterator it = m_IDToCommand.find(cmd_id);
    if(it != m_IDToCommand.end())   {
        const CUICommand* cmd = it->second;

        wxBitmap image;
        const string& alias = cmd->GetIconAlias();
        if( ! alias.empty())    {
            image = wxArtProvider::GetBitmap(ToWxString(alias));
        }
        tool_bar.AddTool(cmd->GetCmdID(),
                         wxT(""),
                         image,
                         ToWxString(cmd->GetHint()));

        const string& descr = cmd->GetDescription();
        if( ! descr.empty())   {
            tool_bar.SetToolLongHelp(cmd_id, ToWxString(descr));
        }
    } else {
        ERR_POST("CUICommandRegistry::AppendTool() - command with id " <<
                 cmd_id << " is not registered.");
        _ASSERT(false);
    }
}

void CUICommandRegistry::AppendTool(wxAuiToolBar& tool_bar, TCmdID cmd_id)
{
    TIDToCommand::iterator it = m_IDToCommand.find(cmd_id);
    if(it != m_IDToCommand.end())   {
        const CUICommand* cmd = it->second;

        wxBitmap image;
        const string& alias = cmd->GetIconAlias();
        if( ! alias.empty())    {
            image = wxArtProvider::GetBitmap(ToWxString(alias));
        }
        tool_bar.AddTool(cmd->GetCmdID(),
                         wxT(""),
                         image,
                         ToWxString(cmd->GetHint()));

        const string& descr = cmd->GetDescription();
        if( ! descr.empty())   {
            tool_bar.SetToolLongHelp(cmd_id, ToWxString(descr));
        }
    } else {
        ERR_POST("CUICommandRegistry::AppendTool() - command with id " <<
                 cmd_id << " is not registered.");
        _ASSERT(false);
    }
}



END_NCBI_SCOPE
