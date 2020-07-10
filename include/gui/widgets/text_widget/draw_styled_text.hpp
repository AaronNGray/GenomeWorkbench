#ifndef GUI_WIDGETS_TEXT_WIDGET___DRAW_STYLED_TEXT__HPP
#define GUI_WIDGETS_TEXT_WIDGET___DRAW_STYLED_TEXT__HPP

/*  $Id: draw_styled_text.hpp 38729 2017-06-13 15:50:41Z katargir $
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

#include <wx/colour.h>
#include <wx/dc.h>

BEGIN_NCBI_SCOPE

class CTextStyle;

class CStyledTextOStream
{
public:
    enum StreamHint
    {
        kDisplay,
        kTextPos,
        kCopyText
    };

    virtual ~CStyledTextOStream() {}

    virtual StreamHint GetHint() const = 0;
    virtual wxTextOutputStream* GetTextStream() const { return NULL; }
    virtual bool IsVisible() const { return true; }
    virtual void UpdatePosition(int /*shift*/) {}
    virtual CStyledTextOStream& operator<< (const string& text) = 0;

    virtual int CurrentRowLength() const = 0;

    virtual void SetStyle(const CTextStyle* style) = 0;
    virtual void SetDefaultStyle() = 0;
    virtual void Indent (size_t steps) = 0;
    virtual void NewLine() = 0;
    virtual void NBtoggle() {}

    void WriteMultiLineText(const string& text);

protected:
    virtual void x_OnNBStart() {}
    virtual void x_OnNBEnd() {}
    virtual bool x_InNB() const { return false; }
};

class CTextStyle
{
public:
    CTextStyle(
            unsigned char rFore, unsigned char gFore, unsigned char bFore,
            unsigned char rBack, unsigned char gBack, unsigned char bBack,
            int backgroundMode,
            bool textBold, bool textItalic) :
                m_TextForeground(rFore, gFore, bFore),
                m_TextBackground(rBack, gBack, bBack),
                m_BackgroundMode(backgroundMode),
                m_TextBold(textBold),
                m_TextItalic(textItalic) {}

    friend CStyledTextOStream& operator<<
        (CStyledTextOStream& os, const CTextStyle& style)
            { os.SetStyle(&style); return os; }

    wxColor GetTextForeground() const { return m_TextForeground; }
    wxColor GetTextBackground() const { return m_TextBackground; }
    int     GetBackgroundMode() const { return m_BackgroundMode; }
    bool    IsTextBold()        const { return m_TextBold; }
    bool    IsTextItalic()      const { return m_TextItalic; }

private:
    wxColor m_TextForeground;
    wxColor m_TextBackground;
    int     m_BackgroundMode;
    bool    m_TextBold;
    bool    m_TextItalic;
};

class DefaultStyle
{
public:
    friend CStyledTextOStream& operator<<
        (CStyledTextOStream& os, const DefaultStyle& /*norm*/)
            { os.SetDefaultStyle(); return os; }
};

class NewLine
{
public:
    friend CStyledTextOStream& operator<<
        (CStyledTextOStream& os, const NewLine& /*newLine*/)
            { os.NewLine(); return os; }
};

class Indent
{
public:
    Indent(size_t steps) : m_Steps(steps) {}

    friend CStyledTextOStream& operator<<
        (CStyledTextOStream& os, const Indent& indent)
            { os.Indent(indent.m_Steps); return os; }

private:
    size_t m_Steps;
};

class NB
{
public:
    friend CStyledTextOStream& operator<<
        (CStyledTextOStream& os, const NB& /*newLine*/)
            { os.NBtoggle(); return os; }
};

class NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CBaseTextOStream : public CStyledTextOStream
{
public:
    CBaseTextOStream(wxDC& dc, int x, int y, size_t rowHeight, size_t indentStep);
    ~CBaseTextOStream();

    virtual int CurrentRowLength() const { return m_CurrentLineLength; }

    virtual void SetStyle(const CTextStyle* style);
    virtual void SetDefaultStyle();
    virtual void NewLine();

// NB - Non breakable part functions
    virtual void NBtoggle();

    wxRect GetNBRect() const { return m_NBRect; }

protected:
    void x_UpdateNBRect(wxSize size);
    virtual bool x_InNB() const { return m_NB; }

    void x_UpdatePos(int chars, int width);

    wxDC&  m_DC;
    int    m_StartX;
    int    m_StartY;
    int    m_RowHeight;
    size_t m_IndentStep;
    int    m_x;
    int    m_Row;
    const CTextStyle* m_CurrentStyle;
    int    m_CurrentLineLength;

// save dc state
    int m_BackgroundMode;
    wxColor m_TextForeground;
    wxColor m_TextBackground;
    wxFont  m_SaveFont;
    wxBrush m_SaveBrush;
    wxPen   m_SavePen;

// NB - Non breakable part data
    bool    m_NB;
    wxRect  m_NBRect;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TEXT_WIDGET___DRAW_STYLED_TEXT__HPP
