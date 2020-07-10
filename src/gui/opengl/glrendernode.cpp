/*  $Id: glrendernode.cpp 43891 2019-09-16 13:50:00Z evgeniev $
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
#include <gui/opengl/glrendernode.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/irender.hpp>

BEGIN_NCBI_SCOPE

CGlRenderNode::CGlRenderNode() 
: m_Visible(true)
, m_PixelOffset(0.0f, 0.0f)
, m_RotAngleZ(0.0f)
{
    m_State.Reset(new CGlState());
    SetDefaultPosition();
}

void CGlRenderNode::Render() 
{
    IRender& gl = GetGl();

    m_State->MakeCurrent(gl.GetApi());

    x_Render();
}

void CGlRenderNode::SetState(CGlState* state) 
{ 
    m_State.Reset(state);
}

CGlState& CGlRenderNode::GetState() 
{ 
    return m_State.GetNCObject();
}

CGlState const& CGlRenderNode::GetState() const
{
    return m_State.GetNCObject();
}

void CGlRenderNode::SkipTarget(ERenderTarget target, bool skip)
{
    vector<ERenderTarget>::iterator iter;
    
    iter = std::find(m_SkippedTargets.begin(), m_SkippedTargets.end(), target);

    // Not currently skipped.  Add if skip==true
    if (iter == m_SkippedTargets.end()) {
        if (skip) {
            m_SkippedTargets.push_back(target);
        }
    }
    // Currently skipped. Remove if skip==false
    else {
        if (!skip) {
            m_SkippedTargets.erase(iter);
        }
    }
    
    return;
}

bool CGlRenderNode::IsSkipped(ERenderTarget target) const
{
    vector<ERenderTarget>::const_iterator iter;
    
    iter = std::find(m_SkippedTargets.begin(), m_SkippedTargets.end(), target);

    // Not currently skipped.  Add if skip==true
    if (iter != m_SkippedTargets.end())
        return true;
    
    return false;
}

void CGlRenderNode::SetDefaultPosition() 
{ 
    ClearPositions(); 

    CMatrix4<float> mat;
    mat.Identity();
    m_Positions.push_back(mat);
    m_RotAngleZ = 0.0f;
}

void CGlRenderNode::SetPosition(const CMatrix4<float>& mat)
{ 
    if (m_Positions.size() == 1) {
        m_Positions[0] = mat;
    }
    else {
        ClearPositions(); 
        m_Positions.push_back(mat);
    }
}

CMatrix4<float> CGlRenderNode::GetTransformedPosition(size_t idx)
{
    CMatrix4<float> mat(m_Positions[idx]);

    // Both pixel offsets and rotation are dependent on the current (non-uniform)
    // scaling in x and y so if we want transformations that do not need to be
    // completely updated every time we scale the view (zoom, resize window) we need
    // to apply the scaling factors here.  Get optional (z) rotation matrix first:
    CMatrix4<float> rm;
    rm.Identity();

    float xs = 1.0f;
    float ys = 1.0f;

    if (m_RotAngleZ != 0.0) {
        // Recompute the angle 
        float cos_a = cosf(m_RotAngleZ)*1.0f / (float)m_State->GetScaleFactor().X();
        float sin_a = sinf(m_RotAngleZ)*1.0f / (float)m_State->GetScaleFactor().Y();
        float a = atan2f(sin_a, cos_a);

        // Now create a standard 2D (rotate around z axis) rotation matrix:
        rm(0, 0) = cosf(a);
        rm(0, 1) = -sinf(a);
        rm(1, 0) = sinf(a);
        rm(1, 1) = cosf(a);

        xs = mat(0, 0);
        ys = mat(1, 1);
        mat(0, 0) = 1.0;
        mat(1, 1) = 1.0;
    }

    /// Apply optional pixel offset
    if (m_PixelOffset.Length2() > 0.0f) {
        CMatrix4<float> tm;

        tm.Identity();
        CVect4<float> pix_offset = CVect4<float>(m_PixelOffset.X(), m_PixelOffset.Y(), 0.0f, 1.0f);

        // rotate the offset if needed
        if (m_RotAngleZ != 0.0) {
            pix_offset = rm*pix_offset;
        }

        tm(0, 3) = pix_offset.X()*(float)m_State->GetScaleFactor().X();
        tm(1, 3) = pix_offset.Y()*(float)m_State->GetScaleFactor().Y();

        mat *= tm;
    }

    /// Scale the object to keep size invarient with zoom (this scale comes after rotate)
    if (m_State->GetScaleInvarient()) {
        CMatrix4<float> s;
        s.Identity();
        s(0, 0) = (float)m_State->GetScaleFactor().X();
        s(1, 1) = (float)m_State->GetScaleFactor().Y();
        s(2, 2) = 1.0f;
        mat *= s;
    }

    // Rotate an amount that is proportional to current scaling in 2D
    // projection matrix
    if (m_RotAngleZ != 0.0) {
        mat *= rm;
    }

    // Scale before rotate!
    if (m_RotAngleZ != 0.0f) {
        CMatrix4<float> s;
        s.Identity();
        s(0, 0) = xs;
        s(1, 1) = ys;
        s(2, 2) = 1.0f;
        mat *= s;
    }

    return mat;
}

END_NCBI_SCOPE
