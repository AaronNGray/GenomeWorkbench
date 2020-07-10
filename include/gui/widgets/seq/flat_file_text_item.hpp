#ifndef GUI_WIDGETS_SEQ___FLAT_FILE_TEXT_ITEM__HPP
#define GUI_WIDGETS_SEQ___FLAT_FILE_TEXT_ITEM__HPP

/*  $Id: flat_file_text_item.hpp 39822 2017-11-08 16:43:19Z katargir $
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

#include <gui/widgets/text_widget/text_select_item.hpp>

#include <objtools/format/items/item_base.hpp>

BEGIN_NCBI_SCOPE

class CStyledTextOStream;
class CFlatFileViewContext;
class CEditToolbar;

class CFlatFileTextItem : public CTextSelectItem
{
public:
    friend class CFlatFileViewContext;

    CFlatFileTextItem(CConstRef<objects::IFlatItem> item);
    ~CFlatFileTextItem();

    void SetEditFlags(int flags) { m_EditFlags = flags; }
    int  GetEditFlags() const { return m_EditFlags; }

    virtual void    CalcSize(wxDC& dc, CTextPanelContext* context);
    virtual void    Draw(wxDC& dc, const wxRect& updateRect, CTextPanelContext* context);

    virtual void MouseEvent(wxMouseEvent& event, CTextPanelContext& context);

    virtual string  GetToolTipText(CTextPanelContext* context) const;

    virtual const   CConstRef<CObject> GetAssosiatedObject() const;

    virtual bool    IsSelectable() const { return (m_EditFlags != 0); }

    const objects::IFlatItem* GetFlatItem() const { return m_Item.GetPointerOrNull(); }

    void InitToolbar();

protected:
    virtual void    x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* context) const;
    virtual void    x_CalcLineCount(CTextPanelContext* context);

    CConstRef<objects::IFlatItem> m_Item;
    CConstRef<CObject> m_AssosiatedObject;

    int     m_EditFlags;

    CEditToolbar* m_EditToolbar;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___FLAT_FILE_TEXT_ITEM__HPP
