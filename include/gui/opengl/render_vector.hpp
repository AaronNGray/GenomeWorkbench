#ifndef GUI_OPENGL___GL_RENDER_VECTOR__HPP
#define GUI_OPENGL___GL_RENDER_VECTOR__HPP

/*  $Id: render_vector.hpp 43853 2019-09-10 17:42:16Z evgeniev $
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
 * File Description: Common code of vector graphics renderers
 *
 */

#include <gui/opengl.h>
#include <gui/gui.hpp>

#include <gui/opengl/render_common.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOPENGL_EXPORT CRenderVector : public CRenderCommon
{
public:
    CRenderVector();

    virtual void PushAttrib(GLbitfield mask);
    virtual void PopAttrib();

    virtual void Enable(GLenum glstate);
    virtual void Disable(GLenum glstate);

    virtual void LineWidth(GLfloat w);
    virtual void PointSize(GLfloat s);

    virtual void ShadeModel(GLenum mode);
    virtual void Scissor(GLint x, GLint y,
        GLsizei width, GLsizei height);
    /// Color{3,4}{f,d}{v} commands are defined above....
    virtual void ColorMask(GLboolean red,
        GLboolean green,
        GLboolean blue,
        GLboolean alpha);

    virtual void PolygonMode(GLenum face, GLenum mode);
    virtual void LineStipple(GLint factor, GLushort pattern);
    virtual void PolygonStipple(GLubyte* mask);
    virtual void BlendFunc(GLenum sfactor, GLenum dfactor);
    virtual void TexEnvi(GLenum target, GLenum pname, GLint param);
    virtual void BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB,
        GLenum srcAlpha, GLenum dstAlpha);

    virtual void LineJoinStyle(IGlState::ELineJoinStyle s);
    virtual void LineCapStyle(IGlState::ELineCapStyle c);
    virtual void PdfShadeStyle(IGlState::EPdfShadeStyle s);

    virtual void UseProgram(GLuint /*program*/) {}

    virtual void MatrixMode(GLenum mode);

    virtual void PushMatrix();
    virtual void PopMatrix();
    virtual void LoadIdentity();
    virtual void LoadMatrixf(const GLfloat *m);
    virtual void LoadMatrixd(const GLdouble* m);

    virtual CMatrix4<float> GetModelViewMatrix() const;
    virtual CMatrix4<float> GetProjectionMatrix() const;
    virtual void GetModelViewMatrix(GLdouble* m) const;
    virtual void GetProjectionMatrix(GLdouble* m) const;

    virtual void Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
    virtual void Rotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
    virtual void Scaled(GLdouble x, GLdouble y, GLdouble z);
    virtual void Scalef(GLfloat x, GLfloat y, GLfloat z);
    virtual void Translated(GLdouble x, GLdouble y, GLdouble z);
    virtual void Translatef(GLfloat x, GLfloat y, GLfloat z);

    virtual void Ortho(GLdouble left, GLdouble right,
        GLdouble bottom, GLdouble top,
        GLdouble nearVal, GLdouble farVal);

    /// Get font metrics - metrics differ when the font under opengl
    /// has been replaced by a bitmap font for (screen) readability. Other than
    /// that, it would be fine to use the glTextureFont metric function directly
    virtual TModelUnit GetMetric(const CGlTextureFont* font,
        IGlFont::EMetric metric,
        const char* text = NULL,
        int len = -1) const;
    virtual TModelUnit TextWidth(const CGlTextureFont* font, const char* text) const;
    virtual TModelUnit TextHeight(const CGlTextureFont* font) const;
    virtual TModelUnit GetFontDescender(const CGlTextureFont* font) const;
    virtual TModelUnit GetAdvance(const CGlTextureFont* font, char c) const;
    virtual TModelUnit GetMaxWidth(const CGlTextureFont* font, int max_num) const;

private:
    struct Matrix
    {
        double m[16];
    };

    void x_LoadMatrix(const Matrix& m);
    void x_ApplyTransf(const Matrix& t);

    static void x_MultMatrix(Matrix& d, const Matrix& s1, const Matrix& s2);

    static void x_InitMatrixd(Matrix& d, const GLdouble* s);
    static void x_InitMatrixf(Matrix& d, const GLfloat* s);
    static void x_FromMatrix(const Matrix& d, GLdouble* res);
    static CMatrix4<float> x_FromMatrix(const Matrix& d);


    list<Matrix> m_ProjectionMatrix;
    list<Matrix> m_ModelViewMatrix;

    vector<pair<GLbitfield, CGlState> > m_PushedState;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_RENDER_VECTOR__HPP
