#ifndef GUI_OPENGL___MTL_TREE_NODE_VBO__HPP
#define GUI_OPENGL___MTL_TREE_NODE_VBO__HPP

/*  $Id: mtltreenodevbo.hpp 43137 2019-05-20 20:24:18Z katargir $
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

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

#include "treenodevbo_base.hpp"

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class CMtlTreeNodeVbo : public CTreeNodeVboBase
{
    friend class CGlResMgr;
private:
    CMtlTreeNodeVbo(size_t numSegments);

public:
    virtual ~CMtlTreeNodeVbo();

    virtual void Render(const float* modelView);

private:
    id<MTLBuffer> m_VertexBuffer = nil;
    id<MTLBuffer> m_SecondaryBuffer = nil;
    id<MTLBuffer> m_IndexBuffer = nil;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___MTL_TREE_NODE_VBO__HPP
