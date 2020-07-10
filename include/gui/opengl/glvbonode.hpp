#ifndef GUI_OPENGL___GL_VBO_NODE__HPP
#define GUI_OPENGL___GL_VBO_NODE__HPP

/*  $Id: glvbonode.hpp 43891 2019-09-16 13:50:00Z evgeniev $
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

#include <gui/opengl.h>
#include <gui/gui.hpp>

#include <gui/utils/vect2.hpp>
#include <gui/utils/vect3.hpp>
#include <gui/utils/vect4.hpp>
#include <gui/utils/matrix3.hpp>
#include <gui/utils/matrix4.hpp>

#include <gui/utils/rgba_color.hpp>
#include <gui/opengl/globject.hpp>
#include <gui/opengl/glrendernode.hpp>
#include <gui/opengl/ivbogeom.hpp>


/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

//////////////////////////////////////////////////////////////////////////////
/// CGlVboNode
/// A rendering node that holds a vertex buffer object.  The vbo object
/// includes both the vertices themselves as well as any associated attributes
/// like color, texture coords etc.
///
class NCBI_GUIOPENGL_EXPORT CGlVboNode : public CGlRenderNode
{
public:
    /// Buffers are initially NULL and draw mode is not set.
    CGlVboNode();

    // Create the buffer based on draw type (as point, line, tri or quad vbo)
    // @param draw_mode
    //   One of: GL_{POINTS, LINES, LINE_STRIP, LINE_LOOP, 
    //               TRIANGLES, TRIANGLE_STRIP TRIANGLE_FAN, QUADS, QUAD_STRIP}
    CGlVboNode(GLenum draw_mode);

    /// Delete all buffers
    virtual ~CGlVboNode();

    /// Set/get geometry object.  These can be shared between nodes.
    void SetVBOGeom(IVboGeom* geom) { m_VBOGeom.Reset(geom); };
    void SetVBOGeom(GLenum drawMode);

    CIRef<IVboGeom> GetVBOGeom() { return m_VBOGeom; }

    size_t GetVertexCount() const { return m_VBOGeom ? m_VBOGeom->GetVertexCount() : 0; }

    //void SetScale(const CVect3<float>& f) {m_Scale = f; }

    /// Return the current drawing mode (e.g. GL_TRIANGLES) or GL_INVALID_ENUM
    /// if the geometry is not currently defined.
    GLenum GetDrawMode() const {return (m_VBOGeom==NULL) ? GL_INVALID_ENUM : m_VBOGeom->GetDrawMode();}
    IVboGeom::EVertexFormat GetVertexFormat() const;
    IVboGeom::ESecondaryFormat GetSecondaryFormat() const;
    
    void SetVertexBuffer2D(const vector<CVect2<float> >& data);
    void GetVertexBuffer2D(vector<CVect2<float> >& data) const;

    void SetVertexBuffer3D(const vector<CVect3<float> >& data);
    void GetVertexBuffer3D(vector<CVect3<float> >& data) const;

    void SetColorBuffer(const vector<CRgbaColor>& data);
    void GetColorBuffer(vector<CRgbaColor>& data) const;

    void SetColorBufferUC(const vector<CVect4<unsigned char> >& data);
    void GetColorBufferUC(vector<CVect4<unsigned char> >& data) const;

    void SetTexCoordBuffer1D(const vector<float>& data);
    void GetTexCoordBuffer1D(vector<float>& data) const;

    void SetTexCoordBuffer(const vector<CVect2<float> >& data);
    void GetTexCoordBuffer(vector<CVect2<float> >& data) const;

    void Get2DVertexBuffer(std::vector<CVect2<float> >& verts) const;

    bool GetColors(std::vector<CRgbaColor>& colors, bool is_greyscale) const;

    /// Return default color from node in default_color.
    /// If this function returns false, the color was not explicitly set
    /// but default_color will still be udpated with a default value
    bool GetDefaultColor(CRgbaColor& default_color, bool is_greyscale) const;

protected:

    /// Render VBO(s)
    virtual void x_Render();
    
    /// subclass for vboset representing point, line, tri or quad geometry
    CIRef<IVboGeom> m_VBOGeom;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_VBO_NODE__HPP
