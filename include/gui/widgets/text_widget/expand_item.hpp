#ifndef GUI_WIDGETS_TEXT_WIDGET___EXPAND_ITEM__HPP
#define GUI_WIDGETS_TEXT_WIDGET___EXPAND_ITEM__HPP

/*  $Id: expand_item.hpp 39653 2017-10-24 18:50:24Z katargir $
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

#include <wx/event.h>

#include <gui/widgets/text_widget/text_item.hpp>
#include <gui/widgets/text_widget/expand_glyph.hpp>

BEGIN_NCBI_SCOPE

class CTextPanelContext;

class NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CExpandItem : public ITextItem, public IExpandable, public IContainer
{
public:
    CExpandItem(CTextItem* collapsedItem, CTextItem* expandedItem,
                bool expand = false);
    virtual ~CExpandItem();

    void SetSelectable(bool selectable = true)
        { m_Selectable = selectable; }

    void AddGlyph(CGlyph* glyph);
    
    CTextItem* GetCollapsedItem() { return m_Items[0]; }
    CTextItem* GetExpandedItem()  { return m_Items[1]; }

    // IExpandable implementation
    virtual bool    IsExpanded() const { return (m_Expanded != 0); }
    virtual void    Toggle(CTextPanelContext* context, bool update);
    virtual string  GetToolTipText(CTextPanelContext* context) const;

    // ITextItem overrides
    virtual void SetContainer(IContainer* container, int index)
        { m_Container = container; m_Index = index; }
    virtual void GetContainer(IContainer*& container, int& index)
        { container = m_Container; index = m_Index; }
    virtual wxPoint GetPosition() const
        { return m_Container ? m_Container->GetItemPosition(m_Index) : wxPoint(0, 0); }
    virtual wxSize  GetSize() const { return m_Items[m_Expanded]->GetSize(); }
    virtual int     GetStartLine() const
        { return m_Container ? m_Container->GetItemStartLine(m_Index) : 0; }

    // Mouse handling
    virtual void MouseEvent(wxMouseEvent& event, CTextPanelContext& context);

    virtual size_t  GetLineCount() const { return m_Items[1]->GetLineCount(); }
    virtual ITextItem* GetItemByLine(int lineNum, bool expanded)
    { return m_Items[expanded ? 1 : m_Expanded]->GetItemByLine(lineNum, expanded); }

    virtual int     GetTextLeftMargin(CTextPanelContext* context) const
                        { return m_Items[0]->GetTextLeftMargin(context); }

    virtual void    CalcSize(wxDC& dc, CTextPanelContext* context);
    virtual void    Draw(wxDC& dc, const wxRect& updateRect, CTextPanelContext* context);

    virtual void    GetText(wxTextOutputStream& os,
                            const CTextBlock& block,
                            CTextPanelContext* context);

    virtual const   CConstRef<CObject> GetAssosiatedObject() const;

    virtual bool    IsSelectable() const { return m_Selectable; }

    virtual bool    Traverse(ICompositeTraverser& traverser);

    // IContainer implementation
    virtual wxPoint GetItemPosition(int /*index*/) const { return GetPosition(); }
    virtual int     GetItemStartLine(int /*index*/) const { return GetStartLine(); }
    virtual void    ItemSizeChanged(int index) { if (m_Container) m_Container->ItemSizeChanged(m_Index); }
	virtual void    UpdatePositions();

protected:
    void x_LimitCollapsedSelection(CTextPanelContext* context) const;

    int             m_Expanded;
    CTextItem*      m_Items[2];
    CGlyph*         m_Glyph;
    CGlyph*         m_Glyph2;
    bool            m_Selectable;

    IContainer* m_Container;
    int         m_Index;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TEXT_WIDGET___EXPAND_ITEM__HPP

