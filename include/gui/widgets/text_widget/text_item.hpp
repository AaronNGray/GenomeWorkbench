#ifndef GUI_WIDGETS_TEXT_WIDGET___TEXT_ITEM__HPP
#define GUI_WIDGETS_TEXT_WIDGET___TEXT_ITEM__HPP

/*  $Id: text_item.hpp 29771 2014-02-07 20:25:40Z katargir $
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

#include <gui/widgets/text_widget/text_position.hpp>
#include <gui/widgets/text_widget/composite_traverser.hpp>

BEGIN_NCBI_SCOPE

class CTextPanelContext;
class CTextBlock;

BEGIN_SCOPE(objects)
    class CScope;
END_SCOPE(objects)

class IContainer
{
public:
    virtual ~IContainer() {}
    virtual wxPoint GetItemPosition(int index) const = 0;
    virtual int     GetItemStartLine(int index) const = 0;
    virtual void    ItemSizeChanged(int index) = 0;
	virtual void    UpdatePositions() = 0;
};

#ifdef _DEBUG
extern NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CAtomicCounter_WithAutoInit gITextItemCount;
#endif

class ITextItem
{
public:

#ifdef _DEBUG
    ITextItem() { gITextItemCount.Add(1); }
    virtual ~ITextItem() { gITextItemCount.Add(-1); }
#else
    virtual ~ITextItem() { }
#endif

    virtual wxPoint GetPosition() const = 0;

    virtual void SetContainer(IContainer* container, int index) = 0;
    virtual void GetContainer(IContainer*& container, int& index) = 0;

    virtual wxSize  GetSize() const = 0;
    virtual wxRect  GetRect() const { return wxRect(GetPosition(), GetSize()); }

    // Mouse handling
    virtual void MouseEvent(wxMouseEvent&, CTextPanelContext&) {}

    // line number in fully expanded text view
    virtual int  GetStartLine() const = 0;

    // line count in fully expanded text view
    virtual size_t  GetLineCount() const = 0;
    // Composite item should return item it contains
    virtual ITextItem* GetItemByLine(int lineNum, bool expanded) = 0;

    virtual int     GetTextLeftMargin(CTextPanelContext* context) const = 0;

    virtual void    CalcSize(wxDC& dc, CTextPanelContext* context) = 0;
    virtual void    Draw(wxDC& dc, const wxRect& updateRect, CTextPanelContext* context) = 0;

    virtual string  GetToolTipText(CTextPanelContext*) const { return ""; }

    virtual void    FillContextMenu(wxMenu& WXUNUSED(menu)) const {}

    virtual void    GetText(wxTextOutputStream& os,
                            const CTextBlock& block,
                            CTextPanelContext* context) = 0;

    virtual const   CConstRef<CObject> GetAssosiatedObject() const { return null; }
    virtual objects::CScope*           GetAssosiatedScope() const { return 0; }

    virtual bool    IsSelectable() const { return (GetAssosiatedObject() != 0); }

    virtual bool    Traverse(ICompositeTraverser& /*traverser*/) = 0;


    CTextBlock GetItemBlock() const
        { return CTextBlock((int)GetStartLine(), (int)GetLineCount()); }

    virtual ITextItem* Clone() { return NULL; }

protected:
};

class CStyledTextOStream;

class NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CTextItem : public ITextItem
{
public:
    CTextItem() : m_LineCount(1), m_Size(wxDefaultSize), m_Container(), m_Index(0) {}

    virtual void SetContainer(IContainer* container, int index)
        { m_Container = container; m_Index = index; }
    virtual void GetContainer(IContainer*& container, int& index)
        { container = m_Container; index = m_Index; }

    virtual wxPoint GetPosition() const
        { return m_Container ? m_Container->GetItemPosition(m_Index) : wxPoint(0, 0); }

    virtual wxSize  GetSize() const { return m_Size; }

    // Mouse handling
    virtual void MouseEvent(wxMouseEvent& event, CTextPanelContext& context);

    virtual int     GetStartLine() const
        { return m_Container ? m_Container->GetItemStartLine(m_Index) : 0; }

    virtual size_t  GetLineCount() const { return m_LineCount; }
    virtual ITextItem* GetItemByLine(int lineNum, bool expanded);
    virtual void SetLineCount(size_t lineCount) { m_LineCount = lineCount; }

    virtual int     GetTextLeftMargin(CTextPanelContext* context) const;
    virtual void    CalcSize(wxDC& dc, CTextPanelContext* context);

    virtual void    Draw(wxDC& dc, const wxRect& updateRect, CTextPanelContext* context);

    virtual int     GetIndent() const { return 0; }

    virtual bool    Traverse(ICompositeTraverser& traverser) { return traverser.ProcessItem(*this); }

    virtual void TextPosToExpanded(CTextPosition& WXUNUSED(pos), CTextPanelContext* WXUNUSED(context)) const {}
    virtual void TextPosToCollapsed(CTextPosition& WXUNUSED(pos), CTextPanelContext* WXUNUSED(context)) const {}

    void RenderText(CStyledTextOStream& ostream, CTextPanelContext* context) const
        {  x_RenderText(ostream, context); }

protected:
    virtual void    x_Indent(CStyledTextOStream& ostream) const;
    virtual void    x_RenderText(CStyledTextOStream& WXUNUSED(ostream), CTextPanelContext* WXUNUSED(context)) const {}

    size_t  m_LineCount;
    wxSize  m_Size;

    IContainer* m_Container;
    int         m_Index;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TEXT_WIDGET___TEXT_ITEM__HPP
