#ifndef GUI_WIDGETS_ALNMULTI___SEL_LIST_CONTROLLER__HPP
#define GUI_WIDGETS_ALNMULTI___SEL_LIST_CONTROLLER__HPP

/*  $Id: sel_list_controller.hpp 30579 2014-06-13 21:49:27Z shkeda $
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

#include <gui/widgets/aln_multiple/list_mvc.hpp>
#include <gui/widgets/gl/ievent_handler.hpp>
#include <gui/widgets/wx/gui_event.hpp>


BEGIN_NCBI_SCOPE


////////////////////////////////////////////////////////////////////////////////
/// class CSelListController
template <class Item>   class CSelListController :
        public IGlEventHandler
{
public:
    typedef Item TItem;
    typedef ISelListModel<Item> TSelListModel;

    CSelListController();
    virtual ~CSelListController();

    /// @name IGlEventHandler implementation.
    /// @{
    virtual void    SetPane(CGlPane* pane) = 0;
    virtual wxEvtHandler*   GetEvtHandler()  = 0;
    /// @}

protected:
    // specialized event handlers
    virtual void    x_OnLeftDown(wxMouseEvent& event);
    virtual void    x_OnLeftUp(wxMouseEvent& event);
    virtual void    x_OnMotion(wxMouseEvent& event);
    virtual void    x_OnKeyDown(wxKeyEvent& event);

    // functions to be overriden in the inherited class
    virtual TSelListModel* SLC_GetModel() = 0;
    virtual int     SLC_GetLineByWindowY(int WinY, bool b_clip = false) = 0;
    virtual int     SLC_GetHeight() = 0; // view size in pixels
    virtual void    SLC_VertScrollToMakeVisible(int index) = 0;

    void    x_MoveSelectionBy(int Shift, bool bShift, bool bCtrl);
    void    x_MoveSelLineByPage(bool bDown, bool bShift, bool bCtrl);
    void    x_SelectTo(int index, bool bShift, bool bCtrl);
    void    x_SelectFocusedItem(bool bDeselectEn);

protected:
    bool    m_ProcessMouseUp;
    wxPoint m_MouseDownPos;
};


template<class Item>
    CSelListController<Item>::CSelListController()
:   m_ProcessMouseUp(false)
{
}
template<class Item>
    CSelListController<Item>::~CSelListController()
{
}

template<class Item>
    void CSelListController<Item>::x_OnLeftDown(wxMouseEvent& event)
{
    m_ProcessMouseUp = false;
    wxPoint ms_pos = event.GetPosition();
    m_MouseDownPos = ms_pos;

    TSelListModel* model = SLC_GetModel();

    if(model) {
        int index = SLC_GetLineByWindowY(ms_pos.y, true);
        CGUIEvent::EGUIState state = CGUIEvent::wxGetSelectState(event);

        if(index == -1) {
            m_ProcessMouseUp = true;
        } else {
            switch(state)   {
            case CGUIEvent::eSelectState: {
                m_ProcessMouseUp = model->SLM_IsItemSelected(index);
                if(m_ProcessMouseUp)    {
                    model->SLM_FocusItem(index); // change focus, selection will be changed on MouseUp
                } else  {
                    model->SLM_SelectSingleItem(index);
                }
                break;
            }
            case CGUIEvent::eSelectExtState:
                model->SLM_SelectTo(index);
                break;
            case CGUIEvent::eSelectIncState:    {
                m_ProcessMouseUp = true;
                break;
            default: break;
            }
            }
        }
    }
}


// Small kDragThreshold Value occasionally sets m_ProcessMouseUp to false
// when user is fast-clicking through a number of rows and holding ctrl key
// Setting m_ProcessMouseUp to false makes it skip Ctrl+click event
// which is not what user expects. Perhaps, x_OnMotion shouldn't be processed 
// at all if Ctrl is down?
const static int kDragThreshold = 10; //TODO

template<class Item>
    void CSelListController<Item>::x_OnMotion(wxMouseEvent& event)
{
    if(event.Dragging())    {
        wxPoint pos = event.GetPosition();
        m_ProcessMouseUp = abs(pos.x - m_MouseDownPos.x) < kDragThreshold
                        &&  abs(pos.y - m_MouseDownPos.y) < kDragThreshold;
    }
}


template<class Item>
    void CSelListController<Item>::x_OnLeftUp(wxMouseEvent& event)
{
    TSelListModel* model = SLC_GetModel();

    if(model && m_ProcessMouseUp)  {
        wxPoint pos = event.GetPosition();
        int index = SLC_GetLineByWindowY(pos.y, true);

        CGUIEvent::EGUIState state = CGUIEvent::wxGetSelectState(event);

        if(index == -1) {
            model->SLM_SelectAll(false);
        } else {
            if(state == CGUIEvent::eSelectIncState) {
                model->SLM_InvertSingleItem(index);
            } else if(state == CGUIEvent::eSelectState) {
                        //|| (Btn == FL_RIGHT_MOUSE  &&  state == CGUIEvent::eSelectExtState)) {
                model->SLM_SelectSingleItem(index);
            }
        }
    }
    m_ProcessMouseUp = false;
}


template<class Item>
    void CSelListController<Item>::x_OnKeyDown(wxKeyEvent& event)
{
    TSelListModel* model = SLC_GetModel();

    if(model) {
        int key = event.GetKeyCode();
        CGUIEvent::EGUIState state = CGUIEvent::wxGetSelectState(event);
        bool b_shift = (state == CGUIEvent::eSelectExtState);
        bool b_ctrl = (state == CGUIEvent::eSelectIncState);

        switch(key) {
        case WXK_HOME:
        case WXK_NUMPAD_HOME:
            x_SelectTo(0, b_shift, b_ctrl);
            break;
        case WXK_END:
        case WXK_NUMPAD_END:
            x_SelectTo(model->SLM_GetItemsCount()-1, b_shift, b_ctrl);
            break;
        case WXK_DOWN:
        case WXK_NUMPAD_DOWN:
            x_MoveSelectionBy(1, b_shift, b_ctrl);
            break;
        case WXK_UP:
        case WXK_NUMPAD_UP:
            x_MoveSelectionBy(-1, b_shift, b_ctrl);
            break;
        case WXK_PAGEUP:
        case WXK_NUMPAD_PAGEUP:
            x_MoveSelLineByPage(false, b_shift, b_ctrl);
            break;
        case WXK_PAGEDOWN:
        case WXK_NUMPAD_PAGEDOWN:
            x_MoveSelLineByPage(true, b_shift, b_ctrl);
            break;
        case ' ':
            x_SelectFocusedItem(b_ctrl);
            break;
        case 'a':
        case 'A':
            if(b_ctrl)
                model->SLM_SelectAll(true);
            break;
        default:
            event.Skip();
        }
    }
}


template<class Item>
    void CSelListController<Item>::x_MoveSelectionBy(int Shift, bool b_shift, bool b_ctrl)
{
    TSelListModel* model = SLC_GetModel();

    if(model) {
        int N = model->SLM_GetItemsCount();
        if(N > 0)   {
            int iFocused = model->SLM_GetFocusedItemIndex();
            iFocused = max(iFocused, 0);

            iFocused += Shift;

            iFocused = max(iFocused, 0);
            iFocused = min(iFocused, N - 1);

            x_SelectTo(iFocused, b_shift, b_ctrl);
        }
    }
}

template<class Item>
    void CSelListController<Item>::x_SelectTo(int index, bool b_shift, bool b_ctrl)
{
    TSelListModel* model = SLC_GetModel();
    if(model) {
        if(b_shift)  {
            model->SLM_SelectTo(index);
        } else  {
            if(b_ctrl)   {
                model->SLM_FocusItem(index);
            }   else  {
                model->SLM_SelectSingleItem(index);
            }
        }
        SLC_VertScrollToMakeVisible(index);
    }
}

template<class Item>
    void CSelListController<Item>::x_MoveSelLineByPage(bool bDown, bool b_shift, bool b_ctrl)
{
    TSelListModel* model = SLC_GetModel();
    if(model)  {
        if(bDown)   { // page Down
            int PosY = SLC_GetHeight();
            int iBottom = SLC_GetLineByWindowY(PosY);
            int iFocused = model->SLM_GetFocusedItemIndex();

            if(iFocused == iBottom) { // page down
                PosY += PosY;
                iBottom = SLC_GetLineByWindowY(PosY);
            }
            if(iBottom == -1)
                iBottom = model->SLM_GetItemsCount()-1;

            x_SelectTo(iBottom, b_shift, b_ctrl);
        } else { // page Up
            int iTop = SLC_GetLineByWindowY(0);
            int iFocused = model->SLM_GetFocusedItemIndex();

            if(iFocused == iTop) { // page down
                int PosY = SLC_GetHeight();
                iTop = SLC_GetLineByWindowY(PosY);
            }
            if(iTop == -1)
                iTop = min(0, model->SLM_GetItemsCount()-1);
            if(iTop > -1)
                x_SelectTo(iTop, b_shift, b_ctrl);
        }
    }
}

template<class Item>
    void CSelListController<Item>::x_SelectFocusedItem(bool bDeselectEn)
{
    TSelListModel* model = SLC_GetModel();
    if(model) {
        int iFocused = model->SLM_GetFocusedItemIndex();
        if(iFocused != -1)  {
            bool bSel = model->SLM_IsItemSelected(iFocused);
            if( ! bSel  ||  (bSel  &&  bDeselectEn))
                model->SLM_InvertSingleItem(iFocused);
        }
    }
}


END_NCBI_SCOPE


#endif  // GUI_WIDGETS_ALNMULTI___SEL_LIST_CONTROLLER__HPP
