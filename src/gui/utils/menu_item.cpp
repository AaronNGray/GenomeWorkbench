/*  $Id: menu_item.cpp 39417 2017-09-21 20:20:16Z katargir $
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

#include <corelib/ncbistd.hpp>

#include <gui/utils/menu_item.hpp>


BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
// CMenuItem

CMenuItem::CMenuItem()
    : m_Type(eItem),
      m_CommandID(0),
      m_State(0),
      m_ItemNode(this)
{
    Init(eSeparator);
}

CMenuItem::CMenuItem(const string& label, const string& image_alias)
    : m_Type(eItem),
      m_CommandID(0),
      m_State(0),
      m_ItemNode(this)
{
    Init(eSubmenu, label, eCmdNone, image_alias);
}

CMenuItem::CMenuItem(const string& label, TCmdID cmd, const string& image_alias,
                     const string& tooltip, int state)
    : m_Type(eItem),
      m_CommandID(0),
      m_State(0),
      m_ItemNode(this)
{
    Init(eItem, label, cmd, image_alias, tooltip, state);
}

CMenuItem::CMenuItem(EType type, const string& label, TCmdID cmd,
                     const string& image_alias, const string& tooltip, int state)
    : m_Type(eItem),
      m_CommandID(0),
      m_State(0),
      m_ItemNode(this)
{
    Init(type, label, cmd, image_alias, tooltip, state);
}

///copy contsructor - copes just attributes, not the subitems
CMenuItem::CMenuItem(const CMenuItem& item)
    : m_Type(eItem),
      m_CommandID(0),
      m_State(0),
      m_ItemNode(this)
{
    Init(item.m_Type, item.m_Label, item.m_CommandID, item.m_ImageAlias,
         item.m_Tooltip, item.m_State);
}

CMenuItem::~CMenuItem()
{
    DestroyAllSubNodes();
}

void    CMenuItem::Init(EType type, const string& label, TCmdID cmd,
                        const string& image_alias, const string& tooltip, int state)
{
    m_Type = type;
    m_Label = label;
    m_CommandID = cmd;
    m_ImageAlias = image_alias;
    m_Tooltip = tooltip;
    m_State = state;
}

void    CMenuItem::InitPopup(const string& label, const string& image_alias,
                             const string& tooltip)
{
    Init(eSubmenu, label, eCmdNone, image_alias, tooltip);
}

void    CMenuItem::InitItem(const string& label, TCmdID cmd, const string& image_alias,
                            const string& tooltip, int state)
{
    Init(eItem, label, cmd, image_alias, tooltip, state);
}

void    CMenuItem::InitSeparator()
{
    Init(eSeparator);
}

bool    CMenuItem::Equal(const CMenuItem& item) const
{
    return m_Type == item.m_Type  &&  m_Label == item.m_Label
            &&  m_CommandID == item.m_CommandID  && m_ImageAlias == item.m_ImageAlias;
}

CMenuItem*  CMenuItem::Clone() const
{
    CMenuItem* p_clone = new CMenuItem(*this);

    for(TChildItem_CI    it = SubItemsBegin();  it != SubItemsEnd(); it++ )  {
       CMenuItem* p_subitem = (*it)->GetValue();
       p_clone->AddSubItem(p_subitem->Clone());
    }

    return p_clone;
}

CMenuItem::EType   CMenuItem::GetType()   const
{
    return m_Type;
}

void    CMenuItem::SetType(EType type)
{
    m_Type = type;
}

bool    CMenuItem::IsItem()    const
{
    return m_Type == eItem;
}

bool    CMenuItem::IsSubmenu() const
{
    return m_Type == eSubmenu;
}

bool    CMenuItem::IsSeparator() const
{
    return m_Type == eSeparator;
}

const string&   CMenuItem::GetLabel() const
{
    return m_Label;
}

void    CMenuItem::SetLabel(const string& label)
{
    m_Label = label;
}

const TCmdID&  CMenuItem::GetCommand()    const
{
    return m_CommandID;
}

void    CMenuItem::SetCommand(TCmdID cmd)
{
    m_CommandID = cmd;
}

bool            CMenuItem::HasImage()  const
{
    return m_ImageAlias.size() > 0;
}

const string&   CMenuItem::GetImageAlias() const
{
    return m_ImageAlias;
}

void    CMenuItem::SetImageAlias(const string& image_alias)
{
    m_ImageAlias = image_alias;
}

const string&   CMenuItem::GetTooltip() const
{
    return m_Tooltip;
}

void    CMenuItem::SetTooltip(const string& tooltip)
{
    m_Tooltip = tooltip;
}

int  CMenuItem::GetState()  const
{
    return m_State;
}

void    CMenuItem::SetState(int state)
{
    m_State = state;
}

bool    CMenuItem::IsEnabled() const
{
    return (m_State & eDisabled) == 0;
}

void    CMenuItem::Enable(bool b_en)
{
    x_SetState(eDisabled, ! b_en);
}

bool    CMenuItem::IsCheckType()   const
{
    return (m_State & eCheckItem) != 0;
}

bool    CMenuItem::IsChecked() const
{
    return (m_State & eCheckItem)  &&  m_State & eSet;
}

void    CMenuItem::SetCheck(bool b_set)
{
    x_SetState(eCheckItem | eRadioItem, eCheckItem);
    x_SetState(eSet, b_set);
}

bool    CMenuItem::IsRadioType()   const
{
    return (m_State & eRadioItem) != 0;
}

bool    CMenuItem::IsRadioSelected() const
{
   return m_State & eRadioItem  &&  m_State & eSet;
}

void    CMenuItem::SelectRadio(bool b_set)
{
    x_SetState(eCheckItem | eRadioItem, eRadioItem);
    x_SetState(eSet, b_set);
}

bool    CMenuItem::IsValid() const
{
    bool b_check = IsCheckType();
    bool b_radio = IsRadioType();

    if(b_check  &&  b_radio)    {
        return false; // cannot be both at the same time
    } else if((b_check  ||  b_radio)  &&  ! IsItem())   {
        return false;
    } /*else if((m_State & eHideIfEmpty)  &&  ! IsSubmenu())  {
        return false;
    }*/ else if(! IsSeparator()  &&  m_Label.size()==0)   {
        return false; // label must not be empty
    }
    return true;
}

static const char* kCannotMerge = "CMenuItem::Merge() cannot merge item ";

/// Menu Item trees are merged based on item's labels. Labels specify unique path
/// to an item (similarly to paths in file systems).
void    CMenuItem::Merge(const CMenuItem& item)
{
    // this and "item" must be submenus with the same label
    if(GetLabel() == item.GetLabel())   {
        if(IsSubmenu()  &&  item.IsSubmenu())   {
            // iterating subitems of the given menu
            bool named_group = false;
            TChildItem_I grp_begin = SubItemsBegin();
            TChildItem_I grp_end = SubItemsEnd();

            for( TChildItem_CI it = item.SubItemsBegin(); it != item.SubItemsEnd(); it++ )   {
                const CMenuItem* sub_item = (*it)->GetValue();
                const string& label = sub_item->GetLabel();

                if(sub_item->IsSeparator()) {
                    // separator indicates end of the current group and
                    // beginning of a new one
                    named_group = ! label.empty();
                    if(named_group)   {
                        // find this group in this item
                        grp_begin = FindSubItem(eSeparator, label);
                        if(grp_begin != SubItemsEnd())    {
                            // find the end of the group
                            grp_begin++;
                            for(  grp_end = grp_begin;  grp_end != SubItemsEnd();  ++grp_end)   {
                                const CMenuItem* it_item = (*grp_end)->GetValue();
                                if(it_item->IsSeparator())  {
                                    break;
                                }
                            }
                        } else {
                            //the group does not exist - create a new one
                            AddSeparator(label);
                            grp_begin = grp_end = SubItemsEnd();
                        }
                    } else {
                        grp_begin = SubItemsBegin();
                        grp_end = SubItemsEnd();
                    }
                } else {
                    // not a separator - item that can be merged
                    // find candidate within a group
                    TChildItem_I it_target = x_FindSubItem(grp_begin, grp_end, label, named_group);
                    if(it_target == grp_end)   {
                        // this is a new item - add it
                        if(named_group) {
                            InsertSubItem(it_target, sub_item->Clone());
                        } else {
                            // may need to insert an unnamed separator
                            x_InsertInUnnamedGroup(*sub_item->Clone());
                        }
                    } else {
                        CMenuItem* target_sub_item = (*it_target)->GetValue();
                        x_MergeItems(*target_sub_item, *sub_item);
                    }
                }
            } //for

        } else {
            ERR_POST(kCannotMerge << item.GetLabel() << " into " << GetLabel()
                     << ", because they are not submenus.");
        }
    } else {
        ERR_POST(kCannotMerge << item.GetLabel() << " into " << GetLabel()
                 << ", because they have different labels.");
    }
}


// merge new_item into the target
void CMenuItem::x_MergeItems(CMenuItem& target, const CMenuItem& new_item)
{
    if(target.IsSubmenu())    {
        // submenu - merge recursively
        target.Merge(new_item);
    } else if(target.IsItem()) {
        // command item
        string s_error;
        if(! new_item.IsItem()) {
            s_error = ", because it is not a command item.";
        } else if(target.GetCommand() != new_item.GetCommand())    {
            s_error = ", because they have different commands.";
        }
        if(s_error.size())  {
            ERR_POST(kCannotMerge << new_item.GetLabel() << " into "
                     << target.GetLabel() << s_error);
        }
        // else, do nothing, because new_item is identical to target
    }
}


CMenuItem::TChildItem_I
        CMenuItem::x_FindSubItem(TChildItem_I it_begin, TChildItem_I it_end,
                                 const string& label, bool skip_named_groups)
{
    if(skip_named_groups) {
        bool named_group = false;
        for(TChildItem_I it = it_begin; it != it_end;  ++it )  {
            const CMenuItem* sub_item = (*it)->GetValue();
            const string& s = sub_item->GetLabel();

            if(sub_item->IsSeparator())   {
                named_group = ! s.empty();
            } else if( ! (named_group  &&  skip_named_groups)  &&  (s == label))   {
                return it;
            }
        }
    } else {
        // assume that iterators
        for(TChildItem_I it = it_begin; it != it_end;  ++it )  {
            const CMenuItem* sub_item = (*it)->GetValue();
            const string& s = sub_item->GetLabel();

            if(s == label)
                return it;
        }
    }
    return it_end;
}


void CMenuItem::x_InsertInUnnamedGroup(CMenuItem& item)
{
    bool named = false;
    // find the last separator in the menu
    for( TItemNode::TNodeList_RI it = m_ItemNode.SubNodeRBegin();
                                 it != m_ItemNode.SubNodeREnd();  ++it)   {
        const CMenuItem* it_item = (*it)->GetValue();
        if(it_item->IsSeparator())   {
            named = ! it_item->GetLabel().empty();
            break;
        }
    }

    if(named)   {
        AddSeparator(); // add an unnamed separator to terminate the previous group
    }
    AddSubItem(&item);
}


CMenuItem* CMenuItem::GetParent()
{
    TItemNode* parent_node = m_ItemNode.GetParent();
    return parent_node ? parent_node->GetValue() : NULL;
}

const   CMenuItem*  CMenuItem::GetParent() const
{
    const TItemNode* parent_node = m_ItemNode.GetParent();
    return parent_node ? parent_node->GetValue() : NULL;
}

CMenuItem*    CMenuItem::AddSubItem(CMenuItem* item)
{
    _ASSERT(IsSubmenu());
    if(item)    {
        TItemNode* node = & item->m_ItemNode;
        m_ItemNode.AddNode(node);
    }
    return item;
}


void CMenuItem::DestroyAllSubNodes()
{
    for( TItemNode::TNodeList_I it = m_ItemNode.SubNodeBegin();
        it != m_ItemNode.SubNodeEnd();  )   {
        TItemNode::TNodeList_I it2 = it++;
        TItemNode* node = m_ItemNode.DetachNode(*it2);
        delete node->GetValue();
    }
}


CMenuItem* CMenuItem::AddSeparator()
{
    CMenuItem* item = new CMenuItem();
    return AddSubItem(item);
}


CMenuItem* CMenuItem::AddSeparator(const string& label)
{
    CMenuItem* item = new CMenuItem(eSeparator, label);
    return AddSubItem(item);
}


CMenuItem*  CMenuItem::AddSubMenu(const string& label, const string& image_alias)
{
    CMenuItem* item = new CMenuItem(label, image_alias);
    return AddSubItem(item);
}

CMenuItem*  CMenuItem::AddSubItem(const string& label, TCmdID cmd,
                                  const string& image_alias, const string& tooltip,
                                  int state)
{
    CMenuItem* item = new CMenuItem(label, cmd, image_alias, tooltip, state);
    return AddSubItem(item);
}


CMenuItem* CMenuItem::InsertSubItem(TChildItem_I it, CMenuItem* item)
{
    _ASSERT(IsSubmenu());
    if(item)    {
        TItemNode* node = & item->m_ItemNode;
        m_ItemNode.InsertNode(it, node);
    }
    return item;
}


void CMenuItem::RemoveItem(TChildItem_I it)
{
    CMenuItem* node = (*it)->GetValue();
    m_ItemNode.DetachNode(it);
    delete node;
}


bool CMenuItem::IsSubmenuEmpty()
{
    return m_ItemNode.SubNodeBegin() == m_ItemNode.SubNodeEnd();
}

CMenuItem::TChildItem_I    CMenuItem::SubItemsBegin()
{
    return m_ItemNode.SubNodeBegin();
}

CMenuItem::TChildItem_I    CMenuItem::SubItemsEnd()
{
    return m_ItemNode.SubNodeEnd();
}

CMenuItem::TChildItem_CI    CMenuItem::SubItemsBegin()  const
{
    return m_ItemNode.SubNodeBegin();
}

CMenuItem::TChildItem_CI    CMenuItem::SubItemsEnd() const
{
    return m_ItemNode.SubNodeEnd();
}

CMenuItem*  CMenuItem::FindEqualSubItem(const CMenuItem& item)
{
    for(TChildItem_I it = SubItemsBegin(); it != SubItemsEnd(); ++it)  {
        CMenuItem* curr = (*it)->GetValue();
        if(curr->Equal(item))
            return curr;
    }
    return NULL;
}

const CMenuItem*  CMenuItem::FindEqualSubItem(const CMenuItem& item) const
{
    for(TChildItem_CI it = SubItemsBegin(); it != SubItemsEnd(); ++it)  {
        const CMenuItem* curr = (*it)->GetValue();
        if(curr->Equal(item))
            return curr;
    }
    return NULL;
}

CMenuItem::TChildItem_I    CMenuItem::FindSubItem(const CMenuItem& item)
{
    for(TChildItem_I it = SubItemsBegin(); it != SubItemsEnd(); ++it)  {
        if((*it)->GetValue() == &item)
            return it;
    }
    return SubItemsEnd();
}

CMenuItem::TChildItem_CI   CMenuItem::FindSubItem(const CMenuItem& item) const
{
    for(TChildItem_CI it = SubItemsBegin(); it != SubItemsEnd(); ++it)  {
        if((*it)->GetValue() == &item)
            return it;
    }
    return SubItemsEnd();
}

CMenuItem::TChildItem_I    CMenuItem::FindSubItem(const string& label)
{
    for(TChildItem_I it = SubItemsBegin(); it != SubItemsEnd(); ++it)  {
        if((*it)->GetValue()->GetLabel() == label)
            return it;
    }
    return SubItemsEnd();
}

CMenuItem::TChildItem_CI   CMenuItem::FindSubItem(const string& label) const
{
    for(TChildItem_CI it = SubItemsBegin(); it != SubItemsEnd(); ++it)  {
        if((*it)->GetValue()->GetLabel() == label)
            return it;
    }
    return SubItemsEnd();
}


CMenuItem::TChildItem_I    CMenuItem::FindSubItem(EType type, const string& label)
{
    for(TChildItem_I it = SubItemsBegin(); it != SubItemsEnd(); ++it)  {
        const CMenuItem* item = (*it)->GetValue();
        if(item->GetType() == type  &&  item->GetLabel() == label)
            return it;
    }
    return SubItemsEnd();
}


CMenuItem::TChildItem_CI   CMenuItem::FindSubItem(EType type, const string& label) const
{
    for(TChildItem_CI it = SubItemsBegin(); it != SubItemsEnd(); ++it)  {
        const CMenuItem* item = (*it)->GetValue();
        if(item->GetType() == type  &&  item->GetLabel() == label)
            return it;
    }
    return SubItemsEnd();
}

////////////////////////////////////////////////////////////////////////////////
/// creates hierarchy of CMenuItem objects from the array of SMenuItemRec descriptors
CMenuItem*  CreateMenuItems(const SMenuItemRec* items)
{
    if(items)   {
        CMenuItem* root = NULL;
        vector<CMenuItem*>  path;
        const SMenuItemRec* p_rec = items;

        while(p_rec)    {
            CMenuItem* item = NULL;

            if(p_rec->IsSubMenu())  {
                // create Submenu and add to path
                item = new CMenuItem(p_rec->m_Label, p_rec->m_ImageAlias);
                if(path.size()) {
                    path.back()->AddSubItem(item);
                }   else {
                    root = item;
                }
                path.push_back(item);
            } else if(p_rec->IsSubMenuEnd())  {
                //end current submenu and eject it from path
                path.pop_back();
                _ASSERT(! path.empty());
            } else if(p_rec->IsMenuEnd())   {
                path.pop_back();
                _ASSERT(path.empty()); // all submenus were closed
                 return root;
            } else if(p_rec->m_Type == CMenuItem::eItem)    {
                // add command item
                item = new CMenuItem(p_rec->m_Label, p_rec->m_CommandID,
                                     p_rec->m_ImageAlias, p_rec->m_Tooltip,
                                     p_rec->m_State);
                path.back()->AddSubItem(item);
            } else if(p_rec->m_Type == CMenuItem::eSeparator)    {
                // add separator
                item = new CMenuItem(CMenuItem::eSeparator, p_rec->m_Label);
                path.back()->AddSubItem(item);
            } else {
                _ASSERT(false);
            }
            p_rec++;
        }
    }
    return  NULL;
}



END_NCBI_SCOPE
