/*  $Id: font_window.cpp 17824 2008-09-18 21:41:03Z yazhuk $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 */

#include <ncbi_pch.hpp>

#include "font_window.hpp"

BEGIN_NCBI_SCOPE

BEGIN_EVENT_TABLE(CFontWindow, CGLCanvas)
END_EVENT_TABLE()

CFontWindow::CFontWindow(wxWindow* parent, wxWindowID id) :
    CGLCanvas(parent, id)
{
    m_Font.Reset(new CGlBitmapFont(CGlBitmapFont::eHelvetica10));
}

void CFontWindow::x_Render()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    wxSize size = GetClientSize();
    int h = size.GetHeight();
    int w = size.GetWidth();

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, 1, -1);
    glMatrixMode(GL_MODELVIEW);

    const int panel_border = 10;
    const int panel_size   = 60;
    const int panel_size2  = 20;

    ///
    /// grids of various sorts
    ///
    glBegin(GL_LINES);
    {{
        glColor3f(0.7f, 0.7f, 0.7f);

        ///
        /// borders of all elements
        ///

        glVertex2i(panel_border, 0);
        glVertex2i(panel_border, h);

        glVertex2i(w - panel_border, 0);
        glVertex2i(w - panel_border, h);

        glVertex2i(0,   panel_border);
        glVertex2i(w, panel_border);

        glVertex2i(0,   h - panel_border);
        glVertex2i(w, h - panel_border);

        ///
        /// outlined edge for font rotation tests
        ///

        glVertex2i(panel_border + panel_size, 0);
        glVertex2i(panel_border + panel_size, h);

        glVertex2i(w - (panel_border + panel_size), 0);
        glVertex2i(w - (panel_border + panel_size), h);

        glVertex2i(0,   panel_border + panel_size);
        glVertex2i(w, panel_border + panel_size);

        glVertex2i(0,   h - (panel_border + panel_size));
        glVertex2i(w, h - (panel_border + panel_size));

        ///
        /// outlined edges for central tests
        ///

        glVertex2i(panel_border * 2 + panel_size, 0);
        glVertex2i(panel_border * 2 + panel_size, h);

        glVertex2i(w - (panel_border * 2 + panel_size), 0);
        glVertex2i(w - (panel_border * 2 + panel_size), h);

        glVertex2i(0,   h - (panel_border * 2 + panel_size));
        glVertex2i(w, h - (panel_border * 2 + panel_size));

        glVertex2i(0,   panel_border * 2 + panel_size);
        glVertex2i(w, panel_border * 2 + panel_size);

        ///
        /// outlined edges for ArrayTextOut() tests
        ///

        glVertex2i(panel_border * 2 + panel_size + panel_size2, 0);
        glVertex2i(panel_border * 2 + panel_size + panel_size2, h);

        glVertex2i(w - (panel_border * 2 + panel_size + panel_size2), 0);
        glVertex2i(w - (panel_border * 2 + panel_size + panel_size2), h);

        glVertex2i(0,   h - (panel_border * 2 + panel_size + panel_size2));
        glVertex2i(w, h - (panel_border * 2 + panel_size + panel_size2));

        glVertex2i(0,   panel_border * 2 + panel_size + panel_size2);
        glVertex2i(w, panel_border * 2 + panel_size  + panel_size2);
    }}
    glEnd();

    ///
    /// test 1: rotated fonts
    ///

    {{
        const char* alphabet = "abcdefghijklmnopqrstuvwxyz"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        /// non-rotated font
        glColor3f(0.9f, 0.7f, 0.7f);
        glRecti(panel_border + panel_size,
                h - panel_border - panel_size,
                w - panel_border - panel_size,
                h - panel_border);
        glColor3f(0.0f, 0.0f, 0.0f);

        m_Font->SetFontRotate(CGlBitmapFont::eFontRotate_0);
        TModelUnit x1 = panel_border + panel_size;
        TModelUnit y1 = h - panel_border - panel_size;
        TModelUnit x2 = w - panel_border - panel_size;
        TModelUnit y2 = y1 + panel_size;
        m_Font->TextOut(x1, y1, x2, y2,
                        alphabet, IGlFont::eAlign_Left | IGlFont::eAlign_Top);

        m_Font->TextOut(x1, y1, x2, y2,
                        alphabet, IGlFont::eAlign_Center);

        m_Font->TextOut(x1, y1, x2, y2,
                        alphabet, IGlFont::eAlign_Right | IGlFont::eAlign_Bottom);

        /// 270 degrees - right edge
        glColor3f(0.7f, 0.9f, 0.7f);
        glRecti(w - panel_border - panel_size,
                h - panel_border - panel_size,
                w - panel_border,
                panel_border + panel_size);
        glColor3f(0.0f, 0.0f, 0.0f);

        m_Font->SetFontRotate(CGlBitmapFont::eFontRotate_270);
        x1 = w - panel_border - panel_size;
        y1 = panel_border + panel_size;
        x2 = x1 + panel_size;
        y2 = y1 + h - panel_border * 2 - panel_size * 2;
        m_Font->TextOut(x1, y1, x2, y2,
                        alphabet, IGlFont::eAlign_Left | IGlFont::eAlign_Top);
        m_Font->TextOut(x1, y1, x2, y2,
                        alphabet, IGlFont::eAlign_Center);
        m_Font->TextOut(x1, y1, x2, y2,
                        alphabet, IGlFont::eAlign_Right | IGlFont::eAlign_Bottom);

        //glColor3d(0, 0, 0);
        //glRectd(x1, y1, x2, y2);

        /// 180 degrees - bottom
        glColor3f(0.7f, 0.7f, 0.9f);
        glRecti(panel_border + panel_size,
                panel_border,
                w - panel_border - panel_size,
                panel_border + panel_size);
        glColor3f(0.0f, 0.0f, 0.0f);

        m_Font->SetFontRotate(CGlBitmapFont::eFontRotate_180);
        x1 = panel_border + panel_size;
        y1 = panel_border;
        x2 = x1 + w - panel_border * 2 - panel_size * 2;
        y2 = y1 + panel_size;

        m_Font->TextOut(x1, y1, x2, y2,
                        alphabet, IGlFont::eAlign_Left | IGlFont::eAlign_Top);

        m_Font->TextOut(x1, y1, x2, y2,
                        alphabet, IGlFont::eAlign_Center);

        m_Font->TextOut(x1, y1, x2, y2,
                        alphabet, IGlFont::eAlign_Right | IGlFont::eAlign_Bottom);

        /// 90 degrees - right edge
        glColor3f(0.9f, 0.7f, 0.9f);
        glRecti(panel_border,
                panel_border + panel_size,
                panel_border + panel_size,
                h - panel_border - panel_size);
        glColor3f(0.0f, 0.0f, 0.0f);

        m_Font->SetFontRotate(CGlBitmapFont::eFontRotate_90);
        x1 = panel_border;
        y1 = panel_border + panel_size;
        x2 = x1 + panel_size,
        y2 = y1 + h - panel_border * 2 - panel_size * 2;
        m_Font->TextOut(x1, y1, x2, y2,
                        alphabet, IGlFont::eAlign_Left | IGlFont::eAlign_Top);

        m_Font->TextOut(x1, y1, x2, y2,
                        alphabet, IGlFont::eAlign_Center);

        m_Font->TextOut(x1, y1, x2, y2,
                        alphabet, IGlFont::eAlign_Right | IGlFont::eAlign_Bottom);

        m_Font->SetFontRotate(CGlBitmapFont::eFontRotate_0);
    }}

    ///
    /// test 2:  ArrayTextOut()
    ///

    {{
        const int baseline_offs = 2;
        const char* alphabet = "aBcDeFgHiJkLmNoPqRsTuVwXyZ";

        TModelUnit dx, dy;

        /// non-rotated font
        glColor3f(0.9f, 0.7f, 0.7f);
        glRecti(panel_border * 2 + panel_size + panel_size2,
                h - panel_border * 2 - panel_size - panel_size2,
                w - panel_border * 2 - panel_size - panel_size2,
                h - panel_border * 2 - panel_size);

        dx = (w - panel_border * 4 - panel_size * 2 - panel_size2 * 2) / TModelUnit(strlen(alphabet));
        dy = 0.0f;

        glColor3f(0.7f, 0.7f, 0.7f);
        glBegin(GL_LINES);
        for( TModelUnit xx = panel_border * 2 + panel_size + panel_size2;  xx < w - panel_border * 2 - panel_size - panel_size2;  xx += dx) {
            glVertex2d(xx, h - panel_border * 2 - panel_size - panel_size2);
            glVertex2d(xx, h - panel_border * 2 - panel_size);
        }
        glEnd();

        glColor3f(0.0f, 0.0f, 0.0f);
        m_Font->SetFontRotate(CGlBitmapFont::eFontRotate_0);
        m_Font->ArrayTextOut(panel_border * 2 + panel_size + panel_size2 + dx/2,
                             h - panel_border * 2 - panel_size - panel_size2 + baseline_offs,
                             dx, dy,
                             alphabet);

        /// 270-degree rotated font
        glColor3f(0.7f, 0.9f, 0.7f);
        glRecti(w - panel_border * 2 - panel_size - panel_size2,
                h - panel_border * 2 - panel_size - panel_size2,
                w - panel_border * 2 - panel_size,
                panel_border * 2 + panel_size + panel_size2);

        dx = 0.0f;
        dy = (h - panel_border * 4 - panel_size * 2 - panel_size2 * 2) / TModelUnit(strlen(alphabet));
        glColor3f(0.7f, 0.7f, 0.7f);
        glBegin(GL_LINES);
        for( TModelUnit yy = h - panel_border * 2 - panel_size - panel_size2;  yy > panel_border * 2 + panel_size + panel_size2;  yy -= dy) {
            glVertex2d(w - panel_border * 2 - panel_size - panel_size2, yy);
            glVertex2d(w - panel_border * 2 - panel_size,               yy);
        }
        glEnd();

        glColor3f(0.0f, 0.0f, 0.0f);
        m_Font->SetFontRotate(CGlBitmapFont::eFontRotate_270);
        m_Font->ArrayTextOut(w - panel_border * 2 - panel_size - panel_size2 + baseline_offs,
                             h - panel_border * 2 - panel_size - panel_size2 - dy/2,
                             dx, -dy,
                             alphabet);

        /// 180-degree rotated font
        glColor3f(0.7f, 0.7f, 0.9f);
        glRecti(panel_border * 2 + panel_size + panel_size2,
                panel_border * 2 + panel_size,
                w - panel_border * 2 - panel_size - panel_size2,
                panel_border * 2 + panel_size + panel_size2);

        dx = (w - panel_border * 4 - panel_size * 2 - panel_size2 * 2) / TModelUnit(strlen(alphabet));
        dy = 0.0f;
        glColor3f(0.7f, 0.7f, 0.7f);
        glBegin(GL_LINES);
        for ( TModelUnit xx = panel_border * 2 + panel_size + panel_size2;  xx < w - panel_border * 2 - panel_size - panel_size2;  xx += dx) {
            glVertex2d(xx, panel_border * 2 + panel_size);
            glVertex2d(xx, panel_border * 2 + panel_size + panel_size2);
        }
        glEnd();

        glColor3f(0.0f, 0.0f, 0.0f);
        m_Font->SetFontRotate(CGlBitmapFont::eFontRotate_180);
        m_Font->ArrayTextOut(w - panel_border * 2 - panel_size - panel_size2 - dx/2,
                             panel_border * 2 + panel_size + panel_size2 - baseline_offs,
                             -dx, dy,
                             alphabet);

        /// 90-degree rotated font
        glColor3f(0.9f, 0.7f, 0.9f);
        glRecti(panel_border * 2 + panel_size,
                panel_border * 2 + panel_size + panel_size2,
                panel_border * 2 + panel_size + panel_size2,
                h - panel_border * 2 - panel_size - panel_size2);

        dx = 0.0f;
        dy = (h - panel_border * 4 - panel_size * 2 - panel_size2 * 2) / TModelUnit(strlen(alphabet));
        glColor3f(0.7f, 0.7f, 0.7f);
        glBegin(GL_LINES);
        for ( TModelUnit yy = h - panel_border * 2 - panel_size - panel_size2;  yy > panel_border * 2 + panel_size + panel_size2;  yy -= dy) {
            glVertex2d(panel_border * 2 + panel_size + panel_size2, yy);
            glVertex2d(panel_border * 2 + panel_size,               yy);
        }
        glEnd();

        glColor3f(0.0f, 0.0f, 0.0f);
        m_Font->SetFontRotate(CGlBitmapFont::eFontRotate_90);
        m_Font->ArrayTextOut(panel_border * 2 + panel_size + panel_size2 - baseline_offs,
                             panel_border * 2 + panel_size + panel_size2 + dy/2,
                             dx, dy,
                             alphabet);


        m_Font->SetFontRotate(CGlBitmapFont::eFontRotate_0);
    }}
}

void CFontWindow::SetGlFont(CGlBitmapFont::EFont font)
{
    m_Font.Reset(new CGlBitmapFont(font));
    Refresh(false);
}

END_NCBI_SCOPE
