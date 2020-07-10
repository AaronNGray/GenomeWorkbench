#ifndef GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_ITEM__HPP
#define GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_ITEM__HPP

/*  $Id: desktop_item.hpp 37865 2017-02-23 21:52:38Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>
#include <gui/widgets/seq_desktop/desktop_traverser.hpp>
#include <gui/widgets/seq_desktop/desktop_typed_items.hpp>
#include <wx/dc.h>
#include <wx/string.h>


BEGIN_NCBI_SCOPE

class IDesktopItem : public CObject
{
public:
    virtual ~IDesktopItem() {}

    virtual void Add(IDesktopItem&) = 0;

    virtual void Expand() = 0;
    virtual void Collapse() = 0;

    virtual wxPoint GetPosition() const = 0;
    virtual wxSize  GetSize() const = 0;
    virtual wxRect  GetRect() const { return wxRect(GetPosition(), GetSize()); }
    virtual int GetRelevantHeight() const = 0;
    virtual const wxBrush& GetBkgdBrush() const = 0;
    virtual const wxColour& GetFrameColor() const = 0;

    virtual void SetPosition(const wxPoint& pos) = 0;
    virtual void SetPosY(int y_val) = 0;
    virtual void SetPosX(int x_val) = 0; 
    virtual void SetBkgdBrush(const wxBrush& bkgd_brush) = 0;
    virtual void SetFrameColor(const wxColour& color) = 0;
    
    virtual IDesktopItem* FindItem(const wxPoint& pt) const = 0;
    virtual bool FindSearchItem(const string& search, bool case_sensitive) const = 0;

    virtual bool IsShown() const = 0;
    virtual void Show(bool value) = 0;
    virtual void ShowTillBioseq(bool value) = 0;
    virtual void Draw(wxDC& dc, bool highlight = false) const = 0;
    virtual void SetSelected(bool val) = 0;

    virtual void UpdateSize() {}
    virtual void UpdatePositions(int* cumY, int depth = 0) {}
    virtual bool Traverse(IDesktopCompositeTraverser& /*traverser*/) = 0;

    virtual const string& GetType() const = 0;
    virtual const CConstRef<CObject> GetAssociatedObject() const { return CConstRef<CObject>(); }

    virtual const string& GetDescr() const = 0;
    virtual CConstRef<IDesktopDataItem> GetDataItem() const = 0;
};


class CDesktopItem : public IDesktopItem
{
public:
    virtual ~CDesktopItem() {}

    virtual void Add(IDesktopItem&) {}

    virtual void Expand() {}
    virtual void Collapse() {}

    virtual wxPoint GetPosition() const { return m_Pos; }
    virtual wxSize  GetSize() const { return m_Size; }
    virtual int GetRelevantHeight() const { return 0; }
    virtual const wxBrush& GetBkgdBrush() const { return m_BkgdBrush; }
    virtual const wxColour& GetFrameColor() const { return m_FrameCol; }

    virtual void SetPosition(const wxPoint& pos) { m_Pos = pos; }
    virtual void SetPosY(int y_val) { m_Pos.y = y_val; }
    virtual void SetPosX(int x_val) { m_Pos.x = x_val; }
    virtual void SetBkgdBrush(const wxBrush& bkgd_brush) { m_BkgdBrush = bkgd_brush; }
    virtual void SetFrameColor(const wxColour& color) { m_FrameCol = color; }
    
    virtual IDesktopItem* FindItem(const wxPoint& pt) const { return nullptr; }
    virtual bool FindSearchItem(const string& search, bool case_sensitive) const;

    virtual bool IsShown() const { return m_Show; }
    virtual void Show(bool value) { m_Show = value; }
    virtual void ShowTillBioseq(bool value) {}
    virtual void Draw(wxDC& dc, bool highlight = false) const;
    virtual void SetSelected(bool val) { m_Selected = val; }

    virtual bool Traverse(IDesktopCompositeTraverser& traverser) { return traverser.ProcessItem(*this); }

    virtual void Init();

    virtual const string& GetType() const;
    virtual const CConstRef<CObject> GetAssociatedObject() const;

    virtual const string& GetDescr() const { return m_Text[0]; }

    virtual CConstRef<IDesktopDataItem> GetDataItem() const { return m_Item; }

protected:
    CDesktopItem(CConstRef<IDesktopDataItem> item);
    /// Sets the size of the rectangle based on the height and width of the displayed text
    void x_SetTextToDrawAndSize();

    /// Position of the item on the canvas
    wxPoint m_Pos;
    /// Size of the rectangle
    wxSize m_Size;
    /// Colour of rectangle's outline
    wxColour m_FrameCol;

    /// Foreground colour of the text
    wxColour m_TextCol;
    /// Text that appears in the rectangle
    vector<string> m_Text;
    /// Background colour of the rectangle
    wxBrush m_BkgdBrush;

    /// Height of the text within the rectangle
    int m_TextHeight;
    /// Text that appears on the screen corresponding to the current level of expansion
    wxString m_CurrentText;
    /// Point where the drawing starts
    wxPoint m_Origin;

    /// Storing actual data
    CConstRef<IDesktopDataItem> m_Item;
    /// Flag to show or hide the item
    bool m_Show;
    /// State(level) of expansion
    int m_ExpandLevel;
    /// Text indentation 
    int m_Indent;
    /// Flag to indicate selection of this item
    bool m_Selected;

private:
    /// Determines the size of the text to be displayed 
    /// corresponding to the current expansion level
    virtual wxSize x_GetVisibleTextSize(wxDC& dc) = 0;
};

/// A desktop item that has no children. 
/// Used for example for descriptors, features, alignments, contact-info, etc.
/// @sa CDesktopDataBuilder
class NCBI_GUIWIDGETS_DESKTOP_EXPORT CSimpleDesktopItem : public CDesktopItem
{
public:
    CSimpleDesktopItem(CConstRef<IDesktopDataItem> item);

    virtual int GetRelevantHeight() const { return m_Size.GetHeight(); }
    virtual void Expand();
    virtual void Collapse();

    virtual void UpdateSize();
    virtual IDesktopItem* FindItem(const wxPoint& pt) const;
    virtual void UpdatePositions(int* cumY, int depth = 0);
private:
    virtual wxSize x_GetVisibleTextSize(wxDC& dc);
    
};

/// A desktop item with children.
/// Used for example for bioseqs, sets, seq-annots, etc.
/// @sa CDesktopDataBuilder
class NCBI_GUIWIDGETS_DESKTOP_EXPORT CCompositeDesktopItem : public CDesktopItem
{
public:
    CCompositeDesktopItem(CConstRef<IDesktopDataItem> item);

    virtual int GetRelevantHeight() const;

    virtual void Add(IDesktopItem&);

    virtual void Expand();
    virtual void Collapse();

    virtual void UpdateSize();
    virtual IDesktopItem* FindItem(const wxPoint& pt) const;

    virtual void ShowTillBioseq(bool value);
    virtual bool Traverse(IDesktopCompositeTraverser& traverser);
    virtual void UpdatePositions(int* cumY, int depth = 0);
private:
    virtual wxSize x_GetVisibleTextSize(wxDC& dc);
   
protected:
    typedef list<CIRef<IDesktopItem>> TDeskItemList;
    /// List of child items
    TDeskItemList m_ItemList;
};

/// A special composite desktop item (one that has children).
/// Currently there can only be used for seq-submits, bioseqs and sets.
/// @sa CDesktopDataBuilder
class NCBI_GUIWIDGETS_DESKTOP_EXPORT CRootDesktopItem : public CCompositeDesktopItem
{
public:
    CRootDesktopItem(CConstRef<IDesktopDataItem> item);

    virtual void Expand();
    virtual void Collapse();

    virtual void UpdateSize();
    virtual void Init();

    virtual void UpdatePositions(int* cumY, int depth = 0);
    virtual bool Traverse(IDesktopCompositeTraverser& traverser);
private:
    virtual wxSize x_GetVisibleTextSize(wxDC& dc);
    /// Original size corresponding to 0 expansion level
    wxSize m_OrigSize;
};

END_NCBI_SCOPE

#endif  
    // GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_ITEM__HPP


