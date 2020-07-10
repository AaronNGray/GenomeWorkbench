#ifndef GUI_WIDGETS_WX___COMMAND_REGISTRY__HPP
#define GUI_WIDGETS_WX___COMMAND_REGISTRY__HPP

/*  $Id: ui_command.hpp 42251 2019-01-22 13:56:07Z filippov $
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
 * File Description:
 *      CUICommand and CUICommandRegistry class declarations.
 */

#include <corelib/ncbistd.hpp>
#include <unordered_map>

#include <gui/gui_export.h>

#include <gui/utils/ui_object.hpp>
#include <gui/utils/command.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/menuitem.h>
#include <wx/event.h>
#include <wx/menu.h>

class wxMenu;
class wxMenuItem;
class wxMenuBar;
class wxToolBar;
class wxAuiToolBar;
class wxWindow;
class wxAcceleratorEntry;
class wxAcceleratorTable;


BEGIN_NCBI_SCOPE

struct SwxMenuItemRec;
struct SwxCommandRec;

///////////////////////////////////////////////////////////////////////////////
/// CUICommand
///
/// CUICommand represents a UI command that can be used in menus, toolbars,
/// menu bar and other similar UI controls.
///
/// - Command ID - an integer identifier that is used in GUI Tookit event
///   handling.
/// - Menu Label - a label for menu with access keys and shortcut.
///   Example: “Save as &Web Page\tCtrl+W”
/// For other fields see CUIObject.

class  NCBI_GUIWIDGETS_WX_EXPORT  CUICommand 
    : public CUIObject
{
public:
    /// a collection of wxWidget-style accelerator definitions
    typedef vector<wxAcceleratorEntry>  TAccelerators;

    CUICommand(
        TCmdID cmd_id,
        string menu_label,
        string name,
        string icon_alias,
        string hint = kEmptyStr,
        string description = kEmptyStr,
        string help_id = kEmptyStr,
        wxItemKind kind = wxITEM_NORMAL
    );
    CUICommand( const SwxCommandRec* rec );
    CUICommand( const CUICommand& cmd );

    ~CUICommand();

    /// the class inherits set/get methods from CUIObject

    void    SetCmdID(TCmdID cmd_id);

    void    AddAccelerator(int flags, int key_code); // see wxAcceleratorEntry constructor

    int     GetCmdID() const;
    const string& GetMenuLabel() const;
    const string& GetName() const { return CUIObject::GetLabel(); }

    const TAccelerators* GetAccelerators() const;

    wxMenuItem* AppendMenuItem( wxMenu& menu ) const;
    wxMenuItem* AppendMenuItem( wxMenu& menu, wxItemKind kind ) const;

protected:
    TCmdID  m_CmdID; // menu command ID
    string  m_MenuLabel;
    wxItemKind m_Kind;

    // one or more keyboard accelerators for the command
    auto_ptr<TAccelerators>  m_Accelerators;
};

struct NCBI_GUIWIDGETS_WX_EXPORT SwxCommandRec
{
    TCmdID m_CmdId;
    string m_MenuLabel;
    string m_Name;
    string m_IconAlias;
    string m_Hint;
    string m_Description;
    string m_HelpId;
    wxItemKind m_Kind;
};

///////////////////////////////////////////////////////////////////////////////
///
/// CUICommandRegistry is a centralized registry where all application commands
/// should be registered. The purpose of the registry is to provide completed,
/// consistent and non-redundant information on application commands, their
/// labels, tooltips, icons and other attributes.  Application components shall
/// use CUICommandRegistry for constructing menus, toolbars and other
/// command-based UI components. Components adding new commands to the
/// application shall register them in CUICommandRegistry.

class  NCBI_GUIWIDGETS_WX_EXPORT  CUICommandRegistry
{
public:
    /// the main instance associated with the application
    static CUICommandRegistry&  GetInstance();

    CUICommandRegistry(TCmdID start_id);
    virtual ~CUICommandRegistry();

    /// assumes ownership of the given object
    /// returns a command id (useful when registry is used for auto id assignment)
    int    RegisterCommand(CUICommand* cmd);

    /// create a CUICommand and registers it
    int    RegisterCommand(TCmdID cmd_id,
                           string menu_label,
                           string label,
                           string icon_alias,
                           string hint = kEmptyStr,
                           string description = kEmptyStr,
                           string help_id = kEmptyStr,
                           wxItemKind kind = wxITEM_NORMAL);

    /// returns CUICommand object or NULL
    const CUICommand* FindCommandByID( TCmdID cmd_id ) const;
    /// It is expensive function so use it wisely
    const CUICommand* FindCommandByName( string name ) const;

    /// @name Accelerators support
    /// @{
    /// add a new accelerators to the local table
    void    AddAccelerator(const wxAcceleratorEntry& entry);

    /// apply accumulated accelerators to the specifed frame
    void    ApplyAccelerators(wxWindow* frame);
    /// @}

    /// @name Menu and ToolBar support
    /// @{
    wxMenuItem* AppendMenuItem( wxMenu& menu, TCmdID cmd_id ) const;
    wxMenuItem* AppendMenuItem( wxMenu& menu, TCmdID cmd_id, wxItemKind kind ) const;
    void    AppendMenuItems(wxMenu& menu, const TCmdID* cmd_ids, int count) const;
    void    AppendMenuItems(wxMenu& menu, const vector<TCmdID> cmd_ids) const;

    /// create a menu from a static definition (see WX_*_MENU macros)
    wxMenu* CreateMenu( const SwxMenuItemRec* items );

    void    AppendTool(wxToolBar& tool_bar, TCmdID cmd_id);
    void    AppendTool(wxAuiToolBar& tool_bar, TCmdID cmd_id);
    /// @}
private:
    CUICommandRegistry(const CUICommandRegistry&);
    CUICommandRegistry& operator=(const CUICommandRegistry&);

protected:
    typedef std::unordered_map<int, CUICommand*> TIDToCommand;

    static CUICommandRegistry sm_TheRegistry;

    TCmdID  m_NextID; // the id of the next generated cmd id
    TIDToCommand m_IDToCommand;

    // support for Accelerators
    vector<wxAcceleratorEntry> m_AccelEntries;
};

struct NCBI_GUIWIDGETS_WX_EXPORT SwxMenuItemRec
{
    enum EMenuItemType {
        eDefaultItem = 0,
        eNormalItem, 
        eCheckItem,
        eRadioItem,
        eSubmenu,
        eSeparator
    };

    enum EMenuInternalOrExternal {
        eBoth,
        eInternal,
        eExternal
    };

    int         m_Type;
    const char* m_Label;
    TCmdID      m_CommandID;
    int         m_Kind;
    EMenuInternalOrExternal        m_InternalOrExternal;

    bool IsSubMenu() const
    {
        return m_Type == eSubmenu  &&  m_CommandID == eCmdNone;
    }
    bool IsSubMenuEnd() const
    {
        return m_Type == eSubmenu  &&  m_CommandID == eCmdInvalid;
    }
    bool IsMenuEnd() const
    {
        return m_Type == eSubmenu  &&  m_CommandID == eCmdMenuEnd;
    }

    bool IsInternal() const
    {
        return m_InternalOrExternal == eInternal;
    }

    bool IsExternal() const
    {
        return m_InternalOrExternal == eExternal;
    }

    bool IsBoth() const
    {
        return m_InternalOrExternal == eBoth;
    }
};


////////////////////////////////////////////////////////////////////////////////
/// New macros for defining menus for use with CUICommandRegistry
#define WX_DEFINE_MENU(name) \
    const SwxMenuItemRec name[] = { \
    WX_SUBMENU("Root")

#define WX_MENU_ITEM(cmd) \
    { SwxMenuItemRec::eDefaultItem, "", cmd, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eBoth },

#define WX_MENU_NORMAL_ITEM(cmd) \
    { SwxMenuItemRec::eNormalItem, "", cmd, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eBoth  },

#define WX_MENU_CHECK_ITEM(cmd) \
    { SwxMenuItemRec::eCheckItem, "", cmd, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eBoth },

#define WX_MENU_RADIO_ITEM(cmd) \
    { SwxMenuItemRec::eRadioItem, "", cmd, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eBoth },

#define WX_MENU_SEPARATOR() \
    { SwxMenuItemRec::eSeparator, "", eCmdNone, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eBoth },

#define WX_MENU_SEPARATOR_L(label) \
    { SwxMenuItemRec::eSeparator, label, eCmdNone, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eBoth },

#define WX_SUBMENU(label) \
    { SwxMenuItemRec::eSubmenu, label, eCmdNone, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eBoth },

#define WX_END_SUBMENU() \
    { SwxMenuItemRec::eSubmenu, "", eCmdInvalid, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eBoth },

#define WX_END_MENU() \
    { SwxMenuItemRec::eSubmenu, "", eCmdMenuEnd, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eBoth },  \
    };


#define WX_SUBMENU_INT(label) \
    { SwxMenuItemRec::eSubmenu, label, eCmdNone, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eInternal },

#define WX_MENU_ITEM_INT(cmd) \
    { SwxMenuItemRec::eDefaultItem, "", cmd, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eInternal },

#define WX_MENU_NORMAL_ITEM_INT(cmd) \
    { SwxMenuItemRec::eNormalItem, "", cmd, SwxMenuItemRec::eDefaultItem. SwxMenuItemRec::eInternal },

#define WX_MENU_CHECK_ITEM_INT(cmd) \
    { SwxMenuItemRec::eCheckItem, "", cmd, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eInternal },

#define WX_MENU_RADIO_ITEM_INT(cmd) \
    { SwxMenuItemRec::eRadioItem, "", cmd, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eInternal },

#define WX_MENU_SEPARATOR_INT() \
    { SwxMenuItemRec::eSeparator, "", eCmdNone, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eInternal },

#define WX_MENU_SEPARATOR_L_INT(label) \
    { SwxMenuItemRec::eSeparator, label, eCmdNone, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eInternal },


#define WX_SUBMENU_EXT(label) \
    { SwxMenuItemRec::eSubmenu, label, eCmdNone, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eExternal },

#define WX_MENU_ITEM_EXT(cmd) \
    { SwxMenuItemRec::eDefaultItem, "", cmd, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eExternal },

#define WX_MENU_NORMAL_ITEM_EXT(cmd) \
    { SwxMenuItemRec::eNormalItem, "", cmd, SwxMenuItemRec::eDefaultItem. SwxMenuItemRec::eExternal },

#define WX_MENU_CHECK_ITEM_EXT(cmd) \
    { SwxMenuItemRec::eCheckItem, "", cmd, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eExternal },

#define WX_MENU_RADIO_ITEM_EXT(cmd) \
    { SwxMenuItemRec::eRadioItem, "", cmd, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eExternal },

#define WX_MENU_SEPARATOR_EXT() \
    { SwxMenuItemRec::eSeparator, "", eCmdNone, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eExternal },

#define WX_MENU_SEPARATOR_L_EXT(label) \
    { SwxMenuItemRec::eSeparator, label, eCmdNone, SwxMenuItemRec::eDefaultItem, SwxMenuItemRec::eExternal },


///////////////////////////////////////////////////////////////////////////////
/// IUICommandContributor - contributes commands to the common command space.
///
/// Packages can declare their own set of commands to use inside package-related views.
/// Views (factories, actually) can declare their command sets also.
///
/// This interface is to be implemented by packages and view factories.
class IUICommandContributor
{
public:
    /// returns a vector of commands (takes ownership over CUICommand pointers)
    virtual vector<CUICommand*> GetCommands() = 0;

    /// returns a handler which processes commands from the set above
    /// not needed for views
    virtual wxEvtHandler* GetEventHandler() { return NULL; };

    virtual ~IUICommandContributor() {};
};

///////////////////////////////////////////////////////////////////////////////
/// IMenuContributor - contributes menu to Menu Service.
///
/// Objects such as Views can declare extension points that will be be extended
/// by classes implementing IMenuContributor.
///
/// This interface is to be implemented by packages and views.
class IMenuContributor
{
public:
    /// returns a menu (must be deleted by the caller)
    /// menu injections should follow a separator named "Contribs"
    virtual const wxMenu* GetMenu() {
        return CUICommandRegistry::GetInstance().CreateMenu( GetMenuDef() );
    };

    virtual ~IMenuContributor() {};

protected:
    /// simpler way to provide a static menu using macros defined
    /// earlier in this file
    virtual const SwxMenuItemRec* GetMenuDef() const { return NULL; };
};

///////////////////////////////////////////////////////////////////////////////
/// IToolBarContributor - a factory that produces toolbars.
/// Toolbars can be shared across views, so vie showld implement 
/// a method GetCompatibleToolBars( vector<string>& names )
/// to designate toolbars it handles.
/// 
/// This interface is to be implemented by packages and view factories.
class IToolBarContributor
{
public:
    /// returns the names of toolbars produced 
    virtual void GetToolBarNames( vector<string>& names ) = 0;

    /// creates a toolbar with the given name (must be deleted by the caller)
    virtual wxAuiToolBar* CreateToolBar( const string& name, wxWindow* parent ) = 0;

    virtual ~IToolBarContributor() {};
};


END_NCBI_SCOPE;


#endif  // GUI_WIDGETS_WX___COMMAND_REGISTRY__HPP

