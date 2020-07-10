/*  $Id: selection_control.cpp 23985 2011-06-30 17:59:06Z kuznets $
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

#include <gui/widgets/wx/selection_control.hpp>

#include <wx/event.h>


BEGIN_NCBI_SCOPE


//////////////////////////////////////////////////////////////////////////////
// CSelectionControl


CSelectionControl::CSelectionControl()
:   m_Style(0),
    m_SelectedCount(0),
    m_SingleSelected(-1),
    m_FocusedIndex(-1),
    m_AnchorIndex(-1)
{
}


///////////////////////////////////////////////////////////////////////////////
/// Low-level worker functions - implementation dependend
int CSelectionControl::GetStyle() const
{
    return m_Style;
}


void CSelectionControl::SetStyle(int style)
{
    m_Style = style;
}


int CSelectionControl::GetItemState(TIndex index) const
{
    bool valid = x_AssertIndexValid(index);

    int state = 0;
    if(valid)   {
        if(IsItemSelected(index)) {
            state |= fItemSelected;
        }
        if(IsItemFocused(index)) {
            state |= fItemFocused;
        }
    }
    return state;
}


void CSelectionControl::SetItemState(TIndex index, int state)
{
    bool valid = x_AssertIndexValid(index);

    if(valid)   {
        bool select = (state & fItemSelected) != 0;
        bool selected = IsItemSelected(index);
        if(selected != select) {
            SelectItem(index, select);
        }

        bool focus = (state & fItemFocused) != 0;
        bool focused = IsItemFocused(index);
        if(focus != focused)    {
            FocusItem(focus ? index : -1);
        }
    }
}


bool CSelectionControl::IsItemFocused(TIndex index) const
{
    x_AssertIndexValid(index);

    return m_FocusedIndex == index;
}


CSelectionControl::TIndex   CSelectionControl::GetFocusedIndex() const
{
    return m_FocusedIndex;
}


bool     CSelectionControl::IsItemSelected(TIndex index) const
{
    bool valid = x_AssertIndexValid(index);
    return valid ? m_Entries[index].second : false;
}


CSelectionControl::TIndex CSelectionControl::GetSelectedIndex() const
{
    bool single = (GetStyle() & fSingleSelection) != 0;
    _ASSERT(single  &&  m_SelectedCount <= 1);

    if(single  &&  m_SelectedCount == 1)  {
        TIndex count = x_GetItemsCount();
        for( TIndex i = 0;  i < count;  i++ ) {
            if(m_Entries[i].second)    {
                return i;
            }
        }
    }
    return -1;
}


CSelectionControl::TIndex   CSelectionControl::GetSelectedCount() const
{
    return m_SelectedCount;
}


void CSelectionControl::GetSelectedIndexes(TIndexVector& indexes) const
{
    TIndex count = x_GetItemsCount();
    indexes.reserve(m_SelectedCount);

    for( TIndex i = 0;  i < count;  i++ ) {
        if(m_Entries[i].second)    {
            indexes.push_back(i);
        }
    }
    _ASSERT((int)indexes.size() == m_SelectedCount);
    x_DebugValid();
}


void    CSelectionControl::FocusItem(TIndex index)
{
    bool valid = (index >= -1  && index < x_GetItemsCount());
    _ASSERT(valid);

    if (valid  &&  index != m_FocusedIndex)   {
        TIndexVector update_items;
        update_items.push_back(m_FocusedIndex); // update old focus

        m_FocusedIndex = index;
        if(m_FocusedIndex > -1) {
            update_items.push_back(m_FocusedIndex); // update new one
        }

        x_UpdateItems(update_items); // repaint affected items
    }
    x_DebugValid();
}


void    CSelectionControl::SelectItem(TIndex index, bool select)
{
    bool valid = x_AssertIndexValid(index);
    if(valid)   {
        x_SelectItem(index, select);
        x_UpdateItemsRange(index, index);
    }
}


void CSelectionControl::SelectItems(TIndexVector& indexes, bool select)
{
    x_SelectItems(indexes, select);
}


void CSelectionControl::SelectAll(bool select)
{
    x_SelectAll(select);
}


void CSelectionControl::x_SelectAll(bool select)
{
    TIndex count = x_GetItemsCount();
    for( TIndex i = 0;  i < count;  i++ )  {
        m_Entries[i].second = select;
    }
    m_SelectedCount = select ? count : 0;

    if(count)   {
        x_UpdateItemsRange(0, count - 1);
    }
    x_DebugValid();
}


CSelectionControl::TIndex  CSelectionControl::x_GetItemsCount() const
{
    return (TIndex)m_Entries.size();
}


void CSelectionControl::x_InsertItems(const TItemHandleVector& items,
                                        const TIndexVector& indices,
                                        bool  update)
{
    size_t n_items = items.size();
    _ASSERT(n_items == indices.size());

    TIndex i_min = x_GetItemsCount();
    for( size_t i = 0; i <  n_items;  i++ )     {
        TIndex ind = indices[i];
        ind = min(ind, x_GetItemsCount());

        m_Entries.insert(m_Entries.begin() + ind, make_pair(items[i], false));

        i_min = min(i_min, ind);
    }
    x_UpdateItemToIndex();

    if(update)    {
        TIndex i_max = max(0, x_GetItemsCount() - 1);
        x_UpdateItemsRange(i_min, i_max);
    }
}



bool CSelectionControl::x_InsertItem(TIndex index, const TItemHandle& item, bool update)
{
    if(index >= 0  &&  index <= x_GetItemsCount())  {
        m_Entries.insert(m_Entries.begin() + index, make_pair(item, false));
        x_UpdateItemToIndex();

        if(update)    {
            TIndex i_max = max(0, x_GetItemsCount() - 1);
            x_UpdateItemsRange(index, i_max);
        }
        return true;
    }
    return false;
}


bool CSelectionControl::x_DeleteItem(TIndex index, bool update)
{
    bool valid = x_AssertIndexValid(index);

    if(valid)  {
        TIndex count = x_GetItemsCount(); // take it before deleting

        if(m_FocusedIndex == index)
            m_FocusedIndex = -1;
        if(m_AnchorIndex == index)
            m_AnchorIndex = m_FocusedIndex;

        if(IsItemSelected(index))
            m_SelectedCount--;

        TItemHandle handle = m_Entries[index].first;
        m_ItemToIndex.erase(handle);
        m_Entries.erase(m_Entries.begin() + index);

        if (update)    {
            TIndex i_max = max(0, count - 1);
            x_UpdateItemsRange(index, i_max);
        }
        return true;
    }
    x_DebugValid();
    return false;
}


void    CSelectionControl::x_DeleteAllItems()
{
    x_Clear();
    x_UpdateItemsRange(0, -1);
}


void  CSelectionControl::x_Clear()
{
    m_Entries.clear();
    m_ItemToIndex.clear();

    m_FocusedIndex = m_AnchorIndex = -1;
    m_SelectedCount = 0;
}


///////////////////////////////////////////////////////////////////////////////
/// Protected API -

/// TODO move up
// inline
bool    CSelectionControl::x_AssertIndexValid(TIndex index) const
{
    bool valid = (index >= 0  &&  index  < (TIndex) m_Entries.size());
    _ASSERT(valid);
    return valid;
}


void CSelectionControl::x_SelectItem(TIndex index, bool select)
{
    bool valid = x_AssertIndexValid(index);
    if(valid)   {
        bool selected = m_Entries[index].second;
        if(selected != select)  {
            m_Entries[index].second = select;
            m_SelectedCount += (select ? 1 : -1);

            x_SendSelectionEvent(index);
        }
    }
}


/// handles selection of a single item from UI
void    CSelectionControl::x_SelectSingleItem(TIndex index)
{
    TIndexVector prev_sel;
    GetSelectedIndexes(prev_sel);

    if(m_FocusedIndex != index) {
        if(m_FocusedIndex >= 0  &&  ! IsItemSelected(m_FocusedIndex))
            prev_sel.push_back(m_FocusedIndex); // if it is not selected - we need update it separately
        m_FocusedIndex = index;
    }
    // reset previous selection
    for(size_t i = 0; i < prev_sel.size(); i++ )  {
        TIndex index = prev_sel[i];
        x_SelectItem(index, false);
    }
    if(index > -1) {
        x_SelectItem(index, true);
        prev_sel.push_back(index);
        m_AnchorIndex = index;
        m_SelectedCount = 1;
    } else m_SelectedCount = 0;

    x_UpdateItems(prev_sel);

    x_DebugValid();
}


void    CSelectionControl::x_InvertSingleItem(TIndex index)
{
    bool valid = x_AssertIndexValid(index);
    if(valid)   {
        TIndexVector indexes;
        if(index > -1) {
            if(m_FocusedIndex != index) {
                indexes.push_back(m_FocusedIndex);
                m_FocusedIndex = index;
            }
            bool old_sel = IsItemSelected(index);

            x_SelectItem(index, ! old_sel);
            indexes.push_back(index);
            m_AnchorIndex = index;
        } else m_SelectedCount = 0;

        x_UpdateItems(indexes);
    }

    x_DebugValid();
}

/// Select items with given indices. If "invert_others" == "true" - deselects
/// all other items.
void    CSelectionControl::x_SelectItems(const TIndexVector& indexes,
                                         bool  reset_others)
{
    if(reset_others)     {
        TIndex n_items = x_GetItemsCount();
        vector<bool> v_sel(n_items, false);

        for( size_t i = 0;  i < indexes.size();  i++ ) {
            v_sel[indexes[i]] = true;
        }
        for( TIndex j = 0; j < n_items;  j++ )   {
            x_SelectItem(j, v_sel[j]);
        }

        m_SelectedCount = (TIndex)indexes.size();

        x_UpdateItemsRange(0, n_items - 1);
    } else {
        TIndexVector update_indexes;

        ITERATE(TIndexVector, it, indexes) {
            if ( ! IsItemSelected(*it)) {
                x_SelectItem(*it, true);
                update_indexes.push_back(*it);
            }
        }
        m_SelectedCount += (TIndex)update_indexes.size();

        x_UpdateItems(update_indexes);
    }

    x_DebugValid();
}


// selects a rabge of itmes from m_AnchorIndex to index
void    CSelectionControl::x_SelectTo(TIndex index)
{
    if(m_AnchorIndex<0)
        m_AnchorIndex = 0;

    TIndex iStart = min(m_AnchorIndex, index);
    TIndex iEnd = max(m_AnchorIndex, index);

    // reset old selection and select from iStart to iEnd
    TIndexVector update_indexes;
    bool bPrevFocusedChanged = false;

    TIndex count = x_GetItemsCount();

    for( TIndex i = 0; i < count;  i++ )   {
        bool b_select = (i >= iStart && i <= iEnd);
        if (IsItemSelected(i) != b_select)    {
            x_SelectItem(i, b_select);
            update_indexes.push_back(i);
            if(i == m_FocusedIndex)
                bPrevFocusedChanged = true;
        }
    }
    if(m_FocusedIndex != index)    {
        if( ! bPrevFocusedChanged)
            update_indexes.push_back(m_FocusedIndex); // need to update
        m_FocusedIndex = index;
    }

    x_UpdateItems(update_indexes);

    x_DebugValid();
}

///////////////////////////////////////////////////////////////////////////////
/// Event handling

void CSelectionControl::OnMouseDown(wxMouseEvent& event)
{
    int index = x_GetIndexByWindowPos(event.GetX(), event.GetY(), true);
    CGUIEvent::EGUIState state = CGUIEvent::wxGetSelectState(event);

    if(index == -1) {
        x_SelectAll(false);
    } else {
        switch(state)   {
        case CGUIEvent::eSelectState:
            x_SelectSingleItem(index);
            break;
        case CGUIEvent::eSelectExtState:
            if(m_Style == fSingleSelection) {
                x_SelectSingleItem(index);
            } else {
                x_SelectTo(index);
            }
            break;
        case CGUIEvent::eSelectIncState:
            x_InvertSingleItem(index);
            break;
        default:
            break;
        }
    }
}


void CSelectionControl::OnKeyDown(wxKeyEvent& event)
{
    CGUIEvent::EGUIState state = CGUIEvent::wxGetSelectState(event);
    bool ctrl = (state == CGUIEvent::eSelectIncState);

    int key = event.GetKeyCode();
    switch(key) {
    case WXK_HOME:
        x_SelectTo(0, state);
        break;
    case WXK_END:
        x_SelectTo(x_GetItemsCount()-1, state);
        break;
    case WXK_DOWN:
        x_MoveSelectionBy(1, state);
        break;
    case WXK_UP:
        x_MoveSelectionBy(-1, state);
        break;
    case ' ':
        x_SelectFocusedItem(ctrl);
        break;
    case 'a':
    case 'A':
        if(ctrl) {
            x_SelectAll(true);
        }
        break;
    default:
        event.Skip();
        break;
    }
}


void CSelectionControl::x_MoveSelectionBy(int shift, CGUIEvent::EGUIState state)
{
    int count = x_GetItemsCount();
    if(count > 0)   {
        int i_focused = GetFocusedIndex();
        i_focused = max(i_focused, 0);

        i_focused += shift;

        i_focused = max(i_focused, 0);
        i_focused = min(i_focused, count - 1);

        x_SelectTo(i_focused, state);
    }
}


void CSelectionControl::x_SelectTo(int index, CGUIEvent::EGUIState state)
{
    switch(state)   {
    case CGUIEvent::eSelectState:
        x_SelectSingleItem(index);
        break;
    case CGUIEvent::eSelectIncState:
        FocusItem(index);
        break;
    case CGUIEvent::eSelectExtState:
        if(m_Style == fSingleSelection) {
            x_SelectSingleItem(index);
        } else {
            x_SelectTo(index);
        }
        break;
    default: break;
    }

    x_MakeVisible(index);
    x_DebugValid();
}


void CSelectionControl::x_SelectFocusedItem(bool deselect_en)
{
    int i_focused = GetFocusedIndex();
    if(i_focused != -1)  {
        bool bSel = IsItemSelected(i_focused);
        if( ! bSel  ||  (bSel  &&  deselect_en))
            x_InvertSingleItem(i_focused);
    }

    x_DebugValid();
}


void CSelectionControl::x_UpdateItemToIndex()
{
    m_ItemToIndex.clear();
    for( TIndex i = 0; i < (TIndex) m_Entries.size();  i++ )    {
        TItemHandle handle = m_Entries[i].first;
        m_ItemToIndex[handle] = i;
    }
}

void    CSelectionControl::x_DebugValid() const
{
    _ASSERT(m_Entries.size() == m_ItemToIndex.size());

    int selected = 0;
    for( size_t i = 0;  i< m_Entries.size();  i++ ) {
        if(m_Entries[i].second) {
            selected++;
        }
    }
    _ASSERT(m_SelectedCount == selected);

}


END_NCBI_SCOPE
