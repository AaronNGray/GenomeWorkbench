#ifndef GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_CANVAS__HPP
#define GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_CANVAS__HPP

/*  $Id: desktop_canvas.hpp 37865 2017-02-23 21:52:38Z asztalos $
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

#include <corelib/ncbistl.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/widgets/seq_desktop/desktop_item.hpp>
#include <gui/widgets/seq_desktop/desktop_view_context.hpp>

#include <wx/scrolwin.h>
#include <wx/dc.h>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CSeq_entry;
    class CBioseq;
    class CBioseq_set;
    class CScope;
END_SCOPE(objects)

class IGuiWidgetHost;

/// Defines a scrollable canvas for drawing purposes.
///
class CDesktopCanvas : public wxScrolledWindow
    // based on https://github.com/wxWidgets/wxWidgets/tree/master/samples/dragimag
{
    DECLARE_EVENT_TABLE()

public:

    typedef list<CIRef<IDesktopItem>> TItemList;

    CDesktopCanvas(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxNO_BORDER | wxHSCROLL | wxVSCROLL);

    ~CDesktopCanvas();

    void Init(); 
    void SetHost(IGuiWidgetHost* host) { m_Host = host; }
    void ObjectSelectionChanged();

    void PaintBackground(wxDC& dc);
    void OnEraseBackground(wxEraseEvent& event);

    void OnPaint(wxPaintEvent& event);
    /// Handler for all mouse events
    void OnMouseEvent(wxMouseEvent& event);
    void DrawItems(wxDC& dc);
    
    virtual bool Layout();
    CIRef<IDesktopItem> FindItem(const wxPoint& pt) const;

    void SetSelectedObjects(const TConstObjects& objs);
    void GetSelectedObjects(TConstObjects& objects) const;
    void SetMainItem(IDesktopItem* main_item, int state = 0);

    void ZoomIn(void);
    void ZoomOut(void);
    
    int GetExpansionLevel() const { return m_ExpansionState; }

    void SetContext(CDesktopViewContext* context);
    CDesktopViewContext* GetContext() const { return m_Context.get(); }

    typedef map<const CObject*, CIRef<IDesktopItem>> TObjectToItem;
    TObjectToItem& GetObjectToItem() { return m_ObjectToItem; }

    IDesktopItem* GetSelection() const { return m_Selected.GetNCPointerOrNull(); }
    CIRef<IDesktopItem> GetRootItem() const { return m_RootItem; }

    bool FindText(const string& search);
private:
    void x_EraseItem(CIRef<IDesktopItem> item, wxDC& dc);
    void x_ClearItems();
    void x_ScrollToSelectedItem();
    void x_ResetSelected();

    void x_OnContextMenu(wxContextMenuEvent& event);
    
    /// The root item to be displayed
    CIRef<IDesktopItem> m_RootItem;
    /// the highlighted item
    CIRef<IDesktopItem> m_Highlighted;
    /// the selected item
    CIRef<IDesktopItem> m_Selected;

    /// List of items to be displayed
    TItemList m_DisplayList;

    int m_ExpansionState;

    IGuiWidgetHost* m_Host;
    TObjectToItem m_ObjectToItem;

    unique_ptr<CDesktopViewContext> m_Context;
};


END_NCBI_SCOPE

#endif  
    // GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_CANVAS__HPP

