/*  $Id: glvbogeom.cpp 43451 2019-07-02 14:33:22Z katargir $
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

#include "glvbogeom.hpp"

BEGIN_NCBI_SCOPE

///
/// CGlVboGeom20
///

CGlVboGeom20::~CGlVboGeom20()
{
    if (m_VertexBuffer != GL_ZERO) {
        glDeleteBuffers(1, &m_VertexBuffer);
        _ASSERT(GL_NO_ERROR == glGetError());
        m_VertexBuffer = GL_ZERO;
    }

    if (m_SecondaryBuffer != GL_ZERO) {
        glDeleteBuffers(1, &m_SecondaryBuffer);
        _ASSERT(GL_NO_ERROR == glGetError());
        m_SecondaryBuffer = GL_ZERO;
    }
}

void CGlVboGeom20::x_SetBufferData(int index, const void* data, size_t size)
{
    if (index != 0 && index != 1) {
        _ASSERT(false);
        return;
    }

    GLuint buffer = GL_ZERO;

    if (index == 0) {
        if (m_VertexBuffer == GL_ZERO)
            glGenBuffers(1, &m_VertexBuffer);
        buffer = m_VertexBuffer;
    }
    else {
        if (m_SecondaryBuffer == GL_ZERO)
            glGenBuffers(1, &m_SecondaryBuffer);
        buffer = m_SecondaryBuffer;
    }

    if (buffer == GL_ZERO)
        return;

    GLint bufferSize = 0;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);

    if ((size_t)bufferSize >= size)
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
    else
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CGlVboGeom20::x_GetBufferData(int index, void* data, size_t size) const
{
    if (index != 0 && index != 1) {
        _ASSERT(false);
        return;
    }

    GLuint buffer = (index == 0) ? m_VertexBuffer : m_SecondaryBuffer;
    if (buffer == GL_ZERO)
        return;

    GLint bufferSize = 0;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);

    if ((size_t)bufferSize < size)
        return;

    glGetBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CGlVboGeom20::Render(const float* modelView)
{
    if (m_VertexFormat == kVertexFormatNone)
        return;

    if (m_SecondaryFormat != kSecondaryFormatNone)
        _ASSERT(m_VertexCount == m_SecondaryCount);

    if (m_VertexBuffer == GL_ZERO)
        return;

    int saveMatrixMode = 0;
    glGetIntegerv(GL_MATRIX_MODE, &saveMatrixMode);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMultMatrixf(modelView);

    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    glVertexPointer((m_VertexFormat == kVertexFormatVertex2D) ? 2 : 3, GL_FLOAT, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);

    if (m_SecondaryFormat == kSecondaryFormatColorFloat) {
        glBindBuffer(GL_ARRAY_BUFFER, m_SecondaryBuffer);
        glColorPointer(4, GL_FLOAT, 0, 0);
        glEnableClientState(GL_COLOR_ARRAY);
    }
    else if (m_SecondaryFormat == kSecondaryFormatColorUChar) {
        glBindBuffer(GL_ARRAY_BUFFER, m_SecondaryBuffer);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
        glEnableClientState(GL_COLOR_ARRAY);
    }
    else if (m_SecondaryFormat == kSecondaryFormatTexture2D) {
        glBindBuffer(GL_ARRAY_BUFFER, m_SecondaryBuffer);
        glTexCoordPointer(2, GL_FLOAT, 0, 0);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    else if (m_SecondaryFormat == kSecondaryFormatTexture1D) {
        glBindBuffer(GL_ARRAY_BUFFER, m_SecondaryBuffer);
        glTexCoordPointer(1, GL_FLOAT, 0, 0);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    glBindBuffer(GL_ARRAY_BUFFER, GL_ZERO);

    glDrawArrays(m_DrawMode, 0, (GLsizei)m_VertexCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glPopMatrix();

    if (saveMatrixMode == GL_PROJECTION)
        glMatrixMode(GL_PROJECTION);
}

///
/// CNarrowTreeEdgeVboGeom20
///

void CNarrowTreeEdgeVboGeom20::Render(const float* modelView)
{
    CGlVboGeom20* sharedGeom = dynamic_cast<CGlVboGeom20*>(m_EdgeGeom.GetNCPointerOrNull());
    if (!sharedGeom)
        return;

    if (sharedGeom->GetVertexFormat() != kVertexFormatVertex2D)
        return;

    GLuint vertexBuffer = sharedGeom->GetVertexBuffer();
    if (vertexBuffer == GL_ZERO)
        return;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMultMatrixf(modelView);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexPointer(2, GL_FLOAT, 0, 0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINES, 0, (GLsizei)sharedGeom->GetVertexCount());
    glDisableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glPopMatrix();
}

///
/// CFillerPointVboGeom20
///

void CFillerPointVboGeom20::Render(const float* modelView)
{
    CGlVboGeom20* sharedGeom = dynamic_cast<CGlVboGeom20*>(m_EdgeGeom.GetNCPointerOrNull());
    if (!sharedGeom)
        return;

    if (sharedGeom->GetVertexFormat() != kVertexFormatVertex2D ||
        sharedGeom->GetSecondaryFormat() != kSecondaryFormatColorUChar)
        return;

    size_t vertex_count = sharedGeom->GetVertexCount() / 4;

    GLuint vertexBuffer = sharedGeom->GetVertexBuffer();
    if (vertexBuffer == GL_ZERO)
        return;

    GLuint secondaryBuffer = sharedGeom->GetSecondaryBuffer();
    if (secondaryBuffer == GL_ZERO)
        return;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMultMatrixf(modelView);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexPointer(2, GL_FLOAT, 32, (GLvoid*)16);

    glBindBuffer(GL_ARRAY_BUFFER, secondaryBuffer);
    glColorPointer(4, GL_UNSIGNED_BYTE, 16, (GLvoid*)8);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glDrawArrays(GL_POINTS, 0, (GLsizei)vertex_count);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glPopMatrix();
}

///
/// CGlVboGeom11
///

void CGlVboGeom11::x_SetBufferData(int index, const void* data, size_t size)
{
    if (index != 0 && index != 1) {
        _ASSERT(false);
        return;
    }

    if (index == 0) {
        if (m_VertexBuffer.size() < size)
            m_VertexBuffer.resize(size);
        memcpy(&m_VertexBuffer[0], data, size);
    }
    else {
        if (m_SecondaryBuffer.size() < size)
            m_SecondaryBuffer.resize(size);
        memcpy(&m_SecondaryBuffer[0], data, size);
    }
}

void CGlVboGeom11::x_GetBufferData(int index, void* data, size_t size) const
{
    if (index != 0 && index != 1) {
        _ASSERT(false);
        return;
    }

    if (index == 0) {
        if (m_VertexBuffer.size() < size) {
            _ASSERT(false);
            return;
        }
        memcpy(data, &m_VertexBuffer[0], size);
    }
    else {
        if (m_SecondaryBuffer.size() < size) {
            _ASSERT(false);
            return;
        }
        memcpy(data, &m_SecondaryBuffer[0], size);
    }
}

void CGlVboGeom11::Render(const float* modelView)
{
    if (m_VertexFormat == kVertexFormatNone)
        return;

    if (m_SecondaryFormat != kSecondaryFormatNone)
        _ASSERT(m_VertexCount == m_SecondaryCount);

    if (m_VertexBuffer.empty())
        return;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMultMatrixf(modelView);

    glVertexPointer((m_VertexFormat == kVertexFormatVertex2D) ? 2 : 3, GL_FLOAT, 0, &m_VertexBuffer[0]);

    glEnableClientState(GL_VERTEX_ARRAY);

    if (m_SecondaryFormat != kSecondaryFormatNone) {
        if (m_SecondaryFormat == kSecondaryFormatColorFloat) {
            glColorPointer(4, GL_FLOAT, 0, &m_SecondaryBuffer[0]);
            glEnableClientState(GL_COLOR_ARRAY);
        }
        else if (m_SecondaryFormat == kSecondaryFormatColorUChar) {
            glColorPointer(4, GL_UNSIGNED_BYTE, 0, &m_SecondaryBuffer[0]);
            glEnableClientState(GL_COLOR_ARRAY);
        }
        else if (m_SecondaryFormat == kSecondaryFormatTexture2D) {
            glTexCoordPointer(2, GL_FLOAT, 0, &m_SecondaryBuffer[0]);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        else if (m_SecondaryFormat == kSecondaryFormatTexture1D) {
            glTexCoordPointer(1, GL_FLOAT, 0, &m_SecondaryBuffer[0]);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        }
    }

    glDrawArrays(m_DrawMode, 0, (GLsizei)m_VertexCount);

    glDisableClientState(GL_VERTEX_ARRAY);

    switch (m_SecondaryFormat) {
    case kSecondaryFormatNone:
        break;
    case kSecondaryFormatColorFloat :
    case kSecondaryFormatColorUChar :
        glDisableClientState(GL_COLOR_ARRAY);
        break;
    case kSecondaryFormatTexture2D :
    case kSecondaryFormatTexture1D :
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    glPopMatrix();
}

///
/// CNarrowTreeEdgeVboGeom11
///

void CNarrowTreeEdgeVboGeom11::Render(const float* modelView)
{
    CGlVboGeom11* sharedGeom = dynamic_cast<CGlVboGeom11*>(m_EdgeGeom.GetNCPointerOrNull());
    if (!sharedGeom)
        return;

    if (sharedGeom->GetVertexFormat() != kVertexFormatVertex2D)
        return;

    const vector<char>& vertexBuffer = sharedGeom->GetVertexBuffer();
    if (vertexBuffer.empty())
        return;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMultMatrixf(modelView);

    glVertexPointer(2, GL_FLOAT, 0, &vertexBuffer[0]);

    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINES, 0, (GLsizei)sharedGeom->GetVertexCount());
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
}

///
/// CFillerPointVboGeom11
///

void CFillerPointVboGeom11::Render(const float* modelView)
{
    CGlVboGeom11* sharedGeom = dynamic_cast<CGlVboGeom11*>(m_EdgeGeom.GetNCPointerOrNull());
    if (!sharedGeom)
        return;

    if (sharedGeom->GetVertexFormat() != kVertexFormatVertex2D ||
        sharedGeom->GetSecondaryFormat() != kSecondaryFormatColorUChar)
        return;

    const vector<char>& vertexBuffer = sharedGeom->GetVertexBuffer();
    if (vertexBuffer.empty())
        return;

    const vector<char>& secondaryBuffer = sharedGeom->GetSecondaryBuffer();
    if (secondaryBuffer.empty())
        return;

    size_t vertex_count = sharedGeom->GetVertexCount() / 4;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMultMatrixf(modelView);

    glVertexPointer(2, GL_FLOAT, 32, &vertexBuffer[16]);
    glColorPointer(4, GL_UNSIGNED_BYTE, 16, &secondaryBuffer[8]);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glDrawArrays(GL_POINTS, 0, (GLsizei)vertex_count);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glPopMatrix();
}

END_NCBI_SCOPE
