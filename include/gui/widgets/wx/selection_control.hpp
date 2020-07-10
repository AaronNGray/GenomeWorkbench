#ifndef GUI_WIDGETS_WX___SELECTION_CONTROL__HPP
#define GUI_WIDGETS_WX___SELECTION_CONTROL__HPP

/*  $Id: selection_control.hpp 30858 2014-07-31 14:05:43Z ucko $
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

#include <gui/widgets/wx/gui_event.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/gdicmn.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///
class  NCBI_GUIWIDGETS_WX_EXPORT  CSelectionControl
{
public:
    typedef int     TIndex;
    typedef void*   TItemHandle;
    typedef vector<TIndex>         TIndexVector;
    typedef vector<TItemHandle>    TItemHandleVector;

    enum    EItemState  {
        fDefaultState   = 0x00,
        fItemSelected   = 0x01,
        fItemFocused    = 0x02,
        fWidgetFocused  = 0x04
    };

    enum    EStyle   {
        fDefaultStyle    = 0x00,
        fSingleSelection = 0x01
    };

    CSelectionControl();
    virtual ~CSelectionControl() {}

    virtual int GetStyle() const;
    virtual void    SetStyle(int style);

    /// Items state and selection
    virtual int     GetItemState(TIndex index) const;
    virtual void    SetItemState(TIndex index, int state);

    virtual bool    IsItemFocused(TIndex index) const;
    virtual TIndex  GetFocusedIndex() const;

    virtual bool    IsItemSelected(TIndex index) const;
    virtual TIndex  GetSelectedIndex() const;
    virtual TIndex  GetSelectedCount() const;
    virtual void    GetSelectedIndexes(TIndexVector& indexes) const;

    virtual void    FocusItem(TIndex index);
    virtual void    SelectItem(TIndex index, bool select = true);
    virtual void    SelectItems(TIndexVector& indexes, bool select = true);
    //virtual void    SelectItemRange(TIndex from, TIndex to);
    virtual void    SelectAll(bool select = true);


    void    MoveSelectionBy(int shift, CGUIEvent::EGUIState state)
    {
            this->x_MoveSelectionBy(shift, state);
    }

protected: // utitlity functions
    bool    x_AssertIndexValid(TIndex index) const;
    TIndex  x_GetItemsCount() const;

    void    x_SetItems(const TItemHandleVector& items, bool b_update,
                     bool keep_selection = false);

    bool    x_InsertItem(TIndex index, const TItemHandle& item, bool update);
    void    x_InsertItems(const TItemHandleVector& items, const TIndexVector& indices,
                        bool update);

    bool    x_DeleteItem(TIndex index, bool update = true);
    //void    x_DeleteItems(const TIndexVector& indexes, bool update);
    void    x_DeleteAllItems();

    void    x_Clear();

    virtual void    x_SelectItem(TIndex index, bool select);
    virtual void    x_SendSelectionEvent(TIndex index) = 0;

    virtual void    x_SelectSingleItem(TIndex index);
    virtual void    x_InvertSingleItem(TIndex index);
    virtual void    x_SelectItems(const TIndexVector& vIndexes,
                                  bool reset_others = false);
    virtual void    x_SelectAll(bool select = true);
    virtual void    x_SelectTo(TIndex index);

protected:

    void OnMouseDown(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);

    void    x_SelectTo(int index, CGUIEvent::EGUIState state);
    void    x_MoveSelectionBy(int shift, CGUIEvent::EGUIState state);
    void    x_SelectFocusedItem(bool deselect_en);

    void    x_UpdateItemToIndex();

    void    x_DebugValid() const;
protected:
    // functions to be implemeneted in derived classes
    virtual void    x_UpdateItems(TIndexVector& indexes) = 0;
    virtual void    x_UpdateItemsRange(TIndex start, TIndex end) = 0;

    virtual int     x_GetIndexByWindowPos(int win_x, int win_y, bool clip = false) = 0;

    virtual void    x_MakeVisible(TIndex index) = 0;

protected: // data members
    typedef pair<TItemHandle, bool>   TItemEntry;  // bool for "Selected" state
    typedef vector<TItemEntry>  TEntryVector; // Index -> Item
    typedef map<TItemHandle, int>     TItemToIndexMap;

    TEntryVector        m_Entries;
    TItemToIndexMap     m_ItemToIndex;

    int     m_Style;
    TIndex  m_SelectedCount;  /// number of selected items
    TIndex  m_SingleSelected; /// index of the selected itme (valid only in fSingleSelection mode)
    TIndex  m_FocusedIndex;
    TIndex  m_AnchorIndex;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_WX___SELECTION_CONTROL__HPP
