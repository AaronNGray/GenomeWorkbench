/*  $Id: desktop_canvas.cpp 44193 2019-11-14 16:41:20Z asztalos $
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
 * Authors:  Roman Katargin, Andrea Asztalos
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <objmgr/scope.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/misc/sequence_macros.hpp>

#include <gui/widgets/wx/async_call.hpp>
#include <gui/widgets/wx/gui_widget.hpp>
#include <gui/widgets/data/object_contrib.hpp>
#include <gui/widgets/seq_desktop/desktop_traverser.hpp>
#include <gui/widgets/seq_desktop/desktop_canvas.hpp>

#include <wx/settings.h>
#include <wx/dcclient.h>
#include <wx/menu.h>
#include <wx/dcbuffer.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

BEGIN_EVENT_TABLE(CDesktopCanvas, wxScrolledWindow)
    EVT_PAINT(CDesktopCanvas::OnPaint)
    EVT_MOUSE_EVENTS(CDesktopCanvas::OnMouseEvent)
    EVT_ERASE_BACKGROUND(CDesktopCanvas::OnEraseBackground)

    EVT_CONTEXT_MENU(CDesktopCanvas::x_OnContextMenu)
END_EVENT_TABLE()


CDesktopCanvas::CDesktopCanvas(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : m_ExpansionState(0), m_Host(), m_Context(nullptr)
{
    Create(parent, id, pos, size, style);
    Init();
}

CDesktopCanvas::~CDesktopCanvas()
{
    x_ClearItems();

    if (m_Context.get()) {
        m_Context->SetCanvas(nullptr);
    }
}

void CDesktopCanvas::Init()
{
    SetBackgroundColour(*wxWHITE);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void CDesktopCanvas::PaintBackground(wxDC& dc)
{
    wxColour bg = GetBackgroundColour();
    if (!bg.Ok()) {
        bg = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
    }

    dc.SetBrush(wxBrush(bg));
    dc.SetPen(wxPen(bg, 1));

    wxRect windowRect(wxPoint(0, 0), GetClientSize());

    // need to shift the client rectangle to take into account scrolling
    // converting device to logical coordinates
    CalcUnscrolledPosition(windowRect.x, windowRect.y, &windowRect.x, &windowRect.y);

    dc.DrawRectangle(windowRect);
}

void CDesktopCanvas::OnEraseBackground(wxEraseEvent& event)
{
}

void CDesktopCanvas::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);
    const wxFont font(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Consolas"));
    wxScrolledWindow::SetFont(font);
    dc.SetFont(font);

    SetScrollRate(dc.GetCharWidth(), dc.GetCharHeight());

    // shift the device origin so we don't have to worry about
    // the current scroll position 
    PrepareDC(dc);

    // Paint the background
    PaintBackground(dc);

    // paint the graphic
    DrawItems(dc);
}

bool CDesktopCanvas::Layout()
{
    if (m_RootItem) {
        SetVirtualSize(m_RootItem->GetSize() + wxSize(50, 50)); // add arbitrary white space around the main item
    }
    else
        SetVirtualSize(wxSize(0, 0));
    return true;
}

void CDesktopCanvas::OnMouseEvent(wxMouseEvent& event)
{
    if (event.LeftDown()) {
        wxPoint pnt;
        CalcUnscrolledPosition(event.m_x, event.m_y, &pnt.x, &pnt.y);
        CIRef<IDesktopItem> item = FindItem(pnt);

        if (item) {
            if (m_Selected) {
                if (m_Selected == item) {
                    // deselect the item
                    x_ResetSelected();
                }
                else {
                    // set item as the new selected one
                    x_ResetSelected();
                    m_Selected = item;
                    m_Selected->SetSelected(true);
                }
            }
            else {
                m_Selected = item;
                m_Selected->SetSelected(true);
            }
        }
        else {
            x_ResetSelected();
        }
        Refresh();
        Update();
        // send the event to the parent 
        // as this event does not propagate upwards
        wxWindow* parent = GetParent();
        if (parent) {
            parent->ProcessWindowEvent(event);
        }
        ObjectSelectionChanged();

    } else if (event.LeftDClick()) {
        wxPoint pnt;
        CalcUnscrolledPosition(event.m_x, event.m_y, &pnt.x, &pnt.y);
        CIRef<IDesktopItem> item = FindItem(pnt);

        if (item) {
            if (m_Selected) {
                if (m_Selected != item) {
                    // set item as the new selected one
                    x_ResetSelected();
                    m_Selected = item;
                    m_Selected->SetSelected(true);
                }
            } else {
                m_Selected = item;
                m_Selected->SetSelected(true);
            }
            ObjectSelectionChanged();
        }

        if (m_Selected && m_Context) {
            m_Context->EditSelection();
        }
    }
    event.Skip();
}

void CDesktopCanvas::DrawItems(wxDC& dc)
{
    wxSize size = GetClientSize();

    wxRect clipBox;
    CalcUnscrolledPosition(0, 0, &clipBox.x, &clipBox.y);
    CalcUnscrolledPosition(size.GetWidth(), size.GetHeight(), &clipBox.width, &clipBox.height);
    
    for (auto& it : m_DisplayList) {
        const IDesktopItem& item = it.GetObject();
        wxRect rect_box(item.GetPosition(), item.GetSize());
        if (rect_box.Intersects(clipBox)) {
            item.Draw(dc);
        }
    }
}

CIRef<IDesktopItem> CDesktopCanvas::FindItem(const wxPoint& pt) const
{
    return CIRef<IDesktopItem>(m_RootItem->FindItem(pt));
}

void CDesktopCanvas::ObjectSelectionChanged()
{
    if (m_Host) {
        m_Host->WidgetSelectionChanged();
    }
}

void CDesktopCanvas::SetSelectedObjects(const TConstObjects& objs)
{
    if (m_Selected) {
        m_Selected->SetSelected(false);
        m_Selected.Reset();
    }

    set<const CObject*> objSet;
    ITERATE(vector< CConstRef<CObject> >, it, objs)
        objSet.insert(it->GetPointer());

    if (objSet.empty()) {
        Refresh();
        return;
    }

    set<const CObject*>::iterator iter = objSet.begin();
    auto key_iter = m_ObjectToItem.find(*iter);
    if (key_iter != m_ObjectToItem.end()) {
        m_Selected.Reset(key_iter->second);
        m_Selected->SetSelected(true);
    }

    x_ScrollToSelectedItem();
    Refresh();
}

void CDesktopCanvas::GetSelectedObjects(TConstObjects& objects) const
{
    if (!m_Selected) return;

    CConstRef<CObject> obj = m_Selected->GetAssociatedObject();
    if (obj) {
        objects.push_back(obj);
    }
}

void CDesktopCanvas::x_EraseItem(CIRef<IDesktopItem> item, wxDC& dc)
{
    wxSize sz = GetClientSize();
    wxRect rect(0, 0, sz.x, sz.y);

    wxRect rect2(item->GetRect());
    dc.SetClippingRegion(rect2.x, rect2.y, rect2.width, rect2.height);

    dc.DestroyClippingRegion();
}

void CDesktopCanvas::x_ClearItems()
{
    m_DisplayList.clear();
    m_ObjectToItem.clear();
}

void CDesktopCanvas::x_ScrollToSelectedItem()
{
    if (m_Selected && m_Selected->IsShown()) {
        int xUnit, yUnit;
        GetScrollPixelsPerUnit(&xUnit, &yUnit);
        Scroll(0, m_Selected->GetPosition().y / yUnit);
    }
}

void CDesktopCanvas::x_ResetSelected()
{
    if (m_Selected) {
        m_Selected->SetSelected(false);
        m_Selected.Reset();
    }
}

namespace {
    class CDisplayTraverser : public IDesktopCompositeTraverser
    {
    public:
        CDisplayTraverser(CDesktopCanvas::TItemList& list) : m_List(list) {}

        virtual bool ProcessItem(CDesktopItem& item)
        {
            if (item.IsShown()) {
                m_List.push_back(CIRef<IDesktopItem>(&item));
            }
            return true;
        }
        
        CDesktopCanvas::TItemList& m_List;
    };

    class CObjectToDesktopItemTraverser : public IDesktopCompositeTraverser
    {
    public:
        CObjectToDesktopItemTraverser(map<const CObject*, CIRef<IDesktopItem>>& object_to_item)
            : m_Map(object_to_item) {}

        virtual bool ProcessItem(CDesktopItem& item)
        {
            CConstRef<CObject> obj = item.GetAssociatedObject();
            if (obj)
                m_Map.insert(map<const CObject*, CIRef<IDesktopItem>>::value_type(obj.GetPointer(), CIRef<IDesktopItem>(&item)));
            return true;
        }

        map<const CObject*, CIRef<IDesktopItem>>& m_Map;
    };

}

void CDesktopCanvas::SetMainItem(IDesktopItem* main_item, int state)
{
    m_Highlighted.Reset();
    m_Selected.Reset();
    m_ObjectToItem.clear();
    m_DisplayList.clear();
    
    m_RootItem.Reset(main_item);
    m_RootItem->Show(true);

    // collect the associated objects
    CObjectToDesktopItemTraverser obj_traverser(m_ObjectToItem);
    m_RootItem->Traverse(obj_traverser);

    m_ExpansionState = state;
    if (m_ExpansionState > 0) {
        for (int i = 0; i < m_ExpansionState; ++i) {
            m_RootItem->Expand();
        }
        int cumY;
        m_RootItem->UpdatePositions(&cumY);
        m_RootItem->UpdateSize();
    }

    CDisplayTraverser traverser(m_DisplayList);
    m_RootItem->Traverse(traverser);

    Layout();
    Scroll(0, 0);
    Refresh();

}

void CDesktopCanvas::ZoomIn(void)
{
    m_RootItem->Expand();
    int cumY;
    m_RootItem->UpdatePositions(&cumY);
    m_RootItem->UpdateSize();
    
    m_DisplayList.clear();
    CDisplayTraverser traverser(m_DisplayList);
    m_RootItem->Traverse(traverser);

    m_ExpansionState++;
    Layout();
    x_ScrollToSelectedItem();
    Refresh();
}

void CDesktopCanvas::ZoomOut(void)
{
    m_RootItem->Collapse();
    int cumY;
    m_RootItem->UpdatePositions(&cumY);
    m_RootItem->UpdateSize();

    if (m_Selected && !m_Selected->IsShown()) {
        x_ResetSelected();
        ObjectSelectionChanged();
    }
    m_DisplayList.clear();
    CDisplayTraverser traverser(m_DisplayList);
    m_RootItem->Traverse(traverser);

    if (m_ExpansionState > 0) m_ExpansionState--;
    Layout();
    x_ScrollToSelectedItem();
    Refresh();
}

void CDesktopCanvas::SetContext(CDesktopViewContext* context)
{
    m_Context.reset(context);
    if (m_Context.get()) {
        m_Context->SetCanvas(this);
    }
}

void CDesktopCanvas::x_OnContextMenu(wxContextMenuEvent& event)
{
    if (!m_Context.get() || !m_Selected) return;

    wxMenu* menu = m_Context->CreateMenu();

    // show the menu
    PopupMenu(menu);
    delete menu;
}

namespace {
    class CSearchStringLocal
    {
    public:
        CSearchStringLocal(const string& search, bool case_sensitive, CIRef<IDesktopItem> start_search, const CDesktopCanvas::TItemList& list)
            : m_SearchTerm(search), m_CaseSensitive(case_sensitive), m_StartSearchItem(start_search), m_List(list), m_Count(0), m_Canceled() {}

        bool Execute(ICanceled& canceled) // returns true if search completed (not canceled)
        {
            if (canceled.IsCanceled())
                return false;
            m_Canceled = &canceled;
            x_Execute();
            return !canceled.IsCanceled();
        }

        IDesktopItem* GetFoundItem() const { return m_FoundItem.GetNCPointerOrNull(); }

    private:
        void x_Execute();

        const string& m_SearchTerm;
        bool m_CaseSensitive;
        CIRef<IDesktopItem> m_StartSearchItem;
        CIRef<IDesktopItem> m_FoundItem;
        const CDesktopCanvas::TItemList& m_List;
        short m_Count;
        ICanceled* m_Canceled;
    };

    void CSearchStringLocal::x_Execute()
    {
        if (m_StartSearchItem) {
            auto it = find(m_List.begin(), m_List.end(), m_StartSearchItem);
            while (!m_Canceled->IsCanceled() && it != m_List.end() && !(*it)->FindSearchItem(m_SearchTerm, m_CaseSensitive)) {
                ++it;
            }
            if (it != m_List.end()) {
                m_FoundItem = *it;
            }
            else {
                m_Count++;
                if (m_Count < 2) {
                    m_StartSearchItem = m_List.front();
                    x_Execute();
                }
            }
        }
    }
} // end of namespace


bool CDesktopCanvas::FindText(const string& search)
{
    CIRef<IDesktopItem> start_search = m_RootItem;
    if (m_Selected) {
        auto it = find(m_DisplayList.begin(), m_DisplayList.end(), m_Selected);
        ++it;
        if (it != m_DisplayList.end()) {
            start_search = *it;
        }
    }

    CSearchStringLocal find(search, false, start_search, m_DisplayList);
    if (!GUI_AsyncExec([&find](ICanceled& canceled) { return find.Execute(canceled); }, wxT("Searching...")))
        return true;

    CIRef<IDesktopItem> item(find.GetFoundItem());

    if (item) {
        if (m_Selected) {
            x_ResetSelected();
        }

        m_Selected = item;
        m_Selected->SetSelected(true);
        ObjectSelectionChanged();

        Layout();
        x_ScrollToSelectedItem();
        Refresh();
        return true;
    }
    return false;
}

END_NCBI_SCOPE
