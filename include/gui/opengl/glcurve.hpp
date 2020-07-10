#ifndef GUI_OPENGL___GLCURVE__HPP
#define GUI_OPENGL___GLCURVE__HPP

/*  $Id: glcurve.hpp 31831 2014-11-19 15:04:20Z falkrb $
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
class CGlCurve
{
public:
    typedef ICurve::TPoint TPoint;

    enum ERenderMode {
        eRender_Points,
        eRender_Lines,

        eRender_Default = eRender_Lines
    };

    virtual ~CGlCurve() { }

    /// draw!
    void Draw(ERenderMode mode = eRender_Default) const;

    /// draw a curve with a given error limit, with a minimum number of segs
    void Draw(float error, size_t min_segs,
              ERenderMode mode = eRender_Default) const;

    /// draw into an array and return combined length of segments in buf
    float DrawBuffered(vector<TPoint>& buf) const;

    /// draw (into buffer) curve with an error limit and minimum number of segs
    float DrawBuffered(vector<TPoint>& buf, float error, size_t min_segs) const;

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

private:
    // the curve we render
    Curve m_Curve;

    void x_DrawRecursive(float u_lo, const TPoint& pos_lo,
                         float u_hi, const TPoint& pos_hi,
                         float error) const;

    void x_DrawRecursiveBuffered(vector<TPoint>& buf, 
                                 float& len,
                                 float u_lo, const TPoint& pos_lo,
                                 float u_hi, const TPoint& pos_hi,
                                 float error) const;
}; 


template <class Curve>
inline void
CGlCurve<Curve>::Draw(ERenderMode mode) const
{
    Draw(GetError(), 2, mode);
}


template <class Curve>
inline void
CGlCurve<Curve>::Draw(float error, size_t min_segs, ERenderMode mode) const
{
    float u_step = 1.0f / min_segs;
    float u;
    TPoint pos0 = EvalPos(0.0f);

    static const GLenum modes[] ={
        GL_POINTS,
        GL_LINE_STRIP
    };

    glBegin(modes[mode]);
    glVertex3fv(pos0.GetData());

    for (u = 0;  u < 1.0f;  u += u_step) {
        TPoint pos1 = EvalPos(u + u_step);
        x_DrawRecursive(u, pos0, u+u_step, pos1, error);
        pos0 = pos1;
    }

    glEnd();
}

template <class Curve>
inline float
CGlCurve<Curve>::DrawBuffered(vector<TPoint>& buf) const
{
    return DrawBuffered(buf, GetError(), 2);
}


template <class Curve>
inline float
CGlCurve<Curve>::DrawBuffered(vector<TPoint>& buf, 
                             float error,
                             size_t min_segs) const
{
    float u_step = 1.0f / min_segs;
    float u;
    float len = 0.0f;
    TPoint pos0 = EvalPos(0.0f);

    buf.push_back(pos0);    

    for (u = 0;  u < 1.0f;  u += u_step) {
        TPoint pos1 = EvalPos(u + u_step);
        x_DrawRecursiveBuffered(buf, len, u, pos0, u+u_step, pos1, error);
        pos0 = pos1;
    }

    return len;
}


///
/// internal (recursive) draw function
/// this implements a subdivision with an error level
///
template <class Curve>
inline void
CGlCurve<Curve>::x_DrawRecursive(float u_lo, const TPoint& pos_lo,
                                 float u_hi, const TPoint& pos_hi,
                                 float error) const
{
    /// find the midpoint of the current seg
    float u_mid = (u_lo + u_hi) * 0.5f;
    TPoint pos_mid = EvalPos(u_mid);

    /// find the error associated with this point
    float len0 = (pos_lo - pos_hi).Length();
    float len1 = (pos_mid - pos_lo).Length() + (pos_hi - pos_mid).Length();

    if ((len1 - len0) / len0 > error) {
        /// subdivide!
        x_DrawRecursive(u_lo,  pos_lo,  u_mid, pos_mid, error);
        x_DrawRecursive(u_mid, pos_mid, u_hi,  pos_hi,  error);
    } else {
        /// render!
        glVertex3fv(pos_mid.GetData());
        glVertex3fv(pos_hi.GetData());
    }
}

///
/// internal (recursive) draw function
/// this implements a subdivision with an error level
///
template <class Curve>
inline void
CGlCurve<Curve>::x_DrawRecursiveBuffered(vector<TPoint>& buf,
                                         float& len,
                                         float u_lo, const TPoint& pos_lo,
                                         float u_hi, const TPoint& pos_hi,
                                         float error) const
{
    /// find the midpoint of the current seg
    float u_mid = (u_lo + u_hi) * 0.5f;
    TPoint pos_mid = EvalPos(u_mid);

    /// find the error associated with this point
    float len0 = (pos_lo - pos_hi).Length();
    float len1 = (pos_mid - pos_lo).Length() + (pos_hi - pos_mid).Length();

    if ((len1 - len0) / len0 > error) {
        /// subdivide!
        x_DrawRecursiveBuffered(buf, len, u_lo,  pos_lo,  u_mid, pos_mid, error);
        x_DrawRecursiveBuffered(buf, len, u_mid, pos_mid, u_hi,  pos_hi,  error);
    } else {
        /// buffer!
        len += (pos_mid - buf[buf.size()-1]).Length();
        buf.push_back(pos_mid);
        buf.push_back(pos_hi);
        len += (pos_mid-pos_hi).Length();
    }
}

template <class Curve>
inline void
CGlCurve<Curve>::Recalc()
{
    m_Curve.Recalc();
}


template <class Curve>
inline float
CGlCurve<Curve>::GetError() const
{
    return m_Curve.GetError();
}


template <class Curve>
inline void
CGlCurve<Curve>::SetError(float f)
{
    m_Curve.SetError(f);
}


template <class Curve>
inline const typename CGlCurve<Curve>::TPoint&
CGlCurve<Curve>::GetPoint(size_t i) const
{
    return m_Curve.GetPoint(i);
}


template <class Curve>
inline typename CGlCurve<Curve>::TPoint&
CGlCurve<Curve>::SetPoint(size_t i)
{
    return m_Curve.SetPoint(i);
}


template <class Curve>
inline void
CGlCurve<Curve>::SetPoint(size_t i, const TPoint& point)
{
    m_Curve.SetPoint(i, point);
}


template <class Curve>
inline typename CGlCurve<Curve>::TPoint
CGlCurve<Curve>::EvalPos(float u) const
{
    return m_Curve.EvalPos(u);
}


template <class Curve>
inline typename CGlCurve<Curve>::TPoint
CGlCurve<Curve>::EvalTan(float u) const
{
    return m_Curve.EvalTan(u);
}


template <class Curve>
inline typename CGlCurve<Curve>::TPoint
CGlCurve<Curve>::EvalCurve(float u) const
{
    return m_Curve.EvalCurve(u);
}



END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GLCURVE__HPP
