#ifndef GUI_OPENGL___GL_VBO_GEOM__HPP
#define GUI_OPENGL___GL_VBO_GEOM__HPP

/*  $Id: glvbogeom.hpp 42973 2019-05-01 16:19:00Z katargir $
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
 * CGlVboGeom holds 1 or more vertex buffers to be used in rendering.  If there
 * are multiple buffers they are all enabled for rendering together (except for
 * buffers explicitly marked as not visible).
 *
 * Since there is a difference in how buffers are enabled and used in OpenGL 1.4 
 * and below, there are two concrete implementations of CGlVboGeom.  For CVboGeom11
 * the buffers are assumed to be arrays and the commands glVertexPointer, glColorPointer,
 * etc. provide pointers to the arrays (of vertex and vertex attribute data) to be rendered.
 *
 * For CGlVboGeom20 the buffers are OpenGL vertex buffers, so the buffers are enabled
 * prior to calling glVertexPointer, glColorPointer, etc.  Note that in the future
 * CGlVboGeom20 could use glVertexAttribPointer to specify vertex attributes in a more
 * generic fasion (and in a way that is also compatible with OpenGL ES 2.0, which does not
 * support glVertexPointer, glColorPointer etc.)
 *
 *
 *
 *                             CGlVboGeomBase
 *                              ^    ^
 *                              |    |
 *                        ______|    |__________
 *                        |                     |
 *                        |                     |
 *                   CGlVboGeom11          CGlVboGeom20
 */

#include <gui/opengl.h>
#include <gui/gui.hpp>

#include "vbogeom_base.hpp"

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class CGlVboGeom20 : public CVboGeomBase
{
    friend class CGlResMgr;

public:
    GLuint GetVertexBuffer() const { return m_VertexBuffer; }
    GLuint GetSecondaryBuffer() const { return m_SecondaryBuffer; }

protected:
    CGlVboGeom20(GLenum drawMode, const string& name) : CVboGeomBase(drawMode, name) {}
    virtual ~CGlVboGeom20();

    virtual void Render(const float* modelView);

    virtual void x_SetBufferData(int index, const void* data, size_t size);
    virtual void x_GetBufferData(int index, void* data, size_t size) const;

private:
    GLuint  m_VertexBuffer    = GL_ZERO;
    GLuint  m_SecondaryBuffer = GL_ZERO;
};

class CNarrowTreeEdgeVboGeom20 : public CNarrowTreeEdgeVboGeomBase
{
    friend class CGlResMgr;

private:
    CNarrowTreeEdgeVboGeom20(IVboGeom* edgeGeom) : CNarrowTreeEdgeVboGeomBase(edgeGeom) {}
    virtual void Render(const float* modelView);
};

class CFillerPointVboGeom20 : public CFillerPointVboGeomBase
{
    friend class CGlResMgr;

private:
    CFillerPointVboGeom20(IVboGeom* edgeGeom) : CFillerPointVboGeomBase(edgeGeom) {}
    virtual void Render(const float* modelView);
};

// OpenGL 1.1

class CGlVboGeom11 : public CVboGeomBase
{
    friend class CGlResMgr;

public:
    const vector<char>& GetVertexBuffer() const { return m_VertexBuffer; }
    const vector<char>& GetSecondaryBuffer() const { return m_SecondaryBuffer; }

protected:
    CGlVboGeom11(GLenum drawMode, const string& name) : CVboGeomBase(drawMode, name) {}

    virtual void Render(const float* modelView);

    virtual void x_SetBufferData(int index, const void* data, size_t size);
    virtual void x_GetBufferData(int index, void* data, size_t size) const;

private:
    vector<char>  m_VertexBuffer;
    vector<char>  m_SecondaryBuffer;
};

class CNarrowTreeEdgeVboGeom11 : public CNarrowTreeEdgeVboGeomBase
{
    friend class CGlResMgr;

private:
    CNarrowTreeEdgeVboGeom11(IVboGeom* edgeGeom) : CNarrowTreeEdgeVboGeomBase(edgeGeom) {}
    virtual void Render(const float* modelView);
};

class CFillerPointVboGeom11 : public CFillerPointVboGeomBase
{
    friend class CGlResMgr;

private:
    CFillerPointVboGeom11(IVboGeom* edgeGeom) : CFillerPointVboGeomBase(edgeGeom) {}
    virtual void Render(const float* modelView);
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_VBO_GEOM__HPP
