/*  $Id: glcamera.cpp 14810 2007-08-13 12:27:15Z dicuccio $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/opengl/glcamera.hpp>


BEGIN_NCBI_SCOPE


//
// default ctor
//
CGlCamera::CGlCamera()
    : m_Type(ePerspective),
      m_NearPlane(0.001f),
      m_FarPlane(300.0f),
      m_LeftPlane(-1.0f),
      m_RightPlane(1.0f),
      m_TopPlane(1.0f),
      m_BottomPlane(-1.0f),
      m_Fov(45.0f),
      m_Aspect(1.0f)
{
}


//
// MakeCurrent()
// This performs our camera (= projection matrix) setup
//
void CGlCamera::MakeCurrent(bool save_matrices)
{
    glMatrixMode(GL_PROJECTION);
    if (save_matrices) {
        glPushMatrix();
    }

    glLoadIdentity();
    switch (m_Type) {
    case ePerspective:
        gluPerspective(m_Fov, m_Aspect, m_NearPlane, m_FarPlane);
        break;

    case eOrtho:
        glOrtho(m_LeftPlane,   m_RightPlane,
                m_BottomPlane, m_TopPlane,
                m_NearPlane,   m_FarPlane);
        break;
    }

    // when finished, return to modelview matrix
    glMatrixMode(GL_MODELVIEW);
}


//
// ReleaseCurrent()
// This is a NULL op, unless the flag passed in is true
//
void CGlCamera::ReleaseCurrent(bool restore_matrices)
{
    if (restore_matrices) {
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
}


//
// accessors
//

void CGlCamera::SetLayout(EType type)
{
    m_Type = type;
}

CGlCamera::EType CGlCamera::GetLayout(void) const
{
    return m_Type;
}


void CGlCamera::SetNearPlane(GLdouble d)
{
    m_NearPlane = d;
}

GLdouble CGlCamera::GetNearPlane(void) const
{
    return m_NearPlane;
}


void CGlCamera::SetFarPlane(GLdouble d)
{
    m_FarPlane = d;
}

GLdouble CGlCamera::GetFarPlane(void) const
{
    return m_FarPlane;
}


void CGlCamera::SetLeftPlane(GLdouble d)
{
    m_LeftPlane = d;
}

GLdouble CGlCamera::GetLeftPlane(void) const
{
    return m_LeftPlane;
}


void CGlCamera::SetRightPlane(GLdouble d)
{
    m_RightPlane = d;
}

GLdouble CGlCamera::GetRightPlane(void) const
{
    return m_RightPlane;
}


void CGlCamera::SetTopPlane(GLdouble d)
{
    m_TopPlane = d;
}

GLdouble CGlCamera::GetTopPlane(void) const
{
    return m_TopPlane;
}


void CGlCamera::SetBottomPlane(GLdouble d)
{
    m_BottomPlane = d;
}

GLdouble CGlCamera::GetBottomPlane(void) const
{
    return m_BottomPlane;
}


void CGlCamera::SetFieldOfView(GLdouble d)
{
    m_Fov = d;
}

GLdouble CGlCamera::GetFieldOfView(void) const
{
    return m_Fov;
}


void CGlCamera::SetAspectRatio(GLdouble d)
{
    m_Aspect = d;
}

GLdouble CGlCamera::GetAspectRatio(void) const
{
    return m_Aspect;
}

void CGlCamera::Init2dDefault()
{
    SetLayout(CGlCamera::eOrtho);
    SetNearPlane  (   0.1f);
    SetFarPlane   (1000.0f);
    SetLeftPlane  ( 100.0f);
    SetRightPlane (-100.0f);
    SetTopPlane   ( 100.0f);
    SetBottomPlane(-100.0f);
}

void CGlCamera::Init3dDefault()
{
    SetLayout(CGlCamera::ePerspective);
    SetNearPlane  (0.1f);
    SetFarPlane   (1000.0f);
    SetFieldOfView(45.0f);
    SetAspectRatio(1);
}

END_NCBI_SCOPE
