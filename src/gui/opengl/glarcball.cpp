/*  $Id: glarcball.cpp 14565 2007-05-18 12:32:01Z dicuccio $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software / database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software / database is freely available
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */


#include <ncbi_pch.hpp>
#include <gui/opengl.h>
#include <gui/opengl/glarcball.hpp>
#include <math.h>

BEGIN_NCBI_SCOPE

//
// default ctor
CGlArcBall::CGlArcBall()
    : m_IsDragging(false),
      m_ScreenX(100),
      m_ScreenY(100),
      m_MouseX(0),
      m_MouseY(0),
      m_Center(0.0, 0.0, 0.0, 0.0),
      m_Radius(5.0),
      m_QuatNow(0.0, 0.0, 0.0, 1.0),
      m_QuatDown(0.0, 0.0, 0.0, 1.0),
      m_QuatDrag(0.0, 0.0, 0.0, 1.0)
{
    m_MatNow.Identity();
}


//
// conversion ctor
CGlArcBall::CGlArcBall(const TVect& center, float radius)
    : m_IsDragging(false),
      m_ScreenX(100),
      m_ScreenY(100),
      m_MouseX(0),
      m_MouseY(0),
      m_Center(0.0, 0.0, 0.0, 0.0),
      m_Radius(1.0),
      m_QuatNow(0.0, 0.0, 0.0, 1.0),
      m_QuatDown(0.0, 0.0, 0.0, 1.0),
      m_QuatDrag(0.0, 0.0, 0.0, 1.0)
{
    m_MatNow.Identity();
    Place(center, radius);
}


//
// dtor
CGlArcBall::~CGlArcBall()
{
}


//
// place the arc world with a point of rotation and a radius
void CGlArcBall::Place(const TVect& center, float radius)
{
    m_Center = center;
    m_Radius = radius;
}


//
// place the arc world with a point of rotation, keeping the current radius
void CGlArcBall::Place(const TVect& center)
{
    m_Center = center;
}

//
// begin a drag
void CGlArcBall::BeginDrag()
{
    m_IsDragging = true;
    m_DragFrom = x_ToSphere(m_MouseX, m_MouseY);
}

//
// end a drag
void CGlArcBall::EndDrag()
{
    m_IsDragging = false;
    m_QuatDown = m_QuatNow;
}

//
//update the arc ball with a given position
void CGlArcBall::Update(int x, int y)
{
    m_MouseX =  2.0f * (float(x) / float(m_ScreenX)) - 1.0f;
    m_MouseY = -2.0f * (float(y) / float(m_ScreenY)) + 1.0f;

    if (m_IsDragging)
    {
        CVect4<float> to = x_ToSphere(m_MouseX, m_MouseY);

        // set the dragging quat
        m_QuatDrag.X() = m_DragFrom.Y() * to.Z() - m_DragFrom.Z() * to.Y();
        m_QuatDrag.Y() = m_DragFrom.Z() * to.X() - m_DragFrom.X() * to.Z();
        m_QuatDrag.Z() = m_DragFrom.X() * to.Y() - m_DragFrom.Y() * to.X();
        m_QuatDrag.W() = m_DragFrom.X() * to.X() +
            m_DragFrom.Y() * to.Y() +
            m_DragFrom.Z() * to.Z();

        // set the current quat
        m_QuatNow = m_QuatDrag * m_QuatDown;

        // convert to a matrix for OpenGL
        m_QuatNow.Conjugate().ToMatrix(m_MatNow);
    }
}


//
// convert 2d screen coordinates to 3d sphere coordinates
CVect4<float> CGlArcBall::x_ToSphere(float x, float y)
{
    CVect4<float> result;

    result.X() = (x - m_Center.X()) / m_Radius;
    result.Y() = (y - m_Center.Y()) / m_Radius;
    float mag = result.X() * result.X() + result.Y() * result.Y();
    if (mag > 1.0)
    {
        float scale = 1.0f / ::sqrt(mag);
        result.X() *= scale;
        result.Y() *= scale;
        result.Z() = 0.0;
    }
    else
    {
        result.Z() = (float)::sqrt(1.0 - mag);
    }
    result.W() = 0.0;

    return result;
}


//
// apply the matrix using OpenGL
void CGlArcBall::ApplyGL() const
{
    glMultMatrixf(m_MatNow.GetData());
}


END_NCBI_SCOPE
