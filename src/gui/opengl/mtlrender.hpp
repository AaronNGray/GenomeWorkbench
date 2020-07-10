#ifndef GUI_OPENGL___MTL_RENDER__HPP
#define GUI_OPENGL___MTL_RENDER__HPP

/*  $Id: mtlrender.hpp 43551 2019-07-30 18:05:55Z katargir $
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

#include <gui/opengl.h>
#include <gui/gui.hpp>

#include <simd/simd.h>

#include <gui/opengl/render_common.hpp>


/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class CMtlRender : public CObject, public CRenderCommon
{
public:
    CMtlRender();
    virtual ~CMtlRender();

    /// Overridden in subclasses to return the supported rendering target
    virtual ERenderTarget GetApi() { return eMetal; }

    /// For shaders. Only works with OpenGL 2.0+
    virtual void UseProgram(GLuint program) {}

    /// Finish rendering (create buffer and send to renderer)
    virtual void End();

    /// Vertices
    virtual void Vertex3f(GLfloat x, GLfloat y, GLfloat z);

    /// @name Viewports, matrices and transformations
    /// @{
    virtual void Viewport(GLint x, GLint y, GLsizei width, GLsizei height);
    
    virtual void MatrixMode(GLenum mode);
    
    virtual void PushMatrix();
    virtual void PopMatrix();
    virtual void LoadIdentity();
    virtual void LoadMatrixf(const GLfloat *m);
    virtual void LoadMatrixd(const GLdouble* m);
    virtual void MultMatrixf(const GLfloat* m);
    virtual void MultMatrixd(const GLdouble* m);
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
    virtual void Perspective(GLdouble fovy, GLdouble aspect, 
                       GLdouble zNear, GLdouble zFar);
    /// @}
   
    /// @name IGlState interface implementation
    /// @{

    /// 
    /// These commands just forward their parameters to the render managers
    /// CGlState object m_State.  State can also be updated (replaced) via
    /// Begin(GLenum mode, const CGlState& state);
    ///
    virtual void PushAttrib(GLbitfield mask);
    virtual void PopAttrib();

    virtual void Enable(GLenum glstate);
    virtual void Disable(GLenum glstate);
    virtual void Hint(GLenum target, GLenum mode);
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
    virtual void BlendColor(const CRgbaColor& c);
    virtual void ScaleInvarient(bool b, CVect2<TModelUnit> scale = 
        CVect2<TModelUnit>(TModelUnit(1), TModelUnit(1)));
    virtual void ScaleFactor(const CVect2<TModelUnit>& scale);
    virtual void LineJoinStyle(IGlState::ELineJoinStyle s);
    virtual void LineCapStyle(IGlState::ELineCapStyle c);
    virtual void PdfShadeStyle(IGlState::EPdfShadeStyle s);
    /// @}

    /// Allow caller to access state directly
    virtual CRef<CGlState> GetState() { return m_State; }

private:
    void x_LoadMatrix(const simd::double4x4& m);
    void x_ApplyTransf(const simd::double4x4& t);
    void x_GetScreenMatrices(simd::float4x4& projection, simd::float4x4& modelView);
    
    void x_RenderBuffers();
    void x_RenderStippledPolygon();

    /// color and alpha for text only (other GL options do not apply to text)
    CRgbaColor m_TextColor;

    list<simd::double4x4> m_ProjectionMatrix;
    list<simd::double4x4> m_ModelViewMatrix;

    GLubyte m_PolygonStipple[128];
    bool m_PolygonStippleEnabled = false;

    int  m_ScissorRectX = 0;
    int  m_ScissorRectY = 0;
    int  m_ScissorRectW = 1000;
    int  m_ScissorRectH = 1000;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___MTL_RENDER__HPP
