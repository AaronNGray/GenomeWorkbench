#ifndef GUI_WIDGETS_TEXT_WIDGET___SELECTION_HELPER__HPP
#define GUI_WIDGETS_TEXT_WIDGET___SELECTION_HELPER__HPP

/*  $Id: selection_helper.hpp 29559 2014-01-08 22:09:18Z katargir $
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

BEGIN_NCBI_SCOPE

class CTextPanelContext;

class NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CSelectionHelper : public wxEvtHandler
{
    DECLARE_EVENT_TABLE()
public:
    CSelectionHelper(int offset) : m_Item(0), m_Offset(offset) {}

    void SetItem(ITextItem* item) { m_Item = item; }

    void Draw(wxDC& dc, const wxRect& updateRect, CTextPanelContext* context);

    static CSelectionHelper& GetInstance();

protected:
    void OnMouseMove(wxMouseEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftDClick(wxMouseEvent& event);

    void x_DrawSelectionArea(wxDC& dc, const wxRect& updateRect, CTextPanelContext* context);
    wxRect x_GetSelectionRect(CTextPanelContext* context) const;

    ITextItem* m_Item;
    int m_Offset;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TEXT_WIDGET___SELECTION_HELPER__HPP
