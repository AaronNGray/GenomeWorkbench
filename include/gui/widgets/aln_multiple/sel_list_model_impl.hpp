#ifndef GUI_WIDGETS_ALN_MULTIPLE___SEL_LIST_MODEL_IMPL__HPP
#define GUI_WIDGETS_ALN_MULTIPLE___SEL_LIST_MODEL_IMPL__HPP

/*  $Id: sel_list_model_impl.hpp 19270 2009-05-13 18:18:16Z voronov $
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
 *
 */


#include <corelib/ncbistl.hpp>
#include <corelib/ncbistd.hpp>

#include <gui/widgets/aln_multiple/list_mvc.hpp>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// CSelListModelImpl provides a defult implementation of ISelListModel.
///
/// m_iFocusedItem and m_iAnchor item are not updated automatically.
template <class Item>
class CSelListModelImpl : public ISelListModel<Item>
{
public:
    typedef typename ISelListModel<Item>::TItem        TItem;
    typedef typename ISelListModel<Item>::TIndex       TIndex;
    typedef typename ISelListModel<Item>::TIndexVector TIndexVector;

    typedef vector<TItem>                    TItemVector;

    /// related interfaces
    ///

    typedef ISelListView<TItem>     TSelListView;
    typedef ISelListModel<TItem>    TSelListModel;

    // ISelListModel interface implementation
    virtual TIndex  SLM_GetItemsCount() const;
    virtual TItem   SLM_GetItem(TIndex index) const;

    virtual TIndex  SLM_GetFocusedItemIndex() const;
    virtual bool    SLM_IsItemSelected(TIndex index) const;
    virtual TIndex  SLM_GetSelectedCount() const;
    virtual void    SLM_GetSelectedIndices(TIndexVector& vIndices) const;
    virtual void    SLM_GetSelectedItems(TItemVector& items) const;

    virtual void    SLM_FocusItem(TIndex index);

    virtual void    SLM_SelectSingleItem(TIndex index);
    virtual void    SLM_InvertSingleItem(TIndex index);
    virtual void    SLM_SelectItems(const TIndexVector& vIndeces,
                                    bool b_reset_others = false);
    virtual void    SLM_SelectAll(bool bSelect = true);
    virtual void    SLM_SelectTo(TIndex index);

    virtual void    SLM_AddSLView(TSelListView* pView);
    virtual void    SLM_RemoveSLView(TSelListView* pView);

protected:
    CSelListModelImpl();

    void    SetItems(const TItemVector& vItems, bool b_update,
                     bool b_keep_selection = false);

    bool    InsertItem(TIndex index, const TItem& item, bool b_update);
    void    InsertItems(const TItemVector& v_items, const TIndexVector& v_indices,
                        bool b_update);

    bool    DeleteItem(TIndex index, bool bUpdate = true);
    void    DeleteItems(const TIndexVector& vIndices, bool b_update);
    void    DeleteAllItems();

protected:
    typedef pair<TItem, bool>   TItemEntry;  // bool for "Selected" state
    typedef vector<TItemEntry>  TEntryVector;
    typedef map<TItem, int>     TItemToIndexMap;
    typedef list<TSelListView*> TViewList;

    virtual void    x_SelectItem(TIndex index, bool b_sel) = 0;
    virtual bool    x_IsItemSelected(TIndex index) const = 0;

    virtual TIndex  x_GetItemsCount()   const = 0;
    virtual TItem   x_GetItem(TIndex index)    const = 0;
    virtual void    x_SetEntries(const TEntryVector& v_entries) = 0;
    virtual TIndex  x_GetItemIndex(const TItem& item) = 0;

    /// inserts item, but does not updates all data structures
    virtual void    x_InsertItem(TIndex index, const TItemEntry& entry) = 0;
    /// performs update after all items have been inserted
    virtual void    x_CompleteInsertion() = 0;

    /// mark item for deletion
    virtual void    x_MarkItemForErase(TIndex index) = 0;
    /// deletes all marked items in a single pass, performs neccessary updates
    virtual void    x_EraseMarkedItems() = 0;

    virtual void    x_ClearItems() = 0;

    /// TODO
    void            x_EraseItem(TIndex index)
    {
        // TODO
        _ASSERT(false);
    }

protected:

    void  x_Clear();
    void  x_GetSelectedItems(TIndexVector& vIndices);

    void  x_ViewsUpdateItems(TIndexVector &vIndices);
    void  x_ViewsUpdateItemRange(int iStart, int iEnd);

protected: // data members
    int     m_SelectedCount;
    int     m_iFocusedItem;
    int     m_iAnchorItem;

    TViewList   m_lsViews;
};


////////////////////////////////////////////////////////////////////////////////
///  class CSelListModelImpl
template<class Item>
    CSelListModelImpl<Item>::CSelListModelImpl()
: m_SelectedCount(0),
    m_iFocusedItem(-1),
    m_iAnchorItem(-1)
{
}

template<class Item>
    typename CSelListModelImpl<Item>::TIndex     CSelListModelImpl<Item>::SLM_GetItemsCount() const
{
    return x_GetItemsCount();
}

template<class Item>
    typename CSelListModelImpl<Item>::TItem   CSelListModelImpl<Item>::SLM_GetItem(TIndex index) const
{
    return x_GetItem(index);
}

template<class Item>
    typename CSelListModelImpl<Item>::TIndex     CSelListModelImpl<Item>::SLM_GetFocusedItemIndex() const
{
    return m_iFocusedItem;
}

template<class Item>
    bool    CSelListModelImpl<Item>::SLM_IsItemSelected(TIndex index) const
{
    return x_IsItemSelected(index);
}

template<class Item>
    typename CSelListModelImpl<Item>::TIndex    CSelListModelImpl<Item>::SLM_GetSelectedCount() const
{
    return m_SelectedCount;
}

template<class Item>
    void    CSelListModelImpl<Item>::SLM_GetSelectedIndices(TIndexVector& vIndices) const
{
    int size = (int) x_GetItemsCount();
    for(  int i = 0;  i < size;  i++ ) {
        if (x_IsItemSelected(i))    {
            vIndices.push_back(i);
        }
    }
}

template<class Item>
    void   CSelListModelImpl<Item>:: SLM_GetSelectedItems(TItemVector& items) const
{
    int size = x_GetItemsCount();
    for( int i = 0;  i < size;  i++ ) {
        if (x_IsItemSelected(i))    {
            TItem item = x_GetItem(i);
            items.push_back(item);
        }
    }
}

template<class Item>
    void    CSelListModelImpl<Item>::SLM_FocusItem(TIndex index)
{
    if (index != m_iFocusedItem)   {
        TIndexVector vUpdateItems;
        vUpdateItems.push_back(m_iFocusedItem); // update old focus
        m_iFocusedItem = index;
        vUpdateItems.push_back(m_iFocusedItem); // update new one

        x_ViewsUpdateItems(vUpdateItems);
    }
}

template<class Item>
    void    CSelListModelImpl<Item>::SLM_SelectSingleItem(TIndex index)
{
    TIndexVector vPrevSel;
    x_GetSelectedItems(vPrevSel);

    if(m_iFocusedItem != index) {
        if(m_iFocusedItem >= 0  &&  ! x_IsItemSelected(m_iFocusedItem))
            vPrevSel.push_back(m_iFocusedItem); // if it is not selected - we need update it separately
        m_iFocusedItem = index;
    }

    for(size_t i = 0; i < vPrevSel.size(); i++ )  { // reset selection
        TIndex idx = vPrevSel[i];
        x_SelectItem(idx, false);
    }
    if( index > -1) {
        x_SelectItem(index, true);
        vPrevSel.push_back(index);
        m_iAnchorItem = index;
        m_SelectedCount = 1;
    } else m_SelectedCount = 0;

    x_ViewsUpdateItems(vPrevSel);
}


template<class Item>
    void    CSelListModelImpl<Item>::SLM_InvertSingleItem(TIndex index)
{
    TIndexVector vIndices;
    if( index > -1) {
        if(m_iFocusedItem != index) {
            vIndices.push_back(m_iFocusedItem);
            m_iFocusedItem = index;
        }
        bool b_old_sel = x_IsItemSelected(index);
        x_SelectItem(index, ! b_old_sel);
        vIndices.push_back(index);
        m_iAnchorItem = index;
        m_SelectedCount += b_old_sel ? -1 : 1;
    } else m_SelectedCount = 0;

    x_ViewsUpdateItems(vIndices);
}


/// Select items with given indices. If "b_invert_others" == "true" - deselects
/// all other items.
template<class Item>
    void    CSelListModelImpl<Item>::SLM_SelectItems(const TIndexVector& vIndices,
                                                     bool b_reset_others)
{
    if(b_reset_others)     {
        size_t n_items = x_GetItemsCount();
        vector<bool> v_sel(n_items, false);
        TIndex count = 0;

        for( size_t i = 0;  i < vIndices.size();  i++ ) {
            if( ! v_sel[vIndices[i]])   {  // to count correctly
                v_sel[vIndices[i]] = true;
                count++;
            }
        }
        for( size_t j= 0; j < n_items;  j++ )   {
            x_SelectItem((int) j, v_sel[j]);
        }

        m_SelectedCount = count;

        x_ViewsUpdateItemRange(0, (int) n_items - 1);
    } else {
        TIndexVector vUpdateIndices;

        ITERATE(typename TIndexVector, it, vIndices) {
            if ( ! x_IsItemSelected(*it)) {
                x_SelectItem(*it, true);
                vUpdateIndices.push_back(*it);
            }
        }
        m_SelectedCount += (int) vUpdateIndices.size();

        x_ViewsUpdateItems(vUpdateIndices);
    }
}


template<class Item>
    void    CSelListModelImpl<Item>::SLM_SelectAll(bool b_select)
{
    TIndex count = x_GetItemsCount();
    for( TIndex i = 0; i < count; i++ )  {
        x_SelectItem(i, b_select);
    }
    m_SelectedCount = b_select ? count : 0;

    if(count)
        x_ViewsUpdateItemRange(0, count - 1);
}


template<class Item>
    void    CSelListModelImpl<Item>::SLM_SelectTo(TIndex index)
{
    if(m_iAnchorItem<0)
        m_iAnchorItem = 0;

    TIndex iStart = min(m_iAnchorItem, index);
    TIndex iEnd = max(m_iAnchorItem, index);

    // reset old selection and select from iStart to iEnd
    TIndexVector vUpdateIndices;
    bool bPrevFocusedChanged = false;

    TIndex count = x_GetItemsCount();

    for( TIndex i = 0; i < count;  i++ )   {
        bool b_select = (i >= iStart && i <= iEnd);
        if (x_IsItemSelected(i) != b_select)
        {
            x_SelectItem(i, b_select);
            m_SelectedCount += b_select ? 1 : -1;
            vUpdateIndices.push_back(i);
            if(i == m_iFocusedItem)
                bPrevFocusedChanged = true;
        }
    }
    if(m_iFocusedItem != index)
    {
        if( ! bPrevFocusedChanged)
            vUpdateIndices.push_back(m_iFocusedItem); // need to update
        m_iFocusedItem = index;
    }

    x_ViewsUpdateItems(vUpdateIndices);
}


template<class Item>
    void  CSelListModelImpl<Item>::SLM_AddSLView(TSelListView* pView)
{
    if( find(m_lsViews.begin(), m_lsViews.end(), pView) == m_lsViews.end()) {
        m_lsViews.push_back(pView);
        pView->SLV_SetModel(static_cast<TSelListModel*>(this));
    }
}


template<class Item>
    void  CSelListModelImpl<Item>::SLM_RemoveSLView(TSelListView* pView)
{
    typename TViewList::iterator itView
        = find( m_lsViews.begin(), m_lsViews.end(), pView);
    if (itView != m_lsViews.end())  {
        m_lsViews.erase(itView);
        pView->SLV_SetModel(NULL);
    }
}


template<class Item>
    void  CSelListModelImpl<Item>::SetItems(const TItemVector& vItems,
                                         bool b_update, bool b_keep_selection)
{
    size_t items_n = vItems.size();
    TEntryVector v_entries(items_n);

    int n = x_GetItemsCount();
    int n_sel = 0;
    for(  size_t i = 0;  i < items_n; i++) {
        v_entries[i].first = vItems[i];
        bool b_sel = false;
        if(b_keep_selection)    {
            TIndex index = x_GetItemIndex(vItems[i]);
            b_sel = (index > -1  && index < n) ? x_IsItemSelected(index) : false;
        }
        v_entries[i].second = b_sel;
        if(b_sel)   {
            n_sel++;
        }
    }


    x_Clear();

    x_SetEntries(v_entries);
    m_SelectedCount = n_sel;

    if(b_update)    {
        TIndex i_max = max(0, (int) items_n - 1);
        x_ViewsUpdateItemRange(0, i_max);
    }
}


template<class Item>
    void    CSelListModelImpl<Item>::InsertItems(const TItemVector& v_items,
                                                 const TIndexVector& v_indices,
                                                 bool b_update)
{
    size_t n_items = v_items.size();
    _ASSERT(n_items == v_indices.size());

    TIndex i_min = x_GetItemsCount();
    for( size_t i = 0; i <  n_items;  i++ )     {
        TIndex ind = v_indices[i];
        ind = min(ind, x_GetItemsCount());
        x_InsertItem(ind, TItemEntry(v_items[i], false));

        i_min = min(i_min, ind);
    }
    x_CompleteInsertion();

    if(b_update)    {
        TIndex i_max = max(0, x_GetItemsCount() - 1);
        x_ViewsUpdateItemRange(i_min, i_max);
    }
}


template<class Item>
    bool    CSelListModelImpl<Item>::InsertItem(TIndex index, const TItem& item,
                                                bool b_update)
{
    if(index >= 0  &&  index <= x_GetItemsCount())  {
        x_InsertItem(index, TItemEntry(item, false));
        x_CompleteInsertion();

        if(b_update)    {
            TIndex i_max = max(0, x_GetItemsCount() - 1);
            x_ViewsUpdateItemRange(index, i_max);
        }
        return true;
    }
    return false;
}


template<class Item>
    void    CSelListModelImpl<Item>::DeleteItems(const TIndexVector& vIndices,
                                                 bool bUpdate)
{
    TIndex count = x_GetItemsCount();
    TIndex min_del = count;

    ITERATE(typename TIndexVector, it, vIndices)  {
        TIndex index = *it;
        if(index >= 0  &&  index < count)  {
            if(m_iFocusedItem == index)
                m_iFocusedItem = -1;
            if(m_iAnchorItem == index)
                m_iAnchorItem = m_iFocusedItem;

            if(x_IsItemSelected(index))    //item selected
            {
                x_SelectItem(index, false);
                m_SelectedCount--;
            }

            x_MarkItemForErase(index);
            min_del = min(index, count);
        }
   }
   x_EraseMarkedItems();

   if(bUpdate  &&  min_del < count)    {
        x_ViewsUpdateItemRange(min_del, count-1);
   }
}


template<class Item>
    bool    CSelListModelImpl<Item>::DeleteItem(TIndex index, bool bUpdate)
{
    TIndex count = x_GetItemsCount();
    if(index >= 0  &&  index < count)  {
        if(m_iFocusedItem == index)
            m_iFocusedItem = -1;
        if(m_iAnchorItem == index)
            m_iAnchorItem = m_iFocusedItem;

        if(x_IsItemSelected(index))    //item selected
            m_SelectedCount--;

        x_EraseItem(index);

        if (bUpdate)    {
            TIndex i_max = max(0, count -1);
            x_ViewsUpdateItemRange(0, i_max);
        }
        return true;
    }
    return false;
}


template<class Item>
    void  CSelListModelImpl<Item>::DeleteAllItems()
{
    x_Clear();
    x_ViewsUpdateItemRange(0, -1);
}


template<class Item>
    void  CSelListModelImpl<Item>::x_Clear()
{
    x_ClearItems();

    m_iFocusedItem = m_iAnchorItem = -1;
    m_SelectedCount = 0;
}


template<class Item>
    void  CSelListModelImpl<Item>::x_GetSelectedItems(TIndexVector& vIndices)
{
    vIndices.reserve(m_SelectedCount);
    TIndex count = x_GetItemsCount();
    for( TIndex i = 0; i < count;  i++ ) {
        if(x_IsItemSelected(i))
            vIndices.push_back(i);
    }
    _ASSERT(vIndices.size() == (size_t) m_SelectedCount);
}


template<class Item>
    void  CSelListModelImpl<Item>::x_ViewsUpdateItems(TIndexVector &vIndices)
{
    NON_CONST_ITERATE(typename TViewList, itView, m_lsViews)
        (*itView)->SLV_UpdateItems(vIndices);
}


template<class Item>
    void  CSelListModelImpl<Item>::x_ViewsUpdateItemRange(int iStart, int iEnd)
{
    NON_CONST_ITERATE(typename TViewList, itView, m_lsViews)
        (*itView)->SLV_UpdateRange(iStart, iEnd);
}


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_MULTIPLE___SEL_LIST_MODEL_IMPL__HPP
