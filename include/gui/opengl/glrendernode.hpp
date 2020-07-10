#ifndef GUI_OPENGL___GL_RENDER_NODE__HPP
#define GUI_OPENGL___GL_RENDER_NODE__HPP

/*  $Id: glrendernode.hpp 43891 2019-09-16 13:50:00Z evgeniev $
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
#include <gui/opengl/glstate.hpp>
#include <gui/opengl/globject.hpp>


/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

//////////////////////////////////////////////////////////////////////////////
/// CGlRenderNode
/// A unit of renderable geometry and the associated state information needed
/// for rendering.
///
class NCBI_GUIOPENGL_EXPORT CGlRenderNode : public CGlObject
{
public:
    /// ctor
    CGlRenderNode();

    /// Set state and call x_Render() to render geometry
    virtual void Render();

    /// Set/get current OpenGL state
    void SetState(CGlState* state);
    CGlState& GetState();
    CGlState const& GetState() const;

    /// Set/get node visibility. 
    void SetVisible(bool b) { m_Visible = b; }
    bool IsVisible() const { return m_Visible; }

    /// Turn off visibility for individual render targets.  Allows nodes to be
    /// visible in OpenGL but hidden in PDF, for example.  SetVisible(false)
    /// applies to all targets.
    void SkipTarget(ERenderTarget target, bool skip);
    bool IsSkipped(ERenderTarget target) const;
    /*
     * A node can be rendered at multiple positions (transformations) for
     * efficency. If no positions are given, it will not be rendered.
     */
    
    /// Set 1 transformation and have it be the identity matrix
    void SetDefaultPosition();
    /// Set 1 transformation and have it be "mat"
    void SetPosition(const CMatrix4<float>& mat);
    /// Add a new position
    void AddPosition(const CMatrix4<float>& mat) { m_Positions.push_back(mat); }
    /// Apply scale-dependent rotation (radians) around z (to all instances)
    void SetRotationZ(float a) { m_RotAngleZ = a; }
    /// Set/Get offset in pixels
    void SetPixelOffset(const CVect2<float>& off) { m_PixelOffset = off; }
    CVect2<float> GetPixelOffset() const { return m_PixelOffset; }

    /// return the current set of transformations for thisnode
    vector<CMatrix4<float> >& GetPositions() { return m_Positions; }
    void ClearPositions() { m_Positions.clear(); }

    /// return the position with rotation and pixel offset baked in
    CMatrix4<float> GetTransformedPosition(size_t idx);
    

protected:
    /// Does actual rendering work
    virtual void x_Render() {}

    /// If false, do not render node contents
    bool m_Visible;

    /// Set of rendering targets for which this node will not be rendered
    /// even if m_Visible is true
    vector<ERenderTarget> m_SkippedTargets;

    /// OpenGL state
    CRef<CGlState>  m_State;  

    /// Set of positions (transformations) at which to render this node
    vector<CMatrix4<float> > m_Positions;

    /// Pixel offset to apply to object
    CVect2<float> m_PixelOffset;  

    /// Optional rotation angle to rotate object around Z axis. Stored here
    /// because it can be updated according to the current scale to get
    /// correct scale-dependent rotation angle.
    float m_RotAngleZ;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_RENDER_NODE__HPP
