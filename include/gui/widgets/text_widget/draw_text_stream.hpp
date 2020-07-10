#ifndef GUI_WIDGETS_TEXT_WIDGET___DRAW_TEXT_STREAM__HPP
#define GUI_WIDGETS_TEXT_WIDGET___DRAW_TEXT_STREAM__HPP

/*  $Id: draw_text_stream.hpp 30508 2014-06-04 19:58:21Z katargir $
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

#include <gui/widgets/text_widget/draw_styled_text.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CDrawTextOStream : public CBaseTextOStream
{
public:
    class IStyleModifier
    {
    public:
        virtual ~IStyleModifier() {}
        virtual void SetStyle(wxDC& dc) = 0;
    };

    void AddMarker(int startRow, int startCol, int endRow, int endCol, IStyleModifier* style);

    typedef CBaseTextOStream TParent;

    CDrawTextOStream(wxDC& dc, int x, int y, size_t rowHeight, size_t indentStep);

    virtual StreamHint GetHint() const { return kDisplay; }

    virtual CStyledTextOStream& operator<< (const string& text);
    virtual void Indent (size_t steps);

protected:
    virtual void x_OnNBEnd();

    class CStylePos
    {
    public:
        CStylePos(size_t textPos, IStyleModifier* style)
            : m_TextPos(textPos), m_Style(style) {}

        size_t GetTextPos() const { return m_TextPos; }
        IStyleModifier* GetStyle() const { return m_Style; }

    private:
        size_t m_TextPos;
        IStyleModifier* m_Style;
    };

    list<CStylePos> x_GetStyles(int textLength);

private:
    struct SMarker
    {
        SMarker(int startRow, int startCol, int endRow, int endCol, IStyleModifier* style) :
                m_StartRow(startRow), m_StartCol(startCol), m_EndRow(endRow), m_EndCol(endCol), m_Style(style) {}

        int             m_StartRow;
        int             m_StartCol;
        int             m_EndRow;
        int             m_EndCol;
        IStyleModifier* m_Style;
    };

    vector<SMarker> m_Markers;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TEXT_WIDGET___DRAW_TEXT_STREAM__HPP
