/*  $Id: render_vector.cpp 43853 2019-09-10 17:42:16Z evgeniev $
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

#include <ncbi_pch.hpp>

#include <gui/opengl/render_vector.hpp>

BEGIN_NCBI_SCOPE

static void s_MakeIdentity(double m[16])
{
    m[0] = m[5] = m[10] = m[15] = 1;
    m[1] = m[2] = m[3] = m[4] = 0;
    m[6] = m[7] = m[8] = m[9] = 0;
    m[11] = m[12] = m[13] = m[14] = 0;
}

CRenderVector::CRenderVector()
{
    Matrix m;
    s_MakeIdentity(m.m);
    m_ProjectionMatrix.push_front(m);
    m_ModelViewMatrix.push_front(m);
}

void CRenderVector::MatrixMode(GLenum mode)
{
    m_MatrixMode = mode;
}

void CRenderVector::PushMatrix()
{
    switch (m_MatrixMode) {
    case GL_PROJECTION:
        m_ProjectionMatrix.push_front(m_ProjectionMatrix.front());
        break;
    case GL_MODELVIEW:
        m_ModelViewMatrix.push_front(m_ModelViewMatrix.front());
        break;
    }
}

void CRenderVector::PopMatrix()
{
    switch (m_MatrixMode) {
    case GL_PROJECTION:
        if (m_ProjectionMatrix.size() > 1)
            m_ProjectionMatrix.pop_front();
        break;
    case GL_MODELVIEW:
        if (m_ModelViewMatrix.size() > 1)
            m_ModelViewMatrix.pop_front();
        break;
    }
}

void CRenderVector::LoadIdentity()
{
    Matrix m;
    s_MakeIdentity(m.m);
    x_LoadMatrix(m);
}

void CRenderVector::x_LoadMatrix(const Matrix& m)
{
    switch (m_MatrixMode) {
    case GL_PROJECTION:
        m_ProjectionMatrix.front() = m;
        break;
    case GL_MODELVIEW:
        m_ModelViewMatrix.front() = m;
        break;
    }
}

void CRenderVector::x_ApplyTransf(const Matrix& t)
{
    Matrix m;
    switch (m_MatrixMode) {
    case GL_PROJECTION:
        m = m_ProjectionMatrix.front();
        x_MultMatrix(m_ProjectionMatrix.front(), m, t);
        break;
    case GL_MODELVIEW:
        m = m_ModelViewMatrix.front();
        x_MultMatrix(m_ModelViewMatrix.front(), m, t);
        break;
    }
}

void CRenderVector::x_InitMatrixd(Matrix& d, const GLdouble* s)
{
    d.m[0] = s[0];
    d.m[1] = s[1];
    d.m[2] = s[2];
    d.m[3] = s[3];
    d.m[4] = s[4];
    d.m[5] = s[5];
    d.m[6] = s[6];
    d.m[7] = s[7];
    d.m[8] = s[8];
    d.m[9] = s[9];
    d.m[10] = s[10];
    d.m[11] = s[11];
    d.m[12] = s[12];
    d.m[13] = s[13];
    d.m[14] = s[14];
    d.m[15] = s[15];
}

void CRenderVector::x_InitMatrixf(Matrix& d, const GLfloat* s)
{
    d.m[0] = s[0];
    d.m[1] = s[1];
    d.m[2] = s[2];
    d.m[3] = s[3];
    d.m[4] = s[4];
    d.m[5] = s[5];
    d.m[6] = s[6];
    d.m[7] = s[7];
    d.m[8] = s[8];
    d.m[9] = s[9];
    d.m[10] = s[10];
    d.m[11] = s[11];
    d.m[12] = s[12];
    d.m[13] = s[13];
    d.m[14] = s[14];
    d.m[15] = s[15];
}

CMatrix4<float> CRenderVector::x_FromMatrix(const Matrix& d)
{
    return CMatrix4<float>(
        (float)d.m[0], (float)d.m[4], (float)d.m[8], (float)d.m[12],
        (float)d.m[1], (float)d.m[5], (float)d.m[9], (float)d.m[13],
        (float)d.m[2], (float)d.m[6], (float)d.m[10], (float)d.m[14],
        (float)d.m[3], (float)d.m[7], (float)d.m[11], (float)d.m[15]);
}

void CRenderVector::x_FromMatrix(const Matrix& d, GLdouble* res)
{
    res[0] = d.m[0];
    res[1] = d.m[1];
    res[2] = d.m[2];
    res[3] = d.m[3];
    res[4] = d.m[4];
    res[5] = d.m[5];
    res[6] = d.m[6];
    res[7] = d.m[7];
    res[8] = d.m[8];
    res[9] = d.m[9];
    res[10] = d.m[10];
    res[11] = d.m[11];
    res[12] = d.m[12];
    res[13] = d.m[13];
    res[14] = d.m[14];
    res[15] = d.m[15];
}

void CRenderVector::x_MultMatrix(Matrix& d, const Matrix& s1, const Matrix& s2)
{
    d.m[0] = s1.m[0] * s2.m[0] + s1.m[4] * s2.m[1] + s1.m[8] * s2.m[2] + s1.m[12] * s2.m[3];
    d.m[1] = s1.m[1] * s2.m[0] + s1.m[5] * s2.m[1] + s1.m[9] * s2.m[2] + s1.m[13] * s2.m[3];
    d.m[2] = s1.m[2] * s2.m[0] + s1.m[6] * s2.m[1] + s1.m[10] * s2.m[2] + s1.m[14] * s2.m[3];
    d.m[3] = s1.m[3] * s2.m[0] + s1.m[7] * s2.m[1] + s1.m[11] * s2.m[2] + s1.m[15] * s2.m[3];

    d.m[4] = s1.m[0] * s2.m[4] + s1.m[4] * s2.m[5] + s1.m[8] * s2.m[6] + s1.m[12] * s2.m[7];
    d.m[5] = s1.m[1] * s2.m[4] + s1.m[5] * s2.m[5] + s1.m[9] * s2.m[6] + s1.m[13] * s2.m[7];
    d.m[6] = s1.m[2] * s2.m[4] + s1.m[6] * s2.m[5] + s1.m[10] * s2.m[6] + s1.m[14] * s2.m[7];
    d.m[7] = s1.m[3] * s2.m[4] + s1.m[7] * s2.m[5] + s1.m[11] * s2.m[6] + s1.m[15] * s2.m[7];

    d.m[8] = s1.m[0] * s2.m[8] + s1.m[4] * s2.m[9] + s1.m[8] * s2.m[10] + s1.m[12] * s2.m[11];
    d.m[9] = s1.m[1] * s2.m[8] + s1.m[5] * s2.m[9] + s1.m[9] * s2.m[10] + s1.m[13] * s2.m[11];
    d.m[10] = s1.m[2] * s2.m[8] + s1.m[6] * s2.m[9] + s1.m[10] * s2.m[10] + s1.m[14] * s2.m[11];
    d.m[11] = s1.m[3] * s2.m[8] + s1.m[7] * s2.m[9] + s1.m[11] * s2.m[10] + s1.m[15] * s2.m[11];

    d.m[12] = s1.m[0] * s2.m[12] + s1.m[4] * s2.m[13] + s1.m[8] * s2.m[14] + s1.m[12] * s2.m[15];
    d.m[13] = s1.m[1] * s2.m[12] + s1.m[5] * s2.m[13] + s1.m[9] * s2.m[14] + s1.m[13] * s2.m[15];
    d.m[14] = s1.m[2] * s2.m[12] + s1.m[6] * s2.m[13] + s1.m[10] * s2.m[14] + s1.m[14] * s2.m[15];
    d.m[15] = s1.m[3] * s2.m[12] + s1.m[7] * s2.m[13] + s1.m[11] * s2.m[14] + s1.m[15] * s2.m[15];
}

void CRenderVector::LoadMatrixf(const GLfloat *m)
{
    Matrix t;
    x_InitMatrixf(t, m);
    x_LoadMatrix(t);
}

void CRenderVector::LoadMatrixd(const GLdouble* m)
{
    Matrix t;
    x_InitMatrixd(t, m);
    x_LoadMatrix(t);
}

CMatrix4<float> CRenderVector::GetModelViewMatrix() const
{
    return x_FromMatrix(m_ModelViewMatrix.front());
}

CMatrix4<float> CRenderVector::GetProjectionMatrix() const
{
    return x_FromMatrix(m_ProjectionMatrix.front());
}

void CRenderVector::GetModelViewMatrix(GLdouble* m) const
{
    x_FromMatrix(m_ModelViewMatrix.front(), m);
}

void CRenderVector::GetProjectionMatrix(GLdouble* m) const
{
    x_FromMatrix(m_ProjectionMatrix.front(), m);
}

void CRenderVector::Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    double mod = sqrt(x*x + y * y + z * z);
    if (mod == 0)
        return;
    x /= mod; y /= mod; z /= mod;
    double r = angle * 3.141592653589793238463 / 360.0;
    double c = cos(r), s = sin(r);

    double v[4] = { s*x, s*y, s*z, c };
    double d[16]
    {
    1 - 2 * (v[1] * v[1] + v[2] * v[2]),
        2 * (v[0] * v[1] + v[2] * v[3]),
        2 * (v[0] * v[2] - v[1] * v[3]), 0,

        2 * (v[0] * v[1] - v[2] * v[3]),
    1 - 2 * (v[2] * v[2] + v[0] * v[0]),
        2 * (v[1] * v[2] + v[0] * v[3]), 0,

        2 * (v[2] * v[0] + v[1] * v[3]),
        2 * (v[1] * v[2] - v[0] * v[3]),
    1 - 2 * (v[1] * v[1] + v[0] * v[0]), 0,

    0, 0, 0, 1
    };

    Matrix m;
    x_InitMatrixd(m, d);
    x_ApplyTransf(m);
}

void CRenderVector::Rotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    Rotated(angle, x, y, z);
}

void CRenderVector::Scaled(GLdouble x, GLdouble y, GLdouble z)
{
    Matrix m;
    s_MakeIdentity(m.m);

    m.m[0] = x;
    m.m[5] = y;
    m.m[10] = z;

    x_ApplyTransf(m);
}

void CRenderVector::Scalef(GLfloat x, GLfloat y, GLfloat z)
{
    Scaled(x, y, z);
}

void CRenderVector::Translated(GLdouble x, GLdouble y, GLdouble z)
{
    Matrix m;
    s_MakeIdentity(m.m);

    m.m[12] = x;
    m.m[13] = y;
    m.m[14] = z;

    x_ApplyTransf(m);
}

void CRenderVector::Translatef(GLfloat x, GLfloat y, GLfloat z)
{
    Translated(x, y, z);
}

void CRenderVector::Ortho(GLdouble left, GLdouble right,
    GLdouble bottom, GLdouble top,
    GLdouble nearVal, GLdouble farVal)
{
    Matrix m;
    s_MakeIdentity(m.m);

    m.m[0] = 2.0 / (right - left);
    m.m[5] = 2.0 / (top - bottom);
    m.m[10] = -2.0 / (farVal - nearVal);

    m.m[12] = -(right + left) / (right - left);
    m.m[13] = -(top + bottom) / (top - bottom);
    m.m[14] = -(farVal + nearVal) / (farVal - nearVal);

    x_ApplyTransf(m);
}

TModelUnit CRenderVector::GetMetric(const CGlTextureFont* font,
    IGlFont::EMetric metric,
    const char* text,
    int len) const
{
    // special case if the font is the basis for pdf rendering - compute metrics based 
    // on corresponding, non-bitmap font
    if (font->IsBitmapFont()) {
        // Get the base file name (face name will be the correspnoding texture font face, not
        // the bitmap face)
        string face = CGlTextureFont::GetFontFileForFace(font->GetFontFace());

        // Create a new texture font
        CGlTextureFont non_bitmap_font(face, font->GetFontSize());

        // And use that to get our metrics.  Alternatively, we could create/find a library that returns
        // true pdf font metrics.
        return non_bitmap_font.GetMetric(metric, text, len);
    }
    else {
        return font->GetMetric(metric, text, len);
    }
}

TModelUnit CRenderVector::TextWidth(const CGlTextureFont* font, const char* text) const
{
    if (font->IsBitmapFont()) {
        string face = CGlTextureFont::GetFontFileForFace(font->GetFontFace());

        // Create a new texture font
        CGlTextureFont non_bitmap_font(face, font->GetFontSize());

        return non_bitmap_font.TextWidth(text);
    }
    else {
        return font->TextWidth(text);
    }
}

TModelUnit CRenderVector::TextHeight(const CGlTextureFont* font) const
{
    if (font->IsBitmapFont()) {
        string face = CGlTextureFont::GetFontFileForFace(font->GetFontFace());

        // Create a new texture font
        CGlTextureFont non_bitmap_font(face, font->GetFontSize());

        return non_bitmap_font.TextHeight();
    }
    else {
        return font->TextHeight();
    }
}

TModelUnit CRenderVector::GetFontDescender(const CGlTextureFont* font) const
{
    if (font->IsBitmapFont()) {
        string face = CGlTextureFont::GetFontFileForFace(font->GetFontFace());

        // Create a new texture font
        CGlTextureFont non_bitmap_font(face, font->GetFontSize());

        return non_bitmap_font.GetFontDescender();
    }
    else {
        return font->GetFontDescender();
    }
}

TModelUnit CRenderVector::GetAdvance(const CGlTextureFont* font, char c) const
{
    if (font->IsBitmapFont()) {
        string face = CGlTextureFont::GetFontFileForFace(font->GetFontFace());

        // Create a new texture font
        CGlTextureFont non_bitmap_font(face, font->GetFontSize());

        return non_bitmap_font.GetAdvance(c);
    }
    else {
        return font->GetAdvance(c);
    }
}


TModelUnit CRenderVector::GetMaxWidth(const CGlTextureFont* font, int max_num) const
{
    if (font->IsBitmapFont()) {
        string face = CGlTextureFont::GetFontFileForFace(font->GetFontFace());

        // Create a new texture font
        CGlTextureFont non_bitmap_font(face, font->GetFontSize());

        return non_bitmap_font.GetMaxWidth(max_num);
    }
    else {
        return font->GetMaxWidth(max_num);
    }
}

void CRenderVector::PushAttrib(GLbitfield mask)
{
    pair<GLbitfield, CGlState> s(mask, m_State.GetNCObject());

    // The smallest maximum stacksize for attributes in OpenGL is 16, so we'll
    // give a warning when that number is exceeded (but not an error)
    if (m_PushedState.size() > 16) {
        LOG_POST(Warning << "Attribute stack size exceeded in CGlRender: " << m_PushedState.size() + 1);
        return;
    }

    m_PushedState.push_back(s);
}

void CRenderVector::PopAttrib()
{
    if (m_PushedState.size() == 0) {
        LOG_POST(Error << "Attribute stack underflow - no state to pop");
        return;
    }

    pair<GLbitfield, CGlState> s = m_PushedState.back();
    m_PushedState.pop_back();

    // if the pushed attribute flags were not for all attributes (GL_ALL_ATTRIB_BITS),
    // then merge the ones which were not included in the push (and which are defined
    // in CGlState).
    m_State->MergeStates(s.second, s.first);
}

void CRenderVector::Enable(GLenum glstate)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::Enable() called between Begin() and End()");
    }

    m_State->Enable(glstate);
}

void CRenderVector::Disable(GLenum glstate)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::Disable() called between Begin() and End()");
    }

    m_State->Disable(glstate);
}

void CRenderVector::LineWidth(GLfloat w)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::LineWidth() called between Begin() and End()");
    }

    m_State->LineWidth(w);
}
void CRenderVector::PointSize(GLfloat s)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::PointSize() called between Begin() and End()");
    }

    m_State->PointSize(s);
}

void CRenderVector::ShadeModel(GLenum mode)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::ShadeModel() called between Begin() and End()");
    }

    m_State->ShadeModel(mode);
}

void CRenderVector::Scissor(GLint x, GLint y,
    GLsizei width, GLsizei height)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::ShadeModel() called between Begin() and End()");
    }

    m_State->Scissor(x, y, width, height);
}

void CRenderVector::ColorMask(GLboolean red,
    GLboolean green,
    GLboolean blue,
    GLboolean alpha)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::ColorMask() called between Begin() and End()");
    }

    m_State->ColorMask(red, green, blue, alpha);
}

void CRenderVector::PolygonMode(GLenum face, GLenum mode)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::PolygonMode() called between Begin() and End()");
    }

    m_State->PolygonMode(face, mode);
}
void CRenderVector::LineStipple(GLint factor, GLushort pattern)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::LineStipple() called between Begin() and End()");
    }

    m_State->LineStipple(factor, pattern);
}
void CRenderVector::PolygonStipple(GLubyte* mask)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::PolygonStipple() called between Begin() and End()");
    }

    m_State->PolygonStipple(mask);
}

void CRenderVector::BlendFunc(GLenum sfactor, GLenum dfactor)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::BlendFunc() called between Begin() and End()");
    }

    m_State->BlendFunc(sfactor, dfactor);
}

void CRenderVector::TexEnvi(GLenum target, GLenum pname, GLint param)
{
}

void CRenderVector::BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB,
    GLenum srcAlpha, GLenum dstAlpha)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::BlendFuncSeparate() called between Begin() and End()");
    }

    m_State->BlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
}

void CRenderVector::LineJoinStyle(IGlState::ELineJoinStyle s)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::LineJoinStyle() called between Begin() and End()");
    }
    m_State->LineJoinStyle(s);
}

void CRenderVector::LineCapStyle(IGlState::ELineCapStyle c)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::LineCapStyle() called between Begin() and End()");
    }
    m_State->LineCapStyle(c);
}

void CRenderVector::PdfShadeStyle(IGlState::EPdfShadeStyle s)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::PdfShadeStyle() called between Begin() and End()");
    }
    m_State->PdfShadeStyle(s);
}

END_NCBI_SCOPE
