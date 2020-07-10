#ifndef GUI_MATH___INTERSECT__HPP
#define GUI_MATH___INTERSECT__HPP

/*  $Id: intersect.hpp 14973 2007-09-14 12:11:32Z dicuccio $
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

#include <gui/utils/vect3.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(math)


/// enum describing the sorts of expected results from a hit test
enum EIntersectResult {
    eNoIntersection,    ///< no intersection found
    eIntersects,        ///< intersection found
    eCritical           ///< data are critical; intersection can't be determined
};


///
/// IntersectRayTriangle() computes the intersection point of a ray
/// collided with a triangle.
///
/// @param origin
///     the origin of the colliding ray
/// @param dir
///     a direction vector describing the ray.  This must be normalized.
/// @param vert0, vert1, vert2
///     the points of the triangle, given in counterclockwise winding order
/// @param intersect_pt
///     the point of intersection in the triangle, if it is found
/// @param cull_test
///     flag: should we cull hits to the triangle if the hit occurs on the
///     back face of the triangle?
/// @return EIntersecResult
///     describes the relationship of the hit
///
template <class Data>
inline
EIntersectResult IntersectRayTriangle(const CVect3<Data>& origin,
                                      const CVect3<Data>& dir,
                                      const CVect3<Data>& vert0,
                                      const CVect3<Data>& vert1,
                                      const CVect3<Data>& vert2,
                                      CVect3<Data>& intersect_pt,
                                      bool cull_test = false)
{
    /// compute our edge vectors
    CVect3<Data> edge1 = vert1 - vert0;
    CVect3<Data> edge2 = vert2 - vert0;

    /// calculate the determinant for our collision; if the determinant is
    /// below a certain vanishing value, then intersection cannot be reliably
    /// tested.
    CVect3<Data> pvec = dir.Cross(edge2);
    Data determinant = edge1.Dot(pvec);

    if (cull_test) {
        if (determinant < math::epsilon) {
            return eNoIntersection;
        }
    } else {
        if (determinant >= -math::epsilon  &&  determinant < math::epsilon) {
            return eCritical;
        }
    }

    CVect3<Data> tvec = origin - vert0;

    /// get the 'u' param of our barycentric pair
    Data u = tvec.Dot(pvec);
    if (u < 0.0  ||  u > determinant) {
        return eNoIntersection;
    }

    /// get the 'v' param of the barycentric pair
    CVect3<Data> qvec = tvec.Cross(edge1);
    Data v = dir.Dot(qvec);
    if (v < 0.0  ||  u + v > determinant) {
        return eNoIntersection;
    }

    /// intersection proven; compute the intersection point
    Data t = edge2.Dot(qvec);
    intersect_pt = origin + dir * t;
    return eIntersects;
}


///
/// IntersectRayQuad() computes the intersection point of a ray
/// collided with a quad.
///
/// @param origin
///     the origin of the colliding ray
/// @param dir
///     a direction vector describing the ray.  This must be normalized.
/// @param vert0, vert1, vert2, vert3
///     the points of the quad, given in counterclockwise winding order
/// @param intersect_pt
///     the point of intersection in the quad, if it is found
/// @param cull_test
///     flag: should we cull hits to the quad if the hit occurs on the
///     back face of the quad?
/// @return EIntersecResult
///     describes the relationship of the hit
///
template <class Data>
inline
EIntersectResult IntersectRayQuad(const CVect3<Data>& origin,
                                  const CVect3<Data>& dir,
                                  const CVect3<Data>& vert0,
                                  const CVect3<Data>& vert1,
                                  const CVect3<Data>& vert2,
                                  const CVect3<Data>& vert3,
                                  CVect3<Data>& intersect_pt,
                                  bool cull_test = false)
{
    EIntersectResult res;

    res = IntersectRayTriangle(origin, dir, vert0, vert1, vert2,
                               intersect_pt, cull_test);
    if (res == eIntersects) {
        return res;
    }

    return IntersectRayTriangle(origin, dir, vert0, vert2, vert3,
                                intersect_pt, cull_test);
}


END_SCOPE(math)
END_NCBI_SCOPE

#endif  // GUI_MATH___INTERSECT__HPP
