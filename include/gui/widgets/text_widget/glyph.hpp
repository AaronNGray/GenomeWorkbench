#ifndef GUI_WIDGETS_TEXT_WIDGET___GLYPH__HPP
#define GUI_WIDGETS_TEXT_WIDGET___GLYPH__HPP

/*  $Id: glyph.hpp 29712 2014-01-31 20:37:15Z katargir $
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

BEGIN_NCBI_SCOPE

class CTextPanelContext;

class NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CGlyph
{
public:
    CGlyph() : m_Pos(0,0), m_Size(4,4) {}
    virtual ~CGlyph() {}

    virtual wxPoint GetPosition() const { return m_Pos; }

    virtual wxSize  GetSize() const { return m_Size; }
    virtual wxRect  GetRect() const { return wxRect(m_Pos, m_Size); }

    // Mouse handling
    virtual void MouseEvent(wxMouseEvent&) {}

    void            SetPosition(wxPoint pos) { m_Pos = pos; }
    void            SetSize(wxSize size) { m_Size = size; }
    void            SetRect(wxRect rect)
                        { m_Pos = rect.GetPosition(); m_Size = rect.GetSize(); }

    virtual void    Draw(wxDC& dc, CTextPanelContext* context) = 0;
    virtual void    CalcSize(wxDC& dc, CTextPanelContext* context) = 0;

protected:
    wxPoint m_Pos;
    wxSize m_Size;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TEXT_WIDGET___GLYPH__HPP
