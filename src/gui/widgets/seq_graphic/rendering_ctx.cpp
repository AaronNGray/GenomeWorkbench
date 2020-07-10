/*  $Id: rendering_ctx.cpp 43669 2019-08-13 21:49:54Z katargir $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */


#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <serial/iterator.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/opengl/glresmgr.hpp>
#include <gui/opengl/glframebuffer.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>

#include <math.h>


BEGIN_NCBI_SCOPE

static const float kMinBarHeight = 2.f;

/// extra space for side labeling, in screen pixels
const int CRenderingContext::kLabelSpacePx = 5;

CRenderingContext::CRenderingContext()
    : m_Pane(NULL)
    , m_HighlightsColor(0.4f, 0.4f, 0.4f, 1.0f)
    , m_MinLabelSizePos(1.0)
    , m_Offset(0.0)
    , m_Scale(1.0)
    , m_ScaleInv(1.0)
    , m_ViewWidth(0)
    , m_bHorz(true)
    , m_bFlipped(false)
    , m_Font_Helv10(CGlBitmapFont::eHelvetica10)
{}

CRenderingContext::~CRenderingContext()
{
    m_Pane = NULL;
}

void CRenderingContext::x_CreateGlyphs()
{
    if (!m_GlyphGeom.IsNull())
        return;

    m_GlyphGeom.Reset(CGlResMgr::Instance().CreateVboGeom(GL_TRIANGLE_STRIP, ""));

    vector<CVect2<float> > vb;

    float step = 3.141592653589793238463f / 8;
    float f = step;

    vb.push_back(CVect2<float>(1.0f, 0.0f));
    for (int i = 0; i < 7; ++i) {
        float cosf = cos(f), sinf = sin(f);
        vb.push_back(CVect2<float>(cosf, sinf));
        vb.push_back(CVect2<float>(cosf, -sinf));
        f += step;
    }
    vb.push_back(CVect2<float>(-1.0f, 0.0f));

    m_GlyphGeom->SetVertexBuffer2D(vb);
}

void CRenderingContext::PrepareContext(CGlPane& pane, bool horz, bool flipped)
{
    m_Pane = &pane;
    m_bHorz = horz;
    m_bFlipped = flipped;

    m_Offset = horz ? pane.GetOffsetX() : pane.GetOffsetY();
    m_Scale  = fabs(horz ? pane.GetScaleX() : pane.GetScaleY());
    m_ScaleInv = 1.0 / m_Scale;

    TModelRect rcV = pane.GetVisibleRect();
    TModelRect rcM = pane.GetModelLimitsRect();
    if (horz) {
        if (flipped) {
            m_VisRange.Set(rcV.Right(), rcV.Left());
            m_LimitRange.Set(rcM.Right(), rcM.Left());
        } else {
            m_VisRange.Set(rcV.Left(), rcV.Right());
            m_LimitRange.Set(rcM.Left(), rcM.Right());
        }
    } else {
        if (flipped) {
            m_VisRange.Set(rcV.Top(), rcV.Bottom());
            m_LimitRange.Set(rcM.Top(), rcM.Bottom());
        } else {
            m_VisRange.Set(rcV.Bottom(), rcV.Top());
            m_LimitRange.Set(rcM.Bottom(), rcM.Top());
        }
    }

    m_VisSeqRange.Set(
        (TSeqPos)(m_VisRange.GetFrom() + 0.5), (TSeqPos)(m_VisRange.GetTo() + 0.5));

    m_MinLabelSizePos = ScreenToSeq(m_Font_Helv10.TextWidth("A") + kLabelSpacePx);
}


TModelRange CRenderingContext::IntersectVisible_Y(const TModelRange& r) const
{
    TModelRange vert_range(m_Pane->GetVisibleRect().Top(),
        m_Pane->GetVisibleRect().Bottom());
    return vert_range.IntersectionWith(r);
}


void CRenderingContext::DrawQuad(TModelUnit x1, TModelUnit y1,
                                 TModelUnit x2, TModelUnit y2,
                                 bool border) const
{
    // If x1 and x2 are projected onto the same pixel on screen.
    // We simply draw aline instead of a quad.
    IRender& gl = GetGl();
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (fabs(x2-x1) < GetScale()) {
        TModelUnit mid = (x2+x1)/TModelUnit(2.0);
        x1 = mid-GetScale()/(TModelUnit)2.0;
        x2 = mid+GetScale()/(TModelUnit)2.0;
    }
    gl.Rectd(x1 - m_Offset, y1, x2 - m_Offset, y2);

    if (border) {
        DrawRect(x1, y1, x2, y2);
    }
}


void CRenderingContext::DrawShadedQuad(const TModelRect& rcm,
                                       const CRgbaColor& c1,
                                       const CRgbaColor& c2,
                                       bool shade_vert) const
{
    IRender& gl = GetGl();
    if (gl.IsSimplified()) {
        gl.ShadeModel(GL_FLAT);
        gl.ColorC(c1);
        DrawQuad(rcm.Left(), rcm.Top(), rcm.Right() + 1 , rcm.Bottom());
        return;
    }

    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);


    gl.ShadeModel(GL_SMOOTH);
    gl.Begin(GL_QUADS);
    if (shade_vert) {
        gl.ColorC(c1);
        gl.Vertex3d(rcm.Right() - m_Offset + 1, rcm.Top(), 0.0);
        gl.Vertex3d(rcm.Left() - m_Offset,      rcm.Top(), 0.0);
        gl.ColorC(c2);
        gl.Vertex3d(rcm.Left() - m_Offset,      rcm.Bottom(), 0.0);
        gl.Vertex3d(rcm.Right() - m_Offset + 1, rcm.Bottom(), 0.0);
    } else {
        gl.ColorC(c1);
        gl.Vertex3d(rcm.Left() - m_Offset,      rcm.Top(), 0.0);
        gl.Vertex3d(rcm.Left() - m_Offset,      rcm.Bottom(), 0.0);
        gl.ColorC(c2);
        gl.Vertex3d(rcm.Right() - m_Offset + 1, rcm.Bottom(), 0.0);
        gl.Vertex3d(rcm.Right() - m_Offset + 1, rcm.Top(), 0.0);
    }
    gl.End();
    gl.ShadeModel(GL_FLAT);

}

void CRenderingContext::DrawSquare(const TModelPoint& p,
                                   TModelUnit size,
                                   const CRgbaColor& color,
                                   bool neg_strand) const
{
    TModelUnit size_x = ScreenToSeq(size);
    TModelUnit f = neg_strand ? p.X() - size_x : p.X();
    TModelUnit t = neg_strand ? p.X() - 1.0 : p.X() + size_x - 1.0;
    Draw3DQuad(f, p.Y() - size * 0.5f, t, p.Y() + size * 0.5f, color, true);
}


void CRenderingContext::DrawDisk(const TModelPoint& p,
                                 TModelUnit r,
                                 GLint from, GLint to) const
{
    IRender& gl = GetGl();

    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    const double PI_180 = 3.1415926 / 180.0;
    TModelUnit f = from * PI_180;
    TModelUnit t = to * PI_180;
    TModelUnit r_x = ScreenToSeq(r);
    gl.Begin(GL_TRIANGLE_FAN);
        gl.Vertex2d(p.X() - m_Offset, p.Y());
        TModelUnit step = (t - f) * 0.02;
        for (;  f < t + step * 0.02;  f += step) {
            gl.Vertex2d(p.X() - m_Offset + r_x * cos(f),
                p.Y() - r * sin(f));
        }
    gl.End();
}

void CRenderingContext::DrawDisk(const TModelPoint& p,
                                 TModelUnit radius,
                                 const CRgbaColor& color,
                                 bool neg_strand) const
{
    IRender& gl = GetGl();

    TModelUnit add = ScreenToSeq(radius);
    add = neg_strand ? -add : add;
    TModelPoint center(p.X() + add*TModelUnit(0.5), p.Y());

    CRgbaColor color2 = color;
    color2.Lighten(0.5f);
    CRgbaColor color3 = color;
    color3.SetAlpha(0.3f);

    radius *= TModelUnit(0.5);

    // if blend is explicitly enabled, reset it at the end
    bool blend_enabled = gl.GetState()->IsEnabled(GL_BLEND);

    // Largest circle (to get darker outside border):
    gl.Disable(GL_BLEND);
    gl.ColorC(color);
    DrawDisk(center, radius - TModelUnit(0.5));

    // To anti-alias the edge a little:
    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.ColorC(color3);
    DrawDisk(center, radius);

    // Inner circle (lighter):
    gl.Disable(GL_BLEND);
    gl.ColorC(color2);
    DrawDisk(center, radius - TModelUnit(1.5));

    if (blend_enabled)
        gl.Enable(GL_BLEND);
}

void CRenderingContext::DrawDisk2(const TModelPoint& p,
    TModelUnit size,
    CRgbaColor color)
{
    x_CreateGlyphs();

    IRender& gl = GetGl();

    TModelUnit add = ScreenToSeq(size);

    TModelPoint center(p.X() + add*TModelUnit(0.5), p.Y());

    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // This is a radius: 
    size = size/2.0;

    gl.Color4fv(color.GetColorArray());
    gl.PolygonMode(GL_FRONT, GL_FILL);

    CMatrix4<float> m, pos;
    m.Identity();
    m(0, 0) = (float)ScreenToSeq(size - 0.5);
    m(1, 1) = (float)(size - 0.5);
    m(2, 2) = 1.0f;
    m(0, 3) = center.X() - m_Offset;
    m(1, 3) = center.Y();
    
    CMatrix4<float> current_mat = gl.GetModelViewMatrix();
    pos = current_mat*m;
    pos.Transpose();

    m_GlyphGeom->Render(pos.GetData());


    m(0, 0) = (float)ScreenToSeq(size);
    m(1, 1) = (float)size;
    
    color.SetAlpha(color.GetAlpha()*0.75f);
    gl.Color4fv(color.GetColorArray());
    pos = current_mat*m;
    pos.Transpose();
    m_GlyphGeom->Render(pos.GetData());
}



void CRenderingContext::Draw3DTriangle(const TModelPoint& p, TModelUnit size,
                                       const CRgbaColor& color, bool neg_strand,
                                       bool need_neck) const
{
    TModelUnit off_x = ScreenToSeq(size);
    TModelUnit off_y = size;
    TModelUnit add_x = neg_strand ? off_x : (-off_x);

    CRgbaColor color2 = color;
    color2.Lighten(0.5);

    IRender& gl = GetGl();

    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    gl.ColorC(color);
    // draw "neck"
    if (need_neck) {
        DrawLine(p.X() + add_x/*+ 1*/, p.Y(),
            p.X() + add_x * 1.5 /*+1*/, p.Y());
    }

    gl.ShadeModel(GL_SMOOTH);
    gl.Begin(GL_TRIANGLES);

    TModelPoint p0(p.X() + add_x, p.Y());
    TModelPoint p1(p.X(), p.Y());
    TModelPoint p2(p.X() + add_x, p.Y() - off_y);
    TModelPoint p3(p.X() + add_x, p.Y() + off_y);

    gl.ColorC(color2);
    gl.Vertex2d(p0.X() - m_Offset, p0.Y());
    gl.Vertex2d(p1.X() - m_Offset, p1.Y());
    gl.ColorC(color);
    gl.Vertex2d(p2.X() - m_Offset, p2.Y());

    gl.ColorC(color2);
    gl.Vertex2d(p0.X() - m_Offset, p0.Y());
    gl.Vertex2d(p1.X() - m_Offset, p1.Y());
    gl.ColorC(color);
    gl.Vertex2d(p3.X() - m_Offset, p3.Y());

    gl.End();
    gl.ShadeModel(GL_FLAT);

}


void CRenderingContext::Draw3DArrow (const TModelPoint& p,
                                    TModelUnit size, bool neg_strand) const
{
    TModelUnit off_x = ScreenToSeq(size);
    TModelUnit add_x = neg_strand ? off_x : (-off_x);
    TModelUnit off_y = size;
    DrawLine(p.X(), p.Y(), p.X() + add_x, p.Y() - off_y);
    DrawLine(p.X(), p.Y(), p.X() + add_x, p.Y() + off_y);
    DrawLine(p.X(), p.Y(), p.X() + add_x * 1.5, p.Y());
}


void CRenderingContext::DrawArrow(TModelUnit x1, TModelUnit x2, TModelUnit x3,
                                  TModelUnit line_center, TModelUnit bar_h,
                                  TModelUnit head_h) const
{
    IRender& gl = GetGl();


    if (x1 != x2) {
        gl.Begin(GL_TRIANGLE_STRIP);
            gl.Vertex2d(x1 - m_Offset, line_center - bar_h);
            gl.Vertex2d(x2 - m_Offset, line_center - bar_h);
            gl.Vertex2d(x1 - m_Offset, line_center + bar_h);
            gl.Vertex2d(x2 - m_Offset, line_center + bar_h);
        gl.End();
    }

    gl.Begin(GL_TRIANGLES);
        gl.Vertex2d(x2 - m_Offset, line_center + head_h);
        gl.Vertex2d(x3 - m_Offset, line_center);
        gl.Vertex2d(x2 - m_Offset, line_center - head_h);
    gl.End();

}


void CRenderingContext::DrawArrowBoundary(TModelUnit x1, TModelUnit x2, TModelUnit x3,
                                    TModelUnit line_center, TModelUnit bar_h,
                                    TModelUnit head_h) const
{
    IRender& gl = GetGl();
    gl.LineJoinStyle(IGlState::eRoundedJoin);

    gl.Begin(GL_LINE_LOOP);
        gl.Vertex2d(x1 - m_Offset, line_center + bar_h);
        gl.Vertex2d(x2 - m_Offset, line_center + bar_h);
        gl.Vertex2d(x2 - m_Offset, line_center + head_h);
        gl.Vertex2d(x3 - m_Offset, line_center);
        gl.Vertex2d(x2 - m_Offset, line_center - head_h);
        gl.Vertex2d(x2 - m_Offset, line_center - bar_h);
        gl.Vertex2d(x1 - m_Offset, line_center - bar_h);
    gl.End();

    gl.LineJoinStyle(IGlState::eMiteredJoin);
}



void CRenderingContext::DrawGreaterLessSign(TModelUnit x, TModelUnit y,
                                            TModelUnit bar_h, bool negative) const
{
    TModelUnit size = bar_h * 0.5;
    if (bar_h >= 8.0) {
        size -= 1.0;
    }
    TModelUnit x_off = ScreenToSeq((negative ? -1.0 : 1.0) * size * 0.5);
    IRender& gl = GetGl();

    gl.Begin(GL_LINES);
        gl.Vertex2d(x - m_Offset - x_off, y + size);
        gl.Vertex2d(x - m_Offset + x_off, y);
        gl.Vertex2d(x - m_Offset + x_off, y);
        gl.Vertex2d(x - m_Offset - x_off, y - size);
    gl.End();

}



void CRenderingContext::Draw3DQuad(TModelUnit x1, TModelUnit y1,
                                  TModelUnit x2, TModelUnit y2,
                                  const CRgbaColor& color, bool border) const
{
    IRender& gl = GetGl();
    if (gl.IsSimplified())
    {
        gl.ColorC(color);
        gl.ShadeModel(GL_FLAT);
        DrawQuad(x1, y1, x2 + 1, y2, border);
        return;
    }
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // If x1 and x2 get projected onto the same pixel on screen.
    // we simply draw a vertical line to represent a quad.
    if (x2 - x1 + 1.0 < GetScale()) {
        gl.ColorC(color);
        CGlAttrGuard AttrGuard(GL_LINE_BIT);
        gl.Disable(GL_LINE_SMOOTH);
        // HACK, mesa somehow projects y differently for drawling a line
        // and drawing a quad. We add an slight y offset to make this work.
        TModelUnit y_off = 0.0001;
        DrawLine(x1, y1 + y_off, x1, y2 + y_off);
        //gl.Enable(GL_LINE_SMOOTH);
    } else {
        TModelUnit xx1 = x1 - m_Offset;
        TModelUnit yy1 = y1;
        TModelUnit xx2 = x2 - m_Offset;
        TModelUnit yy2 = y2;
        TModelUnit mid = y1 + (y2 - y1) * 0.5f;
//        LOG_POST(Trace << " drawing 3DQuad at left: " <<  xx1 << " and right: " << xx2);

        CRgbaColor color2 = color;
        color2.Lighten(0.5);

        gl.ShadeModel(GL_SMOOTH);
        gl.Begin(GL_TRIANGLE_STRIP);

        gl.ColorC(color);
        gl.Vertex2d(xx1,     yy1);
        gl.Vertex2d(xx2 + 1, yy1);

        gl.ColorC(color2);
        gl.Vertex2d(xx1,     mid);
        gl.Vertex2d(xx2 + 1, mid);

        gl.ColorC(color);
        gl.Vertex2d(xx1,     yy2);
        gl.Vertex2d(xx2 + 1, yy2);
        gl.End();
        gl.ShadeModel(GL_FLAT);

        if (border) {
            gl.ColorC(color);
            DrawRect(x1, y1, x2 + 1.0, y2);
        }
    }
}

void CRenderingContext::Draw3DQuad_HorzLines(TModelUnit x1, TModelUnit y1,
                                             TModelUnit x2, TModelUnit y2,
                                             const CRgbaColor& color, bool border) const
{
    IRender& gl = GetGl();

    // If x1 and x2 get projected onto the same pixel on screen.
    // we simply draw a vertical line to represent a quad.
    if (x2 - x1 + 1.0 < GetScale()) {
        gl.ColorC(color);
        CGlAttrGuard AttrGuard(GL_LINE_BIT);
        gl.Disable(GL_LINE_SMOOTH);
        // HACK, mesa somehow projects y differently for drawling a line
        // and drawing a quad. We add an slight y offset to make this work.
        TModelUnit y_off = 0.0001;
        DrawLine(x1, y1 + y_off, x1, y2 + y_off);
    } else {
        TModelUnit xx1 = x1 - m_Offset;
        TModelUnit yy1 = y1;
        TModelUnit xx2 = x2 - m_Offset;

        CRgbaColor color2 = color;
        color2.Lighten(0.5);
        CVect4<float> c1(color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
        CVect4<float> c2(color2.GetRed(), color2.GetGreen(), color2.GetBlue(), color2.GetAlpha());

        TModelUnit delta(2.0);
        int steps = (y2-y1)/delta;
        TModelUnit ydelta = (y2-y1)/TModelUnit(steps);
        CVect4<float> color_delta = (c2-c1)/(float)(steps/2);

        gl.LineWidth(1.0f);
        gl.Disable(GL_LINE_SMOOTH);

        gl.Begin(GL_LINES);
        for (int i=0; i<steps; i++) {
            gl.Color4f(c1.X(), c1.Y(), c1.Z(), c1.W());

            if (i<steps/2)
                c1 += color_delta;
            else
                c1 -= color_delta;

            double yval = yy1+ydelta*(TModelUnit)i;

            gl.Vertex3d(xx1, yval, 0.0);
            gl.Vertex3d(xx2, yval, 0.0);
        }
        gl.End();
    }

    if (border) {
        gl.ColorC(color);
        DrawRect(x1, y1, x2, y2);
    }

}

void CRenderingContext::Draw3DFletch(TModelUnit pos_x, TModelUnit line_y1,
        TModelUnit line_y2, TModelUnit tail_height,
        const CRgbaColor& color, bool neg_strand) const
{
     IRender& gl = GetGl();

    TModelUnit off = ScreenToSeq(tail_height);
    TModelUnit mid = line_y1 + (line_y2 - line_y1) * 0.5f;
    TModelUnit add = neg_strand ? -off : off;

    CRgbaColor color2 = color;
    color2.Lighten(0.5);

    gl.ShadeModel(GL_SMOOTH);
    gl.Begin(GL_TRIANGLES);

    // p0 is a start point
    TModelPoint p0(pos_x + add, mid);
    TModelPoint p1(pos_x + add, line_y1);
    TModelPoint p2(pos_x      , line_y1);
    TModelPoint p3(pos_x + add, line_y2);
    TModelPoint p4(pos_x      , line_y2);

    gl.ColorC(color2);
    gl.Vertex2d(p0.X() - m_Offset, p0.Y());
    gl.ColorC(color);
    gl.Vertex2d(p1.X() - m_Offset, p1.Y());
    gl.Vertex2d(p2.X() - m_Offset, p2.Y());

    gl.ColorC(color2);
    gl.Vertex2d(p0.X() - m_Offset, p0.Y());
    gl.ColorC(color);
    gl.Vertex2d(p3.X() - m_Offset, p3.Y());
    gl.Vertex2d(p4.X() - m_Offset, p4.Y());
    gl.End();
    gl.ShadeModel(GL_FLAT);

    gl.ColorC(color);
    gl.LineWidth(1.5f);
    // draw border
    DrawLine(pos_x, line_y1, pos_x + add, line_y1);
    DrawLine(pos_x, line_y1, pos_x + add, mid);

    DrawLine(pos_x, line_y2, pos_x + add, line_y2);
    DrawLine(pos_x, line_y2, pos_x + add, mid);
    gl.LineWidth(1.0f);

}


void CRenderingContext::DrawSelection(TModelUnit x1, TModelUnit y1,
                                      TModelUnit x2, TModelUnit y2) const
{
    IRender& gl = GetGl();

    // expand 1 pix on each side
    TModelUnit padding_y = 1.0;
    TModelUnit padding_x = ScreenToSeq(padding_y);
    gl.ColorC(m_SelColor);
    gl.LineWidth(1.5f);
    DrawRect(x1 - padding_x, y1 + padding_y, x2 + padding_x, y2 - padding_y);
    gl.LineWidth(1.0f);

}


void CRenderingContext::DrawRect(TModelUnit x1, TModelUnit y1,
                                 TModelUnit x2, TModelUnit y2) const
{
    IRender& gl = GetGl();
    gl.LineJoinStyle(IGlState::eRoundedJoin);

    gl.Begin(GL_LINE_LOOP);
         gl.Vertex2d(x1 - m_Offset, y1);
         gl.Vertex2d(x1 - m_Offset, y2);
         gl.Vertex2d(x2 - m_Offset, y2);
         gl.Vertex2d(x2 - m_Offset, y1);
    gl.End();

    gl.LineJoinStyle(IGlState::eMiteredJoin);
}


void CRenderingContext::DrawBackground(const TModelRect& rcm,
                                       TModelUnit border) const
{
    IRender& gl = GetGl();

    TModelRect rect(rcm);
    TModelUnit border_x = ScreenToSeq(border);
    // bottom > top which is the reverse of CGlRect
    rect.Inflate(border_x, -border);

    gl.Rectd(rect.Left() - m_Offset, rect.Bottom(), rect.Right() - m_Offset, rect.Top());
//    LOG_POST(Trace << " drawing background rect at left: " <<  (rect.Left() - m_Offset) << " and right: " << (rect.Right() - m_Offset));
}


void CRenderingContext::DrawStrandIndicators(const TModelPoint& start,
                                             TModelUnit length,
                                             TModelUnit apart,
                                             TModelUnit size,
                                             const CRgbaColor& color,
                                             bool neg_strand,
                                             bool avoid_center,
                                             bool single_indicator_center,
                                             const vector<TModelRange> *labels_ranges) const
{
    IRender& gl = GetGl();

    // adjust the start and lenght to ensure only draw indicators
    // inside the visible range
    TModelUnit start_x(start.X());
    TModelUnit start_y(start.Y());
    TModelUnit min_v = m_VisRange.GetFrom();
    TModelUnit max_v = m_VisRange.GetTo();
    if (start_x < min_v) {
        start_x = min_v;
        length -= start_x - start.X();
    }
    if (start_x + length > max_v) {
        length = max_v - start_x;
    }

    int strand_sign = neg_strand ? -1 : 1;
    TModelUnit w1 = 0.2 * size * m_Scale;
    TModelUnit w2 = 4.0 * w1;
    TModelUnit half = 0.4 * size;

    TModelUnit size_x1 = strand_sign * w1;
    TModelUnit size_x2 = strand_sign * w2;
    TModelUnit size_y1 = half;
    TModelUnit size_y2 = half * 2.0;

    int num = (int)floor(length / apart - 0.15) + 1;
    TModelUnit off = (length - apart * (num - 1)) * 0.5 - strand_sign * w2 * 0.5;

    TModelUnit off_x = off - m_Offset + start_x;
    TModelUnit off_y = start_y + 0.1 * size;

    gl.ShadeModel(GL_FLAT);
    gl.PdfShadeStyle(CGlState::eFlat);
    for (int i = 0; i < num; ++i) {
        CRgbaColor curr_color(color.GetRed(), color.GetGreen(),
                              color.GetBlue(), color.GetAlpha());

        // if caller specified single_indicator_center center arrow will be drawn
        // Only if there is just 1 arrow. Otherwise it will be avoided
        if (num % 2 == 1  &&  i == (num - 1) / 2) {
            if (avoid_center || (single_indicator_center && num > 1)) {
                off_x += apart;
                continue;
            }

            if (!single_indicator_center) {
                curr_color.Darken(i % 3 * 0.12f);
            }
            else {
                 curr_color.Darken(0.12f);
            }
        }
        else {
            curr_color.Darken(i % 3 * 0.12f);
        }
        if (labels_ranges) {
            bool skip = false;
            ITERATE(vector<TModelRange>, it_rng, *labels_ranges) {
                TModelUnit ind_from = off_x + m_Offset;
                TModelUnit ind_to = off_x + size_x2 + m_Offset;
                TModelRange ind_range(ind_from < ind_to ? ind_from : ind_to, ind_from < ind_to ? ind_to : ind_from);
                if (it_rng->IntersectingWith(ind_range)) {
                    skip = true;
                    break;
                }
            }
            if (skip) {
                off_x += apart;
                continue;
            }
        }

        gl.ColorC(curr_color);
        gl.Begin(GL_TRIANGLE_STRIP);
        gl.Vertex2d(off_x + 0.0, off_y + size_y2);
        gl.Vertex2d(off_x + size_x2 , off_y + size_y1);
        gl.Vertex2d(off_x + size_x1, off_y + size_y1);
        gl.Vertex2d(off_x, off_y);
        gl.End();
        off_x += apart;
    }
}


void CRenderingContext::Draw5Prime(TModelUnit x, TModelUnit y,
                                   bool direct, TModelUnit size_h,
                                   TModelUnit size_v) const
{
    TModelUnit two_pixel_w = ScreenToSeq(2.0);
    size_h = ScreenToSeq(size_h);
    TModelUnit tri_w = (size_h - two_pixel_w) * 0.5;
    TModelUnit tri_h = (size_v - 2.0) * 0.5;
    const char* label = "5'";
    CGlTextureFont font(CGlTextureFont::eFontFace_Helvetica_Bold, 8);
    size_v -= 1.0;

    TModelRect rect;
    TModelUnit label_x, label_y;
    if (direct) {
        rect.SetLeft(x);
        rect.SetRight(x + size_h);
        rect.SetTop(y - size_v);
        rect.SetBottom(y);
        label_x = x + two_pixel_w;
        label_y = y - 2.0;
    } else {
        rect.SetLeft(x - size_h);
        rect.SetRight(x);
        rect.SetTop(y + size_v);
        rect.SetBottom(y);
        label_x = x - size_h + tri_w - two_pixel_w * 0.5;
        label_y = y + size_v - 1.0;
    }

    DrawRect(rect);
    if (direct) {
        TModelUnit corner_x = x + size_h;
        DrawTriangle(corner_x, y, corner_x, y - tri_h, corner_x - tri_w, y);
        TModelUnit corner_y = y - size_v;
        DrawTriangle(corner_x, corner_y, corner_x - tri_w, corner_y, corner_x, corner_y + tri_h);
    } else {
        TModelUnit corner_x = x - size_h;
        DrawTriangle(corner_x, y, corner_x, y + tri_h, corner_x + tri_w, y);
        TModelUnit corner_y = y + size_v;
        DrawTriangle(corner_x, corner_y, corner_x + tri_w, corner_y, corner_x, corner_y - tri_h);
    }
    TextOut(&font, label, label_x, label_y, false);

}


void CRenderingContext::DrawTexture(const TModelRect& rc,
                                    const TModelRect& rc_coord) const
{
    IRender& gl = GetGl();

    CGlAttrGuard AttrGuard(GL_LINE_BIT);
    GLint prev_shader;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prev_shader);
    gl.UseProgram(0);

    gl.Disable(GL_LINE_SMOOTH);
    gl.Begin(GL_TRIANGLE_STRIP);
        gl.TexCoord2d(rc_coord.Left(), rc_coord.Bottom());
        gl.Vertex2d(rc.Left() - m_Offset,    rc.Bottom());
        gl.TexCoord2d(rc_coord.Right(), rc_coord.Bottom());
        gl.Vertex2d(rc.Right() - m_Offset,   rc.Bottom());
        gl.TexCoord2d(rc_coord.Left(), rc_coord.Top());
        gl.Vertex2d(rc.Left() - m_Offset,    rc.Top());
        gl.TexCoord2d(rc_coord.Right(), rc_coord.Top());
        gl.Vertex2d(rc.Right() - m_Offset,   rc.Top());
    gl.End();
    gl.Enable(GL_LINE_SMOOTH);

    gl.UseProgram(prev_shader);
}


void CRenderingContext::DrawHairLine(int opt, size_t total,
                                     const multiset <TSeqPos>& all_pos,
                                     TSeqPos pos, TModelUnit line_y,
                                     const CRgbaColor& c_light,
                                     const CRgbaColor& c_dark) const
{
    IRender& gl = GetGl();

    bool need_line = true;
    CRgbaColor color;
    switch (opt) {
    case 1: // 1. Show all hairlines with selections
        color = all_pos.count(pos) > 1 ? c_dark : c_light;
        break;
    case 2: // 2. Show only hairlines that are not shared (i.e., places that are different)
        color = c_light;
        need_line = all_pos.count(pos) == 1;
        break;
    case 3: // 3. Show only hairlines that are shared by any two of the selections
        color = c_dark;
        need_line = all_pos.count(pos) >= 2;
        break;
    case 4: // 4. Show hairlines shared by *all* of the selections.
        color = c_dark;
        need_line = all_pos.count(pos) == total;
        break;
    }

    TModelRect rcV = m_Pane->GetVisibleRect();
    // render "From"
    if (need_line) {
        CGlAttrGuard AttrGuard(GL_LINE_BIT);
        gl.ColorC(color);
        gl.Disable(GL_LINE_SMOOTH);
        DrawLine(pos, line_y, pos, m_bHorz ? rcV.Top() : rcV.Right());
        gl.Enable(GL_LINE_SMOOTH);
    }


}


void CRenderingContext::DrawUnalignedTail(TModelUnit x1, TModelUnit x2,
                                          TModelUnit y1, TModelUnit y2,
                                          bool is_polya, const CRgbaColor& color) const
{
    IRender& gl = GetGl();
    auto c = color;
    c.Lighten(0.5);
    gl.ColorC(c);
    double barh = ::fabs(m_Pane->GetScaleY() * ::fabs(y2-y1));
    // simplified rendering (just thin line)
    // when bar height is less that 2 pix
    bool compressed_mode = barh < kMinBarHeight;
   

    {
        CGlAttrGuard AttrGuard(GL_LINE_BIT);
        gl.Disable(GL_LINE_SMOOTH);
        gl.Begin(GL_LINE_LOOP);
        if (compressed_mode) {
            gl.Vertex2d(x1 - m_Offset, y1);
            gl.Vertex2d(x2 - m_Offset, y1);
        } else {
            gl.Vertex2d(x1 - m_Offset, y1);
            gl.Vertex2d(x1 - m_Offset, y2);
            gl.Vertex2d(x2 - m_Offset, y2);
            gl.Vertex2d(x2 - m_Offset, y1);
        }
        gl.End();
    }
    
    if (!compressed_mode) {
        gl.ColorC(color);
        if (is_polya) {
            CGlTextureFont font(CGlTextureFont::eFontFace_Helvetica_Bold, 8);
            TextOut(&font, "pA", (x1 + x2) * 0.5, y2 - 2, true);
        } else {
            //TModelUnit off_v = (y2 - y1) * 0.2;
            TModelUnit off_h = (x2 - x1) / 6;
            //x1 += off_h;
            //y1 += off_v;
            //y2 -= off_v;
            gl.Begin(GL_LINE_STRIP);
            gl.Vertex2d(x1 - m_Offset, y2);
            x1 += 2 * off_h;
            gl.Vertex2d(x1 - m_Offset, y1);
            x1 += 0.5 * off_h;
            gl.Vertex2d(x1 - m_Offset, y2);
            x1 += off_h;
            gl.Vertex2d(x1 - m_Offset, y1);
            x1 += 0.5 * off_h;
            gl.Vertex2d(x1 - m_Offset, y2);
            x1 += 2 * off_h;
            gl.Vertex2d(x1 - m_Offset, y1);
            gl.End();
        }
    }

}


// slant dashed line
void CRenderingContext::DrawPseudoBar(TModelUnit x1, TModelUnit y1,
                                      TModelUnit x2, TModelUnit y2,
                                      const CRgbaColor& light_color,
                                      const CRgbaColor& dark_color) const
{
    IRender& gl = GetGl();

    TModelUnit xoff = 0.0;
    TModelUnit delta = ScreenToSeq(8.0);
    bool shaded = true;

    if (fabs(x2-x1) <= GetScale()) {
        gl.ColorC(light_color);
        CGlAttrGuard AttrGuard(GL_LINE_BIT);
        gl.Disable(GL_LINE_SMOOTH);
        // HACK, mesa somehow projects y differently for drawling a line
        // and drawing a quad. We add an slight y offset to make this work.
        TModelUnit y_off = 0.0001;
        DrawLine(x1, y1 + y_off, x1, y2 + y_off);
        return;
    }

    // Draw a triangle strip alternating colors for each pair of triangles
    gl.ShadeModel(GL_FLAT);
    gl.PdfShadeStyle(IGlState::eFlat);
    gl.Enable(GL_BLEND);
    gl.Begin(GL_TRIANGLE_STRIP);
        gl.ColorC(light_color);
        gl.Vertex3d(x1 - m_Offset, y1, 0.0);
        gl.Vertex3d(x1 - m_Offset, y2, 0.0);

        while (xoff+delta <= x2-x1) {
            xoff += delta;

            gl.Vertex3d(x1 + xoff - m_Offset, y1, 0.0);
            if (shaded)
                gl.ColorC(dark_color);
            else
                gl.ColorC(light_color);
            gl.Vertex3d(x1 + xoff - m_Offset, y2, 0.0);

            shaded = !shaded;
        }

        // Draw end (should pretty much always be a little more
        // to draw...)
        if (x1 + xoff < x2) {
            gl.Vertex3d(x2 - m_Offset, y1, 0.0);
            if (shaded)
                gl.ColorC(dark_color);
            else
                gl.ColorC(light_color);
            gl.Vertex3d(x2 - m_Offset, y2, 0.0);
        }

    gl.End();
}

// Draw 2 <'s with some anti-aliasing.  The previous texture approach
// worked just as well (or better), but doesn't work with PDF output
void CRenderingContext::DrawPartialBarMark(TModelUnit x1, TModelUnit y1,
    TModelUnit y2, bool loc, TModelUnit dir) const
{
    IRender& gl = GetGl();

    TModelUnit mid = (y1 + y2)/2.0;
    TModelUnit w1 = dir*ScreenToSeq(1.0);
    TModelUnit w2 = dir*ScreenToSeq(5.0);

    if (loc) {
        gl.Color4f(0.3f, 0.1f, 0.1f, 0.7f);
    } else {
        gl.Color4f(1.0f, 1.0f, 1.0f, 0.7f);
    }

    // darker single-width arrow
    gl.Begin(GL_TRIANGLES);
    gl.Vertex3d(x1 + w1 - m_Offset, mid, 0.0);
    gl.Vertex3d(x1 + w2 - m_Offset, y2, 0.0);
    gl.Vertex3d(x1 + w2-w1 - m_Offset, y2, 0.0);

    gl.Vertex3d(x1 + w1 - m_Offset, mid, 0.0);
    gl.Vertex3d(x1 + w2-w1 - m_Offset, y2, 0.0);
    gl.Vertex3d(x1 - m_Offset, mid, 0.0);

    gl.Vertex3d(x1 + w1 - m_Offset, mid, 0.0);
    gl.Vertex3d(x1 + w2 - m_Offset, y1, 0.0);
    gl.Vertex3d(x1 + w2-w1 - m_Offset, y1, 0.0);

    gl.Vertex3d(x1 + w1 - m_Offset, mid, 0.0);
    gl.Vertex3d(x1 + w2-w1 - m_Offset, y1, 0.0);
    gl.Vertex3d(x1 - m_Offset, mid, 0.0);

    gl.End();

    if (loc) {
        gl.Color4f(0.3f, 0.1f, 0.1f, 0.3f);
    } else {
        gl.Color4f(1.0f, 1.0f, 1.0f, 0.3f);
    }

    // Overlay lighter arrow for antialiasing
    w1 = dir*ScreenToSeq(3.0);
    w2 = dir*ScreenToSeq(7.0);
    x1 -= dir*ScreenToSeq(1.0);

    gl.Begin(GL_TRIANGLES);
    gl.Vertex3d(x1 + w1 - m_Offset, mid, 0.0);
    gl.Vertex3d(x1 + w2 - m_Offset, y2, 0.0);
    gl.Vertex3d(x1 + w2-w1 - m_Offset, y2, 0.0);

    gl.Vertex3d(x1 + w1 - m_Offset, mid, 0.0);
    gl.Vertex3d(x1 + w2-w1 - m_Offset, y2, 0.0);
    gl.Vertex3d(x1 - m_Offset, mid, 0.0);

    gl.Vertex3d(x1 + w1 - m_Offset, mid, 0.0);
    gl.Vertex3d(x1 + w2 - m_Offset, y1, 0.0);
    gl.Vertex3d(x1 + w2-w1 - m_Offset, y1, 0.0);

    gl.Vertex3d(x1 + w1 - m_Offset, mid, 0.0);
    gl.Vertex3d(x1 + w2-w1 - m_Offset, y1, 0.0);
    gl.Vertex3d(x1 - m_Offset, mid, 0.0);

    gl.End();
}

// symbol '>>'
void CRenderingContext::DrawPartialBar(TModelUnit x1, TModelUnit y1,
                                       TModelUnit x2, TModelUnit y2,
                                       bool p_start, bool p_stop,
                                       bool loc) const
{
    IRender& gl = GetGl();

    TModelUnit tex_len = ScreenToSeq(8);
    if (x2 - x1 < tex_len) return;

    if (p_start) {
        DrawPartialBarMark(x1+ScreenToSeq(1.0), y1, y2, loc);
        DrawPartialBarMark(x1 + ScreenToSeq(5.0), y1, y2, loc);
    }
    if (p_stop  &&  (!p_start  ||  (x2 - x1) > 2.0 * tex_len)) {
        gl.Disable(GL_CULL_FACE);
        DrawPartialBarMark(x2-ScreenToSeq(1.0), y1, y2, loc, -1);
        DrawPartialBarMark(x2-ScreenToSeq(5.0), y1, y2, loc, -1);
    }

}


void CRenderingContext::DrawHighlight(const TModelRect& rect, const CRgbaColor *color) const
{
    IRender& gl = GetGl();

    TModelUnit band_w = 4.0;
    TModelUnit band_w_x = ScreenToSeq(band_w);
    TModelUnit left = rect.Left() - m_Offset;
    TModelUnit right = rect.Right() - m_Offset;
    TModelUnit frame_gap = ScreenToSeq(1.0);
    left -= frame_gap;
    right += frame_gap;

    TModelUnit top = rect.Top() - 1.0;
    TModelUnit bottom =  rect.Bottom();

    //CRgbaColor color1(0.23f, 0.63f, 0.73f, 1.0f);
    //CRgbaColor color2(0.23f, 0.63f, 0.73f, 0.1f);

    CRgbaColor color1(color ? *color : m_HighlightsColor);
    CRgbaColor color2(0.5f, 0.5f, 0.5f, 0.1f);

    gl.ShadeModel(GL_FLAT);

    gl.ShadeModel(GL_SMOOTH);
    gl.Begin(GL_QUADS);
    gl.ColorC(color1);
    gl.Vertex3d(left, top, 0.0);
    gl.Vertex3d(right, top, 0.0);
    gl.ColorC(color2);
    gl.Vertex3d(right, top - band_w, 0.0);
    gl.Vertex3d(left, top - band_w, 0.0);
    gl.ColorC(color1);
    gl.Vertex3d(right, bottom, 0.0);
    gl.Vertex3d(left, bottom, 0.0);
    gl.ColorC(color2);
    gl.Vertex3d(left, bottom + band_w, 0.0);
    gl.Vertex3d(right, bottom + band_w, 0.0);

    gl.ColorC(color1);
    gl.Vertex3d(left, bottom, 0.0);
    gl.Vertex3d(left, top, 0.0);
    gl.ColorC(color2);
    gl.Vertex3d(left - band_w_x, top, 0.0);
    gl.Vertex3d(left - band_w_x, bottom, 0.0);
    gl.ColorC(color1);
    gl.Vertex3d(right, bottom, 0.0);
    gl.Vertex3d(right, top, 0.0);
    gl.ColorC(color2);
    gl.Vertex3d(right + band_w_x, top, 0.0);
    gl.Vertex3d(right + band_w_x, bottom, 0.0);

    gl.ColorC(color1);
    gl.Vertex3d(left, top, 0.0);
    gl.ColorC(color2);
    gl.Vertex3d(left, top - band_w, 0.0);
    gl.Vertex3d(left - band_w_x, top - band_w, 0.0);
    gl.Vertex3d(left - band_w_x, top, 0.0);
    gl.ColorC(color1);
    gl.Vertex3d(right, top, 0.0);
    gl.ColorC(color2);
    gl.Vertex3d(right + band_w_x, top, 0.0);
    gl.Vertex3d(right + band_w_x, top - band_w, 0.0);
    gl.Vertex3d(right, top - band_w, 0.0);

    gl.ColorC(color1);
    gl.Vertex3d(left, bottom, 0.0);
    gl.ColorC(color2);
    gl.Vertex3d(left, bottom + band_w, 0.0);
    gl.Vertex3d(left - band_w_x, bottom + band_w, 0.0);
    gl.Vertex3d(left - band_w_x, bottom, 0.0);
    gl.ColorC(color1);
    gl.Vertex3d(right, bottom, 0.0);
    gl.ColorC(color2);
    gl.Vertex3d(right + band_w_x, bottom, 0.0);
    gl.Vertex3d(right + band_w_x, bottom + band_w, 0.0);
    gl.Vertex3d(right, bottom + band_w, 0.0);

    gl.End();

    gl.ShadeModel(GL_FLAT);

}

void CRenderingContext::TextOut(const CGlTextureFont* font,
                                const char* text,
                                TModelUnit x, TModelUnit y,
                                bool center, bool adjust_flip) const
{
    IRender& gl = GetGl();

    x -= m_Offset;
    TModelUnit width = ScreenToSeq(gl.TextWidth(font, text));
    if (center) {
        x += m_bHorz ? -width * 0.5 : width * 0.5;
        //x = max(x, TModelUnit(0.0));
    }
    if (m_bFlipped  &&  adjust_flip) {
        x += m_bHorz ? width : -width;
    }

    gl.BeginText(font);
    gl.WriteText(x, y, text);
    gl.EndText();
}

void CRenderingContext::TextOut(const CGlBitmapFont* font,
                                const char* text,
                                TModelUnit x, TModelUnit y,
                                bool center, bool adjust_flip) const
{
    x -= m_Offset;
    TModelUnit width = ScreenToSeq(font->TextWidth(text));
    if (center) {
        x += m_bHorz ? -width * 0.5 : width * 0.5;
        //x = max(x, TModelUnit(0.0));
    }
    if (m_bFlipped  &&  adjust_flip) {
        x += m_bHorz ? width : -width;
    }

    font->TextOut(x, y, text);

}

void CRenderingContext::DrawLine(TModelUnit x1, TModelUnit y1,
                                 TModelUnit x2, TModelUnit y2) const
{
    IRender& gl = GetGl();

    gl.Begin(GL_LINES);
        gl.Vertex2d(x1 - m_Offset, y1);
        gl.Vertex2d(x2 - m_Offset, y2);
    gl.End();
}


void CRenderingContext::SetIsDrawn(const string& sPName, bool isDrawn)
{
    if(isDrawn) {
        m_DrawnSet.insert(sPName);
    } else {
        m_DrawnSet.erase(sPName);
    }
}



bool CRenderingContext::GetIsDrawn(const string& sPName) const
{
    return m_DrawnSet.count(sPName) > 0;
}



END_NCBI_SCOPE
