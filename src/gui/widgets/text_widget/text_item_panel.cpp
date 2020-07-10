/*  $Id: text_item_panel.cpp 44273 2019-11-21 17:33:12Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/utils/view_event.hpp>
#include <gui/utils/event_handler.hpp>

#include <gui/widgets/wx/popup_event.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/async_call.hpp>

#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/text_widget/text_widget_host.hpp>
#include <gui/widgets/text_widget/text_item.hpp>
#include <gui/widgets/text_widget/composite_text_item.hpp>
#include <gui/widgets/text_widget/calc_caretpos_stream.hpp>
#include <gui/widgets/text_widget/text_line_iterator.hpp>
#include <gui/widgets/text_widget/expand_item.hpp>
#include <gui/widgets/text_widget/expand_traverser.hpp>

#include <gui/widgets/text_widget/root_text_item.hpp>
#include <gui/widgets/text_widget/plain_text_item.hpp>

#include <gui/widgets/data/object_contrib.hpp>


#include <gui/utils/menu_item.hpp>

#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/dcclient.h>
#include <wx/dcbuffer.h>
#include <wx/settings.h>
#include <wx/caret.h>

#include <wx/sstream.h>
#include <wx/txtstrm.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>

BEGIN_NCBI_SCOPE

CTextPanelContext::~CTextPanelContext()
{
    if (m_Panel) {
        m_Panel->PopEventHandler();
    }
}

void CTextPanelContext::CalcWSize(wxDC& dc)
{
    m_WSize = dc.GetTextExtent(wxT("W"));
}

void CTextPanelContext::SetPanel(CTextItemPanel* panel)
{
    if (m_Panel) {
        m_Panel->PopEventHandler();
    }

    m_Panel = panel;

    if (m_Panel) {
        m_Panel->PushEventHandler(this);
    }
}

void  CTextPanelContext::SetFontDesc(wxWindow& wnd)
{
    wxFont font = wnd.GetFont();
    m_FontDesc = font.GetNativeFontInfoDesc();
}

void CTextPanelContext::SelectItem(ITextItem* item, bool controlDown, bool WXUNUSED(shiftDown))
{
    if (!item) {
        m_SelectedItems.clear();
        m_Panel->Refresh();
        m_Panel->ObjectSelectionChanged();
        return;
    }

    CConstRef<CObject> obj = item->GetAssosiatedObject();
    if (!controlDown) {
        m_SelectedItems.clear();
    }

    set<ITextItem*>::iterator it = m_SelectedItems.find(item);
    if (it != m_SelectedItems.end()) {
        if (obj) {
            set<ITextItem*> newSelection;
            for (it = m_SelectedItems.begin(); it != m_SelectedItems.end();++it) {
                if ((*it)->GetAssosiatedObject() != obj)
                    newSelection.insert(newSelection.end(), *it);
            }
            m_SelectedItems = newSelection;
        } else {
            m_SelectedItems.erase(it);
        }
    }
    else {
        if (obj) {
            std::pair <CTextItemPanel::TObjectToItem::iterator, CTextItemPanel::TObjectToItem::iterator> ret =
                m_Panel->GetObjectToItem().equal_range(obj.GetPointer());
            for (CTextItemPanel::TObjectToItem::iterator it2=ret.first; it2!=ret.second; ++it2) {
                m_SelectedItems.insert(it2->second);
            }
        } else {
            m_SelectedItems.insert(item);
        }
    }

    m_Panel->Refresh();
    m_Panel->ObjectSelectionChanged();
}

void CTextPanelContext::EditItem(ITextItem* WXUNUSED(item), bool WXUNUSED(controlDown), bool WXUNUSED(shiftDown))
{
}

bool CTextPanelContext::IsItemSelected(ITextItem* item)
{
    return (m_SelectedItems.find(item) != m_SelectedItems.end());
}

void CTextPanelContext::GetSelectedObjects (TConstObjects& objs)
{
    set<const CObject*> objSet;
    ITERATE (set<ITextItem*>, it, m_SelectedItems) {
        CConstRef<CObject> object = (*it)->GetAssosiatedObject();
        if (object && objSet.find(object.GetPointer()) == objSet.end()) {
            objSet.insert(object.GetPointer());
            objs.push_back(CConstRef<CObject>(object));
        }
    }
}

void CTextPanelContext::GetSelectedObjects (TConstScopedObjects& objects)
{
    set<const CObject*> objSet;
    ITERATE (set<ITextItem*>, it, m_SelectedItems) {
        CConstRef<CObject> object = (*it)->GetAssosiatedObject();
        if (object && objSet.find(object.GetPointer()) == objSet.end()) {
            objSet.insert(object.GetPointer());
            SConstScopedObject scopes_object;
            scopes_object.object = object;
            objects::CScope* scope = (*it)->GetAssosiatedScope();
            if (scope == 0)
                scope = m_Scope.GetPointerOrNull();
            scopes_object.scope.Reset(scope);
            objects.push_back(scopes_object);
        }
    }
}

void CTextPanelContext::GetActiveObjects(TConstScopedObjects& objects)
{
    GetSelectedObjects(objects);
    GetVisibleObjects(objects);
    GetMainObjects(objects);
}

void CTextPanelContext::GetActiveObjects(vector<TConstScopedObjects>& objects)
{
    TConstScopedObjects objs;

    GetSelectedObjects(objs);
    if (!objs.empty()) {
        if (objects.size() < 1)
            objects.resize(1);
        objects[0].insert(objects[0].end(), objs.begin(), objs.end());
    }
    objs.clear();

    GetVisibleObjects(objs);
    if (!objs.empty()) {
        if (objects.size() < 2)
            objects.resize(2);
        objects[1].insert(objects[1].end(), objs.begin(), objs.end());
    }
    objs.clear();

    GetMainObjects(objs);
    if (!objs.empty()) {
        if (objects.size() < 3)
            objects.resize(3);
        objects[2].insert(objects[2].end(), objs.begin(), objs.end());
    }
    objs.clear();
}

void CTextPanelContext::GetMainObjects (TConstScopedObjects& objects)
{
    if (m_SO) {
        SConstScopedObject scoped_object;
        scoped_object.object = m_SO;
        scoped_object.scope = m_Scope;
        objects.push_back(scoped_object);
    }
}

void CTextPanelContext::ClearMouseMove()
{
    m_Hier.clear();
    m_MouseOverItem = 0;
}

void CTextPanelContext::AddHier(ITextItem* item)
{
    m_Hier.push_back(item);
}

void CTextPanelContext::MouseOverItem(ITextItem* item)
{
    m_MouseOverItem = item;
}

void CTextPanelContext::TriggerTooltip()
{
    if (m_Panel)
        m_Panel->TriggerTooltip();
}

void CTextPanelContext::SetSelectedObjects (const TConstObjects& objs)
{
    m_SelectedItems.clear();

    set<const CObject*> objSet;
    ITERATE (vector< CConstRef<CObject> >, it, objs)
        objSet.insert(it->GetPointer());

    if (objSet.empty())
        return;

    CTextItemPanel::TObjectToItem& objToItem = m_Panel->GetObjectToItem();

    ITERATE(set<const CObject*>, it, objSet) {
        std::pair <CTextItemPanel::TObjectToItem::iterator, CTextItemPanel::TObjectToItem::iterator> ret =
            objToItem.equal_range(*it);
        for (CTextItemPanel::TObjectToItem::iterator it2=ret.first; it2!=ret.second; ++it2) {
            m_SelectedItems.insert(it2->second);
        }

    }

    if (m_Panel->GetTrackSelection() && m_SelectedItems.size() > 0) {
        m_Panel->MakeItemVisible(*m_SelectedItems.begin());
    }
    m_Panel->Refresh();
}

BEGIN_EVENT_TABLE( CTextItemPanel, wxScrolledWindow )
    EVT_MOUSE_EVENTS( CTextItemPanel::OnMouseEvent )
    EVT_MOUSE_CAPTURE_LOST(CTextItemPanel::OnCaptureLost)
    EVT_SET_FOCUS(CTextItemPanel::OnSetFocus)
    EVT_KILL_FOCUS(CTextItemPanel::OnKillFocus)
    EVT_TIMER(-1, CTextItemPanel::OnTimer)
    EVT_CONTEXT_MENU( CTextItemPanel::OnContextMenu )
    EVT_MENU( wxID_COPY, CTextItemPanel::OnCopy )
    EVT_UPDATE_UI( wxID_COPY, CTextItemPanel::OnUpdateCopy )
    EVT_MENU( wxID_SELECTALL, CTextItemPanel::OnSelectAll )
END_EVENT_TABLE()

CTextItemPanel::CTextItemPanel(wxWindow* parent, wxWindowID id,
                               const wxPoint& pos, const wxSize& size, long style)
    : m_Selecting(false), m_Timer(this), m_TooltipWindow(0), m_MousePos(-1,-1),
      m_TrackSelection(false), m_Host()
{
    Create(parent, id, pos, size, style);
    Init();
}

CTextItemPanel::~CTextItemPanel()
{
    if (m_Context.get())
        m_Context->SetPanel(0);
}

void CTextItemPanel::Init()
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    wxClientDC dc(this);
    InitDC(dc);
    SetScrollRate(dc.GetCharWidth(),dc.GetCharHeight());

#ifndef __WXOSX_COCOA__
    wxCaret *caret = new wxCaret(this, 1, dc.GetCharHeight());
    SetCaret(caret);
#endif
}

bool CTextItemPanel::IsTextSelected() const
{
    return !m_TextSelection.IsEmpty();
}

void CTextItemPanel::SelStart(int row, int col, bool shift)
{
    CTextPosition pos(row, col);

    if (shift)
        m_TextSelection = CTextBlock(m_TextSelection.GetStart(), pos);
    else
        m_TextSelection = CTextBlock(pos, pos);

    m_Selecting = true;
    CaptureMouse();
    x_TextPosChanged();
}

void CTextItemPanel::SelMove(int row, int col)
{
    m_TextSelection = CTextBlock(m_TextSelection.GetStart(), CTextPosition(row, col));
    x_TextPosChanged();
}

void CTextItemPanel::SelEnd(int row, int col)
{
    m_TextSelection = CTextBlock(m_TextSelection.GetStart(), CTextPosition(row, col));
    m_Selecting = false;
    if (HasCapture())
        ReleaseMouse();
    x_TextPosChanged();
}

void CTextItemPanel::SetSelection(const CTextBlock& selection)
{
    m_TextSelection = selection;
    x_TextPosChanged();
}

void CTextItemPanel::ObjectSelectionChanged()
{
    if (m_Host) m_Host->WidgetSelectionChanged();
}

void CTextItemPanel::x_TextPosChanged()
{
    Refresh();

    if (FindFocus() != this) return;

    if (m_Host) m_Host->OnTextPositionChanged(m_TextSelection.GetEnd().GetRow(), m_TextSelection.GetEnd().GetCol());
}

bool CTextItemPanel::Layout()
{
    if (m_RootItem.get())
        SetVirtualSize(m_RootItem->GetSize());
    else
        SetVirtualSize(wxSize(0,0));
    return true;
}

bool CTextItemPanel::SetFont(const wxFont& font)
{
    bool retVal = wxScrolledWindow::SetFont(font);

    wxClientDC dc(this);
    InitDC(dc);

    SetScrollRate(dc.GetCharWidth(),dc.GetCharHeight());

    wxCaret* caret = GetCaret();
    if (caret)
        caret->SetSize(1, dc.GetCharHeight());

    return retVal;
}

namespace {
    class CObjectToItemTraverser : public ICompositeTraverser
    {
    public:
        CObjectToItemTraverser(multimap<const CObject*, ITextItem*>& map) : m_Map(map) {}

        virtual bool ProcessItem(CTextItem& textItem)
        {
            CConstRef<CObject> obj = textItem.GetAssosiatedObject();
            if (obj)
                m_Map.insert(multimap<const CObject*, ITextItem*>::value_type(obj.GetPointer(), &textItem));
            return true;
        }
        virtual bool ProcessItem(CExpandItem& expandItem)
        {
            CConstRef<CObject> obj = expandItem.GetAssosiatedObject();
            if (obj)
                m_Map.insert(multimap<const CObject*, ITextItem*>::value_type(obj.GetPointer(), &expandItem));
            return true;
        }

        multimap<const CObject*, ITextItem*>& m_Map;
    };
}

void CTextItemPanel::SetMainItem(ITextItem* item, CTextPanelContext* context)
{
    m_ObjectToItem.clear();

    m_Context.reset(context);
    if (m_Context.get()) {
        m_Context->SetPanel(this);
    }

    m_TextSelection = CTextBlock();

    m_RootItem.reset(item);

    if (m_RootItem.get()) {
        CObjectToItemTraverser traverser(m_ObjectToItem);
        m_RootItem->Traverse(traverser);

        if (m_Context.get())
            m_Context->RootInitialized();
    }

    Layout();
    Scroll(0, 0);
    Refresh();
}

void CTextItemPanel::ReportError(const string& report)
{
    CRootTextItem* root = new CRootTextItem();
    CPlainTextItem* textItem = new CPlainTextItem();
    textItem->SetStyle(CTextStyle(255,0,0,255,255,255,wxTRANSPARENT,false,false));
    textItem->AddLine(report);
    root->AddItem(textItem);

    CTextPanelContext* context = new CTextPanelContext(1, 0, 0);

    wxClientDC dc(this);
    InitDC(dc);
    context->CalcWSize(dc);
    root->CalcSize(dc, context);
    SetMainItem(root, context);
}

void CTextItemPanel::ReportLoading()
{
    CRootTextItem* root = new CRootTextItem();
    CPlainTextItem* textItem = new CPlainTextItem();
    textItem->SetStyle(CTextStyle(255,0,0,255,255,255,wxTRANSPARENT,false,false));
    textItem->AddLine("Loading...");
    root->AddItem(textItem);

    CTextPanelContext* context = new CTextPanelContext(1, 0, 0);

    wxClientDC dc(this);
    InitDC(dc);
    context->CalcWSize(dc);
    root->CalcSize(dc, context);
    SetMainItem(root, context);
}

void CTextItemPanel::RecalSize()
{
    if (!m_RootItem.get()) return;

    wxBusyCursor wait;

    wxClientDC dc(this);
    InitDC(dc);

    m_Context->CalcWSize(dc);
    m_RootItem->CalcSize(dc, m_Context.get());
}

void CTextItemPanel::OnDraw(wxDC& dc)
{
    dc.SetDeviceOrigin(0, 0);

    if (IsDoubleBuffered()) {
        x_Draw(dc);
        return;
    }

    wxSize clientSize = GetClientRect().GetSize();
    int w = clientSize.GetWidth(), h = clientSize.GetHeight();

    if (!m_Buffer.IsOk() || m_Buffer.GetWidth() != w || m_Buffer.GetHeight() != h) {
        if (!m_Buffer.Create(w, h, dc)) {
            x_Draw(dc);
            return;
        }
    }

    wxMemoryDC memDC(m_Buffer);
    x_Draw(memDC);

    memDC.SetDeviceOrigin(0, 0);
    dc.Blit(0, 0, w, h, &memDC, 0, 0);
}

void CTextItemPanel::x_Draw(wxDC& dc)
{
    InitDC(dc);

// Fill background
    wxRect updateRect = GetUpdateRegion().GetBox();
    dc.DrawRectangle(updateRect);

    if (!m_RootItem.get() || !m_Context.get())
        return;

    wxPoint origin;
    CalcScrolledPosition(0,0,&origin.x, &origin.y);
    m_Context->SetDeviceOrigin(origin);
    CalcUnscrolledPosition(updateRect.x,updateRect.y,&updateRect.x, &updateRect.y);

    try {
        m_RootItem->Draw(dc, updateRect, m_Context.get());
    }
    catch (const exception&) {
    }
}

void CTextItemPanel::InitDC(wxDC& dc)
{
    wxColor bkgdColor = GetBackgroundColour();
    if (!bkgdColor.Ok())
        bkgdColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);

    dc.SetBrush(wxBrush(bkgdColor));
    dc.SetPen(wxPen(bkgdColor,1));
    dc.SetTextBackground(bkgdColor);
    dc.SetFont(GetFont());
}

void CTextItemPanel::OnMouseEvent(wxMouseEvent& event)
{
    if (event.LeftDown() && FindFocus() != this)
        SetFocus();

    if (m_TooltipWindow && event.GetEventType() != wxEVT_LEAVE_WINDOW) {
        m_TooltipWindow->Destroy();
        m_TooltipWindow = 0;
    }

    m_MousePos = event.GetPosition();

    if (m_RootItem.get() && m_Context.get()) {
        wxMouseEvent ev(event);
        CalcUnscrolledPosition(event.m_x, event.m_y, &ev.m_x, &ev.m_y);
        m_Context->ClearMouseMove();
        m_RootItem->MouseEvent(ev, *m_Context);

        if (event.Moving())
            TriggerTooltip();
        //wxRect rect = m_RootItem->GetRect();
        //if (rect.Contains(ev.m_x, ev.m_y))
            //m_RootItem->ProcessEvent(ev);
        //else if (event.GetEventType() == wxEVT_MOTION)
            //SetCursor(wxCursor(wxCURSOR_ARROW));
    }

    if (!event.LeftDown())
        event.Skip();
}

void CTextItemPanel::OnCaptureLost(wxMouseCaptureLostEvent& event)
{
    if (m_RootItem.get() && m_Context.get()) {
        wxMouseEvent ev(wxEVT_LEFT_UP);
        ev.SetEventObject(this);
        CalcUnscrolledPosition(m_MousePos.x, m_MousePos.y, &ev.m_x, &ev.m_y);
        m_Context->ClearMouseMove();
        m_RootItem->MouseEvent(ev, *m_Context);
    }
}

void CTextItemPanel::SetCaretPos(int x, int y)
{
    wxCaret* caret = GetCaret();
    if (!caret) return;

    CalcScrolledPosition(x, y, &x, &y);
    caret->Move(x, y);
}

void CTextItemPanel::UpdateCaretPos()
{
    wxCaret* caret = GetCaret();
    if (!caret) return;

    if (!m_RootItem.get()) return;

    CTextPosition caretPos = m_TextSelection.GetEnd();

    CTextItem* textItem = dynamic_cast<CTextItem*>(m_RootItem->GetItemByLine(caretPos.GetRow(), false));
    if (textItem == 0)
        return;

    wxClientDC dc(this);
    InitDC(dc);

    caretPos.ShiftRow(-textItem->GetStartLine());
    textItem->TextPosToCollapsed(caretPos, m_Context.get());
    CCalcCaretPosOStream ostr(dc, m_Context->GetLeftMargin(), 0,
                              dc.GetCharHeight(), m_Context->GetIndentWidth(),
                              caretPos.GetCol(), caretPos.GetRow());

    textItem->RenderText(ostr, m_Context.get());
    int x, y;
    ostr.GetCaretPos(x, y);
    wxPoint itemPos = textItem->GetPosition();
    CalcScrolledPosition(x + itemPos.x, y + itemPos.y, &x, &y);
    caret->Move(x, y);
}

static
WX_DEFINE_MENU(kPopupMenu)
    WX_MENU_SEPARATOR_L("Top Actions")
    WX_MENU_SEPARATOR_L("Text Commands")
    WX_MENU_ITEM(wxID_COPY)
    WX_MENU_ITEM(wxID_SELECTALL)
WX_END_MENU()

void CTextItemPanel::OnSetFocus(wxFocusEvent& WXUNUSED(event))
{
    wxCaret* caret = GetCaret();
    if (caret) caret->Show();
    if (m_Host) m_Host->OnTextGotFocus();
}

void CTextItemPanel::OnKillFocus(wxFocusEvent& WXUNUSED(event))
{
    wxCaret* caret = GetCaret();
    if (caret) caret->Hide();
    if (m_Host) m_Host->OnTextLostFocus();
}

void CTextItemPanel::OnContextMenu(wxContextMenuEvent& event)
{
    wxPoint point = event.GetPosition();
    // If from keyboard
    if (point.x == -1 && point.y == -1) {
        wxSize size = GetSize();
        point.x = size.x / 2;
        point.y = size.y / 2;
    }
    else {
        point = ScreenToClient(point);
    }

    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    wxMenu* menu = cmd_reg.CreateMenu(kPopupMenu);

    TConstScopedObjects sel_objects;
    if (m_Context.get()) {
        unique_ptr<wxMenu> contextMenu(m_Context->CreateMenu());
        if (contextMenu.get()) {
            Merge(*menu, *contextMenu);
        }
        m_Context->GetActiveObjects(sel_objects);
    }

    vector<wxEvtHandler*> handlers;
    AddCommandsForScopedObjects( 
        *menu, 
        handlers, 
        EXT_POINT__SCOPED_OBJECTS__CMD_CONTRIBUTOR, 
        sel_objects 
    );

    // obtain the modified menu and clean it empty sections
    CleanupSeparators(*menu);

    for( size_t  i = 0;  i < handlers.size();  i++  )   {
        PushEventHandler(handlers[i]);
    }

    // show the menu
    PopupMenu(menu, point);
    delete menu;

    // disconnect and destroy the handlers
    for( size_t  i = 0;  i < handlers.size();  i++  )   {
        wxEvtHandler* h = PopEventHandler();
        _ASSERT(h == handlers[i]);
        delete h;
    }
}

void CTextItemPanel::OnCopy(wxCommandEvent& WXUNUSED(event))
{
    if (!m_RootItem.get()) return;

    if (!IsTextSelected())
        return;

    if (wxTheClipboard->Open()) {
        wxStringOutputStream stream;
        wxTextOutputStream os(stream);
        m_RootItem->GetText(os, m_TextSelection.Normalized(), m_Context.get());
        wxTheClipboard->SetData( new wxTextDataObject(stream.GetString()));
        wxTheClipboard->Close();
    }
}

void CTextItemPanel::OnUpdateCopy(wxUpdateUIEvent& event)
{
    event.Enable(IsTextSelected());
}

void CTextItemPanel::OnSelectAll(wxCommandEvent& WXUNUSED(event))
{
    if (!m_RootItem.get()) return;
    SetSelection(CTextBlock(0, m_RootItem->GetLineCount()));
}

void CTextItemPanel::ScrollToBottom()
{
    int w, h, cw, ch;
    GetClientSize(&cw, &ch);
    GetVirtualSize(&w, &h);
    if (h <= ch) return;

    int xUnit, yUnit;
    GetScrollPixelsPerUnit(&xUnit, &yUnit);
    Scroll(0, (h - ch)/yUnit);
/*
    GetViewStart(&x, &y);
    CalcUnscrolledPosition(x, y, &x, &y);

    CalcScrolledPosition(x, h - ch, &x, &y);
    Scroll(x, y);
 */
}

bool CTextItemPanel::FindText(const wxString& searchStr, bool matchCase, bool forward)
{
    bool found = false, searchCanceled = false;

    CTextPosition pos = GUI_AsyncExec(
        [this, &searchStr, matchCase, forward, &found, &searchCanceled](ICanceled& canceled) -> CTextPosition
        {
            string str = ToStdString(searchStr);
            if (!matchCase)
                str = NStr::ToUpper(str);

            CTextBlock selNorm = this->GetTextSelection().Normalized();
            CTextPosition cursorPos = forward ? selNorm.GetEnd() : selNorm.GetStart();
            CTextPosition foundPos;
            for (CTextLineIterator it(this, cursorPos, forward); !canceled.IsCanceled() && it; ++it) {
                string line = *it;
                if (line.length() >= str.length()) {
                    size_t pos = matchCase ? line.find(str) : NStr::ToUpper(line).find(str);
                    if (pos != string::npos) {
                        foundPos = it.GetPosition();
                        foundPos.ShiftCol((int)pos);
                        found = true;
                        break;
                    }
                }
            }
            searchCanceled = canceled.IsCanceled();
            return foundPos;
        },
        wxT("Searching for \"") + searchStr + wxT("\"..."));

    if (searchCanceled)
        return true;

    if (!found)
        return false;

    CTextPosition start = pos;
    CTextPosition end(start);
    end.ShiftCol((int)searchStr.length());
    MakePosVisible(start);
    SetSelection(CTextBlock(start, end));
    return true;
}

bool CTextItemPanel::FindSequence(const wxString& text)
{
    return m_Context.get() ? m_Context->FindSequence(text) : false;
}

void CTextItemPanel::MakePosVisible(const CTextPosition& pos)
{
    if (!m_RootItem.get()) return;

    CTextItem* posItem = dynamic_cast<CTextItem*>(m_RootItem->GetItemByLine(pos.GetRow(), true));
    if (posItem == 0)
        return;

    for (ITextItem* item = posItem; item;) {
        IContainer* container = 0;
        int index = 0;
        item->GetContainer(container, index);
        CExpandItem* expandItem = dynamic_cast<CExpandItem*>(container);
        if (expandItem && !expandItem->IsExpanded())
            expandItem->Toggle(m_Context.get(), true);
        item = dynamic_cast<ITextItem*>(container);
    }

    int w, h, cw, ch;
    GetClientSize(&cw, &ch);
    GetVirtualSize(&w, &h);
    if (h <= ch) return;

    wxPoint itemPos = posItem->GetPosition();

    wxClientDC dc(this);
    InitDC(dc);

    CCalcCaretPosOStream ostr(dc, m_Context->GetLeftMargin(), 0,
                              dc.GetCharHeight(), m_Context->GetIndentWidth(),
                              pos.GetCol(), pos.GetRow() - posItem->GetStartLine());

    posItem->RenderText(ostr, m_Context.get());
    int x, y;
    ostr.GetCaretPos(x, y);
    int yN = y + itemPos.y;
    CalcScrolledPosition(x + itemPos.x, y + itemPos.y, &x, &y);

    int xUnit, yUnit;
    GetScrollPixelsPerUnit(&xUnit, &yUnit);

    //if (y + ch > h)
        //y = h - ch;
    if (y < 0)
        Scroll(0, yN/yUnit);
    else if (y + yUnit > ch)
        Scroll(0, (yN + 2*yUnit - 1 - ch)/yUnit);
}

void CTextItemPanel::MakeItemVisible(ITextItem* item)
{
    for (ITextItem* it = item; it;) {
        IContainer* container = 0;
        int index = 0;
        it->GetContainer(container, index);
        CExpandItem* expandItem = dynamic_cast<CExpandItem*>(container);
        if (expandItem && !expandItem->IsExpanded())
            expandItem->Toggle(m_Context.get(), true);
        it = dynamic_cast<ITextItem*>(container);
    }

    int w, h, cw, ch;
    GetClientSize(&cw, &ch);
    GetVirtualSize(&w, &h);
    if (h <= ch) return;

    wxPoint itemPos = item->GetPosition();
    wxSize itemSize = item->GetSize();

    int x, y;
    CalcScrolledPosition(0, itemPos.y, &x, &y);

    int xUnit, yUnit;
    GetScrollPixelsPerUnit(&xUnit, &yUnit);

    //if (y + ch > h)
        //y = h - ch;
    if (y < 0)
        Scroll(0, itemPos.y/yUnit);
    else if (y + itemSize.GetY() > ch) {
        if (itemSize.GetY() >= ch)
            Scroll(0, itemPos.y/yUnit);
        else
            Scroll(0, (itemPos.y + itemSize.GetY() - ch + yUnit - 1)/yUnit);
    }
}

bool CTextItemPanel::MakeObjectVisible(const CObject& object)
{
    std::pair <TObjectToItem::iterator, TObjectToItem::iterator> ret =
        m_ObjectToItem.equal_range(&object);

    if (ret.first != ret.second) {
        MakeItemVisible(ret.first->second);
        return true;
    }
    return false;
}


void CTextItemPanel::TriggerTooltip()
{
    if (m_TooltipWindow) {
        m_TooltipWindow->Destroy();
        m_TooltipWindow = 0;
    }

    m_Timer.Stop();
    m_Timer.Start(500, wxTIMER_ONE_SHOT);
}

void CTextItemPanel::OnTimer(wxTimerEvent& WXUNUSED(event))
{
    if (m_Context.get() == 0)
        return;

    if (m_Timer.IsRunning())
        return;

    m_TooltipWindow = m_Context->CreateTooltipWindow();
    if (m_TooltipWindow) {
        wxSize tooltipSize = m_TooltipWindow->GetSize();
        wxSize clientSize = GetClientSize();

        wxPoint pos = m_MousePos;

        if (pos.y + tooltipSize.GetHeight() > clientSize.GetHeight() &&
            pos.y - tooltipSize.GetHeight() >= 0) {
            pos.y -= tooltipSize.GetHeight();
        }

        if (pos.x + tooltipSize.GetWidth() > clientSize.GetWidth()) {
            pos.x = max(0, clientSize.GetWidth() - tooltipSize.GetWidth());
        }

        m_TooltipWindow->Move(pos.x, pos.y);
        m_TooltipWindow->Show();
    }
}

END_NCBI_SCOPE
