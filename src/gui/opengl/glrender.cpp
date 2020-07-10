/*  $Id: glrender.cpp 43136 2019-05-20 18:37:04Z katargir $
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
 * Authors:  Bob Falk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/opengl/glrender.hpp>
#include <gui/opengl/glresmgr.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/i3dtexture.hpp>
#include <gui/utils/matrix4.hpp>

// define this to cause OpenGL actual state to be updated immediately on state
// update calls (useful if some code is using rendermanager and some isn't, and
// state is shared between the two)
#define GL_STATE_UPDATE

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CGlRender::CGlRender() 
{
    m_ModelView.Identity();
}

void CGlRender::Clear()
{
    CRenderCommon::Clear();
    m_ModelView.Identity();
}

void CGlRender::Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    CRenderCommon::Viewport(x, y, width, height);
    glViewport(x, y, width, height);
}

void CGlRender::MatrixMode(GLenum mode)
{
    glMatrixMode(mode);
    m_MatrixMode = mode;
}

void CGlRender::PushMatrix()
{
    glPushMatrix();
}
void CGlRender::PopMatrix()
{
    glPopMatrix();
    if (m_MatrixMode == GL_MODELVIEW) {
        glGetFloatv(GL_MODELVIEW_MATRIX, m_ModelView.GetData());
        m_ModelView.Transpose();
    }
}
void CGlRender::LoadIdentity()
{
    glLoadIdentity();
    if (m_MatrixMode == GL_MODELVIEW) {
        m_ModelView.Identity();
    }
}
void CGlRender::LoadMatrixf(const GLfloat *m)
{
    glLoadMatrixf(m);
    if (m_MatrixMode == GL_MODELVIEW) {        
        glGetFloatv(GL_MODELVIEW_MATRIX, m_ModelView.GetData());
        m_ModelView.Transpose();
    }
}
void CGlRender::LoadMatrixd(const GLdouble* m)
{
    glLoadMatrixd(m);
    if (m_MatrixMode == GL_MODELVIEW) {        
        glGetFloatv(GL_MODELVIEW_MATRIX, m_ModelView.GetData());
        m_ModelView.Transpose();
    }
}
void CGlRender::MultMatrixf(const GLfloat* m)
{
    glMultMatrixf(m);
    if (m_MatrixMode == GL_MODELVIEW) {        
        glGetFloatv(GL_MODELVIEW_MATRIX, m_ModelView.GetData());
        m_ModelView.Transpose();
    }
}
void CGlRender::MultMatrixd(const GLdouble* m)
{
    glMultMatrixd(m);
    if (m_MatrixMode == GL_MODELVIEW) {        
        glGetFloatv(GL_MODELVIEW_MATRIX, m_ModelView.GetData());
        m_ModelView.Transpose();
    }
}

CMatrix4<float> CGlRender::GetProjectionMatrix() const
{
    CMatrix4<float> m;
    glGetFloatv(GL_PROJECTION_MATRIX, m.GetData());
    m.Transpose();
    return m;
}

void CGlRender::GetModelViewMatrix(GLdouble* m) const
{
    glGetDoublev(GL_MODELVIEW_MATRIX, m);
}

void CGlRender::GetProjectionMatrix(GLdouble* m) const
{
    glGetDoublev(GL_PROJECTION_MATRIX, m);
}

void CGlRender::Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    glRotated(angle, x, y, z);
    if (m_MatrixMode == GL_MODELVIEW) {        
        glGetFloatv(GL_MODELVIEW_MATRIX, m_ModelView.GetData());
        m_ModelView.Transpose();
    }
}
void CGlRender::Rotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    glRotatef(angle, x, y, z);
    if (m_MatrixMode == GL_MODELVIEW) {        
        glGetFloatv(GL_MODELVIEW_MATRIX, m_ModelView.GetData());
        m_ModelView.Transpose();
    }
}
void CGlRender::Scaled(GLdouble x, GLdouble y, GLdouble z)
{
    glScaled(x, y, z);
    if (m_MatrixMode == GL_MODELVIEW) {        
        glGetFloatv(GL_MODELVIEW_MATRIX, m_ModelView.GetData());
        m_ModelView.Transpose();
    }
}
void CGlRender::Scalef(GLfloat x, GLfloat y, GLfloat z)
{
    glScalef(x, y, z);
    if (m_MatrixMode == GL_MODELVIEW) {        
        glGetFloatv(GL_MODELVIEW_MATRIX, m_ModelView.GetData());
        m_ModelView.Transpose();
    }
}
void CGlRender::Translated(GLdouble x, GLdouble y, GLdouble z)
{
    glTranslated(x, y, z);
    if (m_MatrixMode == GL_MODELVIEW) {        
        glGetFloatv(GL_MODELVIEW_MATRIX, m_ModelView.GetData());
        m_ModelView.Transpose();
    }
}
void CGlRender::Translatef(GLfloat x, GLfloat y, GLfloat z)
{
    glTranslatef(x, y, z);
    if (m_MatrixMode == GL_MODELVIEW) {        
        glGetFloatv(GL_MODELVIEW_MATRIX, m_ModelView.GetData());
        m_ModelView.Transpose();
    }
}

void CGlRender::Ortho(GLdouble left, GLdouble right,
                      GLdouble bottom, GLdouble top,
                      GLdouble nearVal, GLdouble farVal)
{
    glOrtho(left, right, bottom, top, nearVal, farVal);
}
void CGlRender::Perspective(GLdouble fovy, GLdouble aspect, 
                       GLdouble zNear, GLdouble zFar)
{
    gluPerspective(fovy, aspect, zNear, zFar);
}

///
/// All the OpenGL state commands set both the state object AND update
/// the opengl state.  The OpenGL state call should be unnecessary, but
/// as long as the product has both rendermanager-based rendering
/// and non-rendermanager based rendering, keeping the state as it
/// was before should help prevent visual artifacts in non-rendermanager code.
///

void CGlRender::PushAttrib(GLbitfield mask)
{
    pair<GLbitfield,CGlState> s(mask, m_State.GetNCObject());

    // The smallest maximum stacksize for attributes in OpenGL is 16, so we'll
    // give a warning when that number is exceeded (but not an error)
    if (m_PushedState.size() > 16) {
        LOG_POST(Warning << "Attribute stack size exceeded in CGlRender: " << m_PushedState.size()+1 );
        return;
    }

    m_PushedState.push_back(s);
}

void CGlRender::PopAttrib()
{
    if (m_PushedState.size() == 0) {
        LOG_POST(Error << "Attribute stack underflow - no state to pop");
        return;
    }

    pair<GLbitfield,CGlState> s = m_PushedState.back();
    m_PushedState.pop_back();

    // if the pushed attribute flags were not for all attributes (GL_ALL_ATTRIB_BITS),
    // then merge the ones which were not included in the push (and which are defined
    // in CGlState).
    m_State->MergeStates(s.second, s.first);
}

void CGlRender::Enable(GLenum glstate) 
{ 
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::Enable() called between Begin() and End()");
    }

#ifdef GL_STATE_UPDATE
    glEnable(glstate);
#endif

    m_State->Enable(glstate); 
}
void CGlRender::Disable(GLenum glstate) 
{ 
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::Disable() called between Begin() and End()");
    }

#ifdef GL_STATE_UPDATE
    glDisable(glstate);
#endif

    m_State->Disable(glstate); 
}

void CGlRender::LineWidth(GLfloat w)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::LineWidth() called between Begin() and End()");
    }

#ifdef GL_STATE_UPDATE
    glLineWidth(w);
#endif

    m_State->LineWidth(w);
}
void CGlRender::PointSize(GLfloat s) 
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::PointSize() called between Begin() and End()");
    }
    
#ifdef GL_STATE_UPDATE
    glPointSize(s);
#endif

    m_State->PointSize(s);
}
void CGlRender::ShadeModel(GLenum mode)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::ShadeModel() called between Begin() and End()");
    }
    
#ifdef GL_STATE_UPDATE
    glShadeModel(mode);
#endif

    m_State->ShadeModel(mode);
}

void CGlRender::Scissor(GLint x, GLint y,
                         GLsizei width, GLsizei height)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::ShadeModel() called between Begin() and End()");
    }
    
#ifdef GL_STATE_UPDATE
    glScissor(x,y,width,height);
#endif

    m_State->Scissor(x,y,width,height);
}

void CGlRender::ColorMask(GLboolean red, 
                                 GLboolean green, 
                                 GLboolean blue, 
                                 GLboolean alpha)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::ColorMask() called between Begin() and End()");
    }
    
#ifdef GL_STATE_UPDATE
    glColorMask(red, green, blue, alpha);
#endif

    m_State->ColorMask(red, green, blue, alpha);
}
void CGlRender::PolygonMode(GLenum face, GLenum mode)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::PolygonMode() called between Begin() and End()");
    }
    
#ifdef GL_STATE_UPDATE
    glPolygonMode(face, mode);
#endif

    m_State->PolygonMode(face, mode);
}
void CGlRender::LineStipple(GLint factor, GLushort pattern)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::LineStipple() called between Begin() and End()");
    }
    
#ifdef GL_STATE_UPDATE
    glLineStipple(factor, pattern);
#endif

    m_State->LineStipple(factor, pattern);
}
void CGlRender::PolygonStipple(GLubyte* mask)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::PolygonStipple() called between Begin() and End()");
    }
    
#ifdef GL_STATE_UPDATE
    glPolygonStipple(mask);
#endif

    m_State->PolygonStipple(mask);
}
void CGlRender::BlendFunc(GLenum sfactor, GLenum dfactor)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::BlendFunc() called between Begin() and End()");
    }
    
#ifdef GL_STATE_UPDATE
    glBlendFunc(sfactor, dfactor);
#endif

    m_State->BlendFunc(sfactor, dfactor);
}

void CGlRender::TexEnvi(GLenum target, GLenum pname, GLint param)
{
    glTexEnvi(target, pname, param);
}

void CGlRender::BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB,
                                         GLenum srcAlpha, GLenum dstAlpha) 
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::BlendFuncSeparate() called between Begin() and End()");
    }
    
#ifdef GL_STATE_UPDATE
    // not 1.1
#endif

    m_State->BlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
}
void CGlRender::BlendColor(const CRgbaColor& c)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::BlendColor() called between Begin() and End()");
    }
    
#ifdef GL_STATE_UPDATE
    // Not 1.1
    //glBlendColor(c.GetRed(), c.GetGreen(), c.GetBlue(), c.GetAlpha());
#endif

    m_State->BlendColor(c);
}
void CGlRender::ScaleInvarient(bool b, CVect2<TModelUnit> scale)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::ScaleInvarient() called between Begin() and End()");
    }
    m_State->ScaleInvarient(b, scale);
}
void CGlRender::ScaleFactor(const CVect2<TModelUnit>& scale)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::ScaleFactor() called between Begin() and End()");
    } 
    m_State->ScaleFactor(scale);
}
void CGlRender::LineJoinStyle(IGlState::ELineJoinStyle s)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::LineJoinStyle() called between Begin() and End()");
    }
    m_State->LineJoinStyle(s);
}
void CGlRender::LineCapStyle(IGlState::ELineCapStyle c)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::LineCapStyle() called between Begin() and End()");
    }
    m_State->LineCapStyle(c);
}
void CGlRender::PdfShadeStyle(IGlState::EPdfShadeStyle s)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "CGlState::PdfShadeStyle() called between Begin() and End()");
    }
    m_State->PdfShadeStyle(s);
}

void CGlRender11::Hint(GLenum target, GLenum mode)
{
    glHint(target, mode);
}

void CGlRender20::UseProgram(GLuint program)
{
    glUseProgram(program);
}

void CGlRender20::Hint(GLenum target, GLenum mode)
{
    glHint(target, mode);
}















////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CGlRenderDebug::Begin(GLenum mode)
{
    glBegin(mode);
}

void CGlRenderDebug::Vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    glVertex3f(x,y,z);
}

void CGlRenderDebug::RectC(const TModelRect& rc)
{
    glRectC(rc);   
}

void CGlRenderDebug::Rectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    //glRectf(x1, y1, x2, y2);
    glBegin(GL_QUADS);
        glVertex2f(x1, y1);
        glVertex2f(x2, y1);
        glVertex2f(x2, y2);
        glVertex2f(x1, y2);
    glEnd(); 
}

void CGlRenderDebug::Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    glColor4f(r,g,b,a);
}

void CGlRenderDebug::TexCoord2f(GLfloat s, GLfloat t)
{
    glTexCoord2f(s, t);
}

void CGlRenderDebug::End()
{   
    glEnd();
}

void CGlRenderDebug::BeginText(const CGlTextureFont* font, 
                               const CRgbaColor& color)
{
    m_CurrentFont = font;
    m_TextColor = color;

    m_CurrentFont->BeginText();
}

void CGlRenderDebug::BeginText(const CGlTextureFont* font)
{
    m_CurrentFont = font;    
    m_TextColor = m_State->GetColor();

    m_CurrentFont->BeginText();

    /*
    // Bitmap fonts now may crash on linux, so remove from debug feature.
    if (font->GetFontFace() == CGlTextureFont::eFontFace_Courier) {
        m_BitmapFont.SetFontFace(CGlBitmapFont::eFontFace_Courier);
    }
    else if (font->GetFontFace() == CGlTextureFont::eFontFace_Helvetica) {
        m_BitmapFont.SetFontFace(CGlBitmapFont::eFontFace_Helvetica);
    }
    else if (font->GetFontFace() == CGlTextureFont::eFontFace_Lucida) {
        m_BitmapFont.SetFontFace(CGlBitmapFont::eFontFace_Lucida);
    }
    else if (font->GetFontFace() == CGlTextureFont::eFontFace_TimesRoman) {
        m_BitmapFont.SetFontFace(CGlBitmapFont::eFontFace_TimesRoman);
    }
    else if (font->GetFontFace() == CGlTextureFont::eFontFace_Fixed) {
        m_BitmapFont.SetFontFace(CGlBitmapFont::eFontFace_Fixed);
    }

    m_BitmapFont.SetFontSize((CGlBitmapFont::EFontSize)font->GetFontSize());
    */
}

void CGlRenderDebug::EndText()
{    
    m_CurrentFont->EndText();
    m_CurrentFont = NULL;
}

void CGlRenderDebug::WriteText(TModelUnit x, TModelUnit y, 
                               const char* text,
                               TModelUnit rotate_degrees)
{
    // bitmap fonts can crash in release mode on linux...
    // m_BitmapFont.TextOut(x, y, text);

    if (m_CurrentFont == NULL) {
        LOG_POST(Error << "Unable to write text - must call BeginText() first");
        return;
    }

    glColor4fv(m_TextColor.GetColorArray());
    m_CurrentFont->WriteText(x, y, text, rotate_degrees);
}

void CGlRenderDebug::WriteText(TModelUnit x, TModelUnit y, 
                               TModelUnit width, TModelUnit height,
                               const char* text,
                               CGlTextureFont::TAlign align,
                               CGlTextureFont::ETruncate trunc,
                               TModelUnit rotate_degrees)
{    
    //m_BitmapFont.TextOut(x, y, width, height, text, align, trunc);
    m_CurrentFont->BeginText();
    m_CurrentFont->WriteText(x, y, width, height, text, align, trunc, rotate_degrees);
    m_CurrentFont->EndText();
}


///
/// All the OpenGL state commands set both the state object AND update
/// the opengl state.  The OpenGL state call should be unnecessary, but
/// as long as the product has both rendermanager-based rendering
/// and non-rendermanager based rendering, keeping the state as it
/// was before should help prevent visual artifacts in non-rendermanager code.
///

void CGlRenderDebug::PushAttrib(GLbitfield mask)
{
    glPushAttrib(mask);
}

void CGlRenderDebug::PopAttrib() 
{
    glPopAttrib();
}

void CGlRenderDebug::Enable(GLenum glstate) 
{ 
    glEnable(glstate);
}

void CGlRenderDebug::Disable(GLenum glstate) 
{ 
    glDisable(glstate);
}

void CGlRenderDebug::Hint(GLenum target, GLenum mode)
{
    glHint(target, mode);
}

void CGlRenderDebug::LineWidth(GLfloat w)
{
    glLineWidth(w);
}

void CGlRenderDebug::PointSize(GLfloat s) 
{
    glPointSize(s);
}

void CGlRenderDebug::ShadeModel(GLenum mode)
{
    glShadeModel(mode);
}

void CGlRenderDebug::Scissor(GLint x, GLint y,
                            GLsizei width, GLsizei height)
{
   glScissor(x,y,width,height);
}

void CGlRenderDebug::ColorMask(GLboolean red, 
                                 GLboolean green, 
                                 GLboolean blue, 
                                 GLboolean alpha)
{
    glColorMask(red, green, blue, alpha);
}

void CGlRenderDebug::PolygonMode(GLenum face, GLenum mode)
{
    glPolygonMode(face, mode);
}

void CGlRenderDebug::LineStipple(GLint factor, GLushort pattern)
{
    glLineStipple(factor, pattern);
}

void CGlRenderDebug::PolygonStipple(GLubyte* mask)
{
    glPolygonStipple(mask);
}

void CGlRenderDebug::BlendFunc(GLenum sfactor, GLenum dfactor)
{
    glBlendFunc(sfactor, dfactor);
}

void CGlRenderDebug::BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB,
                                         GLenum srcAlpha, GLenum dstAlpha) 
{
}

void CGlRenderDebug::BlendColor(const CRgbaColor& c)
{
}

void CGlRenderDebug::ScaleInvarient(bool b, CVect2<TModelUnit> scale)
{
}

void CGlRenderDebug::ScaleFactor(const CVect2<TModelUnit>& scale)
{
}

void CGlRenderDebug::LineJoinStyle(IGlState::ELineJoinStyle s)
{
}

void CGlRenderDebug::LineCapStyle(IGlState::ELineCapStyle c)
{
}

void CGlRenderDebug::PdfShadeStyle(IGlState::EPdfShadeStyle s)
{
}


END_NCBI_SCOPE
