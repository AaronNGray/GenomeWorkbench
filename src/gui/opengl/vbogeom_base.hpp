#ifndef GUI_OPENGL___VBOGEOM_BASE___HPP
#define GUI_OPENGL___VBOGEOM_BASE___HPP

/*  $Id: vbogeom_base.hpp 43891 2019-09-16 13:50:00Z evgeniev $
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

#include <corelib/ncbiobj.hpp>

#include <gui/opengl/ivbogeom.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class CVboGeomBase : public CObject, public IVboGeom
{
public:
    /// Buffers are initially NULL and draw mode is not set.
    CVboGeomBase(GLenum drawMode, const string& name) : m_DrawMode(drawMode), m_Name(name) {}

    virtual string GetName() const { return m_Name; }

    virtual GLenum GetDrawMode() const { return m_DrawMode; }

    virtual EVertexFormat GetVertexFormat() const;
    virtual ESecondaryFormat GetSecondaryFormat() const;

    virtual size_t GetVertexCount() const;

    virtual void SetVertexBuffer2D(const vector<CVect2<float> >& data);
    virtual void GetVertexBuffer2D(vector<CVect2<float> >& data) const;

    virtual void SetVertexBuffer3D(const vector<CVect3<float> >& data);
    virtual void GetVertexBuffer3D(vector<CVect3<float> >& data) const;

    virtual void SetColorBuffer(const vector<CRgbaColor>& data);
    virtual void GetColorBuffer(vector<CRgbaColor>& data) const;

    virtual void SetColorBufferUC(const vector<CVect4<unsigned char> >& data);
    virtual void GetColorBufferUC(vector<CVect4<unsigned char> >& data) const;

    virtual void SetTexCoordBuffer1D(const vector<float>& data);
    virtual void GetTexCoordBuffer1D(vector<float>& data) const;

    virtual void SetTexCoordBuffer(const vector<CVect2<float> >& data);
    virtual void GetTexCoordBuffer(vector<CVect2<float> >& data) const;

protected:
    virtual void x_SetDrawMode(GLenum drawMode);

    virtual void x_SetBufferData(int index, const void* data, size_t size) = 0;
    virtual void x_GetBufferData(int index, void* data, size_t size) const = 0;

    /// Drawmode based on how vertices are organized and buffer subtype:
    ///  e.g. GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP, GL_TRIANGLES, GL_POINTS etc
    GLenum m_DrawMode;
    string m_Name;

    EVertexFormat    m_VertexFormat = kVertexFormatNone;
    ESecondaryFormat m_SecondaryFormat = kSecondaryFormatNone;
    size_t m_VertexCount = 0;
    size_t m_SecondaryCount = 0;
};

class CNarrowTreeEdgeVboGeomBase : public CObject, public IVboGeom
{
public:
    CNarrowTreeEdgeVboGeomBase(IVboGeom* edgeGeom) : m_EdgeGeom(edgeGeom), m_Name("NarrowTreeEdges") {}

    virtual string GetName() const { return m_Name; }

    virtual GLenum GetDrawMode() const { return GL_LINES; }

    virtual EVertexFormat GetVertexFormat() const { return kVertexFormatVertex2D; }
    virtual ESecondaryFormat GetSecondaryFormat() const { return kSecondaryFormatNone;  }

    virtual size_t GetVertexCount() const;

    virtual void SetVertexBuffer2D(const vector<CVect2<float> >& data);
    virtual void GetVertexBuffer2D(vector<CVect2<float> >& data) const;

    virtual void SetVertexBuffer3D(const vector<CVect3<float> >& data);
    virtual void GetVertexBuffer3D(vector<CVect3<float> >& data) const;

    virtual void SetColorBuffer(const vector<CRgbaColor>& data);
    virtual void GetColorBuffer(vector<CRgbaColor>& data) const;

    virtual void SetColorBufferUC(const vector<CVect4<unsigned char> >& data);
    virtual void GetColorBufferUC(vector<CVect4<unsigned char> >& data) const;

    virtual void SetTexCoordBuffer1D(const vector<float>& data);
    virtual void GetTexCoordBuffer1D(vector<float>& data) const;

    virtual void SetTexCoordBuffer(const vector<CVect2<float> >& data);
    virtual void GetTexCoordBuffer(vector<CVect2<float> >& data) const;

protected:
    virtual void x_SetDrawMode(GLenum drawMode);

    CIRef<IVboGeom> m_EdgeGeom;
    string m_Name;
};

class CFillerPointVboGeomBase : public CObject, public IVboGeom
{
public:
    CFillerPointVboGeomBase(IVboGeom* edgeGeom) : m_EdgeGeom(edgeGeom), m_Name("FillerPoints") {}

    virtual string GetName() const { return m_Name; }

    virtual GLenum GetDrawMode() const { return GL_POINTS; }

    virtual EVertexFormat GetVertexFormat() const { return kVertexFormatVertex2D; }
    virtual ESecondaryFormat GetSecondaryFormat() const { return kSecondaryFormatColorUChar;  }

    virtual size_t GetVertexCount() const;

    virtual void SetVertexBuffer2D(const vector<CVect2<float> >& data);
    virtual void GetVertexBuffer2D(vector<CVect2<float> >& data) const;

    virtual void SetVertexBuffer3D(const vector<CVect3<float> >& data);
    virtual void GetVertexBuffer3D(vector<CVect3<float> >& data) const;

    virtual void SetColorBuffer(const vector<CRgbaColor>& data);
    virtual void GetColorBuffer(vector<CRgbaColor>& data) const;

    virtual void SetColorBufferUC(const vector<CVect4<unsigned char> >& data);
    virtual void GetColorBufferUC(vector<CVect4<unsigned char> >& data) const;

    virtual void SetTexCoordBuffer1D(const vector<float>& data);
    virtual void GetTexCoordBuffer1D(vector<float>& data) const;

    virtual void SetTexCoordBuffer(const vector<CVect2<float> >& data);
    virtual void GetTexCoordBuffer(vector<CVect2<float> >& data) const;

protected:
    virtual void x_SetDrawMode(GLenum drawMode);

    CIRef<IVboGeom> m_EdgeGeom;
    string m_Name;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___VBOGEOM_BASE___HPP
