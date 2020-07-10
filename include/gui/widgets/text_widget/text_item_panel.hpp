#ifndef GUI_WIDGETS_TEXT_WIDGET___TEXT_ITEM_PANEL__HPP
#define GUI_WIDGETS_TEXT_WIDGET___TEXT_ITEM_PANEL__HPP

/*  $Id: text_item_panel.hpp 41060 2018-05-16 18:11:00Z katargir $
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

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <vector>
#include <set>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/scrolwin.h>
#include <wx/timer.h>
#include <wx/dc.h>

#include <util/icanceled.hpp>

#include <gui/objutils/objects.hpp>

#include <gui/widgets/text_widget/text_item.hpp>
#include <gui/widgets/text_widget/composite_text_item.hpp>

BEGIN_NCBI_SCOPE

class CTextItemPanel;
class ICanceled;
class ITextWidgetHost;
class CDrawTextOStream;

class NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CTextPanelContext
    : public wxEvtHandler
{
friend class CTextItemPanel;
friend class CItemStackGuard;

public:
    CTextPanelContext(int leftMargin, objects::CScope* scope, const CSerialObject* so) :
                          m_ItemCollapsed(false), m_Panel(0),
                          m_Scope(scope), m_SO(so),
                          m_MouseOverItem(0),
                          m_LeftMargin(leftMargin), 
                          m_HighlightedItem(0), m_WSize(wxDefaultSize), m_Canceled() {}
    ~CTextPanelContext();

    CTextItemPanel* GetPanel() { return m_Panel; }

    // in pixels
    int GetIndentWidth() const { return GetWWidth(); }
    virtual int GetLeftMargin() const { return m_LeftMargin*GetWWidth(); }

    virtual void RootInitialized() {}

    void SelectItem(ITextItem* item, bool controlDown, bool shiftDown);
    virtual void EditItem(ITextItem* item, bool controlDown, bool shiftDown);
    bool IsItemSelected(ITextItem* item);

    void GetSelectedObjects (TConstObjects& objs);
    void SetSelectedObjects (const TConstObjects& objs);

    void GetActiveObjects(TConstScopedObjects& objects);
    void GetActiveObjects(vector<TConstScopedObjects>& objects);

    void GetSelectedObjects (TConstScopedObjects& objects);
    virtual void GetVisibleObjects (TConstScopedObjects& /*objects*/) {}
    virtual void GetMainObjects (TConstScopedObjects& objects);
    const set<ITextItem*>  &GetSelectedTextItems(void) {return  m_SelectedItems;}

// Tooltip functions    
    void    ClearMouseMove();
    void    AddHier(ITextItem* item);
    void    MouseOverItem(ITextItem* item);
    void    TriggerTooltip();

    virtual void MouseMoveInContainer(ITextItem* /*container*/) {}
    virtual wxWindow* CreateTooltipWindow() { return NULL; }

    bool    m_ItemCollapsed;

    ITextItem* GetHighlightedItem() { return m_HighlightedItem; }
    void SetHighlightedItem(ITextItem* pItem) { m_HighlightedItem = pItem; }

    virtual wxMenu* CreateMenu() const { return 0; }

    virtual void SetPanel(CTextItemPanel* panel);

    objects::CScope* GetScope() { return m_Scope.GetPointerOrNull(); }

    void   CalcWSize(wxDC& dc);
    wxSize GetWSize() const { return m_WSize; }
    void   SetWSize(wxSize wSize) { m_WSize = wSize; }
    int    GetWWidth() const { return m_WSize.GetWidth(); }
    int    GetWHeight() const { return m_WSize.GetHeight(); }

    void  SetFontDesc(wxWindow& wnd);
    void  SetFontDesc(const wxString& fontDesc) { m_FontDesc = fontDesc; }
    const wxString& GetFontDesc() const { return m_FontDesc; }

// The MS Windows device size is 2^27
// We can overflow 
    wxPoint GetDeviceOrigin() const { return m_DeviceOrigin; }
    void    SetDeviceOrigin(wxPoint drawDeviceOrigin) { m_DeviceOrigin = drawDeviceOrigin; }
    void    SetDeviceOrigin(wxDC& dc) const { dc.SetDeviceOrigin(m_DeviceOrigin.x, m_DeviceOrigin.y); }

    ICanceled* SetCanceled(ICanceled* canceled)
    {
        ICanceled* old = m_Canceled;
        m_Canceled = canceled;
        return old;
    }

    bool IsCanceled() const { return m_Canceled ? m_Canceled->IsCanceled() : false; }

    virtual bool FindSequence(const wxString& /*text*/) { return false; }

    virtual void InitDrawStream(CDrawTextOStream& /*stream*/, int /*startLine*/) {}

    virtual bool IsEditingEnabled() const { return false; }

    void SetWorkDir(const wxString& workDir) { m_WorkDir = workDir; }

protected:
    CTextItemPanel* m_Panel;
    CRef<objects::CScope> m_Scope;
    CConstRef<CSerialObject> m_SO;

// Tooltip data
    vector<ITextItem*> m_Hier;
    ITextItem*         m_MouseOverItem;
    set<ITextItem*>    m_SelectedItems;
    wxString m_WorkDir;

private:
    int m_LeftMargin; // number of chars
    ITextItem* m_HighlightedItem;

    wxSize  m_WSize;
    wxPoint m_DeviceOrigin;

    wxString m_FontDesc;

    ICanceled* m_Canceled;
};

class NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CTextItemPanel : public wxScrolledWindow
{
public:
    CTextItemPanel(wxWindow* parent, wxWindowID id = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize,
                      long style = wxBORDER_NONE | wxHSCROLL | wxVSCROLL);
    ~CTextItemPanel();

    void SetHost(ITextWidgetHost* host) { m_Host = host; }
    void ObjectSelectionChanged();

    void Init();

    virtual bool Layout();
    virtual bool SetFont(const wxFont& font);

    void SetMainItem(ITextItem* item, CTextPanelContext* context);

    CTextPanelContext* GetContext() { return m_Context.get(); }
    void RecalSize();

    void ReportError(const string& report);
    void ReportLoading();

    void ScrollToBottom();

    ITextItem* GetRootItem() { return m_RootItem.get(); }
    void InitDC(wxDC& dc);

	virtual void OnDraw(wxDC& dc);

    bool FindText(const wxString& text, bool matchCase, bool forward);
    bool FindSequence(const wxString& text);

    void MakePosVisible(const CTextPosition& pos);
    void MakeItemVisible(ITextItem* item);

    bool MakeObjectVisible(const CObject& object);

//
// Text selection functions
//
    bool IsTextSelected() const;
    wxString GetSelectedText() const;

    CTextBlock GetTextSelection() const { return m_TextSelection; }
    bool Selecting() const { return m_Selecting; }

    void SelStart(int row, int col, bool shift);
    void SelMove(int row, int col);
    void SelEnd(int row, int col);

    void SetSelection(const CTextBlock& selection);

    void SetCaretPos(int x, int y);
    void UpdateCaretPos();

    void TriggerTooltip();
    wxPoint GetMousePos() const { return m_MousePos; }

	typedef multimap<const CObject*, ITextItem*> TObjectToItem;
	TObjectToItem& GetObjectToItem() { return m_ObjectToItem; }

	void SetTrackSelection(bool trackSelection) { m_TrackSelection = trackSelection; }
	bool GetTrackSelection() const { return m_TrackSelection; }

protected:
    void OnMouseEvent(wxMouseEvent& event);
    void OnContextMenu(wxContextMenuEvent& event);
    void OnSetFocus(wxFocusEvent& event);
    void OnKillFocus(wxFocusEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnCaptureLost(wxMouseCaptureLostEvent& event);

	void x_Draw(wxDC& dc);

// Text selection
    void OnCopy(wxCommandEvent& event);
    void OnUpdateCopy(wxUpdateUIEvent& event);
    void OnSelectAll(wxCommandEvent& event);

    void x_TextPosChanged();

    auto_ptr<CTextPanelContext> m_Context;
    auto_ptr<ITextItem> m_RootItem;
    int                 m_SaveIndex;

    bool m_Selecting;
    CTextBlock m_TextSelection;

    wxTimer     m_Timer;
    wxWindow*   m_TooltipWindow;
    wxPoint     m_MousePos;

	bool m_TrackSelection;

	TObjectToItem m_ObjectToItem;

    DECLARE_EVENT_TABLE()

	wxBitmap m_Buffer;

    ITextWidgetHost* m_Host;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TEXT_WIDGET___TEXT_ITEM_PANEL__HPP
