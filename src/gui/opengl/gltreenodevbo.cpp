/*  $Id: gltreenodevbo.cpp 43287 2019-06-07 15:23:59Z katargir $
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

#include "gltreenodevbo.hpp"

BEGIN_NCBI_SCOPE

///
/// CGlTreeNodeVbo20
///

CGlTreeNodeVbo20::CGlTreeNodeVbo20(size_t numSegments) : CTreeNodeVboBase(numSegments)
{
    if (numSegments < 4)
        return;

    glGenBuffers(1, &m_VertexBuffer);
    if (m_VertexBuffer == GL_ZERO)
        return;

    glGenBuffers(1, &m_SecondaryBuffer);
    if (m_SecondaryBuffer == GL_ZERO)
        return;

    vector <CVect2<float> > vertices;
    GetVertexBuffer2D(vertices);

    m_ColorBuffer.resize(vertices.size(), CRgbaColor(0, 0, 0, 0));
    m_ColorBuffer[0].SetAlpha(1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(vertices[0]), &vertices[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_SecondaryBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_ColorBuffer.size()*sizeof(m_ColorBuffer[0]), &m_ColorBuffer[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, GL_ZERO);

    string vertex_shader_gs = "#version 120\n"
                              "varying vec4 frag_color;\n"
                              "uniform vec4 color;\n"
                              "void main(void)\n"
                              "{\n"
                                  "frag_color = mix(color, vec4(1,1,1,color.a), gl_Color.a);\n"
                                  "gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
                              "}";

    string fragment_shader = "#version 120\n"
                             "varying vec4 frag_color\n;"
                             "void main(void)\n"
                             "{\n"
                                 "gl_FragColor = frag_color;\n"
                             "}";
    // standard node look - highlight in center, solid around edge
    m_NodeShader.CreateShader(vertex_shader_gs, fragment_shader);
    m_ShaderColorLoc = m_NodeShader.GetUniformLocation("color");
}

CGlTreeNodeVbo20::~CGlTreeNodeVbo20()
{
    if (m_VertexBuffer != GL_ZERO) {
        glDeleteBuffers(1, &m_VertexBuffer);
        _ASSERT(GL_NO_ERROR == glGetError());
    }

    if (m_SecondaryBuffer != GL_ZERO) {
        glDeleteBuffers(1, &m_SecondaryBuffer);
        _ASSERT(GL_NO_ERROR == glGetError());
    }
}

void CGlTreeNodeVbo20::Render(const float* modelView)
{
    if (m_VertexBuffer == GL_ZERO || m_SecondaryBuffer == GL_ZERO)
        return;

    m_NodeShader.MakeCurrent();

    float currentColor[4];
    glGetFloatv(GL_CURRENT_COLOR, currentColor);
    glUniform4fv(m_ShaderColorLoc, 1, currentColor);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMultMatrixf(modelView);

    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    glVertexPointer(2, GL_FLOAT, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, m_SecondaryBuffer);
    glColorPointer(4, GL_FLOAT, 0, 0);
    glEnableClientState(GL_COLOR_ARRAY);

    glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)(m_NumSegments + 2));

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, GL_ZERO);
    glPopMatrix();

    m_NodeShader.Release();
}

///
/// CGlTreeNodeVbo11
///

CGlTreeNodeVbo11::CGlTreeNodeVbo11(size_t numSegments) : CTreeNodeVboBase(numSegments)
{
    if (numSegments < 4)
        return;

    GetVertexBuffer2D(m_VertexBuffer);

    m_ColorBuffer.resize(m_VertexBuffer.size(), CRgbaColor(0, 0, 0, 0));
    m_ColorBuffer[0].SetAlpha(1.0f);
}

void CGlTreeNodeVbo11::Render(const float* modelView)
{
    if (m_VertexBuffer.empty())
        return;

    float currentColor[4];
    glGetFloatv(GL_CURRENT_COLOR, currentColor);

    m_ColorBuffer.clear();
    m_ColorBuffer.resize(m_VertexBuffer.size(), CRgbaColor(currentColor, 4));
    m_ColorBuffer[0] = CRgbaColor(1.0f, 1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMultMatrixf(modelView);

    glVertexPointer(2, GL_FLOAT, 0, &m_VertexBuffer[0]);
    glEnableClientState(GL_VERTEX_ARRAY);

    glColorPointer(4, GL_FLOAT, 0, &m_ColorBuffer[0]);
    glEnableClientState(GL_COLOR_ARRAY);

    glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)(m_NumSegments + 2));

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
}

END_NCBI_SCOPE
