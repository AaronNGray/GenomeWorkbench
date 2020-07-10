#ifndef GUI_OPENGL___MTL_VBO_GEOM__HPP
#define GUI_OPENGL___MTL_VBO_GEOM__HPP

/*  $Id: mtlvbogeom.hpp 43891 2019-09-16 13:50:00Z evgeniev $
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
 */

#include <gui/opengl.h>
#include <gui/gui.hpp>

#include "vbogeom_base.hpp"

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class CMtlVboGeom : public CVboGeomBase
{
    friend class CGlResMgr;
public:
    id<MTLBuffer> GetVertexBuffer() const { return m_VertexBuffer; }
    id<MTLBuffer> GetSecondaryBuffer() const  { return m_SecondaryBuffer; }

protected:
    CMtlVboGeom(GLenum drawMode, const string& name) : CVboGeomBase(drawMode, name) {}
    virtual ~CMtlVboGeom();

    virtual void Render(const float* modelView) override;

    virtual void x_SetBufferData(int index, const void* data, size_t size) override;
    virtual void x_GetBufferData(int index, void* data, size_t size) const override;

private:
    id<MTLBuffer> m_VertexBuffer    = nil;
    id<MTLBuffer> m_SecondaryBuffer = nil;
};

class CMtlVboGeomLines : public CObject, public IVboGeom
{
    friend class CGlResMgr;
    friend class CMtlNarrowTreeEdgeVboGeom;
    friend class CMtlFillerPointVboGeom;
public:
    virtual string GetName() const override { return m_Name; }

    virtual void Render(const float* modelView) override;

    virtual GLenum GetDrawMode() const override { return GL_LINES; }

    virtual EVertexFormat GetVertexFormat() const override
        { return m_Vertices.empty() ? kVertexFormatNone : kVertexFormatVertex2D; }
    virtual ESecondaryFormat GetSecondaryFormat() const override
        { return m_Colors.empty() ? kSecondaryFormatNone : kSecondaryFormatColorUChar; }

    virtual size_t GetVertexCount() const override { return m_Vertices.size(); }

    virtual void SetVertexBuffer2D(const vector<CVect2<float> >& data) override
        { m_Vertices = data; }
    virtual void GetVertexBuffer2D(vector<CVect2<float> >& data) const override
        { data = m_Vertices; }

    virtual void SetVertexBuffer3D(const vector<CVect3<float> >&) override { _ASSERT(false); }
    virtual void GetVertexBuffer3D(vector<CVect3<float> >&) const override { _ASSERT(false); }

    virtual void SetColorBuffer(const vector<CRgbaColor>&) override { _ASSERT(false); }
    virtual void GetColorBuffer(vector<CRgbaColor>&) const override { _ASSERT(false); }

    virtual void SetColorBufferUC(const vector<CVect4<unsigned char> >& data) override
        { m_Colors = data; }
    virtual void GetColorBufferUC(vector<CVect4<unsigned char> >& data) const override
        { data = m_Colors; }

    virtual void SetTexCoordBuffer1D(const vector<float>&) override { _ASSERT(false); }
    virtual void GetTexCoordBuffer1D(vector<float>&) const override { _ASSERT(false); }

    virtual void SetTexCoordBuffer(const vector<CVect2<float> >&) override { _ASSERT(false); }
    virtual void GetTexCoordBuffer(vector<CVect2<float> >&) const override { _ASSERT(false); }

protected:
    CMtlVboGeomLines(const string& name) : m_Name(name) {}

private:
    virtual void x_SetDrawMode(GLenum) override { _ASSERT(false); }

    string m_Name;
    vector<CVect2<float> > m_Vertices;
    vector<CVect4<unsigned char> > m_Colors;
};

class CMtlNarrowTreeEdgeVboGeom : public CNarrowTreeEdgeVboGeomBase
{
    friend class CGlResMgr;

private:
    CMtlNarrowTreeEdgeVboGeom(IVboGeom* edgeGeom) : CNarrowTreeEdgeVboGeomBase(edgeGeom) {}
    virtual void Render(const float* modelView) override;
};

class CMtlFillerPointVboGeom : public CFillerPointVboGeomBase
{
    friend class CGlResMgr;

private:
    CMtlFillerPointVboGeom(IVboGeom* edgeGeom) : CFillerPointVboGeomBase(edgeGeom) {}

    virtual void Render(const float* modelView) override;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___MTL_VBO_GEOM__HPP

