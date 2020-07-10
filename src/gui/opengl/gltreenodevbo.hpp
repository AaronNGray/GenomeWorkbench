#ifndef GUI_OPENGL___GL_TREE_NODE_VBO__HPP
#define GUI_OPENGL___GL_TREE_NODE_VBO__HPP

/*  $Id: gltreenodevbo.hpp 43138 2019-05-20 20:47:12Z katargir $
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

#include <gui/opengl/glshader.hpp>

#include "treenodevbo_base.hpp"

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class CGlTreeNodeVbo20 : public CTreeNodeVboBase
{
    friend class CGlResMgr;
public:
    CGlTreeNodeVbo20(size_t numSegments);
    virtual ~CGlTreeNodeVbo20();

    virtual void Render(const float* modelView);

private:
    GLuint  m_VertexBuffer = GL_ZERO;
    GLuint  m_SecondaryBuffer = GL_ZERO;

    CGlShader m_NodeShader;
    GLint m_ShaderColorLoc;
};

class CGlTreeNodeVbo11 : public CTreeNodeVboBase
{
    friend class CGlResMgr;
public:
    CGlTreeNodeVbo11(size_t numSegments);

    virtual void Render(const float* modelView);

private:
    vector<CVect2<float> > m_VertexBuffer;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_TREE_NODE_VBO__HPP
