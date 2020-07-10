#ifndef GUI_UTILS___MENU_ITEM__HPP
#define GUI_UTILS___MENU_ITEM__HPP

/*  $Id: menu_item.hpp 21023 2010-03-10 01:12:14Z voronov $
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

/** @addtogroup GUI_UTILS
*
* @{
*/

/// @file menu_item.hpp

#include <corelib/ncbistd.hpp>
#include <corelib/ncbi_tree.hpp>
#include <gui/gui.hpp>
#include <gui/utils/command.hpp>

BEGIN_NCBI_SCOPE

struct SMenuItemRecRec;

////////////////////////////////////////////////////////////////////////////////
/// CMenuItem - represents a menu items in IMenu-style menus.

class NCBI_GUIUTILS_EXPORT CMenuItem
{
public:
    /// Type of menu item
    enum    EType   {
        eItem, // command item (includes "checkbox" and "radio" items)
        eSubmenu,
        eSeparator
    };
    /// State and subtype of menu item
    enum   EState  {
        eDefault    = 0,
        eDisabled   = 0x1, /// item is disabled (visible but inactive)
        eCheckItem  = 0x2, /// "check-box" item
        eRadioItem  = 0x4, /// "radio" item
        eSet        = 0x8  /// indicates that chekbox is checked or radio item is selected
    };
protected:
    typedef CTreeNode<CMenuItem*>   TItemNode;
public:
    /// creates a separator item
    CMenuItem();

    /// creates a submenu item
    CMenuItem(const string& label, const string& image_alias = "");

    /// creates a command item
    CMenuItem(const string& label, TCmdID cmd, const string& image_alias = "",
              const string& tooltip = "", int state = eDefault);

    /// generic constructor, can be used to create any type of menuitem
    CMenuItem(EType type, const string& label = "", TCmdID cmd = eCmdNone,
              const string& image_alias = "", const string& tooltip = "",
              int state = eDefault);

    /// copy contsructor - copies attributes but not subitems
    CMenuItem(const CMenuItem& item);

    virtual ~CMenuItem();

    void    Init(EType type, const string& label = "", TCmdID cmd = eCmdNone,
                 const string& image_alias = "", const string& tooltip = "",
                 int state = eDefault);

    void    InitPopup(const string& label, const string& image_alias = "",
                      const string& tooltip = "");
    void    InitItem(const string& label, TCmdID cmd, const string& image_alias = "",
                     const string& tooltip = "", int state = eDefault);
    void    InitSeparator();

    bool    Equal(const CMenuItem& item) const;

    /// clones item and its subitems
    CMenuItem*  Clone() const;

    EType   GetType()   const;
    void    SetType(EType type);
    bool    IsItem()    const;
    bool    IsSubmenu() const;
    bool    IsSeparator() const;

    const string&   GetLabel() const;
    void    SetLabel(const string& label);

    const   TCmdID&  GetCommand()    const;
    void    SetCommand(TCmdID cmd);

    bool            HasImage()  const;
    const string&   GetImageAlias() const;
    void    SetImageAlias(const string& image_alias);

    const string&   GetTooltip() const;
    void    SetTooltip(const string& tooltip);

    int     GetState()  const;
    void    SetState(int state)  ;

    bool    IsEnabled() const;
    void    Enable(bool b_en);

    bool    IsCheckType()   const;
    bool    IsChecked() const;
    void    SetCheck(bool b_set);

    bool    IsRadioType()   const;
    bool    IsRadioSelected() const;
    void    SelectRadio(bool b_set);

    bool    IsValid() const;

    bool    IsEnabledItem() const   {   return IsEnabled()  &&  IsItem();   }
    bool    IsEnabledSubmenu() const   {   return IsEnabled()  &&  IsSubmenu();   }

    /// merges menu tree represented by "item" into menu tree represented by "this"
    void    Merge(const CMenuItem& item);

public:
    typedef TItemNode::TNodeList_I  TChildItem_I;
    typedef TItemNode::TNodeList_CI TChildItem_CI;

    /// @name Operations with submenus
    /// @{
    CMenuItem*  GetParent();
    const   CMenuItem*  GetParent() const;

    CMenuItem*  AddSubItem(CMenuItem* item);

    /// Adds Separator
    CMenuItem*  AddSeparator();
    CMenuItem*  AddSeparator(const string& label);
        /// it is recommended that labels start with '-'

    /// Adds Submenu
    CMenuItem*  AddSubMenu(const string& Label,  const string& image_alias = "");

    /// Adds command subitem
    CMenuItem*  AddSubItem(const string& label, TCmdID cmd, const string& image_alias = "",
                           const string& tooltip = "", int state = eDefault);

    CMenuItem*  InsertSubItem(TChildItem_I it, CMenuItem* item);
    void    RemoveItem(TChildItem_I it);

    void    DestroyAllSubNodes();

    bool    IsSubmenuEmpty();

    TChildItem_I    SubItemsBegin();
    TChildItem_I    SubItemsEnd();
    TChildItem_CI   SubItemsBegin() const;
    TChildItem_CI   SubItemsEnd() const;

    CMenuItem*  FindEqualSubItem(const CMenuItem& item);
    const CMenuItem*  FindEqualSubItem(const CMenuItem& item) const;

    TChildItem_I    FindSubItem(const CMenuItem& item);
    TChildItem_CI   FindSubItem(const CMenuItem& item) const;

    TChildItem_I    FindSubItem(const string& label);
    TChildItem_CI   FindSubItem(const string& label) const;

    TChildItem_I    FindSubItem(EType type, const string& label);
    TChildItem_CI   FindSubItem(EType type, const string& label) const;
    /// @}

protected:
    void    x_MergeItems(CMenuItem& target, const CMenuItem& new_item);
    TChildItem_I x_FindSubItem(TChildItem_I it_begin, TChildItem_I it_end,
                               const string& label, bool skip_named_groups);
    void    x_InsertInUnnamedGroup(CMenuItem& item);

    inline void    x_SetState(int mask, bool b_en)
    {
        if(b_en)    {
            m_State |= mask;
        } else {
            m_State &= ~mask;
        }
    }
    inline void    x_SetState(int mask, int values)
    {
        m_State &= ~mask;
        m_State |= values;
    }
private:
    EType   m_Type;
    string  m_Label;
    TCmdID  m_CommandID;
    string  m_ImageAlias;
    string  m_Tooltip;
    int     m_State;

    TItemNode   m_ItemNode;
};

////////////////////////////////////////////////////////////////////////////////
/// SMenuItemRec
struct NCBI_GUIUTILS_EXPORT SMenuItemRec
{
    int         m_Type;
    const char* m_Label;
    TCmdID      m_CommandID;
    const char* m_ImageAlias;
    const char* m_Tooltip;
    int         m_State;

    bool    IsSubMenu()  const
    {
        return m_Type == CMenuItem::eSubmenu  &&  m_CommandID == eCmdNone;
    }
    bool    IsSubMenuEnd()  const
    {
        return m_Type == CMenuItem::eSubmenu  &&  m_CommandID == eCmdInvalid;
    }
    bool    IsMenuEnd()  const
    {
        return m_Type == CMenuItem::eSubmenu  &&  m_CommandID == eCmdMenuEnd;
    }
};

/// creates CMenuItem hierarchy from an array of SMenuItemRecRec
NCBI_GUIUTILS_EXPORT  CMenuItem*  CreateMenuItems(const SMenuItemRec* items);

#define DEFINE_MENU(name) \
    const SMenuItemRec name[] = \
    {   SUBMENU("Root")

#define MENU_ITEM(cmd, label) \
{ CMenuItem::eItem, label, cmd, "", "", CMenuItem::eDefault},

#define MENU_ITEM_T(cmd, label, tooltip) \
    { CMenuItem::eItem, label, cmd, "", tooltip, CMenuItem::eDefault},

#define MENU_ITEM_IM(cmd, label, image) \
    { CMenuItem::eItem, label, cmd, image, "", CMenuItem::eDefault},

#define MENU_ITEM_IM_T(cmd, label, image, tooltip) \
    { CMenuItem::eItem, label, cmd, image, tooltip, CMenuItem::eDefault},

#define MENU_ITEM_RADIO(cmd, label) \
    { CMenuItem::eItem, label, cmd, "", "", CMenuItem::eRadioItem},

#define MENU_ITEM_CHECK(cmd, label) \
    { CMenuItem::eItem, label, cmd, "", "", CMenuItem::eCheckItem},

#define MENU_SEPARATOR() \
    { CMenuItem::eSeparator, "", eCmdNone, "", "", CMenuItem::eDefault},

#define MENU_SEPARATOR_L(label) \
    { CMenuItem::eSeparator, label, eCmdNone, "", "", CMenuItem::eDefault},

#define SUBMENU(label) \
    { CMenuItem::eSubmenu, label, eCmdNone, "", "", CMenuItem::eDefault},

#define SUBMENU_IM(label, image) \
    { CMenuItem::eSubmenu, label, eCmdNone, image, "", CMenuItem::eDefault},

#define END_SUBMENU() \
    { CMenuItem::eSubmenu, "", eCmdInvalid, "", "", CMenuItem::eDefault},

#define END_MENU() \
    { CMenuItem::eSubmenu, "", eCmdMenuEnd, "", "", CMenuItem::eDefault},  \
    };


END_NCBI_SCOPE

/* @} */

#endif  // GUI_UTILS___MENU_ITEM__HPP
