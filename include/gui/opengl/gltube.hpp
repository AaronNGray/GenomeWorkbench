#ifndef GUI_OPENGL___GLTUBE__HPP
#define GUI_OPENGL___GLTUBE__HPP

/*  $Id: gltube.hpp 14973 2007-09-14 12:11:32Z dicuccio $
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

#include <corelib/ncbistd.hpp>
#include <gui/utils/vect3.hpp>
#include <gui/utils/curve.hpp>
#include <gui/utils/quat.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

///
/// class ICurve defines a basic interface for all curves.  This representation
/// is independent of rendering
///
template <class Curve>
class CGlTube
{
public:
    typedef ICurve::TPoint TPoint;

    enum ERenderMode {
        eRender_Points,
        eRender_Lines,
        eRender_Quads,

        eRender_Default = eRender_Quads
    };

    CGlTube();
    virtual ~CGlTube() { }

    /// draw!
    void Draw(ERenderMode mode = eRender_Default) const;

    /// draw a curve with a given error limit, with a minimum number of segs
    void Draw(float error, size_t min_segs,
              ERenderMode mode = eRender_Default) const;

    /// recalculate the curve (curve-specific)
    void Recalc();

    /// access control points of the curve
    const TPoint& GetPoint(size_t i) const;
    TPoint&       SetPoint(size_t i);
    void          SetPoint(size_t i, const TPoint& );

    //// evaluate the current curve at a given value [0, 1]
    TPoint EvalPos  (float u) const;
    TPoint EvalTan  (float u) const;
    TPoint EvalCurve(float u) const;

    /// access the level of detail
    float GetError() const;
    void  SetError(float f);

    /// non-curve specifics
    float GetStartRadius(void) const;
    void SetStartRadius(float f);

    float GetEndRadius(void) const;
    void SetEndRadius(float f);

    void SetRadius(float f);

    size_t GetNumSpokes(void) const;
    void SetNumSpokes(size_t spokes);

private:
    // the curve we render
    Curve m_Curve;

    // our start/end radii
    float m_Start;
    float m_End;

    // the numberof spokes in each ring
    size_t m_Spokes;

    struct SRingPos {
        TPoint norm;
        TPoint pos;
    };
    typedef vector<SRingPos> TRing;

    void x_DrawRecursive(float u_lo, const TPoint& pos_lo,
                         TPoint& norm_lo, TRing& ring_lo,
                         float u_hi, const TPoint& pos_hi,
                         float error) const;

    // evaluate a ring at a given u value
    void x_GetRing(float u, const TPoint& normal,
                   size_t spokes, float radius, TRing& ring) const;
};


template <class Curve>
inline
CGlTube<Curve>::CGlTube()
    : m_Start(1.0f),
      m_End(1.0f),
      m_Spokes(10)
{
}


template <class Curve>
inline void
CGlTube<Curve>::SetStartRadius(float f)
{
    m_Start = f;
}


template <class Curve>
inline void
CGlTube<Curve>::SetEndRadius(float f)
{
    m_End = f;
}


template <class Curve>
inline void
CGlTube<Curve>::SetRadius(float f)
{
    m_Start = f;
    m_End = f;
}


template <class Curve>
inline float
CGlTube<Curve>::GetStartRadius() const
{
    return m_Start;
}


template <class Curve>
inline float
CGlTube<Curve>::GetEndRadius() const
{
    return m_End;
}


template <class Curve>
inline void
CGlTube<Curve>::Draw(ERenderMode mode) const
{
    Draw(GetError(), 2, mode);
}


template <class Curve>
inline void
CGlTube<Curve>::Draw(float error, size_t min_segs, ERenderMode mode) const
{
    float u_step = 1.0f / min_segs;
    float u;
    TPoint pos0 = EvalPos(0.0f);

    static const GLenum modes[] ={
        GL_POINT,
        GL_LINE,
        GL_FILL
    };

    glPolygonMode(GL_FRONT_AND_BACK, modes[mode]);

    // precompute our first ring
    TPoint norm(0.0f, 1.0f, 0.0f);
    TPoint axis = EvalPos(0.01f) - EvalPos(0.0f);
    axis.Normalize();
    if (norm.Dot(axis) < 0.0001f) {
        norm = TPoint(0.0f, 0.0f, 1.0f);
    }

    norm = axis.Cross(norm);

    TRing ring;
    x_GetRing(0, norm, m_Spokes, m_Start, ring);

    for (u = 0;  u < 1.0f;  u += u_step) {
        TPoint pos1 = EvalPos(u + u_step);
        x_DrawRecursive(u, pos0, norm, ring, u+u_step, pos1, error);
        pos0 = pos1;
    }

    glEnd();
}


///
/// internal (recursive) draw function
/// this implements a subdivision with an error level
///
template <class Curve>
inline void
CGlTube<Curve>::x_DrawRecursive(float u_lo, const TPoint& pos_lo,
                                TPoint& norm_lo, TRing& ring,
                                float u_hi, const TPoint& pos_hi,
                                float error) const
{
    u_hi = min(u_hi, 1.0f);

    /// find the midpoint of the current seg
    float u_mid = (u_lo + u_hi) * 0.5f;
    TPoint pos_mid = EvalPos(u_mid);

    /// find the error associated with this point
    float len0 = (pos_lo - pos_hi).Length();
    float len1 = (pos_mid - pos_lo).Length() + (pos_hi - pos_mid).Length();

    TPoint axis_hi_mid(pos_hi - pos_mid);
    TPoint norm_hi = axis_hi_mid.Cross(norm_lo);
    norm_hi = norm_hi.Cross(axis_hi_mid);

    TPoint norm_mid(norm_hi + norm_lo);
    norm_mid.Normalize();
    norm_hi.Normalize();

    if ((len1 - len0) / len0 > error) {
        /// subdivide!
        x_DrawRecursive(u_lo,  pos_lo,  norm_lo,  ring, u_mid, pos_mid, error);
        x_DrawRecursive(u_mid, pos_mid, norm_mid, ring, u_hi,  pos_hi,  error);
    } else {

        // retrieve a ring for our current position
        TRing next;
        x_GetRing(u_hi, norm_hi, m_Spokes,
                  m_Start + u_hi * (m_End - m_Start),
                  next);

        /**
         * Testing code to draw the normals of our curve
        glBegin(GL_LINES);
            glVertex3fv(pos_lo.GetData());
            glVertex3fv(pos_hi.GetData());

            glVertex3fv(pos_hi.GetData());
            glVertex3fv((pos_hi + norm_hi).GetData());
        glEnd();
        **/

        glBegin(GL_QUAD_STRIP);

        typename TRing::const_iterator this_iter = ring.begin();
        typename TRing::const_iterator next_iter = next.begin();
        for ( ;  this_iter != ring.end();  ++this_iter, ++next_iter) {
            glNormal3fv(this_iter->norm.GetData());
            glVertex3fv(this_iter->pos.GetData());

            glNormal3fv(next_iter->norm.GetData());
            glVertex3fv(next_iter->pos.GetData());
        }

        glEnd();

        ring.swap(next);
    }

    norm_lo = norm_hi;
}


template <class Curve>
inline void
CGlTube<Curve>::x_GetRing(float u, const TPoint& norm,
                          size_t spokes, float radius,
                          TRing& ring) const
{
    float u1 = u + 0.005f;
    if (u1 > 1.0f) {
        u -= 0.005f;
        u1 -= 0.005f;
    }
    TPoint start = EvalPos(u);
    TPoint end   = EvalPos(u1);
    TPoint axis = start - end;
    axis.Normalize();

    CQuat<float> quat(axis, 360.0f / float(spokes));
    TPoint nv(norm);
    nv *= radius;

    ring.clear();
    ring.reserve(spokes + 1);
    for (size_t i =0;  i < spokes;  ++i) {
        SRingPos rp;
        rp.pos = start + nv;
        rp.norm = nv;
        rp.norm.Normalize();

        ring.push_back(rp);
        quat.Rotate(nv);
    }
    ring.push_back(ring.front());
}


template <class Curve>
inline void
CGlTube<Curve>::Recalc()
{
    m_Curve.Recalc();
}


template <class Curve>
inline float
CGlTube<Curve>::GetError() const
{
    return m_Curve.GetError();
}


template <class Curve>
inline void
CGlTube<Curve>::SetError(float f)
{
    m_Curve.SetError(f);
}


template <class Curve>
inline const typename CGlTube<Curve>::TPoint&
CGlTube<Curve>::GetPoint(size_t i) const
{
    return m_Curve.GetPoint(i);
}


template <class Curve>
inline typename CGlTube<Curve>::TPoint&
CGlTube<Curve>::SetPoint(size_t i)
{
    return m_Curve.SetPoint(i);
}


template <class Curve>
inline void
CGlTube<Curve>::SetPoint(size_t i, const TPoint& point)
{
    m_Curve.SetPoint(i, point);
}


template <class Curve>
inline typename CGlTube<Curve>::TPoint
CGlTube<Curve>::EvalPos(float u) const
{
    return m_Curve.EvalPos(u);
}


template <class Curve>
inline typename CGlTube<Curve>::TPoint
CGlTube<Curve>::EvalTan(float u) const
{
    return m_Curve.EvalTan(u);
}


template <class Curve>
inline typename CGlTube<Curve>::TPoint
CGlTube<Curve>::EvalCurve(float u) const
{
    return m_Curve.EvalCurve(u);
}



END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GLTUBE__HPP
