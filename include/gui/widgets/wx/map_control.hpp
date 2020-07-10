#ifndef GUI_WIDGETS_WX___MAP_CONTROL__HPP
#define GUI_WIDGETS_WX___MAP_CONTROL__HPP

/*  $Id: map_control.hpp 25849 2012-05-23 01:03:31Z voronov $
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

#include <gui/utils/event_handler.hpp>
#include <gui/utils/menu_item.hpp>

#include <gui/widgets/wx/selection_control.hpp>
#include <gui/widgets/wx/imap_item.hpp>

#include <wx/scrolwin.h>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
///
enum    EColorType  {
    eBackground,
    eText,
    eDisabledText,
    eReadOnlyText,
    eSelectedBack,
    eSelectedText,
    eFocused,
    eFocusedText
};


///////////////////////////////////////////////////////////////////////////////
///
/// CMapControl owns all its items.
class NCBI_GUIWIDGETS_WX_EXPORT CMapControl :
    public wxScrolledCanvas,
    public CSelectionControl//,
    //public CEventHandler
{
    typedef wxScrolledCanvas TParent;
    DECLARE_EVENT_TABLE();

public:
    typedef CIRef<IwxMapItem> TItemRef;
    typedef CConstIRef<IwxMapItem> TCItemRef;

    enum    ESizePolicy {
        eFixedSize, // model space does not depend on available viewport space

        /// widget adjusts its model space so that its horizontal size fits into
        /// available viewport
        eAdjustHorzSize,

        /// widget adjusts its model space so that its vertical size fits into
        /// available viewport
        eAdjustVertSize
    };

    struct SProperties  {
        int m_ColumnWidth;
        int m_MaxItemHeight; /// max item height
        int m_SepLineWidth; /// width of the separation line between the columns
        int m_SepLineVertOffset; ///  vertical space between separation lines and the borders
        int m_ItemOffsetX; /// horz space between an item and separation line or border
        int m_ItemOffsetY; /// vert space around an item
        bool m_SeparateGroups; // place each group in a separate column
        bool m_SingleColumn;
        ESizePolicy m_SizePolicy;

        SProperties();
    };

    CMapControl(wxWindow* parent,
                  wxWindowID id = wxID_ANY,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  long style = wxHSCROLL | wxVSCROLL | wxTAB_TRAVERSAL |
                               wxWANTS_CHARS | wxFULL_REPAINT_ON_RESIZE,
                  const wxString& name = wxT("mapcontrol"));
    virtual ~CMapControl();

    virtual void    SetColumnWidth(int w, bool update = true);
    virtual void    SetMaxItemHeight(int h, bool update = true);
    SProperties& GetProperties() {  return m_Properties;    }

    virtual SwxMapItemProperties& GetMapItemProperties();

    /// block layout and repainting until control is unlocked
    virtual void    LockUpdates(bool lock = true);

    virtual TIndex GetItemsCount() const;

    virtual TItemRef    GetItem(TIndex index);
    virtual TCItemRef   GetItem(TIndex index) const;

    virtual void    AddItem(IwxMapItem* item);
    virtual void    InsertItem(TIndex index, IwxMapItem* item);
    virtual void    DeleteItem(TIndex index);
    virtual void    DeleteItem(IwxMapItem& item);
    virtual void    DeleteAllItems();

    virtual TIndex  GetItemIndex(IwxMapItem& item) const;
    virtual bool    Layout();

// override CSelectionControl member for hot tracking
    virtual TIndex  GetSelectedIndex() const;
    TIndex  GetHotIndex() const { return m_HotItem; }

    void    OnPaint(wxPaintEvent& event);
    void    OnSize(wxSizeEvent& event);

    void    UpdateSelection();

protected:
    void    x_Init();

    TItemRef   x_GetItem(TIndex index);
    TCItemRef  x_GetItem(TIndex index) const;

    bool    x_IsUpdatesLocked() const;

    virtual void    x_InitItemProperties();
    virtual void    x_DrawSeparationLines(wxDC& dc);
    virtual void    x_DrawItemsRange(wxDC& dc, TIndex from, TIndex to);


    virtual wxSize  x_CalculateLayout(int width, int height);

    virtual void    x_DoDefaultActionOnSelected();

    /// @name CSelectionControl pure virtual functions
    /// @{
    virtual void    x_SendSelectionEvent(TIndex index);
    virtual void    x_UpdateItems(TIndexVector& indexes);
    virtual void    x_UpdateItemsRange(TIndex start, TIndex end);

    virtual int     x_GetIndexByWindowPos(int win_x, int win_y, bool clip = false);

    virtual void    x_MakeVisible(TIndex index);
    /// @}

    void    OnContextMenu(wxContextMenuEvent& event);
    void    OnFocusChanged(wxFocusEvent& event);
    void    OnMouseDown(wxMouseEvent& evt);
    void    OnLeftDoubleClick(wxMouseEvent& evt);
    void    OnMouseUp(wxMouseEvent& evt);
    void    OnMouseWheel(wxMouseEvent& event);
    void    OnKeyDown(wxKeyEvent& event);
    void    OnMouseMove(wxMouseEvent& event);
    void    OnMouseLeave(wxMouseEvent& event);

    void    x_HorzMoveSelectionBy(int shift, CGUIEvent::EGUIState state);

    void    x_UpdateHotItem(wxPoint ms_pos);

    //virtual void        x_OnShowPopupMenu();
    //virtual CMenuItem*  x_CreatePopupMenu();
protected:
    SProperties m_Properties;

    SwxMapItemProperties  m_ItemProps;

    /// this is kind of Model
    typedef vector<TItemRef>   TItems;

    TItems  m_Items;
    int m_UpdateLockCounter; // when updates are locked the counter is incremented //TODO

    wxPoint  m_PageInc;
    int m_HotItem;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_WX___MAP_CONTROL__HPP
