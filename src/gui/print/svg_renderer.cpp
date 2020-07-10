/*  $Id: svg_renderer.cpp 45038 2020-05-13 01:22:22Z evgeniev $
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
 * Authors:  Vladislav Evgeniev
 *
 * File Description: SVG file rendering
 *
 */

#include <ncbi_pch.hpp>

#include <sstream>
#include <iomanip>

#include <gui/print/svg_renderer.hpp>
#include <gui/print/svg.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/opengl/tri_perimeter.hpp>

#include <gui/utils/matrix4.hpp>
#include <gui/utils/matrix3.hpp>

#include <gui/utils/string_utils.hpp>

#include <util/image/image.hpp>
#include <util/image/image_io.hpp>
#include <util/md5.hpp>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CSVGRenderer::Initialize(const TVPRect& viewport)
{
    static string svg_css = \
        " <![CDATA[\n" \
        "\t\t@import url(\"https://www.ncbi.nlm.nih.gov/projects/sviewer/css/svg_fonts.css\");\n" \
        "\t]]>\n";

    m_Height = viewport.Height();

    m_SVG.reset(new svg::SVG(svg::ViewBox(0, 0, viewport.Width(), viewport.Height()), svg::Style(svg_css), NStr::XmlEncode("Generated by NCBI Genome Workbench http://www.ncbi.nlm.nih.gov/tools/gbench/")));
}

void CSVGRenderer::Finalize()
{
    m_SVG.reset();
    m_PolygonStipplePatterns.clear();
    m_LinearGradients.clear();
    m_ClippingRects.clear();
}

void CSVGRenderer::Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    //CRenderVector::Viewport(x, m_Height - y - height, width, height);
    CRenderVector::Viewport(x, y, width, height);
}

void CSVGRenderer::Ortho(GLdouble left, GLdouble right,
    GLdouble bottom, GLdouble top,
    GLdouble nearVal, GLdouble farVal)
{
    //CRenderVector::Ortho(left, right, top, bottom, nearVal, farVal);
    CRenderVector::Ortho(left, right, bottom, top, nearVal, farVal);
}

void CSVGRenderer::BeginText(const CGlTextureFont* font,
    const CRgbaColor& color)
{
    if (m_CurrentFont != NULL) {
        LOG_POST("BeginText() called without first calling CSVGRenderer::EndText()");
        return;
    }

    m_CurrentFont = font;
    m_CurrentFont->BeginText();
    m_TextColor = color;
}

void CSVGRenderer::BeginText(const CGlTextureFont* font)
{
    if (m_CurrentFont != NULL) {
        LOG_POST("BeginText() called without first calling CSVGRenderer::EndText()");
        return;
    }

    m_CurrentFont = font;
    m_CurrentFont->BeginText();
    m_TextColor = m_RenderNode.GetState().GetColor();
}

void CSVGRenderer::EndText()
{
    _ASSERT(m_CurrentFont);
    if (m_CurrentFont == NULL) {
        LOG_POST("EndText() called without first calling CSVGRenderer::BeginText()");
        return;
    }

    m_CurrentFont->EndText();
    m_CurrentFont = NULL;
}

void CSVGRenderer::WriteText(TModelUnit x, TModelUnit y,
    const char* text,
    TModelUnit rotate_degrees)
{
    _ASSERT(m_CurrentFont);
    if (m_CurrentFont == NULL) {
        LOG_POST(Error << "Unable to write text - must call BeginText() first");
        return;
    }

    CVect2<float> origin((float)x, (float)y);
    m_CurrentFont->ProjectVertex(origin);
    origin.Set(origin.X(), m_Height - origin.Y() - 1);

    string encoded_text = NStr::XmlEncode(text);
    *m_SVG << svg::Text(origin, encoded_text, svg::Font(*m_CurrentFont), svg::Fill(m_TextColor), m_ClippingId, rotate_degrees);
}

void CSVGRenderer::WriteText(TModelUnit x, TModelUnit y,
    TModelUnit width, TModelUnit height,
    const char* text,
    CGlTextureFont::TAlign align,
    CGlTextureFont::ETruncate trunc,
    TModelUnit rotate_degrees)
{
    _ASSERT(m_CurrentFont);
    if (m_CurrentFont == NULL) {
        LOG_POST(Error << "Unable to write text - must call BeginText() first");
        return;
    }

    CVect2<float> origin((float)x, (float)y);
    if (align & CGlTextureFont::eAlign_HCenter) {
        origin.Set((float)(x + width / 2.0), (float)y);
    }
    else if (align & CGlTextureFont::eAlign_Right) {
        origin.Set((float)(x + width), (float)y);
    }
    if (align & CGlTextureFont::eAlign_VCenter) {
        origin.Set(origin.X(), (float)(y + (height - m_CurrentFont->TextHeight()) / 2.0));
    }
    m_CurrentFont->ProjectVertex(origin);
    origin.Set(origin.X(), m_Height - origin.Y() - 1);

    string encoded_text = NStr::XmlEncode((trunc == CGlTextureFont::eTruncate_None) ? text : m_CurrentFont->Truncate(text, width, trunc));
    *m_SVG << svg::Text(origin, encoded_text, svg::Font(*m_CurrentFont, align), svg::Fill(m_TextColor), m_ClippingId, rotate_degrees);
}

void CSVGRenderer::Write(CNcbiOstream& ostrm)
{
    m_SVG->WriteToStream(ostrm);
    ostrm.flush();
}

void CSVGRenderer::BeginClippingRect(GLint x, GLint y, GLsizei width, GLsizei height)
{
    if (!m_ClippingId.empty())
        m_ClippingStack.push(m_ClippingId);
    m_ClippingId = x_ClippingRect(x, y, width, height);
}

void CSVGRenderer::EndClippingRect()
{
    if (!m_ClippingStack.empty()) {
        m_ClippingId = m_ClippingStack.top();
        m_ClippingStack.pop();
    }
    else {
        m_ClippingId.clear();
    }
}

void CSVGRenderer::x_RenderBuffer(CGlVboNode* node)
{
    MatrixMode(GL_MODELVIEW);
    PushMatrix();

    for (size_t j = 0; j < node->GetPositions().size(); ++j) {
        CMatrix4<float> mat = node->GetTransformedPosition(j);
        mat.Transpose();
        LoadMatrixf(mat.GetData());

        if (node->GetDrawMode() == GL_LINES ||
            node->GetDrawMode() == GL_LINE_STRIP ||
            node->GetDrawMode() == GL_LINE_LOOP) {
            x_PrintLineBuffer(*node);
        }
        if (node->GetDrawMode() == GL_POINTS) {
            x_PrintPointBuffer(*node);
        }
        else if (node->GetDrawMode() == GL_TRIANGLES ||
            node->GetDrawMode() == GL_TRIANGLE_FAN ||
            node->GetDrawMode() == GL_TRIANGLE_STRIP) {
            x_PrintTriBuffer(*node);
        }
        else if (node->GetDrawMode() == GL_QUADS) {
            x_PrintQuadBuffer(*node);
        }
    }

    PopMatrix();
}

void CSVGRenderer::x_PrintLineBuffer(CGlVboNode &node)
{
    _ASSERT(m_SVG);
    vector<CVect2<float> > vertices;
    vector<CRgbaColor> colors;

    node.Get2DVertexBuffer(vertices);
    x_ProjectVertices(vertices);

    if (vertices.size() == 0)
        return;

    GLushort pattern = 0xffff; // glLineStipple pattern
    GLint factor = 1; // glLineStipple repeat factor
    double width = 0.0; // linewidth
    svg::Stroke::ELineCapStyle lcap{ svg::Stroke::eDefaultCap };
    svg::Stroke::ELineJoinStyle ljoin{ svg::Stroke::eDefaultJoin };

    bool has_color = node.GetColors(colors, m_IsGreyscale);
    // Use color buffer for color or, if none, use color in State.
    CRgbaColor default_color;
    node.GetDefaultColor(default_color, m_IsGreyscale);

    x_GetLineStyle(node, pattern, factor, width, lcap, ljoin);

    if (node.GetDrawMode() == GL_LINES) {
        for (size_t i = 0; i < vertices.size(); i += 2) {
            if (i + 1 >= vertices.size())
                break;

            *m_SVG << svg::Line(vertices[i], vertices[i + 1], svg::Stroke(has_color ? colors[i] : default_color, width, ljoin, (svg::Stroke::ELineCapStyle)lcap, pattern, factor), m_ClippingId);
        }
    }
    else if (node.GetDrawMode() == GL_LINE_STRIP) {
        for (size_t i = 1; i < vertices.size(); i++) {
            *m_SVG << svg::Line(vertices[i - 1], vertices[i], svg::Stroke(has_color ? colors[i - 1] : default_color, width, ljoin, (svg::Stroke::ELineCapStyle)lcap, pattern, factor), m_ClippingId);
        }
    }
    else if (node.GetDrawMode() == GL_LINE_LOOP) {
        for (size_t i = 1; i < vertices.size(); i++) {
            *m_SVG << svg::Line(vertices[i - 1], vertices[i], svg::Stroke(has_color ? colors[i - 1] : default_color, width, ljoin, (svg::Stroke::ELineCapStyle)lcap, pattern, factor), m_ClippingId);
        }
        *m_SVG << svg::Line(vertices[0], vertices[vertices.size() - 1], svg::Stroke(has_color ? colors[0] : default_color, width, ljoin, (svg::Stroke::ELineCapStyle)lcap, pattern, factor), m_ClippingId);
    }
}

void CSVGRenderer::x_PrintPointBuffer(CGlVboNode &node)
{
    _ASSERT(m_SVG);
    vector<CVect2<float> > vertices;
    vector<CRgbaColor> colors;

    node.Get2DVertexBuffer(vertices);
    x_ProjectVertices(vertices);

    if (vertices.size() == 0)
        return;

    bool has_color = node.GetColors(colors, m_IsGreyscale);

    // Use color buffer for color or, if none, use color in State, if set 
    CRgbaColor default_color;
    node.GetDefaultColor(default_color, m_IsGreyscale);

    double width = 1; // pointsize
    if (node.GetState().PointSizeSet()) {
        width = node.GetState().GetPointSize();
    }

    if (node.GetDrawMode() == GL_POINTS) {
        for (size_t i = 0; i < vertices.size(); ++i) {
            *m_SVG << svg::Circle(vertices[i], width, svg::Fill(has_color ? colors[i] : default_color), svg::Stroke(), m_ClippingId);
        }
    }
}

void CSVGRenderer::x_PrintTriBuffer(CGlVboNode &node)
{
    _ASSERT(m_SVG);
    vector<CVect2<float> > vertices;
    vector<CRgbaColor> colors;

    node.Get2DVertexBuffer(vertices);
    x_ProjectVertices(vertices);

    if (vertices.size() == 0)
        return;

    vector<CRgbaColor> gradient;
    bool has_color = node.GetColors(colors, m_IsGreyscale);
    bool flat_shading = (node.GetState().GetShadeModel() == GL_FLAT);
    if (has_color && !flat_shading) {
        x_GetGradientColors(vertices, colors, gradient);
    }

    // Use color buffer for color or, if none, use color in State, if set 
    CRgbaColor default_color;
    node.GetDefaultColor(default_color, m_IsGreyscale);

    GLushort pattern = 0xffff; // glLineStipple pattern
    GLint factor = 1; // glLineStipple repeat factor
    double width = 0.0; // linewidth
    svg::Stroke::ELineCapStyle lcap{ svg::Stroke::eDefaultCap };
    svg::Stroke::ELineJoinStyle ljoin{ svg::Stroke::eDefaultJoin };
    
    // GL_LINE or GL_FILL
    bool filled = true;
    if (node.GetState().GetPolygonMode() == GL_LINE) {
        filled = false;

        x_GetLineStyle(node, pattern, factor, width, lcap, ljoin);
    }

    string fill_pattern;
    if (node.GetState().IsEnabled(GL_POLYGON_STIPPLE) && node.GetState().PolygonStippleSet()) {
        fill_pattern = x_PolygonStippleToPattern(node.GetState().GetPolygonStipple(), has_color ? colors[0] : default_color);
    }

    svg::Fill fill;
    if (!fill_pattern.empty()) {
        fill = svg::Fill((string const&)fill_pattern);
    }
    else if (gradient.size() > 1) {
        fill = svg::Fill((string const&)x_LinearGradient(gradient));
    }
    else {
        fill = svg::Fill(has_color ? colors[0] : default_color);
    }

    if (node.GetDrawMode() == GL_TRIANGLES) {

        CTriPerimeter tp;
        bool uniform_color{ true };
        for (size_t i = 0; i < vertices.size(); i += 3) {
            if (i + 2 >= vertices.size())
                break;

            if (has_color && flat_shading) {
                if (!(colors[i] == colors[0])) {
                    tp.Clear();
                    uniform_color = false;
                    break;
                }
            }

            tp.AddTri(vertices[i], vertices[i + 1], vertices[i + 2]);
        }

        // Can we convert this into a single perimeter?
        vector<CVect2<float> > perimeter = tp.GetPerimiter();

        if (perimeter.size() > 2 && filled) {
            *m_SVG << svg::Polygon(perimeter, fill, svg::Stroke(), m_ClippingId);
        }
        else {
            for (size_t i = 0; i < vertices.size(); i += 3) {
                if (i + 2 >= vertices.size())
                    break;

                vector<CVect2<float> > triangle;
                triangle.push_back(vertices[i]);
                triangle.push_back(vertices[i + 1]);
                triangle.push_back(vertices[i + 2]);

                if (filled) {
                    if (uniform_color)
                        *m_SVG << svg::Polygon(triangle, fill, svg::Stroke(), m_ClippingId);
                    else
                        *m_SVG << svg::Polygon(triangle, svg::Fill(colors[i]), svg::Stroke(), m_ClippingId);
                }
                else {
                    *m_SVG << svg::Polygon(triangle, svg::Fill(), svg::Stroke(has_color ? colors[0] : default_color, width, ljoin, (svg::Stroke::ELineCapStyle)lcap, pattern, factor), m_ClippingId);
                }
            }
        }
    }
    else if (node.GetDrawMode() == GL_TRIANGLE_STRIP) {
        bool uniform_color{ true };
        CTriPerimeter tp;
        for (size_t i = 2; i < vertices.size(); i++) {
            tp.AddTri(vertices[i], vertices[i - 1], vertices[i - 2]);

            if (has_color && flat_shading) {
                if (!(colors[i] == colors[0])) {
                    tp.Clear();
                    uniform_color = false;
                    break;
                }
            }
        }

        // Can we convert this into a single perimeter?
        vector<CVect2<float> > perimeter = tp.GetPerimiter();

        if (perimeter.size() > 2 && filled) {
            *m_SVG << svg::Polygon(perimeter, fill, svg::Stroke(), m_ClippingId);
        }
        else {
            for (size_t i = 2; i < vertices.size(); i++) {
                vector<CVect2<float> > triangle;
                triangle.push_back(vertices[i]);
                triangle.push_back(vertices[i - 1]);
                triangle.push_back(vertices[i - 2]);

                if (filled) {
                    if (uniform_color)
                        *m_SVG << svg::Polygon(triangle, fill, svg::Stroke(), m_ClippingId);
                    else
                        *m_SVG << svg::Polygon(triangle, svg::Fill(colors[i]), svg::Stroke(), m_ClippingId);
                }
                else {
                    *m_SVG << svg::Polygon(triangle, svg::Fill(), svg::Stroke(has_color ? colors[0] : default_color, width, ljoin, (svg::Stroke::ELineCapStyle)lcap, pattern, factor), m_ClippingId);
                }
            }
        }
    }
    else if (node.GetDrawMode() == GL_TRIANGLE_FAN) {
        CVect2<float> v1p = vertices[0];

        vector<CVect2<float> > perimeter;
        for (size_t i = 1; i < vertices.size(); i++) {
            perimeter.push_back(vertices[i]);
        }

        // would user maybe want perimiter for edges? The could have always
        // just drawn edges for that, rather than tris.
        if (filled) {
            *m_SVG << svg::Polygon(perimeter, fill, svg::Stroke(), m_ClippingId);
        }
        else {
            for (size_t i = 2; i < vertices.size(); i++) {
                CVect2<float> v2p = vertices[i - 1];
                CVect2<float> v3p = vertices[i];

                *m_SVG << svg::Line(v1p, v2p, svg::Stroke(has_color ? colors[0] : default_color, width, ljoin, (svg::Stroke::ELineCapStyle)lcap, pattern, factor), m_ClippingId);
                *m_SVG << svg::Line(v2p, v3p, svg::Stroke(has_color ? colors[0] : default_color, width, ljoin, (svg::Stroke::ELineCapStyle)lcap, pattern, factor), m_ClippingId);
                *m_SVG << svg::Line(v3p, v1p, svg::Stroke(has_color ? colors[0] : default_color, width, ljoin, (svg::Stroke::ELineCapStyle)lcap, pattern, factor), m_ClippingId);
            }
        }
    }
}

void CSVGRenderer::x_PrintQuadBuffer(CGlVboNode &node)
{
    _ASSERT(m_SVG);
    vector<CVect2<float> > vertices;
    vector<CRgbaColor> colors;

    node.Get2DVertexBuffer(vertices);
    x_ProjectVertices(vertices);

    if (vertices.size() == 0)
        return;

    bool has_color = node.GetColors(colors, m_IsGreyscale);

    // use color in State.  Since triangles are flat, there is no color buffer
    // (if we see a buffer we assume it is not flat)
    CRgbaColor default_color;
    node.GetDefaultColor(default_color, m_IsGreyscale);

    GLushort pattern = 0xffff; // glLineStipple pattern
    GLint factor = 1; // glLineStipple repeat factor
    double width = 1.0; // linewidth
    svg::Stroke::ELineCapStyle lcap{ svg::Stroke::eDefaultCap };
    svg::Stroke::ELineJoinStyle ljoin{ svg::Stroke::eDefaultJoin };

    // GL_LINE or GL_FILL
    bool filled = true;
    if (node.GetState().GetPolygonMode() == GL_LINE) {
        filled = false;

        x_GetLineStyle(node, pattern, factor, width, lcap, ljoin);
    }
    svg::Fill fill(has_color ? colors[0] : default_color);   

    for (size_t i = 3; i < vertices.size(); i += 4) {

        vector<CVect2<float> > quad;
        quad.push_back(vertices[i - 3]);
        quad.push_back(vertices[i - 2]);
        quad.push_back(vertices[i - 1]);
        quad.push_back(vertices[i]);

        if (filled) {
            *m_SVG << svg::Polygon(quad, fill, svg::Stroke(has_color ? colors[0] : default_color, width, ljoin, (svg::Stroke::ELineCapStyle)lcap, pattern, factor), m_ClippingId);
        }
        else {
            *m_SVG << svg::Polygon(quad, svg::Fill(), svg::Stroke(has_color ? colors[0] : default_color, width, ljoin, (svg::Stroke::ELineCapStyle)lcap, pattern, factor), m_ClippingId);
        }
    }
}



inline void CSVGRenderer::x_ProjectVertex(CVect2<float>& vertex)
{
    double px, py;
    double dummyz;

    GLdouble model_view_matrix[16];
    GLdouble projection_matrix[16];
    GetModelViewMatrix(model_view_matrix);
    GetProjectionMatrix(projection_matrix);

    gluProject(vertex.X(), vertex.Y(), 0.0, model_view_matrix, projection_matrix, m_Viewport, &px, &py, &dummyz);
    vertex.Set(float(px), float(m_Height - py - 1));
}


inline void CSVGRenderer::x_ProjectVertices(vector<CVect2<float>>& vertices)
{
    GLdouble model_view_matrix[16];
    GLdouble projection_matrix[16];
    GetModelViewMatrix(model_view_matrix);
    GetProjectionMatrix(projection_matrix);

    for (auto& vertex : vertices) {
        double px, py;
        double dummyz;

        gluProject(vertex.X(), vertex.Y(), 0.0, model_view_matrix, projection_matrix, m_Viewport, &px, &py, &dummyz);
        vertex.Set(float(px), float(m_Height - py - 1));
    }
}

string CSVGRenderer::x_PolygonStippleToPattern(const GLubyte* stipple, const CRgbaColor& fill_rgba_color)
{
    string stipple_hash;
    {
        CMD5 md5;
        md5.Update((const char*)stipple, sizeof(GLubyte) * 128);
        string color = fill_rgba_color.ToString();
        md5.Update(color.c_str(), color.length());
        stipple_hash = md5.GetHexSum();
    }
    if (m_PolygonStipplePatterns.end() != m_PolygonStipplePatterns.find(stipple_hash))
        return m_PolygonStipplePatterns[stipple_hash];

    unsigned char fill_color[4];
    fill_color[CImage::eRed] = fill_rgba_color.GetRedUC();
    fill_color[CImage::eGreen] = fill_rgba_color.GetGreenUC();
    fill_color[CImage::eBlue] = fill_rgba_color.GetBlueUC();
    fill_color[CImage::eAlpha] = fill_rgba_color.GetAlphaUC();
    unsigned char empty_color[4];
    empty_color[CImage::eRed] = 0xff;
    empty_color[CImage::eGreen] = 0xff;
    empty_color[CImage::eBlue] = 0xff;
    empty_color[CImage::eAlpha] = 0x00;
    CImage stipple_pttn(32, 32, 4);
    unsigned k = 0;
    for (unsigned i = 0; i < 128; ++i) {

        for (int j = 7; j >= 0; j--) {

            size_t y = (k % 32);
            size_t x = 31 - (k / 32);

            if (stipple[i] & (1 << j)) {
                memcpy(stipple_pttn(x, y), fill_color, sizeof(fill_color));
            }
            else {
                memcpy(stipple_pttn(x, y), empty_color, sizeof(empty_color));
            }
            k++;
        }
    }
    std::stringstream ss;
    CImageIO::WriteImage(stipple_pttn, ss, CImageIO::ePng);
    string pattern_id("polygon-stipple");
    pattern_id += NStr::NumericToString(m_PolygonStipplePatterns.size() + 1);
    m_PolygonStipplePatterns[stipple_hash] = pattern_id;

    *m_SVG << svg::Pattern(pattern_id, svg::Image("data:image/png;base64," + CStringUtil::base64Encode(ss.str())));

    return pattern_id;
}

string CSVGRenderer::x_LinearGradient(std::vector<CRgbaColor> const& colors)
{
    string gradient_hash;
    {
        CMD5 md5;
        for (auto const& color : colors) {
            string color_str = color.ToString();
            md5.Update(color_str.c_str(), color_str.length());
        }
        gradient_hash = md5.GetHexSum();
    }
    if (m_LinearGradients.end() != m_LinearGradients.find(gradient_hash))
        return m_LinearGradients[gradient_hash];

    string gradient_id("gradient");
    gradient_id += NStr::NumericToString(m_LinearGradients.size() + 1);
    m_LinearGradients[gradient_hash] = gradient_id;

    *m_SVG << svg::LinearGradient(gradient_id, colors);

    return gradient_id;
}

string CSVGRenderer::x_ClippingRect(GLint x, GLint y, GLsizei width, GLsizei height)
{
    string rect_hash;
    {
        CMD5 md5;
        md5.Update((const char*)&x, sizeof(x));
        md5.Update((const char*)&y, sizeof(y));
        md5.Update((const char*)&width, sizeof(width));
        md5.Update((const char*)&height, sizeof(height));
        rect_hash = md5.GetHexSum();
    }
    if (m_ClippingRects.end() != m_ClippingRects.find(rect_hash))
        return m_ClippingRects[rect_hash];

    string clipping_id("clipping-rect");
    clipping_id += NStr::NumericToString(m_ClippingRects.size() + 1);
    m_ClippingRects[rect_hash] = clipping_id;
    
    *m_SVG << svg::ClipRect(clipping_id, x, m_Height - y - 1, width, height);

    return clipping_id;
}

void CSVGRenderer::x_GetGradientColors(vector<CVect2<float>> const& vertices, vector<CRgbaColor> const &colors, vector<CRgbaColor>& gradient)
{
    if (vertices.size() > 6)
        return;
    float x = vertices[0].X();
    float y = vertices[0].Y();
    gradient.push_back(colors[0]);
    for (size_t i = 1; i < vertices.size(); ++i) {
        if ((vertices[i].X() != x) || (vertices[i].Y() == y))
            continue;
        if (colors[i] == gradient[gradient.size() - 1])
            continue;
        gradient.push_back(colors[i]);
    }
}

inline void CSVGRenderer::x_GetLineStyle(CGlVboNode &node, GLushort &pattern, GLint &factor, double &width, svg::Stroke::ELineCapStyle &lcap, svg::Stroke::ELineJoinStyle &ljoin)
{
    if (node.GetState().LineCapStyleSet()) {
        lcap = (svg::Stroke::ELineCapStyle)(int)node.GetState().GetLineCapStyle();
    }
    if (node.GetState().LineJoinStyleSet()) {
        ljoin = (svg::Stroke::ELineJoinStyle)(int)node.GetState().GetLineJoinStyle();
    }
    if (node.GetState().LineWidthSet()) {
        width = node.GetState().GetLineWidth();
    }
    if (node.GetState().IsEnabled(GL_LINE_STIPPLE) && node.GetState().LineStippleSet()) {
        node.GetState().GetLineStipple(factor, pattern);
    }
}

END_NCBI_SCOPE
