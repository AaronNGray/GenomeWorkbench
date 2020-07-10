/*  $Id: geometry.cpp 24862 2011-12-14 18:43:44Z falkrb $
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
#include <gui/opengl/geometry.hpp>
#include <gui/utils/quat.hpp>
#include <gui/utils/matrix4.hpp>

#include <gui/opengl.h>


BEGIN_NCBI_SCOPE

/*
void CGlGeometry::DrawSphere(const CVect3<float>& center, float radius,
                             int lat_rings, int long_rings)
{
}
*/


void CGlGeometry::DrawCylinder(const CVect3<float>& from,
                               const CVect3<float>& to,
                               float radius,
                               int rings, int slices)
{
    CVect3<float> axis = to - from;
    float axis_len = axis.Length();
    axis /= axis_len;

    // determine the up axis vector
    CVect3<float> up = axis.Cross(CVect3<float>(0.0f, 1.0f, 0.0f));
    float len = up.Length();
    if (len < 1e-5f) {
        up = axis.Cross(CVect3<float>(0.0f, 0.0f, 1.0f));
    }

    up.Normalize();
    up *= radius;

    // create a series of ring points
    // these will be translated along the course of the axis
    CQuat<float> quat(axis, 360.0f / float(slices));

    typedef vector< CVect3<float> > TRingPts;
    TRingPts normals(slices + 1);
    TRingPts ring0_pts(slices + 1);
    TRingPts::iterator iter = ring0_pts.begin();
    TRingPts::iterator norm_iter = normals.begin();
    for ( ;  iter != ring0_pts.end();  ++iter, ++norm_iter) {
        *iter = from + up;
        *norm_iter = up;
        norm_iter->Normalize();

        // rotate for the next
        quat.Rotate(up);
    }

    // make sure back and front are synonymous
    ring0_pts.back() = ring0_pts.front();
    normals.back() = normals.front();

    // create the second ring
    axis *= axis_len / float(rings);
    TRingPts ring1_pts(slices + 1);
    TRingPts::iterator iter0 = ring0_pts.begin();
    TRingPts::iterator iter1 = ring1_pts.begin();
    for (; iter0 != ring0_pts.end();  ++iter0, ++iter1) {
        *iter1 = *iter0 + axis;
    }

    // now, draw!
    for (int ring = 0;  ring < rings;  ++ring) {
        glBegin(GL_QUAD_STRIP);
        TRingPts::iterator iter0 = ring0_pts.begin();
        TRingPts::iterator iter1 = ring1_pts.begin();
        TRingPts::iterator norm_iter = normals.begin();
        for (;  iter0 != ring0_pts.end();  ++iter0, ++iter1, ++norm_iter) {
            glNormal3fv(norm_iter->GetData());
            glVertex3fv(iter1->GetData());
            glVertex3fv(iter0->GetData());
            *iter0 = *iter1 + axis;
        }
        glEnd();
        ring0_pts.swap(ring1_pts);
    }
}


END_NCBI_SCOPE
