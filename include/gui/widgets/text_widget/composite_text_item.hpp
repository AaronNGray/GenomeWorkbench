#ifndef GUI_WIDGETS_TEXT_WIDGET___COMPOSITE_TEXT_ITEM__HPP
#define GUI_WIDGETS_TEXT_WIDGET___COMPOSITE_TEXT_ITEM__HPP

/*  $Id: composite_text_item.hpp 35549 2016-05-20 15:37:51Z katargir $
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

#include <gui/widgets/text_widget/text_item.hpp>

BEGIN_NCBI_SCOPE

class ICompositeTraverser;

class NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CCompositeTextItem : public CTextItem, public IContainer
{
public:
    typedef CTextItem TParent;

    CCompositeTextItem(bool deleteChildren = true) : m_DeleteChildren(deleteChildren) {}
    ~CCompositeTextItem();

    void SetDeleteChildren(bool val) { m_DeleteChildren = val; }

    virtual ITextItem* GetItemByLine(int lineNum, bool expanded);

    // Mouse handling
    virtual void MouseEvent(wxMouseEvent& event, CTextPanelContext& context);

    virtual void    CalcSize(wxDC& dc, CTextPanelContext* context);
    virtual void    Draw(wxDC& dc, const wxRect& updateRect, CTextPanelContext* context);

    void AddItem (ITextItem* item, bool updatePositions = true);
    ITextItem* GetItem(size_t index);
    size_t GetItemCount() const { return m_Items.size(); }

    void UpdateSelection(ITextItem* start, ITextItem* end, bool selected = false);

    virtual void    GetText(wxTextOutputStream& os,
                            const CTextBlock& block,
                            CTextPanelContext* context);

    virtual bool    Traverse(ICompositeTraverser& traverser);

    // IContainer implementation
    virtual wxPoint GetItemPosition(int index) const;
    virtual int     GetItemStartLine(int index) const;
    virtual void    ItemSizeChanged(int index);
    virtual void    UpdatePositions();

    void InsertItems (size_t index, ITextItem* const * items, size_t count, bool updatePositions = true);
    void RemoveItem (size_t index);

protected:
    vector<ITextItem*> m_Items;
    vector<int>        m_AccumLines;  // total lines up to this item
    vector<int>        m_AccumHeight; // total height up to this item

	bool m_DeleteChildren;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TEXT_WIDGET___COMPOSITE_TEXT_ITEM__HPP
