/*  $Id: progress_panel.cpp 31681 2014-11-05 14:32:56Z falkrb $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/gl/progress_panel.hpp>

#include <gui/opengl/glhelpers.hpp>

#include <math.h>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CProgressPanel
CProgressPanel::CProgressPanel()
:   m_Font(CGlTextureFont::eFontFace_Helvetica, 12),
    m_Progress(0)
{
}


CProgressPanel::~CProgressPanel()
{
}


void CProgressPanel::Update(float progress, const string& msg)
{
    m_Progress = progress;
    m_Message = msg;
}


const static double kAlpha = 0.9;
const static int kPrOff = 4; // progress bar offset
const static int kPrH = 10; // progress bar height

void CProgressPanel::Render(CGlPane& pane)
{
    CGlAttrGuard guard(GL_LINE_BIT |  GL_ENABLE_BIT  | GL_COLOR_BUFFER_BIT );

    pane.OpenPixels();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    int x1 = m_VPRect.Left();
    int y1 = m_VPRect.Bottom();
    int x2 = m_VPRect.Right();
    int y2 = m_VPRect.Top();

    glColor4d(0.9, 0.9, 0.9, kAlpha);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glRectd(x1, y1, x2, y2);

    glColor4d(0.5, 0.5, 0.5, kAlpha);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glRectd(x1, y1, x2, y2);

    TVPPoint pt = PreferredSize();
    if(m_VPRect.Width() >= pt.X()  &&  m_VPRect.Height() >= pt.Y()) {
        ///draw progress bar
        int pr_x1 = x1 + kPrOff;
        int pr_x2 = x2 - kPrOff;
        int pr_y1 = y1 + kPrOff;
        int pr_y2 = pr_y1 + kPrH - 1;

        // background
        glColor4d(0.8, 0.8, 1.0, kAlpha);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glRectd(pr_x1, pr_y1, pr_x2, pr_y2);

        // completed bar
        glColor4d(0.5, 0.5, 1.0, kAlpha);
        int w = int((x2 - x1 - 2 * kPrOff) * m_Progress);
        glRectd(pr_x1, pr_y1, pr_x1 + w - 1, pr_y2);

        // countour
        glColor4d(0.0, 0.0, 1.0, kAlpha);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glRectd(pr_x1, pr_y1, pr_x2, pr_y2);

        // text message
        glColor4d(0.2, 0.2, 0.2, 1.0);
        m_Font.TextOut(pr_x1, pr_y2 + kPrOff, pr_x2, y2 - kPrOff,
            m_Message.c_str(), IGlFont::eAlign_Left);
    }

    pane.Close();
}


TVPPoint CProgressPanel::PreferredSize()
{
    int w = kPrOff * 2 + 20;
    int h = kPrOff * 3 + kPrH + int(ceil(m_Font.TextHeight()));
    return TVPPoint(w, h);
}


END_NCBI_SCOPE
