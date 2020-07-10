#ifndef GUI_OPENGL___GL_TREE_NODE_VBO_BASE__HPP
#define GUI_OPENGL___GL_TREE_NODE_VBO_BASE__HPP

/*  $Id: treenodevbo_base.hpp 43891 2019-09-16 13:50:00Z evgeniev $
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

class CTreeNodeVboBase : public CObject, public IVboGeom
{
    friend class CGlResMgr;

protected:
    CTreeNodeVboBase(size_t numSegments) : m_NumSegments(numSegments) {}

public:
    virtual string GetName() const { return "TreeNodeVbo"; };

    virtual GLenum GetDrawMode() const { return GL_TRIANGLE_FAN; }

    virtual EVertexFormat GetVertexFormat() const { return kVertexFormatVertex2D; }
    virtual ESecondaryFormat GetSecondaryFormat() const { return kSecondaryFormatColorFloat; }

    virtual size_t GetVertexCount() const { return m_NumSegments + 2; }

    virtual void SetVertexBuffer2D(const vector<CVect2<float> >&) { _ASSERT(false); }
    virtual void GetVertexBuffer2D(vector<CVect2<float> >& data) const;

    virtual void SetVertexBuffer3D(const vector<CVect3<float> >&) { _ASSERT(false); }
    virtual void GetVertexBuffer3D(vector<CVect3<float> >&) const { _ASSERT(false); }

    virtual void SetColorBuffer(const vector<CRgbaColor>& data) { m_ColorBuffer = data; }
    virtual void GetColorBuffer(vector<CRgbaColor>& data) const { data = m_ColorBuffer; }

    virtual void SetColorBufferUC(const vector<CVect4<unsigned char> >&) { _ASSERT(false); }
    virtual void GetColorBufferUC(vector<CVect4<unsigned char> >&) const { _ASSERT(false); }

    virtual void SetTexCoordBuffer1D(const vector<float>&) { _ASSERT(false); }
    virtual void GetTexCoordBuffer1D(vector<float>&) const { _ASSERT(false); }

    virtual void SetTexCoordBuffer(const vector<CVect2<float> >&) { _ASSERT(false); }
    virtual void GetTexCoordBuffer(vector<CVect2<float> >&) const { _ASSERT(false); }

protected:
    virtual void x_SetDrawMode(GLenum) { _ASSERT(false); }

    vector<CRgbaColor> m_ColorBuffer;

    size_t m_NumSegments;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_TREE_NODE_VBO_BASE__HPP
