/*  $Id: glrenderpdf.cpp 45024 2020-05-09 02:03:16Z evgeniev $
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
#include <gui/print/glrenderpdf.hpp>
#include <gui/opengl/glutils.hpp>

#include <gui/utils/matrix4.hpp>
#include <gui/utils/matrix3.hpp>

#include <gui/print/pdf.hpp>
#include "page_buffers.hpp"

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool CGlRenderPdf::IsSimplified() const
{
    if (!m_Pdf)
        return false;

    return m_Pdf->GetOptions().GetGouraudShadingDisabled();
}

void CGlRenderPdf::SetIsGreyscale(bool b)
{
    if (!m_Pdf)
        return;

    m_Pdf->SetIsGreyscale(b);
}

void CGlRenderPdf::Render(CGlPane& pane, CGlModel2D* model)
{
    if (model != NULL) {       
        model->RenderPDF(pane, m_Pdf);        
    }
}

void CGlRenderPdf::Vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    if (m_CurrentMode == GL_NONE) {
        LOG_POST(Error << "Vertex*() called without first calling CGlRender::Begin");
        return;
    }

    m_VertexBuffer.push_back(CVect3<float>(x,y,z) );

    // In pdf mode, break quads into triangles if smooth shading is indicated
    // in opengl, OR if smooth shading is requested for pdf (the default).  
    // In PDF, smooth shading also lets polygon vertices merge, which they don't
    // for the flat shading model
    if ((m_State->GetShadeModel() == GL_SMOOTH || 
         m_State->GetPdfShadeStyle() == CGlState::eGouraud) &&
         m_State->GetPolygonMode() != GL_LINE) {

            // Break after 4 new vertices have been added (ignore groups of 6 since
            // they are the previous triangles)
            if (m_CurrentMode == GL_QUADS && m_VertexBuffer.size() % 6 == 4) {
                size_t v1 = m_VertexBuffer.size()-4;
                size_t v3 = v1 + 2;

                if (m_ColorBuffer.size() > 0) {
                    x_SyncAttribs(m_ColorBuffer);
                    m_ColorBuffer.push_back(m_ColorBuffer[v1]);
                    m_ColorBuffer.push_back(m_ColorBuffer[v3]);
                }

                if (m_TexCoord2DBuffer1.size() > 0) {
                    x_SyncAttribs(m_TexCoord2DBuffer1);
                    m_TexCoord2DBuffer1.push_back(m_TexCoord2DBuffer1[v1]);
                    m_TexCoord2DBuffer1.push_back(m_TexCoord2DBuffer1[v3]);
                }

                // Do vertices last since x_SyncAttribs syncs size of attribute
                // buffers to current size of this (m_VertexBuffer) buffer.
                m_VertexBuffer.push_back(m_VertexBuffer[v1]);
                m_VertexBuffer.push_back(m_VertexBuffer[v3]);
            }
    }
}

void CGlRenderPdf::x_RenderBuffer(CGlVboNode* node)
{
    // m_Viewport is x,y,w,h and TVPRect is l,b,r,t
    TVPRect viewport(m_Viewport[0], 
                     m_Viewport[1], 
                     m_Viewport[0] + m_Viewport[2], 
                     m_Viewport[1] + m_Viewport[3]);
    m_Pdf->PrintBuffer(node, viewport);
}
 
void CGlRenderPdf::BeginText(const CGlTextureFont* font, 
                             const CRgbaColor& color)
{
    if (m_CurrentFont != NULL) {
        LOG_POST("BeginText() called without first calling CGlRenderPdf::EndText()");
        return;
    }

    m_CurrentFont = font;
    m_TextColor = color;

    CRef<CPdfObject> content = m_Pdf->BeginContent(CPdf::ePdfText); 
    
    content->PushGraphicsState();
    content->SetClipBox(m_Viewport[0], m_Viewport[1], m_Viewport[2], m_Viewport[3]);
}
void CGlRenderPdf::BeginText(const CGlTextureFont* font)
{
    if (m_CurrentFont != NULL) {
        LOG_POST("BeginText() called without first calling CGlRenderPdf::EndText()");
        return;
    }

    m_CurrentFont = font;
    m_TextColor = m_RenderNode.GetState().GetColor();

    CRef<CPdfObject> content = m_Pdf->BeginContent(CPdf::ePdfText); 

    content->PushGraphicsState();
    content->SetClipBox(m_Viewport[0], m_Viewport[1], m_Viewport[2], m_Viewport[3]);
}

void CGlRenderPdf::EndText()
{
    if (m_CurrentFont == NULL) {
        LOG_POST("EndText() called without first calling CGlRenderPdf::BeginText()");
        return;
    }

    m_CurrentFont = NULL;
    m_Pdf->GetCurrentContent()->PopGraphicsState();
    m_Pdf->EndContent();
}

void CGlRenderPdf::WriteText(TModelUnit x, TModelUnit y, 
                             const char* text,
                             TModelUnit rotate_degrees)
{
    if (m_CurrentFont == NULL) {
        LOG_POST("Unable to write text - must call BeginText() first");
        return;
    }
 
    CVect2<float> posp;                    
    GLdouble px, py;
    GLdouble dummyz;

    GLdouble modelview_matrix[16];
    GLdouble projection_matrix[16];

    GetModelViewMatrix(modelview_matrix);
    GetProjectionMatrix(projection_matrix);

    // Use viewport from the rendermanager object since OpenGL viewport maximum
    // size is below the max viewable area for a pdf page (and so
    // the OpenGL value will in that case be truncated).
    gluProject(GLdouble(x), GLdouble(y), GLdouble(0.0), 
        modelview_matrix, projection_matrix, m_Viewport, 
        &px, &py, &dummyz);
    posp.X() = float(px);
    posp.Y() = float(py);

    CGlTextureFont::EFontFace face = m_CurrentFont->GetFontFace();
    unsigned int font_size = m_CurrentFont->GetFontSize();

    m_Pdf->PrintText(m_Pdf->GetFontHandler(), face, float(font_size), posp, text, m_TextColor);
}

void CGlRenderPdf::WriteText(TModelUnit x, TModelUnit y, 
                             TModelUnit width, TModelUnit height,
                             const char* text,
                             CGlTextureFont::TAlign align,
                             CGlTextureFont::ETruncate trunc,
                             TModelUnit rotate_degrees)
{
    if (m_CurrentFont == NULL) {
        LOG_POST("Unable to write text - must call BeginText() first");
        return;
    }

    CGlTextureFont::EFontFace face = m_CurrentFont->GetFontFace();
    unsigned int font_size = m_CurrentFont->GetFontSize();
   
    m_Pdf->GetCurrentContent()->SetFillColor(m_TextColor);    
    string truncated_text = text;
    CMatrix4<double> mat;
    
    // Get viewport from the rendermanager object since OpenGL viewport maximum
    // size is below the max viewable area for a pdf page (and so
    // the OpenGL value will in that case be truncated).
    TVPRect viewport(m_Viewport[0], m_Viewport[1], m_Viewport[2], m_Viewport[3]);  
    m_CurrentFont->BeginText(viewport);
    // Get transformation matrix needed for text at the desired location and rotation
    mat = m_CurrentFont->GetTextXform(x, y, 
                                      width, height, 
                                      truncated_text, align, trunc, rotate_degrees);
    m_CurrentFont->EndText();

    // returned mat is 4x4 row-order matrix.  Create a 3x3 row matrix (homgeneous x,y
    // coordinates) for transforming text
    CMatrix3<double> mat33(mat(0,0), mat(0,1), mat(0,3),
                           mat(1,0), mat(1,1), mat(1,3),
                           mat(2,0), mat(2,1), mat(2,3));

    m_Pdf->PrintText(m_Pdf->GetFontHandler(), face, float(font_size), mat33, truncated_text.c_str(), m_TextColor);  
}

void CGlRenderPdf::Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    CRenderCommon::Viewport(x, y, width, height);
}

void CGlRenderPdf::GetScreenSize(int& width, int& height) const
{
    width = (int)m_Pdf->GetOptions().GetMedia().GetWidth();
    height = (int)m_Pdf->GetOptions().GetMedia().GetHeight();
}

END_NCBI_SCOPE
